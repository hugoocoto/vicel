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

/*---*/
#include "options.h"
#include "common.h"
#include "debug.h"
#include "escape_code.h"
#include "keyboard.h"
#define INCLUDE_CONF_IMPLEMENTATION
#include "conf.h"
/*---*/

#include <stddef.h>
#include <stdio.h>
#include <string.h>

Win_opts win_opts;
Col_opts col_opts;
Mappings_opts user_mappings;
static Conf conf;

#define GET_STR(_name_, _var_)                                            \
        do {                                                              \
                const char *value;                                        \
                if (Conf_get_str(conf, &value, (_name_)) == CONF_OK) {    \
                        free((_var_));                                    \
                        (_var_) = strdup(value);                           \
                        report("CONFIG  %s = \"%s\"", (_name_), (_var_)); \
                }                                                         \
        } while (0)

#define GET_COL(_name_, _var_)                                                \
        do {                                                                  \
                const char *value;                                            \
                if (Conf_get_str(conf, &value, (_name_)) == CONF_OK) {        \
                        free((_var_));                                        \
                        (_var_) = col_format(value);                          \
                        report("CONFIG  %s = \"%s\"", (_name_), (_var_));     \
                }                                                             \
        } while (0)

#define GET_INT(_name_, _var_)                                         \
        do {                                                           \
                int value;                                              \
                if (Conf_get_int(conf, &value, (_name_)) == CONF_OK) {  \
                        (_var_) = value;                                \
                        report("CONFIG  %s = %d", (_name_), (_var_));  \
                }                                                      \
        } while (0)

#define GET_BOOL(_name_, _var_)                                        \
        do {                                                           \
                int value;                                             \
                if (Conf_get_bool(conf, &value, (_name_)) == CONF_OK) { \
                        (_var_) = value;                               \
                        report("CONFIG  %s = %d", (_name_), (_var_)); \
                }                                                      \
        } while (0)


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

        free(user_mappings.func_should_quit);
        free(user_mappings.func_render);
        free(user_mappings.func_a_select_toggle_cell);
        free(user_mappings.func_get_set_cell_input);
        free(user_mappings.func_a_set_cell_type_numeric);
        free(user_mappings.func_a_set_cell_type_text);
        free(user_mappings.func_a_delete);
        free(user_mappings.func_a_set_cell_type_formula);
        free(user_mappings.func_a_copy_moving_down);
        free(user_mappings.func_a_copy_moving_up);
        free(user_mappings.func_a_copy_moving_left);
        free(user_mappings.func_a_copy_moving_right);
        free(user_mappings.func_a_insert_moving_down);
        free(user_mappings.func_a_insert_moving_up);
        free(user_mappings.func_a_insert_moving_left);
        free(user_mappings.func_a_insert_moving_right);
        free(user_mappings.func_a_goto_top_left);
        free(user_mappings.func_a_goto_max_left);
        free(user_mappings.func_a_goto_max_right);
        free(user_mappings.func_a_goto_top);
        free(user_mappings.func_a_goto_bottom);
        free(user_mappings.func_a_yank);
        free(user_mappings.func_a_paste);
        free(user_mappings.func_a_save);
        free(user_mappings.func_a_add_col);
        free(user_mappings.func_a_add_row);
        free(user_mappings.func_a_insert_zero_col);
        free(user_mappings.func_a_insert_zero_row);
        free(user_mappings.func_a_insert_before_row);
        free(user_mappings.func_a_insert_before_col);
        free(user_mappings.func_a_insert_after_row);
        free(user_mappings.func_a_insert_after_col);
        free(user_mappings.func_a_delete_up_row);
        free(user_mappings.func_a_delete_left_col);
        free(user_mappings.func_a_delete_down_row);
        free(user_mappings.func_a_delete_right_col);
        free(user_mappings.func_a_col_increase);
        free(user_mappings.func_a_col_decrease);
        free(user_mappings.func_a_scroll_up);
        free(user_mappings.func_a_scroll_down);
        free(user_mappings.func_a_scroll_left);
        free(user_mappings.func_a_scroll_right);
}

