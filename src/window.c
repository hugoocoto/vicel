#include "window.h"
#include "cellmap.h"
#include "common.h"
#include "da.h"
#include "debug.h"
#include "escape_code.h"
#include "flag.h"
#include "keyboard.h"
#include "mappings.h"
#include "saving.h"

#define CELL_FG FG_BLUE
#define CELL_BG BG_BLACK
#define CELL_SELECT_FG FG_GREEN
#define CELL_SELECT_BG BG_BLACK

#define column_width 10
#define row_width 1

Context active_ctx = INIT_CONTEXT;

int
parse_coords(char *coords, int *x, int *y)
{
        char *c = coords;
        *x = 0;
        *y = 0;

        if (!isalpha(*c)) return 1;
        while (isalpha(*c)) {
                *y *= 'Z' - 'A' + 1;
                *y += toupper(*c) - 'A';
                ++c;
        }
        if (*c < '0' || *c > '9') return 1;
        while ('0' <= *c && *c <= '9') {
                *x *= 10;
                *x += *c - '0';
                ++c;
        }
        return 0;
}

Cell *
get_cell_from_coords(char *coords)
{
        int x, y;
        if (parse_coords(coords, &x, &y)) {
                report("Impossible to parse coords: %s", coords);
                exit(443);
        }
        if (active_ctx.body == NULL) {
                report("get_cell_from_coords: using no yet initialized body", x, coords);
                exit(444);
        }
        if (x < 0 || x >= active_ctx.body->size) {
                report("Invalid x coord: %d from %s", x, coords);
                return NULL;
        }
        if (y < 0 || y >= active_ctx.body->data->size) {
                report("Invalid x coord: %d from %s", x, coords);
                return NULL;
        }
        return cm_get_cell_ptr(active_ctx.body, x, y);
}

void
get_current_position(int *x, int *y)
{
        // starting at 1,1
        printf(T_DSR());
        fflush(stdout);
        char buf[1024];
        ssize_t n;
repeat:
        if ((n = read(STDIN_FILENO, buf, sizeof buf - 1)) < 0) {
                report("Error reading stdin from get_current_position");
                exit(1);
        }
        buf[n] = 0;
        if (sscanf(buf, T_CSI "%d;%dR", x, y) != 2) goto repeat;
}

char mappings_buffer[24];

void
print_status_bar()
{
        /* Quite hardcoded for now */
        char status[1024];
        char buf[1024];

        *status = 0;
        strcat(status, "vicel");
        strcat(status, " | ");
        strcat(status, "filename: ");
        strcat(status, active_ctx.filename ?: "(unnamed)");
        strcat(status, mappings_buffer);
        buf[snprintf(buf, active_ctx.ws.ws_col + 1, "%-*s %*s @",
                     active_ctx.ws.ws_col - 3 - 15, status,
                     15, cm_type_repr(get_cursor_cell()->value.type))] = 0;

        assert(active_ctx.status_bar_height == 1);
        printf(T_CUP()); // go to top left corner
        printf(EFFECT(BG_BLACK, FG_YELLOW, BOLD));
        printf("%s", buf);
        printf(EFFECT(RESET));
}

void
print_mapping_buffer(char *buf, int len, int n, int repeat)
{
        if (repeat == 0) {
                mappings_buffer[snprintf(
                mappings_buffer, sizeof mappings_buffer,
                " [%-*.*s  ] ", n, len, buf)] = 0;
        } else {
                mappings_buffer[snprintf(
                mappings_buffer, sizeof mappings_buffer,
                " [x%d %-*.*s] ", repeat, n, len, buf)] = 0;
        }
        print_status_bar();
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

#define NUM_COL_WIDTH 5
void
cursor_gotocell(int x, int y)
{
        int first_cell_col = 1;
        int first_cell_row = 2;
        printCUP(first_cell_row + row_width * x, NUM_COL_WIDTH + first_cell_col + column_width * (y - 1));
}

void
display_add_names(CellMat *mat, int x_off, int y_off, int scr_h, int scr_w, int x0, int y0)
{
        int cx = x0;
        int cy = y0;
        int av = scr_w - cy;
        char *col = strdup("A");
        int n = 0;

        printf(EFFECT(BG_BLACK, FG_YELLOW));

        cy += NUM_COL_WIDTH;
        av -= NUM_COL_WIDTH;

        /* The top left gap */
        printf("%-*.*s", NUM_COL_WIDTH, NUM_COL_WIDTH, "");

        printCUP(cx, cy);

        for_da_each(cell, *mat->data)
        {
                int wwww = min(column_width, av);
                int ww = wwww / 2;
                printf("%*.*s%*.*s", ww, ww, col,
                       wwww - ww, wwww - ww, "");

                cy += column_width;
                av -= column_width;
                if (av <= 0) break;
                col[0]++;
        }

        cx = x0 += row_width;
        cy = y0;

        for_da_each(_, *mat)
        {
                printCUP(cx, cy);
                printf("%*d ", NUM_COL_WIDTH - 1, n);
                cx += row_width;
                n++;
        }

        free(col);
        printf(EFFECT(RESET));
}

/* Print the submatrix of mat, starting at x_off and y_off (top left) that
 * fits into the screen, starting at cursor position with the size
 * scr_x x scr_h. It doesn't need to be left neither top aligned. */

void
cm_display(CellMat *mat, int x_off, int y_off, int scr_h, int scr_w, int x0, int y0)
{
        printf(EFFECT(CELL_BG, CELL_FG));
        int cx = x0;
        int cy = y0;
        int av = scr_w - cy;
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

                        if (active_ctx.cursor_pos_r == xx &&
                            active_ctx.cursor_pos_c == yy)
                                printf(EFFECT(REVERSE));

                        printf("[%-*.*s]", min(column_width, av) - 2, min(column_width, av) - 2, cell->repr);

                        if (active_ctx.cursor_pos_r == xx &&
                            active_ctx.cursor_pos_c == yy)
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
                av = scr_w - cy;
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
        print_status_bar();
        display_add_names(active_ctx.body, 0, 0, active_ctx.ws.ws_row, active_ctx.ws.ws_col + 1, 2, 1);
        cm_display(active_ctx.body, 0, 0, active_ctx.ws.ws_row, active_ctx.ws.ws_col + 1, 3, 1 + NUM_COL_WIDTH);
        fflush(stdout);
}

void
resize_handler(int _)
{
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
        resize_handler(0); // get current winsize
}

void
reset_at_exit()
{
        printf(EFFECT(RESET));
        printf(T_ASBD());
        printf(T_CUSHW());
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
        // printf(T_ASBE());
        printf(T_CUHDE());
        printf(EFFECT(RESET));
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
