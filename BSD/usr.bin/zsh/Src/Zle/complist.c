/*
 * complist.c - completion listing enhancements
 *
 * This file is part of zsh, the Z shell.
 *
 * Copyright (c) 1999 Sven Wischnowsky
 * All rights reserved.
 *
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and to distribute modified versions of this software for any
 * purpose, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 *
 * In no event shall Sven Wischnowsky or the Zsh Development Group be liable
 * to any party for direct, indirect, special, incidental, or consequential
 * damages arising out of the use of this software and its documentation,
 * even if Sven Wischnowsky and the Zsh Development Group have been advised of
 * the possibility of such damage.
 *
 * Sven Wischnowsky and the Zsh Development Group specifically disclaim any
 * warranties, including, but not limited to, the implied warranties of
 * merchantability and fitness for a particular purpose.  The software
 * provided hereunder is on an "as is" basis, and Sven Wischnowsky and the
 * Zsh Development Group have no obligation to provide maintenance,
 * support, updates, enhancements, or modifications.
 *
 */

#include "complist.mdh"
#include "complist.pro"


/* Information about the list shown. */

/*
 * noselect: 1 if complistmatches indicated we shouldn't do selection;
 *           -1 if interactive mode needs to reset the selection list.
 *           Tested in domenuselect, and in complistmatches to skip redraw.
 * mselect:  Local copy of the index of the currently selected match.
 *           Initialised to the gnum entry of the current match for
 *           each completion.
 * inselect: 1 if we already selecting matches; tested in complistmatches()
 * mcol:     The column for the selected completion.  As we never scroll
 *           horizontally this applies both the screen and the logical array.
 * mline:    The line for the selected completion in the logical array of
 *           all matches, not all of which may be on screen at once.
 * mcols:    Local copy of columns used in sizing arrays. 
 * mlines:   The number of lines in the logical array of all matches,
 *           initialised from listdat.nlines.
 */
static int noselect, mselect, inselect, mcol, mline, mcols, mlines;
/*
 * selected: Used to signal between domenucomplete() and menuselect()
 *           that a selected entry has been found.  Or something.
 * mlbeg:    The first line of the logical array of all matches that
 *           fits on screen.  Setting this to -1 forces a redraw.
 * mlend:    The line after the last that fits on screen.
 * mscroll:  1 if the scrolling prompt is shown on screen.
 * mrestlines: The number of screen lines remaining to be processed.
 */
static int selected, mlbeg = -1, mlend = 9999999, mscroll, mrestlines;
/*
 * mnew: 1 if a new selection menu is being displayed.
 * mlastcols: stored value of mcols for use in calculating mnew.
 * mlastlines: stored value of mlines for use in calculating mnew.
 * mhasstat: Indicates if the status line is present (but maybe not
 *           yet printed).
 * mfirstl: The first line of the logical array of all matches to
 *          be shown on screen, -1 if this has not yet been determined.
 * mlastm: The index of the selected match in some circumstances; used
 *         if an explicit number for a match is passed to compprintfmt();
 *         initialised from the total number of matches.  I realise this
 *         isn't very illuminating.
 */
static int mnew, mlastcols, mlastlines, mhasstat, mfirstl, mlastm;
/*
 * mlprinted: Used to signal the number of additional lines printed
 *            when outputting matches (as argument passing is a bit
 *            screwy within the completion system).
 * molbeg:    The last value of mlbeg; -1 if invalid, -42 if, er, very
 *            invalid.  Used in calculations of how much to draw.
 * mocol:     The last value of mcol.
 * moline:    The last value of mline.
 * mstatprinted: Indicates that the status line has now been printed,
 *               c.f. mhasstat.
 */
static int mlprinted, molbeg = -2, mocol = 0, moline = 0, mstatprinted;
/*
 * mstatus: The message printed when scrolling.
 * mlistp: The message printed when merely listing.
 */
static char *mstatus, *mlistp;
/*
 * mtab is the logical array of all matches referred to above.  It
 * contains mcols*mlines entries.  These entries contain a pointer to
 * the match structure which is in use at a particular point.  Note
 * that for multiple line entries lines after the first contain NULL.
 *
 * mmtabp is a pointer to the selected entry in mtab.
 */
static Cmatch **mtab, **mmtabp;
/*
 * Used to indicate that the list has changed and needs redisplaying.
 */
static int mtab_been_reallocated;
/*
 * Array and pointer for the match group in exactly the same layout
 * as mtab and mmtabp.
 */
static Cmgroup *mgtab, *mgtabp;
#ifdef DEBUG
/*
 * Allow us to keep track of pointer arithmetic for mgtab; could
 * just as well have been for mtab but wasn't.
 */
static int mgtabsize;
#endif

/*
 * Used in mtab/mgtab, for explanations.
 *
 * UUUUUUUUUUUUUUURRRRGHHHHHHHHHH!!!!!!!!! --- pws
 */

#define MMARK       ((unsigned long) 1)
#define mmarked(v)  (((unsigned long) (v)) & MMARK)
#define mtmark(v)   ((Cmatch *) (((unsigned long) (v)) | MMARK))
#define mtunmark(v) ((Cmatch *) (((unsigned long) (v)) & ~MMARK))
#define mgmark(v)   ((Cmgroup)  (((unsigned long) (v)) | MMARK))
#define mgunmark(v) ((Cmgroup)  (((unsigned long) (v)) & ~MMARK))

/* Information for in-string colours. */

/* Maximum number of in-string colours supported. */

#define MAX_POS 11

static int nrefs;
static int begpos[MAX_POS], curisbeg;
static int endpos[MAX_POS];
static int sendpos[MAX_POS], curissend; /* sorted end positions */
static char **patcols, *curiscols[MAX_POS];
static int curiscol;

/* The last color used. */

static char *last_cap;


/* We use the parameters ZLS_COLORS and ZLS_COLOURS in the same way as
 * the color ls does. It's just that we don't support the `or' file
 * type. */


/*
 * menu-select widget: used to test if it's already loaded.
 */
static Widget w_menuselect;
/*
 * Similarly for the menuselect and listscroll keymaps.
 */
static Keymap mskeymap, lskeymap;

/* Indixes into the terminal string arrays. */

#define COL_NO  0
#define COL_FI  1
#define COL_DI  2
#define COL_LN  3
#define COL_PI  4
#define COL_SO  5
#define COL_BD  6
#define COL_CD  7
#define COL_OR  8
#define COL_MI  9
#define COL_SU 10
#define COL_SG 11
#define COL_TW 12
#define COL_OW 13
#define COL_ST 14
#define COL_EX 15
#define COL_LC 16
#define COL_RC 17
#define COL_EC 18
#define COL_TC 19
#define COL_SP 20
#define COL_MA 21
#define COL_HI 22
#define COL_DU 23
#define COL_SA 24

#define NUM_COLS 25

/* Names of the terminal strings. */

static char *colnames[] = {
    "no", "fi", "di", "ln", "pi", "so", "bd", "cd", "or", "mi",
    "su", "sg", "tw", "ow", "st", "ex",
    "lc", "rc", "ec", "tc", "sp", "ma", "hi", "du", "sa", NULL
};

/* Default values. */

static char *defcols[] = {
    "0", "0", "1;31", "1;36", "33", "1;35", "1;33", "1;33", NULL, NULL,
    "37;41", "30;43", "30;42", "34;42", "37;44", "1;32", 
    "\033[", "m", NULL, "0", "0", "7", NULL, NULL, "0"
};

/* This describes a terminal string for a file type. */

typedef struct filecol *Filecol;

struct filecol {
    Patprog prog;		/* group pattern */
    char *col;			/* color string */
    Filecol next;		/* next one */
};

/* This describes a terminal string for a pattern. */

typedef struct patcol *Patcol;

struct patcol {
    Patprog prog;
    Patprog pat;		/* pattern for match */
    char *cols[MAX_POS + 1];
    Patcol next;
};

/* This describes a terminal string for a filename extension. */

typedef struct extcol *Extcol;

struct extcol {
    Patprog prog;		/* group pattern or NULL */
    char *ext;			/* the extension */
    char *col;			/* the terminal color string */
    Extcol next;		/* the next one in the list */
};

/* This holds all terminal strings. */

typedef struct listcols *Listcols;

/* values for listcol flags */
enum {
    /* ln=target:  follow symlinks to determine highlighting */
    LC_FOLLOW_SYMLINKS = 0x0001
};

struct listcols {
    Filecol files[NUM_COLS];	/* strings for file types */
    Patcol pats;		/* strings for patterns */
    Extcol exts;		/* strings for extensions */
    int flags;			/* special settings, see above */
};

/*
 * Contains information about the colours to be used for entries.
 * Sometimes mcolors is passed as an argument even though it's
 * available to all the functions.
 */
static struct listcols mcolors;

/* Combined length of LC and RC, maximum length of capability strings. */

static int lr_caplen, max_caplen;

/* This parses the value of a definition (the part after the `=').
 * The return value is a pointer to the character after it. */

static char *
getcolval(char *s, int multi)
{
    char *p, *o = s;

    for (p = s; *s && *s != ':' && (!multi || *s != '='); p++, s++) {
	if (*s == '\\' && s[1]) {
	    switch (*++s) {
	    case 'a': *p = '\007'; break;
	    case 'n': *p = '\n'; break;
	    case 'b': *p = '\b'; break;
	    case 't': *p = '\t'; break;
	    case 'v': *p = '\v'; break;
	    case 'f': *p = '\f'; break;
	    case 'r': *p = '\r'; break;
	    case 'e': *p = '\033'; break;
	    case '_': *p = ' '; break;
	    case '?': *p = '\177'; break;
	    default:
		if (*s >= '0' && *s <= '7') {
		    int i = STOUC(*s);

		    if (*++s >= '0' && *s <= '7') {
			i = (i * 8) + STOUC(*s);
			if (*++s >= '0' && *s <= '7')
			    i = (i * 8) + STOUC(*s);
		    }
		    *p = (char) i;
		} else
		    *p = *s;
	    }
	} else if (*s == '^') {
	    if ((s[1] >= '@' && s[1] <= '_') ||
		(s[1] >= 'a' && s[1] <= 'z'))
		*p = (char) (STOUC(*s) & ~0x60);
	    else if (s[1] == '?')
		*p = '\177';
	    else {
		*p++ = *s;
		*p = s[1];
	    }
	    s++;
	} else
	    *p = *s;
    }
    if (p != s)
	*p = '\0';
    if ((s - o) > max_caplen)
	max_caplen = s - o;
    return s;
}

/* This parses one definition. Return value is a pointer to the
 * character after it. */

static char *
getcoldef(char *s)
{
    Patprog gprog = NULL;

    if (*s == '(') {
	char *p;
	int l = 0;

	for (p = s + 1, l = 0; *p && (*p != ')' || l); p++)
	    if (*p == '\\' && p[1])
		p++;
	    else if (*p == '(')
		l++;
	    else if (*p == ')')
		l--;

	if (*p == ')') {
	    char sav = p[1];

	    p[1] = '\0';
	    s = metafy(s, -1, META_USEHEAP);
	    tokenize(s);
	    gprog = patcompile(s, 0, NULL);
	    p[1] = sav;

	    s = p + 1;
	}
    }
    if (*s == '*') {
	Extcol ec, eo;
	char *n, *p;

	/* This is for an extension. */

	n = ++s;
	while (*s && *s != '=')
	    s++;
	if (!*s)
	    return s;
	*s++ = '\0';
	p = getcolval(s, 0);
	ec = (Extcol) zhalloc(sizeof(*ec));
	ec->prog = gprog;
	ec->ext = n;
	ec->col = s;
	ec->next = NULL;
	if ((eo = mcolors.exts)) {
	    while (eo->next)
		eo = eo->next;
	    eo->next = ec;
	} else
	    mcolors.exts = ec;
	if (*p)
	    *p++ = '\0';
	return p;
    } else if (*s == '=') {
	char *p = ++s, *t, *cols[MAX_POS];
	int ncols = 0;
	int nesting = 0;
	Patprog prog;

	/* This is for a pattern. */

	while (*s && (nesting || *s != '=')) {
	    switch (*s++) {
		case '\\':
		    if (*s)
			s++;
		    break;
		case '(':
		    nesting++;
		    break;
		case ')':
		    nesting--;
		    break;
	    }
	}
	if (!*s)
	    return s;
	*s++ = '\0';
	while (1) {
	    t = getcolval(s, 1);
	    if (ncols < MAX_POS)
		cols[ncols++] = s;
	    s = t;
	    if (*s != '=')
		break;
	    *s++ = '\0';
	}
	p = metafy(p, -1, META_USEHEAP);
	tokenize(p);
	if ((prog = patcompile(p, 0, NULL))) {
	    Patcol pc, po;
	    int i;

	    pc = (Patcol) zhalloc(sizeof(*pc));
	    pc->prog = gprog;
	    pc->pat = prog;
	    for (i = 0; i < ncols; i++)
		pc->cols[i] = cols[i];
	    pc->cols[i] = NULL;
	    pc->next = NULL;
	    if ((po = mcolors.pats)) {
		while (po->next)
		    po = po->next;
		po->next = pc;
	    } else
		mcolors.pats = pc;
	}
	if (*t)
	    *t++ = '\0';
	return t;
    } else {
	char *n = s, *p, **nn;
	int i;

	/* This is for a file type. */

	while (*s && *s != '=')
	    s++;
	if (!*s)
	    return s;
	*s++ = '\0';
	for (i = 0, nn = colnames; *nn; i++, nn++)
	    if (!strcmp(n, *nn))
		break;
	/*
	 * special case:  highlighting link targets
	 */
	if (i == COL_LN && strpfx("target", s) &&
	    (s[6] == ':' || !s[6])) {
	    mcolors.flags |= LC_FOLLOW_SYMLINKS;
	    p = s + 6;
	} else {
	    p = getcolval(s, 0);
	    if (*nn) {
		Filecol fc, fo;

		fc = (Filecol) zhalloc(sizeof(*fc));
		fc->prog = (i == COL_EC || i == COL_LC || i == COL_RC ?
			    NULL : gprog);
		fc->col = s;
		fc->next = NULL;
		if ((fo = mcolors.files[i])) {
		    while (fo->next)
			fo = fo->next;
		    fo->next = fc;
		} else
		    mcolors.files[i] = fc;
	    }
	    if (*p)
		*p++ = '\0';
	}
	return p;
    }
}

