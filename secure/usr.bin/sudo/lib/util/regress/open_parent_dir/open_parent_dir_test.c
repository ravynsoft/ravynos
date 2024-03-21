/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2022 Todd C. Miller <Todd.Miller@sudo.ws>
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

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SUDO_ERROR_WRAP 0

#include <sudo_compat.h>
#include <sudo_fatal.h>
#include <sudo_util.h>

sudo_dso_public int main(int argc, char *argv[]);

static int errors = 0, ntests = 0;

static int
run_test(const char *tdir, const char *path, uid_t uid, gid_t gid)
{
    char *cp, fullpath[PATH_MAX];
    struct stat sb1, sb2;
    int dfd, len;
    int ret = -1;

    /* Test creating full path. */
    len = snprintf(fullpath, sizeof(fullpath), "%s/%s", tdir, path);
    if (len < 0 || len >= ssizeof(fullpath)) {
	errno = ENAMETOOLONG;
	sudo_fatal("%s/%s", tdir, path);
    }
    ntests++;
    dfd = sudo_open_parent_dir(fullpath, uid, gid, 0700, false);
    if (dfd == -1) {
	errors++;
	goto done;
    }

    /* Verify that we only created the parent dir, not full path. */
    ntests++;
    if (stat(fullpath, &sb1) == 0) {
	sudo_warnx("created full path \"%s\", not just parent dir",
	    fullpath);
	errors++;
	goto done;
    }

    /* Verify that dfd refers to the parent dir. */
    ntests++;
    cp = strrchr(fullpath, '/');
    *cp = '\0';
    if (stat(fullpath, &sb1) == -1) {
	sudo_warn("%s", fullpath);
	errors++;
	goto done;
    }
    if (fstat(dfd, &sb2) == -1) {
	sudo_warn("%s", fullpath);
	errors++;
	goto done;
    }
    if (sb1.st_dev != sb2.st_dev || sb1.st_ino != sb2.st_ino) {
	sudo_warn("%s: sudo_open_parent_dir fd mismatch", fullpath);
	errors++;
	goto done;
    }

done:
    if (dfd != -1)
	close(dfd);
    return ret;
}

int
main(int argc, char *argv[])
{
    char tdir[] = "open_parent_dir.XXXXXXXX";
    int ch, dfd, fd, len;
    char cmd[1024];
    uid_t uid;
    gid_t gid;

    initprogname(argc > 0 ? argv[0] : "open_parent_dir_test");

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

    uid = geteuid();
    gid = getegid();

    /* All tests relative to tdir. */
    if (mkdtemp(tdir) == NULL)
	sudo_fatal("%s", tdir);

    /* Test creating new path. */
    dfd = run_test(tdir, "level1/level2/level3", uid, gid);

    /* Verify we can create a new file in the new parent dir. */
    if (dfd != -1) {
	ntests++;
	fd = openat(dfd, "testfile", O_WRONLY|O_CREAT|O_EXCL, 0600);
	if (fd == -1) {
	    errors++;
	} else {
	    close(fd);
	}
	close(dfd);
	dfd = -1;
    }

    /* Test exiting path when final component exists. */
    dfd = run_test(tdir, "level1/level2/testfile", uid, gid);
    if (dfd != -1) {
	unlinkat(dfd, "testfile", 0);
	close(dfd);
    }

    /* Test exiting path when final component doesn't exist. */
    dfd = run_test(tdir, "level1/level2/testfile", uid, gid);
    if (dfd != -1)
	close(dfd);

    /* Cleanup */
    len = snprintf(cmd, sizeof(cmd), "rm -rf \"%s\"", tdir);
    if (len < 0 || len >= ssizeof(cmd)) {
	errno = ENAMETOOLONG;
	sudo_fatalx("rm -rf %s", tdir);
    }
    ignore_result(system(cmd));

    if (ntests != 0) {
	printf("%s: %d tests run, %d errors, %d%% success rate\n",
	    getprogname(), ntests, errors, (ntests - errors) * 100 / ntests);
    }
    return errors;
}
