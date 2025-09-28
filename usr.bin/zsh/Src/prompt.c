/*
 * prompt.c - construct zsh prompts
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
#include "prompt.pro"

/* text attribute mask */

/**/
mod_export zattr txtattrmask;

/* the command stack for use with %_ in prompts */

/**/
unsigned char *cmdstack;
/**/
int cmdsp;

/* parser states, for %_ */

static char *cmdnames[CS_COUNT] = {
    "for",      "while",     "repeat",    "select",
    "until",    "if",        "then",      "else",
    "elif",     "math",      "cond",      "cmdor",
    "cmdand",   "pipe",      "errpipe",   "foreach",
    "case",     "function",  "subsh",     "cursh",
    "array",    "quote",     "dquote",    "bquote",
    "cmdsubst", "mathsubst", "elif-then", "heredoc",
    "heredocd", "brace",     "braceparam", "always",
};


struct buf_vars;

struct buf_vars {
/* Previous set of prompt variables on the stack. */

    struct buf_vars *last;

/* The buffer into which an expanded and metafied prompt is being written, *
 * and its size.                                                           */

    char *buf;
    int bufspc;

/* bp is the pointer to the current position in the buffer, where the next *
 * character will be added.                                                */

    char *bp;

/* Position of the start of the current line in the buffer */

    char *bufline;

/* bp1 is an auxiliary pointer into the buffer, which when non-NULL is *
 * moved whenever the buffer is reallocated.  It is used when data is   *
 * being temporarily held in the buffer.                                */

    char *bp1;

/* The format string, for %-expansion. */

    char *fm;

/* Non-zero if truncating the current segment of the buffer. */

    int truncwidth;

/* Current level of nesting of %{ / %} sequences. */

    int dontcount;

/* Level of %{ / %} surrounding a truncation segment. */

    int trunccount;

/* Strings to use for %r and %R (for the spelling prompt). */

    char *rstring, *Rstring;
};

typedef struct buf_vars *Buf_vars;

/* The currently active prompt output variables */
static Buf_vars bv;

/*
 * Expand path p; maximum is npath segments where 0 means the whole path.
 * If tilde is 1, try and find a named directory to use.
 */

static void
promptpath(char *p, int npath, int tilde)
{
    char *modp = p;
    Nameddir nd;

    if (tilde && ((nd = finddir(p))))
	modp = tricat("~", nd->node.nam, p + strlen(nd->dir));

    if (npath) {
	char *sptr;
	if (npath > 0) {
	    for (sptr = modp + strlen(modp); sptr > modp; sptr--) {
		if (*sptr == '/' && !--npath) {
		    sptr++;
		    break;
		}
	    }
	    if (*sptr == '/' && sptr[1] && sptr != modp)
		sptr++;
	    stradd(sptr);
	} else {
	    char cbu;
	    for (sptr = modp+1; *sptr; sptr++)
		if (*sptr == '/' && !++npath)
		    break;
	    cbu = *sptr;
	    *sptr = 0;
	    stradd(modp);
	    *sptr = cbu;
	}
    } else
	stradd(modp);

    if (p != modp)
	zsfree(modp);
}

/*
 * Perform prompt expansion on a string, putting the result in a
 * permanently-allocated string.  If ns is non-zero, this string
 * may have embedded Inpar and Outpar, which indicate a toggling
 * between spacing and non-spacing parts of the prompt, and
 * Nularg, which (in a non-spacing sequence) indicates a
 * `glitch' space.
 *
 * txtchangep gives an integer controlling the attributes of
 * the prompt.  This is for use in zle to maintain the attributes
 * consistently.  Other parts of the shell should not need to use it.
 */

/**/
mod_export char *
promptexpand(char *s, int ns, char *rs, char *Rs, zattr *txtchangep)
{
    struct buf_vars new_vars;

    if(!s)
	return ztrdup("");

    if ((termflags & TERM_UNKNOWN) && (unset(INTERACTIVE)))
        init_term();

    if (isset(PROMPTSUBST)) {
	int olderr = errflag;
	int oldval = lastval;

	s = dupstring(s);
	if (!parsestr(&s))
	    singsub(&s);
	/*
	 * We don't need the special Nularg hack here and we're
	 * going to be using Nularg for other things.
	 */
	if (*s == Nularg && s[1] == '\0')
	    *s = '\0';

	/*
	 * Ignore errors and status change in prompt substitution.
	 * However, keep any user interrupt error that occurred.
	 */
	errflag = olderr | (errflag & ERRFLAG_INT);
	lastval = oldval;
    }

    memset(&new_vars, 0, sizeof(new_vars));
    new_vars.last = bv;
    bv = &new_vars;

    new_vars.rstring = rs;
    new_vars.Rstring = Rs;
    new_vars.fm = s;
    new_vars.bufspc = 256;
    new_vars.bp = new_vars.bufline = new_vars.buf = zshcalloc(new_vars.bufspc);
    new_vars.bp1 = NULL;
    new_vars.truncwidth = 0;

    putpromptchar(1, '\0', txtchangep);
    addbufspc(2);
    if (new_vars.dontcount)
	*new_vars.bp++ = Outpar;
    *new_vars.bp = '\0';
    if (!ns) {
	/* If zero, Inpar, Outpar and Nularg should be removed. */
	for (new_vars.bp = new_vars.buf; *new_vars.bp; ) {
	    if (*new_vars.bp == Meta)
		new_vars.bp += 2;
	    else if (*new_vars.bp == Inpar || *new_vars.bp == Outpar ||
		     *new_vars.bp == Nularg)
		chuck(new_vars.bp);
	    else
		new_vars.bp++;
	}
    }

    bv = new_vars.last;

    return new_vars.buf;
}

/* Parse the argument for %F and %K */
static zattr
parsecolorchar(zattr arg, int is_fg)
{
    if (bv->fm[1] == '{') {
	char *ep;
	bv->fm += 2; /* skip over F{ */
	if ((ep = strchr(bv->fm, '}'))) {
	    char oc = *ep, *col, *coll;
	    int ops = opts[PROMPTSUBST], opb = opts[PROMPTBANG];
	    int opp = opts[PROMPTPERCENT];

	    opts[PROMPTPERCENT] = 1;
	    opts[PROMPTSUBST] = opts[PROMPTBANG] = 0;

	    *ep = '\0';
	    /* expand the contents of the argument so you can use
	     * %v for example */
	    coll = col = promptexpand(bv->fm, 0, NULL, NULL, NULL);
	    *ep = oc;
	    arg = match_colour((const char **)&coll, is_fg, 0);
	    free(col);
	    bv->fm = ep;

	    opts[PROMPTSUBST] = ops;
	    opts[PROMPTBANG] = opb;
	    opts[PROMPTPERCENT] = opp;
	} else {
	    arg = match_colour((const char **)&bv->fm, is_fg, 0);
	    if (*bv->fm != '}')
		bv->fm--;
	}
    } else
	arg = match_colour(NULL, is_fg, arg);
    return arg;
}

/* Perform %- and !-expansion as required on a section of the prompt.  The *
 * section is ended by an instance of endchar.  If doprint is 0, the valid *
 * % sequences are merely skipped over, and nothing is stored.             */

