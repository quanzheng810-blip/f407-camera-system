#ifndef RVM_LOG_H
#define RVM_LOG_H

typedef enum
{
    RVM_LOG_LEVEL_DEBUG = 0,
    RVM_LOG_LEVEL_INFO,
    RVM_LOG_LEVEL_WARN,
    RVM_LOG_LEVEL_ERROR
} RVM_LogLevel;

void RVM_Log_Write(RVM_LogLevel level,
                   const char *module,
                   const char *format,
                   ...);

#define RVM_LOG_DEBUG(module, format, ...) \
    RVM_Log_Write(RVM_LOG_LEVEL_DEBUG, module, format, ##__VA_ARGS__)
#define RVM_LOG_INFO(module, format, ...) \
    RVM_Log_Write(RVM_LOG_LEVEL_INFO, module, format, ##__VA_ARGS__)
#define RVM_LOG_WARN(module, format, ...) \
    RVM_Log_Write(RVM_LOG_LEVEL_WARN, module, format, ##__VA_ARGS__)
#define RVM_LOG_ERROR(module, format, ...) \
    RVM_Log_Write(RVM_LOG_LEVEL_ERROR, module, format, ##__VA_ARGS__)

#endif /* RVM_LOG_H */
