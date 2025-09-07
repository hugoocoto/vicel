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
/* VISPEL interpreter - Core lib - list implementation
 *
 * Author: Hugo Coto Florez
 * Repo: github.com/hugocotoflorez/vispel
 *
 * */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h> // alloc
#include <string.h> // memmove

#include "core.h"

#define DA_REALLOC(dest, size) realloc((dest), (size));

typedef struct {
        int capacity;
        int size;
        Value *data;
} *List;

Value *
get_values(Expr *e)
{
        Value *v = NULL;
        int i = 0;
        while (e) {
                v = realloc(v, sizeof *v * (i + 1));
                v[i] = vspl_eval_expr(e);
                e = e->next;
                i++;
        }
        return v;
}

Value *
get_values_n(Expr *e, int *n)
{
        Value *v = NULL;
        *n = 0;
        while (e) {
                v = realloc(v, sizeof *v * (*n + 1));
                v[*n] = vspl_eval_expr(e);
                e = e->next;
                (*n)++;
        }
        return v;
}


static void
check_valid_list(Value l)
{
        if (l.type != TYPE_ADDR) {
                report("Argument `l` of type %s incompatible with LIST",
                       VALTYPE_REPR[l.type]);
                longjmp(eval_runtime_error, 1);
        }
}

// add E to DA_PTR that is a pointer to a DA of the same type as E
#define da_append(da_ptr, e)                                             \
        ({                                                               \
                if ((da_ptr)->size >= (da_ptr)->capacity) {              \
                        (da_ptr)->capacity += 3;                         \
                        (da_ptr)->data = DA_REALLOC(                     \
                        (da_ptr)->data,                                  \
                        sizeof(*((da_ptr)->data)) * (da_ptr)->capacity); \
                        assert(da_ptr);                                  \
                }                                                        \
                assert((da_ptr)->size < (da_ptr)->capacity);             \
                (da_ptr)->data[(da_ptr)->size++] = (e);                  \
                (da_ptr)->size - 1;                                      \
        })

void
list_append(Value l, Value e)
{
        check_valid_list(l);
        da_append((List) l.addr, e);
}

Value
core_list_append(Expr *e)
{
        Value *v = get_values(e);
        list_append(v[0], v[1]);
        free(v);
        return NO_VALUE;
}

/* Destroy DA pointed by DA_PTR. DA can be initialized again but previous
 * values are not accessible anymore. */
#define da_destroy(da_ptr)              \
        ({                              \
                (da_ptr)->capacity = 0; \
                (da_ptr)->size = 0;     \
                free((da_ptr)->data);   \
                (da_ptr)->data = NULL;  \
        })

void
list_destroy(Value l)
{
        check_valid_list(l);
        da_destroy((List) l.addr);
}

Value
core_list_destroy(Expr *e)
{
        Value *v = get_values(e);
        list_destroy(v[0]);
        free(v[0].addr);
        free(v);
        return NO_VALUE;
}

/* Insert element E into DA pointed by DA_PTR at index I. */
#define da_insert(da_ptr, e, i)                                                 \
        ({                                                                      \
                assert((i) >= 0 && (i) <= (da_ptr)->size);                      \
                da_append(da_ptr, e);                                           \
                memmove((da_ptr)->data + (i) + 1, (da_ptr)->data + (i),         \
                        ((da_ptr)->size - (i) - 1) * sizeof *((da_ptr)->data)); \
                (da_ptr)->data[i] = (e);                                        \
                (da_ptr)->size - 1;                                             \
        })

void
list_insert(Value l, Value e, Value i)
{
        check_valid_list(l);
        if (i.type != TYPE_NUM) {
                report("Argument `i` of type %s incompatible with NUM",
                       VALTYPE_REPR[TYPE_ADDR]);
                longjmp(eval_runtime_error, 1);
        }
        da_insert((List) l.addr, e, i.num);
}

Value
core_list_insert(Expr *e)
{
        Value *v = get_values(e);
        list_insert(v[0], v[1], v[2]);
        free(v);
        return NO_VALUE;
}

