#include "env.h"
#include "interpreter.h"
#include "tokens.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

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
        if (lex_analize(buf)) return false;
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
