#ifndef WINDOW_H
#define WINDOW_H

#include "common.h"

typedef struct Context {
        struct winsize ws;
        char *filename;
        int status_bar_height;
} Context;

#define INIT_CONTEXT ((Context) { \
.ws = { 0 },                      \
.filename = NULL,                 \
.status_bar_height = 1,           \
})

void print_at(int r, int c, char *buf, int buflen, int n);

#endif // !WINDOW_H
