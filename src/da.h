#ifndef DYNAMIC_ARRAY_H
#define DYNAMIC_ARRAY_H

#include <stdlib.h> // alloc

#if defined(__cplusplus)
#if defined(__GNUC__)
/* Gnu dependent */
#define DA_REALLOC(dest, size) (__typeof__ (dest))realloc ((dest), (size))
#else
#error "DA requires gnu extensions if using C++."
#endif
#else
#define DA_REALLOC(dest, size) realloc ((dest), (size));
#define DA_MALLOC(size) malloc ((size));
#endif

#ifdef __cplusplus
#define AUTO_TYPE auto
#else
#define AUTO_TYPE __auto_type
#endif

#define DA(type)                                                              \
        struct {                                                              \
                int capacity;                                                 \
                int size;                                                     \
                type *data;                                                   \
        }

#include <assert.h>
/* Initialize DA_PTR (that is a pointer to a DA). Initial size (int) can be
 * passed as second argument, default is 4. */
#define da_init(da_ptr, ...)                                                  \
        ({                                                                    \
                DEPRECATED ("da_init -> zero initialize it\n");               \
                (da_ptr)->capacity = 256;                                     \
                __VA_OPT__ ((da_ptr)->capacity = (__VA_ARGS__));              \
                (da_ptr)->size = 0;                                           \
                (da_ptr)->data = NULL;                                        \
                (da_ptr)->data                                                \
                = DA_REALLOC ((da_ptr)->data,                                 \
                              sizeof *((da_ptr)->data) * (da_ptr)->capacity); \
                assert (da_ptr);                                              \
                da_ptr;                                                       \
        })

#include <assert.h>
// add E to DA_PTR that is a pointer to a DA of the same type as E
#define da_append(da_ptr, e)                                                  \
        ({                                                                    \
                if ((da_ptr)->size >= (da_ptr)->capacity) {                   \
                        (da_ptr)->capacity += 3;                              \
                        (da_ptr)->data = DA_REALLOC (                         \
                        (da_ptr)->data,                                       \
                        sizeof (*((da_ptr)->data)) * (da_ptr)->capacity);     \
                        assert (da_ptr);                                      \
                }                                                             \
                assert ((da_ptr)->size < (da_ptr)->capacity);                 \
                (da_ptr)->data[(da_ptr)->size++] = (e);                       \
                (da_ptr)->size - 1;                                           \
        })

/* Destroy DA pointed by DA_PTR. DA can be initialized again but previous
 * values are not accessible anymore. */
#define da_destroy(da_ptr)                                                    \
        ({                                                                    \
                (da_ptr)->capacity = 0;                                       \
                (da_ptr)->size = 0;                                           \
                free ((da_ptr)->data);                                        \
                (da_ptr)->data = NULL;                                        \
        })

/* Insert element E into DA pointed by DA_PTR at index I. */
#include <string.h> // memmove
#define da_insert(da_ptr, e, i)                                               \
        ({                                                                    \
                assert ((i) >= 0 && (i) <= (da_ptr)->size);                   \
                da_append (da_ptr, e);                                        \
                memmove ((da_ptr)->data + (i) + 1, (da_ptr)->data + (i),      \
                         ((da_ptr)->size - (i) - 1)                           \
                         * sizeof *((da_ptr)->data));                         \
                (da_ptr)->data[i] = (e);                                      \
                (da_ptr)->size - 1;                                           \
        })

/* Get size */
#define da_getsize(da) ((da).size)

/* Get the index of an element given a pointer to this element */
#define da_index(da_elem_ptr, da) (int)((da_elem_ptr) - (da.data))

/* Remove element al index I */
#define da_remove(da_ptr, i)                                                  \
        ({                                                                    \
                if (i >= 0 && i < (da_ptr)->size) {                           \
                        --(da_ptr)->size;                                     \
                        memmove (                                             \
                        (da_ptr)->data + (i), (da_ptr)->data + (i) + 1,       \
                        ((da_ptr)->size - (i)) * sizeof *(da_ptr)->data);     \
                }                                                             \
        })

/* can be used as:
 * for_da_each(i, DA), where
 * - i: variable where a pointer to an element from DA is going to be stored
 * - DA: is a valid DA */
#define for_da_each(_i_, da)                                                  \
        for (AUTO_TYPE _i_ = (da).data; (int)(_i_ - (da).data) < (da).size;   \
             ++_i_)

#endif