static Filecol
filecol(char *col)
{
    Filecol fc;

    fc = (Filecol) zhalloc(sizeof(*fc));
    fc->prog = NULL;
    fc->col = col;
    fc->next = NULL;

    return fc;
}

/*
 * This initializes the given terminal color structure.
 */

static void
getcols(void)
{
    char *s;
    int i, l;

    max_caplen = lr_caplen = 0;
    mcolors.flags = 0;
    queue_signals();
    if (!(s = getsparam_u("ZLS_COLORS")) &&
	!(s = getsparam_u("ZLS_COLOURS"))) {
	for (i = 0; i < NUM_COLS; i++)
	    mcolors.files[i] = filecol("");
	mcolors.pats = NULL;
	mcolors.exts = NULL;
	
	if ((s = tcstr[TCSTANDOUTBEG]) && s[0]) {
	    mcolors.files[COL_MA] = filecol(s);
	    mcolors.files[COL_EC] = filecol(tcstr[TCSTANDOUTEND]);
	} else
	    mcolors.files[COL_MA] = filecol(defcols[COL_MA]);
	lr_caplen = 0;
	if ((max_caplen = strlen(mcolors.files[COL_MA]->col)) <
	    (l = strlen(mcolors.files[COL_EC]->col)))
	    max_caplen = l;
	unqueue_signals();
	return;
    }
    /* Reset the global color structure. */
    memset(&mcolors, 0, sizeof(mcolors));
    s = dupstring(s);
    while (*s)
	if (*s == ':')
	    s++;
	else
	    s = getcoldef(s);
    unqueue_signals();

    /* Use default values for those that aren't set explicitly. */
    for (i = 0; i < NUM_COLS; i++) {
	if (!mcolors.files[i] || !mcolors.files[i]->col)
	    mcolors.files[i] = filecol(defcols[i]);
	if (mcolors.files[i] && mcolors.files[i]->col &&
	    (l = strlen(mcolors.files[i]->col)) > max_caplen)
	    max_caplen = l;
    }
    lr_caplen = strlen(mcolors.files[COL_LC]->col) +
	strlen(mcolors.files[COL_RC]->col);

    /* Default for orphan is same as link. */
    if (!mcolors.files[COL_OR] || !mcolors.files[COL_OR]->col)
	mcolors.files[COL_OR] = mcolors.files[COL_LN];
    /* Default for missing files:  currently not used */
    if (!mcolors.files[COL_MI] || !mcolors.files[COL_MI]->col)
	mcolors.files[COL_MI] = mcolors.files[COL_FI];

    return;
}

static void
zlrputs(char *cap)
{
    if (!*last_cap || strcmp(last_cap, cap)) {
	VARARR(char, buf, lr_caplen + max_caplen + 1);

	strcpy(buf, mcolors.files[COL_LC]->col);
	strcat(buf, cap);
	strcat(buf, mcolors.files[COL_RC]->col);

	tputs(buf, 1, putshout);

	strcpy(last_cap, cap);
    }
}

static void
zcputs(char *group, int colour)
{
    Filecol fc;

    for (fc = mcolors.files[colour]; fc; fc = fc->next)
	if (fc->col &&
	    (!fc->prog || !group || pattry(fc->prog, group))) {
	    zlrputs(fc->col);

	    return;
	}
    zlrputs("0");
}

/* Turn off colouring. */

static void
zcoff(void)
{
    if (mcolors.files[COL_EC] && mcolors.files[COL_EC]->col) {
	tputs(mcolors.files[COL_EC]->col, 1, putshout);
	*last_cap = '\0';
    } else
	zcputs(NULL, COL_NO);
}

/* Clear to end of line, if possible and necessary. */
static void
cleareol()
{
    if (mlbeg >= 0 && tccan(TCCLEAREOL)) {
        if (*last_cap)
            zcoff();    /* If we used colors, prevent them from bleeding. */
        tcout(TCCLEAREOL);
    }
}

static void
initiscol(void)
{
    int i;

    zlrputs(patcols[0]);

    curiscols[curiscol = 0] = *patcols++;

    curisbeg = curissend = 0;

    for (i = 0; i < nrefs; i++)
	sendpos[i] = 0xfffffff;
    for (; i < MAX_POS; i++)
	begpos[i] = endpos[i] = sendpos[i] = 0xfffffff;
}

static void
doiscol(int pos)
{
    int fi;

    while (pos > sendpos[curissend]) {
	curissend++;
	if (curiscol) {
	    zcputs(NULL, COL_NO);
	    zlrputs(curiscols[--curiscol]);
	}
    }
    while (((fi = (endpos[curisbeg] < begpos[curisbeg] || 
		  begpos[curisbeg] == -1)) ||
	    pos == begpos[curisbeg]) && *patcols) {
	if (!fi) {
	    int i, j, e = endpos[curisbeg];
	    
	    /* insert e in sendpos */
	    for (i = curissend; sendpos[i] <= e; ++i)
		;
	    for (j = MAX_POS - 1; j > i; --j)
		sendpos[j] = sendpos[j-1];
	    sendpos[i] = e;
	    
	    zcputs(NULL, COL_NO);
	    zlrputs(*patcols);
	    curiscols[++curiscol] = *patcols;
	}
	++patcols;
	++curisbeg;
    }
}

/* Stripped-down version of printfmt(). But can do in-string colouring. */

static int
clprintfmt(char *p, int ml)
{
    int cc = 0, i = 0, ask, beg;

    initiscol();

    while (*p) {
	convchar_t chr;
	int chrlen = MB_METACHARLENCONV(p, &chr);
	doiscol(i++);
	cc++;
	if (*p == '\n') {
	    cleareol();
	    cc = 0;
	}
	if (ml == mlend - 1 && (cc % zterm_columns) == zterm_columns - 1)
	    return 0;

	while (chrlen) {
	    if (*p == Meta) {
		p++;
		chrlen--;
		putc(*p ^ 32, shout);
	    } else
		putc(*p, shout);
	    chrlen--;
	    p++;
	}
	if ((beg = !(cc % zterm_columns)))
	    ml++;
	if (mscroll && !(cc % zterm_columns) &&
	    !--mrestlines && (ask = asklistscroll(ml)))
	    return ask;
    }
    cleareol();
    return 0;
}

/*
 * Local version of nicezputs() with in-string colouring
 * and scrolling.
 */

static int
clnicezputs(int do_colors, char *s, int ml)
{
    int i = 0, col = 0, ask, oml = ml;
    char *t;
    ZLE_CHAR_T cc;
#ifdef MULTIBYTE_SUPPORT
    /*
     * ums is the untokenized, unmetafied string (length umlen)
     * uptr is a pointer into it
     * sptr is the start of the nice character representation
     * wptr is the point at which the wide character itself starts
     *  (but may be the end of the string if the character was fully
     *  prettified).
     * ret is the return status from the conversion to a wide character
     * umleft is the remaining length of the unmetafied string to output
     * umlen is the full length of the unmetafied string
     * width is the full printing width of a prettified character,
     *  including both ASCII prettification and the wide character itself.
     * mbs is the shift state of the conversion to wide characters.
     */
    char *ums, *uptr, *sptr, *wptr;
    int umleft, umlen, eol = 0;
    size_t width;
    mbstate_t mbs;

    memset(&mbs, 0, sizeof mbs);
    ums = ztrdup(s);
    untokenize(ums);
    uptr = unmetafy(ums, &umlen);
    umleft = umlen;

    if (do_colors)
	initiscol();

    mb_charinit();
    while (umleft > 0) {
	size_t cnt = eol ? MB_INVALID : mbrtowc(&cc, uptr, umleft, &mbs);

	switch (cnt) {
	case MB_INCOMPLETE:
	    eol = 1;
	    /* FALL THROUGH */
	case MB_INVALID:
	    /* This handles byte values that aren't valid wide-character
	     * sequences. */
	    sptr = nicechar(*uptr);
	    /* everything here is ASCII... */
	    width = strlen(sptr);
	    wptr = sptr + width;
	    cnt = 1;
	    /* Get mbs out of its undefined state. */
	    memset(&mbs, 0, sizeof mbs);
	    break;
	case 0:
	    /* This handles a '\0' in the input (which is a real char
	     * to us, not a terminator). */
	    cnt = 1;
	    /* FALL THROUGH */
	default:
	    sptr = wcs_nicechar(cc, &width, &wptr);
	    break;
	}

	umleft -= cnt;
	uptr += cnt;
	if (do_colors) {
	    /*
	     * The code for the colo[u]ri[s/z]ation is obscure (surprised?)
	     * but if we do it for every input character, as we do in
	     * the simple case, we shouldn't go too far wrong.
	     */
	    while (cnt--)
		doiscol(i++);
	}

	/*
	 * Loop over characters in the output of the nice
	 * representation.  This will often correspond to one input
	 * (possibly multibyte) character.
	 */
	for (t = sptr; *t; t++) {
	    /* Input is metafied... */
	    int nc = (*t == Meta) ? STOUC(*++t ^ 32) : STOUC(*t);
	    /* Is the screen full? */
	    if (ml == mlend - 1 && col == zterm_columns - 1) {
		mlprinted = ml - oml;
		free(ums);
		return 0;
	    }
	    if (t < wptr) {
		/* outputting ASCII, so single-width */
		putc(nc, shout);
		col++;
		width--;
	    } else {
		/* outputting a single wide character, do the lot */
		putc(nc, shout);
		/* don't check column until finished */
		if (t[1])
		    continue;
		/* now we've done the entire rest of the representation */
		col += width;
	    }
	    /*
	     * There might be problems with characters of printing width
	     * greater than one here.
	     */
	    if (col > zterm_columns) {
		ml++;
		if (mscroll && !--mrestlines && (ask = asklistscroll(ml))) {
		    mlprinted = ml - oml;
		    free(ums);
		    return ask;
		}
		col -= zterm_columns;
		if (do_colors)
		    fputs(" \010", shout);
	    }
	}
    }

    free(ums);
#else

    if (do_colors)
	initiscol();

    while ((cc = *s++)) {
	if (do_colors)
	    doiscol(i++);
	if (itok(cc)) {
	    if (cc <= Comma)
		cc = ztokens[cc - Pound];
	    else
		continue;
	}
	if (cc == Meta)
	    cc = *s++ ^ 32;

	for (t = nicechar(cc); *t; t++) {
	    int nc = (*t == Meta) ? STOUC(*++t ^ 32) : STOUC(*t);
	    if (ml == mlend - 1 && col == zterm_columns - 1) {
		mlprinted = ml - oml;
		return 0;
	    }
	    putc(nc, shout);
	    if (++col > zterm_columns) {
		ml++;
		if (mscroll && !--mrestlines && (ask = asklistscroll(ml))) {
		    mlprinted = ml - oml;
		    return ask;
		}
		col = 0;
		if (do_colors)
		    fputs(" \010", shout);
	    }
	}
    }
#endif
    mlprinted = ml - oml;
    return 0;
}

/* Get the terminal color string for the given match. */

static int
putmatchcol(char *group, char *n)
{
    Patcol pc;

    for (pc = mcolors.pats; pc; pc = pc->next) {
	nrefs = MAX_POS - 1;

	if ((!pc->prog || !group || pattry(pc->prog, group)) &&
	    pattryrefs(pc->pat, n, -1, -1, NULL, 0, &nrefs, begpos, endpos)) {
	    if (pc->cols[1]) {
		patcols = pc->cols;

		return 1;
	    }
	    zlrputs(pc->cols[0]);

	    return 0;
	}
    }

    zcputs(group, COL_NO);

    return 0;
}

/* Get the terminal color string for the file with the given name and
 * file modes. */

