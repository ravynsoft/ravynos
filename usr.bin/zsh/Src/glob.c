/*
 * glob.c - filename generation
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
#include "glob.pro"

#if defined(OFF_T_IS_64_BIT) && defined(__GNUC__)
# define ALIGN64 __attribute__((aligned(8)))
#else
# define ALIGN64
#endif

/* flag for CSHNULLGLOB */

typedef struct gmatch *Gmatch;

struct gmatch {
    /* Metafied file name */
    char *name;
    /* Unmetafied file name; embedded nulls can't occur in file names */
    char *uname;
    /*
     * Array of sort strings:  one for each GS_EXEC sort type in
     * the glob qualifiers.
     */
    char **sortstrs;
    off_t size ALIGN64;
    long atime;
    long mtime;
    long ctime;
    long links;
    off_t _size ALIGN64;
    long _atime;
    long _mtime;
    long _ctime;
    long _links;
#ifdef GET_ST_ATIME_NSEC
    long ansec;
    long _ansec;
#endif
#ifdef GET_ST_MTIME_NSEC
    long mnsec;
    long _mnsec;
#endif
#ifdef GET_ST_CTIME_NSEC
    long cnsec;
    long _cnsec;
#endif
};

#define GS_NAME   1
#define GS_DEPTH  2
#define GS_EXEC	  4

#define GS_SHIFT_BASE	8

#define GS_SIZE  (GS_SHIFT_BASE)
#define GS_ATIME (GS_SHIFT_BASE << 1)
#define GS_MTIME (GS_SHIFT_BASE << 2)
#define GS_CTIME (GS_SHIFT_BASE << 3)
#define GS_LINKS (GS_SHIFT_BASE << 4)

#define GS_SHIFT  5
#define GS__SIZE  (GS_SIZE << GS_SHIFT)
#define GS__ATIME (GS_ATIME << GS_SHIFT)
#define GS__MTIME (GS_MTIME << GS_SHIFT)
#define GS__CTIME (GS_CTIME << GS_SHIFT)
#define GS__LINKS (GS_LINKS << GS_SHIFT)

#define GS_DESC  (GS_SHIFT_BASE << (2*GS_SHIFT))
#define GS_NONE  (GS_SHIFT_BASE << (2*GS_SHIFT+1))

#define GS_NORMAL (GS_SIZE | GS_ATIME | GS_MTIME | GS_CTIME | GS_LINKS)
#define GS_LINKED (GS_NORMAL << GS_SHIFT)

/**/
int badcshglob;

/**/
int pathpos;		/* position in pathbuf (needed by pattern code) */

/*
 * pathname buffer (needed by pattern code).
 * It is currently believed the string in here is stored metafied and is
 * unmetafied temporarily as needed by system calls.
 */

/**/
char *pathbuf;

typedef struct stat *Statptr;	 /* This makes the Ultrix compiler happy.  Go figure. */

/* modifier for unit conversions */

#define TT_DAYS 0
#define TT_HOURS 1
#define TT_MINS 2
#define TT_WEEKS 3
#define TT_MONTHS 4
#define TT_SECONDS 5

#define TT_BYTES 0
#define TT_POSIX_BLOCKS 1
#define TT_KILOBYTES 2
#define TT_MEGABYTES 3
#define TT_GIGABYTES 4
#define TT_TERABYTES 5


typedef int (*TestMatchFunc) _((char *, struct stat *, off_t, char *));

struct qual {
    struct qual *next;		/* Next qualifier, must match                */
    struct qual *or;		/* Alternative set of qualifiers to match    */
    TestMatchFunc func;		/* Function to call to test match            */
    off_t data ALIGN64;		/* Argument passed to function               */
    int sense;			/* Whether asserting or negating             */
    int amc;			/* Flag for which time to test (a, m, c)     */
    int range;			/* Whether to test <, > or = (as per signum) */
    int units;			/* Multiplier for time or size, respectively */
    char *sdata;		/* currently only: expression to eval        */
};

/* Prefix, suffix for doing zle trickery */

/**/
mod_export char *glob_pre, *glob_suf;

/* Element of a glob sort */
struct globsort {
    /* Sort type */
    int tp;
    /* Sort code to eval, if type is GS_EXEC */
    char *exec;
};

/* Maximum entries in sort array */
#define MAX_SORTS	(12)

/* struct to easily save/restore current state */

struct globdata {
    int gd_pathpos;
    char *gd_pathbuf;

    int gd_matchsz;		/* size of matchbuf                     */
    int gd_matchct;		/* number of matches found              */
    int gd_pathbufsz;		/* size of pathbuf			*/
    int gd_pathbufcwd;		/* where did we chdir()'ed		*/
    Gmatch gd_matchbuf;		/* array of matches                     */
    Gmatch gd_matchptr;		/* &matchbuf[matchct]                   */
    char *gd_colonmod;		/* colon modifiers in qualifier list    */

    /* Qualifiers pertaining to current pattern */
    struct qual *gd_quals;

    /* Other state values for current pattern */
    int gd_qualct, gd_qualorct;
    int gd_range, gd_amc, gd_units;
    int gd_gf_nullglob, gd_gf_markdirs, gd_gf_noglobdots, gd_gf_listtypes;
    int gd_gf_numsort;
    int gd_gf_follow, gd_gf_sorts, gd_gf_nsorts;
    struct globsort gd_gf_sortlist[MAX_SORTS];
    LinkList gd_gf_pre_words, gd_gf_post_words;

    char *gd_glob_pre, *gd_glob_suf;
};

/* The variable with the current globbing state and convenience macros */

static struct globdata curglobdata;

#define matchsz       (curglobdata.gd_matchsz)
#define matchct       (curglobdata.gd_matchct)
#define pathbufsz     (curglobdata.gd_pathbufsz)
#define pathbufcwd    (curglobdata.gd_pathbufcwd)
#define matchbuf      (curglobdata.gd_matchbuf)
#define matchptr      (curglobdata.gd_matchptr)
#define colonmod      (curglobdata.gd_colonmod)
#define quals         (curglobdata.gd_quals)
#define qualct        (curglobdata.gd_qualct)
#define qualorct      (curglobdata.gd_qualorct)
#define g_range       (curglobdata.gd_range)
#define g_amc         (curglobdata.gd_amc)
#define g_units       (curglobdata.gd_units)
#define gf_nullglob   (curglobdata.gd_gf_nullglob)
#define gf_markdirs   (curglobdata.gd_gf_markdirs)
#define gf_noglobdots (curglobdata.gd_gf_noglobdots)
#define gf_listtypes  (curglobdata.gd_gf_listtypes)
#define gf_numsort    (curglobdata.gd_gf_numsort)
#define gf_follow     (curglobdata.gd_gf_follow)
#define gf_sorts      (curglobdata.gd_gf_sorts)
#define gf_nsorts     (curglobdata.gd_gf_nsorts)
#define gf_sortlist   (curglobdata.gd_gf_sortlist)
#define gf_pre_words  (curglobdata.gd_gf_pre_words)
#define gf_post_words (curglobdata.gd_gf_post_words)

/* and macros for save/restore */

#define save_globstate(N) \
  do { \
    queue_signals(); \
    memcpy(&(N), &curglobdata, sizeof(struct globdata)); \
    (N).gd_pathpos = pathpos; \
    (N).gd_pathbuf = pathbuf; \
    (N).gd_glob_pre = glob_pre; \
    (N).gd_glob_suf = glob_suf; \
    pathbuf = NULL; \
    unqueue_signals(); \
  } while (0)

#define restore_globstate(N) \
  do { \
    queue_signals(); \
    zfree(pathbuf, pathbufsz); \
    memcpy(&curglobdata, &(N), sizeof(struct globdata)); \
    pathpos = (N).gd_pathpos; \
    pathbuf = (N).gd_pathbuf; \
    glob_pre = (N).gd_glob_pre; \
    glob_suf = (N).gd_glob_suf; \
    unqueue_signals(); \
  } while (0)

/* pathname component in filename patterns */

struct complist {
    Complist next;
    Patprog pat;
    int closure;		/* 1 if this is a (foo/)# */
    int follow; 		/* 1 to go thru symlinks */
};

/* Add a component to pathbuf: This keeps track of how    *
 * far we are into a file name, since each path component *
 * must be matched separately.                            */

/**/
static void
addpath(char *s, int l)
{
    DPUTS(!pathbuf, "BUG: pathbuf not initialised");
    while (pathpos + l + 1 >= pathbufsz)
	pathbuf = zrealloc(pathbuf, pathbufsz *= 2);
    while (l--)
	pathbuf[pathpos++] = *s++;
    pathbuf[pathpos++] = '/';
    pathbuf[pathpos] = '\0';
}

/* stat the filename s appended to pathbuf.  l should be true for lstat,    *
 * false for stat.  If st is NULL, the file is only checked for existence.  *
 * s == "" is treated as s == ".".  This is necessary since on most systems *
 * foo/ can be used to reference a non-directory foo.  Returns nonzero if   *
 * the file does not exists.                                                */

static int
statfullpath(const char *s, struct stat *st, int l)
{
    char buf[PATH_MAX+1];
    int check_for_being_a_directory = 0;

    DPUTS(strlen(s) + !*s + pathpos - pathbufcwd >= PATH_MAX,
	  "BUG: statfullpath(): pathname too long");
    strcpy(buf, pathbuf + pathbufcwd);
    strcpy(buf + pathpos - pathbufcwd, s);
    if (!*s && *buf) {
	/*
	 * Don't add the '.' if the path so far is empty, since
	 * then we get bogus empty strings inserted as files.
	 */
	if (st) {
	    buf[pathpos - pathbufcwd] = '.';
	    buf[pathpos - pathbufcwd + 1] = '\0';
	    l = 0;
	}
	else {
	    check_for_being_a_directory = 1;
	}
    }
    unmetafy(buf, NULL);
    if (st) {
	return l ? lstat(buf, st) : stat(buf, st);
    }
    else if (check_for_being_a_directory) {
	struct stat tmp;
	if (stat(buf, &tmp))
	    return -1;

	return S_ISDIR(tmp.st_mode) ? 0 : -1;
    }
    else {
	char lbuf[1];

	/* If it exists, signal success. */
	if (access(buf, F_OK) == 0)
	    return 0;

	/* Would a dangling symlink be good enough? */
	if (l == 0)
	    return -1;

	/* Is it a dangling symlink? */
	if (readlink(buf, lbuf, 1) >= 0)
	    return 0;

	/* Guess it doesn't exist, then. */
	return -1;
    }
}

/* This may be set by qualifier functions to an array of strings to insert
 * into the list instead of the original string. */

static char **inserts;

/* add a match to the list */

/**/
static void
insert(char *s, int checked)
{
    struct stat buf, buf2, *bp;
    char *news = s;
    int statted = 0;

    queue_signals();
    inserts = NULL;

    if (gf_listtypes || gf_markdirs) {
	/* Add the type marker to the end of the filename */
	mode_t mode;
	if (statfullpath(s, &buf, 1)) {
	    unqueue_signals();
	    return;
	}
	else {
	    checked = statted = 1;
	}
	mode = buf.st_mode;
	if (gf_follow) {
	    if (!S_ISLNK(mode) || statfullpath(s, &buf2, 0))
		memcpy(&buf2, &buf, sizeof(buf));
	    statted |= 2;
	    mode = buf2.st_mode;
	}
	if (gf_listtypes || S_ISDIR(mode)) {
	    int ll = strlen(s);

	    news = (char *) hcalloc(ll + 2);
	    strcpy(news, s);
	    news[ll] = file_type(mode);
	    news[ll + 1] = '\0';
	}
    }
    if (qualct || qualorct) {
	/* Go through the qualifiers, rejecting the file if appropriate */
	struct qual *qo, *qn;

	if (!statted && statfullpath(s, &buf, 1)) {
	    unqueue_signals();
	    return;
	}
	news = dyncat(pathbuf, news);

	statted = 1;
	qo = quals;
	for (qn = qo; qn && qn->func;) {
	    g_range = qn->range;
	    g_amc = qn->amc;
	    g_units = qn->units;
	    if ((qn->sense & 2) && !(statted & 2)) {
		/* If (sense & 2), we're following links */
		if (!S_ISLNK(buf.st_mode) || statfullpath(s, &buf2, 0))
		    memcpy(&buf2, &buf, sizeof(buf));
		statted |= 2;
	    }
	    bp = (qn->sense & 2) ? &buf2 : &buf;
	    /* Reject the file if the function returned zero *
	     * and the sense was positive (sense&1 == 0), or *
	     * vice versa.                                   */
	    if ((!((qn->func) (news, bp, qn->data, qn->sdata))
		 ^ qn->sense) & 1) {
		/* Try next alternative, or return if there are no more */
		if (!(qo = qo->or)) {
		    unqueue_signals();
		    return;
		}
		qn = qo;
		continue;
	    }
	    qn = qn->next;
	}
    } else if (!checked) {
	if (statfullpath(s, NULL, 1)) {
	    unqueue_signals();
	    return;
	}
	news = dyncat(pathbuf, news);
    } else
	news = dyncat(pathbuf, news);

    while (!inserts || (news = dupstring(*inserts++))) {
	if (colonmod) {
	    /* Handle the remainder of the qualifier:  e.g. (:r:s/foo/bar/). */
	    char *mod = colonmod;
	    modify(&news, &mod, 1);
	}
	if (!statted && (gf_sorts & GS_NORMAL)) {
	    statfullpath(s, &buf, 1);
	    statted = 1;
	}
	if (!(statted & 2) && (gf_sorts & GS_LINKED)) {
	    if (statted) {
		if (!S_ISLNK(buf.st_mode) || statfullpath(s, &buf2, 0))
		    memcpy(&buf2, &buf, sizeof(buf));
	    } else if (statfullpath(s, &buf2, 0))
		statfullpath(s, &buf2, 1);
	    statted |= 2;
	}
	matchptr->name = news;
	if (statted & 1) {
	    matchptr->size = buf.st_size;
	    matchptr->atime = buf.st_atime;
	    matchptr->mtime = buf.st_mtime;
	    matchptr->ctime = buf.st_ctime;
	    matchptr->links = buf.st_nlink;
#ifdef GET_ST_ATIME_NSEC
	    matchptr->ansec = GET_ST_ATIME_NSEC(buf);
#endif
#ifdef GET_ST_MTIME_NSEC
	    matchptr->mnsec = GET_ST_MTIME_NSEC(buf);
#endif
#ifdef GET_ST_CTIME_NSEC
	    matchptr->cnsec = GET_ST_CTIME_NSEC(buf);
#endif
	}
	if (statted & 2) {
	    matchptr->_size = buf2.st_size;
	    matchptr->_atime = buf2.st_atime;
	    matchptr->_mtime = buf2.st_mtime;
	    matchptr->_ctime = buf2.st_ctime;
	    matchptr->_links = buf2.st_nlink;
#ifdef GET_ST_ATIME_NSEC
	    matchptr->_ansec = GET_ST_ATIME_NSEC(buf2);
#endif
#ifdef GET_ST_MTIME_NSEC
	    matchptr->_mnsec = GET_ST_MTIME_NSEC(buf2);
#endif
#ifdef GET_ST_CTIME_NSEC
	    matchptr->_cnsec = GET_ST_CTIME_NSEC(buf2);
#endif
	}
	matchptr++;

	if (++matchct == matchsz) {
	    matchbuf = (Gmatch)zrealloc((char *)matchbuf,
					sizeof(struct gmatch) * (matchsz *= 2));

	    matchptr = matchbuf + matchct;
	}
	if (!inserts)
	    break;
    }
    unqueue_signals();
    return;
}

