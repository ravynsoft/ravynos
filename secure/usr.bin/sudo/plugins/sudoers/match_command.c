/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 1996, 1998-2005, 2007-2023
 *	Todd C. Miller <Todd.Miller@sudo.ws>
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
 *
 * Sponsored in part by the Defense Advanced Research Projects
 * Agency (DARPA) and Air Force Research Laboratory, Air Force
 * Materiel Command, USAF, under agreement number F39502-99-1-0512.
 */

/*
 * This is an open source non-commercial project. Dear PVS-Studio, please check it.
 * PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
 */

#include <config.h>

#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifndef SUDOERS_NAME_MATCH
# ifdef HAVE_GLOB
#  include <glob.h>
# else
#  include <compat/glob.h>
# endif /* HAVE_GLOB */
#endif /* SUDOERS_NAME_MATCH */
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>
#ifdef HAVE_FNMATCH
# include <fnmatch.h>
#else
# include <compat/fnmatch.h>
#endif /* HAVE_FNMATCH */
#include <regex.h>

#include <sudoers.h>
#include <gram.h>

#if !defined(O_EXEC) && defined(O_PATH)
# define O_EXEC O_PATH
#endif

static int
regex_matches(const char *pattern, const char *str)
{
    const char *errstr;
    regex_t re;
    int ret;
    debug_decl(regex_matches, SUDOERS_DEBUG_MATCH);

    if (!sudo_regex_compile(&re, pattern, &errstr)) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
	    "unable to compile regular expression \"%s\": %s",
	    pattern, errstr);
	debug_return_int(DENY);
    }

    if (regexec(&re, str, 0, NULL, 0) == 0)
	ret = ALLOW;
    else
	ret = DENY;
    regfree(&re);

    debug_return_int(ret);
}

static int
command_args_match(struct sudoers_context *ctx, const char *sudoers_cmnd,
    const char *sudoers_args)
{
    const char *args = ctx->user.cmnd_args ? ctx->user.cmnd_args : "";
    int flags = 0;
    debug_decl(command_args_match, SUDOERS_DEBUG_MATCH);

    /*
     * If no args specified in sudoers, any user args are allowed.
     * If the empty string is specified in sudoers, no user args are allowed.
     */
    if (sudoers_args == NULL)
	debug_return_int(ALLOW);
    if (strcmp("\"\"", sudoers_args) == 0)
	debug_return_int(ctx->user.cmnd_args ? DENY : ALLOW);

    /*
     * If args are specified in sudoers, they must match the user args.
     * Args are matched either as a regular expression or glob pattern.
     */
    if (sudoers_args[0] == '^') {
	size_t len = strlen(sudoers_args);
	if (len > 0 && sudoers_args[len - 1] == '$')
	    debug_return_int(regex_matches(sudoers_args, args));
    }

    /* If running as sudoedit, all args are assumed to be paths. */
    if (strcmp(sudoers_cmnd, "sudoedit") == 0)
	flags = FNM_PATHNAME;
    if (fnmatch(sudoers_args, args, flags) == 0)
	debug_return_int(ALLOW);
    debug_return_int(DENY);
}

#ifndef SUDOERS_NAME_MATCH
/*
 * Stat file by fd is possible, else by path.
 * Returns true on success, else false.
 */
static bool
do_stat(int fd, const char *path, struct stat *sb)
{
    bool ret;
    debug_decl(do_stat, SUDOERS_DEBUG_MATCH);

    if (fd != -1) {
	ret = fstat(fd, sb) == 0;
    } else {
	ret = stat(path, sb) == 0;
    }
    debug_return_bool(ret);
}
#endif /* SUDOERS_NAME_MATCH */

/*
 * Check whether the fd refers to a shell script with a "#!" shebang.
 */
static bool
is_script(int fd)
{
    bool ret = false;
    char magic[2];
    debug_decl(is_script, SUDOERS_DEBUG_MATCH);

    if (pread(fd, magic, 2, 0) == 2) {
	if (magic[0] == '#' && magic[1] == '!')
	    ret = true;
    }
    debug_return_bool(ret);
}

