/*
 * compat.c - compatibility routines for the deprived
 *
 * This file is part of zsh, the Z shell.
 *
 * Copyright (c) 1992-1997 Paul Falstad
 * All rights reserved.
 *
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and to distribute modified versions of this software for any
 * purpose, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 *
 * In no event shall Paul Falstad or the Zsh Development Group be liable
 * to any party for direct, indirect, special, incidental, or consequential
 * damages arising out of the use of this software and its documentation,
 * even if Paul Falstad and the Zsh Development Group have been advised of
 * the possibility of such damage.
 *
 * Paul Falstad and the Zsh Development Group specifically disclaim any
 * warranties, including, but not limited to, the implied warranties of
 * merchantability and fitness for a particular purpose.  The software
 * provided hereunder is on an "as is" basis, and Paul Falstad and the
 * Zsh Development Group have no obligation to provide maintenance,
 * support, updates, enhancements, or modifications.
 *
 */

#include "zsh.mdh"
#include "compat.pro"

/* Return pointer to first occurrence of string t *
 * in string s.  Return NULL if not present.      */

/**/
#ifndef HAVE_STRSTR

/**/
char *
strstr(const char *s, const char *t)
{
    const char *p1, *p2;

    for (; *s; s++) {
        for (p1 = s, p2 = t; *p2; p1++, p2++)
            if (*p1 != *p2)
                break;
        if (!*p2)
            return (char *)s;
    }
    return NULL;
}

/**/
#endif


/**/
#ifndef HAVE_GETHOSTNAME

/**/
int
gethostname(char *name, size_t namelen)
{
    struct utsname uts;

    uname(&uts);
    if(strlen(uts.nodename) >= namelen) {
	errno = EINVAL;
	return -1;
    }
    strcpy(name, uts.nodename);
    return 0;
}

/**/
#endif


/**/
#ifndef HAVE_GETTIMEOFDAY

/**/
int
gettimeofday(struct timeval *tv, struct timezone *tz)
{
    tv->tv_usec = 0;
    tv->tv_sec = (long)time((time_t) 0);
    return 0;
}

/**/
#endif


/* Provide clock time with nanoseconds */

/**/
mod_export int
zgettime(struct timespec *ts)
{
    int ret = -1;

#ifdef HAVE_CLOCK_GETTIME
    struct timespec dts;
    if (clock_gettime(CLOCK_REALTIME, &dts) < 0) {
	zwarn("unable to retrieve time: %e", errno);
	ret--;
    } else {
	ret++;
	ts->tv_sec = (time_t) dts.tv_sec;
	ts->tv_nsec = (long) dts.tv_nsec;
    }
#endif

    if (ret) {
	struct timeval dtv;
	struct timezone dtz;
	gettimeofday(&dtv, &dtz);
	ret++;
	ts->tv_sec = (time_t) dtv.tv_sec;
	ts->tv_nsec = (long) dtv.tv_usec * 1000;
    }

    return ret;
}

/* Likewise with CLOCK_MONOTONIC if available. */

/**/
mod_export int
zgettime_monotonic_if_available(struct timespec *ts)
{
    int ret = -1;

#if defined(HAVE_CLOCK_GETTIME) && defined(CLOCK_MONOTONIC)
    struct timespec dts;
    if (clock_gettime(CLOCK_MONOTONIC, &dts) < 0) {
	zwarn("unable to retrieve CLOCK_MONOTONIC time: %e", errno);
	ret--;
    } else {
	ret++;
	ts->tv_sec = (time_t) dts.tv_sec;
	ts->tv_nsec = (long) dts.tv_nsec;
    }
#endif

    if (ret) {
	ret = zgettime(ts);
    }
    return ret;
}


/* compute the difference between two calendar times */

/**/
#ifndef HAVE_DIFFTIME

/**/
double
difftime(time_t t2, time_t t1)
{
    return ((double)t2 - (double)t1);
}

/**/
#endif


/**/
#ifndef HAVE_STRERROR
extern char *sys_errlist[];

/* Get error message string associated with a particular  *
 * error number, and returns a pointer to that string.    *
 * This is not a particularly robust version of strerror. */

/**/
char *
strerror(int errnum)
{
    return (sys_errlist[errnum]);
}

