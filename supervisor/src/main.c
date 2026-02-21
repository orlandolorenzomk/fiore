/*
 * main.c — Fiore Supervisor CLI
 * ============================================================
 * Entry point for the Fiore process supervisor. Parses command-line
 * arguments and dispatches to the appropriate lifecycle command.
 *
 * The supervisor manages Spring Boot JAR processes on a Fiore host.
 * Each managed service is represented as a node in a persistent
 * linked-list process table, serialized to disk so that state survives
 * across invocations.
 *
 * Commands
 * --------
 *   start   <name> <jar> [--port <p>] [--restart <policy>]
 *                        [--env <file>] [--log <file>]
 *             Fork and exec a JAR as a detached background process.
 *
 *   stop    <name>
 *             Send SIGTERM, escalating to SIGKILL after a grace period.
 *
 *   restart <name>
 *             Stop then re-launch the service, incrementing its restart counter.
 *
 *   status  [<name>]
 *             Live status for one service, or a formatted table for all.
 *
 *   list
 *             List all registered services with their current running state.
 *
 *   monitor
 *             Check every process once and restart any that are down,
 *             according to their configured restart policy. Intended to
 *             be called periodically (e.g. from cron).
 *
 *   remove  <name>
 *             Stop the service (if running) and remove it from the table.
 *
 * Persistence
 * -----------
 *   The process table is stored as a binary file at state/processes.dat.
 *   It is loaded at startup and written back after every mutating command.
 *
 * Logging
 * -------
 *   Internal supervisor events  →  logs/supervisor.log
 *   Process table operations    →  logs/process_table.log
 *   Managed process output      →  path supplied via --log (per service)
 * ============================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include "process_table.h"
#include "supervisor.h"

/* ------------------------------------------------------------------ */
/* Helpers                                                            */
/* ------------------------------------------------------------------ */

static void usage(const char *argv0) {
    fprintf(stderr,
        "Usage:\n"
        "  %s start   <name> <jar> [--restart never|on-failure|always] [--port <port>] [--env <file>] [--log <file>]\n"
        "  %s stop    <name>\n"
        "  %s restart <name>\n"
        "  %s status  [<name>]\n"
        "  %s list\n"
        "  %s monitor\n"
        "  %s remove  <name>\n",
        argv0, argv0, argv0, argv0, argv0, argv0, argv0);
}

static RestartPolicy parse_policy(const char *s) {
    if (s == NULL)                    return RESTART_ON_FAILURE;
    if (strcmp(s, "never")      == 0) return RESTART_NEVER;
    if (strcmp(s, "always")     == 0) return RESTART_ALWAYS;
    if (strcmp(s, "on-failure") == 0) return RESTART_ON_FAILURE;
    fprintf(stderr, "Unknown restart policy '%s', defaulting to on-failure\n", s);
    return RESTART_ON_FAILURE;
}

static const char *policy_str(RestartPolicy p) {
    switch (p) {
        case RESTART_NEVER:      return "never";
        case RESTART_ON_FAILURE: return "on-failure";
        case RESTART_ALWAYS:     return "always";
    }
    return "unknown";
}

/* Find a node by service name. Returns NULL if not found. */
static ProcessNode *find_by_name(ProcessNode *head, const char *name) {
    for (ProcessNode *n = head; n != NULL; n = n->next) {
        if (strcmp(n->name, name) == 0) return n;
    }
    return NULL;
}

static ProcessNode *make_node(const char *name, const char *path, RestartPolicy policy, const uint16_t port, const char *log_path) {
    ProcessNode *node = calloc(1, sizeof(ProcessNode));
    if (node == NULL) {
        fprintf(stderr, "out of memory\n");
        exit(EXIT_FAILURE);
    }
    strncpy(node->name, name, sizeof(node->name) - 1);
    strncpy(node->path, path, sizeof(node->path) - 1);
    node->restart_policy = policy;
    node->port = port;
    if (log_path != NULL)
        strncpy(node->log_path, log_path, sizeof(node->log_path) - 1);
    return node;
}

/* ------------------------------------------------------------------ */
/* Commands                                                            */
/* ------------------------------------------------------------------ */

