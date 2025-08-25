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
#include "color.h"
#include "common.h"
#include "da.h"
#include "debug.h"
#include "eval.h"
#include "formula.h"
#include "window.h"

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
builtin_sum(Expr *e)
{
        Value v;
        if (e == NULL) {
                report("Null first expr at sum");
                return VALUE_EMPTY;
        }
        v = vadd(eval_expr(e), AS_NUMBER(0)); // for ranges
        while ((e = e->next)) {
                v = vadd(v, eval_expr(e));
        }
        return v;
}

Value
builtin_mul(Expr *e)
{
        Value v;
        if (e == NULL) return VALUE_EMPTY;
        v = vmul(eval_expr(e), AS_NUMBER(1)); // for ranges
        while ((e = e->next)) {
                v = vmul(v, eval_expr(e));
        }
        return v;
}

Value
builtin_count(Expr *e)
{
        Value v = AS_NUMBER(0);
        if (e == NULL) return v;
        do {
                v = vcountnum(v, eval_expr(e));
        } while ((e = e->next));
        return v;
}

Value
builtin_avg(Expr *e)
{
        Value a = builtin_sum(e);
        Value b = builtin_count(e);
        if (a.type == TYPE_NUMBER && a.as.num == 0.0 &&
            b.type == TYPE_NUMBER && b.as.num == 0.0) return VALUE_EMPTY;
        return vdiv(a, b);
}

Value
builtin_min(Expr *e)
{
        if (e == NULL) return VALUE_EMPTY;
        Value min = vmin(VALUE_EMPTY, eval_expr(e));
        while ((e = e->next)) {
                min = vmin(min, eval_expr(e));
        }
        return min.type == TYPE_NUMBER ? min : VALUE_EMPTY;
}

Value
builtin_max(Expr *e)
{
        if (e == NULL) return VALUE_EMPTY;
        Value max = vmax(VALUE_EMPTY, eval_expr(e));
        while ((e = e->next)) {
                max = vmax(max, eval_expr(e));
        }
        return max.type == TYPE_NUMBER ? max : VALUE_EMPTY;
}

Value
builtin_if(Expr *e)
{
        if (e == NULL) return VALUE_EMPTY;
        Value cond = eval_expr(e);
        if (cond.type != TYPE_BOOL) return VALUE_EMPTY;

        if (cond.as.bol) {
                if (e->next == NULL) return VALUE_EMPTY;
                return eval_expr(e->next);
        } else {
                if (e->next == NULL) return VALUE_EMPTY;
                if (e->next->next == NULL) return VALUE_EMPTY;
                return eval_expr(e->next->next);
        }
}

static void
set_color(Cell *cell, char *col)
{
        char *c = get_color(col);
        if (c == NULL) {
                add_color(col);
                c = get_color(col);
        }
        assert(c);
        char *name = cm_get_cell_name(active_ctx.body, cell);
        report("changing color for `%s` to `%s`", name, c == NULL ? col : c);
        free(name);
        cell->color = (Color) {
                .active = true,
                .scolor = c,
        };
}

/* set color if none */
static void
set_color_b(Cell *cell, char *col)
{
        if (cell->color.active) return;

        char *c = get_color(col);
        if (c == NULL) {
                add_color(col);
                c = get_color(col);
        }
        assert(c);
        char *name = cm_get_cell_name(active_ctx.body, cell);
        report("changing color for `%s` to `%s`", name, c == NULL ? col : c);
        free(name);
        cell->color = (Color) {
                .active = true,
                .scolor = c,
        };
}

Value
builtin_literal(Expr *e)
{
        if (e == NULL) return VALUE_EMPTY;
        return eval_expr(e);
}

Value
builtin_colorb(Expr *e)
{
        if (e == NULL) return VALUE_EMPTY;

        char *col = NULL;
        Value v = eval_expr(e);

        if (v.type == TYPE_TEXT || v.type == TYPE_NUMBER) col = get_repr(v);

        while ((e = e->next)) {
                if (e->type == EXPR_IDENTIFIER) {
                        set_color_b(e->as.identifier.cell, col);
                }
                if (e->type == EXPR_LITERAL) {
                        if (e->as.literal.value.type == TYPE_RANGE) {
                                int x, y;
                                Cell *c;
                                __auto_type r = e->as.literal.value.as.range;

                                for (x = r.startx; x <= r.endx; x++) {
                                        for (y = r.starty; y <= r.endy; y++) {
                                                c = cm_get_cell_ptr(active_ctx.body, x, y);
                                                if (!c) break;
                                                set_color_b(c, col);
                                        }
                                }
                        }
                }
        }
        free(col);
        return VALUE_EMPTY;
}

Value
builtin_color(Expr *e)
{
        if (e == NULL) return VALUE_EMPTY;

        char *col = NULL;
        Value v = eval_expr(e);

        if (v.type == TYPE_TEXT || v.type == TYPE_NUMBER) col = get_repr(v);

        while ((e = e->next)) {
                if (e->type == EXPR_IDENTIFIER) {
                        set_color(e->as.identifier.cell, col);
                }
                if (e->type == EXPR_LITERAL) {
                        if (e->as.literal.value.type == TYPE_RANGE) {
                                int x, y;
                                Cell *c;
                                __auto_type r = e->as.literal.value.as.range;

                                for (x = r.startx; x <= r.endx; x++) {
                                        for (y = r.starty; y <= r.endy; y++) {
                                                c = cm_get_cell_ptr(active_ctx.body, x, y);
                                                if (!c) break;
                                                set_color(c, col);
                                        }
                                }
                        }
                }
        }
        free(col);
        return VALUE_EMPTY;
}

static __attribute__((constructor)) void
__setup__()
{
        builtin_add("sum", builtin_sum);
        builtin_add("avg", builtin_avg);
        builtin_add("mul", builtin_mul);
        builtin_add("count", builtin_count);
        builtin_add("min", builtin_min);
        builtin_add("max", builtin_max);
        builtin_add("if", builtin_if);
        builtin_add("color", builtin_color);
        builtin_add("colorb", builtin_colorb);
        builtin_add("literal", builtin_literal);
}
