/* FLAG.H
 *
 * About: Just a small file to handle flags.
 * Author: Hugo Coto Florez
 *
 * void flag_set(int *argc, char **argv[])
 * Init flags. Needed to use following functions.
 *
 * int flag_get_value(char **value, char* flag1 [, char* flag2 [, ...]])
 * int flag_get(char* flag1 [, char* flag2 [, ...]])
 * Both funtions return 0 if any flag is set. Otherwise, 1 is returned (and
 * value is set to the next argument after the flag, if using the value func).
 */

/* TODO: Accept name=value flags. Now it read name but is not able to get value */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef FLAG_H_
#define FLAG_H_

/* Flag context */
static int *flag_argc;
static char ***flag_argv;
static int init = 0;
static const char *empty_string = "";

/* Init flags */
static void
flag_set(int *argc, char **argv[])
{
        flag_argc = argc;
        flag_argv = argv;
        init = 1;
}

static void
__flag_check_init()
{
        if (init != 0) return;
        printf("ERROR: Bad usage of flag.h! Dont forget to call `flag_set()`.\n");
        abort();
}

static int
__flag_get_value(char **value, char *flag_name)
{
        /* This function is not fast at all but I thint it does not matter */
        int i;
        __flag_check_init();
        for (i = 0; i < *flag_argc - 1; i++) {
                if (!memcmp((*flag_argv)[i], flag_name, strlen(flag_name))) {
                        *value = (*flag_argv)[i + 1];
                        memcpy((*flag_argv) + i, (*flag_argv) + i + 2,
                               (*flag_argc - i) * sizeof(char *));
                        *flag_argc -= 2;
                        return 1;
                }
        }

        /* This is for the case where a flag that expects a name and a value is
         * in the last position, so the value can not be found. */
        if (!memcmp((*flag_argv)[*flag_argc - 1], flag_name, strlen(flag_name))) {
                *value = (char *) empty_string;
                (*flag_argv)[*flag_argc - 1] = NULL;
                *flag_argc -= 1;
                return 1;
        }
        return 0;
}

static int
__flag_get(char *flag_name)
{
        int i;
        __flag_check_init();
        for (i = 0; i < *flag_argc; i++) {
                if (!memcmp((*flag_argv)[i], flag_name, strlen(flag_name))) {
                        (*flag_argv)[i] = (*flag_argv)[*flag_argc - 1];
                        --*flag_argc;
                        return 1;
                }
        }
        return 0;
}


static int
__flag_get_value_v(char **value, ...)
{
        va_list flag_ap;
        char *current_flag;
        va_start(flag_ap, value);
        while ((current_flag = va_arg(flag_ap, char *))) {
                if (__flag_get_value(value, current_flag))
                        return 1;
        }
        return 0;
}

static int
__flag_get_v(char *flag1, ...)
{
        va_list flag_ap;
        va_start(flag_ap, flag1);
        char *current_flag = flag1;
        do {
                if (__flag_get(current_flag))
                        return 1;
        } while ((current_flag = va_arg(flag_ap, char *)));
        return 0;
}

/* int flag_get_value(char **value, char* flag1 [, char* flag2 [, ...]])
 * Return 0 if flag is not in arguments, else 1 and set value to a . Flag is
 * removed from flag list so it would not be found if called again. */
#define flag_get_value(value, ...) __flag_get_value_v(value, ##__VA_ARGS__, NULL)

/* int flag_get(char* flag1 [, char* flag2 [, ...]])
 * Return 0 if flag is not in arguments, else 1. Flag is removed from flag list
 * so it would not be found if called again. */
#define flag_get(value, ...) __flag_get_v(value, ##__VA_ARGS__, NULL)

#endif // FLAG_H_
