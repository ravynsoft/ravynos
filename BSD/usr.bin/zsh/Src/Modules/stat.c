/*
 * stat.c - stat builtin interface to system call
 *
 * This file is part of zsh, the Z shell.
 *
 * Copyright (c) 1996-1997 Peter Stephenson
 * All rights reserved.
 *
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and to distribute modified versions of this software for any
 * purpose, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 *
 * In no event shall Peter Stephenson or the Zsh Development Group be liable
 * to any party for direct, indirect, special, incidental, or consequential
 * damages arising out of the use of this software and its documentation,
 * even if Peter Stephenson and the Zsh Development Group have been advised of
 * the possibility of such damage.
 *
 * Peter Stephenson and the Zsh Development Group specifically disclaim any
 * warranties, including, but not limited to, the implied warranties of
 * merchantability and fitness for a particular purpose.  The software
 * provided hereunder is on an "as is" basis, and Peter Stephenson and the
 * Zsh Development Group have no obligation to provide maintenance,
 * support, updates, enhancements, or modifications.
 *
 */

#include "stat.mdh"
#include "stat.pro"

enum statnum { ST_DEV, ST_INO, ST_MODE, ST_NLINK, ST_UID, ST_GID,
		   ST_RDEV, ST_SIZE, ST_ATIM, ST_MTIM, ST_CTIM,
		   ST_BLKSIZE, ST_BLOCKS, ST_READLINK, ST_COUNT };
enum statflags { STF_NAME = 1,  STF_FILE = 2, STF_STRING = 4, STF_RAW = 8,
		     STF_PICK = 16, STF_ARRAY = 32, STF_GMT = 64,
		     STF_HASH = 128, STF_OCTAL = 256 };
static char *statelts[] = { "device", "inode", "mode", "nlink",
				"uid", "gid", "rdev", "size", "atime",
				"mtime", "ctime", "blksize", "blocks",
				"link", NULL };
#define HNAMEKEY "name"

/**/
static void
statmodeprint(mode_t mode, char *outbuf, int flags)
{
    if (flags & STF_RAW) {
	sprintf(outbuf, (flags & STF_OCTAL) ? "0%lo" : "%lu",
		(unsigned long)mode);
	if (flags & STF_STRING)
	    strcat(outbuf, " (");
    }
    if (flags & STF_STRING) {
	static const char *modes = "?rwxrwxrwx";
#ifdef __CYGWIN__
	static mode_t mflags[9] = { 0 };
#else
	static const mode_t mflags[9] = {
	    S_IRUSR, S_IWUSR, S_IXUSR,
	    S_IRGRP, S_IWGRP, S_IXGRP,
	    S_IROTH, S_IWOTH, S_IXOTH
	};
#endif
	const mode_t *mfp = mflags;
	char pm[11];
	int i;

#ifdef __CYGWIN__
	if (mflags[0] == 0) {
	    mflags[0] = S_IRUSR;
	    mflags[1] = S_IWUSR;
	    mflags[2] = S_IXUSR;
	    mflags[3] = S_IRGRP;
	    mflags[4] = S_IWGRP;
	    mflags[5] = S_IXGRP;
	    mflags[6] = S_IROTH;
	    mflags[7] = S_IWOTH;
	    mflags[8] = S_IXOTH;
	}
#endif

	if (S_ISBLK(mode))
	    *pm = 'b';
	else if (S_ISCHR(mode))
	    *pm = 'c';
	else if (S_ISDIR(mode))
	    *pm = 'd';
	else if (S_ISDOOR(mode))
	    *pm = 'D';
	else if (S_ISFIFO(mode))
	    *pm = 'p';
	else if (S_ISLNK(mode))
	    *pm = 'l';
	else if (S_ISMPC(mode))
	    *pm = 'm';
	else if (S_ISNWK(mode))
	    *pm = 'n';
	else if (S_ISOFD(mode))
	    *pm = 'M';
	else if (S_ISOFL(mode))
	    *pm = 'M';
	else if (S_ISREG(mode))
	    *pm = '-';
	else if (S_ISSOCK(mode))
	    *pm = 's';
	else
	    *pm = '?';

	for (i = 1; i <= 9; i++)
	    pm[i] = (mode & *mfp++) ? modes[i] : '-';
	pm[10] = '\0';

	if (mode & S_ISUID)
	    pm[3] = (mode & S_IXUSR) ? 's' : 'S';
	if (mode & S_ISGID)
	    pm[6] = (mode & S_IXGRP) ? 's' : 'S';
	if (mode & S_ISVTX)
	    pm[9] = (mode & S_IXOTH) ? 't' : 'T';

	pm[10] = 0;
	strcat(outbuf, pm);
	if (flags & STF_RAW)
	    strcat(outbuf, ")");
    }
}


