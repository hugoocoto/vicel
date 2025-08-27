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

#ifndef FORMULA_H_
#define FORMULA_H_

#include "cellmap.h"
#include "da.h"

typedef enum ExprType {
        EXPR_LITERAL = 0,
        EXPR_IDENTIFIER,
        EXPR_BIN,
        EXPR_UN,
        EXPR_FUNC,
        EXPRLEN,
} ExprType;

/* clang-format off */
typedef struct Expr {
        ExprType type;
        union {
                struct { Value value; } literal;
                struct { struct Expr *lhs; char *op; struct Expr *rhs; } binop;
                struct { char *op; struct Expr *rhs; } unop;
                struct { Cell *cell; char* name; } identifier;
                struct { struct Expr* name; struct Expr* args; } func;
        } as;
        struct Expr * next; 
} Expr;
/* clang-format on */

typedef struct Token {
        union {
                char *str;
                char *id;
                double num;
        } as;
        enum {
                TOK_STRING,
                TOK_IDENTIFIER,
                TOK_NUMERIC,
        } type;
        struct Token *next;
} Token;


typedef struct Formula {
        Expr *body;
        Value value;
        Token *tokens;
        struct {
                int capacity;
                int size;
                Cell **data;
        } subscribed;
} Formula;

/* write formula stuff in SELF */
void build_formula(char *, Cell *self);
Formula *formula_dup(Formula *f);

void clear_cell(Cell *c);
Expr *parse_formula(char *, Cell *self);
void destroy_formula(Cell *c);

Formula *formula_extend(Cell *self, Formula *f, int r, int c);
void get_ast_repr(Expr *e, char *buffer, size_t leng); 
char * create_id(int r, int c, bool freeze_r, bool freeze_c);

#endif //! FORMULA_H_
