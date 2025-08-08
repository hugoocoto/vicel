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

#include "formula.h"
#include "cellmap.h"
#include "common.h"
#include "da.h"
#include "debug.h"
#include "window.h"
#include <ctype.h>

Cell *cell_self = NULL;


void
refresh_formula_value(Cell *cell)
{
        cell->value.as.formula->value = eval_formula(*cell->value.as.formula);
        free(cell->repr);
        cell->repr = get_repr(cell->value.as.formula->value);

        for_da_each(c, cell->subscribers) cm_notify(cell, *c);
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
TOK_AS_STR(char *c, int len)
{
        Token *t = new_tok();
        char prev = c[len];
        c[len] = 0;
        t->as.str = strdup(c);
        c[len] = prev;
        t->type = TOK_STRING;
        report("tok as str: %s", t->as.str);
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
        // report("call get_identifier with %s", *c);
        char *id = *c;
        if (!isalpha(**c)) {
                report("get_identifier at %c do not match isalpha", **c);
                return NULL;
        }
        while (isalpha(**c)) {
                // report("Consume alpha %c", **c);
                ++*c;
        }
        if (**c < '0' || **c > '9') {
                report("get_identifier at %c do not match isdigit", **c);
                return NULL;
        }
        while ('0' <= **c && **c <= '9') {
                // report("Consume digit %c", **c);
                ++*c;
        }

        /* without this it segfault???????? */
        if (**c == 0) {
                return strdup(id);
        }

        char prev = **c;
        **c = 0;
        // report("call get_identifier id %s", id);
        id = strdup(id);
        **c = prev;
        // report("call get_identifier return %s", id);
        return id;
}

Token *
lexer(char *c)
{
        report("Lexer for `%s`", c);
        Token *last = new_tok();
        Token *zero = last;
        while (*c) {
                switch (*c) {
                case '-':
                case '/':
                case '^':
                case '(':
                case ')':
                        last->next = TOK_AS_STR(c, 1);
                        last = last->next;
                        ++c;
                        break;
                case '+':
                case '*':
                        if (c[1] == c[0]) {
                                last->next = TOK_AS_STR(c, 2);
                                last = last->next;
                                c += 2;
                                break;
                        }
                        last->next = TOK_AS_STR(c, 1);
                        last = last->next;
                        ++c;
                        break;
                case '0' ... '9':
                        last->next = TOK_AS_NUM(strtod(c, &c));
                        last = last->next;
                        break;
                default:
                        if (isspace(*c)) {
                                while (isspace(*c))
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
                if (c == NULL) {
                        report("Can not get cell from id: %s", (*t)->as.id);
                        return NULL;
                }
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
                exit(ERR_GETLITERAL);
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
                        exit(ERR_EXPECT);
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
get_power(Token **t)
{
        Expr *e = get_unary(t);
        Token *op;
        while ((op = match(t, "^"))) {
                e = new_binop(e, op->as.str, get_unary(t));
        }
        return e;
}

Expr *
get_factor(Token **t)
{
        Expr *e = get_power(t);
        Token *op;
        while ((op = match(t, "/")) ||
               (op = match(t, "*")) || (op = match(t, "**"))) {
                e = new_binop(e, op->as.str, get_power(t));
        }
        return e;
}

Expr *
get_term(Token **t)
{
        Expr *e = get_factor(t);
        Token *op;
        while ((op = match(t, "-")) ||
               (op = match(t, "+")) || (op = match(t, "++"))) {
                e = new_binop(e, op->as.str, get_factor(t));
        }
        return e;
}

void
get_ast_repr(Expr *e, char *buffer)
{
        if (e == NULL) return;
        switch (e->type) {
        case EXPR_LITERAL:
                sprintf(buffer + strlen(buffer), "%g ", e->as.literal.value.as.num);
                break;
        case EXPR_BIN:
                get_ast_repr(e->as.binop.lhs, buffer);
                sprintf(buffer + strlen(buffer), "%s ", e->as.binop.op);
                get_ast_repr(e->as.binop.rhs, buffer);
                break;
        case EXPR_UN:
                sprintf(buffer + strlen(buffer), "%s ", e->as.unop.op);
                get_ast_repr(e->as.unop.rhs, buffer);
                break;
        case EXPR_IDENTIFIER: {
                char *c;
                sprintf(buffer + strlen(buffer), "%s ",
                        c = cm_get_cell_name(active_ctx.body, e->as.identifier.cell));
                free(c);
                break;
        }
        default:
                report("No yet implemented: get_ast_repr for %d", e->type);
                exit(ERR_REPAST);
        }
}

Expr *
report_ast(Expr *e)
{
        if (e == NULL) return e;

        char buffer[1024] = { 0 };
        get_ast_repr(e, buffer);
        report("Ast: %s", buffer);
        return e;
}

void
free_tokens(Token *t)
{
        Token *prev;
        while (t) {
                switch (t->type) {
                case TOK_STRING:
                        free(t->as.str);
                        break;
                case TOK_IDENTIFIER:
                        free(t->as.id);
                        break;
                case TOK_NUMERIC:
                        break;
                default:
                        report("No yet implemented: free_tokens for %d", t->type);
                }
                prev = t;
                t = t->next;
                free(prev);
        }
}

Expr *
parse_formula(char *c, Cell *self)
{
        cell_self = self;
        Token *t = lexer(c);
        Token *tt = t;
        // - term -> factor (("-" | "+") factor)*
        // - factor -> power (("/" | "\*") power)*
        // - power -> unary ("^") unary)*
        // - unary -> ("!" | "-") unary | group
        // - group -> "(" expr ")" | literal
        // - literal -> NUM | STR | "true" | "false" | IDENTIFIER
        Expr *e = report_ast(get_term(&t));
        self->value.as.formula->tokens = tt;
        return e;
}

void
clear_cell(Cell *c)
{
        cm_clear_cell(c);
        __auto_type s = c->subscribers;
        *c = EMPTY_CELL;
        c->subscribers = s;
}

void
build_formula(char *_str, Cell *self)
{
        char *str = strdup(_str);

        if (*str != '=') {
                report("Invalid formula: `%s` does not start with `=`", str);
                exit(ERR_INVFORM);
        }

        clear_cell(self);
        self->value.as.formula = calloc(1, sizeof(Formula));
        self->value.type = TYPE_FORMULA;
        self->value.as.formula->body = parse_formula(str + 1, self);
        refresh_formula_value(self);

        free(self->input_repr);
        self->input_repr = get_input_repr(self->value);

        free(str);
        assert(self->value.type == TYPE_FORMULA);
}

void
cm_notify(Cell *actor, Cell *observer)
{
        actor = actor;
        if (observer->value.type != TYPE_FORMULA) {
                report("Invalid cm_notify for observer type %s",
                       cm_type_repr(observer->value.type));
                exit(ERR_OBSVAL);
        }

        refresh_formula_value(observer);
}

void
free_expr(Expr *e)
{
        if (e == NULL) return;
        switch (e->type) {
        case EXPR_BIN:
                free_expr(e->as.binop.lhs);
                free_expr(e->as.binop.rhs);
                break;
        case EXPR_UN:
                free_expr(e->as.unop.rhs);
                break;
        case EXPR_LITERAL:
        case EXPR_IDENTIFIER:
                break;
        default:
                report("No yet implemented: free_expr for %d", e->type);
        }
        free(e);
}

void
destroy_formula(Cell *c)
{
        assert(c->value.type == TYPE_FORMULA);
        for_da_each(a, c->value.as.formula->subscribed) cm_unsubscribe(*a, c);
        da_destroy(&c->value.as.formula->subscribed);
        free_expr(c->value.as.formula->body);
        free_tokens(c->value.as.formula->tokens);
        free(c->value.as.formula);
}

static Token *
dup_tokens(Token *t)
{
        if (t == NULL) return NULL;

        Token *last = calloc(1, sizeof(Token));
        Token *ret = last;

        while (t) {
                last->type = t->type;
                switch (last->type) {
                case TOK_STRING:
                        last->as.str = strdup(t->as.str);
                        break;
                case TOK_IDENTIFIER:
                        last->as.id = strdup(t->as.id);
                        break;
                default:
                        last->as = t->as;
                        break;
                }
                t = t->next;
                if (t)
                        last = last->next = calloc(1, sizeof(Token));
        }
        return ret;
}

char *
create_id(int r, int c)
{
        char buf[32];
        int start = 0;
        do {
                memcpy(buf + 1, buf, start + 1);
                buf[start] = 'A' + c % ('Z' - 'A' + 1);
                c /= 'Z' - 'A' + 1;
                ++start;
        } while (c);
        snprintf(buf + start, sizeof buf - start, "%d", r);
        return strdup(buf);
}

static __attribute__((constructor)) void
test_create_id()
{
        char *c0, *c1, *c2;
        assert(!strcmp(c0 = create_id(0, 0), "A0"));
        assert(!strcmp(c1 = create_id(2, 0), "A2"));
        assert(!strcmp(c2 = create_id(0, 2), "C0"));
        free(c0);
        free(c1);
        free(c2);
}

static int
extend_identifiers(Token *t, int r, int c)
{
        // return 0 on success
        while (t) {
                if (t->type == TOK_IDENTIFIER) {
                        int rr, cc;
                        if (parse_coords(t->as.id, &cc, &rr)) {
                                report("Impossible to parse coords at: %s", t->as.id);
                                return 1;
                        }
                        cc += c;
                        rr += r;
                        report("Parse coords on `%s` (%+d, %+d) -> (%d, %d) "
                               "with the following result:",
                               t->as.id, c, r, cc, rr);
                        free(t->as.id);
                        t->as.id = create_id(rr, cc);
                        report("%s", t->as.id);
                }
                t = t->next;
        }
        return 0;
}

Formula *
formula_extend(Cell *self, Formula *f, int r, int c)
{
        Formula *new = calloc(1, sizeof *f);
        self->value.as.formula = new;
        self->value.type = TYPE_FORMULA;
        Token *t = new->tokens = dup_tokens(f->tokens);
        cell_self = self;
        report("set cell_self to %p", cell_self);
        if (extend_identifiers(t, r, c)) {
                report("Can not extend formula");
                return NULL;
        }
        report(">>> formula extend <<<<<<<<<<<");
        new->body = report_ast(get_term(&t));
        new->value = eval_formula(*new);
        report(">>> formula extend end <<<<<<<");
        return new;
}
