// logger.h

#ifndef LOGGER_H
#define LOGGER_H

typedef enum {
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR
} LogLevel;

void initLogger(const char* logFilePath);
void cleanupLogger();
void logMessage(LogLevel level, const char* message, ...);

#endif // LOGGER_H