/*
 * Open path if fdexec is enabled or if a digest is present.
 * Returns false on error, else true.
 */
static bool
open_cmnd(const char *path, const struct command_digest_list *digests, int *fdp)
{
    int fd;
    debug_decl(open_cmnd, SUDOERS_DEBUG_MATCH);

    /* Only open the file for fdexec or for digest matching. */
    if (def_fdexec != always && TAILQ_EMPTY(digests))
	debug_return_bool(true);

    fd = open(path, O_RDONLY|O_NONBLOCK);
# ifdef O_EXEC
    if (fd == -1 && errno == EACCES && TAILQ_EMPTY(digests)) {
	/* Try again with O_EXEC if no digest is specified. */
	const int saved_errno = errno;
	if ((fd = open(path, O_EXEC)) == -1)
	    errno = saved_errno;
    }
# endif
    if (fd == -1)
	debug_return_bool(false);

    (void)fcntl(fd, F_SETFD, FD_CLOEXEC);
    *fdp = fd;
    debug_return_bool(true);
}

static void
set_cmnd_fd(struct sudoers_context *ctx, int fd, int real_root)
{
    debug_decl(set_cmnd_fd, SUDOERS_DEBUG_MATCH);

    if (ctx->runas.execfd != -1)
	close(ctx->runas.execfd);

    if (fd != -1) {
	if (def_fdexec == never) {
	    /* Never use fexedcve() */
	    close(fd);
	    fd = -1;
	} else if (is_script(fd)) {
	    char fdpath[PATH_MAX];
	    struct stat sb;
	    int error, flags;

	    /* We can only use fexecve() on a script if /dev/fd/N exists. */
	    if (real_root != -1) {
		/* Path relative to old root directory. */
		(void)snprintf(fdpath, sizeof(fdpath), "dev/fd/%d", fd);
		error = fstatat(real_root, fdpath, &sb, 0);
	    } else {
		/* Absolute path. */
		(void)snprintf(fdpath, sizeof(fdpath), "/dev/fd/%d", fd);
		error = stat(fdpath, &sb);
	    }
	    if (error != 0) {
		/* Missing /dev/fd file, can't use fexecve(). */
		close(fd);
		fd = -1;
	    } else {
		/*
		 * Shell scripts go through namei twice so we can't have the
		 * close on exec flag set on the fd for fexecve(2).
		 */
		flags = fcntl(fd, F_GETFD) & ~FD_CLOEXEC;
		(void)fcntl(fd, F_SETFD, flags);
	    }
	}
    }

    ctx->runas.execfd = fd;

    debug_return;
}

#ifndef SUDOERS_NAME_MATCH
/*
 * Return true if ctx->user.cmnd names one of the inodes in dir, else false.
 */
static int
command_matches_dir(struct sudoers_context *ctx, const char *sudoers_dir,
    size_t dlen, int real_root, const struct command_digest_list *digests)
{
    struct stat sudoers_stat;
    char path[PATH_MAX];
    int len, fd = -1;
    int ret = DENY;
    debug_decl(command_matches_dir, SUDOERS_DEBUG_MATCH);

    /* Compare the canonicalized directories, if possible. */
    if (ctx->user.cmnd_dir != NULL) {
	char *resolved = canon_path(sudoers_dir);
	if (resolved != NULL) {
	    if (strcmp(resolved, ctx->user.cmnd_dir) != 0) {
		canon_path_free(resolved);
		goto done;
	    }
	    canon_path_free(resolved);
	}
    }

    /* Check for command in sudoers_dir. */
    len = snprintf(path, sizeof(path), "%s/%s", sudoers_dir, ctx->user.cmnd_base);
    if (len < 0 || len >= ssizeof(path))
	goto done;

    /* Open the file for fdexec or for digest matching. */
    if (!open_cmnd(path, digests, &fd))
	goto done;
    if (!do_stat(fd, path, &sudoers_stat))
	goto done;

    if (ctx->user.cmnd_stat == NULL ||
	(ctx->user.cmnd_stat->st_dev == sudoers_stat.st_dev &&
	ctx->user.cmnd_stat->st_ino == sudoers_stat.st_ino)) {
	if (digest_matches(fd, path, digests) != ALLOW)
	    goto done;
	free(ctx->runas.cmnd);
	if ((ctx->runas.cmnd = strdup(path)) == NULL) {
	    sudo_warnx(U_("%s: %s"), __func__,
		U_("unable to allocate memory"));
	}
	ret = ALLOW;
	goto done;
    }
    ret = DENY;

done:
    if (fd != -1)
	close(fd);
    debug_return_int(ret);
}
#else /* SUDOERS_NAME_MATCH */
/*
 * Return true if ctx->user.cmnd names one of the inodes in dir, else false.
 */
