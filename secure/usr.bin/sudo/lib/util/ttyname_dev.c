/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2012-2018 Todd C. Miller <Todd.Miller@sudo.ws>
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
#if defined(MAJOR_IN_MKDEV)
# include <sys/mkdev.h>
#elif defined(MAJOR_IN_SYSMACROS)
# include <sys/sysmacros.h>
#else
# include <sys/param.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <dirent.h>

#include <pathnames.h>
#include <sudo_compat.h>
#include <sudo_debug.h>
#include <sudo_conf.h>
#include <sudo_util.h>

#if defined(HAVE_DEVNAME)
/*
 * Like ttyname() but uses a dev_t instead of an open fd.
 * Returns name on success and NULL on failure, setting errno.
 * The BSD version uses devname().
 */
char *
sudo_ttyname_dev_v1(dev_t tdev, char *name, size_t namelen)
{
    char *dev;
    debug_decl(sudo_ttyname_dev, SUDO_DEBUG_UTIL);

    /* Some versions of devname() return NULL on failure, others do not. */
    dev = devname(tdev, S_IFCHR);
    if (dev != NULL && *dev != '?' && *dev != '#') {
	if (strlcpy(name, _PATH_DEV, namelen) < namelen &&
	    strlcat(name, dev, namelen) < namelen)
	    debug_return_str(name);
	errno = ERANGE;
    } else {
	/* Not all versions of devname() set errno. */
	errno = ENOENT;
    }
    debug_return_str(NULL);
}
#elif defined(HAVE__TTYNAME_DEV)
extern char *_ttyname_dev(dev_t rdev, char *buffer, size_t buflen);

/*
 * Like ttyname() but uses a dev_t instead of an open fd.
 * Returns name on success and NULL on failure, setting errno.
 * This version is just a wrapper around _ttyname_dev().
 */
char *
sudo_ttyname_dev_v1(dev_t tdev, char *name, size_t namelen)
{
    int serrno = errno;
    debug_decl(sudo_ttyname_dev, SUDO_DEBUG_UTIL);

    /*
     * _ttyname_dev() sets errno to ERANGE if namelen is too small
     * but does not modify it if tdev is not found.
     */
    errno = ENOENT;
    if (_ttyname_dev(tdev, name, namelen) == NULL)
	debug_return_str(NULL);
    errno = serrno;

    debug_return_str(name);
}
#else
/*
 * Device nodes to ignore.
 */
static const char *ignore_devs[] = {
    _PATH_DEV "stdin",
    _PATH_DEV "stdout",
    _PATH_DEV "stderr",
    NULL
};

/*
 * Do a scan of a directory looking for the specified device.
 * Does not descend into subdirectories.
 * Returns name on success and NULL on failure, setting errno.
 */
static char *
sudo_ttyname_scan(const char *dir, dev_t rdev, char *name, size_t namelen)
{
    size_t sdlen;
    char pathbuf[PATH_MAX];
    char *ret = NULL;
    struct dirent *dp;
    struct stat sb;
    size_t i;
    DIR *d = NULL;
    debug_decl(sudo_ttyname_scan, SUDO_DEBUG_UTIL);

    if (dir[0] == '\0') {
	errno = ENOENT;
	goto done;
    }
    if ((d = opendir(dir)) == NULL)
	goto done;

    if (fstat(dirfd(d), &sb) == -1) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
	    "unable to fstat %s", dir);
	goto done;
    }
    if ((sb.st_mode & S_IWOTH) != 0) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
	    "ignoring world-writable directory %s", dir);
	errno = ENOENT;
	goto done;
    }

    sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
	"scanning for dev %u in %s", (unsigned int)rdev, dir);

    sdlen = strlen(dir);
    while (sdlen > 0 && dir[sdlen - 1] == '/')
	sdlen--;
    if (sdlen + 1 >= sizeof(pathbuf)) {
	errno = ERANGE;
	goto done;
    }
    memcpy(pathbuf, dir, sdlen);
    pathbuf[sdlen++] = '/';

    while ((dp = readdir(d)) != NULL) {
	/* Skip anything starting with "." */
	if (dp->d_name[0] == '.')
	    continue;

	pathbuf[sdlen] = '\0';
	if (strlcat(pathbuf, dp->d_name, sizeof(pathbuf)) >= sizeof(pathbuf)) {
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		"%s%s is too big to fit in pathbuf", pathbuf, dp->d_name);
	    continue;
	}

	/* Ignore device nodes listed in ignore_devs[]. */
	for (i = 0; ignore_devs[i] != NULL; i++) {
	    if (strcmp(pathbuf, ignore_devs[i]) == 0)
		break;
	}
	if (ignore_devs[i] != NULL) {
	    sudo_debug_printf(SUDO_DEBUG_DEBUG|SUDO_DEBUG_LINENO,
		"ignoring %s", pathbuf);
	    continue;
	}

