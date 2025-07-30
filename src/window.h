#ifndef WINDOW_H
#define WINDOW_H

#include "cellmap.h" /* shut up */
#include "common.h"

typedef struct Context {
        struct winsize ws;
        CellMat *body;
        char *filename;
        int status_bar_height;

        // starting at 0,0
        int cursor_pos_x;
        int cursor_pos_y;
} Context;

#define INIT_CONTEXT ((Context) { \
.ws = { 0 },                      \
.body = NULL,                     \
.filename = NULL,                 \
.status_bar_height = 1,           \
.cursor_pos_x = 0,                \
.cursor_pos_y = 0,                \
})

void print_at(int r, int c, char *buf, int buflen, int n);
void render();
void cursor_gotocell(int x, int y);
void print_mapping_buffer(char *buf, int len, int n, int repeat);
Cell *get_cell_from_coords(char *coords);

extern Context active_ctx;

#endif // !WINDOW_H
