/*
 * watch.c - login/logout watching
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

#include "watch.mdh"

/* Headers for utmp/utmpx structures */
#ifdef HAVE_UTMP_H
# include <utmp.h>
#endif
#ifdef HAVE_UTMPX_H
# include <utmpx.h>
#endif

/* Find utmp file */
#if !defined(REAL_UTMP_FILE) && defined(UTMP_FILE)
# define REAL_UTMP_FILE UTMP_FILE
#endif
#if !defined(REAL_UTMP_FILE) && defined(_PATH_UTMP)
# define REAL_UTMP_FILE _PATH_UTMP
#endif
#if !defined(REAL_UTMP_FILE) && defined(PATH_UTMP_FILE)
# define REAL_UTMP_FILE PATH_UTMP_FILE
#endif

/* Find wtmp file */
#if !defined(REAL_WTMP_FILE) && defined(WTMP_FILE)
# define REAL_WTMP_FILE WTMP_FILE
#endif
#if !defined(REAL_WTMP_FILE) && defined(_PATH_WTMP)
# define REAL_WTMP_FILE _PATH_WTMP
#endif
#if !defined(REAL_WTMP_FILE) && defined(PATH_WTMP_FILE)
# define REAL_WTMP_FILE PATH_WTMP_FILE
#endif

/* Find utmpx file */
#if !defined(REAL_UTMPX_FILE) && defined(UTMPX_FILE)
# define REAL_UTMPX_FILE UTMPX_FILE
#endif
#if !defined(REAL_UTMPX_FILE) && defined(_PATH_UTMPX)
# define REAL_UTMPX_FILE _PATH_UTMPX
#endif
#if !defined(REAL_UTMPX_FILE) && defined(PATH_UTMPX_FILE)
# define REAL_UTMPX_FILE PATH_UTMPX_FILE
#endif

/* Find wtmpx file */
#if !defined(REAL_WTMPX_FILE) && defined(WTMPX_FILE)
# define REAL_WTMPX_FILE WTMPX_FILE
#endif
#if !defined(REAL_WTMPX_FILE) && defined(_PATH_WTMPX)
# define REAL_WTMPX_FILE _PATH_WTMPX
#endif
#if !defined(REAL_WTMPX_FILE) && defined(PATH_WTMPX_FILE)
# define REAL_WTMPX_FILE PATH_WTMPX_FILE
#endif

/* Decide which structure to use.  We use a structure that exists in *
 * the headers, and require that its corresponding utmp file exist.  *
 * (wtmp is less important.)                                         */

#if !defined(WATCH_STRUCT_UTMP) && defined(HAVE_STRUCT_UTMPX) && defined(REAL_UTMPX_FILE)
# define WATCH_STRUCT_UTMP struct utmpx
# if defined(HAVE_SETUTXENT) && defined(HAVE_GETUTXENT) && defined(HAVE_ENDUTXENT)
#  define setutent setutxent
#  define getutent getutxent
#  define endutent endutxent
#  ifndef HAVE_GETUTENT
#   define HAVE_GETUTENT 1
#  endif
# endif

/*
 * In utmpx, the ut_name field is replaced by ut_user.
 * However, on some systems ut_name may already be defined this
 * way for the purposes of utmp.
 */
# ifndef ut_name
#  define ut_name ut_user
# endif
# ifdef HAVE_STRUCT_UTMPX_UT_XTIME
#  undef ut_time
#  define ut_time ut_xtime
# else /* !HAVE_STRUCT_UTMPX_UT_XTIME */
#  ifdef HAVE_STRUCT_UTMPX_UT_TV
#   undef ut_time
#   define ut_time ut_tv.tv_sec
#  endif /* HAVE_STRUCT_UTMPX_UT_TV */
# endif /* !HAVE_STRUCT_UTMPX_UT_XTIME */
# define WATCH_UTMP_FILE REAL_UTMPX_FILE
# ifdef REAL_WTMPX_FILE
#  define WATCH_WTMP_FILE REAL_WTMPX_FILE
# endif
# ifdef HAVE_STRUCT_UTMPX_UT_HOST
#  define WATCH_UTMP_UT_HOST 1
# endif
#endif

