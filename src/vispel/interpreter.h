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
#ifndef VINTERPRETER
#define VINTERPRETER

#include "tokens.h"
#include <setjmp.h>

#define VAARGS (1 << ((sizeof(int) * 8) - 1))

#define NO_VALUE ((Value) { .type = TYPE_NONE })

struct ValueNode;

typedef enum Valtype {
        TYPE_NONE,
        TYPE_NUM,
        TYPE_STR,
        TYPE_ADDR,
        TYPE_CALLABLE,
        TYPE_CORE_CALL,
} Valtype;

extern const char *VALTYPE_REPR[];

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
void eval_quiet();

typedef struct {
        char none;
        char nostrdelim;
} PrintOpts;

void __print_val(Value v, PrintOpts opts);
#define print_val(v, ...) __print_val((v), (PrintOpts) { .none = 0, __VA_ARGS__ });

void __print_valnl(Value v, PrintOpts opts);
#define print_valnl(v, ...) __print_valnl((v), (PrintOpts) { .none = 0, __VA_ARGS__ });

int resolve();


#endif
