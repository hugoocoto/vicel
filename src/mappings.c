#include "mappings.h"
#include "common.h"
#include "escape_code.h"

void
a_quit()
{
        printf(EFFECT(RESET));
        printf(T_ASBD());
        fflush(stdout);
        exit(0);
}
