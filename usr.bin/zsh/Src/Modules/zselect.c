/*
 * zselect.c - builtin support for select system call
 *
 * This file is part of zsh, the Z shell.
 *
 * Copyright (c) 1998-2001 Peter Stephenson
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

#include "zselect.mdh"
#include "zselect.pro"

/* Helper functions */

/*
 * Handle an fd by adding it to the current fd_set.
 * Return 1 for error (after printing a message), 0 for OK.
 */
static int
handle_digits(char *nam, char *argptr, fd_set *fdset, int *fdmax)
{
    int fd;
    char *endptr;

    if (!idigit(*argptr)) {
	zwarnnam(nam, "expecting file descriptor: %s", argptr);
	return 1;
    }
    fd = (int)zstrtol(argptr, &endptr, 10);
    if (*endptr) {
	zwarnnam(nam, "garbage after file descriptor: %s", endptr);
	return 1;
    }

    FD_SET(fd, fdset);
    if (fd+1 > *fdmax)
	*fdmax = fd+1;
    return 0;
}

/* The builtin itself */

/**/
static int
bin_zselect(char *nam, char **args, UNUSED(Options ops), UNUSED(int func))
{
#ifdef HAVE_SELECT
    int i, fd, fdsetind = 0, fdmax = 0, fdcount;
    fd_set fdset[3];
    const char fdchar[3] = "rwe";
    struct timeval tv, *tvptr = NULL;
    char *outarray = "reply", **outdata, **outptr;
    char *outhash = NULL;
    LinkList fdlist;

    for (i = 0; i < 3; i++)
	FD_ZERO(fdset+i);

    for (; *args; args++) {
	char *argptr = *args, *endptr;
	zlong tempnum;
	if (*argptr == '-') {
	    for (argptr++; *argptr; argptr++) {
		switch (*argptr) {
		    /*
		     * Array name for reply, if not $reply.
		     * This gets set to e.g. `-r 0 -w 1' if 0 is ready
		     * for reading and 1 is ready for writing.
		     */
		case 'a':
		case 'A':
		    i = *argptr;
		    if (argptr[1])
			argptr++;
		    else if (args[1]) {
			argptr = *++args;
		    } else {
			zwarnnam(nam, "argument expected after -%c", *argptr);
			return 1;
		    }
		    if (idigit(*argptr) || !isident(argptr)) {
			zwarnnam(nam, "invalid array name: %s", argptr);
			return 1;
		    }
		    if (i == 'a')
			outarray = argptr;
		    else
			outhash = argptr;
		    /* set argptr to next to last char because of increment */
		    while (argptr[1])
			argptr++;
		    break;

		    /* Following numbers indicate fd's for reading */
		case 'r':
		    fdsetind = 0;
		    break;

		    /* Following numbers indicate fd's for writing */
		case 'w':
		    fdsetind = 1;
		    break;

		    /* Following numbers indicate fd's for errors */
		case 'e':
		    fdsetind = 2;
		    break;

		    /*
		     * Get a timeout value in hundredths of a second
		     * (same units as KEYTIMEOUT).  0 means just poll.
		     * If not given, blocks indefinitely.
		     */
		case 't':
		    if (argptr[1])
			argptr++;
		    else if (args[1]) {
			argptr = *++args;
		    } else {
			zwarnnam(nam, "argument expected after -%c", *argptr);
			return 1;
		    }
		    if (!idigit(*argptr)) {
			zwarnnam(nam, "number expected after -t");
			return 1;
		    }
		    tempnum = zstrtol(argptr, &endptr, 10);
		    if (*endptr) {
			zwarnnam(nam, "garbage after -t argument: %s",
				 endptr);
			return 1;
		    }
		    /* timevalue now active */
		    tvptr = &tv;
		    tv.tv_sec = (long)(tempnum / 100);
		    tv.tv_usec = (long)(tempnum % 100) * 10000L;

		    /* remember argptr is incremented at end of loop */
		    argptr = endptr - 1;
		    break;

		    /* Digits following option without arguments are fd's. */
		default:
		    if (handle_digits(nam, argptr, fdset+fdsetind,
				      &fdmax))
			return 1;
		}
	    }
	} else if (handle_digits(nam, argptr, fdset+fdsetind, &fdmax))
	    return 1;
    }

    errno = 0;
    do {
	i = select(fdmax, (SELECT_ARG_2_T)fdset, (SELECT_ARG_2_T)(fdset+1),
		   (SELECT_ARG_2_T)(fdset+2), tvptr);
    } while (i < 0 && errno == EINTR && !errflag);

    if (i <= 0) {
	if (i < 0)
	    zwarnnam(nam, "error on select: %e", errno);
	/* else no fd's set.  Presumably a timeout. */
	return 1;
    }

    /*
     * Make a linked list of all file descriptors which are ready.
     * These go into an array preceded by -r, -w or -e for read, write,
     * error as appropriate.  Typically there will only be one set
     * so this looks rather like overkill.
     */
    fdlist = znewlinklist();
    for (i = 0; i < 3; i++) {
	int doneit = 0;
	for (fd = 0; fd < fdmax; fd++) {
	    if (FD_ISSET(fd, fdset+i)) {
		char buf[BDIGBUFSIZE];
		if (outhash) {
		    /*
		     * Key/value pairs; keys are fd's (as strings),
		     * value is a (possibly improper) subset of "rwe".
		     */
		    LinkNode nptr;
		    int found = 0;

		    convbase(buf, fd, 10);
		    for (nptr = firstnode(fdlist); nptr; 
			 nptr = nextnode(nextnode(nptr))) {
			if (!strcmp((char *)getdata(nptr), buf)) {
			    /* Already there, add new character. */
			    void **dataptr = getaddrdata(nextnode(nptr));
			    char *data = (char *)*dataptr, *ptr;
			    found = 1;
			    if (!strchr(data, fdchar[i])) {
				strcpy(buf, data);
				for (ptr = buf; *ptr; ptr++)
				    ;
				*ptr++ = fdchar[i];
				*ptr = '\0';
				zsfree(data);
				*dataptr = ztrdup(buf);
			    }
			    break;
			}
		    }
		    if (!found) {
			/* Add new key/value pair. */
			zaddlinknode(fdlist, ztrdup(buf));
			buf[0] = fdchar[i];
			buf[1] = '\0';
			zaddlinknode(fdlist, ztrdup(buf));
		    }
		} else {
		    /* List of fd's preceded by -r, -w, -e. */
		    if (!doneit) {
			buf[0] = '-';
			buf[1] = fdchar[i];
			buf[2] = 0;
			zaddlinknode(fdlist, ztrdup(buf));
			doneit = 1;
		    }
		    convbase(buf, fd, 10);
		    zaddlinknode(fdlist, ztrdup(buf));
		}
	    }
	}
    }

    /* convert list to array */
    fdcount = countlinknodes(fdlist);
    outptr = outdata = (char **)zalloc((fdcount+1)*sizeof(char *));
    while (nonempty(fdlist))
	*outptr++ = getlinknode(fdlist);
    *outptr = NULL;
    /* and store in array parameter */
    if (outhash)
	sethparam(outhash, outdata);
    else
	setaparam(outarray, outdata);
    freelinklist(fdlist, NULL);

    return 0;
#else
    /* TODO: use poll */
    zerrnam(nam, "your system does not implement the select system call.");
    return 2;
#endif
}


static struct builtin bintab[] = {
    BUILTIN("zselect", 0, bin_zselect, 0, -1, 0, NULL, NULL),
};

static struct features module_features = {
    bintab, sizeof(bintab)/sizeof(*bintab),
    NULL, 0,
    NULL, 0,
    NULL, 0,
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