#if !defined(WATCH_STRUCT_UTMP) && defined(HAVE_STRUCT_UTMP) && defined(REAL_UTMP_FILE)
# define WATCH_STRUCT_UTMP struct utmp
# define WATCH_UTMP_FILE REAL_UTMP_FILE
# ifdef REAL_WTMP_FILE
#  define WATCH_WTMP_FILE REAL_WTMP_FILE
# endif
# ifdef HAVE_STRUCT_UTMP_UT_HOST
#  define WATCH_UTMP_UT_HOST 1
# endif
#endif

#ifdef WATCH_UTMP_UT_HOST
# define DEFAULT_WATCHFMT "%n has %a %l from %m."
#else /* !WATCH_UTMP_UT_HOST */
# define DEFAULT_WATCHFMT "%n has %a %l."
#endif /* !WATCH_UTMP_UT_HOST */

#ifdef WATCH_STRUCT_UTMP

# include "watch.pro"

# ifndef WATCH_WTMP_FILE
#  define WATCH_WTMP_FILE "/dev/null"
# endif

static int wtabsz = 0;
static WATCH_STRUCT_UTMP *wtab = NULL;

/* the last time we checked the people in the WATCH variable */
static time_t lastwatch;

static time_t lastutmpcheck = 0;

/* get the time of login/logout for WATCH */

static time_t
getlogtime(WATCH_STRUCT_UTMP *u, int inout)
{
    FILE *in;
    WATCH_STRUCT_UTMP uu;
    int first = 1;
    int srchlimit = 50;		/* max number of wtmp records to search */

    if (inout)
	return u->ut_time;
    if (!(in = fopen(WATCH_WTMP_FILE, "r")))
	return time(NULL);
    fseek(in, 0, SEEK_END);
    do {
	if (fseek(in, ((first) ? -1 : -2) * sizeof(WATCH_STRUCT_UTMP), SEEK_CUR)) {
	    fclose(in);
	    return time(NULL);
	}
	first = 0;
	if (!fread(&uu, sizeof(WATCH_STRUCT_UTMP), 1, in)) {
	    fclose(in);
	    return time(NULL);
	}
	if (uu.ut_time < lastwatch || !srchlimit--) {
	    fclose(in);
	    return time(NULL);
	}
    }
    while (memcmp(&uu, u, sizeof(uu)));

    do
	if (!fread(&uu, sizeof(WATCH_STRUCT_UTMP), 1, in)) {
	    fclose(in);
	    return time(NULL);
	}
    while (strncmp(uu.ut_line, u->ut_line, sizeof(u->ut_line)));
    fclose(in);
    return uu.ut_time;
}

/* Mutually recursive call to handle ternaries in $WATCHFMT */

# define BEGIN3 '('
# define END3 ')'

static char *
watch3ary(int inout, WATCH_STRUCT_UTMP *u, char *fmt, int prnt)
{
    int truth = 1, sep;

    switch (*fmt++) {
    case 'n':
	truth = (u->ut_name[0] != 0);
	break;
    case 'a':
	truth = inout;
	break;
    case 'l':
	if (!strncmp(u->ut_line, "tty", 3))
	    truth = (u->ut_line[3] != 0);
	else
	    truth = (u->ut_line[0] != 0);
	break;
# ifdef WATCH_UTMP_UT_HOST
    case 'm':
    case 'M':
	truth = (u->ut_host[0] != 0);
	break;
# endif /* WATCH_UTMP_UT_HOST */
    default:
	prnt = 0;		/* Skip unknown conditionals entirely */
	break;
    }
    sep = *fmt++;
    fmt = watchlog2(inout, u, fmt, (truth && prnt), sep);
    return watchlog2(inout, u, fmt, (!truth && prnt), END3);
}

