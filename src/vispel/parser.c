/* VISPEL parser - Create AST from tokens
 *
 * Author: Hugo Coto Florez
 * Repo: github.com/hugocotoflorez/vispel
 *
 * */

#include <assert.h>
#include <fcntl.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "env.h"
#include "interpreter.h"
#include "tokens.h"

vtok *current_token = NULL;
Stmt *head_stmt = NULL;
jmp_buf panik_jmp;

static inline void
panik_exit()
{
        longjmp(panik_jmp, 1);
}

static void
print_ast_expr_branch(Expr *e)
{
        static int indent = 0;
        const int indent_size = 2;
        printf("Expression: %s\n", EXPR_REPR[e->type]);
        ++indent;

        switch (e->type) {
        case ASSIGNEXPR:
                printf("%*s", indent * indent_size, "");
                printf("- [name] %s\n", e->assignexpr.name->str_literal);
                printf("%*s", indent * indent_size, "");
                printf("- [value] %s\n", TOKEN_REPR[e->assignexpr.value->type]);
                printf("%*s", indent * indent_size, "");
                print_ast_expr_branch(e->assignexpr.value);
                break;
        case BINEXPR:
                printf("%*s", indent * indent_size, "");
                printf("- [lhs] ");
                print_ast_expr_branch(e->binexpr.lhs);
                printf("%*s", indent * indent_size, "");
                printf("- [op] %s\n", TOKEN_REPR[e->binexpr.op->token]);
                printf("%*s", indent * indent_size, "");
                printf("- [rhs] ");
                print_ast_expr_branch(e->binexpr.rhs);
                break;
        case UNEXPR:
                printf("%*s", indent * indent_size, "");
                printf("- [op] %s\n", TOKEN_REPR[e->unexpr.op->token]);
                printf("%*s", indent * indent_size, "");
                printf("- [rhs] ");
                print_ast_expr_branch(e->unexpr.rhs);
                break;
        case LITEXPR:
                printf("%*s", indent * indent_size, "");
                printf("- [lit] ");
                print_literal(e->litexpr.value);
                printf("\n");
                break;
        case CALLEXPR:
                printf("%*s", indent * indent_size, "");
                printf("- [CALL] name: ");
                print_ast_expr_branch(e->callexpr.name);
                Expr *ex = e->callexpr.args;
                while (ex) {
                        printf("%*s", indent * indent_size, "");
                        printf("- [Param] ");
                        print_ast_expr_branch(ex);
                        ex = ex->next;
                }
                break;
        case ANDEXPR:
                printf("- [AND]\n");
                printf("%*s", indent * indent_size, "");
                printf("  - [lhs] ");
                print_ast_expr_branch(e->andexpr.lhs);
                printf("%*s", indent * indent_size, "");
                printf("  - [rhs] ");
                print_ast_expr_branch(e->andexpr.rhs);
                break;
        case OREXPR:
                printf("- [OR]\n");
                printf("%*s", indent * indent_size, "");
                printf("  - [lhs] ");
                print_ast_expr_branch(e->orexpr.lhs);
                printf("%*s", indent * indent_size, "");
                printf("  - [rhs] ");
                print_ast_expr_branch(e->orexpr.rhs);
                break;
        default:
                report("print_ast_expr_branch not yet implemeted for %s\n",
                       EXPR_REPR[e->type]);
                break;
        }

        --indent;
}

static void
print_ast_branch(Stmt *s)
{
        switch (s->type) {
        case EXPRSTMT:
                print_ast_expr_branch(s->expr.body);
                break;
        case VARDECLSTMT:
                printf("var %s = ", s->vardecl.name->str_literal);
                print_ast_expr_branch(s->vardecl.value);
                break;
        case ASSERTSTMT:
                printf("assert ");
                print_ast_expr_branch(s->assert.body);
                break;
        case IFSTMT:
                printf("if ");
                print_ast_expr_branch(s->ifstmt.cond);
                printf("if true: ");
                print_ast_branch(s->ifstmt.body);
                if (s->ifstmt.elsebody) {
                        printf("if false: ");
                        print_ast_branch(s->ifstmt.elsebody);
                }
                break;
        case BLOCKSTMT:
                printf("block ");
                s = s->block.body;
                while (s) {
                        print_ast_branch(s);
                        s = s->next;
                }
                printf("\n");
                break;
        case RETSTMT:
                printf("return: ");
                print_ast_expr_branch(s->retstmt.value);
                break;
        case WHILESTMT:
                printf("while ");
                print_ast_expr_branch(s->ifstmt.cond);
                printf("body: ");
                print_ast_branch(s->ifstmt.body);
                break;
        case FUNDECLSTMT:
                printf("Function %s (", s->funcdecl.name->str_literal);
                vtok *ex = s->funcdecl.params;
                if (ex) {
                        printf("%s", ex->str_literal);
                        ex = ex->next;
                }
                while (ex) {
                        printf(", %s", ex->str_literal);
                        ex = ex->next;
                }
                printf(")\n");
                print_ast_branch(s->funcdecl.body);
                break;
        default:
                report("No yet implemented: print_ast_branch for %s\n",
                       STMT_REPR[s->type]);
                break;
        }
}

