#ifndef WINDOW_H
#define WINDOW_H

#include "common.h"

typedef struct Context {
        struct winsize ws;
        char *filename;
        int status_bar_height;
} Context;

#endif // !WINDOW_H
