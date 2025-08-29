#include <stdbool.h>
#include <stdio.h>

void vspl_start();
bool vspl_parse(FILE *file);
bool vspl_get_int(char *name, int *value);
bool vspl_get_str(char *name, char **value);
void vspl_end();

void vspl_addint(char *name, int value);
void vspl_addstr(char *name, char *value);
void vspl_dump_env();

#define vspl_addvar(name, v) _Generic((v), \
char *: vspl_addstr,                       \
const char *: vspl_addstr,                 \
int: vspl_addint,                          \
bool: vspl_addint)(name, v)
