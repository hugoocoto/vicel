/* VISPEL interpreter - Evalue AST branch
 *
 * Author: Hugo Coto Florez
 * Repo: github.com/hugocotoflorez/vispel
 *
 * */



#include <assert.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "env.h"
#include "interpreter.h"
#include "tokens.h"

/* return jump and value storage */
Value ret_val;
jmp_buf ret_env;

jmp_buf eval_runtime_error;

static inline _Noreturn void
runtime_error()
{
        longjmp(eval_runtime_error, 1);
}

/* Todo: use this instead of print_val and printf newline */
void
print_valnl(Value v)
{
        switch (v.type) {
        case TYPE_NUM:
        case TYPE_STR:
        case TYPE_NONE:
                print_val(v);
                printf("\n");
                break;
        default:
                report("No yet implemented: print_val for %s\n",
                       VALTYPE_REPR[v.type]);
                runtime_error();
        }
}

void
print_val(Value v)
{
        switch (v.type) {
        case TYPE_NUM:
                printf("%d", v.num);
                break;
        case TYPE_STR:
                printf("\"%s\"", v.str);
                break;
        case TYPE_NONE:
                break;
        default:
                report("No yet implemented: print_val for %s\n",
                       VALTYPE_REPR[v.type]);
                runtime_error();
        }
}

static Value
eval_litexpr(Expr *e)
{
        Value v;
        switch (e->litexpr.value->token) {
        case STRING:
                v.type = TYPE_STR;
                v.str = e->litexpr.value->str_literal;
                break;
        case NUMBER:
                v.type = TYPE_NUM;
                v.num = e->litexpr.value->num_literal;
                break;
        case TRUE:
                v.type = TYPE_NUM;
                v.num = 1;
                break;
        case FALSE:
                v.type = TYPE_NUM;
                v.num = 0;
                break;
        case IDENTIFIER:
                v = env_get_l(e, e->litexpr.value->str_literal);
                break;
        default:
                report("No yet implemented: eval_litexpr for %s\n",
                       TOKEN_REPR[e->litexpr.value->token]);
                runtime_error();
        }
        return v;
}

static _Noreturn void
panik_invalid_binop(Value L, vtoktype OP, Value R)
{
        report("Invalid binary operation %s between %s and %s\n",
               TOKEN_REPR[OP], VALTYPE_REPR[L.type], VALTYPE_REPR[R.type]);
        runtime_error();
}

static _Noreturn void
panik_invalid_unop(vtoktype OP, Value R)
{
        report("Invalid binary operation %s for %s\n",
               TOKEN_REPR[OP], VALTYPE_REPR[R.type]);
        runtime_error();
}

static int
is_true(Value v)
{
        switch (v.type) {
        case TYPE_NUM:
                return v.num;
        case TYPE_STR:
                return v.str && *v.str;
        default:
                report("No yet implemented: is_true for %s\n",
                       VALTYPE_REPR[v.type]);
                runtime_error();
        }
}

static int
is_equal(Value v1, Value v2)
{
        if (v1.type != v2.type) {
                report("Type missmatch in is_equal: %s and %s\n",
                       VALTYPE_REPR[v1.type], VALTYPE_REPR[v2.type]);
                return 0;
        }

        switch (v1.type) {
        case TYPE_NUM:
                return v1.num == v2.num;
        case TYPE_STR:
                return strcmp(v1.str, v2.str) == 0;
        default:
                report("No yet implemented: is_equal for %s and %s\n",
                       VALTYPE_REPR[v1.type], VALTYPE_REPR[v2.type]);
                runtime_error();
        }
}