static int
putfilecol(char *group, char *filename, mode_t m, int special)
{
    int colour = -1;
    Extcol ec;
    Patcol pc;
    int len;

    for (pc = mcolors.pats; pc; pc = pc->next) {
	nrefs = MAX_POS - 1;

	if ((!pc->prog || !group || pattry(pc->prog, group)) &&
	    pattryrefs(pc->pat, filename, -1, -1, NULL,
		       0, &nrefs, begpos, endpos)) {
	    if (pc->cols[1]) {
		patcols = pc->cols;

		return 1;
	    }
	    zlrputs(pc->cols[0]);

	    return 0;
	}
    }

    if (special != -1) {
	colour = special;
    } else if (S_ISDIR(m)) {
	if (m & S_IWOTH)
	    if (m & S_ISVTX)
		colour = COL_TW;
	    else
		colour = COL_OW;
	else if (m & S_ISVTX)
	    colour = COL_ST;
	else
	    colour = COL_DI;
    } else if (S_ISLNK(m))
	colour = COL_LN;
    else if (S_ISFIFO(m))
	colour = COL_PI;
    else if (S_ISSOCK(m))
	colour = COL_SO;
    else if (S_ISBLK(m))
	colour = COL_BD;
    else if (S_ISCHR(m))
	colour = COL_CD;
    else if (m & S_ISUID)
	colour = COL_SU;
    else if (m & S_ISGID)
	colour = COL_SG;
    else if (S_ISREG(m) && (m & S_IXUGO))
	colour = COL_EX;

    if (colour != -1) {
	zcputs(group, colour);
	return 0;
    }

    for (ec = mcolors.exts; ec; ec = ec->next)
	if (strsfx(ec->ext, filename) &&
	    (!ec->prog || !group || pattry(ec->prog, group))) {
	    zlrputs(ec->col);

	    return 0;
	}

    /* Check for suffix alias */
    len = strlen(filename);
    /* shortest valid suffix format is a.b */
    if (len > 2) {
	char *suf = filename + len - 1;
	while (suf > filename+1) {
	    if (suf[-1] == '.') {
		if (sufaliastab->getnode(sufaliastab, suf)) {
		    zcputs(group, COL_SA);
		    return 0;
		}
		break;
	    }
	    suf--;
	}
    }
    zcputs(group, COL_FI);

    return 0;
}

static Cmgroup last_group;

/**/
static int
asklistscroll(int ml)
{
    Thingy cmd;
    int i, ret = 0;

    compprintfmt(NULL, 1, 1, 1, ml, NULL);

    fflush(shout);
    zsetterm();
    menuselect_bindings();	/* sanity in case deleted by user */
    selectlocalmap(lskeymap);
    if (!(cmd = getkeycmd()) || cmd == Th(z_sendbreak))
	ret = 1;
    else if (cmd == Th(z_acceptline) ||
	     cmd == Th(z_downhistory) ||
	     cmd == Th(z_downlineorhistory) ||
	     cmd == Th(z_downlineorsearch) ||
	     cmd == Th(z_vidownlineorhistory))
	mrestlines = 1;
    else if (cmd == Th(z_completeword) ||
		   cmd == Th(z_expandorcomplete) ||
		   cmd == Th(z_expandorcompleteprefix) ||
		   cmd == Th(z_menucomplete) ||
		   cmd == Th(z_menuexpandorcomplete) ||
		   !strcmp(cmd->nam, "menu-select") ||
		   !strcmp(cmd->nam, "complete-word") ||
		   !strcmp(cmd->nam, "expand-or-complete") ||
		   !strcmp(cmd->nam, "expand-or-complete-prefix") ||
		   !strcmp(cmd->nam, "menu-complete") ||
	     !strcmp(cmd->nam, "menu-expand-or-complete"))
	mrestlines = zterm_lines - 1;
    else if (cmd == Th(z_acceptsearch))
	ret = 1;
    else {
	ungetkeycmd();
	ret = 1;
    }
    selectlocalmap(NULL);
    settyinfo(&shttyinfo);
    putc('\r', shout);
    for (i = zterm_columns - 1; i-- > 0; )
	putc(' ', shout);
    putc('\r', shout);

    return ret;
}

#define dolist(X)   ((X) >= mlbeg && (X) < mlend)
#define dolistcl(X) ((X) >= mlbeg && (X) < mlend + 1)
#define dolistnl(X) ((X) >= mlbeg && (X) < mlend - 1)

/**/
static int
compprintnl(int ml)
{
    int ask;

    cleareol();
    putc('\n', shout);

    if (mscroll && !--mrestlines && (ask = asklistscroll(ml)))
	return ask;

    return 0;
}

/* This is used to print the strings (e.g. explanations). *
 * It returns the number of lines printed.       */

/**/
static int
compprintfmt(char *fmt, int n, int dopr, int doesc, int ml, int *stop)
{
    char *p, nc[2*DIGBUFSIZE + 12], nbuf[2*DIGBUFSIZE + 12];
    int l = 0, cc = 0, b = 0, s = 0, u = 0, m, ask, beg, stat;

    if ((stat = !fmt)) {
	if (mlbeg >= 0) {
	    if (!(fmt = mstatus)) {
		mlprinted = 0;
		return 0;
	    }
	    cc = -1;
	} else
	    fmt = mlistp;
    }
    MB_METACHARINIT();
    for (p = fmt; *p; ) {
	convchar_t cchar;
	int len, width;

	len = MB_METACHARLENCONV(p, &cchar);
#ifdef MULTIBYTE_SUPPORT
	if (cchar == WEOF) {
	    cchar = (wchar_t)(*p == Meta ? p[1] ^ 32 : *p);
	    width = 1;
	}
	else
#endif
	    width = WCWIDTH_WINT(cchar);

	if (doesc && cchar == ZWC('%')) {
	    p += len;
	    if (*p) {
		int arg = 0, is_fg;
		zattr atr;

		if (idigit(*p))
		    arg = zstrtol(p, &p, 10);

		len = MB_METACHARLENCONV(p, &cchar);
#ifdef MULTIBYTE_SUPPORT
		if (cchar == WEOF)
		    cchar = (wchar_t)(*p == Meta ? p[1] ^ 32 : *p);
#endif
		p += len;

		m = 0;
		switch (cchar) {
		case ZWC('%'):
		    if (dopr == 1)
			putc('%', shout);
		    cc++;
		    break;
		case ZWC('n'):
		    if (!stat) {
			sprintf(nc, "%d", n);
			if (dopr == 1)
			    fputs(nc, shout);
			/* everything here is ASCII... */
			cc += strlen(nc);
		    }
		    break;
		case ZWC('B'):
		    b = 1;
		    if (dopr)
			tcout(TCBOLDFACEBEG);
		    break;
		case ZWC('b'):
		    b = 0; m = 1;
		    if (dopr)
			tcout(TCALLATTRSOFF);
		    break;
		case ZWC('S'):
		    s = 1;
		    if (dopr)
			tcout(TCSTANDOUTBEG);
		    break;
		case ZWC('s'):
		    s = 0; m = 1;
		    if (dopr)
			tcout(TCSTANDOUTEND);
		    break;
		case ZWC('U'):
		    u = 1;
		    if (dopr)
			tcout(TCUNDERLINEBEG);
		    break;
		case ZWC('u'):
		    u = 0; m = 1;
		    if (dopr)
			tcout(TCUNDERLINEEND);
		    break;
		case ZWC('F'):
		case ZWC('K'):
		    is_fg = (cchar == ZWC('F'));
		    /* colours must be ASCII */
		    if (*p == '{') {
			p++;
			atr = match_colour((const char **)&p, is_fg, 0);
			if (*p == '}')
			    p++;
		    } else
			atr = match_colour(NULL, is_fg, arg);
		    if (atr != TXT_ERROR && dopr)
			set_colour_attribute(atr, is_fg ? COL_SEQ_FG :
					     COL_SEQ_BG, 0);
		    break;
		case ZWC('f'):
		    if (dopr)
			set_colour_attribute(TXTNOFGCOLOUR, COL_SEQ_FG, 0);
		    break;
		case ZWC('k'):
		    if (dopr)
			set_colour_attribute(TXTNOBGCOLOUR, COL_SEQ_BG, 0);
		    break;
		case ZWC('{'):
		    if (arg)
			cc += arg;
		    for (; *p && (*p != '%' || p[1] != '}'); p++)
			if (dopr)
			    putc(*p == Meta ? *++p ^ 32 : *p, shout);
		    if (*p)
			p += 2;
		    break;
		case ZWC('m'):
		    if (stat) {
			sprintf(nc, "%d/%d", (n ? mlastm : mselect),
				listdat.nlist);
			m = 2;
		    }
		    break;
		case ZWC('M'):
		    if (stat) {
			sprintf(nbuf, "%d/%d", (n ? mlastm : mselect),
				listdat.nlist);
			sprintf(nc, "%-9s", nbuf);
			m = 2;
		    }
		    break;
		case ZWC('l'):
		    if (stat) {
			sprintf(nc, "%d/%d", ml + 1, listdat.nlines);
			m = 2;
		    }
		    break;
		case ZWC('L'):
		    if (stat) {
			sprintf(nbuf, "%d/%d", ml + 1, listdat.nlines);
			sprintf(nc, "%-9s", nbuf);
			m = 2;
		    }
		    break;
		case ZWC('p'):
		    if (stat) {
			if (ml == listdat.nlines - 1)
			    strcpy(nc, "Bottom");
			else if (n ? mfirstl : (mlbeg > 0 || ml != mfirstl))
			    sprintf(nc, "%d%%",
				    ((ml + 1) * 100) / listdat.nlines);
			else
			    strcpy(nc, "Top");
			m = 2;
		    }
		    break;
		case ZWC('P'):
		    if (stat) {
			if (ml == listdat.nlines - 1)
			    strcpy(nc, "Bottom");
			else if (n ? mfirstl : (mlbeg > 0 || ml != mfirstl))
			    sprintf(nc, "%2d%%   ",
				    ((ml + 1) * 100) / listdat.nlines);
			else
			    strcpy(nc, "Top   ");
			m = 2;
		    }
		    break;
		}
		if (m == 2 && dopr == 1) {
		    /* nc only contains ASCII text */
		    int l = strlen(nc);

		    if (l + cc > zterm_columns - 2)
			nc[l -= l + cc - (zterm_columns - 2)] = '\0';
		    fputs(nc, shout);
		    cc += l;
		} else if (dopr && m == 1) {
		    if (b)
			tcout(TCBOLDFACEBEG);
		    if (s)
			tcout(TCSTANDOUTBEG);
		    if (u)
			tcout(TCUNDERLINEBEG);
		}
	    } else
		break;
	} else {
	    cc += width;

	    if ((cc >= zterm_columns - 2 || cchar == ZWC('\n')) && stat)
		dopr = 2;
	    if (cchar == ZWC('\n')) {
		if (dopr == 1)
		    cleareol();
		l += 1 + ((cc - 1) / zterm_columns);
		cc = 0;
	    }
	    if (dopr == 1) {
		if (ml == mlend - 1 && (cc % zterm_columns) ==
		    zterm_columns - 1) {
		    dopr = 0;
		    p += len;
		    continue;
		}
		while (len--) {
		    if (*p == Meta) {
			len--;
			p++;
			putc(*p++ ^ 32, shout);
		    } else
			putc(*p++, shout);
		}
		/*
		 * TODO: the following doesn't allow for
		 * character widths greater than 1.
		 */
		if ((beg = !(cc % zterm_columns)) && !stat) {
		    ml++;
                    fputs(" \010", shout);
                }
		if (mscroll && beg && !--mrestlines && (ask = asklistscroll(ml))) {
		    *stop = 1;
		    if (stat && n)
			mfirstl = -1;
		    mlprinted = l + (cc ? ((cc-1) / zterm_columns) : 0);
		    return mlprinted;
		}
	    }
	    else
		p += len;
	}
    }
    if (dopr) {
        if (!(cc % zterm_columns))
            fputs(" \010", shout);
        cleareol();
    }
    if (stat && n)
	mfirstl = -1;

    /*
     * *Not* subtracting 1 from cc at this point appears to be
     * correct.  C.f. printfmt in zle_tricky.c.
     */
    mlprinted = l + (cc / zterm_columns);
    return mlprinted;
}

/* This is like zputs(), but allows scrolling. */

/**/
static int
compzputs(char const *s, int ml)
{
    int c, col = 0, ask;

    while (*s) {
	if (*s == Meta)
	    c = *++s ^ 32;
	else if(itok(*s)) {
	    s++;
	    continue;
	} else
	    c = *s;
	s++;
	putc(c, shout);
	if (c == '\n')
	    cleareol();
	if (mscroll && (++col == zterm_columns || c == '\n')) {
	    ml++;
	    if (!--mrestlines && (ask = asklistscroll(ml)))
		return ask;

	    col = 0;
	}
    }
    return 0;
}

