/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2014 Theo de Raadt <deraadt@openbsd.org>
 * Copyright (c) 2014 Bob Beck <beck@obtuse.com>
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
 * Emulation of getentropy(2) as documented at:
 * http://man.openbsd.org/getentropy.2
 */

/*
 * This is an open source non-commercial project. Dear PVS-Studio, please check it.
 * PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
 */

#include <config.h>

#ifndef HAVE_GETENTROPY

#include <sys/param.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/stat.h>
#ifdef HAVE_SYS_SYSCTL_H
# include <sys/sysctl.h>
#endif
#ifdef HAVE_SYS_STATVFS_H
# include <sys/statvfs.h>
#endif
#include <sys/stat.h>
#include <sys/time.h>
#ifdef HAVE_SYS_SYSCALL_H
# include <sys/syscall.h>
#endif
#ifdef HAVE_LINUX_RANDOM_H
# include <linux/types.h>
# include <linux/random.h>
#endif
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#if defined(HAVE_STDINT_H)
# include <stdint.h>
#elif defined(HAVE_INTTYPES_H)
# include <inttypes.h>
#endif
#ifdef HAVE_GETAUXVAL
# include <sys/auxv.h>
#endif
#ifdef HAVE_DL_ITERATE_PHDR
# include <link.h>
#endif
#ifdef HAVE_OPENSSL
# if defined(HAVE_WOLFSSL)
#  include <wolfssl/options.h>
# endif
# include <openssl/rand.h>
#endif

#include <sudo_compat.h>
#include <sudo_digest.h>
#include <sudo_rand.h>

#if !defined(MAP_ANON) && defined(MAP_ANONYMOUS)
# define MAP_ANON MAP_ANONYMOUS
#endif

#ifndef MAP_FAILED
# define MAP_FAILED ((void *) -1)
#endif

#define REPEAT 5
#define min(a, b) (((a) < (b)) ? (a) : (b))

#define HX(a, b) \
	do { \
		if ((a)) \
			HD(errno); \
		else \
			HD(b); \
	} while (0)

#define HR(x, l) (sudo_digest_update(ctx, (char *)(x), (l)))
#define HD(x)    (sudo_digest_update(ctx, (char *)&(x), sizeof (x)))
#define HF(x)    (sudo_digest_update(ctx, (char *)&(x), sizeof (void*)))

int	sudo_getentropy(void *buf, size_t len);

static int getentropy_getrandom(void *buf, size_t len);
static int getentropy_sysctl(void *buf, size_t len);
static int getentropy_urandom(void *buf, size_t len, const char *path,
    int devfscheck);
static int getentropy_fallback(void *buf, size_t len);
static int gotdata(char *buf, size_t len);
#ifdef HAVE_DL_ITERATE_PHDR
static int getentropy_phdr(struct dl_phdr_info *info, size_t size, void *data);
#endif

static void *
mmap_anon(void *addr, size_t len, int prot, int flags, off_t offset)
{
#ifdef MAP_ANON
	return mmap(addr, len, prot, flags | MAP_ANON, -1, offset);
#else
	int fd;

	if ((fd = open("/dev/zero", O_RDWR)) == -1)
		return MAP_FAILED;
	addr = mmap(addr, len, prot, flags, fd, offset);
	close(fd);
	return addr;
#endif
}

int
sudo_getentropy(void *buf, size_t len)
{
	int ret = -1;

	if (len > 256) {
		errno = EIO;
		return (-1);
	}

	ret = getentropy_getrandom(buf, len);
	if (ret != -1)
		return (ret);

#ifdef HAVE_OPENSSL
	if (RAND_bytes(buf, (int)len) == 1)
		return (0);
#endif

	ret = getentropy_sysctl(buf, len);
	if (ret != -1)
		return (ret);

	/*
	 * Try to get entropy with /dev/urandom
	 */
	ret = getentropy_urandom(buf, len, "/dev/urandom", 0);
	if (ret != -1)
		return (ret);

	/*
	 * Entropy collection via /dev/urandom has failed.
	 *
	 * No other API exists for collecting entropy, and we have no
	 * failsafe way to get it that is not sensitive to resource exhaustion.
	 *
	 * We have very few options:
	 *     - Even syslog_r is unsafe to call at this low level, so
	 *	 there is no way to alert the user or program.
	 *     - Cannot call abort() because some systems have unsafe
	 *	 corefiles.
	 *     - Could raise(SIGKILL) resulting in silent program termination.
	 *     - Return EIO, to hint that arc4random's stir function
	 *       should raise(SIGKILL)
	 *     - Do the best under the circumstances....
	 *
	 * This code path exists to bring light to the issue that the OS
	 * does not provide a failsafe API for entropy collection.
	 *
	 * We hope this demonstrates that the OS should consider
	 * providing a new failsafe API which works in a chroot or
	 * when file descriptors are exhausted.
	 */
#undef FAIL_INSTEAD_OF_TRYING_FALLBACK
#ifdef FAIL_INSTEAD_OF_TRYING_FALLBACK
	raise(SIGKILL);
#endif
	ret = getentropy_fallback(buf, len);
	if (ret != -1)
		return (ret);

	errno = EIO;
	return (ret);
}

