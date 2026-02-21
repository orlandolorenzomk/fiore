#ifndef PROCESS_TABLE_H
#define PROCESS_TABLE_H

#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>
#include <time.h>
#include "logger.h"

/** @brief Path to the binary file used to persist the process table across runs. */
#define PROCESS_PATH "state/processes.dat"

/**
 * @brief Defines when the supervisor should attempt to restart a managed process.
 */
typedef enum {
    RESTART_NEVER      = 0, /* Never restart the process after it exits. */
    RESTART_ON_FAILURE = 1, /* Restart the process only if it exits with a non-zero status. */
    RESTART_ALWAYS     = 2  /* Always restart the process regardless of exit status. */
} RestartPolicy;

/**
 * @brief A node in the singly-linked process table.
 *
 * Each node represents a single managed Spring Boot process and holds
 * all runtime state required for supervision, monitoring, and persistence.
 */
typedef struct ProcessNode {
    char          name[64];       /* Human-readable service name. */
    char          path[256];      /* Absolute path to the application JAR. */
    char          env_path[256];  /* Absolute path of the .env associated to the JAR */        
    pid_t         pid;            /* OS process ID assigned at launch. */
    RestartPolicy restart_policy; /* Restart behaviour applied when the process exits. */
    uint32_t      restart_count;  /* Number of times the process has been restarted. */
    bool          running;        /* Whether the process is currently alive. */
    time_t        start_time;     /* Unix timestamp of the most recent process start. */
    struct ProcessNode *next;     /* Pointer to the next node in the list, or NULL. */
} ProcessNode;

/**
 * @brief Loads the process table from a binary file into a linked list.
 *
 * Reads all serialized @ref ProcessNode records from @p path and appends
 * them to the list at @p head. If the file does not exist, the function
 * returns @c true without modifying the list.
 *
 * @param head  Address of the list head pointer. Must not be NULL.
 * @param path  Path to the binary state file to read from. Must not be NULL.
 * @return      @c true on success or if the file does not yet exist,
 *              @c false if a memory allocation or read error occurs.
 */
bool process_load(ProcessNode **head, const char *path);

/**
 * @brief Appends a new node to the tail of the process table.
 *
 * Inserts @p new_node at the end of the linked list. If @p fsave is
 * @c true, the entire list is immediately serialized to @ref PROCESS_PATH.
 *
 * @param head      Address of the list head pointer. Must not be NULL.
 * @param new_node  The node to append. Must not be NULL. Ownership is
 *                  transferred to the list.
 * @param fsave     If @c true, persist the updated list to disk.
 * @return          @c true on success, @c false if any argument is invalid.
 */
bool process_append(ProcessNode **head, ProcessNode *new_node, bool fsave);

/**
 * @brief Removes the node with the given PID from the process table.
 *
 * Finds the first node whose @c pid matches @p pid, unlinks it from the
 * list, frees its memory, and persists the updated list to disk.
 *
 * @param head  Address of the list head pointer. Must not be NULL.
 * @param pid   PID of the process to remove.
 * @return      @c true if the node was found and removed,
 *              @c false if the list is empty or no matching node exists.
 */
bool process_remove(ProcessNode **head, pid_t pid);

/**
 * @brief Checks whether a process with the given PID exists in the table.
 *
 * @param head  Address of the list head pointer.
 * @param pid   PID to search for.
 * @return      @c true if a node with @p pid was found, @c false otherwise.
 */
bool process_find(ProcessNode **head, pid_t pid);

/**
 * @brief Persists the current in-memory process table to @ref PROCESS_PATH.
 *
 * Useful after in-place mutation of node fields (e.g. marking a process
 * as stopped) without needing to remove and re-insert the node.
 *
 * @param head  Address of the list head pointer. Must not be NULL.
 */
void process_table_save(ProcessNode **head);

/**
 * @brief Initialises the module-level logger used by all process table functions.
 *
 * Must be called before any other process table function if logging to a file
 * or stdout is desired. If never called, all diagnostic output is silently
 * discarded.
 *
 * @param logfile_path   Path to the log file opened in append mode. May be NULL
 *                       to disable file logging.
 * @param stdout_enabled If @c true, log messages are also written to stdout.
 * @return               0 on success, -1 if the log file could not be opened.
 */
int process_table_logger_init(const char *logfile_path, bool stdout_enabled);

#endif // PROCESS_TABLE_H
