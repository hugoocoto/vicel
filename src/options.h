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

typedef struct {
        char *func_should_quit;
        char *func_render;
        char *func_a_move_cursor_down;
        char *func_a_move_cursor_up;
        char *func_a_move_cursor_left;
        char *func_a_move_cursor_right;
        char *func_a_select_toggle_cell;
        char *func_get_set_cell_input;
        char *func_a_set_cell_type_numeric;
        char *func_a_set_cell_type_text;
        char *func_a_delete;
        char *func_a_set_cell_type_formula;
        char *func_a_copy_moving_down;
        char *func_a_copy_moving_up;
        char *func_a_copy_moving_left;
        char *func_a_copy_moving_right;
        char *func_a_insert_moving_down;
        char *func_a_insert_moving_up;
        char *func_a_insert_moving_left;
        char *func_a_insert_moving_right;
        char *func_a_goto_top_left;
        char *func_a_goto_max_left;
        char *func_a_goto_max_right;
        char *func_a_goto_top;
        char *func_a_goto_bottom;
        char *func_a_yank;
        char *func_a_paste;
        char *func_a_save;
        char *func_a_add_col;
        char *func_a_add_row;
        char *func_a_insert_zero_col;
        char *func_a_insert_zero_row;
        char *func_a_insert_before_row;
        char *func_a_insert_before_col;
        char *func_a_insert_after_row;
        char *func_a_insert_after_col;
        char *func_a_delete_up_row;
        char *func_a_delete_left_col;
        char *func_a_delete_down_row;
        char *func_a_delete_right_col;
        char *func_a_col_increase;
        char *func_a_col_decrease;
        char *func_a_scroll_up;
        char *func_a_scroll_down;
        char *func_a_scroll_left;
        char *func_a_scroll_right;
} Mappings_opts;

#ifndef OPTS_H_
#define OPTS_H_

#include "common.h"

typedef struct {
        char *filename;
        char *fileextension;
        void *none;
} OptOpts;

typedef struct win_opts {
        int num_col_width;
        int col_width;
        int row_width;
        int save_time;
        bool use_cell_color_for_sep;
        char *cell_l_sep;
        char *cell_r_sep;
        char *ui_celltext_l_sep;
        char *ui_celltext_m_sep;
        char *ui_celltext_r_sep;
        char *status_l_stuff;
        char *status_filename;
        char *status_r_end;
        char *ui_status_bottom_end;
        bool use_mouse;
        bool natural_scroll;
} Win_opts;

typedef struct Col_opts {
        char *ui;
        char *cell;
        char *cell_over;
        char *cell_selected;
        char *ln_over;
        char *ln;
        char *sheet_ui;
        char *sheet_ui_over;
        char *sheet_ui_selected;
        char *ui_cell_text;
        char *ui_report;
        char *insert;
} Col_opts;

extern Win_opts win_opts;
extern Col_opts col_opts;
extern Mappings_opts user_mappings;

#define options_init(...) __options_init((OptOpts) { .none = 0, __VA_ARGS__ })

void parse_options_destroy();
void parse_options_init();
void parse_options_default_file();
void parse_options_file(char *filename);
void __options_init(OptOpts);
void parse_options_dump();

#endif //! OPTS_H_
