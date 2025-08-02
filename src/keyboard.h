#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "cellmap.h"

void start_kbhandler();
void set_cell_text(Cell *c, char *text);
char * get_input_at_cursor();

#endif //! KEYBOARD_H
