/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2014-2023 Todd C. Miller <Todd.Miller@sudo.ws>
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
#include <sys/ioctl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#if defined(HAVE_STDINT_H)
# include <stdint.h>
#elif defined(HAVE_INTTYPES_H)
# include <inttypes.h>
#endif
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <signal.h>

#include <sudoers.h>
#include <timestamp.h>

#define TIMESTAMP_OPEN_ERROR	-1
#define TIMESTAMP_PERM_ERROR	-2

/*
 * Each user has a single time stamp file that contains multiple records.
 * Records are locked to ensure that changes are serialized.
 *
 * The first record is of type TS_LOCKEXCL and is used to gain exclusive
 * access to create new records.  This is a short-term lock and sudo
 * should not sleep while holding it (or the user will not be able to sudo).
 * The TS_LOCKEXCL entry must be unlocked before locking the actual record.
 */

struct ts_cookie {
    const struct sudoers_context *ctx;
    char *fname;
    int fd;
    bool locked;
    off_t pos;
    struct timestamp_entry key;
};

static uid_t timestamp_uid = ROOT_UID;
static gid_t timestamp_gid = ROOT_GID;

uid_t
timestamp_get_uid(void)
{
    return timestamp_uid;
}

/*
 * Returns true if entry matches key, else false.
 * We don't match on the sid or actual time stamp.
 */
static bool
ts_match_record(struct timestamp_entry *key, struct timestamp_entry *entry,
    unsigned int recno)
{
    debug_decl(ts_match_record, SUDOERS_DEBUG_AUTH);

    if (entry->version != key->version) {
	sudo_debug_printf(SUDO_DEBUG_DEBUG,
	    "%s:%u record version mismatch (want %u, got %u)", __func__, recno,
	    key->version, entry->version);
	debug_return_bool(false);
    }
    if (!ISSET(key->flags, TS_ANYUID) && entry->auth_uid != key->auth_uid) {
	sudo_debug_printf(SUDO_DEBUG_DEBUG,
	    "%s:%u record uid mismatch (want %u, got %u)", __func__, recno,
	    (unsigned int)key->auth_uid, (unsigned int)entry->auth_uid);
	debug_return_bool(false);
    }
    if (entry->type != key->type) {
	sudo_debug_printf(SUDO_DEBUG_DEBUG,
	    "%s:%u record type mismatch (want %u, got %u)", __func__, recno,
	    key->type, entry->type);
	debug_return_bool(false);
    }
    switch (entry->type) {
    case TS_GLOBAL:
	/* no ppid or tty to match */
	break;
    case TS_PPID:
	/* verify parent pid */
	if (entry->u.ppid != key->u.ppid) {
	    sudo_debug_printf(SUDO_DEBUG_DEBUG,
		"%s:%u record ppid mismatch (want %d, got %d)", __func__, recno,
		(int)key->u.ppid, (int)entry->u.ppid);
	    debug_return_bool(false);
	}
	if (sudo_timespeccmp(&entry->start_time, &key->start_time, !=)) {
	    sudo_debug_printf(SUDO_DEBUG_DEBUG,
		"%s:%u ppid start time mismatch", __func__, recno);
	    debug_return_bool(false);
	}
	break;
    case TS_TTY:
	if (entry->u.ttydev != key->u.ttydev) {
	    sudo_debug_printf(SUDO_DEBUG_DEBUG,
		"%s:%u record tty mismatch (want 0x%x, got 0x%x)", __func__,
		recno, (unsigned int)key->u.ttydev, (unsigned int)entry->u.ttydev);
	    debug_return_bool(false);
	}
	if (sudo_timespeccmp(&entry->start_time, &key->start_time, !=)) {
	    sudo_debug_printf(SUDO_DEBUG_DEBUG,
		"%s:%u session leader start time mismatch", __func__, recno);
	    debug_return_bool(false);
	}
	break;
    default:
	/* unknown record type, ignore it */
	sudo_debug_printf(SUDO_DEBUG_WARN|SUDO_DEBUG_LINENO,
	    "%s:%u unknown time stamp record type %d", __func__, recno,
	    entry->type);
	debug_return_bool(false);
    }
    debug_return_bool(true);
}

/*
 * Searches the time stamp file descriptor for a record that matches key.
 * On success, fills in entry with the matching record and returns true.
 * On failure, returns false.
 *
 * Note that records are searched starting at the current file offset,
 * which may not be the beginning of the file.
 */
static bool
ts_find_record(int fd, struct timestamp_entry *key, struct timestamp_entry *entry)
{
    struct timestamp_entry cur;
    unsigned int recno = 0;
    debug_decl(ts_find_record, SUDOERS_DEBUG_AUTH);

    /*
     * Find a matching record (does not match sid or time stamp value).
     */
    while (read(fd, &cur, sizeof(cur)) == sizeof(cur)) {
	recno++;
	if (cur.size != sizeof(cur)) {
	    /* wrong size, seek to start of next record */
	    sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
		"wrong sized record, got %hu, expected %zu",
		cur.size, sizeof(cur));
	    if (lseek(fd, (off_t)cur.size - (off_t)sizeof(cur), SEEK_CUR) == -1) {
		sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_ERRNO|SUDO_DEBUG_LINENO,
		    "unable to seek forward %zd",
		    cur.size - ssizeof(cur));
		break;
	    }
	    if (cur.size == 0)
		break;			/* size must be non-zero */
	    continue;
	}
	if (ts_match_record(key, &cur, recno)) {
	    memcpy(entry, &cur, sizeof(struct timestamp_entry));
	    debug_return_bool(true);
	}
    }
    debug_return_bool(false);
}

