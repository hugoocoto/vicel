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
 * For questions or support, contact: me@hugocoto.com
 */

#include "saving.h"
#include "cellmap.h"
#include "common.h"
#include "da.h"
#include "debug.h"
#include "keyboard.h"
#include "window.h"

void
remove_spaces(char *c)
{
        char *cc;
        while (*c) {
                if (*c == '"' && (cc = strchr(c + 1, '"'))) {
                        c = cc + 1;
                        continue;
                }
                if (isspace(*c))
                        memmove(c, c + 1, strlen(c));
                else
                        ++c;
        }
}

void
get_sep(char **r, char **c, bool *last)
{
        if (**r == '"') {
                if ((*c = strstr(*r + 1, "\","))) {
                        **c = 0;
                        ++*c;
                        *last = false;
                        ++*r;
                        return;
                }
                if (!strchr(*r + 1, ',')) {
                        if ((*c = strrchr(*r + 1, '"'))) {
                                **c = 0;
                                ++*r;
                        }
                        *last = true;
                        return;
                }
        }

        if ((*c = strchr(*r, ','))) {
                *last = false;
                return;
        }

        *last = true;
        *c = *r + strlen(*r);
        return;
}

CellArr
get_line_data(char *line)
{
        CellArr ca = (CellArr) { 0 };
        size_t len = strlen(line);
        bool last = false;
        Cell cell;
        char *r = line;
        char *c = line;

        do {
                get_sep(&r, &c, &last);
                if (line + len == r) break;
                *c = 0;
                cell = EMPTY_CELL;
                free(cell.repr);
                cell.repr = strdup(r);
                da_append(&ca, cell);
                r = c + 1;
        } while (!last);
        return ca;
}

bool
get_data(CellMat *cm, FILE *f, int *max_size)
{
        char line[1024 * 1024];
        CellArr ca;
        char *c;
        *max_size = 0;

        while (fgets(line, sizeof line, f)) {
                if ((c = strchr(line, '\n'))) *c = 0;
                if ((c = strchr(line, '\r'))) *c = 0;

                report("Line: `%s`", line);
                remove_spaces(line);
                ca = get_line_data(line);
                da_append(cm, ca);
                *max_size = max(*max_size, ca.size);
        }
        return *max_size == 0;
}

void
load(char *filename, Context *ctx)
{
        int max_size;
        FILE *f;
        ctx->cursor_pos_c = 0;
        ctx->cursor_pos_r = 0;
        ctx->scroll_c = 0;
        ctx->scroll_r = 0;

        if (filename == NULL) goto load_blank;

        ctx->filename = strdup(filename);
        f = fopen(filename, "r");

        if (f == NULL) {
                report("Fail to load from %s", filename);
                goto load_blank;
        }

        ctx->body = calloc(1, sizeof(CellMat));
        if (get_data(ctx->body, f, &max_size)) {
                free(ctx->body);
                report("Load empty file");
                goto load_blank;
        }

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
                for_da_each(c, *row)
                {
                        if (c->input_repr && *c->input_repr)
                                dprintf(fd, "\"%s\",", c->input_repr);
                        else
                                dprintf(fd, "%s,", c->input_repr);
                }
                dprintf(fd, "\n");
        }

        if (ftruncate(fd, lseek(fd, 0, SEEK_CUR))) {
                report("ftruncate failed: csv might be corrupted");
        }
}