void
print_ast()
{
        printf("-----------|AST|-----------\n");
        Stmt *e = head_stmt;
        while (e) {
                print_ast_branch(e);
                e = e->next;
        }
        printf("---------------------------\n");
}

static void
free_exprs(Expr *e)
{
        Expr *current = e;
        while (current) {
                Expr *next = current->next;
                switch (current->type) {
                case ASSIGNEXPR:
                        free_exprs(current->assignexpr.value);
                        break;
                case BINEXPR:
                        free_exprs(current->binexpr.lhs);
                        free_exprs(current->binexpr.rhs);
                        break;
                case UNEXPR:
                        free_exprs(current->unexpr.rhs);
                        break;
                case CALLEXPR:
                        free_exprs(current->callexpr.name);
                        free_exprs(current->callexpr.args);
                        break;
                case LITEXPR:
                        break;
                case VAREXPR:
                        free_exprs(current->varexpr.value);
                        break;
                case OREXPR:
                        free_exprs(current->orexpr.lhs);
                        free_exprs(current->orexpr.rhs);
                        break;
                case ANDEXPR:
                        free_exprs(current->andexpr.lhs);
                        free_exprs(current->andexpr.rhs);
                        break;
                        break;
                }
                free(current);
                current = next;
        }
}

static void
free_stmts(Stmt *s)
{
        Stmt *current = s;
        while (current) {
                Stmt *next = current->next;
                switch (current->type) {
                case VARDECLSTMT:
                        free_exprs(current->vardecl.value);
                        break;
                case BLOCKSTMT:
                        free_stmts(current->block.body);
                        break;
                case EXPRSTMT:
                        free_exprs(current->expr.body);
                        break;
                case ASSERTSTMT:
                        free_exprs(current->assert.body);
                        break;
                case RETSTMT:
                        free_exprs(current->retstmt.value);
                        break;
                case IFSTMT:
                        free_exprs(current->ifstmt.cond);
                        free_stmts(current->ifstmt.body);
                        free_stmts(current->ifstmt.elsebody);
                        break;
                case WHILESTMT:
                        free_exprs(current->whilestmt.cond);
                        free_stmts(current->whilestmt.body);
                        break;
                case FUNDECLSTMT:
                        free_stmts(current->funcdecl.body);
                        break;
                default:
                        report("free_stmts not yet implemeted for %s\n",
                               STMT_REPR[current->type]);
                        panik_exit();
                }
                free(current);
                current = next;
        }
}

void
free_stmt_head()
{
        free_stmts(head_stmt);
}

/* Use only to create a expr of a concrete type */
static Expr *
new_expr()
{
        Expr *e = malloc(sizeof(Expr));
        memset(e, 0, sizeof(Expr));
        return e;
}

static Expr *
new_orexpr(Expr *lhs, Expr *rhs)
{
        Expr *e = new_expr();
        e->orexpr.lhs = lhs;
        e->orexpr.rhs = rhs;
        e->type = OREXPR;
        return e;
}

static Expr *
new_andexpr(Expr *lhs, Expr *rhs)
{
        Expr *e = new_expr();
        e->andexpr.lhs = lhs;
        e->andexpr.rhs = rhs;
        e->type = ANDEXPR;
        return e;
}

static Expr *
new_binexpr(Expr *lhs, vtok *op, Expr *rhs)
{
        Expr *e = new_expr();
        e->binexpr.lhs = lhs;
        e->binexpr.op = op;
        e->binexpr.rhs = rhs;
        e->type = BINEXPR;
        return e;
}

vtok *
tokdup(vtok *tok)
{
        vtok *t = malloc(sizeof *tok);
        memcpy(t, tok, sizeof *tok);
        t->next = NULL;
        return t;
}

static void
append_arg_tok(vtok **arg, vtok *new)
{
        if (*arg == NULL) {
                *arg = tokdup(new);
                return;
        }
        vtok *e = *arg;
        while (e->next)
                e = e->next;
        e->next = tokdup(new);
}

