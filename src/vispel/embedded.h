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
#include <stdbool.h>
#include <stdio.h>

void vspl_start();
bool vspl_parse(FILE *file);
bool vspl_get_int(char *name, int *value);
bool vspl_get_str(char *name, char **value);
void vspl_end();

void vspl_addint(char *name, int value);
void vspl_addstr(char *name, char *value);
void vspl_dump_env();

#define vspl_addvar(name, v) _Generic((v), \
char *: vspl_addstr,                       \
const char *: vspl_addstr,                 \
int: vspl_addint,                          \
bool: vspl_addint)(name, v)