/*
 * Create a directory and any missing parent directories with the
 * specified mode.
 * Returns an fd usable with the *at() functions on success.
 * Returns -1 on failure, setting errno.
 */
static int
ts_mkdirs(const char *path, uid_t owner, gid_t group, mode_t mode,
    mode_t parent_mode, bool quiet)
{
    int parentfd, fd = -1;
    const char *base;
    mode_t omask;
    debug_decl(ts_mkdirs, SUDOERS_DEBUG_AUTH);

    /* Child directory we will create. */
    base = sudo_basename(path);

    /* umask must not be more restrictive than the file modes. */
    omask = umask(ACCESSPERMS & ~(mode|parent_mode));
    parentfd = sudo_open_parent_dir(path, owner, group, parent_mode, quiet);
    if (parentfd != -1) {
	/* Create final path component. */
	sudo_debug_printf(SUDO_DEBUG_DEBUG|SUDO_DEBUG_LINENO,
	    "mkdir %s, mode 0%o, uid %d, gid %d", path, (unsigned int)mode,
	    (int)owner, (int)group);
	if (mkdirat(parentfd, base, mode) != 0 && errno != EEXIST) {
	    if (!quiet)
		sudo_warn(U_("unable to mkdir %s"), path);
	} else {
	    fd = openat(parentfd, base, O_RDONLY|O_NONBLOCK, 0);
	    if (fd == -1) {
		sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_ERRNO,
		    "%s: unable to open %s", __func__, path);
	    } else if (fchown(fd, owner, group) != 0) {
		sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_ERRNO,
		    "%s: unable to chown %d:%d %s", __func__,
		    (int)owner, (int)group, path);
	    }
	}
	close(parentfd);
    }
    umask(omask);
    debug_return_int(fd);
}

/*
 * Check that path is owned by timestamp_uid and not writable by
 * group or other.  If path is missing and make_it is true, create
 * the directory and its parent dirs.
 *
 * Returns an fd usable with the *at() functions on success.
 * Returns -1 on failure, setting errno.
 */
static int
ts_secure_opendir(const char *path, bool make_it, bool quiet)
{
    int error, fd;
    struct stat sb;
    debug_decl(ts_secure_opendir, SUDOERS_DEBUG_AUTH);

    sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO, "checking %s", path);
    fd = sudo_secure_open_dir(path, timestamp_uid, timestamp_gid, &sb, &error);
    if (fd == -1) {
	switch (error) {
	case SUDO_PATH_MISSING:
	    if (make_it) {
		fd = ts_mkdirs(path, timestamp_uid, timestamp_gid, S_IRWXU,
		    S_IRWXU|S_IXGRP|S_IXOTH, quiet);
		if (fd != -1)
		    break;
	    }
	    if (!quiet)
		sudo_warn("%s", path);
	    break;
	case SUDO_PATH_BAD_TYPE:
	    errno = ENOTDIR;
	    if (!quiet)
		sudo_warn("%s", path);
	    break;
	case SUDO_PATH_WRONG_OWNER:
	    if (!quiet) {
		sudo_warnx(U_("%s is owned by uid %u, should be %u"),
		    path, (unsigned int)sb.st_uid, (unsigned int)timestamp_uid);
	    }
	    errno = EACCES;
	    break;
	case SUDO_PATH_WORLD_WRITABLE:
	    if (!quiet)
		sudo_warnx(U_("%s is world writable"), path);
	    errno = EACCES;
	    break;
	case SUDO_PATH_GROUP_WRITABLE:
	    if (!quiet) {
		sudo_warnx(U_("%s is owned by gid %u, should be %u"),
		    path, (unsigned int)sb.st_gid, (unsigned int)timestamp_gid);
	    }
	    errno = EACCES;
	    break;
	default:
	    if (!quiet) {
		sudo_warnx("%s: internal error, unexpected error %d",
		    __func__, error);
		errno = EINVAL;
	    }
	    break;
	}
    }

    debug_return_int(fd);
}

/*
 * Open the specified timestamp or lecture file and set the
 * close on exec flag.
 * Returns open file descriptor on success.
 * Returns TIMESTAMP_OPEN_ERROR or TIMESTAMP_PERM_ERROR on error.
 */
static int
ts_openat(int dfd, const char *path, int flags)
{
    bool uid_changed = false;
    int fd;
    debug_decl(ts_openat, SUDOERS_DEBUG_AUTH);

    if (timestamp_uid != 0)
	uid_changed = set_perms(NULL, PERM_TIMESTAMP);
    fd = openat(dfd, path, flags, S_IRUSR|S_IWUSR);
    if (uid_changed && !restore_perms()) {
	/* Unable to restore permissions, should not happen. */
	if (fd != -1) {
	    int serrno = errno;
	    close(fd);
	    errno = serrno;
	    fd = TIMESTAMP_PERM_ERROR;
	}
    }
    if (fd >= 0)
	(void)fcntl(fd, F_SETFD, FD_CLOEXEC);

    debug_return_int(fd);
}

static ssize_t
ts_write(const struct sudoers_context *ctx, int fd, const char *fname,
    struct timestamp_entry *entry, off_t offset)
{
    ssize_t nwritten;
    off_t old_eof;
    debug_decl(ts_write, SUDOERS_DEBUG_AUTH);