char *
col_format(const char *col)
{
        if (!strncmp(col, T_CSI, strlen(T_CSI))) return strdup(col);
        char *new = calloc(strlen(col) + strlen(T_CSI "m") + 1, 1);
        sprintf(new, T_CSI "%sm", col);
        return new;
}

static void
get_window_options()
{
        GET_STR("cell_l_sep", win_opts.cell_l_sep);
        GET_STR("cell_l_sep", win_opts.cell_l_sep);
        GET_STR("cell_r_sep", win_opts.cell_r_sep);
        GET_INT("num_col_width", win_opts.num_col_width);
        GET_INT("col_width", win_opts.col_width);
        GET_INT("save_time", win_opts.save_time);
        GET_BOOL("use_cell_color_for_sep", win_opts.use_cell_color_for_sep);
        GET_BOOL("use_mouse", win_opts.use_mouse);
        GET_BOOL("natural_scroll", win_opts.natural_scroll);
        GET_STR("ui_celltext_l_sep", win_opts.ui_celltext_l_sep);
        GET_STR("ui_celltext_m_sep", win_opts.ui_celltext_m_sep);
        GET_STR("ui_celltext_r_sep", win_opts.ui_celltext_r_sep);
        GET_STR("status_l_stuff", win_opts.status_l_stuff);
        GET_STR("status_filename", win_opts.status_filename);
        GET_STR("status_r_end", win_opts.status_r_end);
        GET_STR("ui_status_bottom_end", win_opts.ui_status_bottom_end);
}

static void
get_color_options()
{
        GET_COL("ui", col_opts.ui);
        GET_COL("cell_over", col_opts.cell_over);
        GET_COL("cell_selected", col_opts.cell_selected);
        GET_COL("ln_over", col_opts.ln_over);
        GET_COL("ln", col_opts.ln);
        GET_COL("sheet_ui", col_opts.sheet_ui);
        GET_COL("sheet_ui_over", col_opts.sheet_ui_over);
        GET_COL("sheet_ui_selected", col_opts.sheet_ui_selected);
        GET_COL("ui_cell_text", col_opts.ui_cell_text);
        GET_COL("ui_report", col_opts.ui_report);
        GET_COL("insert", col_opts.insert);
}

