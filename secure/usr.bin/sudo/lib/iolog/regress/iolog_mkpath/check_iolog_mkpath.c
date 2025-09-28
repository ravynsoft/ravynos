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

#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define SUDO_ERROR_WRAP 0

#include <sudo_compat.h>
#include <sudo_util.h>
#include <sudo_fatal.h>
#include <sudo_iolog.h>

sudo_dso_public int main(int argc, char *argv[]);

static const char *test_paths[] = {
    "testdir/a/b/c/user",		/* create new */
    "testdir/a/b/c/user",		/* open existing */
    "testdir/a/b/c/user.XXXXXX",	/* mkdtemp new */
    NULL
};

static void
test_iolog_mkpath(const char *testdir, int *ntests, int *nerrors)
{
    const char **tp;
    char *path;

    iolog_set_owner(geteuid(), getegid());

    for (tp = test_paths; *tp != NULL; tp++) {
	if (asprintf(&path, "%s/%s", testdir, *tp) == -1)
	    sudo_fatalx("unable to allocate memory");

	(*ntests)++;
	if (!iolog_mkpath(path)) {
	    sudo_warnx("unable to mkpath %s", path);
	    (*nerrors)++;
	}
	free(path);
    }
}

int
main(int argc, char *argv[])
{
    char testdir[] = "mkpath.XXXXXX";
    const char *rmargs[] = { "rm", "-rf", NULL, NULL };
    int ch, status, ntests = 0, errors = 0;

    initprogname(argc > 0 ? argv[0] : "check_iolog_mkpath");

    while ((ch = getopt(argc, argv, "v")) != -1) {
	switch (ch) {
	case 'v':
	    /* ignore */
	    break;
	default:
	    fprintf(stderr, "usage: %s [-v]\n", getprogname());
	    return EXIT_FAILURE;
	}
    }
    argc -= optind;
    argv += optind;

    if (mkdtemp(testdir) == NULL)
	sudo_fatal("unable to create test dir");
    rmargs[2] = testdir;

    test_iolog_mkpath(testdir, &ntests, &errors);

    if (ntests != 0) {
	printf("iolog_mkpath: %d test%s run, %d errors, %d%% success rate\n",
	    ntests, ntests == 1 ? "" : "s", errors,
	    (ntests - errors) * 100 / ntests);
    }

    /* Clean up (avoid running via shell) */
    switch (fork()) {
    case -1:
	sudo_warn("fork");
	_exit(1);
    case 0:
	execvp("rm", (char **)rmargs);
	_exit(1);
    default:
	wait(&status);
	if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
	    errors++;
	break;
    }

    return errors;
}