static int
command_matches_dir(struct sudoers_context *ctx, const char *sudoers_dir,
    size_t dlen, int real_root, const struct command_digest_list *digests)
{
    int fd = -1;
    debug_decl(command_matches_dir, SUDOERS_DEBUG_MATCH);

    /* Match ctx->user.cmnd against sudoers_dir. */
    if (strncmp(ctx->user.cmnd, sudoers_dir, dlen) != 0 || ctx->user.cmnd[dlen] != '/')
	goto bad;

    /* Make sure ctx->user.cmnd is not in a subdir of sudoers_dir. */
    if (strchr(ctx->user.cmnd + dlen + 1, '\0') != NULL)
	goto bad;

    /* Open the file for fdexec or for digest matching. */
    if (!open_cmnd(ctx->user.cmnd, digests, &fd))
	goto bad;
    if (digest_matches(fd, ctx->user.cmnd, digests) != ALLOW)
	goto bad;
    set_cmnd_fd(ctx, fd, real_root);

    debug_return_int(ALLOW);
bad:
    if (fd != -1)
	close(fd);
    debug_return_int(DENY);
}
#endif /* SUDOERS_NAME_MATCH */

static int
command_matches_all(struct sudoers_context *ctx, int real_root,
    const struct command_digest_list *digests)
{
#ifndef SUDOERS_NAME_MATCH
    struct stat sb;
#endif
    int fd = -1;
    debug_decl(command_matches_all, SUDOERS_DEBUG_MATCH);

    if (strchr(ctx->user.cmnd, '/') != NULL) {
#ifndef SUDOERS_NAME_MATCH
	/* Open the file for fdexec or for digest matching. */
	bool open_error = !open_cmnd(ctx->user.cmnd, digests, &fd);

	/* A non-existent file is not an error for "sudo ALL". */
	if (do_stat(fd, ctx->user.cmnd, &sb)) {
	    if (open_error) {
		/* File exists but we couldn't open it above? */
		goto bad;
	    }
	}
#else
	/* Open the file for fdexec or for digest matching. */
	(void)open_cmnd(ctx->user.cmnd, digests, &fd);
#endif
    }

    /* Check digest of ctx->user.cmnd since we have no sudoers_cmnd for ALL. */
    if (digest_matches(fd, ctx->user.cmnd, digests) != ALLOW)
	goto bad;
    set_cmnd_fd(ctx, fd, real_root);

    /* No need to set ctx->runas.cmnd for ALL. */
    debug_return_int(ALLOW);
bad:
    if (fd != -1)
	close(fd);
    debug_return_int(DENY);
}

