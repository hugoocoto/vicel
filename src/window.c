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

#include "window.h"
#include "cellmap.h"
#include "common.h"
#include "da.h"
#include "debug.h"
#include "escape_code.h"
#include "mappings.h"

Context active_ctx = INIT_CONTEXT;

int
parse_coords(char *c, int *x, int *y)
{
        *x = 0;
        *y = 0;

        if (!isalpha(*c)) return 1;
        while (isalpha(*c)) {
                *x *= 'Z' - 'A' + 1;
                *x += toupper(*c) - 'A';
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
        if (parse_coords(coords, &x, &y)) {
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
get_current_position(int *x, int *y)
{
        // starting at 1,1
        T_DSR();
        fflush(stdout);
        char buf[1024];
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
        /* Quite hardcoded for now */
        char status[1024];
        char buf[1024];

        *status = 0;
        strcat(status, "Text under the cursor: ");
        buf[snprintf(buf, active_ctx.ws.ws_col + 1, "%*.*s %-*s @",
                     (int) strlen(status), (int) strlen(status), status,
                     active_ctx.ws.ws_col - 3 - (int) strlen(status),
                     get_cursor_cell()->input_repr)] = 0;

        assert(active_ctx.status_bar_height == 1);
        T_CUP(active_ctx.ws.ws_row, 1);
        EFFECT(UI_BG, UI_FG, BOLD);
        printf("%s", buf);
        EFFECT(RESET);
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
        T_CUP(1, 1);
        EFFECT(UI_BG, UI_FG, BOLD);
        printf("%s", buf);
        EFFECT(RESET);
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
        T_CUP(first_cell_row + row_width * y - active_ctx.scroll_r,
              num_col_width + first_cell_col + column_width * (x - 1 - active_ctx.scroll_c));
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
        EFFECT(UI_BG, UI_FG);

        /* The top left gap */
        printf("%-*.*s", num_col_width, num_col_width, "");

        _cx += num_col_width;
        avx -= num_col_width;
        avy -= row_width;

        col[1] = 'A' + x_off % range;
        if (x_off / range) col[0] = 'A' + x_off / range - 1;

        for_da_each(cell, *mat->data)
        {
                if (xx < x_off) {
                        ++xx;
                        continue;
                }
                int wwww = min(column_width, avx);
                int ww = wwww / 2;
                printf("%*.*s%*.*s", ww, ww, col,
                       wwww - ww, wwww - ww, "");

                _cx += column_width;
                avx -= column_width;
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

        free(col);
        _cy = y0 += row_width;
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
                printf("%*d ", num_col_width - 1, n);
                _cy += row_width;
                avy -= row_width;
                if (avy <= 0) break;
                n++;
        }

        EFFECT(RESET);
}

void
cm_display(CellMat *mat, int x_off, int y_off, int scr_w, int scr_h, int x0, int y0)
{
        EFFECT(CELL_BG, CELL_FG);
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
                        ++m_c;
                        T_CUP(_cy, _cx);
                        assert(cell->heigh == 1);

                        int w = min(column_width, avx) - 2;
                        if (w <= 0) {
                                break;
                        }

                        if (cell->selected)
                                EFFECT(CELL_SELECT_BG, CELL_SELECT_FG);

                        if (active_ctx.cursor_pos_r == yy &&
                            active_ctx.cursor_pos_c == xx)
                                EFFECT(REVERSE);

                        printf("[%-*.*s]", w, w, cell->repr);

                        if (active_ctx.cursor_pos_r == yy &&
                            active_ctx.cursor_pos_c == xx)
                                EFFECT(REVERSE_OFF);

                        if (cell->selected)
                                EFFECT(CELL_BG, CELL_FG);

                        _cx += column_width;
                        avx -= column_width;
                        if (avx <= 0) break;
                        ++xx;
                }
                if (avx > 0) T_EL(1);
                avy -= row_width;
                _cy += row_width;
                ++yy;
                if (avy <= 0) break;
        }

        if (avy > 0) T_ED(0);
        EFFECT(RESET);
        active_ctx.max_display_c = m_c;
        active_ctx.max_display_r = m_r;
}

void
render()
{
        EFFECT(RESET);
        print_status_bar();
        display_add_names(active_ctx.body, active_ctx.scroll_c, active_ctx.scroll_r, active_ctx.ws.ws_col + 1, active_ctx.ws.ws_row, 1, 2);
        cm_display(active_ctx.body, active_ctx.scroll_c, active_ctx.scroll_r, active_ctx.ws.ws_col + 1, active_ctx.ws.ws_row, num_col_width + 1, 3);
        print_status_bar2();
        fflush(stdout);
}