/**/
static void
statuidprint(uid_t uid, char *outbuf, int flags)
{
    if (flags & STF_RAW) {
	sprintf(outbuf, "%lu", (unsigned long)uid);
	if (flags & STF_STRING)
	    strcat(outbuf, " (");
    }
    if (flags & STF_STRING) {
#ifdef HAVE_GETPWUID
	struct passwd *pwd;
	pwd = getpwuid(uid);
	if (pwd)
	    strcat(outbuf, pwd->pw_name);
	else
#endif /* !HAVE_GETPWUID */
	{
	    char *optr;
	    for (optr = outbuf; *optr; optr++)
		;
	    sprintf(optr, "%lu", (unsigned long)uid);
	}
	if (flags & STF_RAW)
	    strcat(outbuf, ")");
    }
}


/**/
static void
statgidprint(gid_t gid, char *outbuf, int flags)
{
    if (flags & STF_RAW) {
	sprintf(outbuf, "%lu", (unsigned long)gid);
	if (flags & STF_STRING)
	    strcat(outbuf, " (");
    }
    if (flags & STF_STRING) {
#ifdef USE_GETGRGID
	struct group *gr;
	gr = getgrgid(gid);
	if (gr)
	    strcat(outbuf, gr->gr_name);
	else
#endif /* !USE_GETGRGID */
	{
	    char *optr;
	    for (optr = outbuf; *optr; optr++)
		;
	    sprintf(optr, "%lu", (unsigned long)gid);
	}
	if (flags & STF_RAW)
	    strcat(outbuf, ")");
    }
}

static char *timefmt;

/**/
static void
stattimeprint(time_t tim, long nsecs, char *outbuf, int flags)
{
    if (flags & STF_RAW) {
	sprintf(outbuf, "%ld", (unsigned long)tim);
	if (flags & STF_STRING)
	    strcat(outbuf, " (");
    }
    if (flags & STF_STRING) {
	char *oend = outbuf + strlen(outbuf);
	/* Where the heck does "40" come from? */
	ztrftime(oend, 40, timefmt, (flags & STF_GMT) ? gmtime(&tim) :
			   localtime(&tim), nsecs);
	if (flags & STF_RAW)
	    strcat(oend, ")");
    }
}


/**/
static void
statulprint(unsigned long num, char *outbuf)
{
    sprintf(outbuf, "%lu", num);
}


/**/
static void
statlinkprint(struct stat *sbuf, char *outbuf, char *fname)
{
    int num;

    /* fname is NULL if we are looking at an fd */
    if (fname && S_ISLNK(sbuf->st_mode) &&
 	(num = readlink(fname, outbuf, PATH_MAX)) > 0) {
	/* readlink doesn't terminate the buffer itself */
	outbuf[num] = '\0';
    }
}


