// logger.c

#include "logger.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <pthread.h>

static FILE* log_file = NULL;
static pthread_mutex_t logLock = PTHREAD_MUTEX_INITIALIZER;

void init_logger(const char* logFilePath) {
    log_file = fopen(logFilePath, "a");
    if (!log_file) {
        perror("Error opening log file");
        exit(EXIT_FAILURE);
    }
}

void cleanup_logger() {
    fclose(log_file);
}

void log_message(LogLevel level, const char* message, ...) {
    va_list args;
    va_start(args, message);

    pthread_mutex_lock(&logLock);

    switch (level) {
        case LOG_INFO:
            fprintf(stdout, "[INFO] ");
            vfprintf(stdout, message, args);
            break;
        case LOG_WARNING:
            fprintf(stdout, "[WARNING] ");
            vfprintf(stdout, message, args);
            break;
        case LOG_ERROR:
            fprintf(stdout, "[ERROR] ");
            vfprintf(stdout, message, args);
            break;
    }

    vfprintf(log_file, message, args);

    pthread_mutex_unlock(&logLock);

    va_end(args);
}