/**/
static int
putpromptchar(int doprint, int endchar, zattr *txtchangep)
{
    char *ss, *hostnam;
    int t0, arg, test, sep, j, numjobs, len;
    zattr atr;
    struct tm *tm;
    struct timespec ts;
    time_t timet;
    Nameddir nd;

    for (; *bv->fm && *bv->fm != endchar; bv->fm++) {
	arg = 0;
	if (*bv->fm == '%' && isset(PROMPTPERCENT)) {
	    int minus = 0;
	    bv->fm++;
	    if (*bv->fm == '-') {
		minus = 1;
		bv->fm++;
	    }
	    if (idigit(*bv->fm)) {
		arg = zstrtol(bv->fm, &bv->fm, 10);
		if (minus)
		    arg *= -1;
	    } else if (minus)
		arg = -1;
	    if (*bv->fm == '(') {
		int tc, otruncwidth;

		if (idigit(*++bv->fm)) {
		    arg = zstrtol(bv->fm, &bv->fm, 10);
		} else if (arg < 0) {
		    /* negative numbers don't make sense here */
		    arg *= -1;
		}
		test = 0;
		ss = pwd;
		switch (tc = *bv->fm) {
		case 'c':
		case '.':
		case '~':
		    if ((nd = finddir(ss))) {
			arg--;
			ss += strlen(nd->dir);
		    } /*FALLTHROUGH*/
		case '/':
		case 'C':
		    /* `/' gives 0, `/any' gives 1, etc. */
		    if (*ss && *ss++ == '/' && *ss)
			arg--;
		    for (; *ss; ss++)
			if (*ss == '/')
			    arg--;
		    if (arg <= 0)
			test = 1;
		    break;
		case 't':
		case 'T':
		case 'd':
		case 'D':
		case 'w':
		    timet = time(NULL);
		    tm = localtime(&timet);
		    switch (tc) {
		    case 't':
			test = (arg == tm->tm_min);
			break;
		    case 'T':
			test = (arg == tm->tm_hour);
			break;
		    case 'd':
			test = (arg == tm->tm_mday);
			break;
		    case 'D':
			test = (arg == tm->tm_mon);
			break;
		    case 'w':
			test = (arg == tm->tm_wday);
			break;
		    }
		    break;
		case '?':
		    if (lastval == arg)
			test = 1;
		    break;
		case '#':
		    if (geteuid() == (uid_t)arg)
			test = 1;
		    break;
		case 'g':
		    if (getegid() == (gid_t)arg)
			test = 1;
		    break;
		case 'j':
		    for (numjobs = 0, j = 1; j <= maxjob; j++)
			if (jobtab[j].stat && jobtab[j].procs &&
		    	    !(jobtab[j].stat & STAT_NOPRINT)) numjobs++;
		    if (numjobs >= arg)
		    	test = 1;
		    break;
		case 'l':
		    *bv->bp = '\0';
		    countprompt(bv->bufline, &t0, 0, 0);
		    if (minus)
			t0 = zterm_columns - t0;
		    if (t0 >= arg)
			test = 1;
		    break;
		case 'e':
		    {
			Funcstack fsptr = funcstack;
			test = arg;
			while (fsptr && test > 0) {
			    test--;
			    fsptr = fsptr->prev;
			}
			test = !test;
		    }
		    break;
		case 'L':
		    if (shlvl >= arg)
			test = 1;
		    break;
		case 'S':
		    if (time(NULL) - shtimer.tv_sec >= arg)
			test = 1;
		    break;
		case 'v':
		    if (arrlen_ge(psvar, arg))
			test = 1;
		    break;
		case 'V':
		    if (psvar && *psvar && arrlen_ge(psvar, arg)) {
			if (*psvar[(arg ? arg : 1) - 1])
			    test = 1;
		    }
		    break;
		case '_':
		    test = (cmdsp >= arg);
		    break;
		case '!':
		    test = privasserted();
		    break;
		default:
		    test = -1;
		    break;
		}
		if (!*bv->fm || !(sep = *++bv->fm))
		    return 0;
		bv->fm++;
		/* Don't do the current truncation until we get back */
		otruncwidth = bv->truncwidth;
		bv->truncwidth = 0;
		if (!putpromptchar(test == 1 && doprint, sep,
				   txtchangep) || !*++bv->fm ||
		    !putpromptchar(test == 0 && doprint, ')',
				   txtchangep)) {
		    bv->truncwidth = otruncwidth;
		    return 0;
		}
		bv->truncwidth = otruncwidth;
		continue;
	    }
	    if (!doprint)
		switch(*bv->fm) {
		  case '[':
		    while(idigit(*++bv->fm));
		    while(*++bv->fm != ']');
		    continue;
		  case '<':
		    while(*++bv->fm != '<');
		    continue;
		  case '>':
		    while(*++bv->fm != '>');
		    continue;
		  case 'D':
		    if(bv->fm[1]=='{')
			while(*++bv->fm != '}');
		    continue;
		  default:
		    continue;
		}
	    switch (*bv->fm) {
	    case '~':
		promptpath(pwd, arg, 1);
		break;
	    case 'd':
	    case '/':
		promptpath(pwd, arg, 0);
		break;
	    case 'c':
	    case '.':
		promptpath(pwd, arg ? arg : 1, 1);
		break;
	    case 'C':
		promptpath(pwd, arg ? arg : 1, 0);
		break;
	    case 'N':
		promptpath(scriptname ? scriptname : argzero, arg, 0);
		break;
	    case 'h':
	    case '!':
		addbufspc(DIGBUFSIZE);
		convbase(bv->bp, curhist, 10);
		bv->bp += strlen(bv->bp);
		break;
	    case 'j':
		for (numjobs = 0, j = 1; j <= maxjob; j++)
		    if (jobtab[j].stat && jobtab[j].procs &&
		    	!(jobtab[j].stat & STAT_NOPRINT)) numjobs++;
		addbufspc(DIGBUFSIZE);
		sprintf(bv->bp, "%d", numjobs);
		bv->bp += strlen(bv->bp);
		break;
	    case 'M':
		queue_signals();
		if ((hostnam = getsparam("HOST")))
		    stradd(hostnam);
		unqueue_signals();
		break;
	    case 'm':
		if (!arg)
		    arg++;
		queue_signals();
		if (!(hostnam = getsparam("HOST"))) {
		    unqueue_signals();
		    break;
		}
		if (arg < 0) {
		    for (ss = hostnam + strlen(hostnam); ss > hostnam; ss--)
			if (ss[-1] == '.' && !++arg)
			    break;
		    stradd(ss);
		} else {
		    for (ss = hostnam; *ss; ss++)
			if (*ss == '.' && !--arg)
			    break;
		    stradd(*ss ? dupstrpfx(hostnam, ss - hostnam) : hostnam);
		}
		unqueue_signals();
		break;
	    case 'S':
		txtchangeset(txtchangep, TXTSTANDOUT, TXTNOSTANDOUT);
		txtset(TXTSTANDOUT);
		tsetcap(TCSTANDOUTBEG, TSC_PROMPT);
		break;
	    case 's':
		txtchangeset(txtchangep, TXTNOSTANDOUT, TXTSTANDOUT);
		txtunset(TXTSTANDOUT);
		tsetcap(TCSTANDOUTEND, TSC_PROMPT|TSC_DIRTY);
		break;
	    case 'B':
		txtchangeset(txtchangep, TXTBOLDFACE, TXTNOBOLDFACE);
		txtset(TXTBOLDFACE);
		tsetcap(TCBOLDFACEBEG, TSC_PROMPT|TSC_DIRTY);
		break;
	    case 'b':
		txtchangeset(txtchangep, TXTNOBOLDFACE, TXTBOLDFACE);
		txtunset(TXTBOLDFACE);
		tsetcap(TCALLATTRSOFF, TSC_PROMPT|TSC_DIRTY);
		break;
	    case 'U':
		txtchangeset(txtchangep, TXTUNDERLINE, TXTNOUNDERLINE);
		txtset(TXTUNDERLINE);
		tsetcap(TCUNDERLINEBEG, TSC_PROMPT);
		break;
	    case 'u':
		txtchangeset(txtchangep, TXTNOUNDERLINE, TXTUNDERLINE);
		txtunset(TXTUNDERLINE);
		tsetcap(TCUNDERLINEEND, TSC_PROMPT|TSC_DIRTY);
		break;
	    case 'F':
		atr = parsecolorchar(arg, 1);
		if (!(atr & (TXT_ERROR | TXTNOFGCOLOUR))) {
		    txtchangeset(txtchangep, atr & TXT_ATTR_FG_ON_MASK,
				 TXTNOFGCOLOUR | TXT_ATTR_FG_COL_MASK);
		    txtunset(TXT_ATTR_FG_COL_MASK);
		    txtset(atr & TXT_ATTR_FG_ON_MASK);
		    set_colour_attribute(atr, COL_SEQ_FG, TSC_PROMPT);
		    break;
		}
		/* else FALLTHROUGH */
	    case 'f':
		txtchangeset(txtchangep, TXTNOFGCOLOUR, TXT_ATTR_FG_ON_MASK);
		txtunset(TXT_ATTR_FG_ON_MASK);
		set_colour_attribute(TXTNOFGCOLOUR, COL_SEQ_FG, TSC_PROMPT);
		break;
	    case 'K':
		atr = parsecolorchar(arg, 0);
		if (!(atr & (TXT_ERROR | TXTNOBGCOLOUR))) {
		    txtchangeset(txtchangep, atr & TXT_ATTR_BG_ON_MASK,
				 TXTNOBGCOLOUR | TXT_ATTR_BG_COL_MASK);
		    txtunset(TXT_ATTR_BG_COL_MASK);
		    txtset(atr & TXT_ATTR_BG_ON_MASK);
		    set_colour_attribute(atr, COL_SEQ_BG, TSC_PROMPT);
		    break;
		}
		/* else FALLTHROUGH */
	    case 'k':
		txtchangeset(txtchangep, TXTNOBGCOLOUR, TXT_ATTR_BG_ON_MASK);
		txtunset(TXT_ATTR_BG_ON_MASK);
		set_colour_attribute(TXTNOBGCOLOUR, COL_SEQ_BG, TSC_PROMPT);
		break;
	    case '[':
		if (idigit(*++bv->fm))
		    arg = zstrtol(bv->fm, &bv->fm, 10);
		if (!prompttrunc(arg, ']', doprint, endchar, txtchangep))
		    return *bv->fm;
		break;
	    case '<':
	    case '>':
		/* Test (minus) here so -0 means "at the right margin" */
		if (minus) {
		    *bv->bp = '\0';
		    countprompt(bv->bufline, &t0, 0, 0);
		    arg = zterm_columns - t0 + arg;
		    if (arg <= 0)
			arg = 1;
		}
		if (!prompttrunc(arg, *bv->fm, doprint, endchar, txtchangep))
		    return *bv->fm;
		break;
	    case '{': /*}*/
		if (!bv->dontcount++) {
		    addbufspc(1);
		    *bv->bp++ = Inpar;
		}
		if (arg <= 0)
		    break;
		/* else */
		/* FALLTHROUGH */
	    case 'G':
		if (arg > 0) {
		    addbufspc(arg);
		    while (arg--)
			*bv->bp++ = Nularg;
		} else {
		    addbufspc(1);
		    *bv->bp++ = Nularg;
		}
		break;
	    case /*{*/ '}':
		if (bv->trunccount && bv->trunccount >= bv->dontcount)
		    return *bv->fm;
		if (bv->dontcount && !--bv->dontcount) {
		    addbufspc(1);
		    *bv->bp++ = Outpar;
		}
		break;
	    case 't':
	    case '@':
	    case 'T':
	    case '*':
	    case 'w':
	    case 'W':
	    case 'D':
		{
		    char *tmfmt, *dd, *tmbuf = NULL;

		    switch (*bv->fm) {
		    case 'T':
			tmfmt = "%K:%M";
			break;
		    case '*':
			tmfmt = "%K:%M:%S";
			break;
		    case 'w':
			tmfmt = "%a %f";
			break;
		    case 'W':
			tmfmt = "%m/%d/%y";
			break;
		    case 'D':
			if (bv->fm[1] == '{' /*}*/) {
			    for (ss = bv->fm + 2; *ss && *ss != /*{*/ '}'; ss++)
				if(*ss == '\\' && ss[1])
				    ss++;
			    dd = tmfmt = tmbuf = zalloc(ss - bv->fm);
			    for (ss = bv->fm + 2; *ss && *ss != /*{*/ '}';
				 ss++) {
				if(*ss == '\\' && ss[1])
				    ss++;
				*dd++ = *ss;
			    }
			    *dd = 0;
			    bv->fm = ss - !*ss;
			    if (!*tmfmt) {
				free(tmbuf);
				continue;
			    }
			} else
			    tmfmt = "%y-%m-%d";
			break;
		    default:
			tmfmt = "%l:%M%p";
			break;
		    }
		    zgettime(&ts);
		    tm = localtime(&ts.tv_sec);
		    /*
		     * Hack because strftime won't say how
		     * much space it actually needs.  Try to add it
		     * a few times until it works.  Some formats don't
		     * actually have a length, so we could go on for
		     * ever.
		     */
		    for(j = 0, t0 = strlen(tmfmt)*8; j < 3; j++, t0*=2) {
			addbufspc(t0);
			if ((len = ztrftime(bv->bp, t0, tmfmt, tm, ts.tv_nsec))
			    >= 0)
			    break;
		    }
		    /* There is enough room for this because addbufspc(t0)
		     * allocates room for t0 * 2 bytes. */
		    if (len >= 0)
			metafy(bv->bp, len, META_NOALLOC);
		    bv->bp += strlen(bv->bp);
		    zsfree(tmbuf);
		    break;
		}
	    case 'n':
		stradd(get_username());
		break;
	    case 'l':
		if (*ttystrname) {
                   ss = (strncmp(ttystrname, "/dev/tty", 8) ?
                           ttystrname + 5 : ttystrname + 8);
		    stradd(ss);
		} else
		    stradd("()");
		break;
	    case 'y':
		if (*ttystrname) {
		    ss = (strncmp(ttystrname, "/dev/", 5) ?
			    ttystrname : ttystrname + 5);
		    stradd(ss);
		} else
		    stradd("()");
		break;
	    case 'L':
		addbufspc(DIGBUFSIZE);
#if defined(ZLONG_IS_LONG_LONG) && defined(PRINTF_HAS_LLD)
		sprintf(bv->bp, "%lld", shlvl);
#else
		sprintf(bv->bp, "%ld", (long)shlvl);
#endif
		bv->bp += strlen(bv->bp);
		break;
	    case '?':
		addbufspc(DIGBUFSIZE);
#if defined(ZLONG_IS_LONG_LONG) && defined(PRINTF_HAS_LLD)
		sprintf(bv->bp, "%lld", lastval);
#else
		sprintf(bv->bp, "%ld", (long)lastval);
#endif
		bv->bp += strlen(bv->bp);
		break;
	    case '%':
	    case ')':
		addbufspc(1);
		*bv->bp++ = *bv->fm;
		break;
	    case '#':
		addbufspc(1);
		*bv->bp++ = privasserted() ? '#' : '%';
		break;
	    case 'v':
		if (!arg)
		    arg = 1;
		else if (arg < 0)
		    arg += arrlen(psvar) + 1;
		if (arg > 0 && arrlen_ge(psvar, arg))
		    stradd(psvar[arg - 1]);
		break;
	    case 'E':
                tsetcap(TCCLEAREOL, TSC_PROMPT);
		break;
	    case '^':
		if (cmdsp) {
		    if (arg >= 0) {
			if (arg > cmdsp || arg == 0)
			    arg = cmdsp;
			for (t0 = cmdsp - 1; arg--; t0--) {
			    stradd(cmdnames[cmdstack[t0]]);
			    if (arg) {
				addbufspc(1);
				*bv->bp++=' ';
			    }
			}
		    } else {
			arg = -arg;
			if (arg > cmdsp)
			    arg = cmdsp;
			for (t0 = arg - 1; arg--; t0--) {
			    stradd(cmdnames[cmdstack[t0]]);
			    if (arg) {
				addbufspc(1);
				*bv->bp++=' ';
			    }
			}
		    }
		}
		break;
	    case '_':
		if (cmdsp) {
		    if (arg >= 0) {
			if (arg > cmdsp || arg == 0)
			    arg = cmdsp;
			for (t0 = cmdsp - arg; arg--; t0++) {
			    stradd(cmdnames[cmdstack[t0]]);
			    if (arg) {
				addbufspc(1);
				*bv->bp++=' ';
			    }
			}
		    } else {
			arg = -arg;
			if (arg > cmdsp)
			    arg = cmdsp;
			for (t0 = 0; arg--; t0++) {
			    stradd(cmdnames[cmdstack[t0]]);
			    if (arg) {
				addbufspc(1);
				*bv->bp++=' ';
			    }
			}
		    }
		}
		break;
	    case 'r':
		if(bv->rstring)
		    stradd(bv->rstring);
		break;
	    case 'R':
		if(bv->Rstring)
		    stradd(bv->Rstring);
		break;
	    case 'e':
	    {
		int depth = 0;
		Funcstack fsptr = funcstack;
		while (fsptr) {
		    depth++;
		    fsptr = fsptr->prev;
		}
		addbufspc(DIGBUFSIZE);
		sprintf(bv->bp, "%d", depth);
		bv->bp += strlen(bv->bp);
		break;
	    }
	    case 'I':
		if (funcstack && funcstack->tp != FS_SOURCE &&
		    !IN_EVAL_TRAP()) {
		    /*
		     * We're in a function or an eval with
		     * EVALLINENO.  Calculate the line number in
		     * the file.
		     */
		    zlong flineno = lineno + funcstack->flineno;
		    /* take account of eval line nos. starting at 1 */
		    if (funcstack->tp == FS_EVAL)
			lineno--;
		    addbufspc(DIGBUFSIZE);
#if defined(ZLONG_IS_LONG_LONG) && defined(PRINTF_HAS_LLD)
		    sprintf(bv->bp, "%lld", flineno);
#else
		    sprintf(bv->bp, "%ld", (long)flineno);
#endif
		    bv->bp += strlen(bv->bp);
		    break;
		}
		/* else we're in a file and lineno is already correct */
		/* FALLTHROUGH */
	    case 'i':
		addbufspc(DIGBUFSIZE);
#if defined(ZLONG_IS_LONG_LONG) && defined(PRINTF_HAS_LLD)
		sprintf(bv->bp, "%lld", lineno);
#else
		sprintf(bv->bp, "%ld", (long)lineno);
#endif
		bv->bp += strlen(bv->bp);
		break;
	    case 'x':
		if (funcstack && funcstack->tp != FS_SOURCE &&
		    !IN_EVAL_TRAP())
		    promptpath(funcstack->filename ? funcstack->filename : "",
			       arg, 0);
		else
		    promptpath(scriptfilename ? scriptfilename : argzero,
			       arg, 0);
		break;
	    case '\0':
		return 0;
	    case Meta:
		bv->fm++;
		break;
	    }
	} else if(*bv->fm == '!' && isset(PROMPTBANG)) {
	    if(doprint) {
		if(bv->fm[1] == '!') {
		    bv->fm++;
		    addbufspc(1);
		    pputc('!');
		} else {
		    addbufspc(DIGBUFSIZE);
		    convbase(bv->bp, curhist, 10);
		    bv->bp += strlen(bv->bp);
		}
	    }
	} else {
	    char c = *bv->fm == Meta ? *++bv->fm ^ 32 : *bv->fm;

	    if (doprint) {
		addbufspc(1);
		pputc(c);
	    }
	}
    }

    return *bv->fm;
}