static int
command_matches_fnmatch(struct sudoers_context *ctx, const char *sudoers_cmnd,
    const char *sudoers_args, int real_root,
    const struct command_digest_list *digests)
{
    const char *cmnd = ctx->user.cmnd;
    char buf[PATH_MAX];
    int len, fd = -1;
#ifndef SUDOERS_NAME_MATCH
    struct stat sb;
#endif
    debug_decl(command_matches_fnmatch, SUDOERS_DEBUG_MATCH);

    /* A relative ctx->user.cmnd will not match, try canonicalized version. */
    if (ctx->user.cmnd[0] != '/') {
	if (ctx->user.cmnd_dir == NULL)
	    debug_return_int(DENY);
	len = snprintf(buf, sizeof(buf), "%s/%s", ctx->user.cmnd_dir,
	    ctx->user.cmnd_base);
	if (len < 0 || len >= ssizeof(buf))
	    debug_return_int(DENY);
	cmnd = buf;
    }

    /*
     * Return ALLOW if fnmatch(3) succeeds AND
     *  a) there are no args in sudoers OR
     *  b) there are no args on command line and none required by sudoers OR
     *  c) there are args in sudoers and on command line and they match
     *     else return DENY.
     */
    if (fnmatch(sudoers_cmnd, cmnd, FNM_PATHNAME) != 0)
	debug_return_int(DENY);

    if (command_args_match(ctx, sudoers_cmnd, sudoers_args) == ALLOW) {
	/* Open the file for fdexec or for digest matching. */
	if (!open_cmnd(cmnd, digests, &fd))
	    goto bad;
#ifndef SUDOERS_NAME_MATCH
	if (!do_stat(fd, cmnd, &sb))
	    goto bad;
#endif
	/* Check digest of cmnd since sudoers_cmnd is a pattern. */
	if (digest_matches(fd, cmnd, digests) != ALLOW)
	    goto bad;
	set_cmnd_fd(ctx, fd, real_root);

	/* No need to set ctx->runas.cmnd since cmnd matches sudoers_cmnd */
	debug_return_int(ALLOW);
bad:
	if (fd != -1)
	    close(fd);
    }
    debug_return_int(DENY);
}

static int
command_matches_regex(struct sudoers_context *ctx, const char *sudoers_cmnd,
    const char *sudoers_args, int real_root,
    const struct command_digest_list *digests)
{
    const char *cmnd = ctx->user.cmnd;
    char buf[PATH_MAX];
    int len, fd = -1;
#ifndef SUDOERS_NAME_MATCH
    struct stat sb;
#endif
    debug_decl(command_matches_regex, SUDOERS_DEBUG_MATCH);

    /* A relative ctx->user.cmnd will not match, try canonicalized version. */
    if (ctx->user.cmnd[0] != '/') {
	if (ctx->user.cmnd_dir == NULL)
	    debug_return_int(DENY);
	len = snprintf(buf, sizeof(buf), "%s/%s", ctx->user.cmnd_dir,
	    ctx->user.cmnd_base);
	if (len < 0 || len >= ssizeof(buf))
	    debug_return_int(DENY);
	cmnd = buf;
    }

    /*
     * Return ALLOW if sudoers_cmnd regex matches cmnd AND
     *  a) there are no args in sudoers OR
     *  b) there are no args on command line and none required by sudoers OR
     *  c) there are args in sudoers and on command line and they match
     *     else return DENY.
     */
    if (regex_matches(sudoers_cmnd, cmnd) != ALLOW)
	debug_return_int(DENY);

    if (command_args_match(ctx, sudoers_cmnd, sudoers_args) == ALLOW) {
	/* Open the file for fdexec or for digest matching. */
	if (!open_cmnd(cmnd, digests, &fd))
	    goto bad;
#ifndef SUDOERS_NAME_MATCH
	if (!do_stat(fd, cmnd, &sb))
	    goto bad;
#endif
	/* Check digest of cmnd since sudoers_cmnd is a pattern. */
	if (digest_matches(fd, cmnd, digests) != ALLOW)
	    goto bad;
	set_cmnd_fd(ctx, fd, real_root);

	/* No need to set ctx->runas.cmnd since cmnd matches sudoers_cmnd */
	debug_return_int(ALLOW);
bad:
	if (fd != -1)
	    close(fd);
    }
    debug_return_int(DENY);
}