/**/
#endif


#if 0
/* pathconf(_PC_PATH_MAX) is not currently useful to zsh.  The value *
 * returned varies depending on a number of factors, e.g. the amount *
 * of memory available to the operating system at a given time; thus *
 * it can't be used for buffer allocation, or even as an indication  *
 * of whether an attempt to use or create a given pathname may fail  *
 * at any future time.                                               *
 *                                                                   *
 * The call is also permitted to fail if the argument path is not an *
 * existing directory, so even to make sense of that one must search *
 * for a valid directory somewhere in the path and adjust.  Even if  *
 * it succeeds, the return value is relative to the input directory, *
 * and therefore potentially relative to the length of the shortest  *
 * path either to that directory or to our working directory.        *
 *                                                                   *
 * Finally, see the note below for glibc; detection of pathconf() is *
 * not by itself an indication that it works reliably.               */

/* The documentation for pathconf() says something like:             *
 *     The limit is returned, if one exists.  If the system  does    *
 *     not  have  a  limit  for  the  requested  resource,  -1 is    *
 *     returned, and errno is unchanged.  If there is  an  error,    *
 *     -1  is returned, and errno is set to reflect the nature of    *
 *     the error.                                                    *
 *                                                                   *
 * System calls are not permitted to set errno to 0; but we must (or *
 * some other flag value) in order to determine that the resource is *
 * unlimited.  What use is leaving errno unchanged?  Instead, define *
 * a wrapper that resets errno to 0 and returns 0 for "the system    *
 * does not have a limit," so that -1 always means a real error.     */

/**/
mod_export long
zpathmax(char *dir)
{
#ifdef HAVE_PATHCONF
    long pathmax;

    errno = 0;
    if ((pathmax = pathconf(dir, _PC_PATH_MAX)) >= 0) {
	/* Some versions of glibc pathconf return a hardwired value! */
	return pathmax;
    } else if (errno == EINVAL || errno == ENOENT || errno == ENOTDIR) {
	/* Work backward to find a directory, until we run out of path. */
	char *tail = strrchr(dir, '/');
	while (tail > dir && tail[-1] == '/')
	    --tail;
	if (tail > dir) {
	    *tail = 0;
	    pathmax = zpathmax(dir);
	    *tail = '/';
	} else {
	    errno = 0;
	    if (tail)
		pathmax = pathconf("/", _PC_PATH_MAX);
	    else
		pathmax = pathconf(".", _PC_PATH_MAX);
	}
	if (pathmax > 0) {
	    long taillen = (tail ? strlen(tail) : (strlen(dir) + 1));
	    if (taillen < pathmax)
		return pathmax - taillen;
	    else
		errno = ENAMETOOLONG;
	}
    }
    if (errno)
	return -1;
    else
	return 0; /* pathmax should be considered unlimited */
#else
    long dirlen = strlen(dir);

    /* The following is wrong if dir is not an absolute path. */
    return ((long) ((dirlen >= PATH_MAX) ?
		    ((errno = ENAMETOOLONG), -1) :
		    ((errno = 0), PATH_MAX - dirlen)));
#endif
}
#endif /* 0 */

/**/
#ifdef HAVE_SYSCONF
/*
 * This is replaced by a macro from system.h if not HAVE_SYSCONF.
 * 0 is returned by sysconf if _SC_OPEN_MAX is unavailable;
 * -1 is returned on error
 *
 * Neither of these should happen, but resort to OPEN_MAX rather
 * than return 0 or -1 just in case.
 *
 * We'll limit the open maximum to ZSH_INITIAL_OPEN_MAX to
 * avoid probing ridiculous numbers of file descriptors.
 */

