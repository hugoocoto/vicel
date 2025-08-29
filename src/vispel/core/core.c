/* VISPEL interpreter - Core lib
 *
 * Author: Hugo Coto Florez
 * Repo: github.com/hugocotoflorez/vispel
 *
 * */
#include <stdlib.h>
#include <string.h>
extern char *strdup(const char *);

#include "core.h"

CoreFunc *core_func_list = NULL;

static void
load(CoreFunc *c)
{
        Value v;
        v.type = TYPE_CORE_CALL;
        v.call.arity = c->arity; // number of params
        v.call.ifunc = c->func;  // C function
        v.call.name = c->name;   // vispel function name
        v.call.params = NULL;
        v.call.closure = get_current_env();
        env_add(v.call.name, v);
}

static CoreFunc *
new_corefunc()
{
        return calloc(1, sizeof(CoreFunc));
}

void
preload(const char *name, Value (*func)(Expr *), int arity)
{
        CoreFunc *c = new_corefunc();
        c->name = strdup(name);
        c->func = func;
        c->arity = arity;
        /* insert at 0 */
        c->next = core_func_list;
        core_func_list = c;
}

void
load_core_lib()
{
        CoreFunc *c = core_func_list;
        while (c) {
                load(c);
                c = c->next;
        }
}
