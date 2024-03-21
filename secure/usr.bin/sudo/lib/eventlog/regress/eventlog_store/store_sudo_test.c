/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2020, 2023 Todd C. Miller <Todd.Miller@sudo.ws>
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
#include <sudo_lbuf.h>
#include <sudo_util.h>

sudo_dso_public int main(int argc, char *argv[]);

sudo_noreturn static void
usage(void)
{
    fprintf(stderr, "usage: %s [-cv] input_file ...\n",
	getprogname());
    exit(EXIT_FAILURE);
}

static bool
compare(FILE *fp, const char *infile, const char *output)
{
    static size_t linesize = 0;
    static char *line = NULL;
    int i;

    /* Expect two log lines, one for accept, one for exit. */
    for (i = 0; i < 2; i++) {
	ssize_t output_len = (ssize_t)strcspn(output, "\n");
	ssize_t len = getdelim(&line, &linesize, '\n', fp);
	if (len == -1) {
	    sudo_warn("getdelim");
	    return false;
	}
	if (line[len - 1] == '\n')
	    len--;

	if (len != output_len || strncmp(line, output, (size_t)len) != 0) {
	    fprintf(stderr, "%s: %s mismatch\n", infile, i ? "exit" : "accept");
	    fprintf(stderr, "expected: %.*s\n", (int)len, line);
	    fprintf(stderr, "got     : %.*s\n", (int)output_len, output);
	    return false;
	}
	if (i == 0) {
	    /* Skip past newline in accept record output. */
	    output += output_len;
	    if (output[0] != '\n' || output[1] == '\0') {
		sudo_warnx("missing exit record");
		return false;
	    }
	    output++;
	}
    }

    return true;
}

int
main(int argc, char *argv[])
{
    int ch, i, ntests = 0, errors = 0;
    struct sudo_lbuf lbuf;
    bool cat = false;

    initprogname(argc > 0 ? argv[0] : "store_sudo_test");

    while ((ch = getopt(argc, argv, "v")) != -1) {
	switch (ch) {
	    case 'c':
		cat = true;
		break;
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

    if (argc < 1)
	usage();

    sudo_lbuf_init(&lbuf, NULL, 0, NULL, 0);
    for (i = 0; i < argc; i++) {
	struct eventlog_json_object *root;
	struct eventlog *evlog = NULL;
	const char *infile = argv[i];
	const char *outfile = argv[i];
	char pathbuf[PATH_MAX];
	FILE *infp = NULL;
	FILE *outfp = NULL;
	size_t len;

	ntests++;

	/* Parse input file. */
	if ((infp = fopen(infile, "r")) == NULL) {
	    sudo_warn("%s", argv[i]);
	    errors++;
	    continue;
	}
	root = eventlog_json_read(infp, infile);
	if (root == NULL) {
	    errors++;
	    goto next;
	}

	/* Convert JSON to event log. */
	evlog = calloc(1, sizeof(*evlog));
	if (evlog == NULL) {
	    sudo_warnx("%s: %s", __func__, "unable to allocate memory");
	    errors++;
	    goto next;
	}
	if (!eventlog_json_parse(root, evlog)) {
	    errors++;
	    goto next;
	}

	/* Format event log in sudo log format. */
	if (!eventlog_store_sudo(EVLOG_ACCEPT, evlog, &lbuf)) {
	    errors++;
	    goto next;
	}
	sudo_lbuf_append(&lbuf, "\n");
	if (!eventlog_store_sudo(EVLOG_EXIT, evlog, &lbuf)) {
	    errors++;
	    goto next;
	}
	sudo_lbuf_append(&lbuf, "\n");

	/* Write the formatted output to stdout for -c (cat) */
	if (cat) {
	    puts(lbuf.buf);
	    fflush(stdout);
	}

	/* Check for a .out.ok file in the same location as the .in file. */
	len = strlen(infile);
	if (len < sizeof(".json.in")) {
	    sudo_warnx("%s must end in .json.in", infile); 
	    errors++;
	    goto next;
	}
	len -= sizeof(".json.in") - 1;
	if (strcmp(&infile[len], ".json.in") != 0) {
	    sudo_warnx("%s must end in .json.in", infile); 
	    errors++;
	    goto next;
	}
	snprintf(pathbuf, sizeof(pathbuf), "%.*s.sudo.out.ok",
	    (int)len, infile);
	if ((outfp = fopen(pathbuf, "r")) == NULL) {
	    sudo_warn("%s", pathbuf); 
	    errors++;
	    goto next;
	}

	/* Compare output to expected output. */
	if (!compare(outfp, outfile, lbuf.buf))
	    errors++;

next:
	lbuf.len = 0;
	eventlog_json_free(root);
	eventlog_free(evlog);
	if (infp != NULL)
	    fclose(infp);
	if (outfp != NULL && outfp != infp)
	    fclose(outfp);
    }
    sudo_lbuf_destroy(&lbuf);

    if (ntests != 0) {
	printf("%s: %d test%s run, %d errors, %d%% success rate\n",
	    getprogname(), ntests, ntests == 1 ? "" : "s", errors,
	    (ntests - errors) * 100 / ntests);
    }

    return errors;
}
