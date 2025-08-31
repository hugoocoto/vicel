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
#include "mappings.h"
#include "options.h"
#include "saving.h"
#include "vispel/embedded.h"
#include "window.h"

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

volatile int should_autosave = 0;
volatile int save_time = 0;

void
autosave_handler(int s)
{
        assert(s == SIGALRM);
        if (should_autosave) return;
        should_autosave = 1;
        alarm(save_time);
}

void
set_autosave_handler()
{
        signal(SIGALRM, autosave_handler);
        save_time = win_opts.save_time;
        alarm(save_time);
}

void
toggle_mouse_support()
{
        static bool enabled = false;
        if (win_opts.use_mouse == false) return;
        set_ui_report("Mouse on");
        if (!enabled) {
                printf("\033[?1003h");
                enabled = true;
        } else {
                printf("\033[?1003l");
                enabled = false;
        }
}

void
reset_at_exit()
{
        EFFECT(RESET);
        T_ASBD();
        T_CUSHW();
        toggle_mouse_support();
        fflush(stdout);
}

void
catch_sigint()
{
        signal(SIGINT, exit);
}

char *
get_extension(char *filename)
{
        if (filename == NULL) return NULL;
        char *c = strrchr(filename, '.');
        if (c == NULL) return filename;
        return c + 1;
}

int
main(int argc, char *argv[])
{
        char *filename = NULL;
        char *cfile;

        flag_set(&argc, &argv);

        if (*argv[1] != '-') {
                filename = argv[1];
        }

        if (flag_get("--repl")) {
                extern int REPL();
                extern int VSPL(char *);
                extern bool repl_verbose;
                char *file;
                repl_verbose = flag_get("-V");
                if (flag_get_value(&file, "-f")) return VSPL(file);

                if (flag_get("--preload")) {
                        /* Add config file if interactive repl */
                        options_init(.filename = filename,
                                     .fileextension = get_extension(filename));
                        parse_options_default_file(); // before parse custom config file
                        if (flag_get_value(&cfile, "-c", "--config-file")) parse_options_file(fopen(cfile, "r"));
                }
                return REPL();
        }

        if (flag_get("-m", "--use-mouse")) {
                printf("Are you idiot?\n");
                exit(ERR_NONE);
        }

        if (flag_get("-D", "--debug")) debug_level = 1;

        report("------| Starting |------");
        options_init(.filename = filename,
                     .fileextension = get_extension(filename));
        parse_options_default_file(); // before parse custom config file

        if (flag_get_value(&cfile, "-c", "--config-file")) {
                parse_options_file(fopen(cfile, "r"));
        }

        if (flag_get("--dump-options")) {
                vspl_dump_env();
                return 0;
        }

        options_destroy();

        if (filename == NULL && argc == 2 && *argv[1] != '-') {
                filename = argv[1];
        }

        if (debug_level == 0) T_ASBE();
        T_CUHDE();
        EFFECT(RESET);
        toggle_mouse_support();
        clear_screen();
        atexit(reset_at_exit);
        catch_sigint();

        set_default_colors(); // after parse config file
        load(filename, &active_ctx);
        set_resize_handler();
        set_autosave_handler();

        start_kbhandler(); // loop

        save(&active_ctx);
        cm_destroy(active_ctx.body);
        a_free_yank_buffer();
        free_opts();

        report("---| End without error |---");
        return 0;
}