/**/
mod_export long
zopenmax(void)
{
    long openmax;

    if ((openmax = sysconf(_SC_OPEN_MAX)) < 1) {
	openmax = OPEN_MAX;
    } else if (openmax > OPEN_MAX) {
	/* On some systems, "limit descriptors unlimited" or the  *
	 * equivalent will set openmax to a huge number.  Unless  *
	 * there actually is a file descriptor > OPEN_MAX already *
	 * open, nothing in zsh requires the true maximum, and in *
	 * fact it causes inefficiency elsewhere if we report it. *
	 * So, report the maximum of OPEN_MAX or the largest open *
	 * descriptor (is there a better way to find that?).      */
	long i, j = OPEN_MAX;
	if (openmax > ZSH_INITIAL_OPEN_MAX)
	    openmax = ZSH_INITIAL_OPEN_MAX;
	for (i = j; i < openmax; i += (errno != EINTR)) {
	    errno = 0;
	    if (fcntl(i, F_GETFL, 0) < 0 &&
		(errno == EBADF || errno == EINTR))
		continue;
	    j = i;
	}
	openmax = j;
    }

    return (max_zsh_fd > openmax) ? max_zsh_fd : openmax;
}

/**/
#endif

/*
 * Rationalise the current directory, returning the string.
 *
 * If "d" is not NULL, it is used to store information about the
 * directory.  The returned name is also present in d->dirname and is in
 * permanently allocated memory.  The handling of this case depends on
 * whether the fchdir() system call is available; if it is, it is assumed
 * the caller is able to restore the current directory.  On successfully
 * identifying the directory the function returns immediately rather
 * than ascending the hierarchy.
 *
 * If "d" is NULL, no assumption about the caller's behaviour is
 * made.  The returned string is in heap memory.  This case is
 * always handled by changing directory up the hierarchy.
 *
 * On Cygwin or other systems where USE_GETCWD is defined (at the
 * time of writing only QNX), we skip all the above and use the
 * getcwd() system call.
 */

/**/
mod_export char *
zgetdir(struct dirsav *d)
{
    char nbuf[PATH_MAX+3];
    char *buf;
    int bufsiz, pos;
    struct stat sbuf;
    ino_t pino;
    dev_t pdev;
#if !defined(__CYGWIN__) && !defined(USE_GETCWD)
    struct dirent *de;
    DIR *dir;
    dev_t dev;
    ino_t ino;
    int len;
#endif

    buf = zhalloc(bufsiz = PATH_MAX+1);
    pos = bufsiz - 1;
    buf[pos] = '\0';
    strcpy(nbuf, "../");
    if (stat(".", &sbuf) < 0) {
	return NULL;
    }

    /* Record the initial inode and device */
    pino = sbuf.st_ino;
    pdev = sbuf.st_dev;
    if (d)
	d->ino = pino, d->dev = pdev;
#if !defined(__CYGWIN__) && !defined(USE_GETCWD)
#ifdef HAVE_FCHDIR
    else
#endif
	holdintr();

    for (;;) {
	/* Examine the parent of the current directory. */
	if (stat("..", &sbuf) < 0)
	    break;

	/* Inode and device of curtent directory */
	ino = pino;
	dev = pdev;
	/* Inode and device of current directory's parent */
	pino = sbuf.st_ino;
	pdev = sbuf.st_dev;

	/* If they're the same, we've reached the root directory... */
	if (ino == pino && dev == pdev) {
	    /*
	     * ...well, probably.  If this was an orphaned . after
	     * an unmount, or something such, we could be in trouble...
	     */
	    if (stat("/", &sbuf) < 0 ||
		sbuf.st_ino != ino ||
		sbuf.st_dev != dev) {
		zerr("Failed to get current directory: path invalid");
		return NULL;
	    }
	    if (!buf[pos])
		buf[--pos] = '/';
	    if (d) {
#ifndef HAVE_FCHDIR
		zchdir(buf + pos);
		noholdintr();
#endif
		return d->dirname = ztrdup(buf + pos);
	    }
	    zchdir(buf + pos);
	    noholdintr();
	    return buf + pos;
	}

	/* Search the parent for the current directory. */
	if (!(dir = opendir("..")))
	    break;

	while ((de = readdir(dir))) {
	    char *fn = de->d_name;
	    /* Ignore `.' and `..'. */
	    if (fn[0] == '.' &&
		(fn[1] == '\0' ||
		 (fn[1] == '.' && fn[2] == '\0')))
		continue;
#ifdef HAVE_STRUCT_DIRENT_D_STAT
	    if(de->d_stat.st_dev == dev && de->d_stat.st_ino == ino) {
		/* Found the directory we're currently in */
		strncpy(nbuf + 3, fn, PATH_MAX);
		break;
	    }
#else /* !HAVE_STRUCT_DIRENT_D_STAT */
# ifdef HAVE_STRUCT_DIRENT_D_INO
	    if (dev != pdev || (ino_t) de->d_ino == ino)
# endif /* HAVE_STRUCT_DIRENT_D_INO */
	    {
		/* Maybe found directory, need to check device & inode */
		strncpy(nbuf + 3, fn, PATH_MAX);
		lstat(nbuf, &sbuf);
		if (sbuf.st_dev == dev && sbuf.st_ino == ino)
		    break;
	    }
#endif /* !HAVE_STRUCT_DIRENT_D_STAT */
	}
	closedir(dir);
	if (!de)
	    break;		/* Not found */
	/*
	 * We get the "/" free just by copying from nbuf+2 instead
	 * of nbuf+3, which is where we copied the path component.
	 * This means buf[pos] is always a "/".
	 */
	len = strlen(nbuf + 2);
	pos -= len;
	while (pos <= 1) {
	    char *newbuf = zhalloc(2*bufsiz);
	    memcpy(newbuf + bufsiz, buf, bufsiz);
	    buf = newbuf;
	    pos += bufsiz;
	    bufsiz *= 2;
	}
	memcpy(buf + pos, nbuf + 2, len);
#ifdef HAVE_FCHDIR
	if (d)
	    return d->dirname = ztrdup(buf + pos + 1);
#endif
	if (chdir(".."))
	    break;
    }

    /*
     * Fix up the directory, if necessary.
     * We're changing back down the hierarchy, ignore the
     * "/" at buf[pos].
     */
    if (d) {
#ifndef HAVE_FCHDIR
	if (buf[pos])
	    zchdir(buf + pos + 1);
	noholdintr();
#endif
	return NULL;
    }

    if (buf[pos])
	zchdir(buf + pos + 1);
    noholdintr();

#else  /* __CYGWIN__, USE_GETCWD cases */

    if (!getcwd(buf, bufsiz)) {
	if (d) {
	    return NULL;
	}
    } else {
	if (d) {
	    return d->dirname = ztrdup(buf);
	}
	return buf;
    }
#endif

    /*
     * Something bad happened.
     * This has been seen when inside a special directory,
     * such as the Netapp .snapshot directory, that doesn't
     * appear as a directory entry in the parent directory.
     * We'll just need our best guess.
     *
     * We only get here from zgetcwd(); let that fall back to pwd.
     */

    return NULL;
}