/* Do the globbing:  scanner is called recursively *
 * with successive bits of the path until we've    *
 * tried all of it.                                */

/**/
static void
scanner(Complist q, int shortcircuit)
{
    Patprog p;
    int closure;
    int pbcwdsav = pathbufcwd;
    int errssofar = errsfound;
    struct dirsav ds;

    if (!q || errflag)
	return;
    init_dirsav(&ds);

    if ((closure = q->closure)) {
	/* (foo/)# - match zero or more dirs */
	if (q->closure == 2)	/* (foo/)## - match one or more dirs */
	    q->closure = 1;
	else {
	    scanner(q->next, shortcircuit);
	    if (shortcircuit && shortcircuit == matchct)
		return;
	}
    }
    p = q->pat;
    /* Now the actual matching for the current path section. */
    if (p->flags & PAT_PURES) {
	/*
	 * It's a straight string to the end of the path section.
	 */
	char *str = (char *)p + p->startoff;
	int l = p->patmlen;

	if (l + !l + pathpos - pathbufcwd >= PATH_MAX) {
	    int err;

	    if (l >= PATH_MAX)
		return;
	    err = lchdir(unmeta(pathbuf + pathbufcwd), &ds, 0);
	    if (err == -1)
		return;
	    if (err) {
		zerr("current directory lost during glob");
		return;
	    }
	    pathbufcwd = pathpos;
	}
	if (q->next) {
	    /* Not the last path section. Just add it to the path. */
	    int oppos = pathpos;

	    if (!errflag) {
		int add = 1;

		if (q->closure && *pathbuf) {
		    if (!strcmp(str, "."))
			add = 0;
		    else if (!strcmp(str, "..")) {
			struct stat sc, sr;

			add = (stat("/", &sr) || stat(unmeta(pathbuf), &sc) ||
			       sr.st_ino != sc.st_ino ||
			       sr.st_dev != sc.st_dev);
		    }
		}
		if (add) {
		    addpath(str, l);
		    if (!closure || !statfullpath("", NULL, 1)) {
			scanner((q->closure) ? q : q->next, shortcircuit);
			if (shortcircuit && shortcircuit == matchct)
			    return;
		    }
		    pathbuf[pathpos = oppos] = '\0';
		}
	    }
	} else {
	    if (str[l])
		str = dupstrpfx(str, l);
	    insert(str, 0);
	    if (shortcircuit && shortcircuit == matchct)
		return;
	}
    } else {
	/* Do pattern matching on current path section. */
	char *fn = pathbuf[pathbufcwd] ? unmeta(pathbuf + pathbufcwd) : ".";
	int dirs = !!q->next;
	DIR *lock = opendir(fn);
	char *subdirs = NULL;
	int subdirlen = 0;

	if (lock == NULL)
	    return;
	while ((fn = zreaddir(lock, 1)) && !errflag) {
	    /* prefix and suffix are zle trickery */
	    if (!dirs && !colonmod &&
		((glob_pre && !strpfx(glob_pre, fn))
		 || (glob_suf && !strsfx(glob_suf, fn))))
		continue;
	    errsfound = errssofar;
	    if (pattry(p, fn)) {
		/* if this name matches the pattern... */
		if (pbcwdsav == pathbufcwd &&
		    strlen(fn) + pathpos - pathbufcwd >= PATH_MAX) {
		    int err;

		    DPUTS(pathpos == pathbufcwd,
			  "BUG: filename longer than PATH_MAX");
		    err = lchdir(unmeta(pathbuf + pathbufcwd), &ds, 0);
		    if (err == -1)
			break;
		    if (err) {
			zerr("current directory lost during glob");
			break;
		    }
		    pathbufcwd = pathpos;
		}
		if (dirs) {
		    int l;

		    /*
		     * If not the last component in the path:
		     *
		     * If we made an approximation in the new path segment,
		     * then it is possible we made too many errors.  For
		     * example, (ab)#(cb)# will match the directory abcb
		     * with one error if allowed to, even though it can
		     * match with none.  This will stop later parts of the
		     * path matching, so we need to check by reducing the
		     * maximum number of errors and seeing if the directory
		     * still matches.  Luckily, this is not a terribly
		     * common case, since complex patterns typically occur
		     * in the last part of the path which is not affected
		     * by this problem.
		     */
		    if (errsfound > errssofar) {
			forceerrs = errsfound - 1;
			while (forceerrs >= errssofar) {
			    errsfound = errssofar;
			    if (!pattry(p, fn))
				break;
			    forceerrs = errsfound - 1;
			}
			errsfound = forceerrs + 1;
			forceerrs = -1;
		    }
		    if (closure) {
			/* if matching multiple directories */
			struct stat buf;

			if (statfullpath(fn, &buf, !q->follow)) {
			    if (errno != ENOENT && errno != EINTR &&
				errno != ENOTDIR && !errflag) {
				zwarn("%e: %s", errno, fn);
			    }
			    continue;
			}
			if (!S_ISDIR(buf.st_mode))
			    continue;
		    }
		    l = strlen(fn) + 1;
		    subdirs = hrealloc(subdirs, subdirlen, subdirlen + l
				       + sizeof(int));
		    strcpy(subdirs + subdirlen, fn);
		    subdirlen += l;
		    /* store the count of errors made so far, too */
		    memcpy(subdirs + subdirlen, (char *)&errsfound,
			   sizeof(int));
		    subdirlen += sizeof(int);
		} else {
		    /* if the last filename component, just add it */
		    insert(fn, 1);
		    if (shortcircuit && shortcircuit == matchct) {
			closedir(lock);
			return;
		    }
		}
	    }
	}
	closedir(lock);
	if (subdirs) {
	    int oppos = pathpos;

	    for (fn = subdirs; fn < subdirs+subdirlen; ) {
		int l = strlen(fn);
		addpath(fn, l);
		fn += l + 1;
		memcpy((char *)&errsfound, fn, sizeof(int));
		fn += sizeof(int);
		/* scan next level */
		scanner((q->closure) ? q : q->next, shortcircuit); 
		if (shortcircuit && shortcircuit == matchct)
		    return;
		pathbuf[pathpos = oppos] = '\0';
	    }
	    hrealloc(subdirs, subdirlen, 0);
	}
    }
    if (pbcwdsav < pathbufcwd) {
	if (restoredir(&ds))
	    zerr("current directory lost during glob");
	zsfree(ds.dirname);
	if (ds.dirfd >= 0)
	    close(ds.dirfd);
	pathbufcwd = pbcwdsav;
    }
    return;
}

/* This function tokenizes a zsh glob pattern */

/**/
static Complist
parsecomplist(char *instr)
{
    Patprog p1;
    Complist l1;
    char *str;
    int compflags = gf_noglobdots ? (PAT_FILE|PAT_NOGLD) : PAT_FILE;

    if (instr[0] == Star && instr[1] == Star) {
	int shortglob = 0;
        if (instr[2] == '/' || (instr[2] == Star && instr[3] == '/')
	    || (shortglob = isset(GLOBSTARSHORT))) {
	    /* Match any number of directories. */
	    int follow;

	    /* with three stars, follow symbolic links */
	    follow = (instr[2] == Star);
	    /*
	     * With GLOBSTARSHORT, leave a star in place for the
	     * pattern inside the directory.
	     */
	    instr += ((shortglob ? 1 : 3) + follow);

	    /* Now get the next path component if there is one. */
	    l1 = (Complist) zhalloc(sizeof *l1);
	    if ((l1->next = parsecomplist(instr)) == NULL) {
		errflag |= ERRFLAG_ERROR;
		return NULL;
	    }
	    l1->pat = patcompile(NULL, compflags | PAT_ANY, NULL);
	    l1->closure = 1;		/* ...zero or more times. */
	    l1->follow = follow;
	    return l1;
	}
    }

    /* Parse repeated directories such as (dir/)# and (dir/)## */
    if (*(str = instr) == zpc_special[ZPC_INPAR] &&
	!skipparens(Inpar, Outpar, (char **)&str) &&
        *str == zpc_special[ZPC_HASH] && str[-2] == '/') {
	instr++;
	if (!(p1 = patcompile(instr, compflags, &instr)))
	    return NULL;
	if (instr[0] == '/' && instr[1] == Outpar && instr[2] == Pound) {
	    int pdflag = 0;

	    instr += 3;
	    if (*instr == Pound) {
		pdflag = 1;
		instr++;
	    }
	    l1 = (Complist) zhalloc(sizeof *l1);
	    l1->pat = p1;
	    /* special case (/)# to avoid infinite recursion */
	    l1->closure = (*((char *)p1 + p1->startoff)) ? 1 + pdflag : 0;
	    l1->follow = 0;
	    l1->next = parsecomplist(instr);
	    return (l1->pat) ? l1 : NULL;
	}
    } else {
	/* parse single path component */
	if (!(p1 = patcompile(instr, compflags|PAT_FILET, &instr)))
	    return NULL;
	/* then do the remaining path components */
	if (*instr == '/' || !*instr) {
	    int ef = *instr == '/';

	    l1 = (Complist) zhalloc(sizeof *l1);
	    l1->pat = p1;
	    l1->closure = 0;
	    l1->next = ef ? parsecomplist(instr+1) : NULL;
	    return (ef && !l1->next) ? NULL : l1;
	}
    }
    errflag |= ERRFLAG_ERROR;
    return NULL;
}

/* turn a string into a Complist struct:  this has path components */

/**/
static Complist
parsepat(char *str)
{
    long assert;
    int ignore;

    patcompstart();
    /*
     * Check for initial globbing flags, so that they don't form
     * a bogus path component.
     */
    if ((*str == zpc_special[ZPC_INPAR] && str[1] == zpc_special[ZPC_HASH]) ||
	(*str == zpc_special[ZPC_KSH_AT] && str[1] == Inpar &&
	 str[2] == zpc_special[ZPC_HASH])) {
	str += (*str == Inpar) ? 2 : 3;
	if (!patgetglobflags(&str, &assert, &ignore))
	    return NULL;
    }

    /* Now there is no (#X) in front, we can check the path. */
    if (!pathbuf)
	pathbuf = zalloc(pathbufsz = PATH_MAX+1);
    DPUTS(pathbufcwd, "BUG: glob changed directory");
    if (*str == '/') {		/* pattern has absolute path */
	str++;
	pathbuf[0] = '/';
	pathbuf[pathpos = 1] = '\0';
    } else			/* pattern is relative to pwd */
	pathbuf[pathpos = 0] = '\0';

    return parsecomplist(str);
}

/* get number after qualifier */

/**/
static off_t
qgetnum(char **s)
{
    off_t v = 0;

    if (!idigit(**s)) {
	zerr("number expected");
	return 0;
    }
    while (idigit(**s))
	v = v * 10 + *(*s)++ - '0';
    return v;
}

/* get mode spec after qualifier */

/**/
static zlong
qgetmodespec(char **s)
{
    zlong yes = 0, no = 0, val, mask, t;
    char *p = *s, c, how, end;

    if ((c = *p) == '=' || c == Equals || c == '+' || c == '-' ||
	c == '?' || c == Quest || (c >= '0' && c <= '7')) {
	end = 0;
	c = 0;
    } else {
	end = (c == '<' ? '>' :
	       (c == '[' ? ']' :
		(c == '{' ? '}' :
		 (c == Inang ? Outang :
		  (c == Inbrack ? Outbrack :
		   (c == Inbrace ? Outbrace : c))))));
	p++;
    }
    do {
	mask = 0;
	while (((c = *p) == 'u' || c == 'g' || c == 'o' || c == 'a') && end) {
	    switch (c) {
	    case 'o': mask |= 01007; break;
	    case 'g': mask |= 02070; break;
	    case 'u': mask |= 04700; break;
	    case 'a': mask |= 07777; break;
	    }
	    p++;
	}
	how = ((c == '+' || c == '-') ? c : '=');
	if (c == '+' || c == '-' || c == '=' || c == Equals)
	    p++;
	val = 0;
	if (mask) {
	    while ((c = *p++) != ',' && c != end) {
		switch (c) {
		case 'x': val |= 00111; break;
		case 'w': val |= 00222; break;
		case 'r': val |= 00444; break;
		case 's': val |= 06000; break;
		case 't': val |= 01000; break;
		case '0': case '1': case '2': case '3':
		case '4': case '5': case '6': case '7':
		    t = ((zlong) c - '0');
		    val |= t | (t << 3) | (t << 6);
		    break;
		default:
		    zerr("invalid mode specification");
		    return 0;
		}
	    }
	    if (how == '=' || how == '+') {
		yes |= val & mask;
		val = ~val;
	    }
	    if (how == '=' || how == '-')
		no |= val & mask;
	} else if (!(end && c == end) && c != ',' && c) {
	    t = 07777;
	    while ((c = *p) == '?' || c == Quest ||
		   (c >= '0' && c <= '7')) {
		if (c == '?' || c == Quest) {
		    t = (t << 3) | 7;
		    val <<= 3;
		} else {
		    t <<= 3;
		    val = (val << 3) | ((zlong) c - '0');
		}
		p++;
	    }
	    if (end && c != end && c != ',') {
		zerr("invalid mode specification");
		return 0;
	    }
	    if (how == '=') {
		yes = (yes & ~t) | val;
		no = (no & ~t) | (~val & ~t);
	    } else if (how == '+')
		yes |= val;
	    else
		no |= val;
	} else {
	    zerr("invalid mode specification");
	    return 0;
        }
    } while (end && c != end);

    *s = p;
    return ((yes & 07777) | ((no & 07777) << 12));
}