#ifndef SUDOERS_NAME_MATCH
static int
command_matches_glob(struct sudoers_context *ctx, const char *sudoers_cmnd,
    const char *sudoers_args, int real_root,
    const struct command_digest_list *digests)
{
    struct stat sudoers_stat;
    bool bad_digest = false;
    char **ap, *base, *cp;
    int fd = -1;
    size_t dlen;
    glob_t gl;
    debug_decl(command_matches_glob, SUDOERS_DEBUG_MATCH);

    /*
     * First check to see if we can avoid the call to glob(3).
     * Short circuit if there are no meta chars in the command itself
     * and ctx->user.cmnd_base and basename(sudoers_cmnd) don't match.
     */
    dlen = strlen(sudoers_cmnd);
    if (sudoers_cmnd[dlen - 1] != '/') {
	base = sudo_basename(sudoers_cmnd);
	if (!has_meta(base) && strcmp(ctx->user.cmnd_base, base) != 0)
	    debug_return_int(DENY);
    }

    /*
     * Return ALLOW if we find a match in the glob(3) results AND
     *  a) there are no args in sudoers OR
     *  b) there are no args on command line and none required by sudoers OR
     *  c) there are args in sudoers and on command line and they match
     * else return DENY.
     */
    if (glob(sudoers_cmnd, GLOB_NOSORT, NULL, &gl) != 0 || gl.gl_pathc == 0) {
	globfree(&gl);
	debug_return_int(DENY);
    }

    /* If ctx->user.cmnd is fully-qualified, check for an exact match. */
    if (ctx->user.cmnd[0] == '/') {
	for (ap = gl.gl_pathv; (cp = *ap) != NULL; ap++) {
	    if (fd != -1) {
		close(fd);
		fd = -1;
	    }

	    if (strcmp(cp, ctx->user.cmnd) != 0)
		continue;
	    /* Open the file for fdexec or for digest matching. */
	    if (!open_cmnd(cp, digests, &fd))
		continue;
	    if (!do_stat(fd, cp, &sudoers_stat))
		continue;
	    if (ctx->user.cmnd_stat == NULL ||
		(ctx->user.cmnd_stat->st_dev == sudoers_stat.st_dev &&
		ctx->user.cmnd_stat->st_ino == sudoers_stat.st_ino)) {
		/* There could be multiple matches, check digest early. */
		if (digest_matches(fd, cp, digests) != ALLOW) {
		    bad_digest = true;
		    continue;
		}
		free(ctx->runas.cmnd);
		if ((ctx->runas.cmnd = strdup(cp)) == NULL) {
		    sudo_warnx(U_("%s: %s"), __func__,
			U_("unable to allocate memory"));
		    cp = NULL;		/* fail closed */
		}
	    } else {
		/* Paths match, but st_dev and st_ino are different. */
		cp = NULL;		/* fail closed */
	    }
	    goto done;
	}
    }
    /* No exact match, compare basename, cmnd_dir, st_dev and st_ino. */
    if (!bad_digest) {
	for (ap = gl.gl_pathv; (cp = *ap) != NULL; ap++) {
	    if (fd != -1) {
		close(fd);
		fd = -1;
	    }

	    /* If it ends in '/' it is a directory spec. */
	    dlen = strlen(cp);
	    if (cp[dlen - 1] == '/') {
		if (command_matches_dir(ctx, cp, dlen, real_root, digests) == ALLOW) {
		    globfree(&gl);
		    debug_return_int(ALLOW);
		}
		continue;
	    }

	    /* Only proceed if ctx->user.cmnd_base and basename(cp) match */
	    base = sudo_basename(cp);
	    if (strcmp(ctx->user.cmnd_base, base) != 0)
		continue;

	    /* Compare the canonicalized parent directories, if possible. */
	    if (ctx->user.cmnd_dir != NULL) {
		char *slash = strrchr(cp, '/');
		if (slash != NULL) {
		    char *resolved;
		    *slash = '\0';
		    resolved = canon_path(cp);
		    *slash = '/';
		    if (resolved != NULL) {
			/* Canonicalized directories must match. */
			int result = strcmp(resolved, ctx->user.cmnd_dir);
			canon_path_free(resolved);
			if (result != 0)
			    continue;
		    }
		}
	    }

	    /* Open the file for fdexec or for digest matching. */
	    if (!open_cmnd(cp, digests, &fd))
		continue;
	    if (!do_stat(fd, cp, &sudoers_stat))
		continue;
	    if (ctx->user.cmnd_stat == NULL ||
		(ctx->user.cmnd_stat->st_dev == sudoers_stat.st_dev &&
		ctx->user.cmnd_stat->st_ino == sudoers_stat.st_ino)) {
		if (digest_matches(fd, cp, digests) != ALLOW)
		    continue;
		free(ctx->runas.cmnd);
		if ((ctx->runas.cmnd = strdup(cp)) == NULL) {
		    sudo_warnx(U_("%s: %s"), __func__,
			U_("unable to allocate memory"));
		    cp = NULL;		/* fail closed */
		}
		goto done;
	    }
	}
    }
done:
    globfree(&gl);
    if (cp != NULL) {
	if (command_args_match(ctx, sudoers_cmnd, sudoers_args) == ALLOW) {
	    /* ctx->runas.cmnd was set above. */
	    set_cmnd_fd(ctx, fd, real_root);
	    debug_return_int(ALLOW);
	}
    }
    if (fd != -1)
	close(fd);
    debug_return_int(DENY);
}