    if (offset == -1) {
	old_eof = lseek(fd, 0, SEEK_CUR);
	if (old_eof == -1)
	    debug_return_ssize_t(-1);
	nwritten = write(fd, entry, entry->size);
    } else {
	old_eof = offset;
	nwritten = pwrite(fd, entry, entry->size, offset);
    }
    if ((size_t)nwritten != entry->size) {
	if (nwritten == -1) {
	    log_warning(ctx, SLOG_SEND_MAIL,
		N_("unable to write to %s"), fname);
	} else {
	    log_warningx(ctx, SLOG_SEND_MAIL,
		N_("unable to write to %s"), fname);
	}

	/* Truncate on partial write to be safe (assumes end of file). */
	if (nwritten > 0) {
	    sudo_debug_printf(SUDO_DEBUG_DEBUG|SUDO_DEBUG_LINENO,
		"short write, truncating partial time stamp record");
	    if (ftruncate(fd, old_eof) != 0) {
		sudo_warn(U_("unable to truncate time stamp file to %lld bytes"),
		    (long long)old_eof);
	    }
	}
	debug_return_ssize_t(-1);
    }
    debug_return_ssize_t(nwritten);
}

/*
 * Full in struct timestamp_entry with the specified flags
 * based on auth user pw.  Does not set the time stamp.
 */
static void
ts_init_key(const struct sudoers_context *ctx,
    struct timestamp_entry *entry, struct passwd *pw,
    unsigned short flags, enum def_tuple ticket_type)
{
    struct stat sb;
    debug_decl(ts_init_key, SUDOERS_DEBUG_AUTH);

    memset(entry, 0, sizeof(*entry));
    entry->version = TS_VERSION;
    entry->size = sizeof(*entry);
    entry->flags = flags;
    if (pw != NULL) {
	entry->auth_uid = pw->pw_uid;
    } else {
	entry->flags |= TS_ANYUID;
    }
    entry->sid = ctx->user.sid;
    switch (ticket_type) {
    default:
	/* Unknown time stamp ticket type, treat as tty (should not happen). */
	sudo_warnx("unknown time stamp ticket type %d", ticket_type);
	FALLTHROUGH;
    case tty:
	if (ctx->user.ttypath != NULL && stat(ctx->user.ttypath, &sb) == 0) {
	    /* tty-based time stamp */
	    entry->type = TS_TTY;
	    entry->u.ttydev = sb.st_rdev;
	    if (entry->sid != -1)
		get_starttime(entry->sid, &entry->start_time);
	    break;
	}
	FALLTHROUGH;
    case kernel:
    case ppid:
	/* ppid-based time stamp */
	entry->type = TS_PPID;
	entry->u.ppid = ctx->user.ppid;
	get_starttime(entry->u.ppid, &entry->start_time);
	break;
    case global:
	/* global time stamp */
	entry->type = TS_GLOBAL;
	break;
    }

    debug_return;
}

static void
ts_init_key_nonglobal(const struct sudoers_context *ctx,
    struct timestamp_entry *entry, struct passwd *pw, unsigned short flags)
{
    /*
     * Even if the timestamp type is global or kernel we still want to do
     * per-tty or per-ppid locking so sudo works predictably in a pipeline.
     */
    ts_init_key(ctx, entry, pw, flags,
	def_timestamp_type == ppid ? ppid : tty);
}

/*
 * Open the user's time stamp file.
 * Returns a cookie or NULL on error, does not lock the file.
 */
void *
timestamp_open(const struct sudoers_context *ctx)
{
    int tries, len, dfd = -1, fd = -1;
    char uidstr[STRLEN_MAX_UNSIGNED(uid_t) + 1];
    struct ts_cookie *cookie;
    char *fname = NULL;
    debug_decl(timestamp_open, SUDOERS_DEBUG_AUTH);

    /* Zero timeout means don't use the time stamp file. */
    if (!sudo_timespecisset(&def_timestamp_timeout)) {
	errno = ENOENT;
	goto bad;
    }

    /* Check the validity of timestamp dir and create if missing. */
    dfd = ts_secure_opendir(def_timestampdir, true, false);
    if (dfd == -1)
	goto bad;

    /* Open time stamp file. */
    len = snprintf(uidstr, sizeof(uidstr), "%u", (unsigned int)ctx->user.uid);
    if (len < 0 || len >= ssizeof(uidstr)) {
	errno = EINVAL;
	goto bad;
    }
    if (asprintf(&fname, "%s/%s", def_timestampdir, uidstr) == -1) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	goto bad;
    }
    for (tries = 1; ; tries++) {
	struct stat sb;

	fd = ts_openat(dfd, uidstr, O_RDWR|O_CREAT);
	switch (fd) {
	case TIMESTAMP_OPEN_ERROR:
	    log_warning(ctx, SLOG_SEND_MAIL, N_("unable to open %s"), fname);
	    goto bad;
	case TIMESTAMP_PERM_ERROR:
	    /* Already logged set_perms/restore_perms error. */
	    goto bad;
	}
	sudo_debug_printf(SUDO_DEBUG_INFO, "%s: opened time stamp file %s",
	    __func__, fname);

	/* Remove time stamp file if its mtime predates boot time. */
	if (tries == 1 && fstat(fd, &sb) == 0) {
	    struct timespec boottime, mtime, now;

	    if (sudo_gettime_real(&now) == 0 && get_boottime(&boottime)) {
		/* Ignore a boot time that is in the future. */
		if (sudo_timespeccmp(&now, &boottime, <)) {
		    sudo_debug_printf(SUDO_DEBUG_WARN|SUDO_DEBUG_LINENO,
			"ignoring boot time that is in the future");
		} else {
		    mtim_get(&sb, mtime);
		    if (sudo_timespeccmp(&mtime, &boottime, <)) {
			/* Time stamp file too old, remove it. */
			sudo_debug_printf(SUDO_DEBUG_WARN|SUDO_DEBUG_LINENO,
			    "removing time stamp file that predates boot time");
			close(fd);
			unlinkat(dfd, uidstr, 0);
			continue;
		    }
		}
	    }
	}
	break;
    }

    /* Allocate and fill in cookie to store state. */
    cookie = malloc(sizeof(*cookie));
    if (cookie == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	goto bad;
    }
    cookie->ctx = ctx;
    cookie->fd = fd;
    cookie->fname = fname;
    cookie->pos = -1;

    close(dfd);
    debug_return_ptr(cookie);
