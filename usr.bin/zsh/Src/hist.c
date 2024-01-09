/*
 * hist.c - history expansion
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
#include "hist.pro"

/* Functions to call for getting/ungetting a character and for history
 * word control. */

/**/
mod_export int (*hgetc) _((void));

/**/
void (*hungetc) _((int));

/**/
void (*hwaddc) _((int));

/**/
void (*hwbegin) _((int));

/**/
void (*hwabort) _((void));

/**/
void (*hwend) _((void));

/**/
void (*addtoline) _((int));

/* != 0 means history substitution is turned off */
 
/**/
mod_export int stophist;

/* if != 0, we are expanding the current line */

/**/
mod_export int expanding;

/* these are used to modify the cursor position during expansion */

/**/
mod_export int excs, exlast;

/*
 * Current history event number
 *
 * Note on curhist: with history inactive, this points to the
 * last line actually added to the history list.  With history active,
 * the line does not get added to the list until hend(), if at all.
 * However, curhist is incremented to reflect the current line anyway
 * and a temporary history entry is inserted while the user is editing.
 * If the resulting line was not added to the list, a flag is set so
 * that curhist will be decremented in hbegin().
 *
 * Note curhist is passed to zle on variable length argument list:
 * type must match that retrieved in zle_main_entry.
 */
 
/**/
mod_export zlong curhist;

/**/
struct histent curline;

/* current line count of allocated history entries */

/**/
zlong histlinect;

/* The history lines are kept in a hash, and also doubly-linked in a ring */

/**/
HashTable histtab;
/**/
mod_export Histent hist_ring;
 
/* capacity of history lists */
 
/**/
zlong histsiz;
 
/* desired history-file size (in lines) */
 
/**/
zlong savehistsiz;
 
/* if = 1, we have performed history substitution on the current line *
 * if = 2, we have used the 'p' modifier                              */
 
/**/
int histdone;
 
/* state of the history mechanism */
 
/**/
int histactive;

/* Current setting of the associated option, but sometimes also includes
 * the setting of the HIST_SAVE_NO_DUPS option. */

/**/
int hist_ignore_all_dups;

/* What flags (if any) we should skip when moving through the history */

/**/
mod_export int hist_skip_flags;

/* Bits of histactive variable */
#define HA_ACTIVE	(1<<0)	/* History mechanism is active */
#define HA_NOINC	(1<<1)	/* Don't store, curhist not incremented */
#define HA_INWORD       (1<<2)  /* We're inside a word, don't add
				   start and end markers */
#define HA_UNGET        (1<<3)  /* Recursively ungetting */

/* Array of word beginnings and endings in current history line. */

/**/
short *chwords;

/* Max, actual position in chwords.
 * nwords = chwordpos/2 because we record beginning and end of words.
 */

/**/
int chwordlen, chwordpos;

/* the last l for s/l/r/ history substitution */
 
/**/
char *hsubl;

/* the last r for s/l/r/ history substitution */
 
/**/
char *hsubr;
 
/* pointer into the history line */
 
/**/
mod_export char *hptr;
 
/* the current history line */
 
/**/
mod_export char *chline;

/*
 * The current history line as seen by ZLE.
 * We modify chline for use in other contexts while ZLE may
 * still be running; ZLE should see only the top-level value.
 *
 * To avoid having to modify this every time we modify chline,
 * we set it when we push the stack, and unset it when we pop
 * the appropriate value off the stack.  As it's never modified
 * on the stack this is the only maintenance we ever do on it.
 * In return, ZLE has to check both zle_chline and (if that's
 * NULL) chline to get the current value.
 */

/**/
mod_export char *zle_chline;

/* true if the last character returned by hgetc was an escaped bangchar *
 * if it is set and NOBANGHIST is unset hwaddc escapes bangchars        */

/**/
int qbang;
 
/* max size of histline */
 
/**/
int hlinesz;
 
/* default event (usually curhist-1, that is, "!!") */
 
static zlong defev;

/*
 * Flag that we stopped reading line when we got to a comment,
 * but we want to keep it in the histofy even if there were no words
 * (i.e. the comment was the entire line).
 */
static int hist_keep_comment;

/* Remember the last line in the history file so we can find it again. */
static struct histfile_stats {
    char *text;
    time_t stim, mtim;
    off_t fpos, fsiz;
    int interrupted;
    zlong next_write_ev;
} lasthist;

static struct histsave {
    struct histfile_stats lasthist;
    char *histfile;
    HashTable histtab;
    Histent hist_ring;
    zlong curhist;
    zlong histlinect;
    zlong histsiz;
    zlong savehistsiz;
    int locallevel;
} *histsave_stack;
static int histsave_stack_size = 0;
static int histsave_stack_pos = 0;

static zlong histfile_linect;

/* save history context */

/**/
void
hist_context_save(struct hist_stack *hs, int toplevel)
{
    if (toplevel) {
	/* top level, make this version visible to ZLE */
	zle_chline = chline;
	/* ensure line stored is NULL-terminated */
	if (hptr)
	    *hptr = '\0';
    }
    hs->histactive = histactive;
    hs->histdone = histdone;
    hs->stophist = stophist;
    hs->hline = chline;
    hs->hptr = hptr;
    hs->chwords = chwords;
    hs->chwordlen = chwordlen;
    hs->chwordpos = chwordpos;
    hs->hgetc = hgetc;
    hs->hungetc = hungetc;
    hs->hwaddc = hwaddc;
    hs->hwbegin = hwbegin;
    hs->hwabort = hwabort;
    hs->hwend = hwend;
    hs->addtoline = addtoline;
    hs->hlinesz = hlinesz;
    hs->defev = defev;
    hs->hist_keep_comment = hist_keep_comment;
    /*
     * We save and restore the command stack with history
     * as it's visible to the user interactively, so if
     * we're preserving history state we'll continue to
     * show the current set of commands from input.
     */
    hs->cstack = cmdstack;
    hs->csp = cmdsp;

    stophist = 0;
    chline = NULL;
    hptr = NULL;
    histactive = 0;
    cmdstack = (unsigned char *)zalloc(CMDSTACKSZ);
    cmdsp = 0;
}

/* restore history context */

/**/
void
hist_context_restore(const struct hist_stack *hs, int toplevel)
{
    if (toplevel) {
	/* Back to top level: don't need special ZLE value */
	DPUTS(hs->hline != zle_chline, "BUG: Ouch, wrong chline for ZLE");
	zle_chline = NULL;
    }
    histactive = hs->histactive;
    histdone = hs->histdone;
    stophist = hs->stophist;
    chline = hs->hline;
    hptr = hs->hptr;
    chwords = hs->chwords;
    chwordlen = hs->chwordlen;
    chwordpos = hs->chwordpos;
    hgetc = hs->hgetc;
    hungetc = hs->hungetc;
    hwaddc = hs->hwaddc;
    hwbegin = hs->hwbegin;
    hwabort = hs->hwabort;
    hwend = hs->hwend;
    addtoline = hs->addtoline;
    hlinesz = hs->hlinesz;
    defev = hs->defev;
    hist_keep_comment = hs->hist_keep_comment;
    if (cmdstack)
	zfree(cmdstack, CMDSTACKSZ);
    cmdstack = hs->cstack;
    cmdsp = hs->csp;
}

/*
 * Mark that the current level of history is within a word whatever
 * characters turn up, or turn that mode off.  This is used for nested
 * parsing of substitutions.
 *
 * The caller takes care only to turn this on or off at the start
 * or end of recursive use of the same mode, so a single flag is
 * good enough here.
 */

/**/
void
hist_in_word(int yesno)
{
    if (yesno)
	histactive |= HA_INWORD;
    else
	histactive &= ~HA_INWORD;
}

/**/
int
hist_is_in_word(void)
{
    return (histactive & HA_INWORD) ? 1 : 0;
}

/* add a character to the current history word */

static void
ihwaddc(int c)
{
    /* Only if history line exists and lexing has not finished. */
    if (chline && !(errflag || lexstop) &&
	/*
	 * If we're reading inside a word for command substitution
	 * we allow the lexer to expand aliases but don't deal
	 * with them here.  Note matching code in ihungetc().
	 * TBD: it might be neater to deal with all aliases in this
	 * fashion as we never need the expansion in the history
	 * line, only in the lexer and above.
	 */
	(inbufflags & (INP_ALIAS|INP_HIST)) != INP_ALIAS) {
	/* Quote un-expanded bangs in the history line. */
	if (c == bangchar && stophist < 2 && qbang)
	    /* If qbang is not set, we do not escape this bangchar as it's *
	     * not necessary (e.g. it's a bang in !=, or it is followed    *
	     * by a space). Roughly speaking, qbang is zero only if the    *
	     * history interpreter has already digested this bang and      *
	     * found that it is not necessary to escape it.                */
	    hwaddc('\\');
	*hptr++ = c;

	/* Resize history line if necessary */
	if (hptr - chline >= hlinesz) {
	    int oldsiz = hlinesz;

	    chline = realloc(chline, hlinesz = oldsiz + 64);
	    hptr = chline + oldsiz;
	}
    }
}

/* This function adds a character to the zle input line. It is used when *
 * zsh expands history (see doexpandhist() in zle_tricky.c). It also     *
 * calculates the new cursor position after the expansion. It is called  *
 * from hgetc() and from gettok() in lex.c for characters in comments.   */

/**/
void
iaddtoline(int c)
{
    if (!expanding || lexstop)
	return;
    if (qbang && c == bangchar && stophist < 2) {
	exlast--;
	zleentry(ZLE_CMD_ADD_TO_LINE, '\\');
    }
    if (excs > zlemetacs) {
	excs += 1 + inbufct - exlast;
	if (excs < zlemetacs)
	    /* this case could be handled better but it is    *
	     * so rare that it does not worth it              */
	    excs = zlemetacs;
    }
    exlast = inbufct;
    zleentry(ZLE_CMD_ADD_TO_LINE, itok(c) ? ztokens[c - Pound] : c);
}


static int
ihgetc(void)
{
    int c = ingetc();

    if (exit_pending)
    {
	lexstop = 1;
	errflag |= ERRFLAG_ERROR;
	return ' ';
    }
    qbang = 0;
    if (!stophist && !(inbufflags & INP_ALIAS)) {
	/* If necessary, expand history characters. */
	c = histsubchar(c);
	if (c < 0) {
	    /* bad expansion */
	    lexstop = 1;
	    errflag |= ERRFLAG_ERROR;
	    return ' ';
	}
    }
    if ((inbufflags & INP_HIST) && !stophist) {
	/* the current character c came from a history expansion          *
	 * (inbufflags & INP_HIST) and history is not disabled            *
	 * (e.g. we are not inside single quotes). In that case, \!       *
	 * should be treated as ! (since this \! came from a previous     *
	 * history line where \ was used to escape the bang). So if       *
	 * c == '\\' we fetch one more character to see if it's a bang,   *
	 * and if it is not, we unget it and reset c back to '\\'         */
	qbang = 0;
	if (c == '\\' && !(qbang = (c = ingetc()) == bangchar))
	    safeinungetc(c), c = '\\';
    } else if (stophist || (inbufflags & INP_ALIAS))
	/* If the result is a bangchar which came from history or alias  *
	 * expansion, we treat it as an escaped bangchar, unless history *
	 * is disabled. If stophist == 1 it only means that history is   *
	 * temporarily disabled by a !" which won't appear in the        *
	 * history, so we still have an escaped bang. stophist > 1 if    *
	 * history is disabled with NOBANGHIST or by someone else (e.g.  *
	 * when the lexer scans single quoted text).                     */
	qbang = c == bangchar && (stophist < 2);
    hwaddc(c);
    addtoline(c);

    return c;
}

/**/
static void
safeinungetc(int c)
{
    if (lexstop)
	lexstop = 0;
    else
	inungetc(c);
}

/**/
void
herrflush(void)
{
    inpopalias();

    if (lexstop)
	return;
    /*
     * The lex_add_raw test is needed if we are parsing a command
     * substitution when expanding history for ZLE: strin is set but we
     * need to finish off the input because the string we are reading is
     * going to be used directly in the line that goes to ZLE.
     *
     * Note that this is a side effect --- this is not the usual reason
     * for testing lex_add_raw which is to add the text to a different
     * buffer used when we are actually parsing the command substitution
     * (nothing to do with ZLE).  Sorry.
     */
    while (inbufct && (!strin || lex_add_raw)) {
	int c = ingetc();
	if (!lexstop) {
	    hwaddc(c);
	    addtoline(c);
	}
    }
}

/*
 * Extract :s/foo/bar/ delimiters and arguments
 *
 * The first character expected is the first delimiter.
 * The arguments are stored in the hsubl and hsubr variables.
 *
 * subline is the part of the command line to be matched.
 *
 * If a ':' was found but was not followed by a 'G',
 * *cflagp is set to 1 and the input is backed up to the
 * character following the colon.
 */

/**/
static int
getsubsargs(UNUSED(char *subline), int *gbalp, int *cflagp)
{
    int del, follow;
    char *ptr1, *ptr2;

    del = ingetc();
    ptr1 = hdynread2(del);
    if (!ptr1)
	return 1;
    ptr2 = hdynread2(del);
    if (strlen(ptr1)) {
	zsfree(hsubl);
	hsubl = ptr1;
    } else if (!hsubl) {		/* fail silently on this */
	zsfree(ptr1);
	zsfree(ptr2);
	return 0;
    }
    zsfree(hsubr);
    hsubr = ptr2;
    follow = ingetc();
    if (follow == ':') {
	follow = ingetc();
	if (follow == 'G')
	    *gbalp = 1;
	else {
	    inungetc(follow);
	    *cflagp = 1;
	}
    } else
	inungetc(follow);
    return 0;
}