/**/
static void
statprint(struct stat *sbuf, char *outbuf, char *fname, int iwhich, int flags)
{
    char *optr = outbuf;

    if (flags & STF_NAME) {
	sprintf(outbuf, (flags & (STF_PICK|STF_ARRAY)) ?
		"%s " : "%-8s", statelts[iwhich]);
	optr += strlen(outbuf);
    }
    *optr = '\0';

    /* cast values to unsigned long as safest bet */
    switch (iwhich) {
    case ST_DEV:
	statulprint((unsigned long)sbuf->st_dev, optr);
	break;

    case ST_INO:
#ifdef INO_T_IS_64_BIT
	convbase(optr, sbuf->st_ino, 0);
#else
	DPUTS(sizeof(sbuf->st_ino) > sizeof(unsigned long),
	      "Shell compiled with wrong ino_t size");
	statulprint((unsigned long)sbuf->st_ino, optr);
#endif
	break;

    case ST_MODE:
	statmodeprint(sbuf->st_mode, optr, flags);
	break;

    case ST_NLINK:
	statulprint((unsigned long)sbuf->st_nlink, optr);
	break;

    case ST_UID:
	statuidprint(sbuf->st_uid, optr, flags);
	break;

    case ST_GID:
	statgidprint(sbuf->st_gid, optr, flags);
	break;

    case ST_RDEV:
	statulprint((unsigned long)sbuf->st_rdev, optr);
	break;

    case ST_SIZE:
#ifdef OFF_T_IS_64_BIT
	convbase(optr, sbuf->st_size, 0);
#else
	DPUTS(sizeof(sbuf->st_size) > sizeof(unsigned long),
	      "Shell compiled with wrong off_t size");
	statulprint((unsigned long)sbuf->st_size, optr);
#endif
	break;

    case ST_ATIM:
#ifdef GET_ST_ATIME_NSEC
	stattimeprint(sbuf->st_atime, GET_ST_ATIME_NSEC(*sbuf), optr, flags);
#else
	stattimeprint(sbuf->st_atime, 0L, optr, flags);
#endif
	break;

    case ST_MTIM:
#ifdef GET_ST_MTIME_NSEC
	stattimeprint(sbuf->st_mtime, GET_ST_MTIME_NSEC(*sbuf), optr, flags);
#else
	stattimeprint(sbuf->st_mtime, 0L, optr, flags);
#endif
	break;

    case ST_CTIM:
#ifdef GET_ST_CTIME_NSEC
	stattimeprint(sbuf->st_ctime, GET_ST_CTIME_NSEC(*sbuf), optr, flags);
#else
	stattimeprint(sbuf->st_ctime, 0L, optr, flags);
#endif
	break;

    case ST_BLKSIZE:
	statulprint((unsigned long)sbuf->st_blksize, optr);
	break;

    case ST_BLOCKS:
	statulprint((unsigned long)sbuf->st_blocks, optr);
	break;

    case ST_READLINK:
	statlinkprint(sbuf, optr, fname);
	break;

    case ST_COUNT:			/* keep some compilers happy */
	break;
    }
}


/*
 *
 * Options:
 *  -f fd:   stat fd instead of file
 *  -g:   use GMT rather than local time for time strings (forces -s on).
 *  -n:   always print file name of file being statted
 *  -N:   never print file name
 *  -l:   list stat types
 *  -L:   do lstat (else links are implicitly dereferenced by stat)
 *  -t:   always print name of stat type
 *  -T:   never print name of stat type
 *  -r:   print raw alongside string data
 *  -s:   string, print mode, times, uid, gid as appropriate strings:
 *        harmless but unnecessary when combined with -r.
 *  -A array:  assign results to given array, one stat result per element.
 *        File names and type names are only added if explicitly requested:
 *        file names are returned as a separate array element, type names as
 *        prefix to element.  Note the formatting deliberately contains
 *        fewer frills when -A is used.
 *  -H hash:  as for -A array, but returns a hash with the keys being those
 *        from stat -l
 *  -F fmt: specify a $TIME-like format for printing times; the default
 *        is the (CTIME-like) "%a %b %e %k:%M:%S %Z %Y".  This option implies
 *        -s as it is not useful for numerical times.
 *
 *  +type selects just element type of stat buffer (-l gives list):
 *        type can be shortened to unambiguous string.  only one type is
 *        allowed.  The extra type, +link, reads the target of a symbolic
 *        link; it is empty if the stat was not an lstat or if 
 *        a file descriptor was stat'd, if the stat'd file is
 *        not a symbolic link, or if symbolic links are not
 *        supported.  If +link is explicitly requested, the -L (lstat)
 *        option is set automatically.
 */