static int cmd_start(ProcessNode **head, int argc, char **argv) {
    /* start <name> <jar> [--restart <policy>] [--port <port>] [--env <file>] */
    if (argc < 4) {
        fprintf(stderr, "start: expected <name> <jar>\n");
        return 1;
    }

    const char     *name     = argv[2];
    const char     *jar      = argv[3];
    RestartPolicy  policy   = RESTART_ON_FAILURE;
    uint16_t       port     = 0;
    const char     *env_path = NULL;
    const char     *log_path = NULL;

    for (int i = 4; i < argc - 1; i++) {
        if (strcmp(argv[i], "--restart") == 0) {
            policy = parse_policy(argv[i + 1]);
        } else if (strcmp(argv[i], "--port") == 0) {
            port = (uint16_t) atoi(argv[i + 1]);
        } else if (strcmp(argv[i], "--env") == 0) {
            env_path = argv[i + 1];
        } else if (strcmp(argv[i], "--log") == 0) {
            log_path = argv[i + 1];
        }
    }

    ProcessNode *existing = find_by_name(*head, name);
    if (existing != NULL) {
        supervisor_status(existing); /* refresh live state before checking */
        if (existing->running) {
            fprintf(stderr, "start: service '%s' is already running (pid %d)\n",
                    name, existing->pid);
            return 1;
        }

        /* Service exists but is stopped — update fields and re-launch. */
        strncpy(existing->path, jar, sizeof(existing->path) - 1);
        existing->restart_policy = policy;
        existing->port = port;
        memset(existing->env_path, 0, sizeof(existing->env_path));
        if (env_path != NULL)
            strncpy(existing->env_path, env_path, sizeof(existing->env_path) - 1);
        memset(existing->log_path, 0, sizeof(existing->log_path));
        if (log_path != NULL)
            strncpy(existing->log_path, log_path, sizeof(existing->log_path) - 1);

        if (supervisor_start(existing) != 0) {
            fprintf(stderr, "start: failed to re-launch '%s'\n", name);
            return 1;
        }
        process_table_save(head);
        printf("Started '%s' (pid %d, restart=%s%s%s%s%s)\n",
               name, existing->pid, policy_str(policy),
               env_path ? ", env=" : "",
               env_path ? env_path : "",
               log_path ? ", log=" : "",
               log_path ? log_path : "");
        return 0;
    }

    ProcessNode *node = make_node(name, jar, policy, port, log_path);
    if (env_path != NULL) {
        strncpy(node->env_path, env_path, sizeof(node->env_path) - 1);
    }

    /* Start first so that fork() fills in pid, running, and start_time. */
    if (supervisor_start(node) != 0) {
        fprintf(stderr, "start: failed to launch '%s'\n", name);
        free(node);
        return 1;
    }

    /* Append and persist now that all fields are populated. */
    process_append(head, node, true);

    printf("Started '%s' (pid %d, restart=%s%s%s%s%s)\n",
           name, node->pid, policy_str(policy),
           env_path ? ", env=" : "",
           env_path ? env_path : "",
           log_path ? ", log=" : "",
           log_path ? log_path : "");
    return 0;
}

static int cmd_stop(ProcessNode **head, int argc, char **argv) {
    if (argc < 3) { fprintf(stderr, "stop: expected <name>\n"); return 1; }
    const char *name = argv[2];

    ProcessNode *node = find_by_name(*head, name);
    if (node == NULL) {
        fprintf(stderr, "stop: service '%s' not found\n", name);
        return 1;
    }

    supervisor_stop(node);
    process_table_save(head);

    printf("Stopped '%s'\n", name);
    return 0;
}

static int cmd_restart(ProcessNode **head, int argc, char **argv) {
    if (argc < 3) { fprintf(stderr, "restart: expected <name>\n"); return 1; }
    const char *name = argv[2];

    ProcessNode *node = find_by_name(*head, name);
    if (node == NULL) {
        fprintf(stderr, "restart: service '%s' not found\n", name);
        return 1;
    }

    if (supervisor_restart(node) != 0) {
        fprintf(stderr, "restart: failed for '%s'\n", name);
        return 1;
    }

    process_table_save(head);
    printf("Restarted '%s' (pid %d, restarts=%u)\n", name, node->pid, node->restart_count);
    return 0;
}

