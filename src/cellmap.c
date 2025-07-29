#include "cellmap.h"
#include "common.h"
#include "da.h"
#include "debug.h"
#include <stdlib.h>
#include <unistd.h>

const char *
cm_type_repr(CellType ct)
{
        static const char *lookup[] = {
                [TYPE_NUMBER] = "TYPE_NUMBER",
                [TYPE_TEXT] = "TYPE_TEXT",
                [TYPE_EMPTY] = "TYPE_EMPTY",
                [TYPE_LEN] = "TYPE_LEN",
        };
        if (ct >= 0 && ct < TYPE_LEN)
                return lookup[ct];
        report("Invalid access to type lookup: %d", ct);
        exit(1);
}

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

char *
get_num_repr(double d)
{
        char buf[1024];
        buf[snprintf(buf, sizeof buf - 1, "%g", d)] = 0;
        return strdup(buf);
}

void
cm_convert(Cell *c, CellType tnew)
{
        if (c->value.type == tnew) return;
        if (tnew == TYPE_EMPTY) {
                free(c->repr);
                *c = EMPTY_CELL;
        }

        switch (c->value.type) {
        case TYPE_NUMBER:
                switch (tnew) {
                case TYPE_TEXT:
                        c->value.type = tnew;
                        c->value.as.text = c->repr;
                        break;
                default:
                        goto no_yet_implemented;
                }
                break;
        case TYPE_TEXT:
                switch (tnew) {
                case TYPE_NUMBER:
                        c->value.type = tnew;
                        c->value.as.num = strtod(c->repr, NULL);
                        free(c->repr);
                        c->repr = get_num_repr(c->value.as.num);
                        break;
                default:
                        goto no_yet_implemented;
                }
                break;
        case TYPE_EMPTY:
                switch (tnew) {
                case TYPE_NUMBER:
                        c->value.type = tnew;
                        c->value.as.num = 0.0;
                        free(c->repr);
                        c->repr = get_num_repr(c->value.as.num);
                        break;
                case TYPE_TEXT:
                        c->value.type = tnew;
                        break;
                default:
                        goto no_yet_implemented;
                }
                break;
        default:
        no_yet_implemented:
                report("No yet implemented: Convert from %s to %s",
                       cm_type_repr(c->value.type), cm_type_repr(tnew));
                break;
        }
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