/**/
static int
bin_stat(char *name, char **args, Options ops, UNUSED(int func))
{
    char **aptr, *arrnam = NULL, **array = NULL, **arrptr = NULL;
    char *hashnam = NULL, **hash = NULL, **hashptr = NULL;
    int len, iwhich = -1, ret = 0, flags = 0, arrsize = 0, fd = 0;
    struct stat statbuf;
    int found = 0, nargs;

    timefmt = "%a %b %e %k:%M:%S %Z %Y";

    for (; *args && (**args == '+' || **args == '-'); args++) {
	char *arg = *args+1;
	if (!*arg || *arg == '-' || *arg == '+') {
	    args++;
	    break;
	}

	if (**args == '+') {
	    if (found)
		break;
	    len = strlen(arg);
	    for (aptr = statelts; *aptr; aptr++)
		if (!strncmp(*aptr, arg, len)) {
		    found++;
		    iwhich = aptr - statelts;
		}
	    if (found > 1) {
		zwarnnam(name, "%s: ambiguous stat element", arg);
		return 1;
	    } else if (found == 0) {
		zwarnnam(name, "%s: no such stat element", arg);
		return 1;
	    }
	    /* if name of link requested, turn on lstat */
	    if (iwhich == ST_READLINK)
		ops->ind['L'] = 1;
	    flags |= STF_PICK;
	} else {
	    for (; *arg; arg++) {
		if (strchr("glLnNorstT", *arg))
		    ops->ind[STOUC(*arg)] = 1;
		else if (*arg == 'A') {
		    if (arg[1]) {
			arrnam = arg+1;
		    } else if (!(arrnam = *++args)) {
			zwarnnam(name, "missing parameter name");
			return 1;
		    }
		    flags |= STF_ARRAY;
		    break;
		} else if (*arg == 'H') {
		    if (arg[1]) {
			hashnam = arg+1;
		    } else if (!(hashnam = *++args)) {
			zwarnnam(name, "missing parameter name");
			return 1;
		    }
		    flags |= STF_HASH;
		    break;
		} else if (*arg == 'f') {
		    char *sfd;
		    ops->ind['f'] = 1;
		    if (arg[1]) {
			sfd = arg+1;
		    } else if (!(sfd = *++args)) {
			zwarnnam(name, "missing file descriptor");
			return 1;
		    }
		    fd = zstrtol(sfd, &sfd, 10);
		    if (*sfd) {
			zwarnnam(name, "bad file descriptor");
			return 1;
		    }
		    break;
		} else if (*arg == 'F') {
		    if (arg[1]) {
			timefmt = arg+1;
		    } else if (!(timefmt = *++args)) {
			zwarnnam(name, "missing time format");
			return 1;
		    }
		    /* force string format in order to use time format */
		    ops->ind['s'] = 1;
		    break;
		} else {
		    zwarnnam(name, "bad option: -%c", *arg);
		    return 1;
		}
	    }
	}
    }

    if ((flags & STF_ARRAY) && (flags & STF_HASH)) {
    	/* We don't implement setting multiple variables at once */
	zwarnnam(name, "both array and hash requested");
	return 1;
	/* Alternate method would be to make -H blank arrnam etc etc *
	 * and so get 'silent loss' of earlier choice, which would   *
	 * be similar to stat -A foo -A bar filename                 */
    }

    if (OPT_ISSET(ops,'l')) {
	/* list types and return:  can also list to array */
	if (arrnam) {
	    arrptr = array = (char **)zalloc((ST_COUNT+1)*sizeof(char *));
	    array[ST_COUNT] = NULL;
	}
	for (aptr = statelts; *aptr; aptr++) {
	    if (arrnam) {
		*arrptr++ = ztrdup(*aptr);
	    } else {
		printf("%s", *aptr);
		if (aptr[1])
		    putchar(' ');
	    }
	}
	if (arrnam) {
	    setaparam(arrnam, array);
	    if (errflag)
		return 1;
	} else
	    putchar('\n');
	return 0;
    }

    if (!*args && !OPT_ISSET(ops,'f')) {
	zwarnnam(name, "no files given");
	return 1;
    } else if (*args && OPT_ISSET(ops,'f')) {
	zwarnnam(name, "no files allowed with -f");
	return 1;
    }

    nargs = 0;
    if (OPT_ISSET(ops,'f'))
	nargs = 1;
    else
	for (aptr = args; *aptr; aptr++) {
	    unmetafy(*aptr, NULL);
	    nargs++;
	}

    if (OPT_ISSET(ops,'g')) {
	flags |= STF_GMT;
	ops->ind['s'] = 1;
    }
    if (OPT_ISSET(ops,'s') || OPT_ISSET(ops,'r'))
	flags |= STF_STRING;
    if (OPT_ISSET(ops,'r') || !OPT_ISSET(ops,'s'))
	flags |= STF_RAW;
    if (OPT_ISSET(ops,'n'))
	flags |= STF_FILE;
    if (OPT_ISSET(ops,'o'))
	flags |= STF_OCTAL;
    if (OPT_ISSET(ops,'t'))
	flags |= STF_NAME;

    if (!(arrnam || hashnam)) {
	if (nargs > 1)
	    flags |= STF_FILE;
	if (!(flags & STF_PICK))
	    flags |= STF_NAME;
    }

    if (OPT_ISSET(ops,'N') || OPT_ISSET(ops,'f'))
	flags &= ~STF_FILE;
    if (OPT_ISSET(ops,'T') || OPT_ISSET(ops,'H'))
	flags &= ~STF_NAME;

    if (hashnam) {
    	if (nargs > 1) {
	    zwarnnam(name, "only one file allowed with -H");
	    return 1;
	}
	arrsize = (flags & STF_PICK) ? 1 : ST_COUNT;
	if (flags & STF_FILE)
	    arrsize++;
	hashptr = hash = (char **)zshcalloc((arrsize+1)*2*sizeof(char *));
    }

    if (arrnam) {
	arrsize = (flags & STF_PICK) ? 1 : ST_COUNT;
	if (flags & STF_FILE)
	    arrsize++;
	arrsize *= nargs;
	arrptr = array = (char **)zshcalloc((arrsize+1)*sizeof(char *));
    }

    for (; OPT_ISSET(ops,'f') || *args; args++) {
	char outbuf[PATH_MAX + 9]; /* "link   " + link name + NULL */
	int rval = OPT_ISSET(ops,'f') ? fstat(fd, &statbuf) :
	    OPT_ISSET(ops,'L') ? lstat(*args, &statbuf) :
	    stat(*args, &statbuf);
	if (rval) {
	    if (OPT_ISSET(ops,'f'))
		sprintf(outbuf, "%d", fd);
	    zwarnnam(name, "%s: %e", OPT_ISSET(ops,'f') ? outbuf : *args,
		     errno);
	    ret = 1;
	    if (OPT_ISSET(ops,'f') || arrnam)
		break;
	    else
		continue;
	}

	if (flags & STF_FILE) {
	    if (arrnam)
		*arrptr++ = ztrdup_metafy(*args);
	    else if (hashnam) {
	    	*hashptr++ = ztrdup(HNAMEKEY);
		*hashptr++ = ztrdup_metafy(*args);
	    } else
		printf("%s%s", *args, (flags & STF_PICK) ? " " : ":\n");
	}
	if (iwhich > -1) {
	    statprint(&statbuf, outbuf, *args, iwhich, flags);
	    if (arrnam)
		*arrptr++ = metafy(outbuf, -1, META_DUP);
	    else if (hashnam) {
		/* STF_NAME explicitly turned off for ops.ind['H'] above */
	    	*hashptr++ = ztrdup(statelts[iwhich]);
		*hashptr++ = metafy(outbuf, -1, META_DUP);
	    } else
		printf("%s\n", outbuf);
	} else {
	    int i;
	    for (i = 0; i < ST_COUNT; i++) {
		statprint(&statbuf, outbuf, *args, i, flags);
		if (arrnam)
		    *arrptr++= metafy(outbuf, -1, META_DUP);
		else if (hashnam) {
		    /* STF_NAME explicitly turned off for ops.ind['H'] above */
		    *hashptr++ = ztrdup(statelts[i]);
		    *hashptr++ = metafy(outbuf, -1, META_DUP);
		} else
		    printf("%s\n", outbuf);
	    }
	}
	if (OPT_ISSET(ops,'f'))
	    break;

	if (!arrnam && !hashnam && args[1] && !(flags & STF_PICK))
	    putchar('\n');
    }

    if (arrnam) {
	if (ret)
	    freearray(array);
	else {
	    setaparam(arrnam, array);
	    if (errflag)
		return 1;
	}
    }

    if (hashnam) {
    	if (ret)
	    freearray(hash);
	else {
	    sethparam(hashnam, hash);
	    if (errflag)
		return 1;
	}
    }

    return ret;
}

static struct builtin bintab[] = {
    BUILTIN("stat", 0, bin_stat, 0, -1, 0, NULL, NULL),
    BUILTIN("zstat", 0, bin_stat, 0, -1, 0, NULL, NULL),
};

static struct features module_features = {
    bintab, sizeof(bintab)/sizeof(*bintab),
    NULL, 0,
    NULL, 0,
    NULL, 0,
    0
};

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
