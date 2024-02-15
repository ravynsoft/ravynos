/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2011-2016 Todd C. Miller <Todd.Miller@sudo.ws>
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
#ifdef HAVE_STDBOOL_H
# include <stdbool.h>
#else
# include <compat/stdbool.h>
#endif /* HAVE_STDBOOL_H */
#include <string.h>

#define SUDO_ERROR_WRAP 0

#include <sudo_compat.h>
#include <sudo_queue.h>
#include <parse.h>
#include <toke.h>
#include <sudo_plugin.h>
#include <sudo_util.h>
#include <gram.h>

sudo_dso_public int main(int argc, char *argv[]);

/*
 * TODO: test realloc
 */

YYSTYPE sudoerslval;

struct fill_test {
    const char *input;
    const char *output;
    int len;
    bool addspace;
};

/*
 * In "normal" fill, anything can be escaped and hex chars are expanded.
 */
static struct fill_test txt_data[] = {
    { "Embedded\\x20Space", "Embedded Space", 0 },
    { "\\x20Leading", " Leading", 0 },
    { "Trailing\\x20", "Trailing ", 0 },
    { "Multiple\\x20\\x20Spaces", "Multiple  Spaces", 0 },
    { "Hexparse\\x200Check", "Hexparse 0Check", 0 },
    { "Escaped\\\\Escape", "Escaped\\Escape", 0 },
    { "LongGroupName", "LongGrou", 8 }
};

/*
 * The only escaped chars in a command should be [,:= \t#]
 * The rest are done by glob() or fnmatch().
 */
static struct fill_test cmd_data[] = {
    { "foo\\,bar", "foo,bar", 0 },
    { "this\\:that", "this:that", 0 },
    { "foo\\=bar", "foo=bar", 0 },
    { "tab\\\tstop", "tab\tstop", 0 },
    { "not a \\#comment", "not a #comment", 0 }
};

/*
 * No escaped characters in command line args.
 * Arguments get appended.
 */
static struct fill_test args_data[] = {
    { "/", "/", 0, false },
    { "-type", "/ -type", 0, true },
    { "f", "/ -type f", 0, true },
    { "-exec", "/ -type f -exec", 0, true },
    { "ls", "/ -type f -exec ls", 0, true },
    { "{}", "/ -type f -exec ls {}", 0, true }
};

static int
check_fill(const char *input, int len, bool addspace, const char *expect, char **resultp)
{
    if (sudoerslval.string != NULL) {
	free(sudoerslval.string);
	sudoerslval.string = NULL;
    }
    if (!fill(input, len))
	return -1;
    *resultp = sudoerslval.string;
    return !strcmp(sudoerslval.string, expect);
}

static int
check_fill_cmnd(const char *input, int len, bool addspace, const char *expect, char **resultp)
{
    if (sudoerslval.command.cmnd != NULL) {
	free(sudoerslval.command.cmnd);
	sudoerslval.command.cmnd = NULL;
    }
    if (!fill_cmnd(input, len))
	return -1;
    *resultp = sudoerslval.command.cmnd;
    return !strcmp(sudoerslval.command.cmnd, expect);
}

static int
check_fill_args(const char *input, int len, bool addspace, const char *expect, char **resultp)
{
    /* Must not free old sudoerslval.command.args as gets appended to. */
    if (!fill_args(input, len, addspace))
	return -1;
    *resultp = sudoerslval.command.args;
    return !strcmp(sudoerslval.command.args, expect);
}

static int
do_tests(int (*checker)(const char *, int, bool, const char *, char **),
    struct fill_test *data, size_t ntests)
{
    int errors = 0;
    unsigned int i;
    int len;
    char *result;

    for (i = 0; i < ntests; i++) {
	if (data[i].len == 0)
	    len = (int)strlen(data[i].input);
	else
	    len = data[i].len;

	switch ((*checker)(data[i].input, len, data[i].addspace, data[i].output, &result)) {
	case 0:
	    /* no match */
	    fprintf(stderr, "Failed parsing %.*s: expected [%s], got [%s]\n",
		(int)data[i].len, data[i].input, data[i].output, result);
	    errors++;
	    break;
	case 1:
	    /* match */
	    break;
	default:
	    /* error */
	    fprintf(stderr, "Failed parsing %.*s: fill function failure\n",
		(int)data[i].len, data[i].input);
	    errors++;
	    break;
	}
    }

    return errors;
}

int
main(int argc, char *argv[])
{
    int ch, ntests, errors = 0;

    initprogname(argc > 0 ? argv[0] : "check_fill");

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

    errors += do_tests(check_fill, txt_data, nitems(txt_data));
    errors += do_tests(check_fill_cmnd, cmd_data, nitems(cmd_data));
    errors += do_tests(check_fill_args, args_data, nitems(args_data));

    ntests = nitems(txt_data) + nitems(cmd_data) + nitems(args_data);
    if (ntests != 0) {
	printf("%s: %d tests run, %d errors, %d%% success rate\n",
	    getprogname(), ntests, errors, (ntests - errors) * 100 / ntests);
    }

    return errors;
}

/* STUB */
void
sudoerserror(const char *s)
{
    return;
}

/* STUB */
bool
sudoers_strict(void)
{
    return false;
}

/* STUB */
bool
parser_leak_add(enum parser_leak_types type, void *v)
{
    return true;
}

/* STUB */
bool
parser_leak_remove(enum parser_leak_types type, void *v)
{
    return true;
}
