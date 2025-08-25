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

typedef struct win_opts {
        int num_col_width;
        int col_width;
        int row_width;
        bool use_cell_color_for_sep;
        char *cell_l_sep;
        char *cell_r_sep;
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
        char *insert;
}Col_opts; 

extern Win_opts win_opts;
extern Col_opts col_opts;

void parse_options_default_file();
void parse_options_file(FILE *f);
void free_opts();

#endif //! OPTS_H_
