#ifndef CORE_LIB_H
#define CORE_LIB_H

#include "../env.h"
#include "../interpreter.h"
#include "../tokens.h"

typedef struct CoreFunc {
        char *name;
        Value (*func)(Expr *);
        int arity;
        struct CoreFunc *next;
} CoreFunc;

extern CoreFunc *core_func_list;

void preload(const char *name, Value (*func)(Expr *), int arity);
void load_core_lib();

#endif // !CORE_LIB_H