/* Get the maximum no. of words for a history entry. */

/**/
static int
getargc(Histent ehist)
{
    return ehist->nwords ? ehist->nwords-1 : 0;
}

/**/
static int
substfailed(void)
{
    herrflush();
    zerr("substitution failed");
    return -1;
}

/*
 * Return a count given by decimal digits after a modifier.
 */
static int
digitcount(void)
{
    int c = ingetc(), count;

    if (idigit(c)) {
	count = 0;
	do {
	    count = 10 * count + (c - '0');
	    c = ingetc();
	} while (idigit(c));
    }
    else
	count = 0;
    inungetc(c);
    return count;
}

/* Perform history substitution, returning the next character afterwards. */

/**/
static int
histsubchar(int c)
{
    int farg, evset = -1, larg, argc, cflag = 0, bflag = 0;
    zlong ev;
    static int marg = -1;
    static zlong mev = -1;
    char *buf, *ptr;
    char *sline;
    int lexraw_mark;
    Histent ehist;
    size_t buflen;

    /*
     * If accumulating raw input for use in command substitution,
     * we don't want the history text, so mark it for later removal.
     * It would be better to do this at a level above the history
     * and below the lexer --- but there isn't one.
     *
     * Include the character we are attempting to substitute.
     */
    lexraw_mark = zshlex_raw_mark(-1); 

    /* look, no goto's */
    if (isfirstch && c == hatchar) {
	int gbal = 0;

	/* Line begins ^foo^bar */
	isfirstch = 0;
	inungetc(hatchar);
	if (!(ehist = gethist(defev))
	    || !(sline = getargs(ehist, 0, getargc(ehist))))
	    return -1;

	if (getsubsargs(sline, &gbal, &cflag))
	    return substfailed();
	if (!hsubl)
	    return -1;
	if (subst(&sline, hsubl, hsubr, gbal))
	    return substfailed();
    } else {
	/* Line doesn't begin ^foo^bar */
	if (c != ' ')
	    isfirstch = 0;
	if (c == '\\') {
	    int g = ingetc();

	    if (g != bangchar)
		safeinungetc(g);
	    else {
		qbang = 1;
		return bangchar;
	    }
	}
	if (c != bangchar)
	    return c;
	*hptr = '\0';
	if ((c = ingetc()) == '{') {
	    bflag = cflag = 1;
	    c = ingetc();
	}
	if (c == '\"') {
	    stophist = 1;
	    return ingetc();
	}
	if ((!cflag && inblank(c)) || c == '=' || c == '(' || lexstop) {
	    safeinungetc(c);
	    return bangchar;
	}
	cflag = 0;
	ptr = buf = zhalloc(buflen = 265);

	/* get event number */

	queue_signals();
	if (c == '?') {
	    for (;;) {
		c = ingetc();
		if (c == '?' || c == '\n' || lexstop)
		    break;
		else {
		    *ptr++ = c;
		    if (ptr == buf + buflen) {
			buf = hrealloc(buf, buflen, 2 * buflen);
			ptr = buf + buflen;
			buflen *= 2;
		    }
		}
	    }
	    if (c != '\n' && !lexstop)
		c = ingetc();
	    *ptr = '\0';
	    mev = ev = hconsearch(hsubl = ztrdup(buf), &marg);
	    evset = 0;
	    if (ev == -1) {
		herrflush();
		unqueue_signals();
		zerr("no such event: %s", buf);
		return -1;
	    }
	} else {
	    zlong t0;

	    for (;;) {
		if (inblank(c) || c == ';' || c == ':' || c == '^' ||
		    c == '$' || c == '*' || c == '%' || c == '}' ||
		    c == '\'' || c == '"' || c == '`' || lexstop)
		    break;
		if (ptr != buf) {
		    if (c == '-')
			break;
		    if ((idigit(buf[0]) || buf[0] == '-') && !idigit(c))
			break;
		}
		*ptr++ = c;
		if (ptr == buf + buflen) {
		    buf = hrealloc(buf, buflen, 2 * buflen);
		    ptr = buf + buflen;
		    buflen *= 2;
		}
		if (c == '#' || c == bangchar) {
		    c = ingetc();
		    break;
		}
		c = ingetc();
	    }
	    if (ptr == buf &&
		(c == '}' ||  c == ';' || c == '\'' || c == '"' || c == '`')) {
	      /* Neither event nor word designator, no expansion */
	      safeinungetc(c);
	      unqueue_signals();
	      return bangchar;
	    }
	    *ptr = 0;
	    if (!*buf) {
		if (c != '%') {
		    if (isset(CSHJUNKIEHISTORY))
			ev = addhistnum(curhist,-1,HIST_FOREIGN);
		    else
			ev = defev;
		    if (c == ':' && evset == -1)
			evset = 0;
		    else
			evset = 1;
		} else {
		    if (marg != -1)
			ev = mev;
		    else
			ev = defev;
		    evset = 0;
		}
	    } else if ((t0 = zstrtol(buf, NULL, 10))) {
		ev = (t0 < 0) ? addhistnum(curhist,t0,HIST_FOREIGN) : t0;
		evset = 1;
	    } else if ((unsigned)*buf == bangchar) {
		ev = addhistnum(curhist,-1,HIST_FOREIGN);
		evset = 1;
	    } else if (*buf == '#') {
		ev = curhist;
		evset = 1;
	    } else if ((ev = hcomsearch(buf)) == -1) {
		herrflush();
		unqueue_signals();
		zerr("event not found: %s", buf);
		return -1;
	    } else
		evset = 1;
	}

	/* get the event */

	if (!(ehist = gethist(defev = ev))) {
	    unqueue_signals();
	    return -1;
	}
	/* extract the relevant arguments */

	argc = getargc(ehist);
	if (c == ':') {
	    cflag = 1;
	    c = ingetc();
	    if (c == '%' && marg != -1) {
		if (!evset) {
		    ehist = gethist(defev = mev);
		    argc = getargc(ehist);
		} else {
		    herrflush();
		    unqueue_signals();
		    zerr("ambiguous history reference");
		    return -1;
		}

	    }
	}
	if (c == '*') {
	    farg = 1;
	    larg = argc;
	    cflag = 0;
	} else {
	    inungetc(c);
	    larg = farg = getargspec(argc, marg, evset);
	    if (larg == -2) {
		unqueue_signals();
		return -1;
	    }
	    if (farg != -1)
		cflag = 0;
	    c = ingetc();
	    if (c == '*') {
		cflag = 0;
		larg = argc;
	    } else if (c == '-') {
		cflag = 0;
		larg = getargspec(argc, marg, evset);
		if (larg == -2) {
		    unqueue_signals();
		    return -1;
		}
		if (larg == -1)
		    larg = argc - 1;
	    } else
		inungetc(c);
	}
	if (farg == -1)
	    farg = 0;
	if (larg == -1)
	    larg = argc;
	if (!(sline = getargs(ehist, farg, larg))) {
	    unqueue_signals();
	    return -1;
	}
	unqueue_signals();
    }

    /* do the modifiers */

    for (;;) {
	c = (cflag) ? ':' : ingetc();
	cflag = 0;
	if (c == ':') {
	    int gbal = 0;

	    if ((c = ingetc()) == 'g') {
		gbal = 1;
		c = ingetc();
		if (c != 's' && c != '&') {
		    zerr("'s' or '&' modifier expected after 'g'");
		    return -1;
		}
	    }
	    switch (c) {
	    case 'p':
		histdone = HISTFLAG_DONE | HISTFLAG_NOEXEC;
		break;
	    case 'a':
		if (!chabspath(&sline)) {
		    herrflush();
		    zerr("modifier failed: a");
		    return -1;
		}
		break;

	    case 'A':
		if (!chrealpath(&sline, 'A', 1)) {
		    herrflush();
		    zerr("modifier failed: A");
		    return -1;
		}
		break;
	    case 'c':
		if (!(sline = equalsubstr(sline, 0, 0))) {
		    herrflush();
		    zerr("modifier failed: c");
		    return -1;
		}
		break;
	    case 'h':
		if (!remtpath(&sline, digitcount())) {
		    herrflush();
		    zerr("modifier failed: h");
		    return -1;
		}
		break;
	    case 'e':
		if (!rembutext(&sline)) {
		    herrflush();
		    zerr("modifier failed: e");
		    return -1;
		}
		break;
	    case 'r':
		if (!remtext(&sline)) {
		    herrflush();
		    zerr("modifier failed: r");
		    return -1;
		}
		break;
	    case 't':
		if (!remlpaths(&sline, digitcount())) {
		    herrflush();
		    zerr("modifier failed: t");
		    return -1;
		}
		break;
	    case 's':
		if (getsubsargs(sline, &gbal, &cflag))
		    return -1; /* fall through */
	    case '&':
		if (hsubl && hsubr) {
		    if (subst(&sline, hsubl, hsubr, gbal))
			return substfailed();
		} else {
		    herrflush();
		    zerr("no previous substitution");
		    return -1;
		}
		break;
	    case 'q':
		quote(&sline);
		break;
	    case 'Q':
		{
		    int one = noerrs, oef = errflag;

		    noerrs = 1;
		    parse_subst_string(sline);
		    noerrs = one;
		    errflag = oef | (errflag & ERRFLAG_INT);
		    remnulargs(sline);
		    untokenize(sline);
		}
		break;
	    case 'x':
		quotebreak(&sline);
		break;
	    case 'l':
		sline = casemodify(sline, CASMOD_LOWER);
		break;
	    case 'u':
		sline = casemodify(sline, CASMOD_UPPER);
		break;
	    case 'P':
		if (*sline != '/') {
		    char *here = zgetcwd();
		    if (here[strlen(here)-1] != '/')
			sline = zhtricat(metafy(here, -1, META_HEAPDUP), "/", sline);
		    else
			sline = dyncat(here, sline);
		}
		sline = xsymlink(sline, 1);
		break;
	    default:
		herrflush();
		zerr("illegal modifier: %c", c);
		return -1;
	    }
	} else {
	    if (c != '}' || !bflag)
		inungetc(c);
	    if (c != '}' && bflag) {
		zerr("'}' expected");
		return -1;
	    }
	    break;
	}
    }

    zshlex_raw_back_to_mark(lexraw_mark);

    /*
     * Push the expanded value onto the input stack,
     * marking this as a history word for purposes of the alias stack.
     */

    lexstop = 0;
    /* this function is called only called from hgetc and only if      *
     * !(inbufflags & INP_ALIAS). History expansion should never be    *
     * done with INP_ALIAS (to prevent recursive history expansion and *
     * histoty expansion of aliases). Escapes are not removed here.    *
     * This is now handled in hgetc.                                   */
    inpush(sline, INP_HIST, NULL); /* sline from heap, don't free */
    histdone |= HISTFLAG_DONE;
    if (isset(HISTVERIFY))
	histdone |= HISTFLAG_NOEXEC | HISTFLAG_RECALL;

    /* Don't try and re-expand line. */
    return ingetc();
}

/* unget a char and remove it from chline. It can only be used *
 * to unget a character returned by hgetc.                     */

static void
ihungetc(int c)
{
    int doit = 1;

    while (!lexstop && !errflag) {
	if (hptr[-1] != (char) c && stophist < 4 &&
	    hptr > chline + 1 && hptr[-1] == '\n' && hptr[-2] == '\\' &&
	    !(histactive & HA_UNGET) &&
	    (inbufflags & (INP_ALIAS|INP_HIST)) != INP_ALIAS) {
	    histactive |= HA_UNGET;
	    hungetc('\n');
	    hungetc('\\');
	    histactive &= ~HA_UNGET;
	}

	if (expanding) {
	    zlemetacs--;
	    zlemetall--;
	    exlast++;
	}
	if ((inbufflags & (INP_ALIAS|INP_HIST)) != INP_ALIAS) {
	    DPUTS(hptr <= chline, "BUG: hungetc attempted at buffer start");
	    hptr--;
	    DPUTS(*hptr != (char) c, "BUG: wrong character in hungetc() ");
	    qbang = (c == bangchar && stophist < 2 &&
		     hptr > chline && hptr[-1] == '\\');
	} else {
	    /* No active bangs in aliases */
	    qbang = 0;
	}
	if (doit)
	    inungetc(c);
	if (!qbang)
	    return;
	doit = !stophist && ((inbufflags & INP_HIST) ||
				 !(inbufflags & INP_ALIAS));
	c = '\\';
    }
}

/* begin reading a string */

/**/
mod_export void
strinbeg(int dohist)
{
    strin++;
    hbegin(dohist);
    lexinit();
    /*
     * Also initialise some variables owned by the parser but
     * used for communication between the parser and lexer.
     */
    init_parse_status();
}

/* done reading a string */

/**/
mod_export void
strinend(void)
{
    hend(NULL);
    DPUTS(!strin, "BUG: strinend() called without strinbeg()");
    strin--;
    isfirstch = 1;
    histdone = 0;
}

/* dummy functions to use instead of hwaddc(), hwbegin(), and hwend() when
 * they aren't needed */

static void
nohw(UNUSED(int c))
{
}

static void
nohwabort(void)
{
}

static void
nohwe(void)
{
}

/* these functions handle adding/removing curline to/from the hist_ring */

