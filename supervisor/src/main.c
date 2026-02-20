#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "process_table.h"
#include "supervisor.h"

static ProcessNode *make_node(const char *name, const char *path, RestartPolicy policy) {
    ProcessNode *node = calloc(1, sizeof(ProcessNode));
    if (node == NULL) {
        fprintf(stderr, "make_node: out of memory\n");
        exit(EXIT_FAILURE);
    }
    strncpy(node->name, name, sizeof(node->name) - 1);
    strncpy(node->path, path, sizeof(node->path) - 1);
    node->restart_policy = policy;
    node->running        = false;
    node->start_time     = 0;
    node->restart_count  = 0;
    return node;
}

static void print_list(ProcessNode *head) {
    if (head == NULL) {
        printf("  (empty)\n");
        return;
    }
    for (ProcessNode *n = head; n != NULL; n = n->next) {
        printf("  [%-16s] pid=%-6d running=%d restarts=%u\n",
               n->name, n->pid, n->running, n->restart_count);
    }
}

int main(void) {
    ProcessNode *head = NULL;

    /* Initialise both modules with a shared log destination. */
    process_table_logger_init("logs/process_table.log", true);
    supervisor_init(&head, "logs/supervisor.log", true);

    /* --- build process table --- */
    printf("\n=== loading table ===\n");
    process_append(&head, make_node("spring-boot-test",
        "/root/spring-boot-test-application/target/spring-boot-test-application-1.0-SNAPSHOT.jar",
        RESTART_ON_FAILURE), true);
    print_list(head);

    /* --- start each service --- */
    printf("\n=== start ===\n");
    for (ProcessNode *n = head; n != NULL; n = n->next) {
        int rc = supervisor_start(n);
        printf("  start '%s': %s\n", n->name, rc == 0 ? "ok" : "failed");
    }
    print_list(head);

    /* Give processes a moment to settle. */
    sleep(1);

    /* --- status check --- */
    printf("\n=== status ===\n");
    for (ProcessNode *n = head; n != NULL; n = n->next) {
        int rc = supervisor_status(n);
        printf("  status '%s': %s\n", n->name,
               rc == 0 ? "running" : rc == 1 ? "stopped" : "error");
    }

    /* --- monitor pass (applies restart policies to any dead processes) --- */
    printf("\n=== monitor_all ===\n");
    supervisor_monitor_all();

    /* --- restart one service --- */
    printf("\n=== restart spring-boot-test ===\n");
    for (ProcessNode *n = head; n != NULL; n = n->next) {
        if (strcmp(n->name, "spring-boot-test") == 0) {
            int rc = supervisor_restart(n);
            printf("  restart '%s': %s (restarts=%u)\n",
                   n->name, rc == 0 ? "ok" : "failed", n->restart_count);
            break;
        }
    }

    /* --- stop all --- */
    printf("\n=== stop all ===\n");
    for (ProcessNode *n = head; n != NULL; n = n->next) {
        int rc = supervisor_stop(n);
        printf("  stop '%s': %s\n", n->name, rc == 0 ? "ok" : "failed/not running");
    }
    print_list(head);

    return EXIT_SUCCESS;
}

