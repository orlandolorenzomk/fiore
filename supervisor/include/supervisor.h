#ifndef SUPERVISOR_H
#define SUPERVISOR_H

#include "process_table.h"
#include "logger.h"

/**
 * @brief Initialises the supervisor module.
 *
 * Must be called once before any other supervisor function. Stores a
 * reference to the process table and sets up the module-level logger.
 *
 * @param head           Address of the process table head pointer.
 * @param logfile_path   Path to the log file opened in append mode. May be NULL.
 * @param stdout_enabled If @c true, log messages are also written to stdout.
 */
void supervisor_init(ProcessNode **head, const char *logfile_path, bool stdout_enabled);

/**
 * @brief Launches the process described by @p node.
 *
 * Forks a child process and executes `java -jar <node->path>`. On success
 * the node's @c pid, @c running, and @c start_time fields are updated.
 *
 * @param node  Process node to start. Must not be NULL.
 * @return      0 on success, -1 on fork or exec failure.
 */
int supervisor_start(ProcessNode *node);

/**
 * @brief Sends SIGTERM to the process and waits for it to exit.
 *
 * Marks @c node->running as @c false after the process terminates.
 * If the process does not exit within a grace period, SIGKILL is sent.
 *
 * @param node  Process node to stop. Must not be NULL.
 * @return      0 on success, -1 if the process could not be signalled.
 */
int supervisor_stop(ProcessNode *node);

/**
 * @brief Stops then restarts a process, incrementing its restart counter.
 *
 * Calls @ref supervisor_stop followed by @ref supervisor_start and
 * increments @c node->restart_count on a successful restart.
 *
 * @param node  Process node to restart. Must not be NULL.
 * @return      0 on success, -1 if stop or start fails.
 */
int supervisor_restart(ProcessNode *node);

/**
 * @brief Checks whether the process is alive and logs its current status.
 *
 * Uses @c kill(pid, 0) to probe liveness without sending a signal.
 * Updates @c node->running to reflect the actual state.
 *
 * @param node  Process node to inspect. Must not be NULL.
 * @return      0 if the process is running, 1 if it is not, -1 on error.
 */
int supervisor_status(ProcessNode *node);

/**
 * @brief Iterates the process table and enforces restart policies.
 *
 * For each node, checks liveness and, if the process is dead, applies
 * its @c restart_policy: restarts on failure or always as configured.
 * Intended to be called periodically from a monitoring loop.
 */
void supervisor_monitor_all(void);

#endif // SUPERVISOR_H
