#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include <stdbool.h>

/**
 * @brief Holds the state for a single logger instance.
 *
 * A Logger can write timestamped messages to a file, to stdout, or both.
 * Initialise with @ref logger_init before use and release resources with
 * @ref logger_close when done.
 */
typedef struct Logger {
    char logfile_path[256]; /**< Absolute or relative path to the log file. */
    FILE *logfile;          /**< Open file handle, or NULL if file logging is disabled. */
    bool stdout_enabled;    /**< When @c true, messages are also written to stdout. */
} Logger;

/**
 * @brief Initialises a Logger instance and opens the log file for appending.
 *
 * If @p logfile_path is NULL or an empty string, file logging is disabled and
 * messages are only written to stdout when @p stdout_enabled is @c true.
 *
 * @param logger          Pointer to an uninitialised Logger struct. Must not be NULL.
 * @param logfile_path    Path to the log file to open in append mode. May be NULL.
 * @param stdout_enabled  If @c true, log messages are also printed to stdout.
 * @return                0 on success, -1 if the log file could not be opened.
 */
int logger_init(Logger *logger, const char *logfile_path, bool stdout_enabled);

/**
 * @brief Writes a formatted, timestamped message to the configured outputs.
 *
 * Uses printf-style format string and variadic arguments. Each message is
 * prefixed with an ISO-8601 timestamp. Output is written to the log file
 * and/or stdout depending on how the logger was initialised.
 *
 * @param logger  Pointer to an initialised Logger. Must not be NULL.
 * @param format  printf-compatible format string. Must not be NULL.
 * @param ...     Variadic arguments matching the format string.
 * @return        0 on success, -1 if the logger is uninitialised or a write fails.
 */
int logger_write(Logger *logger, const char *format, ...);

/**
 * @brief Flushes the log file's write buffer to disk.
 *
 * Has no effect if file logging is disabled.
 *
 * @param logger  Pointer to an initialised Logger. Must not be NULL.
 * @return        0 on success, -1 on flush failure.
 */
int logger_flush(Logger *logger);

/**
 * @brief Flushes and closes the log file handle.
 *
 * After this call the Logger must not be used until re-initialised with
 * @ref logger_init. Has no effect on the file handle if it is already NULL.
 *
 * @param logger  Pointer to an initialised Logger. Must not be NULL.
 * @return        0 on success, -1 if closing the file fails.
 */
int logger_close(Logger *logger);

#endif // LOGGER_H
