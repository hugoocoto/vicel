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

#include "options.h"
#include "common.h"
#include "debug.h"
#include "escape_code.h"
#include "toml.h"
#include "vispel/embedded.h"
#include <stdio.h>
#include <string.h>

Win_opts win_opts;
Col_opts col_opts;

#define copy_free(a, b)             \
        {                           \
                __auto_type _b = b; \
                free(a);            \
                (a) = strdup(_b);   \
        }


void
free_opts()
{
        free(win_opts.cell_l_sep);
        free(win_opts.cell_r_sep);
        free(win_opts.ui_celltext_l_sep);
        free(win_opts.ui_celltext_m_sep);
        free(win_opts.ui_celltext_r_sep);
        free(win_opts.status_l_stuff);
        free(win_opts.status_filename);
        free(win_opts.status_r_end);
        free(win_opts.ui_status_bottom_end);

        free(col_opts.ui);
        free(col_opts.cell);
        free(col_opts.cell_over);
        free(col_opts.cell_selected);
        free(col_opts.ln_over);
        free(col_opts.ln);
        free(col_opts.sheet_ui);
        free(col_opts.sheet_ui_over);
        free(col_opts.sheet_ui_selected);
        free(col_opts.ui_cell_text);
        free(col_opts.ui_report);
        free(col_opts.insert);
}

char *
col_format(char **col)
{
        if (!strncmp(*col, T_CSI, strlen(T_CSI))) return *col;
        char *new = calloc(strlen(*col) + strlen(T_CSI "m") + 1, 1);
        sprintf(new, T_CSI "%sm", *col);
        report("col format: from %s to %s", *col, new);
        return new;
}

void
get_window_options()
{
        report("Getting [window]");
        char *vs;
        int vi;
        if (vspl_get_str("cell_l_sep", &vs)) copy_free(win_opts.cell_l_sep, vs);
        if (vspl_get_str("cell_l_sep", &vs)) copy_free(win_opts.cell_l_sep, vs);
        if (vspl_get_str("cell_r_sep", &vs)) copy_free(win_opts.cell_r_sep, vs);
        if (vspl_get_int("num_col_width", &vi)) win_opts.num_col_width = vi;
        if (vspl_get_int("col_width", &vi)) win_opts.col_width = vi;
        if (vspl_get_int("save_time", &vi)) win_opts.save_time = vi;
        if (vspl_get_int("use_cell_color_for_sep", &vi)) win_opts.use_cell_color_for_sep = vi;
        if (vspl_get_int("use_mouse", &vi)) win_opts.use_mouse = vi;
        if (vspl_get_int("natural_scroll", &vi)) win_opts.natural_scroll = vi;
        if (vspl_get_str("ui_celltext_l_sep", &vs)) copy_free(win_opts.ui_celltext_l_sep, vs);
        if (vspl_get_str("ui_celltext_m_sep", &vs)) copy_free(win_opts.ui_celltext_m_sep, vs);
        if (vspl_get_str("ui_celltext_r_sep", &vs)) copy_free(win_opts.ui_celltext_r_sep, vs);
        if (vspl_get_str("status_l_stuff", &vs)) copy_free(win_opts.status_l_stuff, vs);
        if (vspl_get_str("status_filename", &vs)) copy_free(win_opts.status_filename, vs);
        if (vspl_get_str("status_r_end", &vs)) copy_free(win_opts.status_r_end, vs);
        if (vspl_get_str("ui_status_bottom_end", &vs)) copy_free(win_opts.ui_status_bottom_end, vs);
}

void
get_color_options()
{
        report("Getting [color]");
        char *vs;
        if (vspl_get_str("ui", &vs)) copy_free(col_opts.ui, col_format(&vs));
        if (vspl_get_str("cell_over", &vs)) copy_free(col_opts.cell_over, col_format(&vs));
        if (vspl_get_str("cell_selected", &vs)) copy_free(col_opts.cell_selected, col_format(&vs));
        if (vspl_get_str("ln_over", &vs)) copy_free(col_opts.ln_over, col_format(&vs));
        if (vspl_get_str("ln", &vs)) copy_free(col_opts.ln, col_format(&vs));
        if (vspl_get_str("sheet_ui", &vs)) copy_free(col_opts.sheet_ui, col_format(&vs));
        if (vspl_get_str("sheet_ui_over", &vs)) copy_free(col_opts.sheet_ui_over, col_format(&vs));
        if (vspl_get_str("sheet_ui_selected", &vs)) copy_free(col_opts.sheet_ui_selected, col_format(&vs));
        if (vspl_get_str("ui_cell_text", &vs)) copy_free(col_opts.ui_cell_text, col_format(&vs));
        if (vspl_get_str("ui_report", &vs)) copy_free(col_opts.ui_report, col_format(&vs));
        if (vspl_get_str("insert", &vs)) copy_free(col_opts.insert, col_format(&vs));
}

void
options_get()
{
        get_color_options();
        get_window_options();
}


void
parse_options_file(FILE *f)
{
        if (vspl_parse(f)) options_get();
}

