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
        do {
                print_val(vspl_eval_expr(args), .nostrdelim = 1);
                if ((args = args->next))
                        print_val(((Value) { .type = TYPE_STR, .str = " " }), .nostrdelim = 1);
        } while (args);
        return NO_VALUE;
}

Value
core_print_ln(Expr *args)
{
        core_print(args);
        printf("\n");
        return NO_VALUE;
}

/* core_input read buffer */
char buf[1024]; /* weird trick to avoid usage after return, as it's strduped in
                   env and this value is not used after that */
Value
core_input(Expr *_)
{
        char *c;
        assert(_ == NULL);
        if (fgets(buf, sizeof buf - 1, stdin)) {
                if ((c = strchr(buf, '\n'))) {
                        *c = 0;
                }
                return (Value) { .type = TYPE_STR, .str = buf };
        }
        return NO_VALUE;
}

static __attribute__((constructor)) void
__init__()
{
        preload("print", core_print, 1 | VAARGS);
        preload("println", core_print_ln, 1 | VAARGS);
        preload("input", core_input, 0);
}
