/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2015-2021 Todd C. Miller <Todd.Miller@sudo.ws>
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

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <fcntl.h>

#include <sudo.h>
#include <sudo_edit.h>

#if defined(HAVE_SETRESUID) || defined(HAVE_SETREUID) || defined(HAVE_SETEUID)

static int
switch_user_int(uid_t euid, gid_t egid, int ngroups, GETGROUPS_T *groups,
    bool nonfatal)
{
    int serrno = errno;
    int ret = -1;
    debug_decl(switch_user, SUDO_DEBUG_EDIT);

    sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
	"set uid:gid to %u:%u(%u)", (unsigned int)euid, (unsigned int)egid,
	ngroups > 0 ? (unsigned int)groups[0] : (unsigned int)egid);

    /* When restoring root, change euid first; otherwise change it last. */
    if (euid == ROOT_UID) {
	if (seteuid(ROOT_UID) != 0) {
	    if (nonfatal)
		goto done;
	    sudo_fatal("seteuid(ROOT_UID)");
	}
    }
    if (setegid(egid) != 0) {
	if (nonfatal)
	    goto done;
	sudo_fatal("setegid(%d)", (int)egid);
    }
    if (ngroups != -1) {
	if (sudo_setgroups(ngroups, groups) != 0) {
	    if (nonfatal)
		goto done;
	    sudo_fatal("setgroups");
	}
    }
    if (euid != ROOT_UID) {
	if (seteuid(euid) != 0) {
	    if (nonfatal)
		goto done;
	    sudo_fatal("seteuid(%u)", (unsigned int)euid);
	}
    }
    ret = 0;

done:
    errno = serrno;
    debug_return_int(ret);
}

#if defined(HAVE_FACCESSAT) && defined(AT_EACCESS)
static int
switch_user_nonfatal(uid_t euid, gid_t egid, int ngroups, GETGROUPS_T *groups)
{
    return switch_user_int(euid, egid, ngroups, groups, true);
}
#endif

void
switch_user(uid_t euid, gid_t egid, int ngroups, GETGROUPS_T *groups)
{
    (void)switch_user_int(euid, egid, ngroups, groups, false);
}

static bool
group_matches(gid_t target, const struct sudo_cred *cred)
{
    int i;
    debug_decl(group_matches, SUDO_DEBUG_EDIT);

    if (target == cred->gid) {
	sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
	    "user gid %u matches directory gid %u", (unsigned int)cred->gid,
	    (unsigned int)target);
	debug_return_bool(true);
    }
    for (i = 0; i < cred->ngroups; i++) {
	if (target == cred->groups[i]) {
	    sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
		"user gid %u matches directory gid %u",
		(unsigned int)cred->groups[i], (unsigned int)target);
	    debug_return_bool(true);
	}
    }
    debug_return_bool(false);
}

static bool
is_writable(const struct sudo_cred *user_cred, struct stat *sb)
{
    debug_decl(is_writable, SUDO_DEBUG_EDIT);

    /* Other writable? */
    if (ISSET(sb->st_mode, S_IWOTH)) {
	sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
	    "directory is writable by other");
	debug_return_int(true);
    }

    /* Group writable? */
    if (ISSET(sb->st_mode, S_IWGRP)) {
	if (group_matches(sb->st_gid, user_cred)) {
	    sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
		"directory is writable by one of the user's groups");
	    debug_return_int(true);
	}
    }

    errno = EACCES;
    debug_return_int(false);
}

#if defined(HAVE_FACCESSAT) && defined(AT_EACCESS)
/*
 * Checks whether the open directory dfd is owned or writable by the user.
 * Returns true if writable, false if not, or -1 on error.
 */