static int
gmatchcmp(Gmatch a, Gmatch b)
{
    int i;
    off_t r = 0L;
    struct globsort *s;
    char **asortstrp = NULL, **bsortstrp = NULL;

    for (i = gf_nsorts, s = gf_sortlist; i; i--, s++) {
	switch (s->tp & ~GS_DESC) {
	case GS_NAME:
	    r = zstrcmp(b->uname, a->uname,
			gf_numsort ? SORTIT_NUMERICALLY : 0);
	    break;
	case GS_DEPTH:
	    {
		char *aptr = a->name, *bptr = b->name;
		int slasha = 0, slashb = 0;
		/* Count slashes.  Trailing slashes don't count. */
		while (*aptr && *aptr == *bptr)
		    aptr++, bptr++;
		/* Like I just said... */
		if ((!*aptr || !*bptr) && aptr > a->name && aptr[-1] == '/')
		    aptr--, bptr--;
		if (*aptr)
		    for (; aptr[1]; aptr++)
			if (*aptr == '/') {
			    slasha = 1;
			    break;
			}
		if (*bptr)
		    for (; bptr[1]; bptr++)
			if (*bptr == '/') {
			    slashb = 1;
			    break;
			}
		r = slasha - slashb;
	    }
	    break;
	case GS_EXEC:
	    if (!asortstrp) {
		asortstrp = a->sortstrs;
		bsortstrp = b->sortstrs;
	    } else {
		asortstrp++;
		bsortstrp++;
	    }
	    r = zstrcmp(*bsortstrp, *asortstrp,
			gf_numsort ? SORTIT_NUMERICALLY : 0);
	    break;
	case GS_SIZE:
	    r = b->size - a->size;
	    break;
	case GS_ATIME:
	    r = a->atime - b->atime;
#ifdef GET_ST_ATIME_NSEC
            if (!r)
              r = a->ansec - b->ansec;
#endif
	    break;
	case GS_MTIME:
	    r = a->mtime - b->mtime;
#ifdef GET_ST_MTIME_NSEC
            if (!r)
              r = a->mnsec - b->mnsec;
#endif
	    break;
	case GS_CTIME:
	    r = a->ctime - b->ctime;
#ifdef GET_ST_CTIME_NSEC
            if (!r)
              r = a->cnsec - b->cnsec;
#endif
	    break;
	case GS_LINKS:
	    r = b->links - a->links;
	    break;
	case GS__SIZE:
	    r = b->_size - a->_size;
	    break;
	case GS__ATIME:
	    r = a->_atime - b->_atime;
#ifdef GET_ST_ATIME_NSEC
            if (!r)
              r = a->_ansec - b->_ansec;
#endif
	    break;
	case GS__MTIME:
	    r = a->_mtime - b->_mtime;
#ifdef GET_ST_MTIME_NSEC
            if (!r)
              r = a->_mnsec - b->_mnsec;
#endif
	    break;
	case GS__CTIME:
	    r = a->_ctime - b->_ctime;
#ifdef GET_ST_CTIME_NSEC
            if (!r)
              r = a->_cnsec - b->_cnsec;
#endif
	    break;
	case GS__LINKS:
	    r = b->_links - a->_links;
	    break;
	}
	if (r)
	    return (s->tp & GS_DESC) ?
	      (r < 0L ? 1 : -1) :
	      (r > 0L ? 1 : -1);
    }
    return 0;
}

/*
 * Duplicate a list of qualifiers using the `next' linkage (not the
 * `or' linkage).  Return the head element and set *last (if last non-NULL)
 * to point to the last element of the new list.  All allocation is on the
 * heap (or off the heap?)
 */
static struct qual *dup_qual_list(struct qual *orig, struct qual **lastp)
{
    struct qual *qfirst = NULL, *qlast = NULL;

    while (orig) {
	struct qual *qnew = (struct qual *)zhalloc(sizeof(struct qual));
	*qnew = *orig;
	qnew->next = qnew->or = NULL;

	if (!qfirst)
	    qfirst = qnew;
	if (qlast)
	    qlast->next = qnew;
	qlast = qnew;

	orig = orig->next;
    }

    if (lastp)
	*lastp = qlast;
    return qfirst;
}


/*
 * Get a glob string for execution, following e, P or + qualifiers.
 * Pointer is character after the e, P or +.
 */

/**/
static char *
glob_exec_string(char **sp)
{
    char sav, *tt, *sdata, *s = *sp;
    int plus;

    if (s[-1] == '+') {
	plus = 0;
	tt = itype_end(s, IIDENT, 0);
	if (tt == s)
	{
	    zerr("missing identifier after `+'");
	    return NULL;
	}
    } else {
	tt = get_strarg(s, &plus);
	if (!*tt)
	{
	    zerr("missing end of string");
	    return NULL;
	}
    }

    sav = *tt;
    *tt = '\0';
    sdata = dupstring(s + plus);
    untokenize(sdata);
    *tt = sav;
    if (sav)
	*sp = tt + plus;
    else
	*sp = tt;

    return sdata;
}

/*
 * Insert a glob match.
 * If there were words to prepend given by the P glob qualifier, do so.
 */
static void
insert_glob_match(LinkList list, LinkNode next, char *data)
{
    if (gf_pre_words) {
	LinkNode added;
	for (added = firstnode(gf_pre_words); added; incnode(added)) {
	    next = insertlinknode(list, next, dupstring(getdata(added)));
	}
    }

    next = insertlinknode(list, next, data);

    if (gf_post_words) {
	LinkNode added;
	for (added = firstnode(gf_post_words); added; incnode(added)) {
	    next = insertlinknode(list, next, dupstring(getdata(added)));
	}
    }
}

/*
 * Return
 *   1 if str ends in bare glob qualifiers
 *   2 if str ends in non-bare glob qualifiers (#q)
 *   0 otherwise.
 *
 * str is the string to check.
 * sl is its length (to avoid recalculation).
 * nobareglob is 1 if bare glob qualifiers are not allowed.
 * *sp, if sp is not null, will be a pointer to the opening parenthesis.
 */

/**/
int
checkglobqual(char *str, int sl, int nobareglob, char **sp)
{
    char *s;
    int paren, ret = 1;

    if (str[sl - 1] != Outpar)
	return 0;

    /* Check these are really qualifiers, not a set of *
     * alternatives or exclusions.  We can be more     *
     * lenient with an explicit (#q) than with a bare  *
     * set of qualifiers.                              */
    paren = 0;
    for (s = str + sl - 2; *s && (*s != Inpar || paren); s--) {
	switch (*s) {
	case Outpar:
	    paren++; /*FALLTHROUGH*/
	case Bar:
	    if (!zpc_disables[ZPC_BAR])
		nobareglob = 1;
	    break;
	case Tilde:
	    if (isset(EXTENDEDGLOB) && !zpc_disables[ZPC_TILDE])
		nobareglob = 1;
	    break;
	case Inpar:
	    paren--;
	    break;
	}
	if (s == str)
	    break;
    }
    if (*s != Inpar)
	return 0;
    if (isset(EXTENDEDGLOB) && !zpc_disables[ZPC_HASH] && s[1] == Pound) {
	if (s[2] != 'q')
	    return 0;
	ret = 2;
    } else if (nobareglob)
	return 0;

    if (sp)
	*sp = s;

    return ret;
}

/* Main entry point to the globbing code for filename globbing. *
 * np points to a node in the list which will be expanded  *
 * into a series of nodes.                                      */

