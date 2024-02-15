/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2019-2020 Todd C. Miller <Todd.Miller@sudo.ws>
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
#ifdef HAVE_STDBOOL_H
# include <stdbool.h>
#else
# include <compat/stdbool.h>
#endif
#include <time.h>
#include <unistd.h>

#include <sudo_compat.h>
#include <sudo_fatal.h>
#include <sudo_iolog.h>
#include <sudo_util.h>

sudo_dso_public int main(int argc, char *argv[]);

/*
 * Test that iolog_parse_host_port() works as expected.
 */

struct host_port_test {
    const char *str;		/* input string */
    const char *host;		/* parsed host */
    const char *port;		/* parsed port */
    bool tls;			/* parsed TLS flag */
    const char *defport;	/* default port */
    const char *defport_tls;	/* default port */
    bool ret;			/* return value */
};

static struct host_port_test test_data[] = {
    /* No TLS */
    { "xerxes", "xerxes", "12345", false, "12345", NULL, true },
    { "xerxes:12345", "xerxes", "12345", false, "67890", NULL, true },
    { "127.0.0.1", "127.0.0.1", "12345", false, "12345", NULL, true },
    { "127.0.0.1:12345", "127.0.0.1", "12345", false, "67890", NULL, true },
    { "[::1]", "::1", "12345", false, "12345", NULL, true },
    { "[::1]:12345", "::1", "12345", false, "67890", NULL, true },

    /* With TLS */
    { "xerxes(tls)", "xerxes", "12345", true, "5678", "12345", true },
    { "xerxes:12345(tls)", "xerxes", "12345", true, "5678", "67890", true },
    { "127.0.0.1(tls)", "127.0.0.1", "12345", true, "5678", "12345", true },
    { "127.0.0.1:12345(tls)", "127.0.0.1", "12345", true, "5678", "67890", true },
    { "[::1](tls)", "::1", "12345", true, "5678", "12345", true },
    { "[::1]:12345(tls)", "::1", "12345", true, "5678", "67890", true },

    /* Errors */
    { "xerxes:", NULL, NULL, false, "12345", NULL, false },	/* missing port */
    { "127.0.0.1:", NULL, NULL, false, "12345", NULL, false },	/* missing port */
    { "[::1:12345", NULL, NULL, false, "67890", NULL, false },	/* missing bracket */
    { "[::1]:", NULL, NULL, false, "12345", NULL, false },	/* missing port */
    { NULL }
};

int
main(int argc, char *argv[])
{
    int ch, i, errors = 0, ntests = 0;
    char *host, *port, *copy = NULL;
    bool ret, tls;

    initprogname(argc > 0 ? argv[0] : "host_port_test");

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

    for (i = 0; test_data[i].str != NULL; i++) {
	host = port = NULL;
	tls = false;
	free(copy);
	if ((copy = strdup(test_data[i].str)) == NULL)
	    sudo_fatal_nodebug(NULL);

	ntests++;
	ret = iolog_parse_host_port(copy, &host, &port, &tls,
	    test_data[i].defport, test_data[i].defport_tls);
	if (ret != test_data[i].ret) {
	    sudo_warnx_nodebug("test #%d: %s: returned %s, expected %s",
		ntests, test_data[i].str, ret ? "true" : "false",
		test_data[i].ret ? "true" : "false");
	    errors++;
	    continue;
	}
	if (!ret)
	    continue;

	if (host == NULL) {
	    sudo_warnx_nodebug("test #%d: %s: NULL host",
		ntests, test_data[i].str);
	    errors++;
	    continue;
	}
	if (strcmp(host, test_data[i].host) != 0) {
	    sudo_warnx_nodebug("test #%d: %s: bad host, expected %s, got %s",
		ntests, test_data[i].str, test_data[i].host, host);
	    errors++;
	    continue;
	}
	if (port == NULL) {
	    sudo_warnx_nodebug("test #%d: %s: NULL port",
		ntests, test_data[i].str);
	    errors++;
	    continue;
	}
	if (strcmp(port, test_data[i].port) != 0) {
	    sudo_warnx_nodebug("test #%d: %s: bad port, expected %s, got %s",
		ntests, test_data[i].str, test_data[i].port, port);
	    errors++;
	    continue;
	}
	if (tls != test_data[i].tls) {
	    sudo_warnx_nodebug("test #%d: %s: bad tls, expected %s, got %s",
		ntests, test_data[i].str, test_data[i].tls ? "true" : "false",
		tls ? "true" : "false");
	    errors++;
	    continue;
	}
    }
    free(copy);
    if (ntests != 0) {
	printf("%s: %d tests run, %d errors, %d%% success rate\n",
	    getprogname(), ntests, errors, (ntests - errors) * 100 / ntests);
    }
    return errors;
}
