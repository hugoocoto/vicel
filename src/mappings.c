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

#include "mappings.h"
#include "cellmap.h"
#include "common.h"
#include "escape_code.h"
#include "keyboard.h"
#include "saving.h"
#include "window.h"

inline Cell *
get_cursor_cell()
{
        return cm_get_cell_ptr(active_ctx.body, active_ctx.cursor_pos_c, active_ctx.cursor_pos_r);
}

void
a_add_row()
{
        cm_add_row(active_ctx.body);
}
void
a_add_col()
{
        cm_add_col(active_ctx.body);
}

void
a_insert_zero_row()
{
        cm_insert_row(active_ctx.body, 0);
}

void
a_insert_zero_col()
{
        cm_insert_col(active_ctx.body, 0);
}


void
a_insert_before_row()
{
        cm_insert_row(active_ctx.body, active_ctx.cursor_pos_r);
}

void
a_insert_before_col()
{
        cm_insert_col(active_ctx.body, active_ctx.cursor_pos_c);
}

void
a_insert_after_row()
{
        cm_insert_row(active_ctx.body, active_ctx.cursor_pos_r + 1);
}

void
a_insert_after_col()
{
        cm_insert_col(active_ctx.body, active_ctx.cursor_pos_c + 1);
}


void
a_move_cursor_left()
{
        if (active_ctx.cursor_pos_c == 0) return;
        --active_ctx.cursor_pos_c;
        if (active_ctx.scroll_c > active_ctx.cursor_pos_c) {
                --active_ctx.scroll_c;
        }
}

void
a_move_cursor_right()
{
        if (active_ctx.cursor_pos_c >= active_ctx.body->data->size - 1) return;
        ++active_ctx.cursor_pos_c;
        if (active_ctx.cursor_pos_c >= active_ctx.scroll_c + active_ctx.max_display_c) {
                ++active_ctx.scroll_c;
        }
}

void
a_move_cursor_up()
{
        if (active_ctx.cursor_pos_r == 0) return;
        --active_ctx.cursor_pos_r;
        if (active_ctx.scroll_r > active_ctx.cursor_pos_r) {
                --active_ctx.scroll_r;
        }
}

void
a_move_cursor_down()
{
        if (active_ctx.cursor_pos_r >= active_ctx.body->size - 1) return;
        ++active_ctx.cursor_pos_r;
        if (active_ctx.cursor_pos_r >= active_ctx.scroll_r + active_ctx.max_display_r) {
                ++active_ctx.scroll_r;
        }
}

void
a_select_toggle_cell()
{
        get_cursor_cell()->selected = !get_cursor_cell()->selected;
}

void
a_set_cell_type_numeric()
{
        cm_convert(get_cursor_cell(), TYPE_NUMBER);
}

void
a_set_cell_type_text()
{
        cm_convert(get_cursor_cell(), TYPE_TEXT);
}

void
a_set_cell_type_formula()
{
        cm_convert(get_cursor_cell(), TYPE_FORMULA);
}

void
a_set_cell_type_empty()
{
        cm_convert(get_cursor_cell(), TYPE_EMPTY);
}

void
a_delete()
{
        a_yank();
        cm_convert(get_cursor_cell(), TYPE_EMPTY);
}

void
a_copy_moving_up()
{
        cm_extend(active_ctx.body,
                  active_ctx.cursor_pos_c, active_ctx.cursor_pos_r,
                  active_ctx.cursor_pos_c, active_ctx.cursor_pos_r - 1);
        a_move_cursor_up();
}

void
a_copy_moving_down()
{
        cm_extend(active_ctx.body,
                  active_ctx.cursor_pos_c, active_ctx.cursor_pos_r,
                  active_ctx.cursor_pos_c, active_ctx.cursor_pos_r + 1);
        a_move_cursor_down();
}

void
a_copy_moving_left()
{
        cm_extend(active_ctx.body,
                  active_ctx.cursor_pos_c, active_ctx.cursor_pos_r,
                  active_ctx.cursor_pos_c - 1, active_ctx.cursor_pos_r);
        a_move_cursor_left();
}