# if defined(HAVE_STRUCT_DIRENT_D_TYPE)
	/*
	 * Avoid excessive stat() calls by checking dp->d_type.
	 */
	switch (dp->d_type) {
	    case DT_CHR:
	    case DT_LNK:
	    case DT_UNKNOWN:
		break;
	    default:
		/* Not a character device or link, skip it. */
		sudo_debug_printf(SUDO_DEBUG_DEBUG|SUDO_DEBUG_LINENO,
		    "skipping non-device %s", pathbuf);
		continue;
	}
# endif
	if (stat(pathbuf, &sb) == -1) {
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO|SUDO_DEBUG_ERRNO,
		"unable to stat %s", pathbuf);
	    continue;
	}
	if (S_ISCHR(sb.st_mode) && sb.st_rdev == rdev) {
	    sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
		"resolved dev %u as %s", (unsigned int)rdev, pathbuf);
	    if (strlcpy(name, pathbuf, namelen) < namelen) {
		ret = name;
	    } else {
		sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		    "unable to store %s, have %zu, need %zu",
		    pathbuf, namelen, strlen(pathbuf) + 1);
		errno = ERANGE;
	    }
	    goto done;
	}
    }

done:
    if (d != NULL)
	closedir(d);
    debug_return_str(ret);
}

static char *
sudo_dev_check(dev_t rdev, const char * restrict devname, char * restrict buf, size_t buflen)
{
    struct stat sb;
    debug_decl(sudo_dev_check, SUDO_DEBUG_UTIL);

    if (stat(devname, &sb) == 0) {
	if (S_ISCHR(sb.st_mode) && sb.st_rdev == rdev) {
	    sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
		"comparing dev %u to %s: match!",
		(unsigned int)rdev, devname);
	    if (strlcpy(buf, devname, buflen) < buflen)
		debug_return_str(buf);
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		"unable to store %s, have %zu, need %zu",
		devname, buflen, strlen(devname) + 1);
	    errno = ERANGE;
	}
    }
    sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
	"comparing dev %u to %s: no", (unsigned int)rdev, devname);
    debug_return_str(NULL);
}

/*
 * Like ttyname() but uses a dev_t instead of an open fd.
 * Returns name on success and NULL on failure, setting errno.
 * Generic version.
 */
char *
sudo_ttyname_dev_v1(dev_t rdev, char *buf, size_t buflen)
{
    const char *devsearch, *devsearch_end;
    char path[PATH_MAX], *ret;
    const char *cp, *ep;
    size_t len;
    debug_decl(sudo_ttyname_dev, SUDO_DEBUG_UTIL);

    /*
     * First, check /dev/console.
     */
    ret = sudo_dev_check(rdev, _PATH_DEV "console", buf, buflen);
    if (ret != NULL)
	goto done;

    /*
     * Then check the device search path.
     */
    devsearch = sudo_conf_devsearch_path();
    devsearch_end = devsearch + strlen(devsearch);
    for (cp = sudo_strsplit(devsearch, devsearch_end, ":", &ep);
	cp != NULL; cp = sudo_strsplit(NULL, devsearch_end, ":", &ep)) {

	len = (size_t)(ep - cp);
	if (len >= sizeof(path)) {
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		"devsearch entry %.*s too long", (int)len, cp);
	    continue;
	}
	memcpy(path, cp, len);
	path[len] = '\0';

	if (strcmp(path, _PATH_DEV "pts") == 0) {
	    /* Special case /dev/pts */
	    len = (size_t)snprintf(path, sizeof(path), "%spts/%u",
		_PATH_DEV, (unsigned int)minor(rdev));
	    if (len >= sizeof(path)) {
		sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		    "devsearch entry %spts/%u too long",
		    _PATH_DEV, (unsigned int)minor(rdev));
		continue;
	    }
	    ret = sudo_dev_check(rdev, path, buf, buflen);
	    if (ret != NULL)
		goto done;
	} else {
	    /* Scan path, looking for rdev. */
	    ret = sudo_ttyname_scan(path, rdev, buf, buflen);
	    if (ret != NULL || errno == ENOMEM)
		goto done;
	}
    }

done:
    debug_return_str(ret);
}
#endif
