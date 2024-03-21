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
#include <sudo_util.h>

#include <parse_json.h>

sudo_dso_public int main(int argc, char *argv[]);

sudo_noreturn static void
usage(void)
{
    fprintf(stderr, "usage: %s [-cv] input_file ...\n",
	getprogname());
    exit(EXIT_FAILURE);
}

static bool
compare(FILE *fp, const char *infile, struct json_container *jsonc)
{
    const char *cp;
    unsigned int lineno = 0;
    size_t linesize = 0;
    char *line = NULL;
    ssize_t len;

    cp = sudo_json_get_buf(jsonc);

    while ((len = getdelim(&line, &linesize, '\n', fp)) != -1) {
	lineno++;

	/* skip open/close brace, not present in formatted output */
	if (lineno == 1 && strcmp(line, "{\n") == 0)
	    continue;
	if (*cp == '\0' && strcmp(line, "}\n") == 0)
	    continue;

	/* Ignore newlines in output to make comparison easier. */
	if (*cp == '\n')
	    cp++;
	if (line[len - 1] == '\n')
	    len--;

	if (strncmp(line, cp, (size_t)len) != 0) {
	    fprintf(stderr, "%s: mismatch on line %u\n", infile, lineno);
	    fprintf(stderr, "expected: %s", line);
	    fprintf(stderr, "got     : %.*s\n", (int)len, cp);
	    return false;
	}
	cp += len;
    }
    free(line);

    return true;
}

int
main(int argc, char *argv[])
{
    int ch, i, ntests = 0, errors = 0;
    bool cat = false;

    initprogname(argc > 0 ? argv[0] : "store_json_test");

    while ((ch = getopt(argc, argv, "cv")) != -1) {
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

    for (i = 0; i < argc; i++) {
	struct eventlog_json_object *root;
	struct eventlog *evlog = NULL;
	struct json_container jsonc;
	const char *infile = argv[i];
	const char *outfile = argv[i];
	const char *cp;
	char pathbuf[PATH_MAX];
	FILE *infp = NULL;
	FILE *outfp = NULL;

	ntests++;

	if (!sudo_json_init(&jsonc, 4, false, true, true)) {
	    errors++;
	    continue;
	}

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

	/* Format event log as JSON. */
	if (!eventlog_store_json(&jsonc, evlog)) {
	    errors++;
	    goto next;
	}

	/* Check for a .out.ok file in the same location as the .in file. */
	cp = strrchr(infile, '.');
	if (cp != NULL && strcmp(cp, ".in") == 0) {
	    snprintf(pathbuf, sizeof(pathbuf), "%.*s.out.ok",
		(int)(cp - infile), infile);
	    if ((outfp = fopen(pathbuf, "r")) != NULL)
		outfile = pathbuf;
	}
	if (outfp == NULL)
	    outfp = infp;

	/* Compare output to expected output. */
	rewind(outfp);
	if (!compare(outfp, outfile, &jsonc))
	    errors++;

	/* Write the formatted output to stdout for -c (cat) */
	if (cat) {
	    fprintf(stdout, "{%s\n}\n", sudo_json_get_buf(&jsonc));
	    fflush(stdout);
	}

next:
	eventlog_free(evlog);
	eventlog_json_free(root);
	sudo_json_free(&jsonc);
	if (infp != NULL)
	    fclose(infp);
	if (outfp != NULL && outfp != infp)
	    fclose(outfp);
    }

    if (ntests != 0) {
	printf("%s: %d test%s run, %d errors, %d%% success rate\n",
	    getprogname(), ntests, ntests == 1 ? "" : "s", errors,
	    (ntests - errors) * 100 / ntests);
    }

    return errors;
}