/*
 * This is an open source non-commercial project. Dear PVS-Studio, please check it.
 * PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
 */

/*
 * Basic validity checking; wish we could do better.
 */
static int
gotdata(char *buf, size_t len)
{
	char	any_set = 0;
	size_t	i;

	for (i = 0; i < len; ++i)
		any_set |= buf[i];
	if (any_set == 0)
		return (-1);
	return (0);
}

static int
getentropy_urandom(void *buf, size_t len, const char *path, int devfscheck)
{
	struct stat st;
	size_t i;
	int fd, flags;
	int save_errno = errno;

start:

	/* We do not use O_NOFOLLOW since /dev/urandom is a link on Solaris. */
	flags = O_RDONLY;
#ifdef O_CLOEXEC
	flags |= O_CLOEXEC;
#endif
	fd = open(path, flags, 0);
	if (fd == -1) {
		if (errno == EINTR)
			goto start;
		goto nodevrandom;
	}
#ifndef O_CLOEXEC
	fcntl(fd, F_SETFD, fcntl(fd, F_GETFD) | FD_CLOEXEC);
#endif

	/* Lightly verify that the device node looks OK */
	if (fstat(fd, &st) == -1 || !S_ISCHR(st.st_mode)) {
		close(fd);
		goto nodevrandom;
	}
	for (i = 0; i < len; ) {
		size_t wanted = len - i;
		size_t ret = (size_t)read(fd, (char *)buf + i, wanted);

		if (ret == (size_t)-1) {
			if (errno == EAGAIN || errno == EINTR)
				continue;
			close(fd);
			goto nodevrandom;
		}
		i += ret;
	}
	close(fd);
	if (gotdata(buf, len) == 0) {
		errno = save_errno;
		return (0);		/* satisfied */
	}
nodevrandom:
	errno = EIO;
	return (-1);
}

#if defined(HAVE_SYSCTL) && defined(KERN_ARND)
static int
getentropy_sysctl(void *buf, size_t len)
{
	int save_errno = errno;
	int mib[2];
	size_t i;

	mib[0] = CTL_KERN;
	mib[1] = KERN_ARND;

	for (i = 0; i < len; ) {
		size_t chunk = len - i;

		if (sysctl(mib, 2, (char *)buf + i, &chunk, NULL, 0) == -1)
			goto sysctlfailed;
		i += chunk;
	}
	if (gotdata(buf, len) == 0) {
		errno = save_errno;
		return (0);			/* satisfied */
	}
sysctlfailed:
	errno = EIO;
	return (-1);
}
#elif defined(SYS__sysctl) && defined(RANDOM_UUID)
static int
getentropy_sysctl(void *buf, size_t len)
{
	static int mib[3];
	size_t i;
	int save_errno = errno;

	mib[0] = CTL_KERN;
	mib[1] = KERN_RANDOM;
	mib[2] = RANDOM_UUID;

	for (i = 0; i < len; ) {
		size_t chunk = min(len - i, 16);

		/* SYS__sysctl because some systems already removed sysctl() */
		struct __sysctl_args args = {
			.name = mib,
			.nlen = 3,
			.oldval = (char *)buf + i,
			.oldlenp = &chunk,
		};
		if (syscall(SYS__sysctl, &args) != 0)
			goto sysctlfailed;
		i += chunk;
	}
	if (gotdata(buf, len) == 0) {
		errno = save_errno;
		return (0);			/* satisfied */
	}
sysctlfailed:
	errno = EIO;
	return (-1);
}
#else
static int
getentropy_sysctl(void *buf, size_t len)
{
	errno = ENOTSUP;
	return (-1);
}
#endif

#if defined(SYS_getrandom) && defined(GRND_NONBLOCK)
static int
getentropy_getrandom(void *buf, size_t len)
{
	int pre_errno = errno;
	long ret;

        /*
         * Try descriptor-less getrandom(), in non-blocking mode.
         *
         * The design of Linux getrandom is broken.  It has an
         * uninitialized phase coupled with blocking behaviour, which
         * is unacceptable from within a library at boot time without
         * possible recovery. See http://bugs.python.org/issue26839#msg267745
         */
	do {
		ret = syscall(SYS_getrandom, buf, len, GRND_NONBLOCK);
	} while (ret == -1 && errno == EINTR);

	if (ret < 0 || (size_t)ret != len)
		return (-1);
	errno = pre_errno;
	return (0);
}
#else
static int
getentropy_getrandom(void *buf, size_t len)
{
	errno = ENOTSUP;
	return (-1);
}
#endif

