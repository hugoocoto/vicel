/*
 * VICEL - Visual Cell editor
 * Copyright (C) 2025  Hugo Coto Florez
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * For questions or support, contact: hugo.coto@member.fsf.org
 */

#include "options.h"
#include "common.h"
#include "debug.h"
#include "toml.h"
#include <cstring>

Win_opts win_opts = {
        /* defaults */
        .num_col_width = 5,
        .col_width = 14,
        .use_cell_color_for_sep = true,
        .cell_l_sep = " ",
        .cell_r_sep = " ",
};

void
get_window_options(toml_table_t *tbl)
{
        toml_value_t v;

        if ((v = toml_table_string(tbl, "cell_l_sep")).ok)
                win_opts.cell_l_sep = v.u.s;
        if ((v = toml_table_string(tbl, "cell_r_sep")).ok)
                win_opts.cell_r_sep = v.u.s;
        if ((v = toml_table_int(tbl, "num_col_width")).ok)
                win_opts.num_col_width = v.u.i;
        if ((v = toml_table_int(tbl, "col_width")).ok)
                win_opts.col_width = v.u.i;
        if ((v = toml_table_int(tbl, "use_cell_color_for_sep")).ok)
                win_opts.use_cell_color_for_sep = v.u.b;
}

void
get_options(toml_table_t *tbl)
{
        toml_table_t *win = toml_table_table(tbl, "window");
        if (win) get_window_options(win);
}

void
parse_options(char *content)
{
        char errbuf[1280];
        toml_table_t *tbl = toml_parse(content, errbuf, sizeof errbuf);
        if (!tbl) {
                report("Toml error: %s", errbuf);
                return;
        }
        get_options(tbl);
}

void
parse_options_file(FILE *f)
{
        if (f == NULL) return;

        char buf[1024];
        size_t n;

        while ((n = fread(buf, 1, sizeof buf - 1, f))) {
                buf[n] = 0;
                parse_options(buf);
        }
}

/* unsafe */
char *
path_join(char *base, ...)
{
        va_list v;
        char *c;
        va_start(v, base);
        while ((c = va_arg(v, char *))) {
                strcat(base, c);
        }
        *base = 0;
        return base;
}

#define pjoin(base, ...) path_join((base), ##__VA_ARGS__, NULL)

void
parse_options_default_file()
{
        char path[128];
        char *home = getenv("HOME") ?: "";
        parse_options_file(fopen(pjoin(path, "vicel.toml"), "r"));
        parse_options_file(fopen(pjoin(path, home, "vicel.toml"), "r"));
        parse_options_file(fopen(pjoin(path, home, ".config/vicel.toml"), "r"));
        parse_options_file(fopen(pjoin(path, home, ".config/vicel/vicel.toml"), "r"));
}

// Example:
// #include "../header/toml-c.h"
//
// char *doc = "\n"
// 	"host = 'example.com'\n"
// 	"port = 80\n"
// 	"\n"
// 	"[tbl]\n"
// 	"key = 'value'\n"
// 	"[tbl.sub]\n"
// 	"subkey = 'subvalue'\n";
//
// int main(void) {
// 	char errbuf[200];
// 	toml_table_t *tbl = toml_parse(doc, errbuf, sizeof(errbuf));
// 	if (!tbl) {
// 		fprintf(stderr, "ERROR: %s\n", errbuf);
// 		exit(1);
// 	}
//
// 	// Get specific keys.
// 	toml_value_t host = toml_table_string(tbl, "host");
// 	toml_value_t port = toml_table_int(tbl, "port");
// 	if (!host.ok) // Default values.
// 		host.u.s = "localhost";
// 	if (!port.ok)
// 		host.u.i = 80;
// 	printf("%s:%ld\n", host.u.s, port.u.i);
//
// 	// Get a table.
// 	toml_table_t *sub_tbl = toml_table_table(tbl, "tbl");
// 	if (sub_tbl) {
// 		// Loop over all keys in a table.
// 		int l = toml_table_len(sub_tbl);
// 		for (int i = 0; i < l; i++) {
// 			int keylen;
// 			const char *key = toml_table_key(sub_tbl, i, &keylen);
// 			printf("key #%d: %s\n", i, key);
// 			// TODO: this should return toml_key_t or something, which also
// 			// includes the type. This actually requires a bit of frobbing with
// 			// the lexer, as that just sets the type of everything to STRING.
// 			//
// 			// Then we can also get rid of toml_table_{string,int,...} and just
// 			// parse it automatically.
// 		}
// 	}
// }
