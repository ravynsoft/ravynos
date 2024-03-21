/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2008, 2010-2018, 2020-2022 Todd C. Miller <Todd.Miller@sudo.ws>
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

/*
 * This is an open source non-commercial project. Dear PVS-Studio, please check it.
 * PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
 */

#include <config.h>

#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#ifdef HAVE_STDBOOL_H
# include <stdbool.h>
#else
# include <compat/stdbool.h>
#endif /* HAVE_STDBOOL_H */
#ifdef HAVE_GETOPT_LONG
# include <getopt.h>
# else
# include <compat/getopt.h>
#endif /* HAVE_GETOPT_LONG */

#include <sudo.h>
#include <sudo_exec.h>
#include <sudo_edit.h>

enum sesh_mode {
    SESH_RUN_COMMAND,
    SESH_EDIT_CREATE,
    SESH_EDIT_INSTALL
};

static const char short_opts[] = "+cd:e:ihnw:";
static struct option long_opts[] = {
    { "edit-create",		no_argument,		NULL,	'c' },
    { "directory",		required_argument,	NULL,	'd' },
    { "execfd",			required_argument,	NULL,	'e' },
    { "edit-install",		no_argument,		NULL,	'i' },
    { "no-dereference",		no_argument,		NULL,	'h' },
    { "noexec",			no_argument,		NULL,	'n' },
    { "edit-checkdir",		required_argument,	NULL,	'w' },
    { NULL,			no_argument,		NULL,	'\0' },
};

sudo_dso_public int main(int argc, char *argv[], char *envp[]);

static int sesh_sudoedit(enum sesh_mode mode, int flags, char *user, int argc,
    char *argv[]);

sudo_noreturn void
usage(void)
{
    (void)fprintf(stderr,
	"usage: %s [-n] [-d directory] [-e fd] command [...]\n"
	"       %s [-cih] [-w uid:gids] file [...]\n",
	getprogname(), getprogname());
    exit(EXIT_FAILURE);
}

/*
 * Exit codes defined in sudo_exec.h:
 *  SESH_SUCCESS (0)         ... successful operation
 *  SESH_ERR_FAILURE (1)     ... unspecified error
 *  SESH_ERR_INVALID (30)    ... invalid -e arg value
 *  SESH_ERR_BAD_PATHS (31)  ... odd number of paths
 *  SESH_ERR_NO_FILES (32)   ... copy error, no files copied
 *  SESH_ERR_SOME_FILES (33) ... copy error, no files copied
 */
