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
/* FLAG.H
 *
 * About: Just a small file to handle flags.
 * Author: Hugo Coto Florez
 *
 * void flag_set(int *argc, char **argv[])
 * flag_init flags. Needed to use following functions.
 *
 * int flag_get_value(char **value, char* flag1 [, char* flag2 [, ...]])
 * int flag_get(char* flag1 [, char* flag2 [, ...]])
 * Both funtions return 0 if any flag is set. Otherwise, 1 is returned (and
 * value is set to the next argument after the flag, if using the value func).
 */

/* TODO: Accept name=value flags. Now it read name but is not able to get value */

#include "flag.h"
#include "common.h"

/* Flag context */
static int *flag_argc;
static char ***flag_argv;
static int flag_init = 0;
static const char *empty_string = "";

void
flag_set(int *argc, char **argv[])
{
        flag_argc = argc;
        flag_argv = argv;
        flag_init = 1;
}

void
__flag_check_flag_init()
{
        if (flag_init != 0) return;
        printf("ERROR: Bad usage of flag.h! Dont forget to call `flag_set()`.\n");
        abort();
}

int
__flag_get_value(char **value, char *flag_name)
{
        /* This function is not fast at all but I thint it does not matter */
        int i;
        __flag_check_flag_init();
        for (i = 0; i < *flag_argc - 1; i++) {
                if (!strncmp((*flag_argv)[i], flag_name, strlen((*flag_argv)[i]))) {
                        *value = (*flag_argv)[i + 1];
                        memmove((*flag_argv) + i, (*flag_argv) + i + 2,
                                (*flag_argc - (i + 1)) * sizeof(char *));
                        *flag_argc -= 2;
                        return 1;
                }
        }

        /* This is for the case where a flag that expects a name and a value is
         * in the last position, so the value can not be found. */
        if (!strcmp((*flag_argv)[*flag_argc - 1], flag_name)) {
                *value = (char *) empty_string;
                (*flag_argv)[*flag_argc - 1] = NULL;
                *flag_argc -= 1;
                return 1;
        }
        return 0;
}

int
__flag_get(char *flag_name)
{
        int i;
        __flag_check_flag_init();
        for (i = 0; i < *flag_argc; i++) {
                if (!strcmp((*flag_argv)[i], flag_name)) {
                        memmove((*flag_argv) + i, (*flag_argv) + i + 1,
                                (*flag_argc - (i)) * sizeof(char *));
                        --*flag_argc;
                        return 1;
                }
        }
        return 0;
}


int
__flag_get_value_v(char **value, ...)
{
        va_list flag_ap;
        char *current_flag;
        va_start(flag_ap, value);
        while ((current_flag = va_arg(flag_ap, char *))) {
                if (__flag_get_value(value, current_flag)) {
                        return 1;
                }
        }
        return 0;
}

int
__flag_get_v(char *flag1, ...)
{
        va_list flag_ap;
        va_start(flag_ap, flag1);
        char *current_flag = flag1;
        do {
                if (__flag_get(current_flag)) {
                        return 1;
                }
        } while ((current_flag = va_arg(flag_ap, char *)));
        return 0;
}