/**/
void
zglob(LinkList list, LinkNode np, int nountok)
{
    struct qual *qo, *qn, *ql;
    LinkNode node = prevnode(np);
    char *str;				/* the pattern                   */
    int sl;				/* length of the pattern         */
    Complist q;				/* pattern after parsing         */
    char *ostr = (char *)getdata(np);	/* the pattern before the parser */
					/* chops it up                   */
    int first = 0, end = -1;		/* index of first match to return */
					/* and index+1 of the last match */
    struct globdata saved;		/* saved glob state              */
    int nobareglob = !isset(BAREGLOBQUAL);
    int shortcircuit = 0;		/* How many files to match;      */
					/* 0 means no limit              */

    if (unset(GLOBOPT) || !haswilds(ostr) || unset(EXECOPT)) {
	if (!nountok)
	    untokenize(ostr);
	return;
    }
    save_globstate(saved);

    str = dupstring(ostr);
    uremnode(list, np);

    /* quals will hold the complete list of qualifiers (file static). */
    quals = NULL;
    /*
     * qualct and qualorct indicate we have qualifiers in the last
     * alternative, or a set of alternatives, respectively.  They
     * are not necessarily an accurate count, however.
     */
    qualct = qualorct = 0;
    /*
     * colonmod is a concatenated list of all colon modifiers found in
     * all sets of qualifiers.
     */
    colonmod = NULL;
    /* The gf_* flags are qualifiers which are applied globally. */
    gf_nullglob = isset(NULLGLOB);
    gf_markdirs = isset(MARKDIRS);
    gf_listtypes = gf_follow = 0;
    gf_noglobdots = unset(GLOBDOTS);
    gf_numsort = isset(NUMERICGLOBSORT);
    gf_sorts = gf_nsorts = 0;
    gf_pre_words = gf_post_words = NULL;

    /* Check for qualifiers */
    while (!nobareglob ||
	   (isset(EXTENDEDGLOB) && !zpc_disables[ZPC_HASH])) {
	struct qual *newquals;
	char *s;
	int sense, qualsfound;
	off_t data;
	char *sdata, *newcolonmod, *ptr;
	int (*func) _((char *, Statptr, off_t, char *));

	/*
	 * Initialise state variables for current file pattern.
	 * newquals is the root for the linked list of all qualifiers.
	 * qo is the root of the current list of alternatives.
	 * ql is the end of the current alternative where the `next' will go.
	 * qn is the current qualifier node to be added.
	 *
	 * Here is an attempt at a diagram.  An `or' is added horizontally
	 * to the top line, a `next' at the bottom of the right hand line.
	 * `qn' is usually NULL unless a new `or' has just been added.
	 *
	 * quals -> x  -> x -> qo
	 *          |     |    |
	 *          x     x    x
	 *          |          |
	 *          x          ql
	 *
	 * In fact, after each loop the complete set is in the file static
	 * `quals'.  Then, if we have a second set of qualifiers, we merge
	 * the lists together.  This is only tricky if one or both have an
	 * `or' in them; then we need to distribute over all alternatives.
	 */
	newquals = qo = qn = ql = NULL;

	sl = strlen(str);
	if (!(qualsfound = checkglobqual(str, sl, nobareglob, &s)))
	    break;

	/* Real qualifiers found. */
	nobareglob = 1;
	sense = 0;	   /* bit 0 for match (0)/don't match (1)   */
			   /* bit 1 for follow links (2), don't (0) */
	data = 0;	   /* Any numerical argument required       */
	sdata = NULL;	   /* Any list argument required            */
	newcolonmod = NULL; /* Contains trailing colon modifiers    */

	str[sl-1] = 0;
	*s++ = 0;
	if (qualsfound == 2)
	    s += 2;
	for (ptr = s; *ptr; ptr++)
	    if (*ptr == Dash)
		*ptr = '-';
	while (*s && !newcolonmod) {
	    func = (int (*) _((char *, Statptr, off_t, char *)))0;
	    if (*s == ',') {
		/* A comma separates alternative sets of qualifiers */
		s++;
		sense = 0;
		if (qualct) {
		    qn = (struct qual *)hcalloc(sizeof *qn);
		    qo->or = qn;
		    qo = qn;
		    qualorct++;
		    qualct = 0;
		    ql = NULL;
		}
	    } else {
		switch (*s++) {
		case ':':
		    /* Remaining arguments are history-type     *
		     * colon substitutions, handled separately. */
		    newcolonmod = s - 1;
		    untokenize(newcolonmod);
		    if (colonmod) {
			/* remember we're searching backwards */
			colonmod = dyncat(newcolonmod, colonmod);
		    } else
			colonmod = newcolonmod;
		    break;
		case Hat:
		case '^':
		    /* Toggle sense:  go from positive to *
		     * negative match and vice versa.     */
		    sense ^= 1;
		    break;
		case '-':
		case Dash:
		    /* Toggle matching of symbolic links */
		    sense ^= 2;
		    break;
		case '@':
		    /* Match symbolic links */
		    func = qualislnk;
		    break;
		case Equals:
		case '=':
		    /* Match sockets */
		    func = qualissock;
		    break;
		case 'p':
		    /* Match named pipes */
		    func = qualisfifo;
		    break;
		case '/':
		    /* Match directories */
		    func = qualisdir;
		    break;
		case '.':
		    /* Match regular files */
		    func = qualisreg;
		    break;
		case '%':
		    /* Match special files: block, *
		     * character or any device     */
		    if (*s == 'b')
			s++, func = qualisblk;
		    else if (*s == 'c')
			s++, func = qualischr;
		    else
			func = qualisdev;
		    break;
		case Star:
		    /* Match executable plain files */
		    func = qualiscom;
		    break;
		case 'R':
		    /* Match world-readable files */
		    func = qualflags;
		    data = 0004;
		    break;
		case 'W':
		    /* Match world-writeable files */
		    func = qualflags;
		    data = 0002;
		    break;
		case 'X':
		    /* Match world-executable files */
		    func = qualflags;
		    data = 0001;
		    break;
		case 'A':
		    func = qualflags;
		    data = 0040;
		    break;
		case 'I':
		    func = qualflags;
		    data = 0020;
		    break;
		case 'E':
		    func = qualflags;
		    data = 0010;
		    break;
		case 'r':
		    /* Match files readable by current process */
		    func = qualflags;
		    data = 0400;
		    break;
		case 'w':
		    /* Match files writeable by current process */
		    func = qualflags;
		    data = 0200;
		    break;
		case 'x':
		    /* Match files executable by current process */
		    func = qualflags;
		    data = 0100;
		    break;
		case 's':
		    /* Match setuid files */
		    func = qualflags;
		    data = 04000;
		    break;
		case 'S':
		    /* Match setgid files */
		    func = qualflags;
		    data = 02000;
		    break;
		case 't':
		    func = qualflags;
		    data = 01000;
		    break;
		case 'd':
		    /* Match device files by device number  *
		     * (as given by stat's st_dev element). */
		    func = qualdev;
		    data = qgetnum(&s);
		    break;
		case 'l':
		    /* Match files with the given no. of hard links */
		    func = qualnlink;
		    g_amc = -1;
		    goto getrange;
		case 'U':
		    /* Match files owned by effective user ID */
		    func = qualuid;
		    data = geteuid();
		    break;
		case 'G':
		    /* Match files owned by effective group ID */
		    func = qualgid;
		    data = getegid();
		    break;
		case 'u':
		    /* Match files owned by given user id */
		    func = qualuid;
		    /* either the actual uid... */
		    if (idigit(*s))
			data = qgetnum(&s);
		    else {
			/* ... or a user name */
			char sav, *tt;
			int arglen;

			/* Find matching delimiters */
			tt = get_strarg(s, &arglen);
			if (!*tt) {
			    zerr("missing delimiter for 'u' glob qualifier");
			    data = 0;
			} else {
#ifdef USE_GETPWNAM
			    struct passwd *pw;
			    sav = *tt;
			    *tt = '\0';

			    if ((pw = getpwnam(s + arglen)))
				data = pw->pw_uid;
			    else {
				zerr("unknown username '%s'", s + arglen);
				data = 0;
			    }
			    *tt = sav;
#else /* !USE_GETPWNAM */
			    sav = *tt;
			    *tt = '\0';
			    zerr("unable to resolve non-numeric username '%s'", s + arglen);
			    *tt = sav;
			    data = 0;
#endif /* !USE_GETPWNAM */
			    if (sav)
				s = tt + arglen;
			    else
				s = tt;
			}
		    }
		    break;
		case 'g':
		    /* Given gid or group id... works like `u' */
		    func = qualgid;
		    /* either the actual gid... */
		    if (idigit(*s))
			data = qgetnum(&s);
		    else {
			/* ...or a delimited group name. */
			char sav, *tt;
			int arglen;

			tt = get_strarg(s, &arglen);
			if (!*tt) {
			    zerr("missing delimiter for 'g' glob qualifier");
			    data = 0;
			} else {
#ifdef USE_GETGRNAM
			    struct group *gr;
			    sav = *tt;
			    *tt = '\0';

			    if ((gr = getgrnam(s + arglen)))
				data = gr->gr_gid;
			    else {
				zerr("unknown group");
				data = 0;
			    }
			    *tt = sav;
#else /* !USE_GETGRNAM */
			    sav = *tt;
			    zerr("unknown group");
			    data = 0;
#endif /* !USE_GETGRNAM */
			    if (sav)
				s = tt + arglen;
			    else
				s = tt;
			}
		    }
		    break;
		case 'f':
		    /* Match modes with chmod-spec. */
		    func = qualmodeflags;
		    data = qgetmodespec(&s);
		    break;
		case 'F':
		    func = qualnonemptydir;
		    break;
		case 'M':
		    /* Mark directories with a / */
		    if ((gf_markdirs = !(sense & 1)))
			gf_follow = sense & 2;
		    break;
		case 'T':
		    /* Mark types in a `ls -F' type fashion */
		    if ((gf_listtypes = !(sense & 1)))
			gf_follow = sense & 2;
		    break;
		case 'N':
		    /* Nullglob:  remove unmatched patterns. */
		    gf_nullglob = !(sense & 1);
		    break;
		case 'D':
		    /* Glob dots: match leading dots implicitly */
		    gf_noglobdots = sense & 1;
		    break;
		case 'n':
		    /* Numeric glob sort */
		    gf_numsort = !(sense & 1);
		    break;
		case 'Y':
		{
		    /* Short circuit: limit number of matches */
		    const char *s_saved = s;
		    shortcircuit = !(sense & 1);
		    if (shortcircuit) {
			/* Parse the argument. */
			data = qgetnum(&s);
			if ((shortcircuit = data) != data) {
			    /* Integer overflow */
			    zerr("value too big: Y%s", s_saved);
			    restore_globstate(saved);
			    return;
			}
		    }
		    break;
		}
		case 'a':
		    /* Access time in given range */
		    g_amc = 0;
		    func = qualtime;
		    goto getrange;
		case 'm':
		    /* Modification time in given range */
		    g_amc = 1;
		    func = qualtime;
		    goto getrange;
		case 'c':
		    /* Inode creation time in given range */
		    g_amc = 2;
		    func = qualtime;
		    goto getrange;
		case 'L':
		    /* File size (Length) in given range */
		    func = qualsize;
		    g_amc = -1;
		    /* Get size multiplier */
		    g_units = TT_BYTES;
		    if (*s == 'p' || *s == 'P')
			g_units = TT_POSIX_BLOCKS, ++s;
		    else if (*s == 'k' || *s == 'K')
			g_units = TT_KILOBYTES, ++s;
		    else if (*s == 'm' || *s == 'M')
			g_units = TT_MEGABYTES, ++s;
#if defined(ZSH_64_BIT_TYPE) || defined(LONG_IS_64_BIT)
                    else if (*s == 'g' || *s == 'G')
                        g_units = TT_GIGABYTES, ++s;
                    else if (*s == 't' || *s == 'T')
                        g_units = TT_TERABYTES, ++s;
#endif
		  getrange:
		    /* Get time multiplier */
		    if (g_amc >= 0) {
			g_units = TT_DAYS;
			if (*s == 'h')
			    g_units = TT_HOURS, ++s;
			else if (*s == 'm')
			    g_units = TT_MINS, ++s;
			else if (*s == 'w')
			    g_units = TT_WEEKS, ++s;
			else if (*s == 'M')
			    g_units = TT_MONTHS, ++s;
			else if (*s == 's')
			    g_units = TT_SECONDS, ++s;
			else if (*s == 'd')
			    ++s;
		    }
		    /* See if it's greater than, equal to, or less than */
		    if ((g_range = *s == '+' ? 1 : IS_DASH(*s) ? -1 : 0))
			++s;
		    data = qgetnum(&s);
		    break;

		case 'o':
		case 'O':
		{
		    int t;
		    char *send;

		    if (gf_nsorts == MAX_SORTS) {
			zerr("too many glob sort specifiers");
			restore_globstate(saved);
			return;
		    }

		    /* usually just one character */
		    send = s+1;
		    switch (*s) {
		    case 'n': t = GS_NAME; break;
		    case 'L': t = GS_SIZE; break;
		    case 'l': t = GS_LINKS; break;
		    case 'a': t = GS_ATIME; break;
		    case 'm': t = GS_MTIME; break;
		    case 'c': t = GS_CTIME; break;
		    case 'd': t = GS_DEPTH; break;
		    case 'N': t = GS_NONE; break;
		    case 'e':
		    case '+':
		    {
			t = GS_EXEC;
			if ((gf_sortlist[gf_nsorts].exec =
			     glob_exec_string(&send)) == NULL)
			{
			    restore_globstate(saved);
			    return;
			}
			break;
		    }
		    default:
			zerr("unknown sort specifier");
			restore_globstate(saved);
			return;
		    }
		    if ((sense & 2) &&
			(t & (GS_SIZE|GS_ATIME|GS_MTIME|GS_CTIME|GS_LINKS)))
			t <<= GS_SHIFT; /* HERE: GS_EXEC? */
		    if (t != GS_EXEC) {
			if (gf_sorts & t) {
			    zerr("doubled sort specifier");
			    restore_globstate(saved);
			    return;
			}
		    }
		    gf_sorts |= t;
		    gf_sortlist[gf_nsorts++].tp = t |
			(((sense & 1) ^ (s[-1] == 'O')) ? GS_DESC : 0);
		    s = send;
		    break;
		}
		case '+':
		case 'e':
		{
		    char *tt;

		    tt = glob_exec_string(&s);

		    if (tt == NULL) {
			data = 0;
		    } else {
			func = qualsheval;
			sdata = tt;
		    }
		    break;
		}
		case '[':
		case Inbrack:
		{
		    char *os = --s;
		    struct value v;

		    v.isarr = SCANPM_WANTVALS;
		    v.pm = NULL;
		    v.end = -1;
		    v.flags = 0;
		    if (getindex(&s, &v, 0) || s == os) {
			zerr("invalid subscript");
			restore_globstate(saved);
			return;
		    }
		    first = v.start;
		    end = v.end;
		    break;
		}
		case 'P':
		{
		    char *tt;
		    tt = glob_exec_string(&s);

		    if (tt != NULL)
		    {
			LinkList *words = sense & 1 ? &gf_post_words : &gf_pre_words;
			if (!*words)
			    *words = newlinklist();
			addlinknode(*words, tt);
		    }
		    break;
		}
		default:
		    untokenize(--s);
		    zerr("unknown file attribute: %c", *s);
		    restore_globstate(saved);
		    return;
		}
	    }
	    if (func) {
		/* Requested test is performed by function func */
		if (!qn)
		    qn = (struct qual *)hcalloc(sizeof *qn);
		if (ql)
		    ql->next = qn;
		ql = qn;
		if (!newquals)
		    newquals = qo = qn;
		qn->func = func;
		qn->sense = sense;
		qn->data = data;
		qn->sdata = sdata;
		qn->range = g_range;
		qn->units = g_units;
		qn->amc = g_amc;

		qn = NULL;
		qualct++;
	    }
	    if (errflag) {
		restore_globstate(saved);
		return;
	    }
	}

	if (quals && newquals) {
	    /* Merge previous group of qualifiers with new set. */
	    if (quals->or || newquals->or) {
		/* The hard case. */
		struct qual *qorhead = NULL, *qortail = NULL;
		/*
		 * Distribute in the most trivial way, by creating
		 * all possible combinations of the two sets and chaining
		 * these into one long set of alternatives given
		 * by qorhead and qortail.
		 */
		for (qn = newquals; qn; qn = qn->or) {
		    for (qo = quals; qo; qo = qo->or) {
			struct qual *qfirst, *qlast;
			int islast = !qn->or && !qo->or;
			/* Generate first set of qualifiers... */
			if (islast) {
			    /* Last time round:  don't bother copying. */
			    qfirst = qn;
			    for (qlast = qfirst; qlast->next;
				 qlast = qlast->next)
				;			    
			} else
			    qfirst = dup_qual_list(qn, &qlast);
			/* ... link into new `or' chain ... */
			if (!qorhead)
			    qorhead = qfirst;
			if (qortail)
			    qortail->or = qfirst;
			qortail = qfirst;
			/* ... and concatenate second set. */
			qlast->next = islast ? qo : dup_qual_list(qo, NULL);
		    }
		}
		quals = qorhead;
	    } else {
		/*
		 * Easy: we can just chain the qualifiers together.
		 * This is an optimisation; the code above will work, too.
		 * We retain the original left to right ordering --- remember
		 * we are searching for sets of qualifiers from the right.
		 */
		qn = newquals;
		for ( ; newquals->next; newquals = newquals->next)
		    ;
		newquals->next = quals;
		quals = qn;
	    }
	} else if (newquals)
	    quals = newquals;
    }
    q = parsepat(str);
    if (!q || errflag) {	/* if parsing failed */
	restore_globstate(saved);
	if (unset(BADPATTERN)) {
	    if (!nountok)
		untokenize(ostr);
	    insertlinknode(list, node, ostr);
	    return;
	}
	errflag &= ~ERRFLAG_ERROR;
	zerr("bad pattern: %s", ostr);
	return;
    }
    if (!gf_nsorts) {
	gf_sortlist[0].tp = gf_sorts = (shortcircuit ? GS_NONE : GS_NAME);
	gf_nsorts = 1;
    }
    /* Initialise receptacle for matched files, *
     * expanded by insert() where necessary.    */
    matchptr = matchbuf = (Gmatch)zalloc((matchsz = 16) *
					 sizeof(struct gmatch));
    matchct = 0;
    pattrystart();

    /* The actual processing takes place here: matches go into  *
     * matchbuf.  This is the only top-level call to scanner(). */
    scanner(q, shortcircuit);

    /* Deal with failures to match depending on options */
    if (matchct)
	badcshglob |= 2;	/* at least one cmd. line expansion O.K. */
    else if (!gf_nullglob) {
	if (isset(CSHNULLGLOB)) {
	    badcshglob |= 1;	/* at least one cmd. line expansion failed */
	} else if (isset(NOMATCH)) {
	    zerr("no matches found: %s", ostr);
	    zfree(matchbuf, 0);
	    restore_globstate(saved);
	    return;
	} else {
	    /* treat as an ordinary string */
	    untokenize(matchptr->name = dupstring(ostr));
	    matchptr++;
	    matchct = 1;
	}
    }

    if (!(gf_sortlist[0].tp & GS_NONE)) {
	/*
	 * Get the strings to use for sorting by executing
	 * the code chunk.  We allow more than one of these.
	 */
	int nexecs = 0;
	struct globsort *sortp;
	struct globsort *lastsortp = gf_sortlist + gf_nsorts;
	Gmatch gmptr;

	/* First find out if there are any GS_EXECs, counting them. */
	for (sortp = gf_sortlist; sortp < lastsortp; sortp++)
	{
	    if (sortp->tp & GS_EXEC)
		nexecs++;
	}

	if (nexecs) {
	    Gmatch tmpptr;
	    int iexec = 0;

	    /* Yes; allocate enough space for strings for each */
	    for (tmpptr = matchbuf; tmpptr < matchptr; tmpptr++)
		tmpptr->sortstrs = (char **)zhalloc(nexecs*sizeof(char*));

	    /* Loop over each one, incrementing iexec */
	    for (sortp = gf_sortlist; sortp < lastsortp; sortp++)
	    {
		/* Ignore unless this is a GS_EXEC */
		if (sortp->tp & GS_EXEC) {
		    Eprog prog;

		    if ((prog = parse_string(sortp->exec, 0))) {
			int ef = errflag, lv = lastval;

			/* Parsed OK, execute for each name */
			for (tmpptr = matchbuf; tmpptr < matchptr; tmpptr++) {
			    setsparam("REPLY", ztrdup(tmpptr->name));
			    execode(prog, 1, 0, "globsort");
			    if (!errflag)
				tmpptr->sortstrs[iexec] =
				    dupstring(getsparam("REPLY"));
			    else
				tmpptr->sortstrs[iexec] = tmpptr->name;
			}

			/* Retain any user interrupt error status */
			errflag = ef | (errflag & ERRFLAG_INT);
			lastval = lv;
		    } else {
			/* Failed, let's be safe */
			for (tmpptr = matchbuf; tmpptr < matchptr; tmpptr++)
			    tmpptr->sortstrs[iexec] = tmpptr->name;
		    }

		    iexec++;
		}
	    }
	}

	/*
	 * Where necessary, create unmetafied version of names
	 * for comparison.  If no Meta characters just point
	 * to original string.  All on heap.
	 */
	for (gmptr = matchbuf; gmptr < matchptr; gmptr++)
	{
	    if (strchr(gmptr->name, Meta))
	    {
		int dummy;
		gmptr->uname = dupstring(gmptr->name);
		unmetafy(gmptr->uname, &dummy);
	    } else {
		gmptr->uname = gmptr->name;
	    }
	}

	/* Sort arguments in to lexical (and possibly numeric) order. *
	 * This is reversed to facilitate insertion into the list.    */
	qsort((void *) & matchbuf[0], matchct, sizeof(struct gmatch),
	      (int (*) _((const void *, const void *)))gmatchcmp);
    }

    if (first < 0) {
	first += matchct;
	if (first < 0)
	    first = 0;
    }
    if (end < 0)
	end += matchct + 1;
    else if (end > matchct)
	end = matchct;
    if ((end -= first) > 0) {
	if (gf_sortlist[0].tp & GS_NONE) {
	    /* Match list was never reversed, so insert back to front. */
	    matchptr = matchbuf + matchct - first - 1;
	    while (end-- > 0) {
		/* insert matches in the arg list */
		insert_glob_match(list, node, matchptr->name);
		matchptr--;
	    }
	} else {
	    matchptr = matchbuf + matchct - first - end;
	    while (end-- > 0) {
		/* insert matches in the arg list */
		insert_glob_match(list, node, matchptr->name);
		matchptr++;
	    }
	}
    } else if (!badcshglob && !isset(NOMATCH) && matchct == 1) {
	insert_glob_match(list, node, (--matchptr)->name);
    }
    zfree(matchbuf, 0);

    restore_globstate(saved);
}

