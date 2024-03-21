/*
 * utils.c - miscellaneous utilities
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
#include "utils.pro"

/* name of script being sourced */

/**/
mod_export char *scriptname;     /* is sometimes a function name */

/* filename of script or other file containing code source e.g. autoload */

/**/
mod_export char *scriptfilename;

/* != 0 if we are in a new style completion function */

/**/
mod_export int incompfunc;

#ifdef MULTIBYTE_SUPPORT
struct widechar_array {
    wchar_t *chars;
    size_t len;
};
typedef struct widechar_array *Widechar_array;

/*
 * The wordchars variable turned into a wide character array.
 * This is much more convenient for testing.
 */
static struct widechar_array wordchars_wide;

/*
 * The same for the separators (IFS) array.
 */
static struct widechar_array ifs_wide;

/* Function to set one of the above from the multibyte array */

static void
set_widearray(char *mb_array, Widechar_array wca)
{
    if (wca->chars) {
	free(wca->chars);
	wca->chars = NULL;
    }
    wca->len = 0;

    if (!isset(MULTIBYTE))
	return;

    if (mb_array) {
	VARARR(wchar_t, tmpwcs, strlen(mb_array));
	wchar_t *wcptr = tmpwcs;
	wint_t wci;

	mb_charinit();
	while (*mb_array) {
	    int mblen;

	    if (STOUC(*mb_array) <= 0x7f) {
		mb_array++;
		*wcptr++ = (wchar_t)*mb_array;
		continue;
	    }

	    mblen = mb_metacharlenconv(mb_array, &wci);

	    if (!mblen)
		break;
	    /* No good unless all characters are convertible */
	    if (wci == WEOF)
		return;
	    *wcptr++ = (wchar_t)wci;
#ifdef DEBUG
	    /*
	     * This generates a warning from the compiler (and is
	     * indeed useless) if chars are unsigned.  It's
	     * extreme paranoia anyway.
	     */
	    if (wcptr[-1] < 0)
		fprintf(stderr, "BUG: Bad cast to wchar_t\n");
#endif
	    mb_array += mblen;
	}

	wca->len = wcptr - tmpwcs;
	wca->chars = (wchar_t *)zalloc(wca->len * sizeof(wchar_t));
	wmemcpy(wca->chars, tmpwcs, wca->len);
    }
}
#endif


/* Print an error

   The following functions use the following printf-like format codes
   (implemented by zerrmsg()):

   Code	Argument types		Prints
   %s	const char *		C string (null terminated)
   %l	const char *, int	C string of given length (null not required)
   %L	long			decimal value
   %d	int			decimal value
   %z	zlong			decimal value
   %%	(none)			literal '%'
   %c	int			character at that codepoint
   %e	int			strerror() message (argument is typically 'errno')
 */

static void
zwarning(const char *cmd, const char *fmt, va_list ap)
{
    if (isatty(2))
	zleentry(ZLE_CMD_TRASH);

    char *prefix = scriptname ? scriptname : (argzero ? argzero : "");

    if (cmd) {
	if (unset(SHINSTDIN) || locallevel) {
	    nicezputs(prefix, stderr);
	    fputc((unsigned char)':', stderr);
	}
	nicezputs(cmd, stderr);
	fputc((unsigned char)':', stderr);
    } else {
	/*
	 * scriptname is set when sourcing scripts, so that we get the
	 * correct name instead of the generic name of whatever
	 * program/script is running.  It's also set in shell functions,
	 * so test locallevel, too.
	 */
	nicezputs((isset(SHINSTDIN) && !locallevel) ? "zsh" : prefix, stderr);
	fputc((unsigned char)':', stderr);
    }

    zerrmsg(stderr, fmt, ap);
}


/**/
mod_export void
zerr(VA_ALIST1(const char *fmt))
VA_DCL
{
    va_list ap;
    VA_DEF_ARG(const char *fmt);

    if (errflag || noerrs) {
	if (noerrs < 2)
	    errflag |= ERRFLAG_ERROR;
	return;
    }
    errflag |= ERRFLAG_ERROR;

    VA_START(ap, fmt);
    VA_GET_ARG(ap, fmt, const char *);
    zwarning(NULL, fmt, ap);
    va_end(ap);
}

/**/
mod_export void
zerrnam(VA_ALIST2(const char *cmd, const char *fmt))
VA_DCL
{
    va_list ap;
    VA_DEF_ARG(const char *cmd);
    VA_DEF_ARG(const char *fmt);

    if (errflag || noerrs)
	return;
    errflag |= ERRFLAG_ERROR;

    VA_START(ap, fmt);
    VA_GET_ARG(ap, cmd, const char *);
    VA_GET_ARG(ap, fmt, const char *);
    zwarning(cmd, fmt, ap);
    va_end(ap);
}

/**/
mod_export void
zwarn(VA_ALIST1(const char *fmt))
VA_DCL
{
    va_list ap;
    VA_DEF_ARG(const char *fmt);

    if (errflag || noerrs)
	return;

    VA_START(ap, fmt);
    VA_GET_ARG(ap, fmt, const char *);
    zwarning(NULL, fmt, ap);
    va_end(ap);
}

/**/
mod_export void
zwarnnam(VA_ALIST2(const char *cmd, const char *fmt))
VA_DCL
{
    va_list ap;
    VA_DEF_ARG(const char *cmd);
    VA_DEF_ARG(const char *fmt);

    if (errflag || noerrs)
	return;

    VA_START(ap, fmt);
    VA_GET_ARG(ap, cmd, const char *);
    VA_GET_ARG(ap, fmt, const char *);
    zwarning(cmd, fmt, ap);
    va_end(ap);
}


#ifdef DEBUG

/**/
mod_export void
dputs(VA_ALIST1(const char *message))
VA_DCL
{
    char *filename;
    FILE *file;
    va_list ap;
    VA_DEF_ARG(const char *message);

    VA_START(ap, message);
    VA_GET_ARG(ap, message, const char *);
    if ((filename = getsparam_u("ZSH_DEBUG_LOG")) != NULL &&
	(file = fopen(filename, "a")) != NULL) {
	zerrmsg(file, message, ap);
	fclose(file);
    } else
	zerrmsg(stderr, message, ap);
    va_end(ap);
}

#endif /* DEBUG */

#ifdef __CYGWIN__
/*
 * This works around an occasional problem with dllwrap on Cygwin, seen
 * on at least two installations.  It fails to find the last symbol
 * exported in alphabetical order (in our case zwarnnam).  Until this is
 * properly categorised and fixed we add a dummy symbol at the end.
 */
mod_export void
zz_plural_z_alpha(void)
{
}
#endif

/**/
void
zerrmsg(FILE *file, const char *fmt, va_list ap)
{
    const char *str;
    int num;
    long lnum;
#ifdef HAVE_STRERROR_R
#define ERRBUFSIZE (80)
    int olderrno;
    char errbuf[ERRBUFSIZE];
#endif
    char *errmsg;

    if ((unset(SHINSTDIN) || locallevel) && lineno) {
#if defined(ZLONG_IS_LONG_LONG) && defined(PRINTF_HAS_LLD)
	fprintf(file, "%lld: ", lineno);
#else
	fprintf(file, "%ld: ", (long)lineno);
#endif
    } else
	fputc((unsigned char)' ', file);

    while (*fmt)
	if (*fmt == '%') {
	    fmt++;
	    switch (*fmt++) {
	    case 's':
		str = va_arg(ap, const char *);
		nicezputs(str, file);
		break;
	    case 'l': {
		char *s;
		str = va_arg(ap, const char *);
		num = va_arg(ap, int);
		num = metalen(str, num);
		s = zhalloc(num + 1);
		memcpy(s, str, num);
		s[num] = '\0';
		nicezputs(s, file);
		break;
	    }
	    case 'L':
		lnum = va_arg(ap, long);
		fprintf(file, "%ld", lnum);
		break;
	    case 'd':
		num = va_arg(ap, int);
		fprintf(file, "%d", num);
		break;
	    case 'z':
	    {
		zlong znum = va_arg(ap, zlong);
		char buf[DIGBUFSIZE];
		convbase(buf, znum, 10);
		fputs(buf, file);
		break;
	    }
	    case '%':
		putc('%', file);
		break;
	    case 'c':
		num = va_arg(ap, int);
#ifdef MULTIBYTE_SUPPORT
		mb_charinit();
		zputs(wcs_nicechar(num, NULL, NULL), file);
#else
		zputs(nicechar(num), file);
#endif
		break;
	    case 'e':
		/* print the corresponding message for this errno */
		num = va_arg(ap, int);
		if (num == EINTR) {
		    fputs("interrupt\n", file);
		    errflag |= ERRFLAG_ERROR;
		    return;
		}
		errmsg = strerror(num);
		/* If the message is not about I/O problems, it looks better *
		 * if we uncapitalize the first letter of the message        */
		if (num == EIO)
		    fputs(errmsg, file);
		else {
		    fputc(tulower(errmsg[0]), file);
		    fputs(errmsg + 1, file);
		}
		break;
	    /* When adding format codes, update the comment above zwarning(). */
	    }
	} else {
	    putc(*fmt == Meta ? *++fmt ^ 32 : *fmt, file);
	    fmt++;
	}
    putc('\n', file);
    fflush(file);
}

/*
 * Wrapper for setupterm() and del_curterm().
 * These are called from terminfo.c and termcap.c.
 */
static int term_count;	/* reference count of cur_term */

/**/
mod_export void
zsetupterm(void)
{
#ifdef HAVE_SETUPTERM
    int errret;

    DPUTS(term_count < 0 || (term_count > 0 && !cur_term),
	    "inconsistent term_count and/or cur_term");
    /*
     * Just because we can't set up the terminal doesn't
     * mean the modules hasn't booted---TERM may change,
     * and it should be handled dynamically---so ignore errors here.
     */
    if (term_count++ == 0)
	(void)setupterm((char *)0, 1, &errret);
#endif
}

/**/
mod_export void
zdeleteterm(void)
{
#ifdef HAVE_SETUPTERM
    DPUTS(term_count < 1 || !cur_term,
	    "inconsistent term_count and/or cur_term");
    if (--term_count == 0)
	del_curterm(cur_term);
#endif
}

/* Output a single character, for the termcap routines.     *
 * This is used instead of putchar since it can be a macro. */

/**/
mod_export int
putraw(int c)
{
    putc(c, stdout);
    return 0;
}

/* Output a single character, for the termcap routines. */

/**/
mod_export int
putshout(int c)
{
    putc(c, shout);
    return 0;
}

/*
 * Turn a character into a visible representation thereof.  The visible
 * string is put together in a static buffer, and this function returns
 * a pointer to it.  Printable characters stand for themselves, DEL is
 * represented as "^?", newline and tab are represented as "\n" and
 * "\t", and normal control characters are represented in "^C" form.
 * Characters with bit 7 set, if unprintable, are represented as "\M-"
 * followed by the visible representation of the character with bit 7
 * stripped off.  Tokens are interpreted, rather than being treated as
 * literal characters.
 *
 * Note that the returned string is metafied, so that it must be
 * treated like any other zsh internal string (and not, for example,
 * output directly).
 *
 * This function is used even if MULTIBYTE_SUPPORT is defined: we
 * use it as a fallback in case we couldn't identify a wide character
 * in a multibyte string.
 */

/**/
mod_export char *
nicechar_sel(int c, int quotable)
{
    static char buf[10];
    char *s = buf;
    c &= 0xff;
    if (ZISPRINT(c))
	goto done;
    if (c & 0x80) {
	if (isset(PRINTEIGHTBIT))
	    goto done;
	*s++ = '\\';
	*s++ = 'M';
	*s++ = '-';
	c &= 0x7f;
	if(ZISPRINT(c))
	    goto done;
    }
    if (c == 0x7f) {
	if (quotable) {
	    *s++ = '\\';
	    *s++ = 'C';
	    *s++ = '-';
	} else
	    *s++ = '^';
	c = '?';
    } else if (c == '\n') {
	*s++ = '\\';
	c = 'n';
    } else if (c == '\t') {
	*s++ = '\\';
	c = 't';
    } else if (c < 0x20) {
	if (quotable) {
	    *s++ = '\\';
	    *s++ = 'C';
	    *s++ = '-';
	} else
	    *s++ = '^';
	c += 0x40;
    }
    done:
    /*
     * The resulting string is still metafied, so check if
     * we are returning a character in the range that needs metafication.
     * This can't happen if the character is printed "nicely", so
     * this results in a maximum of two bytes total (plus the null).
     */
    if (imeta(c)) {
	*s++ = Meta;
	*s++ = c ^ 32;
    } else
	*s++ = c;
    *s = 0;
    return buf;
}

/**/
mod_export char *
nicechar(int c)
{
    return nicechar_sel(c, 0);
}

/*
 * Return 1 if nicechar() would reformat this character.
 */

/**/
mod_export int
is_nicechar(int c)
{
    c &= 0xff;
    if (ZISPRINT(c))
	return 0;
    if (c & 0x80)
	return !isset(PRINTEIGHTBIT);
    return (c == 0x7f || c == '\n' || c == '\t' || c < 0x20);
}

/**/
#ifdef MULTIBYTE_SUPPORT
static mbstate_t mb_shiftstate;

/*
 * Initialise multibyte state: called before a sequence of
 * wcs_nicechar(), mb_metacharlenconv(), or
 * mb_charlenconv().
 */

/**/
mod_export void
mb_charinit(void)
{
    memset(&mb_shiftstate, 0, sizeof(mb_shiftstate));
}

/*
 * The number of bytes we need to allocate for a "nice" representation
 * of a multibyte character.
 *
 * We double MB_CUR_MAX to take account of the fact that
 * we may need to metafy.  In fact the representation probably
 * doesn't allow every character to be in the meta range, but
 * we don't need to be too pedantic.
 *
 * The 12 is for the output of a UCS-4 code; we don't actually
 * need this at the same time as MB_CUR_MAX, but again it's
 * not worth calculating more exactly.
 */
#define NICECHAR_MAX (12 + 2*MB_CUR_MAX)
/*
 * Input a wide character.  Output a printable representation,
 * which is a metafied multibyte string.   With widthp return
 * the printing width.
 *
 * swide, if non-NULL, is used to help the completion code, which needs
 * to know the printing width of the each part of the representation.
 * *swide is set to the part of the returned string where the wide
 * character starts.  Any string up to that point is ASCII characters,
 * so the width of it is (*swide - <return_value>).  Anything left is
 * a single wide character corresponding to the remaining width.
 * Either the initial ASCII part or the wide character part may be empty
 * (but not both).  (Note the complication that the wide character
 * part may contain metafied characters.)
 *
 * The caller needs to call mb_charinit() before the first call, to
 * set up the multibyte shift state for a range of characters.
 */

/**/
mod_export char *
wcs_nicechar_sel(wchar_t c, size_t *widthp, char **swidep, int quotable)
{
    static char *buf;
    static int bufalloc = 0, newalloc;
    char *s, *mbptr;
    int ret = 0;
    VARARR(char, mbstr, MB_CUR_MAX);

    /*
     * We want buf to persist beyond the return.  MB_CUR_MAX and hence
     * NICECHAR_MAX may not be constant, so we have to allocate this at
     * run time.  (We could probably get away with just allocating a
     * large buffer, in practice.)  For efficiency, only reallocate if
     * we really need to, since this function will be called frequently.
     */
    newalloc = NICECHAR_MAX;
    if (bufalloc != newalloc)
    {
	bufalloc = newalloc;
	buf = (char *)zrealloc(buf, bufalloc);
    }

    s = buf;
    if (!WC_ISPRINT(c) && (c < 0x80 || !isset(PRINTEIGHTBIT))) {
	if (c == 0x7f) {
	    if (quotable) {
		*s++ = '\\';
		*s++ = 'C';
		*s++ = '-';
	    } else
		*s++ = '^';
	    c = '?';
	} else if (c == L'\n') {
	    *s++ = '\\';
	    c = 'n';
	} else if (c == L'\t') {
	    *s++ = '\\';
	    c = 't';
	} else if (c < 0x20) {
	    if (quotable) {
		*s++ = '\\';
		*s++ = 'C';
		*s++ = '-';
	    } else
		*s++ = '^';
	    c += 0x40;
	} else if (c >= 0x80) {
	    ret = -1;
	}
    }

    if (ret != -1)
	ret = wcrtomb(mbstr, c, &mb_shiftstate);

    if (ret == -1) {
	memset(&mb_shiftstate, 0, sizeof(mb_shiftstate));
	/*
	 * Can't or don't want to convert character: use UCS-2 or
	 * UCS-4 code in print escape format.
	 *
	 * This comparison fails and generates a compiler warning
	 * if wchar_t is 16 bits, but the code is still correct.
	 */
	if (c >=  0x10000) {
	    sprintf(buf, "\\U%.8x", (unsigned int)c);
	    if (widthp)
		*widthp = 10;
	} else if (c >= 0x100) {
	    sprintf(buf, "\\u%.4x", (unsigned int)c);
	    if (widthp)
		*widthp = 6;
	} else {
	    strcpy(buf, nicechar_sel((int)c, quotable));
	    /*
	     * There may be metafied characters from nicechar(),
	     * so compute width and end position independently.
	     */
	    if (widthp)
		*widthp = ztrlen(buf);
	    if (swidep)
	      *swidep = buf + strlen(buf);
	    return buf;
	}
	if (swidep)
	    *swidep = widthp ? buf + *widthp : buf;
	return buf;
    }

    if (widthp) {
	int wcw = WCWIDTH(c);
	*widthp = (s - buf);
	if (wcw >= 0)
	    *widthp += wcw;
	else
	    (*widthp)++;
    }
    if (swidep)
	*swidep = s;
    for (mbptr = mbstr; ret; s++, mbptr++, ret--) {
	DPUTS(s >= buf + NICECHAR_MAX,
	      "BUG: buffer too small in wcs_nicechar");
	if (imeta(*mbptr)) {
	    *s++ = Meta;
	    DPUTS(s >= buf + NICECHAR_MAX,
		  "BUG: buffer too small for metafied char in wcs_nicechar");
	    *s = *mbptr ^ 32;
	} else {
	    *s = *mbptr;
	}
    }
    *s = 0;
    return buf;
}

/**/
mod_export char *
wcs_nicechar(wchar_t c, size_t *widthp, char **swidep)
{
    return wcs_nicechar_sel(c, widthp, swidep, 0);
}

/*
 * Return 1 if wcs_nicechar() would reformat this character for display.
 */

/**/
mod_export int is_wcs_nicechar(wchar_t c)
{
    if (!WC_ISPRINT(c) && (c < 0x80 || !isset(PRINTEIGHTBIT))) {
	if (c == 0x7f || c == L'\n' || c == L'\t' || c < 0x20)
	    return 1;
	if (c >= 0x80) {
	    return (c >= 0x100 || is_nicechar((int)c));
	}
    }
    return 0;
}

/**/
mod_export int
zwcwidth(wint_t wc)
{
    int wcw;
    /* assume a single-byte character if not valid */
    if (wc == WEOF || unset(MULTIBYTE))
	return 1;
    wcw = WCWIDTH(wc);
    /* if not printable, assume width 1 */
    if (wcw < 0)
	return 1;
    return wcw;
}

/**/
#endif /* MULTIBYTE_SUPPORT */

/*
 * Search the path for prog and return the file name.
 * The returned value is unmetafied and in the unmeta storage
 * area (N.B. should be duplicated if not used immediately and not
 * equal to *namep).
 *
 * If namep is not NULL, *namep is set to the metafied programme
 * name, which is in heap storage.
 */
/**/
char *
pathprog(char *prog, char **namep)
{
    char **pp, ppmaxlen = 0, *buf, *funmeta;
    struct stat st;

    for (pp = path; *pp; pp++)
    {
	int len = strlen(*pp);
	if (len > ppmaxlen)
	    ppmaxlen = len;
    }
    buf = zhalloc(ppmaxlen + strlen(prog) + 2);
    for (pp = path; *pp; pp++) {
	sprintf(buf, "%s/%s", *pp, prog);
	funmeta = unmeta(buf);
	if (access(funmeta, F_OK) == 0 &&
	    stat(funmeta, &st) >= 0 &&
	    !S_ISDIR(st.st_mode)) {
	    if (namep)
		*namep = buf;
	    return funmeta;
	}
    }

    return NULL;
}

/* get a symlink-free pathname for s relative to PWD */

/**/
char *
findpwd(char *s)
{
    char *t;

    if (*s == '/')
	return xsymlink(s, 0);
    s = tricat((pwd[1]) ? pwd : "", "/", s);
    t = xsymlink(s, 0);
    zsfree(s);
    return t;
}

/* Check whether a string contains the *
 * name of the present directory.      */

/**/
int
ispwd(char *s)
{
    struct stat sbuf, tbuf;

    /* POSIX: environment PWD must be absolute */
    if (*s != '/')
	return 0;

    if (stat((s = unmeta(s)), &sbuf) == 0 && stat(".", &tbuf) == 0)
	if (sbuf.st_dev == tbuf.st_dev && sbuf.st_ino == tbuf.st_ino) {
	    /* POSIX: No element of $PWD may be "." or ".." */
	    while (*s) {
		if (s[0] == '.' &&
		    (!s[1] || s[1] == '/' ||
		     (s[1] == '.' && (!s[2] || s[2] == '/'))))
		    break;
		while (*s++ != '/' && *s)
		    continue;
	    }
	    return !*s;
	}
    return 0;
}

static char xbuf[PATH_MAX*2+1];

/**/
static char **
slashsplit(char *s)
{
    char *t, **r, **q;
    int t0;

    if (!*s)
	return (char **) zshcalloc(sizeof(char *));

    for (t = s, t0 = 0; *t; t++)
	if (*t == '/')
	    t0++;
    q = r = (char **) zalloc(sizeof(char *) * (t0 + 2));

    while ((t = strchr(s, '/'))) {
	*q++ = ztrduppfx(s, t - s);
	while (*t == '/')
	    t++;
	if (!*t) {
	    *q = NULL;
	    return r;
	}
	s = t;
    }
    *q++ = ztrdup(s);
    *q = NULL;
    return r;
}

/* expands .. or . expressions and one level of symlinks
 *
 * Puts the result in the global "xbuf"
 */

/**/
static int
xsymlinks(char *s)
{
    char **pp, **opp;
    char xbuf2[PATH_MAX*3+1], xbuf3[PATH_MAX*2+1];
    int t0, ret = 0;
    zulong xbuflen = strlen(xbuf), pplen;

    opp = pp = slashsplit(s);
    for (; xbuflen < sizeof(xbuf) && *pp && ret >= 0; pp++) {
	if (!strcmp(*pp, "."))
	    continue;
	if (!strcmp(*pp, "..")) {
	    char *p;

	    if (!strcmp(xbuf, "/"))
		continue;
	    if (!*xbuf)
		continue;
	    p = xbuf + xbuflen;
	    while (*--p != '/')
		xbuflen--;
	    *p = '\0';
	    /* The \0 isn't included in the length */
	    xbuflen--;
	    continue;
	}
	/* Includes null byte. */
	pplen = strlen(*pp) + 1;
	if (xbuflen + pplen + 1 > sizeof(xbuf2)) {
	    *xbuf = 0;
	    ret = -1;
	    break;
	}
	memcpy(xbuf2, xbuf, xbuflen);
	xbuf2[xbuflen] = '/';
	memcpy(xbuf2 + xbuflen + 1, *pp, pplen);
	t0 = readlink(unmeta(xbuf2), xbuf3, PATH_MAX);
	if (t0 == -1) {
	    if ((xbuflen += pplen) < sizeof(xbuf)) {
		strcat(xbuf, "/");
		strcat(xbuf, *pp);
	    } else {
		*xbuf = 0;
		ret = -1;
		break;
	    }
	} else {
	    ret = 1;
	    metafy(xbuf3, t0, META_NOALLOC);
	    {
		/*
		 * If only one expansion requested, ensure the
		 * full path is in xbuf.
		 */
		zulong len = xbuflen;
		if (*xbuf3 == '/')
		    strcpy(xbuf, xbuf3);
		else if ((len += strlen(xbuf3) + 1) < sizeof(xbuf)) {
		    strcpy(xbuf + xbuflen, "/");
		    strcpy(xbuf + xbuflen + 1, xbuf3);
		} else {
		    *xbuf = 0;
		    ret = -1;
		    break;
		}

		while (*++pp) {
		    zulong newlen = len + strlen(*pp) + 1;
		    if (newlen < sizeof(xbuf)) {
			strcpy(xbuf + len, "/");
			strcpy(xbuf + len + 1, *pp);
			len = newlen;
		    } else {
			*xbuf = 01;
			ret = -1;
			break;
		    }
		}
		/*
		 * No need to update xbuflen, we're finished
		 * the expansion (for now).
		 */
		break;
	    }
	}
    }
    freearray(opp);
    return ret;
}

/*
 * expand symlinks in s, and remove other weird things:
 * note that this always expands symlinks.
 *
 * 'heap' indicates whether to malloc() or allocate on the heap.
 */

/**/
mod_export char *
xsymlink(char *s, int heap)
{
    if (*s != '/')
	return NULL;
    *xbuf = '\0';
    if (!chrealpath(&s, 'P', heap)) {
	zwarn("path expansion failed, using root directory");
	return heap ? dupstring("/") : ztrdup("/");
    }
    return s;
}

/**/
void
print_if_link(char *s, int all)
{
    if (*s == '/') {
	if (all) {
	    char *start = s + 1;
	    char xbuflink[PATH_MAX+1];
	    *xbuf = '\0';
	    for (;;) {
		if (xsymlinks(start) > 0) {
		    printf(" -> ");
		    zputs(*xbuf ? xbuf : "/", stdout);
		    if (!*xbuf)
			break;
		    strcpy(xbuflink, xbuf);
		    start = xbuflink + 1;
		    *xbuf = '\0';
		} else {
		    break;
		}
	    }
	} else {
	    if (chrealpath(&s, 'P', 0)) {
		printf(" -> ");
		zputs(*s ? s : "/", stdout);
		zsfree(s);
	    }
	}
    }
}

/* print a directory */

/**/
void
fprintdir(char *s, FILE *f)
{
    Nameddir d = finddir(s);

    if (!d)
	fputs(unmeta(s), f);
    else {
	putc('~', f);
	fputs(unmeta(d->node.nam), f);
	fputs(unmeta(s + strlen(d->dir)), f);
    }
}

/*
 * Substitute a directory using a name.
 * If there is none, return the original argument.
 *
 * At this level all strings involved are metafied.
 */

