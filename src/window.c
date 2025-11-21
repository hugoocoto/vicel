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
 * For questions or support, contact: me@hugocoto.com
 */

#include "window.h"
#include "cellmap.h"
#include "color.h"
#include "common.h"
#include "da.h"
#include "debug.h"
#include "escape_code.h"
#include "mappings.h"
#include "options.h"
#include <unistd.h>

Context active_ctx = INIT_CONTEXT;
char ui_report[64] = "";
time_t ui_report_update_time = 0;

void
set_ui_report(const char *c, ...)
{
        va_list v;
        va_start(v, c);
        vsnprintf(ui_report, sizeof ui_report, c, v);
        ui_report_update_time = time(NULL);
        va_end(v);
}

void
clear_ui_report()
{
        *ui_report = 0;
}

int
get_cell_screen_width(Cell *c)
{
        /* TODO: handle the case when cell is the last one in the screen so it's
         * smaller than usual. Also, return some error i cell is not in screen
         * cell.width is not correct. Idk why. Fix this.
         * */
        c = (void *) c;
        return 10;
}

void
clear_ui_report_ontimeout(time_t maxtime)
{
        if (*ui_report && time(NULL) - ui_report_update_time > maxtime)
                *ui_report = 0;
}

int
parse_coords(char *c, int *x, int *y, bool *freeze_r, bool *freeze_c)
{
        *x = 0;
        *y = 0;
        if (freeze_r) *freeze_r = false;
        if (freeze_c) *freeze_c = false;

        if (*c == '$') {
                if (freeze_c) *freeze_c = true;
                ++c;
        }
        if (!isalpha(*c)) return 1;
        while (isalpha(*c)) {
                *x *= 'Z' - 'A' + 1;
                *x += toupper(*c) - 'A';
                ++c;
        }

        if (*c == '$') {
                if (freeze_r) *freeze_r = true;
                ++c;
        }
        if (*c < '0' || *c > '9') return 1;
        while ('0' <= *c && *c <= '9') {
                *y *= 10;
                *y += *c - '0';
                ++c;
        }

        return 0;
}

Cell *
get_cell_from_coords(char *coords)
{
        int x, y;
        if (parse_coords(coords, &x, &y, 0, 0)) {
                report("Impossible to parse coords: %s", coords);
                return NULL;
        }
        if (active_ctx.body == NULL) {
                report("get_cell_from_coords: using no yet initialized body", x, coords);
                exit(ERR_INVBODY);
        }
        if (y < 0 || y >= active_ctx.body->size) {
                report("Invalid y coord: %d from %s", y, coords);
                return NULL;
        }
        if (x < 0 || x >= active_ctx.body->data->size) {
                report("Invalid x coord: %d from %s", x, coords);
                return NULL;
        }
        return cm_get_cell_ptr(active_ctx.body, x, y);
}

void
set_cell_color(Cell *cell)
{
        if (cell->color.active) {
                apply_color(cell->color.scolor);
                return;
        }
        apply_color("cell");
}

// can block - it's better to throw an error I think
void
get_current_position(int *x, int *y)
{
        // starting at 1,1
        T_DSR();
        fflush(stdout);
        char buf[12];
        ssize_t n;
repeat:
        if ((n = read(STDIN_FILENO, buf, sizeof buf - 1)) < 0) {
                report("Error reading stdin from get_current_position");
                exit(ERR_STDIN);
        }
        buf[n] = 0;
        if (sscanf(buf, T_CSI "%d;%dR", y, x) != 2) goto repeat;
}