static void
linkcurline(void)
{
    if (!hist_ring)
	hist_ring = curline.up = curline.down = &curline;
    else {
	curline.up = hist_ring;
	curline.down = hist_ring->down;
	hist_ring->down = hist_ring->down->up = &curline;
	hist_ring = &curline;
    }
    curline.histnum = ++curhist;
}

static void
unlinkcurline(void)
{
    curline.up->down = curline.down;
    curline.down->up = curline.up;
    if (hist_ring == &curline) {
	if (!histlinect)
	    hist_ring = NULL;
	else
	    hist_ring = curline.up;
    }
    curhist--;
}

/* initialize the history mechanism */

/**/
mod_export void
hbegin(int dohist)
{
    char *hf;

    isfirstln = isfirstch = 1;
    errflag &= ~ERRFLAG_ERROR;
    histdone = 0;
    if (!dohist)
	stophist = 2;
    else if (dohist != 2)
	stophist = (!interact || unset(SHINSTDIN)) ? 2 : 0;
    else
	stophist = 0;
    /*
     * pws: We used to test for "|| (inbufflags & INP_ALIAS)"
     * in this test, but at this point we don't have input
     * set up so this can trigger unnecessarily.
     * I don't see how the test at this point could ever be
     * useful, since we only get here when we're initialising
     * the history mechanism, before we've done any input.
     *
     * (I also don't see any point where this function is called with
     * dohist=0.)
     */
    if (stophist == 2) {
	chline = hptr = NULL;
	hlinesz = 0;
	chwords = NULL;
	chwordlen = 0;
	hgetc = ingetc;
	hungetc = inungetc;
	hwaddc = nohw;
	hwbegin = nohw;
	hwabort = nohwabort;
	hwend = nohwe;
	addtoline = nohw;
    } else {
	chline = hptr = zshcalloc(hlinesz = 64);
	chwords = zalloc((chwordlen = 64) * sizeof(short));
	hgetc = ihgetc;
	hungetc = ihungetc;
	hwaddc = ihwaddc;
	hwbegin = ihwbegin;
	hwabort = ihwabort;
	hwend = ihwend;
	addtoline = iaddtoline;
	if (!isset(BANGHIST))
	    stophist = 4;
    }
    chwordpos = 0;

    if (hist_ring && !hist_ring->ftim && !strin)
	hist_ring->ftim = time(NULL);
    if ((dohist == 2 || (interact && isset(SHINSTDIN))) && !strin) {
	histactive = HA_ACTIVE;
	attachtty(mypgrp);
	linkcurline();
	defev = addhistnum(curhist, -1, HIST_FOREIGN);
    } else
	histactive = HA_ACTIVE | HA_NOINC;

    /*
     * For INCAPPENDHISTORYTIME, when interactive, save the history here
     * as it gives a better estimate of the times of commands.
     *
     * If INCAPPENDHISTORY is also set we've already done it.
     *
     * If SHAREHISTORY is also set continue to do so in the
     * standard place, because that's safer about reading and
     * rewriting history atomically.
     *
     * The histsave_stack_pos test won't usually fail here.
     * We need to test the opposite for the hend() case because we
     * need to save in the history file we've switched to, but then
     * we pop immediately after that so the variable becomes zero.
     * We will already have saved the line and restored the history
     * so that (correctly) nothing happens here.  But it shows
     * I thought about it.
     */
    if (isset(INCAPPENDHISTORYTIME) && !isset(SHAREHISTORY) &&
	!isset(INCAPPENDHISTORY) &&
	!(histactive & HA_NOINC) && !strin && histsave_stack_pos == 0) {
	hf = getsparam("HISTFILE");
	savehistfile(hf, 0, HFILE_USE_OPTIONS | HFILE_FAST);
    }
}

/**/
void
histreduceblanks(void)
{
    int i, len, pos, needblank, spacecount = 0, trunc_ok;
    char *lastptr, *ptr;

    if (isset(HISTIGNORESPACE))
	while (chline[spacecount] == ' ') spacecount++;

    for (i = 0, len = spacecount; i < chwordpos; i += 2) {
	len += chwords[i+1] - chwords[i]
	     + (i > 0 && chwords[i] > chwords[i-1]);
    }
    if (chline[len] == '\0')
	return;

    /* Remember where the delimited words end */
    if (chwordpos)
	lastptr = chline + chwords[chwordpos-1];
    else
	lastptr = chline;

    for (i = 0, pos = spacecount; i < chwordpos; i += 2) {
	len = chwords[i+1] - chwords[i];
	needblank = (i < chwordpos-2 && chwords[i+2] > chwords[i+1]);
	if (pos != chwords[i]) {
	    memmove(chline + pos, chline + chwords[i], len + needblank);
	    chwords[i] = pos;
	    chwords[i+1] = chwords[i] + len;
	}
	pos += len + needblank;
    }

    /*
     * A terminating comment isn't recorded as a word.
     * Only truncate the line if just whitespace remains.
     */
    trunc_ok = 1;
    for (ptr = lastptr; *ptr; ptr++) {
	if (!inblank(*ptr)) {
	    trunc_ok = 0;
	    break;
	}
    }
    if (trunc_ok) {
	chline[pos] = '\0';
    } else {
	ptr = chline + pos;
	if (ptr < lastptr)
	    while ((*ptr++ = *lastptr++))
		;
    }
}

/**/
void
histremovedups(void)
{
    Histent he, next;
    for (he = hist_ring; he; he = next) {
	next = up_histent(he);
	if (he->node.flags & HIST_DUP)
	    freehistnode(&he->node);
    }
}

/**/
mod_export zlong
addhistnum(zlong hl, int n, int xflags)
{
    int dir = n < 0? -1 : n > 0? 1 : 0;
    Histent he = gethistent(hl, dir);
			     
    if (!he)
	return 0;
    if (he->histnum != hl)
	n -= dir;
    if (n)
	he = movehistent(he, n, xflags);
    if (!he)
	return dir < 0? firsthist() - 1 : curhist + 1;
    return he->histnum;
}

/**/
mod_export Histent
movehistent(Histent he, int n, int xflags)
{
    while (n < 0) {
	if (!(he = up_histent(he)))
	    return NULL;
	if (!(he->node.flags & xflags))
	    n++;
    }
    while (n > 0) {
	if (!(he = down_histent(he)))
	    return NULL;
	if (!(he->node.flags & xflags))
	    n--;
    }
    checkcurline(he);
    return he;
}

/**/
mod_export Histent
up_histent(Histent he)
{
    return !he || he->up == hist_ring? NULL : he->up;
}

/**/
mod_export Histent
down_histent(Histent he)
{
    return he == hist_ring? NULL : he->down;
}

/**/
mod_export Histent
gethistent(zlong ev, int nearmatch)
{
    Histent he;

    if (!hist_ring)
	return NULL;

    if (ev - hist_ring->down->histnum < hist_ring->histnum - ev) {
	for (he = hist_ring->down; he->histnum < ev; he = he->down) ;
	if (he->histnum != ev) {
	    if (nearmatch == 0
	     || (nearmatch < 0 && (he = up_histent(he)) == NULL))
		return NULL;
	}
    }
    else {
	for (he = hist_ring; he->histnum > ev; he = he->up) ;
	if (he->histnum != ev) {
	    if (nearmatch == 0
	     || (nearmatch > 0 && (he = down_histent(he)) == NULL))
		return NULL;
	}
    }

    checkcurline(he);
    return he;
}

static void
putoldhistentryontop(short keep_going)
{
    static Histent next = NULL;
    Histent he = (keep_going || !hist_ring) ? next : hist_ring->down;
    if (he)
	next = he->down;
    else
	return;
    if (isset(HISTEXPIREDUPSFIRST) && !(he->node.flags & HIST_DUP)) {
	static zlong max_unique_ct = 0;
	if (!keep_going)
	    max_unique_ct = savehistsiz;
	do {
	    if (max_unique_ct-- <= 0 || he == hist_ring) {
		max_unique_ct = 0;
		he = hist_ring->down;
		next = hist_ring;
		break;
	    }
	    he = next;
	    next = he->down;
	} while (!(he->node.flags & HIST_DUP));
    }
    if (he != hist_ring->down) {
	he->up->down = he->down;
	he->down->up = he->up;
	he->up = hist_ring;
	he->down = hist_ring->down;
	hist_ring->down = he->down->up = he;
    }
    hist_ring = he;
}

/**/
Histent
prepnexthistent(void)
{
    Histent he; 
    int curline_in_ring = hist_ring == &curline;

    if (curline_in_ring)
	unlinkcurline();
    if (hist_ring && hist_ring->node.flags & HIST_TMPSTORE) {
	curhist--;
	freehistnode(&hist_ring->node);
    }

    if (histlinect < histsiz || !hist_ring) {
	he = (Histent)zshcalloc(sizeof *he);
	if (!hist_ring)
	    hist_ring = he->up = he->down = he;
	else {
	    he->up = hist_ring;
	    he->down = hist_ring->down;
	    hist_ring->down = he->down->up = he;
	    hist_ring = he;
	}
	histlinect++;
    }
    else {
	putoldhistentryontop(0);
	freehistdata(hist_ring, 0);
	he = hist_ring;
    }
    he->histnum = ++curhist;
    if (curline_in_ring)
	linkcurline();
    return he;
}

/* A helper function for hend() */

static int
should_ignore_line(Eprog prog)
{
    if (isset(HISTIGNORESPACE)) {
	if (*chline == ' ' || aliasspaceflag)
	    return 1;
    }

    if (!prog)
	return 0;

    if (isset(HISTNOFUNCTIONS)) {
	Wordcode pc = prog->prog;
	wordcode code = *pc;
	if (wc_code(code) == WC_LIST && WC_LIST_TYPE(code) & Z_SIMPLE
	 && wc_code(pc[2]) == WC_FUNCDEF)
	    return 1;
    }

    if (isset(HISTNOSTORE)) {
	char *b = getjobtext(prog, NULL);
	int saw_builtin;
	if (*b == 'b' && strncmp(b,"builtin ",8) == 0) {
	    b += 8;
	    saw_builtin = 1;
	} else
	    saw_builtin = 0;
	if (*b == 'h' && strncmp(b,"history",7) == 0 && (!b[7] || b[7] == ' ')
	 && (saw_builtin || !shfunctab->getnode(shfunctab,"history")))
	    return 1;
	if (*b == 'r' && (!b[1] || b[1] == ' ')
	 && (saw_builtin || !shfunctab->getnode(shfunctab,"r")))
	    return 1;
	if (*b == 'f' && b[1] == 'c' && b[2] == ' ' && b[3] == '-'
	 && (saw_builtin || !shfunctab->getnode(shfunctab,"fc"))) {
	    b += 3;
	    do {
		if (*++b == 'l')
		    return 1;
	    } while (ialpha(*b));
	}
    }

    return 0;
}

/* say we're done using the history mechanism */