/**/
static int
compprintlist(int showall)
{
    static int lasttype = 0, lastbeg = 0, lastml = 0, lastinvcount = -1;
    static int lastn = 0, lastnl = 0, lastnlnct = -1;
    static Cmgroup lastg = NULL;
    static Cmatch *lastp = NULL;
    static Cexpl *lastexpl = NULL;

    Cmgroup g;
    Cmatch *p, m;
    Cexpl *e;
    int pnl = 0, cl, mc = 0, ml = 0, printed = 0, stop = 0, asked = 1;
    int lastused = 0;

    mfirstl = -1;
    if (mnew || lastinvcount != invcount || lastbeg != mlbeg || mlbeg < 0) {
	lasttype = 0;
	lastg = NULL;
	lastexpl = NULL;
	lastml = 0;
	lastnlnct = -1;
    }
    cl = (listdat.nlines > zterm_lines - nlnct - mhasstat ?
	  zterm_lines - nlnct - mhasstat :
	  listdat.nlines) - (lastnlnct > nlnct);
    lastnlnct = nlnct;
    mrestlines = zterm_lines - 1;
    lastinvcount = invcount;

    if (cl < 2) {
	cl = -1;
	if (tccan(TCCLEAREOD))
	    tcout(TCCLEAREOD);
    } else if (mlbeg >= 0 && !tccan(TCCLEAREOL) && tccan(TCCLEAREOD))
	tcout(TCCLEAREOD);

    g = ((lasttype && lastg) ? lastg : amatches);
    while (g && !errflag) {
	char **pp = g->ylist;

#ifdef ZSH_HEAP_DEBUG
	if (memory_validate(g->heap_id)) {
	    HEAP_ERROR(g->heap_id);
	}
#endif
	if ((e = g->expls)) {
	    if (!lastused && lasttype == 1) {
		e = lastexpl;
		ml = lastml;
		lastused = 1;
	    }
	    while (*e && !errflag) {
		if (((*e)->count || (*e)->always) &&
		    (!listdat.onlyexpl ||
		     (listdat.onlyexpl & ((*e)->always > 0 ? 2 : 1)))) {
		    if (pnl) {
			if (dolistnl(ml) && compprintnl(ml))
			    goto end;
			pnl = 0;
			ml++;
			if (dolistcl(ml) && cl >= 0 && --cl <= 1) {
			    cl = -1;
			    if (tccan(TCCLEAREOD))
				tcout(TCCLEAREOD);
			}
		    }
		    if (mlbeg < 0 && mfirstl < 0)
			mfirstl = ml;
		    (void)compprintfmt((*e)->str,
				       ((*e)->always ? -1 : (*e)->count),
				       dolist(ml), 1, ml, &stop);
		    if (mselect >= 0) {
			int mm = (mcols * ml), i;

			for (i = mcols; i-- > 0; ) {
			    DPUTS(mm+i >= mgtabsize, "BUG: invalid position");
			    mtab[mm + i] = mtmark(NULL);
			    mgtab[mm + i] = mgmark(NULL);
			}
		    }
		    if (stop)
			goto end;
		    if (!lasttype && ml >= mlbeg) {
			lasttype = 1;
			lastg = g;
			lastbeg = mlbeg;
			lastml = ml;
			lastexpl = e;
			lastp = NULL;
			lastused = 1;
		    }
		    ml += mlprinted;
		    if (dolistcl(ml) && cl >= 0 && (cl -= mlprinted) <= 1) {
			cl = -1;
			if (tccan(TCCLEAREOD))
			    tcout(TCCLEAREOD);
		    }
		    pnl = 1;
		}
		e++;
		if (!mnew && ml > mlend)
		    goto end;
	    }
	}
	if (!listdat.onlyexpl && mlbeg < 0 && pp && *pp) {
	    if (pnl) {
		if (dolistnl(ml) && compprintnl(ml))
		    goto end;
		pnl = 0;
		ml++;
		if (cl >= 0 && --cl <= 1) {
		    cl = -1;
		    if (tccan(TCCLEAREOD))
			tcout(TCCLEAREOD);
		}
	    }
	    if (mlbeg < 0 && mfirstl < 0)
		mfirstl = ml;
	    if (g->flags & CGF_LINES) {
		while (*pp) {
		    if (compzputs(*pp, ml))
			goto end;
		    if (*++pp && compprintnl(ml))
			goto end;
		}
	    } else {
		int n = g->lcount, nl, nc, i, a;
		char **pq;

		nl = nc = g->lins;

		while (n && nl-- && !errflag) {
		    i = g->cols;
		    mc = 0;
		    pq = pp;
		    while (n && i-- && !errflag) {
			if (pq - g->ylist >= g->lcount)
			    break;
			if (compzputs(*pq, mscroll))
			    goto end;
			if (i) {
			    a = (g->widths ? g->widths[mc] : g->width) -
				strlen(*pq);
			    while (a--)
				putc(' ', shout);
			}
			pq += ((g->flags & CGF_ROWS) ? 1 : nc);
			mc++;
			n--;
		    }
		    if (n) {
			if (compprintnl(ml))
			    goto end;
			ml++;
			if (cl >= 0 && --cl <= 1) {
			    cl = -1;
			    if (tccan(TCCLEAREOD))
				tcout(TCCLEAREOD);
			}
		    }
		    pp += ((g->flags & CGF_ROWS) ? g->cols : 1);
		}
	    }
	} else if (!listdat.onlyexpl &&
		   (g->lcount || (showall && g->mcount))) {
	    int n = g->dcount, nl, nc, i, j, wid;
	    Cmatch *q;

	    nl = nc = g->lins;

	    if ((g->flags & CGF_HASDL) &&
		(lastused || !lasttype || lasttype == 2)) {
		if (!lastused && lasttype == 2) {
		    p = lastp;
		    ml = lastml;
		    n = lastn;
		    nl = lastnl;
		    lastused = 1;
		    pnl = 0;
		} else
		    p = g->matches;

		for (; (m = *p); p++) {
		    if (m->disp && (m->flags & CMF_DISPLINE) &&
                        (showall || !(m->flags & (CMF_HIDE|CMF_NOLIST)))) {
			if (pnl) {
			    if (dolistnl(ml) && compprintnl(ml))
				goto end;
			    pnl = 0;
			    ml++;
			    if (dolistcl(ml) && cl >= 0 && --cl <= 1) {
				cl = -1;
				if (tccan(TCCLEAREOD))
				    tcout(TCCLEAREOD);
			    }
			}
			if (!lasttype && ml >= mlbeg) {
			    lasttype = 2;
			    lastg = g;
			    lastbeg = mlbeg;
			    lastml = ml;
			    lastp = p;
			    lastn = n;
			    lastnl = nl;
			    lastused = 1;
			}
			if (mfirstl < 0)
			    mfirstl = ml;
			if (dolist(ml))
			    printed++;
			if (clprintm(g, p, 0, ml, 1, 0))
			    goto end;
			ml += mlprinted;
			if (dolistcl(ml) && (cl -= mlprinted) <= 1) {
			    cl = -1;
			    if (tccan(TCCLEAREOD))
				tcout(TCCLEAREOD);
			}
			pnl = 1;
		    }
		    if (!mnew && ml > mlend)
			goto end;
		}
	    }
	    if (n && pnl) {
		if (dolistnl(ml) && compprintnl(ml))
		    goto end;
		pnl = 0;
		ml++;
		if (dolistcl(ml) && cl >= 0 && --cl <= 1) {
		    cl = -1;
		    if (tccan(TCCLEAREOD))
			tcout(TCCLEAREOD);
		}
	    }
	    if (!lastused && lasttype == 3) {
		p = lastp;
		n = lastn;
		nl = lastnl;
		ml = lastml;
		lastused = 1;
	    } else
		p = skipnolist(g->matches, showall);

	    while (n && nl-- && !errflag) {
		if (!lasttype && ml >= mlbeg) {
		    lasttype = 3;
		    lastg = g;
		    lastbeg = mlbeg;
		    lastml = ml;
		    lastp = p;
		    lastn = n;
		    lastnl = nl + 1;
		    lastused = 1;
		}
		i = g->cols;
		mc = 0;
		q = p;
		while (n && i-- && !errflag) {
		    wid = (g->widths ? g->widths[mc] : g->width);
		    if (!(m = *q)) {
			if (clprintm(g, NULL, mc, ml, (!i), wid))
			    goto end;
			break;
		    }
                    if (clprintm(g, q, mc, ml, (!i), wid))
                        goto end;

		    if (dolist(ml))
			printed++;
		    ml += mlprinted;
		    if (dolistcl(ml) && (cl -= mlprinted) < 1) {
			cl = -1;
			if (tccan(TCCLEAREOD))
			    tcout(TCCLEAREOD);
		    }
		    if (mfirstl < 0)
			mfirstl = ml;

		    if (--n)
			for (j = ((g->flags & CGF_ROWS) ? 1 : nc);
			     j && *q; j--)
			    q = skipnolist(q + 1, showall);
		    mc++;
		}
		while (i-- > 0) {
		    if (clprintm(g, NULL, mc, ml, (!i),
				 (g->widths ? g->widths[mc] : g->width)))
			goto end;
		    mc++;
		}
		if (n) {
		    if (dolistnl(ml) && compprintnl(ml))
			goto end;
		    ml++;
		    if (dolistcl(ml) && cl >= 0 && --cl <= 1) {
			cl = -1;
			if (tccan(TCCLEAREOD))
			    tcout(TCCLEAREOD);
		    }
		    if (nl)
			for (j = ((g->flags & CGF_ROWS) ? g->cols : 1);
			     j && *p; j--)
			    p = skipnolist(p + 1, showall);
		}
		if (!mnew && ml > mlend)
		    goto end;
	    }
	}
	if (g->lcount || (showall && g->mcount))
	    pnl = 1;
	g = g->next;
    }
    asked = 0;
 end:
    mstatprinted = 0;
    lastlistlen = 0;
    if (nlnct <= 1)
	mscroll = 0;
    if (clearflag) {
	int nl;

	/* Move the cursor up to the prompt, if always_last_prompt *
	 * is set and all that...                                  */
	if (mlbeg >= 0) {
	    if ((nl = listdat.nlines + nlnct) >= zterm_lines) {
		if (mhasstat) {
		    putc('\n', shout);
		    compprintfmt(NULL, 0, 1, 1, mline, NULL);
                    mstatprinted = 1;
		}
		nl = zterm_lines - 1;
	    } else
		nl--;
	    tcmultout(TCUP, TCMULTUP, nl);
	    showinglist = -1;

	    lastlistlen = listdat.nlines;
	} else if ((nl = listdat.nlines + nlnct - 1) < zterm_lines) {
	    cleareol();
	    tcmultout(TCUP, TCMULTUP, nl);
	    showinglist = -1;

	    lastlistlen = listdat.nlines;
	} else {
	    clearflag = 0;
	    if (!asked) {
		mrestlines = (ml + nlnct > zterm_lines);
		compprintnl(ml);
	    }
	}
    } else if (!asked) {
	mrestlines = (ml + nlnct > zterm_lines);
	compprintnl(ml);
    }
    listshown = (clearflag ? 1 : -1);
    mnew = 0;

    return printed;
}

