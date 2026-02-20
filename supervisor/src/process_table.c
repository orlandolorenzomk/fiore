#include "process_table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Module-level logger — initialised via process_table_logger_init(). */
static Logger pt_logger;
static bool   pt_logger_ready = false;

/* Internal helper: write to the module logger when it is ready. */
#define PT_LOG(fmt, ...) \
    do { if (pt_logger_ready) logger_write(&pt_logger, fmt, ##__VA_ARGS__); } while (0)

/* On-disk record layout — excludes the linked-list pointer. */
typedef struct {
    char          name[64];
    char          path[256];
    pid_t         pid;
    RestartPolicy restart_policy;
    bool          running;
    time_t        start_time;
    uint32_t      restart_count;
} ProcessRecord;

int process_table_logger_init(const char *logfile_path, bool stdout_enabled) {
    int rc = logger_init(&pt_logger, logfile_path, stdout_enabled);
    if (rc == 0) {
        pt_logger_ready = true;
    }
    return rc;
}

static void file_update_content(ProcessNode **head) {
    FILE *fptr = fopen(PROCESS_PATH, "wb");
    if (fptr == NULL) {
        PT_LOG("file_update_content: could not open %s for writing", PROCESS_PATH);
        return;
    }

    ProcessNode *current = *head;
    while (current != NULL) {
        ProcessRecord record;
        memset(&record, 0, sizeof(record));

        strncpy(record.name, current->name, sizeof(record.name) - 1);
        strncpy(record.path, current->path, sizeof(record.path) - 1);
        record.pid            = current->pid;
        record.restart_policy = current->restart_policy;
        record.running        = current->running;
        record.start_time     = current->start_time;
        record.restart_count  = current->restart_count;

        if (fwrite(&record, sizeof(ProcessRecord), 1, fptr) != 1) {
            PT_LOG("file_update_content: failed to write record for %s", current->name);
        }

        current = current->next;
    }

    fclose(fptr);
}

bool process_load(ProcessNode **head, const char *path) {
    if (head == NULL) {
        PT_LOG("process_load: head argument is NULL");
        exit(EXIT_FAILURE);
    }

    if (path == NULL) {
        PT_LOG("process_load: path argument is NULL");
        exit(EXIT_FAILURE);
    }

    FILE *fptr = fopen(path, "rb");
    if (fptr == NULL) {
        /* No file yet — nothing to load, not an error. */
        return true;
    }

    ProcessRecord record;
    while (fread(&record, sizeof(ProcessRecord), 1, fptr) == 1) {
        ProcessNode *node = calloc(1, sizeof(ProcessNode));
        if (node == NULL) {
            PT_LOG("process_load: out of memory");
            fclose(fptr);
            return false;
        }

        strncpy(node->name, record.name, sizeof(node->name) - 1);
        strncpy(node->path, record.path, sizeof(node->path) - 1);
        node->pid            = record.pid;
        node->restart_policy = record.restart_policy;
        node->running        = record.running;
        node->start_time     = record.start_time;
        node->restart_count  = record.restart_count;
        node->next           = NULL;

        if (!process_append(head, node, false)) {
            free(node);
            fclose(fptr);
            return false;
        }
    }

    fclose(fptr);
    PT_LOG("process_load: loaded processes from %s", path);
    return true;
}

bool process_append(ProcessNode **head, ProcessNode *new_node, bool fsave) {
    if (head == NULL) {
        PT_LOG("process_append: head argument is NULL");
        return false;
    }

    if (new_node == NULL) {
        PT_LOG("process_append: new_node argument is NULL");
        return false;
    }

    new_node->next = NULL;

    if (*head == NULL) {
        *head = new_node;
    } else {
        ProcessNode *current = *head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_node;
    }

    PT_LOG("process_append: appended '%s' (pid %d)", new_node->name, new_node->pid);

    if (fsave) {
        file_update_content(head);
    }

    return true;
}

bool process_remove(ProcessNode **head, pid_t pid) {
    if (head == NULL || *head == NULL) {
        PT_LOG("process_remove: list is empty or head is NULL");
        return false;
    }

    ProcessNode *current  = *head;
    ProcessNode *previous = NULL;

    while (current != NULL) {
        if (current->pid == pid) {
            if (previous == NULL) {
                *head = current->next;
            } else {
                previous->next = current->next;
            }
            PT_LOG("process_remove: removed '%s' (pid %d)", current->name, current->pid);
            free(current);
            file_update_content(head);
            return true;
        }
        previous = current;
        current  = current->next;
    }

    PT_LOG("process_remove: no process found with pid %d", pid);
    return false;
}

bool process_find(ProcessNode **head, pid_t pid) {
    if (head == NULL || *head == NULL) {
        return false;
    }

    ProcessNode *current = *head;
    while (current != NULL) {
        if (current->pid == pid) {
            PT_LOG("process_find: found '%s' (pid %d)", current->name, current->pid);
            return true;
        }
        current = current->next;
    }

    PT_LOG("process_find: pid %d not found", pid);
    return false;
}
