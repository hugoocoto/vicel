#include "formula.h"
#include "cellmap.h"
#include "common.h"
#include "da.h"
#include "debug.h"
#include "window.h"

Cell *cell_self = NULL;

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

        if (strcmp(e->as.unop.op, "-")) {
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
vdiv(Value a, Value b)
{
        if (a.type != b.type) return VALUE_EMPTY;

        if (a.type == TYPE_NUMBER) {
                return AS_NUMBER(a.as.num / b.as.num);
        }

        report("No yet implemented: vadd for %s and %s",
               cm_type_repr(a.type), cm_type_repr(b.type));
        return VALUE_EMPTY;
}

Value
vmul(Value a, Value b)
{
        if (a.type != b.type) return VALUE_EMPTY;

        if (a.type == TYPE_NUMBER) {
                return AS_NUMBER(a.as.num * b.as.num);
        }

        report("No yet implemented: vadd for %s and %s",
               cm_type_repr(a.type), cm_type_repr(b.type));
        return VALUE_EMPTY;
}

Value
vpow(Value a, Value b)
{
        if (a.type != b.type) return VALUE_EMPTY;

        if (a.type == TYPE_NUMBER) {
                return AS_NUMBER(pow(a.as.num, b.as.num));
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

        if (!strcmp(e->as.binop.op, "/")) {
                return vdiv(lhs, rhs);
        };

        if (!strcmp(e->as.binop.op, "*")) {
                return vmul(lhs, rhs);
        };

        if (!strcmp(e->as.binop.op, "^")) {
                return vpow(lhs, rhs);
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
        case EXPR_IDENTIFIER: return eval_identifier(e);
        default:
                report("No yet implemented: eval_expr for %d", e->type);
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

Expr *
new_identifier(Cell *c)
{
        Expr *e = new_expr();
        e->type = EXPR_IDENTIFIER;
        e->as.identifier.cell = c;
        return e;
}

double
get_number(char **c)
{
        return strtod(*c, c);
}

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
TOK_AS_IDENTIFIER(char *id)
{
        Token *t = new_tok();
        t->as.id = id;
        t->type = TOK_IDENTIFIER;
        return t;
}

char *
get_identifier(char **c)
{
        report("call get_identifier with %s", *c);
        char *id = *c;
        if (!isalpha(**c)) {
                report("get_identifier at %c do not match isalpha", **c);
                return NULL;
        }
        while (isalpha(**c)) {
                report("Consume alpha %c", **c);
                ++*c;
        }
        if (**c < '0' || **c > '9') {
                report("get_identifier at %c do not match isdigit", **c);
                return NULL;
        }
        while ('0' <= **c && **c <= '9') {
                report("Consume digit %c", **c);
                ++*c;
        }

        /* without this it segfault???????? */
        if (**c == 0) {
                return strdup(id);
        }

        char prev = **c;
        **c = 0;
        report("call get_identifier id %s", id);
        id = strdup(id);
        **c = prev;
        report("call get_identifier return %s", id);
        return id;
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
                case '/':
                        last->next = TOK_AS_STR("/");
                        last = last->next;
                        ++c;
                        break;
                case '*':
                        last->next = TOK_AS_STR("*");
                        last = last->next;
                        ++c;
                        break;
                case '^':
                        last->next = TOK_AS_STR("^");
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
                        char *id;
                        if ((id = get_identifier(&c))) {
                                last->next = TOK_AS_IDENTIFIER(id);
                                last = last->next;
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
        switch ((*t)->type) {
        case TOK_NUMERIC: {
                double n = (*t)->as.num;
                *t = (*t)->next;
                return new_literal(n);
        }
        case TOK_IDENTIFIER: {
                report("get_literal from identifier %s", (*t)->as.id);
                Cell *c = get_cell_from_coords((*t)->as.id);
                *t = (*t)->next;

                if (c == NULL) { // invalid coords
                        return new_literal(0);
                }

                assert(cell_self);
                cm_subscribe(c, cell_self);

                return new_identifier(c);
        }
        case TOK_STRING:
        default:
                report("No yet implemented: get_literal for %d", (*t)->type);
                exit(123);
        }
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
        case EXPR_IDENTIFIER:
                report("%-*sIDENTIFIER:", indent, "");
                indent += 4;
                report("%-*scell value: %s", indent, "", e->as.identifier.cell->repr);
                indent -= 4;
                break;
        default:
                report("No yet implemented: report_ast for %d", e->type);
                break;
        }
        return e;
}

Expr *
parse_formula(char *c, Cell *self)
{
        cell_self = self;
        Token *t = lexer(c);
        // - term -> factor (("-" | "+") factor)*
        // - factor -> unary (("/" | "\*") unary)*
        // - unary -> ("!" | "-") unary | group
        // - group -> "(" expr ")" | literal
        // - literal -> NUM | STR | "true" | "false" | IDENTIFIER
        return report_ast(get_term(&t));
}

void
build_formula(char *_str, Cell *self)
{
        char *str = strdup(_str);
        self->value.as.formula = calloc(1, sizeof(Formula));
        self->value.type = TYPE_FORMULA;

        if (*str != '=') {
                report("Invalid formula: `%s` does not start with `=`", str);
                exit(2);
        }

        self->value.as.formula->body = parse_formula(str + 1, self);
        self->value.as.formula->value =
        eval_expr(report_ast(self->value.as.formula->body));

        free(str);
        assert(self->value.type == TYPE_FORMULA);
}

static __attribute__((constructor)) void
test()
{
        // assert(eval_expr(report_ast(parse_formula(""))).as.num == 0);
        // assert(eval_expr(report_ast(parse_formula("1"))).as.num == 1);
        // assert(eval_expr(report_ast(parse_formula("1.6"))).as.num == 1.6);
        // assert(eval_expr(report_ast(parse_formula("-1.6"))).as.num == -1.6);
        // assert(eval_expr(report_ast(parse_formula("1 + 1.6"))).as.num == 2.6);
        // assert(eval_expr(report_ast(parse_formula("1.0 - 1.0"))).as.num == 0.0);
        // assert(eval_expr(report_ast(parse_formula("-1.0 - 1.0 + 2.0"))).as.num == 0.0);
}

void
cm_notify(Cell *actor, Cell *observer)
{
        if (observer->value.type != TYPE_FORMULA) {
                report("Invalid cm_notify for observer type %s",
                       cm_type_repr(observer->value.type));
                exit(987);
        }

        observer->value.as.formula->value = eval_expr(observer->value.as.formula->body);
        free(observer->repr);
        observer->repr = get_repr(observer->value.as.formula->value);

        for_da_each(c, observer->subscribers)
        {
                cm_notify(observer, *c);
        }
}

void
destroy_formula(Formula f)
{
}

void
free_formula_subscribers(Cell *c)
{
        for_da_each(a, c->value.as.formula->subscribed)
        {
                cm_unsubscribe(*a, c);
        }
}
