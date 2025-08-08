/*
 * VICEL - Visual Cell editor
 * Copyright (C) 2025  Hugo Coto Florez
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * For questions or support, contact: hugo.coto@member.fsf.org
 */

#include "saving.h"
#include "cellmap.h"
#include "common.h"
#include "da.h"
#include "debug.h"
#include "keyboard.h"
#include "window.h"

void
unmask_comma(char *c)
{
        while ((c = strchr(c, -','))) {
                *c = ',';
        }
}

char *
get_value_from_cell_literal(char *r)
{
        char *c;
        char *t;

        // remove left spaces
        while (isspace(*r))
                ++r;

        unmask_comma(r);

        // if values are between `"` then remove it
        if (*r == '"' && (t = strrchr(r + 1, '"'))) {
                *t = 0;
                ++r;
        }
        c = r;

        for (; *c; ++c) {
                switch (*c) {
                case ' ' ... 126:
                        break;
                default:
                        report("Invalid char %d (%c)", *c, *c);
                        *c = '?';
                        break;
                }
        }
        return strdup(r);
}

void
load(char *filename, Context *ctx)
{
        ctx->cursor_pos_c = 0;
        ctx->cursor_pos_r = 0;

        if (filename) ctx->filename = strdup(filename);

        FILE *f = fopen(filename, "r");
        if (f == NULL) {
                report("Fail to load from %s", filename);
                goto load_blank;
        }

        ctx->body = calloc(1, sizeof(CellMat));

        char line[1024];
        char *r;
        char *c;
        CellArr ca;
        Cell cell;
        int max_size = 0;
        bool last;

        while (fgets(line, sizeof line, f)) {
                if ((c = strchr(line, '\n'))) *c = 0;
                if ((c = strchr(line, '\r'))) *c = 0;
                ca = (CellArr) { 0 };
                c = r = line;
                last = false;
                do {
                        if ((c = strchr(r, ','))) {
                                *c = 0;
                        } else {
                                last = true;
                        }
                        cell = EMPTY_CELL;
                        free(cell.repr);
                        cell.repr = get_value_from_cell_literal(r);
                        da_append(&ca, cell);
                        r = c + 1;
                } while (!last);

                da_append(ctx->body, ca);
                max_size = max(max_size, ca.size);
        }

        for_da_each(row, *ctx->body)
        {
                while (row->size < max_size)
                        da_append(row, EMPTY_CELL);
                for_da_each(c, *row)
                set_cell_text(c, strdup(c->repr));
        }

        return;

load_blank:
        ctx->body = cm_init();
}

void
mask_comma(char *c)
{
        while ((c = strchr(c, ','))) {
                *c = -',';
        }
}

void
save(Context *ctx)
{
        if (ctx->filename == NULL) {
                report("Can't save unamed sheet");
                return;
        }
        int fd;

        fd = open(ctx->filename, O_WRONLY | O_CREAT, 0600);

        if (fd < 0) {
                report("Can't open %s to write", ctx->filename);
                return;
        }

        for_da_each(row, *ctx->body)
        {
                bool first = true;
                for_da_each(c, *row)
                {
                        if (first) {
                                first = false;
                        } else {
                                dprintf(fd, ",");
                        }
                        mask_comma(c->input_repr);
                        dprintf(fd, "%s", c->input_repr);
                }
                dprintf(fd, "\n");
        }

        ftruncate(fd, lseek(fd, 0, SEEK_CUR));
}
