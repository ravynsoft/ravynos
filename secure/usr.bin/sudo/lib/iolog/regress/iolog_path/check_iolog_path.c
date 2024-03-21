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
#include <time.h>
#include <unistd.h>

#define SUDO_ERROR_WRAP 0

#include <sudo_compat.h>
#include <sudo_util.h>
#include <sudo_fatal.h>
#include <sudo_iolog.h>

static struct iolog_escape_data {
    char sessid[7];
    char *user;
    char *group;
    char *runas_user;
    char *runas_group;
    char *host;
    char *command;
} escape_data;

sudo_dso_public int main(int argc, char *argv[]);

sudo_noreturn static void
usage(void)
{
    fprintf(stderr, "usage: %s datafile\n", getprogname());
    exit(EXIT_FAILURE);
}

static void
reset_escape_data(struct iolog_escape_data *data)
{
    free(data->user);
    free(data->group);
    free(data->runas_user);
    free(data->runas_group);
    free(data->host);
    free(data->command);
    memset(data, 0, sizeof(*data));
}

static size_t
fill_seq(char *str, size_t strsize, void *unused)
{
    int len;

    /* Path is of the form /var/log/sudo-io/00/00/01. */
    len = snprintf(str, strsize, "%c%c/%c%c/%c%c", escape_data.sessid[0],
	escape_data.sessid[1], escape_data.sessid[2], escape_data.sessid[3],
	escape_data.sessid[4], escape_data.sessid[5]);
    if (len < 0)
	return strsize; /* handle non-standard snprintf() */
    return (size_t)len;
}

static size_t
fill_user(char *str, size_t strsize, void *unused)
{
    return strlcpy(str, escape_data.user, strsize);
}

static size_t
fill_group(char *str, size_t strsize, void *unused)
{
    return strlcpy(str, escape_data.group, strsize);
}

static size_t
fill_runas_user(char *str, size_t strsize, void *unused)
{
    return strlcpy(str, escape_data.runas_user, strsize);
}

static size_t
fill_runas_group(char *str, size_t strsize, void *unused)
{
    return strlcpy(str, escape_data.runas_group, strsize);
}

static size_t
fill_hostname(char *str, size_t strsize, void *unused)
{
    return strlcpy(str, escape_data.host, strsize);
}

static size_t
fill_command(char *str, size_t strsize, void *unused)
{
    return strlcpy(str, escape_data.command, strsize);
}

/* Note: "seq" must be first in the list. */
static struct iolog_path_escape path_escapes[] = {
    { "seq", fill_seq },
    { "user", fill_user },
    { "group", fill_group },
    { "runas_user", fill_runas_user },
    { "runas_group", fill_runas_group },
    { "hostname", fill_hostname },
    { "command", fill_command },
    { NULL, NULL }
};

static int
do_check(char *dir_in, char *file_in, char *tdir_out, char *tfile_out)
{
    char dir[PATH_MAX], dir_out[PATH_MAX] = "";
    char file[PATH_MAX], file_out[PATH_MAX] = "";
    int error = 0;
    struct tm tm;
    time_t now;
    size_t len;

    /*
     * Expand any strftime(3) escapes
     * XXX - want to pass tm to expand_iolog_path
     */
    time(&now);
    if (localtime_r(&now, &tm) == NULL)
	sudo_fatal("localtime_r");
    if (tdir_out[0] != '\0') {
	len = strftime(dir_out, sizeof(dir_out), tdir_out, &tm);
	if (len == 0 || dir_out[sizeof(dir_out) - 1] != '\0')
	    sudo_fatalx("dir_out: strftime overflow");
    }
    if (tfile_out[0] != '\0') {
	len = strftime(file_out, sizeof(file_out), tfile_out, &tm);
	if (len == 0 || file_out[sizeof(file_out) - 1] != '\0')
	    sudo_fatalx("file_out: strftime overflow");
    }

    if (!expand_iolog_path(dir_in, dir, sizeof(dir), &path_escapes[1], NULL))
	sudo_fatalx("unable to expand I/O log dir");
    if (!expand_iolog_path(file_in, file, sizeof(file), &path_escapes[0], dir))
	sudo_fatalx("unable to expand I/O log file");

    if (strcmp(dir, dir_out) != 0) {
	sudo_warnx("%s: expected %s, got %s", dir_in, dir_out, dir);
	error = 1;
    }
    if (strcmp(file, file_out) != 0) {
	sudo_warnx("%s: expected %s, got %s", file_in, file_out, file);
	error = 1;
    }

    return error;
}

