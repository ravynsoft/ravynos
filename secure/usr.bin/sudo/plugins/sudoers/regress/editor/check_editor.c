/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2021-2022 Todd C. Miller <Todd.Miller@sudo.ws>
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

#define SUDO_ERROR_WRAP 0

#include <sudoers.h>
#include <def_data.c>

/* Note hard-coded array lengths. */
struct test_data {
    const char *editor_var;
    int nfiles;
    const char *files[4];
    const char *editor_path;
    int edit_argc;
    const char *edit_argv[10];
} test_data[] = {
    {
	/* Bug #942 */
	"SUDO_EDITOR=sh -c \"vi \\$1\"",
	1,
	{ "/etc/motd", NULL },
	"/usr/bin/sh",
	5,
	{ "sh", "-c", "vi $1", "--", "/etc/motd", NULL }
    },
    {
	/* Try connecting to the emacs server, falling back on plain emacs. */
	"VISUAL=sh -c \"emacsclient -a emacs -n \\\"\\$@\\\" || emacs \\\"\\$@\\\"\"",
	1,
	{ "/etc/motd", NULL },
	"/usr/bin/sh",
	5,
	{ "sh", "-c", "emacsclient -a emacs -n \"$@\" || emacs \"$@\"", "--", "/etc/motd", NULL }
    },
    {
	/* GitHub issue #99 */
	"EDITOR=/usr/bin/vi\\",
	1,
	{ "/etc/hosts", "/bogus/file", NULL },
	"/usr/bin/vi\\",
	3,
	{ "/usr/bin/vi\\", "--", "/etc/hosts", "/bogus/file", NULL }
    },
    {
	/* GitHub issue #179 */
	"EDITOR=sed -rie s/^\\\\(foo\\\\)/waldo\\\\1/",
	1,
	{ "/etc/sudoers", NULL },
	"/usr/bin/sed",
	5,
	{ "sed", "-rie", "s/^\\(foo\\)/waldo\\1/", "--", "/etc/sudoers", NULL }
    },
    { NULL }
};

sudo_dso_public int main(int argc, char *argv[]);

/* STUB */
int
find_path(const char *infile, char **outfile, struct stat *sbp,
    const char *path, bool ignore_dot, char * const *allowlist)
{
    if (infile[0] == '/') {
	*outfile = strdup(infile);
    } else {
	if (asprintf(outfile, "/usr/bin/%s", infile) == -1)
	    *outfile = NULL;
    }
    if (*outfile == NULL)
	return NOT_FOUND_ERROR;
    return FOUND;
}

int
main(int argc, char *argv[])
{
    struct test_data *data;
    int ch, ntests = 0, errors = 0;

    initprogname(argc > 0 ? argv[0] : "check_editor");

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

    for (data = test_data; data->editor_var != NULL; data++) {
	const char *env_editor = NULL;
	char *cp, *editor_path, **edit_argv = NULL;
	int i, edit_argc = 0;

	/* clear existing editor environment vars */
	putenv((char *)"VISUAL=");
	putenv((char *)"EDITOR=");
	putenv((char *)"SUDO_EDITOR=");

	putenv((char *)data->editor_var);
	editor_path = find_editor(data->nfiles, (char **)data->files,
	    &edit_argc, &edit_argv, NULL, &env_editor);
	ntests++;
	if (strcmp(editor_path, data->editor_path) != 0) {
	    sudo_warnx("test %d: editor_path: expected \"%s\", got \"%s\"",
		ntests, data->editor_path, editor_path);
	    errors++;
	}
	ntests++;
	cp = strchr(data->editor_var, '=') + 1;
	if (strcmp(env_editor, cp) != 0) {
	    sudo_warnx("test %d: env_editor: expected \"%s\", got \"%s\"",
		ntests, cp, env_editor ? env_editor : "(NULL)");
	    errors++;
	}
	ntests++;
	if (edit_argc != data->edit_argc) {
	    sudo_warnx("test %d: edit_argc: expected %d, got %d",
		ntests, data->edit_argc, edit_argc);
	    errors++;
	} else {
	    ntests++;
	    for (i = 0; i < edit_argc; i++) {
		if (strcmp(edit_argv[i], data->edit_argv[i]) != 0) {
		    sudo_warnx("test %d: edit_argv[%d]: expected \"%s\", got \"%s\"",
			ntests, i, data->edit_argv[i], edit_argv[i]);
		    errors++;
		    break;
		}
	    }
	}

	free(editor_path);
	edit_argc -= data->nfiles + 1;
	for (i = 0; i < edit_argc; i++) {
	    free(edit_argv[i]);
	}
	free(edit_argv);
    }

    if (ntests != 0) {
	printf("%s: %d tests run, %d errors, %d%% success rate\n",
	    getprogname(), ntests, errors, (ntests - errors) * 100 / ntests);
    }

    exit(errors);
}