/**/
char *
substnamedir(char *s)
{
    Nameddir d = finddir(s);

    if (!d)
	return quotestring(s, QT_BACKSLASH);
    return zhtricat("~", d->node.nam, quotestring(s + strlen(d->dir),
						  QT_BACKSLASH));
}


/* Returns the current username.  It caches the username *
 * and uid to try to avoid requerying the password files *
 * or other source.                                      */

/**/
uid_t cached_uid;
/**/
char *cached_username;

/**/
char *
get_username(void)
{
#ifdef USE_GETPWUID
    struct passwd *pswd;
    uid_t current_uid;

    current_uid = getuid();
    if (current_uid != cached_uid) {
	cached_uid = current_uid;
	zsfree(cached_username);
	if ((pswd = getpwuid(current_uid)))
	    cached_username = ztrdup(pswd->pw_name);
	else
	    cached_username = ztrdup("");
    }
#else /* !USE_GETPWUID */
    cached_uid = getuid();
#endif /* !USE_GETPWUID */
    return cached_username;
}

/* static variables needed by finddir(). */

static char *finddir_full;
static Nameddir finddir_last;
static int finddir_best;

/* ScanFunc used by finddir(). */

/**/
static void
finddir_scan(HashNode hn, UNUSED(int flags))
{
    Nameddir nd = (Nameddir) hn;

    if(nd->diff > finddir_best && !dircmp(nd->dir, finddir_full)
       && !(nd->node.flags & ND_NOABBREV)) {
	finddir_last=nd;
	finddir_best=nd->diff;
    }
}

/*
 * See if a path has a named directory as its prefix.
 * If passed a NULL argument, it will invalidate any 
 * cached information.
 *
 * s here is metafied.
 */

/**/
Nameddir
finddir(char *s)
{
    static struct nameddir homenode = { {NULL, "", 0}, NULL, 0 };
    static int ffsz;
    char **ares;
    int len;

    /* Invalidate directory cache if argument is NULL.  This is called *
     * whenever a node is added to or removed from the hash table, and *
     * whenever the value of $HOME changes.  (On startup, too.)        */
    if (!s) {
	homenode.dir = home ? home : "";
	homenode.diff = home ? strlen(home) : 0;
	if(homenode.diff==1)
	    homenode.diff = 0;
	if(!finddir_full)
	    finddir_full = zalloc(ffsz = PATH_MAX+1);
	finddir_full[0] = 0;
	return finddir_last = NULL;
    }

#if 0
    /*
     * It's not safe to use the cache while we have function
     * transformations, and it's not clear it's worth the
     * complexity of guessing here whether subst_string_by_hook
     * is going to turn up the goods.
     */
    if (!strcmp(s, finddir_full) && *finddir_full)
	return finddir_last;
#endif

    if ((int)strlen(s) >= ffsz) {
	free(finddir_full);
	finddir_full = zalloc(ffsz = strlen(s) * 2);
    }
    strcpy(finddir_full, s);
    finddir_best=0;
    finddir_last=NULL;
    finddir_scan(&homenode.node, 0);
    scanhashtable(nameddirtab, 0, 0, 0, finddir_scan, 0);

    ares = subst_string_by_hook("zsh_directory_name", "d", finddir_full);
    if (ares && arrlen_ge(ares, 2) &&
	(len = (int)zstrtol(ares[1], NULL, 10)) > finddir_best) {
	/* better duplicate this string since it's come from REPLY */
	finddir_last = (Nameddir)hcalloc(sizeof(struct nameddir));
	finddir_last->node.nam = zhtricat("[", dupstring(ares[0]), "]");
	finddir_last->dir = dupstrpfx(finddir_full, len);
	finddir_last->diff = len - strlen(finddir_last->node.nam);
	finddir_best = len;
    }

    return finddir_last;
}

/* add a named directory */

/**/
mod_export void
adduserdir(char *s, char *t, int flags, int always)
{
    Nameddir nd;
    char *eptr;

    /* We don't maintain a hash table in non-interactive shells. */
    if (!interact)
	return;

    /* The ND_USERNAME flag means that this possible hash table *
     * entry is derived from a passwd entry.  Such entries are  *
     * subordinate to explicitly generated entries.             */
    if ((flags & ND_USERNAME) && nameddirtab->getnode2(nameddirtab, s))
	return;

    /* Normal parameter assignments generate calls to this function, *
     * with always==0.  Unless the AUTO_NAME_DIRS option is set, we  *
     * don't let such assignments actually create directory names.   *
     * Instead, a reference to the parameter as a directory name can *
     * cause the actual creation of the hash table entry.            */
    if (!always && unset(AUTONAMEDIRS) &&
	    !nameddirtab->getnode2(nameddirtab, s))
	return;

    if (!t || *t != '/' || strlen(t) >= PATH_MAX) {
	/* We can't use this value as a directory, so simply remove *
	 * the corresponding entry in the hash table, if any.       */
	HashNode hn = nameddirtab->removenode(nameddirtab, s);

	if(hn)
	    nameddirtab->freenode(hn);
	return;
    }

    /* add the name */
    nd = (Nameddir) zshcalloc(sizeof *nd);
    nd->node.flags = flags;
    eptr = t + strlen(t);
    while (eptr > t && eptr[-1] == '/')
	eptr--;
    if (eptr == t) {
	/*
	 * Don't abbreviate multiple slashes at the start of a
	 * named directory, since these are sometimes used for
	 * special purposes.
	 */
	nd->dir = metafy(t, -1, META_DUP);
    } else
	nd->dir = metafy(t, eptr - t, META_DUP);
    /* The variables PWD and OLDPWD are not to be displayed as ~PWD etc. */
    if (!strcmp(s, "PWD") || !strcmp(s, "OLDPWD"))
	nd->node.flags |= ND_NOABBREV;
    nameddirtab->addnode(nameddirtab, metafy(s, -1, META_DUP), nd);
}

/* Get a named directory: this function can cause a directory name *
 * to be added to the hash table, if it isn't there already.       */

/**/
char *
getnameddir(char *name)
{
    Param pm;
    char *str;
    Nameddir nd;

    /* Check if it is already in the named directory table */
    if ((nd = (Nameddir) nameddirtab->getnode(nameddirtab, name)))
	return dupstring(nd->dir);

    /* Check if there is a scalar parameter with this name whose value *
     * begins with a `/'.  If there is, add it to the hash table and   *
     * return the new value.                                           */
    if ((pm = (Param) paramtab->getnode(paramtab, name)) &&
	    (PM_TYPE(pm->node.flags) == PM_SCALAR) &&
	    (str = getsparam(name)) && *str == '/') {
	pm->node.flags |= PM_NAMEDDIR;
	adduserdir(name, str, 0, 1);
	return str;
    }

#ifdef USE_GETPWNAM
    {
	/* Retrieve an entry from the password table/database for this user. */
	struct passwd *pw;
	if ((pw = getpwnam(name))) {
	    char *dir = isset(CHASELINKS) ? xsymlink(pw->pw_dir, 0)
		: ztrdup(pw->pw_dir);
	    if (dir) {
		adduserdir(name, dir, ND_USERNAME, 1);
		str = dupstring(dir);
		zsfree(dir);
		return str;
	    } else
		return dupstring(pw->pw_dir);
	}
    }
#endif /* USE_GETPWNAM */

    /* There are no more possible sources of directory names, so give up. */
    return NULL;
}

/*
 * Compare directories.  Both are metafied.
 */

/**/
static int
dircmp(char *s, char *t)
{
    if (s) {
	for (; *s == *t; s++, t++)
	    if (!*s)
		return 0;
	if (!*s && *t == '/')
	    return 0;
    }
    return 1;
}

/*
 * Extra functions to call before displaying the prompt.
 * The data is a Prepromptfn.
 */

static LinkList prepromptfns;

/* Add a function to the list of pre-prompt functions. */

/**/
mod_export void
addprepromptfn(voidvoidfnptr_t func)
{
    Prepromptfn ppdat = (Prepromptfn)zalloc(sizeof(struct prepromptfn));
    ppdat->func = func;
    if (!prepromptfns)
	prepromptfns = znewlinklist();
    zaddlinknode(prepromptfns, ppdat);
}

/* Remove a function from the list of pre-prompt functions. */

/**/
mod_export void
delprepromptfn(voidvoidfnptr_t func)
{
    LinkNode ln;

    if (!prepromptfns)
	return;

    for (ln = firstnode(prepromptfns); ln; ln = nextnode(ln)) {
	Prepromptfn ppdat = (Prepromptfn)getdata(ln);
	if (ppdat->func == func) {
	    (void)remnode(prepromptfns, ln);
	    zfree(ppdat, sizeof(struct prepromptfn));
	    return;
	}
    }
#ifdef DEBUG
    dputs("BUG: failed to delete node from prepromptfns");
#endif
}

/*
 * Functions to call at a particular time even if not at
 * the prompt.  This is handled by zle.  The data is a
 * Timedfn.  The functions must be in time order, but this
 * is enforced by addtimedfn().
 *
 * Note on debugging:  the code in sched.c currently assumes it's
 * the only user of timedfns for the purposes of checking whether
 * there's a function on the list.  If this becomes no longer the case,
 * the DPUTS() tests in sched.c need rewriting.
 */

/**/
mod_export LinkList timedfns;

/* Add a function to the list of timed functions. */

/**/
mod_export void
addtimedfn(voidvoidfnptr_t func, time_t when)
{
    Timedfn tfdat = (Timedfn)zalloc(sizeof(struct timedfn));
    tfdat->func = func;
    tfdat->when = when;

    if (!timedfns) {
	timedfns = znewlinklist();
	zaddlinknode(timedfns, tfdat);
    } else {
	LinkNode ln = firstnode(timedfns);

	/*
	 * Insert the new element in the linked list.  We do
	 * rather too much work here since the standard
	 * functions insert after a given node, whereas we
	 * want to insert the new data before the first element
	 * with a greater time.
	 *
	 * In practice, the only use of timed functions is
	 * sched, which only adds the one function; so this
	 * whole branch isn't used beyond the following block.
	 */
	if (!ln) {
	    zaddlinknode(timedfns, tfdat);
	    return;
	}
	for (;;) {
	    Timedfn tfdat2;
	    LinkNode next = nextnode(ln);
	    if (!next) {
		zaddlinknode(timedfns, tfdat);
		return;
	    }
	    tfdat2 = (Timedfn)getdata(next);
	    if (when < tfdat2->when) {
		zinsertlinknode(timedfns, ln, tfdat);
		return;
	    }
	    ln = next;
	}
    }
}

/*
 * Delete a function from the list of timed functions.
 * Note that if the function apperas multiple times only
 * the first occurrence will be removed.
 *
 * Note also that when zle calls the function it does *not*
 * automatically delete the entry from the list.  That must
 * be done by the function called.  This is recommended as otherwise
 * the function will keep being called immediately.  (It just so
 * happens this "feature" fits in well with the only current use
 * of timed functions.)
 */

/**/
mod_export void
deltimedfn(voidvoidfnptr_t func)
{
    LinkNode ln;

    for (ln = firstnode(timedfns); ln; ln = nextnode(ln)) {
	Timedfn ppdat = (Timedfn)getdata(ln);
	if (ppdat->func == func) {
	    (void)remnode(timedfns, ln);
	    zfree(ppdat, sizeof(struct timedfn));
	    return;
	}
    }
#ifdef DEBUG
    dputs("BUG: failed to delete node from timedfns");
#endif
}

/* the last time we checked mail */

/**/
time_t lastmailcheck;

/*
 * Call a function given by "name" with optional arguments
 * "lnklst".  If these are present the first argument is the function name.
 *
 * If "arrayp" is not zero, we also look through
 * the array "name"_functions and execute functions found there.
 *
 * If "retval" is not NULL, the return value of the first hook function to
 * return non-zero is stored in *"retval".  The return value is not otherwise
 * available as the calling context is restored.
 *
 * Returns 0 if at least one function was called (regardless of that function's
 * exit status), and 1 otherwise.
 */

/**/
mod_export int
callhookfunc(char *name, LinkList lnklst, int arrayp, int *retval)
{
    Shfunc shfunc;
	/*
	 * Save stopmsg, since user doesn't get a chance to respond
	 * to a list of jobs generated in a hook.
	 */
    int osc = sfcontext, osm = stopmsg, stat = 1, ret = 0;
    int old_incompfunc = incompfunc;

    sfcontext = SFC_HOOK;
    incompfunc = 0;

    if ((shfunc = getshfunc(name))) {
	if (!lnklst) {
	    lnklst = newlinklist();
	    addlinknode(lnklst, name);
	}
	ret = doshfunc(shfunc, lnklst, 1);
	stat = 0;
    }

    if (arrayp) {
	char **arrptr;
	int namlen = strlen(name);
	VARARR(char, arrnam, namlen + HOOK_SUFFIX_LEN);
	memcpy(arrnam, name, namlen);
	memcpy(arrnam + namlen, HOOK_SUFFIX, HOOK_SUFFIX_LEN);

	if ((arrptr = getaparam(arrnam))) {
	    char **argarr = lnklst ? hlinklist2array(lnklst, 0) : NULL;
	    arrptr = arrdup(arrptr);
	    for (; *arrptr; arrptr++) {
		if ((shfunc = getshfunc(*arrptr))) {
		    int newret, i = 1;
		    LinkList arg0 = newlinklist();
		    addlinknode(arg0, *arrptr);
		    while (argarr && argarr[i])
			addlinknode(arg0, argarr[i++]);
		    newret = doshfunc(shfunc, arg0, 1);
		    if (!ret)
			ret = newret;
		    stat = 0;
		}
	    }
	}
    }

    sfcontext = osc;
    stopmsg = osm;
    incompfunc = old_incompfunc;

    if (retval)
	*retval = ret;
    return stat;
}

/* do pre-prompt stuff */

/**/
void
preprompt(void)
{
    static time_t lastperiodic;
    time_t currentmailcheck;
    LinkNode ln;
    zlong period = getiparam("PERIOD");
    zlong mailcheck = getiparam("MAILCHECK");

    /*
     * Handle any pending window size changes before we compute prompts,
     * then block them again to avoid interrupts during prompt display.
     */
    winch_unblock();
    winch_block();

    if (isset(PROMPTSP) && isset(PROMPTCR) && !use_exit_printed && shout) {
	/* The PROMPT_SP heuristic will move the prompt down to a new line
	 * if there was any dangling output on the line (assuming the terminal
	 * has automatic margins, but we try even if hasam isn't set).
	 * Unfortunately it interacts badly with ZLE displaying message
	 * when ^D has been pressed. So just disable PROMPT_SP logic in
	 * this case */
	char *eolmark = getsparam("PROMPT_EOL_MARK");
	char *str;
	int percents = opts[PROMPTPERCENT], w = 0;
	if (!eolmark)
	    eolmark = "%B%S%#%s%b";
	opts[PROMPTPERCENT] = 1;
	str = promptexpand(eolmark, 1, NULL, NULL, NULL);
	countprompt(str, &w, 0, -1);
	opts[PROMPTPERCENT] = percents;
	zputs(str, shout);
	fprintf(shout, "%*s\r%*s\r", (int)zterm_columns - w - !hasxn,
		"", w, "");
	fflush(shout);
	free(str);
    }

    /* If NOTIFY is not set, then check for completed *
     * jobs before we print the prompt.               */
    if (unset(NOTIFY))
	scanjobs();
    if (errflag)
	return;

    /* If a shell function named "precmd" exists, *
     * then execute it.                           */
    callhookfunc("precmd", NULL, 1, NULL);
    if (errflag)
	return;

    /* If 1) the parameter PERIOD exists, 2) a hook function for    *
     * "periodic" exists, 3) it's been greater than PERIOD since we *
     * executed any such hook, then execute it now.                 */
    if (period && ((zlong)time(NULL) > (zlong)lastperiodic + period) &&
	!callhookfunc("periodic", NULL, 1, NULL))
	lastperiodic = time(NULL);
    if (errflag)
	return;

    /* Check mail */
    currentmailcheck = time(NULL);
    if (mailcheck &&
	(zlong) difftime(currentmailcheck, lastmailcheck) > mailcheck) {
	char *mailfile;

	if (mailpath && *mailpath && **mailpath)
	    checkmailpath(mailpath);
	else {
	    queue_signals();
	    if ((mailfile = getsparam("MAIL")) && *mailfile) {
		char *x[2];

		x[0] = mailfile;
		x[1] = NULL;
		checkmailpath(x);
	    }
	    unqueue_signals();
	}
	lastmailcheck = currentmailcheck;
    }

    if (prepromptfns) {
	for(ln = firstnode(prepromptfns); ln; ln = nextnode(ln)) {
	    Prepromptfn ppnode = (Prepromptfn)getdata(ln);
	    ppnode->func();
	}
    }
}

/**/
static void
checkmailpath(char **s)
{
    struct stat st;
    char *v, *u, c;

    while (*s) {
	for (v = *s; *v && *v != '?'; v++);
	c = *v;
	*v = '\0';
	if (c != '?')
	    u = NULL;
	else
	    u = v + 1;
	if (**s == 0) {
	    *v = c;
	    zerr("empty MAILPATH component: %s", *s);
	} else if (mailstat(unmeta(*s), &st) == -1) {
	    if (errno != ENOENT)
		zerr("%e: %s", errno, *s);
	} else if (S_ISDIR(st.st_mode)) {
	    LinkList l;
	    DIR *lock = opendir(unmeta(*s));
	    char buf[PATH_MAX * 2 + 1], **arr, **ap;
	    int buflen, ct = 1;

	    if (lock) {
		char *fn;

		pushheap();
		l = newlinklist();
		while ((fn = zreaddir(lock, 1)) && !errflag) {
		    if (u)
			buflen = snprintf(buf, sizeof(buf), "%s/%s?%s", *s, fn, u);
		    else
			buflen = snprintf(buf, sizeof(buf), "%s/%s", *s, fn);
		    if (buflen < 0 || buflen >= (int)sizeof(buf))
			continue;
		    addlinknode(l, dupstring(buf));
		    ct++;
		}
		closedir(lock);
		ap = arr = (char **) zhalloc(ct * sizeof(char *));

		while ((*ap++ = (char *)ugetnode(l)));
		checkmailpath(arr);
		popheap();
	    }
	} else if (shout) {
	    if (st.st_size && st.st_atime <= st.st_mtime &&
		st.st_mtime >= lastmailcheck) {
		if (!u) {
		    fprintf(shout, "You have new mail.\n");
		    fflush(shout);
		} else {
		    char *usav;
		    int uusav = underscoreused;

		    usav = zalloc(underscoreused);

		    if (usav)
			memcpy(usav, zunderscore, underscoreused);

		    setunderscore(*s);

		    u = dupstring(u);
		    if (!parsestr(&u)) {
			singsub(&u);
			zputs(u, shout);
			fputc('\n', shout);
			fflush(shout);
		    }
		    if (usav) {
			setunderscore(usav);
			zfree(usav, uusav);
		    }
		}
	    }
	    if (isset(MAILWARNING) && st.st_atime > st.st_mtime &&
		st.st_atime > lastmailcheck && st.st_size) {
		fprintf(shout, "The mail in %s has been read.\n", unmeta(*s));
		fflush(shout);
	    }
	}
	*v = c;
	s++;
    }
}

/* This prints the XTRACE prompt. */

/**/
FILE *xtrerr = 0;

/**/
void
printprompt4(void)
{
    if (!xtrerr)
	xtrerr = stderr;
    if (prompt4) {
	int l, t = opts[XTRACE];
	char *s = dupstring(prompt4);

	opts[XTRACE] = 0;
	unmetafy(s, &l);
	s = unmetafy(promptexpand(metafy(s, l, META_NOALLOC),
				  0, NULL, NULL, NULL), &l);
	opts[XTRACE] = t;

	fprintf(xtrerr, "%s", s);
	free(s);
    }
}

/**/
mod_export void
freestr(void *a)
{
    zsfree(a);
}

/**/
mod_export void
gettyinfo(struct ttyinfo *ti)
{
    if (SHTTY != -1) {
#ifdef HAVE_TERMIOS_H
# ifdef HAVE_TCGETATTR
	if (tcgetattr(SHTTY, &ti->tio) == -1)
# else
	if (ioctl(SHTTY, TCGETS, &ti->tio) == -1)
# endif
	    zerr("bad tcgets: %e", errno);
#else
# ifdef HAVE_TERMIO_H
	ioctl(SHTTY, TCGETA, &ti->tio);
# else
	ioctl(SHTTY, TIOCGETP, &ti->sgttyb);
	ioctl(SHTTY, TIOCLGET, &ti->lmodes);
	ioctl(SHTTY, TIOCGETC, &ti->tchars);
	ioctl(SHTTY, TIOCGLTC, &ti->ltchars);
# endif
#endif
    }
}

/**/
mod_export void
settyinfo(struct ttyinfo *ti)
{
    if (SHTTY != -1) {
#ifdef HAVE_TERMIOS_H
# ifdef HAVE_TCGETATTR
#  ifndef TCSADRAIN
#   define TCSADRAIN 1	/* XXX Princeton's include files are screwed up */
#  endif
	while (tcsetattr(SHTTY, TCSADRAIN, &ti->tio) == -1 && errno == EINTR)
	    ;
# else
	while (ioctl(SHTTY, TCSETS, &ti->tio) == -1 && errno == EINTR)
	    ;
# endif
	/*	zerr("settyinfo: %e",errno);*/
#else
# ifdef HAVE_TERMIO_H
	ioctl(SHTTY, TCSETA, &ti->tio);
# else
	ioctl(SHTTY, TIOCSETN, &ti->sgttyb);
	ioctl(SHTTY, TIOCLSET, &ti->lmodes);
	ioctl(SHTTY, TIOCSETC, &ti->tchars);
	ioctl(SHTTY, TIOCSLTC, &ti->ltchars);
# endif
#endif
    }
}

/* the default tty state */

/**/
mod_export struct ttyinfo shttyinfo;

/* != 0 if we need to call resetvideo() */

/**/
mod_export int resetneeded;

#ifdef TIOCGWINSZ
/* window size changed */

/**/
mod_export int winchanged;
#endif

static int
adjustlines(int signalled)
{
    int oldlines = zterm_lines;

#ifdef TIOCGWINSZ
    if (signalled || zterm_lines <= 0)
	zterm_lines = shttyinfo.winsize.ws_row;
    else
	shttyinfo.winsize.ws_row = zterm_lines;
#endif /* TIOCGWINSZ */
    if (zterm_lines <= 0) {
	DPUTS(signalled && zterm_lines < 0,
	      "BUG: Impossible TIOCGWINSZ rows");
	zterm_lines = tclines > 0 ? tclines : 24;
    }

    if (zterm_lines > 2)
	termflags &= ~TERM_SHORT;
    else
	termflags |= TERM_SHORT;

    return (zterm_lines != oldlines);
}

static int
adjustcolumns(int signalled)
{
    int oldcolumns = zterm_columns;

#ifdef TIOCGWINSZ
    if (signalled || zterm_columns <= 0)
	zterm_columns = shttyinfo.winsize.ws_col;
    else
	shttyinfo.winsize.ws_col = zterm_columns;
#endif /* TIOCGWINSZ */
    if (zterm_columns <= 0) {
	DPUTS(signalled && zterm_columns < 0,
	      "BUG: Impossible TIOCGWINSZ cols");
	zterm_columns = tccolumns > 0 ? tccolumns : 80;
    }

    if (zterm_columns > 2)
	termflags &= ~TERM_NARROW;
    else
	termflags |= TERM_NARROW;

    return (zterm_columns != oldcolumns);
}

/* check the size of the window and adjust if necessary. *
 * The value of from:					 *
 *   0: called from update_job or setupvals		 *
 *   1: called from the SIGWINCH handler		 *
 *   2: called from the LINES parameter callback	 *
 *   3: called from the COLUMNS parameter callback	 */

/**/
void
adjustwinsize(int from)
{
    static int getwinsz = 1;
#ifdef TIOCGWINSZ
    int ttyrows = shttyinfo.winsize.ws_row;
    int ttycols = shttyinfo.winsize.ws_col;
#endif
    int resetzle = 0;

    if (getwinsz || from == 1) {
#ifdef TIOCGWINSZ
	if (SHTTY == -1)
	    return;
	if (ioctl(SHTTY, TIOCGWINSZ, (char *)&shttyinfo.winsize) == 0) {
	    resetzle = (ttyrows != shttyinfo.winsize.ws_row ||
			ttycols != shttyinfo.winsize.ws_col);
	    if (from == 0 && resetzle && ttyrows && ttycols)
		from = 1; /* Signal missed while a job owned the tty? */
	    ttyrows = shttyinfo.winsize.ws_row;
	    ttycols = shttyinfo.winsize.ws_col;
	} else {
	    /* Set to value from environment on failure */
	    shttyinfo.winsize.ws_row = zterm_lines;
	    shttyinfo.winsize.ws_col = zterm_columns;
	    resetzle = (from == 1);
	}
#else
	resetzle = from == 1;
#endif /* TIOCGWINSZ */
    } /* else
	 return; */

    switch (from) {
    case 0:
    case 1:
	getwinsz = 0;
	/* Calling setiparam() here calls this function recursively, but  *
	 * because we've already called adjustlines() and adjustcolumns() *
	 * here, recursive calls are no-ops unless a signal intervenes.   *
	 * The commented "else return;" above might be a safe shortcut,   *
	 * but I'm concerned about what happens on race conditions; e.g., *
	 * suppose the user resizes his xterm during `eval $(resize)'?    */
	if (adjustlines(from) && zgetenv("LINES"))
	    setiparam("LINES", zterm_lines);
	if (adjustcolumns(from) && zgetenv("COLUMNS"))
	    setiparam("COLUMNS", zterm_columns);
	getwinsz = 1;
	break;
    case 2:
	resetzle = adjustlines(0);
	break;
    case 3:
	resetzle = adjustcolumns(0);
	break;
    }

#ifdef TIOCGWINSZ
    if (interact && from >= 2 &&
	(shttyinfo.winsize.ws_row != ttyrows ||
	 shttyinfo.winsize.ws_col != ttycols)) {
	/* shttyinfo.winsize is already set up correctly */
	/* ioctl(SHTTY, TIOCSWINSZ, (char *)&shttyinfo.winsize); */
    }
#endif /* TIOCGWINSZ */

    if (zleactive && resetzle) {
#ifdef TIOCGWINSZ
	winchanged =
#endif /* TIOCGWINSZ */
	    resetneeded = 1;
	zleentry(ZLE_CMD_RESET_PROMPT);
	zleentry(ZLE_CMD_REFRESH);
    }
}

