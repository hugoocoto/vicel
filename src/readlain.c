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

#include "common.h"
#include "debug.h"
#include "escape_code.h"

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
                }
                if (!strcmp(buf, "[C")) {
                        if (rline[rlindex]) {
                                ++rlindex;
                                T_CUF(1);
                        };
                }
                break;
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
        line_refresh();

        /* get line */
        char c;
        quit = false;

        while (!quit) {
                fflush(stdout);
                switch (read(STDIN_FILENO, &c, 1)) {
                case 0:
                case -1:
                        report("Exit from readlain due to invalid (or zero) read");
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