static void
append_arg_expr(Expr **arg, Expr *new)
{
        if (*arg == NULL)
                *arg = new;
        else {
                __auto_type e = *arg;
                while (e->next)
                        e = e->next;
                e->next = new;
        }
}

static Expr *
new_call(Expr *name, Expr *arg, int argc)
{
        Expr *e = new_expr();
        e->callexpr.name = name;
        e->callexpr.args = arg;
        e->callexpr.count = argc;
        e->type = CALLEXPR;
        return e;
}

static Expr *
new_unexpr(vtok *op, Expr *rhs)
{
        Expr *e = new_expr();
        e->unexpr.rhs = rhs;
        e->unexpr.op = op;
        e->type = UNEXPR;
        return e;
}

static Expr *
new_assignexpr(vtok *name, Expr *value)
{
        Expr *e = new_expr();
        e->assignexpr.name = name;
        e->assignexpr.value = value;
        e->type = ASSIGNEXPR;
        return e;
}

static Expr *
new_litexpr(vtok *value)
{
        Expr *e = new_expr();
        e->litexpr.value = value;
        e->type = LITEXPR;
        return e;
}

static vtok *
get_token()
{
        return current_token;
}

static vtok *
consume_token()
{
        if (current_token) current_token = current_token->next;
        return current_token;
}

static vtok *
match(vtoktype token)
{
        vtok *tok = get_token();
        if (tok->token == token) {
                consume_token();
                return tok;
        }
        return NULL;
}

static void
report_expected_token(const char *expected, const char *current, vtok *pos)
{
        report("Expected %s but got %s ", expected, current);
        if (pos)
                report("at line %d, offset %lu", pos->line, pos->offset);
        printf("\n");
        return;
}

static vtok *
get_expect_consume(vtoktype expected)
{
        vtok *tok = get_token();
        if (tok->token != expected) {
                report_expected_token(TOKEN_REPR[expected],
                                      TOKEN_REPR[tok->token], tok);
                panik_exit();
        }
        consume_token();
        return tok;
}

static void
expect(vtoktype expected)
{
        vtok *tok = get_token();
        if (tok->token != expected) {
                report_expected_token(TOKEN_REPR[expected],
                                      TOKEN_REPR[tok->token], tok);
                panik_exit();
        }
}

static void
expect_consume(vtoktype expected)
{
        /* I add EOF here so I can evaluate a single expression witout
         * provide the semicolon.
         * This is not a feature, but a humman-bug fix */
        if (expected == SEMICOLON && get_token()->token == END_OF_FILE) return;
        expect(expected);
        consume_token();
}

/* Convert the literal string into a valid string. Eg: expand '\' */
static void
normalize(vtok *t)
{
        char *c = t->str_literal;
        static const char escape_lookup[] = {
                ['a'] = '\a',
                ['b'] = '\b',
                ['t'] = '\t',
                ['n'] = '\n',
                ['v'] = '\v',
                ['f'] = '\f',
                ['r'] = '\r',
        };

        while (*c && (c = strchr(c, '\\'))) {
                switch (c[1]) {
                case 'a':
                case 'b':
                case 't':
                case 'n':
                case 'v':
                case 'f':
                case 'r':
                        *c = escape_lookup[c[1]];
                        break;
                default:
                        *c = c[1];
                        break;
                }
                if (c[1]) strcpy(c + 1, c + 2);
                c++;
        }
}

static void
link_stmt(Stmt *s)
{
        if (head_stmt == NULL) {
                head_stmt = s;
                return;
        }
        Stmt *last = head_stmt;
        while (last->next)
                last = last->next;
        last->next = s;
}

static vtok *
is_literal()
{
        vtok *t;
        if ((t = match(NUMBER)) ||
            (t = match(STRING)) ||
            (t = match(IDENTIFIER)) ||
            (t = match(TRUE)) ||
            (t = match(FALSE)))
                return t;
        return NULL;
}

static Expr *
get_literal()
{
        vtok *t;
        if ((t = is_literal())) {
                if (t->token == STRING) normalize(t);
                return new_litexpr(t);
        }
        /* can't be used expect() because LITERAL is an expression, not a token */
        report_expected_token("LITERAL", TOKEN_REPR[get_token()->token], t);
        panik_exit();
        return NULL;
}

Expr *get_expression();

static Expr *
get_group()
{
        Expr *e;
        if (match(LEFT_PARENT)) {
                e = get_expression();
                expect_consume(RIGHT_PARENT);
                return e;
        }
        return get_literal();
}

