#ifndef FORMULA_H_
#define FORMULA_H_

#include "cellmap.h"
#include "da.h"

typedef enum ExprType {
        EXPR_LITERAL = 0,
        EXPR_IDENTIFIER,
        EXPR_BIN,
        EXPR_UN,
        EXPRLEN,
} ExprType;

/* clang-format off */
typedef struct Expr {
        ExprType type;
        union {
                struct { Value value; } literal;
                struct { struct Expr *lhs; char *op; struct Expr *rhs; } binop;
                struct { char *op; struct Expr *rhs; } unop;
                struct { Cell *cell; } identifier;
        } as;
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
Formula * formula_dup(Formula *f);

Value eval_formula(Formula f);
void clear_cell(Cell *c);
Expr *parse_formula(char *, Cell *self);
void destroy_formula(Cell *c);

Formula * formula_extend(Cell*self, Formula *f, int r, int c);

#endif //! FORMULA_H_