/* pputc adds a character to the buffer, metafying.  There must *
 * already be space.                                            */

/**/
static void
pputc(char c)
{
    if (imeta(c)) {
	*bv->bp++ = Meta;
	c ^= 32;
    }
    *bv->bp++ = c;
    if (c == '\n' && !bv->dontcount)
	bv->bufline = bv->bp;
}

/* Make sure there is room for `need' more characters in the buffer. */

/**/
static void
addbufspc(int need)
{
    need *= 2;   /* for metafication */
    if((bv->bp - bv->buf) + need > bv->bufspc) {
	int bo = bv->bp - bv->buf;
	int bo1 = bv->bp1 ? bv->bp1 - bv->buf : -1;
	ptrdiff_t bufline_off = bv->bufline ? bv->bufline - bv->buf : -1;

	if(need & 255)
	    need = (need | 255) + 1;
	bv->buf = realloc(bv->buf, bv->bufspc += need);
	memset(bv->buf + bv->bufspc - need, 0, need);
	bv->bp = bv->buf + bo;
	if(bo1 != -1)
	    bv->bp1 = bv->buf + bo1;
	if (bufline_off != -1)
	    bv->bufline = bv->buf + bufline_off;
    }
}

/* stradd() adds a metafied string to the prompt, *
 * in a visible representation.                   */

