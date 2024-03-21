/*
 * sysread.c - interface to system read/write
 *
 * This file is part of zsh, the Z shell.
 *
 * Copyright (c) 1998-2003 Peter Stephenson
 * All rights reserved.
 *
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and to distribute modified versions of this software for any
 * purpose, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 *
 * In no event shall Peter Stephenson or the Zsh Development
 * Group be liable to any party for direct, indirect, special, incidental,
 * or consequential damages arising out of the use of this software and
 * its documentation, even if Peter Stephenson, and the Zsh
 * Development Group have been advised of the possibility of such damage.
 *
 * Peter Stephenson and the Zsh Development Group specifically
 * disclaim any warranties, including, but not limited to, the implied
 * warranties of merchantability and fitness for a particular purpose.  The
 * software provided hereunder is on an "as is" basis, and Peter Stephenson
 * and the Zsh Development Group have no obligation to provide maintenance,
 * support, updates, enhancements, or modifications.
 *
 */

#include "system.mdh"
#include "system.pro"
#include <math.h>

#ifdef HAVE_POLL_H
# include <poll.h>
#endif
#if defined(HAVE_POLL) && !defined(POLLIN)
# undef HAVE_POLL
#endif

#define SYSREAD_BUFSIZE	8192

/**/
static int
getposint(char *instr, char *nam)
{
    char *eptr;
    int ret;

    ret = (int)zstrtol(instr, &eptr, 10);
    if (*eptr || ret < 0) {
	zwarnnam(nam, "integer expected: %s", instr);
	return -1;
    }

    return ret;
}


/*
 * Return values of bin_sysread:
 *	0	Successfully read (and written if appropriate)
 *	1	Error in parameters to command
 *	2	Error on read, or polling read fd ) ERRNO set by
 *      3	Error on write			  ) system
 *	4	Timeout on read
 *	5       Zero bytes read, end of file
 */

