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
/*---*/

#include <Python.h>
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

Win_opts win_opts;
Col_opts col_opts;
Mappings_opts user_mappings;
PyObject *globals = NULL;

#define GET_STR(_name_, _var_)                                            \
        do {                                                              \
                PyObject *obj;                                            \
                if ((obj = PyDict_GetItemString(globals, (_name_))) &&    \
                    PyUnicode_Check(obj)) {                               \
                        free((_var_));                                    \
                        (_var_) = strdup((char *) PyUnicode_AsUTF8(obj)); \
                        report("CONFIG  %s = \"%s\"", (_name_), (_var_)); \
                }                                                         \
        } while (0)

#define GET_COL(_name_, _var_)                                                \
        do {                                                                  \
                PyObject *obj;                                                \
                if ((obj = PyDict_GetItemString(globals, (_name_))) &&        \
                    PyUnicode_Check(obj)) {                                   \
                        free((_var_));                                        \
                        (_var_) = col_format((char *) PyUnicode_AsUTF8(obj)); \
                        report("CONFIG  %s = \"%s\"", (_name_), (_var_));     \
                }                                                             \
        } while (0)

#define GET_INT(_name_, _var_)                                         \
        do {                                                           \
                PyObject *obj;                                         \
                if ((obj = PyDict_GetItemString(globals, (_name_))) && \
                    PyLong_Check(obj)) {                               \
                        (_var_) = PyLong_AsLong(obj);                  \
                        report("CONFIG  %s = %d", (_name_), (_var_));  \
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
col_format(char *col)
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
        GET_INT("use_cell_color_for_sep", win_opts.use_cell_color_for_sep);
        GET_INT("use_mouse", win_opts.use_mouse);
        GET_INT("natural_scroll", win_opts.natural_scroll);
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
        Py_Initialize();
        if (!Py_IsInitialized()) {
                report("Impossible to initiaze python interpreter");
                exit(1);
        }
        globals = PyDict_New();
        if (!globals) {
                report("Impossible to initiaze gobals dict");
                exit(1);
        }
}

void
parse_options_destroy()
{
        Py_DECREF(globals);
        Py_Finalize();
        free_opts();
}

void
parse_options_file(char *filename)
{
        FILE *f = fopen(filename, "r");
        if (!f) return;
        assert(globals);
        report("Loading config file %s", filename);
        PyRun_File(f, filename, Py_file_input, globals, globals);
        options_get();
        fclose(f);
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
        parse_options_file(pjoin(path, home, ".vicel.py"));
        parse_options_file(pjoin(path, home, ".config/vicel.py"));
        parse_options_file(pjoin(path, home, ".config/vicel/vicel.py"));
        parse_options_file(pjoin(path, "config/vicel.py"));
        parse_options_file(pjoin(path, "vicel.py"));
}

void
__options_init(OptOpts opts)
{
        PyDict_SetItemString(globals, "filename", PyUnicode_FromString((opts.filename && *opts.filename) ? opts.filename : ""));
        PyDict_SetItemString(globals, "extension", PyUnicode_FromString((opts.fileextension && *opts.fileextension) ? opts.fileextension : ""));

        // funny chunk of code
        PyDict_SetItemString(globals, "ui", PyUnicode_FromString((col_opts.ui = col_format("49;30"))));
        PyDict_SetItemString(globals, "ui_cell_text", PyUnicode_FromString((col_opts.ui_cell_text = col_format("49;39;1"))));
        PyDict_SetItemString(globals, "ui_report", PyUnicode_FromString((col_opts.ui_report = col_format("41;39"))));
        PyDict_SetItemString(globals, "cell", PyUnicode_FromString((col_opts.cell = col_format("49;39"))));
        PyDict_SetItemString(globals, "cell_over", PyUnicode_FromString((col_opts.cell_over = col_format("49;39;7;1"))));
        PyDict_SetItemString(globals, "cell_selected", PyUnicode_FromString((col_opts.cell_selected = col_format("49;32"))));
        PyDict_SetItemString(globals, "ln_over", PyUnicode_FromString((col_opts.ln_over = col_format("49;32;7;1"))));
        PyDict_SetItemString(globals, "ln", PyUnicode_FromString((col_opts.ln = col_format("49;32"))));
        PyDict_SetItemString(globals, "sheet_ui", PyUnicode_FromString((col_opts.sheet_ui = col_format("49;39"))));
        PyDict_SetItemString(globals, "sheet_ui_over", PyUnicode_FromString((col_opts.sheet_ui_over = col_format("45;39;7;1"))));
        PyDict_SetItemString(globals, "sheet_ui_selected", PyUnicode_FromString((col_opts.sheet_ui_selected = col_format("45;32"))));
        PyDict_SetItemString(globals, "insert", PyUnicode_FromString((col_opts.insert = col_format("49;39"))));
        PyDict_SetItemString(globals, "num_col_width", PyLong_FromLong((win_opts.num_col_width = 5)));
        PyDict_SetItemString(globals, "col_width", PyLong_FromLong((win_opts.col_width = 14)));
        PyDict_SetItemString(globals, "row_width", PyLong_FromLong((win_opts.row_width = 1)));
        PyDict_SetItemString(globals, "use_cell_color_for_sep", PyBool_FromLong((win_opts.use_cell_color_for_sep = true)));
        PyDict_SetItemString(globals, "cell_l_sep", PyUnicode_FromString((win_opts.cell_l_sep = strdup(" "))));
        PyDict_SetItemString(globals, "cell_r_sep", PyUnicode_FromString((win_opts.cell_r_sep = strdup(" "))));
        PyDict_SetItemString(globals, "save_time", PyLong_FromLong((win_opts.save_time = 10)));
        PyDict_SetItemString(globals, "status_l_stuff", PyUnicode_FromString((win_opts.status_l_stuff = strdup(""))));
        PyDict_SetItemString(globals, "status_filename", PyUnicode_FromString((win_opts.status_filename = strdup(" ./"))));
        PyDict_SetItemString(globals, "status_r_end", PyUnicode_FromString((win_opts.status_r_end = strdup(""))));
        PyDict_SetItemString(globals, "ui_celltext_l_sep", PyUnicode_FromString((win_opts.ui_celltext_l_sep = strdup(">> '"))));
        PyDict_SetItemString(globals, "ui_celltext_m_sep", PyUnicode_FromString((win_opts.ui_celltext_m_sep = strdup("' as "))));
        PyDict_SetItemString(globals, "ui_celltext_r_sep", PyUnicode_FromString((win_opts.ui_celltext_r_sep = strdup(" "))));
        PyDict_SetItemString(globals, "ui_status_bottom_end", PyUnicode_FromString((win_opts.ui_status_bottom_end = strdup("github: hugoocoto/vicel"))));
        PyDict_SetItemString(globals, "use_mouse", PyBool_FromLong((win_opts.use_mouse = true)));
        PyDict_SetItemString(globals, "natural_scroll", PyBool_FromLong((win_opts.natural_scroll = true)));

        PyDict_SetItemString(globals, "func_should_quit", PyUnicode_FromString((user_mappings.func_should_quit = strdup("q"))));
        PyDict_SetItemString(globals, "func_render", PyUnicode_FromString((user_mappings.func_render = strdup("r"))));
        PyDict_SetItemString(globals, "func_a_move_cursor_down", PyUnicode_FromString((user_mappings.func_a_move_cursor_down = strdup("j"))));
        PyDict_SetItemString(globals, "func_a_move_cursor_up", PyUnicode_FromString((user_mappings.func_a_move_cursor_up = strdup("k"))));
        PyDict_SetItemString(globals, "func_a_move_cursor_left", PyUnicode_FromString((user_mappings.func_a_move_cursor_left = strdup("h"))));
        PyDict_SetItemString(globals, "func_a_move_cursor_right", PyUnicode_FromString((user_mappings.func_a_move_cursor_right = strdup("l"))));
        PyDict_SetItemString(globals, "func_a_select_toggle_cell", PyUnicode_FromString((user_mappings.func_a_select_toggle_cell = strdup("v"))));
        PyDict_SetItemString(globals, "func_get_set_cell_input", PyUnicode_FromString((user_mappings.func_get_set_cell_input = strdup("i"))));
        PyDict_SetItemString(globals, "func_a_set_cell_type_numeric", PyUnicode_FromString((user_mappings.func_a_set_cell_type_numeric = strdup("sd"))));
        PyDict_SetItemString(globals, "func_a_set_cell_type_text", PyUnicode_FromString((user_mappings.func_a_set_cell_type_text = strdup("st"))));
        PyDict_SetItemString(globals, "func_a_delete", PyUnicode_FromString((user_mappings.func_a_delete = strdup("d"))));
        PyDict_SetItemString(globals, "func_a_set_cell_type_formula", PyUnicode_FromString((user_mappings.func_a_set_cell_type_formula = strdup("sf"))));
        PyDict_SetItemString(globals, "func_a_copy_moving_down", PyUnicode_FromString((user_mappings.func_a_copy_moving_down = strdup("J"))));
        PyDict_SetItemString(globals, "func_a_copy_moving_up", PyUnicode_FromString((user_mappings.func_a_copy_moving_up = strdup("K"))));
        PyDict_SetItemString(globals, "func_a_copy_moving_left", PyUnicode_FromString((user_mappings.func_a_copy_moving_left = strdup("H"))));
        PyDict_SetItemString(globals, "func_a_copy_moving_right", PyUnicode_FromString((user_mappings.func_a_copy_moving_right = strdup("L"))));
        PyDict_SetItemString(globals, "func_a_insert_moving_down", PyUnicode_FromString((user_mappings.func_a_insert_moving_down = strdup("gij"))));
        PyDict_SetItemString(globals, "func_a_insert_moving_up", PyUnicode_FromString((user_mappings.func_a_insert_moving_up = strdup("gik"))));
        PyDict_SetItemString(globals, "func_a_insert_moving_left", PyUnicode_FromString((user_mappings.func_a_insert_moving_left = strdup("gih"))));
        PyDict_SetItemString(globals, "func_a_insert_moving_right", PyUnicode_FromString((user_mappings.func_a_insert_moving_right = strdup("gil"))));
        PyDict_SetItemString(globals, "func_a_goto_top_left", PyUnicode_FromString((user_mappings.func_a_goto_top_left = strdup("g0"))));
        PyDict_SetItemString(globals, "func_a_goto_max_left", PyUnicode_FromString((user_mappings.func_a_goto_max_left = strdup("^"))));
        PyDict_SetItemString(globals, "func_a_goto_max_right", PyUnicode_FromString((user_mappings.func_a_goto_max_right = strdup("$"))));
        PyDict_SetItemString(globals, "func_a_goto_top", PyUnicode_FromString((user_mappings.func_a_goto_top = strdup("gg"))));
        PyDict_SetItemString(globals, "func_a_goto_bottom", PyUnicode_FromString((user_mappings.func_a_goto_bottom = strdup("G"))));
        PyDict_SetItemString(globals, "func_a_yank", PyUnicode_FromString((user_mappings.func_a_yank = strdup("y"))));
        PyDict_SetItemString(globals, "func_a_paste", PyUnicode_FromString((user_mappings.func_a_paste = strdup("p"))));
        PyDict_SetItemString(globals, "func_a_save", PyUnicode_FromString((user_mappings.func_a_save = strdup("w"))));
        PyDict_SetItemString(globals, "func_a_add_col", PyUnicode_FromString((user_mappings.func_a_add_col = strdup("gL"))));
        PyDict_SetItemString(globals, "func_a_add_row", PyUnicode_FromString((user_mappings.func_a_add_row = strdup("gJ"))));
        PyDict_SetItemString(globals, "func_a_insert_zero_col", PyUnicode_FromString((user_mappings.func_a_insert_zero_col = strdup("gL"))));
        PyDict_SetItemString(globals, "func_a_insert_zero_row", PyUnicode_FromString((user_mappings.func_a_insert_zero_row = strdup("gJ"))));
        PyDict_SetItemString(globals, "func_a_insert_before_row", PyUnicode_FromString((user_mappings.func_a_insert_before_row = strdup("gk"))));
        PyDict_SetItemString(globals, "func_a_insert_before_col", PyUnicode_FromString((user_mappings.func_a_insert_before_col = strdup("gh"))));
        PyDict_SetItemString(globals, "func_a_insert_after_row", PyUnicode_FromString((user_mappings.func_a_insert_after_row = strdup("gj"))));
        PyDict_SetItemString(globals, "func_a_insert_after_col", PyUnicode_FromString((user_mappings.func_a_insert_after_col = strdup("gl"))));
        PyDict_SetItemString(globals, "func_a_delete_up_row", PyUnicode_FromString((user_mappings.func_a_delete_up_row = strdup("gdk"))));
        PyDict_SetItemString(globals, "func_a_delete_left_col", PyUnicode_FromString((user_mappings.func_a_delete_left_col = strdup("gdh"))));
        PyDict_SetItemString(globals, "func_a_delete_down_row", PyUnicode_FromString((user_mappings.func_a_delete_down_row = strdup("gdj"))));
        PyDict_SetItemString(globals, "func_a_delete_right_col", PyUnicode_FromString((user_mappings.func_a_delete_right_col = strdup("gdl"))));
        PyDict_SetItemString(globals, "func_a_col_increase", PyUnicode_FromString((user_mappings.func_a_col_increase = strdup("+"))));
        PyDict_SetItemString(globals, "func_a_col_decrease", PyUnicode_FromString((user_mappings.func_a_col_decrease = strdup("-"))));
        PyDict_SetItemString(globals, "func_a_scroll_up", PyUnicode_FromString((user_mappings.func_a_scroll_up = strdup("ej"))));
        PyDict_SetItemString(globals, "func_a_scroll_down", PyUnicode_FromString((user_mappings.func_a_scroll_down = strdup("ek"))));
        PyDict_SetItemString(globals, "func_a_scroll_left", PyUnicode_FromString((user_mappings.func_a_scroll_left = strdup("el"))));
        PyDict_SetItemString(globals, "func_a_scroll_right", PyUnicode_FromString((user_mappings.func_a_scroll_right = strdup("eh"))));
}

void
parse_options_dump()
{
        if (!Py_IsInitialized()) return;
        if (!globals) return;
        printf("No yet implemented!");
}