#define MAX_ARGC 3
static Expr *
get_call()
{
        Expr *e = get_group();
        if (match(LEFT_PARENT)) {
                int argc = 0;
                Expr *arg = NULL;
                while (!match(RIGHT_PARENT)) {
                        if (argc > 0) expect_consume(COMMA);
                        append_arg_expr(&arg, get_expression());
                        ++argc;
                        // if (argc > MAX_ARGC) {
                        //         report("Too much arguments! "
                        //                "Implementation only support %d\n",
                        //                MAX_ARGC);
                        //         panik_exit();
                        // }
                }
                return new_call(e, arg, argc);
        }
        return e;
}

static Expr *
get_unary()
{
        vtok *op;
        if ((op = match(MINUS)) || (op = match(BANG))) {
                return new_unexpr(op, get_unary());
        } else
                return get_call();
}

static Expr *
get_factor()
{
        Expr *e = get_unary();
        vtok *op;
        while ((op = match(SLASH)) || (op = match(STAR))) {
                e = new_binexpr(e, op, get_unary());
        }
        return e;
}

static Expr *
get_term()
{
        Expr *e = get_factor();
        vtok *op;
        while ((op = match(MINUS)) || (op = match(PLUS))) {
                e = new_binexpr(e, op, get_factor());
        }
        return e;
}

static Expr *
get_comparison()
{
        Expr *e = get_term();
        vtok *op;
        while ((op = match(GREATER)) ||
               (op = match(GREATER_EQUAL)) ||
               (op = match(LESS)) ||
               (op = match(LESS_EQUAL))) {
                e = new_binexpr(e, op, get_term());
        }
        return e;
}

static Expr *
get_equality()
{
        Expr *e = get_comparison();
        vtok *op;
        while ((op = match(EQUAL_EQUAL)) || (op = match(BANG_EQUAL))) {
                e = new_binexpr(e, op, get_comparison());
        }
        return e;
}

static Expr *
get_or()
{
        Expr *e = get_equality();
        if (match(OR)) {
                e = new_orexpr(e, get_or());
        }
        return e;
}

static Expr *
get_and()
{
        Expr *e = get_or();
        if (match(AND)) {
                e = new_andexpr(e, get_and());
        }
        return e;
}

static Expr *
get_assignment()
{
        vtok *id;
        if ((id = match(IDENTIFIER))) {
                if (match(EQUAL))
                        return new_assignexpr(id, get_assignment());
                current_token = id;
        }
        return get_and();
}

Expr *
get_expression()
{
        return get_assignment();
}

static Stmt *
new_stmt()
{
        return calloc(1, sizeof(Stmt));
}

static Stmt *
new_blockstmt()
{
        Stmt *s = new_stmt();
        s->type = BLOCKSTMT;
        s->block.body = NULL;
        return s;
}

static Stmt *
new_exprstmt(Expr *e)
{
        Stmt *s = new_stmt();
        s->type = EXPRSTMT;
        s->expr.body = e;
        return s;
}

static Stmt *
new_whilestmt(Expr *e, Stmt *body)
{
        Stmt *s = new_stmt();
        s->type = WHILESTMT;
        s->whilestmt.cond = e;
        s->whilestmt.body = body;
        return s;
}

static Stmt *
new_return(Expr *e)
{
        Stmt *s = new_stmt();
        s->type = RETSTMT;
        s->retstmt.value = e;
        return s;
}

static Stmt *
new_ifstmt(Expr *e, Stmt *body, Stmt *elsebody)
{
        Stmt *s = new_stmt();
        s->type = IFSTMT;
        s->ifstmt.cond = e;
        s->ifstmt.body = body;
        s->ifstmt.elsebody = elsebody;
        return s;
}

static Stmt *
new_funcdecl(vtok *name, vtok *params, int arity, Stmt *body)
{
        Stmt *s = new_stmt();
        s->type = FUNDECLSTMT;
        s->funcdecl.name = name;
        s->funcdecl.params = params;
        s->funcdecl.arity = arity;
        s->funcdecl.body = body;
        return s;
}

static Stmt *
new_assertstmt(Expr *e)
{
        Stmt *s = new_stmt();
        s->type = ASSERTSTMT;
        s->assert.body = e;
        return s;
}

static Stmt *
new_vardecl(vtok *id, Expr *value)
{
        Stmt *s = new_stmt();
        s->type = VARDECLSTMT;
        s->vardecl.name = id;
        s->vardecl.value = value;
        return s;
}

