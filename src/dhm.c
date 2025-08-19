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

#include "dhm.h"
#include "common.h"

void dhmadd(DHmap *, char *key, void *value);
void dhmremove(DHmap *, char *key);
void *dhmget(DHmap *, char *key);
void dhmdestroy(DHmap *);
void dhmdestroya(DHmap *hm, void (*destra)(void *));

static int dhmhash(char *key);
static int hashmask(int hash, int bits);
static void grow(DHmap *hm);


static inline void
grow(DHmap *hm)
{
        assert(hm != NULL);

        int old_entries = (hm->d == 0) ? 1 : (1 << hm->d);
        ++hm->d;
        int new_entries = 1 << hm->d;

        size_t new_size = sizeof(*hm->data) * (size_t) new_entries;
        hm->data = realloc(hm->data, new_size);

        memmove(hm->data + old_entries, hm->data,
                sizeof(*hm->data) * (size_t) old_entries);
}

static int
dhmhash(char *key)
{
        int n = 1;
        while (*key) {
                n <<= 2;
                n += (unsigned char) *key;
                ++key;
        }
        return n;
}

static int
hashmask(int hash, int bits)
{
        assert(bits >= 0);
        // if (bits == 0) return 0;
        return hash & ((1 << bits) - 1);
}


void
dhmadd(DHmap *hm, char *key, void *value)
{
        if (!hm || !key) return;

        if (!hm->data) {
                hm->data = calloc(1, sizeof(DHnode *));
                hm->data[0] = calloc(1, sizeof(DHnode));
        }

        for (;;) {
                int k = dhmhash(key);
                int m = hashmask(k, hm->d);
                DHnode *b = hm->data[m];

                if (!b->key) {
                        b->key = strdup(key);
                        b->value = value;
                        return;
                }

                if (strcmp(b->key, key) == 0) {
                        b->value = value;
                        return;
                }

                if (b->d >= hm->d) {
                        grow(hm);
                }

                DHnode *nb = calloc(1, sizeof(DHnode));
                nb->d = b->d + 1;
                int new_ld = nb->d;
                b->d = new_ld;

                int dir_sz = (hm->d == 0) ? 1 : (1 << hm->d);
                for (int i = 0; i < dir_sz; ++i) {
                        if (hm->data[i] == b) {
                                int bit = (i >> (new_ld - 1)) & 1;
                                if (bit)
                                        hm->data[i] = nb;
                        }
                }

                char *old_key = b->key;
                void *old_val = b->value;
                b->key = NULL;
                b->value = NULL;

                dhmadd(hm, old_key, old_val);
                free(old_key);
        }
}

void *
dhmget(DHmap *hm, char *key)
{
        if (!hm || !key) return NULL;
        if (!hm->data) return NULL;

        int k = dhmhash(key);
        int m = hashmask(k, hm->d);
        DHnode *b = hm->data[m];
        if (!b) return NULL;

        if (b->key && strcmp(b->key, key) == 0)
                return b->value;

        return NULL;
}

void
dhmremove(DHmap *hm, char *key)
{
        if (!hm || !key || !hm->data) return;

        int k = dhmhash(key);
        int m = hashmask(k, hm->d);
        DHnode *b = hm->data[m];
        if (!b || !b->key) return;
        if (strcmp(b->key, key) != 0) return;

        free(b->key);
        b->key = NULL;
        b->value = NULL;
}

void
dhmdestroya(DHmap *hm, void (*destra)(void *))
{
        if (!hm || !hm->data) return;

        int n = (hm->d == 0) ? 1 : (1 << hm->d);
        DHnode **uniq = calloc((size_t) n, sizeof(*uniq));
        int u = 0;

        for (int i = 0; i < n; ++i) {
                DHnode *p = hm->data[i];
                if (!p) continue;
                int seen = 0;
                for (int j = 0; j < u; ++j) {
                        if (uniq[j] == p) {
                                seen = 1;
                                break;
                        }
                }
                if (!seen) uniq[u++] = p;
        }

        for (int i = 0; i < u; ++i) {
                DHnode *p = uniq[i];
                if (p->key) {
                        free(p->key);
                        p->key = NULL;
                }
                if (destra && p->value)
                        destra(p->value);
                free(p);
        }

        free(uniq);
        free(hm->data);
        hm->data = NULL;
        hm->d = 0;
}

void
dhmdestroy(DHmap *hm)
{
        dhmdestroya(hm, NULL);
}

void
dhmprint(DHmap hm)
{
        int i;
        if (hm.data == NULL) return;
        printf("------------------------------\n");
        printf("d = %d\n", hm.d);
        int n = (hm.d == 0) ? 1 : (1 << hm.d);
        for (i = 0; i < n; i++) {
                printf("[%2d] --> %s = %p (d = %d)\n",
                       i,
                       hm.data[i] && hm.data[i]->key ? hm.data[i]->key : "(null)",
                       hm.data[i] ? hm.data[i]->value : NULL,
                       hm.data[i] ? hm.data[i]->d : -1);
        }
        printf("------------------------------\n");
}
