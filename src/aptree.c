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
#include "dhm.h"

APTree
ap_init()
{
        APTree aptree = calloc(1, sizeof(*aptree));
        if (!aptree) {
                perror("calloc");
                exit(EXIT_FAILURE);
        }
        return aptree;
}

void
ap_add(APTree t, char *prefix, int len, Action action)
{
        if (!t || !prefix || len <= 0) return;

        int i;
        APTree node = t;
        APTree next;
        char s[2] = { 0, 0 }; // un solo carácter + '\0'

        for (i = 0; i < len; i++) {
                s[0] = prefix[i];

                /* Si no está inicializado */
                if (!node->node)
                        node->node = calloc(1, sizeof(DHmap));

                /* Obtener siguiente entrada */
                next = dhmget(node->node, s);

                /* Si no existe, crearlo */
                if (!next) {
                        next = ap_init();
                        dhmadd(node->node, s, next);
                }

                ++node->descents;
                node = next;
        }

        node->action = action;
}

APTree
ap_get(APTree t, char *prefix, int len)
{
        if (t == NULL) return NULL;
        if (prefix == NULL) return NULL;
        if (len <= 0) return NULL;

        APTree node = t;
        char s[2] = { 0, 0 };

        for (int i = 0; i < len; i++) {
                s[0] = prefix[i];

                /* If not initialized */
                if (node->node == NULL) return NULL;

                /* Get next entry */
                APTree next = dhmget(node->node, s);

                /* If entry doesn't exist */
                if (next == NULL) return NULL;

                node = next;
        }
        return node;
}

void
ap_destroyv(void *t)
{
        if (!t) return;
        APTree self = (APTree) t;
        if (self->node) {
                dhmdestroya(self->node, ap_destroyv); /* destruye recursivamente los hijos */
                free(self->node);
                self->node = NULL;
        }
        free(self);
}

Action
ap_get_last(APTree t, char *prefix, int len)
{
        int i = len;
        for (; i > 0; i--) {
                APTree node = ap_get(t, prefix, i);
                if (node == NULL) continue;
                if (action_is_valid(node->action)) return node->action;
        }
        return NOACTION;
}

Action
ap_get_unique(APTree t, char *prefix, int len)
{
        /* Not sure what "unique" was */
        APTree node = ap_get(t, prefix, len);
        if (node == NULL) return NOACTION;
        return node->descents == 0 ? node->action : NOACTION;
}

bool
ap_has_descents(APTree t, char *prefix, int len)
{
        APTree node = ap_get(t, prefix, len);
        if (node == NULL) return false;
        return node->descents > 0;
}

void
ap_remove(APTree t, char *prefix, int len)
{
        assert("No yet implemented: ap_remove" && 0);
        assert(t);
        assert(prefix);
        assert(len);
}

void
ap_destroy(APTree t)
{
        ap_destroyv(t);
}

static __attribute__((constructor)) void
test()
{
        APTree t = ap_init();
        APTree n;
        ap_add(t, "hello", 4, (Action) { .action = (void *) 1 });
        ap_add(t, "h", 1, (Action) { .action = (void *) 2 });
        ap_add(t, "hola", 4, (Action) { .action = (void *) 3 });
        n = ap_get(t, "hello", 4);
        assert(n && "hello");
        assert(n->action.action == (void *) 1);
        n = ap_get(t, "h", 1);
        assert(n && "h");
        assert(n->action.action == (void *) 2);
        n = ap_get(t, "hola", 4);
        assert(n && "hola");
        assert(n->action.action == (void *) 3);
        assert(ap_get_unique(t, "hello", 4).action == (void *) 1);
        assert(ap_get_unique(t, "h", 1).action == NULL);
        assert(ap_get_unique(t, "hola", 4).action == (void *) 3);
        assert(ap_get_last(t, "hel", 3).action == (void *) 2);
        ap_destroy(t);
}
