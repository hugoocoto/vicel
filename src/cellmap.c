#include "cellmap.h"
#include "common.h"
#include "da.h"
#include "debug.h"
#include "formula.h"
#include <assert.h>

const char *
cm_type_repr(CellType ct)
{
        static const char *lookup[] = {
                [TYPE_NUMBER] = "TYPE_NUMBER",
                [TYPE_TEXT] = "TYPE_TEXT",
                [TYPE_EMPTY] = "TYPE_EMPTY",
                [TYPE_FORMULA] = "TYPE_FORMULA",
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

void
cm_subscribe(Cell *actor, Cell *observer)
{
        report("Add subscriber %p to %p", observer, actor);
        da_append(&actor->subscribers, observer);
        da_append(&observer->value.as.formula->subscribed, actor);
}

void
cm_unsubscribe(Cell *actor, Cell *observer)
{
        int i = 0;
        for (; i < actor->subscribers.size; i++) {
                if (actor->subscribers.data[i] == observer) {
                        da_remove(&actor->subscribers, i);
                        report("Remove subscriber %p to %p", observer, actor);
                        return;
                }
        }
        report("Fail to remove subscriber %p to %p", observer, actor);
}

Cell *
cm_get_cell_ptr(CellMat *mat, int x, int y)
{
        return &mat->data[x].data[y];
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

char *
get_repr(Value v)
{
        switch (v.type) {
        case TYPE_NUMBER:
                return get_num_repr(v.as.num);
        case TYPE_TEXT:
                return strdup(v.as.text);
        case TYPE_EMPTY:
                return strdup("");
        default:
                report("No yet implemented: get_repr for %s", cm_type_repr(v.type));
                return strdup("Err");
        }
}

void
cm_convert(Cell *c, CellType tnew)
{
        if (c->value.type == tnew) return;
        if (tnew == TYPE_EMPTY) {
                free(c->repr);
                *c = EMPTY_CELL;
                goto notify;
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
                case TYPE_FORMULA: {
                        c->value.type = tnew;
                        build_formula(c->value.as.text, c);
                        free(c->repr);
                        c->repr = get_repr(c->value.as.formula->value);
                        assert(c->value.type == tnew);
                        break;
                }
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
        case TYPE_FORMULA:
                switch (tnew) {
                case TYPE_NUMBER:
                        free_formula_subscribers(c);
                        c->value.type = tnew;
                        free(c->value.as.formula);
                        c->value.as.num = 0.0;
                        free(c->repr);
                        c->repr = get_num_repr(c->value.as.num);
                        break;
                case TYPE_TEXT:
                        free_formula_subscribers(c);
                        c->value.type = tnew;
                        free(c->value.as.formula);
                        c->value.as.text = c->repr;
                        break;
                default:
                        goto no_yet_implemented;
                }
                break;
        default:
        no_yet_implemented:
                report("No yet implemented: Convert from %s to %s",
                       cm_type_repr(c->value.type), cm_type_repr(tnew));
                return;
        }

notify:
        for_da_each(s, c->subscribers)
        {
                cm_notify(c, *s);
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
