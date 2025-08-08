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

#include "cellmap.h"
#include "common.h"
#include "debug.h"
#include "formula.h"
#include "window.h"
#include <unistd.h>

Value eval_formula(Formula f);
Value eval_expr(Expr *e);

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
eval_unop(Expr *e)
{
        Value rhs = eval_expr(e->as.unop.rhs);

        if (rhs.type != TYPE_NUMBER) {
                report("No yet implemented: unop for %s", cm_type_repr(rhs.type));
                return VALUE_ERROR;
        }

        if (!strcmp(e->as.unop.op, "-")) {
                return AS_NUMBER(-rhs.as.num);
        };
        report("No yet implemented: unop for `%s`", e->as.unop.op);
        return VALUE_ERROR;
}

bool
are_valid_operands(Value a, Value b)
{
        switch (a.type) {
        case TYPE_FORMULA:
                return are_valid_operands(a.as.formula->value, b);
        case TYPE_NUMBER:
                if (b.type == TYPE_FORMULA) return are_valid_operands(a, b.as.formula->value);
                if (b.type == TYPE_NUMBER) return true;
                return false;
        case TYPE_TEXT:
        case TYPE_EMPTY:
        default:
                report("Invalid operands %s and %s",
                       cm_type_repr(a.type), cm_type_repr(b.type));
                return false;
        }
}

Value
vsub(Value a, Value b)
{
        if (are_valid_operands(a, b)) {
                return AS_NUMBER(a.as.num - b.as.num);
        }

        if (a.type == TYPE_NUMBER) return a;
        if (b.type == TYPE_NUMBER)
                return AS_NUMBER(-b.as.num);
        else
                return AS_NUMBER(0);

        report("No yet implemented: vsub for %s and %s",
               cm_type_repr(a.type), cm_type_repr(b.type));
        return VALUE_ERROR;
}
Value
vdiv(Value a, Value b)
{
        if (are_valid_operands(a, b)) {
                return AS_NUMBER(a.as.num / b.as.num);
        }

        if (a.type == TYPE_NUMBER) return AS_NUMBER(0);
        if (b.type == TYPE_NUMBER)
                return AS_NUMBER(0);
        else
                return AS_NUMBER(0);

        report("No yet implemented: vdiv for %s and %s",
               cm_type_repr(a.type), cm_type_repr(b.type));
        return VALUE_ERROR;
}

Value
vmul(Value a, Value b)
{
        if (are_valid_operands(a, b)) {
                return AS_NUMBER(a.as.num * b.as.num);
        }

        if (a.type == TYPE_NUMBER) return AS_NUMBER(0);
        if (b.type == TYPE_NUMBER)
                return AS_NUMBER(0);
        else
                return AS_NUMBER(0);

        report("No yet implemented: vsub for %s and %s",
               cm_type_repr(a.type), cm_type_repr(b.type));
        return VALUE_ERROR;
}

Value
vpow(Value a, Value b)
{
        if (are_valid_operands(a, b)) {
                return AS_NUMBER(pow(a.as.num, b.as.num));
        }

        if (a.type == TYPE_NUMBER) return AS_NUMBER(0);
        if (b.type == TYPE_NUMBER)
                return AS_NUMBER(0);
        else
                return AS_NUMBER(0);

        report("No yet implemented: vpow for %s and %s",
               cm_type_repr(a.type), cm_type_repr(b.type));
        return VALUE_ERROR;
}

Value
vadd(Value a, Value b)
{
        if (are_valid_operands(a, b)) {
                return AS_NUMBER(a.as.num + b.as.num);
        }

        if (a.type == TYPE_NUMBER) return a;
        if (b.type == TYPE_NUMBER)
                return b;
        else
                return AS_NUMBER(0);

        report("No yet implemented: vadd for %s and %s",
               cm_type_repr(a.type), cm_type_repr(b.type));
        return VALUE_ERROR;
}

Value
range_map(Expr *lhs, Expr *rhs, Value (*func)(Value, Value))
{
        char *lhs_s;
        char *rhs_s;
        int lhs_x, lhs_y;
        int rhs_x, rhs_y;
        int i;
        Value v;

        if (rhs == NULL || lhs == NULL) {
                report("lhs or rhs are null");
                return VALUE_ERROR;
        }

        if (lhs->type != EXPR_IDENTIFIER) {
                report("Left operand is not an Identifier");
                return VALUE_ERROR;
        }

        if (rhs->type != EXPR_IDENTIFIER) {
                report("Right operand is not an Identifier");
                return VALUE_ERROR;
        }

        lhs_s = cm_get_cell_name(active_ctx.body, lhs->as.identifier.cell);
        if (lhs_s == NULL) {
                report("Left Identifier is not valid");
                return VALUE_ERROR;
        }

        rhs_s = cm_get_cell_name(active_ctx.body, rhs->as.identifier.cell);
        if (rhs_s == NULL) {
                report("Right Identifier is not valid");
                return VALUE_ERROR;
        }

        if (parse_coords(lhs_s, &lhs_x, &lhs_y)) {
                report("Left Identifier Coords can not be parsed");
                return VALUE_ERROR;
        }

        if (parse_coords(rhs_s, &rhs_x, &rhs_y)) {
                report("Left Identifier Coords can not be parsed");
                return VALUE_ERROR;
        }

        report("lhs: %s (%d, %d) ==> rhs: %s (%d, %d)",
               lhs_s, lhs_x, lhs_y,
               rhs_s, rhs_x, rhs_y);

        if (rhs_x < lhs_x) {
                int tmp = rhs_x;
                rhs_x = lhs_x;
                lhs_x = tmp;
        }

        if (rhs_y < lhs_y) {
                int tmp = rhs_y;
                rhs_y = lhs_y;
                lhs_y = tmp;
        }

        v = lhs->as.identifier.cell->value;

        Cell *c;
        if (rhs_x == lhs_x) {
                for (i = lhs_y + 1; i <= rhs_y; i++) {
                        c = cm_get_cell_ptr(active_ctx.body, lhs_x, i);
                        if (c == NULL) break;
                        v = func(c->value, v);
                }
                return v;
        }

        if (rhs_y == lhs_y) {
                for (i = lhs_x + 1; i <= rhs_x; i++) {
                        c = cm_get_cell_ptr(active_ctx.body, i, lhs_y);
                        if (c == NULL) break;
                        v = func(c->value, v);
                }
                return v;
        }

        return VALUE_ERROR;
}

Value
vadd_range(Expr *lhs, Expr *rhs)
{
        return range_map(lhs, rhs, vadd);
}

Value
vmul_range(Expr *lhs, Expr *rhs)
{
        return range_map(lhs, rhs, vmul);
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

        if (!strcmp(e->as.binop.op, "++")) return vadd_range(e->as.binop.lhs, e->as.binop.rhs);
        if (!strcmp(e->as.binop.op, "**")) return vmul_range(e->as.binop.lhs, e->as.binop.rhs);

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