static int
command_matches_normal(struct sudoers_context *ctx, const char *sudoers_cmnd,
    const char *sudoers_args, int real_root,
    const struct command_digest_list *digests)
{
    struct stat sudoers_stat;
    const char *base;
    size_t dlen;
    int fd = -1;
    debug_decl(command_matches_normal, SUDOERS_DEBUG_MATCH);

    /* If it ends in '/' it is a directory spec. */
    dlen = strlen(sudoers_cmnd);
    if (sudoers_cmnd[dlen - 1] == '/') {
	debug_return_int(command_matches_dir(ctx, sudoers_cmnd, dlen,
	    real_root, digests));
    }

    /* Only proceed if ctx->user.cmnd_base and basename(sudoers_cmnd) match */
    base = sudo_basename(sudoers_cmnd);
    if (strcmp(ctx->user.cmnd_base, base) != 0)
	debug_return_int(DENY);

    /* Compare the canonicalized parent directories, if possible. */
    if (ctx->user.cmnd_dir != NULL) {
	const char *slash = strrchr(sudoers_cmnd, '/');
	if (slash != NULL) {
	    char sudoers_cmnd_dir[PATH_MAX], *resolved;
	    const size_t len = (size_t)(slash - sudoers_cmnd);
	    if (len >= sizeof(sudoers_cmnd_dir))
		goto bad;
	    if (len != 0)
		memcpy(sudoers_cmnd_dir, sudoers_cmnd, len);
	    sudoers_cmnd_dir[len] = '\0';
	    resolved = canon_path(sudoers_cmnd_dir);
	    if (resolved != NULL) {
		if (strcmp(resolved, ctx->user.cmnd_dir) != 0) {
		    canon_path_free(resolved);
		    goto bad;
		}
		canon_path_free(resolved);
	    }
	}
    }

    /* Open the file for fdexec or for digest matching. */
    if (!open_cmnd(sudoers_cmnd, digests, &fd))
	goto bad;

    /*
     * Return true if command matches AND
     *  a) there are no args in sudoers OR
     *  b) there are no args on command line and none req by sudoers OR
     *  c) there are args in sudoers and on command line and they match
     *  d) there is a digest and it matches
     */
    if (ctx->user.cmnd_stat != NULL && do_stat(fd, sudoers_cmnd, &sudoers_stat)) {
	if (ctx->user.cmnd_stat->st_dev != sudoers_stat.st_dev ||
	    ctx->user.cmnd_stat->st_ino != sudoers_stat.st_ino)
	    goto bad;
    } else {
	/* Either user or sudoers command does not exist, match by name. */
	if (strcmp(ctx->user.cmnd, sudoers_cmnd) != 0)
	    goto bad;
    }
    if (command_args_match(ctx, sudoers_cmnd, sudoers_args) != ALLOW)
	goto bad;
    if (digest_matches(fd, sudoers_cmnd, digests) != ALLOW) {
	/* XXX - log functions not available but we should log very loudly */
	goto bad;
    }
    free(ctx->runas.cmnd);
    if ((ctx->runas.cmnd = strdup(sudoers_cmnd)) == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	goto bad;
    }
    set_cmnd_fd(ctx, fd, real_root);
    debug_return_int(ALLOW);
bad:
    if (fd != -1)
	close(fd);
    debug_return_int(DENY);
}
#else /* SUDOERS_NAME_MATCH */
static int
command_matches_glob(struct sudoers_context *ctx, const char *sudoers_cmnd,
    const char *sudoers_args, int real_root,
    const struct command_digest_list *digests)
{
    return command_matches_fnmatch(ctx, sudoers_cmnd, sudoers_args, real_root,
	digests);
}

