/* conf.h - v1.0 - public domain lua config reader - Hugo Coto Florez 2026

   To use this library, do this in *one* C file:
      #define INCLUDE_CONF_IMPLEMENTATION
      #include "conf.h"

   In other source files, just include the header normally:
      #include "conf.h"

TABLE OF CONTENTS

  Table of Contents
  License
  Documentation
  Notes
  Credits

LICENSE

  Placed in the public domain and also MIT licensed.
  See end of file for detailed license information.

DOCUMENTATION

  The library revolves around an opaque handle 'Conf' which holds the Lua
  state associated with a loaded configuration file.

  Declare an empty configuration handle:
    Conf conf;

  You must call Conf_open() before any other function. Always call
  Conf_close() when done to release resources.

  Functions:

    Conf_open
      Opens a Lua file and returns a Conf handle.
      Supports both global-assignment and return-table style config files.
      Returns CONF_OK on success, CONF_NOTFOUND on failure.

    Conf_get_num
      Reads a numeric value from the loaded config. The path is given as a
      printf-style format string. Returns CONF_OK on success, CONF_UNDEF if a
      key is missing, CONF_INVALID if a type mismatch occurs, CONF_TRUNC if
      the formatted path exceeds the internal buffer.

    Conf_get_int
      Reads an integer value from the loaded config (rejects floats).
      Accepts a printf-style format string path.
      Returns CONF_OK on success, CONF_UNDEF if a key in the path is missing,
      CONF_INVALID if the value is a float or a non-number, CONF_TRUNC if
      the formatted path exceeds the internal buffer.

    Conf_get_str
      Reads a string value from the loaded config.
      Accepts a printf-style format string path.
      Returns CONF_OK on success, CONF_UNDEF if a key is missing, CONF_INVALID
      if a type mismatch occurs, CONF_TRUNC if the formatted path exceeds the
      internal buffer.
      The returned string is owned by Lua and is only valid until the next
      Lua API call on this Conf handle. strdup it if you need it longer.

    Conf_get_bool
      Reads a boolean value from the loaded config.
      Accepts a printf-style format string path.
      Returns CONF_OK on success, CONF_UNDEF if a key is missing, CONF_INVALID
      if a type mismatch occurs, CONF_TRUNC if the formatted path exceeds the
      internal buffer.

    Conf_get_len
      Returns the length of an array-like Lua table at the given path.
      Accepts a printf-style format string path.
      Returns CONF_OK on success, CONF_UNDEF if a key is missing, CONF_INVALID
      if the value at the path is not a table, CONF_TRUNC if the formatted
      path exceeds the internal buffer.

    Conf_close
      Releases all resources owned by Conf.
      Returns CONF_OK on success, CONF_INVALID if conf is NULL.

  Error codes (Conf_Error):

    CONF_OK       All operations completed successfully.
    CONF_UNDEF    A symbol in the path was not defined.
    CONF_NOTFOUND The Lua file could not be found or loaded.
    CONF_INVALID  A type mismatch was encountered (e.g. expected a table or
                  the final value had the wrong type).
    CONF_TRUNC    The formatted path exceeded the internal path buffer
                  (currently 1024 bytes).

  Path syntax:

    The path is given as a printf-style format string. For static paths
    use a plain string literal. For dynamic paths use %d for numeric
    indices and %s for field names.

    Example: given this Lua config --
      Table = { foo = { bar = 1 } }
      List  = { { x = 10 }, { x = 20 } }

    -- static path:
      Conf_get_int(conf, &val, "Table.foo.bar");

    -- dynamic path with an index:
      Conf_get_int(conf, &val, "List.%d.x", i);

    -- dynamic path with a field name:
      Conf_get_str(conf, &str, "List.1.%s", field);

    The path buffer is limited to 1024 bytes. If the formatted path
    exceeds this, CONF_TRUNC is returned.

NOTES

  * The Lua standard libraries are loaded automatically on Conf_open()
    unless the internal flag do_not_load_stdlib is set. This flag is
    currently not exposed through the public API.

  * This library is single-threaded and not thread-safe. Each Conf handle
    owns its own lua_State and must not be accessed concurrently.

  * The Lua stack is managed internally and left in its original state
    after each Conf_get_* call.

  * Conf_get_str returns a pointer to Lua's internal string. The pointer
    is valid until the next call to any Conf_get_* function on the same
    handle. If you need the string to live longer, strdup it.

CREDITS

  Hugo Coto Florez -- design and implementation

*/

#ifndef CONF_H_
#define CONF_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if LUA_VERSION_NUM >= 502
#define CONF_RAWLEN lua_rawlen
#else
#define CONF_RAWLEN lua_objlen
#endif