static int cmd_status(ProcessNode **head, int argc, char **argv) {
    if (argc >= 3) {
        ProcessNode *node = find_by_name(*head, argv[2]);
        if (node == NULL) {
            fprintf(stderr, "status: service '%s' not found\n", argv[2]);
            return 1;
        }
        int rc = supervisor_status(node);
        process_table_save(head);
        printf("%-20s pid=%-6d %-10s restarts=%-4u port=%hu restart-policy=%s\n",
               node->name, node->pid,
               rc == 0 ? "running" : "stopped",
               node->restart_count,
               node->port,
               policy_str(node->restart_policy));
        return 0;
    }

    /* Status for all. */
    if (*head == NULL) { printf("No services registered.\n"); return 0; }
    printf("%-20s %-8s %-10s %-10s %-6s %s\n", "NAME", "PID", "RUNNING", "RESTARTS", "PORT", "RESTART POLICY");
    printf("%-20s %-8s %-10s %-10s %-6s %s\n", "----", "---", "-------", "--------", "-----", "--------------");
    for (ProcessNode *n = *head; n != NULL; n = n->next) {
        int rc = supervisor_status(n);
        printf("%-20s %-8d %-10s %-10u %-6hu %s\n",
               n->name, n->pid,
               rc == 0 ? "running" : "stopped",
               n->restart_count,
               n->port,
               policy_str(n->restart_policy));
    }
    process_table_save(head);
    return 0;
}

static int cmd_list(ProcessNode **head) {
    if (*head == NULL) { printf("No services registered.\n"); return 0; }
    printf("%-20s %-8s %-10s %-10s %s\n", "NAME", "PID", "RUNNING", "RESTARTS", "RESTART POLICY");
    printf("%-20s %-8s %-10s %-10s %s\n", "----", "---", "-------", "--------", "--------------");
    for (ProcessNode *n = *head; n != NULL; n = n->next) {
        supervisor_status(n); /* refresh running state via kill(pid, 0) */
        printf("%-20s %-8d %-10s %-10u %s\n",
               n->name, n->pid,
               n->running ? "yes" : "no",
               n->restart_count,
               policy_str(n->restart_policy));
    }
    process_table_save(head);
    return 0;
}

static int cmd_monitor(ProcessNode **head) {
    supervisor_monitor_all(head);
    process_table_save(head);
    return 0;
}

static int cmd_remove(ProcessNode **head, int argc, char **argv) {
    if (argc < 3) { fprintf(stderr, "remove: expected <name>\n"); return 1; }
    const char *name = argv[2];

    ProcessNode *node = find_by_name(*head, name);
    if (node == NULL) {
        fprintf(stderr, "remove: service '%s' not found\n", name);
        return 1;
    }

    if (node->running) supervisor_stop(node);
    process_remove(head, node->pid);
    printf("Removed '%s'\n", name);
    return 0;
}

/* ------------------------------------------------------------------ */
/* Entry point                                                        */
/* ------------------------------------------------------------------ */

int main(int argc, char **argv) {
    puts("\n=============================== FIORE SUPERVISOR ===============================\n");
    if (argc < 2) { usage(argv[0]); return 1; }

    for (int i = 0; i < argc; i++) {
        printf("arg[%d] = %s\n", i, argv[i]);
    } puts("");

    ProcessNode *head = NULL;

    process_table_logger_init("logs/process_table.log", false);
    supervisor_init(&head, "logs/supervisor.log", false);

    process_load(&head, PROCESS_PATH);

    const char *cmd = argv[1];

    if      (strcmp(cmd, "start")   == 0) return cmd_start(&head, argc, argv);
    else if (strcmp(cmd, "stop")    == 0) return cmd_stop(&head, argc, argv);
    else if (strcmp(cmd, "restart") == 0) return cmd_restart(&head, argc, argv);
    else if (strcmp(cmd, "status")  == 0) return cmd_status(&head, argc, argv);
    else if (strcmp(cmd, "list")    == 0) return cmd_list(&head);
    else if (strcmp(cmd, "monitor") == 0) return cmd_monitor(&head);
    else if (strcmp(cmd, "remove")  == 0) return cmd_remove(&head, argc, argv);
    else {
        fprintf(stderr, "Unknown command '%s'\n\n", cmd);
        usage(argv[0]);
        return 1;
    }
}