/**/
void
stradd(char *d)
{
#ifdef MULTIBYTE_SUPPORT
    char *ums, *ups;
    int upslen, eol = 0;
    mbstate_t mbs;

    memset(&mbs, 0, sizeof mbs);
    ums = ztrdup(d);
    ups = unmetafy(ums, &upslen);

    /*
     * We now have a raw string of possibly multibyte characters.
     * Read each character one by one.
     */
    while (upslen > 0) {
	wchar_t cc;
	char *pc;
	size_t cnt = eol ? MB_INVALID : mbrtowc(&cc, ups, upslen, &mbs);

	switch (cnt) {
	case MB_INCOMPLETE:
	    eol = 1;
	    /* FALL THROUGH */
	case MB_INVALID:
	    /* Bad character.  Take the next byte on its own. */
	    pc = nicechar(*ups);
	    cnt = 1;
	    memset(&mbs, 0, sizeof mbs);
	    break;
	case 0:
	    cnt = 1;
	    /* FALL THROUGH */
	default:
	    /* Take full wide character in one go */
	    mb_charinit();
	    pc = wcs_nicechar(cc, NULL, NULL);
	    break;
	}
	/* Keep output as metafied string. */
	addbufspc(strlen(pc));

	upslen -= cnt;
	ups += cnt;

	/* Put printed representation into the buffer */
	while (*pc)
	    *bv->bp++ = *pc++;
    }

    free(ums);
#else
    char *ps, *pc;
    addbufspc(niceztrlen(d));
    /* This loop puts the nice representation of the string into the
     * prompt buffer. */
    for (ps = d; *ps; ps++) {
	for (pc = nicechar(*ps == Meta ? *++ps^32 : *ps); *pc; pc++)
	    *bv->bp++ = *pc;
    }
#endif
}

/* tsetcap(), among other things, can write a termcap string into the buffer. */

/**/
mod_export void
tsetcap(int cap, int flags)
{
    if (tccan(cap) && !isset(SINGLELINEZLE) &&
        !(termflags & (TERM_NOUP|TERM_BAD|TERM_UNKNOWN))) {
	switch (flags & TSC_OUTPUT_MASK) {
	case TSC_RAW:
	    tputs(tcstr[cap], 1, putraw);
	    break;
	case 0:
	default:
	    tputs(tcstr[cap], 1, putshout);
	    break;
	case TSC_PROMPT:
	    if (!bv->dontcount) {
		addbufspc(1);
		*bv->bp++ = Inpar;
	    }
	    tputs(tcstr[cap], 1, putstr);
	    if (!bv->dontcount) {
		int glitch = 0;

		if (cap == TCSTANDOUTBEG || cap == TCSTANDOUTEND)
		    glitch = tgetnum("sg");
		else if (cap == TCUNDERLINEBEG || cap == TCUNDERLINEEND)
		    glitch = tgetnum("ug");
		if(glitch < 0)
		    glitch = 0;
		addbufspc(glitch + 1);
		while(glitch--)
		    *bv->bp++ = Nularg;
		*bv->bp++ = Outpar;
	    }
	    break;
	}

	if (flags & TSC_DIRTY) {
	    flags &= ~TSC_DIRTY;
	    if (txtisset(TXTBOLDFACE) && cap != TCBOLDFACEBEG)
		tsetcap(TCBOLDFACEBEG, flags);
	    if (txtisset(TXTSTANDOUT))
		tsetcap(TCSTANDOUTBEG, flags);
	    if (txtisset(TXTUNDERLINE))
		tsetcap(TCUNDERLINEBEG, flags);
	    if (txtisset(TXTFGCOLOUR))
		set_colour_attribute(txtattrmask, COL_SEQ_FG, flags);
	    if (txtisset(TXTBGCOLOUR))
		set_colour_attribute(txtattrmask, COL_SEQ_BG, flags);
	}
    }
}

/**/
int
putstr(int d)
{
    addbufspc(1);
    pputc(d);
    return 0;
}

