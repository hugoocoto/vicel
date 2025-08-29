#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "core/core.h"
#include "env.h"
#include "interpreter.h"
#include "tokens.h"

#define PROMPT "[vispel] >> "

void
prompt()
{
        printf("%s", PROMPT);
        fflush(stdout);
}

bool using_repl = false;

int
REPL()
{
        char buf[1024 * 1024];
        ssize_t n;

        using_repl = true;
        env_create();
        load_core_lib();
        prompt();
        while ((n = read(STDIN_FILENO, buf, sizeof buf - 2)) > 0) {
                buf[n] = 0;
                buf[n + 1] = EOF;
                lex_analize(buf);
                tok_parse();
                if (resolve() == 0) eval();
                prompt();
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