/* Return the trailing character for marking file types */

/**/
mod_export char
file_type(mode_t filemode)
{
    if(S_ISBLK(filemode))
	return '#';
    else if(S_ISCHR(filemode))
	return '%';
    else if(S_ISDIR(filemode))
	return '/';
    else if(S_ISFIFO(filemode))
	return '|';
    else if(S_ISLNK(filemode))
	return '@';
    else if(S_ISREG(filemode))
	return (filemode & S_IXUGO) ? '*' : ' ';
    else if(S_ISSOCK(filemode))
	return '=';
    else
	return '?';
}

/* check to see if str is eligible for brace expansion */

/**/
mod_export int
hasbraces(char *str)
{
    char *lbr, *mbr, *comma;

    if (isset(BRACECCL)) {
	/* In this case, any properly formed brace expression  *
	 * will match and expand to the characters in between. */
	int bc, c;

	for (bc = 0; (c = *str); ++str)
	    if (c == Inbrace) {
		if (!bc && str[1] == Outbrace)
		    *str++ = '{', *str = '}';
		else
		    bc++;
	    } else if (c == Outbrace) {
		if (!bc)
		    *str = '}';
		else if (!--bc)
		    return 1;
	    }
	return 0;
    }
    /* Otherwise we need to look for... */
    lbr = mbr = comma = NULL;
    for (;;) {
	switch (*str++) {
	case Inbrace:
	    if (!lbr) {
		if (bracechardots(str-1, NULL, NULL))
		    return 1;
		lbr = str - 1;
		if (IS_DASH(*str))
		    str++;
		while (idigit(*str))
		    str++;
		if (*str == '.' && str[1] == '.') {
		    str++; str++;
		    if (IS_DASH(*str))
			str++;
		    while (idigit(*str))
			str++;
		    if (*str == Outbrace &&
			(idigit(lbr[1]) || idigit(str[-1])))
			return 1;
		    else if (*str == '.' && str[1] == '.') {
			str++; str++;
			if (IS_DASH(*str))
			    str++;
			while (idigit(*str))
			    str++;
			if (*str == Outbrace &&
			    (idigit(lbr[1]) || idigit(str[-1])))
			    return 1;
		    }
		}
	    } else {
		char *s = --str;

		if (skipparens(Inbrace, Outbrace, &str)) {
		    *lbr = *s = '{';
		    if (comma)
			str = comma;
		    if (mbr && mbr < str)
			str = mbr;
		    lbr = mbr = comma = NULL;
		} else if (!mbr)
		    mbr = s;
	    }
	    break;
	case Outbrace:
	    if (!lbr)
		str[-1] = '}';
	    else if (comma)
		return 1;
	    else {
		*lbr = '{';
		str[-1] = '}';
		if (mbr)
		    str = mbr;
		mbr = lbr = NULL;
	    }
	    break;
	case Comma:
	    if (!lbr)
		str[-1] = ',';
	    else if (!comma)
		comma = str - 1;
	    break;
	case '\0':
	    if (lbr)
		*lbr = '{';
	    if (!mbr && !comma)
		return 0;
	    if (comma)
		str = comma;
	    if (mbr && mbr < str)
		str = mbr;
	    lbr = mbr = comma = NULL;
	    break;
	}
    }
}

/* expand stuff like >>*.c */

/**/
int
xpandredir(struct redir *fn, LinkList redirtab)
{
    char *nam;
    struct redir *ff;
    int ret = 0;
    local_list1(fake);

    /* Stick the name in a list... */
    init_list1(fake, fn->name);
    /* ...which undergoes all the usual shell expansions */
    prefork(&fake, isset(MULTIOS) ? 0 : PREFORK_SINGLE, NULL);
    /* Globbing is only done for multios. */
    if (!errflag && isset(MULTIOS))
	globlist(&fake, 0);
    if (errflag)
	return 0;
    if (nonempty(&fake) && !nextnode(firstnode(&fake))) {
	/* Just one match, the usual case. */
	char *s = peekfirst(&fake);
	fn->name = s;
	untokenize(s);
	if (fn->type == REDIR_MERGEIN || fn->type == REDIR_MERGEOUT) {
	    if (IS_DASH(s[0]) && !s[1])
		fn->type = REDIR_CLOSE;
	    else if (s[0] == 'p' && !s[1])
		fn->fd2 = -2;
	    else {
		while (idigit(*s))
		    s++;
		if (!*s && s > fn->name)
		    fn->fd2 = zstrtol(fn->name, NULL, 10);
		else if (fn->type == REDIR_MERGEIN)
		    zerr("file number expected");
		else
		    fn->type = REDIR_ERRWRITE;
	    }
	}
    } else if (fn->type == REDIR_MERGEIN)
	zerr("file number expected");
    else {
	if (fn->type == REDIR_MERGEOUT)
	    fn->type = REDIR_ERRWRITE;
	while ((nam = (char *)ugetnode(&fake))) {
	    /* Loop over matches, duplicating the *
	     * redirection for each file found.   */
	    ff = (struct redir *) zhalloc(sizeof *ff);
	    *ff = *fn;
	    ff->name = nam;
	    addlinknode(redirtab, ff);
	    ret = 1;
	}
    }
    return ret;
}

/*
 * Check for a brace expansion of the form {<char>..<char>}.
 * On input str must be positioned at an Inbrace, but the sequence
 * of characters beyond that has not necessarily been checked.
 * Return 1 if found else 0.
 *
 * The other parameters are optionaland if the function returns 1 are
 * used to return:
 * - *c1p: the first character in the expansion.
 * - *c2p: the final character in the expansion.
 */

/**/
static int
bracechardots(char *str, convchar_t *c1p, convchar_t *c2p)
{
    convchar_t cstart, cend;
    char *pnext = str + 1, *pconv, convstr[2];
    if (itok(*pnext)) {
	if (*pnext == Inbrace)
	    return 0;
	convstr[0] = ztokens[*pnext - Pound];
	convstr[1] = '\0';
	pconv = convstr;
    } else
	pconv = pnext;
    MB_METACHARINIT();
    pnext += MB_METACHARLENCONV(pconv, &cstart);
    if (
#ifdef MULTIBYTE_SUPPORT
	cstart == WEOF ||
#else
	!*pconv ||
#endif
	pnext[0] != '.' || pnext[1] != '.')
	return 0;
    pnext += 2;
    if (!*pnext)
	return 0;
    if (itok(*pnext)) {
	if (*pnext == Inbrace)
	    return 0;
	convstr[0] = ztokens[*pnext - Pound];
	convstr[1] = '\0';
	pconv = convstr;
    } else
	pconv = pnext;
    MB_METACHARINIT();
    pnext += MB_METACHARLENCONV(pconv, &cend);
    if (
#ifdef MULTIBYTE_SUPPORT
	cend == WEOF ||
#else
	!*pconv ||
#endif
	*pnext != Outbrace)
	return 0;
    if (c1p)
	*c1p = cstart;
    if (c2p)
	*c2p = cend;
    return 1;
}

/* brace expansion */

/**/
mod_export void
xpandbraces(LinkList list, LinkNode *np)
{
    LinkNode node = (*np), last = prevnode(node);
    char *str = (char *)getdata(node), *str3 = str, *str2;
    int prev, bc, comma, dotdot;

    for (; *str != Inbrace; str++);
    /* First, match up braces and see what we have. */
    for (str2 = str, bc = comma = dotdot = 0; *str2; ++str2)
	if (*str2 == Inbrace)
	    ++bc;
	else if (*str2 == Outbrace) {
	    if (--bc == 0)
		break;
	} else if (bc == 1) {
	    if (*str2 == Comma)
		++comma;	/* we have {foo,bar} */
	    else if (*str2 == '.' && str2[1] == '.') {
		dotdot++;	/* we have {num1..num2} */
		++str2;
	    }
	}
    DPUTS(bc, "BUG: unmatched brace in xpandbraces()");
    if (!comma && dotdot) {
	/* Expand range like 0..10 numerically: comma or recursive
	   brace expansion take precedence. */
	char *dots, *p, *dots2 = NULL;
	LinkNode olast = last;
	/* Get the first number of the range */
	zlong rstart, rend;
	int err = 0, rev = 0, rincr = 1;
	int wid1, wid2, wid3, strp;
	convchar_t cstart, cend;

	if (bracechardots(str, &cstart, &cend)) {
	    int lenalloc;
	    /*
	     * This is a character range.
	     */
	    if (cend < cstart) {
		convchar_t ctmp = cend;
		cend = cstart;
		cstart = ctmp;
		rev = 1;
	    }
	    uremnode(list, node);
	    strp = str - str3;
	    lenalloc = strp + strlen(str2+1) + 1;
	    do {
		char *ncptr;
		int nclen;
#ifdef MULTIBYTE_SUPPORT
		mb_charinit();
		ncptr = wcs_nicechar(cend, NULL, NULL);
#else
		ncptr = nicechar(cend);
#endif
		nclen = strlen(ncptr);
		p = zhalloc(lenalloc + nclen);
		memcpy(p, str3, strp);
		memcpy(p + strp, ncptr, nclen);
		strcpy(p + strp + nclen, str2 + 1);
		insertlinknode(list, last, p);
		if (rev)	/* decreasing:  add in reverse order. */
		    last = nextnode(last);
	    } while (cend-- > cstart);
	    *np = nextnode(olast);
	    return;
	}

	/* Get the first number of the range */
	rstart = zstrtol(str+1,&dots,10);
	rend = 0;
	wid1 = (dots - str) - 1;
	wid2 = (str2 - dots) - 2;
	wid3 = 0;
	strp = str - str3;

	if (dots == str + 1 || *dots != '.' || dots[1] != '.')
	    err++;
	else {
	    /* Get the last number of the range */
	    rend = zstrtol(dots+2,&p,10);
	    if (p == dots+2)
		err++;
	    /* check for {num1..num2..incr} */
	    if (p != str2) {
		wid2 = (p - dots) - 2;
		dots2 = p;
		if (dotdot == 2 && *p == '.' && p[1] == '.') {
		    rincr = zstrtol(p+2, &p, 10);
		    wid3 = p - dots2 - 2;
		    if (p != str2 || !rincr)
			err++;
		} else
		    err++;
	    }
	}
	if (!err) {
	    /* If either no. begins with a zero, pad the output with   *
	     * zeroes. Otherwise, set min width to 0 to suppress them.
	     * str+1 is the first number in the range, dots+2 the last,
	     * and dots2+2 is the increment if that's given. */
	    /* TODO: sorry about this */
	    int minw = (str[1] == '0' ||
			(IS_DASH(str[1]) && str[2] == '0'))
		       ? wid1
		       : (dots[2] == '0' ||
			  (IS_DASH(dots[2]) && dots[3] == '0'))
		       ? wid2
		       : (dots2 && (dots2[2] == '0' ||
				    (IS_DASH(dots2[2]) && dots2[3] == '0')))
		       ? wid3
		       : 0;
	    if (rincr < 0) {
		/* Handle negative increment */
		rincr = -rincr;
		rev = !rev;
	    }
	    if (rstart > rend) {
		/* Handle decreasing ranges correctly. */
		zlong rt = rend;
		rend = rstart;
		rstart = rt;
		rev = !rev;
	    } else if (rincr > 1) {
		/* when incr > 1, range is aligned to the highest number of str1,
		 * compensate for this so that it is aligned to the first number */
		rend -= (rend - rstart) % rincr;
	    }
	    uremnode(list, node);
	    for (; rend >= rstart; rend -= rincr) {
		/* Node added in at end, so do highest first */
		p = dupstring(str3);
#if defined(ZLONG_IS_LONG_LONG) && defined(PRINTF_HAS_LLD)
		sprintf(p + strp, "%0*lld", minw, rend);
#else
		sprintf(p + strp, "%0*ld", minw, (long)rend);
#endif
		strcat(p + strp, str2 + 1);
		insertlinknode(list, last, p);
		if (rev)	/* decreasing:  add in reverse order. */
		    last = nextnode(last);
	    }
	    *np = nextnode(olast);
	    return;
	}
    }
    if (!comma && isset(BRACECCL)) {	/* {a-mnop} */
	/* Here we expand each character to a separate node,      *
	 * but also ranges of characters like a-m.  ccl is a      *
	 * set of flags saying whether each character is present; *
	 * the final list is in lexical order.                    */
	char ccl[256], *p;
	unsigned char c1, c2;
	unsigned int len, pl;
	int lastch = -1;

	uremnode(list, node);
	memset(ccl, 0, sizeof(ccl) / sizeof(ccl[0]));
	for (p = str + 1; p < str2;) {
	    if (itok(c1 = *p++))
		c1 = ztokens[c1 - STOUC(Pound)];
	    if ((char) c1 == Meta)
		c1 = 32 ^ *p++;
	    if (itok(c2 = *p))
		c2 = ztokens[c2 - STOUC(Pound)];
	    if ((char) c2 == Meta)
		c2 = 32 ^ p[1];
	    if (IS_DASH((char)c1) && lastch >= 0 &&
		p < str2 && lastch <= (int)c2) {
		while (lastch < (int)c2)
		    ccl[lastch++] = 1;
		lastch = -1;
	    } else
		ccl[lastch = c1] = 1;
	}
	pl = str - str3;
	len = pl + strlen(++str2) + 2;
	for (p = ccl + 256; p-- > ccl;)
	    if (*p) {
		c1 = p - ccl;
		if (imeta(c1)) {
		    str = hcalloc(len + 1);
		    str[pl] = Meta;
		    str[pl+1] = c1 ^ 32;
		    strcpy(str + pl + 2, str2);
		} else {
		    str = hcalloc(len);
		    str[pl] = c1;
		    strcpy(str + pl + 1, str2);
		}
		memcpy(str, str3, pl);
		insertlinknode(list, last, str);
	    }
	*np = nextnode(last);
	return;
    }
    prev = str++ - str3;
    str2++;
    uremnode(list, node);
    node = last;
    /* Finally, normal comma expansion               *
     * str1{foo,bar}str2 -> str1foostr2 str1barstr2. *
     * Any number of intervening commas is allowed.  */
    for (;;) {
	char *zz, *str4;
	int cnt;

	for (str4 = str, cnt = 0; cnt || (*str != Comma && *str !=
					  Outbrace); str++) {
	    if (*str == Inbrace)
		cnt++;
	    else if (*str == Outbrace)
		cnt--;
	    DPUTS(!*str, "BUG: illegal brace expansion");
	}
	/* Concatenate the string before the braces (str3), the section *
	 * just found (str4) and the text after the braces (str2)       */
	zz = (char *) hcalloc(prev + (str - str4) + strlen(str2) + 1);
	ztrncpy(zz, str3, prev);
	strncat(zz, str4, str - str4);
	strcat(zz, str2);
	/* and add this text to the argument list. */
	insertlinknode(list, node, zz);
	incnode(node);
	if (*str != Outbrace)
	    str++;
	else
	    break;
    }
    *np = nextnode(last);
}