/* Get size */
#define da_getsize(da) ((da).size)

Value
list_size(Value l)
{
        check_valid_list(l);
        return (Value) {
                .num = da_getsize(*(List) l.addr),
                .type = TYPE_NUM,
        };
}

Value
core_list_size(Expr *e)
{
        Value *v = get_values(e);
        Value ret = list_size(v[0]);
        free(v);
        return ret;
}

/* Get the index of an element given a pointer to this element */
#define da_index(da_elem_ptr, da) (int) ((da_elem_ptr) - (da.data))

/* Remove element al index I */
#define da_remove(da_ptr, i)                                              \
        ({                                                                \
                if (i >= 0 && i < (da_ptr)->size) {                       \
                        --(da_ptr)->size;                                 \
                        memmove(                                          \
                        (da_ptr)->data + (i), (da_ptr)->data + (i) + 1,   \
                        ((da_ptr)->size - (i)) * sizeof *(da_ptr)->data); \
                }                                                         \
        })

void
list_remove(Value l, Value i)
{
        check_valid_list(l);
        if (i.type != TYPE_NUM) {
                report("Argument `i` of type %s incompatible with NUM",
                       VALTYPE_REPR[TYPE_ADDR]);
                longjmp(eval_runtime_error, 1);
        }
        da_remove((List) l.addr, i.num);
}

Value
core_list_remove(Expr *e)
{
        Value *v = get_values(e);
        list_remove(v[0], v[1]);
        free(v);
        return NO_VALUE;
}

/* can be used as:
 * for_da_each(i, DA), where
 * - i: variable where a pointer to an element from DA is going to be stored
 * - DA: is a valid DA */
#define for_da_each(_i_, da)                                                 \
        for (AUTO_TYPE _i_ = (da).data; (int) (_i_ - (da).data) < (da).size; \
             ++_i_)

Value
list_get(Value l, Value i)
{
        check_valid_list(l);
        if (i.type != TYPE_NUM) {
                report("Argument `i` of type %s incompatible with NUM",
                       VALTYPE_REPR[TYPE_ADDR]);
                longjmp(eval_runtime_error, 1);
        }
        if (i.num < 0 || i.num >= ((List) l.addr)->size) {
                report("List index out of range: %d for list length %d",
                       i.num, ((List) l.addr)->size);
                longjmp(eval_runtime_error, 1);
        }
        return ((List) l.addr)->data[i.num];
}

Value
core_list_get(Expr *e)
{
        Value *v = get_values(e);
        Value ret = list_get(v[0], v[1]);
        free(v);
        return ret;
}

/* Initialize DA_PTR (that is a pointer to a DA). Initial size (int) can be
 * passed as second argument. */
#define da_init(da_ptr, ...)                                                                      \
        ({                                                                                        \
                (da_ptr)->capacity = 256;                                                         \
                __VA_OPT__((da_ptr)->capacity = (__VA_ARGS__);)                                   \
                (da_ptr)->size = 0;                                                               \
                (da_ptr)->data = DA_REALLOC(NULL, sizeof *((da_ptr)->data) * (da_ptr)->capacity); \
                assert(da_ptr);                                                                   \
                da_ptr;                                                                           \
        })

Value
core_list_init(Expr *e)
{
        List l = calloc(1, sizeof *l);
        da_init(l);
        int n;
        Value *v = get_values_n(e, &n);

        for (int i = 0; i < n; i++)
                da_append(l, v[i]);

        free(v);
        return (Value) { .addr = l, .type = TYPE_ADDR };
}

static __attribute__((constructor)) void
__init__()
{
        preload("append", core_list_append, 2);
        preload("add", core_list_insert, 3);
        preload("remove", core_list_remove, 2);
        preload("destroy", core_list_destroy, 1);
        preload("length", core_list_size, 1);
        preload("get", core_list_get, 2);
        preload("list", core_list_init, 0 | VAARGS); // 0 or more arguments
}
