/* Action Prefix Tree
 *
 * Author: Hugo Coto Florez
 */

#include "action.h"
#include "common.h"
#include <assert.h>
#include <stdio.h>

typedef struct __APTree {
        Action action;
        struct __APTree *after[128]; // can be reduced
        int descents;
} *APTree;

APTree ap_init();
void ap_add(APTree t, char *prefix, Action action);
Action ap_get(APTree t, char *prefix);
void ap_remove(APTree t, char *prefix);

APTree
ap_init()
{
        return calloc(1, sizeof(struct __APTree));
}

void
ap_add(APTree t, char *prefix, Action action)
{
        char *cchar;
        APTree ct = t;
        for (cchar = prefix; *cchar; cchar++) {
                if (ct->after[*cchar] == NULL) {
                        ct->after[*cchar] = ap_init();
                        ++ct->descents;
                }

                ct = ct->after[*cchar];
        }
        ct->action = action;
}


Action
ap_get(APTree t, char *prefix)
{
        char *cchar;
        APTree ct = t;
        for (cchar = prefix; *cchar; cchar++) {
                if (ct->after[*cchar] == NULL)
                        return NoAction;

                ct = ct->after[*cchar];
        }
        return ct->action;
}

void
ap_remove(APTree t, char *prefix)
{
        if (t->after[*prefix] == NULL) return;

        ap_remove(t->after[*prefix], prefix + 1);
        if (t->after[*prefix]->descents <= 0) {
                free(t->after[*prefix]);
                t->after[*prefix] = NULL;
                return;
        }
        if (prefix[1] == 0)
                t->after[*prefix]->action = NoAction;
}

void
ap_print_branch(APTree t, int indent)
{
        int i = 0;
        int len = sizeof(t->after) / sizeof(*t->after);
        printf("%p (descents %d)\n", t->action.action, t->descents);
        for (; i < len; i++) {
                if (t->after[i]) {
                        indent += 4;
                        printf("%-*s%c: ", indent, "", i);
                        ap_print_branch(t->after[i], indent);
                }
        }
}

void
ap_print(APTree t)
{
        printf("Default: ");
        ap_print_branch(t, 0);
}

static __attribute__((constructor)) void
test()
{
        APTree t = ap_init();
        assert(t);
        ap_add(t, "hugo", (Action) { .action = (void *) 0x3 });
        ap_add(t, "hugoL", (Action) { .action = (void *) 0x4 });
        ap_add(t, "hu", (Action) { .action = (void *) 0x5 });
        assert(ap_get(t, "hugo").action == (void *) 0x3);
        assert(ap_get(t, "hugoL").action == (void *) 0x4);
        assert(ap_get(t, "hu").action == (void *) 0x5);
        ap_remove(t, "hu");
        assert(ap_get(t, "hu").action == NoAction.action);
        ap_add(t, "hu", (Action) { .action = (void *) 0x6 });
        assert(ap_get(t, "hu").action == (void *) 0x6);
}