static int
is_greater(Value v1, Value v2)
{
        if (v1.type != v2.type) {
                report("Type missmatch in is_greater: %s and %s\n",
                       VALTYPE_REPR[v1.type], VALTYPE_REPR[v2.type]);
                return 0;
        }

        switch (v1.type) {
        case TYPE_NUM:
                return v1.num > v2.num;
        case TYPE_STR:
                // return strcmp(v1.str, v2.str) > 0;
        default:
                report("No yet implemented: is_greater for %s and %s\n",
                       VALTYPE_REPR[v1.type], VALTYPE_REPR[v2.type]);
                runtime_error();
        }
}

static int
is_greater_equal(Value v1, Value v2)
{
        return is_equal(v1, v2) || is_greater(v1, v2);
}

static Value
eval_binexpr(Expr *e)
{
        Value v;
        Value lhs = vspl_eval_expr(e->binexpr.lhs);
        Value rhs = vspl_eval_expr(e->binexpr.rhs);

        switch (e->binexpr.op->token) {
        case MINUS:
                if (rhs.type == TYPE_NUM && lhs.type == TYPE_NUM) {
                        v.type = TYPE_NUM;
                        v.num = lhs.num - rhs.num;
                        break;
                }
                panik_invalid_binop(lhs, e->binexpr.op->token, rhs);

        case PLUS:
                if (rhs.type == TYPE_NUM && lhs.type == TYPE_NUM) {
                        v.type = TYPE_NUM;
                        v.num = lhs.num + rhs.num;
                        break;
                }
                panik_invalid_binop(lhs, e->binexpr.op->token, rhs);

        case SLASH:
                if (rhs.type == TYPE_NUM && lhs.type == TYPE_NUM) {
                        v.type = TYPE_NUM;
                        v.num = lhs.num / rhs.num;
                        break;
                }
                panik_invalid_binop(lhs, e->binexpr.op->token, rhs);

        case STAR:
                if (rhs.type == TYPE_NUM && lhs.type == TYPE_NUM) {
                        v.type = TYPE_NUM;
                        v.num = lhs.num * rhs.num;
                        break;
                }
                panik_invalid_binop(lhs, e->binexpr.op->token, rhs);

        case AND:
                v.type = TYPE_NUM;
                v.num = is_true(lhs) && is_true(rhs);
                break;

        case OR:
                v.type = TYPE_NUM;
                v.num = is_true(lhs) && is_true(rhs);
                break;

        case BITWISE_AND:
                if (rhs.type == TYPE_NUM && lhs.type == TYPE_NUM) {
                        v.type = TYPE_NUM;
                        v.num = rhs.num & lhs.num;
                        break;
                }
                panik_invalid_binop(lhs, e->binexpr.op->token, rhs);

        case BITWISE_OR:
                if (rhs.type == TYPE_NUM && lhs.type == TYPE_NUM) {
                        v.type = TYPE_NUM;
                        v.num = rhs.num | lhs.num;
                        break;
                }
                panik_invalid_binop(lhs, e->binexpr.op->token, rhs);

        case BITWISE_XOR:
                if (rhs.type == TYPE_NUM && lhs.type == TYPE_NUM) {
                        v.type = TYPE_NUM;
                        v.num = rhs.num ^ lhs.num;
                        break;
                }
                panik_invalid_binop(lhs, e->binexpr.op->token, rhs);

        case EQUAL_EQUAL:
                v.type = TYPE_NUM;
                v.num = is_equal(lhs, rhs);
                break;

        case BANG_EQUAL:
                v.type = TYPE_NUM;
                v.num = !is_equal(lhs, rhs);
                break;

        case GREATER:
                v.type = TYPE_NUM;
                v.num = is_greater(lhs, rhs);
                break;

        case GREATER_EQUAL:
                v.type = TYPE_NUM;
                v.num = is_greater_equal(lhs, rhs);
                break;

        case LESS:
                v.type = TYPE_NUM;
                v.num = !is_greater_equal(lhs, rhs);
                break;

        case LESS_EQUAL:
                v.type = TYPE_NUM;
                v.num = !is_greater(lhs, rhs);
                break;

        default:
                report("Binexpr Operation no yet implemented: %s\n",
                       TOKEN_REPR[e->litexpr.value->token]);
                runtime_error();
        }
        return v;
}

