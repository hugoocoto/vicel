#include "common.h"
#include "action.h"

#define MAX_MAPPING_LEN 6


// void
// start_kbhandler()
// {
//         char buf[MAX_MAPPING_LEN];
//         int read_index = 0;
//         Action action;
//
//         while (read(STDIN_FILENO, buf + read_index, 1)) {
//                 if ((action = find_action(buf, read_index + 1))) {
//                         action.action();
//                         read_index = 0;
//                 }
//                 if (++read_index == MAX_MAPPING_LEN) {
//                         if ((action = find_action_force(buf, read_index + 1))) {
//                                 action.action();
//                         }
//                         read_index = 0;
//                 }
//         }
// }
