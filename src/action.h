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

#ifndef ACTION_H
#define ACTION_H

#include <stdlib.h>
typedef struct Action {
        void (*action)(void);
} Action;

#define NoAction ((struct Action) { \
.action = NULL,                     \
})

#define action_is_valid(a) ((a).action != NULL)

#define ACTION(f) ((struct Action) { .action = (f) })

#endif // !ACTION_H