/**/
mod_export int
hend(Eprog prog)
{
    int flag, hookret, stack_pos = histsave_stack_pos;
    /*
     * save:
     * 0: don't save
     * 1: save normally
     * -1: save temporarily, delete after next line
     * -2: save internally but mark for not writing
     */
    int save = 1;
    char *hf;

    DPUTS(stophist != 2 && !(inbufflags & INP_ALIAS) && !chline,
	  "BUG: chline is NULL in hend()");
    queue_signals();
    if (histdone & HISTFLAG_SETTY)
	settyinfo(&shttyinfo);
    if (!(histactive & HA_NOINC))
	unlinkcurline();
    if (histactive & HA_NOINC) {
	zfree(chline, hlinesz);
	zfree(chwords, chwordlen*sizeof(short));
	chline = hptr = NULL;
	chwords = NULL;
	histactive = 0;
	unqueue_signals();
	return 1;
    }
    if (hist_ignore_all_dups != isset(HISTIGNOREALLDUPS)
     && (hist_ignore_all_dups = isset(HISTIGNOREALLDUPS)) != 0)
	histremovedups();

    if (hptr) {
	/*
	 * Added the following in case the test "hptr < chline + 1"
	 * is more than just paranoia.
	 */
	DPUTS(hptr < chline, "History end pointer off start of line");
	*hptr = '\0';
    }
    if (*chline) {
	LinkList hookargs = newlinklist();
	int save_errflag = errflag;
	errflag = 0;

	addlinknode(hookargs, "zshaddhistory");
	addlinknode(hookargs, chline);
	callhookfunc("zshaddhistory", hookargs, 1, &hookret);

	errflag &= ~ERRFLAG_ERROR;
	errflag |= save_errflag;
    }
    /* For history sharing, lock history file once for both read and write */
    hf = getsparam("HISTFILE");
    if (isset(SHAREHISTORY) && !lockhistfile(hf, 0)) {
	readhistfile(hf, 0, HFILE_USE_OPTIONS | HFILE_FAST);
	curline.histnum = curhist+1;
    }
    flag = histdone;
    histdone = 0;
    if (hptr < chline + 1)
	save = 0;
    else {
	if (hptr[-1] == '\n') {
	    if (chline[1]) {
		*--hptr = '\0';
	    } else
		save = 0;
	}
	if (chwordpos <= 2 && !hist_keep_comment)
	    save = 0;
	else if (should_ignore_line(prog))
	    save = -1;
	else if (hookret == 2)
	    save = -2;
	else if (hookret)
	    save = -1;
    }
    if (flag & (HISTFLAG_DONE | HISTFLAG_RECALL)) {
	char *ptr;

	ptr = ztrdup(chline);
	if ((flag & (HISTFLAG_DONE | HISTFLAG_RECALL)) == HISTFLAG_DONE) {
	    zputs(ptr, shout);
	    fputc('\n', shout);
	    fflush(shout);
	}
	if (flag & HISTFLAG_RECALL) {
	    zpushnode(bufstack, ptr);
	    save = 0;
	} else
	    zsfree(ptr);
    }
    if (save || *chline == ' ') {
	Histent he;
	for (he = hist_ring; he && he->node.flags & HIST_FOREIGN;
	     he = up_histent(he)) ;
	if (he && he->node.flags & HIST_TMPSTORE) {
	    if (he == hist_ring)
		curline.histnum = curhist--;
	    freehistnode(&he->node);
	}
    }
    if (save) {
	Histent he;
	int newflags;

#ifdef DEBUG
	/* debugging only */
	if (chwordpos%2) {
	    hwend();
	    DPUTS(1, "BUG: uncompleted line in history");
	}
#endif
	/* get rid of pesky \n which we've already nulled out */
	if (chwordpos > 1 && !chline[chwords[chwordpos-2]]) {
	    chwordpos -= 2;
	    /* strip superfluous blanks, if desired */
	    if (isset(HISTREDUCEBLANKS))
		histreduceblanks();
	}
	if (save == -1)
	    newflags = HIST_TMPSTORE;
	else if (save == -2)
	    newflags = HIST_NOWRITE;
	else
	    newflags = 0;
	if ((isset(HISTIGNOREDUPS) || isset(HISTIGNOREALLDUPS)) && save > 0
	 && hist_ring && histstrcmp(chline, hist_ring->node.nam) == 0) {
	    /* This history entry compares the same as the previous.
	     * In case minor changes were made, we overwrite the
	     * previous one with the current one.  This also gets the
	     * timestamp right.  Perhaps, preserve the HIST_OLD flag.
	     */
	    he = hist_ring;
	    newflags |= he->node.flags & HIST_OLD; /* Avoid re-saving */
	    freehistdata(he, 0);
	    curline.histnum = curhist;
	} else
	    he = prepnexthistent();

	he->node.nam = ztrdup(chline);
	he->stim = time(NULL);
	he->ftim = 0L;
	he->node.flags = newflags;

	if ((he->nwords = chwordpos/2)) {
	    he->words = (short *)zalloc(chwordpos * sizeof(short));
	    memcpy(he->words, chwords, chwordpos * sizeof(short));
	}
	if (!(newflags & HIST_TMPSTORE))
	    addhistnode(histtab, he->node.nam, he);
    }
    zfree(chline, hlinesz);
    zfree(chwords, chwordlen*sizeof(short));
    chline = hptr = NULL;
    chwords = NULL;
    histactive = 0;
    /*
     * For normal INCAPPENDHISTORY case and reasoning, see hbegin().
     */
    if (isset(SHAREHISTORY) ? histfileIsLocked() :
	(isset(INCAPPENDHISTORY) || (isset(INCAPPENDHISTORYTIME) &&
				     histsave_stack_pos != 0)))
	savehistfile(hf, 0, HFILE_USE_OPTIONS | HFILE_FAST);
    unlockhistfile(hf); /* It's OK to call this even if we aren't locked */
    /*
     * No good reason for the user to push the history more than once, but
     * it's easy to be tidy...
     */
    while (histsave_stack_pos > stack_pos)
	pophiststack();
    hist_keep_comment = 0;
    unqueue_signals();
    return !(flag & HISTFLAG_NOEXEC || errflag);
}

/* begin a word */

/**/
void
ihwbegin(int offset)
{
    if (stophist == 2 || (histactive & HA_INWORD) ||
	(inbufflags & (INP_ALIAS|INP_HIST)) == INP_ALIAS)
	return;
    if (chwordpos%2)
	chwordpos--;	/* make sure we're on a word start, not end */
    chwords[chwordpos++] = hptr - chline + offset;
}

/* Abort current history word, not needed */

/**/
void
ihwabort(void)
{
    if (chwordpos%2)
	chwordpos--;
    hist_keep_comment = 1;
}

/* add a word to the history List */

/**/
void
ihwend(void)
{
    if (stophist == 2 || (histactive & HA_INWORD) ||
	(inbufflags & (INP_ALIAS|INP_HIST)) == INP_ALIAS)
	return;
    if (chwordpos%2 && chline) {
	/* end of word reached and we've already begun a word */
	if (hptr > chline + chwords[chwordpos-1]) {
	    chwords[chwordpos++] = hptr - chline;
	    if (chwordpos >= chwordlen) {
		chwords = (short *) realloc(chwords,
					    (chwordlen += 32) * 
					    sizeof(short));
	    }
	} else {
	    /* scrub that last word, it doesn't exist */
	    chwordpos--;
	}
    }
}

/* Go back to immediately after the last word, skipping space. */

/**/
void
histbackword(void)
{
    if (!(chwordpos%2) && chwordpos)
	hptr = chline + chwords[chwordpos-1];
}

/* Get the start and end point of the current history word */

/**/
static void
hwget(char **startptr)
{
    int pos = chwordpos - 2;

#ifdef DEBUG
    /* debugging only */
    if (!chwordpos) {
	/* no words available */
	DPUTS(1, "BUG: hwget() called with no words");
	*startptr = "";
	return;
    }
    else if (chwordpos%2) {
	DPUTS(1, "BUG: hwget() called in middle of word");
	*startptr = "";
	return;
    }
#endif

    *startptr = chline + chwords[pos];
    chline[chwords[++pos]] = '\0';
}

/* Replace the current history word with rep, if different */

/**/
void
hwrep(char *rep)
{
    char *start;
    hwget(&start);

    if (!strcmp(rep, start))
	return;

    hptr = start;
    chwordpos = chwordpos - 2;
    hwbegin(0);
    qbang = 1;
    while (*rep)
	hwaddc(*rep++);
    hwend();
}

/* Get the entire current line, deleting it in the history. */

/**/
mod_export char *
hgetline(void)
{
    /* Currently only used by pushlineoredit().
     * It's necessary to prevent that from getting too pally with
     * the history code.
     */
    char *ret;

    if (!chline || hptr == chline)
	return NULL;
    *hptr = '\0';
    ret = dupstring(chline);

    /* reset line */
    hptr = chline;
    chwordpos = 0;

    return ret;
}

/* get an argument specification */

/**/
static int
getargspec(int argc, int marg, int evset)
{
    int c, ret = -1;

    if ((c = ingetc()) == '0')
	return 0;
    if (idigit(c)) {
	ret = 0;
	while (idigit(c)) {
	    ret = ret * 10 + c - '0';
	    c = ingetc();
	}
	inungetc(c);
    } else if (c == '^')
	ret = 1;
    else if (c == '$')
	ret = argc;
    else if (c == '%') {
	if (evset) {
	    herrflush();
	    zerr("Ambiguous history reference");
	    return -2;
	}
	if (marg == -1) {
	    herrflush();
	    zerr("%% with no previous word matched");
	    return -2;
	}
	ret = marg;
    } else
	inungetc(c);
    return ret;
}

/* do ?foo? search */

/**/
static zlong
hconsearch(char *str, int *marg)
{
    int t1 = 0;
    char *s;
    Histent he;

    for (he = up_histent(hist_ring); he; he = up_histent(he)) {
	if (he->node.flags & HIST_FOREIGN)
	    continue;
	if ((s = strstr(he->node.nam, str))) {
	    int pos = s - he->node.nam;
	    while (t1 < he->nwords && he->words[2*t1] <= pos)
		t1++;
	    *marg = t1 - 1;
	    return he->histnum;
	}
    }
    return -1;
}

/* do !foo search */

/**/
zlong
hcomsearch(char *str)
{
    Histent he;
    int len = strlen(str);

    for (he = up_histent(hist_ring); he; he = up_histent(he)) {
	if (he->node.flags & HIST_FOREIGN)
	    continue;
	if (strncmp(he->node.nam, str, len) == 0)
	    return he->histnum;
    }
    return -1;
}

/* various utilities for : modifiers */

/**/
int
chabspath(char **junkptr)
{
    char *current, *dest;

    if (!**junkptr)
	return 1;

    if (**junkptr != '/') {
	char *here = zgetcwd();
	if (here[strlen(here)-1] != '/')
	    *junkptr = zhtricat(metafy(here, -1, META_HEAPDUP), "/", *junkptr);
	else
	    *junkptr = dyncat(here, *junkptr);
    }

    current = *junkptr;
    dest = *junkptr;

#ifdef HAVE_SUPERROOT
    while (*current == '/' && current[1] == '.' && current[2] == '.' &&
	   (!current[3] || current[3] == '/')) {
	*dest++ = '/';
	*dest++ = '.';
	*dest++ = '.';
	current += 3;
    }
#endif

    for (;;) {
	if (*current == '/') {
#ifdef __CYGWIN__
	    if (current == *junkptr && current[1] == '/')
		*dest++ = *current++;
#endif
	    *dest++ = *current++;
	    while (*current == '/')
		current++;
	} else if (!*current) {
	    while (dest > *junkptr + 1 && dest[-1] == '/')
		dest--;
	    *dest = '\0';
	    break;
	} else if (current[0] == '.' && current[1] == '.' &&
		   (!current[2] || current[2] == '/')) {
		if (current == *junkptr || dest == *junkptr) {
		    *dest++ = '.';
		    *dest++ = '.';
		    current += 2;
		} else if (dest > *junkptr + 2 &&
			   !strncmp(dest - 3, "../", 3)) {
		    *dest++ = '.';
		    *dest++ = '.';
		    current += 2;
		} else if (dest > *junkptr + 1) {
		    *dest = '\0';
		    for (dest--;
			 dest > *junkptr + 1 && dest[-1] != '/';
			 dest--);
		    if (dest[-1] != '/')
			dest--;
		    current += 2;
		    if (*current == '/')
			current++;
		} else if (dest == *junkptr + 1) {
		    /* This might break with Cygwin's leading double slashes? */
		    current += 2;
		} else {
		    return 0;
		}
	} else if (current[0] == '.' && (current[1] == '/' || !current[1])) {
	     while (*++current == '/');
	} else {
	    while (*current != '/' && *current != '\0')
		if ((*dest++ = *current++) == Meta)
		    *dest++ = *current++;
	}
    }
    return 1;
}

/*
 * Resolve symlinks in junkptr.
 *
 * If mode is 'A', resolve dot-dot before symlinks.  Else, mode should be 'P'.
 * Refer to the documentation of the :A and :P modifiers for details.
 *
 * use_heap is 1 if the result is to be allocated on the heap, 0 otherwise.
 *
 * Return 0 for error, non-zero for success.
 */

/**/
int
chrealpath(char **junkptr, char mode, int use_heap)
{
    char *str;
#ifdef HAVE_REALPATH
# ifdef REALPATH_ACCEPTS_NULL
    char *lastpos, *nonreal, *real;
# else
    char *lastpos, *nonreal, pathbuf[PATH_MAX+1];
    char *real = pathbuf;
# endif
#endif

    DPUTS1(mode != 'A' && mode != 'P', "chrealpath: mode='%c' is invalid", mode);

    if (!**junkptr)
	return 1;

    if (mode == 'A')
	if (!chabspath(junkptr))
	    return 0;

#ifndef HAVE_REALPATH
    return 1;
#else
    /*
     * Notice that this means you cannot pass relative paths into this
     * function!
     */
    if (**junkptr != '/')
	return 0;

    unmetafy(*junkptr, NULL);

    lastpos = strend(*junkptr);
    nonreal = lastpos + 1;

    while (!
#ifdef REALPATH_ACCEPTS_NULL
	   /* realpath() with a NULL second argument uses malloc() to get
	    * memory so we don't need to worry about overflowing PATH_MAX */
	   (real = realpath(*junkptr, NULL))
#else
	   realpath(*junkptr, real)
#endif
	) {
	if (errno == EINVAL || errno == ENOMEM)
	    return 0;

	if (nonreal == *junkptr) {
#ifndef REALPATH_ACCEPTS_NULL
	    real = NULL;
#endif
	    break;
	}

	while (*nonreal != '/' && nonreal >= *junkptr)
	    nonreal--;
	*nonreal = '\0';
    }

    str = nonreal;
    while (str <= lastpos) {
	if (*str == '\0')
	    *str = '/';
	str++;
    }

    use_heap = (use_heap ? META_HEAPDUP : META_DUP);
    if (real) {
	*junkptr = metafy(str = bicat(real, nonreal), -1, use_heap);
	zsfree(str);
#ifdef REALPATH_ACCEPTS_NULL
	free(real);
#endif
    } else {
	*junkptr = metafy(nonreal, lastpos - nonreal + 1, use_heap);
    }
#endif

    return 1;
}