/* check to see if a matches b (b is not a filename pattern) */

/**/
int
matchpat(char *a, char *b)
{
    Patprog p;
    int ret;

    queue_signals();	/* Protect PAT_STATIC */

    if (!(p = patcompile(b, PAT_STATIC, NULL))) {
	zerr("bad pattern: %s", b);
	ret = 0;
    } else
      ret = pattry(p, a);

    unqueue_signals();

    return ret;
}

/* do the ${foo%%bar}, ${foo#bar} stuff */
/* please do not laugh at this code. */

/* Having found a match in getmatch, decide what part of string
 * to return.  The matched part starts b characters into string imd->ustr
 * and finishes e characters in: 0 <= b <= e <= imd->ulen on input
 * (yes, empty matches should work).
 *
 * imd->flags is a set of the SUB_* matches defined in zsh.h from
 * SUB_MATCH onwards; the lower parts are ignored.
 *
 * imd->replstr is the replacement string for a substitution
 *
 * imd->replstr is metafied and the values put in imd->repllist are metafied.
 */

/**/
static char *
get_match_ret(Imatchdata imd, int b, int e)
{
    char buf[80], *r, *p, *rr, *replstr = imd->replstr;
    int ll = 0, bl = 0, t = 0, add = 0, fl = imd->flags, i;

    /* Account for b and e referring to unmetafied string */
    for (p = imd->ustr; p < imd->ustr + b; p++)
	if (imeta(*p))
	    add++;
    b += add;
    for (; p < imd->ustr + e; p++)
	if (imeta(*p))
	    add++;
    e += add;

    /* Everything now refers to metafied lengths. */
    if (replstr || (fl & SUB_LIST)) {
	if (fl & SUB_DOSUBST) {
	    replstr = dupstring(replstr);
	    singsub(&replstr);
	    untokenize(replstr);
	}
	if ((fl & (SUB_GLOBAL|SUB_LIST)) && imd->repllist) {
	    /* We are replacing the chunk, just add this to the list */
	    Repldata rd = (Repldata)
		((fl & SUB_LIST) ? zalloc(sizeof(*rd)) : zhalloc(sizeof(*rd)));
	    rd->b = b;
	    rd->e = e;
	    rd->replstr = replstr;
	    if (fl & SUB_LIST)
		zaddlinknode(imd->repllist, rd);
	    else
		addlinknode(imd->repllist, rd);
	    return imd->mstr;
	}
	if (replstr)
	    ll += strlen(replstr);
    }
    if (fl & SUB_MATCH)			/* matched portion */
	ll += 1 + (e - b);
    if (fl & SUB_REST)		/* unmatched portion */
	ll += 1 + (imd->mlen - (e - b));
    if (fl & SUB_BIND) {
	/* position of start of matched portion */
	sprintf(buf, "%d ", MB_METASTRLEN2END(imd->mstr, 0, imd->mstr+b) + 1);
	ll += (bl = strlen(buf));
    }
    if (fl & SUB_EIND) {
	/* position of end of matched portion */
	sprintf(buf + bl, "%d ",
		MB_METASTRLEN2END(imd->mstr, 0, imd->mstr+e) + 1);
	ll += (bl = strlen(buf));
    }
    if (fl & SUB_LEN) {
	/* length of matched portion */
	sprintf(buf + bl, "%d ", MB_METASTRLEN2END(imd->mstr+b, 0,
						   imd->mstr+e));
	ll += (bl = strlen(buf));
    }
    if (bl)
	buf[bl - 1] = '\0';

    if (ll == 0)
	return NULL;

    rr = r = (char *) hcalloc(ll);

    if (fl & SUB_MATCH) {
	/* copy matched portion to new buffer */
	for (i = b, p = imd->mstr + b; i < e; i++)
	    *rr++ = *p++;
	t = 1;
    }
    if (fl & SUB_REST) {
	/* Copy unmatched portion to buffer.  If both portions *
	 * requested, put a space in between (why?)            */
	if (t)
	    *rr++ = ' ';
	/* there may be unmatched bits at both beginning and end of string */
	for (i = 0, p = imd->mstr; i < b; i++)
	    *rr++ = *p++;
	if (replstr)
	    for (p = replstr; *p; )
		*rr++ = *p++;
	for (i = e, p = imd->mstr + e; i < imd->mlen; i++)
	    *rr++ = *p++;
	t = 1;
    }
    *rr = '\0';
    if (bl) {
	/* if there was a buffer (with a numeric result), add it; *
	 * if there was other stuff too, stick in a space first.  */
	if (t)
	    *rr++ = ' ';
	strcpy(rr, buf);
    }
    return r;
}

static Patprog
compgetmatch(char *pat, int *flp, char **replstrp)
{
    Patprog p;
    /*
     * Flags to pattern compiler:  use static buffer since we only
     * have one pattern at a time; we will try the must-match test ourselves,
     * so tell the pattern compiler we are scanning.
     */

    /* int patflags = PAT_STATIC|PAT_SCAN|PAT_NOANCH;*/

    /* Unfortunately, PAT_STATIC doesn't work if we have a replstr with
     * something like ${x#...} in it which will be singsub()ed below because
     * that would overwrite the pattern buffer. */

    int patflags = PAT_SCAN|PAT_NOANCH | (*replstrp ? 0 : PAT_STATIC);

    /*
     * Search is anchored to the end of the string if we want to match
     * it all, or if we are matching at the end of the string and not
     * using substrings.
     */
    if ((*flp & SUB_ALL) || ((*flp & SUB_END) && !(*flp & SUB_SUBSTR)))
	patflags &= ~PAT_NOANCH;
    p = patcompile(pat, patflags, NULL);
    if (!p) {
	zerr("bad pattern: %s", pat);
	return NULL;
    }
    if (*replstrp) {
	if (p->patnpar || (p->globend & GF_MATCHREF)) {
	    /*
	     * Either backreferences or match references, so we
	     * need to re-substitute replstr each time round.
	     */
	    *flp |= SUB_DOSUBST;
	} else {
	    singsub(replstrp);
	    untokenize(*replstrp);
	}
    }

    return p;
}

/*
 * This is called from paramsubst to get the match for ${foo#bar} etc.
 * fl is a set of the SUB_* flags defined in zsh.h
 * *sp points to the string we have to modify. The n'th match will be
 * returned in *sp. The heap is used to get memory for the result string.
 * replstr is the replacement string from a ${.../orig/repl}, in
 * which case pat is the original.
 *
 * n is now ignored unless we are looking for a substring, in
 * which case the n'th match from the start is counted such that
 * there is no more than one match from each position.
 */

/**/
int
getmatch(char **sp, char *pat, int fl, int n, char *replstr)
{
    Patprog p;

    if (!(p = compgetmatch(pat, &fl, &replstr)))
	return 1;

    return igetmatch(sp, p, fl, n, replstr, NULL);
}

/*
 * This is the corresponding function for array variables.
 * Matching is done with the same pattern on each element.
 */

/**/
void
getmatcharr(char ***ap, char *pat, int fl, int n, char *replstr)
{
    char **arr = *ap, **pp;
    Patprog p;

    if (!(p = compgetmatch(pat, &fl, &replstr)))
	return;

    *ap = pp = hcalloc(sizeof(char *) * (arrlen(arr) + 1));
    while ((*pp = *arr++))
	if (igetmatch(pp, p, fl, n, replstr, NULL))
	    pp++;
}

/*
 * Match against str using pattern pp; return a list of
 * Repldata matches in the linked list *repllistp; this is
 * in permanent storage and to be freed by freematchlist()
 */

/**/
mod_export int
getmatchlist(char *str, Patprog p, LinkList *repllistp)
{
    char **sp = &str;

    /*
     * We don't care if we have longest or shortest match, but SUB_LONG
     * is cheaper since the pattern code does that by default.
     * We need SUB_GLOBAL to get all matches.
     * We need SUB_SUBSTR to scan through for substrings.
     * We need SUB_LIST to activate the special handling of the list
     * passed in.
     */
    return igetmatch(sp, p, SUB_LONG|SUB_GLOBAL|SUB_SUBSTR|SUB_LIST,
		     0, NULL, repllistp);
}

static void
freerepldata(void *ptr)
{
    zfree(ptr, sizeof(struct repldata));
}

/**/
mod_export void
freematchlist(LinkList repllist)
{
    freelinklist(repllist, freerepldata);
}

/**/
static void
set_pat_start(Patprog p, int offs)
{
    /*
     * If we are messing around with the test string by advancing up
     * it from the start, we need to tell the pattern matcher that
     * a start-of-string assertion, i.e. (#s), should fail.  Hence
     * we test whether the offset of the real start of string from
     * the actual start, passed as offs, is zero.
     */
    if (offs)
	p->flags |= PAT_NOTSTART;
    else
	p->flags &= ~PAT_NOTSTART;
}

/**/
static void
set_pat_end(Patprog p, char null_me)
{
    /*
     * If we are messing around with the string by shortening it at the
     * tail, we need to tell the pattern matcher that an end-of-string
     * assertion, i.e. (#e), should fail.  Hence we test whether
     * the character null_me about to be zapped is or is not already a null.
     */
    if (null_me)
	p->flags |= PAT_NOTEND;
    else
	p->flags &= ~PAT_NOTEND;
}

/**/
#ifdef MULTIBYTE_SUPPORT

/*
 * Increment *tp over character which may be multibyte.
 * Return number of bytes.
 * All unmetafied here.
 */

/**/
static int iincchar(char **tp, int left)
{
    char *t = *tp;
    int mbclen = mb_charlenconv(t, left, NULL);
    *tp = t + mbclen;

    return mbclen;
}

