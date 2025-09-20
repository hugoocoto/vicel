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
#include "vispel/embedded.h"

Win_opts win_opts;
Col_opts col_opts;

#define copy_free(a, b)  \
        {                \
                free(a); \
                (a) = b; \
        }

#define copy(a, b)               \
        {                        \
                free(a);         \
                (a) = strdup(b); \
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
col_format(char *col)
{
        if (!strncmp(col, T_CSI, strlen(T_CSI))) return strdup(col);
        char *new = calloc(strlen(col) + strlen(T_CSI "m") + 1, 1);
        sprintf(new, T_CSI "%sm", col);
        return new;
}

void
get_window_options()
{
        char *vs;
        int vi;
        if (vspl_get_str("cell_l_sep", &vs)) copy(win_opts.cell_l_sep, vs);
        if (vspl_get_str("cell_l_sep", &vs)) copy(win_opts.cell_l_sep, vs);
        if (vspl_get_str("cell_r_sep", &vs)) copy(win_opts.cell_r_sep, vs);
        if (vspl_get_int("num_col_width", &vi)) win_opts.num_col_width = vi;
        if (vspl_get_int("col_width", &vi)) win_opts.col_width = vi;
        if (vspl_get_int("save_time", &vi)) win_opts.save_time = vi;
        if (vspl_get_int("use_cell_color_for_sep", &vi)) win_opts.use_cell_color_for_sep = vi;
        if (vspl_get_int("use_mouse", &vi)) win_opts.use_mouse = vi;
        if (vspl_get_int("natural_scroll", &vi)) win_opts.natural_scroll = vi;
        if (vspl_get_str("ui_celltext_l_sep", &vs)) copy(win_opts.ui_celltext_l_sep, vs);
        if (vspl_get_str("ui_celltext_m_sep", &vs)) copy(win_opts.ui_celltext_m_sep, vs);
        if (vspl_get_str("ui_celltext_r_sep", &vs)) copy(win_opts.ui_celltext_r_sep, vs);
        if (vspl_get_str("status_l_stuff", &vs)) copy(win_opts.status_l_stuff, vs);
        if (vspl_get_str("status_filename", &vs)) copy(win_opts.status_filename, vs);
        if (vspl_get_str("status_r_end", &vs)) copy(win_opts.status_r_end, vs);
        if (vspl_get_str("ui_status_bottom_end", &vs)) copy(win_opts.ui_status_bottom_end, vs);
}

void
get_color_options()
{
        char *vs;
        if (vspl_get_str("ui", &vs)) copy_free(col_opts.ui, col_format(vs));
        if (vspl_get_str("cell_over", &vs)) copy_free(col_opts.cell_over, col_format(vs));
        if (vspl_get_str("cell_selected", &vs)) copy_free(col_opts.cell_selected, col_format(vs));
        if (vspl_get_str("ln_over", &vs)) copy_free(col_opts.ln_over, col_format(vs));
        if (vspl_get_str("ln", &vs)) copy_free(col_opts.ln, col_format(vs));
        if (vspl_get_str("sheet_ui", &vs)) copy_free(col_opts.sheet_ui, col_format(vs));
        if (vspl_get_str("sheet_ui_over", &vs)) copy_free(col_opts.sheet_ui_over, col_format(vs));
        if (vspl_get_str("sheet_ui_selected", &vs)) copy_free(col_opts.sheet_ui_selected, col_format(vs));
        if (vspl_get_str("ui_cell_text", &vs)) copy_free(col_opts.ui_cell_text, col_format(vs));
        if (vspl_get_str("ui_report", &vs)) copy_free(col_opts.ui_report, col_format(vs));
        if (vspl_get_str("insert", &vs)) copy_free(col_opts.insert, col_format(vs));
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

char *
path_join(char *base, ...)
{
        /* unsafe */
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
        char *home = getenv("HOME") ?: "";
        report("home=%s", home);
        parse_options_file(fopen(pjoin(path, home, "vicel.vspl"), "r"));
        parse_options_file(fopen(pjoin(path, home, ".config/vicel.vspl"), "r"));
        parse_options_file(fopen(pjoin(path, home, ".config/vicel/vicel.vspl"), "r"));
        parse_options_file(fopen(pjoin(path, "vicel.vspl"), "r"));
}

void
__options_init(OptOpts opts)
{
        vspl_addstr("filename", (opts.filename && *opts.filename) ? opts.filename : "");
        vspl_addstr("extension", (opts.fileextension && *opts.fileextension) ? opts.fileextension : "");

        vspl_addvar("ui", (col_opts.ui = col_format("49;30"), ("49;30")));
        vspl_addvar("ui_cell_text", (col_opts.ui_cell_text = col_format("49;39;1"), ("49;39;1")));
        vspl_addvar("ui_report", (col_opts.ui_report = col_format("41;39"), ("41;39")));
        vspl_addvar("cell", (col_opts.cell = col_format("49;39"), ("49;39")));
        vspl_addvar("cell_over", (col_opts.cell_over = col_format("49;39;7;1"), ("49;39;7;1")));
        vspl_addvar("cell_selected", (col_opts.cell_selected = col_format("49;32"), ("49;32")));
        vspl_addvar("ln_over", (col_opts.ln_over = col_format("49;32;7;1"), ("49;32;7;1")));
        vspl_addvar("ln", (col_opts.ln = col_format("49;32"), ("49;32")));
        vspl_addvar("sheet_ui", (col_opts.sheet_ui = col_format("49;39"), ("49;39")));
        vspl_addvar("sheet_ui_over", (col_opts.sheet_ui_over = col_format("45;39;7;1"), ("45;39;7;1")));
        vspl_addvar("sheet_ui_selected", (col_opts.sheet_ui_selected = col_format("45;32"), ("45;32")));
        vspl_addvar("insert", (col_opts.insert = col_format("49;39"), ("49;39")));
        vspl_addvar("num_col_width", (win_opts.num_col_width = 5));
        vspl_addvar("col_width", (win_opts.col_width = 14));
        vspl_addvar("row_width", (win_opts.row_width = 1));
        vspl_addvar("use_cell_color_for_sep", (win_opts.use_cell_color_for_sep = true));
        vspl_addvar("cell_l_sep", (win_opts.cell_l_sep = strdup(" ")));
        vspl_addvar("cell_r_sep", (win_opts.cell_r_sep = strdup(" ")));
        vspl_addvar("save_time", (win_opts.save_time = 10));
        vspl_addvar("status_l_stuff", (win_opts.status_l_stuff = strdup("")));
        vspl_addvar("status_filename", (win_opts.status_filename = strdup(" ./")));
        vspl_addvar("status_r_end", (win_opts.status_r_end = strdup("")));
        vspl_addvar("ui_celltext_l_sep", (win_opts.ui_celltext_l_sep = strdup(">> '")));
        vspl_addvar("ui_celltext_m_sep", (win_opts.ui_celltext_m_sep = strdup("' as ")));
        vspl_addvar("ui_celltext_r_sep", (win_opts.ui_celltext_r_sep = strdup(" ")));
        vspl_addvar("ui_status_bottom_end", (win_opts.ui_status_bottom_end = strdup("github: hugoocoto/vicel")));
        vspl_addvar("use_mouse", (win_opts.use_mouse = true));
        vspl_addvar("natural_scroll", (win_opts.natural_scroll = true));
}

void
vspl_env_start()
{
        vspl_start();
}

void
vspl_env_end()
{
        vspl_end();
}
