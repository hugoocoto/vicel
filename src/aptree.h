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
 * For questions or support, contact: me@hugocoto.com
 */

#ifndef APTREE_H
#define APTREE_H

#include "action.h"
#include "common.h"
#include "dhm.h"

typedef struct {
        DHmap *node;
        Action action;
        int descents;
} *APTree;

APTree ap_init();
void ap_add(APTree t, char *prefix, int len, Action action);
APTree ap_get(APTree t, char *prefix, int len);
Action ap_get_last(APTree t, char *prefix, int len);
Action ap_get_unique(APTree t, char *prefix, int len);
bool ap_has_descents(APTree t, char *prefix, int len);
void ap_remove(APTree t, char *prefix, int len);
void ap_destroy(APTree t);


#endif //! APTREE_H