/**/
static int
igetmatch(char **sp, Patprog p, int fl, int n, char *replstr,
	  LinkList *repllistp)
{
    char *s = *sp, *t, *tmatch, *send;
    /*
     * Note that ioff counts (possibly multibyte) characters in the
     * character set (Meta's are not included), while l counts characters in
     * the metafied string.
     *
     * umlen is a counter for (unmetafied) byte lengths---neither characters
     * nor raw byte indices; this is simply an optimisation for allocation.
     * umltot is the full length of the string in this scheme.
     *
     * l is the raw string length, used together with any pointers into
     * the string (typically t).
     */
    int ioff, l = strlen(*sp), matched = 1, umltot = ztrlen(*sp);
    int umlen, nmatches;
    struct patstralloc patstralloc;
    struct imatchdata imd;

    (void)patallocstr(p, s, l, umltot, 1, &patstralloc);
    s = patstralloc.alloced;
    DPUTS(!s, "forced patallocstr failed");
    send = s + umltot;

    imd.mstr = *sp;
    imd.mlen = l;
    imd.ustr = s;
    imd.ulen = umltot;
    imd.flags = fl;
    imd.replstr = replstr;
    imd.repllist = NULL;

    /* perform must-match test for complex closures */
    if (p->mustoff)
    {
	char *muststr = (char *)p + p->mustoff;

	matched = 0;
	if (p->patmlen <= umltot)
	{
	    for (t = s; t <= send - p->patmlen; t++)
	    {
		if (!memcmp(muststr, t, p->patmlen)) {
		    matched = 1;
		    break;
		}
	    }
	}
    }

    /* in case we used the prog before... */
    p->flags &= ~(PAT_NOTSTART|PAT_NOTEND);

    if (fl & SUB_ALL) {
	int i = matched && pattrylen(p, s, umltot, 0, &patstralloc, 0);
	if (!i) {
	    /* Perform under no-match conditions */
	    umltot = 0;
	    imd.replstr = NULL;
	}
	*sp = get_match_ret(&imd, 0, umltot);
	if (! **sp && (((fl & SUB_MATCH) && !i) || ((fl & SUB_REST) && i)))
	    return 0;
	return 1;
    }
    if (matched) {
	/*
	 * The default behaviour is to match at the start; this
	 * is modified by SUB_END and SUB_SUBSTR.  SUB_END matches
	 * at the end of the string instead of the start.  SUB_SUBSTR
	 * without SUB_END matches substrings searching from the start;
	 * with SUB_END it matches substrings searching from the end.
	 *
	 * The possibilities are further modified by whether we want the
	 * longest (SUB_LONG) or shortest possible match.
	 *
	 * SUB_START is only used in the case where we are also
	 * forcing a match at the end (SUB_END with no SUB_SUBSTR,
	 * with or without SUB_LONG), to indicate we should match
	 * the entire string.
	 */
	switch (fl & (SUB_END|SUB_LONG|SUB_SUBSTR)) {
	case 0:
	case SUB_LONG:
	    /*
	     * Largest/smallest possible match at head of string.
	     * First get the longest match...
	     */
	    if (pattrylen(p, s, umltot, 0, &patstralloc, 0)) {
		/* patmatchlen returns unmetafied length in this case */
	        int mlen = patmatchlen();
		if (!(fl & SUB_LONG) && !(p->flags & PAT_PURES)) {
		    send = s + mlen;
		    /*
		     * ... now we know whether it's worth looking for the
		     * shortest, which we do by brute force.
		     */
		    mb_charinit();
		    for (t = s, umlen = 0; t < send; ) {
			set_pat_end(p, *t);
			if (pattrylen(p, s, umlen, 0, &patstralloc, 0)) {
			    mlen = patmatchlen();
			    break;
			}
			umlen += iincchar(&t, send - t);
		    }
		}
		*sp = get_match_ret(&imd, 0, mlen);
		return 1;
	    }
	    break;

	case SUB_END:
	    /*
	     * Smallest possible match at tail of string.
	     * As we can only be sure we've got wide characters right
	     * when going forwards, we need to match at every point
	     * until we fail and record the last successful match.
	     *
	     * It's important that we return the last successful match
	     * so that match, mbegin, mend and MATCH, MBEGIN, MEND are
	     * correct.
	     */
	    mb_charinit();
	    tmatch = NULL;
	    set_pat_start(p, l);
	    if (pattrylen(p, send, 0, 0, &patstralloc, umltot) &&
		!--n) {
		*sp = get_match_ret(&imd, umltot, umltot);
		return 1;
	    }
	    for (ioff = 0, t = s, umlen = umltot; t < send; ioff++) {
		set_pat_start(p, t-s);
		if (pattrylen(p, t, umlen, 0, &patstralloc, ioff))
		    tmatch = t;
		if (fl & SUB_START)
		    break;
		umlen -= iincchar(&t, send - t);
	    }
	    if (tmatch) {
		*sp = get_match_ret(&imd, tmatch - s, umltot);
		return 1;
	    }
	    if (!(fl & SUB_START) && pattrylen(p, s + umltot, 0, 0,
					       &patstralloc, ioff)) {
		*sp = get_match_ret(&imd, umltot, umltot);
		return 1;
	    }
	    break;

	case (SUB_END|SUB_LONG):
	    /* Largest possible match at tail of string:       *
	     * move forward along string until we get a match. *
	     * Again there's no optimisation.                  */
	    mb_charinit();
	    for (ioff = 0, t = s, umlen = umltot; t <= send ; ioff++) {
		set_pat_start(p, t-s);
		if (pattrylen(p, t, umlen, 0, &patstralloc, ioff)) {
		    *sp = get_match_ret(&imd, t-s, umltot);
		    return 1;
		}
		if (fl & SUB_START)
		    break;
		if (t == send)
		    break;
		umlen -= iincchar(&t, send - t);
	    }
	    if (!(fl & SUB_START) && pattrylen(p, send, 0, 0,
					       &patstralloc, ioff)) {
		*sp = get_match_ret(&imd, umltot, umltot);
		return 1;
	    }
	    break;

	case SUB_SUBSTR:
	    /* Smallest at start, but matching substrings. */
	    set_pat_start(p, l);
	    if (!(fl & SUB_GLOBAL) &&
		pattrylen(p, send, 0, 0, &patstralloc, 0) &&
		!--n) {
		*sp = get_match_ret(&imd, 0, 0);
		return 1;
	    } /* fall through */
	case (SUB_SUBSTR|SUB_LONG):
	    /* longest or smallest at start with substrings */
	    t = s;
	    if (fl & SUB_GLOBAL) {
		imd.repllist = (fl & SUB_LIST) ? znewlinklist() : newlinklist();
		if (repllistp)
		     *repllistp = imd.repllist;
	    }
	    ioff = 0;		/* offset into string */
	    umlen = umltot;
	    mb_charinit();
	    do {
		/* loop over all matches for global substitution */
		matched = 0;
		for (; t <= send; ioff++) {
		    /* Find the longest match from this position. */
		    set_pat_start(p, t-s);
		    if (pattrylen(p, t, umlen, 0, &patstralloc, ioff)) {
			char *mpos = t + patmatchlen();
			if (!(fl & SUB_LONG) && !(p->flags & PAT_PURES)) {
			    char *ptr;
			    int umlen2;
			    /*
			     * If searching for the shortest match,
			     * start with a zero length and increase
			     * it until we reach the longest possible
			     * match, accepting the first successful
			     * match.
			     */
			    for (ptr = t, umlen2 = 0; ptr < mpos;) {
				set_pat_end(p, *ptr);
				if (pattrylen(p, t, umlen2, 0,
					      &patstralloc, ioff)) {
				    mpos = t + patmatchlen();
				    break;
				}
				umlen2 += iincchar(&ptr, mpos - ptr);
			    }
			}
			if (!--n || (n <= 0 && (fl & SUB_GLOBAL))) {
			    *sp = get_match_ret(&imd, t-s, mpos-s);
			    if (mpos == t)
				mpos += mb_charlenconv(mpos, send - mpos, NULL);
			}
			if (!(fl & SUB_GLOBAL)) {
			    if (n) {
				/*
				 * Looking for a later match: in this case,
				 * we can continue looking for matches from
				 * the next character, even if it overlaps
				 * with what we just found.
				 */
				umlen -= iincchar(&t, send - t);
				continue;
			    } else {
				return 1;
			    }
			}
			/*
			 * For a global match, we need to skip the stuff
			 * which is already marked for replacement.
			 */
			matched = 1;
			if (t == send)
			    break;
			while (t < mpos) {
			    ioff++;
			    umlen -= iincchar(&t, send - t);
			}
			break;
		    }
		    if (t == send)
			break;
		    umlen -= iincchar(&t, send - t);
		}
	    } while (matched && t < send);
	    /*
	     * check if we can match a blank string, if so do it
	     * at the start.  Goodness knows if this is a good idea
	     * with global substitution, so it doesn't happen.
	     */
	    set_pat_start(p, l);
	    if ((fl & (SUB_LONG|SUB_GLOBAL)) == SUB_LONG &&
		pattrylen(p, send, 0, 0, &patstralloc, 0) && !--n) {
		*sp = get_match_ret(&imd, 0, 0);
		return 1;
	    }
	    break;

	case (SUB_END|SUB_SUBSTR):
	case (SUB_END|SUB_LONG|SUB_SUBSTR):
	    /* Longest/shortest at end, matching substrings.       */
	    {
		set_pat_start(p, l);
		if (pattrylen(p, send, 0, 0, &patstralloc, umltot) &&
		    !--n) {
		    *sp = get_match_ret(&imd, umltot, umltot);
		    return 1;
		}
	    }
	    /*
	     * If multibyte characters are present we need to start from the
	     * beginning.  This is a bit unpleasant because we can't tell in
	     * advance how many times it will match and from where, so if n is
	     * greater then 1 we will need to count the number of times it
	     * matched and then go through again until we reach the right
	     * point.  (Either that or record every single match in a list,
	     * which isn't stupid; it involves more memory management at this
	     * level but less use of the pattern matcher.)
	     */
	    nmatches = 0;
	    tmatch = NULL;
	    mb_charinit();
	    for (ioff = 0, t = s, umlen = umltot; t < send; ioff++) {
		set_pat_start(p, t-s);
		if (pattrylen(p, t, umlen, 0, &patstralloc, ioff)) {
		    nmatches++;
		    tmatch = t;
		}
		umlen -= iincchar(&t, send - t);
	    }
	    if (nmatches) {
		char *mpos;
		if (n > 1) {
		    /*
		     * We need to find the n'th last match.
		     */
		    n = nmatches - n;
		    mb_charinit();
		    for (ioff = 0, t = s, umlen = umltot; t < send; ioff++) {
			set_pat_start(p, t-s);
			if (pattrylen(p, t, umlen, 0, &patstralloc, ioff) &&
			    !n--) {
			    tmatch = t;
			    break;
			}
			umlen -= iincchar(&t, send - t);
		    }
		}
		mpos = tmatch + patmatchlen();
		/* Look for the shortest match if necessary */
		if (!(fl & SUB_LONG) && !(p->flags & PAT_PURES)) {
		    for (t = tmatch, umlen = 0; t < mpos; ) {
			set_pat_end(p, *t);
			if (pattrylen(p, tmatch, umlen, 0,
				      &patstralloc, ioff)) {
			    mpos = tmatch + patmatchlen();
			    break;
			}
			umlen += iincchar(&t, mpos - t);
		    }
		}
		*sp = get_match_ret(&imd, tmatch-s, mpos-s);
		return 1;
	    }
	    set_pat_start(p, l);
	    if ((fl & SUB_LONG) && pattrylen(p, send, 0, 0,
					     &patstralloc, umltot) &&
		!--n) {
		*sp = get_match_ret(&imd, umltot, umltot);
		return 1;
	    }
	    break;
	}
    }

    if (imd.repllist && nonempty(imd.repllist)) {
	/* Put all the bits of a global search and replace together. */
	LinkNode nd;
	Repldata rd;
	int lleft;
	char *ptr, *start;
	int i;

	/*
	 * Use metafied string again.
	 * Results from get_match_ret in repllist are all metafied.
	 */
	s = *sp;
	if (!(fl & SUB_LIST)) {
	    lleft = 0;		/* size of returned string */
	    i = 0;	       /* start of last chunk we got from *sp */
	    for (nd = firstnode(imd.repllist); nd; incnode(nd)) {
		rd = (Repldata) getdata(nd);
		lleft += rd->b - i; /* previous chunk of *sp */
		lleft += strlen(rd->replstr);	/* the replaced bit */
		i = rd->e;		/* start of next chunk of *sp */
	    }
	    lleft += l - i;	/* final chunk from *sp */
	    start = t = zhalloc(lleft+1);
	    i = 0;
	    for (nd = firstnode(imd.repllist); nd; incnode(nd)) {
		rd = (Repldata) getdata(nd);
		memcpy(t, s + i, rd->b - i);
		t += rd->b - i;
		ptr = rd->replstr;
		while (*ptr)
		    *t++ = *ptr++;
		i = rd->e;
	    }
	    memcpy(t, s + i, l - i);
	    start[lleft] = '\0';
	    *sp = (char *)start;
	}
	return 1;
    }
    if (fl & SUB_LIST) {	/* safety: don't think this can happen */
	return 0;
    }

    /* munge the whole string: no match, so no replstr */
    imd.replstr = NULL;
    imd.repllist = NULL;
    *sp = get_match_ret(&imd, 0, 0);
    return (fl & SUB_RETFAIL) ? 0 : 1;
}

/**/
#else

/*
 * Increment pointer which may be on a Meta (x is a pointer variable),
 * returning the incremented value (i.e. like pre-increment).
 */
#define METAINC(x)	((x) += (*(x) == Meta) ? 2 : 1)

/**/
static int
igetmatch(char **sp, Patprog p, int fl, int n, char *replstr,
	  LinkList *repllistp)
{
    char *s = *sp, *t, *send;
    /*
     * Note that ioff and uml count characters in the character
     * set (Meta's are not included), while l counts characters in the
     * metafied string.  umlen is a counter for (unmetafied) character
     * lengths.
     */
    int ioff, l = strlen(*sp), uml = ztrlen(*sp), matched = 1, umlen;
    struct patstralloc patstralloc;
    struct imatchdata imd;

    (void)patallocstr(p, s, l, uml, 1, &patstralloc);
    s = patstralloc.alloced;
    DPUTS(!s, "forced patallocstr failed");
    send = s + uml;

    imd.mstr = *sp;
    imd.mlen = l;
    imd.ustr = s;
    imd.ulen = uml;
    imd.flags = fl;
    imd.replstr = replstr;
    imd.repllist = NULL;

    /* perform must-match test for complex closures */
    if (p->mustoff)
    {
	char *muststr = (char *)p + p->mustoff;

	matched = 0;
	if (p->patmlen <= uml)
	{
	    for (t = s; t <= send - p->patmlen; t++)
	    {
		if (!memcmp(muststr, t, p->patmlen)) {
		    matched = 1;
		    break;
		}
	    }
	}
    }

    /* in case we used the prog before... */
    p->flags &= ~(PAT_NOTSTART|PAT_NOTEND);

    if (fl & SUB_ALL) {
	int i = matched && pattrylen(p, s, uml, 0, &patstralloc, 0);
	if (!i)
	    imd.replstr = NULL;
	*sp = get_match_ret(&imd, 0, i ? l : 0);
	if (! **sp && (((fl & SUB_MATCH) && !i) || ((fl & SUB_REST) && i)))
	    return 0;
	return 1;
    }
    if (matched) {
        /* Default is to match at the start; see comment in MULTIBYTE above */
	switch (fl & (SUB_END|SUB_LONG|SUB_SUBSTR)) {
	case 0:
	case SUB_LONG:
	    /*
	     * Largest/smallest possible match at head of string.
	     * First get the longest match...
	     */
	    if (pattrylen(p, s, uml, 0, &patstralloc, 0)) {
		/* patmatchlen returns metafied length, as we need */
	        int mlen = patmatchlen();
		if (!(fl & SUB_LONG) && !(p->flags & PAT_PURES)) {
		    send = s + mlen;
		    /*
		     * ... now we know whether it's worth looking for the
		     * shortest, which we do by brute force.
		     */
		    for (t = s, umlen = 0; t < s + mlen; METAINC(t), umlen++) {
			set_pat_end(p, *t);
			if (pattrylen(p, s, umlen, 0, &patstralloc, 0)) {
			    mlen = patmatchlen();
			    break;
			}
		    }
		}
		*sp = get_match_ret(&imd, 0, mlen);
		return 1;
	    }
	    break;

	case SUB_END:
	    /* Smallest possible match at tail of string:  *
	     * move back down string until we get a match. *
	     * There's no optimization here.               */
	    for (ioff = uml, t = send, umlen = 0; t >= s;
		 t--, ioff--, umlen++) {
		set_pat_start(p, t-s);
		if (pattrylen(p, t, umlen, 0, &patstralloc, ioff)) {
		    *sp = get_match_ret(&imd, t - s, uml);
		    return 1;
		}
	    }
	    break;

	case (SUB_END|SUB_LONG):
	    /* Largest possible match at tail of string:       *
	     * move forward along string until we get a match. *
	     * Again there's no optimisation.                  */
	    for (ioff = 0, t = s, umlen = uml; t <= send;
		 ioff++, t++, umlen--) {
		set_pat_start(p, t-s);
		if (pattrylen(p, t, send - t, umlen, &patstralloc, ioff)) {
		    *sp = get_match_ret(&imd, t-s, uml);
		    return 1;
		}
	    }
	    break;

	case SUB_SUBSTR:
	    /* Smallest at start, but matching substrings. */
	    set_pat_start(p, l);
	    if (!(fl & SUB_GLOBAL) &&
		pattrylen(p, send, 0, 0, &patstralloc, 0) && !--n) {
		*sp = get_match_ret(&imd, 0, 0);
		return 1;
	    } /* fall through */
	case (SUB_SUBSTR|SUB_LONG):
	    /* longest or smallest at start with substrings */
	    t = s;
	    if (fl & SUB_GLOBAL) {
		imd.repllist = (fl & SUB_LIST) ? znewlinklist() : newlinklist();
		if (repllistp)
		    *repllistp = imd.repllist;
	    }
	    ioff = 0;		/* offset into string */
	    umlen = uml;
	    do {
		/* loop over all matches for global substitution */
		matched = 0;
		for (; t <= send; t++, ioff++, umlen--) {
		    /* Find the longest match from this position. */
		    set_pat_start(p, t-s);
		    if (pattrylen(p, t, send - t, umlen, &patstralloc, ioff)) {
			char *mpos = t + patmatchlen();
			if (!(fl & SUB_LONG) && !(p->flags & PAT_PURES)) {
			    char *ptr;
			    int umlen2;
			    for (ptr = t, umlen2 = 0; ptr < mpos;
				 ptr++, umlen2++) {
				set_pat_end(p, *ptr);
				if (pattrylen(p, t, ptr - t, umlen2,
					      &patstralloc, ioff)) {
				    mpos = t + patmatchlen();
				    break;
				}
			    }
			}
			if (!--n || (n <= 0 && (fl & SUB_GLOBAL))) {
			    *sp = get_match_ret(&imd, t-s, mpos-s);
			    if (mpos == t)
				mpos++;
			}
			if (!(fl & SUB_GLOBAL)) {
			    if (n) {
				/*
				 * Looking for a later match: in this case,
				 * we can continue looking for matches from
				 * the next character, even if it overlaps
				 * with what we just found.
				 */
				continue;
			    } else {
				return 1;
			    }
			}
			/*
			 * For a global match, we need to skip the stuff
			 * which is already marked for replacement.
			 */
			matched = 1;
			if (t == send)
			    break;
			while (t < mpos) {
			    ioff++;
			    umlen--;
			    t++;
			}
			break;
		    }
		    if (t == send)
			break;
		}
	    } while (matched && t < send);
	    /*
	     * check if we can match a blank string, if so do it
	     * at the start.  Goodness knows if this is a good idea
	     * with global substitution, so it doesn't happen.
	     */
	    set_pat_start(p, l);
	    if ((fl & (SUB_LONG|SUB_GLOBAL)) == SUB_LONG &&
		pattrylen(p, send, 0, 0, &patstralloc, 0) && !--n) {
		*sp = get_match_ret(&imd, 0, 0);
		return 1;
	    }
	    break;

	case (SUB_END|SUB_SUBSTR):
	case (SUB_END|SUB_LONG|SUB_SUBSTR):
	    /* Longest/shortest at end, matching substrings.       */
	    {
		set_pat_start(p, l);
		if (pattrylen(p, send, 0, 0, &patstralloc, uml) && !--n) {
		    *sp = get_match_ret(&imd, uml, uml);
		    return 1;
		}
	    }
	    for (ioff = uml - 1, t = send - 1, umlen = 1; t >= s;
		 t--, ioff--, umlen++) {
		set_pat_start(p, t-s);
		if (pattrylen(p, t, send - t, umlen, &patstralloc, ioff) &&
		    !--n) {
		    /* Found the longest match */
		    char *mpos = t + patmatchlen();
		    if (!(fl & SUB_LONG) && !(p->flags & PAT_PURES)) {
			char *ptr;
			int umlen2;
			for (ptr = t, umlen2 = 0; ptr < mpos;
			     ptr++, umlen2++) {
			    set_pat_end(p, *ptr);
			    if (pattrylen(p, t, umlen2, 0, &patstralloc,
					  ioff)) {
				mpos = t + patmatchlen();
				break;
			    }
			}
		    }
		    *sp = get_match_ret(&imd, t-s, mpos-s);
		    return 1;
		}
	    }
	    set_pat_start(p, l);
	    if ((fl & SUB_LONG) && pattrylen(p, send, 0, 0,
					     &patstralloc, uml) &&
		!--n) {
		*sp = get_match_ret(&imd, uml, uml);
		return 1;
	    }
	    break;
	}
    }

    if (imd.repllist && nonempty(imd.repllist)) {
	/* Put all the bits of a global search and replace together. */
	LinkNode nd;
	Repldata rd;
	int lleft = 0;		/* size of returned string */
	char *ptr, *start;
	int i;

	/*
	 * Use metafied string again.
	 * Results from get_match_ret in repllist are all metafied.
	 */
	s = *sp;
	if (fl & SUB_LIST)
	    return 1;
	i = 0;			/* start of last chunk we got from *sp */
	for (nd = firstnode(imd.repllist); nd; incnode(nd)) {
	    rd = (Repldata) getdata(nd);
	    lleft += rd->b - i; /* previous chunk of *sp */
	    lleft += strlen(rd->replstr);	/* the replaced bit */
	    i = rd->e;		/* start of next chunk of *sp */
	}
	lleft += l - i;	/* final chunk from *sp */
	start = t = zhalloc(lleft+1);
	i = 0;
	for (nd = firstnode(imd.repllist); nd; incnode(nd)) {
	    rd = (Repldata) getdata(nd);
	    memcpy(t, s + i, rd->b - i);
	    t += rd->b - i;
	    ptr = rd->replstr;
	    while (*ptr)
		*t++ = *ptr++;
	    i = rd->e;
	}
	memcpy(t, s + i, l - i);
	start[lleft] = '\0';
	*sp = (char *)start;
	return 1;
    }

    /* munge the whole string: no match, so no replstr */
    imd.replstr = NULL;
    imd.repllist = NULL;
    *sp = get_match_ret(&imd, 0, 0);
    return (fl & SUB_RETFAIL) ? 0 : 1;
}

