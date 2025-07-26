#ifndef ACTION_H
#define ACTION_H

#include <stdlib.h>
typedef struct Action {
        void (*action)(void);
} Action;

#define NoAction ((struct Action) { \
.action = NULL,                     \
})

#define action_is_valid(a) ((a).action != NULL)

#define ACTION(f) ((struct Action) { .action = (f) })

#endif // !ACTION_H
