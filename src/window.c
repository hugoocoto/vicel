#include "window.h"
#include "common.h"
#include "escape_code.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

Context active_ctx = { 0 };

/* Todo: Remove commented lines */
void
toggle_raw_mode()
{
        static struct termios origin_termios;
        // static int flags;
        static bool enabled = false;

        /* Disable raw mode if enabled */
        if (enabled) {
                tcsetattr(STDIN_FILENO, TCSANOW, &origin_termios);
                // fcntl(STDIN_FILENO, F_SETFL, flags);
                return;
        }

        /* Enable raw mode if disabled */
        struct termios raw_opts;
        tcgetattr(STDIN_FILENO, &origin_termios);
        raw_opts = origin_termios;
        cfmakeraw(&raw_opts);
        raw_opts.c_oflag |= (OPOST | ONLCR); // '\n' -> '\r\n'
        // raw_opts.c_cc[VMIN] = 0;
        // raw_opts.c_cc[VTIME] = 2; // wait 200ms for input
        tcsetattr(STDIN_FILENO, TCSANOW, &raw_opts);
        // flags = fcntl(STDIN_FILENO, F_GETFL, 0);
        //  READ is blocking. It returns after VTIME * 100 ms
        // fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
}

void
print_status_bar()
{
        /* Quite hardcoded for now */
        char status[1024];
        *status = 0;
        strcat(status, "vicel");
        strcat(status, " | ");
        strcat(status, "filename: ");
        strcat(status, active_ctx.filename ?: "(unnamed)");
        printf(T_CUP()); // go to top left corner
        printf(EFFECT(BG_BLACK, FG_YELLOW, BOLD));
        if (active_ctx.ws.ws_col <= strlen(status))
                status[active_ctx.ws.ws_col - 1] = 0;
        printf("%-*s@", active_ctx.ws.ws_col - 1, status);
        assert(active_ctx.status_bar_height == 1);
}

void
clear_screen()
{
        printf(T_ED() T_CUP()); // clear screen
        fflush(stdout);
}

void
render_all()
{
        clear_screen();
        print_status_bar();
}

void
resize_handler(int _)
{
        if (ioctl(0, TIOCGWINSZ, &active_ctx.ws) == -1) {
                perror("resize_handler: ioctl");
                exit(errno);
        }
        render_all();
}

void
set_resize_handler()
{
        signal(SIGWINCH, resize_handler);
        resize_handler(0); // get current winsize
}

int
main(int argc, char *argv[])
{
        // set_resize_handler();
        // toggle_raw_mode();
        // render_all();
        // char c;
        // while (read(STDIN_FILENO, &c, 1) || 1) {
        // }
        // toggle_raw_mode();
        return 0;
}