void
print_status_bar2()
{
        char buf[1024];
        bool has_ui_report = *ui_report;
        int asize = strlen(win_opts.ui_celltext_l_sep) +
                    strlen(get_cursor_cell()->input_repr) +
                    strlen(win_opts.ui_celltext_m_sep) +
                    strlen(cm_type_repr(get_cursor_cell()->value.type)) +
                    strlen(win_opts.ui_celltext_r_sep) >
                    active_ctx.ws.ws_col ?
                    0 :
                    strlen(get_color(has_ui_report ? "ui_report" : "ui"));

        buf[snprintf(buf, active_ctx.ws.ws_col + 1 + asize,
                     has_ui_report ? "%s%s%s%s%s%s%-*.*s" : "%s%s%s%s%s%s%*.*s",
                     win_opts.ui_celltext_l_sep,
                     get_cursor_cell()->input_repr,
                     win_opts.ui_celltext_m_sep,
                     cm_type_repr(get_cursor_cell()->value.type),
                     win_opts.ui_celltext_r_sep,
                     get_color(has_ui_report ? "ui_report" : "ui"),
                     max((int) (active_ctx.ws.ws_col -
                                +strlen(win_opts.ui_celltext_l_sep) -
                                +strlen(win_opts.ui_celltext_m_sep) -
                                +strlen(win_opts.ui_celltext_r_sep) -
                                +strlen(get_cursor_cell()->input_repr) -
                                +strlen(cm_type_repr(get_cursor_cell()->value.type))),
                         0),
                     max((int) (active_ctx.ws.ws_col -
                                +strlen(win_opts.ui_celltext_l_sep) -
                                +strlen(win_opts.ui_celltext_m_sep) -
                                +strlen(win_opts.ui_celltext_r_sep) -
                                +strlen(get_cursor_cell()->input_repr) -
                                +strlen(cm_type_repr(get_cursor_cell()->value.type))),
                         0),
                     has_ui_report ? ui_report : win_opts.ui_status_bottom_end)] = 0;

        assert(active_ctx.status_bar_height == 1);
        T_CUP(active_ctx.ws.ws_row, 1);

        apply_color("ui_cell_text");
        printf("%s", buf);
}

char mappings_buffer[16];

void
print_status_bar()
{
        char buf[1024];

        buf[snprintf(buf, active_ctx.ws.ws_col + 1, "%s%s%s%s%*.*s",
                     win_opts.status_l_stuff,
                     win_opts.status_filename,
                     active_ctx.filename ?: "(unnamed)",
                     mappings_buffer,
                     (int) (active_ctx.ws.ws_col -
                            +strlen(win_opts.status_l_stuff) -
                            +strlen(active_ctx.filename ?: "(unnamed)") -
                            +strlen(mappings_buffer) -
                            +strlen(win_opts.status_filename)),
                     (int) (active_ctx.ws.ws_col -
                            +strlen(win_opts.status_l_stuff) -
                            +strlen(active_ctx.filename ?: "(unnamed)") -
                            +strlen(mappings_buffer) -
                            +strlen(win_opts.status_filename)),
                     win_opts.status_r_end)] = 0;

        assert(active_ctx.status_bar_height == 1);
        T_CUP(1, 1);

        apply_color("ui");
        printf("%s", buf);
}

void
print_mapping_buffer(char *buf, int len, int n, int repeat)
{
        if (repeat == 0) {
                mappings_buffer[snprintf(
                mappings_buffer, sizeof mappings_buffer,
                " [%-*.*s   ] ", n, len, buf)] = 0;
        } else {
                mappings_buffer[snprintf(
                mappings_buffer, sizeof mappings_buffer,
                " [x%2d %-*.*s] ", repeat, n, len, buf)] = 0;
        }
        print_status_bar();
}

void
print_at(int r, int c, char *buf, int buflen, int n)
{
        T_SCP();
        T_CUP(r, c);
        printf("%*.*s", n, buflen, buf);
        T_RCP();
}

void
cursor_gotocell(int x, int y)
{
        int first_cell_col = 1;
        int first_cell_row = 2;
        T_CUP(first_cell_row + win_opts.row_width * y - active_ctx.scroll_r,
              win_opts.num_col_width + first_cell_col + win_opts.col_width * (x - 1 - active_ctx.scroll_c));
}

void
display_add_names(CellMat *mat, int x_off, int y_off, int scr_w, int scr_h, int x0, int y0)
{
        int _cy = y0;
        int _cx = x0;
        int avx = scr_w - _cx;
        int avy = scr_h - _cy;
        char *col = strdup("  "); // Up to ZZ
        int range = 'Z' - 'A' + 1;
        int xx = 0;
        int n;

        T_CUP(_cy, _cx);
        apply_color("ln");

        /* The top left gap */
        printf("%-*.*s", win_opts.num_col_width, win_opts.num_col_width, "");

        _cx += win_opts.num_col_width;
        avx -= win_opts.num_col_width;
        avy -= win_opts.row_width;

        col[1] = 'A' + x_off % range;
        if (x_off / range) col[0] = 'A' + x_off / range - 1;

        for_da_each(cell, *mat->data)
        {
                if (xx < x_off) {
                        ++xx;
                        continue;
                }

                if (xx == active_ctx.cursor_pos_c) apply_color("ln_over");

                int wwww = min(win_opts.col_width, avx);
                int ww = (wwww + 1) / 2;
                printf("%*.*s%*.*s", ww, ww, col,
                       wwww - ww, wwww - ww, "");

                if (xx == active_ctx.cursor_pos_c) apply_color("ln");

                _cx += win_opts.col_width;
                avx -= win_opts.col_width;
                xx++;
                if (avx <= 0) break;

                if (col[1] == 'Z') {
                        col[1] = 'A';
                        col[0] = (col[0] == ' ') ? 'A' : col[0] + 1;
                        if (col[0] == 'Z') {
                                /* For now it don't support more than this */
                                break;
                        }
                } else {
                        col[1] += 1;
                }
        }

        if (avx > 0) T_EL(0);
        free(col);
        _cy = y0 += win_opts.row_width;
        _cx = x0;
        n = y_off;
        int yy = 0;
        for_da_each(_, *mat)
        {
                if (yy < y_off) {
                        ++yy;
                        continue;
                }
                T_CUP(_cy, _cx);

                if (yy == active_ctx.cursor_pos_r) apply_color("ln_over");

                printf("%*d ", win_opts.num_col_width - 1, n);

                if (yy == active_ctx.cursor_pos_r) apply_color("ln");

                _cy += win_opts.row_width;
                avy -= win_opts.row_width;
                if (avy <= 0) break;
                n++;
                yy++;
        }
}

