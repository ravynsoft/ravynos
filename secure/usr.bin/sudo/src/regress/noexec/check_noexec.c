/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2016, 2022 Todd C. Miller <Todd.Miller@sudo.ws>
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
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_STDBOOL_H
# include <stdbool.h>
#else
# include <compat/stdbool.h>
#endif /* HAVE_STDBOOL_H */
#include <string.h>
#ifdef HAVE_WORDEXP_H
# include <wordexp.h>
#endif
#include <signal.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>

#include <sudo_compat.h>
#include <sudo_fatal.h>
#include <sudo_util.h>
#include <sudo_queue.h>
#include <sudo_exec.h>

static bool verbose;

sudo_dso_public int main(int argc, char *argv[], char *envp[]);

static bool
report_status(int status, const char *what)
{
    bool ret = false;

    /* system() returns -1 for exec failure. */
    if (status == -1) {
	if (verbose)
	    printf("%s: OK (%s)\n", getprogname(), what);
	return true;
    }

    /* check exit value, expecting 127 for failure */
    if (WIFEXITED(status)) {
	int exitval = WEXITSTATUS(status);
	if (exitval == 127) {
	    if (verbose)
		printf("%s: OK (%s)\n", getprogname(), what);
	    ret = true;
	} else {
	    printf("%s: FAIL (%s) [%d]\n", getprogname(), what, exitval);
	}
    } else if (WIFSIGNALED(status)) {
	printf("%s: FAIL (%s) [signal %d]\n", getprogname(), what,
	    WTERMSIG(status));
    } else {
	/* should not happen */
	printf("%s: FAIL (%s) [status %d]\n", getprogname(), what, status);
    }

    return ret;
}

static int
try_execl(void)
{
    pid_t child, pid;
    int status;

    child = fork();
    switch (child) {
    case -1:
	sudo_fatal_nodebug("fork");
    case 0:
	/* child */
	/* Try to exec /bin/true, else exit with value 127. */
	execl("/bin/true", "true", (char *)0);
	_exit(127);
    default:
	/* parent */
	do {
	    pid = waitpid(child, &status, 0);
	} while (pid == -1 && errno == EINTR);
	if (pid == -1)
	    sudo_fatal_nodebug("waitpid");

	if (report_status(status, "execl"))
	    return 0;
	return 1;
    }
}

static int
try_system(void)
{
    int status;

    /* Try to run /bin/true, system() returns 127 on exec failure. */
    status = system("/bin/true > /dev/null 2>&1");

    if (report_status(status, "system"))
	return 0;
    return 1;
}

#ifdef HAVE_WORDEXP_H
static int
try_wordexp(void)
{
    wordexp_t we;
    int rc, ret = 1;

    /*
     * sudo_noexec.so prevents command substitution via the WRDE_NOCMD flag
     * where possible.
     */
    rc = wordexp("$(/bin/echo foo)", &we, 0);
    switch (rc) {
    case -1:
	/* sudo's wordexp() wrapper returns -1 if RTLD_NEXT is not supported. */
    case 127:
	/* Solaris 10 wordexp() returns 127 for execve() failure. */
#ifdef WRDE_ERRNO
    case WRDE_ERRNO:
	/* Solaris 11 wordexp() returns WRDE_ERRNO for execve() failure. */
#endif
	if (verbose)
	    printf("%s: OK (wordexp) [%d]\n", getprogname(), rc);
	ret = 0;
	break;
    case WRDE_SYNTAX:
	/* FreeBSD returns WRDE_SYNTAX if it can't write to the shell process */
	if (verbose)
	    printf("%s: OK (wordexp) [WRDE_SYNTAX]\n", getprogname());
	ret = 0;
	break;
    case WRDE_CMDSUB:
	if (verbose)
	    printf("%s: OK (wordexp) [WRDE_CMDSUB]\n", getprogname());
	ret = 0;
	break;
    case 0:
	/*
	 * On HP-UX 11.00 we don't seem to be able to add WRDE_NOCMD
	 * but the execve() wrapper prevents the command substitution.
	 */
	if (we.we_wordc == 0) {
	    if (verbose)
		printf("%s: OK (wordexp) [%d]\n", getprogname(), rc);
	    wordfree(&we);
	    ret = 0;
	    break;
	}
	wordfree(&we);
	FALLTHROUGH;
    default:
	printf("%s: FAIL (wordexp) [%d]\n", getprogname(), rc);
	break;
    }
    return ret;
}
#endif

sudo_noreturn static void
usage(void)
{
    fprintf(stderr, "usage: %s [-v] rexec | /path/to/sudo_noexec.so\n",
	getprogname());
    exit(EXIT_FAILURE);
}

int
main(int argc, char *argv[], char *envp[])
{
    int ch, errors = 0, ntests = 0;

    initprogname(argc > 0 ? argv[0] : "check_noexec");

    while ((ch = getopt(argc, argv, "v")) != -1) {
	switch (ch) {
	case 'v':
	    verbose = true;
	    break;
	default:
	    usage();
	}
    }

    if (argc - optind != 1)
	usage();

    /* Disable execution for post-exec and re-exec ourself. */
    if (strcmp(argv[optind], "rexec") != 0) {
	const char *noexec = argv[optind];
	argv[optind] = (char *)"rexec";
	execve(argv[0], argv, disable_execute(envp, noexec));
	sudo_fatalx_nodebug("execve");
    }

    ntests++;
    errors += try_execl();
    ntests++;
    errors += try_system();
#ifdef HAVE_WORDEXP_H
    ntests++;
    errors += try_wordexp();
#endif

    if (ntests != 0) {
        printf("%s: %d tests run, %d errors, %d%% success rate\n",
            getprogname(), ntests, errors, (ntests - errors) * 100 / ntests);
    }
    return errors;
}