bad:
    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
	"%s: unable to open time stamp file %s", __func__, fname);
    if (dfd != -1)
	close(dfd);
    if (fd >= 0)
	close(fd);
    free(fname);
    debug_return_ptr(NULL);
}

static volatile sig_atomic_t got_signal;

static void
timestamp_handler(int s)
{
    got_signal = s;
}

/*
 * Wrapper for sudo_lock_region() that is interruptible.
 */
static bool
timestamp_lock_record(int fd, off_t pos, off_t len)
{
    struct sigaction sa, saveint, savequit;
    sigset_t mask, omask;
    bool ret;
    debug_decl(timestamp_lock_record, SUDOERS_DEBUG_AUTH);

    if (pos >= 0 && lseek(fd, pos, SEEK_SET) == -1) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_ERRNO|SUDO_DEBUG_LINENO,
	    "unable to seek to %lld", (long long)pos);
	debug_return_bool(false);
    }

    /* Allow SIGINT and SIGQUIT to interrupt a lock. */
    got_signal = 0;
    memset(&sa, 0, sizeof(sa));
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0; /* don't restart system calls */
    sa.sa_handler = timestamp_handler;
    (void) sigaction(SIGINT, &sa, &saveint);
    (void) sigaction(SIGQUIT, &sa, &savequit);
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGQUIT);
    (void) sigprocmask(SIG_UNBLOCK, &mask, &omask);

    ret = sudo_lock_region(fd, SUDO_LOCK, len);
    if (!ret) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_ERRNO|SUDO_DEBUG_LINENO,
	    "failed to lock fd %d [%lld, %lld]", fd,
	    (long long)pos, (long long)len);
    }

    /* Restore the old mask (SIGINT and SIGQUIT blocked) and handlers. */
    (void) sigprocmask(SIG_SETMASK, &omask, NULL);
    (void) sigaction(SIGINT, &saveint, NULL);
    (void) sigaction(SIGQUIT, &savequit, NULL);

    /* Re-deliver the signal that interrupted the lock, if any. */
    if (!ret && got_signal)
	kill(getpid(), got_signal);

    debug_return_bool(ret);
}

static bool
timestamp_unlock_record(int fd, off_t pos, off_t len)
{
    debug_decl(timestamp_unlock_record, SUDOERS_DEBUG_AUTH);

    if (pos >= 0 && lseek(fd, pos, SEEK_SET) == -1) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_ERRNO|SUDO_DEBUG_LINENO,
	    "unable to seek to %lld", (long long)pos);
	debug_return_bool(false);
    }
    debug_return_bool(sudo_lock_region(fd, SUDO_UNLOCK, len));
}

/*
 * Seek to the record's position and read it, locking as needed.
 */
static ssize_t
ts_read(struct ts_cookie *cookie, struct timestamp_entry *entry)
{
    ssize_t nread = -1;
    bool should_unlock = false;
    debug_decl(ts_read, SUDOERS_DEBUG_AUTH);

    /* If the record is not already locked, lock it now.  */
    if (!cookie->locked) {
	if (!timestamp_lock_record(cookie->fd, cookie->pos, sizeof(*entry)))
	    goto done;
	should_unlock = true;
    }

    /* Seek to the record position and read it.  */
    nread = pread(cookie->fd, entry, sizeof(*entry), cookie->pos);
    if (nread != sizeof(*entry)) {
	/* short read, should not happen */
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
	    "short read (%zd vs %zu), truncated time stamp file?",
	    nread, sizeof(*entry));
	goto done;
    }
    sudo_debug_printf(SUDO_DEBUG_DEBUG|SUDO_DEBUG_LINENO,
	"read %zd byte record at %lld", nread, (long long)cookie->pos);

done:
    /* If the record was not locked initially, unlock it. */
    if (should_unlock)
	timestamp_unlock_record(cookie->fd, cookie->pos, sizeof(*entry));

    debug_return_ssize_t(nread);
}

/*
 * Write a TS_LOCKEXCL record at the beginning of the time stamp file.
 */
static bool
timestamp_lock_write(struct ts_cookie *cookie)
{
    struct timestamp_entry entry;
    bool ret = true;
    debug_decl(timestamp_lock_write, SUDOERS_DEBUG_AUTH);

    memset(&entry, 0, sizeof(entry));
    entry.version = TS_VERSION;
    entry.size = sizeof(entry);
    entry.type = TS_LOCKEXCL;
    if (ts_write(cookie->ctx, cookie->fd, cookie->fname, &entry, -1) == -1)
	ret = false;
    debug_return_bool(ret);
}

/*
 * Lock a record in the time stamp file for exclusive access.
 * If the record does not exist, it is created (as disabled).
 */