/**/
static int
bin_sysread(char *nam, char **args, Options ops, UNUSED(int func))
{
    int infd = 0, outfd = -1, bufsize = SYSREAD_BUFSIZE, count;
    char *outvar = NULL, *countvar = NULL, *inbuf;

    /* -i: input file descriptor if not stdin */
    if (OPT_ISSET(ops, 'i')) {
	infd = getposint(OPT_ARG(ops, 'i'), nam);
	if (infd < 0)
	    return 1;
    }

    /* -o: output file descriptor, else store in REPLY */
    if (OPT_ISSET(ops, 'o')) {
	outfd = getposint(OPT_ARG(ops, 'o'), nam);
	if (outfd < 0)
	    return 1;
    }

    /* -s: buffer size if not default SYSREAD_BUFSIZE */
    if (OPT_ISSET(ops, 's')) {
	bufsize = getposint(OPT_ARG(ops, 's'), nam);
	if (bufsize < 0)
	    return 1;
    }

    /* -c: name of variable to store count of transferred bytes */
    if (OPT_ISSET(ops, 'c')) {
	countvar = OPT_ARG(ops, 'c');
	if (!isident(countvar)) {
	    zwarnnam(nam, "not an identifier: %s", countvar);
	    return 1;
	}
    }

    if (*args) {
	/*
	 * Variable in which to store result if doing a plain read.
	 * Default variable if not specified is REPLY.
	 * If writing, only stuff we couldn't write is stored here,
	 * no default in that case (we just discard it if no variable).
	 */
	outvar = *args;
	if (!isident(outvar)) {
	    zwarnnam(nam, "not an identifier: %s", outvar);
	    return 1;
	}
    }

    inbuf = zhalloc(bufsize);

#if defined(HAVE_POLL) || defined(HAVE_SELECT)
    /* -t: timeout */
    if (OPT_ISSET(ops, 't'))
    {
# ifdef HAVE_POLL
	struct pollfd poll_fd;
	mnumber to_mn;
	int to_int, ret;

	poll_fd.fd = infd;
	poll_fd.events = POLLIN;

	to_mn = matheval(OPT_ARG(ops, 't'));
	if (errflag)
	    return 1;
	if (to_mn.type == MN_FLOAT)
	    to_int = (int) (1000 * to_mn.u.d);
	else
	    to_int = 1000 * (int)to_mn.u.l;

	while ((ret = poll(&poll_fd, 1, to_int)) < 0) {
	    if (errno != EINTR || errflag || retflag || breaks || contflag)
		break;
	}
	if (ret <= 0) {
	    /* treat non-timeout error as error on read */
	    return ret ? 2 : 4;
	}
# else
	/* using select */
	struct timeval select_tv;
	fd_set fds;
	mnumber to_mn;
	int ret;

	FD_ZERO(&fds);
	FD_SET(infd, &fds);
	to_mn = matheval(OPT_ARG(ops, 't'));
	if (errflag)
	    return 1;

	if (to_mn.type == MN_FLOAT) {
	    select_tv.tv_sec = (int) to_mn.u.d;
	    select_tv.tv_usec =
		(int) ((to_mn.u.d - select_tv.tv_sec) * 1e6);
	} else {
	    select_tv.tv_sec = (int) to_mn.u.l;
	    select_tv.tv_usec = 0;
	}

	while ((ret = select(infd+1, (SELECT_ARG_2_T) &fds,
			     NULL, NULL,&select_tv)) < 0) {
	    if (errno != EINTR || errflag || retflag || breaks || contflag)
		break;
	}
	if (ret <= 0) {
	    /* treat non-timeout error as error on read */
	    return ret ? 2 : 4;
	}
# endif
    }
#endif

    while ((count = read(infd, inbuf, bufsize)) < 0) {
	if (errno != EINTR || errflag || retflag || breaks || contflag)
	    break;
    }
    if (countvar)
	setiparam(countvar, count);
    if (count < 0)
	return 2;

    if (outfd >= 0) {
	if (!count)
	    return 5;
	while (count > 0) {
	    int ret;

	    ret = write(outfd, inbuf, count);
	    if (ret < 0) {
		if (errno == EINTR && !errflag &&
		    !retflag && !breaks && !contflag)
		    continue;
		if (outvar)
		    setsparam(outvar, metafy(inbuf, count, META_DUP));
		if (countvar)
		    setiparam(countvar, count);
		return 3;
	    }
	    inbuf += ret;
	    count -= ret;
	}
	return 0;
    }

    if (!outvar)
	    outvar = "REPLY";
    /* do this even if we read zero bytes */
    setsparam(outvar, metafy(inbuf, count, META_DUP));

    return count ? 0 : 5;
}


/*
 * Return values of bin_syswrite:
 *	0	Successfully written
 *	1	Error in parameters to command
 *	2	Error on write, ERRNO set by system
 */

/**/
static int
bin_syswrite(char *nam, char **args, Options ops, UNUSED(int func))
{
    int outfd = 1, len, count, totcount;
    char *countvar = NULL;

    /* -o: output file descriptor if not stdout */
    if (OPT_ISSET(ops, 'o')) {
	outfd = getposint(OPT_ARG(ops, 'o'), nam);
	if (outfd < 0)
	    return 1;
    }

    /* -c: variable in which to store count of bytes written */
    if (OPT_ISSET(ops, 'c')) {
	countvar = OPT_ARG(ops, 'c');
	if (!isident(countvar)) {
	    zwarnnam(nam, "not an identifier: %s", countvar);
	    return 1;
	}
    }

    totcount = 0;
    unmetafy(*args, &len);
    while (len) {
	while ((count = write(outfd, *args, len)) < 0) {
	    if (errno != EINTR || errflag || retflag || breaks || contflag)
	    {
		if (countvar)
		    setiparam(countvar, totcount);
		return 2;
	    }
	}
	*args += count;
	totcount += count;
	len -= count;
    }
    if (countvar)
	setiparam(countvar, totcount);

    return 0;
}


