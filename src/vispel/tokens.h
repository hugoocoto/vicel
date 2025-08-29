#ifndef VTOKENS_H
#define VTOKENS_H

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

extern char *strdup(const char *);

void report(char *format, ...);

extern const char *TOKEN_REPR[];

typedef enum {
        LEFT_PARENT,
        RIGHT_PARENT,
        LEFT_BRACE,
        RIGHT_BRACE,
        LEFT_BRACKET,
        RIGHT_BRACKET,
        COMMA,
        DOT,
        MINUS,
        PLUS,
        SEMICOLON,
        SLASH,
        STAR,
        BANG,
        BANG_EQUAL,
        EQUAL,
        EQUAL_EQUAL,
        GREATER,
        GREATER_EQUAL,
        LESS,
        LESS_EQUAL,
        IDENTIFIER,
        STRING,
        NUMBER,
        AND,
        CLASS,
        ELSE,
        FALSE,
        FUNCTION,
        VAR,
        FOR,
        IF,
        NIL,
        OR,
        EXTERN,
        RETURN,
        TRUE,
        WHILE,
        END_OF_FILE,
        BITWISE_AND,
        BITWISE_OR,
        BITWISE_XOR,
        BITWISE_NOT,
        PLUS_PLUS,
        LESS_LESS,
        SHIFT_LEFT,
        SHIFT_RIGHT,
        FUNC_INPUT,
        FUNC_OUTPUT,
        ASSERT,
        UNKNOWN,
} vtoktype;


typedef struct vtok {
        const char *lexeme;
        vtoktype token;
        union {
                int num_literal;
                char *str_literal;
        };
        int line;
        intptr_t offset;
        /* Linked list stuff */
        struct vtok *next;
} vtok;

typedef enum Exprtype {
        ASSIGNEXPR,
        BINEXPR,
        UNEXPR,
        CALLEXPR,
        LITEXPR,
        VAREXPR,
        ANDEXPR,
        OREXPR,
} Exprtype;


// clang-format off
typedef struct Expr {
        union {
                struct { struct Expr *value; vtok *name; } assignexpr;
                struct { struct Expr *rhs; struct Expr *lhs; vtok *op; } binexpr;
                struct { struct Expr *rhs; struct Expr *lhs; } andexpr;
                struct { struct Expr *rhs; struct Expr *lhs; } orexpr;
                struct { struct Expr *rhs; vtok *op; } unexpr;
                struct { struct Expr *name; int count; struct Expr *args; } callexpr;
                struct { struct Expr *value; vtok *name; } varexpr;
                struct { vtok *value; } litexpr;
        };
        Exprtype type;
        /* Linked list stuff */
        struct Expr *next;
} Expr;
// clang-format on

typedef enum {
        VARDECLSTMT,
        BLOCKSTMT,
        EXPRSTMT,
        ASSERTSTMT,
        IFSTMT,
        WHILESTMT,
        FUNDECLSTMT,
        RETSTMT,
} Stmttype;

// clang-format off
typedef struct Stmt {
        union {
                struct { vtok *name; Expr *value; } vardecl;
                struct { struct Stmt *body; } block;
                struct { Expr *body; } expr;
                struct { Expr *cond; struct Stmt *body; struct Stmt *elsebody; } ifstmt;
                struct { Expr *cond; struct Stmt *body; } whilestmt;
                struct { Expr *body; } assert;
                struct { Expr *value; } retstmt;
                struct { vtok *name; vtok *params; int arity; struct Stmt *body; } funcdecl;
        };
        Stmttype type;
        struct Stmt *next;
} Stmt;
// clang-format on

extern const char *STMT_REPR[];
extern const char *EXPR_REPR[]; 

extern vtok *head_token;
extern Stmt *head_stmt;

/* ./lexer.c */
void lex_analize(char *source);
void print_tokens();
void vspl_free_tokens();
void print_literal(vtok *tok);

/* ./parser.c */
void tok_parse();
void print_ast();
void free_stmt_head();

#endif