static int
command_matches_normal(struct sudoers_context *ctx, const char *sudoers_cmnd,
    const char *sudoers_args, int real_root,
    const struct command_digest_list *digests)
{
    size_t dlen;
    int fd = -1;
    debug_decl(command_matches_normal, SUDOERS_DEBUG_MATCH);

    /* If it ends in '/' it is a directory spec. */
    dlen = strlen(sudoers_cmnd);
    if (sudoers_cmnd[dlen - 1] == '/') {
	debug_return_int(command_matches_dir(ctx, sudoers_cmnd, dlen, real_root,
	    digests));
    }

    if (strcmp(ctx->user.cmnd, sudoers_cmnd) == 0) {
	if (command_args_match(ctx, sudoers_cmnd, sudoers_args) == ALLOW) {
	    /* Open the file for fdexec or for digest matching. */
	    if (!open_cmnd(ctx->user.cmnd, digests, &fd))
		goto bad;
	    if (digest_matches(fd, ctx->user.cmnd, digests) != ALLOW)
		goto bad;

	    /* Successful match. */
	    free(ctx->runas.cmnd);
	    if ((ctx->runas.cmnd = strdup(sudoers_cmnd)) == NULL) {
		sudo_warnx(U_("%s: %s"), __func__,
		    U_("unable to allocate memory"));
		goto bad;
	    }
	    set_cmnd_fd(ctx, fd, real_root);
	    debug_return_int(ALLOW);
	}
    }
bad:
    if (fd != -1)
	close(fd);
    debug_return_int(DENY);
}
#endif /* SUDOERS_NAME_MATCH */

/*
 * If path doesn't end in /, return ALLOW iff cmnd & path name the same inode;
 * otherwise, return ALLOW if ctx->user.cmnd names one of the inodes in path.
 * Returns DENY on failure.
 */
