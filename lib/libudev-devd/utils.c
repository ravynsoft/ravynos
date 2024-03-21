/*
 * Copyright (c) 2015, 2021 Vladimir Kondratyev <vladimir@kondratyev.su>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "config.h"

#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#ifdef HAVE_LIBPROCSTAT_H
#include <sys/sysctl.h>
#include <sys/queue.h>
#include <sys/socket.h>
#include <kvm.h>
#include <libprocstat.h>
#endif

#ifdef HAVE_DEVINFO_H
#include <devinfo.h>
#include <pthread.h>
static pthread_mutex_t devinfo_mtx = PTHREAD_MUTEX_INITIALIZER;
#endif

#ifndef HAVE_PIPE2
#include <fcntl.h>
#endif

#include "utils.h"

/*
 * locates the occurrence of last component of the pathname
 * pointed to by path
 */
char *
strbase(const char *path)
{
	char *base;

	base = strrchr(path, '/');
	if (base != NULL)
		base++;

	return (base);
}

char *
get_kern_prop_value(const char *buf, const char *prop, size_t *len)
{
	char *prop_pos;
	size_t prop_len;

	prop_len = strlen(prop);
	prop_pos = strstr(buf, prop);
	if (prop_pos == NULL ||
	    (prop_pos != buf && prop_pos[-1] != ' ') ||
	    prop_pos[prop_len] != '=')
		return (NULL);

	*len = strchrnul(prop_pos + prop_len + 1, ' ') - prop_pos - prop_len - 1;
	return (prop_pos + prop_len + 1);
}

int
match_kern_prop_value(const char *buf, const char *prop,
    const char *match_value)
{
	const char *value;
	size_t len;

	value = get_kern_prop_value(buf, prop, &len);
	if (value != NULL &&
	    len == strlen(match_value) &&
	    strncmp(value, match_value, len) == 0)
		return (1);

	return (0);
}

int
path_to_fd(const char *path)
{
	int fd = -1;

#ifdef HAVE_LIBPROCSTAT_H
	struct procstat *procstat;
	struct kinfo_proc *kip;
	struct filestat_list *head = NULL;
	struct filestat *fst;
	unsigned int count;
#else
	struct stat st, fst;
#define	MAX_FD	128
#endif

#ifdef HAVE_LIBPROCSTAT_H
	procstat = procstat_open_sysctl();
	if (procstat == NULL)
		return (-1);

	count = 0;
	kip = procstat_getprocs(procstat, KERN_PROC_PID, getpid(), &count);
	if (kip == NULL || count != 1)
		goto out;

	head = procstat_getfiles(procstat, kip, 0);
	if (head == NULL)
		goto out;

	STAILQ_FOREACH(fst, head, next) {
		if (fst->fs_uflags == 0 &&
		    fst->fs_type == PS_FST_TYPE_VNODE &&
		    fst->fs_path != NULL &&
		    strcmp(fst->fs_path, path) == 0) {
			fd = fst->fs_fd;
			break;
		}
	}

out:
	if (head != NULL)
		procstat_freefiles(procstat, head);
	if (kip != NULL)
		procstat_freeprocs(procstat, kip);
	procstat_close(procstat);
#else
	if (stat(path, &st) != 0)
		return (-1);

	for (fd = 0; fd < MAX_FD; ++fd) {

		if (fstat(fd, &fst) != 0) {
			if (errno != EBADF) {
				return -1;
			} else {
				continue;
			}
		}

		if (fst.st_rdev == st.st_rdev)
			break;
	}

	if (fd == MAX_FD)
		return (-1);
#endif

	return (fd);
}

static int
scandir_sub(char *path, int off, int rem, struct scan_ctx *ctx)
{
	DIR *dir;
	struct dirent *ent;
	int ret = 0;

	dir = opendir(path);
	if (dir == NULL)
		return (errno == ENOMEM ? -1 : 0);

	while (ret >= 0 && (ent = readdir(dir)) != NULL) {
		if (strcmp(ent->d_name, ".") == 0 ||
		    strcmp(ent->d_name, "..") == 0)
			continue;

		int len = strlen(ent->d_name);
		if (len > rem)
			continue;

		strcpy(path + off, ent->d_name);
		off += len;
		rem -= len;

		if ((ctx->recursive) && (ent->d_type == DT_DIR)) {
			if (rem < 1)
				break;
			path[off] = '/';
			path[off+1] = '\0';
			off++;
			rem--;
			/* recurse */
			ret = scandir_sub(path, off, rem, ctx);
			off--;
			rem++;
		} else {
			ret = (ctx->cb)(path, DTTOIF(ent->d_type), ctx->args);
		}
		off -= len;
		rem += len;
	}
	closedir(dir);
	return (ret);
}

