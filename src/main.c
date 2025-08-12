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
#include "flag.h"
#include "keyboard.h"
#include "saving.h"
#include "window.h"

/* default debug level
 * 0: do not report
 * 1: report if DEBUG is set
 */
int debug_level = 0;

void
resize_handler(int s)
{
        assert(s == SIGWINCH);

        if (ioctl(0, TIOCGWINSZ, &active_ctx.ws) == -1) {
                perror("resize_handler: ioctl");
                exit(errno);
        }

        /* render again on resize */
        clear_screen();
        render();
}

void
set_resize_handler()
{
        signal(SIGWINCH, resize_handler);
        resize_handler(SIGWINCH); // get current winsize
}

void
reset_at_exit()
{
        EFFECT(RESET);
        T_ASBD();
        T_CUSHW();
        fflush(stdout);
}

int
main(int argc, char *argv[])
{
        char *filename = NULL;
        flag_set(&argc, &argv);
        if (flag_get("-m", "--use-mouse")) {
                printf("Are you idiot?\n");
                exit(ERR_NONE);
        }
        if (flag_get("-D", "--debug")) {
                debug_level = 1;
        }

        if (argc == 2) {
                filename = argv[1];
        }

        report("------| Starting |------");
        T_ASBE();
        T_CUHDE();
        EFFECT(RESET);
        clear_screen();
        atexit(reset_at_exit);
        load(filename, &active_ctx);

        set_resize_handler();
        start_kbhandler();

        save(&active_ctx);
        cm_destroy(active_ctx.body);

        report("---| End without error |---");
        return 0;
}
