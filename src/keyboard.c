#include "keyboard.h"
#include "action.h"
#include "aptree.h"
#include "common.h"
#include "window.h"
#include "mappings.h"

#define MAX_MAPPING_LEN 6

static void
print_mapping_buffer(char *buf, int len, int n)
{
        print_at(1, 1, buf, len, n);
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

void
start_kbhandler()
{
        char buf[MAX_MAPPING_LEN];
        int read_index = 0;
        Action action;
        APTree mappings = ap_init();
        add_action(mappings, "q", ACTION(a_quit));

        while (read(STDIN_FILENO, buf + read_index, 1)) {
                if (buf[read_index] == '\033') {
                        read_index = 0;
                }

                else if ((action_is_valid(action = find_action(mappings, buf, read_index + 1)))) {
                        action.action();
                        read_index = 0;
                } else if ((action_is_valid(action = find_action(mappings, buf, read_index)))) {
                        action.action();
                        buf[0] = buf[read_index];
                        read_index = 1;
                } else if (++read_index == MAX_MAPPING_LEN) {
                        if ((action_is_valid(action = find_action_force(mappings, buf, read_index + 1)))) {
                                action.action();
                        }
                        read_index = 0;
                }
                print_mapping_buffer(buf, read_index, MAX_MAPPING_LEN);
                fflush(stdout);
        }
}