bool
timestamp_lock(void *vcookie, struct passwd *pw)
{
    struct ts_cookie *cookie = vcookie;
    struct timestamp_entry entry;
    bool overwrite = false;
    off_t lock_pos;
    ssize_t nread;
    debug_decl(timestamp_lock, SUDOERS_DEBUG_AUTH);

    if (cookie == NULL) {
	sudo_debug_printf(SUDO_DEBUG_DEBUG|SUDO_DEBUG_LINENO,
	    "called with a NULL cookie!");
	debug_return_bool(false);
    }

    /*
     * Take a lock on the "write" record (the first record in the file).
     * This will let us seek for the record or extend as needed
     * without colliding with anyone else.
     */
    if (!timestamp_lock_record(cookie->fd, 0, sizeof(struct timestamp_entry)))
	debug_return_bool(false);

    /* Make sure the first record is of type TS_LOCKEXCL. */
    memset(&entry, 0, sizeof(entry));
    nread = read(cookie->fd, &entry, sizeof(entry));
    if (nread < ssizeof(struct timestamp_entry_v1)) {
	/* New or invalid time stamp file. */
	overwrite = true;
    } else if (entry.type != TS_LOCKEXCL) {
	if (entry.size == sizeof(struct timestamp_entry_v1)) {
	    /* Old sudo record, convert it to TS_LOCKEXCL. */
	    entry.type = TS_LOCKEXCL;
	    memset((char *)&entry + offsetof(struct timestamp_entry, flags), 0,
		(size_t)nread - offsetof(struct timestamp_entry, flags));
	    if (ts_write(cookie->ctx, cookie->fd, cookie->fname, &entry, 0) == -1)
		debug_return_bool(false);
	} else {
	    /* Corrupted time stamp file?  Just overwrite it. */
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_ERRNO|SUDO_DEBUG_LINENO,
		"corrupt initial record, type: %hu, size: %hu (expected %zu)",
		entry.type, entry.size, sizeof(struct timestamp_entry_v1));
	    overwrite = true;
	}
    }
    if (overwrite) {
	/* Rewrite existing time stamp file or create new one. */
	if (ftruncate(cookie->fd, 0) != 0) {
	    sudo_warn(U_("unable to truncate time stamp file to %lld bytes"),
		0LL);
	    debug_return_bool(false);
	}
	if (!timestamp_lock_write(cookie))
	    debug_return_bool(false);
    } else if (entry.size != sizeof(entry)) {
	/* Reset position if the lock record has an unexpected size. */
	if (lseek(cookie->fd, entry.size, SEEK_SET) == -1) {
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_ERRNO|SUDO_DEBUG_LINENO,
		"unable to seek to %hu", entry.size);
	    debug_return_bool(false);
	}
    }

    /* Search for a tty/ppid-based record or append a new one. */
    sudo_debug_printf(SUDO_DEBUG_DEBUG|SUDO_DEBUG_LINENO,
	"searching for %s time stamp record",
	def_timestamp_type == ppid ? "ppid" : "tty");
    ts_init_key_nonglobal(cookie->ctx, &cookie->key, pw, TS_DISABLED);
    if (ts_find_record(cookie->fd, &cookie->key, &entry)) {
	sudo_debug_printf(SUDO_DEBUG_DEBUG|SUDO_DEBUG_LINENO,
	    "found existing %s time stamp record",
	    def_timestamp_type == ppid ? "ppid" : "tty");
	lock_pos = lseek(cookie->fd, 0, SEEK_CUR) - (off_t)entry.size;
    } else {
	sudo_debug_printf(SUDO_DEBUG_DEBUG|SUDO_DEBUG_LINENO,
	    "appending new %s time stamp record",
	    def_timestamp_type == ppid ? "ppid" : "tty");
	lock_pos = lseek(cookie->fd, 0, SEEK_CUR);
	if (ts_write(cookie->ctx, cookie->fd, cookie->fname, &cookie->key, -1) == -1)
	    debug_return_bool(false);
    }
    sudo_debug_printf(SUDO_DEBUG_DEBUG|SUDO_DEBUG_LINENO,
	"%s time stamp position is %lld",
	def_timestamp_type == ppid ? "ppid" : "tty", (long long)lock_pos);

    if (def_timestamp_type == global) {
	/*
	 * For global tickets we use a separate record lock that we
	 * cannot hold long-term since it is shared between all ttys.
	 */
	cookie->locked = false;
	cookie->key.type = TS_GLOBAL;	/* find a global record */

	if (lseek(cookie->fd, 0, SEEK_SET) == -1) {
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_ERRNO|SUDO_DEBUG_LINENO,
		"unable to rewind fd");
	    debug_return_bool(false);
	}
	if (ts_find_record(cookie->fd, &cookie->key, &entry)) {
	    sudo_debug_printf(SUDO_DEBUG_DEBUG|SUDO_DEBUG_LINENO,
		"found existing global record");
	    cookie->pos = lseek(cookie->fd, 0, SEEK_CUR) - (off_t)entry.size;
	} else {
	    sudo_debug_printf(SUDO_DEBUG_DEBUG|SUDO_DEBUG_LINENO,
		"appending new global record");
	    cookie->pos = lseek(cookie->fd, 0, SEEK_CUR);
	    if (ts_write(cookie->ctx, cookie->fd, cookie->fname, &cookie->key, -1) == -1)
		debug_return_bool(false);
	}
    } else {
	/* For tty/ppid tickets the tty lock is the same as the record lock. */
	cookie->pos = lock_pos;
	cookie->locked = true;
    }

    /* Unlock the TS_LOCKEXCL record. */
    timestamp_unlock_record(cookie->fd, 0, sizeof(struct timestamp_entry));

    /* Lock the per-tty record (may sleep). */
    if (!timestamp_lock_record(cookie->fd, lock_pos, sizeof(struct timestamp_entry)))
	debug_return_bool(false);

    debug_return_bool(true);
}

void
timestamp_close(void *vcookie)
{
    struct ts_cookie *cookie = vcookie;
    debug_decl(timestamp_close, SUDOERS_DEBUG_AUTH);

    if (cookie != NULL) {
	close(cookie->fd);
	free(cookie->fname);
	free(cookie);
    }

    debug_return;
}

