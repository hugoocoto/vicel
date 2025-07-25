#include <sys/ioctl.h>

typedef struct Context {
        struct winsize ws;
        char *filename;
        int status_bar_height;
} Context;
