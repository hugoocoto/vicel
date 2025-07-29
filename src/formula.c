#include "formula.h"
#include "cellmap.h"
#include "common.h"
#include "debug.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// EXPR_BIN,
// EXPR_UN,
// EXPR_LITERAL,

Value eval_expr(Expr *e);

Value
eval_identifier(Expr *e)
{
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

        if (strcmp(e->as.binop.op, "-")) {
                return AS_NUMBER(-rhs.as.num);
        };
        report("No yet implemented: unop for %s", e->as.unop.op);
        return VALUE_EMPTY;
}

Value
vsub(Value a, Value b)
{
        if (a.type != b.type) return VALUE_EMPTY;

        if (a.type == TYPE_NUMBER) {
                return AS_NUMBER(a.as.num - b.as.num);
        }

        report("No yet implemented: vadd for %s and %s",
               cm_type_repr(a.type), cm_type_repr(b.type));
        return VALUE_EMPTY;
}

Value
vadd(Value a, Value b)
{
        if (a.type != b.type) return VALUE_EMPTY;

        if (a.type == TYPE_NUMBER) {
                return AS_NUMBER(a.as.num + b.as.num);
        }

        report("No yet implemented: vadd for %s and %s",
               cm_type_repr(a.type), cm_type_repr(b.type));
        return VALUE_EMPTY;
}

Value
eval_binop(Expr *e)
{
        Value lhs = eval_expr(e->as.binop.lhs);
        Value rhs = eval_expr(e->as.binop.rhs);

        if (!strcmp(e->as.binop.op, "+")) {
                return vadd(lhs, rhs);
        };

        if (!strcmp(e->as.binop.op, "-")) {
                return vsub(lhs, rhs);
        };

        report("No yet implemented: binop for %s", e->as.binop.op);
        return VALUE_EMPTY;
}

Value
eval_expr(Expr *e)
{
        if (e == NULL) return AS_NUMBER(0);

        switch (e->type) {
        case EXPR_LITERAL: return eval_literal(e);
        case EXPR_BIN: return eval_binop(e);
        case EXPR_UN: return eval_unop(e);
        default:
                report("No yet implemented: binop for %s", e->as.binop.op);
                return VALUE_EMPTY;
        }
}

Value
eval_formula(Formula f)
{
        if (!f.body) return VALUE_EMPTY;
        return eval_expr(f.body);
}

Expr *
new_expr()
{
        return calloc(1, sizeof(Expr));
}

Expr *
new_binop(Expr *lhs, char *op, Expr *rhs)
{
        Expr *e = new_expr();
        e->type = EXPR_BIN;
        e->as.binop.lhs = lhs;
        e->as.binop.op = op;
        e->as.binop.rhs = rhs;
        return e;
}

Expr *
new_unop(char *op, Expr *rhs)
{
        Expr *e = new_expr();
        e->type = EXPR_UN;
        e->as.unop.op = op;
        e->as.unop.rhs = rhs;
        return e;
}

Expr *
new_literal(double value)
{
        Expr *e = new_expr();
        e->type = EXPR_LITERAL;
        e->as.literal.value = AS_NUMBER(value);
        return e;
}

double
get_number(char **c)
{
        return strtod(*c, c);
}

typedef struct Token {
        union {
                char *str;
                double num;
        } as;
        enum {
                TOK_STRING,
                TOK_NUMERIC,
        } type;
        struct Token *next;
} Token;

Token *
new_tok()
{
        return calloc(1, sizeof(Token));
}

Token *
TOK_AS_STR(char *c)
{
        Token *t = new_tok();
        t->as.str = strdup(c);
        t->type = TOK_STRING;
        return t;
}

Token *
TOK_AS_NUM(double n)
{
        Token *t = new_tok();
        t->as.num = n;
        t->type = TOK_NUMERIC;
        return t;
}

