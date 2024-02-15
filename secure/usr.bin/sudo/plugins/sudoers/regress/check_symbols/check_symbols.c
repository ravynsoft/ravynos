/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2012-2015 Todd C. Miller <Todd.Miller@sudo.ws>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#include <sudo_compat.h>
#include <sudo_dso.h>
#include <sudo_util.h>
#include <sudo_fatal.h>

sudo_dso_public int main(int argc, char *argv[]);

sudo_noreturn static void
usage(void)
{
    fprintf(stderr, "usage: %s [-v] plugin.so symbols_file\n", getprogname());
    exit(EXIT_FAILURE);
}

int
main(int argc, char *argv[])
{
    void *handle, *sym;
    const char *plugin_path;
    const char *symbols_file;
    char *cp, line[LINE_MAX];
    FILE *fp;
    int ch, ntests = 0, errors = 0;

    initprogname(argc > 0 ? argv[0] : "check_symbols");

    while ((ch = getopt(argc, argv, "v")) != -1) {
	switch (ch) {
	case 'v':
	    /* ignored */
	    break;
	default:
	    usage();
	    /* NOTREACHED */
	}
    }
    argc -= optind;
    argv += optind;

    if (argc != 2)
	usage();
    plugin_path = argv[0];
    symbols_file = argv[1];

    handle = sudo_dso_load(plugin_path, SUDO_DSO_LAZY|SUDO_DSO_GLOBAL);
    if (handle == NULL) {
	const char *errstr = sudo_dso_strerror();
	sudo_fatalx_nodebug("unable to load %s: %s", plugin_path,
	    errstr ? errstr : "unknown error");
    }

    fp = fopen(symbols_file, "r");
    if (fp == NULL)
	sudo_fatal_nodebug("unable to open %s", symbols_file);

    while (fgets(line, sizeof(line), fp) != NULL) {
	ntests++;
	if ((cp = strchr(line, '\n')) != NULL)
	    *cp = '\0';
	sym = sudo_dso_findsym(handle, line);
	if (sym == NULL) {
	    const char *errstr = sudo_dso_strerror();
	    printf("%s: test %d: unable to resolve symbol %s: %s\n",
		getprogname(), ntests, line, errstr ? errstr : "unknown error");
	    errors++;
	}
    }

    /*
     * Make sure unexported symbols are not available.
     */
    ntests++;
    sym = sudo_dso_findsym(handle, "user_in_group");
    if (sym != NULL) {
	printf("%s: test %d: able to resolve local symbol user_in_group\n",
	    getprogname(), ntests);
	errors++;
    }

    sudo_dso_unload(handle);

    if (ntests != 0) {
	printf("%s: %d tests run, %d errors, %d%% success rate\n",
	    getprogname(), ntests, errors, (ntests - errors) * 100 / ntests);
    }

    exit(errors);
}