// All functions return one of the following error codes. CONF_OK (0) means success.
typedef enum Conf_Error {
        CONF_OK = 0,   // Success
        CONF_UNDEF,    // Symbol in path not defined
        CONF_NOTFOUND, // Lua file not found or could not be loaded
        CONF_INVALID,  // Type mismatch along the path
        CONF_TRUNC,    // Formatted path exceeds internal buffer
} Conf_Error;

// Opaque handle; declared as pointer to struct to avoid exposing lua_State.
typedef struct __conf *Conf;

/* API
 *
 * The 'name' parameter is a dot-separated path (e.g. "Section.Key").
 * See the DOCUMENTATION section above for details.
 */

int Conf_open(Conf *conf, const char *filename);                     // Open a Lua file and produce a Conf handle
int Conf_get_num(Conf conf, double *val, const char *fmt, ...);      // Format path, store numeric result
int Conf_get_int(Conf conf, int *val, const char *fmt, ...);         // Format path, store integer result
int Conf_get_str(Conf conf, const char **val, const char *fmt, ...); // Format path, store string result
int Conf_get_bool(Conf conf, int *val, const char *fmt, ...);        // Format path, store bool result
int Conf_get_len(Conf conf, int *len, const char *fmt, ...);         // Format path, store array length
int Conf_close(Conf conf);                                           // Release all resources held by 'conf'

#if defined(INCLUDE_CONF_IMPLEMENTATION)

struct __conf {
        lua_State *L;
        unsigned char do_not_load_stdlib : 1;
};

// Traverse a dot-separated path (supports numeric indices) and leave the final value on the Lua stack.
// Returns the Lua type of the final value on success (positive),
// or a negative error code ( -(CONF_UNDEF) or -(CONF_INVALID) ) on failure.
// On success the caller must pop the value from the stack.
static int
conf_traverse(lua_State *L, char *path)
{
        char *content = strdup(path);
        char *save    = NULL;
        char *part    = strtok_r(content, ".", &save);
        char *next    = strtok_r(NULL, ".", &save);
        int base      = lua_gettop(L);
        int ret;
        long idx;
        char *endptr;

        lua_getglobal(L, part);
        if (lua_isnil(L, -1)) {
                ret = -(CONF_UNDEF);
                goto cleanup;
        }

        while (next) {
                part = next;
                next = strtok_r(NULL, ".", &save);
                if (!lua_istable(L, -1)) {
                        ret = -(CONF_INVALID);
                        goto cleanup;
                }
                idx = strtol(part, &endptr, 10);
                if (*endptr == '\0' && endptr != part)
                        lua_rawgeti(L, -1, idx);
                else
                        lua_getfield(L, -1, part);
                lua_remove(L, -2); // pop the table, keep the field value

                if (lua_isnil(L, -1)) {
                        ret = -(CONF_UNDEF);
                        goto cleanup;
                }
        }

        ret = lua_type(L, -1);
        free(content);
        return ret;

cleanup:
        lua_settop(L, base);
        free(content);
        return ret;
}

int
Conf_open(Conf *conf, const char *filename)
{
        int base = 0, nret = 0;
        Conf c = (Conf) calloc(1, sizeof(struct __conf));
        if (c == NULL) goto err;
        *conf = c;
        c->L  = luaL_newstate();
        if (c->L == NULL) goto err;
        if (!c->do_not_load_stdlib) luaL_openlibs(c->L);
        base = lua_gettop(c->L);
        if (luaL_dofile(c->L, filename)) {
                fprintf(stderr, "conf: %s\n", lua_tostring(c->L, -1));
                lua_pop(c->L, 1);
                goto err;
        }
        /* lift entries from all returned tables to globals */
        nret = lua_gettop(c->L) - base;
        for (int i = 1; i <= nret; i++) {
                if (!lua_istable(c->L, i))
                        continue;
                lua_pushnil(c->L);
                while (lua_next(c->L, i) != 0) {
                        /* stack: table, key, value */
                        if (lua_type(c->L, -2) == LUA_TSTRING) {
                                const char *k = lua_tostring(c->L, -2);
                                lua_setglobal(c->L, k); /* pops value, sets global */
                        } else {
                                lua_pop(c->L, 1); /* pop value, keep key */
                        }
                }
        }
        lua_settop(c->L, base);
        return CONF_OK;
err:
        *conf = NULL;
        Conf_close(c);
        return CONF_NOTFOUND;
}

int
Conf_close(Conf conf)
{
        if (!conf) return CONF_INVALID;
        if (conf->L) lua_close(conf->L);
        free(conf);
        return CONF_OK;
}