void
cm_display(CellMat *mat, int x_off, int y_off, int scr_w, int scr_h, int x0, int y0)
{
        int _cy = y0;
        int _cx;
        int avx;
        int avy;
        int xx;
        int yy = 0;
        int m_r = 0;
        int m_c = 0;
        avy = scr_h - _cy;
        for_da_each(ca, *mat)
        {
                if (yy < y_off) {
                        ++yy;
                        continue;
                }
                ++m_r;
                _cx = x0;
                avx = scr_w - _cx;
                xx = 0;
                m_c = 0;
                for_da_each(cell, *ca)
                {
                        if (xx < x_off) {
                                ++xx;
                                continue;
                        }
                        T_CUP(_cy, _cx);
                        assert(cell->heigh == 1);

                        int w = min(win_opts.col_width, avx) -
                                strlen(win_opts.cell_l_sep) - strlen(win_opts.cell_r_sep);

                        if (w <= 0) {
                                break;
                        }
                        ++m_c;

                        if (cell->selected) {
                                apply_color("sheet_ui_selected");
                                printf("%s", win_opts.cell_l_sep);
                                apply_color("cell_selected");
                        }

                        else if (active_ctx.cursor_pos_r == yy &&
                                 active_ctx.cursor_pos_c == xx) {
                                apply_color("sheet_ui_over");
                                printf("%s", win_opts.cell_l_sep);
                                apply_color("cell_over");
                        }

                        else {
                                if (!win_opts.use_cell_color_for_sep) {
                                        apply_color("sheet_ui");
                                        printf("%s", win_opts.cell_l_sep);
                                }
                                set_cell_color(cell);
                                if (win_opts.use_cell_color_for_sep) {
                                        printf("%s", win_opts.cell_l_sep);
                                }
                        }

                        printf("%-*.*s", w, w, cell->repr);

                        if (active_ctx.cursor_pos_r == yy &&
                            active_ctx.cursor_pos_c == xx) {
                                apply_color("sheet_ui_over");
                                printf("%s", win_opts.cell_r_sep);
                        }

                        else if (cell->selected) {
                                apply_color("sheet_ui_selected");
                                printf("%s", win_opts.cell_r_sep);
                        }

                        else {
                                if (!win_opts.use_cell_color_for_sep) {
                                        apply_color("sheet_ui");
                                }
                                printf("%s", win_opts.cell_r_sep);
                        }

                        _cx += win_opts.col_width;
                        avx -= win_opts.col_width;
                        if (avx <= 0) break;
                        ++xx;
                }
                apply_color(C_RESET);
                if (avx > 0) T_EL(0);
                avy -= win_opts.row_width;
                _cy += win_opts.row_width;
                ++yy;
                if (avy <= 0) break;
        }

        if (avy > 0) T_ED(0);
        active_ctx.max_display_c = m_c;
        active_ctx.max_display_r = m_r;
}

void
render()
{
        print_status_bar();
        display_add_names(active_ctx.body, active_ctx.scroll_c, active_ctx.scroll_r, active_ctx.ws.ws_col + 1, active_ctx.ws.ws_row, 1, 2);
        cm_display(active_ctx.body, active_ctx.scroll_c, active_ctx.scroll_r, active_ctx.ws.ws_col + 1, active_ctx.ws.ws_row, win_opts.num_col_width + 1, 3);
        print_status_bar2();
        fflush(stdout);
}