/**/
#endif /* MULTIBYTE_SUPPORT */

/* blindly turn a string into a tokenised expression without lexing */

/**/
mod_export void
tokenize(char *s)
{
    zshtokenize(s, 0);
}

/*
 * shtokenize is used when we tokenize a string with GLOB_SUBST set.
 * In that case we need to retain backslashes when we turn the
 * pattern back into a string, so that the string is not
 * modified if it failed to match a pattern.
 *
 * It may be modified by the effect of SH_GLOB which turns off
 * various zsh-specific options.
 */

/**/
mod_export void
shtokenize(char *s)
{
    int flags = ZSHTOK_SUBST;
    if (isset(SHGLOB))
	flags |= ZSHTOK_SHGLOB;
    zshtokenize(s, flags);
}

/**/
static void
zshtokenize(char *s, int flags)
{
    char *t;
    int bslash = 0;

    for (; *s; s++) {
      cont:
	switch (*s) {
	case Meta:
	    /* skip both Meta and following character */
	    s++;
	    break;
	case Bnull:
	case Bnullkeep:
	case '\\':
	    if (bslash) {
		s[-1] = (flags & ZSHTOK_SUBST) ? Bnullkeep : Bnull;
		break;
	    }
	    bslash = 1;
	    continue;
	case '<':
	    if (flags & ZSHTOK_SHGLOB)
		break;
	    if (bslash) {
		s[-1] = (flags & ZSHTOK_SUBST) ? Bnullkeep : Bnull;
		break;
	    }
	    t = s;
	    while (idigit(*++s));
	    if (!IS_DASH(*s))
		goto cont;
	    while (idigit(*++s));
	    if (*s != '>')
		goto cont;
	    *t = Inang;
	    *s = Outang;
	    break;
	case '(':
	case '|':
	case ')':
	    if (flags & ZSHTOK_SHGLOB)
		break;
	    /*FALLTHROUGH*/
	case '>':
	case '^':
	case '#':
	case '~':
	case '[':
	case ']':
	case '*':
	case '?':
	case '=':
	case '-':
	case '!':
	    for (t = ztokens; *t; t++) {
		if (*t == *s) {
		    if (bslash)
			s[-1] = (flags & ZSHTOK_SUBST) ? Bnullkeep : Bnull;
		    else
			*s = (t - ztokens) + Pound;
		    break;
		}
	    }
	    break;
	}
	bslash = 0;
    }
}

/* remove unnecessary Nulargs */

/**/
mod_export void
remnulargs(char *s)
{
    if (*s) {
	char *o = s, c;

	while ((c = *s++))
	    if (c == Bnullkeep) {
		/*
		 * An active backslash that needs to be turned back into
		 * a real backslash for output.  However, we don't
		 * do that yet since we need to ignore it during
		 * pattern matching.
		 */
		continue;
	    } else if (inull(c)) {
		char *t = s - 1;

		while ((c = *s++)) {
		    if (c == Bnullkeep)
			*t++ = '\\';
		    else if (!inull(c))
			*t++ = c;
		}
		*t = '\0';
		if (!*o) {
		    o[0] = Nularg;
		    o[1] = '\0';
		}
		break;
	    }
    }
}

/* qualifier functions:  mostly self-explanatory, see glob(). */

/* device number */

/**/
static int
qualdev(UNUSED(char *name), struct stat *buf, off_t dv, UNUSED(char *dummy))
{
    return (off_t)buf->st_dev == dv;
}

/* number of hard links to file */

/**/
static int
qualnlink(UNUSED(char *name), struct stat *buf, off_t ct, UNUSED(char *dummy))
{
    return (g_range < 0 ? buf->st_nlink < ct :
	    g_range > 0 ? buf->st_nlink > ct :
	    buf->st_nlink == ct);
}

/* user ID */

/**/
static int
qualuid(UNUSED(char *name), struct stat *buf, off_t uid, UNUSED(char *dummy))
{
    return buf->st_uid == uid;
}

/* group ID */

/**/
static int
qualgid(UNUSED(char *name), struct stat *buf, off_t gid, UNUSED(char *dummy))
{
    return buf->st_gid == gid;
}

/* device special file? */

/**/
static int
qualisdev(UNUSED(char *name), struct stat *buf, UNUSED(off_t junk), UNUSED(char *dummy))
{
    return S_ISBLK(buf->st_mode) || S_ISCHR(buf->st_mode);
}

/* block special file? */

/**/
static int
qualisblk(UNUSED(char *name), struct stat *buf, UNUSED(off_t junk), UNUSED(char *dummy))
{
    return S_ISBLK(buf->st_mode);
}

/* character special file? */

/**/
static int
qualischr(UNUSED(char *name), struct stat *buf, UNUSED(off_t junk), UNUSED(char *dummy))
{
    return S_ISCHR(buf->st_mode);
}

/* directory? */

/**/
static int
qualisdir(UNUSED(char *name), struct stat *buf, UNUSED(off_t junk), UNUSED(char *dummy))
{
    return S_ISDIR(buf->st_mode);
}

/* FIFO? */

/**/
static int
qualisfifo(UNUSED(char *name), struct stat *buf, UNUSED(off_t junk), UNUSED(char *dummy))
{
    return S_ISFIFO(buf->st_mode);
}

/* symbolic link? */

/**/
static int
qualislnk(UNUSED(char *name), struct stat *buf, UNUSED(off_t junk), UNUSED(char *dummy))
{
    return S_ISLNK(buf->st_mode);
}

/* regular file? */

/**/
static int
qualisreg(UNUSED(char *name), struct stat *buf, UNUSED(off_t junk), UNUSED(char *dummy))
{
    return S_ISREG(buf->st_mode);
}

/* socket? */

/**/
static int
qualissock(UNUSED(char *name), struct stat *buf, UNUSED(off_t junk), UNUSED(char *dummy))
{
    return S_ISSOCK(buf->st_mode);
}

/* given flag is set in mode */

/**/
static int
qualflags(UNUSED(char *name), struct stat *buf, off_t mod, UNUSED(char *dummy))
{
    return mode_to_octal(buf->st_mode) & mod;
}

/* mode matches specification */

/**/
static int
qualmodeflags(UNUSED(char *name), struct stat *buf, off_t mod, UNUSED(char *dummy))
{
    long v = mode_to_octal(buf->st_mode), y = mod & 07777, n = mod >> 12;

    return ((v & y) == y && !(v & n));
}

/* regular executable file? */

/**/
static int
qualiscom(UNUSED(char *name), struct stat *buf, UNUSED(off_t mod), UNUSED(char *dummy))
{
    return S_ISREG(buf->st_mode) && (buf->st_mode & S_IXUGO);
}

/* size in required range? */

/**/
static int
qualsize(UNUSED(char *name), struct stat *buf, off_t size, UNUSED(char *dummy))
{
#if defined(ZSH_64_BIT_TYPE) || defined(LONG_IS_64_BIT)
# define QS_CAST_SIZE()
    zlong scaled = buf->st_size;
#else
# define QS_CAST_SIZE() (unsigned long)
    unsigned long scaled = (unsigned long)buf->st_size;
#endif

    switch (g_units) {
    case TT_POSIX_BLOCKS:
	scaled += 511l;
	scaled /= 512l;
	break;
    case TT_KILOBYTES:
	scaled += 1023l;
	scaled /= 1024l;
	break;
    case TT_MEGABYTES:
	scaled += 1048575l;
	scaled /= 1048576l;
	break;
#if defined(ZSH_64_BIT_TYPE) || defined(LONG_IS_64_BIT)
    case TT_GIGABYTES:
        scaled += ZLONG_CONST(1073741823);
        scaled /= ZLONG_CONST(1073741824);
        break;
    case TT_TERABYTES:
        scaled += ZLONG_CONST(1099511627775);
        scaled /= ZLONG_CONST(1099511627776);
        break;
#endif
    }

    return (g_range < 0 ? scaled < QS_CAST_SIZE() size :
	    g_range > 0 ? scaled > QS_CAST_SIZE() size :
	    scaled == QS_CAST_SIZE() size);
#undef QS_CAST_SIZE
}

/* time in required range? */

/**/
static int
qualtime(UNUSED(char *name), struct stat *buf, off_t days, UNUSED(char *dummy))
{
    time_t now, diff;

    time(&now);
    diff = now - (g_amc == 0 ? buf->st_atime : g_amc == 1 ? buf->st_mtime :
		  buf->st_ctime);
    /* handle multipliers indicating units */
    switch (g_units) {
    case TT_DAYS:
	diff /= 86400l;
	break;
    case TT_HOURS:
	diff /= 3600l;
	break;
    case TT_MINS:
	diff /= 60l;
	break;
    case TT_WEEKS:
	diff /= 604800l;
	break;
    case TT_MONTHS:
	diff /= 2592000l;
	break;
    }

    return (g_range < 0 ? diff < days :
	    g_range > 0 ? diff > days :
	    diff == days);
}

/* evaluate a string */

/**/
static int
qualsheval(char *name, UNUSED(struct stat *buf), UNUSED(off_t days), char *str)
{
    Eprog prog;

    if ((prog = parse_string(str, 0))) {
	int ef = errflag, lv = lastval, ret;
	int cshglob = badcshglob;

	unsetparam("reply");
	setsparam("REPLY", ztrdup(name));
	badcshglob = 0;

	execode(prog, 1, 0, "globqual");

	if ((ret = lastval))
	    badcshglob |= cshglob;
	/* Retain any user interrupt error status */
	errflag = ef | (errflag & ERRFLAG_INT);
	lastval = lv;

	if (!(inserts = getaparam("reply")) &&
	    !(inserts = gethparam("reply"))) {
	    char *tmp;

	    if ((tmp = getsparam("reply")) || (tmp = getsparam("REPLY"))) {
		static char *tmparr[2];

		tmparr[0] = tmp;
		tmparr[1] = NULL;

		inserts = tmparr;
	    }
	}

	return !ret;
    }
    return 0;
}

/**/
static int
qualnonemptydir(char *name, struct stat *buf, UNUSED(off_t days), UNUSED(char *str))
{
    DIR *dirh;
    struct dirent *de;
    int unamelen;
    char *uname = unmetafy(dupstring(name), &unamelen);

    if (!S_ISDIR(buf->st_mode))
	return 0;

    if (buf->st_nlink > 2)
	return 1;

    if (!(dirh = opendir(uname)))
	return 0;

    while ((de = readdir(dirh))) {
	if (strcmp(de->d_name, ".") && strcmp(de->d_name, "..")) {
	    closedir(dirh);
	    return 1;
	}
    }

    closedir(dirh);
    return 0;
}
