/*
 * Copyright (c) 1989, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Michael Fischbein.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
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

#include <sys/cdefs.h>
#ifndef lint
__used static const char copyright[] =
"@(#) Copyright (c) 1989, 1993, 1994\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#if 0
#ifndef lint
static char sccsid[] = "@(#)ls.c	8.5 (Berkeley) 4/2/94";
#endif /* not lint */
#endif
#include <sys/cdefs.h>
__RCSID("$FreeBSD: src/bin/ls/ls.c,v 1.66 2002/09/21 01:28:36 wollman Exp $");

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include <dirent.h>
#include <err.h>
#include <errno.h>
#include <fts.h>
#include <grp.h>
#include <limits.h>
#include <locale.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifdef COLORLS
#include <termcap.h>
#include <signal.h>
#endif
#ifdef __APPLE__
#include <sys/acl.h>
#include <sys/xattr.h>
#include <sys/param.h>
#include <get_compat.h>
#include <sys/sysctl.h>
#include <System/sys/fsctl.h>
#else
#define COMPAT_MODE(a,b) (1)
#endif /* __APPLE__ */
#include "ls.h"
#include "extern.h"

/*
 * Upward approximation of the maximum number of characters needed to
 * represent a value of integral type t as a string, excluding the
 * NUL terminator, with provision for a sign.
 */
#define	STRBUF_SIZEOF(t)	(1 + CHAR_BIT * sizeof(t) / 3 + 1)

static void	 display(FTSENT *, FTSENT *);
static u_quad_t	 makenines(u_quad_t);
static int	 mastercmp(const FTSENT **, const FTSENT **);
static void	 traverse(int, char **, int);

static void (*printfcn)(DISPLAY *);
static int (*sortfcn)(const FTSENT *, const FTSENT *);

long blocksize;			/* block size units */
int termwidth = 80;		/* default terminal width */

/* flags */
       int f_accesstime;	/* use time of last access */
       int f_birthtime;		/* use time of file birth */
       int f_flags;		/* show flags associated with a file */
       int f_humanval;		/* show human-readable file sizes */
       int f_inode;		/* print inode */
static int f_kblocks;		/* print size in kilobytes */
static int f_listdir;		/* list actual directory, not contents */
static int f_listdot;		/* list files beginning with . */
       int f_longform;		/* long listing format */
       int f_nonprint;		/* show unprintables as ? */
static int f_nosort;		/* don't sort output */
       int f_notabs;		/* don't use tab-separated multi-col output */
       int f_numericonly;	/* don't convert uid/gid to name */
       int f_octal;		/* show unprintables as \xxx */
       int f_octal_escape;	/* like f_octal but use C escapes if possible */
static int f_recursive;		/* ls subdirectories also */
static int f_reversesort;	/* reverse whatever sort is used */
       int f_sectime;		/* print the real time for all files */
static int f_singlecol;		/* use single column output */
       int f_size;		/* list size in short listing */
       int f_slash;		/* similar to f_type, but only for dirs */
       int f_sortacross;	/* sort across rows, not down columns */ 
       int f_statustime;	/* use time of last mode change */
       int f_stream;		/* stream the output, separate with commas */
static int f_timesort;		/* sort by time vice name */
static int f_sizesort;		/* sort by size */
       int f_type;		/* add type character for non-regular files */
static int f_whiteout;		/* show whiteout entries */
       int f_acl;		/* show ACLs in long listing */
       int f_xattr;		/* show extended attributes in long listing */
       int f_group;		/* show group */
       int f_owner;		/* show owner */
       int f_dataless;          /* distinguish dataless files in long listing,
				   and don't materialize dataless directories. */
#ifdef COLORLS
       int f_color;		/* add type in color for non-regular files */

char *ansi_bgcol;		/* ANSI sequence to set background colour */
char *ansi_fgcol;		/* ANSI sequence to set foreground colour */
char *ansi_coloff;		/* ANSI sequence to reset colours */
char *attrs_off;		/* ANSI sequence to turn off attributes */
char *enter_bold;		/* ANSI sequence to set color to bold mode */
#endif

static int rval;

