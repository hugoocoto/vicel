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
 * For questions or support, contact: me@hugocoto.com
 */

#include "eval.h"
#include "builtin.h"
#include "cellmap.h"
#include "common.h"
#include "debug.h"
#include "formula.h"
#include "window.h"

Value
eval_identifier(Expr *e)
{
        if (e->as.identifier.cell->value.type == TYPE_FORMULA)
                return e->as.identifier.cell->value.as.formula->value;
        return e->as.identifier.cell->value;
}

Value
eval_literal(Expr *e)
{
        return e->as.literal.value;
}

Value
eval_func(Expr *e)
{
        Value name = eval_expr(e->as.func.name);
        if (name.type != TYPE_TEXT) {
                report("eval_func func name is not text");
                return VALUE_ERROR;
        }

        Func f = builtin_get(name.as.text);
        if (f == NULL) {
                report("No builtin function for name %s", name.as.text);
                return VALUE_ERROR;
        }
        return f(e->as.func.args);
}

Value
eval_unop(Expr *e)
{
        Value rhs = eval_expr(e->as.unop.rhs);

        if (rhs.type != TYPE_NUMBER) {
                report("No yet implemented: unop for %s",
                       cm_type_repr(rhs.type));
                return VALUE_ERROR;
        }

        if (e->as.unop.op[1] == 0)
                switch (*e->as.unop.op) {
                case '-':
                        return AS_NUMBER(-rhs.as.num);
                case '+':
                        return AS_NUMBER(+rhs.as.num);
                }

        report("No yet implemented: unop for `%s`", e->as.unop.op);
        return VALUE_ERROR;
}

Value
rangemap(Value base, Value v, Value (*f)(Value, Value))
{
        report("CALL RANGEMAP");
        assert(v.type == TYPE_RANGE);
        int x, y;
        Value val = base;
        Cell *c;

        for (x = v.as.range.startx; x <= v.as.range.endx; x++) {
                for (y = v.as.range.starty; y <= v.as.range.endy; y++) {
                        c = cm_get_cell_ptr(active_ctx.body, x, y);
                        if (!c) break;
                        val = f(val, c->value);
                }
        }
        return val;
}

Value
vadd(Value a, Value b)
{
        if (a.type == TYPE_FORMULA) return vadd(a.as.formula->value, b);
        if (b.type == TYPE_FORMULA) return vadd(a, b.as.formula->value);
        if (a.type == TYPE_RANGE) return vadd(rangemap(AS_NUMBER(0), a, vadd), b);
        if (b.type == TYPE_RANGE) return vadd(a, rangemap(AS_NUMBER(0), b, vadd));

        if (a.type == TYPE_NUMBER) {
                if (b.type == TYPE_NUMBER) return AS_NUMBER(a.as.num + b.as.num);
                return a;
        }
        if (b.type == TYPE_NUMBER) return b;
        return VALUE_EMPTY;
}

Value
vsub(Value a, Value b)
{
        if (a.type == TYPE_FORMULA) return vsub(a.as.formula->value, b);
        if (b.type == TYPE_FORMULA) return vsub(a, b.as.formula->value);
        if (a.type == TYPE_RANGE) return vsub(rangemap(AS_NUMBER(0), a, vsub), b);
        if (b.type == TYPE_RANGE) return vsub(a, rangemap(AS_NUMBER(0), b, vsub));

        if (a.type == TYPE_NUMBER) {
                if (b.type == TYPE_NUMBER) return AS_NUMBER(a.as.num - b.as.num);
                return a;
        }
        if (b.type == TYPE_NUMBER) return b;
        return VALUE_EMPTY;
}
Value
vdiv(Value a, Value b)
{
        if (a.type == TYPE_FORMULA) return vdiv(a.as.formula->value, b);
        if (b.type == TYPE_FORMULA) return vdiv(a, b.as.formula->value);
        if (a.type == TYPE_RANGE) return vdiv(rangemap(AS_NUMBER(1), a, vdiv), b);
        if (b.type == TYPE_RANGE) return vdiv(a, rangemap(AS_NUMBER(1), b, vdiv));

        if (a.type == TYPE_NUMBER) {
                if (b.type == TYPE_NUMBER) return AS_NUMBER(a.as.num / b.as.num);
                return a;
        }
        if (b.type == TYPE_NUMBER) return b;
        return VALUE_EMPTY;
}

