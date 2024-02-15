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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define SUDO_ERROR_WRAP 0

#include <sudo_compat.h>
#include <sudo_util.h>
#include <sudo_fatal.h>

#include <sudo_iolog.h>

sudo_dso_public int main(int argc, char *argv[]);

int
main(int argc, char *argv[])
{
    int dfd = -1, ttyin_fd = -1, ttyout_fd = -1, ttyin_ok_fd = -1;
    int ch, i, ntests = 0, errors = 0;
    void *passprompt_regex = NULL;

    initprogname(argc > 0 ? argv[0] : "check_iolog_filter");

    while ((ch = getopt(argc, argv, "v")) != -1) {
	switch (ch) {
	case 'v':
	    /* ignore */
	    break;
	default:
	    fprintf(stderr, "usage: %s [-v] iolog_dir ...\n", getprogname());
	    return EXIT_FAILURE;
	}
    }
    argc -= optind;
    argv += optind;

    passprompt_regex = iolog_pwfilt_alloc();
    if (passprompt_regex == NULL)
	sudo_fatalx("unable to allocate memory");
    if (!iolog_pwfilt_add(passprompt_regex, "(?i)password[: ]*"))
	exit(1);

    for (i = 0; i < argc; i++) {
	struct iolog_file iolog_timing = { true };
	struct timing_closure timing;
	const char *logdir = argv[i];
	char tbuf[8192], fbuf[8192];
	ssize_t nread;

	ntests++;

	/* I/O logs consist of multiple files in a directory. */
	dfd = open(logdir, O_RDONLY);
	if (dfd == -1) {
	    sudo_warn("%s", logdir);
	    errors++;
	    continue;
	}

	if (!iolog_open(&iolog_timing, dfd, IOFD_TIMING, "r")) {
	    sudo_warn("timing");
	    errors++;
	    goto next;
	}

	ttyout_fd = openat(dfd, "ttyout", O_RDONLY, 0644);
	if (ttyout_fd == -1) {
	    sudo_warn("ttyout");
	    errors++;
	    goto next;
	}

	ttyin_fd = openat(dfd, "ttyin", O_RDONLY, 0644);
	if (ttyin_fd == -1) {
	    sudo_warn("ttyin");
	    errors++;
	    goto next;
	}

	ttyin_ok_fd = openat(dfd, "ttyin.filtered", O_RDONLY, 0644);
	if (ttyin_ok_fd == -1) {
	    sudo_warn("ttyin.filtered");
	    errors++;
	    goto next;
	}

	memset(&timing, 0, sizeof(timing));
	timing.decimal = ".";
	for (;;) {
	    char *newbuf = NULL;
	    const char *name;
	    int fd;

	    if (iolog_read_timing_record(&iolog_timing, &timing) != 0)
		break;

	    switch (timing.event) {
	    case IO_EVENT_TTYOUT:
		fd = ttyout_fd;
		name = "ttyout";
		break;
	    case IO_EVENT_TTYIN:
		fd = ttyin_fd;
		name = "ttyin";
		break;
	    default:
		continue;
	    }

	    if (timing.u.nbytes > sizeof(tbuf)) {
		sudo_warn("buffer too small, %zu > %zu", timing.u.nbytes,
		    sizeof(tbuf));
		errors++;
		continue;
	    }

	    nread = read(fd, tbuf, timing.u.nbytes);
	    if ((size_t)nread != timing.u.nbytes) {
		if (nread == -1)
		    sudo_warn("%s/%s", argv[i], name);
		else
		    sudo_warnx("%s/%s: short read", argv[i], name);
		errors++;
		continue;
	    }

	    /* Apply filter. */
	    if (!iolog_pwfilt_run(passprompt_regex, timing.event, tbuf,
		    timing.u.nbytes, &newbuf)) {
		errors++;
		continue;
	    }

	    if (timing.event == IO_EVENT_TTYIN) {
		nread = read(ttyin_ok_fd, fbuf, timing.u.nbytes);
		if (nread == -1) {
		    if (nread == -1)
			sudo_warn("%s/ttyin.filtered", argv[i]);
		    else
			sudo_warnx("%s/ttyin.filtered: short read", argv[i]);
		    errors++;
		    free(newbuf);
		    break;
		}
		if (memcmp(fbuf, newbuf ? newbuf : tbuf, timing.u.nbytes) != 0) {
		    sudo_warnx("%s: ttyin mismatch at byte %lld", argv[i],
			(long long)lseek(fd, 0, SEEK_CUR));
		    errors++;
		    free(newbuf);
		    break;
		}
	    }

	    free(newbuf);
	}
next:
	if (ttyin_fd != -1) {
	    close(ttyin_fd);
	    ttyin_fd = -1;
	}
	if (ttyin_ok_fd != -1) {
	    close(ttyin_ok_fd);
	    ttyin_ok_fd = -1;
	}
	if (dfd != -1) {
	    close(dfd);
	    dfd = -1;
	}
	if (iolog_timing.enabled)
	    iolog_close(&iolog_timing, NULL);
    }
    iolog_pwfilt_free(passprompt_regex);

    if (ntests != 0) {
	printf("iolog_filter: %d test%s run, %d errors, %d%% success rate\n",
	    ntests, ntests == 1 ? "" : "s", errors,
	    (ntests - errors) * 100 / ntests);
    }

    return errors;
}
