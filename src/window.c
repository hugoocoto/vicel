#include "window.h"
#include "cellmap.h"
#include "common.h"
#include "da.h"
#include "debug.h"
#include "escape_code.h"
#include "keyboard.h"

#define CELL_BG BG_BLUE
#define CELL_FG FG_BLACK
#define CELL_SELECT_BG BG_GREEN
#define CELL_SELECT_FG FG_BLACK

Context active_ctx = INIT_CONTEXT;

void
get_current_position(int *x, int *y)
{
        // starting at 1,1
        printf(T_DSR());
        fflush(stdout);
        fscanf(stdin, T_CSI "%d;%dR", x, y);
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

void
cursor_gotocell(int x, int y)
{
        int first_cell_col = 1;
        int first_cell_row = 2;
        int column_width = 10; // should be in column
        int row_width = 1;     // should be in row
        printCUP(first_cell_row + row_width * x, first_cell_col + column_width * y);
}

/* Print the submatrix of mat, starting at x_off and y_off (top left) that
 * fits into the screen, starting at cursor position with the size
 * scr_x x scr_h. It doesn't need to be left neither top aligned. */
void
cm_display(CellMat *mat, int x_off, int y_off, int scr_h, int scr_w)
{
        report("Call display");
        int x0, y0;
        int column_width = 10; // should be in column
        int row_width = 1;     // should be in row
        get_current_position(&x0, &y0);
        printf(EFFECT(CELL_BG, CELL_FG));
        int cx = x0;
        int cy = y0;
        int av = active_ctx.ws.ws_col - cy;
        int xx = 0;
        int yy = 0;
        for_da_each(ca, *mat)
        {
                for_da_each(cell, *ca)
                {
                        printCUP(cx, cy); // can optimize this
                        assert(cell->heigh == 1);

                        if (cell->selected)
                                printf(EFFECT(CELL_SELECT_BG, CELL_SELECT_FG));

                        if (active_ctx.cursor_pos_x == xx &&
                            active_ctx.cursor_pos_y == yy)
                                printf(EFFECT(REVERSE));

                        printf("%-*.*s", min(column_width, av), min(column_width, av), cell->repr);

                        if (active_ctx.cursor_pos_x == xx &&
                            active_ctx.cursor_pos_y == yy)
                                printf(EFFECT(REVERSE_OFF));

                        if (cell->selected)
                                printf(EFFECT(CELL_BG, CELL_FG));

                        cy += column_width;
                        av -= column_width;
                        if (av <= 0) break;
                        ++xx;
                }
                cx += row_width;
                cy = y0;
                av = active_ctx.ws.ws_col - cy;
                xx = 0;
                ++yy;
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
        set_resize_handler();
        start_kbhandler(); // render from here

        report("---| End without error |---");
        return 0;
}