static void
blockstmt_addstmt(Stmt *block, Stmt *s)
{
        if (block->block.body == NULL) {
                block->block.body = s;
                return;
        }
        Stmt *current = block->block.body;
        while (current->next)
                current = current->next;
        current->next = s;
}

static Stmt *get_declaration();

static Stmt *
get_program()
{
        Stmt *c = NULL;
        Stmt *ret;
        Stmt *s;
        while (get_token()->token != END_OF_FILE) {
                s = get_declaration();
                if (c) {
                        c->next = s;
                } else
                        ret = s;
                c = s;
        }
        return ret;
}

static Stmt *get_vardecl();
static Stmt *get_funcdecl();
static Stmt *get_stmt();

static Stmt *
get_declaration()
{
        if (match(VAR)) return get_vardecl();
        if (match(FUNCTION)) return get_funcdecl();
        return get_stmt();
}

static Expr *
littok_novalue()
{
        Expr *e = new_expr();
        e->litexpr.value = calloc(1, sizeof(vtok));
        e->litexpr.value->str_literal = "no-value";
        e->litexpr.value->token = STRING;
        e->type = LITEXPR;
        return e;
}

static Stmt *
get_vardecl()
{
        Expr *value = NULL;
        vtok *id = get_expect_consume(IDENTIFIER);
        if (match(EQUAL)) {
                value = get_expression();
        } else
                value = littok_novalue();
        expect_consume(SEMICOLON);
        return new_vardecl(id, value);
}

static Stmt *get_block();

static Stmt *
get_funcdecl()
{
        // func a(a, b) {
        // }
        vtok *param = NULL;
        int paramc = 0;
        vtok *id = get_expect_consume(IDENTIFIER);
        expect_consume(LEFT_PARENT);
        while (!match(RIGHT_PARENT)) {
                if (paramc > 0) expect_consume(COMMA);
                append_arg_tok(&param, get_token());
                consume_token();
                ++paramc;
        }
        expect_consume(LEFT_BRACE);
        return new_funcdecl(id, param, paramc, get_block());
}

static Stmt *get_block();
static Stmt *get_exprstmt();
static Stmt *get_assert();
static Stmt *get_ifstmt();
static Stmt *get_whilestmt();
static Stmt *get_return();

static Stmt *
get_stmt()
{
        if (match(ASSERT)) return get_assert();
        if (match(LEFT_BRACE)) return get_block();
        if (match(IF)) return get_ifstmt();
        if (match(WHILE)) return get_whilestmt();
        if (match(RETURN)) return get_return();
        return get_exprstmt();
}

static Stmt *
get_whilestmt()
{
        expect_consume(LEFT_PARENT);
        Expr *e = get_expression();
        expect_consume(RIGHT_PARENT);
        return new_whilestmt(e, get_declaration());
}

static Stmt *
get_return()
{
        Stmt *s = new_return(get_expression());
        expect_consume(SEMICOLON);
        return s;
}

static Stmt *
get_ifstmt()
{
        expect_consume(LEFT_PARENT);
        Expr *e = get_expression();
        expect_consume(RIGHT_PARENT);
        Stmt *body = get_declaration();
        Stmt *elsebody = NULL;
        if (match(ELSE)) {
                elsebody = get_declaration();
        }
        return new_ifstmt(e, body, elsebody);
}

static Stmt *
get_exprstmt()
{
        Stmt *s = new_exprstmt(get_expression());
        expect_consume(SEMICOLON);
        return s;
}

static Stmt *
get_assert()
{
        Stmt *s = new_assertstmt(get_expression());
        expect_consume(SEMICOLON);
        return s;
}

static Stmt *
get_block()
{
        Stmt *s = new_blockstmt();
        while (!match(RIGHT_BRACE))
                blockstmt_addstmt(s, get_declaration());
        return s;
}

void
tok_parse()
{
        if (head_token == NULL) {
                report("tok_parse: invalid token list. Call lex_analize() first.\n");
                exit(1);
        }

        head_stmt = NULL;
        current_token = head_token;

        /* Set point to reset after failure */
        if (setjmp(panik_jmp)) {
                /* This is executed after failure. Go down to the
                 * next semicolon, as current expression failed. After
                 * the semicolon it should continue without problems. */
                vtok *tok;
                for (;;) {
                        tok = get_token();
                        if (tok->token == END_OF_FILE) return;
                        consume_token();
                        if (tok->token == SEMICOLON) break;
                }
        }
        link_stmt(get_program());
}
