#include "mappings.h"
#include "cellmap.h"
#include "common.h"
#include "escape_code.h"
#include "window.h"

void
a_quit()
{
        printf(EFFECT(RESET));
        printf(T_ASBD());
        printf(T_CUSHW());
        fflush(stdout);
        exit(0);
}

void
a_add_row()
{
        cm_add_row(active_ctx.body);
        render();
}

void
a_add_col()
{
        cm_add_col(active_ctx.body);
        render();
}