#ifdef HAVE_CLOCK_GETTIME
static const int cl[] = {
	CLOCK_REALTIME,
#ifdef CLOCK_MONOTONIC
	CLOCK_MONOTONIC,
#endif
#ifdef CLOCK_MONOTONIC_RAW
	CLOCK_MONOTONIC_RAW,
#endif
#ifdef CLOCK_TAI
	CLOCK_TAI,
#endif
#ifdef CLOCK_VIRTUAL
	CLOCK_VIRTUAL,
#endif
#ifdef CLOCK_UPTIME
	CLOCK_UPTIME,
#endif
#ifdef CLOCK_PROCESS_CPUTIME_ID
	CLOCK_PROCESS_CPUTIME_ID,
#endif
#ifdef CLOCK_THREAD_CPUTIME_ID
	CLOCK_THREAD_CPUTIME_ID,
#endif
};
#endif /* HAVE_CLOCK_GETTIME */

#ifdef HAVE_DL_ITERATE_PHDR
static int
getentropy_phdr(struct dl_phdr_info *info, size_t size, void *data)
{
	struct sudo_digest *ctx = data;

	sudo_digest_update(ctx, &info->dlpi_addr, sizeof (info->dlpi_addr));
	return (0);
}
#endif

static int
getentropy_fallback(void *buf, size_t len)
{
	unsigned char *results = NULL;
	int save_errno = errno, e, faster = 0;
	int ret = -1;
	static size_t cnt;
	unsigned int repeat;
	size_t pgs;
	struct timespec ts;
	struct timeval tv;
	struct rusage ru;
	sigset_t set;
	struct stat st;
	struct sudo_digest *ctx;
	static pid_t lastpid;
	pid_t pid;
	size_t i, ii, m, digest_len;
	char *p;

	if (len == 0)
		return 0;
	pgs = (size_t)sysconf(_SC_PAGESIZE);
	if (pgs == (size_t)-1)
		return -1;
	if ((ctx = sudo_digest_alloc(SUDO_DIGEST_SHA512)) == NULL)
		return -1;
	digest_len = sudo_digest_getlen(SUDO_DIGEST_SHA512);
	if (digest_len == 0 || (results = malloc(digest_len)) == NULL)
		goto done;

	pid = getpid();
	if (lastpid == pid) {
		faster = 1;
		repeat = 2;
	} else {
		faster = 0;
		lastpid = pid;
		repeat = REPEAT;
	}
	i = 0;
	do {
		unsigned int j;
		for (j = 0; j < repeat; j++) {
			HX((e = gettimeofday(&tv, NULL)) == -1, tv);
			if (e != -1) {
				cnt += (size_t)tv.tv_sec;
				cnt += (size_t)tv.tv_usec;
			}
#ifdef HAVE_DL_ITERATE_PHDR
			dl_iterate_phdr(getentropy_phdr, ctx);
#endif

#ifdef HAVE_CLOCK_GETTIME
			for (ii = 0; ii < sizeof(cl)/sizeof(cl[0]); ii++)
				HX(clock_gettime(cl[ii], &ts) == -1, ts);
#endif /* HAVE_CLOCK_GETTIME */

			HX((pid = getpid()) == -1, pid);
			HX((pid = getsid(pid)) == -1, pid);
			HX((pid = getppid()) == -1, pid);
			HX((pid = getpgid(0)) == -1, pid);
			HX((e = getpriority(0, 0)) == -1, e);

			if (!faster) {
				ts.tv_sec = 0;
				ts.tv_nsec = 1;
				(void) nanosleep(&ts, NULL);
			}

			HX(sigpending(&set) == -1, set);
			HX(sigprocmask(SIG_BLOCK, NULL, &set) == -1, set);

			HF(sudo_getentropy);	/* an addr in this library */
			HF(printf);		/* an addr in libc */
			p = (char *)&p;
			HD(p);		/* an addr on stack */
			p = (char *)&errno;
			HD(p);		/* the addr of errno */

			if (i == 0) {
#ifdef HAVE_SYS_STATVFS_H
				struct statvfs stvfs;
#endif
				struct termios tios;
				off_t off;

				/*
				 * Prime-sized mappings encourage fragmentation;
				 * thus exposing some address entropy.
				 */
				struct mm {
					size_t	npg;
					void	*p;
				} mm[] =	 {
					{ 17, MAP_FAILED }, { 3, MAP_FAILED },
					{ 11, MAP_FAILED }, { 2, MAP_FAILED },
					{ 5, MAP_FAILED }, { 3, MAP_FAILED },
					{ 7, MAP_FAILED }, { 1, MAP_FAILED },
					{ 57, MAP_FAILED }, { 3, MAP_FAILED },
					{ 131, MAP_FAILED }, { 1, MAP_FAILED },
				};

				for (m = 0; m < sizeof mm/sizeof(mm[0]); m++) {
					HX(mm[m].p = mmap_anon(NULL,
					    mm[m].npg * pgs,
					    PROT_READ|PROT_WRITE,
					    MAP_PRIVATE,
					    (off_t)0), mm[m].p);
					if (mm[m].p != MAP_FAILED) {
						size_t mo;

						/* Touch some memory... */
						p = mm[m].p;
						mo = cnt %
						    (mm[m].npg * pgs - 1);
						p[mo] = 1;
						cnt += (size_t)mm[m].p / pgs;
					}

#ifdef HAVE_CLOCK_GETTIME
					/* Check cnts and times... */
					for (ii = 0; ii < sizeof(cl)/sizeof(cl[0]);
					    ii++) {
						HX((e = clock_gettime(cl[ii],
						    &ts)) == -1, ts);
						if (e != -1)
							cnt += (size_t)ts.tv_nsec;
					}
#endif /* HAVE_CLOCK_GETTIME */

					HX((e = getrusage(RUSAGE_SELF,
					    &ru)) == -1, ru);
					if (e != -1) {
						cnt += (size_t)ru.ru_utime.tv_sec;
						cnt += (size_t)ru.ru_utime.tv_usec;
					}
				}

				for (m = 0; m < sizeof mm/sizeof(mm[0]); m++) {
					if (mm[m].p != MAP_FAILED)
						munmap(mm[m].p, mm[m].npg * pgs);
					mm[m].p = MAP_FAILED;
				}

				HX(stat(".", &st) == -1, st);
				HX(stat("/", &st) == -1, st);

#ifdef HAVE_SYS_STATVFS_H
				HX(statvfs(".", &stvfs) == -1, stvfs);
				HX(statvfs("/", &stvfs) == -1, stvfs);
#endif
				HX((e = fstat(0, &st)) == -1, st);
				if (e == -1) {
					if (S_ISREG(st.st_mode) ||
					    S_ISFIFO(st.st_mode) ||
					    S_ISSOCK(st.st_mode)) {
#ifdef HAVE_SYS_STATVFS_H
						HX(fstatvfs(0, &stvfs) == -1,
						    stvfs);
#endif
						HX((off = lseek(0, (off_t)0,
						    SEEK_CUR)) < 0, off);
					}
					if (S_ISCHR(st.st_mode)) {
						HX(tcgetattr(0, &tios) == -1,
						    tios);
#if 0
					} else if (S_ISSOCK(st.st_mode)) {
						struct sockaddr_storage ss;
						socklen_t ssl;
						memset(&ss, 0, sizeof ss);
						ssl = sizeof(ss);
						HX(getpeername(0,
						    (void *)&ss, &ssl) == -1,
						    ss);
#endif
					}
				}

				HX((e = getrusage(RUSAGE_CHILDREN,
				    &ru)) == -1, ru);
				if (e != -1) {
					cnt += (size_t)ru.ru_utime.tv_sec;
					cnt += (size_t)ru.ru_utime.tv_usec;
				}
			} else {
				/* Subsequent hashes absorb previous result */
				HR(results, digest_len);
			}

			HX((e = gettimeofday(&tv, NULL)) == -1, tv);
			if (e != -1) {
				cnt += (size_t)tv.tv_sec;
				cnt += (size_t)tv.tv_usec;
			}

			HD(cnt);
		}

#ifdef HAVE_GETAUXVAL
#ifdef AT_RANDOM
		/* Not as random as you think but we take what we are given */
		p = (char *) getauxval(AT_RANDOM);
		if (p)
			HR(p, 16);
#endif
#ifdef AT_SYSINFO_EHDR
		p = (char *) getauxval(AT_SYSINFO_EHDR);
		if (p)
			HR(p, pgs);
#endif
#ifdef AT_BASE
		p = (char *) getauxval(AT_BASE);
		if (p)
			HD(p);
#endif
#endif /* HAVE_GETAUXVAL  */

		sudo_digest_final(ctx, results);
		sudo_digest_reset(ctx);
		memcpy((char *)buf + i, results, min(digest_len, len - i));
		i += min(digest_len, len - i);
	} while (i < len);
	if (gotdata(buf, len) == 0) {
		errno = save_errno;
		ret = 0;		/* satisfied */
	} else {
		errno = EIO;
	}
done:
	sudo_digest_free(ctx);
	if (results != NULL)
		freezero(results, digest_len);
	return (ret);
}

#endif /* HAVE_GETENTROPY */
