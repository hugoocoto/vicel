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
                lex_analize(buf);
                print_tokens();
                tok_parse();
                print_ast();
                if (resolve() == 0) eval();

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
