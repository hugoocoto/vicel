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
#include <unistd.h>

Hmap colors;

static void
del_default_colors()
{
        hmdestroy(&colors);
}

static __attribute__((constructor)) void
set_default_colors()
{
        hmnew(&colors, 32); // random size
        atexit(del_default_colors);

        hmadd(&colors, "ui", C(C_BG_DEFAULT, C_FG_BLACK));
        hmadd(&colors, "cell", C(C_BG_DEFAULT, C_FG_DEFAULT));
        hmadd(&colors, "cell_over", C(C_BG_DEFAULT, C_FG_DEFAULT, C_REVERSE, C_BOLD));
        hmadd(&colors, "cell_selected", C(C_BG_DEFAULT, C_FG_GREEN));
        hmadd(&colors, "ln_over", C(C_BG_DEFAULT, C_FG_GREEN, C_REVERSE, C_BOLD));
        hmadd(&colors, "ln", C(C_BG_DEFAULT, C_FG_GREEN));
        hmadd(&colors, "sheet_ui", C(C_BG_DEFAULT, C_FG_DEFAULT));
        hmadd(&colors, "sheet_ui_over", C(C_BG_MAGENTA, C_FG_DEFAULT, C_REVERSE, C_BOLD));
        hmadd(&colors, "sheet_ui_selected", C(C_BG_MAGENTA, C_FG_GREEN));
        hmadd(&colors, "ui_cell_text", C(C_BG_DEFAULT, C_FG_DEFAULT, C_BOLD));

        // #define pc(s) printf("%s%s%s\n", get_color(s), s, get_color(NULL))
        //         pc("ui");
        //         pc("cell");
        //         pc("cell_over");
        //         pc("cell_selected");
        //         pc("ln_over");
        //         pc("ln");
        //         pc("sheet_ui");
        //         pc("sheet_ui_over");
        //         pc("sheet_ui_selected");
        //         pc("ui_cell_text");
        //         exit(0);
}

char *
get_color(char *key)
{
        char *col;
        if (key == NULL) return C(C_NORMAL);
        hmget(colors, key, (void *) &col);
        if (col == NULL) {
                report("Invalid color key: %s", key);
                return C(C_NORMAL);
        }
        return col;
}

void
apply_color(char *key)
{
        printf("%s", get_color(NULL));
        printf("%s", get_color(key));
}
