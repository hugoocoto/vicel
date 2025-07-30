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
        struct {
                int capacity;
                int size;
                Cell **data;
        } subscribed;
} Formula;

/* write formula stuff in SELF */
void build_formula(char *, Cell *self);

Value eval_formula(Formula f);
Expr *parse_formula(char *, Cell *self);
void destroy_formula(Formula f);

void free_formula_subscribers(Cell *c);


#endif //! FORMULA_H_