int
main(int argc, char *argv[], char *envp[])
{
    enum sesh_mode mode = SESH_RUN_COMMAND;
    const char *errstr, *rundir = NULL;
    unsigned int flags = CD_SUDOEDIT_FOLLOW;
    char *edit_user = NULL;
    int ch, ret, fd = -1;
    debug_decl(main, SUDO_DEBUG_MAIN);

    initprogname(argc > 0 ? argv[0] : "sesh");

    setlocale(LC_ALL, "");
    bindtextdomain(PACKAGE_NAME, LOCALEDIR);
    textdomain(PACKAGE_NAME);

    while ((ch = getopt_long(argc, argv, short_opts, long_opts, NULL)) != -1) {
	switch (ch) {
	case 'c':
	    if (mode != SESH_RUN_COMMAND) {
		sudo_warnx("%s",
		    U_("Only one of the -c or -i options may be specified"));
		usage();
	    }
	    mode = SESH_EDIT_CREATE;
	    break;
	case 'd':
	    rundir = optarg;
	    if (*rundir == '+') {
		SET(flags, CD_CWD_OPTIONAL);
		rundir++;
	    }
	    break;
	case 'e':
	    fd = sudo_strtonum(optarg, 0, INT_MAX, &errstr);
	    if (errstr != NULL)
		sudo_fatalx(U_("invalid file descriptor number: %s"), optarg);
	    break;
	case 'i':
	    if (mode != SESH_RUN_COMMAND) {
		sudo_warnx("%s",
		    U_("Only one of the -c or -i options may be specified"));
		usage();
	    }
	    mode = SESH_EDIT_INSTALL;
	    break;
	case 'h':
	    CLR(flags, CD_SUDOEDIT_FOLLOW);
	    break;
	case 'n':
	    SET(flags, CD_NOEXEC);
	    break;
	case 'w':
	    SET(flags, CD_SUDOEDIT_CHECKDIR);
	    edit_user = optarg;
	    break;
	default:
	    usage();
	    /* NOTREACHED */
	}
    }
    argc -= optind;
    argv += optind;
    if (argc == 0)
	usage();

    /* Read sudo.conf and initialize the debug subsystem. */
    if (sudo_conf_read(NULL, SUDO_CONF_DEBUG) == -1)
	return EXIT_FAILURE;
    sudo_debug_register(getprogname(), NULL, NULL,
	sudo_conf_debug_files(getprogname()), -1);

    if (mode != SESH_RUN_COMMAND) {
	if (rundir != NULL) {
	    sudo_warnx(U_("The -%c option may not be used in edit mode."), 'd');
	    usage();
	}
	if (fd != -1) {
	    sudo_warnx(U_("The -%c option may not be used in edit mode."), 'e');
	    usage();
	}
	if (ISSET(flags, CD_NOEXEC)) {
	    sudo_warnx(U_("The -%c option may not be used in edit mode."), 'n');
	    usage();
	}
	ret = sesh_sudoedit(mode, flags, edit_user, argc, argv);
    } else {
	bool login_shell;
	char *cmnd;

	if (!ISSET(flags, CD_SUDOEDIT_FOLLOW)) {
	    sudo_warnx(U_("The -%c option may only be used in edit mode."),
		'h');
	    usage();
	}
	if (edit_user != NULL) {
	    sudo_warnx(U_("The -%c option may only be used in edit mode."),
		'w');
	    usage();
	}

	/* If the first char of argv[0] is '-', we are running a login shell. */
	login_shell = argv[0][0] == '-';

	/* We must change the directory in sesh after the context changes. */
	if (rundir != NULL && chdir(rundir) == -1) {
	    sudo_warnx(U_("unable to change directory to %s"), rundir);
	    if (!ISSET(flags, CD_CWD_OPTIONAL))
		return EXIT_FAILURE;
	}

	/* Make a copy of the command to execute. */
	if ((cmnd = strdup(argv[0])) == NULL)
	    sudo_fatalx(U_("%s: %s"), __func__, U_("unable to allocate memory"));

	/* If invoked as a login shell, modify argv[0] accordingly. */
	if (login_shell) {
	    char *cp = strrchr(argv[0], '/');
	    if (cp != NULL) {
		*cp = '-';
		argv[0] = cp;
	    }
	}
	sudo_execve(fd, cmnd, argv, envp, -1, flags);
	sudo_warn(U_("unable to execute %s"), cmnd);
	ret = SESH_ERR_FAILURE;
    }
    sudo_debug_exit_int(__func__, __FILE__, __LINE__, sudo_debug_subsys, ret);
    _exit(ret);
}

/*
 * Destructively parse a string in the format:
 *  uid:gid:groups,...
 *
 * On success, fills in ud and returns true, else false.
 */
static bool
parse_user(char *userstr, struct sudo_cred *cred)
{
    char *cp, *ep;
    const char *errstr;
    debug_decl(parse_user, SUDO_DEBUG_EDIT);

    /* UID */
    cp = userstr;
    if ((ep = strchr(cp, ':')) == NULL) {
	sudo_warnx(U_("%s: %s"), cp, U_("invalid value"));
	debug_return_bool(false);
    }
    *ep++ = '\0';
    cred->uid = cred->euid = sudo_strtoid(cp, &errstr);
    if (errstr != NULL) {
	sudo_warnx(U_("%s: %s"), cp, errstr);
	debug_return_bool(false);
    }

    /* GID */
    cp = ep;
    if ((ep = strchr(cp, ':')) == NULL) {
	sudo_warnx(U_("%s: %s"), cp, U_("invalid value"));
	debug_return_bool(false);
    }
    *ep++ = '\0';
    cred->gid = cred->egid = sudo_strtoid(cp, &errstr);
    if (errstr != NULL) {
	sudo_warnx(U_("%s: %s"), cp, errstr);
	debug_return_bool(false);
    }

    /* group vector */
    cp = ep;
    cred->ngroups = sudo_parse_gids(cp, NULL, &cred->groups);
    if (cred->ngroups == -1)
	debug_return_bool(false);

    debug_return_bool(true);
}