/*
 * Count height etc. of a prompt string returned by promptexpand().
 * This depends on the current terminal width, and tabs and
 * newlines require nontrivial processing.
 * Passing `overf' as -1 means to ignore columns (absolute width).
 *
 * If multibyte is enabled, take account of multibyte characters
 * by locating them and finding out their screen width.
 */

/**/
mod_export void
countprompt(char *str, int *wp, int *hp, int overf)
{
    int w = 0, h = 1, multi = 0, wcw = 0;
    int s = 1;
#ifdef MULTIBYTE_SUPPORT
    char inchar;
    mbstate_t mbs;
    wchar_t wc;

    memset(&mbs, 0, sizeof(mbs));
#endif

    for (; *str; str++) {
	/*
	 * Avoid double-incrementing the height when there's a newline in the
	 * prompt and the line it terminates takes up exactly the width of the
	 * terminal
	 */
	while (w > zterm_columns && overf >= 0 && !multi) {
	    h++;
	    if (wcw) {
		/*
		 * Wide characters don't get split off. They move to the
		 * next line if there is not enough space.
		 */
		w = wcw;
		break;
	    } else {
		/*
		 * Tabs overflow to the next line as if they were made of spaces.
		 */
		w -= zterm_columns;
	    }
	}
	wcw = 0;
	/*
	 * Input string should be metafied, so tokens in it should
	 * be real tokens, even if there are multibyte characters.
	 */
	if (*str == Inpar)
	    s = 0;
	else if (*str == Outpar)
	    s = 1;
	else if (*str == Nularg)
	    w++;
	else if (s) {
	    if (*str == Meta) {
#ifdef MULTIBYTE_SUPPORT
		inchar = *++str ^ 32;
#else
		str++;
#endif
	    } else {
#ifdef MULTIBYTE_SUPPORT
		/*
		 * Don't look for tab or newline in the middle
		 * of a multibyte character.  Otherwise, we are
		 * relying on the character set being an extension
		 * of ASCII so it's safe to test a single byte.
		 */
		if (!multi) {
#endif
		    if (*str == '\t') {
			w = (w | 7) + 1;
			continue;
		    } else if (*str == '\n') {
			w = 0;
			h++;
			continue;
		    }
#ifdef MULTIBYTE_SUPPORT
		}

		inchar = *str;
#endif
	    }

#ifdef MULTIBYTE_SUPPORT
	    switch (mbrtowc(&wc, &inchar, 1, &mbs)) {
	    case MB_INCOMPLETE:
		/* Character is incomplete -- keep looking. */
		multi = 1;
		break;
	    case MB_INVALID:
		memset(&mbs, 0, sizeof mbs);
		/* Invalid character: assume single width. */
		multi = 0;
		w++;
		break;
	    case 0:
		multi = 0;
		break;
	    default:
		/*
		 * If the character isn't printable, WCWIDTH() returns
		 * -1.  We assume width 1.
		 */
		wcw = WCWIDTH(wc);
		if (wcw >= 0)
		    w += wcw;
		else
		    w++;
		multi = 0;
		break;
	    }
#else
	    w++;
#endif
	}
    }
    /*
     * multi may still be set if we were in the middle of the character.
     * This isn't easy to handle generally; just assume there's no
     * output.
     */
    while (w > zterm_columns && overf >= 0) {
	h++;
	if (wcw) {
	    w = wcw;
	    break;
	} else {
	    w -= zterm_columns;
	}
    }
    if (w == zterm_columns && overf == 0) {
	w = 0;
	h++;
    }
    if(wp)
	*wp = w;
    if(hp)
	*hp = h;
}