Value
vmul(Value a, Value b)
{
        if (a.type == TYPE_FORMULA) return vmul(a.as.formula->value, b);
        if (b.type == TYPE_FORMULA) return vmul(a, b.as.formula->value);
        if (a.type == TYPE_RANGE) return vmul(rangemap(AS_NUMBER(1), a, vmul), b);
        if (b.type == TYPE_RANGE) return vmul(a, rangemap(AS_NUMBER(1), b, vmul));

        if (a.type == TYPE_NUMBER) {
                if (b.type == TYPE_NUMBER) return AS_NUMBER(a.as.num * b.as.num);
                return a;
        }
        if (b.type == TYPE_NUMBER) return b;
        return VALUE_EMPTY;
}

Value
vpow(Value a, Value b)
{
        if (a.type == TYPE_FORMULA) return vpow(a.as.formula->value, b);
        if (b.type == TYPE_FORMULA) return vpow(a, b.as.formula->value);
        if (a.type == TYPE_RANGE) return vpow(rangemap(AS_NUMBER(1), a, vpow), b);
        if (b.type == TYPE_RANGE) return vpow(a, rangemap(AS_NUMBER(1), b, vpow));

        if (a.type == TYPE_NUMBER) {
                if (b.type == TYPE_NUMBER) return AS_NUMBER(pow(a.as.num, b.as.num));
                return a;
        }
        if (b.type == TYPE_NUMBER) return b;
        return VALUE_EMPTY;
}

Value
vcountnum(Value start, Value a)
{
        if (a.type == TYPE_NUMBER) return vadd(start, AS_NUMBER(1));
        if (a.type == TYPE_FORMULA) return vadd(start, a.as.formula->value);
        if (a.type != TYPE_RANGE) return start;
        return vadd(start, rangemap(AS_NUMBER(0), a, vcountnum));
}

Value
vmin(Value a, Value b)
{
        if (a.type == TYPE_FORMULA) return vmin(a.as.formula->value, b);
        if (b.type == TYPE_FORMULA) return vmin(a, b.as.formula->value);
        if (a.type == TYPE_RANGE) return rangemap(VALUE_EMPTY, a, vmin);
        if (b.type == TYPE_RANGE) return rangemap(VALUE_EMPTY, b, vmin);
        if (a.type == TYPE_NUMBER) {
                if (b.type == TYPE_NUMBER) return a.as.num <= b.as.num ? a : b;
                return a;
        }
        if (b.type == TYPE_NUMBER) return b;
        return VALUE_EMPTY;
}

Value
vmax(Value a, Value b)
{
        if (a.type == TYPE_FORMULA) return vmax(a.as.formula->value, b);
        if (b.type == TYPE_FORMULA) return vmax(a, b.as.formula->value);
        if (a.type == TYPE_RANGE) return rangemap(VALUE_EMPTY, a, vmax);
        if (b.type == TYPE_RANGE) return rangemap(VALUE_EMPTY, b, vmax);
        if (a.type == TYPE_NUMBER) {
                if (b.type == TYPE_NUMBER) return a.as.num >= b.as.num ? a : b;
                return a;
        }
        if (b.type == TYPE_NUMBER) return b;
        return VALUE_EMPTY;
}

Value
veq(Value a, Value b)
{
        if (a.type == TYPE_FORMULA) return veq(a.as.formula->value, b);
        if (b.type == TYPE_FORMULA) return veq(a, b.as.formula->value);
        if (a.type == TYPE_NUMBER) {
                if (b.type == TYPE_NUMBER) return AS_BOOL(a.as.num == b.as.num);
                return AS_BOOL(true);
        }
        if (b.type == TYPE_NUMBER) return AS_BOOL(false);
        return VALUE_EMPTY;
}

Value
vneq(Value a, Value b)
{
        if (a.type == TYPE_FORMULA) return vneq(a.as.formula->value, b);
        if (b.type == TYPE_FORMULA) return vneq(a, b.as.formula->value);
        if (a.type == TYPE_NUMBER) {
                if (b.type == TYPE_NUMBER) return AS_BOOL(a.as.num != b.as.num);
                return AS_BOOL(true);
        }
        if (b.type == TYPE_NUMBER) return AS_BOOL(false);
        return VALUE_EMPTY;
}


