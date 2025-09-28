/*
 * Copyright (c) 1989, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Guido van Rossum.
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
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS "AS IS" AND
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

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)glob.c	8.3 (Berkeley) 10/13/93";
/* most changes between the version above and the one below have been ported:
static char sscsid[]=  "$OpenBSD: glob.c,v 1.8.10.1 2001/04/10 jason Exp $";
 */
#endif /* LIBC_SCCS and not lint */

/*
 * glob(3) -- a superset of the one defined in POSIX 1003.2.
 *
 * The [!...] convention to negate a range is supported (SysV, Posix, ksh).
 *
 * Optional extra services, controlled by flags not defined by POSIX:
 *
 * GLOB_QUOTE:
 *	Escaping convention: \ inhibits any special meaning the following
 *	character might have (except \ at end of string is retained).
 * GLOB_MAGCHAR:
 *	Set in gl_flags if pattern contained a globbing character.
 * GLOB_NOMAGIC:
 *	Same as GLOB_NOCHECK, but it will only append pattern if it did
 *	not contain any magic characters.  [Used in csh style globbing]
 * GLOB_ALTDIRFUNC:
 *	Use alternately specified directory access functions.
 * GLOB_TILDE:
 *	expand ~user/foo to the /home/dir/of/user/foo
 * GLOB_BRACE:
 *	expand {1,2}{a,b} to 1a 1b 2a 2b
 * gl_matchc:
 *	Number of matches in the current invocation of glob.
 * GLOB_ALPHASORT:
 *	sort alphabetically like csh (case doesn't matter) instead of in ASCII
 *	order
 */

#include <EXTERN.h>
#include <perl.h>
#include <XSUB.h>

#include "bsd_glob.h"
#ifdef I_PWD
#	include <pwd.h>
#else
#if defined(HAS_PASSWD) && !defined(VMS)
        struct passwd *getpwnam(char *);
        struct passwd *getpwuid(Uid_t);
#endif
#endif

#ifndef MAXPATHLEN
#  ifdef PATH_MAX
#    define MAXPATHLEN  PATH_MAX
#  else
#    define MAXPATHLEN  1024
#  endif
#endif

#include <limits.h>

#ifndef ARG_MAX
#  ifdef _SC_ARG_MAX
#    define     ARG_MAX         (sysconf(_SC_ARG_MAX))
#  else
#    ifdef _POSIX_ARG_MAX
#      define   ARG_MAX         _POSIX_ARG_MAX
#    else
#      ifdef WIN32
#        define ARG_MAX         14500   /* from VC's limits.h */
#      else
#        define ARG_MAX         4096    /* from POSIX, be conservative */
#      endif
#    endif
#  endif
#endif

#define BG_DOLLAR       '$'
#define BG_DOT          '.'
#define BG_EOS          '\0'
#define BG_LBRACKET     '['
#define BG_NOT          '!'
#define BG_QUESTION     '?'
#define BG_QUOTE        '\\'
#define BG_RANGE        '-'
#define BG_RBRACKET     ']'
#define BG_SEP  '/'
#ifdef DOSISH
#define BG_SEP2		'\\'
#endif
#define BG_STAR         '*'
#define BG_TILDE        '~'
#define BG_UNDERSCORE   '_'
#define BG_LBRACE       '{'
#define BG_RBRACE       '}'
#define BG_SLASH        '/'
#define BG_COMMA        ','

#ifndef GLOB_DEBUG

#define M_QUOTE         0x8000
#define M_PROTECT       0x4000
#define M_MASK          0xffff
#define M_ASCII         0x00ff

typedef U16 Char;

#else

#define M_QUOTE         0x80
#define M_PROTECT       0x40
#define M_MASK          0xff
#define M_ASCII         0x7f

typedef U8 Char;

#endif /* !GLOB_DEBUG */


#define CHAR(c)         ((Char)((c)&M_ASCII))
#define META(c)         ((Char)((c)|M_QUOTE))
#define M_ALL           META('*')
#define M_END           META(']')
#define M_NOT           META('!')
#define M_ONE           META('?')
#define M_RNG           META('-')
#define M_SET           META('[')
#define ismeta(c)       (((c)&M_QUOTE) != 0)


