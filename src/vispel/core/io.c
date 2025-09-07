/*
 * VICEL - Visual Cell editor
 * Copyright (C) 2025  Hugo Coto Florez
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * For questions or support, contact: hugo.coto@member.fsf.org
 */
 /*
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
