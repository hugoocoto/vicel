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
        static int loaded = 0;
        if (loaded) return;
        loaded = 1;
        CoreFunc *c = core_func_list;
        while (c) {
                load(c);
                c = c->next;
        }
}