/* unsafe */
char *
path_join(char *base, ...)
{
        va_list v;
        char *c;
        int n = 0;
        va_start(v, base);
        *base = 0;
        while ((c = va_arg(v, char *))) {
                if (n++) strcat(base, "/");
                strcat(base, c);
        }

        report("New route: %s", base);
        return base;
}

#define pjoin(base, ...) path_join((base), ##__VA_ARGS__, NULL)

void
parse_options_default_file()
{
        char path[128];
        char *home = getenv("HOME");
        report("home=%s", home);
        if (home == NULL) home = "";
        parse_options_file(fopen(pjoin(path, home, "vicel.vspl"), "r"));
        parse_options_file(fopen(pjoin(path, home, ".config/vicel.vspl"), "r"));
        parse_options_file(fopen(pjoin(path, home, ".config/vicel/vicel.vspl"), "r"));
        parse_options_file(fopen(pjoin(path, "vicel.vspl"), "r"));
}

static __attribute__((constructor)) void
init_default_values()
{
        win_opts = (Win_opts) {
                .num_col_width = 5,
                .col_width = 14,
                .row_width = 1,
                .save_time = 0,
                .use_cell_color_for_sep = true,
                .use_mouse = false,
                .natural_scroll = true,
                .cell_l_sep = STRDUP(" "),
                .cell_r_sep = STRDUP(" "),
                .ui_celltext_l_sep = strdup("cell text: "),
                .ui_celltext_m_sep = strdup(" ("),
                .ui_celltext_r_sep = strdup(")"),
                .status_l_stuff = strdup("vicel | "),
                .status_filename = strdup("filename: "),
                .status_r_end = strdup("github: hugoocoto/vicel"),
                .ui_status_bottom_end = strdup("(Report issues to hugo.coto@member.fsf.org)"),
        };

        col_opts = (Col_opts) {
                .ui = Cdup(C_BG_DEFAULT, C_FG_BLACK),
                .cell = Cdup(C_BG_DEFAULT, C_FG_DEFAULT),
                .cell_over = Cdup(C_BG_DEFAULT, C_FG_DEFAULT, C_REVERSE, C_BOLD),
                .cell_selected = Cdup(C_BG_DEFAULT, C_FG_GREEN),
                .ln_over = Cdup(C_BG_DEFAULT, C_FG_GREEN, C_REVERSE, C_BOLD),
                .ln = Cdup(C_BG_DEFAULT, C_FG_GREEN),
                .sheet_ui = Cdup(C_BG_DEFAULT, C_FG_DEFAULT),
                .sheet_ui_over = Cdup(C_BG_MAGENTA, C_FG_DEFAULT, C_REVERSE, C_BOLD),
                .sheet_ui_selected = Cdup(C_BG_MAGENTA, C_FG_GREEN),
                .ui_cell_text = Cdup(C_BG_DEFAULT, C_FG_DEFAULT, C_BOLD),
                .ui_report = Cdup(C_BG_RED, C_FG_DEFAULT),
                .insert = Cdup(C_BG_DEFAULT, C_FG_DEFAULT),
        };
}

void
options_init()
{
        vspl_start();

        vspl_addvar("ui", col_opts.ui);
        vspl_addvar("ui_cell_text", col_opts.ui_cell_text);
        vspl_addvar("ui_report", col_opts.ui_report);
        vspl_addvar("cell", col_opts.cell);
        vspl_addvar("cell_over", col_opts.cell_over);
        vspl_addvar("cell_selected", col_opts.cell_selected);
        vspl_addvar("ln_over", col_opts.ln_over);
        vspl_addvar("ln", col_opts.ln);
        vspl_addvar("sheet_ui", col_opts.sheet_ui);
        vspl_addvar("sheet_ui_over", col_opts.sheet_ui_over);
        vspl_addvar("sheet_ui_selected", col_opts.sheet_ui_selected);
        vspl_addvar("insert", col_opts.insert);
        vspl_addvar("num_col_width", win_opts.num_col_width);
        vspl_addvar("col_width", win_opts.col_width);
        vspl_addvar("row_width", win_opts.row_width);
        vspl_addvar("use_cell_color_for_sep", win_opts.use_cell_color_for_sep);
        vspl_addvar("cell_l_sep", win_opts.cell_l_sep);
        vspl_addvar("cell_r_sep", win_opts.cell_r_sep);
        vspl_addvar("save_time", win_opts.save_time);
        vspl_addvar("status_l_stuff", win_opts.status_l_stuff);
        vspl_addvar("status_filename", win_opts.status_filename);
        vspl_addvar("status_r_end", win_opts.status_r_end);
        vspl_addvar("ui_celltext_l_sep", win_opts.ui_celltext_l_sep);
        vspl_addvar("ui_celltext_m_sep", win_opts.ui_celltext_m_sep);
        vspl_addvar("ui_celltext_r_sep", win_opts.ui_celltext_r_sep);
        vspl_addvar("ui_status_bottom_end", win_opts.ui_status_bottom_end);
        vspl_addvar("use_mouse", win_opts.use_mouse);
        vspl_addvar("natural_scroll", win_opts.natural_scroll);
}

void
options_destroy()
{
        vspl_end();
}
