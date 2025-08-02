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
#include "window.h"
#include <termios.h>

#define MAX_MAPPING_LEN 6

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
        raw_opts.c_oflag |= (OPOST | ONLCR);
        tcsetattr(STDIN_FILENO, TCSANOW, &raw_opts);
        enabled = true;
}

static void
get_escape_sequence()
{
        int flags;
        char buf[32];
        ssize_t n;

        flags = fcntl(STDIN_FILENO, F_GETFL, 0);
        fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);

        switch (n = read(STDIN_FILENO, buf, sizeof buf - 1)) {
        case 0:
        case -1:
                break;
        default:
                buf[n] = 0;
                report("[KB] Escape sequence: \033%s", buf);
        }

        fcntl(STDIN_FILENO, F_SETFL, flags);
}

static inline bool
has_descents(APTree t, char *prefix, int len)
{
        return ap_has_descentsl(t, prefix, len);
}

static inline Action
find_action(APTree t, char *prefix, int len)
{
        return ap_getl_unique(t, prefix, len);
}

static inline Action
find_action_force(APTree t, char *prefix, int len)
{
        return ap_getl_last(t, prefix, len);
}

static inline void
add_action(APTree t, char *prefix, Action action)
{
        ap_add(t, prefix, action);
}

char *
get_input_at_cursor()
{
        int column_width = 10;
        char buf[1024];
        char *c;

        printf(T_SCP());
        cursor_gotocell(active_ctx.cursor_pos_c + 1, active_ctx.cursor_pos_r + 1);
        printf("%*s", column_width, "");
        printf(T_CUB(10));
        printf(T_CUSHW());
        toggle_raw_mode();

        if (fgets(buf, sizeof buf - 1, stdin)) {
                if ((c = strchr(buf, '\n'))) *c = 0;   // trim newline
                if ((c = strchr(buf, '\r'))) *c = 0;   // trim cr
                if ((c = strchr(buf, '\t'))) *c = ' '; // change tab by space

        } else
                buf[0] = 0;

        toggle_raw_mode();
        printf(T_CUHDE() T_RCP());

        return strdup(buf);
}

void
detect_cell_type()
{
        char *buf = get_cursor_cell()->repr;
        if (*buf == '=') {
                cm_convert(get_cursor_cell(), TYPE_FORMULA);
                return;
        }
        bool is_numeric = *buf;
        while (*buf && is_numeric) {
                if (('0' <= *buf && *buf <= '9') || *buf == '.')
                        ++buf;
                else
                        is_numeric = false;
        }
        if (is_numeric) {
                cm_convert(get_cursor_cell(), TYPE_NUMBER);
                return;
        }
}

void
get_set_cell_input()
{
        char *buf = get_input_at_cursor();

        if (get_cursor_cell()->value.type == TYPE_FORMULA) {
                destroy_formula(get_cursor_cell());
        }
        free(get_cursor_cell()->repr);
        get_cursor_cell()->repr = buf;
        get_cursor_cell()->value.as.text = buf;
        get_cursor_cell()->value.type = TYPE_TEXT;
        detect_cell_type();

        for_da_each(o, get_cursor_cell()->subscribers)
        {
                cm_notify(get_cursor_cell(), *o);
        }
}

void
get_set_selection_input()
{
        char *buf = get_input_at_cursor();
        for_da_each(ca, *active_ctx.body)
        {
                for_da_each(cell, *ca)
                {
                        if (cell->selected) {
                                free(cell->repr);
                                cell->repr = strdup(buf);
                                cell->value.as.text = buf;
                                cell->value.type = TYPE_TEXT;

                                for_da_each(o, cell->subscribers)
                                {
                                        cm_notify(cell, *o);
                                }
                        }
                }
        }
        free(buf);
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
        add_action(mappings, "v", ACTION(a_select_toggle_cell));
        add_action(mappings, "i", ACTION(get_set_cell_input));
        add_action(mappings, "I", ACTION(get_set_selection_input));
        add_action(mappings, "sd", ACTION(a_set_cell_type_numeric));
        add_action(mappings, "st", ACTION(a_set_cell_type_text));
        add_action(mappings, "d", ACTION(a_set_cell_type_empty));
        add_action(mappings, "sf", ACTION(a_set_cell_type_formula));
        add_action(mappings, "J", ACTION(a_copy_moving_down));
        add_action(mappings, "K", ACTION(a_copy_moving_up));
        add_action(mappings, "H", ACTION(a_copy_moving_left));
        add_action(mappings, "L", ACTION(a_copy_moving_right));

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
                        // have to fall down (out of else-if)
                }

                if (buf[0] >= '0' && buf[0] <= '9') {
                        read_index = 0;
                        repeat *= 10;
                        repeat += buf[0] - '0';
                }

                else if (buf[read_index - 1] == '\033') {
                        get_escape_sequence();
                        read_index = 0;
                        repeat = 0;
                }

                /* Buffer contains a valid action whose prefix is unique */
                else if ((action_is_valid(action = find_action(mappings, buf, read_index)))) {
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
