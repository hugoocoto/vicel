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

#ifndef MAPPINGS_H
#define MAPPINGS_H

#include "cellmap.h"
Cell *get_cursor_cell();

void a_quit();
void a_add_row();
void a_add_col();
void a_move_cursor_up();
void a_move_cursor_down();
void a_move_cursor_left();
void a_move_cursor_right();
void a_select_toggle_cell();
void a_set_cell_type_numeric();
void a_set_cell_type_text();
void a_set_cell_type_empty();
void a_set_cell_type_formula();
void a_copy_moving_up();
void a_copy_moving_down();
void a_copy_moving_left();
void a_copy_moving_right();
void a_insert_moving_up();
void a_insert_moving_down();
void a_insert_moving_left();
void a_insert_moving_right();
void a_goto_top_left();
void a_goto_max_left();
void a_goto_max_right();
void a_goto_top();
void a_goto_bottom();
void a_yank();
void a_paste();
void a_free_yank_buffer();
void a_save();
void a_delete();
void a_insert_before_row();
void a_insert_before_col();
void a_insert_after_row();
void a_insert_after_col();
void a_insert_zero_row();
void a_insert_zero_col();

#endif //! MAPPINGS_H
