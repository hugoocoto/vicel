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

#ifndef COLOR_H_
#define COLOR_H_

#include "hm.h"
#include "common.h"

#define C_RESET NULL

typedef struct Color {
        bool active;
        char *scolor;
} Color;

extern Hmap colors;
void apply_color(char *key);
void apply_color_custom(char *color);
char *get_color(char *key);
void add_color(char *c);
void set_default_colors();

#endif // !COLOR_H_