Value
vlt(Value a, Value b)
{
        if (a.type == TYPE_FORMULA) return vlt(a.as.formula->value, b);
        if (b.type == TYPE_FORMULA) return vlt(a, b.as.formula->value);
        if (a.type == TYPE_NUMBER) {
                if (b.type == TYPE_NUMBER) return AS_BOOL(a.as.num < b.as.num);
                return AS_BOOL(true);
        }
        if (b.type == TYPE_NUMBER) return AS_BOOL(false);
        return VALUE_EMPTY;
}

Value
vleqt(Value a, Value b)
{
        if (a.type == TYPE_FORMULA) return vleqt(a.as.formula->value, b);
        if (b.type == TYPE_FORMULA) return vleqt(a, b.as.formula->value);
        if (a.type == TYPE_NUMBER) {
                if (b.type == TYPE_NUMBER) return AS_BOOL(a.as.num <= b.as.num);
                return AS_BOOL(true);
        }
        if (b.type == TYPE_NUMBER) return AS_BOOL(false);
        return VALUE_EMPTY;
}

Value
vgt(Value a, Value b)
{
        if (a.type == TYPE_FORMULA) return vgt(a.as.formula->value, b);
        if (b.type == TYPE_FORMULA) return vgt(a, b.as.formula->value);
        if (a.type == TYPE_NUMBER) {
                if (b.type == TYPE_NUMBER) return AS_BOOL(a.as.num > b.as.num);
                return AS_BOOL(true);
        }
        if (b.type == TYPE_NUMBER) return AS_BOOL(false);
        return VALUE_EMPTY;
}

Value
vgeqt(Value a, Value b)
{
        if (a.type == TYPE_FORMULA) return vgeqt(a.as.formula->value, b);
        if (b.type == TYPE_FORMULA) return vgeqt(a, b.as.formula->value);
        if (a.type == TYPE_NUMBER) {
                if (b.type == TYPE_NUMBER) return AS_BOOL(a.as.num >= b.as.num);
                return AS_BOOL(true);
        }
        if (b.type == TYPE_NUMBER) return AS_BOOL(false);
        return VALUE_EMPTY;
}

Value
eval_binop(Expr *e)
{
        Value lhs = eval_expr(e->as.binop.lhs);
        Value rhs = eval_expr(e->as.binop.rhs);

        if (!e->as.binop.op[1])
                switch (*e->as.binop.op) {
                case '+': return vadd(lhs, rhs);
                case '-': return vsub(lhs, rhs);
                case '/': return vdiv(lhs, rhs);
                case '*': return vmul(lhs, rhs);
                case '^': return vpow(lhs, rhs);
                }
        if (!strcmp(e->as.binop.op, "<")) return vlt(lhs, rhs);
        if (!strcmp(e->as.binop.op, "<=")) return vleqt(lhs, rhs);
        if (!strcmp(e->as.binop.op, ">")) return vgt(lhs, rhs);
        if (!strcmp(e->as.binop.op, ">=")) return vgeqt(lhs, rhs);
        if (!strcmp(e->as.binop.op, "==")) return veq(lhs, rhs);
        if (!strcmp(e->as.binop.op, "=")) return veq(lhs, rhs);
        if (!strcmp(e->as.binop.op, "!=")) return vneq(lhs, rhs);

        report("No yet implemented: binop for %s", e->as.binop.op);
        return VALUE_ERROR;
}

Value
eval_expr(Expr *e)
{
        if (e == NULL) return AS_NUMBER(0);

        switch (e->type) {
        case EXPR_LITERAL: return eval_literal(e);
        case EXPR_BIN: return eval_binop(e);
        case EXPR_UN: return eval_unop(e);
        case EXPR_IDENTIFIER: return eval_identifier(e);
        case EXPR_FUNC: return eval_func(e);
        default:
                report("No yet implemented: eval_expr for %d", e->type);
                return VALUE_ERROR;
        }
}

Value
eval_formula(Formula f)
{
        if (!f.body) return VALUE_ERROR;
        return eval_expr(f.body);
}