/* print a login/logout event */

/**/
static char *
watchlog2(int inout, WATCH_STRUCT_UTMP *u, char *fmt, int prnt, int fini)
{
    char buf[40], buf2[80];
    time_t timet;
    struct tm *tm;
    char *fm2;
    int len;
    zattr atr;
# ifdef WATCH_UTMP_UT_HOST
    char *p;
    int i;
# endif /* WATCH_UTMP_UT_HOST */

    while (*fmt)
	if (*fmt == '\\') {
	    if (*++fmt) {
		if (prnt)
		    putchar(*fmt);
		++fmt;
	    } else if (fini)
		return fmt;
	    else
		break;
	}
	else if (*fmt == fini)
	    return ++fmt;
	else if (*fmt != '%') {
	    if (prnt)
		putchar(*fmt);
	    ++fmt;
	} else {
	    if (*++fmt == BEGIN3)
		fmt = watch3ary(inout, u, ++fmt, prnt);
	    else if (!prnt)
		++fmt;
	    else
		switch (*(fm2 = fmt++)) {
		case 'n':
		    printf("%.*s", (int)sizeof(u->ut_name), u->ut_name);
		    break;
		case 'a':
		    printf("%s", (!inout) ? "logged off" : "logged on");
		    break;
		case 'l':
		    if (!strncmp(u->ut_line, "tty", 3))
			printf("%.*s", (int)sizeof(u->ut_line) - 3, u->ut_line + 3);
		    else
			printf("%.*s", (int)sizeof(u->ut_line), u->ut_line);
		    break;
# ifdef WATCH_UTMP_UT_HOST
		case 'm':
		    for (p = u->ut_host, i = sizeof(u->ut_host); i && *p; i--, p++) {
			if (*p == '.' && !idigit(p[1]))
			    break;
			putchar(*p);
		    }
		    break;
		case 'M':
		    printf("%.*s", (int)sizeof(u->ut_host), u->ut_host);
		    break;
# endif /* WATCH_UTMP_UT_HOST */
		case 'T':
		case 't':
		case '@':
		case 'W':
		case 'w':
		case 'D':
		    switch (*fm2) {
		    case '@':
		    case 't':
			fm2 = "%l:%M%p";
			break;
		    case 'T':
			fm2 = "%K:%M";
			break;
		    case 'w':
			fm2 = "%a %f";
			break;
		    case 'W':
			fm2 = "%m/%d/%y";
			break;
		    case 'D':
			if (fm2[1] == '{') {
			    char *dd, *ss;
			    int n = 79;

			    for (ss = fm2 + 2, dd = buf2;
				 n-- && *ss && *ss != '}'; ++ss, ++dd)
				*dd = *((*ss == '\\' && ss[1]) ? ++ss : ss);
			    if (*ss == '}') {
				*dd = '\0';
				fmt = ss + 1;
				fm2 = buf2;
			    }
			    else fm2 = "%y-%m-%d";
			}
			else fm2 = "%y-%m-%d";
			break;
		    }
		    timet = getlogtime(u, inout);
		    tm = localtime(&timet);
		    len = ztrftime(buf, 40, fm2, tm, 0L);
		    if (len > 0)
			metafy(buf, len, META_NOALLOC);
		    printf("%s", (*buf == ' ') ? buf + 1 : buf);
		    break;
		case '%':
		    putchar('%');
		    break;
		case 'F':
		    if (*fmt == '{') {
			fmt++;
			atr = match_colour((const char**)&fmt, 1, 0);
			if (*fmt == '}')
			    fmt++;
			if (!(atr & (TXT_ERROR | TXTNOFGCOLOUR))) {
			    txtunset(TXT_ATTR_FG_COL_MASK);
			    txtset(atr & TXT_ATTR_FG_ON_MASK);
			    set_colour_attribute(atr, COL_SEQ_FG, TSC_RAW);
			}
		    }
		    break;
		case 'f':
		    txtunset(TXT_ATTR_FG_ON_MASK);
		    set_colour_attribute(TXTNOFGCOLOUR, COL_SEQ_FG, TSC_RAW);
		    break;
		case 'K':
		    if (*fmt == '{') {
			fmt++;
			atr = match_colour((const char**)&fmt, 0, 0);
			if (*fmt == '}')
			    fmt++;
			if (!(atr & (TXT_ERROR | TXTNOBGCOLOUR))) {
			    txtunset(TXT_ATTR_BG_COL_MASK);
			    txtset(atr & TXT_ATTR_BG_ON_MASK);
			    set_colour_attribute(atr, COL_SEQ_BG, TSC_RAW);
			}
		    }
		    break;
		case 'k':
		    txtunset(TXT_ATTR_BG_ON_MASK);
		    set_colour_attribute(TXTNOBGCOLOUR, COL_SEQ_BG, TSC_RAW);
		    break;
		case 'S':
		    txtset(TXTSTANDOUT);
		    tsetcap(TCSTANDOUTBEG, TSC_RAW);
		    break;
		case 's':
		    txtunset(TXTSTANDOUT);
		    tsetcap(TCSTANDOUTEND, TSC_RAW|TSC_DIRTY);
		    break;
		case 'B':
		    txtset(TXTBOLDFACE);
		    tsetcap(TCBOLDFACEBEG, TSC_RAW|TSC_DIRTY);
		    break;
		case 'b':
		    txtunset(TXTBOLDFACE);
		    tsetcap(TCALLATTRSOFF, TSC_RAW|TSC_DIRTY);
		    break;
		case 'U':
		    txtset(TXTUNDERLINE);
		    tsetcap(TCUNDERLINEBEG, TSC_RAW);
		    break;
		case 'u':
		    txtunset(TXTUNDERLINE);
		    tsetcap(TCUNDERLINEEND, TSC_RAW|TSC_DIRTY);
		    break;
		default:
		    putchar('%');
		    putchar(*fm2);
		    break;
		}
	}
    if (prnt)
	putchar('\n');

    return fmt;
}

