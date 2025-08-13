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

#include "hm.h"
#include "common.h"

void
hmnew(Hmap *table, int size)
{
        table->node_arr = calloc(size, sizeof(Hnode));
        table->size = size;
}

void
hmadd(Hmap *table, const char *key, void *value)
{
        Hnode *node;
        int index;

        index = hmhash(*table, key);
        node = table->node_arr + index;
        while (node->next)
                node = node->next;
        node->key = strdup(key);
        node->value = value;
        node->next = calloc(1, sizeof(Hnode));
}

void
hmpop(Hmap *table, const char *key)
{
        /* TODO: this is a piece of shit */
        Hnode *node;
        Hnode *last;
        int index;

        index = hmhash(*table, key);
        node = table->node_arr + index;
        last = table->node_arr + index;

        /* Get the node with key KEY */
        while (node->next && strcmp(node->key, key))
                node = node->next;

        /* Key is not in the list */
        if (!node)
                return;

        /* Get the last entry */
        while (last->next)
                last = last->next;

        free(node->key);
        node->key = last->key;
        node->value = last->value;

        while (node->next != last)
                node = node->next;

        node->next = NULL;

        free(last);
}

void *
hmget(Hmap table, const char *key, void **value)
{
        Hnode *node;
        int index;

        if (!key || !table.size)
                return NULL;

        index = hmhash(table, key);
        node = table.node_arr + index;

        while (node->next && strcmp(node->key, key))
                node = node->next;

        if (value)
                *value = node->value;

        return node->value;
}

int
hmhash(Hmap table, const char *key)
{
        unsigned sum = 0;

        if (!key || !table.size)
                return 0;

        for (int i = 0; key[i]; ++i)
                sum += i * key[i] * 7;

        return sum % table.size;
}

void
hmdestroy(Hmap *table)
{
        Hnode *node;
        Hnode *next;

        if (!table)
                return;

        for (int i = 0; i < table->size; i++) {
                node = table->node_arr + i;
                free(node->key);
                free(node->value);
                next = node->next;

                while ((node = next)) {
                        next = node->next;
                        free(node->key);
                        free(node->value);
                        free(node);
                }
        }

        free(table->node_arr);
        table->node_arr = NULL;
        table->size = 0;
}
