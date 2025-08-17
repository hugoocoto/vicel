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
#include "keyboard.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


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

        report("Invalid insert case");
        exit(0);
}

static void
line_refresh()
{
        T_RCP();
        printf("%s%s", lprompt, rline);
        fflush(stdout);
}

static void
handle_char(char c)
{
        if (isprint(c)) {
                rlinsert(c);

                if (rline[rlindex] == 0) {
                        putchar(c);
                        fflush(stdout);
                        return;
                }

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
                        --rlindex;
                        rline[rlindex] = 0;
                        /* BS but it doesn't work by default */
                        T_CUB(1);
                        putchar(' ');
                        T_CUB(1);
                        fflush(stdout);
                        /* ******************************** */
                        break;
                }

                memcpy(rline + rlindex - 1,
                       rline + rlindex,
                       strlen(rline + rlindex) + 1);
                --rlindex;
                line_refresh();
                break;

        default:
                report("Unhandled char: %d", c);
                break;
        }
}

char *
readlain(char *prompt)
{
        toggle_raw_mode();
        lprompt = prompt;
        T_SCP();
        line_refresh();

        /* get line */
        char c;
        quit = false;

        while (!quit) {
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

        /* end get line */
        toggle_raw_mode();

        char *l = strdup(rline);
        reset_line();
        return l;
}
