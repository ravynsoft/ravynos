/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2012-2020 Todd C. Miller <Todd.Miller@sudo.ws>
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
#if defined(HAVE_KINFO_PROC_44BSD) || defined (HAVE_KINFO_PROC_OPENBSD) || defined(HAVE_KINFO_PROC2_NETBSD)
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

#if defined(__gnu_hurd__)
# include <hurd.h>
# include <mach/task_info.h>
# include <mach/time_value.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <unistd.h>

#include <sudoers.h>
#include <timestamp.h>

/*
 * Arguments for sysctl(2) when reading the process start time.
 */
#if defined(HAVE_KINFO_PROC2_NETBSD)
# define SUDO_KERN_PROC		KERN_PROC2
# define sudo_kinfo_proc	kinfo_proc2
# define sudo_kp_namelen	6
#elif defined(HAVE_KINFO_PROC_OPENBSD)
# define SUDO_KERN_PROC		KERN_PROC
# define sudo_kinfo_proc	kinfo_proc
# define sudo_kp_namelen	6
#elif defined(HAVE_KINFO_PROC_FREEBSD) || defined(HAVE_KINFO_PROC_DFLY) || defined(HAVE_KINFO_PROC_44BSD)
# define SUDO_KERN_PROC		KERN_PROC
# define sudo_kinfo_proc	kinfo_proc
# define sudo_kp_namelen	4
#endif

/*
 * Store start time of the specified process in starttime.
 */

#if defined(sudo_kinfo_proc)
int
get_starttime(pid_t pid, struct timespec *starttime)
{
    struct sudo_kinfo_proc *ki_proc = NULL;
    size_t size = sizeof(*ki_proc);
    int mib[6], rc;
    debug_decl(get_starttime, SUDOERS_DEBUG_UTIL);

    /*
     * Lookup start time for pid via sysctl.
     */
    mib[0] = CTL_KERN;
    mib[1] = SUDO_KERN_PROC;
    mib[2] = KERN_PROC_PID;
    mib[3] = (int)pid;
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
    if (rc != -1) {
#if defined(HAVE_KINFO_PROC_FREEBSD)
	/* FreeBSD. */
	TIMEVAL_TO_TIMESPEC(&ki_proc->ki_start, starttime);
#elif defined(HAVE_KINFO_PROC_DFLY)
	/* Dragonfly. */
	TIMEVAL_TO_TIMESPEC(&ki_proc->kp_start, starttime);
#elif defined(HAVE_KINFO_PROC_44BSD)
	/* 4.4BSD and macOS */
	TIMEVAL_TO_TIMESPEC(&ki_proc->kp_proc.p_starttime, starttime);
#else
	/* NetBSD and OpenBSD */
	starttime->tv_sec = (time_t)ki_proc->p_ustart_sec;
	starttime->tv_nsec = (long)(ki_proc->p_ustart_usec * 1000);
#endif
	sudo_debug_printf(SUDO_DEBUG_INFO,
	    "%s: start time for %d: { %lld, %ld }", __func__,
	    (int)pid, (long long)starttime->tv_sec, (long)starttime->tv_nsec);
    } else {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO|SUDO_DEBUG_ERRNO,
	    "unable to get start time for %d via KERN_PROC", (int)pid);
    }
    free(ki_proc);

    debug_return_int(rc == -1 ? -1 : 0);
}
#elif defined(HAVE_STRUCT_PSINFO_PR_TTYDEV)
int
get_starttime(pid_t pid, struct timespec *starttime)
{
    struct psinfo psinfo;
    char path[PATH_MAX];
    ssize_t nread;
    int fd, ret = -1;
    debug_decl(get_starttime, SUDOERS_DEBUG_UTIL);

    /* Determine the start time from pr_start in /proc/pid/psinfo. */
    (void)snprintf(path, sizeof(path), "/proc/%u/psinfo", (unsigned int)pid);
    if ((fd = open(path, O_RDONLY, 0)) != -1) {
	nread = read(fd, &psinfo, sizeof(psinfo));
	close(fd);
	if (nread == (ssize_t)sizeof(psinfo)) {
	    starttime->tv_sec = psinfo.pr_start.tv_sec;
	    starttime->tv_nsec = psinfo.pr_start.tv_nsec;
	    ret = 0;

	    sudo_debug_printf(SUDO_DEBUG_INFO,
		"%s: start time for %d: { %lld, %ld }", __func__, (int)pid,
		(long long)starttime->tv_sec, (long)starttime->tv_nsec);
	}
    }

    if (ret == -1)
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO|SUDO_DEBUG_ERRNO,
	    "unable to get start time for %d via %s", (int)pid, path);
    debug_return_int(ret);
}
#elif defined(__linux__)
int
get_starttime(pid_t pid, struct timespec *starttime)
{
    char path[PATH_MAX];
    char *cp, buf[1024];
    ssize_t nread;
    int ret = -1;
    int fd = -1;
    long tps;
    debug_decl(get_starttime, SUDOERS_DEBUG_UTIL);

    /*
     * Start time is in ticks per second on Linux.
     */
    tps = sysconf(_SC_CLK_TCK);
    if (tps == -1)
	goto done;

    /*
     * Determine the start time from 22nd field in /proc/pid/stat.
     * Ignore /proc/self/stat if it contains embedded NUL bytes.
     * XXX - refactor common code with ttyname.c?
     */
    (void)snprintf(path, sizeof(path), "/proc/%u/stat", (unsigned int)pid);
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
	     * Field 22 is the start time (%ull).
	     * Since the process name at field 2 "(comm)" may include
	     * whitespace (including newlines), start at the last ')' found.
	     */
	    *cp = '\0';
	    cp = strrchr(buf, ')');
	    if (cp != NULL) {
		char *ep = cp;
		int field = 1;

		while (*++ep != '\0') {
		    if (*ep == ' ') {
			if (++field == 22) {
			    unsigned long long ullval;

			    /* Must start with a digit (not negative). */
			    if (!isdigit((unsigned char)*cp)) {
				errno = EINVAL;
				goto done;
			    }

			    /* starttime is %ul in 2.4 and %ull in >= 2.6 */
			    errno = 0;
			    ullval = strtoull(cp, &ep, 10);
			    if (ep == cp || *ep != ' ') {
				errno = EINVAL;
				goto done;
			    }
			    if (errno == ERANGE && ullval == ULLONG_MAX)
				goto done;

			    /* Convert from ticks to timespec */
			    starttime->tv_sec = (time_t)(ullval / tps);
			    starttime->tv_nsec =
				(long)(ullval % tps) * (1000000000 / tps);
			    ret = 0;

			    sudo_debug_printf(SUDO_DEBUG_INFO,
				"%s: start time for %d: { %lld, %ld }",
				__func__, (int)pid,
				(long long)starttime->tv_sec,
				(long)starttime->tv_nsec);

			    goto done;
			}
			cp = ep + 1;
		    }
		}
	    }
	}
    }
    errno = ENOENT;

