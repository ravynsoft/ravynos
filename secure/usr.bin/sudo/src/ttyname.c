/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2012-2022 Todd C. Miller <Todd.Miller@sudo.ws>
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

/* Large files not supported by procfs.h on Solaris. */
#if defined(HAVE_STRUCT_PSINFO_PR_TTYDEV)
# undef _FILE_OFFSET_BITS
# undef _LARGE_FILES
#endif

#include <sys/types.h>
#include <sys/stat.h>
#if defined(MAJOR_IN_MKDEV)
# include <sys/mkdev.h>
#elif defined(MAJOR_IN_SYSMACROS)
# include <sys/sysmacros.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <dirent.h>
#if defined(HAVE_KINFO_PROC2_NETBSD) || defined (HAVE_KINFO_PROC_OPENBSD) || defined(HAVE_KINFO_PROC_44BSD)
# include <sys/sysctl.h>
#elif defined(HAVE_KINFO_PROC_FREEBSD) || defined(HAVE_KINFO_PROC_DFLY)
# include <sys/param.h>
# include <sys/sysctl.h>
# include <sys/user.h>
#endif
#if defined(HAVE_PROCFS_H)
# include <procfs.h>
#elif defined(HAVE_SYS_PROCFS_H)
# include <sys/procfs.h>
#endif
#ifdef HAVE_PSTAT_GETPROC
# include <sys/pstat.h>
#endif

#include <sudo.h>

/*
 * How to access the tty device number in struct kinfo_proc.
 */
#if defined(HAVE_KINFO_PROC2_NETBSD)
# define SUDO_KERN_PROC		KERN_PROC2
# define sudo_kinfo_proc	kinfo_proc2
# define sudo_kp_tdev		p_tdev
# define sudo_kp_namelen	6
#elif defined(HAVE_KINFO_PROC_OPENBSD)
# define SUDO_KERN_PROC		KERN_PROC
# define sudo_kinfo_proc	kinfo_proc
# define sudo_kp_tdev		p_tdev
# define sudo_kp_namelen	6
#elif defined(HAVE_KINFO_PROC_FREEBSD)
# define SUDO_KERN_PROC		KERN_PROC
# define sudo_kinfo_proc	kinfo_proc
# define sudo_kp_tdev		ki_tdev
# define sudo_kp_namelen	4
#elif defined(HAVE_KINFO_PROC_DFLY)
# define SUDO_KERN_PROC		KERN_PROC
# define sudo_kinfo_proc	kinfo_proc
# define sudo_kp_tdev		kp_tdev
# define sudo_kp_namelen	4
#elif defined(HAVE_KINFO_PROC_44BSD)
# define SUDO_KERN_PROC		KERN_PROC
# define sudo_kinfo_proc	kinfo_proc
# define sudo_kp_tdev		kp_eproc.e_tdev
# define sudo_kp_namelen	4
#endif

#if defined(sudo_kp_tdev)
/*
 * Store the name of the tty to which the process is attached in name.
 * Returns name on success and NULL on failure, setting errno.
 */
char *
get_process_ttyname(char *name, size_t namelen)
{
    struct sudo_kinfo_proc *ki_proc = NULL;
    size_t size = sizeof(*ki_proc);
    int mib[6], rc, serrno = errno;
    char *ret = NULL;
    debug_decl(get_process_ttyname, SUDO_DEBUG_UTIL);

    /*
     * Lookup controlling tty for this process via sysctl.
     * This will work even if std{in,out,err} are redirected.
     */
    mib[0] = CTL_KERN;
    mib[1] = SUDO_KERN_PROC;
    mib[2] = KERN_PROC_PID;
    mib[3] = (int)getpid();
    mib[4] = sizeof(*ki_proc);
    mib[5] = 1;
    for (;;) {
	struct sudo_kinfo_proc *kp;

	size += size / 10;
	if ((kp = realloc(ki_proc, size)) == NULL) {
	    rc = -1;
	    break;		/* really out of memory. */
	}
	ki_proc = kp;
	rc = sysctl(mib, sudo_kp_namelen, ki_proc, &size, NULL, 0);
	if (rc != -1 || errno != ENOMEM)
	    break;
    }
    errno = ENOENT;
    if (rc != -1) {
	if ((dev_t)ki_proc->sudo_kp_tdev != (dev_t)-1) {
	    errno = serrno;
	    ret = sudo_ttyname_dev((dev_t)ki_proc->sudo_kp_tdev, name, namelen);
	    if (ret == NULL) {
		sudo_debug_printf(SUDO_DEBUG_WARN|SUDO_DEBUG_LINENO|SUDO_DEBUG_ERRNO,
		    "unable to map device number %lu to name",
		    (unsigned long)ki_proc->sudo_kp_tdev);
	    }
	}
    } else {
	sudo_debug_printf(SUDO_DEBUG_WARN|SUDO_DEBUG_LINENO|SUDO_DEBUG_ERRNO,
	    "unable to resolve tty via KERN_PROC");
    }
    free(ki_proc);

    debug_return_str(ret);
}
#elif defined(HAVE_STRUCT_PSINFO_PR_TTYDEV)
/*
 * Store the name of the tty to which the process is attached in name.
 * Returns name on success and NULL on failure, setting errno.
 */
