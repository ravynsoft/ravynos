/*
 * Copyright (c) 2021 Todd C. Miller <Todd.Miller@sudo.ws>
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

#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#if defined(HAVE_STDINT_H)
# include <stdint.h>
#elif defined(HAVE_INTTYPES_H)
# include <inttypes.h>
#endif
#ifdef HAVE_STDBOOL_H
# include <stdbool.h>
#else
# include <compat/stdbool.h>
#endif /* HAVE_STDBOOL_H */

#include <sudo_compat.h>
#include <sudo_debug.h>
#include <sudo_eventlog.h>
#include <sudo_fatal.h>
#include <sudo_iolog.h>
#include <sudo_plugin.h>
#include <sudo_util.h>

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size);

static int
fuzz_conversation(int num_msgs, const struct sudo_conv_message msgs[],
    struct sudo_conv_reply replies[], struct sudo_conv_callback *callback)
{
    int n;

    for (n = 0; n < num_msgs; n++) {
	const struct sudo_conv_message *msg = &msgs[n];

	switch (msg->msg_type & 0xff) {
	    case SUDO_CONV_PROMPT_ECHO_ON:
	    case SUDO_CONV_PROMPT_MASK:
	    case SUDO_CONV_PROMPT_ECHO_OFF:
		/* input not supported */
		return -1;
	    case SUDO_CONV_ERROR_MSG:
	    case SUDO_CONV_INFO_MSG:
		/* no output for fuzzers */
		break;
	    default:
		return -1;
	}
    }
    return 0;
}

int
LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    struct iolog_file iolog_file = { true };
    struct timing_closure closure;
    char logdir[] = "/tmp/timing.XXXXXX";
    int dfd = -1, fd = -1;

    initprogname("fuzz_iolog_timing");
    if (getenv("SUDO_FUZZ_VERBOSE") == NULL)
	sudo_warn_set_conversation(fuzz_conversation);

    /* I/O logs consist of multiple files in a directory. */
    if (mkdtemp(logdir) == NULL)
	return 0;

    /* Create a timing file from the supplied data. */
    dfd = open(logdir, O_RDONLY);
    if (dfd == -1)
	goto cleanup;

    fd = openat(dfd, "timing", O_WRONLY|O_CREAT|O_EXCL, S_IRWXU);
    if (fd == -1)
	goto cleanup;

    if (write(fd, data, size) != (ssize_t)size)
	goto cleanup;
    close(fd);
    fd = -1;

    /* Open the timing file we wrote and try to parse it. */
    if (!iolog_open(&iolog_file, dfd, IOFD_TIMING, "r"))
	goto cleanup;

    memset(&closure, 0, sizeof(closure));
    closure.decimal = ".";
    for (;;) {
	if (iolog_read_timing_record(&iolog_file, &closure) != 0)
	    break;
    }
    iolog_close(&iolog_file, NULL);

cleanup:
    if (dfd != -1) {
	if (fd != -1)
	    close(fd);
	unlinkat(dfd, "timing", 0);
	close(dfd);
    }
    rmdir(logdir);
    fflush(stdout);

    return 0;
}

/* STUB */
bool
iolog_swapids(bool restore)
{
    return false;
}
