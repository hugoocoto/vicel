#include <string.h>
#include <stdlib.h>

#ifndef strdup
#define strdup(s) strdup(s)
char *
strdup(const char *s)
{
        return memcpy(malloc(strlen(s) + 1), s, strlen(s) + 1);
}
#endif