void
get_func_mappings_opts()
{
        GET_STR("func_should_quit", user_mappings.func_should_quit);
        GET_STR("func_render", user_mappings.func_render);
        GET_STR("func_a_select_toggle_cell", user_mappings.func_a_select_toggle_cell);
        GET_STR("func_get_set_cell_input", user_mappings.func_get_set_cell_input);
        GET_STR("func_a_set_cell_type_numeric", user_mappings.func_a_set_cell_type_numeric);
        GET_STR("func_a_set_cell_type_text", user_mappings.func_a_set_cell_type_text);
        GET_STR("func_a_delete", user_mappings.func_a_delete);
        GET_STR("func_a_set_cell_type_formula", user_mappings.func_a_set_cell_type_formula);
        GET_STR("func_a_copy_moving_down", user_mappings.func_a_copy_moving_down);
        GET_STR("func_a_copy_moving_up", user_mappings.func_a_copy_moving_up);
        GET_STR("func_a_copy_moving_left", user_mappings.func_a_copy_moving_left);
        GET_STR("func_a_copy_moving_right", user_mappings.func_a_copy_moving_right);
        GET_STR("func_a_insert_moving_down", user_mappings.func_a_insert_moving_down);
        GET_STR("func_a_insert_moving_up", user_mappings.func_a_insert_moving_up);
        GET_STR("func_a_insert_moving_left", user_mappings.func_a_insert_moving_left);
        GET_STR("func_a_insert_moving_right", user_mappings.func_a_insert_moving_right);
        GET_STR("func_a_goto_top_left", user_mappings.func_a_goto_top_left);
        GET_STR("func_a_goto_max_left", user_mappings.func_a_goto_max_left);
        GET_STR("func_a_goto_max_right", user_mappings.func_a_goto_max_right);
        GET_STR("func_a_goto_top", user_mappings.func_a_goto_top);
        GET_STR("func_a_goto_bottom", user_mappings.func_a_goto_bottom);
        GET_STR("func_a_yank", user_mappings.func_a_yank);
        GET_STR("func_a_paste", user_mappings.func_a_paste);
        GET_STR("func_a_save", user_mappings.func_a_save);
        GET_STR("func_a_add_col", user_mappings.func_a_add_col);
        GET_STR("func_a_add_row", user_mappings.func_a_add_row);
        GET_STR("func_a_insert_zero_col", user_mappings.func_a_insert_zero_col);
        GET_STR("func_a_insert_zero_row", user_mappings.func_a_insert_zero_row);
        GET_STR("func_a_insert_before_row", user_mappings.func_a_insert_before_row);
        GET_STR("func_a_insert_before_col", user_mappings.func_a_insert_before_col);
        GET_STR("func_a_insert_after_row", user_mappings.func_a_insert_after_row);
        GET_STR("func_a_insert_after_col", user_mappings.func_a_insert_after_col);
        GET_STR("func_a_delete_up_row", user_mappings.func_a_delete_up_row);
        GET_STR("func_a_delete_left_col", user_mappings.func_a_delete_left_col);
        GET_STR("func_a_delete_down_row", user_mappings.func_a_delete_down_row);
        GET_STR("func_a_delete_right_col", user_mappings.func_a_delete_right_col);
        GET_STR("func_a_col_increase", user_mappings.func_a_col_increase);
        GET_STR("func_a_col_decrease", user_mappings.func_a_col_decrease);
        GET_STR("func_a_scroll_up", user_mappings.func_a_scroll_up);
        GET_STR("func_a_scroll_down", user_mappings.func_a_scroll_down);
        GET_STR("func_a_scroll_left", user_mappings.func_a_scroll_left);
        GET_STR("func_a_scroll_right", user_mappings.func_a_scroll_right);
}


static void
options_get()
{
        get_color_options();
        get_window_options();
        get_func_mappings_opts();
}

void
parse_options_init()
{
        conf = NULL;
}

void
parse_options_destroy()
{
        if (conf) Conf_close(conf);
        free_opts();
}

void
parse_options_file(char *filename)
{
        FILE *f = fopen(filename, "r");
        if (!f) return;
        fclose(f);
        report("Loading config file %s", filename);
        if (Conf_open(&conf, filename) != CONF_OK) return;
        options_get();
        Conf_close(conf);
        conf = NULL;
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
        parse_options_file(pjoin(path, home, ".vicel.lua"));
        parse_options_file(pjoin(path, home, ".config/vicel.lua"));
        parse_options_file(pjoin(path, home, ".config/vicel/vicel.lua"));
        parse_options_file(pjoin(path, "config/vicel.lua"));
        parse_options_file(pjoin(path, "vicel.lua"));
}

