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

#include "keyboard.h"
#include "action.h"
#include "aptree.h"
#include "cellmap.h"
#include "common.h"
#include "da.h"
#include "debug.h"
#include "escape_code.h"
#include "formula.h"
#include "mappings.h"
#include "readlain.h"
#include "window.h"

bool quit = false;

void
should_quit()
{
        quit = true;
}

void
toggle_raw_mode()
{
        static struct termios origin_termios;
        static bool enabled = false;
        struct termios raw_opts;

        /* Disable raw mode if enabled */
        if (enabled) {
                tcsetattr(STDIN_FILENO, TCSANOW, &origin_termios);
                enabled = false;
                return;
        }

        /* Enable raw mode if disabled */
        tcgetattr(STDIN_FILENO, &origin_termios);
        raw_opts = origin_termios;
        cfmakeraw(&raw_opts);
        raw_opts.c_lflag |= ISIG; // enable C-c
        raw_opts.c_oflag |= (OPOST | ONLCR);
        tcsetattr(STDIN_FILENO, TCSANOW, &raw_opts);
        enabled = true;
}

static char
get_escape_sequence()
{
        int flags;
        char buf[16];
        ssize_t n;
        char ret = 0;

        flags = fcntl(STDIN_FILENO, F_GETFL, 0);
        fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);

        switch (n = read(STDIN_FILENO, buf, sizeof buf - 1)) {
        case 0:
        case -1:
                break;
        default:
                buf[n] = 0;
                if (buf[0] == '[')
                        ret = 128 + buf[1];
        }
        report("[KB] Escape sequence: \033%s", buf);
        fcntl(STDIN_FILENO, F_SETFL, flags);
        return ret;
}

bool
has_descents(APTree t, char *prefix, int len)
{
        return ap_has_descents(t, prefix, len);
}

Action
find_action(APTree t, char *prefix, int len)
{
        return ap_get_unique(t, prefix, len);
}

Action
find_action_force(APTree t, char *prefix, int len)
{
        return ap_get_last(t, prefix, len);
}

void
add_action(APTree t, char *prefix, Action action)
{
        ap_add(t, prefix, strlen(prefix), action);
}

char *
get_input_at_cursor()
{
        char *buf;
        char *c;

        cursor_gotocell(active_ctx.cursor_pos_c + 1, active_ctx.cursor_pos_r + 1);
        printf("%*s", column_width, "");
        T_CUB(column_width - 1);
        T_CUSHW();

        rlain_insert(get_cursor_cell()->input_repr);
        buf = readlain("");

        if (buf == NULL) return strdup("");
        if ((c = strchr(buf, '\n'))) *c = 0;   // trim newline
        if ((c = strchr(buf, '\r'))) *c = 0;   // trim cr
        if ((c = strchr(buf, '\t'))) *c = ' '; // change tab by space

        T_CUHDE();

        return buf;
}

void
detect_cell_type(Cell *c)
{
        char *buf = c->repr;
        if (*buf == '=') {
                cm_convert(c, TYPE_FORMULA);
                return;
        }
        bool is_numeric = true && *buf;
        while (*buf && is_numeric) {
                if (('0' <= *buf && *buf <= '9') || *buf == '.')
                        ++buf;
                else if (buf == c->repr && (*buf == '-' || *buf == '+'))
                        ++buf;
                else
                        is_numeric = false;
        }
        if (is_numeric) {
                cm_convert(c, TYPE_NUMBER);
                return;
        }

        if (*c->repr == 0) c->value.type = TYPE_EMPTY;
}

void
set_cell_text(Cell *c, char *text)
{
        if (c->value.type == TYPE_FORMULA) {
                destroy_formula(c);
        }

        free(c->repr);
        free(c->input_repr);

        c->value.as.text = text;
        c->repr = text;
        c->value.type = TYPE_TEXT;
        c->input_repr = strdup(c->repr);
        detect_cell_type(c);

        for_da_each(o, c->subscribers)
        {
                cm_notify(c, *o);
        }
}

void
get_set_cell_input()
{
        char *buf = get_input_at_cursor();
        set_cell_text(get_cursor_cell(), buf);
}

