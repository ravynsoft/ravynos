/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2011-2013 Todd C. Miller <Todd.Miller@sudo.ws>
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
#include <limits.h>
#include <unistd.h>

#define SUDO_ERROR_WRAP 0

#include <sudo_compat.h>
#include <sudo_eventlog.h>
#include <sudo_fatal.h>
#include <sudo_plugin.h>
#include <sudo_util.h>

sudo_dso_public int main(int argc, char *argv[]);

sudo_noreturn static void
usage(void)
{
    fprintf(stderr, "usage: %s [-v] inputfile\n", getprogname());
    exit(EXIT_FAILURE);
}

int
main(int argc, char *argv[])
{
    int ch, lineno = 0, which = 0;
    char *line, lines[2][2048];
    const char *infile;
    unsigned int len;
    FILE *fp;

    initprogname(argc > 0 ? argv[0] : "check_wrap");

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

    if (argc != 1)
	usage();
    infile = argv[0];

    fp = fopen(infile, "r");
    if (fp == NULL)
	sudo_fatalx("unable to open %s", infile);

    /*
     * Each test record consists of a log entry on one line and a list of
     * line lengths to test it with on the next.  E.g.
     *
     * Jun 30 14:49:51 : millert : TTY=ttypn ; PWD=/usr/src/local/millert/hg/sudo/trunk/plugins/sudoers ; USER=root ; TSID=0004LD ; COMMAND=/usr/local/sbin/visudo
     * 60-80,40
     */
    while ((line = fgets(lines[which], sizeof(lines[which]), fp)) != NULL) {
	char *cp, *last;

	line[strcspn(line, "\n")] = '\0';

	/* If we read the 2nd line, parse list of line lengths and check. */
	if (which) {
	    lineno++;
	    for (cp = strtok_r(lines[1], ",", &last); cp != NULL; cp = strtok_r(NULL, ",", &last)) {
		unsigned int maxlen;
		const char *errstr;
		char *dash;

		/* May be either a number or a range. */
		dash = strchr(cp, '-');
		if (dash != NULL) {
		    *dash = '\0';
		    len = (unsigned int)sudo_strtonum(cp, 0, INT_MAX, &errstr);
		    if (errstr == NULL)
			maxlen = (unsigned int)sudo_strtonum(dash + 1, 0, INT_MAX, &errstr);
		} else {
		    len = maxlen = (unsigned int)sudo_strtonum(cp, 0, INT_MAX, &errstr);
		}
		if (errstr != NULL) {
		    sudo_fatalx("%s: invalid length on line %d", infile, lineno);
		}
		while (len <= maxlen) {
		    if (len == 0) {
			puts("# word wrap disabled");
		    } else {
			printf("# word wrap at %u characters\n", len);
		    }
		    eventlog_writeln(stdout, lines[0], strlen(lines[0]), len);
		    len++;
		}
	    }
	}
	which = !which;
    }

    return EXIT_SUCCESS;
}