int
main(int argc, char *argv[])
{
	static char dot[] = ".", *dotav[] = {dot, NULL};
	struct winsize win;
	int ch, fts_options, notused;
	char *p;
#ifdef COLORLS
	char termcapbuf[1024];	/* termcap definition buffer */
	char tcapbuf[512];	/* capability buffer */
	char *bp = tcapbuf;
#endif

	if (argc < 1)
		usage();
	(void)setlocale(LC_ALL, "");

	/* Terminal defaults to -Cq, non-terminal defaults to -1. */
	if (isatty(STDOUT_FILENO)) {
		termwidth = 80;
		if ((p = getenv("COLUMNS")) != NULL && *p != '\0')
			termwidth = atoi(p);
		else if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &win) != -1 &&
		    win.ws_col > 0)
			termwidth = win.ws_col;
		f_nonprint = 1;
	} else {
		f_singlecol = 1;
		/* retrieve environment variable, in case of explicit -C */
		p = getenv("COLUMNS");
		if (p)
			termwidth = atoi(p);
	}

	/* Root is -A automatically. */
	if (!getuid())
		f_listdot = 1;

	fts_options = FTS_PHYSICAL;
	while ((ch = getopt(argc, argv, "1@ABCFGHLOPRSTUWabcdefghiklmnopqrstuvwx%"))
	    != -1) {
		switch (ch) {
		/*
		 * The -1, -C, -x and -l options all override each other so
		 * shell aliasing works right.
		 */
		case '1':
			f_singlecol = 1;
			f_longform = 0;
			f_stream = 0;
			break;
		case 'B':
			f_nonprint = 0;
			f_octal = 1;
			f_octal_escape = 0;
			break;
		case 'C':
			f_sortacross = f_longform = f_singlecol = 0;
			break;
		case 'l':
			f_longform = 1;
			f_singlecol = 0;
			f_stream = 0;
			break;
		case 'x':
			f_sortacross = 1;
			f_longform = 0;
			f_singlecol = 0;
			break;
		/* The -c and -u options override each other. */
		case 'c':
			f_statustime = 1;
			f_accesstime = f_birthtime = 0;
			break;
		case 'u':
			f_accesstime = 1;
			f_statustime = f_birthtime = 0;
			break;
		case 'U':
			f_birthtime = 1;
			f_statustime = f_accesstime = 0;
			break;
		case 'F':
			f_type = 1;
			f_slash = 0;
			break;
		case 'H':
			if (COMPAT_MODE("bin/ls", "Unix2003")) {
				fts_options &= ~FTS_LOGICAL;
				fts_options |= FTS_PHYSICAL;
				fts_options |= FTS_COMFOLLOWDIR;
			} else
				fts_options |= FTS_COMFOLLOW;
			break;
		case 'G':
			setenv("CLICOLOR", "", 1);
			break;
		case 'L':
			fts_options &= ~FTS_PHYSICAL;
			fts_options |= FTS_LOGICAL;
			if (COMPAT_MODE("bin/ls", "Unix2003")) {
				fts_options &= ~(FTS_COMFOLLOW|FTS_COMFOLLOWDIR);
			}
			break;
		case 'P':
			fts_options &= ~(FTS_COMFOLLOW|FTS_COMFOLLOWDIR);
			fts_options &= ~FTS_LOGICAL;
			fts_options |= FTS_PHYSICAL;
			break;
		case 'R':
			f_recursive = 1;
			break;
		case 'a':
			fts_options |= FTS_SEEDOT;
			/* FALLTHROUGH */
		case 'A':
			f_listdot = 1;
			break;
		/* The -d option turns off the -R option. */
		case 'd':
			f_listdir = 1;
			f_recursive = 0;
			break;
		case 'f':
			f_nosort = 1;
			if (COMPAT_MODE("bin/ls", "Unix2003")) {
				fts_options |= FTS_SEEDOT;
				f_listdot = 1;
			}
			break;
		case 'g':	/* Compatibility with Unix03 */
			if (COMPAT_MODE("bin/ls", "Unix2003")) {
				f_group = 1;
				f_longform = 1;
				f_singlecol = 0;
				f_stream = 0;
			}
			break;
		case 'h':
			f_humanval = 1;
			break;
		case 'i':
			f_inode = 1;
			break;
		case 'k':
			f_kblocks = 1;
			break;
		case 'm':
			f_stream = 1;
			f_singlecol = 0;
			f_longform = 0;
			break;
		case 'n':
			f_numericonly = 1;
			if (COMPAT_MODE("bin/ls", "Unix2003")) {
				f_longform = 1;
				f_singlecol = 0;
				f_stream = 0;
			}
			break;
		case 'o':
			if (COMPAT_MODE("bin/ls", "Unix2003")) {
				f_owner = 1;
				f_longform = 1;
				f_singlecol = 0;
				f_stream = 0;
			} else {
				f_flags = 1;
			}
			break;
		case 'p':
			f_slash = 1;
			f_type = 1;
			break;
		case 'q':
			f_nonprint = 1;
			f_octal = 0;
			f_octal_escape = 0;
			break;
		case 'r':
			f_reversesort = 1;
			break;
		case 'S':
			/* Darwin 1.4.1 compatibility */
			f_sizesort = 1;
			break;
		case 's':
			f_size = 1;
			break;
		case 'T':
			f_sectime = 1;
			break;
		case 't':
			f_timesort = 1;
			break;
		case 'W':
			f_whiteout = 1;
			break;
		case 'v':
			/* Darwin 1.4.1 compatibility */
			f_nonprint = 0;
			break;
		case 'b':
			f_nonprint = 0;
			f_octal = 0;
			f_octal_escape = 1;
			break;
		case 'w':
			f_nonprint = 0;
			f_octal = 0;
			f_octal_escape = 0;
			break;
		case 'e':
			f_acl = 1;
			break;
		case '@':
			f_xattr = 1;
			break;
		case 'O':
			f_flags = 1;
			break;
		case '%':
			f_dataless = 1;
			break;
		default:
		case '?':
			usage();
		}
	}
	argc -= optind;
	argv += optind;

	/* Enabling of colours is conditional on the environment. */
	if (getenv("CLICOLOR") &&
	    (isatty(STDOUT_FILENO) || getenv("CLICOLOR_FORCE")))
