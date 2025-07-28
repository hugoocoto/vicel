#ifndef WINDOW_H
#define WINDOW_H

#include "cellmap.h"
#include "common.h"

typedef struct Context {
        struct winsize ws;
        CellMat *body;
        char *filename;
        int status_bar_height;

} Context;

#define INIT_CONTEXT ((Context) { \
.ws = { 0 },                      \
.body = NULL,                     \
.filename = NULL,                 \
.status_bar_height = 1,           \
})

void print_at(int r, int c, char *buf, int buflen, int n);
void render();

extern Context active_ctx;

#endif // !WINDOW_H
