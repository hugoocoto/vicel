#ifndef ACTION_H
#define ACTION_H

typedef struct Action {
        void (*action)(void);
} Action;

#endif // !ACTION_H