/**/
static int
clprintm(Cmgroup g, Cmatch *mp, int mc, int ml, int lastc, int width)
{
    Cmatch m;
    int len, subcols = 0, stop = 0, ret = 0;

    DPUTS2(mselect >= 0 && ml >= mlines,
	   "clprintm called with ml too large (%d/%d)",
	   ml, mlines);
    if (g != last_group)
        *last_cap = '\0';

    last_group = g;

    if (!mp) {
	if (dolist(ml)) {
	    zcputs(g->name, COL_SP);
	    len = width - 2;
	    while (len-- > 0)
		putc(' ', shout);
	    zcoff();
	}
	mlprinted = 0;
	return 0;
    }
    m = *mp;

    if ((m->flags & CMF_ALL) && (!m->disp || !m->disp[0]))
	bld_all_str(m);

    mlastm = m->gnum;
    if (m->disp && (m->flags & CMF_DISPLINE)) {
	if (mselect >= 0) {
	    int mm = (mcols * ml), i;

            if (m->flags & CMF_DUMMY) {
                for (i = mcols; i-- > 0; ) {
		    DPUTS(mm+i >= mgtabsize, "BUG: invalid position");
                    mtab[mm + i] = mtmark(mp);
                    mgtab[mm + i] = mgmark(g);
                }
            } else {
                for (i = mcols; i-- > 0; ) {
		    DPUTS(mm+i >= mgtabsize, "BUG: invalid position");
                    mtab[mm + i] = mp;
                    mgtab[mm + i] = g;
                }
            }
	}
	if (!dolist(ml)) {
	    mlprinted = printfmt(m->disp, 0, 0, 0);
	    return 0;
	}
	if (m->gnum == mselect) {
	    int mm = (mcols * ml);
	    DPUTS(mm >= mgtabsize, "BUG: invalid position");
	    mline = ml;
	    mcol = 0;
	    mmtabp = mtab + mm;
	    mgtabp = mgtab + mm;
	    zcputs(g->name, COL_MA);
	} else if ((m->flags & CMF_NOLIST) &&
                   mcolors.files[COL_HI] && mcolors.files[COL_HI]->col)
	    zcputs(g->name, COL_HI);
	else if (mselect >= 0 && (m->flags & (CMF_MULT | CMF_FMULT)) &&
                 mcolors.files[COL_DU] && mcolors.files[COL_DU]->col)
	    zcputs(g->name, COL_DU);
	else
	    subcols = putmatchcol(g->name, m->disp);
	if (subcols)
	    ret = clprintfmt(m->disp, ml);
	else {
	    compprintfmt(m->disp, 0, 1, 0, ml, &stop);
	    if (stop)
		ret = 1;
	}
	zcoff();
    } else {
	int mx, modec;

	if (g->widths) {
	    int i;

	    for (i = mx = 0; i < mc; i++)
		mx += g->widths[i];
	} else
	    mx = mc * g->width;

	if (mselect >= 0) {
	    int mm = mcols * ml, i;

            if (m->flags & CMF_DUMMY) {
                for (i = (width ? width : mcols); i-- > 0; ) {
		    DPUTS(mx+mm+i >= mgtabsize, "BUG: invalid position");
                    mtab[mx + mm + i] = mtmark(mp);
                    mgtab[mx + mm + i] = mgmark(g);
                }
            } else {
                for (i = (width ? width : mcols); i-- > 0; ) {
		    DPUTS(mx+mm+i >= mgtabsize, "BUG: invalid position");
                    mtab[mx + mm + i] = mp;
                    mgtab[mx + mm + i] = g;
                }
            }
	}
	if (!dolist(ml)) {
	    int nc = ZMB_nicewidth(m->disp ? m->disp : m->str);
	    if (nc)
		mlprinted = (nc-1) / zterm_columns;
	    else
		mlprinted = 0;
	    return 0;
	}
	if (m->gnum == mselect) {
	    int mm = mcols * ml;
	    DPUTS(mx+mm >= mgtabsize, "BUG: invalid position");

	    mcol = mx;
	    mline = ml;
	    mmtabp = mtab + mx + mm;
	    mgtabp = mgtab + mx + mm;
	    zcputs(g->name, COL_MA);
	} else if (m->flags & CMF_NOLIST)
	    zcputs(g->name, COL_HI);
	else if (mselect >= 0 && (m->flags & (CMF_MULT | CMF_FMULT)))
	    zcputs(g->name, COL_DU);
	else if (m->mode) {
	    /*
	     * Symlink is orphaned if we read the mode with lstat
	     * but couldn't read one with stat.  That's the
	     * only way they can be different so the following
	     * test should be enough.
	     */
	    int orphan_colour = (m->mode && !m->fmode) ? COL_OR : -1;
	    if (mcolors.flags & LC_FOLLOW_SYMLINKS) {
		subcols = putfilecol(g->name, m->str, m->fmode, orphan_colour);
	    } else {
		subcols = putfilecol(g->name, m->str, m->mode, orphan_colour);
	    }
	}
	else
	    subcols = putmatchcol(g->name, (m->disp ? m->disp : m->str));

	ret = clnicezputs(subcols,
			  (m->disp ? m->disp : m->str), ml);
	if (ret) {
	    zcoff();
	    return 1;
	}
	len = ZMB_nicewidth(m->disp ? m->disp : m->str);
	mlprinted = len ? (len-1) / zterm_columns : 0;

	modec = (mcolors.flags & LC_FOLLOW_SYMLINKS) ? m->fmodec : m->modec;
	if ((g->flags & CGF_FILES) && modec) {
	    if (m->gnum != mselect) {
		zcoff();
		zcputs(g->name, COL_TC);
	    }
	    putc(modec, shout);
	    len++;
        }
	if ((len = width - len - 2) > 0) {
	    if (m->gnum != mselect) {
		zcoff();
		zcputs(g->name, COL_SP);
	    }
	    while (len-- > 0)
		putc(' ', shout);
	}
	zcoff();
	if (!lastc) {
	    zcputs(g->name, COL_SP);
	    fputs("  ", shout);
	    zcoff();
	}
    }
    return ret;
}

static int
singlecalc(int *cp, int l, int *lcp)
{
    int c = *cp, n, j, first = 1;
    Cmatch **p, *op, *mp = mtab[l * zterm_columns + c];

    for (n = 0, j = c, p = mtab + l * zterm_columns + c, op = NULL;
	 j >= 0;
	 j--, p--) {
        if (*p == mp)
            c = j;
        if (!first && *p != op)
            n++;
        op = *p;
        first = 0;
    }
    *cp = c;
    *lcp = 1;
    for (p = mtab + l * zterm_columns + c; c < zterm_columns; c++, p++)
        if (*p && mp != *p)
            *lcp = 0;

    return n;
}

static void
singledraw(void)
{
    Cmgroup g;
    int mc1, mc2, ml1, ml2, md1, md2, mcc1, mcc2, lc1, lc2, t1, t2;

    t1 = mline - mlbeg;
    t2 = moline - molbeg;

    if (t2 < t1) {
        mc1 = mocol; ml1 = moline; md1 = t2;
        mc2 = mcol; ml2 = mline; md2 = t1;
    } else {
        mc1 = mcol; ml1 = mline; md1 = t1;
        mc2 = mocol; ml2 = moline; md2 = t2;
    }
    mcc1 = singlecalc(&mc1, ml1, &lc1);
    mcc2 = singlecalc(&mc2, ml2, &lc2);

    if (md1)
        tc_downcurs(md1);
    if (mc1)
        tcmultout(TCRIGHT, TCMULTRIGHT, mc1);
    DPUTS(ml1 * zterm_columns + mc1 >= mgtabsize, "BUG: invalid position");
    g = mgtab[ml1 * zterm_columns + mc1];
    clprintm(g, mtab[ml1 * zterm_columns + mc1], mcc1, ml1, lc1,
             (g->widths ? g->widths[mcc1] : g->width));
    if (mlprinted)
	(void) tcmultout(TCUP, TCMULTUP, mlprinted);
    putc('\r', shout);

    if (md2 != md1)
        tc_downcurs(md2 - md1);
    if (mc2)
        tcmultout(TCRIGHT, TCMULTRIGHT, mc2);
    DPUTS(ml2 * zterm_columns + mc2 >= mgtabsize, "BUG: invalid position");
    g = mgtab[ml2 * zterm_columns + mc2];
    clprintm(g, mtab[ml2 * zterm_columns + mc2], mcc2, ml2, lc2,
             (g->widths ? g->widths[mcc2] : g->width));
    if (mlprinted)
	(void) tcmultout(TCUP, TCMULTUP, mlprinted);
    putc('\r', shout);

    if (mstatprinted) {
        int i = zterm_lines - md2 - nlnct;

        tc_downcurs(i - 1);
        compprintfmt(NULL, 0, 1, 1, mline, NULL);
        tcmultout(TCUP, TCMULTUP, zterm_lines - 1);
    } else
        tcmultout(TCUP, TCMULTUP, md2 + nlnct);

    showinglist = -1;
    listshown = 1;
}

static int
complistmatches(UNUSED(Hookdef dummy), Chdata dat)
{
    static int onlnct = -1;
    static int extendedglob;

    Cmgroup oamatches = amatches;

    amatches = dat->matches;
#ifdef ZSH_HEAP_DEBUG
    if (memory_validate(amatches->heap_id)) {
	HEAP_ERROR(amatches->heap_id);
    }
#endif

    if (noselect > 0)
	noselect = 0;

    if ((minfo.asked == 2 && mselect < 0) || nlnct >= zterm_lines ||
	errflag) {
	showinglist = 0;
	amatches = oamatches;
	return (noselect = 1);
    }

    /*
     * There's a lot of memory allocation from this function
     * for setting up the color display which isn't needed
     * after the function exits, so it's worthwhile pushing
     * another heap.  As this is called from a hook in the main
     * completion handler nothing temporarily allocated from here can be
     * useful outside.
     */
    pushheap();
    extendedglob = opts[EXTENDEDGLOB];
    opts[EXTENDEDGLOB] = 1;

    getcols();

    mnew = ((calclist(mselect >= 0) || mlastcols != zterm_columns ||
	     mlastlines != listdat.nlines) && mselect >= 0);

    if (!listdat.nlines || (mselect >= 0 &&
			    !(isset(USEZLE) && !termflags &&
			      complastprompt && *complastprompt))) {
	showinglist = listshown = 0;
	noselect = 1;
	amatches = oamatches;
	popheap();
	opts[EXTENDEDGLOB] = extendedglob;
	return 1;
    }
    if (inselect || mlbeg >= 0)
	clearflag = 0;

    mscroll = 0;
    mlistp = NULL;

    queue_signals();
    if (mselect >= 0 || mlbeg >= 0 ||
	(mlistp = dupstring(getsparam("LISTPROMPT")))) {
	unqueue_signals();
	if (mlistp && !*mlistp)
	    mlistp = "%SAt %p: Hit TAB for more, or the character to insert%s";
	trashzle();
	showinglist = listshown = 0;

	lastlistlen = 0;

	if (mlistp) {
	    clearflag = (isset(USEZLE) && !termflags && dolastprompt);
	    mscroll = 1;
	} else {
	    clearflag = 1;
	    minfo.asked = (listdat.nlines + nlnct <= zterm_lines);
	}
    } else {
	unqueue_signals();
	mlistp = NULL;
	if (asklist()) {
	    amatches = oamatches;
	    popheap();
	    opts[EXTENDEDGLOB] = extendedglob;
	    return (noselect = 1);
	}
    }
    if (mlbeg >= 0) {
	mlend = mlbeg + zterm_lines - nlnct - mhasstat;
	while (mline >= mlend)
	    mlbeg++, mlend++;
    } else
	mlend = 9999999;

    if (mnew) {
	int i;

    	mtab_been_reallocated = 1;

	i = zterm_columns * listdat.nlines;
	free(mtab);
	mtab = (Cmatch **) zalloc(i * sizeof(Cmatch *));
	memset(mtab, 0, i * sizeof(Cmatch *));
	free(mgtab);
	mgtab = (Cmgroup *) zalloc(i * sizeof(Cmgroup));
#ifdef DEBUG
	mgtabsize = i;
#endif
	memset(mgtab, 0, i * sizeof(Cmgroup));
	mlastcols = mcols = zterm_columns;
	mlastlines = mlines = listdat.nlines;
	mmtabp = 0;
    }
    last_cap = (char *) zhalloc(max_caplen + 1);
    *last_cap = '\0';

    if (!mnew && inselect &&
	onlnct == nlnct && mlbeg >= 0 && mlbeg == molbeg) {
	if (!noselect)
	    singledraw();
    } else if (!compprintlist(mselect >= 0) || !clearflag)
	noselect = 1;

    onlnct = nlnct;
    molbeg = mlbeg;
    mocol = mcol;
    moline = mline;

    amatches = oamatches;

    popheap();
    opts[EXTENDEDGLOB] = extendedglob;

    return (noselect < 0 ? 0 : noselect);
}

static int
adjust_mcol(int wish, Cmatch ***tabp, Cmgroup **grp)
{
    Cmatch **matchtab = *tabp;
    int p, n, c;

    matchtab -= mcol;

    for (p = wish; p >= 0 && (!matchtab[p] || mmarked(matchtab[p])); p--);
    for (n = wish; n < mcols && (!matchtab[n] || mmarked(matchtab[n])); n++);
    if (n == mcols)
	n = -1;

    if (p < 0) {
	if (n < 0)
	    return 1;
	c = n;
    } else if (n < 0)
	c = p;
    else
	c = ((mcol - p) < (n - mcol) ? p : n);

    *tabp = matchtab + c;
    if (grp)
	*grp = *grp + c - mcol;

    mcol = c;
    
    return 0;
}

typedef struct menustack *Menustack;

struct menustack {
    Menustack prev;
    char *line;
    Brinfo brbeg;
    Brinfo brend;
    int nbrbeg, nbrend;
    int cs, acc, nmatches, mline, mlbeg, nolist;
    struct menuinfo info;
    Cmgroup amatches, pmatches, lastmatches, lastlmatches;
    /*
     * Status for how line looked like previously.
     */
    char *origline;
    int origcs, origll;
    /*
     * Status for interactive mode.  status is the line
     * printed above the matches saying what the interactive
     * completion prefix is.  mode says whether we are in
     * interactive or some search mode.
     * typed.
     */
    char *status;
    int mode;
};

typedef struct menusearch *Menusearch;

struct menusearch {
    Menusearch prev;
    char *str;
    int line;
    int col;
    int back;
    int state;
    Cmatch **ptr;
};

#define MS_OK       0
#define MS_FAILED   1
#define MS_WRAPPED  2

#define MAX_STATUS 128

static char *
setmstatus(char *status, char *sline, int sll, int scs,
           int *csp, int *llp, int *lenp)
{
    char *p, *s, *ret = NULL;
    int pl, sl, max;

    METACHECK();

    if (csp) {
        *csp = zlemetacs;
        *llp = zlemetall;
        *lenp = lastend - wb;

        ret = dupstring(zlemetaline);

        p = (char *) zhalloc(zlemetacs - wb + 1);
        strncpy(p, zlemetaline + wb, zlemetacs - wb);
        p[zlemetacs - wb] = '\0';
        if (lastend < zlemetacs)
            s = "";
        else {
            s = (char *) zhalloc(lastend - zlemetacs + 1);
            strncpy(s, zlemetaline + zlemetacs, lastend - zlemetacs);
            s[lastend - zlemetacs] = '\0';
        }
        zlemetacs = 0;
        foredel(zlemetall, CUT_RAW);
        spaceinline(sll);
        memcpy(zlemetaline, sline, sll);
        zlemetacs = scs;
    } else {
        p = complastprefix;
        s = complastsuffix;
    }
    pl = strlen(p);
    sl = strlen(s);
    max = (zterm_columns < MAX_STATUS ? zterm_columns : MAX_STATUS) - 14;

    if (max > 12) {
        int h = (max - 2) >> 1;

        strcpy(status, "interactive: ");
        if (pl > h - 3) {
            strcat(status, "...");
            strcat(status, p + pl - h - 3);
        } else
            strcat(status, p);

        strcat(status, "[]");
        if (sl > h - 3) {
            strncat(status, s, h - 3);
            strcat(status, "...");
        } else
            strcat(status, s);
    }
    return ret;
}

