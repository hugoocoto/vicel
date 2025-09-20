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
#include "embedded.h"
#include "env.h"
#include "interpreter.h"
#include "tokens.h"
#include <stdio.h>

Value last_eval_val = { .type = TYPE_NONE };

char *
vspl_get_eval_val_repr()
{
        Value v = last_eval_val;
        char last_eval_val_repr[1024];
        switch (v.type) {
        case TYPE_NUM:
                sprintf(last_eval_val_repr, "%d", v.num);
                break;
        case TYPE_STR:
                sprintf(last_eval_val_repr, "\"%s\"", v.str);
                break;
        case TYPE_CORE_CALL:
                sprintf(last_eval_val_repr, "(core func %s)", v.call.name);
                break;
        case TYPE_NONE:
                *last_eval_val_repr = 0;
                break;
        default:
                report("No yet implemented: print_val for %s",
                       VALTYPE_REPR[v.type]);
                *last_eval_val_repr = 0;
                break;
        }
        return strdup(last_eval_val_repr);
}

void
vspl_start()
{
        env_create();
}

void
vspl_addint(char *name, int value)
{
        env_add(name, (Value) { .type = TYPE_NUM, .num = value });
}

void
vspl_addstr(char *name, char *value)
{
        if (value == NULL) {
                env_add(name, NO_VALUE);
                return;
        }
        report("Adding string: `%s`", value);
        env_add(name, (Value) { .type = TYPE_STR, .str = value });
}

bool
vspl_parse(FILE *file)
{
        if (file == NULL) return false;
        char buf[1024 * 1024];
        ssize_t n;
        /* Todo: allow split file in multiple reads */
        if ((n = fread(buf, 1, sizeof buf - 2, file)) <= 0) return false;
        buf[n] = 0;
        buf[n + 1] = EOF;
        return vspl_parse_str(buf);
}

bool
vspl_parse_str(char *str)
{
        if (lex_analize(str)) return false;
        tok_parse();
        if (resolve() == 0) eval_quiet();
        return true;
}

bool
vspl_get_int(char *name, int *value)
{
        node *n = env_get_node(name);
        if (n == NULL) return false;
        *value = n->value.num;
        return true;
}

bool
vspl_get_str(char *name, char **value)
{
        node *n = env_get_node(name);
        if (n == NULL) return false;
        *value = n->value.str;
        return true;
}

void
vspl_dump_env()
{
        env_dump();
}

void
vspl_end()
{
        env_destroy();
        vspl_free_tokens();
        free_stmt_head();
}