static int	 compare(const void *, const void *);
static int	 ci_compare(const void *, const void *);
static int	 g_Ctoc(const Char *, char *, STRLEN);
static int	 g_lstat(Char *, Stat_t *, glob_t *);
static DIR	*g_opendir(Char *, glob_t *);
static Char	*g_strchr(Char *, int);
static int	 g_stat(Char *, Stat_t *, glob_t *);
static int	 glob0(const Char *, glob_t *);
static int	 glob1(Char *, Char *, glob_t *, size_t *);
static int	 glob2(Char *, Char *, Char *, Char *, Char *, Char *,
                       glob_t *, size_t *);
static int	 glob3(Char *, Char *, Char *, Char *, Char *,
                       Char *, Char *, glob_t *, size_t *);
static int	 globextend(const Char *, glob_t *, size_t *);
static const Char *
                 globtilde(const Char *, Char *, size_t, glob_t *);
static int	 globexp1(const Char *, glob_t *);
static int	 globexp2(const Char *, const Char *, glob_t *, int *);
static int	 match(Char *, Char *, Char *, int);
#ifdef GLOB_DEBUG
static void	 qprintf(const char *, Char *);
#endif /* GLOB_DEBUG */

#ifdef MULTIPLICITY
static Direntry_t *	my_readdir(DIR*);

static Direntry_t *
my_readdir(DIR *d)
{
    return PerlDir_read(d);
}
#else

/* ReliantUNIX (OS formerly known as SINIX) defines readdir
 * in LFS-mode to be a 64-bit version of readdir.  */

#   ifdef sinix
static Direntry_t *    my_readdir(DIR*);

static Direntry_t *
my_readdir(DIR *d)
{
    return readdir(d);
}
#   else

#       define my_readdir       readdir

#   endif

#endif