/**/
static int
prompttrunc(int arg, int truncchar, int doprint, int endchar,
	    zattr *txtchangep)
{
    if (arg > 0) {
	char ch = *bv->fm, *ptr, *truncstr;
	int truncatleft = ch == '<';
	int w = bv->bp - bv->buf;

	/*
	 * If there is already a truncation active, return so that
	 * can be finished, backing up so that the new truncation
	 * can be started afterwards.
	 */
	if (bv->truncwidth) {
	    while (*--bv->fm != '%')
		;
	    bv->fm--;
	    return 0;
	}

	bv->truncwidth = arg;
	if (*bv->fm != ']')
	    bv->fm++;
	while (*bv->fm && *bv->fm != truncchar) {
	    if (*bv->fm == '\\' && bv->fm[1])
		++bv->fm;
	    addbufspc(1);
	    *bv->bp++ = *bv->fm++;
	}
	if (!*bv->fm)
	    return 0;
	if (bv->bp - bv->buf == w && truncchar == ']') {
	    addbufspc(1);
	    *bv->bp++ = '<';
	}
	ptr = bv->buf + w;	/* addbufspc() may have realloc()'d bv->buf */
	/*
	 * Now:
	 *   bv->buf is the start of the output prompt buffer
	 *   ptr is the start of the truncation string
	 *   bv->bp is the end of the truncation string
	 */
	truncstr = ztrduppfx(ptr, bv->bp - ptr);

	bv->bp = ptr;
	w = bv->bp - bv->buf;
	bv->fm++;
	bv->trunccount = bv->dontcount;
	putpromptchar(doprint, endchar, txtchangep);
	bv->trunccount = 0;
	ptr = bv->buf + w;	/* putpromptchar() may have realloc()'d */
	*bv->bp = '\0';
	/*
	 * Now:
	 *   ptr is the start of the truncation string and also
	 *     where we need to start putting any truncated output
	 *   bv->bp is the end of the string we have just added, which
	 *     may need truncating.
	 */

	/*
	 * w below is screen width if multibyte support is enabled
	 * (note that above it was a raw string pointer difference).
	 * It's the full width of the string we may need to truncate.
	 *
	 * bv->truncwidth has come from the user, so we interpret this
	 * as a screen width, too.
	 */
	countprompt(ptr, &w, 0, -1);
	if (w > bv->truncwidth) {
	    /*
	     * We need to truncate.  t points to the truncation string
	     * -- which is inserted literally, without nice
	     * representation.  twidth is its printing width, and maxwidth
	     * is the amount of the main string that we want to keep.
	     * Note that if the truncation string is longer than the
	     * truncation length (twidth > bv->truncwidth), the truncation
	     * string is used in full.
	     */
	    char *t = truncstr;
	    int fullen = bv->bp - ptr;
	    int twidth, maxwidth;
	    int ntrunc = strlen(t);

	    twidth = MB_METASTRWIDTH(t);
	    if (twidth < bv->truncwidth) {
		maxwidth = bv->truncwidth - twidth;
		/*
		 * It's not safe to assume there are no invisible substrings
		 * just because the width is less than the full string
		 * length since there may be multibyte characters.
		 */
		addbufspc(ntrunc+1);
		/* may have realloc'd */
		ptr = bv->bp - fullen;

		if (truncatleft) {
		    /*
		     * To truncate at the left, selectively copy
		     * maxwidth bytes from the main prompt, preceded
		     * by the truncation string in full.
		     *
		     * We're overwriting the string containing the
		     * text to be truncated, so copy it.  We've
		     * just ensured there's sufficient space at the
		     * end of the prompt string.
		     *
		     * Pointer into text to be truncated.
		     */
		    char *fulltextptr, *fulltext;
		    int remw;
#ifdef MULTIBYTE_SUPPORT
		    mbstate_t mbs;
		    memset(&mbs, 0, sizeof mbs);
#endif

		    fulltextptr = fulltext = ptr + ntrunc;
		    memmove(fulltext, ptr, fullen);
		    fulltext[fullen] = '\0';

		    /* Copy the truncstr into place. */
		    while (*t)
			*ptr++ = *t++;

		    /*
		     * Find the point in the text at which we should
		     * start copying, i.e. when the remaining width
		     * is less than or equal to the maximum width.
		     */
		    remw = w;
		    while (remw > maxwidth && *fulltextptr) {
			if (*fulltextptr == Inpar) {
			    /*
			     * Text marked as invisible: copy
			     * regardless, since we don't know what
			     * this does.  It only affects the width
			     * if there are Nularg's present.
			     * However, even in that case we
			     * can't break the sequence down, so
			     * we still loop over the entire group.
			     */
			    for (;;) {
				*ptr++ = *fulltextptr;
				if (*fulltextptr == '\0' ||
				    *fulltextptr++ == Outpar)
				    break;
				if (fulltextptr[-1] == Nularg)
				    remw--;
			    }
			} else {
#ifdef MULTIBYTE_SUPPORT
			    /*
			     * Normal text: build up a multibyte character.
			     */
			    char inchar;
			    wchar_t cc;
			    int wcw;

			    /*
			     * careful: string is still metafied (we
			     * need that because we don't know a
			     * priori when to stop and the resulting
			     * string must be metafied).
			     */
			    if (*fulltextptr == Meta)
				inchar = *++fulltextptr ^ 32;
			    else
				inchar = *fulltextptr;
			    fulltextptr++;
			    switch (mbrtowc(&cc, &inchar, 1, &mbs)) {
			    case MB_INCOMPLETE:
				/* Incomplete multibyte character. */
				break;
			    case MB_INVALID:
				/* Reset invalid state. */
				memset(&mbs, 0, sizeof mbs);
				/* FALL THROUGH */
			    case 0:
				/* Assume a single-byte character. */
				remw--;
				break;
			    default:
				wcw = WCWIDTH(cc);
				if (wcw >= 0)
				    remw -= wcw;
				else
				    remw--;
				break;
			    }
#else
			    /* Single byte character */
			    if (*fulltextptr == Meta)
				fulltextptr++;
			    fulltextptr++;
			    remw--;
#endif
			}
		    }

		    /*
		     * Now simply copy the rest of the text.  Still
		     * metafied, so this is easy.
		     */
		    while (*fulltextptr)
			*ptr++ = *fulltextptr++;
		    /* Mark the end of copying */
		    bv->bp = ptr;
		} else {
		    /*
		     * Truncating at the right is easier: just leave
		     * enough characters until we have reached the
		     * maximum width.
		     */
		    char *skiptext = ptr;
#ifdef MULTIBYTE_SUPPORT
		    mbstate_t mbs;
		    memset(&mbs, 0, sizeof mbs);
#endif

		    while (maxwidth > 0 && *skiptext) {
			if (*skiptext == Inpar) {
			    /* see comment on left truncation above */
			    for (;;) {
				if (*skiptext == '\0' ||
				    *skiptext++ == Outpar)
				    break;
				if (skiptext[-1] == Nularg)
				    maxwidth--;
			    }
			} else {
#ifdef MULTIBYTE_SUPPORT
			    char inchar;
			    wchar_t cc;
			    int wcw;

			    if (*skiptext == Meta)
				inchar = *++skiptext ^ 32;
			    else
				inchar = *skiptext;
			    skiptext++;
			    switch (mbrtowc(&cc, &inchar, 1, &mbs)) {
			    case MB_INCOMPLETE:
				/* Incomplete character. */
				break;
			    case MB_INVALID:
				/* Reset invalid state. */
				memset(&mbs, 0, sizeof mbs);
				/* FALL THROUGH */
			    case 0:
				/* Assume a single-byte character. */
				maxwidth--;
				break;
			    default:
				wcw = WCWIDTH(cc);
				if (wcw >= 0)
				    maxwidth -= wcw;
				else
				    maxwidth--;
				break;
			    }
#else
			    if (*skiptext == Meta)
				skiptext++;
			    skiptext++;
			    maxwidth--;
#endif
			}
		    }
		    /*
		     * We don't need the visible text from now on,
		     * but we'd better copy any invisible bits.
		     * History dictates that these go after the
		     * truncation string.  This is sensible since
		     * they may, for example, turn off an effect which
		     * should apply to all text at this point.
		     *
		     * Copy the truncstr.
		     */
		    ptr = skiptext;
		    while (*t)
			*ptr++ = *t++;
		    bv->bp = ptr;
		    if (*skiptext) {
			/* Move remaining text so we don't overwrite it */
			memmove(bv->bp, skiptext, strlen(skiptext)+1);
			skiptext = bv->bp;

			/*
			 * Copy anything we want, updating bv->bp
			 */
			while (*skiptext) {
			    if (*skiptext == Inpar) {
				for (;;) {
				    *bv->bp++ = *skiptext;
				    if (*skiptext == Outpar ||
					*skiptext == '\0')
					break;
				    skiptext++;
				}
			    }
			    else
				skiptext++;
			}
		    }
		}
	    } else {
		/* Just copy truncstr; no other text appears. */
		while (*t)
		    *ptr++ = *t++;
		bv->bp = ptr;
	    }
	    *bv->bp = '\0';
	}
	zsfree(truncstr);
	bv->truncwidth = 0;
	/*
	 * We may have returned early from the previous putpromptchar *
	 * because we found another truncation following this one.    *
	 * In that case we need to do the rest now.                   *
	 */
	if (!*bv->fm)
	    return 0;
	if (*bv->fm != endchar) {
	    bv->fm++;
	    /*
	     * With bv->truncwidth set to zero, we always reach endchar *
	     * (or the terminating NULL) this time round.         *
	     */
	    if (!putpromptchar(doprint, endchar, txtchangep))
		return 0;
	}
	/* Now we have to trick it into matching endchar again */
	bv->fm--;
    } else {
	if (*bv->fm != endchar)
	    bv->fm++;
	while(*bv->fm && *bv->fm != truncchar) {
	    if (*bv->fm == '\\' && bv->fm[1])
		bv->fm++;
	    bv->fm++;
	}
	if (bv->truncwidth || !*bv->fm)
	    return 0;
    }
    return 1;
}

/**/
void
cmdpush(int cmdtok)
{
    if (cmdsp >= 0 && cmdsp < CMDSTACKSZ)
	cmdstack[cmdsp++] = (unsigned char)cmdtok;
}

/**/
void
cmdpop(void)
{
    if (cmdsp <= 0) {
	DPUTS(1, "BUG: cmdstack empty");
	fflush(stderr);
    } else
	cmdsp--;
}


/*****************************************************************************
 * Utilities dealing with colour and other forms of highlighting.
 *
 * These are shared by prompts and by zle, so it's easiest to have them
 * in the main shell.
 *****************************************************************************/

/* Defines standard ANSI colour names in index order */
static const char *ansi_colours[] = {
    "black", "red", "green", "yellow", "blue", "magenta", "cyan", "white",
    "default", NULL
};

/* Defines the available types of highlighting */
struct highlight {
    const char *name;
    zattr mask_on;
    zattr mask_off;
};

static const struct highlight highlights[] = {
    { "none", 0, TXT_ATTR_ON_MASK },
    { "bold", TXTBOLDFACE, 0 },
    { "standout", TXTSTANDOUT, 0 },
    { "underline", TXTUNDERLINE, 0 },
    { NULL, 0, 0 }
};

/*
 * Return index of ANSI colour for which *teststrp is an abbreviation.
 * Any non-alphabetic character ends the abbreviation.
 * 8 is the special value for default (note this is *not* the
 * right sequence for default which is typically 9).
 * -1 is failure.
 */

static int
match_named_colour(const char **teststrp)
{
    const char *teststr = *teststrp, *end, **cptr;
    int len;

    for (end = teststr; ialpha(*end); end++)
	;
    len = end - teststr;
    *teststrp = end;

    for (cptr = ansi_colours; *cptr; cptr++) {
	if (!strncmp(teststr, *cptr, len))
	    return cptr - ansi_colours;
    }

    return -1;
}

/*
 * Match just the colour part of a highlight specification.
 * If teststrp is NULL, use the already parsed numeric colour.
 * Return the attributes to set in the attribute variable.
 * Return -1 for out of range.  Does not check the character
 * following the colour specification.
 */