#define TIMESPEC_VALID(ts) \
    ((ts)->tv_sec >= 0 && (ts)->tv_nsec >= 0 && (ts)->tv_nsec < 1000000000L)

/*
 * Check the time stamp file and directory and return their status.
 * Called with the file position before the locked record to read.
 * Returns one of TS_CURRENT, TS_OLD, TS_MISSING, TS_ERROR, TS_FATAL.
 * Fills in fdp with an open file descriptor positioned at the
 * appropriate (and locked) record.
 */
int
timestamp_status(void *vcookie, struct passwd *pw)
{
    struct ts_cookie *cookie = vcookie;
    struct timestamp_entry entry;
    struct timespec diff, now;
    int status = TS_ERROR;		/* assume the worst */
    ssize_t nread;
    debug_decl(timestamp_status, SUDOERS_DEBUG_AUTH);

    /* Zero timeout means don't use time stamp files. */
    if (!sudo_timespecisset(&def_timestamp_timeout)) {
	sudo_debug_printf(SUDO_DEBUG_DEBUG|SUDO_DEBUG_LINENO,
	    "timestamps disabled");
	status = TS_OLD;
	goto done;
    }
    if (cookie == NULL || cookie->pos < 0) {
	sudo_debug_printf(SUDO_DEBUG_DEBUG|SUDO_DEBUG_LINENO,
	    "NULL cookie or invalid position");
	status = TS_OLD;
	goto done;
    }

#ifdef TIOCCHKVERAUTH
    if (def_timestamp_type == kernel) {
	int fd = open(_PATH_TTY, O_RDWR);
	if (fd != -1) {
	    if (ioctl(fd, TIOCCHKVERAUTH) == 0)
		status = TS_CURRENT;
	    else
		status = TS_OLD;
	    close(fd);
	    goto done;
	}
    }
#endif

    /* Read the record at the correct position. */
    if ((nread = ts_read(cookie, &entry)) != sizeof(entry))
	goto done;

    /* Make sure what we read matched the expected record. */
    if (entry.version != TS_VERSION || entry.size != nread) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
	    "invalid time stamp file @ %lld", (long long)cookie->pos);
	status = TS_OLD;
	goto done;
    }

    /* Sanity check time stamps. */
    if (!TIMESPEC_VALID(&entry.start_time) || !TIMESPEC_VALID(&entry.ts)) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
	    "invalid timespec in time stamp file @ %lld",
	    (long long)cookie->pos);
	status = TS_OLD;
	goto done;
    }

    if (ISSET(entry.flags, TS_DISABLED)) {
	sudo_debug_printf(SUDO_DEBUG_DEBUG|SUDO_DEBUG_LINENO,
	    "time stamp record disabled");
	status = TS_OLD;	/* disabled via sudo -k */
	goto done;
    }

    if (entry.type != TS_GLOBAL && entry.sid != cookie->ctx->user.sid) {
	sudo_debug_printf(SUDO_DEBUG_DEBUG|SUDO_DEBUG_LINENO,
	    "time stamp record sid mismatch");
	status = TS_OLD;	/* belongs to different session */
	goto done;
    }

    /* Negative timeouts only expire manually (sudo -k).  */
    sudo_timespecclear(&diff);
    if (sudo_timespeccmp(&def_timestamp_timeout, &diff, <)) {
	sudo_debug_printf(SUDO_DEBUG_DEBUG|SUDO_DEBUG_LINENO,
	    "time stamp record does not expire");
	status = TS_CURRENT;
	goto done;
    }

    /* Compare stored time stamp with current time. */
    if (sudo_gettime_mono(&now) == -1) {
	log_warning(cookie->ctx, 0, N_("unable to read the clock"));
	status = TS_ERROR;
	goto done;
    }
    sudo_timespecsub(&now, &entry.ts, &diff);
    if (sudo_timespeccmp(&diff, &def_timestamp_timeout, <)) {
	status = TS_CURRENT;
#if defined(CLOCK_MONOTONIC) || defined(__MACH__)
	/* A monotonic clock should never run backwards. */
	if (diff.tv_sec < 0) {
	    log_warningx(cookie->ctx, SLOG_SEND_MAIL,
		N_("ignoring time stamp from the future"));
	    status = TS_OLD;
	    SET(entry.flags, TS_DISABLED);
	    (void)ts_write(cookie->ctx, cookie->fd, cookie->fname, &entry, cookie->pos);
	}
#else
	/*
	 * Check for bogus (future) time in the stampfile.
	 * If diff / 2 > timeout, someone has been fooling with the clock.
	 */
	sudo_timespecsub(&entry.ts, &now, &diff);
	diff.tv_nsec /= 2;
	if (diff.tv_sec & 1)
	    diff.tv_nsec += 500000000;
	diff.tv_sec /= 2;
	while (diff.tv_nsec >= 1000000000) {
	    diff.tv_sec++;
	    diff.tv_nsec -= 1000000000;
	}

	if (sudo_timespeccmp(&diff, &def_timestamp_timeout, >)) {
	    time_t tv_sec = (time_t)entry.ts.tv_sec;
	    log_warningx(cookie->ctx, SLOG_SEND_MAIL,
		N_("time stamp too far in the future: %20.20s"),
		4 + ctime(&tv_sec));
	    status = TS_OLD;
	    SET(entry.flags, TS_DISABLED);
	    (void)ts_write(cookie->ctx, cookie->fd, cookie->fname, &entry, cookie->pos);
	}
#endif /* CLOCK_MONOTONIC */
    } else {
	status = TS_OLD;
    }

done:
    debug_return_int(status);
}

