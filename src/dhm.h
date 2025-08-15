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

#ifndef DHM_H_
#define DHM_H_

typedef struct {
        char *key;
        void *value;
        int d;
} DHnode;

typedef struct {
        int d;
        DHnode **data;
} DHmap;

void dhmadd(DHmap *, char *key, void *value);
void dhmremove(DHmap *, char *key);
void *dhmget(DHmap *, char *key);
void dhmdestroy(DHmap *);
void dhmprint(DHmap hm);
void dhmdestroya(DHmap *hm, void (*destra)(void *));

#endif //! DHM_H_
