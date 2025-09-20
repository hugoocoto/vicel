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

#define options_init(...) __options_init((OptOpts) { .none = 0, __VA_ARGS__ })

void parse_options_default_file();
void parse_options_file(FILE *f);
void free_opts();
void __options_init(OptOpts);
void vspl_env_end();
void vspl_env_start();

#endif //! OPTS_H_
