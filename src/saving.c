#include "saving.h"
#include "cellmap.h"
#include "common.h"
#include "da.h"
#include "debug.h"
#include "formula.h"
#include "keyboard.h"
#include "window.h"
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

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
                                dprintf(fd, "%s", c->repr);
                                first = false;
                        } else {
                                dprintf(fd, ",%s", c->repr);
                        }
                }
                write(fd, &"\n", 1);
        }
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

        while (fgets(line, sizeof line, f)) {
                if ((c = strchr(line, '\n'))) *c = 0;
                ca = (CellArr) { 0 };
                c = r = line;
                bool last = false;
                do {
                        if ((c = strchr(r, ','))) {
                                *c = 0;
                        } else {
                                last = true;
                        }
                        cell = EMPTY_CELL;
                        set_cell_text(&cell, strdup(r));
                        da_append(&ca, cell);
                        r = c + 1;
                } while (!last);
                da_append(ctx->body, ca);
        }

        return;

load_blank:
        ctx->body = cm_init();
}
