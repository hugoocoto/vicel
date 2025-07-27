#include "window.h"
#include "cellmap.h"
#include "common.h"
#include "da.h"
#include "debug.h"
#include "escape_code.h"
#include "keyboard.h"

#define CELL_BG BG_BLUE
#define CELL_FG FG_BLACK

Context active_ctx = INIT_CONTEXT;

void
get_current_position(int *x, int *y)
{
        // starting at 1,1
        printf(T_DSR());
        fflush(stdout);
        fscanf(stdin, T_CSI "%d;%dR", x, y);
}

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
        printf(EFFECT(RESET));
}

void
print_at(int r, int c, char *buf, int buflen, int n)
{
        printf(T_SCP());
        printCUP(r, c);
        printf("%*.*s", n, buflen, buf);
        printf(T_RCP());
}

#define min(a, b) ((a) < (b) ? (a) : (b))

/* Print the submatrix of mat, starting at x_off and y_off (top left) that
 * fits into the screen, starting at cursor position with the size
 * scr_x x scr_h. It doesn't need to be left neither top aligned. */
void
cm_display(CellMat *mat, int x_off, int y_off, int scr_h, int scr_w)
{
        report("Call display");
        int x0, y0;
        int max = 10; // should be in column
        get_current_position(&x0, &y0);
        printf(EFFECT(CELL_BG, CELL_FG));
        int cx = x0;
        int cy = y0;
        int av = active_ctx.ws.ws_col - cy;
        for_da_each(ca, *mat)
        {
                report("Mat iteration");
                for_da_each(cell, *ca)
                {
                        report("Line iteration");
                        printCUP(cx, cy); // can optimize this
                        assert(cell->heigh == 1);
                        printf("%-*.*s", min(max, av), min(max, av), cell->repr);
                        cy += max;
                        av -= max;
                        if (av <= 0) break;
                }
                ++cx;
                cy = y0;
                av = active_ctx.ws.ws_col - cy;
        }
        printf(EFFECT(RESET));
}

void
clear_screen()
{
        printf(T_ED() T_CUP()); // clear screen
        fflush(stdout);
}

void
render()
{
        printf(EFFECT(RESET));
        clear_screen();
        print_status_bar();
        T_CUP(2, 1);
        cm_display(active_ctx.body, 0, 0, active_ctx.ws.ws_row - 1, active_ctx.ws.ws_col);
        fflush(stdout);
}

void
resize_handler(int _)
{
        if (ioctl(0, TIOCGWINSZ, &active_ctx.ws) == -1) {
                perror("resize_handler: ioctl");
                exit(errno);
        }
        render();
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
        report("---| Starting |---");
        printf(T_ASBE());
        printf(T_CUHDE());
        printf(EFFECT(RESET));
        active_ctx.body = cm_init();
        toggle_raw_mode();

        /* This should be called when rendering can be done */
        set_resize_handler();
        render();

        start_kbhandler();
        toggle_raw_mode();
        return 0;
}