int
bsd_glob(const char *pattern, int flags,
         int (*errfunc)(const char *, int), glob_t *pglob)
{
        const U8 *patnext;
        int c;
        Char *bufnext, *bufend, patbuf[MAXPATHLEN];
        patnext = (U8 *) pattern;
        /* TODO: GLOB_APPEND / GLOB_DOOFFS aren't supported yet */
#if 0
        if (!(flags & GLOB_APPEND)) {
                pglob->gl_pathc = 0;
                pglob->gl_pathv = NULL;
                if (!(flags & GLOB_DOOFFS))
                        pglob->gl_offs = 0;
        }
#else
        pglob->gl_pathc = 0;
        pglob->gl_pathv = NULL;
        pglob->gl_offs = 0;
#endif
        pglob->gl_flags = flags & ~GLOB_MAGCHAR;
        pglob->gl_errfunc = errfunc;
        pglob->gl_matchc = 0;

        bufnext = patbuf;
        bufend = bufnext + MAXPATHLEN - 1;
#ifdef DOSISH
        /* Nasty hack to treat patterns like "C:*" correctly. In this
         * case, the * should match any file in the current directory
         * on the C: drive. However, the glob code does not treat the
         * colon specially, so it looks for files beginning "C:" in
         * the current directory. To fix this, change the pattern to
         * add an explicit "./" at the start (just after the drive
         * letter and colon - ie change to "C:./").
         */
        if (isalpha(pattern[0]) && pattern[1] == ':' &&
            pattern[2] != BG_SEP && pattern[2] != BG_SEP2 &&
            bufend - bufnext > 4) {
                *bufnext++ = pattern[0];
                *bufnext++ = ':';
                *bufnext++ = '.';
                *bufnext++ = BG_SEP;
                patnext += 2;
        }
#endif

        if (flags & GLOB_QUOTE) {
                /* Protect the quoted characters. */
                while (bufnext < bufend && (c = *patnext++) != BG_EOS)
                        if (c == BG_QUOTE) {
#ifdef DOSISH
                                    /* To avoid backslashitis on Win32,
                                     * we only treat \ as a quoting character
                                     * if it precedes one of the
                                     * metacharacters []-{}~\
                                     */
                                if ((c = *patnext++) != '[' && c != ']' &&
                                    c != '-' && c != '{' && c != '}' &&
                                    c != '~' && c != '\\') {
#else
                                if ((c = *patnext++) == BG_EOS) {
#endif
                                        c = BG_QUOTE;
                                        --patnext;
                                }
                                *bufnext++ = c | M_PROTECT;
                        } else
                                *bufnext++ = c;
        } else
                while (bufnext < bufend && (c = *patnext++) != BG_EOS)
                        *bufnext++ = c;
        *bufnext = BG_EOS;

        if (flags & GLOB_BRACE)
            return globexp1(patbuf, pglob);
        else
            return glob0(patbuf, pglob);
}

/*
 * Expand recursively a glob {} pattern. When there is no more expansion
 * invoke the standard globbing routine to glob the rest of the magic
 * characters
 */
static int
globexp1(const Char *pattern, glob_t *pglob)
{
        const Char* ptr = pattern;
        int rv;

        /* Protect a single {}, for find(1), like csh */
        if (pattern[0] == BG_LBRACE && pattern[1] == BG_RBRACE && pattern[2] == BG_EOS)
                return glob0(pattern, pglob);

        while ((ptr = (const Char *) g_strchr((Char *) ptr, BG_LBRACE)) != NULL)
                if (!globexp2(ptr, pattern, pglob, &rv))
                        return rv;

        return glob0(pattern, pglob);
}


/*
 * Recursive brace globbing helper. Tries to expand a single brace.
 * If it succeeds then it invokes globexp1 with the new pattern.
 * If it fails then it tries to glob the rest of the pattern and returns.
 */
static int
globexp2(const Char *ptr, const Char *pattern,
         glob_t *pglob, int *rv)
{
        int     i;
        Char   *lm, *ls;
        const Char *pe, *pm, *pm1, *pl;
        Char    patbuf[MAXPATHLEN];

        /* copy part up to the brace */
        for (lm = patbuf, pm = pattern; pm != ptr; *lm++ = *pm++)
                ;
        *lm = BG_EOS;
        ls = lm;

        /* Find the balanced brace */
        for (i = 0, pe = ++ptr; *pe; pe++)
                if (*pe == BG_LBRACKET) {
                        /* Ignore everything between [] */
                        for (pm = pe++; *pe != BG_RBRACKET && *pe != BG_EOS; pe++)
                                ;
                        if (*pe == BG_EOS) {
                                /*
                                 * We could not find a matching BG_RBRACKET.
                                 * Ignore and just look for BG_RBRACE
                                 */
                                pe = pm;
                        }
                } else if (*pe == BG_LBRACE)
                        i++;
                else if (*pe == BG_RBRACE) {
                        if (i == 0)
                                break;
                        i--;
                }

        /* Non matching braces; just glob the pattern */
        if (i != 0 || *pe == BG_EOS) {
                *rv = glob0(patbuf, pglob);
                return 0;
        }

        for (i = 0, pl = pm = ptr; pm <= pe; pm++) {
                switch (*pm) {
                case BG_LBRACKET:
                        /* Ignore everything between [] */
                        for (pm1 = pm++; *pm != BG_RBRACKET && *pm != BG_EOS; pm++)
                                ;
                        if (*pm == BG_EOS) {
                                /*
                                 * We could not find a matching BG_RBRACKET.
                                 * Ignore and just look for BG_RBRACE
                                 */
                                pm = pm1;
                        }
                        break;

                case BG_LBRACE:
                        i++;
                        break;

                case BG_RBRACE:
                        if (i) {
                                i--;
                                break;
                        }
                        /* FALLTHROUGH */
                case BG_COMMA:
                        if (i && *pm == BG_COMMA)
                                break;
                        else {
                                /* Append the current string */
                                for (lm = ls; (pl < pm); *lm++ = *pl++)
                                        ;

                                /*
                                 * Append the rest of the pattern after the
                                 * closing brace
                                 */
                                for (pl = pe + 1; (*lm++ = *pl++) != BG_EOS; )
                                        ;

                                /* Expand the current pattern */
#ifdef GLOB_DEBUG
                                qprintf("globexp2:", patbuf);
#endif /* GLOB_DEBUG */
                                *rv = globexp1(patbuf, pglob);

                                /* move after the comma, to the next string */
                                pl = pm + 1;
                        }
                        break;

                default:
                        break;
                }
        }
        *rv = 0;
        return 0;
}



/*
 * expand tilde from the passwd file.
 */
static const Char *
globtilde(const Char *pattern, Char *patbuf, size_t patbuf_len, glob_t *pglob)
{
        char *h;
        const Char *p;
        Char *b, *eb;

        if (*pattern != BG_TILDE || !(pglob->gl_flags & GLOB_TILDE))
                return pattern;

        /* Copy up to the end of the string or / */
        eb = &patbuf[patbuf_len - 1];
        for (p = pattern + 1, h = (char *) patbuf;
             h < (char*)eb && *p && *p != BG_SLASH; *h++ = (char)*p++)
                ;

        *h = BG_EOS;

#if 0
        if (h == (char *)eb)
                return what;
#endif

        if (((char *) patbuf)[0] == BG_EOS) {
                /*
                 * handle a plain ~ or ~/ by expanding $HOME
                 * first and then trying the password file
                 * or $USERPROFILE on DOSISH systems
                 */
                if ((h = PerlEnv_getenv("HOME")) == NULL) {
#ifdef HAS_PASSWD
                        struct passwd *pwd;
                        if ((pwd = getpwuid(getuid())) == NULL)
                                return pattern;
                        else
                                h = pwd->pw_dir;
#elif DOSISH
                        /*
                         * When no passwd file, fallback to the USERPROFILE
                         * environment variable on DOSish systems.
                         */
                        if ((h = PerlEnv_getenv("USERPROFILE")) == NULL) {
                            return pattern;
                        }
#else
                        return pattern;
#endif
                }
        } else {
                /*
                 * Expand a ~user
                 */
#ifdef HAS_PASSWD
                struct passwd *pwd;
                if ((pwd = getpwnam((char*) patbuf)) == NULL)
                        return pattern;
                else
                        h = pwd->pw_dir;
#else
                return pattern;
#endif
        }

        /* Copy the home directory */
        for (b = patbuf; b < eb && *h; *b++ = *h++)
                ;

        /* Append the rest of the pattern */
        while (b < eb && (*b++ = *p++) != BG_EOS)
                ;
        *b = BG_EOS;

        return patbuf;
}


/*
 * The main glob() routine: compiles the pattern (optionally processing
 * quotes), calls glob1() to do the real pattern matching, and finally
 * sorts the list (unless unsorted operation is requested).  Returns 0
 * if things went well, nonzero if errors occurred.  It is not an error
 * to find no matches.
 */
static int
glob0(const Char *pattern, glob_t *pglob)
{
        const Char *qpat, *qpatnext;
        int c, err, oldflags, oldpathc;
        Char *bufnext, patbuf[MAXPATHLEN];
        size_t limit = 0;

        qpat = globtilde(pattern, patbuf, MAXPATHLEN, pglob);
        qpatnext = qpat;
        oldflags = pglob->gl_flags;
        oldpathc = pglob->gl_pathc;
        bufnext = patbuf;

        /* We don't need to check for buffer overflow any more. */
        while ((c = *qpatnext++) != BG_EOS) {
                switch (c) {
                case BG_LBRACKET:
                        c = *qpatnext;
                        if (c == BG_NOT)
                                ++qpatnext;
                        if (*qpatnext == BG_EOS ||
                            g_strchr((Char *) qpatnext+1, BG_RBRACKET) == NULL) {
                                *bufnext++ = BG_LBRACKET;
                                if (c == BG_NOT)
                                        --qpatnext;
                                break;
                        }
                        *bufnext++ = M_SET;
                        if (c == BG_NOT)
                                *bufnext++ = M_NOT;
                        c = *qpatnext++;
                        do {
                                *bufnext++ = CHAR(c);
                                if (*qpatnext == BG_RANGE &&
                                    (c = qpatnext[1]) != BG_RBRACKET) {
                                        *bufnext++ = M_RNG;
                                        *bufnext++ = CHAR(c);
                                        qpatnext += 2;
                                }
                        } while ((c = *qpatnext++) != BG_RBRACKET);
                        pglob->gl_flags |= GLOB_MAGCHAR;
                        *bufnext++ = M_END;
                        break;
                case BG_QUESTION:
                        pglob->gl_flags |= GLOB_MAGCHAR;
                        *bufnext++ = M_ONE;
                        break;
                case BG_STAR:
                        pglob->gl_flags |= GLOB_MAGCHAR;
                        /* Collapse adjacent stars to one.
                         * This is required to ensure that a pattern like
                         * "a**" matches a name like "a", as without this
                         * check when the first star matched everything it would
                         * cause the second star to return a match fail.
                         * As long ** is folded here this does not happen.
                         */
                        if (bufnext == patbuf || bufnext[-1] != M_ALL)
                                *bufnext++ = M_ALL;
                        break;
                default:
                        *bufnext++ = CHAR(c);
                        break;
                }
        }
        *bufnext = BG_EOS;
#ifdef GLOB_DEBUG
        qprintf("glob0:", patbuf);
#endif /* GLOB_DEBUG */

        if ((err = glob1(patbuf, patbuf+MAXPATHLEN-1, pglob, &limit)) != 0) {
                pglob->gl_flags = oldflags;
                return(err);
        }

        /*
         * If there was no match we are going to append the pattern
         * if GLOB_NOCHECK was specified or if GLOB_NOMAGIC was specified
         * and the pattern did not contain any magic characters
         * GLOB_NOMAGIC is there just for compatibility with csh.
         */
        if (pglob->gl_pathc == oldpathc &&
            ((pglob->gl_flags & GLOB_NOCHECK) ||
              ((pglob->gl_flags & GLOB_NOMAGIC) &&
               !(pglob->gl_flags & GLOB_MAGCHAR))))
        {
#ifdef GLOB_DEBUG
                printf("calling globextend from glob0\n");
#endif /* GLOB_DEBUG */
                pglob->gl_flags = oldflags;
                return(globextend(qpat, pglob, &limit));
        }
        else if (!(pglob->gl_flags & GLOB_NOSORT))
            if (pglob->gl_pathv)
                qsort(pglob->gl_pathv + pglob->gl_offs + oldpathc,
                    pglob->gl_pathc - oldpathc, sizeof(char *),
                    (pglob->gl_flags & (GLOB_ALPHASORT|GLOB_NOCASE))
                        ? ci_compare : compare);
        pglob->gl_flags = oldflags;
        return(0);
}

static int
ci_compare(const void *p, const void *q)
{
        const char *pp = *(const char **)p;
        const char *qq = *(const char **)q;
        int ci;
        while (*pp && *qq) {
                if (toFOLD(*pp) != toFOLD(*qq))
                        break;
                ++pp;
                ++qq;
        }
        ci = toFOLD(*pp) - toFOLD(*qq);
        if (ci == 0)
                return compare(p, q);
        return ci;
}

static int
compare(const void *p, const void *q)
{
        return(strcmp(*(char **)p, *(char **)q));
}

static int
glob1(Char *pattern, Char *pattern_last, glob_t *pglob, size_t *limitp)
{
        Char pathbuf[MAXPATHLEN];

        assert(pattern < pattern_last);

        /* A null pathname is invalid -- POSIX 1003.1 sect. 2.4. */
        if (*pattern == BG_EOS)
                return(0);
        return(glob2(pathbuf, pathbuf+MAXPATHLEN-1,
                     pathbuf, pathbuf+MAXPATHLEN-1,
                     pattern, pattern_last, pglob, limitp));
}

/*
 * The functions glob2 and glob3 are mutually recursive; there is one level
 * of recursion for each segment in the pattern that contains one or more
 * meta characters.
 */
static int
glob2(Char *pathbuf, Char *pathbuf_last, Char *pathend, Char *pathend_last,
      Char *pattern, Char *pattern_last, glob_t *pglob, size_t *limitp)
{
        Stat_t sb;
        Char *p, *q;
        int anymeta;

        assert(pattern < pattern_last);

        /*
         * Loop over pattern segments until end of pattern or until
         * segment with meta character found.
         */
        for (anymeta = 0;;) {
                if (*pattern == BG_EOS) {		/* End of pattern? */
                        *pathend = BG_EOS;
                        if (g_lstat(pathbuf, &sb, pglob))
                                return(0);

                        if (((pglob->gl_flags & GLOB_MARK) &&
                            pathend[-1] != BG_SEP
#ifdef DOSISH
                            && pathend[-1] != BG_SEP2
#endif
                            ) && (S_ISDIR(sb.st_mode) ||
                                  (S_ISLNK(sb.st_mode) &&
                            (g_stat(pathbuf, &sb, pglob) == 0) &&
                            S_ISDIR(sb.st_mode)))) {
                                if (pathend+1 > pathend_last)
                                        return (1);
                                *pathend++ = BG_SEP;
                                *pathend = BG_EOS;
                        }
                        ++pglob->gl_matchc;
#ifdef GLOB_DEBUG
                        printf("calling globextend from glob2\n");
#endif /* GLOB_DEBUG */
                        return(globextend(pathbuf, pglob, limitp));
                }

                /* Find end of next segment, copy tentatively to pathend. */
                q = pathend;
                p = pattern;
                while (*p != BG_EOS && *p != BG_SEP
#ifdef DOSISH
                       && *p != BG_SEP2
#endif
                       ) {
                        assert(p < pattern_last);
                        if (ismeta(*p))
                                anymeta = 1;
                        if (q+1 > pathend_last)
                                return (1);
                        *q++ = *p++;
                }

                if (!anymeta) {		/* No expansion, do next segment. */
                        pathend = q;
                        pattern = p;
                        while (*pattern == BG_SEP
#ifdef DOSISH
                               || *pattern == BG_SEP2
#endif
                               ) {
                                assert(p < pattern_last);
                                if (pathend+1 > pathend_last)
                                        return (1);
                                *pathend++ = *pattern++;
                        }
                } else
                        /* Need expansion, recurse. */
                        return(glob3(pathbuf, pathbuf_last, pathend,
                                     pathend_last, pattern,
                                     p, pattern_last, pglob, limitp));
        }
        /* NOTREACHED */
}

static int
glob3(Char *pathbuf, Char *pathbuf_last, Char *pathend, Char *pathend_last,
      Char *pattern,
      Char *restpattern, Char *restpattern_last, glob_t *pglob, size_t *limitp)
{
        Direntry_t *dp;
        DIR *dirp;
        int err;
        int nocase;
        char buf[MAXPATHLEN];

        /*
         * The readdirfunc declaration can't be prototyped, because it is
         * assigned, below, to two functions which are prototyped in glob.h
         * and dirent.h as taking pointers to differently typed opaque
         * structures.
         */
        Direntry_t *(*readdirfunc)(DIR*);

        assert(pattern < restpattern_last);
        assert(restpattern < restpattern_last);

        if (pathend > pathend_last)
                return (1);
        *pathend = BG_EOS;
        errno = 0;

#ifdef VMS
        {
                Char *q = pathend;
                if (q - pathbuf > 5) {
                        q -= 5;
                        if (q[0] == '.' &&
                            tolower(q[1]) == 'd' && tolower(q[2]) == 'i' &&
                            tolower(q[3]) == 'r' && q[4] == '/')
                        {
                                q[0] = '/';
                                q[1] = BG_EOS;
                                pathend = q+1;
                        }
                }
        }
#endif

        if ((dirp = g_opendir(pathbuf, pglob)) == NULL) {
                /* TODO: don't call for ENOENT or ENOTDIR? */
                if (pglob->gl_errfunc) {
                        if (g_Ctoc(pathbuf, buf, sizeof(buf)))
                                return (GLOB_ABEND);
                        if (pglob->gl_errfunc(buf, errno) ||
                            (pglob->gl_flags & GLOB_ERR))
                                return (GLOB_ABEND);
                }
                return(0);
        }

        err = 0;
        nocase = ((pglob->gl_flags & GLOB_NOCASE) != 0);

        /* Search directory for matching names. */
        if (pglob->gl_flags & GLOB_ALTDIRFUNC)
                readdirfunc = (Direntry_t *(*)(DIR *))pglob->gl_readdir;
        else
                readdirfunc = (Direntry_t *(*)(DIR *))my_readdir;
        while ((dp = (*readdirfunc)(dirp))) {
                U8 *sc;
                Char *dc;

                /* Initial BG_DOT must be matched literally. */
                if (dp->d_name[0] == BG_DOT && *pattern != BG_DOT)
                        continue;
                dc = pathend;
                sc = (U8 *) dp->d_name;
                while (dc < pathend_last && (*dc++ = *sc++) != BG_EOS)
                        ;
                if (dc >= pathend_last) {
                        *dc = BG_EOS;
                        err = 1;
                        break;
                }

                if (!match(pathend, pattern, restpattern, nocase)) {
                        *pathend = BG_EOS;
                        continue;
                }
                err = glob2(pathbuf, pathbuf_last, --dc, pathend_last,
                            restpattern, restpattern_last, pglob, limitp);
                if (err)
                        break;
        }

        if (pglob->gl_flags & GLOB_ALTDIRFUNC)
                (*pglob->gl_closedir)(dirp);
        else
                PerlDir_close(dirp);
        return(err);
}


/*
 * Extend the gl_pathv member of a glob_t structure to accommodate a new item,
 * add the new item, and update gl_pathc.
 *
 * This assumes the BSD realloc, which only copies the block when its size
 * crosses a power-of-two boundary; for v7 realloc, this would cause quadratic
 * behavior.
 *
 * Return 0 if new item added, error code if memory couldn't be allocated.
 *
 * Invariant of the glob_t structure:
 *	Either gl_pathc is zero and gl_pathv is NULL; or gl_pathc > 0 and
 *	gl_pathv points to (gl_offs + gl_pathc + 1) items.
 */
static int
globextend(const Char *path, glob_t *pglob, size_t *limitp)
{
        char **pathv;
        int i;
        STRLEN newsize, len;
        char *copy;
        const Char *p;

#ifdef GLOB_DEBUG
        printf("Adding ");
        for (p = path; *p; p++)
                (void)printf("%c", CHAR(*p));
        printf("\n");
#endif /* GLOB_DEBUG */

        newsize = sizeof(*pathv) * (2 + pglob->gl_pathc + pglob->gl_offs);
        if (pglob->gl_pathv)
                pathv = Renew(pglob->gl_pathv,newsize,char*);
        else
                Newx(pathv,newsize,char*);
        if (pathv == NULL) {
                if (pglob->gl_pathv) {
                        Safefree(pglob->gl_pathv);
                        pglob->gl_pathv = NULL;
                }
                return(GLOB_NOSPACE);
        }

        if (pglob->gl_pathv == NULL && pglob->gl_offs > 0) {
                /* first time around -- clear initial gl_offs items */
                pathv += pglob->gl_offs;
                for (i = pglob->gl_offs; --i >= 0; )
                        *--pathv = NULL;
        }
        pglob->gl_pathv = pathv;

        for (p = path; *p++;)
                ;
        len = (STRLEN)(p - path);
        *limitp += len;
        Newx(copy, p-path, char);
        if (copy != NULL) {
                if (g_Ctoc(path, copy, len)) {
                        Safefree(copy);
                        return(GLOB_NOSPACE);
                }
                pathv[pglob->gl_offs + pglob->gl_pathc++] = copy;
        }
        pathv[pglob->gl_offs + pglob->gl_pathc] = NULL;

        if ((pglob->gl_flags & GLOB_LIMIT) &&
            newsize + *limitp >= (unsigned long)ARG_MAX) {
                errno = 0;
                return(GLOB_NOSPACE);
        }

        return(copy == NULL ? GLOB_NOSPACE : 0);
}


/*
 * pattern matching function for filenames using state machine to avoid
 * recursion. We maintain a "nextp" and "nextn" to allow us to backtrack
 * without additional callframes, and to do cleanly prune the backtracking
 * state when multiple '*' (start) matches are included in the pattern.
 *
 * Thanks to Russ Cox for the improved state machine logic to avoid quadratic
 * matching on failure.
 *
 * https://research.swtch.com/glob
 *
 * An example would be a pattern
 *  ("a*" x 100) . "y"
 * against a file name like
 *  ("a" x 100) . "x"
 *
 */
static int
match(Char *name, Char *pat, Char *patend, int nocase)
{
        int ok, negate_range;
        Char c, k;
        Char *nextp = NULL;
        Char *nextn = NULL;

    redo:
        while (pat < patend) {
                c = *pat++;
                switch (c & M_MASK) {
                case M_ALL:
                        if (pat == patend)
                                return(1);
                        if (*name == BG_EOS)
                                return 0;
                        nextn = name + 1;
                        nextp = pat - 1;
                        break;
                case M_ONE:
                        /* since * matches leftmost-shortest first   *
                         * if we encounter the EOS then backtracking *
                         * will not help, so we can exit early here. */
                        if (*name++ == BG_EOS)
                                return 0;
                        break;
                case M_SET:
                        ok = 0;
                        /* since * matches leftmost-shortest first   *
                         * if we encounter the EOS then backtracking *
                         * will not help, so we can exit early here. */
                        if ((k = *name++) == BG_EOS)
                                return 0;
                        if ((negate_range = ((*pat & M_MASK) == M_NOT)) != BG_EOS)
                                ++pat;
                        while (((c = *pat++) & M_MASK) != M_END)
                                if ((*pat & M_MASK) == M_RNG) {
                                        if (nocase) {
                                                if (tolower(c) <= tolower(k) && tolower(k) <= tolower(pat[1]))
                                                        ok = 1;
                                        } else {
                                                if (c <= k && k <= pat[1])
                                                        ok = 1;
                                        }
                                        pat += 2;
                                } else if (nocase ? (tolower(c) == tolower(k)) : (c == k))
                                        ok = 1;
                        if (ok == negate_range)
                                goto fail;
                        break;
                default:
                        k = *name++;
                        if (nocase ? (tolower(k) != tolower(c)) : (k != c))
                                goto fail;
                        break;
                }
        }
        if (*name == BG_EOS)
                return 1;

    fail:
        if (nextn) {
                pat = nextp;
                name = nextn;
                goto redo;
        }
        return 0;
}

/* Free allocated data belonging to a glob_t structure. */
void
bsd_globfree(glob_t *pglob)
{
        int i;
        char **pp;

        if (pglob->gl_pathv != NULL) {
                pp = pglob->gl_pathv + pglob->gl_offs;
                for (i = pglob->gl_pathc; i--; ++pp)
                        if (*pp)
                                Safefree(*pp);
                Safefree(pglob->gl_pathv);
                pglob->gl_pathv = NULL;
        }
}

static DIR *
g_opendir(Char *str, glob_t *pglob)
{
        char buf[MAXPATHLEN];

        if (!*str) {
                my_strlcpy(buf, ".", sizeof(buf));
        } else {
                if (g_Ctoc(str, buf, sizeof(buf)))
                        return(NULL);
        }

        if (pglob->gl_flags & GLOB_ALTDIRFUNC)
                return((DIR*)(*pglob->gl_opendir)(buf));

        return(PerlDir_open(buf));
}

static int
g_lstat(Char *fn, Stat_t *sb, glob_t *pglob)
{
        char buf[MAXPATHLEN];

        if (g_Ctoc(fn, buf, sizeof(buf)))
                return(-1);
        if (pglob->gl_flags & GLOB_ALTDIRFUNC)
                return((*pglob->gl_lstat)(buf, sb));
#ifdef HAS_LSTAT
        return(PerlLIO_lstat(buf, sb));
#else
        return(PerlLIO_stat(buf, sb));
#endif /* HAS_LSTAT */
}

static int
g_stat(Char *fn, Stat_t *sb, glob_t *pglob)
{
        char buf[MAXPATHLEN];

        if (g_Ctoc(fn, buf, sizeof(buf)))
                return(-1);
        if (pglob->gl_flags & GLOB_ALTDIRFUNC)
                return((*pglob->gl_stat)(buf, sb));
        return(PerlLIO_stat(buf, sb));
}

static Char *
g_strchr(Char *str, int ch)
{
        do {
                if (*str == ch)
                        return (str);
        } while (*str++);
        return (NULL);
}

static int
g_Ctoc(const Char *str, char *buf, STRLEN len)
{
        while (len--) {
                if ((*buf++ = (char)*str++) == BG_EOS)
                        return (0);
        }
        return (1);
}

#ifdef GLOB_DEBUG
static void
qprintf(const char *str, Char *s)
{
        Char *p;

        (void)printf("%s:\n", str);
        for (p = s; *p; p++)
                (void)printf("%c", CHAR(*p));
        (void)printf("\n");
        for (p = s; *p; p++)
                (void)printf("%c", *p & M_PROTECT ? '"' : ' ');
        (void)printf("\n");
        for (p = s; *p; p++)
                (void)printf("%c", ismeta(*p) ? '_' : ' ');
        (void)printf("\n");
}
#endif /* GLOB_DEBUG */