/*
 * Try to find the current directory.
 * If we couldn't work it out internally, fall back to getcwd().
 * If it fails, fall back to pwd; if zgetcwd() is being used
 * to set pwd, pwd should be NULL and we just return ".".
 */

/**/
mod_export char *
zgetcwd(void)
{
    char *ret = zgetdir(NULL);
#ifdef HAVE_GETCWD
    if (!ret) {
#ifdef GETCWD_CALLS_MALLOC
	char *cwd = getcwd(NULL, 0);
	if (cwd) {
	    ret = dupstring(cwd);
	    free(cwd);
	}
#else
	char *cwdbuf = zalloc(PATH_MAX+1);
	ret = getcwd(cwdbuf, PATH_MAX);
	if (ret)
	    ret = dupstring(ret);
	zfree(cwdbuf, PATH_MAX+1);
#endif /* GETCWD_CALLS_MALLOC */
    }
#endif /* HAVE_GETCWD */
    if (!ret)
	ret = unmeta(pwd);
    if (!ret || *ret == '\0')
	ret = dupstring(".");
    return ret;
}

/*
 * chdir with arbitrary long pathname.  Returns 0 on success, -1 on normal *
 * failure and -2 when chdir failed and the current directory is lost.
 *
 * This is to be treated as if at system level, so dir is unmetafied but
 * terminated by a NULL.
 */

