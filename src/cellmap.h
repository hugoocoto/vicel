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
        .as.num = (n),        \
}


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

void cm_destroy(CellMat *mat);
void cm_free_cell(Cell *c);
void cm_clear_cell(Cell *c);

char *get_repr(Value v);

void cm_extend(CellMat *mat, int base_x, int base_y, int next_x, int next_y);
const char *cm_type_repr(CellType);

#endif
