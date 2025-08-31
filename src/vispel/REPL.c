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

int
REPL()
{
        char *buf;
        ssize_t n;

        using_repl = true;
        env_create();
        load_core_lib();

        toggle_raw_mode();
        buf = readlain(PROMPT);
        toggle_raw_mode();
        puts("");

        while (1) {
                if (lex_analize(buf)) goto _input;
                if (repl_verbose) print_tokens();
                tok_parse();
                if (repl_verbose) print_ast();
                if (resolve() == 0) eval();
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

        if (n < 0) {
                report("Can't read\n");
                return -1;
        }

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
                if (resolve() == 0) eval();
        _input:
                if ((n = read(fd, buf, sizeof buf - 2)) <= 0) return -1;
                buf[n] = 0;
        }
        env_destroy();
        vspl_free_tokens();
        free_stmt_head();
        using_repl = false;

        if (n < 0) {
                report("Can't read\n");
                return -1;
        }

        return 0;
}
