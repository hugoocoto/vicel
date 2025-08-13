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

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "cellmap.h"

#define MAX_MAPPING_LEN 6

#define KEY_UP "\xC1"
#define KEY_DOWN "\xC2"
#define KEY_RIGHT "\xC3"
#define KEY_LEFT "\xC4"

void start_kbhandler();
void set_cell_text(Cell *c, char *text);
char * get_input_at_cursor();

#endif //! KEYBOARD_H
