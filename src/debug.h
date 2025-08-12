/*
 * VICEL - Visual Cell editor
 * Copyright (C) 2025  Hugo Coto Florez
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * For questions or support, contact: hugo.coto@member.fsf.org
 */

#ifndef DEBUG_H
#define DEBUG_H

#include "common.h"

enum ErrorCode {
        ERR_NONE = 0,
        ERR_CELFROMID,
        ERR_GETLITERAL,
        ERR_EXPECT,
        ERR_PARSECOORDS,
        ERR_INVBODY,
        ERR_REPAST,
        ERR_INVFORM,
        ERR_OBSVAL,
        ERR_STDIN,
        ERR_INVRANGE,
};

#define DEBUG_LOG "report.log"

extern int debug_level;

#if defined(DEBUG) && DEBUG
static void
report(char *format, ...)
{
        if (debug_level == 0) return;
        va_list arg;
        FILE *file = fopen(DEBUG_LOG, "a");
        va_start(arg, format);
        time_t t = time(0);
        char *strt = ctime(&t);
        *strchr(strt, 10) = 0;
        fprintf(file, "[%s] ", strt);
        vfprintf(file, format, arg);
        putc(10, file);
        va_end(arg);
        fclose(file);
}

#else
#define report(...)
#endif

#endif //! DEBUG_H