#ifdef COLORLS
		if (tgetent(termcapbuf, getenv("TERM")) == 1) {
			ansi_fgcol = tgetstr("AF", &bp);
			ansi_bgcol = tgetstr("AB", &bp);
			attrs_off = tgetstr("me", &bp);
			enter_bold = tgetstr("md", &bp);

			/* To switch colours off use 'op' if
			 * available, otherwise use 'oc', or
			 * don't do colours at all. */
			ansi_coloff = tgetstr("op", &bp);
			if (!ansi_coloff)
				ansi_coloff = tgetstr("oc", &bp);
			if (ansi_fgcol && ansi_bgcol && ansi_coloff)
				f_color = 1;
		}
#else
		(void)fprintf(stderr, "Color support not compiled in.\n");
#endif /*COLORLS*/

#ifdef COLORLS
	if (f_color) {
		/*
		 * We can't put tabs and color sequences together:
		 * column number will be incremented incorrectly
		 * for "stty oxtabs" mode.
		 */
		f_notabs = 1;
		(void)signal(SIGINT, colorquit);
		(void)signal(SIGQUIT, colorquit);
		parsecolors(getenv("LSCOLORS"));
	}
#endif

	/*
	 * If not -F, -i, -l, -s or -t options, don't require stat
	 * information, unless in color mode in which case we do
	 * need this to determine which colors to display.
	 */
	if (!f_inode && !f_longform && !f_size && !f_timesort && !f_type && !f_sizesort
#ifdef COLORLS
	    && !f_color
#endif
	    )
		fts_options |= FTS_NOSTAT;

	/*
	 * If not -F, -d or -l options, follow any symbolic links listed on
	 * the command line.
	 */
	if (!f_longform && !f_listdir && !f_type && !f_inode)
		fts_options |= FTS_COMFOLLOW;

	/*
	 * If -W, show whiteout entries
	 */
#ifdef FTS_WHITEOUT
	if (f_whiteout)
		fts_options |= FTS_WHITEOUT;