Token *
lexer(char *c)
{
        Token *last = new_tok();
        Token *zero = last;
        while (*c) {
                switch (*c) {
                case '+':
                        last->next = TOK_AS_STR("+");
                        last = last->next;
                        ++c;
                        break;
                case '-':
                        last->next = TOK_AS_STR("-");
                        last = last->next;
                        ++c;
                        break;
                case '0' ... '9':
                        last->next = TOK_AS_NUM(strtod(c, &c));
                        last = last->next;
                        break;
                default:
                        if (isspace(*c)) {
                                ++c;
                                break;
                        }
                        report("Invalid lexeme found: `%c`", *c);
                        ++c;
                        break;
                }
        }

        Token *r = zero->next;
        free(zero);
        return r;
}

Token *
match(Token **t, char *str)
{
        if (*t == NULL) return NULL;
        if ((*t)->type != TOK_STRING) return NULL;

        if (!strcmp((*t)->as.str, str)) {
                Token *cur = *t;
                *t = (*t)->next;
                return cur;
        }
        return NULL;
}

Expr *
get_literal(Token **t)
{
        if (*t == NULL) return NULL;
        double n = (*t)->as.num;
        *t = (*t)->next;
        return new_literal(n);
}

Expr *get_term(Token **);

Expr *
get_group(Token **t)
{
        if (match(t, "(")) {
                Expr *e = get_term(t);
                if (!match(t, ")")) {
                        // todo: invalid formula
                        report("Expected parenthesis at formula");
                        exit(112);
                }
                return e;
        }
        return get_literal(t);
}

Expr *
get_unary(Token **t)
{
        Token *op;
        if ((op = match(t, "-"))) {
                return new_unop(op->as.str, get_unary(t));
        }
        return get_group(t);
}

Expr *
get_factor(Token **t)
{
        Expr *e = get_unary(t);
        Token *op;
        while ((op = match(t, "/")) || (op = match(t, "*"))) {
                e = new_binop(e, op->as.str, get_unary(t));
        }
        return e;
}

Expr *
get_term(Token **t)
{
        Expr *e = get_factor(t);
        Token *op;
        while ((op = match(t, "-")) || (op = match(t, "+"))) {
                e = new_binop(e, op->as.str, get_factor(t));
        }
        return e;
}

Expr *
parse_formula(char *c)
{
        Token *t = lexer(c);
        // - term -> factor (("-" | "+") factor)*
        // - factor -> unary (("/" | "\*") unary)*
        // - unary -> ("!" | "-") unary | group
        // - group -> "(" expr ")" | literal
        // - literal -> NUM | STR | "true" | "false" | IDENTIFIER
        return get_term(&t);
}

Formula
build_formula(char *str)
{
        Formula f;
        if (*str != '=') {
                report("Invalid formula: `%s` does not start with `=`", str);
                exit(2);
        }
        f.body = parse_formula(str);
        f.value = eval_expr(f.body);
        return f;
}

Expr *
report_ast(Expr *e)
{
        static int indent = 0;

        if (e == NULL) return e;

        switch (e->type) {
        case EXPR_LITERAL:
                report("%-*sLIT: %g", indent, "", e->as.literal.value.as.num);
                break;
        case EXPR_BIN:
                report("%-*sBINOP: %s", indent, "", e->as.binop.op);
                indent += 4;
                report_ast(e->as.binop.lhs);
                report_ast(e->as.binop.rhs);
                indent -= 4;
                break;
        case EXPR_UN:
                report("%-*sUNOP: %s", indent, "", e->as.unop.op);
                indent += 4;
                report_ast(e->as.unop.rhs);
                indent -= 4;
                break;
        default:
                report("No yet implemented: report_ast for %d", e->type);
                break;
        }
        return e;
}

static __attribute__((constructor)) void
test()
{
        assert(eval_expr(report_ast(parse_formula(""))).as.num == 0);
        assert(eval_expr(report_ast(parse_formula("1"))).as.num == 1);
        assert(eval_expr(report_ast(parse_formula("1.6"))).as.num == 1.6);
        assert(eval_expr(report_ast(parse_formula("-1.6"))).as.num == -1.6);
        assert(eval_expr(report_ast(parse_formula("1 + 1.6"))).as.num == 2.6);
        assert(eval_expr(report_ast(parse_formula("1.0 - 1.0"))).as.num == 0.0);
        assert(eval_expr(report_ast(parse_formula("-1.0 - 1.0 + 2.0"))).as.num == 0.0);
}

void
destroy_formula(Formula f)
{
}