static Menusearch msearchstack;
static char *msearchstr = NULL;
static int msearchstate;

static void
msearchpush(Cmatch **p, int back)
{
    Menusearch s = (Menusearch) zhalloc(sizeof(struct menusearch));

    s->prev = msearchstack;
    msearchstack = s;
    s->str = dupstring(msearchstr);
    s->line = mline;
    s->col = mcol;
    s->back = back;
    s->state = msearchstate;
    s->ptr = p;
}

static Cmatch **
msearchpop(int *backp)
{
    Menusearch s = msearchstack;

    if (!s)
        return NULL;

    if (s->prev)
        msearchstack = s->prev;

    msearchstr = s->str;
    mline = s->line;
    mcol = s->col;
    msearchstate = s->state;

    *backp = s->back;

    return s->ptr;
}

static Cmatch **
msearch(Cmatch **ptr, char *ins, int back, int rep, int *wrapp)
{
    Cmatch **p, *l = NULL, m;
    int x = mcol, y = mline;
    int ex, ey, wrap = 0, owrap = (msearchstate & MS_WRAPPED);

    msearchpush(ptr, back);

    if (ins)
        msearchstr = dyncat(msearchstr, ins);
    if (back) {
        ex = mcols - 1;
        ey = -1;
    } else {
        ex = 0;
        ey = listdat.nlines;
    }
    p = mtab + (mline * mcols) + mcol;
    if (rep)
        l = *p;
    while (1) {
        if (!rep && mtunmark(*p) && *p != l) {
            l = *p;
            m = *mtunmark(*p);

            if (strstr((m->disp ? m->disp : m->str), msearchstr)) {
                mcol = x;
                mline = y;

                return p;
            }
        }
        rep = 0;

        if (back) {
            p--;
            if (--x < 0) {
                x = mcols - 1;
                y--;
            }
        } else {
            p++;
            if (++x == mcols) {
                x = 0;
                y++;
            }
        }
        if (x == ex && y == ey) {

            if (back) {
                x = mcols - 1;
                y = listdat.nlines - 1;
                p = mtab + (y * mcols) + x;
            } else {
                x = y = 0;
                p = mtab;
            }
            ex = mcol;
            ey = mline;

            if (wrap || (x == ex && y == ey)) {
                msearchstate = MS_FAILED | owrap;
                break;
            }

            msearchstate |= MS_WRAPPED;
            wrap = 1;
            *wrapp = 1;
        }
    }
    return NULL;
}

/*
 * Values to assign to mode: interactive, etc.
 */
#define MM_INTER   1
#define MM_FSEARCH 2
#define MM_BSEARCH 3

