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

#ifdef __linux__
# include <sys/utsname.h>

# if defined(__ILP32__)
#  define ARCH_LIB	"libx32"
# elif defined(__LP64__)
#  define ARCH_LIB	"lib64"
# else
#  define ARCH_LIB	"lib32"
# endif

struct multiarch_test {
    const char *inpath;
    char *outpath;
};

static struct multiarch_test *
make_test_data(void)
{
    struct multiarch_test *test_data;
    struct utsname unamebuf;
    int i;

    if (uname(&unamebuf) == -1)
        return NULL;

    test_data = calloc(7, sizeof(*test_data));
    if (test_data == NULL)
	return NULL;

    test_data[0].inpath = "/usr/" ARCH_LIB "/libfoo.so";
    i = asprintf(&test_data[0].outpath, "/usr/lib/%s-linux-gnu/libfoo.so",
	unamebuf.machine);
    if (i == -1) {
	test_data[0].outpath = NULL;
	goto bad;
    }

    test_data[1].inpath = "/usr/lib/something.so";
    i = asprintf(&test_data[1].outpath, "/usr/lib/%s-linux-gnu/something.so",
	unamebuf.machine);
    if (i == -1) {
	test_data[1].outpath = NULL;
	goto bad;
    }

    test_data[2].inpath = "/usr/libexec/libbar.so";
    i = asprintf(&test_data[2].outpath, "/usr/libexec/%s-linux-gnu/libbar.so",
	unamebuf.machine);
    if (i == -1) {
	test_data[2].outpath = NULL;
	goto bad;
    }

    test_data[3].inpath = "/usr/local/lib/sudo/libsudo_util.so";
    i = asprintf(&test_data[3].outpath, "/usr/local/lib/%s-linux-gnu/sudo/libsudo_util.so",
	unamebuf.machine);
    if (i == -1) {
	test_data[3].outpath = NULL;
	goto bad;
    }

    test_data[4].inpath = "/opt/sudo/lib/sudoers.so";
    i = asprintf(&test_data[4].outpath, "/opt/sudo/lib/%s-linux-gnu/sudoers.so",
	unamebuf.machine);
    if (i == -1) {
	test_data[4].outpath = NULL;
	goto bad;
    }

    i = asprintf(&test_data[5].outpath, "/usr/lib/%s-linux-gnu/something.so",
	unamebuf.machine);
    if (i == -1) {
	test_data[5].outpath = NULL;
	goto bad;
    }
    test_data[5].inpath = test_data[5].outpath;
    test_data[5].outpath = NULL;

    return test_data;
bad:
    for (i = 0; test_data[i].outpath != NULL; i++)
	free(test_data[i].outpath);
    free(test_data);
    return NULL;
}
#endif /* __linux__ */

int
main(int argc, char *argv[])
{
    int ch, errors = 0;
#ifdef __linux__
    int ntests = 0;
    struct multiarch_test *test_data;
#endif

    initprogname(argc > 0 ? argv[0] : "multiarch_test");

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

#ifdef __linux__
    test_data = make_test_data();
    if (test_data == NULL) {
	sudo_warnx("%s", "failed to generate test data");
	return EXIT_FAILURE;
    }

    for (ch = 0; test_data[ch].inpath != NULL; ch++) {
	char *outpath = sudo_stat_multiarch(test_data[ch].inpath, NULL);
	ntests++;
	if (outpath == NULL) {
	    if (test_data[ch].outpath != NULL) {
		sudo_warnx("%s: sudo_stat_multiarch failed",
		    test_data[ch].inpath);
		errors++;
	    }
	} else if (strcmp(outpath, test_data[ch].outpath) != 0) {
	    sudo_warnx("%s: expected %s got %s", test_data[ch].inpath,
		test_data[ch].outpath, outpath);
	    errors++;
	}
	/* For test_data[5], inpath is allocated and outpath is NULL. */
	if (test_data[ch].outpath != NULL)
	    free(test_data[ch].outpath);
	else
	    free((char *)test_data[ch].inpath);
	free(outpath);
    }
    free(test_data);

    if (ntests != 0) {
	printf("%s: %d tests run, %d errors, %d%% success rate\n",
	    getprogname(), ntests, errors, (ntests - errors) * 100 / ntests);
    }
#endif /* __linux__ */
    return errors;
}