int
scandir_recursive(char *path, size_t len, struct scan_ctx *ctx)
{
	size_t root_len = strlen(path);

	return (scandir_sub(path, root_len, len - root_len - 1, ctx));
}

#ifdef HAVE_DEVINFO_H
static int
scandev_sub(struct devinfo_dev *dev, void *args)
{
	struct scan_ctx *ctx = args;

	if (dev->dd_name[0] != '\0' && dev->dd_state >= DS_ATTACHED)
		if ((ctx->cb)(dev->dd_name, DT_CHR, ctx->args) < 0)
			return (-1);

	/* recurse */
        return (devinfo_foreach_device_child(dev, scandev_sub, args));
}


int
scandev_recursive (struct scan_ctx *ctx)
{
	struct devinfo_dev *root;
	int ret;

	pthread_mutex_lock(&devinfo_mtx);
	if (devinfo_init()) {
		pthread_mutex_unlock(&devinfo_mtx);
		ERR("devinfo_init failed");
		return (-1);
	}

	if ((root = devinfo_handle_to_device(DEVINFO_ROOT_DEVICE)) == NULL) {
		ERR("faled to init devinfo root device");
		ret = -1;
	} else {
		ret = devinfo_foreach_device_child(root, scandev_sub, ctx);
		if (ret < 0)
			ERR("devinfo_foreach_device_child failed");
	}

	devinfo_free();
	pthread_mutex_unlock(&devinfo_mtx);
	return (ret);
}
#endif /* HAVE_DEVINFO_H */

#ifndef HAVE_DEVNAME_R
struct devname_scan_args {
	dev_t dev;
	mode_t type;
	char *buf;
	int len;
};

static int
devname_cb(const char *path, mode_t type, void *args)
{
	struct devname_scan_args *sa = args;
	struct stat st;

	if (sa->type == type &&
	    lstat(path, &st) == 0 &&
	    st.ST_RDEV == sa->dev) {
		strlcpy(sa->buf, path + 5, sa->len);
		return (-1);
        }
        return (0);
}

char *
devname_r(dev_t dev, mode_t type, char *buf, int len)
{
	char path[PATH_MAX] = "/dev/";
	struct devname_scan_args args = {
		.dev = dev,
		.type = type,
		.buf = buf,
		.len = len,
	};
	struct scan_ctx ctx = {
		.recursive = true,
		.cb = devname_cb,
		.args = &args,
	};

	if (dev == NODEV || !(S_ISCHR(type) || S_ISBLK(dev))) {
		strlcpy(buf, "#NODEV", len);
		return (buf);
	}
	if (scandir_recursive(path, sizeof(path), &ctx) == 0)
		/* Finally just format it */
		snprintf(buf, len, "#%c:%#jx",
		    S_ISCHR(type) ? 'C' : 'B', (uintmax_t)dev);

        return (buf);
}
#endif /* !HAVE_DEVNAME_R */

#ifndef HAVE_PIPE2
int
pipe2(int fildes[2], int flags)
{
	int ret;

	ret = pipe(fildes);
	if (ret != -1) {
		if (flags & O_CLOEXEC) {
			fcntl(fildes[0], F_SETFD, FD_CLOEXEC);
			fcntl(fildes[1], F_SETFD, FD_CLOEXEC);
		}
		if (flags & O_NONBLOCK) {
			fcntl(fildes[0], F_SETFL, O_NONBLOCK);
			fcntl(fildes[1], F_SETFL, O_NONBLOCK);
		}
	}

	return (ret);
}
#endif /* !HAVE_PIPE2 */

#ifndef HAVE_STRCHRNUL
char *
strchrnul(const char *p, int ch)
{
	char c;

	c = ch;
	for (;; ++p) {
		if (*p == c || *p == '\0')
			return ((char *)p);
	}
	/* NOTREACHED */
}
#endif /* !HAVE_STRCHRNUL */

#ifndef HAVE_STRLCPY
size_t strlcpy(char *dst, const char *src, size_t dsize)
{
	const char *osrc = src;
	size_t nleft = dsize;

	/* Copy as many bytes as will fit. */
	if (nleft != 0) {
		while (--nleft != 0) {
			if ((*dst++ = *src++) == '\0')
				break;
		}
	}

	/* Not enough room in dst, add NUL and traverse rest of src. */
	if (nleft == 0) {
		if (dsize != 0)
			*dst = '\0';		/* NUL-terminate dst */
		while (*src++)
			;
	}

	return(src - osrc - 1);	/* count does not include NUL */
}
#endif /* !HAVE_STRLCPY */