/**/
int
remtpath(char **junkptr, int count)
{
    char *str = strend(*junkptr);

    /* ignore trailing slashes */
    while (str >= *junkptr && IS_DIRSEP(*str))
	--str;
    if (!count) {
	/* skip filename */
	while (str >= *junkptr && !IS_DIRSEP(*str))
	    --str;
    }
    if (str < *junkptr) {
	if (IS_DIRSEP(**junkptr))
	    *junkptr = dupstring ("/");
	else
	    *junkptr = dupstring (".");

	return 0;
    }

    if (count)
    {
	/*
	 * Return this many components, so start from the front.
	 * Leading slash counts as one component, consistent with
	 * behaviour of repeated applications of :h.
	 */
	char *strp = *junkptr;
	while (strp < str) {
	    if (IS_DIRSEP(*strp)) {
		if (--count <= 0) {
		    if (strp == *junkptr)
			++strp;
		    *strp = '\0';
		    return 1;
		}
		/* Count consecutive separators as one */
		while (IS_DIRSEP(strp[1]))
		    ++strp;
	    }
	    ++strp;
	}

	/* Full string needed */
	return 1;
    }

    /* repeated slashes are considered like a single slash */
    while (str > *junkptr && IS_DIRSEP(str[-1]))
	--str;
    /* never erase the root slash */
    if (str == *junkptr) {
	++str;
	/* Leading doubled slashes (`//') have a special meaning on cygwin
	   and some old flavor of UNIX, so we do not assimilate them to
	   a single slash.  However a greater number is ok to squeeze. */
	if (IS_DIRSEP(*str) && !IS_DIRSEP(str[1]))
	    ++str;
    }
    *str = '\0';
    return 1;
}

/**/
int
remtext(char **junkptr)
{
    char *str;

    for (str = strend(*junkptr); str >= *junkptr && !IS_DIRSEP(*str); --str)
	if (*str == '.') {
	    *str = '\0';
	    return 1;
	}
    return 0;
}

/**/
int
rembutext(char **junkptr)
{
    char *str;

    for (str = strend(*junkptr); str >= *junkptr && !IS_DIRSEP(*str); --str)
	if (*str == '.') {
	    *junkptr = dupstring(str + 1); /* .xx or xx? */
	    return 1;
	}
    /* no extension */
    *junkptr = dupstring ("");
    return 0;
}

/**/
mod_export int
remlpaths(char **junkptr, int count)
{
    char *str = strend(*junkptr);

    if (IS_DIRSEP(*str)) {
	/* remove trailing slashes */
	while (str >= *junkptr && IS_DIRSEP(*str))
	    --str;
	str[1] = '\0';
    }
    for (;;) {
	for (; str >= *junkptr; --str) {
	    if (IS_DIRSEP(*str)) {
		if (--count > 0) {
		    if (str > *junkptr) {
			--str;
			break;
		    } else {
			/* Whole string needed */
			return 1;
		    }
		}
		*str = '\0';
		*junkptr = dupstring(str + 1);
		return 1;
	    }
	}
	/* Count consecutive separators as 1 */
	while (str >= *junkptr && IS_DIRSEP(*str))
	    --str;
	if (str <= *junkptr)
	    break;
    }
    return 0;
}

/*
 * Return modified version of str from the heap with modification
 * according to one of the CASMOD_* types defined in zsh.h; CASMOD_NONE
 * is not handled, for obvious reasons.
 */

/**/
char *
casemodify(char *str, int how)
{
    char *str2 = zhalloc(2 * strlen(str) + 1);
    char *ptr2 = str2;
    int nextupper = 1;

#ifdef MULTIBYTE_SUPPORT
    if (isset(MULTIBYTE)) {
	VARARR(char, mbstr, MB_CUR_MAX);
	mbstate_t ps;

	mb_charinit();
	memset(&ps, 0, sizeof(ps));
	while (*str) {
	    wint_t wc;
	    int len = mb_metacharlenconv(str, &wc), mod = 0, len2;
	    /*
	     * wc is set to WEOF if the start of str couldn't be
	     * converted.  Presumably WEOF doesn't match iswlower(), but
	     * better be safe.
	     */
	    if (wc == WEOF) {
		while (len--)
		    *ptr2++ = *str++;
		/* not alphanumeric */
		nextupper = 1;
		continue;
	    }
	    switch (how) {
	    case CASMOD_LOWER:
		if (iswupper(wc)) {
		    wc = towlower(wc);
		    mod = 1;
		}
		break;

	    case CASMOD_UPPER:
		if (iswlower(wc)) {
		    wc = towupper(wc);
		    mod = 1;
		}
		break;

	    case CASMOD_CAPS:
	    default:		/* shuts up compiler */
		if (IS_COMBINING(wc))
			break;
		if (!iswalnum(wc))
		    nextupper = 1;
		else if (nextupper) {
		    if (iswlower(wc)) {
			wc = towupper(wc);
			mod = 1;
		    }
		    nextupper = 0;
		} else if (iswupper(wc)) {
		    wc = towlower(wc);
		    mod = 1;
		}
		break;
	    }
	    if (mod && (len2 = wcrtomb(mbstr, wc, &ps)) > 0) {
		char *mbptr;

		for (mbptr = mbstr; mbptr < mbstr + len2; mbptr++) {
		    if (imeta(STOUC(*mbptr))) {
			*ptr2++ = Meta;
			*ptr2++ = *mbptr ^ 32;
		    } else
			*ptr2++ = *mbptr;
		}
		str += len;
	    } else {
		while (len--)
		    *ptr2++ = *str++;
	    }
	}
    }
    else
#endif
	while (*str) {
	    int c;
	    int mod = 0;
	    if (*str == Meta) {
		c = STOUC(str[1] ^ 32);
		str += 2;
	    } else
		c = STOUC(*str++);
	    switch (how) {
	    case CASMOD_LOWER:
		if (isupper(c)) {
		    c = tolower(c);
		    mod = 1;
		}
		break;

	    case CASMOD_UPPER:
		if (islower(c)) {
		    c = toupper(c);
		    mod = 1;
		}
		break;

	    case CASMOD_CAPS:
	    default:		/* shuts up compiler */
		if (!ialnum(c))
		    nextupper = 1;
		else if (nextupper) {
		    if (islower(c)) {
			c = toupper(c);
			mod = 1;
		    }
		    nextupper = 0;
		} else if (isupper(c)) {
		    c = tolower(c);
		    mod = 1;
		}
		break;
	    }
	    if (mod && imeta(c)) {
		*ptr2++ = Meta;
		*ptr2++ = c ^ 32;
	    } else
		*ptr2++ = c;
	}
    *ptr2 = '\0';
    return str2;
}


/*
 * Substitute "in" for "out" in "*strptr" and update "*strptr".
 * If "gbal", do global substitution.
 *
 * This returns a result from the heap.  There seems to have
 * been some confusion on this point.
 */

/**/
int
subst(char **strptr, char *in, char *out, int gbal)
{
    char *str = *strptr, *substcut, *sptr;
    int off, inlen, outlen;

    if (!*in)
	in = str, gbal = 0;

    if (isset(HISTSUBSTPATTERN)) {
	int fl = SUB_LONG|SUB_REST|SUB_RETFAIL;
	char *oldin = in;
	if (gbal)
	    fl |= SUB_GLOBAL;
	if (*in == '#' || *in == Pound) {
	    /* anchor at head, flag needed if SUB_END is also set */
	    fl |= SUB_START;
	    in++;
	}
	if (*in == '%') {
	    /* anchor at tail */
	    in++;
	    fl |= SUB_END;
	}
	if (in == oldin) {
	    /* no anchor, substring match */
	    fl |= SUB_SUBSTR;
	}
	if (in == str)
	    in = dupstring(in);
	if (parse_subst_string(in) || errflag)
	    return 1;
	if (parse_subst_string(out) || errflag)
	    return 1;
	singsub(&in);
	if (getmatch(strptr, in, fl, 1, out))
	    return 0;
    } else {
	if ((substcut = (char *)strstr(str, in))) {
	    inlen = strlen(in);
	    sptr = convamps(out, in, inlen);
	    outlen = strlen(sptr);

	    do {
		*substcut = '\0';
		off = substcut - *strptr + outlen;
		substcut += inlen;
		*strptr = zhtricat(*strptr, sptr, substcut);
		str = (char *)*strptr + off;
	    } while (gbal && (substcut = (char *)strstr(str, in)));

	    return 0;
	}
    }

    return 1;
}

/**/
static char *
convamps(char *out, char *in, int inlen)
{
    char *ptr, *ret, *pp;
    int slen, sdup = 0;

    for (ptr = out, slen = 0; *ptr; ptr++, slen++)
	if (*ptr == '\\')
	    ptr++, sdup = 1;
	else if (*ptr == '&')
	    slen += inlen - 1, sdup = 1;
    if (!sdup)
	return out;
    ret = pp = (char *) zhalloc(slen + 1);
    for (ptr = out; *ptr; ptr++)
	if (*ptr == '\\')
	    *pp++ = *++ptr;
	else if (*ptr == '&') {
	    strcpy(pp, in);
	    pp += inlen;
	} else
	    *pp++ = *ptr;
    *pp = '\0';
    return ret;
}

/**/
mod_export void
checkcurline(Histent he)
{
    if (he->histnum == curhist && (histactive & HA_ACTIVE)) {
	curline.node.nam = chline;
	curline.nwords = chwordpos/2;
	curline.words = chwords;
    }
}

/**/
mod_export Histent
quietgethist(int ev)
{
    return gethistent(ev, GETHIST_EXACT);
}

/**/
static Histent
gethist(int ev)
{
    Histent ret;

    ret = quietgethist(ev);
    if (!ret) {
	herrflush();
	zerr("no such event: %d", ev);
    }
    return ret;
}

/**/
static char *
getargs(Histent elist, int arg1, int arg2)
{
    short *words = elist->words;
    int pos1, pos2, nwords = elist->nwords;

    if (arg2 < arg1 || arg1 >= nwords || arg2 >= nwords) {
	/* remember, argN is indexed from 0, nwords is total no. of words */
	herrflush();
	zerr("no such word in event");
	return NULL;
    }

    /* optimization for accessing entire history event */
    if (arg1 == 0 && arg2 == nwords - 1)
	return dupstring(elist->node.nam);

    pos1 = words[2*arg1];
    pos2 = words[2*arg2+1];

    /* a word has to be at least one character long, so if the position
     * of a word is less than its index, we've overflowed our signed
     * short integer word range and the recorded position is garbage. */
    if (pos1 < 0 || pos1 < arg1 || pos2 < 0 || pos2 < arg2) {
	herrflush();
	zerr("history event too long, can't index requested words");
	return NULL;
    }
    return dupstrpfx(elist->node.nam + pos1, pos2 - pos1);
}

/**/
static int
quote(char **tr)
{
    char *ptr, *rptr, **str = tr;
    int len = 3;
    int inquotes = 0;

    for (ptr = *str; *ptr; ptr++, len++)
	if (*ptr == '\'') {
	    len += 3;
	    if (!inquotes)
		inquotes = 1;
	    else
		inquotes = 0;
	} else if (inblank(*ptr) && !inquotes && ptr[-1] != '\\')
	    len += 2;
    ptr = *str;
    *str = rptr = (char *) zhalloc(len);
    *rptr++ = '\'';
    for (; *ptr; ptr++)
	if (*ptr == '\'') {
	    if (!inquotes)
		inquotes = 1;
	    else
		inquotes = 0;
	    *rptr++ = '\'';
	    *rptr++ = '\\';
	    *rptr++ = '\'';
	    *rptr++ = '\'';
	} else if (inblank(*ptr) && !inquotes && ptr[-1] != '\\') {
	    *rptr++ = '\'';
	    *rptr++ = *ptr;
	    *rptr++ = '\'';
	} else
	    *rptr++ = *ptr;
    *rptr++ = '\'';
    *rptr++ = 0;
    return 0;
}

/**/
static int
quotebreak(char **tr)
{
    char *ptr, *rptr, **str = tr;
    int len = 3;

    for (ptr = *str; *ptr; ptr++, len++)
	if (*ptr == '\'')
	    len += 3;
	else if (inblank(*ptr))
	    len += 2;
    ptr = *str;
    *str = rptr = (char *) zhalloc(len);
    *rptr++ = '\'';
    for (; *ptr;)
	if (*ptr == '\'') {
	    *rptr++ = '\'';
	    *rptr++ = '\\';
	    *rptr++ = '\'';
	    *rptr++ = '\'';
	    ptr++;
	} else if (inblank(*ptr)) {
	    *rptr++ = '\'';
	    *rptr++ = *ptr++;
	    *rptr++ = '\'';
	} else
	    *rptr++ = *ptr++;
    *rptr++ = '\'';
    *rptr++ = '\0';
    return 0;
}

/* read an arbitrary amount of data into a buffer until stop is found */

#if 0 /**/
char *
hdynread(int stop)
{
    int bsiz = 256, ct = 0, c;
    char *buf = (char *)zalloc(bsiz), *ptr;

    ptr = buf;
    while ((c = ingetc()) != stop && c != '\n' && !lexstop) {
	if (c == '\\')
	    c = ingetc();
	*ptr++ = c;
	if (++ct == bsiz) {
	    buf = realloc(buf, bsiz *= 2);
	    ptr = buf + ct;
	}
    }
    *ptr = 0;
    if (c == '\n') {
	inungetc('\n');
	zerr("delimiter expected");
	zfree(buf, bsiz);
	return NULL;
    }
    return buf;
}
#endif

/**/
static char *
hdynread2(int stop)
{
    int bsiz = 256, ct = 0, c;
    char *buf = (char *)zalloc(bsiz), *ptr;

    ptr = buf;
    while ((c = ingetc()) != stop && c != '\n' && !lexstop) {
	if (c == '\\')
	    c = ingetc();
	*ptr++ = c;
	if (++ct == bsiz) {
	    buf = realloc(buf, bsiz *= 2);
	    ptr = buf + ct;
	}
    }
    *ptr = 0;
    if (c == '\n')
	inungetc('\n');
    return buf;
}

