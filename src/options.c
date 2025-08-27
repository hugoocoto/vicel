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

Win_opts win_opts;
Col_opts col_opts;

#define toml_cpyfree(a, b)          \
        {                           \
                __auto_type _b = b; \
                free(a);            \
                (a) = strdup(_b);   \
                free(_b);           \
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
        char *new = calloc(strlen(*col) + strlen(T_CSI "m") + 1, 1);
        *new = 0;
        strcat(new, T_CSI);
        strcat(new, *col);
        strcat(new, "m");
        free(*col);
        *col = new;
        return *col;
}

void
get_window_options(toml_table_t *tbl)
{
        report("Getting [window]");
        toml_value_t v;
        if ((v = toml_table_string(tbl, "cell_l_sep")).ok) toml_cpyfree(win_opts.cell_l_sep, v.u.s);
        if ((v = toml_table_string(tbl, "cell_r_sep")).ok) toml_cpyfree(win_opts.cell_r_sep, v.u.s);
        if ((v = toml_table_int(tbl, "num_col_width")).ok) win_opts.num_col_width = v.u.i;
        if ((v = toml_table_int(tbl, "col_width")).ok) win_opts.col_width = v.u.i;
        if ((v = toml_table_int(tbl, "save_time")).ok) win_opts.save_time = v.u.i;
        if ((v = toml_table_bool(tbl, "use_cell_color_for_sep")).ok) win_opts.use_cell_color_for_sep = v.u.b;
        if ((v = toml_table_string(tbl, "ui_celltext_l_sep")).ok) toml_cpyfree(win_opts.ui_celltext_l_sep, v.u.s);
        if ((v = toml_table_string(tbl, "ui_celltext_m_sep")).ok) toml_cpyfree(win_opts.ui_celltext_m_sep, v.u.s);
        if ((v = toml_table_string(tbl, "ui_celltext_r_sep")).ok) toml_cpyfree(win_opts.ui_celltext_r_sep, v.u.s);
        if ((v = toml_table_string(tbl, "status_l_stuff")).ok) toml_cpyfree(win_opts.status_l_stuff, v.u.s);
        if ((v = toml_table_string(tbl, "status_filename")).ok) toml_cpyfree(win_opts.status_filename, v.u.s);
        if ((v = toml_table_string(tbl, "status_r_end")).ok) toml_cpyfree(win_opts.status_r_end, v.u.s);
        if ((v = toml_table_string(tbl, "ui_status_bottom_end")).ok) toml_cpyfree(win_opts.ui_status_bottom_end, v.u.s);
}

void
get_color_options(toml_table_t *tbl)
{
        report("Getting [color]");
        toml_value_t v;
        if ((v = toml_table_string(tbl, "ui")).ok) toml_cpyfree(col_opts.ui, col_format(&v.u.s));
        if ((v = toml_table_string(tbl, "cell_over")).ok) toml_cpyfree(col_opts.cell_over, col_format(&v.u.s));
        if ((v = toml_table_string(tbl, "cell_selected")).ok) toml_cpyfree(col_opts.cell_selected, col_format(&v.u.s));
        if ((v = toml_table_string(tbl, "ln_over")).ok) toml_cpyfree(col_opts.ln_over, col_format(&v.u.s));
        if ((v = toml_table_string(tbl, "ln")).ok) toml_cpyfree(col_opts.ln, col_format(&v.u.s));
        if ((v = toml_table_string(tbl, "sheet_ui")).ok) toml_cpyfree(col_opts.sheet_ui, col_format(&v.u.s));
        if ((v = toml_table_string(tbl, "sheet_ui_over")).ok) toml_cpyfree(col_opts.sheet_ui_over, col_format(&v.u.s));
        if ((v = toml_table_string(tbl, "sheet_ui_selected")).ok) toml_cpyfree(col_opts.sheet_ui_selected, col_format(&v.u.s));
        if ((v = toml_table_string(tbl, "ui_cell_text")).ok) toml_cpyfree(col_opts.ui_cell_text, col_format(&v.u.s));
        if ((v = toml_table_string(tbl, "ui_report")).ok) toml_cpyfree(col_opts.ui_report, col_format(&v.u.s));
        if ((v = toml_table_string(tbl, "insert")).ok) toml_cpyfree(col_opts.insert, col_format(&v.u.s));
}

void
get_options(toml_table_t *tbl)
{
        toml_table_t *stbl;

        stbl = toml_table_table(tbl, "window");
        if (stbl) get_window_options(stbl);

        stbl = toml_table_table(tbl, "color");
        if (stbl) get_color_options(stbl);
}

void
parse_options(char *content)
{
        char errbuf[1280];
        toml_table_t *tbl = toml_parse(content, errbuf, sizeof errbuf);
        if (!tbl) {
                report("Toml error: %s", errbuf);
                return;
        }
        get_options(tbl);
        toml_free(tbl);
}

void
parse_options_file(FILE *f)
{
        if (f == NULL) return;

        char buf[1024];
        size_t n;

        while ((n = fread(buf, 1, sizeof buf - 1, f))) {
                buf[n] = 0;
                parse_options(buf);
        }
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
        parse_options_file(fopen(pjoin(path, home, "vicel.toml"), "r"));
        parse_options_file(fopen(pjoin(path, home, ".config/vicel.toml"), "r"));
        parse_options_file(fopen(pjoin(path, home, ".config/vicel/vicel.toml"), "r"));
        parse_options_file(fopen(pjoin(path, "vicel.toml"), "r"));
}