char *
get_process_ttyname(char *name, size_t namelen)
{
    char path[PATH_MAX], *ret = NULL;
    struct psinfo psinfo;
    ssize_t nread;
    int fd, serrno = errno;
    debug_decl(get_process_ttyname, SUDO_DEBUG_UTIL);

    /* Try to determine the tty from pr_ttydev in /proc/pid/psinfo. */
    (void)snprintf(path, sizeof(path), "/proc/%u/psinfo", (unsigned int)getpid());
    if ((fd = open(path, O_RDONLY, 0)) != -1) {
	nread = read(fd, &psinfo, sizeof(psinfo));
	close(fd);
	if (nread == (ssize_t)sizeof(psinfo)) {
	    dev_t rdev = (dev_t)psinfo.pr_ttydev;
#if defined(_AIX) && defined(DEVNO64)
	    if ((psinfo.pr_ttydev & DEVNO64) && sizeof(dev_t) == 4)
		rdev = makedev(major64(psinfo.pr_ttydev), minor64(psinfo.pr_ttydev));
#endif
	    if (rdev != (dev_t)-1) {
		errno = serrno;
		ret = sudo_ttyname_dev(rdev, name, namelen);
		goto done;
	    }
	}
    } else {
	struct stat sb;
	int i;

	/* Missing /proc/pid/psinfo file. */
	for (i = STDIN_FILENO; i <= STDERR_FILENO; i++) {
	    if (sudo_isatty(i, &sb)) {
		ret = sudo_ttyname_dev(sb.st_rdev, name, namelen);
		goto done;
	    }
	}
    }
    errno = ENOENT;

done:
    if (ret == NULL)
	sudo_debug_printf(SUDO_DEBUG_WARN|SUDO_DEBUG_LINENO|SUDO_DEBUG_ERRNO,
	    "unable to resolve tty via %s", path);

    debug_return_str(ret);
}
#elif defined(__linux__)
/*
 * Store the name of the tty to which the process is attached in name.
 * Returns name on success and NULL on failure, setting errno.
 */
char *
get_process_ttyname(char *name, size_t namelen)
{
    const char path[] = "/proc/self/stat";
    char *cp, buf[1024];
    char *ret = NULL;
    int serrno = errno;
    pid_t ppid = 0;
    ssize_t nread;
    int fd;
    debug_decl(get_process_ttyname, SUDO_DEBUG_UTIL);

    /*
     * Try to determine the tty from tty_nr in /proc/self/stat.
     * Ignore /proc/self/stat if it contains embedded NUL bytes.
     */
    if ((fd = open(path, O_RDONLY | O_NOFOLLOW)) != -1) {
	cp = buf;
	while ((nread = read(fd, cp, sizeof(buf) - (size_t)(cp - buf))) != 0) {
	    if (nread == -1) {
		if (errno == EAGAIN || errno == EINTR)
		    continue;
		break;
	    }
	    cp += nread;
	    if (cp >= buf + sizeof(buf))
		break;
	}
	if (nread == 0 && memchr(buf, '\0', (size_t)(cp - buf)) == NULL) {
	    /*
	     * Field 7 is the tty dev (0 if no tty).
	     * Since the process name at field 2 "(comm)" may include
	     * whitespace (including newlines), start at the last ')' found.
	     */
	    *cp = '\0';
	    cp = strrchr(buf, ')');
	    if (cp != NULL) {
		char *ep = cp;
		const char *errstr;
		int field = 1;

		while (*++ep != '\0') {
		    if (*ep == ' ') {
			*ep = '\0';
			field++;
			if (field == 7) {
			    int tty_nr = (int)sudo_strtonum(cp, INT_MIN,
				INT_MAX, &errstr);
			    if (errstr) {
				sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
				    "%s: tty device %s: %s", path, cp, errstr);
			    }
			    if (tty_nr != 0) {
				/*
				 * Avoid sign extension when assigning tdev.
				 * tty_nr in /proc/self/stat is printed as a
				 * signed int but the actual device number is an
				 * unsigned int and dev_t is unsigned long long.
				 */
				dev_t tdev = (unsigned int)tty_nr;
				errno = serrno;
				ret = sudo_ttyname_dev(tdev, name, namelen);
				goto done;
			    }
			    break;
			}
			if (field == 3) {
			    ppid =
				(int)sudo_strtonum(cp, INT_MIN, INT_MAX, NULL);
			}
			cp = ep + 1;
		    }
		}
	    }
	}
    }
    if (ppid == 0) {
	struct stat sb;
	int i;

	/* No parent pid found, /proc/self/stat is missing or corrupt. */
	for (i = STDIN_FILENO; i <= STDERR_FILENO; i++) {
	    if (sudo_isatty(i, &sb)) {
		ret = sudo_ttyname_dev(sb.st_rdev, name, namelen);
		goto done;
	    }
	}
    }
    errno = ENOENT;

