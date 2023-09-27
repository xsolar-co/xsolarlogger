// logger.h

#ifndef LOGGER_H
#define LOGGER_H

typedef enum {
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR
} LogLevel;

void init_logger(const char* logFilePath);
void cleanup_logger();
void log_message(LogLevel level, const char* message, ...);

#endif // LOGGER_H
