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

#include "eval.h"
#include "builtin.h"
#include "cellmap.h"
#include "common.h"
#include "debug.h"
#include "formula.h"
#include "window.h"
#include <assert.h>
#include <stdbool.h>

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
        // char buf[64] = { 0 };
        // get_ast_repr(e, buf, sizeof buf);

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

bool
are_valid_operands(Value a, Value b)
{
        switch (a.type) {
        case TYPE_FORMULA:
                return are_valid_operands(a.as.formula->value, b);
        case TYPE_NUMBER:
                if (b.type == TYPE_NUMBER) return true;
                if (b.type == TYPE_FORMULA)
                        return are_valid_operands(a, b.as.formula->value);
                return false;
        default:
                report("Invalid operands %s and %s",
                       cm_type_repr(a.type), cm_type_repr(b.type));
                return false;
        }
}

Value
vadd(Value a, Value b)
{
        if (are_valid_operands(a, b)) {
                return AS_NUMBER(a.as.num + b.as.num);
        }
        if (a.type == TYPE_RANGE) return vadd(rangemap(AS_NUMBER(0), a, vadd), b);
        if (b.type == TYPE_RANGE) return vadd(a, rangemap(AS_NUMBER(0), b, vadd));
        if (a.type == TYPE_NUMBER) return a;
        if (b.type == TYPE_NUMBER) return b;
        return VALUE_EMPTY;
}

Value
vsub(Value a, Value b)
{
        if (are_valid_operands(a, b)) {
                return AS_NUMBER(a.as.num - b.as.num);
        }

        if (a.type == TYPE_RANGE) return vsub(rangemap(AS_NUMBER(0), a, vsub), b);
        if (b.type == TYPE_RANGE) return vsub(a, rangemap(AS_NUMBER(0), b, vsub));
        if (a.type == TYPE_NUMBER) return a;
        if (b.type == TYPE_NUMBER) return AS_NUMBER(-b.as.num);
        return VALUE_EMPTY;
}
Value
vdiv(Value a, Value b)
{
        if (are_valid_operands(a, b)) {
                return AS_NUMBER(a.as.num / b.as.num);
        }
        if (a.type == TYPE_RANGE) return vdiv(rangemap(AS_NUMBER(1), a, vdiv), b);
        if (b.type == TYPE_RANGE) return vdiv(a, rangemap(AS_NUMBER(1), b, vdiv));
        return VALUE_EMPTY;
}

Value
vmul(Value a, Value b)
{
        if (are_valid_operands(a, b)) {
                return AS_NUMBER(a.as.num * b.as.num);
        }
        if (a.type == TYPE_RANGE) return vmul(rangemap(AS_NUMBER(1), a, vmul), b);
        if (b.type == TYPE_RANGE) return vmul(a, rangemap(AS_NUMBER(1), b, vmul));
        return VALUE_EMPTY;
}

Value
vpow(Value a, Value b)
{
        if (are_valid_operands(a, b)) {
                return AS_NUMBER(pow(a.as.num, b.as.num));
        }
        if (a.type == TYPE_RANGE) return vpow(rangemap(AS_NUMBER(1), a, vpow), b);
        if (b.type == TYPE_RANGE) return vpow(a, rangemap(AS_NUMBER(1), b, vpow));
        return VALUE_EMPTY;
}

Value
vcountnum(Value start, Value a)
{
        if (a.type == TYPE_NUMBER) return vadd(start, AS_NUMBER(1));
        if (a.type != TYPE_RANGE) return start;
        return vadd(start, rangemap(AS_NUMBER(0), a, vcountnum));
}

Value
vmin(Value a, Value b)
{
        if (a.type == TYPE_NUMBER) {
                if (b.type == TYPE_NUMBER) return a.as.num <= b.as.num ? a : b;
                if (b.type == TYPE_RANGE) return rangemap(a, b, vmin);
                return a;
        }
        if (b.type == TYPE_NUMBER) {
                if (a.type == TYPE_RANGE) return rangemap(b, a, vmin);
                return b;
        }

        if (a.type == TYPE_RANGE) return rangemap(VALUE_EMPTY, a, vmin);
        if (b.type == TYPE_RANGE) return rangemap(VALUE_EMPTY, b, vmin);
        return VALUE_EMPTY;
}

Value
vmax(Value a, Value b)
{
        if (a.type == TYPE_NUMBER) {
                if (b.type == TYPE_NUMBER) return a.as.num >= b.as.num ? a : b;
                if (b.type == TYPE_RANGE) return rangemap(a, b, vmax);
                return a;
        }
        if (b.type == TYPE_NUMBER) {
                if (a.type == TYPE_RANGE) return rangemap(b, a, vmax);
                return b;
        }

        if (a.type == TYPE_RANGE) return rangemap(VALUE_EMPTY, a, vmax);
        if (b.type == TYPE_RANGE) return rangemap(VALUE_EMPTY, b, vmax);
        return VALUE_EMPTY;
}

Value
eval_binop(Expr *e)
{
        Value lhs = eval_expr(e->as.binop.lhs);
        Value rhs = eval_expr(e->as.binop.rhs);

        switch (*e->as.binop.op) {
        case '+': return vadd(lhs, rhs);
        case '-': return vsub(lhs, rhs);
        case '/': return vdiv(lhs, rhs);
        case '*': return vmul(lhs, rhs);
        case '^': return vpow(lhs, rhs);
        }

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