/* See if the watch entry matches */

static int
watchlog_match(char *teststr, char *actual, int len)
{
    int ret = 0;
    Patprog pprog;
    char *str = dupstring(teststr);

    tokenize(str);

    if ((pprog = patcompile(str, PAT_STATIC, 0))) {
	queue_signals();
	if (pattry(pprog, actual))
	    ret = 1;
	unqueue_signals();
    } else if (!strncmp(actual, teststr, len))
	ret = 1;
    return ret;
}

/* check the List for login/logouts */

static void
watchlog(int inout, WATCH_STRUCT_UTMP *u, char **w, char *fmt)
{
    char *v, *vv, sav;
    int bad;

    if (!*u->ut_name)
	return;

    if (*w && !strcmp(*w, "all")) {
	(void)watchlog2(inout, u, fmt, 1, 0);
	return;
    }
    if (*w && !strcmp(*w, "notme") &&
	strncmp(u->ut_name, get_username(), sizeof(u->ut_name))) {
	(void)watchlog2(inout, u, fmt, 1, 0);
	return;
    }
    for (; *w; w++) {
	bad = 0;
	v = *w;
	if (*v != '@' && *v != '%') {
	    for (vv = v; *vv && *vv != '@' && *vv != '%'; vv++);
	    sav = *vv;
	    *vv = '\0';
	    if (!watchlog_match(v, u->ut_name, sizeof(u->ut_name)))
		bad = 1;
	    *vv = sav;
	    v = vv;
	}
	for (;;)
	    if (*v == '%') {
		for (vv = ++v; *vv && *vv != '@'; vv++);
		sav = *vv;
		*vv = '\0';
		if (!watchlog_match(v, u->ut_line, sizeof(u->ut_line)))
		    bad = 1;
		*vv = sav;
		v = vv;
	    }
# ifdef WATCH_UTMP_UT_HOST
	    else if (*v == '@') {
		for (vv = ++v; *vv && *vv != '%'; vv++);
		sav = *vv;
		*vv = '\0';
		if (!watchlog_match(v, u->ut_host, strlen(v)))
		    bad = 1;
		*vv = sav;
		v = vv;
	    }
# endif /* WATCH_UTMP_UT_HOST */
	    else
		break;
	if (!bad) {
	    (void)watchlog2(inout, u, fmt, 1, 0);
	    return;
	}
    }
}

