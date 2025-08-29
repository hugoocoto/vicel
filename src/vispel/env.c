#include <assert.h>
#include <setjmp.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

#include "env.h"
#include "interpreter.h"
#include "tokens.h"

Env *lower_env = NULL;

static char *
gen_env_random_name()
{
        static int counter = 0;
        char name[12];
        name[snprintf(name, 11, "Env %d", counter)] = 0;
        counter++;
        return strdup(name);
}

// static void
// print_env_list()
// {
//         printf("\e[90m");
//         Env *e = lower_env;
//         while (e) {
//                 if (e != lower_env) printf(", ");
//                 printf("%s", e->name);
//                 e = e->upper;
//         }
//         printf("\e[0m");
//         printf("\n");
// }

void
env_dump()
{
        Env *e = lower_env;
        while (e) {
                int len = shlenu(e->map);
                for (int i = 0; i < len; i++) {
                        printf("%s = ", e->map[i].key);
                        print_val(e->map[i].value);
                        puts(";");
                }
                e = e->upper;
        }
        printf("\n");
}

struct Env *
get_current_env()
{
        return lower_env;
}

Env *
env_change_upper(Env *newupper)
{
        if (!lower_env) {
                report("no env created!\n");
                longjmp(eval_runtime_error, 1);
        }

        Env *ret = lower_env->upper;
        lower_env->upper = newupper;
        return ret;
}

Value
env_add_e(struct Env *e, char *name, Value value)
{
        if (!e) {
                report("no env created!\n");
                longjmp(eval_runtime_error, 1);
        }
        if (shgeti(e->map, name) >= 0) {
                report("Var `%s` already declared\n", name);
                longjmp(eval_runtime_error, 1);
        }

        return shput(e->map, name, value);
}

node *
env_get_node(char *name)
{
        Env *e = lower_env;
        node *ret;
        while (e && (ret = shgetp_null(e->map, name)) == NULL) {
                e = e->upper;
        }
        return ret;
}

Value
env_get_e(struct Env *env, char *name)
{
        if (!env) {
                report("no env created!\n");
                longjmp(eval_runtime_error, 1);
        }

        Env *e = env;
        node *ret;
        while (e && (ret = shgetp_null(e->map, name)) == NULL) {
                e = e->upper;
        }
        if (ret == NULL) {
                report("env_get_e: var `%s` not declared\n", name);
                longjmp(eval_runtime_error, 1);
        }
        return ret->value;
}

int
env_get_offset(char *name)
{
        if (!lower_env) {
                report("no env created!\n");
                longjmp(resolve_error_jmp, 1);
        }

        int offset = 0;
        Env *e = lower_env;
        node *ret;
        while (e && (ret = shgetp_null(e->map, name)) == NULL) {
                e = e->upper;
                ++offset;
        }
        if (ret == NULL) {
                report("env_get_offset: Var `%s` not declared\n", name);
                longjmp(resolve_error_jmp, 1);
                return -1;
        }
        return offset;
}

static Env *
env_get_by_offset(int offset)
{
        Env *e = lower_env;
        int o = offset;
        while (o > 0 && e) {
                e = e->upper;
                --o;
        }
        if (!e) {
                report("Offset greater than possible jumps\n");
                longjmp(eval_runtime_error, 1);
        }
        return e;
}

Value
env_set_e(struct Env *env, char *name, Value value)
{
        if (!env) {
                report("no env created!\n");
                longjmp(eval_runtime_error, 1);
        }

        Env *e = env;
        node *ret;
        while (e && (ret = shgetp_null(e->map, name)) == NULL) {
                e = e->upper;
        }
        if (ret == NULL) {
                report("Var %s not declared\n", name);
                longjmp(eval_runtime_error, 1);
        }
        if (ret->value.type == TYPE_STR) free(ret->value.str);
        return ret->value = value;
}

Env *
new_env()
{
        Env *e = calloc(1, sizeof(Env));
        e->name = gen_env_random_name();
        return e;
}

/* Create a new env and link with UPPER. Old current env is returned */
Env *
env_create_e(Env *upper)
{
        Env *ret = lower_env;
        Env *e = new_env();
        e->upper = upper;
        lower_env = e;
        return ret;
}

/* Destroy current env and set current env to CURRENT */
void
env_destroy_e(Env *current)
{
        Env *e = lower_env;
        if (!e) {
                report("Destroying a non existing env!\n");
                longjmp(eval_runtime_error, 1);
        }
        lower_env = current;
        int len = shlenu(e->map);
        for (int i = 0; i < len; i++) {
                if (e->map[i].value.type == TYPE_STR) {
                        free(e->map[i].value.str);
                }
        }
        free(e->name);
        shfree(e->map);
        free(e);
}

void
env_create()
{
        Env *e = new_env();
        if (lower_env != NULL) {
                e->upper = lower_env;
        }
        lower_env = e;
}

void
env_destroy()
{
        Env *e = lower_env;
        if (!e) {
                report("Destroying a non existing env!\n");
                longjmp(eval_runtime_error, 1);
        }
        lower_env = lower_env->upper;
        int len = shlenu(e->map);
        for (int i = 0; i < len; i++) {
                if (e->map[i].value.type == TYPE_STR) {
                        free(e->map[i].value.str);
                }
        }
        free(e->name);
        shfree(e->map);
        free(e);
}

Value
env_add(char *name, Value value)
{
        return env_add_e(lower_env, name, value);
}

Value
env_get(char *name)
{
        return env_get_e(lower_env, name);
}

Value
env_set(char *name, Value value)
{
        return env_set_e(lower_env, name, value);
}

Value
env_add_o(int offset, char *name, Value value)
{
        return env_add_e(env_get_by_offset(offset), name, value);
}

Value
env_get_o(int offset, char *name)
{
        return env_get_e(env_get_by_offset(offset), name);
}

Value
env_set_o(int offset, char *name, Value value)
{
        return env_set_e(env_get_by_offset(offset), name, value);
}