/**/
mod_export zattr
match_colour(const char **teststrp, int is_fg, int colour)
{
    int shft, named = 0, tc;
    zattr on;

    if (is_fg) {
	shft = TXT_ATTR_FG_COL_SHIFT;
	on = TXTFGCOLOUR;
	tc = TCFGCOLOUR;
    } else {
	shft = TXT_ATTR_BG_COL_SHIFT;
	on = TXTBGCOLOUR;
	tc = TCBGCOLOUR;
    }
    if (teststrp) {
	if (**teststrp == '#' && isxdigit(STOUC((*teststrp)[1]))) {
	    struct color_rgb color;
	    char *end;
	    zlong col = zstrtol(*teststrp+1, &end, 16);
            if (end - *teststrp == 4) {
		color.red = col >> 8 | ((col >> 8) << 4);
		color.green = (col & 0xf0) >> 4;
		color.green |= color.green << 4;
		color.blue = col & 0xf;
		color.blue |= color.blue << 4;
	    } else if (end - *teststrp == 7) {
		color.red = col >> 16;
		color.green = (col & 0xff00) >> 8;
		color.blue = col & 0xff;
	    } else
		return TXT_ERROR;
	    *teststrp = end;
	    colour = runhookdef(GETCOLORATTR, &color) - 1;
	    if (colour == -1) { /* no hook function added, try true color (24-bit) */
		colour = (((color.red << 8) + color.green) << 8) + color.blue;
		return on | (is_fg ? TXT_ATTR_FG_24BIT : TXT_ATTR_BG_24BIT) |
			(zattr)colour << shft;
	    } else if (colour <= -2) {
		return TXT_ERROR;
	    }
	} else if ((named = ialpha(**teststrp))) {
	    colour = match_named_colour(teststrp);
	    if (colour == 8) {
		/* default */
		return is_fg ? TXTNOFGCOLOUR : TXTNOBGCOLOUR;
	    }
	    if (colour < 0)
		return TXT_ERROR;
	}
	else {
	    colour = (int)zstrtol(*teststrp, (char **)teststrp, 10);
	    if (colour < 0 || colour >= 256)
		return TXT_ERROR;
	}
    }

    /* Out of range of termcap colours and basic ANSI set. */
    if (tccan(tc) && colour > 7 && colour >= tccolours)
	return TXT_ERROR;

    return on | (zattr)colour << shft;
}

/*
 * Match a set of highlights in the given teststr.
 * Set *on_var to reflect the values found.
 * Return a pointer to the first character not consumed.
 */

/**/
mod_export const char *
match_highlight(const char *teststr, zattr *on_var)
{
    int found = 1;

    *on_var = 0;
    while (found && *teststr) {
	const struct highlight *hl;

	found = 0;
	if (strpfx("fg=", teststr) || strpfx("bg=", teststr)) {
	    int is_fg = (teststr[0] == 'f');
	    zattr atr;

	    teststr += 3;
	    atr = match_colour(&teststr, is_fg, 0);
	    if (*teststr == ',')
		teststr++;
	    else if (*teststr && *teststr != ' ')
		break;
	    found = 1;
	    /* skip out of range colours but keep scanning attributes */
	    if (atr != TXT_ERROR)
		*on_var |= atr;
	} else {
	    for (hl = highlights; hl->name; hl++) {
		if (strpfx(hl->name, teststr)) {
		    const char *val = teststr + strlen(hl->name);

		    if (*val == ',')
			val++;
		    else if (*val && *val != ' ')
			break;

		    *on_var |= hl->mask_on;
		    *on_var &= ~hl->mask_off;
		    teststr = val;
		    found = 1;
		}
	    }
	}
    }

    return teststr;
}

/*
 * Count or output a string for colour information: used
 * by output_highlight(). count when buf is NULL.
 * returned count excludes the terminating null byte.
 */

static int
output_colour(int colour, int fg_bg, int truecol, char *buf)
{
    int atrlen = 3, len;
    char *ptr = buf;
    if (buf) {
	strcpy(ptr, fg_bg == COL_SEQ_FG ? "fg=" : "bg=");
	ptr += 3;
    }
    if (truecol) {
	/* length of hex triplet always 7, don't need sprintf to count */
	atrlen += buf ? sprintf(ptr, "#%02x%02x%02x", colour >> 16,
		(colour >> 8) & 0xff, colour & 0xff) : 7;
    /* colour should only be > 7 if using termcap but let's be safe. Update:
     * currently other places in code don't always imply that colour > 7 =>
     * using-termcap - if zle_highlight will be non-default, then it will be
     * used instead of termcap even for colour > 7. Here this just emits the
     * color number, so it works fine for both zle_highlight and tercap cases
     */
    } else if (colour > 7) {
	char digbuf[DIGBUFSIZE];
	sprintf(digbuf, "%d", colour);
	len = strlen(digbuf);
	atrlen += len;
	if (buf)
	    strcpy(ptr, digbuf);
    } else {
	len = strlen(ansi_colours[colour]);
	atrlen += len;
	if (buf)
	    strcpy(ptr, ansi_colours[colour]);
    }

    return atrlen;
}

/*
 * Count the length needed for outputting highlighting information
 * as a string based on the bits for the attributes.
 *
 * If buf is not NULL, output the strings into the buffer, too.
 * As conventional with strings, the allocated length should be
 * at least the returned value plus 1 for the NUL byte.
 */

/**/
mod_export int
output_highlight(zattr atr, char *buf)
{
    const struct highlight *hp;
    int atrlen = 0, len;
    char *ptr = buf;

    if (atr & TXTFGCOLOUR) {
	len = output_colour(txtchangeget(atr, TXT_ATTR_FG_COL),
			    COL_SEQ_FG,
			    (atr & TXT_ATTR_FG_24BIT),
			    ptr);
	atrlen += len;
	if (buf)
	    ptr += len;
    }
    if (atr & TXTBGCOLOUR) {
	if (atrlen) {
	    atrlen++;
	    if (buf) {
		strcpy(ptr, ",");
		ptr++;
	    }
	}
	len = output_colour(txtchangeget(atr, TXT_ATTR_BG_COL),
			    COL_SEQ_BG,
			    (atr & TXT_ATTR_BG_24BIT),
			    ptr);
	atrlen += len;
	if (buf)
	    ptr += len;
    }
    for (hp = highlights; hp->name; hp++) {
	if (hp->mask_on & atr) {
	    if (atrlen) {
		atrlen++;
		if (buf) {
		    strcpy(ptr, ",");
		    ptr++;
		}
	    }
	    len = strlen(hp->name);
	    atrlen += len;
	    if (buf) {
		strcpy(ptr, hp->name);
		ptr += len;
	    }
	}
    }

    if (atrlen == 0) {
	if (buf)
	    strcpy(ptr, "none");
	return 4;
    }
    return atrlen;
}

/* Structure and array for holding special colour terminal sequences */

/* Start of escape sequence for foreground colour */
#define TC_COL_FG_START	"\033[3"
/* End of escape sequence for foreground colour */
#define TC_COL_FG_END	"m"
/* Code to reset foreground colour */
#define TC_COL_FG_DEFAULT	"9"

/* Start of escape sequence for background colour */
#define TC_COL_BG_START	"\033[4"
/* End of escape sequence for background colour */
#define TC_COL_BG_END	"m"
/* Code to reset background colour */
#define TC_COL_BG_DEFAULT	"9"

struct colour_sequences {
    char *start;		/* Escape sequence start */
    char *end;			/* Escape sequence terminator */
    char *def;			/* Code to reset default colour */
};
static struct colour_sequences fg_bg_sequences[2];

/*
 * We need a buffer for colour sequence composition.  It may
 * vary depending on the sequences set.  However, it's inefficient
 * allocating it separately every time we send a colour sequence,
 * so do it once per refresh.
 */
static char *colseq_buf;

/*
 * Count how often this has been allocated, for recursive usage.
 */
static int colseq_buf_allocs;

/**/
void
set_default_colour_sequences(void)
{
    fg_bg_sequences[COL_SEQ_FG].start = ztrdup(TC_COL_FG_START);
    fg_bg_sequences[COL_SEQ_FG].end = ztrdup(TC_COL_FG_END);
    fg_bg_sequences[COL_SEQ_FG].def = ztrdup(TC_COL_FG_DEFAULT);

    fg_bg_sequences[COL_SEQ_BG].start = ztrdup(TC_COL_BG_START);
    fg_bg_sequences[COL_SEQ_BG].end = ztrdup(TC_COL_BG_END);
    fg_bg_sequences[COL_SEQ_BG].def = ztrdup(TC_COL_BG_DEFAULT);
}