/**/
void
inithist(void)
{
    createhisttable();
}

/**/
void
resizehistents(void)
{
    if (histlinect > histsiz) {
	/* The reason we don't just call freehistnode(hist_ring->down) is
	 * so that we can honor the HISTEXPIREDUPSFIRST setting. */
	putoldhistentryontop(0);
	freehistnode(&hist_ring->node);
	while (histlinect > histsiz) {
	    putoldhistentryontop(1);
	    freehistnode(&hist_ring->node);
	}
    }
}

static int
readhistline(int start, char **bufp, int *bufsiz, FILE *in, int *readbytes)
{
    char *buf = *bufp;
    if (fgets(buf + start, *bufsiz - start, in)) {
	int len = strlen(buf + start);
	*readbytes += len;
	len += start;
	if (len == start)
	    return -1;
	if (buf[len - 1] != '\n') {
	    if (!feof(in)) {
		if (len < (*bufsiz) - 1)
		    return -1;
		*bufp = zrealloc(buf, 2 * (*bufsiz));
		*bufsiz = 2 * (*bufsiz);
		return readhistline(len, bufp, bufsiz, in, readbytes);
	    }
	}
	else {
	    int spc;
	    buf[len - 1] = '\0';
	    if (len > 1 && buf[len - 2] == '\\') {
		buf[--len - 1] = '\n';
		if (!feof(in))
		    return readhistline(len, bufp, bufsiz, in, readbytes);
	    }

	    spc = len - 2;
	    while (spc >= 0 && buf[spc] == ' ')
		spc--;
	    if (spc != len - 2 && buf[spc] == '\\')
		buf[--len - 1] = '\0';
	}
	return len;
    }
    return 0;
}

/**/
void
readhistfile(char *fn, int err, int readflags)
{
    char *buf, *start = NULL;
    FILE *in;
    Histent he;
    time_t stim, ftim, tim = time(NULL);
    off_t fpos;
    short *words;
    struct stat sb;
    int nwordpos, nwords, bufsiz;
    int searching, newflags, l, ret, uselex, readbytes;

    if (!fn && !(fn = getsparam("HISTFILE")))
	return;
    if (stat(unmeta(fn), &sb) < 0 ||
	sb.st_size == 0)
	return;
    if (readflags & HFILE_FAST) {
	if (!lasthist.interrupted &&
	    ((lasthist.fsiz == sb.st_size && lasthist.mtim == sb.st_mtime)
	     || lockhistfile(fn, 0)))
	    return;
	lasthist.fsiz = sb.st_size;
	lasthist.mtim = sb.st_mtime;
	lasthist.interrupted = 0;
    } else if ((ret = lockhistfile(fn, 1))) {
	if (ret == 2) {
	    zwarn("locking failed for %s: %e: reading anyway", fn, errno);
	} else {
	    zerr("locking failed for %s: %e", fn, errno);
	    return;
	}
    }
    if ((in = fopen(unmeta(fn), "r"))) {
	nwords = 64;
	words = (short *)zalloc(nwords*sizeof(short));
	bufsiz = 1024;
	buf = zalloc(bufsiz);

	pushheap();
	if (readflags & HFILE_FAST && lasthist.text) {
	    if (lasthist.fpos < lasthist.fsiz) {
		fseek(in, lasthist.fpos, SEEK_SET);
		searching = 1;
	    }
	    else {
		histfile_linect = 0;
		searching = -1;
	    }
	} else
	    searching = 0;

	fpos = ftell(in);
	readbytes = 0;
	newflags = HIST_OLD | HIST_READ;
	if (readflags & HFILE_FAST)
	    newflags |= HIST_FOREIGN;
	if (readflags & HFILE_SKIPOLD
	 || (hist_ignore_all_dups && newflags & hist_skip_flags))
	    newflags |= HIST_MAKEUNIQUE;
	while (fpos += readbytes, readbytes = 0, (l = readhistline(0, &buf, &bufsiz, in, &readbytes))) {
	    char *pt;
	    int remeta = 0;

	    if (l < 0) {
		zerr("corrupt history file %s", fn);
		break;
	    }

	    /*
	     * Handle the special case that we're reading from an
	     * old shell with fewer meta characters, so we need to
	     * metafy some more.  (It's not clear why the history
	     * file is metafied at all; some would say this is plain
	     * stupid.  But we're stuck with it now without some
	     * hairy workarounds for compatibility).
	     *
	     * This is rare so doesn't need to be that efficient; just
	     * allocate space off the heap.
	     */
	    for (pt = buf; *pt; pt++) {
		if (*pt == Meta && pt[1])
		    pt++;
		else if (imeta(*pt)) {
		    remeta = 1;
		    break;
		}
	    }
	    if (remeta) {
		unmetafy(buf, &remeta);
		pt = metafy(buf, remeta, META_USEHEAP);
	    } else {
		pt = buf;
	    }

	    if (*pt == ':') {
		pt++;
		stim = zstrtol(pt, NULL, 0);
		for (; *pt != ':' && *pt; pt++);
		if (*pt) {
		    pt++;
		    ftim = zstrtol(pt, NULL, 0);
		    for (; *pt != ';' && *pt; pt++);
		    if (*pt)
			pt++;
		} else
		    ftim = stim;
	    } else {
		if (*pt == '\\' && pt[1] == ':')
		    pt++;
		stim = ftim = 0;
	    }

	    if (searching) {
		if (searching > 0) {
		    if (stim == lasthist.stim
		     && histstrcmp(pt, lasthist.text) == 0)
			searching = 0;
		    else {
			fseek(in, 0, SEEK_SET);
			histfile_linect = 0;
			searching = -1;
		    }
		    continue;
		}
		else if (stim < lasthist.stim) {
		    histfile_linect++;
		    continue;
		}
		searching = 0;
	    }

	    if (readflags & HFILE_USE_OPTIONS) {
		histfile_linect++;
		lasthist.fpos = fpos;
		lasthist.stim = stim;
	    }

	    he = prepnexthistent();
	    he->node.nam = ztrdup(pt);
	    he->node.flags = newflags;
	    if ((he->stim = stim) == 0)
		he->stim = he->ftim = tim;
	    else if (ftim < stim)
		he->ftim = stim + ftim;
	    else
		he->ftim = ftim;

	    /*
	     * Divide up the words.
	     */
	    start = pt;
	    uselex = isset(HISTLEXWORDS) && !(readflags & HFILE_FAST);
	    histsplitwords(pt, &words, &nwords, &nwordpos, uselex);

	    he->nwords = nwordpos/2;
	    if (he->nwords) {
		he->words = (short *)zalloc(nwordpos*sizeof(short));
		memcpy(he->words, words, nwordpos*sizeof(short));
	    } else
		he->words = (short *)NULL;
	    addhistnode(histtab, he->node.nam, he);
	    if (he->node.flags & HIST_DUP) {
		freehistnode(&he->node);
		curhist--;
	    }
	    /*
	     * Do this last out of paranoia in case use of
	     * heap is disguised...
	     */
	    if (uselex || remeta)
		freeheap();
	    if (errflag & ERRFLAG_INT) {
		/* Can't assume fast read next time if interrupted. */
		lasthist.interrupted = 1;
		break;
	    }
	}
	if (start && readflags & HFILE_USE_OPTIONS) {
	    zsfree(lasthist.text);
	    lasthist.text = ztrdup(start);
	}
	zfree(words, nwords*sizeof(short));
	zfree(buf, bufsiz);

	popheap();
	fclose(in);
    } else if (err)
	zerr("can't read history file %s", fn);

    unlockhistfile(fn);

    if (zleactive)
	zleentry(ZLE_CMD_SET_HIST_LINE, curhist);
}

#ifdef HAVE_FCNTL_H
static int flock_fd = -1;

/*
 * Lock file using fcntl().  Return 0 on success, 1 on failure of
 * locking mechanism, 2 on permanent failure (e.g. permission).
 */

static int
flockhistfile(char *fn, int keep_trying)
{
    struct flock lck;
    long sleep_us = 0x10000; /* about 67 ms */
    time_t end_time;

    if (flock_fd >= 0)
	return 0; /* already locked */

    if ((flock_fd = open(unmeta(fn), O_RDWR | O_NOCTTY)) < 0)
	return errno == ENOENT ? 0 : 2; /* "successfully" locked missing file */

    lck.l_type = F_WRLCK;
    lck.l_whence = SEEK_SET;
    lck.l_start = 0;
    lck.l_len = 0;  /* lock the whole file */

    /*
     * Timeout is ten seconds.
     */
    end_time = time(NULL) + (time_t)10;
    while (fcntl(flock_fd, F_SETLKW, &lck) == -1) {
	if (!keep_trying || time(NULL) >= end_time ||
	    /*
	     * Randomise wait to minimise clashes with shells exiting at
	     * the same time.
	     */
	    !zsleep_random(sleep_us, end_time)) {
	    close(flock_fd);
	    flock_fd = -1;
	    return 1;
	}
	sleep_us <<= 1;
    }

    return 0;
}
#endif

/**/
void
savehistfile(char *fn, int err, int writeflags)
{
    char *t, *tmpfile, *start = NULL;
    FILE *out;
    Histent he;
    zlong xcurhist = curhist - !!(histactive & HA_ACTIVE);
    int extended_history = isset(EXTENDEDHISTORY);
    int ret;

    if (!interact || savehistsiz <= 0 || !hist_ring
     || (!fn && !(fn = getsparam("HISTFILE"))))
	return;
    if (writeflags & HFILE_FAST) {
	he = gethistent(lasthist.next_write_ev, GETHIST_DOWNWARD);
	while (he && he->node.flags & HIST_OLD) {
	    lasthist.next_write_ev = he->histnum + 1;
	    he = down_histent(he);
	}
	if (!he || lockhistfile(fn, 0))
	    return;
	if (histfile_linect > savehistsiz + savehistsiz / 5)
	    writeflags &= ~HFILE_FAST;
    }
    else {
	if (lockhistfile(fn, 1)) {
	    zerr("locking failed for %s: %e", fn, errno);
	    return;
	}
	he = hist_ring->down;
    }
    if (writeflags & HFILE_USE_OPTIONS) {
	if (isset(APPENDHISTORY) || isset(INCAPPENDHISTORY)
	 || isset(INCAPPENDHISTORYTIME) || isset(SHAREHISTORY))
	    writeflags |= HFILE_APPEND | HFILE_SKIPOLD;
	else
	    histfile_linect = 0;
	if (isset(HISTSAVENODUPS))
	    writeflags |= HFILE_SKIPDUPS;
	if (isset(SHAREHISTORY))
	    extended_history = 1;
    }
    errno = 0;
    if (writeflags & HFILE_APPEND) {
	int fd = open(unmeta(fn), O_CREAT | O_WRONLY | O_APPEND | O_NOCTTY, 0600);
	tmpfile = NULL;
	out = fd >= 0 ? fdopen(fd, "a") : NULL;
    } else if (!isset(HISTSAVEBYCOPY)) {
	int fd = open(unmeta(fn), O_CREAT | O_WRONLY | O_TRUNC | O_NOCTTY, 0600);
	tmpfile = NULL;
	out = fd >= 0 ? fdopen(fd, "w") : NULL;
    } else {
	tmpfile = bicat(unmeta(fn), ".new");
	if (unlink(tmpfile) < 0 && errno != ENOENT)
	    out = NULL;
	else {
	    struct stat sb;
	    int old_exists = stat(unmeta(fn), &sb) == 0;
	    uid_t euid = geteuid();

	    if (old_exists
#if defined HAVE_FCHMOD && defined HAVE_FCHOWN
	     && euid
#endif
	     && sb.st_uid != euid) {
		free(tmpfile);
		tmpfile = NULL;
		if (err) {
		    if (isset(APPENDHISTORY) || isset(INCAPPENDHISTORY)
		     || isset(INCAPPENDHISTORYTIME) || isset(SHAREHISTORY))
			zerr("rewriting %s would change its ownership -- skipped", fn);
		    else
			zerr("rewriting %s would change its ownership -- history not saved", fn);
		    err = 0; /* Don't report a generic error below. */
		}
		out = NULL;
	    } else {
		int fd = open(tmpfile, O_CREAT | O_WRONLY | O_EXCL, 0600);
		if (fd >=0) {
		    out = fdopen(fd, "w");
		    if (!out)
			close(fd);
		} else
		    out = NULL;
	    }

#ifdef HAVE_FCHMOD
	    if (old_exists && out) {
#ifdef HAVE_FCHOWN
		if (fchown(fileno(out), sb.st_uid, sb.st_gid) < 0) {} /* IGNORE FAILURE */
#endif
		if (fchmod(fileno(out), sb.st_mode) < 0) {} /* IGNORE FAILURE */
	    }
#endif
	}
    }
    if (out) {
	char *history_ignore;
	Patprog histpat = NULL;

	pushheap();

	if ((history_ignore = getsparam("HISTORY_IGNORE")) != NULL) {
	    tokenize(history_ignore = dupstring(history_ignore));
	    remnulargs(history_ignore);
	    histpat = patcompile(history_ignore, 0, NULL);
	}

	ret = 0;
	for (; he && he->histnum <= xcurhist; he = down_histent(he)) {
	    int end_backslashes = 0;

	    if ((writeflags & HFILE_SKIPDUPS && he->node.flags & HIST_DUP)
	     || (writeflags & HFILE_SKIPFOREIGN && he->node.flags & HIST_FOREIGN)
	     || he->node.flags & HIST_TMPSTORE)
		continue;
	    if (histpat &&
		pattry(histpat, metafy(he->node.nam, -1, META_HEAPDUP))) {
		continue;
	    }
	    if (writeflags & HFILE_SKIPOLD) {
		if (he->node.flags & (HIST_OLD|HIST_NOWRITE))
		    continue;
		he->node.flags |= HIST_OLD;
		if (writeflags & HFILE_USE_OPTIONS)
		    lasthist.next_write_ev = he->histnum + 1;
	    }
	    if (writeflags & HFILE_USE_OPTIONS) {
		lasthist.fpos = ftell(out);
		lasthist.stim = he->stim;
		histfile_linect++;
	    }
	    t = start = he->node.nam;
	    if (extended_history) {
		ret = fprintf(out, ": %ld:%ld;", (long)he->stim,
			      he->ftim? (long)(he->ftim - he->stim) : 0L);
	    } else if (*t == ':')
		ret = fputc('\\', out);

	    for (; ret >= 0 && *t; t++) {
		if (*t == '\n')
		    if ((ret = fputc('\\', out)) < 0)
			break;
		end_backslashes = (*t == '\\' || (end_backslashes && *t == ' '));
		if ((ret = fputc(*t, out)) < 0)
		    break;
	    }
	    if (ret < 0)
	    	break;
	    if (end_backslashes)
		ret = fputc(' ', out);
	    if (ret < 0 || (ret = fputc('\n', out)) < 0)
		break;
	}
	if (ret >= 0 && start && writeflags & HFILE_USE_OPTIONS) {
	    struct stat sb;
	    if ((ret = fflush(out)) >= 0) {
		if (fstat(fileno(out), &sb) == 0) {
		    lasthist.fsiz = sb.st_size;
		    lasthist.mtim = sb.st_mtime;
		}
		zsfree(lasthist.text);
		lasthist.text = ztrdup(start);
	    }
	}
	if (fclose(out) < 0 && ret >= 0)
	    ret = -1;
	if (ret >= 0) {
	    if (tmpfile) {
		if (rename(tmpfile, unmeta(fn)) < 0) {
		    zerr("can't rename %s.new to $HISTFILE", fn);
		    ret = -1;
		    err = 0;
#ifdef HAVE_FCNTL_H
		} else {
		    /* We renamed over the locked HISTFILE, so close fd.
		     * If we do more writing, we'll get a lock then. */
		    if (flock_fd >= 0) {
			close(flock_fd);
			flock_fd = -1;
		    }
#endif
		}
	    }

	    if (ret >= 0 && writeflags & HFILE_SKIPOLD
		&& !(writeflags & (HFILE_FAST | HFILE_NO_REWRITE))) {
		int remember_histactive = histactive;

		/* Zeroing histactive avoids unnecessary munging of curline. */
		histactive = 0;
		/* The NULL leaves HISTFILE alone, preserving fn's value. */
		pushhiststack(NULL, savehistsiz, savehistsiz, -1);

		hist_ignore_all_dups |= isset(HISTSAVENODUPS);
		readhistfile(fn, err, 0);
		hist_ignore_all_dups = isset(HISTIGNOREALLDUPS);
		if (histlinect)
		    savehistfile(fn, err, 0);

		pophiststack();
		histactive = remember_histactive;
	    }
	}

	popheap();
    } else
	ret = -1;

    if (ret < 0 && err) {
	if (tmpfile)
	    zerr("failed to write history file %s.new: %e", fn, errno);
	else
	    zerr("failed to write history file %s: %e", fn, errno);
    }
    if (tmpfile)
	free(tmpfile);

    unlockhistfile(fn);
}