static int
domenuselect(Hookdef dummy, Chdata dat)
{
    static Chdata fdat = NULL;
    static char *lastsearch = NULL;
    Cmatch **p;
    Cmgroup *pg;
    Thingy cmd = 0;
    int     do_last_key = 0;
    Menustack u = NULL;
    int i = 0, acc = 0, wishcol = 0, setwish = 0, oe = onlyexpl, wasnext = 0;
    int space, lbeg = 0, step = 1, wrap, pl = nlnct, broken = 0, first = 1;
    int nolist = 0, mode = 0, modecs, modell, modelen, wasmeta;
    char *s;
    char status[MAX_STATUS], *modeline = NULL;

    msearchstack = NULL;
    msearchstr = "";
    msearchstate = MS_OK;

    status[0] = '\0';
    queue_signals();
    if (fdat || (dummy && (!(s = getsparam("MENUSELECT")) ||
			   (dat && dat->num < atoi(s))))) {
	if (fdat) {
	    fdat->matches = dat->matches;
	    fdat->num = dat->num;
	    fdat->nmesg = dat->nmesg;
	}
	unqueue_signals();
	return 0;
    }
    /*
     * Lots of the logic here doesn't really make sense if the
     * line isn't metafied, but the evidence was that it only used
     * to be metafied locally in a couple of places.
     * It's horrifically difficult to work out where the line
     * is metafied, so I've resorted to the following.
     * Unfortunately we need to unmetafy in zrefresh() when
     * we want to display something.  Maybe this function can
     * be done better.
     */
    if (zlemetaline != NULL)
	wasmeta = 1;
    else {
	wasmeta = 0;
	metafy_line();
    }
    
    if ((s = getsparam("MENUSCROLL"))) {
	if (!(step = mathevali(s)))
	    step = (zterm_lines - nlnct) >> 1;
	else if (step < 0)
	    if ((step += zterm_lines - nlnct) < 0)
		step = 1;
    }
    if ((s = getsparam("MENUMODE"))) {
        if (!strcmp(s, "interactive")) {
            int l = strlen(origline);

	    /*
	     * In interactive completion mode we don't insert
	     * the completion onto the command line, instead
	     * we show just what the user has typed and
	     * the match so far underneath (stored in "status").
	     * So put the command line back to how it
	     * was before completion started.
	     */
            mode = MM_INTER;
            zlemetacs = 0;
            foredel(zlemetall, CUT_RAW);
            spaceinline(l);
            strncpy(zlemetaline, origline, l);
            zlemetacs = origcs;
            setmstatus(status, NULL, 0 , 0, NULL, NULL, NULL);
        } else if (strpfx("search", s)) {
            mode = (strstr(s, "back") ? MM_BSEARCH : MM_FSEARCH);
        }
    }
    if ((mstatus = dupstring(getsparam("MENUPROMPT"))) && !*mstatus)
	mstatus = "%SScrolling active: current selection at %p%s";
    unqueue_signals();
    mhasstat = (mstatus && *mstatus);
    fdat = dat;
    menuselect_bindings();	/* sanity in case deleted by user */
    selectlocalmap(mskeymap);
    noselect = 1;
    while ((menuacc &&
	    !hasbrpsfx(*(minfo.cur), minfo.prebr, minfo.postbr)) ||
	   ((*minfo.cur)->flags & CMF_DUMMY) ||
	   (((*minfo.cur)->flags & (CMF_NOLIST | CMF_MULT)) &&
	    (!(*minfo.cur)->str || !*(*minfo.cur)->str)))
	do_menucmp(0);

    mselect = (*(minfo.cur))->gnum;
    mline = 0;
    mlines = 999999;
    mlbeg = 0;
    molbeg = -42;
    mtab_been_reallocated = 0;
    for (;;) {
	METACHECK();

	if (mline < 0 || mtab_been_reallocated) {
	    int x, y;
	    Cmatch **p = mtab;

	    for (y = 0; y < mlines; y++) {
		for (x = mcols; x > 0; x--, p++)
		    if (*p && !mmarked(*p) && **p && mselect == (**p)->gnum)
			break;
		if (x) {
                    mcol = mcols - x;
		    break;
                }
	    }
	    if (y < mlines)
		mline = y;
	}
    	mtab_been_reallocated = 0;
	DPUTS(mline < 0,
	      "BUG: mline < 0 after re-scanning mtab in domenuselect()");
	while (mline < mlbeg)
	    if ((mlbeg -= step) < 0) {
		mlbeg = 0;
		/* Crude workaround for BUG above */
		if (mline < 0)
		    break;
	    }

	if (mlbeg && lbeg != mlbeg) {
	    Cmatch **p = mtab + ((mlbeg - 1) * zterm_columns), **q;
	    int c;

	    while (mlbeg) {
		for (q = p, c = zterm_columns; c > 0; q++, c--)
		    if (*q && !mmarked(*q))
			break;
		if (c)
		    break;
		p -= zterm_columns;
		mlbeg--;
	    }
	}
	if ((space = zterm_lines - pl - mhasstat) > 0)
	    while (mline >= mlbeg + space)
		if ((mlbeg += step) + space > mlines)
		    mlbeg = mlines - space;
	if (lbeg != mlbeg) {
	    Cmatch **p = mtab + (mlbeg * zterm_columns), **q;
	    int c;

	    while (mlbeg < mlines) {
		for (q = p, c = zterm_columns; c > 0; q++, c--)
		    if (*q)
			break;
		if (c)
		    break;
		p += zterm_columns;
		mlbeg++;
	    }
	}
	lbeg = mlbeg;
        onlyexpl = 0;
        showinglist = -2;
        if (first && !listshown && isset(LISTBEEP))
            zbeep();
        if (first) {
	    /*
	     * remember the original data that we will use when
	     * performing interactive completion to restore the
	     * command line when a menu completion is inserted.
	     * this is because menu completion will insert
	     * the next match in the loop; for interactive
	     * completion we don't want that, we always want to
	     * be able to type the next character.
	     */
	    modeline = dupstring(zlemetaline);
            modecs = zlemetacs;
            modell = zlemetall;
            modelen = minfo.len;
        }
        first = 0;
        if (mode == MM_INTER)
	    statusline = status;
        else if (mode) {
            int l = sprintf(status, "%s%sisearch%s: ",
                            ((msearchstate & MS_FAILED) ? "failed " : ""),
                            ((msearchstate & MS_WRAPPED) ? "wrapped " : ""),
                            (mode == MM_FSEARCH ? "" : " backward"));

            strncat(status, msearchstr, MAX_STATUS - l - 1);

            statusline = status;
        } else {
            statusline = NULL;
        }
	if (noselect < 0) {
	    showinglist = clearlist = 0;
	    clearflag = 1;
	}
        zrefresh();
	statusline = NULL;
        inselect = 1;
	selected = 1;
        if (noselect) {
	    if (noselect < 0) {
		/* no selection until after processing keystroke */
		noselect = 0;
		goto getk;
	    }
            broken = 1;
            break;
        }
	if (!i) {
	    i = mcols * mlines;
	    while (i--)
		if (mtab[i])
		    break;
	    if (!i)
		break;
	    i = 1;
	}
	p = mmtabp;
	pg = mgtabp;
	if (!p) /* selected match not in display, find line */
	    continue;
	minfo.cur = *p;
	minfo.group = *pg;
	if (setwish)
	    wishcol = mcol;
	else if (mcol > wishcol) {
	    while (mcol > 0 && p[-1] == minfo.cur)
		mcol--, p--, pg--;
	} else if (mcol < wishcol) {
	    while (mcol < mcols - 1 && p[1] == minfo.cur)
		mcol++, p++, pg++;
	}
	setwish = wasnext = 0;

    getk:

    	if (!do_last_key) {
	    zmult = 1;
	    cmd = getkeycmd();
	    /*
	     * On interrupt, we'll exit due to cmd being empty.
	     * Don't propagate the interrupt any further, which
	     * can screw up redrawing.
	     */
	    errflag &= ~ERRFLAG_INT;
	    if (mtab_been_reallocated) {
		do_last_key = 1;
		continue;
	    }
    	}
	do_last_key = 0;

	if (!cmd || cmd == Th(z_sendbreak)) {
	    zbeep();
            molbeg = -1;
	    break;
	} else if (nolist && cmd != Th(z_undo) &&
                   (!mode || (cmd != Th(z_backwarddeletechar) &&
                              cmd != Th(z_selfinsert) &&
                              cmd != Th(z_selfinsertunmeta)))) {
	    ungetkeycmd();
	    break;
	} else if (cmd == Th(z_acceptline) || cmd == Th(z_acceptsearch)) {
            if (mode == MM_FSEARCH || mode == MM_BSEARCH) {
                mode = 0;
                continue;
            }
	    acc = 1;
	    break;
        } else if (cmd == Th(z_viinsert)) {
            if (mode == MM_INTER)
                mode = 0;
            else {
                int l = strlen(origline);

		/*
		 * Entering interactive completion mode:
		 * same code as when we enter it on menu selection
		 * start.
		 */
                mode = MM_INTER;
                zlemetacs = 0;
                foredel(zlemetall, CUT_RAW);
                spaceinline(l);
                strncpy(zlemetaline, origline, l);
                zlemetacs = origcs;
                setmstatus(status, NULL, 0, 0, NULL, NULL, NULL);

                continue;
            }
	} else if (cmd == Th(z_acceptandinfernexthistory) ||
                   (mode == MM_INTER && (cmd == Th(z_selfinsert) ||
                                         cmd == Th(z_selfinsertunmeta)))) {
            char *saveline = NULL;
            int savell = 0;
            int savecs = 0;
	    Menustack s = (Menustack) zhalloc(sizeof(*s));

	    s->prev = u;
	    u = s;
	    s->line = dupstring(zlemetaline);
	    s->cs = zlemetacs;
	    s->mline = mline;
	    s->mlbeg = mlbeg;
	    memcpy(&(s->info), &minfo, sizeof(struct menuinfo));
	    s->amatches = amatches;
#ifdef ZSH_HEAP_DEBUG
	    if (memory_validate(amatches->heap_id)) {
		HEAP_ERROR(amatches->heap_id);
	    }
#endif
	    s->pmatches = pmatches;
	    s->lastmatches = lastmatches;
	    s->lastlmatches = lastlmatches;
            s->nolist = nolist;
	    s->acc = menuacc;
	    s->brbeg = dupbrinfo(brbeg, NULL, 1);
	    s->brend = dupbrinfo(brend, NULL, 1);
	    s->nbrbeg = nbrbeg;
	    s->nbrend = nbrend;
	    s->nmatches = nmatches;
	    s->origline = dupstring(origline);
	    s->origcs = origcs;
	    s->origll = origll;
            s->status = dupstring(status);
	    /*
	     * with just the slightest hint of a note of infuriation:
	     * mode here is the menu mode, not the file mode, despite
	     * the fact we're in a file dealing with file highlighting;
	     * but that's OK, because s is a menu stack entry, despite
	     * the fact we're in a function declaring s as char *.
	     * anyway, in functions we really mean *mode* it's
	     * called m, to be clear.
	     */
            s->mode = mode;
	    menucmp = menuacc = hasoldlist = 0;
	    minfo.cur = NULL;
	    fixsuffix();
	    handleundo();
	    validlist = 0;
	    amatches = pmatches = lastmatches = NULL;
	    invalidate_list();
	    iforcemenu = 1;
	    comprecursive = 1;
            if (cmd != Th(z_acceptandinfernexthistory)) {
                int l = strlen(origline);

		/*
		 * Interactive mode: we need to restore the
		 * line, add the character, then remember how
		 * this new line looks in order to keep
		 * the command line as it is with just the
		 * characters typed by the user.
		 */
                zlemetacs = 0;
                foredel(zlemetall, CUT_RAW);
                spaceinline(l);
                strncpy(zlemetaline, origline, l);
                zlemetacs = origcs;

		/*
		 * Horrible quick fix:
		 * we shouldn't need to metafy and unmetafy
		 * quite as much.  If we kept unmetafied through
		 * here we could fix up setmstatus to use unmetafied
		 * as well.  This is the only use of setmstatus which
		 * restores the line so that should be doable.
		 */
		unmetafy_line();
                if (cmd == Th(z_selfinsert))
                    selfinsert(zlenoargs);
                else
                    selfinsertunmeta(zlenoargs);
		metafy_line();
		minfo.len++;
		minfo.end++;

                saveline = (char *) zhalloc(zlemetall);
                memcpy(saveline, zlemetaline, zlemetall);
                savell = zlemetall;
                savecs = zlemetacs;
                iforcemenu = -1;
            } else
                mode = 0;
	    /* Nested completion assumes line is unmetafied */
	    unmetafy_line();
	    menucomplete(zlenoargs);
	    metafy_line();
	    iforcemenu = 0;

            if (cmd != Th(z_acceptandinfernexthistory))
                modeline = setmstatus(status, saveline, savell, savecs,
                                      &modecs, &modell, &modelen);

	    if (nmatches < 1 || !minfo.cur || !*(minfo.cur)) {
		nolist = 1;
                if (mode == MM_INTER) {
                    statusline = status;
                } else {
		    /* paranoia */
		    statusline = NULL;
		}
		if (nmessages) {
		    showinglist = -2;
		    zrefresh();
		    noselect = -1;
		} else {
		    trashzle();
		    zsetterm();
		    if (tccan(TCCLEAREOD))
			tcout(TCCLEAREOD);
		    fputs("no matches\r", shout);
		    fflush(shout);
		    tcmultout(TCUP, TCMULTUP, nlnct);
		    showinglist = clearlist = 0;
		    clearflag = 1;
		    zrefresh();
		    showinglist = clearlist = 0;
		}
		statusline = NULL;

		goto getk;
	    }
	    clearlist = listshown = 1;
	    mselect = (*(minfo.cur))->gnum;
	    setwish = wasnext = 1;
	    mline = 0;
            molbeg = -42;
	    continue;
	} else if (cmd == Th(z_acceptandhold) ||
		   cmd == Th(z_acceptandmenucomplete)) {
	    Menustack s = (Menustack) zhalloc(sizeof(*s));
	    int ol;

	    if (mode == MM_INTER)
		do_single(*minfo.cur);
	    mode = 0;
	    s->prev = u;
	    u = s;
	    s->line = dupstring(zlemetaline);
	    s->cs = zlemetacs;
	    s->mline = mline;
	    s->mlbeg = mlbeg;
	    memcpy(&(s->info), &minfo, sizeof(struct menuinfo));
	    s->amatches = s->pmatches =
		s->lastmatches = s->lastlmatches = NULL;
            s->nolist = nolist;
	    s->acc = menuacc;
	    s->brbeg = dupbrinfo(brbeg, NULL, 1);
	    s->brend = dupbrinfo(brend, NULL, 1);
	    s->nbrbeg = nbrbeg;
	    s->nbrend = nbrend;
	    s->nmatches = nmatches;
	    s->origline = dupstring(origline);
	    s->origcs = origcs;
	    s->origll = origll;
            s->status = dupstring(status);
	    /* see above */
            s->mode = mode;
	    accept_last();
	    handleundo();
	    comprecursive = 1;
	    do_menucmp(0);
	    mselect = (*(minfo.cur))->gnum;

	    p -= mcol;
	    mcol = 0;
	    ol = mline;
	    do {
		for (mcol = 0; mcol < mcols; mcol++, p++)
		    if (*p == minfo.cur)
			break;
		if (mcol != mcols)
		    break;
		if (++mline == mlines) {
		    mline = 0;
		    p -= mlines * mcols;
		}
	    } while (mline != ol);
	    if (*p != minfo.cur) {
		noselect = clearlist = listshown = 1;
		onlyexpl = 0;
		zrefresh();
		break;
	    }
	    setwish = 1;
	    continue;
	} else if (cmd == Th(z_undo) ||
                   (mode == MM_INTER && cmd == Th(z_backwarddeletechar))) {
	    int l;

	    if (!u)
		break;

	    handleundo();
	    zlemetacs = 0;
	    foredel(zlemetall, CUT_RAW);
	    spaceinline(l = strlen(u->line));
	    strncpy(zlemetaline, u->line, l);
	    zlemetacs = u->cs;
	    menuacc = u->acc;
	    memcpy(&minfo, &(u->info), sizeof(struct menuinfo));
	    p = &(minfo.cur);
	    mline = u->mline;
	    mlbeg = u->mlbeg;
	    if (u->lastmatches && lastmatches != u->lastmatches) {
		if (lastmatches)
		    freematches(lastmatches, 0);
		amatches = u->amatches;
#ifdef ZSH_HEAP_DEBUG
		if (memory_validate(amatches->heap_id)) {
		    HEAP_ERROR(amatches->heap_id);
		}
#endif
		pmatches = u->pmatches;
		lastmatches = u->lastmatches;
		lastlmatches = u->lastlmatches;
		nmatches = u->nmatches;
		hasoldlist = validlist = 1;
	    }
	    freebrinfo(brbeg);
	    freebrinfo(brend);
	    brbeg = dupbrinfo(u->brbeg, &lastbrbeg, 0);
	    brend = dupbrinfo(u->brend, &lastbrend, 0);
	    nbrbeg = u->nbrbeg;
	    nbrend = u->nbrend;
	    zsfree(origline);
	    origline = ztrdup(u->origline);
	    origcs = u->origcs;
	    origll = u->origll;
            strcpy(status, u->status);
            mode = u->mode;
            nolist = u->nolist;

	    u = u->prev;
	    clearlist = 1;
	    setwish = 1;
	    listdat.valid = 0;
            molbeg = -42;

            if (nolist) {
                if (mode == MM_INTER) {
                    statusline = status;
                } else {
		    /* paranoia */
		    statusline = NULL;
		}
                zrefresh();
		statusline = NULL;
                goto getk;
            }
            if (mode)
                continue;
	} else if (cmd == Th(z_redisplay)) {
	    redisplay(zlenoargs);
            molbeg = -42;
	    continue;
	} else if (cmd == Th(z_clearscreen)) {
	    clearscreen(zlenoargs);
            molbeg = -42;
	    continue;
	} else if (cmd == Th(z_downhistory) ||
		   cmd == Th(z_downlineorhistory) ||
		   cmd == Th(z_downlineorsearch) ||
		   cmd == Th(z_vidownlineorhistory)) {
	    int omline;
	    Cmatch **op;

            mode = 0;
	    wrap = 0;

	down:

	    omline = mline;
	    op = p;

	    do {
		if (mline == mlines - 1) {
		    if (wrap & 2) {
			mline = omline; 
			p = op;
			break;
		    }
		    p -= mline * mcols;
		    mline = 0;
		    wrap |= 1;
		} else {
		    mline++;
		    p += mcols;
		}
		if (adjust_mcol(wishcol, &p, NULL))
		    continue;
	    } while (!*p || mmarked(*p));

	    if (wrap == 1)
		goto right;
	} else if (cmd == Th(z_uphistory) ||
		   cmd == Th(z_uplineorhistory) ||
		   cmd == Th(z_uplineorsearch) ||
		   cmd == Th(z_viuplineorhistory)) {
	    int omline;
	    Cmatch **op;

            mode = 0;
	    wrap = 0;

	up:

	    omline = mline;
	    op = p;

	    do {
		if (!mline) {
		    if (wrap & 2) {
			mline = omline; 
			p = op;
			break;
		    }
		    mline = mlines - 1;
		    p += mline * mcols;
		    wrap |= 1;
		} else {
		    mline--;
		    p -= mcols;
		}
		if (adjust_mcol(wishcol, &p, NULL))
		    continue;
	    } while (!*p || mmarked(*p));

	    if (wrap == 1) {
		if (mcol == wishcol)
		    goto left;

		wishcol = mcol;
	    }
	} else if (cmd == Th(z_emacsforwardword) ||
		   cmd == Th(z_viforwardword) ||
		   cmd == Th(z_viforwardwordend) ||
		   cmd == Th(z_forwardword)) {
	    int i = zterm_lines - pl - 1, oi = i, ll = 0;
	    Cmatch **lp = NULL;

            mode = 0;
	    if (mline == mlines - 1)
		goto top;
	    while (i > 0) {
		if (mline == mlines - 1) {
		    if (i != oi && lp)
			break;
		    goto top;
		} else {
		    mline++;
		    p += mcols;
		}
		if (adjust_mcol(wishcol, &p, NULL))
		    continue;
		if (*p && !mmarked(*p)) {
		    i--;
		    lp = p;
		    ll = mline;
		}
	    }
	    p = lp;
	    mline = ll;
	} else if (cmd == Th(z_emacsbackwardword) ||
		   cmd == Th(z_vibackwardword) ||
		   cmd == Th(z_backwardword)) {
	    int i = zterm_lines - pl - 1, oi = i, ll = 0;
	    Cmatch **lp = NULL;

            mode = 0;
	    if (!mline)
		goto bottom;
	    while (i > 0) {
		if (!mline) {
		    if (i != oi && lp)
			break;
		    goto bottom;
		} else {
		    mline--;
		    p -= mcols;
		}
		if (adjust_mcol(wishcol, &p, NULL))
		    continue;
		if (*p || !mmarked(*p)) {
		    i--;
		    lp = p;
		    ll = mline;
		}
	    }
	    p = lp;
	    mline = ll;
	} else if (cmd == Th(z_beginningofhistory)) {
	    int ll;
	    Cmatch **lp;

            mode = 0;

	top:

	    ll = mline;
	    lp = p;
	    while (mline) {
		mline--;
		p -= mcols;
		if (adjust_mcol(wishcol, &p, NULL))
		    continue;
		if (*p && !mmarked(*p)) {
		    lp = p;
		    ll = mline;
		}
	    }
	    mline = ll;
	    p = lp;
	} else if (cmd == Th(z_endofhistory)) {
	    int ll;
	    Cmatch **lp;

            mode = 0;

	bottom:

	    ll = mline;
	    lp = p;
	    while (mline < mlines - 1) {
		mline++;
		p += mcols;
		if (adjust_mcol(wishcol, &p, NULL))
		    continue;
		if (*p && !mmarked(*p)) {
		    lp = p;
		    ll = mline;
		}
	    }
	    mline = ll;
	    p = lp;
	} else if (cmd == Th(z_forwardchar) || cmd == Th(z_viforwardchar)) {
	    int omcol;
	    Cmatch **op;

            mode = 0;
	    wrap = 0;

	right:

	    omcol = mcol;
	    op = p;

	    do {
		if (mcol == mcols - 1) {
		    if (wrap & 1) {
			p = op;
			mcol = omcol;
			break;
		    }
		    p -= mcol;
		    mcol = 0;
		    wrap |= 2;
		} else {
		    mcol++;
		    p++;
		}
	    } while (!*p || mmarked(*p) || (mcol != omcol && *p == *op));
	    wishcol = mcol;

	    if (wrap == 2)
		goto down;
	} else if (cmd == Th(z_backwardchar) || cmd == Th(z_vibackwardchar)) {
	    int omcol;
	    Cmatch **op;

            mode = 0;
	    wrap = 0;

	left:

	    omcol = mcol;
	    op = p;

	    do {
		if (!mcol) {
		    if (wrap & 1) {
			p = op;
			mcol = omcol;
			break;
		    }
		    mcol = mcols - 1;
		    p += mcol;
		    wrap |= 2;
		} else {
		    mcol--;
		    p--;
		}
	    } while (!*p || mmarked(*p) || (mcol != omcol && *p == *op));
	    wishcol = mcol;

	    if (wrap == 2) {
		p += mcols - 1 - mcol;
		wishcol = mcol = mcols - 1;
		adjust_mcol(wishcol, &p, NULL);
		goto up;
	    }
	} else if (cmd == Th(z_beginningofbufferorhistory) ||
		   cmd == Th(z_beginningofline) ||
		   cmd == Th(z_beginningoflinehist) ||
		   cmd == Th(z_vibeginningofline)) {
            mode = 0;
	    p -= mcol;
	    mcol = 0;
	    while (!*p || mmarked(*p)) {
		mcol++;
		p++;
	    }
	    wishcol = 0;
	} else if (cmd == Th(z_endofbufferorhistory) ||
		   cmd == Th(z_endofline) ||
		   cmd == Th(z_endoflinehist) ||
		   cmd == Th(z_viendofline)) {
            mode = 0;
	    p += mcols - mcol - 1;
	    mcol = mcols - 1;
	    while (!*p || mmarked(*p)) {
		mcol--;
		p--;
	    }
	    wishcol = mcols - 1;
	} else if (cmd == Th(z_viforwardblankword) ||
		   cmd == Th(z_viforwardblankwordend)) {
	    Cmgroup g = *pg;
	    int ol = mline;

            mode = 0;
	    do {
		if (mline == mlines - 1) {
		    p -= mline * mcols;
		    pg -= mline * mcols;
		    mline = 0;
		} else {
		    mline++;
		    p += mcols;
		    pg += mcols;
		}
		if (adjust_mcol(wishcol, &p, &pg))
		    continue;
	    } while (ol != mline && (*pg == g || !*pg || mmarked(*pg)));
	} else if (cmd == Th(z_vibackwardblankword)) {
	    Cmgroup g = *pg;
	    int ol = mline;

            mode = 0;
	    do {
		if (!mline) {
		    mline = mlines - 1;
		    p += mline * mcols;
		    pg += mline * mcols;
		} else {
		    mline--;
		    p -= mcols;
		    pg -= mcols;
		}
		if (adjust_mcol(wishcol, &p, &pg))
		    continue;
	    } while (ol != mline && (*pg == g || !*pg || mmarked(*pg)));
	} else if (cmd == Th(z_completeword) ||
		   cmd == Th(z_expandorcomplete) ||
		   cmd == Th(z_expandorcompleteprefix) ||
		   cmd == Th(z_menucomplete) ||
		   cmd == Th(z_menuexpandorcomplete) ||
		   !strcmp(cmd->nam, "menu-select") ||
		   !strcmp(cmd->nam, "complete-word") ||
		   !strcmp(cmd->nam, "expand-or-complete") ||
		   !strcmp(cmd->nam, "expand-or-complete-prefix") ||
		   !strcmp(cmd->nam, "menu-complete") ||
		   !strcmp(cmd->nam, "menu-expand-or-complete")) {
            if (mode == MM_INTER) {
		/*
		 * do_menucmp() has inserted the completion onto
		 * the command line.  In interactive mode we
		 * don't want that, just what the user typed,
		 * so restore the information.
		 */
		zsfree(origline);
		origline = ztrdup(modeline);
                origcs = modecs;
                origll = modell;
                zlemetacs = 0;
                foredel(zlemetall, CUT_RAW);
                spaceinline(origll);
                strncpy(zlemetaline, origline, origll);
                zlemetacs = origcs;
                minfo.len = modelen;
            } else {
                mode = 0;
                comprecursive = 1;
                do_menucmp(0);
                mselect = (*(minfo.cur))->gnum;
                setwish = 1;
                mline = -1;
            }
	    continue;
	} else if (cmd == Th(z_reversemenucomplete) ||
		   !strcmp(cmd->nam, "reverse-menu-complete")) {
            mode = 0;
	    comprecursive = 1;
	    zmult = -zmult;
	    do_menucmp(0);
	    mselect = (*(minfo.cur))->gnum;
	    setwish = 1;
	    mline = -1;
	    continue;
        } else if (cmd == Th(z_historyincrementalsearchforward) ||
                   cmd == Th(z_historyincrementalsearchbackward) ||
                   ((mode == MM_FSEARCH || mode == MM_BSEARCH) &&
                    (cmd == Th(z_selfinsert) ||
                     cmd == Th(z_selfinsertunmeta) ||
		     cmd == Th(z_bracketedpaste)))) {
            Cmatch **np, **op = p;
            int was = (mode == MM_FSEARCH || mode == MM_BSEARCH);
            int ins = (cmd == Th(z_selfinsert) || cmd == Th(z_selfinsertunmeta) ||
		cmd == Th(z_bracketedpaste));
            int back = (cmd == Th(z_historyincrementalsearchbackward));
            int wrap;

            do {
		char *toins = NULL;
#ifdef MULTIBYTE_SUPPORT
		/* MB_CUR_MAX may not be constant */
		VARARR(char, insert, MB_CUR_MAX+1);
#else
		char insert[2];
#endif
                if (was) {
                    p += wishcol - mcol;
                    mcol = wishcol;
                }
                if (!ins) {
                    if (was) {
                        if (!*msearchstr && lastsearch &&
			    back == (mode == MM_BSEARCH)) {
                            msearchstr = dupstring(lastsearch);
                            mode = 0;
                        }
                    } else {
                        msearchstr = "";
                        msearchstack = NULL;
			msearchstate = MS_OK;
                    }
                } else {
		    if (cmd == Th(z_selfinsertunmeta)) {
			fixunmeta();
		    }
		    if (cmd == Th(z_bracketedpaste)) {
			toins = bracketedstring();
		    } else {
			toins = insert;
#ifdef MULTIBYTE_SUPPORT
			if (lastchar_wide_valid)
			{
			    mbstate_t mbs;
			    int len;

			    memset(&mbs, 0, sizeof(mbs));
			    len = wcrtomb(toins, lastchar_wide, &mbs);
			    if (len < 0)
				len = 0;
			    insert[len] = '\0';
			} else
#endif
			{
			    insert[0] = lastchar;
			    insert[1] = '\0';
			}
		    }
		}
                wrap = 0;
                np = msearch(p, toins, (ins ? (mode == MM_BSEARCH) : back),
                             (was && !ins), &wrap);

                if (!ins)
                    mode = (back ? MM_BSEARCH : MM_FSEARCH);
		else if (cmd == Th(z_bracketedpaste))
		    free(toins);

                if (*msearchstr) {
                    zsfree(lastsearch);
                    lastsearch = ztrdup(msearchstr);
                }
                if (np) {
                    wishcol = mcol;
                    p = np;
                }
                adjust_mcol(wishcol, &p, NULL);

            } while ((back || cmd == Th(z_historyincrementalsearchforward)) &&
                     np && !wrap && was && **p == **op);

        } else if ((mode == MM_FSEARCH || mode == MM_BSEARCH) &&
                   cmd == Th(z_backwarddeletechar)) {
            int back = 1;
            Cmatch **np = msearchpop(&back);

            mode = (back ? MM_BSEARCH : MM_FSEARCH);
            wishcol = mcol;
            if (np) {
                p = np;
                adjust_mcol(wishcol, &p, NULL);
            }
	} else if (cmd == Th(z_undefinedkey)) {
            mode = 0;
	    continue;
	} else {
	    ungetkeycmd();
	    if (cmd->widget && (cmd->widget->flags & WIDGET_NCOMP)) {
		acc = 0;
		broken = 2;
	    } else
		acc = 1;
	    break;
	}
	do_single(**p);
	mselect = (**p)->gnum;
    }
    if (u)
	for (; u; u = u->prev)
	    if (u->lastmatches != lastmatches)
		freematches(u->lastmatches, 0);

    selectlocalmap(NULL);
    mselect = mlastcols = mlastlines = -1;
    mstatus = NULL;
    inselect = mhasstat = 0;
    if (nolist)
        clearlist = listshown = 1;
    if (acc && validlist && minfo.cur) {
	menucmp = lastambig = hasoldlist = 0;
	do_single(*(minfo.cur));
    }
    if (wasnext || broken) {
	menucmp = 1;
	showinglist = ((validlist && !nolist) ? -2 : 0);
	minfo.asked = 0;
	if (!noselect) {
	    int nos = noselect;

	    zrefresh();
	    noselect = nos;
	}
    }
    if (!noselect && (!dat || acc)) {
	/*
	 * I added the following because in certain cases the zrefresh()
	 * here was screwing up the list.  Forcing it to redraw the
	 * screen worked.  The case in question (courtesy of
	 * "Matt Wozniski" <godlygeek@gmail.com>) is in zsh-workers/24756.
	 *
	 * *** PLEASE DON'T ASK ME WHY THIS IS NECESSARY ***
	 */
	mlbeg = -1;
	showinglist = ((validlist && !nolist) ? -2 : 0);
	onlyexpl = oe;
	if (acc && listshown) {
	    /*
	     * Clear the list without spending sixteen weeks of
	     * redrawing it in slightly different states first.
	     * The following seems to work.  I'm not sure what
	     * the difference is between listshown and showinglist,
	     * but listshown looks like the traditional thing to
	     * check for in this file at least.
	     *
	     * showinglist has a normally undocumented value of 1,
	     * and an extra-specially undocumented value of -2, which
	     * seems to be a force---it appears we need to kick it out
	     * of that state, though it worries me that in some places
	     * the code actually forces it back into that state.
	     */
	    clearlist = listshown = showinglist = 1;
	} else if (!smatches)
	    clearlist = listshown = 1;
	zrefresh();
    }
    mlbeg = -1;
    fdat = NULL;

    if (!wasmeta)
	unmetafy_line();

    return (broken == 2 ? 3 :
	    ((dat && !broken) ? (acc ? 1 : 2) : (!noselect ^ acc)));
}