#endif

	/* If -l or -s, figure out block size. */
	if (f_longform || f_size) {
		if (f_kblocks)
			blocksize = 2;
		else {
			(void)getbsize(&notused, &blocksize);
			blocksize /= 512;
		}
	}
	/* Select a sort function. */
	if (f_reversesort) {
		if (f_sizesort)
			sortfcn = revsizecmp;
		else if (!f_timesort)
			sortfcn = revnamecmp;
		else if (f_accesstime)
			sortfcn = revacccmp;
		else if (f_statustime)
			sortfcn = revstatcmp;
		else if (f_birthtime)
			sortfcn = revbirthcmp;
		else		/* Use modification time. */
			sortfcn = revmodcmp;
	} else {
		if (f_sizesort)
			sortfcn = sizecmp;
		else if (!f_timesort)
			sortfcn = namecmp;
		else if (f_accesstime)
			sortfcn = acccmp;
		else if (f_statustime)
			sortfcn = statcmp;
		else if (f_birthtime)
			sortfcn = birthcmp;
		else		/* Use modification time. */
			sortfcn = modcmp;
	}

	/* Select a print function. */
	if (f_singlecol)
		printfcn = printscol;
	else if (f_longform)
		printfcn = printlong;
	else if (f_stream)
		printfcn = printstream;
	else
		printfcn = printcol;

#ifdef __APPLE__
	if (f_dataless) {
		// don't materialize dataless directories from the cloud
		// (particularly usefull when listing recursively)
		int state = 1;
		if (sysctlbyname("vfs.nspace.prevent_materialization", NULL, NULL, &state, sizeof(state)) < 0) {
			err(1, "prevent_materialization");
		}
	}
#endif /* __APPLE__ */

	if (argc)
		traverse(argc, argv, fts_options);
	else
		traverse(1, dotav, fts_options);
	exit(rval);
}

static int output;		/* If anything output. */

/*
 * Traverse() walks the logical directory structure specified by the argv list
 * in the order specified by the mastercmp() comparison function.  During the
 * traversal it passes linked lists of structures to display() which represent
 * a superset (may be exact set) of the files to be displayed.
 */
static void
traverse(int argc, char *argv[], int options)
{
	FTS *ftsp;
	FTSENT *p, *chp;
	int ch_options, error;

	if ((ftsp =
	    fts_open(argv, options, f_nosort ? NULL : mastercmp)) == NULL)
		err(1, "fts_open");

	display(NULL, fts_children(ftsp, 0));
	if (f_listdir) {
		fts_close(ftsp);
		return;
	}

	/*
	 * If not recursing down this tree and don't need stat info, just get
	 * the names.
	 */
	ch_options = !f_recursive && options & FTS_NOSTAT ? FTS_NAMEONLY : 0;

	while ((p = fts_read(ftsp)) != NULL)
		switch (p->fts_info) {
		case FTS_DC:
			warnx("%s: directory causes a cycle", p->fts_name);
			if (COMPAT_MODE("bin/ls", "Unix2003")) {
				rval = 1;
			}
			break;
		case FTS_DNR:
		case FTS_ERR:
			warnx("%s: %s", p->fts_name, strerror(p->fts_errno));
			rval = 1;
			break;
		case FTS_D:
			if (p->fts_level != FTS_ROOTLEVEL &&
			    p->fts_name[0] == '.' && !f_listdot) {
				fts_set(ftsp, p, FTS_SKIP);
				break;
			}

			/*
			 * If already output something, put out a newline as
			 * a separator.  If multiple arguments, precede each
			 * directory with its name.
			 */
			if (output)
				(void)printf("\n%s:\n", p->fts_path);
			else if (argc > 1) {
				(void)printf("%s:\n", p->fts_path);
				output = 1;
			}
			chp = fts_children(ftsp, ch_options);
			if (COMPAT_MODE("bin/ls", "Unix2003") && ((options & FTS_LOGICAL)!=0)) {
				FTSENT *curr;
				for (curr = chp; curr; curr = curr->fts_link) {
					if (curr->fts_info == FTS_SLNONE)
						curr->fts_number = NO_PRINT;
				}
			}
			display(p, chp);

			if (!f_recursive && chp != NULL)
				(void)fts_set(ftsp, p, FTS_SKIP);
			break;
		case FTS_SLNONE:	/* Same as default unless Unix conformance */
			if (COMPAT_MODE("bin/ls", "Unix2003")) {
				if ((options & FTS_LOGICAL)!=0) {	/* -L was specified */
					warnx("%s: %s", p->fts_name, strerror(p->fts_errno ?: ENOENT));
					rval = 1;
				}
			}
			break;
		default:
			break;
		}
	error = errno;
	fts_close(ftsp);
	errno = error;

	if (errno)
		err(1, "fts_read");
}

