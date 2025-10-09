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

/*---*/
#include "options.h"
#include "common.h"
#include "debug.h"
#include "escape_code.h"
/*---*/

#include <Python.h>

Win_opts win_opts;
Col_opts col_opts;
PyObject *globals = NULL;

#define GET_STR(_name_, _var_)                                         \
        do {                                                           \
                PyObject *obj;                                         \
                if ((obj = PyDict_GetItemString(globals, (_name_))) && \
                    PyUnicode_Check(obj)) {                            \
                        (_var_) = (char *) PyUnicode_AsUTF8(obj);      \
                }                                                      \
        } while (0)

#define GET_INT(_name_, _var_)                                         \
        do {                                                           \
                PyObject *obj;                                         \
                if ((obj = PyDict_GetItemString(globals, (_name_))) && \
                    PyLong_Check(obj)) {                               \
                        (_var_) = PyLong_AsLong(obj);                  \
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

void
get_color_options()
{
        GET_STR("ui", col_opts.ui);
        GET_STR("cell_over", col_opts.cell_over);
        GET_STR("cell_selected", col_opts.cell_selected);
        GET_STR("ln_over", col_opts.ln_over);
        GET_STR("ln", col_opts.ln);
        GET_STR("sheet_ui", col_opts.sheet_ui);
        GET_STR("sheet_ui_over", col_opts.sheet_ui_over);
        GET_STR("sheet_ui_selected", col_opts.sheet_ui_selected);
        GET_STR("ui_cell_text", col_opts.ui_cell_text);
        GET_STR("ui_report", col_opts.ui_report);
        GET_STR("insert", col_opts.insert);
}

void
options_get()
{
        get_color_options();
        get_window_options();
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
        parse_options_file(pjoin(path, home, "vicel.py"));
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
        PyDict_SetItemString(globals, "ui", PyUnicode_FromString((col_opts.ui = col_format("49;30"), ("49;30"))));
        PyDict_SetItemString(globals, "ui_cell_text", PyUnicode_FromString((col_opts.ui_cell_text = col_format("49;39;1"), ("49;39;1"))));
        PyDict_SetItemString(globals, "ui_report", PyUnicode_FromString((col_opts.ui_report = col_format("41;39"), ("41;39"))));
        PyDict_SetItemString(globals, "cell", PyUnicode_FromString((col_opts.cell = col_format("49;39"), ("49;39"))));
        PyDict_SetItemString(globals, "cell_over", PyUnicode_FromString((col_opts.cell_over = col_format("49;39;7;1"), ("49;39;7;1"))));
        PyDict_SetItemString(globals, "cell_selected", PyUnicode_FromString((col_opts.cell_selected = col_format("49;32"), ("49;32"))));
        PyDict_SetItemString(globals, "ln_over", PyUnicode_FromString((col_opts.ln_over = col_format("49;32;7;1"), ("49;32;7;1"))));
        PyDict_SetItemString(globals, "ln", PyUnicode_FromString((col_opts.ln = col_format("49;32"), ("49;32"))));
        PyDict_SetItemString(globals, "sheet_ui", PyUnicode_FromString((col_opts.sheet_ui = col_format("49;39"), ("49;39"))));
        PyDict_SetItemString(globals, "sheet_ui_over", PyUnicode_FromString((col_opts.sheet_ui_over = col_format("45;39;7;1"), ("45;39;7;1"))));
        PyDict_SetItemString(globals, "sheet_ui_selected", PyUnicode_FromString((col_opts.sheet_ui_selected = col_format("45;32"), ("45;32"))));
        PyDict_SetItemString(globals, "insert", PyUnicode_FromString((col_opts.insert = col_format("49;39"), ("49;39"))));
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
}