void
__options_init(OptOpts opts)
{
        (void) opts;

        col_opts.ui = col_format("49;30");
        col_opts.ui_cell_text = col_format("49;39;1");
        col_opts.ui_report = col_format("41;39");
        col_opts.cell = col_format("49;39");
        col_opts.cell_over = col_format("49;39;7;1");
        col_opts.cell_selected = col_format("49;32");
        col_opts.ln_over = col_format("49;32;7;1");
        col_opts.ln = col_format("49;32");
        col_opts.sheet_ui = col_format("49;39");
        col_opts.sheet_ui_over = col_format("45;39;7;1");
        col_opts.sheet_ui_selected = col_format("45;32");
        col_opts.insert = col_format("49;39");
        win_opts.num_col_width = 5;
        win_opts.col_width = 14;
        win_opts.row_width = 1;
        win_opts.use_cell_color_for_sep = true;
        win_opts.cell_l_sep = strdup(" ");
        win_opts.cell_r_sep = strdup(" ");
        win_opts.save_time = 10;
        win_opts.status_l_stuff = strdup("");
        win_opts.status_filename = strdup(" ./");
        win_opts.status_r_end = strdup("");
        win_opts.ui_celltext_l_sep = strdup(">> '");
        win_opts.ui_celltext_m_sep = strdup("' as ");
        win_opts.ui_celltext_r_sep = strdup(" ");
        win_opts.ui_status_bottom_end = strdup("github: hugoocoto/vicel");
        win_opts.use_mouse = true;
        win_opts.natural_scroll = true;

        user_mappings.func_should_quit = strdup("q");
        user_mappings.func_render = strdup("r");
        user_mappings.func_a_move_cursor_down = strdup("j");
        user_mappings.func_a_move_cursor_up = strdup("k");
        user_mappings.func_a_move_cursor_left = strdup("h");
        user_mappings.func_a_move_cursor_right = strdup("l");
        user_mappings.func_a_select_toggle_cell = strdup("v");
        user_mappings.func_get_set_cell_input = strdup("i");
        user_mappings.func_a_set_cell_type_numeric = strdup("sd");
        user_mappings.func_a_set_cell_type_text = strdup("st");
        user_mappings.func_a_delete = strdup("d");
        user_mappings.func_a_set_cell_type_formula = strdup("sf");
        user_mappings.func_a_copy_moving_down = strdup("J");
        user_mappings.func_a_copy_moving_up = strdup("K");
        user_mappings.func_a_copy_moving_left = strdup("H");
        user_mappings.func_a_copy_moving_right = strdup("L");
        user_mappings.func_a_insert_moving_down = strdup("gij");
        user_mappings.func_a_insert_moving_up = strdup("gik");
        user_mappings.func_a_insert_moving_left = strdup("gih");
        user_mappings.func_a_insert_moving_right = strdup("gil");
        user_mappings.func_a_goto_top_left = strdup("g0");
        user_mappings.func_a_goto_max_left = strdup("^");
        user_mappings.func_a_goto_max_right = strdup("$");
        user_mappings.func_a_goto_top = strdup("gg");
        user_mappings.func_a_goto_bottom = strdup("G");
        user_mappings.func_a_yank = strdup("y");
        user_mappings.func_a_paste = strdup("p");
        user_mappings.func_a_save = strdup("w");
        user_mappings.func_a_add_col = strdup("gL");
        user_mappings.func_a_add_row = strdup("gJ");
        user_mappings.func_a_insert_zero_col = strdup("gL");
        user_mappings.func_a_insert_zero_row = strdup("gJ");
        user_mappings.func_a_insert_before_row = strdup("gk");
        user_mappings.func_a_insert_before_col = strdup("gh");
        user_mappings.func_a_insert_after_row = strdup("gj");
        user_mappings.func_a_insert_after_col = strdup("gl");
        user_mappings.func_a_delete_up_row = strdup("gdk");
        user_mappings.func_a_delete_left_col = strdup("gdh");
        user_mappings.func_a_delete_down_row = strdup("gdj");
        user_mappings.func_a_delete_right_col = strdup("gdl");
        user_mappings.func_a_col_increase = strdup("+");
        user_mappings.func_a_col_decrease = strdup("-");
        user_mappings.func_a_scroll_up = strdup("ej");
        user_mappings.func_a_scroll_down = strdup("ek");
        user_mappings.func_a_scroll_left = strdup("el");
        user_mappings.func_a_scroll_right = strdup("eh");
}

void
parse_options_dump()
{
        printf("No yet implemented!");
}
