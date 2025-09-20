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
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "core/core.h"
#include "env.h"
#include "interpreter.h"
#include "tokens.h"

#include "../readlain.h"

#define PROMPT "[vispel] >> "

extern void toggle_raw_mode();

bool repl_verbose = false;
bool using_repl = false;
bool vspl_should_quit = false;

int
REPL()
{
        char *buf;
        using_repl = true;
        env_create();
        load_core_lib();

        goto _input;

        while (1) {
                if (lex_analize(buf)) goto _input;
                if (repl_verbose) print_tokens();
                tok_parse();
                if (repl_verbose) print_ast();
                if (resolve() == 0) eval();

                if (vspl_should_quit) break;
        _input:
                toggle_raw_mode();
                buf = readlain(PROMPT);
                toggle_raw_mode();
                puts("");
        }
        env_destroy();
        vspl_free_tokens();
        free_stmt_head();
        using_repl = false;

        return 0;
}

int
VSPL(char *filename)
{
        char buf[1024 * 1024];
        ssize_t n;
        int fd;

        fd = open(filename, O_RDONLY);
        if (fd < 0) return -1;

        using_repl = true;
        env_create();
        load_core_lib();

        if ((n = read(fd, buf, sizeof buf - 2)) <= 0) return -1;
        buf[n] = 0;

        while (1) {
                if (lex_analize(buf)) goto _input;
                if (repl_verbose) print_tokens();
                tok_parse();
                if (repl_verbose) print_ast();
                if (resolve() == 0) eval_quiet();
        _input:
                if ((n = read(fd, buf, sizeof buf - 2)) <= 0) return -1;
                buf[n] = 0;
        }
        env_destroy();
        vspl_free_tokens();
        free_stmt_head();
        using_repl = false;

        if (n < 0) {
                report("Can't read");
                return -1;
        }

        return 0;
}
