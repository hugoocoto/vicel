#ifndef VINTERPRETER
#define VINTERPRETER

#include "tokens.h"
#include <setjmp.h>

#define VAARGS (1 << ((sizeof(int) * 8) - 1))

#define NO_VALUE ((Value) { .type = TYPE_NONE })

struct ValueNode;

typedef enum Valtype {
        TYPE_NUM,
        TYPE_STR,
        TYPE_ADDR,
        TYPE_NONE,
        TYPE_CALLABLE,
        TYPE_CORE_CALL,
} Valtype;

static const char *VALTYPE_REPR[] = {
        [TYPE_NUM] = "NUMBER",
        [TYPE_STR] = "STRING",
        [TYPE_ADDR] = "MEMORY ADDRESS",
        [TYPE_NONE] = "NONE",
        [TYPE_CALLABLE] = "CALLABLE",
        [TYPE_CORE_CALL] = "CORE CALL",
};

struct Env;

typedef struct Value {
        union {
                int num;
                char *str;
                void *addr; // reserve for core functions
                struct {
                        int arity;
                        vtok *params;
                        char *name;
                        union {
                                Stmt *body;
                                struct Value (*ifunc)(Expr *);
                        };
                        struct Env *closure;
                } call;
        };
        Valtype type;
} Value;

typedef struct ValueNode {
        Value v;
        struct ValueNode *next;
} ValueNode;

typedef struct node {
        char *key;
        Value value;
} node;

typedef struct Env {
        node *map;
        char *name;
        struct Env *upper;
} Env;

/* Get the result of eval a single expression */
Value vspl_eval_expr(Expr *e);

/* Eval all expressions from parsing and print result to stdout */
void eval();
void print_val(Value v);

int resolve();


#endif
