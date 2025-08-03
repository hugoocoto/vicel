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
                report("LOAD `%s`", line);
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
                        free(cell.repr);
                        cell.repr = strdup(r);
                        da_append(&ca, cell);
                        r = c + 1;
                } while (!last);
                da_append(ctx->body, ca);
        }

        for_da_each(row, *ctx->body)
        {
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
                                report(",");
                        }
                        report("%s", c->input_repr);
                        dprintf(fd, "%s", c->input_repr);
                }
                report("\\n");
                dprintf(fd, "\n");
        }
}