int
dir_is_writable(int dfd, const struct sudo_cred *user_cred,
    const struct sudo_cred *cur_cred)
{
    struct stat sb;
    int rc;
    debug_decl(dir_is_writable, SUDO_DEBUG_EDIT);

    if (fstat(dfd, &sb) == -1)
	debug_return_int(-1);

    /* If the user owns the dir we always consider it writable. */
    if (sb.st_uid == user_cred->uid) {
	sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
	    "user uid %u matches directory uid %u",
	    (unsigned int)user_cred->uid, (unsigned int)sb.st_uid);
	debug_return_int(true);
    }

    /* Change uid/gid/groups to invoking user, usually needs root perms. */
    if (cur_cred->euid != ROOT_UID) {
	if (seteuid(ROOT_UID) != 0) {
	    sudo_debug_printf(
		SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO|SUDO_DEBUG_ERRNO,
		"seteuid(ROOT_UID)");
	    goto fallback;
	}
    }
    if (switch_user_nonfatal(user_cred->uid, user_cred->gid, user_cred->ngroups,
	    user_cred->groups) == -1) {
	sudo_debug_printf(
	    SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO|SUDO_DEBUG_ERRNO,
	    "unable to switch to user_cred");
	goto fallback;
    }

    /* Access checks are done using the euid/egid and group vector. */
    rc = faccessat(dfd, ".", W_OK, AT_EACCESS);

    /* Restore uid/gid/groups, may need root perms. */
    if (user_cred->uid != ROOT_UID) {
	if (seteuid(ROOT_UID) != 0)
	    sudo_fatal("seteuid(ROOT_UID)");
    }
    switch_user(cur_cred->euid, cur_cred->egid, cur_cred->ngroups,
	cur_cred->groups);

    if (rc == 0)
	debug_return_int(true);
    if (errno == EACCES || errno == EPERM || errno == EROFS)
	debug_return_int(false);
    debug_return_int(-1);

fallback:
    debug_return_int(is_writable(user_cred, &sb));
}
#endif /* HAVE_FACCESSAT && AT_EACCESS */

#if !defined(HAVE_FACCESSAT) || !defined(AT_EACCESS)
/*
 * Checks whether the open directory dfd is owned or writable by the user.
 * Returns true if writable, false if not, or -1 on error.
 */
int
dir_is_writable(int dfd, const struct sudo_cred *user_cred,
    const struct sudo_cred *cur_cred)
{
    struct stat sb;
    debug_decl(dir_is_writable, SUDO_DEBUG_EDIT);

    if (fstat(dfd, &sb) == -1)
	debug_return_int(-1);

    /* If the user owns the dir we always consider it writable. */
    if (sb.st_uid == user_cred->uid) {
	sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
	    "user uid %u matches directory uid %u",
	    (unsigned int)user_cred->uid, (unsigned int)sb.st_uid);
	debug_return_int(true);
    }

    debug_return_int(is_writable(user_cred, &sb));
}
#endif /* HAVE_FACCESSAT && AT_EACCESS */

#ifdef O_NOFOLLOW
static int
sudo_edit_openat_nofollow(int dfd, char *path, int oflags, mode_t mode)
{
    int fd;
    debug_decl(sudo_edit_openat_nofollow, SUDO_DEBUG_EDIT);

    fd = openat(dfd, path, oflags|O_NOFOLLOW, mode);
    if (fd == -1) {
	/* Handle non-standard O_NOFOLLOW errno values. */
	if (errno == EMLINK)
	    errno = ELOOP;		/* FreeBSD */
#ifdef EFTYPE
	else if (errno == EFTYPE)
	    errno = ELOOP;		/* NetBSD */
#endif
    }

    debug_return_int(fd);
}
#else
/*
 * Returns true if fd and path don't match or path is a symlink.
 * Used on older systems without O_NOFOLLOW.
 */
static bool
sudo_edit_is_symlink(int fd, char *path)
{
    struct stat sb1, sb2;
    debug_decl(sudo_edit_is_symlink, SUDO_DEBUG_EDIT);

    /*
     * Treat [fl]stat() failure like there was a symlink.
     */
    if (fstat(fd, &sb1) == -1 || lstat(path, &sb2) == -1)
	debug_return_bool(true);

    /*
     * Make sure we did not open a link and that what we opened
     * matches what is currently on the file system.
     */
    if (S_ISLNK(sb2.st_mode) ||
	sb1.st_dev != sb2.st_dev || sb1.st_ino != sb2.st_ino) {
	debug_return_bool(true);
    }

    debug_return_bool(false);
}

