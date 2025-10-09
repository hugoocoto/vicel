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
 * For questions or support, contact: me@hugocoto.com
 */
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

void flag_set(int *argc, char **argv[]);
void __flag_check_init();
int __flag_get_value(char **value, char *flag_name);
int __flag_get(char *flag_name);
int __flag_get_value_v(char **value, ...);
int __flag_get_v(char *flag1, ...);

/* int flag_get_value(char **value, char* flag1 [, char* flag2 [, ...]])
 * Return 0 if flag is not in arguments, else 1 and set value to a . Flag is
 * removed from flag list so it would not be found if called again. */
#define flag_get_value(value, ...) __flag_get_value_v(value, ##__VA_ARGS__, NULL)

/* int flag_get(char* flag1 [, char* flag2 [, ...]])
 * Return 0 if flag is not in arguments, else 1. Flag is removed from flag list
 * so it would not be found if called again. */
#define flag_get(value, ...) __flag_get_v(value, ##__VA_ARGS__, NULL)


#endif // FLAG_H_