static int lockhistct;

static int
checklocktime(char *lockfile, long *sleep_usp, time_t then)
{
    time_t now = time(NULL);

    if (now + 10 < then) {
	/* File is more than 10 seconds in the future? */
	errno = EEXIST;
	return -1;
    }

    if (now - then < 10) {
	/*
	 * To give the effect of a gradually increasing backoff,
	 * we'll sleep a period based on the time we've spent so far.
	 */
	DPUTS(now < then, "time flowing backwards through history");
	/*
	 * Randomise to minimise clashes with shells exiting at the same
	 * time.
	 */
	(void)zsleep_random(*sleep_usp, then + 10);
	*sleep_usp <<= 1;
    } else
	unlink(lockfile);

    return 0;
}

/*
 * Lock history file.  Return 0 on success, 1 on failure to lock this
 * time, 2 on permanent failure (e.g. permission).
 */

/**/
int
lockhistfile(char *fn, int keep_trying)
{
    int ct = lockhistct;
    int ret = 0;
    long sleep_us = 0x10000; /* about 67 ms */

    if (!fn && !(fn = getsparam("HISTFILE")))
	return 1;

    if (!lockhistct++) {
	struct stat sb;
	int fd;
	char *lockfile;
#ifdef HAVE_LINK
# ifdef HAVE_SYMLINK
	char pidbuf[32], *lnk;
# else
	char *tmpfile;
# endif
#endif

#ifdef HAVE_FCNTL_H
	if (isset(HISTFCNTLLOCK))
	    return flockhistfile(fn, keep_trying);
#endif

	lockfile = bicat(unmeta(fn), ".LOCK");
	/* NOTE: only use symlink locking on a link()-having host in order to
	 * avoid a change from open()-based locking to symlink()-based. */
#ifdef HAVE_LINK
# ifdef HAVE_SYMLINK
	sprintf(pidbuf, "/pid-%ld/host-", (long)mypid);
	lnk = getsparam("HOST");
	lnk = bicat(pidbuf, lnk ? lnk : "");
	/* We'll abuse fd as our success flag. */
	while ((fd = symlink(lnk, lockfile)) < 0) {
	    if (errno != EEXIST) {
		ret = 2;
		break;
	    } else if (!keep_trying) {
		ret = 1;
		break;
	    }
	    if (lstat(lockfile, &sb) < 0) {
		if (errno == ENOENT)
		    continue;
		break;
	    }
	    if (checklocktime(lockfile, &sleep_us, sb.st_mtime) < 0) {
		ret = 1;
		break;
	    }
	}
	if (fd < 0)
	    lockhistct--;
	free(lnk);
# else /* not HAVE_SYMLINK */
	if ((fd = gettempfile(fn, 0, &tmpfile)) >= 0) {
	    FILE *out = fdopen(fd, "w");
	    if (out) {
		fprintf(out, "%ld %s\n", (long)getpid(), getsparam("HOST"));
		fclose(out);
	    } else
		close(fd);
	    while (link(tmpfile, lockfile) < 0) {
		if (errno != EEXIST) {
		    ret = 2;
		    break;
		} else if (!keep_trying) {
		    ret = 1;
		    break;
		} else if (lstat(lockfile, &sb) < 0) {
		    if (errno == ENOENT)
			continue;
		    ret = 2;
		} else {
		    if (checklocktime(lockfile, &sleep_us, sb.st_mtime) < 0) {
			ret = 1;
			break;
		    }
		    continue;
		}
		lockhistct--;
		break;
	    }
	    unlink(tmpfile);
	    free(tmpfile);
	}
# endif /* not HAVE_SYMLINK */
#else /* not HAVE_LINK */
	while ((fd = open(lockfile, O_WRONLY|O_CREAT|O_EXCL, 0644)) < 0) {
	    if (errno != EEXIST) {
		ret = 2;
		break;
	    } else if (!keep_trying) {
		ret = 1;
		break;
	    }
	    if (lstat(lockfile, &sb) < 0) {
		if (errno == ENOENT)
		    continue;
		ret = 2;
		break;
	    }
	    if (checklocktime(lockfile, &sleep_us, sb.st_mtime) < 0) {
		ret = 1;
		break;
	    }
	}
	if (fd < 0)
	    lockhistct--;
	else {
	    FILE *out = fdopen(fd, "w");
	    if (out) {
		fprintf(out, "%ld %s\n", (long)mypid, getsparam("HOST"));
		fclose(out);
	    } else
		close(fd);
	}
#endif /* not HAVE_LINK */
	free(lockfile);
    }

    if (ct == lockhistct) {
#ifdef HAVE_FCNTL_H
	if (flock_fd >= 0) {
	    close(flock_fd);
	    flock_fd = -1;
	}
#endif
	DPUTS(ret == 0, "BUG: return value non-zero on locking error");
	return ret;
    }
    return 0;
}

/* Unlock the history file if this corresponds to the last nested lock
 * request.  If we don't have the file locked, just return.
 */

/**/
void
unlockhistfile(char *fn)
{
    if (!fn && !(fn = getsparam("HISTFILE")))
	return;
    if (--lockhistct) {
	if (lockhistct < 0)
	    lockhistct = 0;
    }
    else {
	char *lockfile;
	fn = unmeta(fn);
	lockfile = zalloc(strlen(fn) + 5 + 1);
	sprintf(lockfile, "%s.LOCK", fn);
	unlink(lockfile);
	free(lockfile);
#ifdef HAVE_FCNTL_H
	if (flock_fd >= 0) {
	    close(flock_fd);
	    flock_fd = -1;
	}
#endif
    }
}

/**/
int
histfileIsLocked(void)
{
    return lockhistct > 0;
}

/*
 * Get the words in the current buffer. Using the lexer. 
 *
 * As far as I can make out, this is a gross hack based on a gross hack.
 * When analysing lines from within zle, we tweak the metafied line
 * positions (zlemetall and zlemetacs) directly in the lexer.  That's
 * bad enough, but this function appears to be designed to be called
 * from outside zle, pretending to be in zle and calling out, so
 * we set zlemetall and zlemetacs locally and copy the current zle line,
 * which may not even be valid at this point.
 *
 * However, I'm so confused it could simply be baking Bakewell tarts.
 *
 * list may be an existing linked list (off the heap), in which case
 * it will be appended to; otherwise it will be created.
 *
 * If buf is set we will take input from that string, else we will
 * attempt to use ZLE directly in a way they tell you not to do on all
 * programming courses.
 *
 * If index is non-NULL, and input is from a string in ZLE, *index
 * is set to the position of the end of the current editor word.
 *
 * flags is passed directly to lexflags, see lex.c, except that
 * we 'or' in the bit LEXFLAGS_ACTIVE to make sure the variable
 * is set.
 */