/**/
mod_export int
zchdir(char *dir)
{
    char *s;
    int currdir = -2;

    for (;;) {
	if (!*dir || chdir(dir) == 0) {
#ifdef HAVE_FCHDIR
           if (currdir >= 0)
               close(currdir);
#endif
	    return 0;
	}
	if ((errno != ENAMETOOLONG && errno != ENOMEM) ||
	    strlen(dir) < PATH_MAX)
	    break;
	for (s = dir + PATH_MAX - 1; s > dir && *s != '/'; s--)
	    ;
	if (s == dir)
	    break;
#ifdef HAVE_FCHDIR
	if (currdir == -2)
	    currdir = open(".", O_RDONLY|O_NOCTTY);
#endif
	*s = '\0';
	if (chdir(dir) < 0) {
	    *s = '/';
	    break;
	}
#ifndef HAVE_FCHDIR
	currdir = -1;
#endif
	*s = '/';
	while (*++s == '/')
	    ;
	dir = s;
    }
#ifdef HAVE_FCHDIR
    if (currdir >= 0) {
	if (fchdir(currdir) < 0) {
	    close(currdir);
	    return -2;
	}
	close(currdir);
	return -1;
    }
#endif
    return currdir == -2 ? -1 : -2;
}

/*
 * How to print out a 64 bit integer.  This isn't needed (1) if longs
 * are 64 bit, since ordinary %ld will work (2) if we couldn't find a
 * 64 bit type anyway.
 */
/**/
#ifdef ZSH_64_BIT_TYPE
/**/
mod_export char *
output64(zlong val)
{
    static char llbuf[DIGBUFSIZE];
    convbase(llbuf, val, 0);
    return llbuf;
}
/**/
#endif /* ZSH_64_BIT_TYPE */

/**/
#ifndef HAVE_STRTOUL

/*
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * Convert a string to an unsigned long integer.
 *
 * Ignores `locale' stuff.  Assumes that the upper and lower case
 * alphabets and digits are each contiguous.
 */

/**/
unsigned long
strtoul(nptr, endptr, base)
	const char *nptr;
	char **endptr;
	int base;
{
	const char *s;
	unsigned long acc, cutoff;
	int c;
	int neg, any, cutlim;

	/* endptr may be NULL */

	s = nptr;
	do {
		c = (unsigned char) *s++;
	} while (isspace(c));
	if (c == '-') {
		neg = 1;
		c = *s++;
	} else {
		neg = 0;
		if (c == '+')
			c = *s++;
	}
	if ((base == 0 || base == 16) &&
	    c == '0' && (*s == 'x' || *s == 'X')) {
		c = s[1];
		s += 2;
		base = 16;
	}
	if (base == 0)
		base = c == '0' ? 8 : 10;

	cutoff = ULONG_MAX / (unsigned long)base;
	cutlim = (int)(ULONG_MAX % (unsigned long)base);
	for (acc = 0, any = 0;; c = (unsigned char) *s++) {
		if (isdigit(c))
			c -= '0';
		else if (isalpha(c)) {
			c -= isupper(c) ? 'A' - 10 : 'a' - 10;
		} else
			break;
		if (c >= base)
			break;
		if (any < 0)
			continue;
		if (acc > cutoff || (acc == cutoff && c > cutlim)) {
			any = -1;
			acc = ULONG_MAX;
			errno = ERANGE;
		} else {
			any = 1;
			acc *= (unsigned long)base;
			acc += c;
		}
	}
	if (neg && any > 0)
		acc = -acc;
	if (endptr != NULL)
		*endptr = any ? s - 1 : nptr;
	return (acc);
}

/**/
#endif /* HAVE_STRTOUL */

/**/
#ifdef ENABLE_UNICODE9
#include "./wcwidth9.h"

/**/
int
u9_wcwidth(wchar_t ucs)
{
  int w = wcwidth9(ucs);
  if (w < -1)
    return 1;
  return w;
}

/**/
int
u9_iswprint(wint_t ucs)
{
    if (ucs == 0)
	return 0;
    return wcwidth9(ucs) != -1;
}

/**/
#endif	/* ENABLE_UNICODE9 */

/**/
#if defined(__APPLE__) && defined(BROKEN_ISPRINT)

/**/
int
isprint_ascii(int c)
{
    if (!strcmp(nl_langinfo(CODESET), "UTF-8"))
	return (c >= 0x20 && c <= 0x7e);
    else
	return isprint(c);
}

/**/
#endif /* __APPLE__ && BROKEN_ISPRINT */