int
Conf_get_num(Conf conf, double *val, const char *fmt, ...)
{
        char path[1024];
        int n;
        va_list ap;
        va_start(ap, fmt);
        n = vsnprintf(path, sizeof(path), fmt, ap);
        va_end(ap);
        if (n < 0 || (size_t) n >= sizeof(path))
                return CONF_TRUNC;

        int type = conf_traverse(conf->L, path);
        if (type < 0) return -type;
        if (type != LUA_TNUMBER) {
                lua_pop(conf->L, 1);
                return CONF_INVALID;
        }
        *val = lua_tonumber(conf->L, -1);
        lua_pop(conf->L, 1);
        return CONF_OK;
}

int
Conf_get_str(Conf conf, const char **val, const char *fmt, ...)
{
        char path[1024];
        int n;
        va_list ap;
        va_start(ap, fmt);
        n = vsnprintf(path, sizeof(path), fmt, ap);
        va_end(ap);
        if (n < 0 || (size_t) n >= sizeof(path))
                return CONF_TRUNC;

        int type = conf_traverse(conf->L, path);
        if (type < 0) return -type;
        if (type != LUA_TSTRING) {
                lua_pop(conf->L, 1);
                return CONF_INVALID;
        }
        *val = lua_tostring(conf->L, -1);
        lua_pop(conf->L, 1);
        return CONF_OK;
}

int
Conf_get_bool(Conf conf, int *val, const char *fmt, ...)
{
        char path[1024];
        int n;
        va_list ap;
        va_start(ap, fmt);
        n = vsnprintf(path, sizeof(path), fmt, ap);
        va_end(ap);
        if (n < 0 || (size_t) n >= sizeof(path))
                return CONF_TRUNC;

        int type = conf_traverse(conf->L, path);
        if (type < 0) return -type;
        if (type != LUA_TBOOLEAN) {
                lua_pop(conf->L, 1);
                return CONF_INVALID;
        }
        *val = lua_toboolean(conf->L, -1);
        lua_pop(conf->L, 1);
        return CONF_OK;
}

int
Conf_get_int(Conf conf, int *val, const char *fmt, ...)
{
        char path[1024];
        int n;
        va_list ap;
        va_start(ap, fmt);
        n = vsnprintf(path, sizeof(path), fmt, ap);
        va_end(ap);
        if (n < 0 || (size_t) n >= sizeof(path))
                return CONF_TRUNC;

        int type = conf_traverse(conf->L, path);
        if (type < 0) return -type;
        if (type != LUA_TNUMBER) {
                lua_pop(conf->L, 1);
                return CONF_INVALID;
        }
        double __v = lua_tonumber(conf->L, -1);
        if (__v != (double) (int) __v) {
                lua_pop(conf->L, 1);
                return CONF_INVALID;
        }
        *val = (int) __v;
        lua_pop(conf->L, 1);
        return CONF_OK;
}

int
Conf_get_len(Conf conf, int *len, const char *fmt, ...)
{
        char path[1024];
        int n;
        va_list ap;
        va_start(ap, fmt);
        n = vsnprintf(path, sizeof(path), fmt, ap);
        va_end(ap);
        if (n < 0 || (size_t) n >= sizeof(path))
                return CONF_TRUNC;

        int type = conf_traverse(conf->L, path);
        if (type < 0) return -type;
        if (type != LUA_TTABLE) {
                lua_pop(conf->L, 1);
                return CONF_INVALID;
        }
        *len = (int) CONF_RAWLEN(conf->L, -1);
        lua_pop(conf->L, 1);
        return CONF_OK;
}

#endif

#ifdef __cplusplus
}
#endif

#endif /* CONF_H_ */

/*
-----------------------------------------------------------------------------
This software is available under 2 licenses -- choose whichever you prefer.
-----------------------------------------------------------------------------
ALTERNATIVE A - MIT License
Copyright (c) 2026 Hugo Coto Florez
Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
-----------------------------------------------------------------------------
ALTERNATIVE B - Public Domain (www.unlicense.org)
This is free and unencumbered software released into the public domain.
Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
software, either in source code form or as a compiled binary, for any purpose,
commercial or non-commercial, and by any means.
In jurisdictions that recognize copyright laws, the author or authors of this
software dedicate any and all copyright interest in the software to the public
domain. We make this dedication for the benefit of the public at large and to
the detriment of our heirs and successors. We intend this dedication to be an
overt act of relinquishment in perpetuity of all present and future rights to
this software under copyright law.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
-----------------------------------------------------------------------------
*/