static struct { const char *name; int oflag; } openopts[] = {
#ifdef O_CLOEXEC
    { "cloexec", O_CLOEXEC },
#else
# ifdef FD_CLOEXEC
    { "cloexec", 0  }, /* this needs to be first in the table */
# endif
#endif
#ifdef O_NOFOLLOW
    { "nofollow", O_NOFOLLOW },
#endif
#ifdef O_SYNC
    { "sync", O_SYNC },
#endif
#ifdef O_NOATIME
    { "noatime", O_NOATIME },
#endif
#ifdef O_NONBLOCK
    { "nonblock", O_NONBLOCK},
#endif
    { "excl", O_EXCL | O_CREAT },
    { "creat", O_CREAT },
    { "create", O_CREAT },
    { "truncate", O_TRUNC },
    { "trunc", O_TRUNC }
};

/**/
static int
bin_sysopen(char *nam, char **args, Options ops, UNUSED(int func))
{
    int read = OPT_ISSET(ops, 'r');
    int write = OPT_ISSET(ops, 'w');
    int append = OPT_ISSET(ops, 'a') ? O_APPEND : 0;
    int flags = O_NOCTTY | append | ((append || write) ?
	(read ? O_RDWR : O_WRONLY) : O_RDONLY);
    char *opt, *ptr, *nextopt, *fdvar;
    int o, fd, moved_fd, explicit = -1;
    mode_t perms = 0666;
#if defined(FD_CLOEXEC) && !defined(O_CLOEXEC)
    int fdflags = 0;
#endif

    if (!OPT_ISSET(ops, 'u')) {
	zwarnnam(nam, "file descriptor not specified");
	return 1;
    }

    /* file descriptor, either 0-9 or a variable name */
    fdvar = OPT_ARG(ops, 'u');
    if (idigit(*fdvar) && !fdvar[1]) {
	explicit = atoi(fdvar);
    } else if (!isident(fdvar)) {
	zwarnnam(nam, "not an identifier: %s", fdvar);
	return 1;
    }

    /* open options */
    if (OPT_ISSET(ops, 'o')) {
	opt = OPT_ARG(ops, 'o');
	while (opt) {
	    if (!strncasecmp(opt, "O_", 2)) /* ignore initial O_ */
		opt += 2;
	    if ((nextopt = strchr(opt, ',')))
		*nextopt++ = '\0';
	    for (o = sizeof(openopts)/sizeof(*openopts) - 1; o >= 0 &&
		strcasecmp(openopts[o].name, opt); o--) {}
	    if (o < 0) {
		zwarnnam(nam, "unsupported option: %s\n", opt);
		return 1;
	    }
#if defined(FD_CLOEXEC) && !defined(O_CLOEXEC)
	    if (!openopts[o].oflag)
		fdflags = FD_CLOEXEC;
#endif
	    flags |= openopts[o].oflag;
	    opt = nextopt;
	}
    }

    /* -m: permissions or mode for created files */
    if (OPT_ISSET(ops, 'm')) {
	ptr = opt = OPT_ARG(ops, 'm');
	while (*ptr >= '0' && *ptr <= '7') ptr++;
	if (*ptr || ptr - opt < 3) {
	    zwarnnam(nam, "invalid mode %s", opt);
	    return 1;
	}
	perms = zstrtol(opt, 0, 8); /* octal number */
    }

    if (flags & O_CREAT)
	fd = open(*args, flags, perms);
    else
	fd = open(*args, flags);

    if (fd == -1) {
	zwarnnam(nam, "can't open file %s: %e", *args, errno);
	return 1;
    }
    moved_fd = (explicit > -1) ? redup(fd, explicit) : movefd(fd);
    if (moved_fd == -1) {
	zwarnnam(nam, "can't open file %s", *args);
	return 1;
    }

#ifdef FD_CLOEXEC
#ifdef O_CLOEXEC
    /*
     * the O_CLOEXEC is a flag attached to the *file descriptor*, not the
     * *open file description* so it doesn't survive a dup(). If that flag was
     * requested and the fd was moved, we need to reapply it to the moved fd
     * even if the original one was open with O_CLOEXEC
     */
    if ((flags & O_CLOEXEC) && fd != moved_fd)
#else
    if (fdflags)
#endif /* O_CLOEXEC */
	fcntl(moved_fd, F_SETFD, FD_CLOEXEC);
#endif /* FD_CLOEXEC */
    fdtable[moved_fd] = FDT_EXTERNAL;
    if (explicit == -1) {
	setiparam(fdvar, moved_fd);
	/* if setting the variable failed, close moved_fd to avoid leak */
	if (errflag)
	    zclose(moved_fd);
    }

    return 0;
}


