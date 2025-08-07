#ifndef WINDOW_H
#define WINDOW_H

#include "cellmap.h" /* shut up */
#include "common.h"

#define UI_FG FG_YELLOW
#define UI_BG BG_DEFAULT
#define CELL_FG FG_BLUE
#define CELL_BG BG_DEFAULT
#define CELL_SELECT_FG FG_GREEN
#define CELL_SELECT_BG CELL_BG

#define min(a, b) ((a) < (b) ? (a) : (b))
#define num_col_width 5
#define column_width 10
#define row_width 1


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
int parse_coords(char *coords, int *x, int *y);

extern Context active_ctx;

#endif // !WINDOW_H