static int
sesh_edit_create_tfiles(int edit_flags, struct sudo_cred *user_cred,
    struct sudo_cred *run_cred, int argc, char *argv[])
{
    int i, fd_src = -1, fd_dst = -1;
    struct timespec times[2];
    struct stat sb;
    debug_decl(sesh_edit_create_tfiles, SUDO_DEBUG_EDIT);

    for (i = 0; i < argc - 1; i += 2) {
	char *path_src = argv[i];
	const char *path_dst = argv[i + 1];

	/*
	 * Try to open the source file for reading.
	 * If it doesn't exist, we'll create an empty destination file.
	 */
	fd_src = sudo_edit_open(path_src, O_RDONLY,
	    S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH, edit_flags, user_cred, run_cred);
	if (fd_src == -1) {
	    if (errno != ENOENT) {
		if (errno == ELOOP) {
		    sudo_warnx(U_("%s: editing symbolic links is not "
			"permitted"), path_src);
		} else if (errno == EISDIR) {
		    sudo_warnx(U_("%s: editing files in a writable directory "
			"is not permitted"), path_src);
		} else {
		    sudo_warn("%s", path_src);
		}
		goto cleanup;
	    }
	    /* New file, verify parent dir exists and is not writable. */
	    if (!sudo_edit_parent_valid(path_src, edit_flags, user_cred, run_cred))
		goto cleanup;
	}
	if (fd_src == -1) {
	    /* New file. */
	    memset(&sb, 0, sizeof(sb));
	} else if (fstat(fd_src, &sb) == -1 || !S_ISREG(sb.st_mode)) {
	    sudo_warnx(U_("%s: not a regular file"), path_src);
	    goto cleanup;
	}

	/*
	 * Create temporary file using O_EXCL to ensure that temporary
	 * files are created by us and that we do not open any symlinks.
	 */
	fd_dst = open(path_dst, O_WRONLY|O_CREAT|O_EXCL, S_IRUSR|S_IWUSR);
	if (fd_dst == -1) {
	    sudo_warn("%s", path_dst);
	    goto cleanup;
	}

	if (fd_src != -1) {
	    if (sudo_copy_file(path_src, fd_src, -1, path_dst, fd_dst, -1) == -1)
		goto cleanup;
	    close(fd_src);
	}

	/* Make mtime on temp file match src (sb filled in above). */
	mtim_get(&sb, times[0]);
	times[1].tv_sec = times[0].tv_sec;
	times[1].tv_nsec = times[0].tv_nsec;
	if (futimens(fd_dst, times) == -1) {
	    if (utimensat(AT_FDCWD, path_dst, times, 0) == -1)
		sudo_warn("%s", path_dst);
	}
	close(fd_dst);
	fd_dst = -1;
    }
    debug_return_int(SESH_SUCCESS);

cleanup:
    /* Remove temporary files. */
    for (i = 0; i < argc - 1; i += 2)
	unlink(argv[i + 1]);
    if (fd_src != -1)
	close(fd_src);
    if (fd_dst != -1)
	close(fd_dst);
    debug_return_int(SESH_ERR_NO_FILES);
}

static int
sesh_edit_copy_tfiles(int edit_flags, struct sudo_cred *user_cred,
    struct sudo_cred *run_cred, int argc, char *argv[])
{
    int i, ret = SESH_SUCCESS;
    int fd_src = -1, fd_dst = -1;
    debug_decl(sesh_edit_copy_tfiles, SUDO_DEBUG_EDIT);

