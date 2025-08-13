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
                if (ct->after[(*cchar + AP_ENTRIES) % AP_ENTRIES] == NULL) {
                        ct->after[(*cchar + AP_ENTRIES) % AP_ENTRIES] = ap_init();
                        ++ct->descents;
                }

                ct = ct->after[(*cchar + AP_ENTRIES) % AP_ENTRIES];
        }
        ct->action = action;
}

bool
ap_has_descents(APTree t, char *prefix)
{
        char *cchar;
        APTree ct = t;
        for (cchar = prefix; *cchar; cchar++) {
                if (ct->after[(*cchar + AP_ENTRIES) % AP_ENTRIES] == NULL)
                        return false;

                ct = ct->after[(*cchar + AP_ENTRIES) % AP_ENTRIES];
        }
        return ct->descents > 0;
}

bool
ap_has_descentsl(APTree t, char *prefix, int len)
{
        APTree ct = t;
        int i = 0;
        for (; i < len; i++) {
                if (ct->after[(prefix[i] + AP_ENTRIES) % AP_ENTRIES] == NULL)
                        return false;

                ct = ct->after[(prefix[i] + AP_ENTRIES) % AP_ENTRIES];
        }
        return ct->descents > 0;
}

Action
ap_get(APTree t, char *prefix)
{
        char *cchar;
        APTree ct = t;
        for (cchar = prefix; *cchar; cchar++) {
                if (ct->after[(*cchar + AP_ENTRIES) % AP_ENTRIES] == NULL)
                        return NoAction;

                ct = ct->after[(*cchar + AP_ENTRIES) % AP_ENTRIES];
        }
        return ct->action;
}

Action
ap_getl(APTree t, char *prefix, int len)
{
        APTree ct = t;
        int i = 0;
        for (; i < len; i++) {
                if (ct->after[(prefix[i] + AP_ENTRIES) % AP_ENTRIES] == NULL)
                        return NoAction;

                ct = ct->after[(prefix[i] + AP_ENTRIES) % AP_ENTRIES];
        }
        return ct->action;
}

Action
ap_get_unique(APTree t, char *prefix)
{
        char *cchar;
        APTree ct = t;
        for (cchar = prefix; *cchar; cchar++) {
                if (ct->after[(*cchar + AP_ENTRIES) % AP_ENTRIES] == NULL)
                        return NoAction;

                ct = ct->after[(*cchar + AP_ENTRIES) % AP_ENTRIES];
        }
        return ct->descents == 0 ? ct->action : NoAction;
}

Action
ap_getl_unique(APTree t, char *prefix, int len)
{
        APTree ct = t;
        int i = 0;
        for (; i < len; i++) {
                if (ct->after[(prefix[i] + AP_ENTRIES) % AP_ENTRIES] == NULL)
                        return NoAction;

                ct = ct->after[(prefix[i] + AP_ENTRIES) % AP_ENTRIES];
        }

        return ct->descents == 0 ? ct->action : NoAction;
}

Action
ap_get_last(APTree t, char *prefix)
{
        char *cchar;
        APTree ct = t;
        Action last = NoAction;
        for (cchar = prefix; *cchar; cchar++) {
                if (ct->after[(*cchar + AP_ENTRIES) % AP_ENTRIES] == NULL)
                        return last;

                ct = ct->after[(*cchar + AP_ENTRIES) % AP_ENTRIES];
                if (action_is_valid(ct->action)) last = ct->action;
        }
        return last;
}

Action
ap_getl_last(APTree t, char *prefix, int len)
{
        APTree ct = t;
        int i = 0;
        Action last = NoAction;
        for (; i < len; i++) {
                if (ct->after[(prefix[i] + AP_ENTRIES) % AP_ENTRIES] == NULL)
                        return last;

                ct = ct->after[(prefix[i] + AP_ENTRIES) % AP_ENTRIES];
                if (action_is_valid(ct->action)) last = ct->action;
        }
        return last;
}

void
ap_remove(APTree t, char *prefix)
{
        if (t->after[(*prefix + AP_ENTRIES) % AP_ENTRIES] == NULL) return;

        ap_remove(t->after[(*prefix + AP_ENTRIES) % AP_ENTRIES], prefix + 1);
        if (t->after[(*prefix + AP_ENTRIES) % AP_ENTRIES]->descents <= 0) {
                free(t->after[(*prefix + AP_ENTRIES) % AP_ENTRIES]);
                t->after[(*prefix + AP_ENTRIES) % AP_ENTRIES] = NULL;
                return;
        }
        if (prefix[1] == 0)
                t->after[(*prefix + AP_ENTRIES) % AP_ENTRIES]->action = NoAction;
}

void
ap_print_branch(APTree t, int indent)
{
        int i = 0;
        int len = sizeof(t->after) / sizeof(*t->after);
        printf("%p (descents %d)\n", t->action.action, t->descents);
        for (; i < len; i++) {
                if (t->after[(i + AP_ENTRIES) % AP_ENTRIES]) {
                        indent += 4;
                        printf("%-*s%c: ", indent, "", i);
                        ap_print_branch(t->after[(i + AP_ENTRIES) % AP_ENTRIES], indent);
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
                if (t->after[(i + AP_ENTRIES) % AP_ENTRIES]) {
                        ap_destroy(t->after[(i + AP_ENTRIES) % AP_ENTRIES]);
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