int
command_matches(struct sudoers_context *ctx, const char *sudoers_cmnd,
    const char *sudoers_args, const char *runchroot, struct cmnd_info *info,
    const struct command_digest_list *digests)
{
    struct sudoers_pivot pivot_state = SUDOERS_PIVOT_INITIALIZER;
    char *saved_user_cmnd = NULL;
    struct stat saved_user_stat;
    bool reset_cmnd = false;
    int real_root = -1;
    int ret = DENY;
    debug_decl(command_matches, SUDOERS_DEBUG_MATCH);

    if (ctx->runas.chroot != NULL) {
	if (runchroot != NULL && strcmp(runchroot, "*") != 0 &&
		strcmp(runchroot, ctx->runas.chroot) != 0) {
	    /* CHROOT mismatch */
	    goto done;
	}
	/* User-specified runchroot (cmnd_stat already set appropriately). */
	runchroot = ctx->runas.chroot;
    } else if (runchroot == NULL) {
	/* No rule-specific runchroot, use global (cmnd_stat already set). */
	if (def_runchroot != NULL && strcmp(def_runchroot, "*") != '\0')
	    runchroot = def_runchroot;
    } else {
	/* Rule-specific runchroot, must reset cmnd and cmnd_stat. */
	reset_cmnd = true;
    }

    /* Pivot root. */
    if (runchroot != NULL) {
	if (!pivot_root(runchroot, &pivot_state))
	    goto done;
	real_root = pivot_state.saved_root;
    }

    if (reset_cmnd) {
	/* Rule-specific runchroot, set cmnd and cmnd_stat after pivot. */
	int status;

	/* Save old ctx->user.cmnd first, set_cmnd_path() will free it. */
	saved_user_cmnd = ctx->user.cmnd;
	ctx->user.cmnd = NULL;
	if (ctx->user.cmnd_stat != NULL)
	    saved_user_stat = *ctx->user.cmnd_stat;
	status = set_cmnd_path(ctx, NULL);
	if (status != FOUND) {
	    ctx->user.cmnd = saved_user_cmnd;
	    saved_user_cmnd = NULL;
	}
	if (info != NULL)
	    info->status = status;
    }

    if (sudoers_cmnd == NULL) {
	sudoers_cmnd = "ALL";
	ret = command_matches_all(ctx, real_root, digests);
	goto done;
    }

    /* Check for regular expressions first. */
    if (sudoers_cmnd[0] == '^') {
	ret = command_matches_regex(ctx, sudoers_cmnd, sudoers_args, real_root,
	    digests);
	goto done;
    }

    /* Check for pseudo-commands */
    if (sudoers_cmnd[0] != '/') {
	/*
	 * Return true if sudoers_cmnd and cmnd match a pseudo-command AND
	 *  a) there are no args in sudoers OR
	 *  b) there are no args on command line and none req by sudoers OR
	 *  c) there are args in sudoers and on command line and they match
	 */
	if (strcmp(sudoers_cmnd, "list") == 0 ||
		strcmp(sudoers_cmnd, "sudoedit") == 0) {
	    if (strcmp(ctx->user.cmnd, sudoers_cmnd) == 0 &&
		    command_args_match(ctx, sudoers_cmnd, sudoers_args) == ALLOW) {
		/* No need to set ctx->user.cmnd since cmnd == sudoers_cmnd */
		ret = ALLOW;
	    }
	}
	goto done;
    }

    if (has_meta(sudoers_cmnd)) {
	/*
	 * If sudoers_cmnd has meta characters in it, we need to
	 * use glob(3) and/or fnmatch(3) to do the matching.
	 */
	if (def_fast_glob) {
	    ret = command_matches_fnmatch(ctx, sudoers_cmnd, sudoers_args,
		real_root, digests);
	} else {
	    ret = command_matches_glob(ctx, sudoers_cmnd, sudoers_args,
		real_root, digests);
	}
    } else {
	ret = command_matches_normal(ctx, sudoers_cmnd, sudoers_args,
	    real_root, digests);
    }
done:
    /* Restore root. */
    if (runchroot != NULL)
	(void)unpivot_root(&pivot_state);

    /* Restore ctx->user.cmnd and ctx->user.cmnd_stat. */
    if (saved_user_cmnd != NULL) {
	if (info != NULL) {
	    free(info->cmnd_path);
	    info->cmnd_path = ctx->user.cmnd;
	    if (ctx->user.cmnd_stat != NULL)
		info->cmnd_stat = *ctx->user.cmnd_stat;
	} else {
	    free(ctx->user.cmnd);
	}
	ctx->user.cmnd = saved_user_cmnd;
	if (ctx->user.cmnd_stat != NULL)
	    *ctx->user.cmnd_stat = saved_user_stat;
    }
    sudo_debug_printf(SUDO_DEBUG_DEBUG|SUDO_DEBUG_LINENO,
	"user command \"%s%s%s\" matches sudoers command \"%s%s%s\"%s%s: %s",
	ctx->user.cmnd, ctx->user.cmnd_args ? " " : "",
	ctx->user.cmnd_args ? ctx->user.cmnd_args : "", sudoers_cmnd,
	sudoers_args ? " " : "", sudoers_args ? sudoers_args : "",
	runchroot ? ", chroot " : "", runchroot ? runchroot : "",
	ret == ALLOW ? "ALLOW" : "DENY");
    debug_return_int(ret);
}
