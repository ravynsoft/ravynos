/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2020 Todd C. Miller <Todd.Miller@sudo.ws>
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

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define SUDO_ERROR_WRAP 0

#include <sudoers.h>

#include <def_data.c>

struct test_data {
    const char *input;
    const char *output;
    const char *user;
    bool result;
} test_data[] = {
    { "foo/bar", NULL, NULL, false },
    { "~root", "/", NULL, true },
    { "~", "/home/millert", "millert", true },
    { "~/foo", "/home/millert/foo", "millert", true },
    { "~millert", "/home/millert", "millert", true },
    { "~millert/bar", "/home/millert/bar", "millert", true },
    { NULL }
};

sudo_dso_public int main(int argc, char *argv[]);

int
main(int argc, char *argv[])
{
    int ch, ntests = 0, errors = 0;
    struct test_data *td;
    struct passwd *pw;
    char *path = NULL;
    bool result;

    initprogname(argc > 0 ? argv[0] : "check_exptilde");

    while ((ch = getopt(argc, argv, "v")) != -1) {
	switch (ch) {
	case 'v':
	    /* ignored */
	    break;
	default:
	    fprintf(stderr, "usage: %s [-v]\n", getprogname());
	    return EXIT_FAILURE;
	}
    }
    argc -= optind;
    argv += optind;

    /* Prime the passwd cache */
    pw = sudo_mkpwent("root", 0, 0, "/", "/bin/sh");
    if (pw == NULL)
	sudo_fatalx("unable to create passwd entry for root");
    sudo_pw_delref(pw);

    pw = sudo_mkpwent("millert", 8036, 20, "/home/millert", "/bin/tcsh");
    if (pw == NULL)
	sudo_fatalx("unable to create passwd entry for millert");
    sudo_pw_delref(pw);

    for (td = test_data; td->input != NULL; td++) {
	ntests++;
	if ((path = strdup(td->input)) == NULL)
	    sudo_fatal(NULL);
	result = expand_tilde(&path, td->user);
	if (result != td->result) {
	    errors++;
	    if (result) {
		sudo_warnx("unexpected success: input %s, output %s", 
		    td->input, path);
	    } else {
		sudo_warnx("unexpected failure: input %s", td->input);
	    }
	} else if (td->result && strcmp(path, td->output) != 0) {
	    errors++;
	    sudo_warnx("incorrect output for input %s: expected %s, got %s",
		td->input, td->output, path);
	}
	free(path);
    }

    if (ntests != 0) {
	printf("%s: %d tests run, %d errors, %d%% success rate\n",
	    getprogname(), ntests, errors, (ntests - errors) * 100 / ntests);
    }

    return errors;
}
