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
#include "eval.h"
#include "window.h"
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

Cell *cell_self = NULL;

Value
build_range(Cell *cstart, Cell *cend)
{
        Value r = (Value) { .type = TYPE_RANGE };
        char *cs;

        cs = cm_get_cell_name(active_ctx.body, cstart);
        if (cs == NULL) return VALUE_ERROR;
        if (parse_coords(cs, &r.as.range.startx, &r.as.range.starty)) {
                free(cs);
                return VALUE_ERROR;
        }
        free(cs);

        cs = cm_get_cell_name(active_ctx.body, cend);
        if (cs == NULL) return VALUE_ERROR;
        if (parse_coords(cs, &r.as.range.endx, &r.as.range.endy)) {
                free(cs);
                return VALUE_ERROR;
        }
        free(cs);

        /* should add subscribers */
        int x, y;
        Cell *c;
        for (x = r.as.range.startx; x <= r.as.range.endx; x++) {
                for (y = r.as.range.starty; y <= r.as.range.endy; y++) {
                        c = cm_get_cell_ptr(active_ctx.body, x, y);
                        if (!c) break;
                        assert(cell_self);
                        cm_subscribe(c, cell_self);
                }
        }

        return r;
}

void
refresh_formula_value(Cell *cell)
{
        if (cell->updated) {
                cell->value.as.formula->value = VALUE_ERROR;
                return;
        };
        cell->updated = true;
        cell->value.as.formula->value = eval_expr(cell->value.as.formula->body);
        free(cell->repr);
        cell->repr = get_repr(cell->value.as.formula->value);

        for_da_each(c, cell->subscribers) cm_notify(cell, *c);
        cell->updated = false;
}

Expr *
new_expr()
{
        return calloc(1, sizeof(Expr));
}