done:
    if (fd != -1)
	close(fd);
    if (ret == -1)
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO|SUDO_DEBUG_ERRNO,
	    "unable to get start time for %d via %s", (int)pid, path);

    debug_return_int(ret);
}
#elif defined(HAVE_PSTAT_GETPROC)
int
get_starttime(pid_t pid, struct timespec *starttime)
{
    struct pst_status pst;
    int rc;
    debug_decl(get_starttime, SUDOERS_DEBUG_UTIL);

    /*
     * Determine the start time from pst_start in struct pst_status.
     * EOVERFLOW is not a fatal error for the fields we use.
     * See the "EOVERFLOW Error" section of pstat_getvminfo(3).
     */
    rc = pstat_getproc(&pst, sizeof(pst), 0, pid);
    if (rc != -1 || errno == EOVERFLOW) {
	starttime->tv_sec = (time_t)pst.pst_start;
	starttime->tv_nsec = 0;

	sudo_debug_printf(SUDO_DEBUG_INFO,
	    "%s: start time for %d: { %lld, %ld }", __func__,
	    (int)pid, (long long)starttime->tv_sec, (long)starttime->tv_nsec);

	debug_return_int(0);
    }

    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO|SUDO_DEBUG_ERRNO,
	"unable to get start time for %d via pstat_getproc", (int)pid);
    debug_return_int(-1);
}
#elif defined(__gnu_hurd__)
int
get_starttime(pid_t pid, struct timespec *starttime)
{
    mach_msg_type_number_t count;
    struct task_basic_info info;
    kern_return_t error;
    task_t target;
    debug_decl(get_starttime, SUDOERS_DEBUG_UTIL);

    target = pid2task(pid);
    if (target != MACH_PORT_NULL) {
	count = sizeof(info) / sizeof(integer_t);
	error = task_info(target, TASK_BASIC_INFO, (task_info_t)&info, &count);
	if (error == KERN_SUCCESS) {
	    starttime->tv_sec = (time_t)info.creation_time.seconds;
	    starttime->tv_nsec = (long)(info.creation_time.microseconds * 1000);
	    debug_return_int(0);
	}
    }

    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO|SUDO_DEBUG_ERRNO,
	"unable to get start time for %d via task_info", (int)pid);
    debug_return_int(-1);
}
#else
int
get_starttime(pid_t pid, struct timespec *starttime)
{
    debug_decl(get_starttime, SUDOERS_DEBUG_UTIL);

    sudo_debug_printf(SUDO_DEBUG_WARN|SUDO_DEBUG_LINENO,
	"process start time not supported by sudo on this system");
    debug_return_int(-1);
}
#endif