static int
sudo_edit_openat_nofollow(int dfd, char *path, int oflags, mode_t mode)
{
    int fd = -1, odfd = -1;
    struct stat sb;
    debug_decl(sudo_edit_openat_nofollow, SUDO_DEBUG_EDIT);

    /* Save cwd and chdir to dfd */
    if ((odfd = open(".", O_RDONLY)) == -1)
	debug_return_int(-1);
    if (fchdir(dfd) == -1) {
	close(odfd);
	debug_return_int(-1);
    }

    /*
     * Check if path is a symlink.  This is racey but we detect whether
     * we lost the race in sudo_edit_is_symlink() after the open.
     */
    if (lstat(path, &sb) == -1 && errno != ENOENT)
	goto done;
    if (S_ISLNK(sb.st_mode)) {
	errno = ELOOP;
	goto done;
    }

    fd = open(path, oflags, mode);
    if (fd == -1)
	goto done;

    /*
     * Post-open symlink check.  This will leave a zero-length file if
     * O_CREAT was specified but it is too dangerous to try and remove it.
     */
    if (sudo_edit_is_symlink(fd, path)) {
	close(fd);
	fd = -1;
	errno = ELOOP;
    }

done:
    /* Restore cwd */
    if (odfd != -1) {
	if (fchdir(odfd) == -1)
	    sudo_fatal("%s", U_("unable to restore current working directory"));
	close(odfd);
    }

    debug_return_int(fd);
}
#endif /* O_NOFOLLOW */

static int
sudo_edit_open_nonwritable(char *path, int oflags, mode_t mode,
    const struct sudo_cred *user_cred, const struct sudo_cred *cur_cred)
{
    const int dflags = DIR_OPEN_FLAGS;
    int dfd, fd, writable;
    debug_decl(sudo_edit_open_nonwritable, SUDO_DEBUG_EDIT);

    if (path[0] == '/') {
	dfd = open("/", dflags);
	path++;
    } else {
	dfd = open(".", dflags);
	if (path[0] == '.' && path[1] == '/')
	    path += 2;
    }
    if (dfd == -1)
	debug_return_int(-1);

    for (;;) {
	char *slash;
	int subdfd;

	/*
	 * Look up one component at a time, avoiding symbolic links in
	 * writable directories.
	 */
	writable = dir_is_writable(dfd, user_cred, cur_cred);
	if (writable == -1) {
	    close(dfd);
	    debug_return_int(-1);
	}

	path += strspn(path, "/");
	slash = strchr(path, '/');
	if (slash == NULL)
	    break;
	*slash = '\0';
	if (writable)
	    subdfd = sudo_edit_openat_nofollow(dfd, path, dflags, 0);
	else
	    subdfd = openat(dfd, path, dflags, 0);
	*slash = '/';			/* restore path */
	close(dfd);
	if (subdfd == -1)
	    debug_return_int(-1);
	path = slash + 1;
	dfd = subdfd;
    }

    if (writable) {
	close(dfd);
	errno = EISDIR;
	debug_return_int(-1);
    }

    /*
     * For "sudoedit /" we will receive ENOENT from openat() and sudoedit
     * will try to create a file with an empty name.  We treat an empty
     * path as the cwd so sudoedit can give a sensible error message.
     */
    fd = openat(dfd, *path ? path : ".", oflags, mode);
    close(dfd);
    debug_return_int(fd);
}

