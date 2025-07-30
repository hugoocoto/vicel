#include "mappings.h"
#include "cellmap.h"
#include "common.h"
#include "escape_code.h"
#include "window.h"

inline Cell *
get_cursor_cell()
{
        return cm_get_cell_ptr(active_ctx.body, active_ctx.cursor_pos_y, active_ctx.cursor_pos_x);
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
a_move_cursor_left()
{
        if (active_ctx.cursor_pos_x)
                --active_ctx.cursor_pos_x;
}

void
a_move_cursor_right()
{
        if (active_ctx.cursor_pos_x < active_ctx.body->data->size - 1)
                ++active_ctx.cursor_pos_x;
}

void
a_move_cursor_up()
{
        if (active_ctx.cursor_pos_y)
                --active_ctx.cursor_pos_y;
}

void
a_move_cursor_down()
{
        if (active_ctx.cursor_pos_y < active_ctx.body->size - 1)
                ++active_ctx.cursor_pos_y;
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