static Value
eval_unexpr(Expr *e)
{
        Value v;
        Value lhs = vspl_eval_expr(e->unexpr.rhs);

        switch (e->unexpr.op->token) {
        case BANG:
                v.type = TYPE_NUM;
                v.num = !is_true(lhs);
                break;
        case MINUS:
                if (lhs.type == TYPE_NUM) {
                        v.type = TYPE_NUM;
                        v.num = -lhs.num;
                        break;
                }
                panik_invalid_unop(e->unexpr.op->token, lhs);
        case BITWISE_NOT:
                if (lhs.type == TYPE_NUM) {
                        v.type = TYPE_NUM;
                        v.num = ~lhs.num;
                        break;
                }
                panik_invalid_unop(e->unexpr.op->token, lhs);
        default:
                report("Unexpr operation no yet implemented: %s\n",
                       TOKEN_REPR[e->litexpr.value->token]);
                runtime_error();
        }
        return v;
}

Value vspl_eval_expr(Expr *e);

static Value
eval_assignexpr(Expr *s)
{
        char *name = s->assignexpr.name->str_literal;
        Value v = vspl_eval_expr(s->assignexpr.value);
        return env_set_l(s, name, v);
}

static Value
eval_orexpr(Expr *e)
{
        Value v;
        v = vspl_eval_expr(e->orexpr.lhs);
        if (is_true(v)) return v;
        v = vspl_eval_expr(e->orexpr.rhs);
        if (is_true(v)) return v;
        return (Value) { .num = 0, .type = TYPE_NUM };
}

static Value
eval_andexpr(Expr *e)
{
        Value v = (Value) { .num = 0, .type = TYPE_NUM };
        if (is_true(vspl_eval_expr(e->orexpr.lhs)) &&
            is_true(vspl_eval_expr(e->orexpr.rhs)))
                v.num = 1;
        return v;
}

static Value eval_stmt(Stmt *s);

static Value
eval_callexpr(Expr *e)
{
        Value func = vspl_eval_expr(e->callexpr.name);
        switch (func.type) {
        case TYPE_CALLABLE:
        case TYPE_CORE_CALL:
                break;
        default:
                report("Calling a non callable expression\n");
                runtime_error();
        }

        if (func.call.arity & VAARGS) {
                if (e->callexpr.count < (func.call.arity & ~VAARGS)) {
                        report("Function `%s` expect at least %b arguments, "
                               "but got %b\n",
                               func.call.name, (func.call.arity & ~VAARGS),
                               e->callexpr.count);
                        runtime_error();
                }
        } else if (e->callexpr.count != func.call.arity) {
                report("Function `%s` expect %d arguments, but got %d\n",
                       func.call.name, func.call.arity, e->callexpr.count);
                runtime_error();
        }

        Env *prev;
        Value prev_ret_val;
        jmp_buf prev_ret_env;
        Value ret = NO_VALUE;

        Expr *arg = e->callexpr.args;
        vtok *param = func.call.params;

        env_create();

        if (param) {
                while (arg) {
                        env_add(param->str_literal, vspl_eval_expr(arg));
                        arg = arg->next;
                        param = param->next;
                }
                if (arg || param) {
                        report("Compiler Error: function args list and params"
                               "list has not the same length\n");
                        exit(1);
                }
        }

        switch (func.type) {
        case TYPE_CALLABLE:
                prev = env_change_upper(func.call.closure);
                prev_ret_val = ret_val;
                memcpy(prev_ret_env, ret_env, sizeof ret_env);
                if (setjmp(ret_env))
                        ret = ret_val;
                else
                        ret = eval_stmt(func.call.body);
                ret_val = prev_ret_val;
                memcpy(ret_env, prev_ret_env, sizeof ret_env);
                env_destroy_e(prev);
                break;
        case TYPE_CORE_CALL:
                ret = func.call.ifunc(e->callexpr.args);
                env_destroy();
                break;

        default:
                report("No yet implemented: eval_callexpr for %s\n",
                       VALTYPE_REPR[func.type]);
                runtime_error();
        }
        return ret;
}

