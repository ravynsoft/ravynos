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

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_STDBOOL_H
# include <stdbool.h>
#else
# include <compat/stdbool.h>
#endif /* HAVE_STDBOOL_H */
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>

#include <sudo_compat.h>
#include <sudo_util.h>

sudo_dso_public int main(int argc, char *argv[]);

extern int get_net_ifs(char **addrinfo);

int
main(int argc, char *argv[])
{
    int ch, ninterfaces, errors = 0, ntests = 1;
    char *interfaces = NULL;
    bool verbose = false;

    initprogname(argc > 0 ? argv[0] : "check_net_ifs");

    while ((ch = getopt(argc, argv, "v")) != -1) {
	switch (ch) {
	case 'v':
	    verbose = true;
	    break;
	default:
	    fprintf(stderr, "usage: %s [-v]\n", getprogname());
	    return EXIT_FAILURE;
	}
    }

    ninterfaces = get_net_ifs(&interfaces);
    switch (ninterfaces) {
    case -1:
	printf("FAIL: unable to get network interfaces\n");
	errors++;
	break;
    case 0:
	/* no interfaces or STUB_LOAD_INTERFACES defined. */
	if (verbose)
	    printf("OK: (0 interfaces)\n");
	break;
    default:
	if (verbose) {
	    printf("OK: (%d interface%s, %s)\n", ninterfaces,
		ninterfaces > 1 ? "s" : "", interfaces);
	}
	break;
    }
    free(interfaces);

    if (ntests != 0) {
        printf("%s: %d tests run, %d errors, %d%% success rate\n",
            getprogname(), ntests, errors, (ntests - errors) * 100 / ntests);
    }
    return errors;
}