/* The widget function. */

static int
menuselect(char **args)
{
    int d = 0;

    if (!minfo.cur) {
	selected = 0;
	menucomplete(args);
	if ((minfo.cur && minfo.asked == 2) || selected)
	    return 0;
	d = 1;
    }
    if (minfo.cur && (minfo.asked == 2 || domenuselect(NULL, NULL)) && !d)
	menucomplete(args);

    return 0;
}

static struct features module_features = {
    NULL, 0,
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
static void
menuselect_bindings(void)
{
    if (!(mskeymap = openkeymap("menuselect"))) {
	mskeymap = newkeymap(NULL, "menuselect");
	linkkeymap(mskeymap, "menuselect", 1);
	bindkey(mskeymap, "\t", refthingy(t_completeword), NULL);
	bindkey(mskeymap, "\n", refthingy(t_acceptline), NULL);
	bindkey(mskeymap, "\r", refthingy(t_acceptline), NULL);
	bindkey(mskeymap, "\33[A",  refthingy(t_uplineorhistory), NULL);
	bindkey(mskeymap, "\33[B",  refthingy(t_downlineorhistory), NULL);
	bindkey(mskeymap, "\33[C",  refthingy(t_forwardchar), NULL);
	bindkey(mskeymap, "\33[D",  refthingy(t_backwardchar), NULL);
	bindkey(mskeymap, "\33OA",  refthingy(t_uplineorhistory), NULL);
	bindkey(mskeymap, "\33OB",  refthingy(t_downlineorhistory), NULL);
	bindkey(mskeymap, "\33OC",  refthingy(t_forwardchar), NULL);
	bindkey(mskeymap, "\33OD",  refthingy(t_backwardchar), NULL);
    }
    if (!(lskeymap = openkeymap("listscroll"))) {
	lskeymap = newkeymap(NULL, "listscroll");
	linkkeymap(lskeymap, "listscroll", 1);
	bindkey(lskeymap, "\t", refthingy(t_completeword), NULL);
	bindkey(lskeymap, " ", refthingy(t_completeword), NULL);
	bindkey(lskeymap, "\n", refthingy(t_acceptline), NULL);
	bindkey(lskeymap, "\r", refthingy(t_acceptline), NULL);
	bindkey(lskeymap, "\33[B",  refthingy(t_downlineorhistory), NULL);
	bindkey(lskeymap, "\33OB",  refthingy(t_downlineorhistory), NULL);
    }
}

/**/
int
boot_(Module m)
{
    mtab = NULL;
    mgtab = NULL;
    mselect = -1;
    inselect = 0;

    w_menuselect = addzlefunction("menu-select", menuselect,
                                    ZLE_MENUCMP|ZLE_KEEPSUFFIX|ZLE_ISCOMP);
    if (!w_menuselect) {
	zwarnnam(m->node.nam,
		 "name clash when adding ZLE function `menu-select'");
	return -1;
    }
    addhookfunc("comp_list_matches", (Hookfn) complistmatches);
    addhookfunc("menu_start", (Hookfn) domenuselect);
    menuselect_bindings();
    return 0;
}

/**/
int
cleanup_(Module m)
{
    free(mtab);
    free(mgtab);

    deletezlefunction(w_menuselect);
    deletehookfunc("comp_list_matches", (Hookfn) complistmatches);
    deletehookfunc("menu_start", (Hookfn) domenuselect);
    unlinkkeymap("menuselect", 1);
    unlinkkeymap("listscroll", 1);
    return setfeatureenables(m, &module_features, NULL);
}

/**/
int
finish_(UNUSED(Module m))
{
    return 0;
}