/* compare 2 utmp entries */

static int
ucmp(WATCH_STRUCT_UTMP *u, WATCH_STRUCT_UTMP *v)
{
    if (u->ut_time == v->ut_time)
	return strncmp(u->ut_line, v->ut_line, sizeof(u->ut_line));
    return u->ut_time - v->ut_time;
}

/* initialize the user List */

static int
readwtab(WATCH_STRUCT_UTMP **head, int initial_sz)
{
    WATCH_STRUCT_UTMP *uptr;
    int wtabmax = initial_sz < 2 ? 32 : initial_sz;
    int sz = 0;
# ifdef HAVE_GETUTENT
    WATCH_STRUCT_UTMP *tmp;
# else
    FILE *in;
# endif

    uptr = *head = (WATCH_STRUCT_UTMP *)
	zalloc(wtabmax * sizeof(WATCH_STRUCT_UTMP));
# ifdef HAVE_GETUTENT
    setutent();
    while ((tmp = getutent()) != NULL) {
	memcpy(uptr, tmp, sizeof (WATCH_STRUCT_UTMP));
# else
    if (!(in = fopen(WATCH_UTMP_FILE, "r")))
	return 0;
    while (fread(uptr, sizeof(WATCH_STRUCT_UTMP), 1, in)) {
# endif
# ifdef USER_PROCESS
	if (uptr->ut_type == USER_PROCESS)
# else /* !USER_PROCESS */
	if (uptr->ut_name[0])
# endif /* !USER_PROCESS */
	{
	    uptr++;
	    if (++sz == wtabmax) {
		uptr = (WATCH_STRUCT_UTMP *)
		    realloc(*head, (wtabmax *= 2) * sizeof(WATCH_STRUCT_UTMP));
		if (uptr == NULL) {
		    /* memory pressure - so stop consuming and use, what we have
		     * Other option is to exit() here, as zmalloc does on error */
		    sz--;
		    break;
		}
		*head = uptr;
		uptr += sz;
	    }
	}
    }
# ifdef HAVE_GETUTENT
    endutent();
# else
    fclose(in);
# endif

    if (sz)
	qsort((void *) *head, sz, sizeof(WATCH_STRUCT_UTMP),
	           (int (*) _((const void *, const void *)))ucmp);
    return sz;
}

/* Check for login/logout events; executed before *
 * each prompt if WATCH is set                    */

/**/
void
dowatch(void)
{
    WATCH_STRUCT_UTMP *utab, *uptr, *wptr;
    struct stat st;
    char **s;
    char *fmt;
    int utabsz, uct, wct;

    s = watch;

    holdintr();
    if (!wtab)
	wtabsz = readwtab(&wtab, 32);
    if ((stat(WATCH_UTMP_FILE, &st) == -1) || (st.st_mtime <= lastutmpcheck)) {
	noholdintr();
	return;
    }
    lastutmpcheck = st.st_mtime;
    utabsz = readwtab(&utab, wtabsz + 4);
    noholdintr();
    if (errflag) {
	free(utab);
	return;
    }

    wct = wtabsz;
    uct = utabsz;
    uptr = utab;
    wptr = wtab;
    if (errflag) {
	free(utab);
	return;
    }
    queue_signals();
    if (!(fmt = getsparam_u("WATCHFMT")))
	fmt = DEFAULT_WATCHFMT;
    while ((uct || wct) && !errflag) {
	if (!uct || (wct && ucmp(uptr, wptr) > 0))
	    wct--, watchlog(0, wptr++, s, fmt);
	else if (!wct || (uct && ucmp(uptr, wptr) < 0))
	    uct--, watchlog(1, uptr++, s, fmt);
	else
	    uptr++, wptr++, wct--, uct--;
    }
    unqueue_signals();
    free(wtab);
    wtab = utab;
    wtabsz = utabsz;
    fflush(stdout);
    lastwatch = time(NULL);
}

static void
checksched(void)
{
    /* Do nothing if WATCH is not set, or LOGCHECK has not elapsed */
    if (watch && (int) difftime(time(NULL), lastwatch) > getiparam("LOGCHECK"))
	dowatch();
}

/**/
static int
bin_log(UNUSED(char *nam), UNUSED(char **argv), UNUSED(Options ops), UNUSED(int func))
{
    if (!watch)
	return 1;
    if (wtab)
	free(wtab);
    wtab = (WATCH_STRUCT_UTMP *)zalloc(1);
    wtabsz = 0;
    lastutmpcheck = 0;
    dowatch();
    return 0;
}

#else /* !WATCH_STRUCT_UTMP */

static void
checksched(void)
{
}

/**/
static int
bin_log(char *nam, char **argv, Options ops, int func)
{
    return bin_notavail(nam, argv, ops, func);
}

#endif /* !WATCH_STRUCT_UTMP */

/**/
static char **watch; /* $watch */

/* module setup */

static struct builtin bintab[] = {
    BUILTIN("log", 0, bin_log, 0, 0, 0, NULL, NULL),
};

static struct paramdef partab[] = {
    PARAMDEF("WATCH", PM_SCALAR|PM_SPECIAL, &watch, NULL),
    PARAMDEF("watch", PM_ARRAY|PM_SPECIAL, &watch, NULL),
};

static struct features module_features = {
    bintab, sizeof(bintab)/sizeof(*bintab),
    NULL, 0,
    NULL, 0,
    partab, sizeof(partab)/sizeof(*partab),
    0
};

/**/
int
setup_(UNUSED(Module m))
{
    /* On Cygwin, colonarr_gsu exists in libzsh.dll and we can't
     * use &colonarr_gsu in the initialization of partab[] above */
    partab[0].gsu = (void *)&colonarr_gsu;
    partab[1].gsu = (void *)&vararray_gsu;
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
    static char const * const default_watchfmt = DEFAULT_WATCHFMT;

    Param pma = (Param) paramtab->getnode(paramtab, "watch");
    Param pms = (Param) paramtab->getnode(paramtab, "WATCH");
    if (pma && pms && pma->u.arr == watch && pms->u.arr == watch) {
	/* only tie the two parameters if both were added */
	pma->ename = "WATCH";
	pms->ename = "watch";
	pma->node.flags |= PM_TIED;
	pms->node.flags |= PM_TIED;
    }
    watch = mkarray(NULL);

    /* These two parameters are only set to defaults if not set.
     * So setting them in .zshrc will not be enough to load the
     * module. It's useless until the watch array is set anyway. */
    if (!paramtab->getnode(paramtab, "WATCHFMT"))
	setsparam("WATCHFMT", ztrdup_metafy(default_watchfmt));
    if (!paramtab->getnode(paramtab, "LOGCHECK"))
	setiparam("LOGCHECK", 60);

    addprepromptfn(&checksched);

    return 0;
}

/**/
int
cleanup_(Module m)
{
    delprepromptfn(&checksched);
    return setfeatureenables(m, &module_features, NULL);
}

/**/
int
finish_(UNUSED(Module m))
{
    return 0;
}
