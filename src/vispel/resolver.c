#include <setjmp.h>

#include "env.h"
#include "interpreter.h"
#include "tokens.h"

#include "stb_ds.h"

jmp_buf resolve_error_jmp;

#define DECLARED ((Value) { .type = TYPE_NUM, .num = 1 })
#define DEFINED ((Value) { .type = TYPE_NUM, .num = 3 })
#define UNDEFINED ((Value) { .type = TYPE_NUM, .num = 0 })


struct {
        void *key;
        int value;
} *sidetable = NULL;

static void
resolve_error()
{
        longjmp(resolve_error_jmp, 1);
}

static void
sidetable_add_expr(Expr *e)
{
        switch (e->type) {
        case LITEXPR:
                if (e->litexpr.value->token != IDENTIFIER) {
                        report("Sidetable_add_expr case LITEXPR for literal not identifier: error\n");
                        resolve_error();
                }
                hmput(sidetable, e, env_get_offset(e->litexpr.value->str_literal));
                break;
        case ASSIGNEXPR:
                hmput(sidetable, e, env_get_offset(e->assignexpr.name->str_literal));
                break;
        default:
                report("No yet implemented: sidetable_add_expr for %s\n",
                       EXPR_REPR[e->type]);
                resolve_error();
        }
}

// static void
// sidetable_add_stmt(Stmt *s)
// {
//         switch (s->type) {
//         default:
//                 report("No yet implemented: sidetable_add_stmt for %s\n",
//                        STMT_REPR[s->type]);
//                 resolve_error();
//         }
// }

static int
sidetable_get(void *e)
{
        int i;
        i = hmgeti(sidetable, e);
        if (i < 0) {
                report("Can not resolve %p\n", e);
                resolve_error();
        }
        return sidetable[i].value;
}

static void
define(char *name)
{
        env_add(name, DEFINED);
}

static void
declare(char *name)
{
        env_add(name, DECLARED);
}

static void
check_declared(char *name)
{
        if (env_get(name).num & DECLARED.num) return;
        report("Var `%s` not declared\n", name);
        resolve_error();
}

// static void
// resolve_declaration(Stmt *s)
// {
//         switch (s->type) {
//         default:
//                 report("No yet implemented: resolve_declaration for %s\n",
//                        STMT_REPR[s->type]);
//                 resolve_error();
//                 break;
//         }
// }

static void resolve_expr_arr(Expr *e);

static void
resolve_expr(Expr *e)
{
        switch (e->type) {
        case LITEXPR:
                if (e->litexpr.value->token != IDENTIFIER) return;
                sidetable_add_expr(e);
                break;
        case CALLEXPR:
                resolve_expr_arr(e->callexpr.args);
                resolve_expr(e->callexpr.name);
                break;
        case ASSIGNEXPR:
                check_declared(e->assignexpr.name->str_literal);
                sidetable_add_expr(e);
                resolve_expr(e->assignexpr.value);
                break;
        case BINEXPR:
                resolve_expr(e->binexpr.lhs);
                resolve_expr(e->binexpr.rhs);
                break;
        case UNEXPR:
                resolve_expr(e->unexpr.rhs);
                break;
        case OREXPR:
                resolve_expr(e->orexpr.lhs);
                resolve_expr(e->orexpr.rhs);
                break;
        case ANDEXPR:
                resolve_expr(e->andexpr.lhs);
                resolve_expr(e->andexpr.rhs);
                break;
        default:
                report("No yet implemented: resolve_expr for %s\n",
                       EXPR_REPR[e->type]);
                resolve_error();
        }
}

static void
resolve_expr_arr(Expr *e)
{
        while (e) {
                resolve_expr(e);
                e = e->next;
        }
}

static void resolve_stmt_arr(Stmt *s, Stmt **);

static void
resolve_stmt(Stmt *s)
{
        switch (s->type) {
        case VARDECLSTMT:
                resolve_expr_arr(s->vardecl.value);
                if (s->vardecl.value)
                        declare(s->vardecl.name->str_literal);
                else
                        define(s->vardecl.name->str_literal);
                break;
        case FUNDECLSTMT:
                define(s->funcdecl.name->str_literal);
                env_create();
                for (vtok *arg = s->funcdecl.params; arg; arg = arg->next) {
                        declare(arg->str_literal);
                }
                resolve_stmt_arr(s->funcdecl.body, NULL);
                env_destroy();
                break;
        case BLOCKSTMT:
                env_create();
                resolve_stmt_arr(s->block.body, NULL);
                env_destroy();
                break;
        case EXPRSTMT:
                resolve_expr_arr(s->expr.body);
                break;
        case ASSERTSTMT:
                resolve_expr_arr(s->assert.body);
                break;
        case IFSTMT:
                resolve_expr(s->ifstmt.cond);
                resolve_stmt(s->ifstmt.body);
                if (s->ifstmt.elsebody)
                        resolve_stmt(s->ifstmt.elsebody);
                break;
        case WHILESTMT:
                resolve_expr(s->whilestmt.cond);
                resolve_stmt(s->whilestmt.body);
                break;
        case RETSTMT:
                resolve_expr(s->retstmt.value);
                break;
        default:
                report("No yet implemented: resolve_stmt for %s\n",
                       STMT_REPR[s->type]);
                resolve_error();
                break;
        }
}

static void
resolve_stmt_arr(Stmt *s, Stmt **current)
{
        while (s) {
                if (current) *current = s;
                resolve_stmt(s);
                s = s->next;
        }
}

void
load_env_data(Env *env)
{
        int i = 0;
        int len = shlenu(env->map);
        for (; i < len; i++) {
                if (env->map[i].value.type != TYPE_NONE)
                        define(env->map[i].key);
                else
                        declare(env->map[i].key);
        }

        if (env->upper) load_env_data(env->upper);
}

int
resolve()
{
        Env *prev = env_create_e(NULL);
        Stmt *current = NULL;
        int has_error = 0;
        load_env_data(prev);
        if (setjmp(resolve_error_jmp) | setjmp(eval_runtime_error)) {
                has_error = 1;
        }
        resolve_stmt_arr(current ? current->next : head_stmt, &current);
        env_destroy_e(prev);
        return has_error;
}

Value
env_get_l(void *e, char *name)
{
        return env_get_o(sidetable_get(e), name);
}

Value
env_set_l(void *e, char *name, Value value)
{
        return env_set_o(sidetable_get(e), name, value);
}
