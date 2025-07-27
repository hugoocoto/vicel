#ifndef DEBUG_H
#define DEBUG_H

#include "common.h"

#define DEBUG_LOG "report.log"

static void
report(char *format, ...)
{
        va_list arg;
        FILE *file = fopen(DEBUG_LOG, "a");
        va_start(arg, format);
        time_t t = time(0);
        char * strt = ctime(&t);
        *strchr(strt, 10) = 0;
        fprintf(file, "[%s] ", strt);
        vfprintf(file, format, arg);
        putc(10, file);
        va_end(arg);
        fclose(file);
}

#endif //! DEBUG_H
