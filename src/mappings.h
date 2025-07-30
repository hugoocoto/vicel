#ifndef MAPPINGS_H
#define MAPPINGS_H

#include "cellmap.h"
Cell * get_cursor_cell();

void a_quit();
void a_add_row();
void a_add_col();
void a_add_row();
void a_add_col();
void a_move_cursor_left();
void a_move_cursor_right();
void a_move_cursor_up();
void a_move_cursor_down();
void a_select_toggle_cell();
void a_set_cell_type_numeric();
void a_set_cell_type_text();
void a_set_cell_type_empty();
void a_set_cell_type_formula();

#endif //!MAPPINGS_H