#ifdef O_NOFOLLOW
int
sudo_edit_open(char *path, int oflags, mode_t mode, unsigned int sflags,
    const struct sudo_cred *user_cred, const struct sudo_cred *cur_cred)
{
    int fd;
    debug_decl(sudo_edit_open, SUDO_DEBUG_EDIT);

    if (!ISSET(sflags, CD_SUDOEDIT_FOLLOW))
	oflags |= O_NOFOLLOW;
    if (ISSET(sflags, CD_SUDOEDIT_CHECKDIR) && user_cred->uid != ROOT_UID) {
	fd = sudo_edit_open_nonwritable(path, oflags|O_NONBLOCK, mode,
	    user_cred, cur_cred);
    } else {
	fd = open(path, oflags|O_NONBLOCK, mode);
    }
    if (fd == -1 && ISSET(oflags, O_NOFOLLOW)) {
	/* Handle non-standard O_NOFOLLOW errno values. */
	if (errno == EMLINK)
	    errno = ELOOP;		/* FreeBSD */
#ifdef EFTYPE
	else if (errno == EFTYPE)
	    errno = ELOOP;		/* NetBSD */
#endif
    }
    if (fd != -1 && !ISSET(oflags, O_NONBLOCK))
	(void) fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) & ~O_NONBLOCK);
    debug_return_int(fd);
}
#else
int
sudo_edit_open(char *path, int oflags, mode_t mode, unsigned int sflags,
    const struct sudo_cred *user_cred, const struct sudo_cred *cur_cred)
{
    struct stat sb;
    int fd;
    debug_decl(sudo_edit_open, SUDO_DEBUG_EDIT);

    /*
     * Check if path is a symlink.  This is racey but we detect whether
     * we lost the race in sudo_edit_is_symlink() after the file is opened.
     */
    if (!ISSET(sflags, CD_SUDOEDIT_FOLLOW)) {
	if (lstat(path, &sb) == -1 && errno != ENOENT)
	    debug_return_int(-1);
	if (S_ISLNK(sb.st_mode)) {
	    errno = ELOOP;
	    debug_return_int(-1);
	}
    }

    if (ISSET(sflags, CD_SUDOEDIT_CHECKDIR) && user_cred->uid != ROOT_UID) {
	fd = sudo_edit_open_nonwritable(path, oflags|O_NONBLOCK, mode,
	    user_cred, cur_cred);
    } else {
	fd = open(path, oflags|O_NONBLOCK, mode);
    }
    if (fd == -1)
	debug_return_int(-1);
    if (!ISSET(oflags, O_NONBLOCK))
	(void) fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) & ~O_NONBLOCK);

    /*
     * Post-open symlink check.  This will leave a zero-length file if
     * O_CREAT was specified but it is too dangerous to try and remove it.
     */
    if (!ISSET(sflags, CD_SUDOEDIT_FOLLOW) && sudo_edit_is_symlink(fd, path)) {
	close(fd);
	fd = -1;
	errno = ELOOP;
    }

    debug_return_int(fd);
}
#endif /* O_NOFOLLOW */

/*
 * Verify that the parent dir of a new file exists and is not writable
 * by the user.  This fails early so the user knows ahead of time if the
 * edit won't succeed.  Additional checks are performed when copying the
 * temporary file back to the origin so there are no TOCTOU issues.
 * Does not modify the value of errno.
 */
bool
sudo_edit_parent_valid(char *path, unsigned int sflags,
    const struct sudo_cred *user_cred, const struct sudo_cred *cur_cred)
{
    const int serrno = errno;
    struct stat sb;
    bool ret = false;
    char *slash;
    char pathbuf[2];
    int dfd;
    debug_decl(sudo_edit_parent_valid, SUDO_DEBUG_EDIT);

    /* Get dirname of path (the slash is restored later). */
    slash = strrchr(path, '/');
    if (slash == NULL) {
	/* cwd */
	pathbuf[0] = '.';
	pathbuf[1] = '\0';
	path = pathbuf;
    } else if (slash == path) {
	pathbuf[0] = '/';
	pathbuf[1] = '\0';
	path = pathbuf;
	slash = NULL;
    } else {
	*slash = '\0';
    }

    /*
     * The parent directory is allowed to be a symbolic link unless
     * *its* parent is writable and CD_SUDOEDIT_CHECK is set.
     */
    dfd = sudo_edit_open(path, DIR_OPEN_FLAGS, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH,
	sflags|CD_SUDOEDIT_FOLLOW, user_cred, cur_cred);
    if (dfd != -1) {
	if (fstat(dfd, &sb) == 0 && S_ISDIR(sb.st_mode))
	    ret = true;
	close(dfd);
    }
    if (slash != NULL)
	*slash = '/';

    /* Restore errno. */
    errno = serrno;

    debug_return_bool(ret);
}

#endif /* HAVE_SETRESUID || HAVE_SETREUID || HAVE_SETEUID */
