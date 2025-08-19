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

#ifndef HM_H_
#define HM_H_

/* Hash table */

typedef struct Hnode {
        char *key;
        void *value;
        struct Hnode *next;
} Hnode;

typedef struct
{
        int size;
        Hnode *node_arr;
} Hmap;

/* Initialize a hashmap of a given size */
void hmnew(Hmap *table, int size);
void hmadd(Hmap *table, const char *key, void *value);
void hmpop(Hmap *table, const char *key);
void *hmget(Hmap table, const char *key, void **value);
int hmhash(Hmap table, const char *key);
void hmdestroy(Hmap *table);
void hm_set_ondestroy(void (*f)(Hnode *));

#endif // !HM_H_