/*
 * Return values of bin_sysseek:
 *	0	Success
 *	1	Error in parameters to command
 *	2	Error on seek, ERRNO set by system
 */

/**/
static int
bin_sysseek(char *nam, char **args, Options ops, UNUSED(int func))
{
    int w = SEEK_SET, fd = 0;
    char *whence;
    off_t pos;

    /* -u:  file descriptor if not stdin */
    if (OPT_ISSET(ops, 'u')) {
	fd = getposint(OPT_ARG(ops, 'u'), nam);
	if (fd < 0)
	    return 1;
    }

    /* -w:  whence - starting point of seek */
    if (OPT_ISSET(ops, 'w')) {
	whence = OPT_ARG(ops, 'w');
        if (!(strcasecmp(whence, "current") && strcmp(whence, "1")))
	    w = SEEK_CUR;
        else if (!(strcasecmp(whence, "end") && strcmp(whence, "2")))
	    w = SEEK_END;
	else if (strcasecmp(whence, "start") && strcmp(whence, "0")) {
	    zwarnnam(nam, "unknown argument to -w: %s", whence);
	    return 1;
	}
    }

    pos = (off_t)mathevali(*args);
    return (lseek(fd, pos, w) == -1) ? 2 : 0;
}

/**/
static mnumber
math_systell(UNUSED(char *name), UNUSED(int argc), mnumber *argv, UNUSED(int id))
{
    int fd = (argv->type == MN_INTEGER) ? argv->u.l : (int)argv->u.d;
    mnumber ret;
    ret.type = MN_INTEGER;
    ret.u.l = 0;

    if (fd < 0) {
	zerr("file descriptor out of range");
	return ret;
    }
    ret.u.l = lseek(fd, 0, SEEK_CUR);
    return ret;
}


/*
 * Return values of bin_syserror:
 *	0	Successfully processed error
 *		(although if the number was invalid the string
 *		may not be useful)
 *	1	Error in parameters
 *	2	Name of error not recognised.
 */

/**/
static int
bin_syserror(char *nam, char **args, Options ops, UNUSED(int func))
{
    int num = 0;
    char *errvar = NULL, *msg, *pfx = "", *str;

    /* variable in which to write error message */
    if (OPT_ISSET(ops, 'e')) {
	errvar = OPT_ARG(ops, 'e');
	if (!isident(errvar)) {
	    zwarnnam(nam, "not an identifier: %s", errvar);
	    return 1;
	}
    }
    /* prefix for error message */
    if (OPT_ISSET(ops, 'p'))
	pfx = OPT_ARG(ops, 'p');

    if (!*args)
	num = errno;
    else {
	char *ptr = *args;
	while (*ptr && idigit(*ptr))
	    ptr++;
	if (!*ptr && ptr > *args)
	    num = atoi(*args);
	else {
	    const char **eptr;
	    for (eptr = sys_errnames; *eptr; eptr++) {
		if (!strcmp(*eptr, *args)) {
		    num = (eptr - sys_errnames) + 1;
		    break;
		}
	    }
	    if (!*eptr)
		return 2;
	}
    }

    msg = strerror(num);
    if (errvar) {
	str = (char *)zalloc(strlen(msg) + strlen(pfx) + 1);
	sprintf(str, "%s%s", pfx, msg);
	setsparam(errvar, str);
    } else {
	fprintf(stderr, "%s%s\n", pfx, msg);
    }

    return 0;
}