Value
vspl_eval_expr(Expr *e)
{
        switch (e->type) {
        case LITEXPR:
                return eval_litexpr(e);
        case BINEXPR:
                return eval_binexpr(e);
        case UNEXPR:
                return eval_unexpr(e);
        case ASSIGNEXPR:
                return eval_assignexpr(e);
        case OREXPR:
                return eval_orexpr(e);
        case ANDEXPR:
                return eval_andexpr(e);
        case CALLEXPR:
                return eval_callexpr(e);
        case VAREXPR:
        default:
                report("No yet implemented: vspl_eval_expr for %s\n", EXPR_REPR[e->type]);
                runtime_error();
                break;
        }
        return NO_VALUE;
}

static Value eval_stmt_arr(Stmt *s);

// static ValueNode *
// new_valuenode()
// {
//         return calloc(1, sizeof(ValueNode));
// }

// static ValueNode *
// parse_params(Expr *e)
// {
//         ValueNode *vn = new_valuenode();
//         ValueNode *v = vn;
//         while (e) {
//                 v->v = vspl_eval_expr(e);
//                 v->next = new_valuenode();
//                 v = v->next;
//                 e = e->next;
//         }
//         return vn;
// }

static void
eval_funcdeclstmt(Stmt *s)
{
        Value v;
        v.type = TYPE_CALLABLE;
        v.call.arity = s->funcdecl.arity;
        v.call.params = s->funcdecl.params;
        v.call.name = s->funcdecl.name->str_literal;
        v.call.body = s->funcdecl.body;
        v.call.closure = get_current_env();
        env_add(s->funcdecl.name->str_literal, v);
}

static Value
eval_stmt(Stmt *s)
{
        switch (s->type) {
        case EXPRSTMT:
                return vspl_eval_expr(s->expr.body);
        case VARDECLSTMT:
                env_add(s->vardecl.name->str_literal,
                        vspl_eval_expr(s->vardecl.value));
                break;
        case FUNDECLSTMT:
                eval_funcdeclstmt(s);
                break;
        case ASSERTSTMT:
                if (!is_true(vspl_eval_expr(s->assert.body))) {
                        report("Assert failed\n");
                        runtime_error();
                }
                break;
        case BLOCKSTMT:
                env_create();
                eval_stmt_arr(s->block.body);
                env_destroy();
                break;
        case IFSTMT:
                if (is_true(vspl_eval_expr(s->ifstmt.cond))) {
                        eval_stmt(s->ifstmt.body);
                } else if (s->ifstmt.elsebody) {
                        eval_stmt(s->ifstmt.elsebody);
                }
                break;
        case WHILESTMT:
                while (is_true(vspl_eval_expr(s->whilestmt.cond))) {
                        eval_stmt(s->whilestmt.body);
                }
                break;
        case RETSTMT:
                ret_val = vspl_eval_expr(s->retstmt.value);
                longjmp(ret_env, 1);
        default:
                report("Todo: eval_stmt for %s\n", STMT_REPR[s->type]);
                runtime_error();
                break;
        }
        return NO_VALUE;
}

static Value
eval_stmt_arr(Stmt *s)
{
        Value v = NO_VALUE;
        while (s) {
                v = eval_stmt(s);
                s = s->next;
        }
        return v;
}

inline void
eval_quiet()
{
        if (setjmp(eval_runtime_error)) {
                return;
        }
}

inline void
eval()
{
        eval_quiet();
        print_val(eval_stmt_arr(head_stmt));
        printf("\n");
}
