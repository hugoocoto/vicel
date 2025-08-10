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
#include <ctype.h>
#include <stdbool.h>
#include <string.h>


void
remove_spaces(char *c)
{
        while (*c) {
                if (isspace(*c))
                        memmove(c, c + 1, strlen(c));
                else
                        ++c;
        }
}

int
get_sep(char **r, char **c, bool *last)
{
        if (**r == '"') {
                if ((*c = strstr(*r + 1, "\","))) {
                        **c = 0;
                        ++*c;
                        *last = false;
                        ++*r;
                        return 2;
                }
                if (!strchr(*r + 1, ',')) {
                        if ((*c = strrchr(*r + 1, '"'))) {
                                **c = 0;
                                ++*r;
                        }
                        *last = true;
                        return 1;
                }
        }

        if ((*c = strchr(*r, ','))) {
                *last = false;
                return 1;
        }

        *last = true;
        *c = *r + strlen(*r);
        return 1;
}

CellArr
get_line_data(char *line)
{
        char *r;
        char *c;
        CellArr ca = (CellArr) { 0 };
        Cell cell;
        bool last = false;

        report("line: `%s`", line);
        c = r = line;
        do {
                get_sep(&r, &c, &last);
                *c = 0;
                report("=> %s", r);
                cell = EMPTY_CELL;
                free(cell.repr);
                cell.repr = strdup(r);
                da_append(&ca, cell);
                r = c + 1;
        } while (!last);
        return ca;
}

int
get_data(CellMat *cm, FILE *f)
{
        char line[1024];
        int max_size = 0;
        CellArr ca;
        char *c;

        while (fgets(line, sizeof line, f)) {
                if ((c = strchr(line, '\n'))) *c = 0;
                if ((c = strchr(line, '\r'))) *c = 0;

                remove_spaces(line);
                ca = get_line_data(line);
                da_append(cm, ca);
                max_size = max(max_size, ca.size);
        }
        return max_size;
}

void
load(char *filename, Context *ctx)
{
        int max_size;
        ctx->cursor_pos_c = 0;
        ctx->cursor_pos_r = 0;
        ctx->scroll_c = 0;
        ctx->scroll_r = 0;

        if (filename == NULL) goto load_blank;

        ctx->filename = strdup(filename);

        FILE *f = fopen(filename, "r");
        if (f == NULL) {
                report("Fail to load from %s", filename);
                goto load_blank;
        }

        ctx->body = calloc(1, sizeof(CellMat));
        max_size = get_data(ctx->body, f);

        for_da_each(row, *ctx->body)
        {
                while (row->size < max_size)
                        da_append(row, EMPTY_CELL);
                for_da_each(c, *row)
                {
                        set_cell_text(c, strdup(c->repr));
                }
        }

        return;

load_blank:
        ctx->body = cm_init();
}

void
save(Context *ctx)
{
        int fd;
        if (ctx->filename == NULL) {
                report("Can't save unamed sheet");
                return;
        }

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
                        if (c->input_repr && *c->input_repr)
                                dprintf(fd, "\"%s\"", c->input_repr);
                        else
                                dprintf(fd, "%s", c->input_repr);
                }
                dprintf(fd, "\n");
        }

        ftruncate(fd, lseek(fd, 0, SEEK_CUR));
}