/**/
static int
bin_zsystem_flock(char *nam, char **args, UNUSED(Options ops), UNUSED(int func))
{
    int cloexec = 1, unlock = 0, readlock = 0;
    double timeout = -1;
    long timeout_interval = 1e6;
    mnumber timeout_param;
    char *fdvar = NULL;
#ifdef HAVE_FCNTL_H
    struct flock lck;
    int flock_fd, flags;
#endif

    while (*args && **args == '-') {
	int opt;
	char *optptr = *args + 1, *optarg;
	args++;
	if (!*optptr || !strcmp(optptr, "-"))
	    break;
	while ((opt = *optptr)) {
	    switch (opt) {
	    case 'e':
		/* keep lock on "exec" */
		cloexec = 0;
		break;

	    case 'f':
		/* variable for fd */
		if (optptr[1]) {
		    fdvar = optptr + 1;
		    optptr += strlen(fdvar) - 1;
		} else if (*args) {
		    fdvar = *args++;
		}
		if (fdvar == NULL || !isident(fdvar)) {
		    zwarnnam(nam, "flock: option %c requires a variable name",
			     opt);
		    return 1;
		}
		break;

	    case 'r':
		/* read lock rather than read-write lock */
		readlock = 1;
		break;

	    case 't':
		/* timeout in seconds */
		if (optptr[1]) {
		    optarg = optptr + 1;
		    optptr += strlen(optarg) - 1;
		} else if (!*args) {
		    zwarnnam(nam, "flock: option %c requires a numeric timeout",
			     opt);
		    return 1;
		} else {
		    optarg = *args++;
		}
		timeout_param = matheval(optarg);
		timeout = (timeout_param.type & MN_FLOAT) ?
		    timeout_param.u.d : (double)timeout_param.u.l;

		/*
		 * timeout must not overflow time_t, but little is known
		 * about this type's limits.  Conservatively limit to 2^30-1
		 * (34 years).  Then it can only overflow if time_t is only
		 * a 32-bit int and CLOCK_MONOTONIC is not supported, in which
		 * case there is a Y2038 problem anyway.
		 */
		if (timeout > 1073741823.) {
		    zwarnnam(nam, "flock: invalid timeout value: '%s'",
			     optarg);
		    return 1;
		}
		break;

	    case 'i':
		/* retry interval in seconds */
		if (optptr[1]) {
		    optarg = optptr + 1;
		    optptr += strlen(optarg) - 1;
		} else if (!*args) {
		    zwarnnam(nam,
			     "flock: option %c requires "
			     "a numeric retry interval",
			     opt);
		    return 1;
		} else {
		    optarg = *args++;
		}
		timeout_param = matheval(optarg);
		if (!(timeout_param.type & MN_FLOAT)) {
		    timeout_param.type = MN_FLOAT;
		    timeout_param.u.d = (double)timeout_param.u.l;
		}
		timeout_param.u.d = ceil(timeout_param.u.d * 1e6);
		if (timeout_param.u.d < 1
		    || timeout_param.u.d > 0.999 * LONG_MAX) {
		    zwarnnam(nam, "flock: invalid interval value: '%s'",
			     optarg);
		    return 1;
		}
		timeout_interval = (long)timeout_param.u.d;
		break;

	    case 'u':
		/* unlock: argument is fd */
		unlock = 1;
		break;

	    default:
		zwarnnam(nam, "flock: unknown option: %c", *optptr);
		return 1;
	    }
	    optptr++;
	}
    }


    if (!args[0]) {
	zwarnnam(nam, "flock: not enough arguments");
	return 1;
    }
    if (args[1]) {
	zwarnnam(nam, "flock: too many arguments");
	return 1;
    }

#ifdef HAVE_FCNTL_H
    if (unlock) {
	flock_fd = (int)mathevali(args[0]);
	if (zcloselockfd(flock_fd) < 0) {
	    zwarnnam(nam, "flock: file descriptor %d not in use for locking",
		     flock_fd);
	    return 1;
	}
	return 0;
    }

    if (readlock)
	flags = O_RDONLY | O_NOCTTY;
    else
	flags = O_RDWR | O_NOCTTY;
    if ((flock_fd = open(unmeta(args[0]), flags)) < 0) {
	zwarnnam(nam, "failed to open %s for writing: %e", args[0], errno);
	return 1;
    }
    flock_fd = movefd(flock_fd);
    if (flock_fd == -1)
	return 1;
#ifdef FD_CLOEXEC
    if (cloexec)
    {
	long fdflags = fcntl(flock_fd, F_GETFD, 0);
	if (fdflags != (long)-1)
	    fcntl(flock_fd, F_SETFD, fdflags | FD_CLOEXEC);
    }
#endif
    addlockfd(flock_fd, cloexec);

    lck.l_type = readlock ? F_RDLCK : F_WRLCK;
    lck.l_whence = SEEK_SET;
    lck.l_start = 0;
    lck.l_len = 0;  /* lock the whole file */

    if (timeout > 0) {
	/*
	 * Get current time, calculate timeout time.
	 * No need to check for overflow, already checked above.
	 */
	struct timespec now, end;
	double timeout_s;
	long remaining_us;
	zgettime_monotonic_if_available(&now);
	end.tv_sec = now.tv_sec;
	end.tv_nsec = now.tv_nsec;
	end.tv_nsec += ceil(modf(timeout, &timeout_s) * 1000000000L);
	end.tv_sec += timeout_s;
	if (end.tv_nsec >= 1000000000L) {
	    end.tv_nsec -= 1000000000L;
	    end.tv_sec += 1;
	}

	/* Try acquiring lock, loop until timeout. */
	while (fcntl(flock_fd, F_SETLK, &lck) < 0) {
	    if (errflag) {
                zclose(flock_fd);
		return 1;
            }
	    if (errno != EINTR && errno != EACCES && errno != EAGAIN) {
                zclose(flock_fd);
		zwarnnam(nam, "failed to lock file %s: %e", args[0], errno);
		return 1;
	    }
	    zgettime_monotonic_if_available(&now);
	    remaining_us = timespec_diff_us(&now, &end);
	    if (remaining_us <= 0) {
                zclose(flock_fd);
		return 2;
            }
	    if (remaining_us <= timeout_interval) {
		timeout_interval = remaining_us;
	    }
	    zsleep(timeout_interval);
	}
    } else {
	while (fcntl(flock_fd, timeout == 0 ? F_SETLK : F_SETLKW, &lck) < 0) {
	    if (errflag) {
                zclose(flock_fd);
		return 1;
            }
	    if (errno == EINTR)
		continue;
            zclose(flock_fd);
	    zwarnnam(nam, "failed to lock file %s: %e", args[0], errno);
	    return 1;
	}
    }

    if (fdvar)
	setiparam(fdvar, flock_fd);

    return 0;
#else /* HAVE_FCNTL_H */
    zwarnnam(nam, "flock: not implemented on this system");
    return 255;
#endif /* HAVE_FCNTL_H */
}


