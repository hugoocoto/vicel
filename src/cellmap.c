#include "cellmap.h"
#include "common.h"
#include "da.h"

/* Create a 1x1 cell map */
CellMat *
cm_init()
{
        CellMat *cm = calloc(1, sizeof(CellMat));
        CellArr ca = { 0 };
        da_append(&ca, EMPTY_CELL);
        da_append(cm, ca);
        return cm;
}

void
cm_add_row(CellMat *mat)
{
        CellArr ca = { 0 };
        int s = mat->data->size;
        for (int i = 0; i < s; i++)
                da_append(&ca, EMPTY_CELL);
        da_append(mat, ca);
}

void
cm_add_col(CellMat *mat)
{
        for (int i = 0; i < mat->size; i++)
                da_append(&mat->data[i], EMPTY_CELL);
}

Cell
cm_get_cell(CellMat *mat, int x, int y)
{
        return mat->data[x].data[y];
}

void
cm_convert(Cell *c, CellType tnew)
{
        // Todo
}

/* Print the submatrix of mat, starting at x_off and y_off (top left) that
 * fits into the screen, starting at cursor position with the size
 * scr_x x scr_h. It doesn't need to be left neither top aligned. */
void cm_display(CellMat *mat, int x_off, int y_off, int scr_h, int scr_w);

void
cm_destroy(CellMat *mat)
{
        // Todo
}