Expr *
new_function(Expr *name, Expr *args)
{
        report("Creating new fun call");
        Expr *e = new_expr();
        e->type = EXPR_FUNC;
        e->as.func.name = name;
        e->as.func.args = args;
        return e;
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
new_literal_str(char *c)
{
        Expr *e = new_expr();
        e->type = EXPR_LITERAL;
        e->as.literal.value = AS_TEXT(strdup(c));
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
new_range(Cell *cstart, Cell *cend)
{
        Expr *e = new_expr();
        e->type = EXPR_LITERAL;
        e->as.literal.value = build_range(cstart, cend);
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

        if (len > 0) {
                char prev = c[len];
                if (prev) c[len] = 0;
                t->as.str = strdup(c);
                if (prev) c[len] = prev;
        } else
                t->as.str = strdup("");

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
        report("As id: %s", id);
        Token *t = new_tok();
        t->as.id = id;
        t->type = TOK_IDENTIFIER;
        return t;
}

char *
get_identifier(char **c)
{
        char *id = *c;
        while (isalnum(**c)) {
                ++*c;
        }

        char prev = **c;
        if (prev) **c = 0;
        id = strdup(id);
        if (prev) **c = prev;
        return id;
}

void
free_token(Token *t)
{
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
        free(t);
}

void
free_tokens(Token *t)
{
        Token *next;
        while (t) {
                next = t->next;
                free_token(t);
                t = next;
        }
}


Token *
early_expansion(Token *t)
{
        report("Do not call early_expansion. Todo: remove this");
        return t;
        Token *start = t;
        Token *prev = NULL;
        Token *last;
        int x1, x2, y1, y2;
        int x, y;
        int x_inc, y_inc;
        Cell *c;
        char *cn;
        int nest = 0;

        while (t) {
                /* I know I can optimize it but is better looking this way */
                if (t->type != TOK_STRING) goto cont;
                if (strcmp(t->as.str, ")") == 0) --nest;
                if (strcmp(t->as.str, "(") == 0) ++nest;
                if (strcmp(t->as.str, ":") == 0) {
                        if (!prev || prev->type != TOK_IDENTIFIER) goto cont;
                        if (!t->next || t->next->type != TOK_IDENTIFIER) goto cont;
                        if (parse_coords(prev->as.id, &x1, &y1)) goto cont;
                        if (parse_coords(t->next->as.id, &x2, &y2)) goto cont;

                        if (x1 > x2) {
                                int tmp = x1;
                                x1 = x2;
                                x2 = tmp;
                        }

                        if (y1 > y2) {
                                int tmp = y1;
                                y1 = y2;
                                y2 = tmp;
                        }

                        /* What a chunk of code xd */

                        x_inc = x2 != x1;
                        assert(x_inc == 0 || x_inc == 1);
                        y_inc = y2 != y1;
                        assert(y_inc == 0 || y_inc == 1);
                        x = x1 + x_inc;
                        y = y1 + y_inc;
                        last = prev;
                        last->next = TOK_AS_STR(nest ? "," : "+", 1);
                        last = last->next;

                        while ((x1 == x2 || x < x2) &&
                               (y1 == y2 || y < y2)) {
                                c = cm_get_cell_ptr(active_ctx.body, x, y);
                                if (!c) break;
                                cn = cm_get_cell_name(active_ctx.body, c);
                                last->next = TOK_AS_IDENTIFIER(cn);
                                last = last->next;
                                last->next = TOK_AS_STR(nest ? "," : "+", 1);
                                last = last->next;
                                x += x_inc;
                                y += y_inc;
                        }

                        last->next = t->next;
                        Token *f = t;
                        prev = last;
                        t = t->next;
                        free_token(f);

                        continue;
                }
        cont:
                prev = t;
                t = t->next;
        }
        return start;
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
                case ',':
                case ';':
                case ':':
                case '+':
                case '*':
                        last->next = TOK_AS_STR(c, 1);
                        last = last->next;
                        ++c;
                        break;
                case '<':
                case '>':
                case '!':
                case '=':
                        if (c[1] == '=')
                                last->next = TOK_AS_STR(c++, 2);
                        else
                                last->next = TOK_AS_STR(c, 1);
                        last = last->next;
                        ++c;
                        break;

                case '\'': {
                        size_t len = strcspn(c + 1, "'");
                        last->next = TOK_AS_STR(c + 1, len);
                        last = last->next;
                        c += len + 2;
                        break;
                }

                case '0' ... '9': {
                        char *c0 = c;
                        last->next = TOK_AS_NUM(strtod(c, &c));
                        last = last->next;
                        if (c0 == c) {
                                /* should never happen */
                                report("Can not convert %*s to number", 5, c);
                                exit(19);
                        }
                        break;
                }

                default:
                        if (isspace(*c)) {
                                while (isspace(*c))
                                        ++c;
                                break;
                        }

                        char *id;
                        if ((id = get_identifier(&c))) {
                                if (*id == 0) {
                                        free(id);
                                        report("can not get identifier");
                                        last->next = TOK_AS_IDENTIFIER("Error");
                                        last = last->next;
                                        break;
                                }
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

        return early_expansion(r);
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

void free_expr(Expr *e);

Expr *
get_literal(Token **t)
{
        if (*t == NULL) return NULL;
        switch ((*t)->type) {
        case TOK_STRING: {
                char *s = (*t)->as.str;
                *t = (*t)->next;
                return new_literal_str(s);
        }

        case TOK_NUMERIC: {
                double n = (*t)->as.num;
                *t = (*t)->next;
                return new_literal(n);
        }

        case TOK_IDENTIFIER: {
                char *id = (*t)->as.id;

                report("get_literal from identifier %s", id);
                Cell *cell = get_cell_from_coords(id);
                *t = (*t)->next;

                if (cell == NULL) {
                        return new_literal_str(id);
                }

                if (match(t, ":")) {
                        Expr *e = get_literal(t);
                        if (e->type != EXPR_IDENTIFIER) {
                                report("Invalid range");
                                exit(ERR_INVRANGE);
                        }
                        assert(cell_self);

                        /* Should notify into new_range */
                        Expr *ret = new_range(cell, e->as.identifier.cell);
                        free_expr(e);
                        return ret;
                }

                assert(cell_self);
                cm_subscribe(cell, cell_self);

                return new_identifier(cell);
        }
        default:
                report("No yet implemented: get_literal for %d", (*t)->type);
                exit(ERR_GETLITERAL);
        }
}

Expr *get_comparison(Token **);

Expr *
get_function(Token **t)
{
        report("get function");
        Expr *e = get_literal(t);
        Expr *args = NULL;
        Expr *last;

        if (match(t, "(")) {
                while (!match(t, ")")) {
                        if (args == NULL) {
                                args = get_comparison(t);
                                if (args == NULL) {
                                        free_expr(e);
                                        return new_literal_str("Error");
                                }
                                report("Adding argument");
                                last = args;
                                continue;
                        }
                        if (!match(t, ",")) {
                                while (args) {
                                        last = args->next;
                                        free_expr(args);
                                        args = last;
                                }
                                // todo: invalid formula
                                report("Expected parenthesis at formula");
                                free_expr(e);
                                return new_literal_str("Error");
                        }
                        last->next = get_comparison(t);
                        report("Adding argument");
                        last = last->next;
                }
                return new_function(e, args);
        }
        return e;
}

Expr *
get_group(Token **t)
{
        if (match(t, "(")) {
                Expr *e = get_comparison(t);
                if (!match(t, ")")) {
                        // todo: invalid formula
                        report("Expected parenthesis at formula");
                        free_expr(e);
                        return new_literal_str("Error");
                }
                return e;
        }
        return get_function(t);
}

Expr *
get_unary(Token **t)
{
        Token *op;
        if ((op = match(t, "-")) || (op = match(t, "+"))) {
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
        while ((op = match(t, "/")) || (op = match(t, "*"))) {
                e = new_binop(e, op->as.str, get_power(t));
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
get_comparison(Token **t)
{
        Expr *e = get_term(t);
        Token *op;
        while ((op = match(t, "<")) || (op = match(t, "<=")) ||
               (op = match(t, ">")) || (op = match(t, ">=")) ||
               (op = match(t, "==")) || (op = match(t, "!="))) {
                e = new_binop(e, op->as.str, get_factor(t));
        }
        return e;
}

void
get_ast_repr(Expr *e, char *buffer, size_t len)
{
        static bool is_func_param = false;
        if (e == NULL) return;
        if (strlen(buffer) >= len) return;

        switch (e->type) {
        case EXPR_LITERAL:
                switch (e->as.literal.value.type) {
                case TYPE_NUMBER:
                        snprintf(buffer + strlen(buffer), len, "%g",
                                 e->as.literal.value.as.num);
                        break;
                case TYPE_TEXT:
                        snprintf(buffer + strlen(buffer), len,
                                 is_func_param ? "'%s'" : "%s",
                                 e->as.literal.value.as.text);
                        break;
                case TYPE_EMPTY:
                case TYPE_FORMULA:
                case TYPE_RANGE: {
                        char *c = get_input_repr(e->as.literal.value);
                        snprintf(buffer + strlen(buffer), len, "%s", c);
                        free(c);
                        break;
                }
                default: break;
                }
                break;
        case EXPR_BIN:
                get_ast_repr(e->as.binop.lhs, buffer, len);
                snprintf(buffer + strlen(buffer), len, "%s", e->as.binop.op);
                get_ast_repr(e->as.binop.rhs, buffer, len);
                break;
        case EXPR_UN:
                snprintf(buffer + strlen(buffer), len, "%s", e->as.unop.op);
                get_ast_repr(e->as.unop.rhs, buffer, len);
                break;
        case EXPR_IDENTIFIER: {
                char *c;
                snprintf(buffer + strlen(buffer), len, "%s",
                         c = cm_get_cell_name(active_ctx.body,
                                              e->as.identifier.cell));
                free(c);
                break;
        }
        case EXPR_FUNC: {
                get_ast_repr(e->as.func.name, buffer, len);
                snprintf(buffer + strlen(buffer), len, "(");
                Expr *args = e->as.func.args;
                if (args) {
                        is_func_param = true;
                        get_ast_repr(args, buffer, len);
                        while ((args = args->next)) {
                                snprintf(buffer + strlen(buffer), len, ",");
                                get_ast_repr(args, buffer, len);
                        }
                        is_func_param = false;
                }
                snprintf(buffer + strlen(buffer), len, ")");
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

        char buffer[128] = { 0 };
        get_ast_repr(e, buffer, sizeof buffer - 1);
        report("Ast: %s", buffer);
        return e;
}

Expr *
parse_formula(char *c, Cell *self)
{
        cell_self = self;
        Token *t = lexer(c);
        report("Out of lexer");
        Token *tt = t;
        // - comparison -> term ((">" | ">=" | "<" | "<=" | "==" | "!=") term)?
        // - term -> factor (("-" | "+") factor)*
        // - factor -> power (("/" | "\*") power)*
        // - power -> unary ("^") unary)*
        // - unary -> ("!" | "-") unary | group
        // - group -> "(" expr ")" | func
        // - func -> FUNC "(" expr? ("," expr)* ")" | literal
        // - literal -> NUM  | IDENTIFIER
        Expr *e = report_ast(get_comparison(&t));
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
        if (actor == observer) return;
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
                if (e->as.literal.value.type == TYPE_TEXT)
                        free(e->as.literal.value.as.text);
                break;
        case EXPR_IDENTIFIER:
                break;
        case EXPR_FUNC: {
                Expr *cur = e->as.func.args;
                Expr *next;
                while (cur) {
                        next = cur->next;
                        free_expr(cur);
                        cur = next;
                }
                free_expr(e->as.func.name);
                break;
        }
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
        char buf[8];
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
        int rr, cc;
        while (t) {
                if (t->type == TOK_IDENTIFIER) {
                        if (!parse_coords(t->as.id, &cc, &rr)) {
                                cc += c;
                                rr += r;
                                free(t->as.id);
                                t->as.id = create_id(rr, cc);
                        }
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
        new->body = report_ast(get_comparison(&t));
        new->value = eval_formula(*new);
        report(">>> formula extend end <<<<<<<");
        return new;
}
