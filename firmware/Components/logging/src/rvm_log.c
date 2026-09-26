#include "rvm_log.h"

#include <stdarg.h>
#include <stdio.h>

static const char *RVM_Log_LevelName(RVM_LogLevel level)
{
    static const char *const names[] = {"DEBUG", "INFO", "WARN", "ERROR"};

    if ((unsigned int)level >= (sizeof(names) / sizeof(names[0])))
    {
        return "UNKNOWN";
    }

    return names[level];
}

void RVM_Log_Write(RVM_LogLevel level,
                   const char *module,
                   const char *format,
                   ...)
{
    va_list arguments;

    if ((module == 0) || (format == 0))
    {
        return;
    }

    printf("[%s][%s] ", RVM_Log_LevelName(level), module);
    va_start(arguments, format);
    (void)vprintf(format, arguments);
    va_end(arguments);
    printf("\r\n");
}
