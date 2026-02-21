#include "supervisor.h"
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

/* Seconds to wait for SIGTERM before escalating to SIGKILL. */
#define STOP_GRACE_PERIOD 5

/* Module state. */
static Logger      sv_logger;
static bool        sv_logger_ready = false;
static ProcessNode **sv_head       = NULL;

#define SV_LOG(fmt, ...) \
    do { if (sv_logger_ready) logger_write(&sv_logger, fmt, ##__VA_ARGS__); } while (0)

void supervisor_init(ProcessNode **head, const char *logfile_path, bool stdout_enabled) {
    sv_head = head;
    if (logger_init(&sv_logger, logfile_path, stdout_enabled) == 0) {
        sv_logger_ready = true;
    }
    SV_LOG("supervisor_init: supervisor ready");
}

/*
 * Reads a .env file and calls setenv() for each non-empty, non-comment line.
 * Expected format: KEY=VALUE  (no quoting, no export prefix)
 * Lines starting with '#' and blank lines are silently skipped.
 * Called in the child process only.
 */
static void load_env_file(const char *path) {
    FILE *f = fopen(path, "r");
    if (f == NULL) {
        fprintf(stderr, "supervisor_start: could not open env file '%s': %s\n",
                path, strerror(errno));
        return;
    }

    char line[1024];
    while (fgets(line, sizeof(line), f)) {
        /* Strip trailing newline / carriage return. */
        size_t len = strlen(line);
        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r'))
            line[--len] = '\0';

        /* Skip blank lines and comments. */
        if (len == 0 || line[0] == '#') continue;

        /* Must contain '=' to be a valid KEY=VALUE pair. */
        char *eq = strchr(line, '=');
        if (eq == NULL) continue;

        *eq = '\0';
        const char *key   = line;
        const char *value = eq + 1;

        if (setenv(key, value, 1) != 0) {
            fprintf(stderr, "supervisor_start: setenv(%s) failed: %s\n",
                    key, strerror(errno));
        }
    }

    fclose(f);
}

int supervisor_start(ProcessNode *node) {
    if (node == NULL) {
        SV_LOG("supervisor_start: node is NULL");
        return -1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        SV_LOG("supervisor_start: fork failed for '%s': %s", node->name, strerror(errno));
        return -1;
    }

    if (pid == 0) {
        /* Child — detach from the parent's session so it survives the CLI exiting. */
        if (setsid() < 0) {
            fprintf(stderr, "supervisor_start: setsid failed: %s\n", strerror(errno));
            _exit(EXIT_FAILURE);
        }

        /* Redirect stdin/stdout/stderr to /dev/null so the child holds no terminal. */
        int devnull = open("/dev/null", O_RDWR);
        if (devnull >= 0) {
            dup2(devnull, STDIN_FILENO);
            dup2(devnull, STDOUT_FILENO);
            dup2(devnull, STDERR_FILENO);
            if (devnull > STDERR_FILENO) close(devnull);
        }

        /* Load environment variables from the .env file if one is configured. */
        if (node->env_path[0] != '\0') {
            load_env_file(node->env_path);
        }

        /* exec java -jar <path>. Never returns on success. */
        execlp("java", "java", "-jar", node->path, (char *)NULL);
        _exit(EXIT_FAILURE);
    }

    /* Parent — record the new PID. */
    node->pid        = pid;
    node->running    = true;
    node->start_time = time(NULL);

    SV_LOG("supervisor_start: started '%s' (pid %d)", node->name, node->pid);
    return 0;
}

int supervisor_stop(ProcessNode *node) {
    if (node == NULL) {
        SV_LOG("supervisor_stop: node is NULL");
        return -1;
    }

    if (!node->running || node->pid <= 0) {
        SV_LOG("supervisor_stop: '%s' is not running", node->name);
        return -1;
    }

    SV_LOG("supervisor_stop: sending SIGTERM to '%s' (pid %d)", node->name, node->pid);

    if (kill(node->pid, SIGTERM) != 0) {
        SV_LOG("supervisor_stop: kill(SIGTERM) failed for '%s': %s",
               node->name, strerror(errno));
        return -1;
    }

    /* Wait up to STOP_GRACE_PERIOD seconds for clean exit. */
    time_t deadline = time(NULL) + STOP_GRACE_PERIOD;
    int    status;
    while (time(NULL) < deadline) {
        pid_t result = waitpid(node->pid, &status, WNOHANG);
        if (result == node->pid) {
            node->running = false;
            SV_LOG("supervisor_stop: '%s' exited cleanly", node->name);
            return 0;
        }
        sleep(1);
    }

    /* Grace period elapsed — escalate. */
    SV_LOG("supervisor_stop: grace period elapsed, sending SIGKILL to '%s' (pid %d)",
           node->name, node->pid);
    kill(node->pid, SIGKILL);
    waitpid(node->pid, &status, 0);
    node->running = false;
    return 0;
}

int supervisor_restart(ProcessNode *node) {
    if (node == NULL) {
        SV_LOG("supervisor_restart: node is NULL");
        return -1;
    }

    SV_LOG("supervisor_restart: restarting '%s'", node->name);

    if (node->running) {
        if (supervisor_stop(node) != 0) {
            SV_LOG("supervisor_restart: stop failed for '%s'", node->name);
            return -1;
        }
    }

    if (supervisor_start(node) != 0) {
        SV_LOG("supervisor_restart: start failed for '%s'", node->name);
        return -1;
    }

    node->restart_count++;
    SV_LOG("supervisor_restart: '%s' restarted (restart #%u)",
           node->name, node->restart_count);
    return 0;
}

int supervisor_status(ProcessNode *node) {
    if (node == NULL) {
        SV_LOG("supervisor_status: node is NULL");
        return -1;
    }

    /* kill(pid, 0) checks existence without sending a signal. */
    if (kill(node->pid, 0) == 0) {
        node->running = true;
        SV_LOG("supervisor_status: '%s' (pid %d) is running — restarts: %u, uptime: %lds",
               node->name, node->pid, node->restart_count,
               (long)(time(NULL) - node->start_time));
        return 0;
    }

    /* ESRCH means no such process. */
    node->running = false;
    SV_LOG("supervisor_status: '%s' (pid %d) is NOT running", node->name, node->pid);
    return 1;
}

void supervisor_monitor_all(ProcessNode **head) {
    if (head == NULL || *head == NULL) {
        SV_LOG("supervisor_monitor_all: process table is empty");
        return;
    }

    SV_LOG("supervisor_monitor_all: checking all processes");

    for (ProcessNode *node = *head; node != NULL; node = node->next) {
        int alive = supervisor_status(node);

        if (alive == 0) {
            /* Process is healthy — nothing to do. */
            continue;
        }

        /* Process is dead — apply restart policy. */
        switch (node->restart_policy) {
            case RESTART_NEVER:
                SV_LOG("supervisor_monitor_all: '%s' is down, policy=never, not restarting",
                       node->name);
                break;

            case RESTART_ON_FAILURE:
                SV_LOG("supervisor_monitor_all: '%s' is down, policy=on-failure, restarting",
                       node->name);
                supervisor_restart(node);
                break;

            case RESTART_ALWAYS:
                SV_LOG("supervisor_monitor_all: '%s' is down, policy=always, restarting",
                       node->name);
                supervisor_restart(node);
                break;
        }
    }
}
