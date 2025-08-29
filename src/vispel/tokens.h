#ifndef VTOKENS_H
#define VTOKENS_H

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

extern char *strdup(const char *);

static void
report(char *format, ...)
{
        va_list args;
        va_start(args, format);
        if (isatty(STDERR_FILENO)) fprintf(stderr, "\033[31m");
        vfprintf(stderr, format, args);
        if (isatty(STDERR_FILENO)) fprintf(stderr, "\033[0m");
        va_end(args);
        FILE *f = fopen("log.txt", "a");
        if (f) {
                va_start(args, format);
                vfprintf(f, format, args);
                va_end(args);
                fclose(f);
        }
}

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

static const char *TOKEN_REPR[] = {
        [LEFT_PARENT] = "LEFT_PARENT",
        [RIGHT_PARENT] = "RIGHT_PARENT",
        [LEFT_BRACE] = "LEFT_BRACE",
        [RIGHT_BRACE] = "RIGHT_BRACE",
        [LEFT_BRACKET] = "LEFT_BRACKET",
        [RIGHT_BRACKET] = "RIGHT_BRACKET",
        [COMMA] = "COMMA",
        [DOT] = "DOT",
        [MINUS] = "MINUS",
        [PLUS] = "PLUS",
        [SEMICOLON] = "SEMICOLON",
        [SLASH] = "SLASH",
        [STAR] = "STAR",
        [BANG] = "BANG",
        [BANG_EQUAL] = "BANG_EQUAL",
        [EQUAL] = "EQUAL",
        [EQUAL_EQUAL] = "EQUAL_EQUAL",
        [GREATER] = "GREATER",
        [GREATER_EQUAL] = "GREATER_EQUAL",
        [LESS] = "LESS",
        [LESS_EQUAL] = "LESS_EQUAL",
        [IDENTIFIER] = "IDENTIFIER",
        [STRING] = "STRING",
        [NUMBER] = "NUMBER",
        [AND] = "AND",
        [CLASS] = "CLASS",
        [ELSE] = "ELSE",
        [FALSE] = "FALSE",
        [FUNCTION] = "FUNCTION",
        [VAR] = "VAR",
        [FOR] = "FOR",
        [IF] = "IF",
        [NIL] = "NIL",
        [OR] = "OR",
        [EXTERN] = "EXTERN",
        [RETURN] = "RETURN",
        [TRUE] = "TRUE",
        [WHILE] = "WHILE",
        [END_OF_FILE] = "END_OF_FILE",
        [BITWISE_AND] = "BITWISE_AND",
        [BITWISE_OR] = "BITWISE_OR",
        [BITWISE_XOR] = "BITWISE_XOR",
        [BITWISE_NOT] = "BITWISE_NOT",
        [PLUS_PLUS] = "PLUS_PLUS",
        [LESS_LESS] = "LESS_LESS",
        [SHIFT_LEFT] = "SHIFT_LEFT",
        [SHIFT_RIGHT] = "SHIFT_RIGHT",
        [FUNC_INPUT] = "FUNC_INPUT",
        [FUNC_OUTPUT] = "FUNC_OUTPUT",
        [ASSERT] = "ASSERT",
        [UNKNOWN] = "UNKNOWN",
};

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

static const char *EXPR_REPR[] = {
        [ASSIGNEXPR] = "ASSIGNEXPR",
        [BINEXPR] = "BINEXPR",
        [UNEXPR] = "UNEXPR",
        [CALLEXPR] = "CALLEXPR",
        [LITEXPR] = "LITEXPR",
        [VAREXPR] = "VAREXPR",
        [ANDEXPR] = "ANDEXPR",
        [OREXPR] = "OREXPR",
};

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

static const char *STMT_REPR[] = {
        [VARDECLSTMT] = "VARDECLSTMT",
        [BLOCKSTMT] = "BLOCKSTMT",
        [EXPRSTMT] = "EXPRSTMT",
        [ASSERTSTMT] = "ASSERTSTMT",
        [IFSTMT] = "IFSTMT",
        [WHILESTMT] = "WHILESTMT",
        [FUNDECLSTMT] = "FUNDECLSTMT",
        [RETSTMT] = "RETSTMT",
};

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
