#ifndef ACTION_H
#define ACTION_H

typedef struct Action {
        void (*action)(void);
} Action;

#define NoAction ((struct Action) { \
.action = NULL,              \
})

#endif // !ACTION_H