/*
 * Update the time on the time stamp file/dir or create it if necessary.
 * Returns true on success, false on failure or -1 on setuid failure.
 */
bool
timestamp_update(void *vcookie, struct passwd *pw)
{
    struct ts_cookie *cookie = vcookie;
    bool ret = false;
    debug_decl(timestamp_update, SUDOERS_DEBUG_AUTH);

    /* Zero timeout means don't use time stamp files. */
    if (!sudo_timespecisset(&def_timestamp_timeout)) {
	sudo_debug_printf(SUDO_DEBUG_DEBUG|SUDO_DEBUG_LINENO,
	    "timestamps disabled");
	goto done;
    }
    if (cookie == NULL || cookie->pos < 0) {
	sudo_debug_printf(SUDO_DEBUG_DEBUG|SUDO_DEBUG_LINENO,
	    "NULL cookie or invalid position");
	goto done;
    }

#ifdef TIOCSETVERAUTH
    if (def_timestamp_type == kernel) {
	int fd = open(_PATH_TTY, O_RDWR);
	if (fd != -1) {
	    int secs = (int)def_timestamp_timeout.tv_sec;
	    if (secs > 0) {
		if (secs > 3600)
		    secs = 3600;	/* OpenBSD limitation */
		if (ioctl(fd, TIOCSETVERAUTH, &secs) != 0)
		    sudo_warn("TIOCSETVERAUTH");
	    }
	    close(fd);
	    goto done;
	}
    }
#endif

    /* Update timestamp in key and enable it. */
    CLR(cookie->key.flags, TS_DISABLED);
    if (sudo_gettime_mono(&cookie->key.ts) == -1) {
	log_warning(cookie->ctx, 0, N_("unable to read the clock"));
	goto done;
    }

    /* Write out the locked record. */
    sudo_debug_printf(SUDO_DEBUG_DEBUG|SUDO_DEBUG_LINENO,
	"writing %zu byte record at %lld", sizeof(cookie->key),
	(long long)cookie->pos);
    if (ts_write(cookie->ctx, cookie->fd, cookie->fname, &cookie->key, cookie->pos) != -1)
	ret = true;

done:
    debug_return_bool(ret);
}

/*
 * Remove the timestamp entry or file if unlink_it is set.
 * Returns true on success, false on failure or -1 on setuid failure.
 * A missing timestamp entry is not considered an error.
 */
int
timestamp_remove(const struct sudoers_context *ctx, bool unlink_it)
{
    struct timestamp_entry key, entry;
    int len, dfd = -1, fd = -1, ret = true;
    char uidstr[STRLEN_MAX_UNSIGNED(uid_t) + 1];
    char *fname = NULL;
    debug_decl(timestamp_remove, SUDOERS_DEBUG_AUTH);

#ifdef TIOCCLRVERAUTH
    if (def_timestamp_type == kernel) {
	fd = open(_PATH_TTY, O_RDWR);
	if (fd != -1) {
	    ioctl(fd, TIOCCLRVERAUTH);
	    goto done;
	}
    }
#endif

    dfd = open(def_timestampdir, O_RDONLY|O_NONBLOCK);
    if (dfd == -1) {
	if (errno != ENOENT)
	    ret = -1;
	goto done;
    }

    len = snprintf(uidstr, sizeof(uidstr), "%u", (unsigned int)ctx->user.uid);
    if (len < 0 || len >= ssizeof(uidstr)) {
	errno = EINVAL;
	ret = -1;
	goto done;
    }
    if (asprintf(&fname, "%s/%s", def_timestampdir, uidstr) == -1) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	ret = -1;
	goto done;
    }

    /* For "sudo -K" simply unlink the time stamp file. */
    if (unlink_it) {
	ret = unlinkat(dfd, uidstr, 0) ? -1 : true;
	goto done;
    }

    /* Open time stamp file and lock it for exclusive access. */
    fd = ts_openat(dfd, uidstr, O_RDWR);
    switch (fd) {
    case TIMESTAMP_OPEN_ERROR:
	if (errno != ENOENT)
	    ret = false;
	goto done;
    case TIMESTAMP_PERM_ERROR:
	/* Already logged set_perms/restore_perms error. */
	ret = -1;
	goto done;
    }
    /* Lock first record to gain exclusive access. */
    if (!timestamp_lock_record(fd, -1, sizeof(struct timestamp_entry))) {
	sudo_warn(U_("unable to lock time stamp file %s"), fname);
	ret = -1;
	goto done;
    }

    /*
     * Find matching entries and invalidate them.
     */
    ts_init_key(ctx, &key, NULL, 0, def_timestamp_type);
    while (ts_find_record(fd, &key, &entry)) {
	/* Back up and disable the entry. */
	if (!ISSET(entry.flags, TS_DISABLED)) {
	    SET(entry.flags, TS_DISABLED);
	    if (lseek(fd, 0 - (off_t)sizeof(entry), SEEK_CUR) != -1) {
		if (ts_write(ctx, fd, fname, &entry, -1) == -1)
		    ret = false;
	    }
	}
    }

done:
    if (dfd != -1)
	close(dfd);
    if (fd >= 0)
	close(fd);
    free(fname);
    debug_return_int(ret);
}