/*
 * Display() takes a linked list of FTSENT structures and passes the list
 * along with any other necessary information to the print function.  P
 * points to the parent directory of the display list.
 */
static void
display(FTSENT *p, FTSENT *list)
{
	struct stat *sp;
	DISPLAY d;
	FTSENT *cur;
	NAMES *np;
	off_t maxsize;
	u_int64_t btotal, maxblock;
	u_long lattrlen, maxlen, maxnlink, maxlattr;
	ino_t maxinode;
	int bcfile, maxflags;
	gid_t maxgroup;
	uid_t maxuser;
	size_t flen, ulen, glen;
	char *initmax;
	int entries, needstats;
	const char *user, *group;
	char *flags, *lattr = NULL;
	char buf[STRBUF_SIZEOF(u_quad_t) + 1];
	char ngroup[STRBUF_SIZEOF(uid_t) + 1];
	char nuser[STRBUF_SIZEOF(gid_t) + 1];
#ifdef __APPLE__
	acl_entry_t dummy;
	ssize_t xattr_size;
	char *filename;
	char path[MAXPATHLEN+1];
#endif // __APPLE__
	/*
	 * If list is NULL there are two possibilities: that the parent
	 * directory p has no children, or that fts_children() returned an
	 * error.  We ignore the error case since it will be replicated
	 * on the next call to fts_read() on the post-order visit to the
	 * directory p, and will be signaled in traverse().
	 */
	if (list == NULL)
		return;

	needstats = f_inode || f_longform || f_size;
	btotal = 0;
	initmax = getenv("LS_COLWIDTHS");
	/* Fields match -lios order.  New ones should be added at the end. */
	maxlattr = maxblock = maxinode = maxlen = maxnlink =
	    maxuser = maxgroup = maxflags = maxsize = 0;
	if (initmax != NULL && *initmax != '\0') {
		char *initmax2, *jinitmax;
		int ninitmax;

		/* Fill-in "::" as "0:0:0" for the sake of scanf. */
		jinitmax = initmax2 = malloc(strlen(initmax) * 2 + 2);
		if (jinitmax == NULL)
			err(1, "malloc");
		if (*initmax == ':')
			strcpy(initmax2, "0:"), initmax2 += 2;
		else
			*initmax2++ = *initmax, *initmax2 = '\0';
		for (initmax++; *initmax != '\0'; initmax++) {
			if (initmax[-1] == ':' && initmax[0] == ':') {
				*initmax2++ = '0';
				*initmax2++ = initmax[0];
				initmax2[1] = '\0';
			} else {
				*initmax2++ = initmax[0];
				initmax2[1] = '\0';
			}
		}
		if (initmax2[-1] == ':')
			strcpy(initmax2, "0");

		ninitmax = sscanf(jinitmax,
#if _DARWIN_FEATURE_64_BIT_INODE
		    " %llu : %qu : %lu : %i : %i : %i : %qu : %lu : %lu ",
#else
		    " %lu : %qu : %lu : %i : %i : %i : %qu : %lu : %lu ",
#endif
		    &maxinode, &maxblock, &maxnlink, &maxuser,
		    &maxgroup, &maxflags, &maxsize, &maxlen, &maxlattr);
		f_notabs = 1;
		switch (ninitmax) {
		case 0:
			maxinode = 0;
			/* FALLTHROUGH */
		case 1:
			maxblock = 0;
			/* FALLTHROUGH */
		case 2:
			maxnlink = 0;
			/* FALLTHROUGH */
		case 3:
			maxuser = 0;
			/* FALLTHROUGH */
		case 4:
			maxgroup = 0;
			/* FALLTHROUGH */
		case 5:
			maxflags = 0;
			/* FALLTHROUGH */
		case 6:
			maxsize = 0;
			/* FALLTHROUGH */
		case 7:
			maxlen = 0;
			/* FALLTHROUGH */
		case 8:
			maxlattr = 0;
			/* FALLTHROUGH */
#ifdef COLORLS
			if (!f_color)
#endif
				f_notabs = 0;
			/* FALLTHROUGH */
		default:
			break;
		}
		maxinode = makenines(maxinode);
		maxblock = makenines(maxblock);
		maxnlink = makenines(maxnlink);
		maxsize = makenines(maxsize);
	}
	bcfile = 0;
	flags = NULL;
	for (cur = list, entries = 0; cur; cur = cur->fts_link) {
		if (cur->fts_info == FTS_ERR || cur->fts_info == FTS_NS) {
			warnx("%s: %s",
			    cur->fts_name, strerror(cur->fts_errno));
			cur->fts_number = NO_PRINT;
			rval = 1;
			continue;
		}
		/*
		 * P is NULL if list is the argv list, to which different rules
		 * apply.
		 */
		if (p == NULL) {
			/* Directories will be displayed later. */
			if (cur->fts_info == FTS_D && !f_listdir) {
				cur->fts_number = NO_PRINT;
				continue;
			}
		} else {
			/* Only display dot file if -a/-A set. */
			if (cur->fts_name[0] == '.' && !f_listdot) {
				cur->fts_number = NO_PRINT;
				continue;
			}
		}
		if (cur->fts_namelen > maxlen)
			maxlen = cur->fts_namelen;
		if (f_octal || f_octal_escape) {
			u_long t = len_octal(cur->fts_name, cur->fts_namelen);

			if (t > maxlen)
				maxlen = t;
		}
		if (needstats) {
			sp = cur->fts_statp;
			if (sp->st_blocks > maxblock)
				maxblock = sp->st_blocks;
			if (sp->st_ino > maxinode)
				maxinode = sp->st_ino;
			if (sp->st_nlink > maxnlink)
				maxnlink = sp->st_nlink;
			if (sp->st_size > maxsize)
				maxsize = sp->st_size;

			btotal += sp->st_blocks;
			if (f_longform) {
				if (f_numericonly) {
					(void)snprintf(nuser, sizeof(nuser),
					    "%u", sp->st_uid);
					(void)snprintf(ngroup, sizeof(ngroup),
					    "%u", sp->st_gid);
					user = nuser;
					group = ngroup;
				} else {
					user = user_from_uid(sp->st_uid, 0);
					group = group_from_gid(sp->st_gid, 0);
				}
				if ((ulen = strlen(user)) > maxuser)
					maxuser = ulen;
				if ((glen = strlen(group)) > maxgroup)
					maxgroup = glen;
				if (f_flags) {
					flags = fflagstostr(sp->st_flags);
					if (flags != NULL && *flags == '\0') {
						free(flags);
						flags = strdup("-");
					}
					if (flags == NULL)
						err(1, "fflagstostr");
					flen = strlen(flags);
					if (flen > (size_t)maxflags)
						maxflags = flen;
				} else
					flen = 0;
				lattr = NULL;
				lattrlen = 0;
				
				if ((np = calloc(1, sizeof(NAMES) + lattrlen +
				    ulen + glen + flen + 4)) == NULL)
					err(1, "malloc");

				np->user = &np->data[0];
				(void)strcpy(np->user, user);
				np->group = &np->data[ulen + 1];
				(void)strcpy(np->group, group);
#ifdef __APPLE__
				if (cur->fts_level == FTS_ROOTLEVEL) {
					filename = cur->fts_name;
				} else {
					snprintf(path, sizeof(path), "%s/%s", cur->fts_parent->fts_accpath, cur->fts_name);
					filename = path;
				}
				xattr_size = listxattr(filename, NULL, 0, XATTR_NOFOLLOW);
				if (xattr_size < 0) {
					xattr_size = 0;
				}
				if ((xattr_size > 0) && f_xattr) {
					/* collect sizes */
					np->xattr_names = malloc(xattr_size);
					listxattr(filename, np->xattr_names, xattr_size, XATTR_NOFOLLOW);
					for (char *name = np->xattr_names; name < np->xattr_names + xattr_size;
					     name += strlen(name)+1) {
						np->xattr_sizes = reallocf(np->xattr_sizes, (np->xattr_count+1) * sizeof(np->xattr_sizes[0]));
						np->xattr_sizes[np->xattr_count] = getxattr(filename, name, 0, 0, 0, XATTR_NOFOLLOW);
						np->xattr_count++;
					}
				}
				/* symlinks can not have ACLs */
				np->acl = acl_get_link_np(filename, ACL_TYPE_EXTENDED);
				if (np->acl) {
					if (acl_get_entry(np->acl, ACL_FIRST_ENTRY, &dummy) == -1) {
						acl_free(np->acl);
						np->acl = NULL;
					}
				}
				if (xattr_size > 0) {
					np->mode_suffix = '@';
				} else if (np->acl) {
					np->mode_suffix = '+';
				} else {
					np->mode_suffix = ' ';
				}
				if (f_dataless && (sp->st_flags & SF_DATALESS)) {
					np->mode_suffix = '%';
				}
				if (!f_acl) {
					acl_free(np->acl);
					np->acl = NULL;
				}
#endif // __APPLE__
				if (S_ISCHR(sp->st_mode) ||
				    S_ISBLK(sp->st_mode))
					bcfile = 1;

				if (f_flags) {
					np->flags = &np->data[ulen + glen + 2];
					(void)strcpy(np->flags, flags);
					free(flags);
				}
				cur->fts_pointer = np;
			}
		}
		++entries;
	}

	if (!entries)
		return;

	d.list = list;
	d.entries = entries;
	d.maxlen = maxlen;
	if (needstats) {
		d.bcfile = bcfile;
		d.btotal = btotal;
		(void)snprintf(buf, sizeof(buf), "%qu", (u_int64_t)maxblock);
		d.s_block = strlen(buf);
		d.s_flags = maxflags;
		d.s_lattr = maxlattr;
		d.s_group = maxgroup;
#if _DARWIN_FEATURE_64_BIT_INODE
		(void)snprintf(buf, sizeof(buf), "%llu", maxinode);
#else
		(void)snprintf(buf, sizeof(buf), "%lu", maxinode);
#endif
		d.s_inode = strlen(buf);
		(void)snprintf(buf, sizeof(buf), "%lu", maxnlink);
		d.s_nlink = strlen(buf);
		(void)snprintf(buf, sizeof(buf), "%qu", (u_int64_t)maxsize);
		d.s_size = strlen(buf);
		d.s_user = maxuser;
	}
	printfcn(&d);
	output = 1;

	if (f_longform) {
		for (cur = list; cur; cur = cur->fts_link) {
			np = cur->fts_pointer;
			if (np) {
				if (np->acl) {
					acl_free(np->acl);
				}
				free(np->xattr_names);
				free(np->xattr_sizes);
				free(np);
				cur->fts_pointer = NULL;
			}
		}
	}
}