#define MAX_STATE	12

int
main(int argc, char *argv[])
{
    size_t len;
    FILE *fp;
    char line[2048];
    char *file_in = NULL, *file_out = NULL;
    char *dir_in = NULL, *dir_out = NULL;
    int ch, state = 0, errors = 0, ntests = 0;

    initprogname(argc > 0 ? argv[0] : "check_iolog_path");

    while ((ch = getopt(argc, argv, "v")) != -1) {
	switch (ch) {
	case 'v':
	    /* ignore */
	    break;
	default:
	    fprintf(stderr, "usage: %s [-v] data\n", getprogname());
	    return EXIT_FAILURE;
	}
    }
    argc -= optind;
    argv += optind;

    if (argc != 1)
	usage();

    fp = fopen(argv[0], "r");
    if (fp == NULL)
	sudo_fatalx("unable to open %s", argv[0]);

    /*
     * Input consists of 12 lines:
     * sequence number
     * user name
     * user gid
     * runas user name
     * runas gid
     * hostname [short form]
     * command
     * dir [with escapes]
     * file [with escapes]
     * expanded dir
     * expanded file
     * empty line
     */
    while (fgets(line, sizeof(line), fp) != NULL) {
	len = strcspn(line, "\n");
	line[len] = '\0';

	switch (state) {
	case 0:
	    strlcpy(escape_data.sessid, line, sizeof(escape_data.sessid));
	    break;
	case 1:
	    if ((escape_data.user = strdup(line)) == NULL)
		sudo_fatal(NULL);
	    break;
	case 2:
	    if ((escape_data.group = strdup(line)) == NULL)
		sudo_fatal(NULL);
	    break;
	case 3:
	    if ((escape_data.runas_user = strdup(line)) == NULL)
		sudo_fatal(NULL);
	    break;
	case 4:
	    if ((escape_data.runas_group = strdup(line)) == NULL)
		sudo_fatal(NULL);
	    break;
	case 5:
	    if ((escape_data.host = strdup(line)) == NULL)
		sudo_fatal(NULL);
	    break;
	case 6:
	    if ((escape_data.command = strdup(line)) == NULL)
		sudo_fatal(NULL);
	    break;
	case 7:
	    if (dir_in != NULL)
		free(dir_in);
	    dir_in = strdup(line);
	    break;
	case 8:
	    if (file_in != NULL)
		free(file_in);
	    file_in = strdup(line);
	    break;
	case 9:
	    if (dir_out != NULL)
		free(dir_out);
	    dir_out = strdup(line);
	    break;
	case 10:
	    if (file_out != NULL)
		free(file_out);
	    file_out = strdup(line);
	    break;
	case 11:
	    errors += do_check(dir_in, file_in, dir_out, file_out);
	    ntests++;
	    reset_escape_data(&escape_data);
	    break;
	default:
	    sudo_fatalx("internal error, invalid state %d", state);
	}
	state = (state + 1) % MAX_STATE;
    }
    free(dir_in);
    free(dir_out);
    free(file_in);
    free(file_out);

    if (ntests != 0) {
	printf("iolog_path: %d test%s run, %d errors, %d%% success rate\n",
	    ntests, ntests == 1 ? "" : "s", errors,
	    (ntests - errors) * 100 / ntests);
    }

    return errors;
}
