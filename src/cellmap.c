#include "cellmap.h"
#include "common.h"
#include "da.h"
#include "debug.h"
#include "formula.h"
#include <assert.h>

bool
cm_is_valid_pos(CellMat *mat, int x, int y)
{
        return x >= 0 && x < mat->size &&
               y >= 0 && y < mat->data->size;
}

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
        case TYPE_FORMULA:
                return get_repr(v.as.formula->value);
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
                __auto_type s = c->subscribers;
                if (c->value.type == TYPE_FORMULA) {
                        destroy_formula(c);
                } else
                        free(c->repr);
                *c = EMPTY_CELL;
                c->subscribers = s;
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
                        build_formula(c->value.as.text, c);
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
                        destroy_formula(c);
                        c->value.type = tnew;
                        free(c->value.as.formula);
                        c->value.as.num = 0.0;
                        free(c->repr);
                        c->repr = get_num_repr(c->value.as.num);
                        break;
                case TYPE_TEXT:
                        destroy_formula(c);
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

void
cm_clear_cell(Cell *c)
{
        free(c->repr);
        switch (c->value.type) {
        case TYPE_FORMULA:
                destroy_formula(c);
                break;
        case TYPE_TEXT:
        case TYPE_NUMBER:
        case TYPE_EMPTY:
                break;
        default:
                report("No yet implemented: get_repr for %s",
                       cm_type_repr(c->value.type));
        }
}

void
cm_free_cell(Cell *c)
{
        da_destroy(&c->subscribers);
        cm_clear_cell(c);
}

void
cm_destroy(CellMat *mat)
{
        for_da_each(row, *mat)
        {
                for_da_each(c, *row)
                {
                        cm_free_cell(c);
                }
                da_destroy(row);
        }
        da_destroy(mat);
}

static Value
extend_row(Cell *c, Value origin, int displ)
{
        switch (origin.type) {
        case TYPE_EMPTY:
        case TYPE_TEXT:
                return origin;
        case TYPE_NUMBER:
                return AS_NUMBER(origin.as.num + abs(displ));
        case TYPE_FORMULA:
                origin.as.formula = formula_extend(c, origin.as.formula, 0, displ);
                return origin;
        default:
                report("No yet implemented: extend_row for %s",
                       cm_type_repr(origin.type));
        }
        return origin;
}

static Value
extend_col(Cell *c, Value origin, int displ)
{
        switch (origin.type) {
        case TYPE_EMPTY:
        case TYPE_TEXT:
                return origin;
        case TYPE_NUMBER:
                return AS_NUMBER(origin.as.num + abs(displ));
        case TYPE_FORMULA:
                origin.as.formula = formula_extend(c, origin.as.formula, 0, displ);
                return origin;
        default:
                report("No yet implemented: extend_col for %s",
                       cm_type_repr(origin.type));
        }
        return origin;
}

static void
set_extended_value(Cell *c, Value v, int displ_r, int displ_c)
{
        Value vnew = v;
        if (displ_r) vnew = extend_row(c, vnew, displ_r);
        if (displ_c) vnew = extend_col(c, vnew, displ_c);
        c->value = vnew;
        free(c->repr);
        c->repr = get_repr(c->value);
}

void
cm_extend(CellMat *mat, int base_x, int base_y, int next_x, int next_y)
{
        if (!cm_is_valid_pos(mat, base_x, base_y)) {
                report("Invalid position %d, %d in cell matrix", base_x, base_y);
                return;
        }
        if (!cm_is_valid_pos(mat, next_x, next_y)) {
                report("Invalid position %d, %d in cell matrix", next_x, next_y);
                return;
        }

        Value origin = cm_get_cell(mat, base_x, base_y).value;
        int displ_r = next_x - base_x;
        int displ_c = next_y - base_y;
        Cell *next_cell = cm_get_cell_ptr(mat, next_x, next_y);

        report("next_cell at cm_extend: %p", next_cell);

        clear_cell(next_cell);
        set_extended_value(next_cell, origin, displ_r, displ_c);

        for_da_each(o, next_cell->subscribers) cm_notify(next_cell, *o);
}
