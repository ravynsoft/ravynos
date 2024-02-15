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

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#if defined(HAVE_STDINT_H)
# include <stdint.h>
#elif defined(HAVE_INTTYPES_H)
# include <inttypes.h>
#endif

#include <sudo_compat.h>
#include <sudo_debug.h>
#include <sudo_eventlog.h>
#include <sudo_fatal.h>
#include <sudo_iolog.h>
#include <sudo_plugin.h>
#include <sudo_util.h>

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size);

static FILE *
open_data(const uint8_t *data, size_t size)
{
#ifdef HAVE_FMEMOPEN
    /* Operate in-memory. */
    return fmemopen((void *)data, size, "r");
#else
    char tempfile[] = "/tmp/json.XXXXXX";
    size_t nwritten;
    int fd;

    /* Use (unlinked) temporary file. */
    fd = mkstemp(tempfile);
    if (fd == -1)
	return NULL;
    unlink(tempfile);
    nwritten = write(fd, data, size);
    if (nwritten != size) {
	close(fd);
	return NULL;
    }
    lseek(fd, 0, SEEK_SET);
    return fdopen(fd, "r");
#endif
}

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
    struct eventlog *evlog = NULL;
    FILE *fp;

    initprogname("fuzz_iolog_json");
    if (getenv("SUDO_FUZZ_VERBOSE") == NULL)
	sudo_warn_set_conversation(fuzz_conversation);

    fp = open_data(data, size);
    if (fp == NULL)
        return 0;

    /* Parsed contents of an log.json file are stored in evlog. */
    evlog = calloc(1, sizeof(*evlog));
    if (evlog != NULL) {
	evlog->runuid = (uid_t)-1;
	evlog->rungid = (gid_t)-1;
	evlog->exit_value = -1;

	/* Try to parse buffer as a JSON-format I/O log info file. */
	iolog_parse_loginfo_json(fp, "fuzz.json", evlog);
	eventlog_free(evlog);
    }
    fclose(fp);

    fflush(stdout);

    return 0;
}
