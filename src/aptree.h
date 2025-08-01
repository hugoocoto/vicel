#ifndef APTREE_H
#define APTREE_H

#include "action.h"
#include "common.h"

#define AP_ENTRIES 127

typedef struct __APTree {
        Action action;
        struct __APTree *after[AP_ENTRIES]; // can be reduced
        int descents;
} *APTree;

APTree ap_init();
void ap_add(APTree t, char *prefix, Action action);
Action ap_get(APTree t, char *prefix);
Action ap_getl(APTree t, char *prefix, int len);
Action ap_get_last(APTree t, char *prefix);
Action ap_getl_last(APTree t, char *prefix, int len);
Action ap_get_unique(APTree t, char *prefix);
Action ap_getl_unique(APTree t, char *prefix, int len);
bool ap_has_descents(APTree t, char *prefix);
bool ap_has_descentsl(APTree t, char *prefix, int len);
void ap_remove(APTree t, char *prefix);
void ap_destroy(APTree t);


#endif //!APTREE_H