done:
    if (fd != -1)
	close(fd);
    if (ret == NULL)
	sudo_debug_printf(SUDO_DEBUG_WARN|SUDO_DEBUG_LINENO|SUDO_DEBUG_ERRNO,
	    "unable to resolve tty via %s", path);

    debug_return_str(ret);
}
#elif defined(HAVE_PSTAT_GETPROC)
/*
 * Store the name of the tty to which the process is attached in name.
 * Returns name on success and NULL on failure, setting errno.
 */
char *
get_process_ttyname(char *name, size_t namelen)
{
    struct pst_status pst;
    char *ret = NULL;
    int rc, serrno = errno;
    debug_decl(get_process_ttyname, SUDO_DEBUG_UTIL);

    /*
     * Determine the tty from psdev in struct pst_status.
     * EOVERFLOW is not a fatal error for the fields we use.
     * See the "EOVERFLOW Error" section of pstat_getvminfo(3).
     */
    rc = pstat_getproc(&pst, sizeof(pst), 0, getpid());
    if (rc != -1 || errno == EOVERFLOW) {
	if (pst.pst_term.psd_major != -1 && pst.pst_term.psd_minor != -1) {
	    errno = serrno;
	    ret = sudo_ttyname_dev(makedev(pst.pst_term.psd_major,
		pst.pst_term.psd_minor), name, namelen);
	    goto done;
	}
    }
    errno = ENOENT;

done:
    if (ret == NULL)
	sudo_debug_printf(SUDO_DEBUG_WARN|SUDO_DEBUG_LINENO|SUDO_DEBUG_ERRNO,
	    "unable to resolve tty via pstat");

    debug_return_str(ret);
}
#else
/*
 * Store the name of the tty to which the process is attached in name.
 * Returns name on success and NULL on failure, setting errno.
 */
char *
get_process_ttyname(char *name, size_t namelen)
{
    struct stat sb;
    char *tty;
    int i;
    debug_decl(get_process_ttyname, SUDO_DEBUG_UTIL);

    for (i = STDIN_FILENO; i <= STDERR_FILENO; i++) {
	/* Only call ttyname() on a character special device. */
	if (fstat(i, &sb) == -1 || !S_ISCHR(sb.st_mode))
	    continue;
	if ((tty = ttyname(i)) == NULL)
	    continue;

	if (strlcpy(name, tty, namelen) >= namelen) {
	    errno = ENAMETOOLONG;
	    sudo_debug_printf(
		SUDO_DEBUG_WARN|SUDO_DEBUG_LINENO|SUDO_DEBUG_ERRNO,
		"unable to store tty from ttyname");
	    debug_return_str(NULL);
	}
	debug_return_str(name);
    }

    sudo_debug_printf(SUDO_DEBUG_WARN|SUDO_DEBUG_LINENO|SUDO_DEBUG_ERRNO,
	"unable to resolve tty via ttyname");
    errno = ENOENT;
    debug_return_str(NULL);
}
#endif
