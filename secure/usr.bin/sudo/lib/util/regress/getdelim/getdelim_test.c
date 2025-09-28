/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2019-2020 Todd C. Miller <Todd.Miller@sudo.ws>
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

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_STDBOOL_H
# include <stdbool.h>
#else
# include <compat/stdbool.h>
#endif
#include <limits.h>
#include <unistd.h>

#include <sudo_compat.h>
#include <sudo_fatal.h>
#include <sudo_util.h>

sudo_dso_public int main(int argc, char *argv[]);

ssize_t sudo_getdelim(char ** restrict bufp, size_t * restrict bufsizep,
    int delim, FILE * restrict fp);

/*
 * Test that sudo_getdelim() works as expected.
 */

struct getdelim_test {
    const char *input;
    const char *output[4];
    int delim;
};

static char longstr[LINE_MAX * 4];
static struct getdelim_test test_data[] = {
    { "a\nb\nc\n", { "a\n", "b\n", "c\n", NULL }, '\n' },
    { "a\nb\nc", { "a\n", "b\n", "c", NULL }, '\n' },
    { "a\tb\tc\t", { "a\t", "b\t", "c\t", NULL }, '\t' },
    { "a\tb\tc", { "a\t", "b\t", "c", NULL }, '\t' },
    { longstr, { longstr, NULL }, '\n' },
    { NULL, { NULL }, '\0' }
};

static int errors = 0, ntests = 0;

static void
runtests(char **buf, size_t *buflen)
{
    int i, j, sv[2];
    pid_t pid;
    FILE *fp;

    /* Exercise realloc case by injecting an entry > LINE_MAX. */
    memset(longstr, 'A', sizeof(longstr) - 2);
    longstr[sizeof(longstr) - 2] = '\n';
    longstr[sizeof(longstr) - 1] = '\0';

    for (i = 0; test_data[i].input != NULL; i++) {
	if (socketpair(PF_UNIX, SOCK_STREAM, 0, sv) == -1)
	    sudo_fatal_nodebug("socketpair");

	switch ((pid = fork())) {
	case -1:
	    sudo_fatal_nodebug("fork");
	case 0:
	    /* child */
	    close(sv[0]);
	    if (send(sv[1], test_data[i].input, strlen(test_data[i].input), 0) == -1) {
		sudo_warn_nodebug("send");
		_exit(127);
	    }
	    _exit(EXIT_SUCCESS);
	    break;
	default:
	    /* parent */
	    break;
	}

	close(sv[1]);
	if ((fp = fdopen(sv[0], "r")) == NULL)
	    sudo_fatal_nodebug("fdopen");

	for (j = 0; test_data[i].output[j] != NULL; j++) {
	    ntests++;
	    alarm(10);
	    if (sudo_getdelim(buf, buflen, test_data[i].delim, fp) == -1)
		sudo_fatal_nodebug("sudo_getdelim");
	    alarm(0);
	    if (strcmp(*buf, test_data[i].output[j]) != 0) {
		sudo_warnx_nodebug("failed test #%d: expected %s, got %s",
		    ntests, test_data[i].output[j], *buf);
		errors++;
	    }
	}

	/* test EOF */
	ntests++;
	alarm(30);
	if (sudo_getdelim(buf, buflen, test_data[i].delim, fp) != -1) {
	    sudo_warnx_nodebug("failed test #%d: expected EOF, got %s",
		ntests, *buf);
	    errors++;
	} else {
	    if (!feof(fp)) {
		sudo_warn_nodebug("failed test #%d: expected EOF, got error",
		    ntests);
		errors++;
	    }
	}

	/* test error by closing the underlying fd. */
	clearerr(fp);
	close(fileno(fp));
	ntests++;
	alarm(30);
	if (sudo_getdelim(buf, buflen, test_data[i].delim, fp) != -1) {
	    sudo_warnx_nodebug("failed test #%d: expected error, got %s",
		ntests, *buf);
	    errors++;
	} else {
	    /* Use feof(3), not ferror(3) so we can detect out of memory. */
	    if (feof(fp)) {
		sudo_warn_nodebug("failed test #%d: expected error, got EOF",
		    ntests);
		errors++;
	    }
	}

	fclose(fp);
	waitpid(pid, NULL, 0);
	alarm(0);
    }
}

int
main(int argc, char *argv[])
{
    size_t buflen = 0;
    char *buf = NULL;
    int ch;

    initprogname(argc > 0 ? argv[0] : "getdelim_test");

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

    runtests(&buf, &buflen);
    free(buf);

    /* XXX - redo tests with preallocated buffer filled with junk */
    if (ntests != 0) {
	printf("%s: %d tests run, %d errors, %d%% success rate\n",
	    getprogname(), ntests, errors, (ntests - errors) * 100 / ntests);
    }
    return errors;
}
