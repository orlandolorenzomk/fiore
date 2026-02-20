#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "process_table.h"

static ProcessNode *make_node(const char *name, const char *path, pid_t pid, RestartPolicy policy) {
    ProcessNode *node = calloc(1, sizeof(ProcessNode));
    if (node == NULL) {
        fprintf(stderr, "make_node: out of memory\n");
        exit(EXIT_FAILURE);
    }
    strncpy(node->name, name, sizeof(node->name) - 1);
    strncpy(node->path, path, sizeof(node->path) - 1);
    node->pid            = pid;
    node->restart_policy = policy;
    node->running        = true;
    node->start_time     = time(NULL);
    node->restart_count  = 0;
    return node;
}

static void print_list(ProcessNode *head) {
    if (head == NULL) {
        printf("  (empty)\n");
        return;
    }
    for (ProcessNode *n = head; n != NULL; n = n->next) {
        printf("  [%s] pid=%-6d policy=%d running=%d restarts=%u\n",
               n->name, n->pid, n->restart_policy, n->running, n->restart_count);
    }
}

int main(void) {
    process_table_logger_init("logs/process_table.log", true);

    ProcessNode *head = NULL;

    /* --- append --- */
    printf("=== append ===\n");
    process_append(&head, make_node("api-service",   "/opt/api.jar",   1001, RESTART_ON_FAILURE), true);
    process_append(&head, make_node("cache-service",  "/opt/cache.jar", 1002, RESTART_ALWAYS),     true);
    process_append(&head, make_node("worker-service", "/opt/worker.jar",1003, RESTART_NEVER),      true);
    print_list(head);

    /* --- find --- */
    printf("\n=== find ===\n");
    printf("  find pid 1002: %s\n", process_find(&head, 1002) ? "found" : "not found");
    printf("  find pid 9999: %s\n", process_find(&head, 9999) ? "found" : "not found");

    /* --- remove --- */
    printf("\n=== remove pid 1002 ===\n");
    process_remove(&head, 1002);
    print_list(head);

    /* --- persist and reload --- */
    printf("\n=== reload from %s ===\n", PROCESS_PATH);
    ProcessNode *loaded = NULL;
    process_load(&loaded, PROCESS_PATH);
    print_list(loaded);

    return EXIT_SUCCESS;
}
