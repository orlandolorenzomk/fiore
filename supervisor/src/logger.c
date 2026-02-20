#include "logger.h"
#include <stdarg.h>
#include <string.h>
#include <time.h>

/* Maximum length of one formatted log line (excluding timestamp prefix). */
#define LOG_BUF_SIZE 1024

/* Writes an ISO-8601 timestamp ("YYYY-MM-DD HH:MM:SS") into buf. */
static void timestamp(char *buf, size_t size) {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    strftime(buf, size, "%Y-%m-%d %H:%M:%S", tm_info);
}

int logger_init(Logger *logger, const char *logfile_path, bool stdout_enabled) {
    if (logger == NULL) {
        return -1;
    }

    memset(logger, 0, sizeof(Logger));
    logger->stdout_enabled = stdout_enabled;
    logger->logfile        = NULL;

    if (logfile_path != NULL && logfile_path[0] != '\0') {
        strncpy(logger->logfile_path, logfile_path, sizeof(logger->logfile_path) - 1);

        logger->logfile = fopen(logfile_path, "a");
        if (logger->logfile == NULL) {
            fprintf(stderr, "logger_init: could not open log file '%s'\n", logfile_path);
            return -1;
        }
    }

    return 0;
}

int logger_write(Logger *logger, const char *format, ...) {
    if (logger == NULL || format == NULL) {
        return -1;
    }

    char ts[32];
    timestamp(ts, sizeof(ts));

    char message[LOG_BUF_SIZE];
    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);

    if (logger->logfile != NULL) {
        if (fprintf(logger->logfile, "[%s] %s\n", ts, message) < 0) {
            return -1;
        }
    }

    if (logger->stdout_enabled) {
        printf("[%s] %s\n", ts, message);
    }

    return 0;
}

int logger_flush(Logger *logger) {
    if (logger == NULL) {
        return -1;
    }

    if (logger->logfile != NULL) {
        if (fflush(logger->logfile) != 0) {
            return -1;
        }
    }

    return 0;
}

int logger_close(Logger *logger) {
    if (logger == NULL) {
        return -1;
    }

    if (logger->logfile != NULL) {
        logger_flush(logger);
        if (fclose(logger->logfile) != 0) {
            return -1;
        }
        logger->logfile = NULL;
    }

    return 0;
}