static void
set_colour_code(char *str, char **var)
{
    char *keyseq;
    int len;

    zsfree(*var);
    keyseq = getkeystring(str, &len, GETKEYS_BINDKEY, NULL);
    *var = metafy(keyseq, len, META_DUP);
}

/* Allocate buffer for colour code composition */

/**/
mod_export void
allocate_colour_buffer(void)
{
    char **atrs;
    int lenfg, lenbg, len;

    if (colseq_buf_allocs++)
	return;

    atrs = getaparam("zle_highlight");
    if (atrs) {
	for (; *atrs; atrs++) {
	    if (strpfx("fg_start_code:", *atrs)) {
		set_colour_code(*atrs + 14, &fg_bg_sequences[COL_SEQ_FG].start);
	    } else if (strpfx("fg_default_code:", *atrs)) {
		set_colour_code(*atrs + 16, &fg_bg_sequences[COL_SEQ_FG].def);
	    } else if (strpfx("fg_end_code:", *atrs)) {
		set_colour_code(*atrs + 12, &fg_bg_sequences[COL_SEQ_FG].end);
	    } else if (strpfx("bg_start_code:", *atrs)) {
		set_colour_code(*atrs + 14, &fg_bg_sequences[COL_SEQ_BG].start);
	    } else if (strpfx("bg_default_code:", *atrs)) {
		set_colour_code(*atrs + 16, &fg_bg_sequences[COL_SEQ_BG].def);
	    } else if (strpfx("bg_end_code:", *atrs)) {
		set_colour_code(*atrs + 12, &fg_bg_sequences[COL_SEQ_BG].end);
	    }
	}
    }

    lenfg = strlen(fg_bg_sequences[COL_SEQ_FG].def);
    /* always need 1 character for non-default code */
    if (lenfg < 1)
	lenfg = 1;
    lenfg += strlen(fg_bg_sequences[COL_SEQ_FG].start) +
	strlen(fg_bg_sequences[COL_SEQ_FG].end);

    lenbg = strlen(fg_bg_sequences[COL_SEQ_BG].def);
    /* always need 1 character for non-default code */
    if (lenbg < 1)
	lenbg = 1;
    lenbg += strlen(fg_bg_sequences[COL_SEQ_BG].start) +
	strlen(fg_bg_sequences[COL_SEQ_BG].end);

    len = lenfg > lenbg ? lenfg : lenbg;
    /* add 1 for the null and 14 for truecolor */
    colseq_buf = (char *)zalloc(len+15);
}

/* Free the colour buffer previously allocated. */

/**/
mod_export void
free_colour_buffer(void)
{
    if (--colseq_buf_allocs)
	return;

    DPUTS(!colseq_buf, "Freeing colour sequence buffer without alloc");
    /* Free buffer for colour code composition */
    free(colseq_buf);
    colseq_buf = NULL;
}

/*
 * Handle outputting of a colour for prompts or zle.
 * colour is the numeric colour, 0 to 255 (or less if termcap
 * says fewer are supported).
 * fg_bg indicates if we're changing the foreground or background.
 * tc indicates the termcap code to use, if appropriate.
 * def indicates if we're resetting the default colour.
 * flags is either 0 or TSC_PROMPT.
 */

/**/
mod_export void
set_colour_attribute(zattr atr, int fg_bg, int flags)
{
    char *ptr;
    int do_free, is_prompt = (flags & TSC_PROMPT) ? 1 : 0;
    int colour, tc, def, use_truecolor;
    int is_default_zle_highlight = 1;

    if (fg_bg == COL_SEQ_FG) {
	colour = txtchangeget(atr, TXT_ATTR_FG_COL);
	tc = TCFGCOLOUR;
	def = txtchangeisset(atr, TXTNOFGCOLOUR);
	use_truecolor = txtchangeisset(atr, TXT_ATTR_FG_24BIT);
    } else {
	colour = txtchangeget(atr, TXT_ATTR_BG_COL);
	tc = TCBGCOLOUR;
	def = txtchangeisset(atr, TXTNOBGCOLOUR);
	use_truecolor = txtchangeisset(atr, TXT_ATTR_BG_24BIT);
    }

    /* Test if current zle_highlight settings are customized, or
     * the typical "standard" codes */
    if (0 != strcmp(fg_bg_sequences[fg_bg].start, fg_bg == COL_SEQ_FG ? TC_COL_FG_START : TC_COL_BG_START) ||
	/* the same in-fix for both FG and BG */
	0 != strcmp(fg_bg_sequences[fg_bg].def, TC_COL_FG_DEFAULT) ||
	/* the same suffix for both FG and BG */
	0 != strcmp(fg_bg_sequences[fg_bg].end, TC_COL_FG_END))
    {
	is_default_zle_highlight = 0;
    }

    /*
     * If we're not restoring the default or applying true color,
     * try to use the termcap sequence.
     *
     * We have already sanitised the values we allow from the
     * highlighting variables, so much of this shouldn't be
     * necessary at this point, but we might as well be safe.
     */
    if (!def && !use_truecolor && is_default_zle_highlight)
    {
	/*
	 * We can if it's available, and either we couldn't get
	 * the maximum number of colours, or the colour is in range.
	 */
	if (tccan(tc) && (tccolours < 0 || colour < tccolours))
	{
	    if (is_prompt)
	    {
		if (!bv->dontcount) {
		    addbufspc(1);
		    *bv->bp++ = Inpar;
		}
		tputs(tgoto(tcstr[tc], colour, colour), 1, putstr);
		if (!bv->dontcount) {
		    addbufspc(1);
		    *bv->bp++ = Outpar;
		}
	    } else {
		tputs(tgoto(tcstr[tc], colour, colour), 1,
			(flags & TSC_RAW) ? putraw : putshout);
	    }
	    /* That worked. */
	    return;
	}
	/*
	 * Nope, that didn't work.
	 * If 0 to 7, assume standard ANSI works, if 8 to 255, assume
         * typical 256-color escapes works, otherwise it won't.
	 */
	if (colour > 255)
	    return;
    }

    if ((do_free = (colseq_buf == NULL))) {
	/* This can happen when moving the cursor in trashzle() */
	allocate_colour_buffer();
    }

    /* Build the reset-code: .start + .def + . end
     * or the typical true-color code: .start + 8;2;%d;%d;%d + .end
     * or the typical 256-color code: .start + 8;5;%d + .end
     */
    if (use_truecolor)
	strcpy(colseq_buf, fg_bg == COL_SEQ_FG ? TC_COL_FG_START : TC_COL_BG_START);
    else
	strcpy(colseq_buf, fg_bg_sequences[fg_bg].start);

    ptr = colseq_buf + strlen(colseq_buf);
    if (def) {
	if (use_truecolor)
	    strcpy(ptr, fg_bg == COL_SEQ_FG ? TC_COL_FG_DEFAULT : TC_COL_BG_DEFAULT);
	else
	    strcpy(ptr, fg_bg_sequences[fg_bg].def);
	while (*ptr)
	    ptr++;
    } else if (use_truecolor) {
	ptr += sprintf(ptr, "8;2;%d;%d;%d", colour >> 16,
		(colour >> 8) & 0xff, colour & 0xff);
    } else if (colour > 7 && colour <= 255) {
	ptr += sprintf(ptr, "%d", colour);
    } else
	*ptr++ = colour + '0';
    if (use_truecolor)
	strcpy(ptr, fg_bg == COL_SEQ_FG ? TC_COL_FG_END : TC_COL_BG_END);
    else
	strcpy(ptr, fg_bg_sequences[fg_bg].end);

    if (is_prompt) {
	if (!bv->dontcount) {
	    addbufspc(1);
	    *bv->bp++ = Inpar;
	}
	tputs(colseq_buf, 1, putstr);
	if (!bv->dontcount) {
	    addbufspc(1);
	    *bv->bp++ = Outpar;
	}
    } else
	tputs(colseq_buf, 1, (flags & TSC_RAW) ? putraw : putshout);

    if (do_free)
	free_colour_buffer();
}