/**/
mod_export LinkList
bufferwords(LinkList list, char *buf, int *index, int flags)
{
    int num = 0, cur = -1, got = 0, ne = noerrs;
    int owb = wb, owe = we, oadx = addedx, onc = nocomments;
    int ona = noaliases, ocs = zlemetacs, oll = zlemetall;
    int forloop = 0, rcquotes = opts[RCQUOTES];
    int envarray = 0;
    char *p, *addedspaceptr;

    if (!list)
	list = newlinklist();

    /*
     * With RC_QUOTES, 'foo '' bar' comes back as 'foo ' bar'.  That's
     * not very useful.  As nothing in here requires the fully processed
     * string expression, we just turn the option off for this function.
     */
    opts[RCQUOTES] = 0;
    addedx = 0;
    noerrs = 1;
    zcontext_save();
    lexflags = flags | LEXFLAGS_ACTIVE;
    /*
     * Are we handling comments?
     */
    nocomments = !(flags & (LEXFLAGS_COMMENTS_KEEP|
			    LEXFLAGS_COMMENTS_STRIP));
    if (buf) {
	int l = strlen(buf);

	p = (char *) zhalloc(l + 2);
	memcpy(p, buf, l);
	/*
	 * I'm sure this space is here for a reason, but it's
	 * a pain in the neck:  when we get back a string that's
	 * not finished it's very hard to tell if a space at the
	 * end is this one or not.  We use two tricks below to
	 * work around this.
	 */
	addedspaceptr = p + l;
	*addedspaceptr = ' ';
	addedspaceptr[1] = '\0';
	inpush(p, 0, NULL);
	zlemetall = strlen(p) ;
	zlemetacs = zlemetall + 1;
    } else {
	int ll, cs;
	char *linein;

	linein = zleentry(ZLE_CMD_GET_LINE, &ll, &cs);
	zlemetall = ll + 1; /* length of line plus space added below */
	zlemetacs = cs;

	if (!isfirstln && chline) {
	    p = (char *) zhalloc(hptr - chline + ll + 2);
	    memcpy(p, chline, hptr - chline);
	    memcpy(p + (hptr - chline), linein, ll);
	    addedspaceptr = p + (hptr - chline) + ll;
	    *addedspaceptr = ' ';
	    addedspaceptr[1] = '\0';
	    inpush(p, 0, NULL);

	    /*
	     * advance line length and character position over
	     * prepended string.
	     */
	    zlemetall += hptr - chline;
	    zlemetacs += hptr - chline;
	} else {
	    p = (char *) zhalloc(ll + 2);
	    memcpy(p, linein, ll);
	    addedspaceptr = p + ll;
	    *addedspaceptr = ' ';
	    p[zlemetall] = '\0';
	    inpush(p, 0, NULL);
	}
	zsfree(linein);
    }
    if (zlemetacs)
	zlemetacs--;
    strinbeg(0);
    noaliases = 1;
    do {
	if (incond)
	    incond = 1 + (tok != DINBRACK && tok != INPAR &&
			  tok != DBAR && tok != DAMPER &&
			  tok != BANG);
	ctxtlex();
	if (tok == ENDINPUT || tok == LEXERR)
	    break;
	/*
	 * After an array assignment, return to the initial
	 * start-of-command state.  There could be a second ENVARRAY.
	 */
	if (tok == OUTPAR && envarray) {
	    incmdpos = 1;
	    envarray = 0;
	}
	if (tok == FOR) {
	    /*
	     * The way for (( expr1 ; expr2; expr3 )) is parsed is:
	     * - a FOR tok
	     * - a DINPAR with no tokstr
	     * - two DINPARS with tokstr's expr1, expr2.
	     * - a DOUTPAR with tokstr expr3.
	     *
	     * We'll decrement the variable forloop as we verify
	     * the various stages.
	     *
	     * Don't ask me, ma'am, I'm just the programmer.
	     */
	    forloop = 5;
	} else {
	    switch (forloop) {
	    case 1:
		if (tok != DOUTPAR)
		    forloop = 0;
		break;

	    case 2:
	    case 3:
	    case 4:
		if (tok != DINPAR)
		    forloop = 0;
		break;

	    default:
		/* nothing to do */
		break;
	    }
	}
	if (tokstr) {
	    switch (tok) {
	    case ENVARRAY:
		p = dyncat(tokstr, "=(");
		envarray = 1;
		break;

	    case DINPAR:
		if (forloop) {
		    /* See above. */
		    p = dyncat(tokstr, ";");
		} else {
		    /*
		     * Mathematical expressions analysed as a single
		     * word.  That's correct because it behaves like
		     * double quotes.  Whitespace in the middle is
		     * similarly retained, so just add the parentheses back.
		     */
		    p = zhtricat("((", tokstr, "))");
		}
		break;

	    default:
		p = dupstring(tokstr);
		break;
	    }
	    if (*p) {
		untokenize(p);
		if (ingetptr() == addedspaceptr + 1) {
		    /*
		     * Whoops, we've read past the space we added, probably
		     * because we were expecting a terminator but when
		     * it didn't turn up we shrugged our shoulders thinking
		     * it might as well be a complete string anyway.
		     * So remove the space.  C.f. below for the case
		     * where the missing terminator caused a lex error.
		     * We use the same paranoid test.
		     */
		    int plen = strlen(p);
		    if (plen && p[plen-1] == ' ' &&
			(plen == 1 || p[plen-2] != Meta))
			p[plen-1] = '\0';
		}
		addlinknode(list, p);
		num++;
	    }
	} else if (buf) {
	    if (IS_REDIROP(tok) && tokfd >= 0) {
		char b[20];

		sprintf(b, "%d%s", tokfd, tokstrings[tok]);
		addlinknode(list, dupstring(b));
		num++;
	    } else if (tok != NEWLIN) {
		addlinknode(list, dupstring(tokstrings[tok]));
		num++;
	    }
	}
	if (forloop) {
	    if (forloop == 1) {
		/*
		 * Final "))" of for loop to match opening,
		 * since we've just added the preceding element.
 		 */
		addlinknode(list, dupstring("))"));
	    }
	    forloop--;
	}
	if (!got && !lexflags) {
	    got = 1;
	    cur = num - 1;
	}
    } while (tok != ENDINPUT && tok != LEXERR && !(errflag & ERRFLAG_INT));
    if (buf && tok == LEXERR && tokstr && *tokstr) {
	int plen;
	untokenize((p = dupstring(tokstr)));
	plen = strlen(p);
	/*
	 * Strip the space we added for lexing but which won't have
	 * been swallowed by the lexer because we aborted early.
	 * The test is paranoia.
	 */
	if (plen && p[plen-1] == ' ' && (plen == 1 || p[plen-2] != Meta))
	    p[plen - 1] = '\0';
	addlinknode(list, p);
	num++;
    }
    if (cur < 0 && num)
	cur = num - 1;
    noaliases = ona;
    strinend();
    inpop();
    errflag &= ~ERRFLAG_ERROR;
    nocomments = onc;
    noerrs = ne;
    zcontext_restore();
    zlemetacs = ocs;
    zlemetall = oll;
    wb = owb;
    we = owe;
    addedx = oadx;
    opts[RCQUOTES] = rcquotes;

    if (index)
	*index = cur;

    return list;
}

/*
 * Split up a line into words for use in a history file.
 *
 * lineptr is the line to be split.
 *
 * *wordsp and *nwordsp are an array already allocated to hold words
 * and its length.  The array holds both start and end positions,
 * so *nwordsp actually counts twice the number of words in the
 * original string.  *nwordsp may be zero in which case the array
 * will be allocated.
 *
 * *nwordposp returns the used length of *wordsp in the same units as
 * *nwordsp, i.e. twice the number of words in the input line.
 *
 * If uselex is 1, attempt to do this using the lexical analyser.
 * This is more accurate, but slower; for reading history files it's
 * controlled by the option HISTLEXWORDS.  If this failed (which
 * indicates a bug in the shell) it falls back to whitespace-separated
 * strings, printing a message if in debug mode.
 *
 * If uselex is 0, just look for whitespace-separated words; the only
 * special handling is for a backslash-newline combination as used
 * by the history file format to save multiline buffers.
 */
/**/
mod_export void
histsplitwords(char *lineptr, short **wordsp, int *nwordsp, int *nwordposp,
	       int uselex)
{
    int nwords = *nwordsp, nwordpos = 0;
    short *words = *wordsp;
    char *start = lineptr;

    if (uselex) {
	LinkList wordlist;
	LinkNode wordnode;
	int nwords_max;

	wordlist = bufferwords(NULL, lineptr, NULL,
			       LEXFLAGS_COMMENTS_KEEP);
	if (errflag)
	    return;
	nwords_max = 2 * countlinknodes(wordlist);
	if (nwords_max > nwords) {
	    *nwordsp = nwords = nwords_max;
	    *wordsp = words = (short *)zrealloc(words, nwords*sizeof(short));
	}
	for (wordnode = firstnode(wordlist);
	     wordnode;
	     incnode(wordnode)) {
	    char *word = getdata(wordnode);
	    char *lptr, *wptr = word;
	    int loop_next = 0, skipping;

	    /* Skip stuff at the start of the word */
	    for (;;) {
		/*
		 * Not really an oddity: "\\\n" is
		 * removed from input as if whitespace.
		 */
		if (inblank(*lineptr))
		    lineptr++;
		else if (lineptr[0] == '\\' && lineptr[1] == '\n') {
		    /*
		     * Optimisation: we handle this in the loop below,
		     * too.
		     */
		    lineptr += 2;
		} else
		    break;
	    }
	    lptr = lineptr;
	    /*
	     * Skip chunks of word with possible intervening
	     * backslash-newline.
	     *
	     * To get round C's annoying lack of ability to
	     * reference the outer loop, we'll break from this
	     * one with
	     * loop_next = 0: carry on as normal
	     * loop_next = 1: break from outer loop
	     * loop_next = 2: continue round outer loop.
	     */
	    do {
		skipping = 0;
		if (strpfx(wptr, lptr)) {
		    /*
		     * Normal case: word from lexer matches start of
		     * string from line.  Just advance over it.
		     */
		    int len;
		    if (!strcmp(wptr, ";") && strpfx(";;", lptr)) {
			/*
			 * Don't get confused between a semicolon that's
			 * probably really a newline and a double
			 * semicolon that's terminating a case.
			 */
			loop_next = 2;
			break;
		    }
		    len = strlen(wptr);
		    lptr += len;
		    wptr += len;
		} else {
		    /*
		     * Didn't get to the end of the word.
		     * See what's amiss.
		     */
		    int bad = 0;
		    /*
		     * Oddity 1: newlines turn into semicolons.
		     */
		    if (!strcmp(wptr, ";"))
		    {
			loop_next = 2;
			break;
		    }
		    while (*lptr) {
			if (!*wptr) {
			    /*
			     * End of the word before the end of the
			     * line: not good.
			     */
			    bad = 1;
			    loop_next = 1;
			    break;
			}
			/*
			 * Oddity 2: !'s turn into |'s.
			 */
			if (*lptr == *wptr ||
			    (*lptr == '!' && *wptr == '|')) {
			    lptr++;
			    if (!*++wptr)
				break;
			} else if (lptr[0] == '\\' &&
				   lptr[1] == '\n') {
			    /*
			     * \\\n can occur in the middle of a word;
			     * wptr is already pointing at this, we
			     * just need to skip over the break
			     * in lptr and look at the next chunk.
			     */
			    lptr += 2;
			    skipping = 1;
			    break;
			} else {
			    bad = 1;
			    loop_next = 1;
			    break;
			}
		    }
		    if (bad) {
#ifdef DEBUG
			dputs(ERRMSG("bad wordsplit reading history: "
				     "%s\nat: %s\nword: %s"),
			      start, lineptr, word);
#endif
			lineptr = start;
			nwordpos = 0;
			uselex = 0;
			loop_next = 1;
		    }
		}
	    } while (skipping);
	    if (loop_next) {
		if (loop_next == 1)
		    break;
		continue;
	    }
	    /* Record position of current word... */
	    words[nwordpos++] = lineptr - start;
	    words[nwordpos++] = lptr - start;

	    /* ready for start of next word. */
	    lineptr = lptr;
	}
    }
    if (!uselex) {
	do {
	    for (;;) {
		if (inblank(*lineptr))
		    lineptr++;
		else if (lineptr[0] == '\\' && lineptr[1] == '\n')
		    lineptr += 2;
		else
		    break;
	    }
	    if (*lineptr) {
		if (nwordpos >= nwords) {
		    *nwordsp = nwords = nwords + 64;
		    *wordsp = words = (short *)
			zrealloc(words, nwords*sizeof(*words));
		}
		words[nwordpos++] = lineptr - start;
		while (*lineptr && !inblank(*lineptr))
		    lineptr++;
		words[nwordpos++] = lineptr - start;
	    }
	} while (*lineptr);
    }

    *nwordposp = nwordpos;
}

/* Move the current history list out of the way and prepare a fresh history
 * list using hf for HISTFILE, hs for HISTSIZE, and shs for SAVEHIST.  If
 * the hf value is an empty string, HISTFILE will be unset from the new
 * environment; if it is NULL, HISTFILE will not be changed, not even by the
 * pop function (this functionality is used internally to rewrite the current
 * history file without affecting pointers into the environment).
 */

/**/
int
pushhiststack(char *hf, zlong hs, zlong shs, int level)
{
    struct histsave *h;
    int curline_in_ring = (histactive & HA_ACTIVE) && hist_ring == &curline;

    if (histsave_stack_pos == histsave_stack_size) {
	histsave_stack_size += 5;
	histsave_stack = zrealloc(histsave_stack,
			    histsave_stack_size * sizeof (struct histsave));
    }

    if (curline_in_ring)
	unlinkcurline();

    h = &histsave_stack[histsave_stack_pos++];

    h->lasthist = lasthist;
    if (hf) {
	if ((h->histfile = getsparam("HISTFILE")) != NULL && *h->histfile)
	    h->histfile = ztrdup(h->histfile);
	else
	    h->histfile = "";
    } else
	h->histfile = NULL;
    h->histtab = histtab;
    h->hist_ring = hist_ring;
    h->curhist = curhist;
    h->histlinect = histlinect;
    h->histsiz = histsiz;
    h->savehistsiz = savehistsiz;
    h->locallevel = level;

    memset(&lasthist, 0, sizeof lasthist);
    if (hf) {
	if (*hf)
	    setsparam("HISTFILE", ztrdup(hf));
	else
	    unsetparam("HISTFILE");
    }
    hist_ring = NULL;
    curhist = histlinect = 0;
    if (zleactive)
	zleentry(ZLE_CMD_SET_HIST_LINE, curhist);
    histsiz = hs;
    savehistsiz = shs;
    inithist(); /* sets histtab */

    if (curline_in_ring)
	linkcurline();

    return histsave_stack_pos;
}


/**/
int
pophiststack(void)
{
    struct histsave *h;
    int curline_in_ring = (histactive & HA_ACTIVE) && hist_ring == &curline;

    if (histsave_stack_pos == 0)
	return 0;

    if (curline_in_ring)
	unlinkcurline();

    deletehashtable(histtab);
    zsfree(lasthist.text);

    h = &histsave_stack[--histsave_stack_pos];

    lasthist = h->lasthist;
    if (h->histfile) {
	if (*h->histfile)
	    setsparam("HISTFILE", h->histfile);
	else
	    unsetparam("HISTFILE");
    }
    histtab = h->histtab;
    hist_ring = h->hist_ring;
    curhist = h->curhist;
    if (zleactive)
	zleentry(ZLE_CMD_SET_HIST_LINE, curhist);
    histlinect = h->histlinect;
    histsiz = h->histsiz;
    savehistsiz = h->savehistsiz;

    if (curline_in_ring)
	linkcurline();

    return histsave_stack_pos + 1;
}

/* If pop_through > 0, pop all array items >= the 1-relative index value.
 * If pop_through <= 0, pop (-1)*pop_through levels off the stack.
 * If the (new) top of stack is from a higher locallevel, auto-pop until
 * it is not.
 */

/**/
int
saveandpophiststack(int pop_through, int writeflags)
{
    if (pop_through <= 0) {
	pop_through += histsave_stack_pos + 1;
	if (pop_through <= 0)
	    pop_through = 1;
    }
    while (pop_through > 1
     && histsave_stack[pop_through-2].locallevel > locallevel)
	pop_through--;
    if (histsave_stack_pos < pop_through)
	return 0;
    do {
	if (!nohistsave)
	    savehistfile(NULL, 1, writeflags);
	pophiststack();
    } while (histsave_stack_pos >= pop_through);
    return 1;
}