/*
 * Ensure the fdtable is large enough for fd, and that the
 * maximum fd is set appropriately.
 */
static void
check_fd_table(int fd)
{
    if (fd <= max_zsh_fd)
	return;

    if (fd >= fdtable_size) {
	int old_size = fdtable_size;
	while (fd >= fdtable_size)
	    fdtable = zrealloc(fdtable,
			       (fdtable_size *= 2)*sizeof(*fdtable));
	memset(fdtable + old_size, 0,
	       (fdtable_size - old_size) * sizeof(*fdtable));
    }
    max_zsh_fd = fd;
}

/* Move a fd to a place >= 10 and mark the new fd in fdtable.  If the fd *
 * is already >= 10, it is not moved.  If it is invalid, -1 is returned. */

/**/
mod_export int
movefd(int fd)
{
    if(fd != -1 && fd < 10) {
#ifdef F_DUPFD
	int fe = fcntl(fd, F_DUPFD, 10);
#else
	int fe = movefd(dup(fd));
#endif
	/*
	 * To close or not to close if fe is -1?
	 * If it is -1, we haven't moved the fd, so if we close
	 * it we lose it; but we're probably not going to be able
	 * to use it in situ anyway.  So probably better to avoid a leak.
	 */
	zclose(fd);
	fd = fe;
    }
    if(fd != -1) {
	check_fd_table(fd);
	fdtable[fd] = FDT_INTERNAL;
    }
    return fd;
}

/*
 * Move fd x to y.  If x == -1, fd y is closed.
 * Returns y for success, -1 for failure.
 */

/**/
mod_export int
redup(int x, int y)
{
    int ret = y;

    if(x < 0)
	zclose(y);
    else if (x != y) {
	if (dup2(x, y) == -1) {
	    ret = -1;
	} else {
	    check_fd_table(y);
	    fdtable[y] = fdtable[x];
	    if (fdtable[y] == FDT_FLOCK || fdtable[y] == FDT_FLOCK_EXEC)
		fdtable[y] = FDT_INTERNAL;
	}
	/*
	 * Closing any fd to the locked file releases the lock.
	 * This isn't expected to happen, it's here for completeness.
	 */
	if (fdtable[x] == FDT_FLOCK)
	    fdtable_flocks--;
	zclose(x);
    }

    return ret;
}

/*
 * Add an fd opened ithin a module.
 *
 * fdt is the type of the fd; see the FDT_ definitions in zsh.h.
 * The most likely falures are:
 *
 * FDT_EXTERNAL: the fd can be used within the shell for normal I/O but
 * it will not be closed automatically or by normal shell syntax.
 *
 * FDT_MODULE: as FDT_EXTERNAL, but it can only be closed by the module
 * (which should included zclose() as part of the sequence), not by
 * the standard shell syntax for closing file descriptors.
 *
 * FDT_INTERNAL: fd is treated like others created by the shell for
 * internal use; it can be closed and will be closed by the shell if it
 * exec's or performs an exec with a fork optimised out.
 *
 * Safe if fd is -1 to indicate failure.
 */
/**/
mod_export void
addmodulefd(int fd, int fdt)
{
    if (fd >= 0) {
	check_fd_table(fd);
	fdtable[fd] = fdt;
    }
}

/**/

/*
 * Indicate that an fd has a file lock; if cloexec is 1 it will be closed
 * on exec.
 * The fd should already be known to fdtable (e.g. by movefd).
 * Note the fdtable code doesn't care what sort of lock
 * is used; this simply prevents the main shell exiting prematurely
 * when it holds a lock.
 */

/**/
mod_export void
addlockfd(int fd, int cloexec)
{
    if (cloexec) {
	if (fdtable[fd] != FDT_FLOCK)
	    fdtable_flocks++;
	fdtable[fd] = FDT_FLOCK;
    } else {
	fdtable[fd] = FDT_FLOCK_EXEC;
    }
}

/* Close the given fd, and clear it from fdtable. */

/**/
mod_export int
zclose(int fd)
{
    if (fd >= 0) {
	/*
	 * Careful: we allow closing of arbitrary fd's, beyond
	 * max_zsh_fd.  In that case we don't try anything clever.
	 */
	if (fd <= max_zsh_fd) {
	    if (fdtable[fd] == FDT_FLOCK)
		fdtable_flocks--;
	    fdtable[fd] = FDT_UNUSED;
	    while (max_zsh_fd > 0 && fdtable[max_zsh_fd] == FDT_UNUSED)
		max_zsh_fd--;
	    if (fd == coprocin)
		coprocin = -1;
	    if (fd == coprocout)
		coprocout = -1;
	}
	return close(fd);
    }
    return -1;
}

/*
 * Close an fd returning 0 if used for locking; return -1 if it isn't.
 */

/**/
mod_export int
zcloselockfd(int fd)
{
    if (fd > max_zsh_fd)
	return -1;
    if (fdtable[fd] != FDT_FLOCK && fdtable[fd] != FDT_FLOCK_EXEC)
	return -1;
    zclose(fd);
    return 0;
}

#ifdef HAVE__MKTEMP
extern char *_mktemp(char *);
#endif

/* Get a unique filename for use as a temporary file.  If "prefix" is
 * NULL, the name is relative to $TMPPREFIX; If it is non-NULL, the
 * unique suffix includes a prefixed '.' for improved readability.  If
 * "use_heap" is true, we allocate the returned name on the heap.
 * The string passed as "prefix" is expected to be metafied. */

/**/
mod_export char *
gettempname(const char *prefix, int use_heap)
{
    char *ret, *suffix = prefix ? ".XXXXXX" : "XXXXXX";

    queue_signals();
    if (!prefix && !(prefix = getsparam("TMPPREFIX")))
	prefix = DEFAULT_TMPPREFIX;
    if (use_heap)
	ret = dyncat(unmeta(prefix), suffix);
    else
	ret = bicat(unmeta(prefix), suffix);

#ifdef HAVE__MKTEMP
    /* Zsh uses mktemp() safely, so silence the warnings */
    ret = (char *) _mktemp(ret);
#elif HAVE_MKSTEMP && defined(DEBUG)
    {
	/* zsh uses mktemp() safely (all callers use O_EXCL, and one of them
	 * uses mkfifo()/mknod(), as opposed to open()), but some compilers
	 * warn about this anyway and give no way to disable the warning. To
	 * appease them, use mkstemp() and then close the fd and unlink the
	 * filename, to match callers' expectations.
	 *
	 * But do this in debug builds only, because we don't want to suffer
	 * x3 the disk access (touch, unlink, touch again) in production.
	 */
	int fd;
	errno = 0;
	fd = mkstemp(ret);
	if (fd < 0)
	    zwarn("can't get a temporary filename: %e", errno);
	else {
	    close(fd);
	    ret = ztrdup(ret);

	    errno = 0;
	    if (unlink(ret) < 0)
		zwarn("unlinking a temporary filename failed: %e", errno);
	}
    }
#else
    ret = (char *) mktemp(ret);
#endif
    unqueue_signals();

    return ret;
}

/* The gettempfile() "prefix" is expected to be metafied, see hist.c
 * and gettempname(). */

/**/
mod_export int
gettempfile(const char *prefix, int use_heap, char **tempname)
{
    char *fn;
    int fd;
    mode_t old_umask;
#if HAVE_MKSTEMP
    char *suffix = prefix ? ".XXXXXX" : "XXXXXX";

    queue_signals();
    old_umask = umask(0177);
    if (!prefix && !(prefix = getsparam("TMPPREFIX")))
	prefix = DEFAULT_TMPPREFIX;
    if (use_heap)
	fn = dyncat(unmeta(prefix), suffix);
    else
	fn = bicat(unmeta(prefix), suffix);

    fd = mkstemp(fn);
    if (fd < 0) {
	if (!use_heap)
	    free(fn);
	fn = NULL;
    }
#else
    int failures = 0;

    queue_signals();
    old_umask = umask(0177);
    do {
	if (!(fn = gettempname(prefix, use_heap))) {
	    fd = -1;
	    break;
	}
	if ((fd = open(fn, O_RDWR | O_CREAT | O_EXCL, 0600)) >= 0)
	    break;
	if (!use_heap)
	    free(fn);
	fn = NULL;
    } while (errno == EEXIST && ++failures < 16);
#endif
    *tempname = fn;

    umask(old_umask);
    unqueue_signals();
    return fd;
}

/* Check if a string contains a token */

/**/
mod_export int
has_token(const char *s)
{
    while(*s)
	if(itok(*s++))
	    return 1;
    return 0;
}

/* Delete a character in a string */

/**/
mod_export void
chuck(char *str)
{
    while ((str[0] = str[1]))
	str++;
}

/**/
mod_export int
tulower(int c)
{
    c &= 0xff;
    return (isupper(c) ? tolower(c) : c);
}

/**/
mod_export int
tuupper(int c)
{
    c &= 0xff;
    return (islower(c) ? toupper(c) : c);
}

/* copy len chars from t into s, and null terminate */

/**/
void
ztrncpy(char *s, char *t, int len)
{
    while (len--)
	*s++ = *t++;
    *s = '\0';
}

/* copy t into *s and update s */

/**/
mod_export void
strucpy(char **s, char *t)
{
    char *u = *s;

    while ((*u++ = *t++));
    *s = u - 1;
}

/**/
mod_export void
struncpy(char **s, char *t, int n)
{
    char *u = *s;

    while (n-- && (*u = *t++))
	u++;
    *s = u;
    if (n > 0) /* just one null-byte will do, unlike strncpy(3) */
	*u = '\0';
}

/* Return the number of elements in an array of pointers. *
 * It doesn't count the NULL pointer at the end.          */

/**/
mod_export int
arrlen(char **s)
{
    int count;

    for (count = 0; *s; s++, count++);
    return count;
}

/* Return TRUE iff arrlen(s) >= lower_bound, but more efficiently. */

/**/
mod_export char
arrlen_ge(char **s, unsigned lower_bound)
{
    while (lower_bound--)
	if (!*s++)
	    return 0 /* FALSE */;

    return 1 /* TRUE */;
}

/* Return TRUE iff arrlen(s) > lower_bound, but more efficiently. */

/**/
mod_export char
arrlen_gt(char **s, unsigned lower_bound)
{
    return arrlen_ge(s, 1+lower_bound);
}

/* Return TRUE iff arrlen(s) <= upper_bound, but more efficiently. */

/**/
mod_export char
arrlen_le(char **s, unsigned upper_bound)
{
    return arrlen_lt(s, 1+upper_bound);
}

/* Return TRUE iff arrlen(s) < upper_bound, but more efficiently. */

/**/
mod_export char
arrlen_lt(char **s, unsigned upper_bound)
{
    return !arrlen_ge(s, upper_bound);
}

/* Skip over a balanced pair of parenthesis. */

/**/
mod_export int
skipparens(char inpar, char outpar, char **s)
{
    int level;

    if (**s != inpar)
	return -1;

    for (level = 1; *++*s && level;)
	if (**s == inpar)
	   ++level;
	else if (**s == outpar)
	   --level;

   return level;
}

/**/
mod_export zlong
zstrtol(const char *s, char **t, int base)
{
    return zstrtol_underscore(s, t, base, 0);
}

/* Convert string to zlong (see zsh.h).  This function (without the z) *
 * is contained in the ANSI standard C library, but a lot of them seem *
 * to be broken.                                                       */

/**/
mod_export zlong
zstrtol_underscore(const char *s, char **t, int base, int underscore)
{
    const char *inp, *trunc = NULL;
    zulong calc = 0, newcalc = 0;
    int neg;

    while (inblank(*s))
	s++;

    if ((neg = IS_DASH(*s)))
	s++;
    else if (*s == '+')
	s++;

    if (!base) {
	if (*s != '0')
	    base = 10;
	else if (*++s == 'x' || *s == 'X')
	    base = 16, s++;
	else if (*s == 'b' || *s == 'B')
	    base = 2, s++;
	else
	    base = 8;
    }
    inp = s;
    if (base < 2 || base > 36) {
	zerr("invalid base (must be 2 to 36 inclusive): %d", base);
	return (zlong)0;
    } else if (base <= 10) {
	for (; (*s >= '0' && *s < ('0' + base)) ||
		 (underscore && *s == '_'); s++) {
	    if (trunc || *s == '_')
		continue;
	    newcalc = calc * base + *s - '0';
	    if (newcalc < calc)
	    {
		trunc = s;
		continue;
	    }
	    calc = newcalc;
	}
    } else {
	for (; idigit(*s) || (*s >= 'a' && *s < ('a' + base - 10))
	     || (*s >= 'A' && *s < ('A' + base - 10))
	     || (underscore && *s == '_'); s++) {
	    if (trunc || *s == '_')
		continue;
	    newcalc = calc*base + (idigit(*s) ? (*s - '0') : (*s & 0x1f) + 9);
	    if (newcalc < calc)
	    {
		trunc = s;
		continue;
	    }
	    calc = newcalc;
	}
    }

    /*
     * Special case: check for a number that was just too long for
     * signed notation.
     * Extra special case: the lowest negative number would trigger
     * the first test, but is actually representable correctly.
     * This is a 1 in the top bit, all others zero, so test for
     * that explicitly.
     */
    if (!trunc && (zlong)calc < 0 &&
	(!neg || calc & ~((zulong)1 << (8*sizeof(zulong)-1))))
    {
	trunc = s - 1;
	calc /= base;
    }

    if (trunc)
	zwarn("number truncated after %d digits: %s", (int)(trunc - inp), inp);

    if (t)
	*t = (char *)s;
    return neg ? -(zlong)calc : (zlong)calc;
}

/*
 * If s represents a complete unsigned integer (and nothing else)
 * return 1 and set retval to the value.  Otherwise return 0.
 *
 * Underscores are always allowed.
 *
 * Sensitive to OCTAL_ZEROES.
 */

/**/
mod_export int
zstrtoul_underscore(const char *s, zulong *retval)
{
    zulong calc = 0, newcalc = 0, base;

    if (*s == '+')
	s++;

    if (*s != '0')
	base = 10;
    else if (*++s == 'x' || *s == 'X')
	base = 16, s++;
    else if (*s == 'b' || *s == 'B')
	base = 2, s++;
    else
	base = isset(OCTALZEROES) ? 8 : 10;
    if (base <= 10) {
	for (; (*s >= '0' && *s < ('0' + base)) ||
		 *s == '_'; s++) {
	    if (*s == '_')
		continue;
	    newcalc = calc * base + *s - '0';
	    if (newcalc < calc)
	    {
		return 0;
	    }
	    calc = newcalc;
	}
    } else {
	for (; idigit(*s) || (*s >= 'a' && *s < ('a' + base - 10))
	     || (*s >= 'A' && *s < ('A' + base - 10))
	     || *s == '_'; s++) {
	    if (*s == '_')
		continue;
	    newcalc = calc*base + (idigit(*s) ? (*s - '0') : (*s & 0x1f) + 9);
	    if (newcalc < calc)
	    {
		return 0;
	    }
	    calc = newcalc;
	}
    }

    if (*s)
	return 0;
    *retval = calc;
    return 1;
}

/**/
mod_export int
setblock_fd(int turnonblocking, int fd, long *modep)
{
#ifdef O_NDELAY
# ifdef O_NONBLOCK
#  define NONBLOCK (O_NDELAY|O_NONBLOCK)
# else /* !O_NONBLOCK */
#  define NONBLOCK O_NDELAY
# endif /* !O_NONBLOCK */
#else /* !O_NDELAY */
# ifdef O_NONBLOCK
#  define NONBLOCK O_NONBLOCK
# else /* !O_NONBLOCK */
#  define NONBLOCK 0
# endif /* !O_NONBLOCK */
#endif /* !O_NDELAY */

#if NONBLOCK
    struct stat st;

    if (!fstat(fd, &st) && !S_ISREG(st.st_mode)) {
	*modep = fcntl(fd, F_GETFL, 0);
	if (*modep != -1) {
	    if (!turnonblocking) {
		/* We want to know if blocking was off */
		if ((*modep & NONBLOCK) ||
		    !fcntl(fd, F_SETFL, *modep | NONBLOCK))
		    return 1;
	    } else if ((*modep & NONBLOCK) &&
		       !fcntl(fd, F_SETFL, *modep & ~NONBLOCK)) {
		/* Here we want to know if the state changed */
		return 1;
	    }
	}
    } else
#endif /* NONBLOCK */
	*modep = -1;
    return 0;

#undef NONBLOCK
}

/**/
int
setblock_stdin(void)
{
    long mode;
    return setblock_fd(1, 0, &mode);
}

/*
 * Check for pending input on fd.  If polltty is set, we may need to
 * use termio to look for input.  As a final resort, go to non-blocking
 * input and try to read a character, which in this case will be
 * returned in *readchar.
 *
 * Note that apart from setting (and restoring) non-blocking input,
 * this function does not change the input mode.  The calling function
 * should have set cbreak mode if necessary.
 *
 * fd may be -1 to sleep until the timeout in microseconds.  This is a
 * fallback for old systems that don't have nanosleep().  Some very old
 * systems might not have select: get with it, daddy-o.
 */

/**/
mod_export int
read_poll(int fd, int *readchar, int polltty, zlong microseconds)
{
    int ret = -1;
    long mode = -1;
    char c;
#ifdef HAVE_SELECT
    fd_set foofd;
    struct timeval expire_tv;
#else
#ifdef FIONREAD
    int val;
#endif
#endif
#ifdef HAS_TIO
    struct ttyinfo ti;
#endif

    if (fd < 0 || (polltty && !isatty(fd)))
	polltty = 0;		/* no tty to poll */

#if defined(HAS_TIO) && !defined(__CYGWIN__)
    /*
     * Under Solaris, at least, reading from the terminal in non-canonical
     * mode requires that we use the VMIN mechanism to poll.  Any attempt
     * to check any other way, or to set the terminal to non-blocking mode
     * and poll that way, fails; it will just for canonical mode input.
     * We should probably use this mechanism if the user has set non-canonical
     * mode, in which case testing here for isatty() and ~ICANON would be
     * better than testing whether bin_read() set it, but for now we've got
     * enough problems.
     *
     * Under Cygwin, you won't be surprised to here, this mechanism,
     * although present, doesn't work, and we *have* to use ordinary
     * non-blocking reads to find out if there is a character present
     * in non-canonical mode.
     *
     * I am assuming Solaris is nearer the UNIX norm.  This is not necessarily
     * as plausible as it sounds, but it seems the right way to guess.
     *		pws 2000/06/26
     */
    if (polltty && fd >= 0) {
	gettyinfo(&ti);
	if ((polltty = ti.tio.c_cc[VMIN])) {
	    ti.tio.c_cc[VMIN] = 0;
	    /* termios timeout is 10ths of a second */
	    ti.tio.c_cc[VTIME] = (int) (microseconds / (zlong)100000);
	    settyinfo(&ti);
	}
    }
#else
    polltty = 0;
#endif
#ifdef HAVE_SELECT
    expire_tv.tv_sec = (int) (microseconds / (zlong)1000000);
    expire_tv.tv_usec = microseconds % (zlong)1000000;
    FD_ZERO(&foofd);
    if (fd > -1) {
	FD_SET(fd, &foofd);
	ret = select(fd+1, (SELECT_ARG_2_T) &foofd, NULL, NULL, &expire_tv);
    } else
	ret = select(0, NULL, NULL, NULL, &expire_tv);
#else
    if (fd < 0) {
	/* OK, can't do that.  Just quietly sleep for a second. */
	sleep(1);
	return 1;
    }
#ifdef FIONREAD
    if (ioctl(fd, FIONREAD, (char *) &val) == 0)
	ret = (val > 0);
#endif
#endif

    if (fd >= 0 && ret < 0 && !errflag) {
	/*
	 * Final attempt: set non-blocking read and try to read a character.
	 * Praise Bill, this works under Cygwin (nothing else seems to).
	 */
	if ((polltty || setblock_fd(0, fd, &mode)) && read(fd, &c, 1) > 0) {
	    *readchar = c;
	    ret = 1;
	}
	if (mode != -1)
	    fcntl(fd, F_SETFL, mode);
    }
#ifdef HAS_TIO
    if (polltty) {
	ti.tio.c_cc[VMIN] = 1;
	ti.tio.c_cc[VTIME] = 0;
	settyinfo(&ti);
    }
#endif
    return (ret > 0);
}

/*
 * Return the difference between 2 times, given as struct timespec*,
 * expressed in microseconds, as a long.  If the difference doesn't fit
 * into a long, return LONG_MIN or LONG_MAX so that the times can still
 * be compared.
 *
 * Note: returns a long rather than a zlong because zsleep() below
 * takes a long.
 */

/**/
long
timespec_diff_us(const struct timespec *t1, const struct timespec *t2)
{
    int reverse = (t1->tv_sec > t2->tv_sec);
    time_t diff_sec;
    long diff_usec, max_margin, res;

    /* Don't just subtract t2-t1 because time_t might be unsigned. */
    diff_sec = (reverse ? t1->tv_sec - t2->tv_sec : t2->tv_sec - t1->tv_sec);
    if (diff_sec > LONG_MAX / 1000000L) {
	goto overflow;
    }
    res = diff_sec * 1000000L;
    max_margin = LONG_MAX - res;
    diff_usec = (reverse ?
		 t1->tv_nsec - t2->tv_nsec : t2->tv_nsec - t1->tv_nsec
		 ) / 1000;
    if (diff_usec <= max_margin) {
	res += diff_usec;
	return (reverse ? -res : res);
    }
 overflow:
    return (reverse ? LONG_MIN : LONG_MAX);
}

/*
 * Sleep for the given number of microseconds --- must be within
 * range of a long at the moment, but this is only used for
 * limited internal purposes.
 */

/**/
int
zsleep(long us)
{
#ifdef HAVE_NANOSLEEP
    struct timespec sleeptime;

    sleeptime.tv_sec = (time_t)us / (time_t)1000000;
    sleeptime.tv_nsec = (us % 1000000L) * 1000L;
    for (;;) {
	struct timespec rem;
	int ret = nanosleep(&sleeptime, &rem);

	if (ret == 0)
	    return 1;
	else if (errno != EINTR)
	    return 0;
	sleeptime = rem;
    }
#else
    int dummy;
    return read_poll(-1, &dummy, 0, us);
#endif
}

/**
 * Sleep for time (fairly) randomly up to max_us microseconds.
 * Don't let the wallclock time extend beyond end_time.
 * Return 1 if that seemed to work, else 0.
 *
 * For best results max_us should be a multiple of 2**16 or large
 * enough that it doesn't matter.
 */

/**/
int
zsleep_random(long max_us, time_t end_time)
{
    long r;
    time_t now = time(NULL);

    /*
     * Randomish backoff.  Doesn't need to be fundamentally
     * unpredictable, just probably unlike the value another
     * exiting shell is using.  On some systems the bottom 16
     * bits aren't that random but the use here doesn't
     * really care.
     */
    r = (long)(rand() & 0xFFFF);
    /*
     * Turn this into a fraction of sleep_us.  Again, this
     * doesn't need to be particularly accurate and the base time
     * is sufficient that we can do the division first and not
     * worry about the range.
     */
    r = (max_us >> 16) * r;
    /*
     * Don't sleep beyond timeout.
     * Not that important as timeout is ridiculously long, but
     * if there's an interface, interface to it...
     */
    while (r && now + (time_t)(r / 1000000) > end_time)
	r >>= 1;
    if (r) /* pedantry */
	return zsleep(r);
    return 0;
}

/**/
int
checkrmall(char *s)
{
    DIR *rmd;
    int count = 0;
    if (!shout)
	return 1;
    if (*s != '/') {
	if (pwd[1])
	    s = zhtricat(pwd, "/", s);
	else
	    s = dyncat("/", s);
    }
    const int max_count = 100;
    if ((rmd = opendir(unmeta(s)))) {
	int ignoredots = !isset(GLOBDOTS);
	char *fname;

	while ((fname = zreaddir(rmd, 1))) {
	    if (ignoredots && *fname == '.')
		continue;
	    count++;
	    if (count > max_count)
		break;
	}
	closedir(rmd);
    }
    if (count > max_count)
	fprintf(shout, "zsh: sure you want to delete more than %d files in ",
		max_count);
    else if (count == 1)
	fprintf(shout, "zsh: sure you want to delete the only file in ");
    else if (count > 0)
	fprintf(shout, "zsh: sure you want to delete all %d files in ",
		count);
    else {
	/* We don't know how many files the glob will expand to; see 41707. */
	fprintf(shout, "zsh: sure you want to delete all the files in ");
    }
    nicezputs(s, shout);
    if(isset(RMSTARWAIT)) {
	fputs("? (waiting ten seconds)", shout);
	fflush(shout);
	zbeep();
	sleep(10);
	fputc('\n', shout);
    }
    if (errflag)
      return 0;
    fputs(" [yn]? ", shout);
    fflush(shout);
    zbeep();
    return (getquery("ny", 1) == 'y');
}

/**/
mod_export ssize_t
read_loop(int fd, char *buf, size_t len)
{
    ssize_t got = len;

    while (1) {
	ssize_t ret = read(fd, buf, len);
	if (ret == len)
	    break;
	if (ret <= 0) {
	    if (ret < 0) {
		if (errno == EINTR)
		    continue;
		if (fd != SHTTY)
		    zwarn("read failed: %e", errno);
	    }
	    return ret;
	}
	buf += ret;
	len -= ret;
    }

    return got;
}

/**/
mod_export ssize_t
write_loop(int fd, const char *buf, size_t len)
{
    ssize_t wrote = len;

    while (1) {
	ssize_t ret = write(fd, buf, len);
	if (ret == len)
	    break;
	if (ret < 0) {
	    if (errno == EINTR)
		continue;
	    if (fd != SHTTY)
		zwarn("write failed: %e", errno);
	    return -1;
	}
	buf += ret;
	len -= ret;
    }

    return wrote;
}

static int
read1char(int echo)
{
    char c;
    int q = queue_signal_level();

    dont_queue_signals();
    while (read(SHTTY, &c, 1) != 1) {
	if (errno != EINTR || errflag || retflag || breaks || contflag) {
	    restore_queue_signals(q);
	    return -1;
	}
    }
    restore_queue_signals(q);
    if (echo)
	write_loop(SHTTY, &c, 1);
    return STOUC(c);
}

/**/
mod_export int
noquery(int purge)
{
    int val = 0;

#ifdef FIONREAD
    char c;

    ioctl(SHTTY, FIONREAD, (char *)&val);
    if (purge) {
	for (; val; val--) {
	    if (read(SHTTY, &c, 1) != 1) {
		/* Do nothing... */
	    }
	}
    }
#endif

    return val;
}

