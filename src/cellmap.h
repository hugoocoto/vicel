#ifndef CELLMAP_H
#define CELLMAP_H

#include "da.h"

typedef enum {
        TYPE_NUMBER = 0,
        TYPE_TEXT,
        TYPE_EMPTY,
        TYPE_FORMULA,
        TYPE_LEN,
} CellType;

typedef struct Value {
        CellType type;
        union {
                double num;
                char *text;
                struct Formula *formula;
        } as;
} Value;

#define AS_NUMBER(n) (Value){ \
        .type = TYPE_NUMBER,  \
        .as.num = n,          \
};


typedef struct Cell {
        int width, heigh;
        /* Cells that depend on the value of this cell */
        struct {
                int capacity;
                int size;
                struct Cell **data;
        } subscribers;
        Value value;
        int selected;
        char *repr; // string representation
} Cell;

typedef DA(Cell) CellArr;
typedef DA(CellArr) CellMat;

#define VALUE_EMPTY                 \
        (Value)                     \
        {                           \
                .type = TYPE_EMPTY, \
        }

#define EMPTY_CELL                                                                                       \
        (struct Cell)                                                                                    \
        {                                                                                                \
                .width = 20, .heigh = 1, .value = VALUE_EMPTY, .subscribers = { 0 }, .repr = strdup(""), \
        }


/* Create a 1x1 cell map */
CellMat *cm_init();

void cm_add_row(CellMat *mat);
void cm_add_col(CellMat *mat);

Cell cm_get_cell(CellMat *mat, int x, int y);
Cell *cm_get_cell_ptr(CellMat *mat, int x, int y);

void cm_subscribe(Cell *actor, Cell *observer);
void cm_unsubscribe(Cell *actor, Cell *observer);
void cm_notify(Cell *actor, Cell *observer); // implemented in observer

void cm_convert(Cell *c, CellType tnew);

/* Print the submatrix of mat, starting at x_off and y_off (top left) that
 * fits into the screen, starting at cursor position with the size
 * scr_x x scr_h. It doesn't need to be left neither top aligned. */
void cm_display(CellMat *mat, int x_off, int y_off, int scr_h, int scr_w);

void cm_destroy(CellMat *mat);

char *get_repr(Value v);

const char *cm_type_repr(CellType);

#endif
