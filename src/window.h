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

#ifndef WINDOW_H
#define WINDOW_H

#include "cellmap.h" /* shut up */
#include "common.h"

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) >= (b) ? (a) : (b))

typedef struct Context {
        struct winsize ws;
        CellMat *body;
        char *filename;
        int status_bar_height;

        // starting at 0,0
        int cursor_pos_r;
        int cursor_pos_c;

        int scroll_r;
        int scroll_c;
        int max_display_r;
        int max_display_c;
} Context;

#define INIT_CONTEXT ((Context) { \
.ws = { 0 },                      \
.body = NULL,                     \
.filename = NULL,                 \
.status_bar_height = 1,           \
.cursor_pos_c = 0,                \
.cursor_pos_r = 0,                \
.scroll_c = 0,                    \
.scroll_r = 0,                    \
.max_display_c = 0,               \
.max_display_r = 0,               \
})

void print_at(int r, int c, char *buf, int buflen, int n);
void render();
void cursor_gotocell(int x, int y);
void print_mapping_buffer(char *buf, int len, int n, int repeat);
Cell *get_cell_from_coords(char *coords);
int parse_coords(char *c, int *x, int *y, bool *freeze_r, bool *freeze_c);

extern Context active_ctx;

#endif // !WINDOW_H
