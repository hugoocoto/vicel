/* VISPEL interpreter - Core lib - IO stuff
 *
 * Author: Hugo Coto Florez
 * Repo: github.com/hugocotoflorez/vispel
 *
 * */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "core.h"

Value
core_print(Expr *args)
{
        print_val(vspl_eval_expr(args));
        return NO_VALUE;
}

Value
core_print_ln(Expr *args)
{
        print_val(vspl_eval_expr(args));
        printf("\n");
        return NO_VALUE;
}

Value
core_input(Expr *_)
{
        char buf[1024];
        char *c;
        assert((_->next = NULL));
        if (fgets(buf, sizeof buf - 1, stdin)) {
                if ((c = strchr(buf, '\n'))) {
                        *c = 0;
                }
                return (Value) { .type = TYPE_STR, .str = strdup(buf) };
        }
        return NO_VALUE;
}

static __attribute__((constructor)) void
__init__()
{
        preload("print", core_print, 1);
        preload("println", core_print_ln, 1);
        preload("input", core_input, 0);
}
