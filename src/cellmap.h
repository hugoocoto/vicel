#ifndef CELLMAP_H
#define CELLMAP_H

#include "da.h"

typedef enum {
        TYPE_NUMBER = 0,
        TYPE_TEXT,
        TYPE_EMPTY,
        TYPE_LEN,
} CellType;

typedef struct Cell {
        int width, heigh;
        CellType type;
        union {
                double num;
                char *text;
        } as;
        int selected;
        char *repr; // string representation
} Cell;

typedef DA(Cell) CellArr;
typedef DA(CellArr) CellMat;

#define EMPTY_CELL                                                               \
        (Cell)                                                                   \
        {                                                                        \
                .width = 20, .heigh = 1, .type = TYPE_EMPTY, .repr = strdup(""), \
        }


/* Create a 1x1 cell map */
CellMat *cm_init();

void cm_add_row(CellMat *mat);
void cm_add_col(CellMat *mat);

Cell cm_get_cell(CellMat *mat, int x, int y);

void cm_convert(Cell *c, CellType tnew);

/* Print the submatrix of mat, starting at x_off and y_off (top left) that
 * fits into the screen, starting at cursor position with the size
 * scr_x x scr_h. It doesn't need to be left neither top aligned. */
void cm_display(CellMat *mat, int x_off, int y_off, int scr_h, int scr_w);

void cm_destroy(CellMat *mat);

const char* cm_type_repr(CellType);

#endif