/**/
int
getquery(char *valid_chars, int purge)
{
    int c, d, nl = 0;
    int isem = !strcmp(term, "emacs");
    struct ttyinfo ti;

    attachtty(mypgrp);

    gettyinfo(&ti);
#ifdef HAS_TIO
    ti.tio.c_lflag &= ~ECHO;
    if (!isem) {
	ti.tio.c_lflag &= ~ICANON;
	ti.tio.c_cc[VMIN] = 1;
	ti.tio.c_cc[VTIME] = 0;
    }
#else
    ti.sgttyb.sg_flags &= ~ECHO;
    if (!isem)
	ti.sgttyb.sg_flags |= CBREAK;
#endif
    settyinfo(&ti);

    if (noquery(purge)) {
	if (!isem)
	    settyinfo(&shttyinfo);
	write_loop(SHTTY, "n\n", 2);
	return 'n';
    }

    while ((c = read1char(0)) >= 0) {
	if (c == 'Y')
	    c = 'y';
	else if (c == 'N')
	    c = 'n';
	if (!valid_chars)
	    break;
	if (c == '\n') {
	    c = *valid_chars;
	    nl = 1;
	    break;
	}
	if (strchr(valid_chars, c)) {
	    nl = 1;
	    break;
	}
	zbeep();
    }
    if (c >= 0) {
	char buf = (char)c;
	write_loop(SHTTY, &buf, 1);
    }
    if (nl)
	write_loop(SHTTY, "\n", 1);

    if (isem) {
	if (c != '\n')
	    while ((d = read1char(1)) >= 0 && d != '\n');
    } else {
	if (c != '\n' && !valid_chars) {
#ifdef MULTIBYTE_SUPPORT
	    if (isset(MULTIBYTE) && c >= 0) {
		/*
		 * No waiting for a valid character, and no draining;
		 * we should ensure we haven't stopped in the middle
		 * of a multibyte character.
		 */
		mbstate_t mbs;
		char cc = (char)c;
		memset(&mbs, 0, sizeof(mbs));
		for (;;) {
		    size_t ret = mbrlen(&cc, 1, &mbs);

		    if (ret != MB_INCOMPLETE)
			break;
		    c = read1char(1);
		    if (c < 0)
			break;
		    cc = (char)c;
		}
	    }
#endif
	    write_loop(SHTTY, "\n", 1);
	}
    }
    settyinfo(&shttyinfo);
    return c;
}

static int d;
static char *guess, *best;
static Patprog spckpat, spnamepat;

/**/
static void
spscan(HashNode hn, UNUSED(int scanflags))
{
    int nd;

    if (spckpat && pattry(spckpat, hn->nam))
	return;

    nd = spdist(hn->nam, guess, (int) strlen(guess) / 4 + 1);
    if (nd <= d) {
	best = hn->nam;
	d = nd;
    }
}

/* spellcheck a word */
/* fix s ; if hist is nonzero, fix the history list too */

/**/
mod_export void
spckword(char **s, int hist, int cmd, int ask)
{
    char *t, *correct_ignore;
    char ic = '\0';
    int preflen = 0;
    int autocd = cmd && isset(AUTOCD) && strcmp(*s, ".") && strcmp(*s, "..");

    if (!(*s)[0] || !(*s)[1])
	return;
    if ((histdone & HISTFLAG_NOEXEC) ||
	/* Leading % is a job, else leading hyphen is an option */
	(cmd ? **s == '%' : (**s == '-' || **s == Dash)))
	return;
    if (!strcmp(*s, "in"))
	return;
    if (cmd) {
	if (shfunctab->getnode(shfunctab, *s) ||
	    builtintab->getnode(builtintab, *s) ||
	    cmdnamtab->getnode(cmdnamtab, *s) ||
	    aliastab->getnode(aliastab, *s)  ||
	    reswdtab->getnode(reswdtab, *s))
	    return;
	else if (isset(HASHLISTALL)) {
	    cmdnamtab->filltable(cmdnamtab);
	    if (cmdnamtab->getnode(cmdnamtab, *s))
		return;
	}
    }
    t = *s;
    if (*t == Tilde || *t == Equals || *t == String)
	t++;
    for (; *t; t++)
	if (itok(*t)) {
	    if (*t == Dash)
		*t = '-';
	    else
		return;
	}
    best = NULL;
    for (t = *s; *t; t++)
	if (*t == '/')
	    break;
    if (**s == Tilde && !*t)
	return;

    if ((correct_ignore = getsparam("CORRECT_IGNORE")) != NULL) {
	tokenize(correct_ignore = dupstring(correct_ignore));
	remnulargs(correct_ignore);
	spckpat = patcompile(correct_ignore, 0, NULL);
    } else
	spckpat = NULL;

    if ((correct_ignore = getsparam("CORRECT_IGNORE_FILE")) != NULL) {
	tokenize(correct_ignore = dupstring(correct_ignore));
	remnulargs(correct_ignore);
	spnamepat = patcompile(correct_ignore, 0, NULL);
    } else
	spnamepat = NULL;

    if (**s == String && !*t) {
	guess = *s + 1;
	if (itype_end(guess, IIDENT, 1) == guess)
	    return;
	ic = String;
	d = 100;
	scanhashtable(paramtab, 1, 0, 0, spscan, 0);
    } else if (**s == Equals) {
	if (*t)
	    return;
	if (hashcmd(guess = *s + 1, pathchecked))
	    return;
	d = 100;
	ic = Equals;
	scanhashtable(aliastab, 1, 0, 0, spscan, 0);
	scanhashtable(cmdnamtab, 1, 0, 0, spscan, 0);
    } else {
	guess = *s;
	if (*guess == Tilde || *guess == String) {
	    int ne;
	    ic = *guess;
	    if (!*++t)
		return;
	    guess = dupstring(guess);
	    ne = noerrs;
	    noerrs = 2;
	    singsub(&guess);
	    noerrs = ne;
	    if (!guess)
		return;
	    preflen = strlen(guess) - strlen(t);
	}
	if (access(unmeta(guess), F_OK) == 0)
	    return;
	best = spname(guess);
	if (!*t && cmd) {
	    if (hashcmd(guess, pathchecked))
		return;
	    d = 100;
	    scanhashtable(reswdtab, 1, 0, 0, spscan, 0);
	    scanhashtable(aliastab, 1, 0, 0, spscan, 0);
	    scanhashtable(shfunctab, 1, 0, 0, spscan, 0);
	    scanhashtable(builtintab, 1, 0, 0, spscan, 0);
	    scanhashtable(cmdnamtab, 1, 0, 0, spscan, 0);
	    if (autocd) {
		char **pp;
		if (cd_able_vars(unmeta(guess)))
		    return;
		for (pp = cdpath; *pp; pp++) {
		    char bestcd[PATH_MAX + 1];
		    int thisdist;
		    /* Less than d here, instead of less than or equal  *
		     * as used in spscan(), so that an autocd is chosen *
		     * only when it is better than anything so far, and *
		     * so we prefer directories earlier in the cdpath.  */
		    if ((thisdist = mindist(*pp, *s, bestcd, 1)) < d) {
			best = dupstring(bestcd);
			d = thisdist;
		    }
		}
	    }
	}
    }
    if (errflag)
	return;
    if (best && (int)strlen(best) > 1 && strcmp(best, guess)) {
	int x;
	if (ic) {
	    char *u;
	    if (preflen) {
		/* do not correct the result of an expansion */
		if (strncmp(guess, best, preflen))
		    return;
		/* replace the temporarily expanded prefix with the original */
		u = (char *) zhalloc(t - *s + strlen(best + preflen) + 1);
		strncpy(u, *s, t - *s);
		strcpy(u + (t - *s), best + preflen);
	    } else {
		u = (char *) zhalloc(strlen(best) + 2);
		*u = '\0';
		strcpy(u + 1, best);
	    }
	    best = u;
	    guess = *s;
	    *guess = *best = ztokens[ic - Pound];
	}
	if (ask) {
	    if (noquery(0)) {
		x = 'n';
	    } else if (shout) {
		char *pptbuf;
		pptbuf = promptexpand(sprompt, 0, best, guess, NULL);
		zputs(pptbuf, shout);
		free(pptbuf);
		fflush(shout);
		zbeep();
		x = getquery("nyae", 0);
		if (cmd && x == 'n')
		    pathchecked = path;
	    } else
		x = 'n';
	} else
	    x = 'y';
	if (x == 'y') {
	    *s = dupstring(best);
	    if (hist)
		hwrep(best);
	} else if (x == 'a') {
	    histdone |= HISTFLAG_NOEXEC;
	} else if (x == 'e') {
	    histdone |= HISTFLAG_NOEXEC | HISTFLAG_RECALL;
	}
	if (ic)
	    **s = ic;
    }
}

/*
 * Helper for ztrftime.  Called with a pointer to the length left
 * in the buffer, and a new string length to decrement from that.
 * Returns 0 if the new length fits, 1 otherwise.  We assume a terminating
 * NUL and return 1 if that doesn't fit.
 */

static int
ztrftimebuf(int *bufsizeptr, int decr)
{
    if (*bufsizeptr <= decr)
	return 1;
    *bufsizeptr -= decr;
    return 0;
}

/*
 * Like the system function, this returns the number of characters
 * copied, not including the terminating NUL.  This may be zero
 * if the string didn't fit.
 *
 * As an extension, try to detect an error in strftime --- typically
 * not enough memory --- and return -1.  Not guaranteed to be portable,
 * since the strftime() interface doesn't make any guarantees about
 * the state of the buffer if it returns zero.
 *
 * fmt is metafied, but we need to unmetafy it on the fly to
 * pass into strftime / combine with the output from strftime.
 * The return value in buf is not metafied.
 */

