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

#include "color.h"
#include "common.h"
#include "debug.h"
#include "escape_code.h"
#include "hm.h"
#include "options.h"

Hmap colors;

static void
free_value(Hnode *n)
{
        if (n == NULL) return;
        if (n->value) free(n->value);
}

static void
del_default_colors()
{
        hm_set_ondestroy(free_value);
        hmdestroy(&colors);
}

void
set_default_colors()
{
        hmnew(&colors, 32); // random size
        atexit(del_default_colors);

        hmadd(&colors, "ui", strdup(col_opts.ui));
        hmadd(&colors, "cell", strdup(col_opts.cell));
        hmadd(&colors, "cell_over", strdup(col_opts.cell_over));
        hmadd(&colors, "cell_selected", strdup(col_opts.cell_selected));
        hmadd(&colors, "ln_over", strdup(col_opts.ln_over));
        hmadd(&colors, "ln", strdup(col_opts.ln));
        hmadd(&colors, "sheet_ui", strdup(col_opts.sheet_ui));
        hmadd(&colors, "sheet_ui_over", strdup(col_opts.sheet_ui_over));
        hmadd(&colors, "sheet_ui_selected", strdup(col_opts.sheet_ui_selected));
        hmadd(&colors, "ui_cell_text", strdup(col_opts.ui_cell_text));
        hmadd(&colors, "ui_report", strdup(col_opts.ui_report));
        hmadd(&colors, "insert", strdup(col_opts.insert));
}

char *
get_color(char *key)
{
        char *col;
        if (key == NULL) return C(C_NORMAL);
        hmget(colors, key, (void *) &col);
        if (col == NULL) {
                report("Invalid color key: %s", key);
                return NULL;
        }
        return col;
}

void
add_color(char *c)
{
        char buf[128];
        snprintf(buf, sizeof buf, T_CSI "%sm", c);
        hmadd(&colors, buf, strdup(buf));
        hmadd(&colors, c, strdup(buf));
}

void
apply_color(char *key)
{
        printf("%s", get_color(NULL));
        printf("%s", get_color(key) ?: get_color(NULL));
}
