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

#include "cellmap.h"
#include "common.h"
#include "debug.h"
#include "escape_code.h"
#include "options.h"
#include "window.h"
#include <assert.h>

/* Append text to line BEFORE call readline */
void rlain_insert(char *text);

/* Return the heap allocated representation of the writen text */
char *readlain(char *prompt);

static char rline[1024] = "";
static size_t rlindex = 0;
static bool quit = false;
static char *lprompt = "";

void
rlain_insert(char *text)
{
        strncat(rline, text, sizeof rline - 1);
        rlindex = strlen(rline);
}

static void
reset_line()
{
        rlindex = 0;
        *rline = 0;
}

static void
rlinsert(char c)
{
        if (rlindex >= sizeof rline - 1) {
                report("Invalid insert index: %d", rlindex);
                return;
        }

        if (rline[rlindex] == 0) {
                rline[rlindex] = c;
                rline[rlindex + 1] = 0;
                ++rlindex;
                return;
        }

        if (strlen(rline) >= sizeof rline - 1) {
                report("rline is full");
                return;
        }

        memmove(rline + rlindex + 1, rline + rlindex, strlen(rline + rlindex) + 1);
        rline[rlindex] = c;
        ++rlindex;
}

static void
line_refresh()
{
        T_RCP();
        printf("%s%s ", lprompt, rline);
        T_RCP();
        if (rlindex) {
                T_CUF((int) rlindex);
        }
}

static void
handle_esc()
{
        char buf[16];
        ssize_t n;
        switch (n = read(STDIN_FILENO, buf, sizeof buf - 1)) {
        case 0:
                return;
        case -1:
                report("Exit from handle_esc due to invalid read");
                return;
        default:
                buf[n] = 0;
                report("Esc buffer: %s", buf);
                if (!strcmp(buf, "[D")) {
                        if (rlindex) {
                                --rlindex;
                                T_CUB(1);
                        }
                        return;
                }
                if (!strcmp(buf, "[C")) {
                        if (rline[rlindex]) {
                                ++rlindex;
                                T_CUF(1);
                        }
                        return;
                }

                char *name;
                char *ns;
                unsigned char c, r;
                char btn;
                static Cell *selection_start;
                static Cell *selection_end;
                if (sscanf(buf, "[M%c%c%c", &btn, &c, &r) == 3) {
                        static char hold = 0;
                        int cellc = (c - 33 - win_opts.num_col_width) / win_opts.col_width + active_ctx.scroll_c;
                        int cellr = (r - 33 - 2) / win_opts.row_width + active_ctx.scroll_r;
                        set_ui_report("mouse at %d/%d %d/%d", cellc, active_ctx.max_display_r, cellr, active_ctx.max_display_c);
                        switch (btn) {
                        case '"': /* mouse left press */
                                hold = btn;
                                break;
                        case '#': /* mouse release */
                                if (hold == '"') {
                                }
                                if (hold == ' ') {
                                        selection_end = cm_get_cell_ptr(active_ctx.body, cellc, cellr);
                                        if (!selection_start || !selection_end) break;
                                        ns = name = cm_get_cell_name(active_ctx.body, selection_start);
                                        assert(name && 2);
                                        while (*name)
                                                rlinsert(*name++);
                                        free(ns);
                                        if (selection_start != selection_end) {
                                                rlinsert(':');
                                                ns = name = cm_get_cell_name(active_ctx.body, selection_end);
                                                assert(name && 1);
                                                while (*name)
                                                        rlinsert(*name++);
                                                free(ns);
                                        }
                                        line_refresh();
                                }
                                hold = 0;
                                break;
                        case ' ': /* mouse right press */
                                hold = btn;
                                selection_start = cm_get_cell_ptr(active_ctx.body, cellc, cellr);
                                break;
                        case '@': /* mouse left hold move */
                        case 'B': /* mouse right hold move */
                        case 'C': /* mouse move */
                        case 'a': /* mouse wheel up */
                        case '`': /* mouse wheel down */
                        case '!': /* mouse wheel press */
                        case 'A': /* mouse wheel hold move */
                        default:
                                break;
                        }
                }
        }
}

static void
handle_char(char c)
{
        if (isprint(c)) {
                rlinsert(c);

                // if (rline[rlindex] == 0) {
                //         putchar(c);
                //         return;
                // }

                line_refresh();
                return;
        }

        switch (c) {
        case 13: /* CR */
                quit = true;
                break;

        case 8:   /* BS */
        case 127: /* DEL as BS */
                if (rlindex == 0) break;
                if (rline[rlindex] == 0) {
                        rline[rlindex - 1] = 0;
                } else {
                        memmove(rline + rlindex - 1,
                                rline + rlindex,
                                strlen(rline + rlindex) + 1);
                }
                --rlindex;
                line_refresh();
                break;

        case 27: /* ESC */
                handle_esc();
                break;

        default:
                report("Unhandled char: %d", c);
                break;
        }
}

char *
readlain(char *prompt)
{
        // toggle_raw_mode(); as this is yet enabled
        lprompt = prompt;
        T_SCP();
        line_refresh();
        // line_refresh();

        /* get line */
        char c;
        quit = false;

        while (!quit) {
                fflush(stdout);
                switch (read(STDIN_FILENO, &c, 1)) {
                case 0:
                        break;
                case -1:
                        report("Exit from readlain due to invalid read:%s", strerror(errno));
                        quit = true;
                        break;

                default:
                        handle_char(c);
                        break;
                }
        }
        fflush(stdout);

        /* end get line */
        // toggle_raw_mode();

        char *l = strdup(rline);
        reset_line();
        return l;
}
