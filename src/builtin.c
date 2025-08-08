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

#include "builtin.h"
#include "cellmap.h"
#include "da.h"
#include "eval.h"
#include "formula.h"

struct Pair {
        char *name;
        Func f;
};

typedef DA(struct Pair) Table;
Table table = { 0 };

Func
builtin_get(char *name)
{
        for_da_each(p, table)
        {
                if (strcmp(p->name, name) == 0) {
                        return p->f;
                }
        }
        return NULL;
}

void
builtin_add(char *name, Func f)
{
        struct Pair p;
        p.name = name;
        p.f = f;
        da_append(&table, p);
}

Value
sum(Expr *e)
{
        Value v;
        if (e == NULL) return VALUE_ERROR;
        v = eval_expr(e);
        while ((e = e->next)) {
                v = vadd(v, eval_expr(e));
        }
        return v;
}


static __attribute__((constructor)) void
__setup__()
{
        builtin_add("sum", sum);
}