void
start_kbhandler()
{
        char buf[MAX_MAPPING_LEN];
        int read_index = 0;
        char saved_buf[MAX_MAPPING_LEN] = { 0 };
        int saved_read_index = 0;
        int saved_repeat = 0;
        Action action;
        APTree mappings = ap_init();
        int repeat = 0;

        add_action(mappings, "q", ACTION(should_quit));
        add_action(mappings, "r", ACTION(render));
        add_action(mappings, "gl", ACTION(a_add_col));
        add_action(mappings, "gj", ACTION(a_add_row));
        add_action(mappings, "j", ACTION(a_move_cursor_down));
        add_action(mappings, "k", ACTION(a_move_cursor_up));
        add_action(mappings, "h", ACTION(a_move_cursor_left));
        add_action(mappings, "l", ACTION(a_move_cursor_right));
        add_action(mappings, KEY_DOWN, ACTION(a_move_cursor_down));
        add_action(mappings, KEY_UP, ACTION(a_move_cursor_up));
        add_action(mappings, KEY_LEFT, ACTION(a_move_cursor_left));
        add_action(mappings, KEY_RIGHT, ACTION(a_move_cursor_right));
        add_action(mappings, "v", ACTION(a_select_toggle_cell));
        add_action(mappings, "i", ACTION(get_set_cell_input));
        add_action(mappings, "sd", ACTION(a_set_cell_type_numeric));
        add_action(mappings, "st", ACTION(a_set_cell_type_text));
        add_action(mappings, "d", ACTION(a_set_cell_type_empty));
        add_action(mappings, "sf", ACTION(a_set_cell_type_formula));
        add_action(mappings, "J", ACTION(a_copy_moving_down));
        add_action(mappings, "K", ACTION(a_copy_moving_up));
        add_action(mappings, "H", ACTION(a_copy_moving_left));
        add_action(mappings, "L", ACTION(a_copy_moving_right));
        add_action(mappings, "gij", ACTION(a_insert_moving_down));
        add_action(mappings, "gik", ACTION(a_insert_moving_up));
        add_action(mappings, "gih", ACTION(a_insert_moving_left));
        add_action(mappings, "gil", ACTION(a_insert_moving_right));
        add_action(mappings, "g0", ACTION(a_goto_top_left));
        add_action(mappings, "^", ACTION(a_goto_max_left));
        add_action(mappings, "$", ACTION(a_goto_max_right));
        add_action(mappings, "gg", ACTION(a_goto_top));
        add_action(mappings, "G", ACTION(a_goto_bottom));

        print_mapping_buffer("", 0, MAX_MAPPING_LEN, repeat);
        toggle_raw_mode();
        render();

        while (!quit && read(STDIN_FILENO, buf + read_index, 1)) {
                if (buf[read_index] < 0 || buf[read_index] >= 127) {
                        report("Invalid char read: %d", buf[read_index]);
                        continue;
                }

                ++read_index;

                if (buf[0] == '.') {
                        strncpy(buf, saved_buf, MAX_MAPPING_LEN);
                        read_index = saved_read_index;
                        repeat = saved_repeat;
                }

                if (buf[0] >= '0' && buf[0] <= '9') {
                        read_index = 0;
                        repeat *= 10;
                        repeat += buf[0] - '0';
                }

                if (read_index && buf[read_index - 1] == '\033') {
                        buf[read_index - 1] = get_escape_sequence();
                        if (buf[read_index - 1] == 0) {
                                read_index = 0;
                                repeat = 0;
                        }
                }

                /* Buffer contains a valid action whose prefix is unique */
                if ((action_is_valid(action = find_action(mappings, buf, read_index)))) {
                        strncpy(saved_buf, buf, read_index);
                        saved_read_index = read_index;
                        saved_repeat = repeat;
                        do {
                                // report("Action on buffer: [%.*s]", read_index, buf);
                                action.action();
                        } while (--repeat > 0);
                        read_index = 0;
                        repeat = 0;
                }

                /* Buffer contains an invalid action, but the previous buffered action was valid */
                // Todo
                /* Buffer is full */
                else if (read_index == MAX_MAPPING_LEN) {
                        /* Get the action whose prefix is in buffer with or without shared prefix */
                        if ((action_is_valid(action = find_action_force(mappings, buf, read_index)))) {
                                strncpy(saved_buf, buf, read_index);
                                saved_read_index = read_index;
                                saved_repeat = repeat;
                                do {
                                        // report("Action on buffer: [%.*s]", read_index, buf);
                                        action.action();
                                } while (--repeat > 0);
                        }
                        repeat = 0;
                        read_index = 0;
                }

                /* If buffer has no actions and no descents, invalidate buffer */
                else if (!has_descents(mappings, buf, read_index)) {
                        read_index = 0;
                        repeat = 0;
                }

                print_mapping_buffer(buf, read_index, MAX_MAPPING_LEN, repeat);
                render();
        }

        ap_destroy(mappings);
        toggle_raw_mode();
}
