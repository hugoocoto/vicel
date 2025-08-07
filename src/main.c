#include "common.h"
#include "debug.h"
#include "escape_code.h"
#include "flag.h"
#include "keyboard.h"
#include "saving.h"
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
        if (flag_get_value(&filename, "-m", "--use-mouse")) {
                printf("Are you idiot?\n");
                exit(0);
        }

        if (argc == 2) {
                filename = argv[1];
        }

        report("---| Starting |---");
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