void
a_copy_moving_right()
{
        cm_extend(active_ctx.body,
                  active_ctx.cursor_pos_c, active_ctx.cursor_pos_r,
                  active_ctx.cursor_pos_c + 1, active_ctx.cursor_pos_r);
        a_move_cursor_right();
}

void
a_insert_moving_up()
{
        set_cell_text(get_cursor_cell(), get_input_at_cursor());
        a_move_cursor_up();
}

void
a_insert_moving_down()
{
        set_cell_text(get_cursor_cell(), get_input_at_cursor());
        a_move_cursor_down();
}

void
a_insert_moving_left()
{
        set_cell_text(get_cursor_cell(), get_input_at_cursor());
        a_move_cursor_left();
}

void
a_insert_moving_right()
{
        set_cell_text(get_cursor_cell(), get_input_at_cursor());
        a_move_cursor_right();
}

void
a_goto_max_left()
{
        active_ctx.cursor_pos_c = 0;
        active_ctx.scroll_c = 0;
}

void
a_goto_max_right()
{
        active_ctx.cursor_pos_c = active_ctx.body->data[0].size - 1;
        active_ctx.scroll_c = active_ctx.cursor_pos_c - active_ctx.max_display_c + 1;
        if (active_ctx.scroll_c < 0) active_ctx.scroll_c = 0;
}

void
a_goto_top()
{
        active_ctx.cursor_pos_r = 0;
        active_ctx.scroll_r = 0;
}

void
a_goto_bottom()
{
        active_ctx.cursor_pos_r = active_ctx.body->size - 1;
        active_ctx.scroll_r = active_ctx.cursor_pos_r - active_ctx.max_display_r + 1;
        if (active_ctx.scroll_r < 0) active_ctx.scroll_r = 0;
}

void
a_goto_top_left()
{
        a_goto_top();
        a_goto_max_left();
}

char *yank_buffer = NULL;

void
a_yank()
{
        if (!get_cursor_cell()->input_repr) return;
        free(yank_buffer);
        yank_buffer = strdup(get_cursor_cell()->input_repr);
}

void
a_paste()
{
        if (!yank_buffer) return;
        set_cell_text(get_cursor_cell(), strdup(yank_buffer));
}

void
a_free_yank_buffer()
{
        free(yank_buffer);
}


void
a_save()
{
        save(&active_ctx);
}

void
a_delete_left_col()
{
        if (active_ctx.body->data->size == 1) return;
        cm_delete_col(active_ctx.body, active_ctx.cursor_pos_c);
        a_move_cursor_left();
}

void
a_delete_right_col()
{
        bool zero = active_ctx.cursor_pos_c == 0;
        a_delete_left_col();
        if (!zero) a_move_cursor_right();
}

void
a_delete_up_row()
{
        if (active_ctx.body->size == 1) return;
        cm_delete_row(active_ctx.body, active_ctx.cursor_pos_r);
        a_move_cursor_up();
}

void
a_delete_down_row()
{
        bool zero = active_ctx.cursor_pos_r == 0;
        a_delete_up_row();
        if (!zero) a_move_cursor_down();
}

void
a_scroll_up()
{
        if (active_ctx.scroll_r == 0) return;
        --active_ctx.scroll_r;
        if (active_ctx.cursor_pos_r >= active_ctx.max_display_r + active_ctx.scroll_r)
                a_move_cursor_up();
}

void
a_scroll_down()
{
        if (active_ctx.scroll_r + active_ctx.max_display_r >= active_ctx.body->size) return;
        ++active_ctx.scroll_r;
        if (active_ctx.cursor_pos_r < active_ctx.scroll_r)
                a_move_cursor_down();
}

void
a_scroll_left()
{
        if (active_ctx.scroll_c == 0) return;
        --active_ctx.scroll_c;
        if (active_ctx.cursor_pos_c >= active_ctx.max_display_c + active_ctx.scroll_c)
                a_move_cursor_left();
}

void
a_scroll_right()
{
        /* This code is wrong */
        if (active_ctx.scroll_c + active_ctx.max_display_c >= active_ctx.body->data->size) return;
        ++active_ctx.scroll_c;
        if (active_ctx.cursor_pos_c < active_ctx.scroll_c)
                a_move_cursor_right();
}