/*
 * Return status zero if the zsystem feature is supported, else 1.
 * Operates silently for future-proofing.
 */
/**/
static int
bin_zsystem_supports(char *nam, char **args,
		     UNUSED(Options ops), UNUSED(int func))
{
    if (!args[0]) {
	zwarnnam(nam, "supports: not enough arguments");
	return 255;
    }
    if (args[1]) {
	zwarnnam(nam, "supports: too many arguments");
	return 255;
    }

    /* stupid but logically this should work... */
    if (!strcmp(*args, "supports"))
	return 0;
#ifdef HAVE_FCNTL_H
    if (!strcmp(*args, "flock"))
	return 0;
#endif
    return 1;
}


/**/
static int
bin_zsystem(char *nam, char **args, Options ops, int func)
{
    /* If more commands are implemented, this can be more sophisticated */
    if (!strcmp(*args, "flock")) {
	return bin_zsystem_flock(nam, args+1, ops, func);
    } else if (!strcmp(*args, "supports")) {
	return bin_zsystem_supports(nam, args+1, ops, func);
    }
    zwarnnam(nam, "unknown subcommand: %s", *args);
    return 1;
}

static struct builtin bintab[] = {
    BUILTIN("syserror", 0, bin_syserror, 0, 1, 0, "e:p:", NULL),
    BUILTIN("sysread", 0, bin_sysread, 0, 1, 0, "c:i:o:s:t:", NULL),
    BUILTIN("syswrite", 0, bin_syswrite, 1, 1, 0, "c:o:", NULL),
    BUILTIN("sysopen", 0, bin_sysopen, 1, 1, 0, "rwau:o:m:", NULL),
    BUILTIN("sysseek", 0, bin_sysseek, 1, 1, 0, "u:w:", NULL),
    BUILTIN("zsystem", 0, bin_zsystem, 1, -1, 0, NULL, NULL)
};