    for (i = 0; i < argc - 1; i += 2) {
	const char *path_src = argv[i];
	char *path_dst = argv[i + 1];
	off_t len_src, len_dst;
	struct stat sb;

	/* Open temporary file for reading. */
	if (fd_src != -1)
	    close(fd_src);
	fd_src = open(path_src, O_RDONLY|O_NONBLOCK|O_NOFOLLOW);
	if (fd_src == -1) {
	    sudo_warn("%s", path_src);
	    ret = SESH_ERR_SOME_FILES;
	    continue;
	}
	/* Make sure the temporary file is safe and has the proper owner. */
	if (!sudo_check_temp_file(fd_src, path_src, run_cred->uid, &sb)) {
	    sudo_warnx(U_("contents of edit session left in %s"), path_src);
	    ret = SESH_ERR_SOME_FILES;
	    continue;
	}
	(void) fcntl(fd_src, F_SETFL, fcntl(fd_src, F_GETFL, 0) & ~O_NONBLOCK);

	/* Create destination file. */
	if (fd_dst != -1)
	    close(fd_dst);
	fd_dst = sudo_edit_open(path_dst, O_WRONLY|O_CREAT,
	    S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH, edit_flags, user_cred, run_cred);
	if (fd_dst == -1) {
	    if (errno == ELOOP) {
		sudo_warnx(U_("%s: editing symbolic links is not "
		    "permitted"), path_dst);
	    } else if (errno == EISDIR) {
		sudo_warnx(U_("%s: editing files in a writable directory "
		    "is not permitted"), path_dst);
	    } else {
		sudo_warn("%s", path_dst);
	    }
	    sudo_warnx(U_("contents of edit session left in %s"), path_src);
	    ret = SESH_ERR_SOME_FILES;
	    continue;
	}

	/* sudo_check_temp_file() filled in sb for us. */
	len_src = sb.st_size;
	if (fstat(fd_dst, &sb) != 0) {
	    sudo_warn("%s", path_dst);
	    sudo_warnx(U_("contents of edit session left in %s"), path_src);
	    ret = SESH_ERR_SOME_FILES;
	    continue;
	}
	len_dst = sb.st_size;

	if (sudo_copy_file(path_src, fd_src, len_src, path_dst, fd_dst,
		len_dst) == -1) {
	    sudo_warnx(U_("contents of edit session left in %s"), path_src);
	    ret = SESH_ERR_SOME_FILES;
	    continue;
	}
	unlink(path_src);
    }
    if (fd_src != -1)
	close(fd_src);
    if (fd_dst != -1)
	close(fd_dst);

    debug_return_int(ret);
}

static int
sesh_sudoedit(enum sesh_mode mode, int flags, char *user,
    int argc, char *argv[])
{
    struct sudo_cred user_cred, run_cred;
    int ret;
    debug_decl(sesh_sudoedit, SUDO_DEBUG_EDIT);

    memset(&user_cred, 0, sizeof(user_cred));
    memset(&run_cred, 0, sizeof(run_cred));

    /* Parse user for -w option, "uid:gid:gid1,gid2,..." */
    if (user != NULL && !parse_user(user, &user_cred))
	debug_return_int(SESH_ERR_FAILURE);

    /* No files specified, nothing to do. */
    if (argc == 0)
	debug_return_int(SESH_SUCCESS);

    /* Odd number of paths specified. */
    if (argc & 1)
	debug_return_int(SESH_ERR_BAD_PATHS);

    /* Masquerade as sudoedit so the user gets consistent error messages. */
    setprogname("sudoedit");

    /*
     * sudoedit runs us with the effective user-ID and group-ID of
     * the target user as well as with the target user's group list.
     */
    run_cred.uid = run_cred.euid = geteuid();
    run_cred.gid = run_cred.egid = getegid();
    run_cred.ngroups = getgroups(0, NULL); // -V575
    if (run_cred.ngroups > 0) {
	run_cred.groups = reallocarray(NULL, run_cred.ngroups,
	    sizeof(GETGROUPS_T));
	if (run_cred.groups == NULL) {
	    sudo_warnx(U_("%s: %s"), __func__,
		U_("unable to allocate memory"));
	    debug_return_int(SESH_ERR_FAILURE);
	}
	run_cred.ngroups = getgroups(run_cred.ngroups, run_cred.groups);
	if (run_cred.ngroups < 0) {
	    sudo_warn("%s", U_("unable to get group list"));
	    free(run_cred.groups);
	    debug_return_int(SESH_ERR_FAILURE);
	}
    } else {
	run_cred.ngroups = 0;
	run_cred.groups = NULL;
    }

    ret = mode == SESH_EDIT_CREATE ?
	sesh_edit_create_tfiles(flags, &user_cred, &run_cred, argc, argv) :
	sesh_edit_copy_tfiles(flags, &user_cred, &run_cred, argc, argv);
    debug_return_int(ret);
}
