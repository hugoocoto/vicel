#include "keyboard.h"
#include "action.h"
#include "aptree.h"
#include "cellmap.h"
#include "common.h"
#include "escape_code.h"
#include "mappings.h"
#include "window.h"
#include <stdio.h>
#include <string.h>

#define MAX_MAPPING_LEN 6

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
                enabled = false;
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
        enabled = true;
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
        cursor_gotocell(active_ctx.cursor_pos_y, active_ctx.cursor_pos_x);
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
get_set_cell_input()
{
        char *buf = get_input_at_cursor();
        free(get_cursor_cell()->repr);
        get_cursor_cell()->repr = buf;
        get_cursor_cell()->as.text = buf;
        get_cursor_cell()->type = TYPE_TEXT;
        render();
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
                                cell->as.text = buf;
                                cell->type = TYPE_TEXT;
                        }
                }
        }
        free(buf);
        render();
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

        add_action(mappings, "q", ACTION(a_quit));
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

        print_mapping_buffer("", 0, MAX_MAPPING_LEN, repeat);
        toggle_raw_mode();
        render();

        while (read(STDIN_FILENO, buf + read_index, 1)) {
                ++read_index;

                if (buf[0] == '.') {
                        strcpy(buf, saved_buf);
                        read_index = saved_read_index;
                        repeat = saved_repeat;
                }

                if (buf[0] >= '0' && buf[0] <= '9') {
                        read_index = 0;
                        repeat *= 10;
                        repeat += buf[0] - '0';
                }

                /* Buffer contains a valid action whose prefix is unique */
                else if ((action_is_valid(action = find_action(mappings, buf, read_index)))) {
                        strcpy(saved_buf, buf);
                        saved_read_index = read_index;
                        saved_repeat = repeat;
                        read_index = 0;
                        do {
                                action.action();
                        } while (--repeat > 0);
                        repeat = 0;
                }
                /* Buffer contains an invalid action, but the previous buffered action was valid */
                // Todo
                /* Buffer is full */
                else if (read_index == MAX_MAPPING_LEN) {
                        /* Get the action whose prefix is in buffer with or without shared prefix */
                        if ((action_is_valid(action = find_action_force(mappings, buf, read_index)))) {
                                strcpy(saved_buf, buf);
                                saved_read_index = read_index;
                                saved_repeat = repeat;
                                do {
                                        action.action();
                                } while (--repeat > 0);
                                repeat = 0;
                        }
                        read_index = 0;
                }
                /* If buffer has no actions and no descents, invalidate buffer */
                else if (!has_descents(mappings, buf, read_index)) {
                        read_index = 0;
                        repeat = 0;
                }
                /* Clear buffer if pressing esc */
                else if (buf[read_index - 1] == '\033') {
                        read_index = 0;
                        repeat = 0;
                }

                print_mapping_buffer(buf, read_index, MAX_MAPPING_LEN, repeat);
                fflush(stdout);
        }

        toggle_raw_mode();
}