/**/
mod_export int
ztrftime(char *buf, int bufsize, char *fmt, struct tm *tm, long nsec)
{
    int hr12;
#ifdef HAVE_STRFTIME
    int decr;
    char *fmtstart;
#else
    static char *astr[] =
    {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    static char *estr[] =
    {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul",
     "Aug", "Sep", "Oct", "Nov", "Dec"};
#endif
    char *origbuf = buf;


    while (*fmt) {
	if (*fmt == Meta) {
	    int chr = fmt[1] ^ 32;
	    if (ztrftimebuf(&bufsize, 1))
		return -1;
	    *buf++ = chr;
	    fmt += 2;
	} else if (*fmt == '%') {
	    int strip;
	    int digs = 3;

#ifdef HAVE_STRFTIME
	    fmtstart =
#endif
	    fmt++;

	    if (*fmt == '-') {
		strip = 1;
		fmt++;
	    } else
		strip = 0;
	    if (idigit(*fmt)) {
		/* Digit --- only useful with . */
		char *dstart = fmt;
		char *dend = fmt+1;
		while (idigit(*dend))
		    dend++;
		if (*dend == '.') {
		    fmt = dend;
		    digs = atoi(dstart);
		}
	    }
	    /*
	     * Assume this format will take up at least two
	     * characters.  Not always true, but if that matters
	     * we are so close to the edge it's not a big deal.
	     * Fix up some longer cases specially when we get to them.
	     */
	    if (ztrftimebuf(&bufsize, 2))
		return -1;
#ifdef HAVE_STRFTIME
	    /* Our internal handling doesn't handle padding and other gnu extensions,
	     * so here we detect them and pass over to strftime(). We don't want
	     * to do this unconditionally though, as we have some extensions that
	     * strftime() doesn't have (%., %f, %L and %K) */
morefmt:
	    if (!((fmt - fmtstart == 1) || (fmt - fmtstart == 2 && strip) || *fmt == '.')) {
		while (*fmt && strchr("OE^#_-0123456789", *fmt))
		    fmt++;
		if (*fmt) {
		    fmt++;
		    goto strftimehandling;
		}
	    }
#endif
	    switch (*fmt++) {
	    case '.':
	    {
		long fnsec = nsec;
		if (digs < 0 || digs > 9)
		    digs = 9;
		if (ztrftimebuf(&bufsize, digs))
		    return -1;
		if (digs < 9) {
		    int trunc;
		    long max = 100000000;
		    for (trunc = 8 - digs; trunc; trunc--) {
			max /= 10;
			fnsec /= 10;
		    }
		    max -= 1;
		    fnsec = (fnsec + 5) / 10;
		    if (fnsec > max)
			fnsec = max;
		}
		sprintf(buf, "%0*ld", digs, fnsec);
		buf += digs;
		break;
	    }
	    case '\0':
		/* Guard against premature end of string */
		*buf++ = '%';
		fmt--;
		break;
	    case 'f':
		strip = 1;
		/* FALLTHROUGH */
	    case 'e':
		if (tm->tm_mday > 9)
		    *buf++ = '0' + tm->tm_mday / 10;
		else if (!strip)
		    *buf++ = ' ';
		*buf++ = '0' + tm->tm_mday % 10;
		break;
	    case 'K':
		strip = 1;
		/* FALLTHROUGH */
	    case 'H':
	    case 'k':
		if (tm->tm_hour > 9)
		    *buf++ = '0' + tm->tm_hour / 10;
		else if (!strip) {
		    if (fmt[-1] == 'H')
			*buf++ = '0';
		    else
			*buf++ = ' ';
		}
		*buf++ = '0' + tm->tm_hour % 10;
		break;
	    case 'L':
		strip = 1;
		/* FALLTHROUGH */
	    case 'l':
		hr12 = tm->tm_hour % 12;
		if (hr12 == 0)
		    hr12 = 12;
	        if (hr12 > 9)
		    *buf++ = '1';
		else if (!strip)
		    *buf++ = ' ';

		*buf++ = '0' + (hr12 % 10);
		break;
	    case 'd':
		if (tm->tm_mday > 9 || !strip)
		    *buf++ = '0' + tm->tm_mday / 10;
		*buf++ = '0' + tm->tm_mday % 10;
		break;
	    case 'm':
		if (tm->tm_mon > 8 || !strip)
		    *buf++ = '0' + (tm->tm_mon + 1) / 10;
		*buf++ = '0' + (tm->tm_mon + 1) % 10;
		break;
	    case 'M':
		if (tm->tm_min > 9 || !strip)
		    *buf++ = '0' + tm->tm_min / 10;
		*buf++ = '0' + tm->tm_min % 10;
		break;
	    case 'N':
		if (ztrftimebuf(&bufsize, 9))
		    return -1;
		sprintf(buf, "%09ld", nsec);
		buf += 9;
		break;
	    case 'S':
		if (tm->tm_sec > 9 || !strip)
		    *buf++ = '0' + tm->tm_sec / 10;
		*buf++ = '0' + tm->tm_sec % 10;
		break;
	    case 'y':
		if (tm->tm_year > 9 || !strip)
		    *buf++ = '0' + (tm->tm_year / 10) % 10;
		*buf++ = '0' + tm->tm_year % 10;
		break;
#ifndef HAVE_STRFTIME
	    case 'Y':
	    {
		int year, digits, testyear;
		year = tm->tm_year + 1900;
		digits = 1;
		testyear = year;
		while (testyear > 9) {
		    digits++;
		    testyear /= 10;
		}
		if (ztrftimebuf(&bufsize, digits))
		    return -1;
		sprintf(buf, "%d", year);
		buf += digits;
		break;
	    }
	    case 'a':
		if (ztrftimebuf(&bufsize, strlen(astr[tm->tm_wday]) - 2))
		    return -1;
		strucpy(&buf, astr[tm->tm_wday]);
		break;
	    case 'b':
		if (ztrftimebuf(&bufsize, strlen(estr[tm->tm_mon]) - 2))
		    return -1;
		strucpy(&buf, estr[tm->tm_mon]);
		break;
	    case 'p':
		*buf++ = (tm->tm_hour > 11) ? 'p' : 'a';
		*buf++ = 'm';
		break;
	    default:
		*buf++ = '%';
		if (fmt[-1] != '%')
		    *buf++ = fmt[-1];
#else
	    case 'E':
	    case 'O':
	    case '^':
	    case '#':
	    case '_':
	    case '-':
	    case '0': case '1': case '2': case '3': case '4':
	    case '5': case '6': case '7': case '8': case '9':
		goto morefmt;
strftimehandling:
	    default:
		/*
		 * Remember we've already allowed for two characters
		 * in the accounting in bufsize (but nowhere else).
		 */
		{
		    char origchar = fmt[-1];
		    int size = fmt - fmtstart;
		    char *tmp, *last;
		    tmp = zhalloc(size + 1);
		    strncpy(tmp, fmtstart, size);
		    last = fmt-1;
		    if (*last == Meta) {
			/*
			 * This is for consistency in counting:
			 * a metafiable character isn't actually
			 * a valid strftime descriptor.
			 *
			 * Previous characters were explicitly checked,
			 * so can't be metafied.
			 */
			*last = *++fmt ^ 32;
		    }
		    tmp[size] = '\0';
		    *buf = '\1';
		    if (!strftime(buf, bufsize + 2, tmp, tm))
		    {
			/*
			 * Some locales don't have strings for
			 * AM/PM, so empty output is valid.
			 */
			if (*buf || (origchar != 'p' && origchar != 'P')) {
			    if (*buf) {
				buf[0] = '\0';
				return -1;
			    }
			    return 0;
			}
		    }
		    decr = strlen(buf);
		    buf += decr;
		    bufsize -= decr - 2;
		}
#endif
		break;
	    }
	} else {
	    if (ztrftimebuf(&bufsize, 1))
		return -1;
	    *buf++ = *fmt++;
	}
    }
    *buf = '\0';
    return buf - origbuf;
}

/*
 * Return a string consisting of the elements of 'arr' joined by the character
 * 'delim', which will be metafied if necessary.  The string will be allocated
 * on the heap iff 'heap'.
 *
 * Comparable to:
 *
 *     char metafied_delim[] = { Meta, delim ^ 32, '\0' };
 *     sepjoin(arr, metafied_delim, heap)
 */

/**/
mod_export char *
zjoin(char **arr, int delim, int heap)
{
    int len = 0;
    char **s, *ret, *ptr;

    for (s = arr; *s; s++)
	len += strlen(*s) + 1 + (imeta(delim) ? 1 : 0);
    if (!len)
	return heap? "" : ztrdup("");
    ptr = ret = (char *) (heap ? zhalloc(len) : zalloc(len));
    for (s = arr; *s; s++) {
	strucpy(&ptr, *s);
	    if (imeta(delim)) {
		*ptr++ = Meta;
		*ptr++ = delim ^ 32;
	    }
	    else
		*ptr++ = delim;
    }
    ptr[-1 - (imeta(delim) ? 1 : 0)] = '\0';
    return ret;
}

/* Split a string containing a colon separated list *
 * of items into an array of strings.               */

/**/
mod_export char **
colonsplit(char *s, int uniq)
{
    int ct;
    char *t, **ret, **ptr, **p;

    for (t = s, ct = 0; *t; t++) /* count number of colons */
	if (*t == ':')
	    ct++;
    ptr = ret = (char **) zalloc(sizeof(char *) * (ct + 2));

    t = s;
    do {
	s = t;
        /* move t to point at next colon */
	for (; *t && *t != ':'; t++);
	if (uniq)
	    for (p = ret; p < ptr; p++)
		if ((int)strlen(*p) == t - s && ! strncmp(*p, s, t - s))
		    goto cont;
	*ptr = (char *) zalloc((t - s) + 1);
	ztrncpy(*ptr++, s, t - s);
      cont: ;
    }
    while (*t++);
    *ptr = NULL;
    return ret;
}

/**/
static int
skipwsep(char **s)
{
    char *t = *s;
    int i = 0;

    /*
     * Don't need to handle mutlibyte characters, they can't
     * be IWSEP.  Do need to check for metafication.
     */
    while (*t && iwsep(*t == Meta ? t[1] ^ 32 : *t)) {
	if (*t == Meta)
	    t++;
	t++;
	i++;
    }
    *s = t;
    return i;
}

/*
 * haven't worked out what allownull does; it's passed down from
 *   sepsplit but all the cases it's used are either 0 or 1 without
 *   a comment.  it seems to be something to do with the `nulstring'
 *   which i think is some kind of a metafication thing, so probably
 *   allownull's value is associated with whether we are using
 *   metafied strings.
 * see findsep() below for handling of `quote' argument
 */

/**/
mod_export char **
spacesplit(char *s, int allownull, int heap, int quote)
{
    char *t, **ret, **ptr;
    int l = sizeof(*ret) * (wordcount(s, NULL, -!allownull) + 1);
    char *(*dup)(const char *) = (heap ? dupstring : ztrdup);

    /* ### TODO: s/calloc/alloc/ */
    ptr = ret = (char **) (heap ? hcalloc(l) : zshcalloc(l));

    if (quote) {
	/*
	 * we will be stripping quoted separators by hacking string,
	 * so make sure it's hackable.
	 */
	s = dupstring(s);
    }

    t = s;
    skipwsep(&s);
    MB_METACHARINIT();
    if (*s && itype_end(s, ISEP, 1) != s)
	*ptr++ = dup(allownull ? "" : nulstring);
    else if (!allownull && t != s)
	*ptr++ = dup("");
    while (*s) {
	char *iend = itype_end(s, ISEP, 1);
	if (iend != s) {
	    s = iend;
	    skipwsep(&s);
	}
	else if (quote && *s == '\\') {
	    s++;
	    skipwsep(&s);
	}
	t = s;
	(void)findsep(&s, NULL, quote);
	if (s > t || allownull) {
	    *ptr = (char *) (heap ? zhalloc((s - t) + 1) :
		                     zalloc((s - t) + 1));
	    ztrncpy(*ptr++, t, s - t);
	} else
	    *ptr++ = dup(nulstring);
	t = s;
	skipwsep(&s);
    }
    if (!allownull && t != s)
	*ptr++ = dup("");
    *ptr = NULL;
    return ret;
}

/*
 * Find a separator.  Return 0 if already at separator, 1 if separator
 * found later, else -1.  (Historical note: used to return length into
 * string but this is all that is necessary and is less ambiguous with
 * multibyte characters around.)
 *
 * *s is the string we are looking along, which will be updated
 * to the point we have got to.
 *
 * sep is a possibly multicharacter separator to look for.  If NULL,
 * use normal separator characters.  If *sep is NULL, split on individual
 * characters.
 *
 * quote is a flag that '\<sep>' should not be treated as a separator.
 * in this case we need to be able to strip the backslash directly
 * in the string, so the calling function must have sent us something
 * modifiable.  currently this only works for sep == NULL.  also in
 * in this case only, we need to turn \\ into \.
 */

/**/
static int
findsep(char **s, char *sep, int quote)
{
    /*
     */
    int i, ilen;
    char *t, *tt;
    convchar_t c;

    MB_METACHARINIT();
    if (!sep) {
	for (t = *s; *t; t += ilen) {
	    if (quote && *t == '\\') {
		if (t[1] == '\\') {
		    chuck(t);
		    ilen = 1;
		    continue;
		} else {
		    ilen = MB_METACHARLENCONV(t+1, &c);
		    if (WC_ZISTYPE(c, ISEP)) {
			chuck(t);
			/* then advance over new character, length ilen */
		    } else {
			/* treat *t (backslash) as normal byte */
			if (isep(*t))
			    break;
			ilen = 1;
		    }
		}
	    } else {
		ilen = MB_METACHARLENCONV(t, &c);
		if (WC_ZISTYPE(c, ISEP))
		    break;
	    }
	}
	i = (t > *s);
	*s = t;
	return i;
    }
    if (!sep[0]) {
	/*
	 * NULL separator just means advance past first character,
	 * if any.
	 */
	if (**s) {
	    *s += MB_METACHARLEN(*s);
	    return 1;
	}
	return -1;
    }
    for (i = 0; **s; i++) {
	/*
	 * The following works for multibyte characters by virtue of
	 * the fact that sep may be a string (and we don't care how
	 * it divides up, we need to match all of it).
	 */
	for (t = sep, tt = *s; *t && *tt && *t == *tt; t++, tt++);
	if (!*t)
	    return (i > 0);
	*s += MB_METACHARLEN(*s);
    }
    return -1;
}

/**/
char *
findword(char **s, char *sep)
{
    char *r, *t;
    int sl;

    if (!**s)
	return NULL;

    if (sep) {
	sl = strlen(sep);
	r = *s;
	while (! findsep(s, sep, 0)) {
	    r = *s += sl;
	}
	return r;
    }
    MB_METACHARINIT();
    for (t = *s; *t; t += sl) {
	convchar_t c;
	sl = MB_METACHARLENCONV(t, &c);
	if (!WC_ZISTYPE(c, ISEP))
	    break;
    }
    *s = t;
    (void)findsep(s, sep, 0);
    return t;
}

/**/
int
wordcount(char *s, char *sep, int mul)
{
    int r, sl, c;

    if (sep) {
	r = 1;
	sl = strlen(sep);
	for (; (c = findsep(&s, sep, 0)) >= 0; s += sl)
	    if ((c || mul) && (sl || *(s + sl)))
		r++;
    } else {
	char *t = s;

	r = 0;
	if (mul <= 0)
	    skipwsep(&s);
	if ((*s && itype_end(s, ISEP, 1) != s) ||
	    (mul < 0 && t != s))
	    r++;
	for (; *s; r++) {
	    char *ie = itype_end(s, ISEP, 1);
	    if (ie != s) {
		s = ie;
		if (mul <= 0)
		    skipwsep(&s);
	    }
	    (void)findsep(&s, NULL, 0);
	    t = s;
	    if (mul <= 0)
		skipwsep(&s);
	}
	if (mul < 0 && t != s)
	    r++;
    }
    return r;
}

/*
 * 's' is a NULL-terminated array of strings.
 * 'sep' is a string, or NULL to split on ${IFS[1]}.
 *
 * Return a string consisting of the elements of 's' joined by 'sep',
 * allocated on the heap iff 'heap'.
 *
 * See also zjoin().
 */

/**/
mod_export char *
sepjoin(char **s, char *sep, int heap)
{
    char *r, *p, **t;
    int l, sl;
    char sepbuf[2];

    if (!*s)
	return heap ? dupstring("") : ztrdup("");
    if (!sep) {
	/* optimise common case that ifs[0] is space */
	if (ifs && *ifs != ' ') {
	    MB_METACHARINIT();
	    sep = dupstrpfx(ifs, MB_METACHARLEN(ifs));
	} else {
	    p = sep = sepbuf;
	    *p++ = ' ';
	    *p = '\0';
	}
    }
    sl = strlen(sep);
    for (t = s, l = 1 - sl; *t; l += strlen(*t) + sl, t++);
    r = p = (char *) (heap ? zhalloc(l) : zalloc(l));
    t = s;
    while (*t) {
	strucpy(&p, *t);
	if (*++t)
	    strucpy(&p, sep);
    }
    *p = '\0';
    return r;
}

/**/
char **
sepsplit(char *s, char *sep, int allownull, int heap)
{
    int n, sl;
    char *t, *tt, **r, **p;

    /* Null string?  Treat as empty string. */
    if (s[0] == Nularg && !s[1])
	s++;

    if (!sep)
	return spacesplit(s, allownull, heap, 0);

    sl = strlen(sep);
    n = wordcount(s, sep, 1);
    r = p = (char **) (heap ? zhalloc((n + 1) * sizeof(char *)) :
	                       zalloc((n + 1) * sizeof(char *)));

    for (t = s; n--;) {
	tt = t;
	(void)findsep(&t, sep, 0);
	*p = (char *) (heap ? zhalloc(t - tt + 1) :
	                       zalloc(t - tt + 1));
	strncpy(*p, tt, t - tt);
	(*p)[t - tt] = '\0';
	p++;
	t += sl;
    }
    *p = NULL;

    return r;
}

/* Get the definition of a shell function */

/**/
mod_export Shfunc
getshfunc(char *nam)
{
    return (Shfunc) shfunctab->getnode(shfunctab, nam);
}

/*
 * Call the function func to substitute string orig by setting
 * the parameter reply.
 * Return the array from reply, or NULL if the function returned
 * non-zero status.
 * The returned value comes directly from the parameter and
 * so should be used before there is any chance of that
 * being changed or unset.
 * If arg1 is not NULL, it is used as an initial argument to
 * the function, with the original string as the second argument.
 */

/**/
char **
subst_string_by_func(Shfunc func, char *arg1, char *orig)
{
    int osc = sfcontext, osm = stopmsg, old_incompfunc = incompfunc;
    LinkList l = newlinklist();
    char **ret;

    addlinknode(l, func->node.nam);
    if (arg1)
	addlinknode(l, arg1);
    addlinknode(l, orig);
    sfcontext = SFC_SUBST;
    incompfunc = 0;

    if (doshfunc(func, l, 1))
	ret = NULL;
    else
	ret = getaparam("reply");

    sfcontext = osc;
    stopmsg = osm;
    incompfunc = old_incompfunc;
    return ret;
}

/**
 * Front end to subst_string_by_func to use hook-like logic.
 * name can refer to a function, and name + "_hook" can refer
 * to an array containing a list of functions.  The functions
 * are tried in order until one returns success.
 */
/**/
char **
subst_string_by_hook(char *name, char *arg1, char *orig)
{
    Shfunc func;
    char **ret = NULL;

    if ((func = getshfunc(name))) {
	ret = subst_string_by_func(func, arg1, orig);
    }

    if (!ret) {
	char **arrptr;
	int namlen = strlen(name);
	VARARR(char, arrnam, namlen + HOOK_SUFFIX_LEN);
	memcpy(arrnam, name, namlen);
	memcpy(arrnam + namlen, HOOK_SUFFIX, HOOK_SUFFIX_LEN);

	if ((arrptr = getaparam(arrnam))) {
	    /* Guard against internal modification of the array */
	    arrptr = arrdup(arrptr);
	    for (; *arrptr; arrptr++) {
		if ((func = getshfunc(*arrptr))) {
		    ret = subst_string_by_func(func, arg1, orig);
		    if (ret)
			break;
		}
	    }
	}
    }

    return ret;
}

/**/
mod_export char **
mkarray(char *s)
{
    char **t = (char **) zalloc((s) ? (2 * sizeof s) : (sizeof s));

    if ((*t = s))
	t[1] = NULL;
    return t;
}

/**/
mod_export char **
hmkarray(char *s)
{
    char **t = (char **) zhalloc((s) ? (2 * sizeof s) : (sizeof s));

    if ((*t = s))
	t[1] = NULL;
    return t;
}

/**/
mod_export void
zbeep(void)
{
    char *vb;
    queue_signals();
    if ((vb = getsparam_u("ZBEEP"))) {
	int len;
	vb = getkeystring(vb, &len, GETKEYS_BINDKEY, NULL);
	write_loop(SHTTY, vb, len);
    } else if (isset(BEEP))
	write_loop(SHTTY, "\07", 1);
    unqueue_signals();
}

/**/
mod_export void
freearray(char **s)
{
    char **t = s;

    DPUTS(!s, "freearray() with zero argument");

    while (*s)
	zsfree(*s++);
    free(t);
}

/**/
int
equalsplit(char *s, char **t)
{
    for (; *s && *s != '='; s++);
    if (*s == '=') {
	*s++ = '\0';
	*t = s;
	return 1;
    }
    return 0;
}


/* the ztypes table */

/**/
mod_export short int typtab[256];
static int typtab_flags = 0;

/* initialize the ztypes table */

/**/
void
inittyptab(void)
{
    int t0;
    char *s;

    if (!(typtab_flags & ZTF_INIT)) {
	typtab_flags = ZTF_INIT;
	if (interact && isset(SHINSTDIN))
	    typtab_flags |= ZTF_INTERACT;
    }

    queue_signals();

    memset(typtab, 0, sizeof(typtab));
    for (t0 = 0; t0 != 32; t0++)
	typtab[t0] = typtab[t0 + 128] = ICNTRL;
    typtab[127] = ICNTRL;
    for (t0 = '0'; t0 <= '9'; t0++)
	typtab[t0] = IDIGIT | IALNUM | IWORD | IIDENT | IUSER;
    for (t0 = 'a'; t0 <= 'z'; t0++)
	typtab[t0] = typtab[t0 - 'a' + 'A'] = IALPHA | IALNUM | IIDENT | IUSER | IWORD;
#ifndef MULTIBYTE_SUPPORT
    /*
     * This really doesn't seem to me the right thing to do when
     * we have multibyte character support...  it was a hack to assume
     * eight bit characters `worked' for some values of work before
     * we could test for them properly.  I'm not 100% convinced
     * having IIDENT here is a good idea at all, but this code
     * should disappear into history...
     */
    for (t0 = 0240; t0 != 0400; t0++)
	typtab[t0] = IALPHA | IALNUM | IIDENT | IUSER | IWORD;
#endif
    /* typtab['.'] |= IIDENT; */ /* Allow '.' in variable names - broken */
    typtab['_'] = IIDENT | IUSER;
    typtab['-'] = typtab['.'] = typtab[STOUC(Dash)] = IUSER;
    typtab[' '] |= IBLANK | INBLANK;
    typtab['\t'] |= IBLANK | INBLANK;
    typtab['\n'] |= INBLANK;
    typtab['\0'] |= IMETA;
    typtab[STOUC(Meta)  ] |= IMETA;
    typtab[STOUC(Marker)] |= IMETA;
    for (t0 = (int)STOUC(Pound); t0 <= (int)STOUC(LAST_NORMAL_TOK); t0++)
	typtab[t0] |= ITOK | IMETA;
    for (t0 = (int)STOUC(Snull); t0 <= (int)STOUC(Nularg); t0++)
	typtab[t0] |= ITOK | IMETA | INULL;
    for (s = ifs ? ifs : EMULATION(EMULATE_KSH|EMULATE_SH) ?
	DEFAULT_IFS_SH : DEFAULT_IFS; *s; s++) {
	int c = STOUC(*s == Meta ? *++s ^ 32 : *s);
#ifdef MULTIBYTE_SUPPORT
	if (!isascii(c)) {
	    /* see comment for wordchars below */
	    continue;
	}
#endif
	if (inblank(c)) {
	    if (s[1] == c)
		s++;
	    else
		typtab[c] |= IWSEP;
	}
	typtab[c] |= ISEP;
    }
    for (s = wordchars ? wordchars : DEFAULT_WORDCHARS; *s; s++) {
	int c = STOUC(*s == Meta ? *++s ^ 32 : *s);
#ifdef MULTIBYTE_SUPPORT
	if (!isascii(c)) {
	    /*
	     * If we have support for multibyte characters, we don't
	     * handle non-ASCII characters here; instead, we turn
	     * wordchars into a wide character array.
	     * (We may actually have a single-byte 8-bit character set,
	     * but it works the same way.)
	     */
	    continue;
	}
#endif
	typtab[c] |= IWORD;
    }
#ifdef MULTIBYTE_SUPPORT
    set_widearray(wordchars, &wordchars_wide);
    set_widearray(ifs ? ifs : EMULATION(EMULATE_KSH|EMULATE_SH) ?
	DEFAULT_IFS_SH : DEFAULT_IFS, &ifs_wide);
#endif
    for (s = SPECCHARS; *s; s++)
	typtab[STOUC(*s)] |= ISPECIAL;
    if (typtab_flags & ZTF_SP_COMMA)
	typtab[STOUC(',')] |= ISPECIAL;
    if (isset(BANGHIST) && bangchar && (typtab_flags & ZTF_INTERACT)) {
	typtab_flags |= ZTF_BANGCHAR;
	typtab[bangchar] |= ISPECIAL;
    } else
	typtab_flags &= ~ZTF_BANGCHAR;
    for (s = PATCHARS; *s; s++)
	typtab[STOUC(*s)] |= IPATTERN;

    unqueue_signals();
}

/**/
mod_export void
makecommaspecial(int yesno)
{
    if (yesno != 0) {
	typtab_flags |= ZTF_SP_COMMA;
	typtab[STOUC(',')] |= ISPECIAL;
    } else {
	typtab_flags &= ~ZTF_SP_COMMA;
	typtab[STOUC(',')] &= ~ISPECIAL;
    }
}

/**/
mod_export void
makebangspecial(int yesno)
{
    /* Name and call signature for congruence with makecommaspecial(),
     * but in this case when yesno is nonzero we defer to the state
     * saved by inittyptab().
     */ 
    if (yesno == 0) {
	typtab[bangchar] &= ~ISPECIAL;
    } else if (typtab_flags & ZTF_BANGCHAR) {
	typtab[bangchar] |= ISPECIAL;
    }
}


/**/
#ifdef MULTIBYTE_SUPPORT
/* A wide-character version of the iblank() macro. */
/**/
mod_export int
wcsiblank(wint_t wc)
{
    if (iswspace(wc) && wc != L'\n')
	return 1;
    return 0;
}

/*
 * zistype macro extended to support wide characters.
 * Works for IIDENT, IWORD, IALNUM, ISEP.
 * We don't need this for IWSEP because that only applies to
 * a fixed set of ASCII characters.
 * Note here that use of multibyte mode is not tested:
 * that's because for ZLE this is unconditional,
 * not dependent on the option.  The caller must decide.
 */

/**/
mod_export int
wcsitype(wchar_t c, int itype)
{
    int len;
    mbstate_t mbs;
    VARARR(char, outstr, MB_CUR_MAX);

    if (!isset(MULTIBYTE))
	return zistype(c, itype);

    /*
     * Strategy:  the shell requires that the multibyte representation
     * be an extension of ASCII.  So see if converting the character
     * produces an ASCII character.  If it does, use zistype on that.
     * If it doesn't, use iswalnum on the original character.
     * If that fails, resort to the appropriate wide character array.
     */
    memset(&mbs, 0, sizeof(mbs));
    len = wcrtomb(outstr, c, &mbs);

    if (len == 0) {
	/* NULL is special */
	return zistype(0, itype);
    } else if (len == 1 && isascii(outstr[0])) {
	return zistype(outstr[0], itype);
    } else {
	switch (itype) {
	case IIDENT:
	    if (isset(POSIXIDENTIFIERS))
		return 0;
	    return iswalnum(c);

	case IWORD:
	    if (iswalnum(c))
		return 1;
	    /*
	     * If we are handling combining characters, any punctuation
	     * characters with zero width needs to be considered part of
	     * a word.  If we are not handling combining characters then
	     * logically they are still part of the word, even if they
	     * don't get displayed properly, so always do this.
	     */
	    if (IS_COMBINING(c))
		return 1;
	    return !!wmemchr(wordchars_wide.chars, c, wordchars_wide.len);

	case ISEP:
	    return !!wmemchr(ifs_wide.chars, c, ifs_wide.len);

	default:
	    return iswalnum(c);
	}
    }
}

/**/
#endif


/*
 * Find the end of a set of characters in the set specified by itype;
 * one of IALNUM, IIDENT, IWORD or IUSER.  For non-ASCII characters, we assume
 * alphanumerics are part of the set, with the exception that
 * identifiers are not treated that way if POSIXIDENTIFIERS is set.
 *
 * See notes above for identifiers.
 * Returns the same pointer as passed if not on an identifier character.
 * If "once" is set, just test the first character, i.e. (outptr !=
 * inptr) tests whether the first character is valid in an identifier.
 *
 * Currently this is only called with itype IIDENT, IUSER or ISEP.
 */

/**/
mod_export char *
itype_end(const char *ptr, int itype, int once)
{
#ifdef MULTIBYTE_SUPPORT
    if (isset(MULTIBYTE) &&
	(itype != IIDENT || !isset(POSIXIDENTIFIERS))) {
	mb_charinit();
	while (*ptr) {
	    int len;
	    if (itok(*ptr)) {
		/* Not untokenised yet --- can happen in raw command line */
		len = 1;
		if (!zistype(*ptr,itype))
		    break;
	    } else {
		wint_t wc;
		len = mb_metacharlenconv(ptr, &wc);

		if (!len)
		    break;

		if (wc == WEOF) {
		    /* invalid, treat as single character */
		    int chr = STOUC(*ptr == Meta ? ptr[1] ^ 32 : *ptr);
		    /* in this case non-ASCII characters can't match */
		    if (chr > 127 || !zistype(chr,itype))
			break;
		} else if (len == 1 && isascii(*ptr)) {
		    /* ASCII: can't be metafied, use standard test */
		    if (!zistype(*ptr,itype))
			break;
		} else {
		    /*
		     * Valid non-ASCII character.
		     */
		    switch (itype) {
		    case IWORD:
			if (!iswalnum(wc) &&
			    !wmemchr(wordchars_wide.chars, wc,
				     wordchars_wide.len))
			    return (char *)ptr;
			break;

		    case ISEP:
			if (!wmemchr(ifs_wide.chars, wc, ifs_wide.len))
			    return (char *)ptr;
			break;

		    default:
			if (!iswalnum(wc))
			    return (char *)ptr;
		    }
		}
	    }
	    ptr += len;

	    if (once)
		break;
	}
    } else
#endif
	for (;;) {
	    int chr = STOUC(*ptr == Meta ? ptr[1] ^ 32 : *ptr);
	    if (!zistype(chr,itype))
		break;
	    ptr += (*ptr == Meta) ? 2 : 1;

	    if (once)
		break;
	}

    /*
     * Nasty.  The first argument is const char * because we
     * don't modify it here.  However, we really want to pass
     * back the same type as was passed down, to allow idioms like
     *   p = itype_end(p, IIDENT, 0);
     * So returning a const char * isn't really the right thing to do.
     * Without having two different functions the following seems
     * to be the best we can do.
     */
    return (char *)ptr;
}

/**/
mod_export char **
arrdup(char **s)
{
    char **x, **y;

    y = x = (char **) zhalloc(sizeof(char *) * (arrlen(s) + 1));

    while ((*x++ = dupstring(*s++)));

    return y;
}

/* Duplicate at most max elements of the array s with heap memory */

/**/
mod_export char **
arrdup_max(char **s, unsigned max)
{
    char **x, **y, **send;
    int len = 0;

    if (max)
	len = arrlen(s);

    /* Limit has sense only if not equal to len */
    if (max > len)
        max = len;

    y = x = (char **) zhalloc(sizeof(char *) * (max + 1));

    send = s + max;
    while (s < send)
	*x++ = dupstring(*s++);
    *x = NULL;

    return y;
}

/**/
mod_export char **
zarrdup(char **s)
{
    char **x, **y;

    y = x = (char **) zalloc(sizeof(char *) * (arrlen(s) + 1));

    while ((*x++ = ztrdup(*s++)));

    return y;
}

/**/
#ifdef MULTIBYTE_SUPPORT
/**/
mod_export wchar_t **
wcs_zarrdup(wchar_t **s)
{
    wchar_t **x, **y;

    y = x = (wchar_t **) zalloc(sizeof(wchar_t *) * (arrlen((char **)s) + 1));

    while ((*x++ = wcs_ztrdup(*s++)));

    return y;
}
/**/
#endif /* MULTIBYTE_SUPPORT */

/**/
static char *
spname(char *oldname)
{
    char *p, spnameguess[PATH_MAX + 1], spnamebest[PATH_MAX + 1];
    static char newname[PATH_MAX + 1];
    char *new = newname, *old = oldname;
    int bestdist = 0, thisdist, thresh, maxthresh = 0;

    /* This loop corrects each directory component of the path, stopping *
     * when any correction distance would exceed the distance threshold. *
     * NULL is returned only if the first component cannot be corrected; *
     * otherwise a copy of oldname with a corrected prefix is returned.  *
     * Rationale for this, if there ever was any, has been forgotten.    */
    for (;;) {
	while (*old == '/') {
            if (new >= newname + sizeof(newname) - 1)
		return NULL;
	    *new++ = *old++;
	}
	*new = '\0';
	if (*old == '\0')
	    return newname;
	p = spnameguess;
	for (; *old != '/' && *old != '\0'; old++)
	    if (p < spnameguess + PATH_MAX)
		*p++ = *old;
	*p = '\0';
	/* Every component is allowed a single distance 2 correction or two *
	 * distance 1 corrections.  Longer ones get additional corrections. */
	thresh = (int)(p - spnameguess) / 4 + 1;
	if (thresh < 3)
	    thresh = 3;
	else if (thresh > 100)
	    thresh = 100;
	thisdist = mindist(newname, spnameguess, spnamebest, *old == '/');
	if (thisdist >= thresh) {
	    /* The next test is always true, except for the first path    *
	     * component.  We could initialize bestdist to some large     *
	     * constant instead, and then compare to that constant here,  *
	     * because an invariant is that we've never exceeded the      *
	     * threshold for any component so far; but I think that looks *
	     * odd to the human reader, and we may make use of the total  *
	     * distance for all corrections at some point in the future.  */
	    if (bestdist < maxthresh) {
		struncpy(&new, spnameguess, sizeof(newname) - (new - newname));
		struncpy(&new, old, sizeof(newname) - (new - newname));
		return (new >= newname + sizeof(newname) -1) ? NULL : newname;
	    } else
	    	return NULL;
	} else {
	    maxthresh = bestdist + thresh;
	    bestdist += thisdist;
	}
	for (p = spnamebest; (*new = *p++);) {
	    if (new >= newname + sizeof(newname) - 1)
		return NULL;
	    new++;
	}
    }
}

/**/
static int
mindist(char *dir, char *mindistguess, char *mindistbest, int wantdir)
{
    int mindistd, nd;
    DIR *dd;
    char *fn;
    char *buf;
    struct stat st;
    size_t dirlen;

    if (dir[0] == '\0')
	dir = ".";
    mindistd = 100;

    if (!(buf = zalloc((dirlen = strlen(dir)) + strlen(mindistguess) + 2)))
	return 0;
    sprintf(buf, "%s/%s", dir, mindistguess);

    if (stat(unmeta(buf), &st) == 0 && (!wantdir || S_ISDIR(st.st_mode))) {
	strcpy(mindistbest, mindistguess);
	free(buf);
	return 0;
    }

    if ((dd = opendir(unmeta(dir)))) {
	while ((fn = zreaddir(dd, 0))) {
	    if (spnamepat && pattry(spnamepat, fn))
		continue;
	    nd = spdist(fn, mindistguess,
			(int)strlen(mindistguess) / 4 + 1);
	    if (nd <= mindistd) {
		if (wantdir) {
		    if (!(buf = zrealloc(buf, dirlen + strlen(fn) + 2)))
			continue;
		    sprintf(buf, "%s/%s", dir, fn);
		    if (stat(unmeta(buf), &st) != 0 || !S_ISDIR(st.st_mode))
			continue;
		}
		strcpy(mindistbest, fn);
		mindistd = nd;
		if (mindistd == 0)
		    break;
	    }
	}
	closedir(dd);
    }
    free(buf);
    return mindistd;
}

/**/
static int
spdist(char *s, char *t, int thresh)
{
    /* TODO: Correction for non-ASCII and multibyte-input keyboards. */
    char *p, *q;
    const char qwertykeymap[] =
    "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\
\t1234567890-=\t\
\tqwertyuiop[]\t\
\tasdfghjkl;'\n\t\
\tzxcvbnm,./\t\t\t\
\n\n\n\n\n\n\n\n\n\n\n\n\n\n\
\t!@#$%^&*()_+\t\
\tQWERTYUIOP{}\t\
\tASDFGHJKL:\"\n\t\
\tZXCVBNM<>?\n\n\t\
\n\n\n\n\n\n\n\n\n\n\n\n\n\n";
    const char dvorakkeymap[] =
    "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\
\t1234567890[]\t\
\t',.pyfgcrl/=\t\
\taoeuidhtns-\n\t\
\t;qjkxbmwvz\t\t\t\
\n\n\n\n\n\n\n\n\n\n\n\n\n\n\
\t!@#$%^&*(){}\t\
\t\"<>PYFGCRL?+\t\
\tAOEUIDHTNS_\n\t\
\t:QJKXBMWVZ\n\n\t\
\n\n\n\n\n\n\n\n\n\n\n\n\n\n";
    const char *keymap;
    if ( isset( DVORAK ) )
      keymap = dvorakkeymap;
    else
      keymap = qwertykeymap;

    if (!strcmp(s, t))
	return 0;
    /* any number of upper/lower mistakes allowed (dist = 1) */
    for (p = s, q = t; *p && tulower(*p) == tulower(*q); p++, q++);
    if (!*p && !*q)
	return 1;
    if (!thresh)
	return 200;
    for (p = s, q = t; *p && *q; p++, q++)
	if (*p == *q)
	    continue;		/* don't consider "aa" transposed, ash */
	else if (p[1] == q[0] && q[1] == p[0])	/* transpositions */
	    return spdist(p + 2, q + 2, thresh - 1) + 1;
	else if (p[1] == q[0])	/* missing letter */
	    return spdist(p + 1, q + 0, thresh - 1) + 2;
	else if (p[0] == q[1])	/* missing letter */
	    return spdist(p + 0, q + 1, thresh - 1) + 2;
	else if (*p != *q)
	    break;
    if ((!*p && strlen(q) == 1) || (!*q && strlen(p) == 1))
	return 2;
    for (p = s, q = t; *p && *q; p++, q++)
	if (p[0] != q[0] && p[1] == q[1]) {
	    int t0;
	    char *z;

	    /* mistyped letter */

	    if (!(z = strchr(keymap, p[0])) || *z == '\n' || *z == '\t')
		return spdist(p + 1, q + 1, thresh - 1) + 1;
	    t0 = z - keymap;
	    if (*q == keymap[t0 - 15] || *q == keymap[t0 - 14] ||
		*q == keymap[t0 - 13] ||
		*q == keymap[t0 - 1] || *q == keymap[t0 + 1] ||
		*q == keymap[t0 + 13] || *q == keymap[t0 + 14] ||
		*q == keymap[t0 + 15])
		return spdist(p + 1, q + 1, thresh - 1) + 2;
	    return 200;
	} else if (*p != *q)
	    break;
    return 200;
}

/* set cbreak mode, or the equivalent */

/**/
void
setcbreak(void)
{
    struct ttyinfo ti;

    ti = shttyinfo;
#ifdef HAS_TIO
    ti.tio.c_lflag &= ~ICANON;
    ti.tio.c_cc[VMIN] = 1;
    ti.tio.c_cc[VTIME] = 0;
#else
    ti.sgttyb.sg_flags |= CBREAK;
#endif
    settyinfo(&ti);
}

/* give the tty to some process */

/**/
mod_export void
attachtty(pid_t pgrp)
{
    static int ep = 0;

    if (jobbing && interact) {
#ifdef HAVE_TCSETPGRP
	if (SHTTY != -1 && tcsetpgrp(SHTTY, pgrp) == -1 && !ep)
#else
# if ardent
	if (SHTTY != -1 && setpgrp() == -1 && !ep)
# else
	int arg = pgrp;

	if (SHTTY != -1 && ioctl(SHTTY, TIOCSPGRP, &arg) == -1 && !ep)
# endif
#endif
	{
	    if (pgrp != mypgrp && kill(-pgrp, 0) == -1)
		attachtty(mypgrp);
	    else {
		if (errno != ENOTTY)
		{
		    zwarn("can't set tty pgrp: %e", errno);
		    fflush(stderr);
		}
		opts[MONITOR] = 0;
		ep = 1;
	    }
	}
	else
	{
	    last_attached_pgrp = pgrp;
	}
    }
}

/* get the process group associated with the tty */

/**/
pid_t
gettygrp(void)
{
    pid_t arg;

    if (SHTTY == -1)
	return -1;

#ifdef HAVE_TCSETPGRP
    arg = tcgetpgrp(SHTTY);
#else
    ioctl(SHTTY, TIOCGPGRP, &arg);
#endif

    return arg;
}


/* Escape tokens and null characters.  Buf is the string which should be     *
 * escaped.  len is the length of the string.  If len is -1, buf should be   *
 * null terminated.  If len is non-negative and the third parameter is not   *
 * META_DUP, buf should point to an at least len+1 long memory area.  The    *
 * return value points to the quoted string.  If the given string does not   *
 * contain any special character which should be quoted and the third        *
 * parameter is not META_(HEAP|)DUP, buf is returned unchanged (a            *
 * terminating null character is appended to buf if necessary).  Otherwise   *
 * the third `heap' argument determines the method used to allocate space    *
 * for the result.  It can have the following values:                        *
 *   META_REALLOC:  use zrealloc on buf                                      *
 *   META_HREALLOC: use hrealloc on buf                                      *
 *   META_USEHEAP:  get memory from the heap.  This leaves buf unchanged.    *
 *   META_NOALLOC:  buf points to a memory area which is long enough to hold *
 *                  the quoted form, just quote it and return buf.           *
 *   META_STATIC:   store the quoted string in a static area.  The original  *
 *                  string should be at most PATH_MAX long.                  *
 *   META_ALLOC:    allocate memory for the new string with zalloc().        *
 *   META_DUP:      leave buf unchanged and allocate space for the return    *
 *                  value even if buf does not contains special characters   *
 *   META_HEAPDUP:  same as META_DUP, but uses the heap                      */

/**/
mod_export char *
metafy(char *buf, int len, int heap)
{
    int meta = 0;
    char *t, *p, *e;
    static char mbuf[PATH_MAX*2+1];

    if (len == -1) {
	for (e = buf, len = 0; *e; len++)
	    if (imeta(*e++))
		meta++;
    } else
	for (e = buf; e < buf + len;)
	    if (imeta(*e++))
		meta++;

    if (meta || heap == META_DUP || heap == META_HEAPDUP) {
	switch (heap) {
	case META_REALLOC:
	    buf = zrealloc(buf, len + meta + 1);
	    break;
	case META_HREALLOC:
	    buf = hrealloc(buf, len, len + meta + 1);
	    break;
	case META_ALLOC:
	case META_DUP:
	    buf = memcpy(zalloc(len + meta + 1), buf, len);
	    break;
	case META_USEHEAP:
	case META_HEAPDUP:
	    buf = memcpy(zhalloc(len + meta + 1), buf, len);
	    break;
	case META_STATIC:
#ifdef DEBUG
	    if (len > PATH_MAX) {
		fprintf(stderr, "BUG: len = %d > PATH_MAX in metafy\n", len);
		fflush(stderr);
	    }
#endif
	    buf = memcpy(mbuf, buf, len);
	    break;
#ifdef DEBUG
	case META_NOALLOC:
	    break;
	default:
	    fprintf(stderr, "BUG: metafy called with invalid heap value\n");
	    fflush(stderr);
	    break;
#endif
	}
	p = buf + len;
	e = t = buf + len + meta;
	while (meta) {
	    if (imeta(*--t = *--p)) {
		*t-- ^= 32;
		*t = Meta;
		meta--;
	    }
	}
    }
    *e = '\0';
    return buf;
}


/*
 * Duplicate a string, metafying it as we go.
 *
 * Typically, this is used only for strings imported from outside
 * zsh, as strings internally are either already metafied or passed
 * around with an associated length.
 */
/**/
mod_export char *
ztrdup_metafy(const char *s)
{
    /* To mimic ztrdup() behaviour */
    if (!s)
	return NULL;
    /*
     * metafy() does lots of different things, so the pointer
     * isn't const.  Using it with META_DUP should be safe.
     */
    return metafy((char *)s, -1, META_DUP);
}


/*
 * Take a null-terminated, metafied string in s into a literal
 * representation by converting in place.  The length is in *len
 * len is non-NULL; if len is NULL, you don't know the length of
 * the final string, but if it's to be supplied to some system
 * routine that always uses NULL termination, such as a filename
 * interpreter, that doesn't matter.  Note the NULL termination
 * is always copied for purposes of that kind.
 */

/**/
mod_export char *
unmetafy(char *s, int *len)
{
    char *p, *t;

    for (p = s; *p && *p != Meta; p++);
    for (t = p; (*t = *p++);)
	if (*t++ == Meta && *p)
	    t[-1] = *p++ ^ 32;
    if (len)
	*len = t - s;
    return s;
}

/* Return the character length of a metafied substring, given the      *
 * unmetafied substring length.                                        */

/**/
mod_export int
metalen(const char *s, int len)
{
    int mlen = len;

    while (len--) {
	if (*s++ == Meta) {
	    mlen++;
	    s++;
	}
    }
    return mlen;
}

/*
 * This function converts a zsh internal string to a form which can be
 * passed to a system call as a filename.  The result is stored in a
 * single static area, sized to fit.  If there is no Meta character
 * the original string is returned.
 */

/**/
mod_export char *
unmeta(const char *file_name)
{
    static char *fn;
    static int sz;
    char *p;
    const char *t;
    int newsz, meta;

    if (!file_name)
	return NULL;

    meta = 0;
    for (t = file_name; *t; t++) {
	if (*t == Meta)
	    meta = 1;
    }
    if (!meta) {
	/*
	 * don't need allocation... free if it's long, see below
	 */
	if (sz > 4 * PATH_MAX) {
	    zfree(fn, sz);
	    fn = NULL;
	    sz = 0;
	}
	return (char *) file_name;
    }

    newsz = (t - file_name) + 1;
    /*
     * Optimisation: don't resize if we don't have to.
     * We need a new allocation if
     * - nothing was allocated before
     * - the new string is larger than the old one
     * - the old string was larger than an arbitrary limit but the
     *   new string isn't so that we free up significant space by resizing.
     */
    if (!fn || newsz > sz || (sz > 4 * PATH_MAX && newsz <= 4 * PATH_MAX))
    {
	if (fn)
	    zfree(fn, sz);
	sz = newsz;
	fn = (char *)zalloc(sz);
	if (!fn) {
	    sz = 0;
	    /*
	     * will quite likely crash in the caller anyway...
	     */
	    return NULL;
	}
    }

    for (t = file_name, p = fn; *t; p++)
	if ((*p = *t++) == Meta && *t)
	    *p = *t++ ^ 32;
    *p = '\0';
    return fn;
}

/*
 * Unmetafy just one character and store the number of bytes it occupied.
 */
/**/
mod_export convchar_t
unmeta_one(const char *in, int *sz)
{
    convchar_t wc;
    int newsz;
#ifdef MULTIBYTE_SUPPORT
    mbstate_t wstate;
#endif

    if (!sz)
	sz = &newsz;
    *sz = 0;

    if (!in || !*in)
	return 0;

#ifdef MULTIBYTE_SUPPORT
    memset(&wstate, 0, sizeof(wstate));
    *sz = mb_metacharlenconv_r(in, &wc, &wstate);
#else
    if (in[0] == Meta) {
      *sz = 2;
      wc = STOUC(in[1] ^ 32);
    } else {
      *sz = 1;
      wc = STOUC(in[0]);
    }
#endif
    return wc;
}

/*
 * Unmetafy and compare two strings, comparing unsigned character values.
 * "a\0" sorts after "a".
 *
 * Currently this is only used in hash table sorting, where the
 * keys are names of hash nodes and where we don't use strcoll();
 * it's not clear if that's right but it does guarantee the ordering
 * of shell structures on output.
 *
 * As we don't use strcoll(), it seems overkill to convert multibyte
 * characters to wide characters for comparison every time.  In the case
 * of UTF-8, Unicode ordering is preserved when sorted raw, and for
 * other character sets we rely on an extension of ASCII so the result,
 * while it may not be correct, is at least rational.
 */

/**/
int
ztrcmp(char const *s1, char const *s2)
{
    int c1, c2;

    while(*s1 && *s1 == *s2) {
	s1++;
	s2++;
    }

    if(!(c1 = *s1))
	c1 = -1;
    else if(c1 == STOUC(Meta))
	c1 = *++s1 ^ 32;
    if(!(c2 = *s2))
	c2 = -1;
    else if(c2 == STOUC(Meta))
	c2 = *++s2 ^ 32;

    if(c1 == c2)
	return 0;
    else if(c1 < c2)
	return -1;
    else
	return 1;
}

/* Return the unmetafied length of a metafied string. */

/**/
mod_export int
ztrlen(char const *s)
{
    int l;

    for (l = 0; *s; l++) {
	if (*s++ == Meta) {
#ifdef DEBUG
	    if (! *s) {
		fprintf(stderr, "BUG: unexpected end of string in ztrlen()\n");
		break;
	    } else
#endif
	    s++;
	}
    }
    return l;
}

#ifndef MULTIBYTE_SUPPORT
/*
 * ztrlen() but with explicit end point for non-null-terminated
 * segments.  eptr may not be NULL.
 */

/**/
mod_export int
ztrlenend(char const *s, char const *eptr)
{
    int l;

    for (l = 0; s < eptr; l++) {
	if (*s++ == Meta) {
#ifdef DEBUG
	    if (! *s) {
		fprintf(stderr,
			"BUG: unexpected end of string in ztrlenend()\n");
		break;
	    } else
#endif
	    s++;
	}
    }
    return l;
}

#endif /* MULTIBYTE_SUPPORT */

/* Subtract two pointers in a metafied string. */

/**/
mod_export int
ztrsub(char const *t, char const *s)
{
    int l = t - s;

    while (s != t) {
	if (*s++ == Meta) {
#ifdef DEBUG
	    if (! *s || s == t)
		fprintf(stderr, "BUG: substring ends in the middle of a metachar in ztrsub()\n");
	    else
#endif
	    s++;
	    l--;
	}
    }
    return l;
}

/*
 * Wrapper for readdir().
 *
 * If ignoredots is true, skip the "." and ".." entries.
 *
 * When __APPLE__ is defined, recode dirent names from UTF-8-MAC to UTF-8.
 *
 * Return the dirent's name, metafied.
 */

/**/
mod_export char *
zreaddir(DIR *dir, int ignoredots)
{
    struct dirent *de;
#if defined(HAVE_ICONV) && defined(__APPLE__)
    static iconv_t conv_ds = (iconv_t)0;
    static char *conv_name = 0;
    char *conv_name_ptr, *orig_name_ptr;
    size_t conv_name_len, orig_name_len;
#endif

    do {
	de = readdir(dir);
	if(!de)
	    return NULL;
    } while(ignoredots && de->d_name[0] == '.' &&
	(!de->d_name[1] || (de->d_name[1] == '.' && !de->d_name[2])));

#if defined(HAVE_ICONV) && defined(__APPLE__)
    if (!conv_ds)
	conv_ds = iconv_open("UTF-8", "UTF-8-MAC");
    if (conv_ds != (iconv_t)(-1)) {
	/* Force initial state in case re-using conv_ds */
	(void) iconv(conv_ds, 0, &orig_name_len, 0, &conv_name_len);

	orig_name_ptr = de->d_name;
	orig_name_len = strlen(de->d_name);
	conv_name = zrealloc(conv_name, orig_name_len+1);
	conv_name_ptr = conv_name;
	conv_name_len = orig_name_len;
	if (iconv(conv_ds,
		  &orig_name_ptr, &orig_name_len,
		  &conv_name_ptr, &conv_name_len) != (size_t)(-1) &&
	    orig_name_len == 0) {
	    /* Completely converted, metafy and return */
	    *conv_name_ptr = '\0';
	    return metafy(conv_name, -1, META_STATIC);
	}
	/* Error, or conversion incomplete, keep the original name */
    }
#endif

    return metafy(de->d_name, -1, META_STATIC);
}

/* Unmetafy and output a string.  Tokens are skipped. */

/**/
mod_export int
zputs(char const *s, FILE *stream)
{
    int c;

    while (*s) {
	if (*s == Meta)
	    c = *++s ^ 32;
	else if(itok(*s)) {
	    s++;
	    continue;
	} else
	    c = *s;
	s++;
	if (fputc(c, stream) < 0)
	    return EOF;
    }
    return 0;
}

#ifndef MULTIBYTE_SUPPORT
/* Create a visibly-represented duplicate of a string. */

/**/
mod_export char *
nicedup(char const *s, int heap)
{
    char *retstr;

    (void)sb_niceformat(s, NULL, &retstr, heap ? NICEFLAG_HEAP : 0);

    return retstr;
}
#endif

/**/
mod_export char *
nicedupstring(char const *s)
{
    return nicedup(s, 1);
}


#ifndef MULTIBYTE_SUPPORT
/* Unmetafy and output a string, displaying special characters readably. */

/**/
mod_export int
nicezputs(char const *s, FILE *stream)
{
    sb_niceformat(s, stream, NULL, 0);
    return 0;
}


/* Return the length of the visible representation of a metafied string. */

/**/
mod_export size_t
niceztrlen(char const *s)
{
    size_t l = 0;
    int c;

    while ((c = *s++)) {
	if (itok(c)) {
	    if (c <= Comma)
		c = ztokens[c - Pound];
	    else
		continue;
	}
	if (c == Meta)
	    c = *s++ ^ 32;
	l += strlen(nicechar(c));
    }
    return l;
}
#endif


/**/
#ifdef MULTIBYTE_SUPPORT
/*
 * Version of both nicezputs() and niceztrlen() for use with multibyte
 * characters.  Input is a metafied string; output is the screen width of
 * the string.
 *
 * If the FILE * is not NULL, output to that, too.
 *
 * If outstrp is not NULL, set *outstrp to a zalloc'd version of
 * the output (still metafied).
 *
 * If flags contains NICEFLAG_HEAP, use the heap for *outstrp, else
 * zalloc.
 * If flags contsins NICEFLAG_QUOTE, the output is going to be within
 * $'...', so quote "'" and "\" with a backslash.
 */

/**/
mod_export size_t
mb_niceformat(const char *s, FILE *stream, char **outstrp, int flags)
{
    size_t l = 0, newl;
    int umlen, outalloc, outleft, eol = 0;
    wchar_t c;
    char *ums, *ptr, *fmt, *outstr, *outptr;
    mbstate_t mbs;

    if (outstrp) {
	outleft = outalloc = 5 * strlen(s);
	outptr = outstr = zalloc(outalloc);
    } else {
	outleft = outalloc = 0;
	outptr = outstr = NULL;
    }

    ums = ztrdup(s);
    /*
     * is this necessary at this point? niceztrlen does this
     * but it's used in lots of places.  however, one day this may
     * be, too.
     */
    untokenize(ums);
    ptr = unmetafy(ums, &umlen);

    memset(&mbs, 0, sizeof mbs);
    while (umlen > 0) {
	size_t cnt = eol ? MB_INVALID : mbrtowc(&c, ptr, umlen, &mbs);

	switch (cnt) {
	case MB_INCOMPLETE:
	    eol = 1;
	    /* FALL THROUGH */
	case MB_INVALID:
	    /* The byte didn't convert, so output it as a \M-... sequence. */
	    fmt = nicechar_sel(*ptr, flags & NICEFLAG_QUOTE);
	    newl = strlen(fmt);
	    cnt = 1;
	    /* Get mbs out of its undefined state. */
	    memset(&mbs, 0, sizeof mbs);
	    break;
	case 0:
	    /* Careful:  converting '\0' returns 0, but a '\0' is a
	     * real character for us, so we should consume 1 byte. */
	    cnt = 1;
	    /* FALL THROUGH */
	default:
	    if (c == L'\'' && (flags & NICEFLAG_QUOTE)) {
		fmt = "\\'";
		newl = 2;
	    }
	    else if (c == L'\\' && (flags & NICEFLAG_QUOTE)) {
		fmt = "\\\\";
		newl = 2;
	    }
	    else
		fmt = wcs_nicechar_sel(c, &newl, NULL, flags & NICEFLAG_QUOTE);
	    break;
	}

	umlen -= cnt;
	ptr += cnt;
	l += newl;

	if (stream)
	    zputs(fmt, stream);
	if (outstr) {
	    /* Append to output string */
	    int outlen = strlen(fmt);
	    if (outlen >= outleft) {
		/* Reallocate to twice the length */
		int outoffset = outptr - outstr;

		outleft += outalloc;
		outalloc *= 2;
		outstr = zrealloc(outstr, outalloc);
		outptr = outstr + outoffset;
	    }
	    memcpy(outptr, fmt, outlen);
	    /* Update start position */
	    outptr += outlen;
	    /* Update available bytes */
	    outleft -= outlen;
	}
    }

    free(ums);
    if (outstrp) {
	*outptr = '\0';
	/* Use more efficient storage for returned string */
	if (flags & NICEFLAG_NODUP)
	    *outstrp = outstr;
	else {
	    *outstrp = (flags & NICEFLAG_HEAP) ? dupstring(outstr) :
		ztrdup(outstr);
	    free(outstr);
	}
    }

    return l;
}

/*
 * Return 1 if mb_niceformat() would reformat this string, else 0.
 */

/**/
mod_export int
is_mb_niceformat(const char *s)
{
    int umlen, eol = 0, ret = 0;
    wchar_t c;
    char *ums, *ptr;
    mbstate_t mbs;

    ums = ztrdup(s);
    untokenize(ums);
    ptr = unmetafy(ums, &umlen);

    memset(&mbs, 0, sizeof mbs);
    while (umlen > 0) {
	size_t cnt = eol ? MB_INVALID : mbrtowc(&c, ptr, umlen, &mbs);

	switch (cnt) {
	case MB_INCOMPLETE:
	    eol = 1;
	    /* FALL THROUGH */
	case MB_INVALID:
	    /* The byte didn't convert, so output it as a \M-... sequence. */
	    if (is_nicechar(*ptr))  {
		ret = 1;
		break;
	    }
	    cnt = 1;
	    /* Get mbs out of its undefined state. */
	    memset(&mbs, 0, sizeof mbs);
	    break;
	case 0:
	    /* Careful:  converting '\0' returns 0, but a '\0' is a
	     * real character for us, so we should consume 1 byte. */
	    cnt = 1;
	    /* FALL THROUGH */
	default:
	    if (is_wcs_nicechar(c))
		ret = 1;
	    break;
	}

	if (ret)
	    break;

	umlen -= cnt;
	ptr += cnt;
    }

    free(ums);

    return ret;
}

/* ztrdup multibyte string with nice formatting */

/**/
mod_export char *
nicedup(const char *s, int heap)
{
    char *retstr;

    (void)mb_niceformat(s, NULL, &retstr, heap ? NICEFLAG_HEAP : 0);

    return retstr;
}


/*
 * The guts of mb_metacharlenconv().  This version assumes we are
 * processing a true multibyte character string without tokens, and
 * takes the shift state as an argument.
 */

/**/
mod_export int
mb_metacharlenconv_r(const char *s, wint_t *wcp, mbstate_t *mbsp)
{
    size_t ret = MB_INVALID;
    char inchar;
    const char *ptr;
    wchar_t wc;

    if (STOUC(*s) <= 0x7f) {
	if (wcp)
	    *wcp = (wint_t)*s;
	return 1;
    }

    for (ptr = s; *ptr; ) {
	if (*ptr == Meta) {
	    inchar = *++ptr ^ 32;
	    DPUTS(!*ptr,
		  "BUG: unexpected end of string in mb_metacharlen()\n");
	} else if (imeta(*ptr)) {
	    /*
	     * As this is metafied input, this is a token --- this
	     * can't be a part of the string.  It might be
	     * something on the end of an unbracketed parameter
	     * reference, for example.
	     */
	    break;
	} else
	    inchar = *ptr;
	ptr++;
	ret = mbrtowc(&wc, &inchar, 1, mbsp);

	if (ret == MB_INVALID)
	    break;
	if (ret == MB_INCOMPLETE)
	    continue;
	if (wcp)
	    *wcp = wc;
	return ptr - s;
    }

    if (wcp)
	*wcp = WEOF;
    /* No valid multibyte sequence */
    memset(mbsp, 0, sizeof(*mbsp));
    if (ptr > s) {
	return 1 + (*s == Meta);	/* Treat as single byte character */
    } else
	return 0;		/* Probably shouldn't happen */
}

/*
 * Length of metafied string s which contains the next multibyte
 * character; single (possibly metafied) character if string is not null
 * but character is not valid (e.g. possibly incomplete at end of string).
 * Returned value is guaranteed not to reach beyond the end of the
 * string (assuming correct metafication).
 *
 * If wcp is not NULL, the converted wide character is stored there.
 * If no conversion could be done WEOF is used.
 */

/**/
mod_export int
mb_metacharlenconv(const char *s, wint_t *wcp)
{
    if (!isset(MULTIBYTE) || STOUC(*s) <= 0x7f) {
	/* treat as single byte, possibly metafied */
	if (wcp)
	    *wcp = (wint_t)(*s == Meta ? s[1] ^ 32 : *s);
	return 1 + (*s == Meta);
    }
    /*
     * We have to handle tokens here, since we may be looking
     * through a tokenized input.  Obviously this isn't
     * a valid multibyte character, so just return WEOF
     * and let the caller handle it as a single character.
     *
     * TODO: I've a sneaking suspicion we could do more here
     * to prevent the caller always needing to handle invalid
     * characters specially, but sometimes it may need to know.
     */
    if (itok(*s)) {
	if (wcp)
	    *wcp = WEOF;
	return 1;
    }

    return mb_metacharlenconv_r(s, wcp, &mb_shiftstate);
}

/*
 * Total number of multibyte characters in metafied string s.
 * Same answer as iterating mb_metacharlen() and counting calls
 * until end of string.
 *
 * If width is 1, return total character width rather than number.
 * If width is greater than 1, return 1 if character has non-zero width,
 * else 0.
 *
 * Ends if either *ptr is '\0', the normal case (eptr may be NULL for
 * this), or ptr is eptr (i.e.  *eptr is where the null would be if null
 * terminated) for strings not delimited by nulls --- note these are
 * still metafied.
 */

/**/
mod_export int
mb_metastrlenend(char *ptr, int width, char *eptr)
{
    char inchar, *laststart;
    size_t ret;
    wchar_t wc;
    int num, num_in_char, complete;

    if (!isset(MULTIBYTE) || MB_CUR_MAX == 1)
	return eptr ? (int)(eptr - ptr) : ztrlen(ptr);

    laststart = ptr;
    ret = MB_INVALID;
    num = num_in_char = 0;
    complete = 1;

    memset(&mb_shiftstate, 0, sizeof(mb_shiftstate));
    while (*ptr && !(eptr && ptr >= eptr)) {
	if (*ptr == Meta)
	    inchar = *++ptr ^ 32;
	else
	    inchar = *ptr;
	ptr++;

	if (complete && STOUC(inchar) <= STOUC(0x7f)) {
	    /*
	     * We rely on 7-bit US-ASCII as a subset, so skip
	     * multibyte handling if we have such a character.
	     */
	    num++;
	    laststart = ptr;
	    num_in_char = 0;
	    continue;
	}

	ret = mbrtowc(&wc, &inchar, 1, &mb_shiftstate);

	if (ret == MB_INCOMPLETE) {
	    /*
	     * "num_in_char" is only used for incomplete characters.
	     * The assumption is that we will output all trailing octets
	     * that form part of an incomplete character as a single
	     * character (of single width) if we don't get a complete
	     * character.  This is purely pragmatic --- I'm not aware
	     * of a standard way of dealing with incomplete characters.
	     *
	     * If we do get a complete character, num_in_char
	     * becomes irrelevant and is set to zero
	     *
	     * This is in contrast to "num" which counts the characters
	     * or widths in complete characters.  The two are summed,
	     * so we don't count characters twice.
	     */
	    num_in_char++;
	    complete = 0;
	} else {
	    if (ret == MB_INVALID) {
		/* Reset, treat as single character */
		memset(&mb_shiftstate, 0, sizeof(mb_shiftstate));
		ptr = laststart + (*laststart == Meta) + 1;
		num++;
	    } else if (width) {
		/*
		 * Returns -1 if not a printable character.  We
		 * turn this into 0.
		 */
		int wcw = WCWIDTH(wc);
		if (wcw > 0) {
		    if (width == 1)
			num += wcw;
		    else
			num++;
		}
	    } else
		num++;
	    laststart = ptr;
	    num_in_char = 0;
	    complete = 1;
	}
    }

    /* If incomplete, treat remainder as trailing single character */
    return num + (num_in_char ? 1 : 0);
}

/*
 * The equivalent of mb_metacharlenconv_r() for
 * strings that aren't metafied and hence have
 * explicit lengths.
 */

/**/
mod_export int
mb_charlenconv_r(const char *s, int slen, wint_t *wcp, mbstate_t *mbsp)
{
    size_t ret = MB_INVALID;
    char inchar;
    const char *ptr;
    wchar_t wc;

    if (slen && STOUC(*s) <= 0x7f) {
	if (wcp)
	    *wcp = (wint_t)*s;
	return 1;
    }

    for (ptr = s; slen;  ) {
	inchar = *ptr;
	ptr++;
	slen--;
	ret = mbrtowc(&wc, &inchar, 1, mbsp);

	if (ret == MB_INVALID)
	    break;
	if (ret == MB_INCOMPLETE)
	    continue;
	if (wcp)
	    *wcp = wc;
	return ptr - s;
    }

    if (wcp)
	*wcp = WEOF;
    /* No valid multibyte sequence */
    memset(mbsp, 0, sizeof(*mbsp));
    if (ptr > s) {
	return 1;	/* Treat as single byte character */
    } else
	return 0;		/* Probably shouldn't happen */
}

/*
 * The equivalent of mb_metacharlenconv() for
 * strings that aren't metafied and hence have
 * explicit lengths;
 */

/**/
mod_export int
mb_charlenconv(const char *s, int slen, wint_t *wcp)
{
    if (!isset(MULTIBYTE) || STOUC(*s) <= 0x7f) {
	if (wcp)
	    *wcp = (wint_t)*s;
	return 1;
    }

    return mb_charlenconv_r(s, slen, wcp, &mb_shiftstate);
}

/**/
#else /* MULTIBYTE_SUPPORT */

/* Simple replacement for mb_metacharlenconv */

/**/
mod_export int
metacharlenconv(const char *x, int *c)
{
    /*
     * Here we don't use STOUC() on the chars since they
     * may be compared against other chars and this will fail
     * if chars are signed and the high bit is set.
     */
    if (*x == Meta) {
	if (c)
	    *c = x[1] ^ 32;
	return 2;
    }
    if (c)
	*c = (char)*x;
    return 1;
}

/* Simple replacement for mb_charlenconv */

/**/
mod_export int
charlenconv(const char *x, int len, int *c)
{
    if (!len) {
	if (c)
	    *c = '\0';
	return 0;
    }

    if (c)
	*c = (char)*x;
    return 1;
}

/*
 * Non-multibyte version of mb_niceformat() above.  Same basic interface.
 */

/**/
mod_export size_t
sb_niceformat(const char *s, FILE *stream, char **outstrp, int flags)
{
    size_t l = 0, newl;
    int umlen, outalloc, outleft;
    char *ums, *ptr, *eptr, *fmt, *outstr, *outptr;

    if (outstrp) {
	outleft = outalloc = 2 * strlen(s);
	outptr = outstr = zalloc(outalloc);
    } else {
	outleft = outalloc = 0;
	outptr = outstr = NULL;
    }

    ums = ztrdup(s);
    /*
     * is this necessary at this point? niceztrlen does this
     * but it's used in lots of places.  however, one day this may
     * be, too.
     */
    untokenize(ums);
    ptr = unmetafy(ums, &umlen);
    eptr = ptr + umlen;

    while (ptr < eptr) {
	int c = STOUC(*ptr);
	if (c == '\'' && (flags & NICEFLAG_QUOTE)) {
	    fmt = "\\'";
	    newl = 2;
	}
	else if (c == '\\' && (flags & NICEFLAG_QUOTE)) {
	    fmt = "\\\\";
	    newl = 2;
	}
	else {
	    fmt = nicechar_sel(c, flags & NICEFLAG_QUOTE);
	    newl = 1;
	}

	++ptr;
	l += newl;

	if (stream)
	    zputs(fmt, stream);
	if (outstr) {
	    /* Append to output string */
	    int outlen = strlen(fmt);
	    if (outlen >= outleft) {
		/* Reallocate to twice the length */
		int outoffset = outptr - outstr;

		outleft += outalloc;
		outalloc *= 2;
		outstr = zrealloc(outstr, outalloc);
		outptr = outstr + outoffset;
	    }
	    memcpy(outptr, fmt, outlen);
	    /* Update start position */
	    outptr += outlen;
	    /* Update available bytes */
	    outleft -= outlen;
	}
    }

    free(ums);
    if (outstrp) {
	*outptr = '\0';
	/* Use more efficient storage for returned string */
	if (flags & NICEFLAG_NODUP)
	    *outstrp = outstr;
	else {
	    *outstrp = (flags & NICEFLAG_HEAP) ? dupstring(outstr) :
		ztrdup(outstr);
	    free(outstr);
	}
    }

    return l;
}

/*
 * Return 1 if sb_niceformat() would reformat this string, else 0.
 */

/**/
mod_export int
is_sb_niceformat(const char *s)
{
    int umlen, ret = 0;
    char *ums, *ptr, *eptr;

    ums = ztrdup(s);
    untokenize(ums);
    ptr = unmetafy(ums, &umlen);
    eptr = ptr + umlen;

    while (ptr < eptr) {
	if (is_nicechar(*ptr))  {
	    ret = 1;
	    break;
	}
	++ptr;
    }

    free(ums);

    return ret;
}

/**/
#endif /* MULTIBYTE_SUPPORT */

/*
 * Expand tabs to given width, with given starting position on line.
 * len is length of unmetafied string in bytes.
 * Output to fout.
 * Return the end position on the line, i.e. if this is 0 modulo width
 * the next character is aligned with a tab stop.
 *
 * If all is set, all tabs are expanded, else only leading tabs.
 */

/**/
mod_export int
zexpandtabs(const char *s, int len, int width, int startpos, FILE *fout,
	    int all)
{
    int at_start = 1;

#ifdef MULTIBYTE_SUPPORT
    mbstate_t mbs;
    size_t ret;
    wchar_t wc;

    memset(&mbs, 0, sizeof(mbs));
#endif

    while (len) {
	if (*s == '\t') {
	    if (all || at_start) {
		s++;
		len--;
		if (width <= 0 || !(startpos % width)) {
		    /* always output at least one space */
		    fputc(' ', fout);
		    startpos++;
		}
		if (width <= 0)
		    continue;	/* paranoia */
		while (startpos % width) {
		    fputc(' ', fout);
		    startpos++;
		}
	    } else {
		/*
		 * Leave tab alone.
		 * Guess width to apply... we might get this wrong.
		 * This is only needed if there's a following string
		 * that needs tabs expanding, which is unusual.
		 */
		startpos += width - startpos % width;
		s++;
		len--;
		fputc('\t', fout);
	    }
	    continue;
	} else if (*s == '\n' || *s == '\r') {
	    fputc(*s, fout);
	    s++;
	    len--;
	    startpos = 0;
	    at_start = 1;
	    continue;
	}

	at_start = 0;
#ifdef MULTIBYTE_SUPPORT
	if (isset(MULTIBYTE)) {
	    const char *sstart = s;
	    ret = mbrtowc(&wc, s, len, &mbs);
	    if (ret == MB_INVALID) {
		/* Assume single character per character */
		memset(&mbs, 0, sizeof(mbs));
		s++;
		len--;
	    } else if (ret == MB_INCOMPLETE ||
		/* incomplete at end --- assume likewise, best we've got */
	               ret == 0) {
		/* NUL character returns 0, which would loop infinitely, so advance
		 * one byte in this case too */
		s++;
		len--;
	    } else {
		s += ret;
		len -= (int)ret;
	    }
	    if (ret == MB_INVALID || ret == MB_INCOMPLETE) {
		startpos++;
	    } else {
		int wcw = WCWIDTH(wc);
		if (wcw > 0)	/* paranoia */
		    startpos += wcw;
	    }
	    fwrite(sstart, s - sstart, 1, fout);

	    continue;
	}
#endif /* MULTIBYTE_SUPPORT */
	fputc(*s, fout);
	s++;
	len--;
	startpos++;
    }

    return startpos;
}

/* check for special characters in the string */

/**/
mod_export int
hasspecial(char const *s)
{
    for (; *s; s++) {
	if (ispecial(*s == Meta ? *++s ^ 32 : *s))
	    return 1;
    }
    return 0;
}


static char *
addunprintable(char *v, const char *u, const char *uend)
{
    for (; u < uend; u++) {
	/*
	 * Just do this byte by byte; there's no great
	 * advantage in being clever with multibyte
	 * characters if we don't think they're printable.
	 */
	int c;
	if (*u == Meta)
	    c = STOUC(*++u ^ 32);
	else
	    c = STOUC(*u);
	switch (c) {
	case '\0':
	    *v++ = '\\';
	    *v++ = '0';
	    if ('0' <= u[1] && u[1] <= '7') {
		*v++ = '0';
		*v++ = '0';
	    }
	    break;

	case '\007': *v++ = '\\'; *v++ = 'a'; break;
	case '\b': *v++ = '\\'; *v++ = 'b'; break;
	case '\f': *v++ = '\\'; *v++ = 'f'; break;
	case '\n': *v++ = '\\'; *v++ = 'n'; break;
	case '\r': *v++ = '\\'; *v++ = 'r'; break;
	case '\t': *v++ = '\\'; *v++ = 't'; break;
	case '\v': *v++ = '\\'; *v++ = 'v'; break;

	default:
	    *v++ = '\\';
	    *v++ = '0' + ((c >> 6) & 7);
	    *v++ = '0' + ((c >> 3) & 7);
	    *v++ = '0' + (c & 7);
	    break;
	}
    }

    return v;
}

/*
 * Quote the string s and return the result as a string from the heap.
 *
 * The last argument is a QT_ value defined in zsh.h other than QT_NONE.
 *
 * Most quote styles other than backslash assume the quotes are to
 * be added outside quotestring().  QT_SINGLE_OPTIONAL is different:
 * the single quotes are only added where necessary, so the
 * whole expression is handled here.
 *
 * The string may be metafied and contain tokens.
 */

/**/
mod_export char *
quotestring(const char *s, int instring)
{
    const char *u;
    char *v;
    int alloclen;
    char *buf;
    int shownull = 0;
    /*
     * quotesub is used with QT_SINGLE_OPTIONAL.
     * quotesub = 0:  mechanism not active
     * quotesub = 1:  mechanism pending, no "'" yet;
     *                needs adding at quotestart.
     * quotesub = 2:  mechanism active, added opening "'"; need
     *                closing "'".
     */
    int quotesub = 0, slen;
    char *quotestart;
    convchar_t cc;
    const char *uend;

    slen = strlen(s);
    switch (instring)
    {
    case QT_BACKSLASH_SHOWNULL:
	shownull = 1;
	instring = QT_BACKSLASH;
	/*FALLTHROUGH*/
    case QT_BACKSLASH:
	/*
	 * With QT_BACKSLASH we may need to use $'\300' stuff.
	 * Keep memory usage within limits by allocating temporary
	 * storage and using heap for correct size at end.
	 */
	alloclen = slen * 7 + 1;
	break;

    case QT_BACKSLASH_PATTERN:
	alloclen = slen * 2  + 1;
	break;

    case QT_SINGLE_OPTIONAL:
	/*
	 * Here, we may need to add single quotes.
	 * Always show empty strings.
	 */
	alloclen = slen * 4 + 3;
	quotesub = shownull = 1;
	break;

    default:
	alloclen = slen * 4 + 1;
	break;
    }
    if (!*s && shownull)
	alloclen += 2;	/* for '' */

    quotestart = v = buf = zshcalloc(alloclen);

    DPUTS(instring < QT_BACKSLASH || instring == QT_BACKTICK ||
	  instring > QT_BACKSLASH_PATTERN,
	  "BUG: bad quote type in quotestring");
    u = s;
    if (instring == QT_DOLLARS) {
	/*
	 * The only way to get Nularg here is when
	 * it is placeholding for the empty string?
	 */
	if (inull(*u))
	    u++;
	/*
	 * As we test for printability here we need to be able
	 * to look for multibyte characters.
	 */
	MB_METACHARINIT();
	while (*u) {
	    uend = u + MB_METACHARLENCONV(u, &cc);

	    if (
#ifdef MULTIBYTE_SUPPORT
		cc != WEOF &&
#endif
		WC_ISPRINT(cc)) {
		switch (cc) {
		case ZWC('\\'):
		case ZWC('\''):
		    *v++ = '\\';
		    break;

		default:
		    if (isset(BANGHIST) && cc == (wchar_t)bangchar)
			*v++ = '\\';
		    break;
		}
		while (u < uend)
		    *v++ = *u++;
	    } else {
		/* Not printable */
		v = addunprintable(v, u, uend);
		u = uend;
	    }
	}
    } else if (instring == QT_BACKSLASH_PATTERN) {
	while (*u) {
	    if (ipattern(*u))
		*v++ = '\\';
	    *v++ = *u++;
	}
    } else {
	if (shownull) {
	    /* We can't show an empty string with just backslash quoting. */
	    if (!*u) {
		*v++ = '\'';
		*v++ = '\'';
	    }
	}
	/*
	 * Here there are syntactic special characters, so
	 * we start by going through bytewise.
	 */
	while (*u) {
	    int dobackslash = 0;
	    if (*u == Tick || *u == Qtick) {
		char c = *u++;

		*v++ = c;
		while (*u && *u != c)
		    *v++ = *u++;
		*v++ = c;
		if (*u)
		    u++;
		continue;
	    } else if ((*u == Qstring || *u == '$') && u[1] == '\'' &&
		       instring == QT_DOUBLE) {
		/*
		 * We don't need to quote $'...' inside a double-quoted
		 * string.  This is largely cosmetic; it looks neater
		 * if we don't but it doesn't do any harm since the
		 * \ is stripped.
		 */
		*v++ = *u++;
	    } else if ((*u == String || *u == Qstring) &&
		       (u[1] == Inpar || u[1] == Inbrack || u[1] == Inbrace)) {
		char c = (u[1] == Inpar ? Outpar : (u[1] == Inbrace ?
						    Outbrace : Outbrack));
		char beg = *u;
		int level = 0;

		*v++ = *u++;
		*v++ = *u++;
		while (*u && (*u != c || level)) {
		    if (*u == beg)
			level++;
		    else if (*u == c)
			level--;
		    *v++ = *u++;
		}
		if (*u)
		    *v++ = *u++;
		continue;
	    }
	    else if (ispecial(*u) &&
		     ((*u != '=' && *u != '~') ||
		      u == s ||
		      (isset(MAGICEQUALSUBST) &&
		       (u[-1] == '=' || u[-1] == ':')) ||
		      (*u == '~' && isset(EXTENDEDGLOB))) &&
		     (instring == QT_BACKSLASH ||
		      instring == QT_SINGLE_OPTIONAL ||
		      (isset(BANGHIST) && *u == (char)bangchar &&
		       instring != QT_SINGLE) ||
		      (instring == QT_DOUBLE &&
		       (*u == '$' || *u == '`' || *u == '\"' || *u == '\\')) ||
		      (instring == QT_SINGLE && *u == '\''))) {
		if (instring == QT_SINGLE_OPTIONAL) {
		    if (quotesub == 1) {
			/*
			 * We haven't yet had to quote at the start.
			 */
			if (*u == '\'') {
			    /*
			     * We don't need to.
			     */
			    *v++ = '\\';
			} else {
			    /*
			     * It's now time to add quotes.
			     */
			    if (v > quotestart)
			    {
				char *addq;

				for (addq = v; addq > quotestart; addq--)
				    *addq = addq[-1];
			    }
			    *quotestart = '\'';
			    v++;
			    quotesub = 2;
			}
			*v++ = *u++;
			/*
			 * Next place to start quotes is here.
			 */
			quotestart = v;
		    } else if (*u == '\'') {
			if (unset(RCQUOTES)) {
			    *v++ = '\'';
			    *v++ = '\\';
			    *v++ = '\'';
			    /* Don't restart quotes unless we need them */
			    quotesub = 1;
			    quotestart = v;
			} else {
			    /* simplest just to use '' always */
			    *v++ = '\'';
			    *v++ = '\'';
			}
			/* dealt with */
			u++;
		    } else {
			/* else already quoting, just add */
			*v++ = *u++;
		    }
		    continue;
		} else if (*u == '\n' ||
			   (instring == QT_SINGLE && *u == '\'')) {
		    if (*u == '\n') {
			*v++ = '$';
			*v++ = '\'';
			*v++ = '\\';
			*v++ = 'n';
			*v++ = '\'';
		    } else if (unset(RCQUOTES)) {
			*v++ = '\'';
			if (*u == '\'')
			    *v++ = '\\';
			*v++ = *u;
			*v++ = '\'';
		    } else
			*v++ = '\'', *v++ = '\'';
		    u++;
		    continue;
		} else {
		    /*
		     * We'll need a backslash, but don't add it
		     * yet since if the character isn't printable
		     * we'll have to upgrade it to $'...'.
		     */
		    dobackslash = 1;
		}
	    }

	    if (itok(*u) || instring != QT_BACKSLASH) {
		/* Needs to be passed straight through. */
		if (dobackslash)
		    *v++ = '\\';
		if (*u == Inparmath) {
		    /*
		     * Already syntactically quoted: don't
		     * add more.
		     */
		    int inmath = 1;
		    *v++ = *u++;
		    for (;;) {
			char uc = *u;
			*v++ = *u++;
			if (uc == '\0')
			    break;
			else if (uc == Outparmath && !--inmath)
			    break;
			else if (uc == Inparmath)
			    ++inmath;
		    }
		} else
		    *v++ = *u++;
		continue;
	    }

	    /*
	     * Now check if the output is unprintable in the
	     * current character set.
	     */
	    uend = u + MB_METACHARLENCONV(u, &cc);
	    if (
#ifdef MULTIBYTE_SUPPORT
		cc != WEOF &&
#endif
		WC_ISPRINT(cc)) {
		if (dobackslash)
		    *v++ = '\\';
		while (u < uend) {
		    if (*u == Meta)
			*v++ = *u++;
		    *v++ = *u++;
		}
	    } else {
		/* Not printable */
		*v++ = '$';
		*v++ = '\'';
		v = addunprintable(v, u, uend);
		*v++ = '\'';
		u = uend;
	    }
	}
    }
    if (quotesub == 2)
	*v++ = '\'';
    *v = '\0';

    v = dupstring(buf);
    zfree(buf, alloclen);
    return v;
}

/*
 * Unmetafy and output a string, quoted if it contains special
 * characters.
 *
 * If stream is NULL, return the same output with any allocation on the
 * heap.
 */

/**/
mod_export char *
quotedzputs(char const *s, FILE *stream)
{
    int inquote = 0, c;
    char *outstr, *ptr;

    /* check for empty string */
    if(!*s) {
	if (!stream)
	    return dupstring("''");
	fputs("''", stream);
	return NULL;
    }

#ifdef MULTIBYTE_SUPPORT
    if (is_mb_niceformat(s)) {
	if (stream) {
	    fputs("$'", stream);
	    mb_niceformat(s, stream, NULL, NICEFLAG_QUOTE);
	    fputc('\'', stream);
	    return NULL;
	} else {
	    char *substr;
	    mb_niceformat(s, NULL, &substr, NICEFLAG_QUOTE|NICEFLAG_NODUP);
	    outstr = (char *)zhalloc(4 + strlen(substr));
	    sprintf(outstr, "$'%s'", substr);
	    free(substr);
	    return outstr;
	}
    }
#else
    if (is_sb_niceformat(s)){
	if (stream) {
	    fputs("$'", stream);
	    sb_niceformat(s, stream, NULL, NICEFLAG_QUOTE);
	    fputc('\'', stream);
	    return NULL;
	} else {
	    char *substr;
	    sb_niceformat(s, NULL, &substr, NICEFLAG_QUOTE|NICEFLAG_NODUP);
	    outstr = (char *)zhalloc(4 + strlen(substr));
	    sprintf(outstr, "$'%s'", substr);
	    free(substr);
	    return outstr;
	}
    }
#endif /* MULTIBYTE_SUPPORT */

    if (!hasspecial(s)) {
	if (stream) {
	    zputs(s, stream);
	    return NULL;
	} else {
	    return dupstring(s);
	}
    }

    if (!stream) {
	const char *cptr;
	int l = strlen(s) + 2;
	for (cptr = s; *cptr; cptr++) {
	    if (*cptr == Meta)
		cptr++;
	    else if (*cptr == '\'')
		l += isset(RCQUOTES) ? 1 : 3;
	}
	ptr = outstr = zhalloc(l + 1);
    } else {
	ptr = outstr = NULL;
    }
    if (isset(RCQUOTES)) {
	/* use rc-style quotes-within-quotes for the whole string */
	if (stream) {
	    if (fputc('\'', stream) < 0)
		return NULL;
	} else
	    *ptr++ = '\'';
	while(*s) {
	    if (*s == Dash)
		c = '-';
	    else if (*s == Meta)
		c = *++s ^ 32;
	    else
		c = *s;
	    s++;
	    if (c == '\'') {
		if (stream) {
		    if (fputc('\'', stream) < 0)
			return NULL;
		} else
		    *ptr++ = '\'';
	    } else if (c == '\n' && isset(CSHJUNKIEQUOTES)) {
		if (stream) {
		    if (fputc('\\', stream) < 0)
			return NULL;
		} else
		    *ptr++ = '\\';
	    }
	    if (stream) {
		if (fputc(c, stream) < 0)
		    return NULL;
	    } else {
		if (imeta(c)) {
		    *ptr++ = Meta;
		    *ptr++ = c ^ 32;
		} else
		    *ptr++ = c;
	    }
	}
	if (stream) {
	    if (fputc('\'', stream) < 0)
		return NULL;
	} else
	    *ptr++ = '\'';
    } else {
	/* use Bourne-style quoting, avoiding empty quoted strings */
	while (*s) {
	    if (*s == Dash)
		c = '-';
	    else if (*s == Meta)
		c = *++s ^ 32;
	    else
		c = *s;
	    s++;
	    if (c == '\'') {
		if (inquote) {
		    if (stream) {
			if (putc('\'', stream) < 0)
			    return NULL;
		    } else
			*ptr++ = '\'';
		    inquote=0;
		}
		if (stream) {
		    if (fputs("\\'", stream) < 0)
			return NULL;
		} else {
		    *ptr++ = '\\';
		    *ptr++ = '\'';
		}
	    } else {
		if (!inquote) {
		    if (stream) {
			if (fputc('\'', stream) < 0)
			    return NULL;
		    } else
			*ptr++ = '\'';
		    inquote=1;
		}
		if (c == '\n' && isset(CSHJUNKIEQUOTES)) {
		    if (stream) {
			if (fputc('\\', stream) < 0)
			    return NULL;
		    } else
			*ptr++ = '\\';
		}
		if (stream) {
		    if (fputc(c, stream) < 0)
			return NULL;
		} else {
		    if (imeta(c)) {
			*ptr++ = Meta;
			*ptr++ = c ^ 32;
		    } else
			*ptr++ = c;
		}
	    }
	}
	if (inquote) {
	    if (stream) {
		if (fputc('\'', stream) < 0)
		    return NULL;
	    } else
		*ptr++ = '\'';
	}
    }
    if (!stream)
	*ptr++ = '\0';

    return outstr;
}

/* Double-quote a metafied string. */

/**/
mod_export char *
dquotedztrdup(char const *s)
{
    int len = strlen(s) * 4 + 2;
    char *buf = zalloc(len);
    char *p = buf, *ret;

    if(isset(CSHJUNKIEQUOTES)) {
	int inquote = 0;

	while(*s) {
	    int c = *s++;

	    if (c == Meta)
		c = *s++ ^ 32;
	    switch(c) {
		case '"':
		case '$':
		case '`':
		    if(inquote) {
			*p++ = '"';
			inquote = 0;
		    }
		    *p++ = '\\';
		    *p++ = c;
		    break;
		default:
		    if(!inquote) {
			*p++ = '"';
			inquote = 1;
		    }
		    if(c == '\n')
			*p++ = '\\';
		    *p++ = c;
		    break;
	    }
	}
	if (inquote)
	    *p++ = '"';
    } else {
	int pending = 0;

	*p++ = '"';
	while(*s) {
	    int c = *s++;

	    if (c == Meta)
		c = *s++ ^ 32;
	    switch(c) {
		case '\\':
		    if(pending)
			*p++ = '\\';
		    *p++ = '\\';
		    pending = 1;
		    break;
		case '"':
		case '$':
		case '`':
		    if(pending)
			*p++ = '\\';
		    *p++ = '\\';
		    /* FALL THROUGH */
		default:
		    *p++ = c;
		    pending = 0;
		    break;
	    }
	}
	if(pending)
	    *p++ = '\\';
	*p++ = '"';
    }
    ret = metafy(buf, p - buf, META_DUP);
    zfree(buf, len);
    return ret;
}

/* Unmetafy and output a string, double quoting it in its entirety. */

#if 0 /**/
int
dquotedzputs(char const *s, FILE *stream)
{
    char *d = dquotedztrdup(s);
    int ret = zputs(d, stream);

    zsfree(d);
    return ret;
}
#endif

# if defined(HAVE_NL_LANGINFO) && defined(CODESET) && !defined(__STDC_ISO_10646__)
/* Convert a character from UCS4 encoding to UTF-8 */

static size_t
ucs4toutf8(char *dest, unsigned int wval)
{
    size_t len;

    if (wval < 0x80)
      len = 1;
    else if (wval < 0x800)
      len = 2;
    else if (wval < 0x10000)
      len = 3;
    else if (wval < 0x200000)
      len = 4;
    else if (wval < 0x4000000)
      len = 5;
    else
      len = 6;

    switch (len) { /* falls through except to the last case */
    case 6: dest[5] = (wval & 0x3f) | 0x80; wval >>= 6;
    case 5: dest[4] = (wval & 0x3f) | 0x80; wval >>= 6;
    case 4: dest[3] = (wval & 0x3f) | 0x80; wval >>= 6;
    case 3: dest[2] = (wval & 0x3f) | 0x80; wval >>= 6;
    case 2: dest[1] = (wval & 0x3f) | 0x80; wval >>= 6;
	*dest = wval | ((0xfc << (6 - len)) & 0xfc);
	break;
    case 1: *dest = wval;
    }

    return len;
}
#endif


/*
 * The following only occurs once or twice in the code, but in different
 * places depending how character set conversion is implemented.
 */
#define CHARSET_FAILED()		      \
    if (how & GETKEY_DOLLAR_QUOTE) {	      \
	while ((*tdest++ = *++s)) {	      \
	    if (how & GETKEY_UPDATE_OFFSET) { \
		if (s - sstart > *misc)	      \
		    (*misc)++;		      \
	    }				      \
	    if (*s == Snull) {		      \
		*len = (s - sstart) + 1;      \
		*tdest = '\0';		      \
		return buf;		      \
	    }				      \
	}				      \
	*len = tdest - buf;		      \
	return buf;			      \
    }					      \
    *t = '\0';				      \
    *len = t - buf;			      \
    return buf

/*
 * Decode a key string, turning it into the literal characters.
 * The value returned is a newly allocated string from the heap.
 *
 * The length is returned in *len.  This is usually the length of
 * the final unmetafied string.  The exception is the case of
 * a complete GETKEY_DOLLAR_QUOTE conversion where *len is the
 * length of the input string which has been used (up to and including
 * the terminating single quote); as the final string is metafied and
 * NULL-terminated its length is not required.  If both GETKEY_DOLLAR_QUOTE
 * and GETKEY_UPDATE_OFFSET are present in "how", the string is not
 * expected to be terminated (this is used in completion to parse
 * a partial $'...'-quoted string) and the length passed back is
 * that of the converted string.  Note in both cases that this is a length
 * in bytes (i.e. the same as given by a raw pointer difference), not
 * characters, which may occupy multiple bytes.
 *
 * how is a set of bits from the GETKEY_ values defined in zsh.h;
 * not all combinations of bits are useful.  Callers will typically
 * use one of the GETKEYS_ values which define sets of bits.
 * Note, for example that:
 * - GETKEY_SINGLE_CHAR must not be combined with GETKEY_DOLLAR_QUOTE.
 * - GETKEY_UPDATE_OFFSET is only allowed if GETKEY_DOLLAR_QUOTE is
 *   also present.
 *
 * *misc is used for various purposes:
 * - If GETKEY_BACKSLASH_MINUS is set, it indicates the presence
 *   of \- in the input.
 * - If GETKEY_BACKSLASH_C is set, it indicates the presence
 *   of \c in the input.
 * - If GETKEY_UPDATE_OFFSET is set, it is set on input to some
 *   mystical completion offset and is updated to a new offset based
 *   on the converted characters.  All Hail the Completion System
 *   [makes the mystic completion system runic sign in the air].
 *
 * The return value is unmetafied unless GETKEY_DOLLAR_QUOTE is
 * in use.
 *
 * If GETKEY_SINGLE_CHAR is set in how, a next character in the given
 * string is parsed, and the character code for it is returned in misc.
 * The return value of the function is a pointer to the byte in the
 * given string from where the next parsing should start. If the next
 * character can't be found then NULL is returned.
 * CAUTION: Currently, GETKEY_SINGLE_CHAR can be used only via
 *          GETKEYS_MATH. Other use of it may cause trouble.
 */

/**/
mod_export char *
getkeystring(char *s, int *len, int how, int *misc)
{
    char *buf = NULL, tmp[1];
    char *t, *tdest = NULL, *u = NULL, *sstart = s, *tbuf = NULL;
    char svchar = '\0';
    int meta = 0, control = 0, ignoring = 0;
    int i;
#if defined(HAVE_WCHAR_H) && defined(HAVE_WCTOMB) && defined(__STDC_ISO_10646__)
    wint_t wval;
    int count;
#else
    unsigned int wval;
# if defined(HAVE_NL_LANGINFO) && defined(CODESET)
#  if defined(HAVE_ICONV)
    iconv_t cd;
    char inbuf[4];
    size_t inbytes, outbytes;
#  endif
    size_t count;
# endif
#endif

    DPUTS((how & GETKEY_UPDATE_OFFSET) &&
	  (how & ~(GETKEYS_DOLLARS_QUOTE|GETKEY_UPDATE_OFFSET)),
	  "BUG: offset updating in getkeystring only supported with $'.");
    DPUTS((how & (GETKEY_DOLLAR_QUOTE|GETKEY_SINGLE_CHAR)) ==
	  (GETKEY_DOLLAR_QUOTE|GETKEY_SINGLE_CHAR),
	  "BUG: incompatible options in getkeystring");
    DPUTS((how & GETKEY_SINGLE_CHAR) && (how != GETKEYS_MATH),
	  "BUG: unsupported options in getkeystring");

    if (how & GETKEY_SINGLE_CHAR)
	t = tmp;
    else {
	/* Length including terminating NULL */
	int maxlen = 1;
	/*
	 * We're not necessarily guaranteed the output string will
	 * be no longer than the input with \u and \U when output
	 * characters need to be metafied.  As this is the only
	 * case where the string can get longer (?I think),
	 * include it in the allocation length here but don't
	 * bother taking account of other factors.
	 */
	for (t = s; *t; t++) {
	    if (*t == '\\') {
		if (!t[1]) {
		    maxlen++;
		    break;
		}
		if (t[1] == 'u' || t[1] == 'U')
		    maxlen += MB_CUR_MAX * 2;
		else
		    maxlen += 2;
		/* skip the backslash and the following character */
		t++;
	    } else
		maxlen++;
	}
	if (how & GETKEY_DOLLAR_QUOTE) {
	    /*
	     * We're going to unmetafy into a new string, but
	     * to get a proper metafied input we're going to metafy
	     * into an intermediate buffer.  This is necessary if we have
	     * \u and \U's with multiple metafied bytes.  We can't
	     * simply remetafy the entire string because there may
	     * be tokens (indeed, we know there are lexical nulls floating
	     * around), so we have to be aware character by character
	     * what we are converting.
	     *
	     * In this case, buf is the final buffer (as usual),
	     * but t points into a temporary buffer that just has
	     * to be long enough to hold the result of one escape
	     * code transformation.  We count this is a full multibyte
	     * character (MB_CUR_MAX) with every character metafied
	     * (*2) plus a little bit of fuzz (for e.g. the odd backslash).
	     */
	    buf = tdest = zhalloc(maxlen);
	    t = tbuf = zhalloc(MB_CUR_MAX * 3 + 1);
	} else {
	    t = buf = zhalloc(maxlen);
	}
    }
    for (; *s; s++) {
	if (*s == '\\' && s[1]) {
	    int miscadded;
	    if ((how & GETKEY_UPDATE_OFFSET) && s - sstart < *misc) {
		(*misc)--;
		miscadded = 1;
	    } else
		miscadded = 0;
	    switch (*++s) {
	    case 'a':
#ifdef __STDC__
		*t++ = '\a';
#else
		*t++ = '\07';
#endif
		break;
	    case 'n':
		*t++ = '\n';
		break;
	    case 'b':
		*t++ = '\b';
		break;
	    case 't':
		*t++ = '\t';
		break;
	    case 'v':
		*t++ = '\v';
		break;
	    case 'f':
		*t++ = '\f';
		break;
	    case 'r':
		*t++ = '\r';
		break;
	    case 'E':
		if (!(how & GETKEY_EMACS)) {
		    *t++ = '\\', s--;
		    if (miscadded)
			(*misc)++;
		    continue;
		}
		/* FALL THROUGH */
	    case 'e':
		*t++ = '\033';
		break;
	    case 'M':
		/* HERE: GETKEY_UPDATE_OFFSET */
		if (how & GETKEY_EMACS) {
		    if (s[1] == '-')
			s++;
		    meta = 1 + control;	/* preserve the order of ^ and meta */
		} else {
		    if (miscadded)
			(*misc)++;
		    *t++ = '\\', s--;
		}
		continue;
	    case 'C':
		/* HERE: GETKEY_UPDATE_OFFSET */
		if (how & GETKEY_EMACS) {
		    if (s[1] == '-')
			s++;
		    control = 1;
		} else {
		    if (miscadded)
			(*misc)++;
		    *t++ = '\\', s--;
		}
		continue;
	    case Meta:
		if (miscadded)
		    (*misc)++;
		*t++ = '\\', s--;
		break;
	    case '-':
		if (how & GETKEY_BACKSLASH_MINUS) {
		    *misc  = 1;
		    break;
		}
		goto def;
	    case 'c':
		if (how & GETKEY_BACKSLASH_C) {
		    *misc = 1;
		    *t = '\0';
		    *len = t - buf;
		    return buf;
		}
		goto def;
	    case 'U':
		if ((how & GETKEY_UPDATE_OFFSET) && s - sstart < *misc)
		    (*misc) -= 4;
		/* FALLTHROUGH */
	    case 'u':
		if ((how & GETKEY_UPDATE_OFFSET) && s - sstart < *misc) {
		    (*misc) -= 6; /* HERE don't really believe this */
		    /*
		     * We've now adjusted the offset for all the input
		     * characters, so we need to add for each
		     * byte of output below.
		     */
		}
	    	wval = 0;
		for (i=(*s == 'u' ? 4 : 8); i>0; i--) {
		    if (*++s && idigit(*s))
		        wval = wval * 16 + (*s - '0');
		    else if (*s && ((*s >= 'a' && *s <= 'f') ||
				    (*s >= 'A' && *s <= 'F')))
		        wval = wval * 16 + (*s & 0x1f) + 9;
		    else {
		    	s--;
		        break;
		    }
		}
    	    	if (how & GETKEY_SINGLE_CHAR) {
		    *misc = wval;
		    return s+1;
		}
#if defined(HAVE_WCHAR_H) && defined(HAVE_WCTOMB) && defined(__STDC_ISO_10646__)
		count = wctomb(t, (wchar_t)wval);
		if (count == -1) {
		    zerr("character not in range");
		    CHARSET_FAILED();
		}
		if ((how & GETKEY_UPDATE_OFFSET) && s - sstart < *misc)
		    (*misc) += count;
		t += count;
# else
#  if defined(HAVE_NL_LANGINFO) && defined(CODESET)
		if (!strcmp(nl_langinfo(CODESET), "UTF-8")) {
		    count = ucs4toutf8(t, wval);
		    t += count;
		    if ((how & GETKEY_UPDATE_OFFSET) && s - sstart < *misc)
			(*misc) += count;
		} else {
#   ifdef HAVE_ICONV
		    ICONV_CONST char *inptr = inbuf;
		    const char *codesetstr = nl_langinfo(CODESET);
    	    	    inbytes = 4;
		    outbytes = 6;
		    /* store value in big endian form */
		    for (i=3;i>=0;i--) {
			inbuf[i] = wval & 0xff;
			wval >>= 8;
		    }

		    /*
		     * If the code set isn't handled, we'd better
		     * assume it's US-ASCII rather than just failing
		     * hopelessly.  Solaris has a weird habit of
		     * returning 646.  This is handled by the
		     * native iconv(), but not by GNU iconv; what's
		     * more, some versions of the native iconv don't
		     * handle standard names like ASCII.
		     *
		     * This should only be a problem if there's a
		     * mismatch between the NLS and the iconv in use,
		     * which probably only means if libiconv is in use.
		     * We checked at configure time if our libraries
		     * pulled in _libiconv_version, which should be
		     * a good test.
		     *
		     * It shouldn't ever be NULL, but while we're
		     * being paranoid...
		     */
#ifdef ICONV_FROM_LIBICONV
		    if (!codesetstr || !*codesetstr)
			codesetstr = "US-ASCII";
#endif
    	    	    cd = iconv_open(codesetstr, "UCS-4BE");
#ifdef ICONV_FROM_LIBICONV
		    if (cd == (iconv_t)-1 &&  !strcmp(codesetstr, "646")) {
			codesetstr = "US-ASCII";
			cd = iconv_open(codesetstr, "UCS-4BE");
		    }
#endif
		    if (cd == (iconv_t)-1) {
			zerr("cannot do charset conversion (iconv failed)");
			CHARSET_FAILED();
		    }
                    count = iconv(cd, &inptr, &inbytes, &t, &outbytes);
		    iconv_close(cd);
		    if (count == (size_t)-1) {
                        zerr("character not in range");
			CHARSET_FAILED();
		    }
		    if ((how & GETKEY_UPDATE_OFFSET) && s - sstart < *misc)
			(*misc) += count;
#   else
                    zerr("cannot do charset conversion (iconv not available)");
		    CHARSET_FAILED();
#   endif
		}
#  else
                zerr("cannot do charset conversion (NLS not supported)");
		CHARSET_FAILED();
#  endif
# endif
		if (how & GETKEY_DOLLAR_QUOTE) {
		    char *t2;
		    for (t2 = tbuf; t2 < t; t2++) {
			if (imeta(*t2)) {
			    *tdest++ = Meta;
			    *tdest++ = *t2 ^ 32;
			} else
			    *tdest++ = *t2;
		    }
		    /* reset temporary buffer after handling */
		    t = tbuf;
		}
		continue;
	    case '\'':
	    case '\\':
		if (how & GETKEY_DOLLAR_QUOTE) {
		    /*
		     * Usually \' and \\ will have the initial
		     * \ turned into a Bnull, however that's not
		     * necessarily the case when called from
		     * completion.
		     */
		    *t++ = *s;
		    break;
		}
		/* FALLTHROUGH */
	    default:
	    def:
		/* HERE: GETKEY_UPDATE_OFFSET? */
		if ((idigit(*s) && *s < '8') || *s == 'x') {
		    if (!(how & GETKEY_OCTAL_ESC)) {
			if (*s == '0')
			    s++;
			else if (*s != 'x') {
			    *t++ = '\\', s--;
			    continue;
			}
		    }
		    if (s[1] && s[2] && s[3]) {
			svchar = s[3];
			s[3] = '\0';
			u = s;
		    }
		    *t++ = zstrtol(s + (*s == 'x'), &s,
				   (*s == 'x') ? 16 : 8);
		    if ((how & GETKEY_PRINTF_PERCENT) && t[-1] == '%')
		        *t++ = '%';
		    if (svchar) {
			u[3] = svchar;
			svchar = '\0';
		    }
		    s--;
		} else {
		    if (!(how & GETKEY_EMACS) && *s != '\\') {
			if (miscadded)
			    (*misc)++;
			*t++ = '\\';
		    }
		    *t++ = *s;
		}
		break;
	    }
	} else if ((how & GETKEY_DOLLAR_QUOTE) && *s == Snull) {
	    /* return length to following character */
	    *len = (s - sstart) + 1;
	    *tdest = '\0';
	    return buf;
	} else if (*s == '^' && !control && (how & GETKEY_CTRL) && s[1]) {
	    control = 1;
	    continue;
#ifdef MULTIBYTE_SUPPORT
	} else if ((how & GETKEY_SINGLE_CHAR) &&
		   isset(MULTIBYTE) && STOUC(*s) > 127) {
	    wint_t wc;
	    int len;
	    len = mb_metacharlenconv(s, &wc);
	    if (wc != WEOF) {
		*misc = (int)wc;
		return s + len;
	    }
#endif

	} else if (*s == Meta)
	    *t++ = *++s ^ 32;
	else {
	    if (itok(*s)) {
		/*
		 * We need to be quite careful here.  We haven't
		 * necessarily got an input stream with all tokens
		 * removed, so the majority of tokens need passing
		 * through untouched and without Meta handling.
		 * However, me may need to handle tokenized
		 * backslashes.
		 */
		if (meta || control) {
		    /*
		     * Presumably we should be using meta or control
		     * on the character representing the token.
		     *
		     * Special case: $'\M-\\' where the token is a Bnull.
		     * This time we dump the Bnull since we're
		     * replacing the whole thing.  The lexer
		     * doesn't know about the meta or control modifiers.
		     */
		    if ((how & GETKEY_DOLLAR_QUOTE) && *s == Bnull)
			*t++ = *++s;
		    else
			*t++ = ztokens[*s - Pound];
		} else if (how & GETKEY_DOLLAR_QUOTE) {
		    /*
		     * We don't want to metafy this, it's a real
		     * token.
		     */
		    *tdest++ = *s;
		    if (*s == Bnull) {
			/*
			 * Bnull is a backslash which quotes a couple
			 * of special characters that always appear
			 * literally next.  See strquote handling
			 * in gettokstr() in lex.c.  We need
			 * to retain the Bnull (as above) so that quote
			 * handling in completion can tell where the
			 * backslash was.
			 */
			*tdest++ = *++s;
		    }
		    /* reset temporary buffer, now handled */
		    t = tbuf;
		    continue;
		} else
		    *t++ = *s;
	    } else
		*t++ = *s;
	}
	if (meta == 2) {
	    t[-1] |= 0x80;
	    meta = 0;
	}
	if (control) {
	    if (t[-1] == '?')
		t[-1] = 0x7f;
	    else
		t[-1] &= 0x9f;
	    control = 0;
	}
	if (meta) {
	    t[-1] |= 0x80;
	    meta = 0;
	}
	if (how & GETKEY_DOLLAR_QUOTE) {
	    char *t2;
	    for (t2 = tbuf; t2 < t; t2++) {
		/*
		 * In POSIX mode, an embedded NULL is discarded and
		 * terminates processing.  It just does, that's why.
		 */
		if (isset(POSIXSTRINGS)) {
		    if (*t2 == '\0')
			ignoring = 1;
		    if (ignoring)
			break;
		}
		if (imeta(*t2)) {
		    *tdest++ = Meta;
		    *tdest++ = *t2 ^ 32;
		} else {
		    *tdest++ = *t2;
		}
	    }
	    /*
	     * Reset use of temporary buffer.
	     */
	    t = tbuf;
	}
	if ((how & GETKEY_SINGLE_CHAR) && t != tmp) {
	    *misc = STOUC(tmp[0]);
	    return s + 1;
	}
    }
    /*
     * When called from completion, where we use GETKEY_UPDATE_OFFSET to
     * update the index into the metafied editor line, we don't necessarily
     * have the end of a $'...' quotation, else we should do.
     */
    DPUTS((how & (GETKEY_DOLLAR_QUOTE|GETKEY_UPDATE_OFFSET)) ==
	  GETKEY_DOLLAR_QUOTE, "BUG: unterminated $' substitution");

    if (how & GETKEY_SINGLE_CHAR) {
	/* couldn't find a character */
	*misc = 0;
	return NULL;
    }
    if (how & GETKEY_DOLLAR_QUOTE) {
	*tdest = '\0';
	*len = tdest - buf;
    }
    else {
	*t = '\0';
	*len = t - buf;
    }
    return buf;
}

/* Return non-zero if s is a prefix of t. */

/**/
mod_export int
strpfx(const char *s, const char *t)
{
    while (*s && *s == *t)
	s++, t++;
    return !*s;
}

/* Return non-zero if s is a suffix of t. */

/**/
mod_export int
strsfx(char *s, char *t)
{
    int ls = strlen(s), lt = strlen(t);

    if (ls <= lt)
	return !strcmp(t + lt - ls, s);
    return 0;
}

/**/
static int
upchdir(int n)
{
    char buf[PATH_MAX+1];
    char *s;
    int err = -1;

    while (n > 0) {
	for (s = buf; s < buf + PATH_MAX - 4 && n--; )
	    *s++ = '.', *s++ = '.', *s++ = '/';
	s[-1] = '\0';
	if (chdir(buf))
	    return err;
	err = -2;
    }
    return 0;
}

/*
 * Initialize a "struct dirsav".
 * The structure will be set to the directory we want to save
 * the first time we change to a different directory.
 */

/**/
mod_export void
init_dirsav(Dirsav d)
{
    d->ino = d->dev = 0;
    d->dirname = NULL;
    d->dirfd = d->level = -1;
}

/*
 * Change directory, without following symlinks.  Returns 0 on success, -1
 * on failure.  Sets errno to ENOTDIR if any symlinks are encountered.  If
 * fchdir() fails, or the current directory is unreadable, we might end up
 * in an unwanted directory in case of failure.
 *
 * path is an unmetafied but null-terminated string, as needed by system
 * calls.
 */

/**/
mod_export int
lchdir(char const *path, struct dirsav *d, int hard)
{
    char const *pptr;
    int level;
    struct stat st1;
    struct dirsav ds;
#ifdef HAVE_LSTAT
    char buf[PATH_MAX + 1], *ptr;
    int err;
    struct stat st2;
#endif
#ifdef HAVE_FCHDIR
    int close_dir = 0;
#endif

    if (!d) {
	init_dirsav(&ds);
	d = &ds;
    }
#ifdef HAVE_LSTAT
    if ((*path == '/' || !hard) &&
	(d != &ds || hard)){
#else
    if (*path == '/') {
#endif
	level = -1;
#ifndef HAVE_FCHDIR
	if (!d->dirname)
	    zgetdir(d);
#endif
    } else {
	level = 0;
	if (!d->dev && !d->ino) {
	    stat(".", &st1);
	    d->dev = st1.st_dev;
	    d->ino = st1.st_ino;
	}
    }

#ifdef HAVE_LSTAT
    if (!hard)
#endif
    {
	if (d != &ds) {
	    for (pptr = path; *pptr; level++) {
		while (*pptr && *pptr++ != '/');
		while (*pptr == '/')
		    pptr++;
	    }
	    d->level = level;
	}
	return zchdir((char *) path);
    }

#ifdef HAVE_LSTAT
#ifdef HAVE_FCHDIR
    if (d->dirfd < 0) {
	close_dir = 1;
        if ((d->dirfd = open(".", O_RDONLY | O_NOCTTY)) < 0 &&
	    zgetdir(d) && *d->dirname != '/')
	    d->dirfd = open("..", O_RDONLY | O_NOCTTY);
    }
#endif
    if (*path == '/')
	if (chdir("/") < 0)
	    zwarn("failed to chdir(/): %e", errno);
    for(;;) {
	while(*path == '/')
	    path++;
	if(!*path) {
	    if (d == &ds)
		zsfree(ds.dirname);
	    else
		d->level = level;
#ifdef HAVE_FCHDIR
	    if (d->dirfd >=0 && close_dir) {
		close(d->dirfd);
		d->dirfd = -1;
	    }
#endif
	    return 0;
	}
	for(pptr = path; *++pptr && *pptr != '/'; ) ;
	if(pptr - path > PATH_MAX) {
	    err = ENAMETOOLONG;
	    break;
	}
	for(ptr = buf; path != pptr; )
	    *ptr++ = *path++;
	*ptr = 0;
	if(lstat(buf, &st1)) {
	    err = errno;
	    break;
	}
	if(!S_ISDIR(st1.st_mode)) {
	    err = ENOTDIR;
	    break;
	}
	if(chdir(buf)) {
	    err = errno;
	    break;
	}
	if (level >= 0)
	    level++;
	if(lstat(".", &st2)) {
	    err = errno;
	    break;
	}
	if(st1.st_dev != st2.st_dev || st1.st_ino != st2.st_ino) {
	    err = ENOTDIR;
	    break;
	}
    }
    if (restoredir(d)) {
	int restoreerr = errno;
	int i;
	/*
	 * Failed to restore the directory.
	 * Just be definite, cd to root and report the result.
	 */
	for (i = 0; i < 2; i++) {
	    const char *cdest;
	    if (i)
		cdest = "/";
	    else {
		if (!home)
		    continue;
		cdest = home;
	    }
	    zsfree(pwd);
	    pwd = ztrdup(cdest);
	    if (chdir(pwd) == 0)
		break;
	}
	if (i == 2)
	    zerr("lost current directory, failed to cd to /: %e", errno);
	else
	    zerr("lost current directory: %e: changed to `%s'", restoreerr,
		pwd);
	if (d == &ds)
	    zsfree(ds.dirname);
#ifdef HAVE_FCHDIR
	if (d->dirfd >=0 && close_dir) {
	    close(d->dirfd);
	    d->dirfd = -1;
	}
#endif
	errno = err;
	return -2;
    }
    if (d == &ds)
	zsfree(ds.dirname);
#ifdef HAVE_FCHDIR
    if (d->dirfd >=0 && close_dir) {
	close(d->dirfd);
	d->dirfd = -1;
    }
#endif
    errno = err;
    return -1;
#endif /* HAVE_LSTAT */
}

/**/
mod_export int
restoredir(struct dirsav *d)
{
    int err = 0;
    struct stat sbuf;

    if (d->dirname && *d->dirname == '/')
	return chdir(d->dirname);
#ifdef HAVE_FCHDIR
    if (d->dirfd >= 0) {
	if (!fchdir(d->dirfd)) {
	    if (!d->dirname) {
		return 0;
	    } else if (chdir(d->dirname)) {
		close(d->dirfd);
		d->dirfd = -1;
		err = -2;
	    }
	} else {
	    close(d->dirfd);
	    d->dirfd = err = -1;
	}
    } else
#endif
    if (d->level > 0)
	err = upchdir(d->level);
    else if (d->level < 0)
	err = -1;
    if (d->dev || d->ino) {
	stat(".", &sbuf);
	if (sbuf.st_ino != d->ino || sbuf.st_dev != d->dev)
	    err = -2;
    }
    return err;
}


/* Check whether the shell is running with privileges in effect.  *
 * This is the case if EITHER the euid is zero, OR (if the system *
 * supports POSIX.1e (POSIX.6) capability sets) the process'      *
 * Effective or Inheritable capability sets are non-empty.        */

/**/
int
privasserted(void)
{
    if(!geteuid())
	return 1;
#ifdef HAVE_CAP_GET_PROC
    {
	cap_t caps = cap_get_proc();
	if(caps) {
	    /* POSIX doesn't define a way to test whether a capability set *
	     * is empty or not.  Typical.  I hope this is conforming...    */
	    cap_flag_value_t val;
	    cap_value_t n;
	    for(n = 0; !cap_get_flag(caps, n, CAP_EFFECTIVE, &val); n++)
		if(val) {
		    cap_free(caps);
		    return 1;
		}
	}
	cap_free(caps);
    }
#endif /* HAVE_CAP_GET_PROC */
    return 0;
}

/**/
mod_export int
mode_to_octal(mode_t mode)
{
    int m = 0;

    if(mode & S_ISUID)
	m |= 04000;
    if(mode & S_ISGID)
	m |= 02000;
    if(mode & S_ISVTX)
	m |= 01000;
    if(mode & S_IRUSR)
	m |= 00400;
    if(mode & S_IWUSR)
	m |= 00200;
    if(mode & S_IXUSR)
	m |= 00100;
    if(mode & S_IRGRP)
	m |= 00040;
    if(mode & S_IWGRP)
	m |= 00020;
    if(mode & S_IXGRP)
	m |= 00010;
    if(mode & S_IROTH)
	m |= 00004;
    if(mode & S_IWOTH)
	m |= 00002;
    if(mode & S_IXOTH)
	m |= 00001;
    return m;
}

#ifdef MAILDIR_SUPPORT
/*
 *     Stat a file. If it's a maildir, check all messages
 *     in the maildir and present the grand total as a file.
 *     The fields in the 'struct stat' are from the mail directory.
 *     The following fields are emulated:
 *
 *     st_nlink        always 1
 *     st_size         total number of bytes in all files
 *     st_blocks       total number of messages
 *     st_atime        access time of newest file in maildir
 *     st_mtime        modify time of newest file in maildir
 *     st_mode         S_IFDIR changed to S_IFREG
 *
 *     This is good enough for most mail-checking applications.
 */

/**/
int
mailstat(char *path, struct stat *st)
{
       DIR                     *dd;
       struct                  dirent *fn;
       struct stat             st_ret, st_tmp;
       static struct stat      st_ret_last;
       char                    *dir, *file = 0;
       int                     i;
       time_t                  atime = 0, mtime = 0;
       size_t                  plen = strlen(path), dlen;

       /* First see if it's a directory. */
       if ((i = stat(path, st)) != 0 || !S_ISDIR(st->st_mode))
               return i;

       st_ret = *st;
       st_ret.st_nlink = 1;
       st_ret.st_size  = 0;
       st_ret.st_blocks  = 0;
       st_ret.st_mode  &= ~S_IFDIR;
       st_ret.st_mode  |= S_IFREG;

       /* See if cur/ is present */
       dir = appstr(ztrdup(path), "/cur");
       if (stat(dir, &st_tmp) || !S_ISDIR(st_tmp.st_mode)) {
	   zsfree(dir);
	   return 0;
       }
       st_ret.st_atime = st_tmp.st_atime;

       /* See if tmp/ is present */
       dir[plen] = 0;
       dir = appstr(dir, "/tmp");
       if (stat(dir, &st_tmp) || !S_ISDIR(st_tmp.st_mode)) {
	   zsfree(dir);
	   return 0;
       }
       st_ret.st_mtime = st_tmp.st_mtime;

       /* And new/ */
       dir[plen] = 0;
       dir = appstr(dir, "/new");
       if (stat(dir, &st_tmp) || !S_ISDIR(st_tmp.st_mode)) {
	   zsfree(dir);
	   return 0;
       }
       st_ret.st_mtime = st_tmp.st_mtime;

#if THERE_IS_EXACTLY_ONE_MAILDIR_IN_MAILPATH
       {
       static struct stat      st_new_last;
       /* Optimization - if new/ didn't change, nothing else did. */
       if (st_tmp.st_dev == st_new_last.st_dev &&
           st_tmp.st_ino == st_new_last.st_ino &&
           st_tmp.st_atime == st_new_last.st_atime &&
           st_tmp.st_mtime == st_new_last.st_mtime) {
	   *st = st_ret_last;
	   zsfree(dir);
	   return 0;
       }
       st_new_last = st_tmp;
       }
#endif

       /* Loop over new/ and cur/ */
       for (i = 0; i < 2; i++) {
	   dir[plen] = 0;
	   dir = appstr(dir, i ? "/cur" : "/new");
	   if ((dd = opendir(dir)) == NULL) {
	       zsfree(file);
	       zsfree(dir);
	       return 0;
	   }
	   dlen = strlen(dir) + 1; /* include the "/" */
	   while ((fn = readdir(dd)) != NULL) {
	       if (fn->d_name[0] == '.')
		   continue;
	       if (file) {
		   file[dlen] = 0;
		   file = appstr(file, fn->d_name);
	       } else {
		   file = tricat(dir, "/", fn->d_name);
	       }
	       if (stat(file, &st_tmp) != 0)
		   continue;
	       st_ret.st_size += st_tmp.st_size;
	       st_ret.st_blocks++;
	       if (st_tmp.st_atime != st_tmp.st_mtime &&
		   st_tmp.st_atime > atime)
		   atime = st_tmp.st_atime;
	       if (st_tmp.st_mtime > mtime)
		   mtime = st_tmp.st_mtime;
	   }
	   closedir(dd);
       }
       zsfree(file);
       zsfree(dir);

       if (atime) st_ret.st_atime = atime;
       if (mtime) st_ret.st_mtime = mtime;

       *st = st_ret_last = st_ret;
       return 0;
}
#endif