/*
 * Ordering for mastercmp:
 * If ordering the argv (fts_level = FTS_ROOTLEVEL) return non-directories
 * as larger than directories.  Within either group, use the sort function.
 * All other levels use the sort function.  Error entries remain unsorted.
 */
static int
mastercmp(const FTSENT **a, const FTSENT **b)
{
	int a_info, b_info;

	a_info = (*a)->fts_info;
	if (a_info == FTS_ERR)
		return (0);
	b_info = (*b)->fts_info;
	if (b_info == FTS_ERR)
		return (0);

	if (a_info == FTS_NS || b_info == FTS_NS)
		return (namecmp(*a, *b));

	if (a_info != b_info &&
	    (*a)->fts_level == FTS_ROOTLEVEL && !f_listdir) {
		if (a_info == FTS_D)
			return (1);
		if (b_info == FTS_D)
			return (-1);
	}
	return (sortfcn(*a, *b));
}

/*
 * Makenines() returns (10**n)-1.  This is useful for converting a width
 * into a number that wide in decimal.
 */
static u_quad_t
makenines(u_quad_t n)
{
	u_long i;
	u_quad_t reg;

	reg = 1;
	/* Use a loop instead of pow(), since all values of n are small. */
	for (i = 0; i < n; i++)
		reg *= 10;
	reg--;

	return reg;
}