/* Functions for the errnos special parameter. */

/**/
static char **
errnosgetfn(UNUSED(Param pm))
{
    /* arrdup etc. should really take const pointers as arguments */
    return arrdup((char **)sys_errnames);
}

static const struct gsu_array errnos_gsu =
{ errnosgetfn, arrsetfn, stdunsetfn };


/* Functions for the sysparams special parameter. */

/**/
static void
fillpmsysparams(Param pm, const char *name)
{
    char buf[DIGBUFSIZE];
    int num;

    pm->node.nam = dupstring(name);
    pm->node.flags = PM_SCALAR | PM_READONLY;
    pm->gsu.s = &nullsetscalar_gsu;
    if (!strcmp(name, "pid")) {
	num = (int)getpid();
    } else if (!strcmp(name, "ppid")) {
	num = (int)getppid();
    } else if (!strcmp(name, "procsubstpid")) {
	num = (int)procsubstpid;
    } else {
	pm->u.str = dupstring("");
	pm->node.flags |= PM_UNSET;
	return;
    }

    sprintf(buf, "%d", num);
    pm->u.str = dupstring(buf);
}


/**/
static HashNode
getpmsysparams(UNUSED(HashTable ht), const char *name)
{
    Param pm;

    pm = (Param) hcalloc(sizeof(struct param));
    fillpmsysparams(pm, name);
    return &pm->node;
}


/**/
static void
scanpmsysparams(UNUSED(HashTable ht), ScanFunc func, int flags)
{
    struct param spm;

    fillpmsysparams(&spm, "pid");
    func(&spm.node, flags);
    fillpmsysparams(&spm, "ppid");
    func(&spm.node, flags);
    fillpmsysparams(&spm, "procsubstpid");
    func(&spm.node, flags);
}

static struct mathfunc mftab[] = {
    NUMMATHFUNC("systell", math_systell, 1, 1, 0)
};

static struct paramdef partab[] = {
    SPECIALPMDEF("errnos", PM_ARRAY|PM_READONLY,
		 &errnos_gsu, NULL, NULL),
    SPECIALPMDEF("sysparams", PM_READONLY,
		 NULL, getpmsysparams, scanpmsysparams)
};

static struct features module_features = {
    bintab, sizeof(bintab)/sizeof(*bintab),
    NULL, 0,
    mftab, sizeof(mftab)/sizeof(*mftab),
    partab, sizeof(partab)/sizeof(*partab),
    0
};

/* The load/unload routines required by the zsh library interface */

/**/
int
setup_(UNUSED(Module m))
{
    return 0;
}

/**/
int
features_(Module m, char ***features)
{
    *features = featuresarray(m, &module_features);
    return 0;
}

/**/
int
enables_(Module m, int **enables)
{
    return handlefeatures(m, &module_features, enables);
}

/**/
int
boot_(UNUSED(Module m))
{
    return 0;
}


/**/
int
cleanup_(Module m)
{
    return setfeatureenables(m, &module_features, NULL);
}

/**/
int
finish_(UNUSED(Module m))
{
    return 0;
}
