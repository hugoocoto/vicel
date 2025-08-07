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

#include "aptree.h"
#include "action.h"
#include "common.h"
#include "debug.h"
#include <string.h>

void
check_prefix(char *prefix, int length)
{
        for (int i = 0; i < length; i++) {
                if (prefix[i] < ' ' || prefix[i] >= 127) {
                        report("Invalid prefix: `%.*s`", length, prefix);
                        return;
                }
        }
}

APTree
ap_init()
{
        return calloc(1, sizeof(struct __APTree));
}

void
ap_add(APTree t, char *prefix, Action action)
{
        check_prefix(prefix, strlen(prefix));
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

bool
ap_has_descents(APTree t, char *prefix)
{
        check_prefix(prefix, strlen(prefix));
        char *cchar;
        APTree ct = t;
        for (cchar = prefix; *cchar; cchar++) {
                if (ct->after[*cchar] == NULL)
                        return false;

                ct = ct->after[*cchar];
        }
        return ct->descents > 0;
}

bool
ap_has_descentsl(APTree t, char *prefix, int len)
{
        check_prefix(prefix, len);
        APTree ct = t;
        int i = 0;
        for (; i < len; i++) {
                if (ct->after[prefix[i]] == NULL)
                        return false;

                ct = ct->after[prefix[i]];
        }
        return ct->descents > 0;
}

Action
ap_get(APTree t, char *prefix)
{
        check_prefix(prefix, strlen(prefix));
        char *cchar;
        APTree ct = t;
        for (cchar = prefix; *cchar; cchar++) {
                if (ct->after[*cchar] == NULL)
                        return NoAction;

                ct = ct->after[*cchar];
        }
        return ct->action;
}

Action
ap_getl(APTree t, char *prefix, int len)
{
        check_prefix(prefix, len);
        APTree ct = t;
        int i = 0;
        for (; i < len; i++) {
                if (ct->after[prefix[i]] == NULL)
                        return NoAction;

                ct = ct->after[prefix[i]];
        }
        return ct->action;
}

Action
ap_get_unique(APTree t, char *prefix)
{
        check_prefix(prefix, strlen(prefix));
        char *cchar;
        APTree ct = t;
        for (cchar = prefix; *cchar; cchar++) {
                if (ct->after[*cchar] == NULL)
                        return NoAction;

                ct = ct->after[*cchar];
        }
        return ct->descents == 0 ? ct->action : NoAction;
}

Action
ap_getl_unique(APTree t, char *prefix, int len)
{
        check_prefix(prefix, len);
        APTree ct = t;
        int i = 0;
        for (; i < len; i++) {
                if (ct->after[prefix[i]] == NULL)
                        return NoAction;

                ct = ct->after[prefix[i]];
        }

        return ct->descents == 0 ? ct->action : NoAction;
}

Action
ap_get_last(APTree t, char *prefix)
{
        check_prefix(prefix, strlen(prefix));
        char *cchar;
        APTree ct = t;
        Action last = NoAction;
        for (cchar = prefix; *cchar; cchar++) {
                if (ct->after[*cchar] == NULL)
                        return last;

                ct = ct->after[*cchar];
                if (action_is_valid(ct->action)) last = ct->action;
        }
        return last;
}

Action
ap_getl_last(APTree t, char *prefix, int len)
{
        check_prefix(prefix, len);
        APTree ct = t;
        int i = 0;
        Action last = NoAction;
        for (; i < len; i++) {
                if (ct->after[prefix[i]] == NULL)
                        return last;

                ct = ct->after[prefix[i]];
                if (action_is_valid(ct->action)) last = ct->action;
        }
        return last;
}

void
ap_remove(APTree t, char *prefix)
{
        check_prefix(prefix, strlen(prefix));
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

void
ap_destroy(APTree t)
{
        int i = 0;
        for (; i < AP_ENTRIES; ++i) {
                if (t->after[i]) {
                        ap_destroy(t->after[i]);
                }
        }
        free(t);
}

static __attribute__((constructor)) void
test()
{
        APTree t = ap_init();
        assert(t);
        ap_add(t, "hugo", (Action) { .action = (void *) 0x3 });
        ap_add(t, "hugoL", (Action) { .action = (void *) 0x4 });
        ap_add(t, "hu", (Action) { .action = (void *) 0x5 });
        assert(ap_get_last(t, "hul").action == (void *) 0x5);
        assert(ap_get(t, "hugo").action == (void *) 0x3);
        assert(ap_get(t, "hugoL").action == (void *) 0x4);
        assert(ap_get(t, "hu").action == (void *) 0x5);
        ap_remove(t, "hu");
        assert(ap_get(t, "hu").action == NoAction.action);
        ap_add(t, "hu", (Action) { .action = (void *) 0x6 });
        assert(ap_get(t, "hu").action == (void *) 0x6);
        ap_destroy(t);
}