bool
cb_timestampowner(struct sudoers_context *ctx, const char *file,
    int line, int column, const union sudo_defs_val *sd_un, int op)
{
    struct passwd *pw = NULL;
    const char *user = sd_un->str;
    debug_decl(cb_timestampowner, SUDOERS_DEBUG_AUTH);

    if (*user == '#') {
	const char *errstr;
	uid_t uid = sudo_strtoid(user + 1, &errstr);
	if (errstr == NULL)
	    pw = sudo_getpwuid(uid);
    }
    if (pw == NULL)
	pw = sudo_getpwnam(user);
    if (pw == NULL) {
	log_warningx(ctx, SLOG_AUDIT|SLOG_PARSE_ERROR,
	    N_("%s:%d:%d timestampowner: unknown user %s"), file, line,
	    column, user);
	debug_return_bool(false);
    }
    timestamp_uid = pw->pw_uid;
    timestamp_gid = pw->pw_gid;
    sudo_pw_delref(pw);

    debug_return_bool(true);
}

/*
 * Returns true if the user has already been lectured.
 */
bool
already_lectured(const struct sudoers_context *ctx)
{
    char uidstr[STRLEN_MAX_UNSIGNED(uid_t) + 1];
    bool ret = false;
    struct stat sb;
    int dfd, len;
    debug_decl(already_lectured, SUDOERS_DEBUG_AUTH);

    /* Check the existence and validity of timestamp dir. */
    dfd = ts_secure_opendir(def_lecture_status_dir, false, true);
    if (dfd == -1)
	goto done;

    len = snprintf(uidstr, sizeof(uidstr), "%u", (unsigned int)ctx->user.uid);
    if (len < 0 || len >= ssizeof(uidstr))
	goto done;

    ret = fstatat(dfd, uidstr, &sb, AT_SYMLINK_NOFOLLOW) == 0;
    if (!ret && errno == ENOENT && strchr(ctx->user.name, '/') == NULL) {
	/* No uid-based lecture path, check for username-based path. */
	ret = fstatat(dfd, ctx->user.name, &sb, AT_SYMLINK_NOFOLLOW) == 0;
	if (ret) {
	    /* Migrate lecture file to uid-based path. */
#ifdef HAVE_RENAMEAT
	    if (renameat(dfd, ctx->user.name, dfd, uidstr) == -1) {
		sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_ERRNO,
		    "%s: unable to rename %s/%s to %s/%s", __func__,
		    def_lecture_status_dir, ctx->user.name,
		    def_lecture_status_dir, uidstr);
	    }
#else
	    char from[PATH_MAX], to[PATH_MAX];
	    len = snprintf(from, sizeof(from), "%s/%s", def_lecture_status_dir,
		ctx->user.name);
	    if (len < 0 || len >= ssizeof(from))
		goto done;
	    len = snprintf(to, sizeof(to), "%s/%s", def_lecture_status_dir,
		uidstr);
	    if (len < 0 || len >= ssizeof(to))
		goto done;
	    if (rename(from, to) == -1) {
		sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_ERRNO,
		    "%s: unable to rename %s to %s", __func__, from, to);
	    }
#endif
	}
    }

done:
    if (dfd != -1)
	close(dfd);
    debug_return_bool(ret);
}

/*
 * Create the lecture status file.
 * Returns true on success, false on failure or -1 on setuid failure.
 */
int
set_lectured(const struct sudoers_context *ctx)
{
    char uidstr[STRLEN_MAX_UNSIGNED(uid_t) + 1];
    int dfd, fd, len, ret = false;
    debug_decl(set_lectured, SUDOERS_DEBUG_AUTH);

    /* Check the validity of timestamp dir and create if missing. */
    dfd = ts_secure_opendir(def_lecture_status_dir, true, false);
    if (dfd == -1)
	goto done;

    len = snprintf(uidstr, sizeof(uidstr), "%u", (unsigned int)ctx->user.uid);
    if (len < 0 || len >= ssizeof(uidstr))
	goto done;

    /* Create lecture file. */
    fd = ts_openat(dfd, uidstr, O_WRONLY|O_CREAT|O_EXCL);
    switch (fd) {
    case TIMESTAMP_OPEN_ERROR:
	/* Failed to open, not a fatal error. */
	break;
    case TIMESTAMP_PERM_ERROR:
	/* Already logged set_perms/restore_perms error. */
	ret = -1;
	break;
    default:
	/* Success. */
	close(fd);
	ret = true;
	break;
    }

done:
    if (dfd != -1)
	close(dfd);
    debug_return_int(ret);
}

#ifdef _PATH_SUDO_ADMIN_FLAG
int
create_admin_success_flag(const struct sudoers_context *ctx)
{
    struct passwd *pw = ctx->user.pw;
    char *flagfile;
    int ret = -1;
    debug_decl(create_admin_success_flag, SUDOERS_DEBUG_AUTH);

    /* Is the admin flag file even enabled? */
    if (!def_admin_flag)
	debug_return_int(true);

    /* Check whether the user is in the sudo or admin group. */
    if (!user_in_group(pw, "sudo") && !user_in_group(pw, "admin"))
	debug_return_int(true);

    /* Build path to flag file. */
    if ((flagfile = strdup(def_admin_flag)) == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	debug_return_int(-1);
    }
    if (!expand_tilde(&flagfile, pw->pw_name)) {
	free(flagfile);
	debug_return_int(false);
    }

    /* Create admin flag file if it doesn't already exist. */
    if (set_perms(ctx, PERM_USER)) {
	int fd = open(flagfile, O_CREAT|O_WRONLY|O_NONBLOCK|O_EXCL, 0644);
	ret = fd != -1 || errno == EEXIST;
	if (fd != -1)
	    close(fd);
	if (!restore_perms())
	    ret = -1;
    }
    free(flagfile);
    debug_return_int(ret);
}
#else /* !_PATH_SUDO_ADMIN_FLAG */
int
create_admin_success_flag(const struct sudoers_context *ctx)
{
    /* STUB */
    return true;
}
#endif /* _PATH_SUDO_ADMIN_FLAG */
