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
        char buf[64] = { 0 };
        get_ast_repr(e, buf);
        report("Eval function: %s", buf);

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
