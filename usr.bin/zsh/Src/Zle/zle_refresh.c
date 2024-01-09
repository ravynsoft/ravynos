/*
 * zle_refresh.c - screen update
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

#include "zle.mdh"

#ifdef MULTIBYTE_SUPPORT
/*
 * Handling for glyphs that contain more than one wide character,
 * if ZLE_COMBINING_CHARS is set.  Each glyph is one character with
 * non-zero width followed by an arbitrary (but typically small)
 * number of characters that have zero width (combining characters).
 *
 * The allocated size for each array is given by ?mw_size; nmw_ind
 * is the next free element, i.e. nmwbuf[nmw_ind] will be the next
 * element to be written (we never insert into omwbuf).  We initialise
 * nmw_ind to 1 to avoid the index stored in the character looking like a
 * NULL.  This wastees a word but it's safer than messing with pointers.
 *
 * The layout of the buffer is as a string of entries that consist of multiple
 * elements of the allocated array with no boundary (the code keeps track of
 * where each entry starts).  Note distinction between (logical) entries and
 * (array) elements.  Each entry consists of an element giving the total
 * number of wide characters for the entry (there are N+1 wide characters,
 * where N >= 1 is the number of trailing zero width characters), followed by
 * those characters.
 */
static REFRESH_CHAR
    *omwbuf = NULL,		/* old multiword glyph buffer */
    *nmwbuf = NULL;		/* new multiword glyph buffer */
#endif

/*
 * Compare if two characters are equal.
 */
#ifdef MULTIBYTE_SUPPORT
/*
 * We may need to compare values in multiword arrays.  As the arrays are
 * different for the old and new video arrays, it is vital that the comparison
 * always be done in the correct order: an element of the old video array,
 * followed by an element of the new one.  In this case, having ascertained
 * that both elements are multiword (because they have the some attributes),
 * we do the character comparison in two stages: first we check that the
 * lengths are the same, then we check that the characters stored are the
 * same.  This ensures we can't read past the end of either array.  If either
 * character is a constant, then TXT_MULTIWORD_MASK is guaranteed not to be
 * set and this doesn't matter.
 */
#define ZR_equal(oldzr, newzr)					   \
    ((oldzr).atr == (newzr).atr &&				   \
     (((oldzr).atr & TXT_MULTIWORD_MASK) ?			   \
      (omwbuf[(oldzr).chr] == nmwbuf[(newzr).chr] &&		   \
       !memcmp(omwbuf + (oldzr).chr + 1, nmwbuf + (newzr).chr + 1, \
	       omwbuf[(oldzr).chr] * sizeof(*omwbuf))) :	   \
      (oldzr).chr == (newzr).chr))
#else
#define ZR_equal(zr1, zr2) ((zr1).chr == (zr2).chr && (zr1).atr == (zr2).atr)
#endif

static void
ZR_memset(REFRESH_ELEMENT *dst, REFRESH_ELEMENT rc, int len)
{
    while (len--)
	*dst++ = rc;
}

#define ZR_memcpy(d, s, l)  memcpy((d), (s), (l)*sizeof(REFRESH_ELEMENT))

static void
ZR_strcpy(REFRESH_ELEMENT *dst, const REFRESH_ELEMENT *src)
{
    while ((*dst++ = *src++).chr != ZWC('\0'))
	;
}

static size_t
ZR_strlen(const REFRESH_ELEMENT *wstr)
{
    int len = 0;

    while (wstr++->chr != ZWC('\0'))
	len++;

    return len;
}

/*
 * Simplified strcmp: we don't need the sign, just whether
 * the strings and their attributes are equal.
 *
 * In the multibyte case, the two elements must be in the order
 * element from old video array, element from new video array.
 */
static int
ZR_strncmp(const REFRESH_ELEMENT *oldwstr, const REFRESH_ELEMENT *newwstr,
	   int len)
{
    while (len--) {
	if ((!(oldwstr->atr & TXT_MULTIWORD_MASK) && !oldwstr->chr) ||
	    (!(newwstr->atr & TXT_MULTIWORD_MASK) && !newwstr->chr))
	    return !ZR_equal(*oldwstr, *newwstr);
	if (!ZR_equal(*oldwstr, *newwstr))
	    return 1;
	oldwstr++;
	newwstr++;
    }

    return 0;
}

#include "zle_refresh.pro"

/*
 * Expanded prompts.
 *
 * These are always output from the start, except in the special
 * case where we are sure each character in the prompt corresponds
 * to a character on screen.
 */

/**/
char *lpromptbuf, *rpromptbuf;

/* Text attributes after displaying prompts */

/**/
zattr pmpt_attr, rpmpt_attr;

/* number of lines displayed */

/**/
mod_export int nlnct;

/* Most lines of the buffer we've shown at once with the current list *
 * showing.  == 0 if there is no list.  == -1 if a new list has just  *
 * been put on the screen.  == -2 if zrefresh() needs to put up a new *
 * list.                                                              */

/**/
mod_export int showinglist;

/* > 0 if a completion list is displayed below the prompt,
 * < 0 if a list is displayed above the prompt. */

/**/
mod_export int listshown;

/* Length of last list displayed (if it is below the prompt). */

/**/
mod_export int lastlistlen;

/* Non-zero if ALWAYS_LAST_PROMPT has been used, meaning that the *
 * screen below the buffer display should not be cleared by       *
 * zrefresh(), but should be by trashzle().                       */

/**/
mod_export int clearflag;

/* Non-zero if zrefresh() should clear the list below the prompt. */

/**/
mod_export int clearlist;

/* Zle in trashed state - updates may be subtly altered */

/**/
int trashedzle;

/*
 * Information used by PREDISPLAY and POSTDISPLAY parameters which
 * add non-editable text to that being displayed.
 */
/**/
ZLE_STRING_T predisplay, postdisplay;
/**/
int predisplaylen, postdisplaylen;


/*
 * Attributes used by default on the command line, and
 * attributes for highlighting special (unprintable) characters
 * displayed on screen.
 */

static zattr default_atr_on, special_atr_on;

/*
 * Array of region highlights, no special termination.
 * The first N_SPECIAL_HIGHLIGHTS elements describe special uses of
 * highlighting, documented under N_SPECIAL_HIGHLIGHTS.
 * Any other elements are set by the user via the parameter region_highlight.
 */

/**/
struct region_highlight *region_highlights;

/*
 * Number of elements in region_highlights.
 * This includes the special elements above.
 */
/**/
int n_region_highlights;

/*
 * Flag that highlighting of the region is active.
 */
/**/
int region_active;

/*
 * Name of function to use to output termcap values, if defined.
 */
/**/
char *tcout_func_name;

#ifdef HAVE_SELECT
/* cost of last update */
/**/
int cost;

# define SELECT_ADD_COST(X)	(cost += X)
# define zputc(a)		(zwcputc(a, NULL), cost++)
# define zwrite(a, b)		(zwcwrite((a), (b)), \
				 cost += ((b) * ZLE_CHAR_SIZE))
#else
# define SELECT_ADD_COST(X)
# define zputc(a)		zwcputc(a, NULL)
# define zwrite(a, b)		zwcwrite((a), (b))
#endif

static const REFRESH_ELEMENT zr_cr = { ZWC('\r'), 0 };
#ifdef MULTIBYTE_SUPPORT
static const REFRESH_ELEMENT zr_dt = { ZWC('.'), 0 };
#endif
static const REFRESH_ELEMENT zr_nl = { ZWC('\n'), 0 };
static const REFRESH_ELEMENT zr_sp = { ZWC(' '), 0 };
static const REFRESH_ELEMENT zr_zr = { ZWC('\0'), 0 };

/*
 * Constant arrays to be copied into place: these are memcpy'd,
 * so don't have terminating NULLs.
 */
static const REFRESH_ELEMENT zr_end_ellipsis[] = {
    { ZWC(' '), 0 },
    { ZWC('<'), 0 },
    { ZWC('.'), 0 },
    { ZWC('.'), 0 },
    { ZWC('.'), 0 },
    { ZWC('.'), 0 },
    { ZWC(' '), 0 },
};
#define ZR_END_ELLIPSIS_SIZE	\
    ((int)(sizeof(zr_end_ellipsis)/sizeof(zr_end_ellipsis[0])))

static const REFRESH_ELEMENT zr_mid_ellipsis1[] = {
    { ZWC(' '), 0 },
    { ZWC('<'), 0 },
    { ZWC('.'), 0 },
    { ZWC('.'), 0 },
    { ZWC('.'), 0 },
    { ZWC('.'), 0 },
};
#define ZR_MID_ELLIPSIS1_SIZE	\
    ((int)(sizeof(zr_mid_ellipsis1)/sizeof(zr_mid_ellipsis1[0])))

static const REFRESH_ELEMENT zr_mid_ellipsis2[] = {
    { ZWC('>'), 0 },
    { ZWC(' '), 0 },
};
#define ZR_MID_ELLIPSIS2_SIZE	\
    ((int)(sizeof(zr_mid_ellipsis2)/sizeof(zr_mid_ellipsis2[0])))

static const REFRESH_ELEMENT zr_start_ellipsis[] = {
    { ZWC('>'), 0 },
    { ZWC('.'), 0 },
    { ZWC('.'), 0 },
    { ZWC('.'), 0 },
    { ZWC('.'), 0 },
};
#define ZR_START_ELLIPSIS_SIZE	\
    ((int)(sizeof(zr_start_ellipsis)/sizeof(zr_start_ellipsis[0])))

/*
 * Parse the variable zle_highlight to decide how to highlight characters
 * and regions.  Set defaults for anything not explicitly covered.
 */

/**/
static void
zle_set_highlight(void)
{
    char **atrs = getaparam("zle_highlight");
    int special_atr_on_set = 0;
    int region_atr_on_set = 0;
    int isearch_atr_on_set = 0;
    int suffix_atr_on_set = 0;
    int paste_atr_on_set = 0;
    struct region_highlight *rhp;

    special_atr_on = default_atr_on = 0;
    if (!region_highlights) {
	region_highlights = (struct region_highlight *)
	    zshcalloc(N_SPECIAL_HIGHLIGHTS*sizeof(struct region_highlight));
	n_region_highlights = N_SPECIAL_HIGHLIGHTS;
    } else {
	for (rhp = region_highlights;
	     rhp < region_highlights + N_SPECIAL_HIGHLIGHTS;
	     rhp++) {
	    rhp->atr = 0;
	}
    }

    if (atrs) {
	for (; *atrs; atrs++) {
	    if (!strcmp(*atrs, "none")) {
		/* reset attributes for consistency... usually unnecessary */
		special_atr_on = default_atr_on = 0;
		special_atr_on_set = 1;
		paste_atr_on_set = region_atr_on_set =
		    isearch_atr_on_set = suffix_atr_on_set = 1;
	    } else if (strpfx("default:", *atrs)) {
		match_highlight(*atrs + 8, &default_atr_on);
	    } else if (strpfx("special:", *atrs)) {
		match_highlight(*atrs + 8, &special_atr_on);
		special_atr_on_set = 1;
	    } else if (strpfx("region:", *atrs)) {
		match_highlight(*atrs + 7, &region_highlights[0].atr);
		region_atr_on_set = 1;
	    } else if (strpfx("isearch:", *atrs)) {
		match_highlight(*atrs + 8, &(region_highlights[1].atr));
		isearch_atr_on_set = 1;
	    } else if (strpfx("suffix:", *atrs)) {
		match_highlight(*atrs + 7, &(region_highlights[2].atr));
		suffix_atr_on_set = 1;
	    } else if (strpfx("paste:", *atrs)) {
		match_highlight(*atrs + 6, &(region_highlights[3].atr));
		paste_atr_on_set = 1;
	    }
	}
    }

    /* Defaults */
    if (!special_atr_on_set)
	special_atr_on = TXTSTANDOUT;
    if (!region_atr_on_set)
	region_highlights[0].atr = TXTSTANDOUT;
    if (!isearch_atr_on_set)
	region_highlights[1].atr = TXTUNDERLINE;
    if (!suffix_atr_on_set)
	region_highlights[2].atr = TXTBOLDFACE;
    if (!paste_atr_on_set)
	region_highlights[3].atr = TXTSTANDOUT;

    allocate_colour_buffer();
}


/**/
static void
zle_free_highlight(void)
{
    free_colour_buffer();
}

/*
 * Interface to the region_highlight ZLE parameter.
 * Converts between a format like "P32 42 underline,bold" to
 * the format in the region_highlights variable.  Note that
 * the region_highlights variable stores the internal (point/mark)
 * region in element zero.
 */

/**/
char **
get_region_highlight(UNUSED(Param pm))
{
    int arrsize = n_region_highlights;
    char **retarr, **arrp;
    struct region_highlight *rhp;

    /* region_highlights may not have been set yet */
    if (!arrsize)
	return hmkarray(NULL);
    arrsize -= N_SPECIAL_HIGHLIGHTS;
    DPUTS(arrsize < 0, "arrsize is negative from n_region_highlights");
    arrp = retarr = (char **)zhalloc((arrsize+1)*sizeof(char *));

    /* ignore special highlighting */
    for (rhp = region_highlights + N_SPECIAL_HIGHLIGHTS;
	 arrsize--;
	 rhp++, arrp++) {
	char digbuf1[DIGBUFSIZE], digbuf2[DIGBUFSIZE];
	int atrlen, alloclen;
	const char memo_equals[] = "memo=";

	sprintf(digbuf1, "%d", rhp->start);
	sprintf(digbuf2, "%d", rhp->end);

	atrlen = output_highlight(rhp->atr, NULL);
	alloclen = atrlen + strlen(digbuf1) + strlen(digbuf2) +
	    3; /* 2 spaces, 1 terminating NUL */
	if (rhp->flags & ZRH_PREDISPLAY)
	    alloclen += 2; /* "P " */
	if (rhp->memo)
	    alloclen += 1 /* space */ + strlen(memo_equals) + strlen(rhp->memo);
	*arrp = (char *)zhalloc(alloclen * sizeof(char));
	/*
	 * On input we allow a space after the flags.
	 * I haven't put a space here because I think it's
	 * marginally easier to have the output always split
	 * into three words, and then check the first to
	 * see if there are flags.  However, it's arguable.
	 */
	sprintf(*arrp, "%s%s %s ",
		(rhp->flags & ZRH_PREDISPLAY) ? "P" : "",
		digbuf1, digbuf2);
	(void)output_highlight(rhp->atr, *arrp + strlen(*arrp));

	if (rhp->memo) {
	    strcat(*arrp, " ");
	    strcat(*arrp, memo_equals);
	    strcat(*arrp, rhp->memo);
	}
    }
    *arrp = NULL;
    return retarr;
}


/*
 * The parameter system requires the pm argument, but this
 * may be NULL if called directly.
 */

/**/
void
set_region_highlight(UNUSED(Param pm), char **aval)
{
    int len;
    char **av = aval;
    struct region_highlight *rhp;

    len = aval ? arrlen(aval) : 0;
    if (n_region_highlights != len + N_SPECIAL_HIGHLIGHTS) {
	/* no null termination, but include special highlighting at start */
	int newsize = len + N_SPECIAL_HIGHLIGHTS;
	int diffsize = newsize - n_region_highlights;

	free_region_highlights_memos();
	region_highlights = (struct region_highlight *)
	    zrealloc(region_highlights,
		     sizeof(struct region_highlight) * newsize);
	if (diffsize > 0)
	    memset(region_highlights + newsize - diffsize, 0,
		   sizeof(struct region_highlight) * diffsize);
	n_region_highlights = newsize;
    }

    if (!aval)
	return;

    for (rhp = region_highlights + N_SPECIAL_HIGHLIGHTS;
	 *aval;
	 rhp++, aval++) {
	char *strp, *oldstrp;
	const char memo_equals[] = "memo=";

	oldstrp = *aval;
	if (*oldstrp == 'P') {
	    rhp->flags = ZRH_PREDISPLAY;
	    oldstrp++;
	}
	else
	    rhp->flags = 0;
	while (inblank(*oldstrp))
	    oldstrp++;

	rhp->start = (int)zstrtol(oldstrp, &strp, 10);
	if (strp == oldstrp)
	    rhp->start = -1;

	while (inblank(*strp))
	    strp++;

	oldstrp = strp;
	rhp->end = (int)zstrtol(strp, &strp, 10);
	if (strp == oldstrp)
	    rhp->end = -1;

	while (inblank(*strp))
	    strp++;

	strp = (char*) match_highlight(strp, &rhp->atr);

	while (inblank(*strp))
	    strp++;

	if (strpfx(memo_equals, strp)) {
	    const char *memo_start = strp + strlen(memo_equals);
	    const char *i, *memo_end;

	    /* 
	     * Forward compatibility: end parsing at a comma or whitespace to
	     * allow the following extensions:
	     *
	     * - A fifth field: "0 20 bold memo=foo bar".
	     *
	     * - Additional attributes in the fourth field: "0 20 bold memo=foo,bar"
	     *   and "0 20 bold memo=foo\0bar".
	     *
	     * For similar reasons, we don't flag an error if the fourth field
	     * doesn't start with "memo=" as we expect.
	     */
	    i = memo_start;

	    /* ### TODO: Consider optimizing the common case that memo_start to
	     *           end-of-string is entirely ASCII */
	    while (1) {
		int nbytes;
		convchar_t c = unmeta_one(i, &nbytes);

		if (c == '\0' || c == ',' || inblank(c)) {
		    memo_end = i;
		    break;
		} else
		    i += nbytes;
	    }
	    rhp->memo = ztrduppfx(memo_start, memo_end - memo_start);
	} else
	    rhp->memo = NULL;
    }

    freearray(av);
}


/**/
void
unset_region_highlight(Param pm, int exp)
{
    if (exp) {
	set_region_highlight(pm, NULL);
	stdunsetfn(pm, exp);
    }
}


/* The last attributes that were on. */
static zattr lastatr;

/*
 * Clear the last attributes that we set:  used when we're going
 * to be outputting stuff that shouldn't show up as text.
 */
static void
clearattributes(void)
{
    if (lastatr) {
	settextattributes(TXT_ATTR_OFF_FROM_ON(lastatr));
	lastatr = 0;
    }
}

/*
 * Output a termcap capability, clearing any text attributes so
 * as not to mess up the display.
 */

static void
tcoutclear(int cap)
{
    clearattributes();
    tcout(cap);
}

/*
 * Output the character.  This must come from the new video
 * buffer, nbuf, since we access the multiword buffer nmwbuf
 * directly.
 *
 * curatrp may be NULL, otherwise points to an integer specifying
 * what attributes were turned on for a character output immediately
 * before, in order to optimise output of attribute changes.
 */

/**/
void
zwcputc(const REFRESH_ELEMENT *c, zattr *curatrp)
{
    /*
     * Safety: turn attributes off if last heard of turned on.
     * This differs from *curatrp, which is an optimisation for
     * writing lots of stuff at once.
     */
#ifdef MULTIBYTE_SUPPORT
    mbstate_t mbstate;
    int i;
    VARARR(char, mbtmp, MB_CUR_MAX + 1);
#endif

    if (lastatr & ~c->atr) {
	/* Stuff on we don't want, turn it off */
	settextattributes(TXT_ATTR_OFF_FROM_ON(lastatr & ~c->atr));
	lastatr = 0;
    }

    /*
     * Don't output "on" attributes in a string of characters with
     * the same attributes.  Be careful in case a different colour
     * needs setting.
     */
    if ((c->atr & TXT_ATTR_ON_MASK) &&
	(!curatrp ||
	 ((*curatrp & TXT_ATTR_ON_VALUES_MASK) !=
	  (c->atr & TXT_ATTR_ON_VALUES_MASK)))) {
	/* Record just the control flags we might need to turn off... */
	lastatr = c->atr & TXT_ATTR_ON_MASK;
	/* ...but set including the values for colour attributes */
	settextattributes(c->atr & TXT_ATTR_ON_VALUES_MASK);
    }

#ifdef MULTIBYTE_SUPPORT
    if (c->atr & TXT_MULTIWORD_MASK) {
	/* Multiword glyph stored in nmwbuf */
	int nchars = nmwbuf[c->chr];
	REFRESH_CHAR *wcptr = nmwbuf + c->chr + 1;

	memset(&mbstate, 0, sizeof(mbstate_t));
	while (nchars--) {
	    if ((i = wcrtomb(mbtmp, (wchar_t)*wcptr++, &mbstate)) > 0)
		fwrite(mbtmp, i, 1, shout);
	}
    } else if (c->chr != WEOF) {
	memset(&mbstate, 0, sizeof(mbstate_t));
	if ((i = wcrtomb(mbtmp, (wchar_t)c->chr, &mbstate)) > 0)
	    fwrite(mbtmp, i, 1, shout);
    }
#else
    fputc(c->chr, shout);
#endif

    /*
     * Always output "off" attributes since we only turn off at
     * the end of a chunk of highlighted text.
     */
    if (c->atr & TXT_ATTR_OFF_MASK) {
	settextattributes(c->atr & TXT_ATTR_OFF_MASK);
	lastatr &= ~((c->atr & TXT_ATTR_OFF_MASK) >> TXT_ATTR_OFF_ON_SHIFT);
    }
    if (curatrp) {
	/*
	 * Remember the current attributes:  those that are turned
	 * on, less those that are turned off again.  Include
	 * colour attributes here in case the colour changes to
	 * another non-default one.
	 */
	*curatrp = (c->atr & TXT_ATTR_ON_VALUES_MASK) &
	    ~((c->atr & TXT_ATTR_OFF_MASK) >> TXT_ATTR_OFF_ON_SHIFT);
    }
}

static int
zwcwrite(const REFRESH_STRING s, size_t i)
{
    size_t j;
    zattr curatr = 0;

    for (j = 0; j < i; j++)
	zwcputc(s + j, &curatr);
    return i; /* TODO something better for error indication */
}

/* Oct/Nov 94: <mason> some code savagely redesigned to fix several bugs -
   refreshline() & tc_rightcurs() majorly rewritten; zrefresh() fixed -
   I've put my fingers into just about every routine in here -
   any queries about updates to mason@primenet.com.au */

static REFRESH_STRING 
    *nbuf = NULL,		/* new video buffer line-by-line array */
    *obuf = NULL;		/* old video buffer line-by-line array */
static int more_start,		/* more text before start of screen?	    */
    more_end,			/* more stuff after end of screen?	    */
    olnct,			/* previous number of lines		    */
    ovln,			/* previous video cursor position line	    */
    lpromptw, rpromptw,		/* prompt widths on screen                  */
    lpromptwof,			/* left prompt width with real end position */
    lprompth,			/* lines taken up by the prompt		    */
    rprompth,			/* right prompt height                      */
    vcs, vln,			/* video cursor position column & line	    */
    vmaxln,			/* video maximum number of lines	    */
    winw, winh, rwinh,		/* window width & height		    */
    winpos,			/* singlelinezle: line's position in window */
    winprompt,			/* singlelinezle: part of lprompt showing   */
    winw_alloc = -1,		/* allocated window width */
    winh_alloc = -1;		/* allocates window height */
#ifdef MULTIBYTE_SUPPORT
static int
    omw_size,			/* allocated size of omwbuf */
    nmw_size,			/* allocated size of nmwbuf */
    nmw_ind;			/* next insert point in nmw_ind */
#endif

/*
 * Number of words to allocate in one go for the multiword buffers.
 */
#define DEF_MWBUF_ALLOC	(32)

static void
freevideo(void)
{
    if (nbuf) {
	int ln;
	for (ln = 0; ln != winh_alloc; ln++) {
	    zfree(nbuf[ln], (winw_alloc + 2) * sizeof(**nbuf));
	    zfree(obuf[ln], (winw_alloc + 2) * sizeof(**obuf));
	}
	free(nbuf);
	free(obuf);
#ifdef MULTIBYTE_SUPPORT
	zfree(nmwbuf, nmw_size * sizeof(*nmwbuf));
	zfree(omwbuf, omw_size * sizeof(*omwbuf));
	omw_size = nmw_size = 0;
	nmw_ind = 1;
#endif
	nbuf = NULL;
	obuf = NULL;
	winw_alloc = -1;
	winh_alloc = -1;
    }
}

/**/
void
resetvideo(void)
{
    int ln;
 
    winw = zterm_columns;  /* terminal width */
    if (termflags & TERM_SHORT)
	winh = 1;
    else
	winh = (zterm_lines < 2) ? 24 : zterm_lines;
    rwinh = zterm_lines;		/* keep the real number of lines */
    vln = vmaxln = winprompt = 0;
    winpos = -1;
    if (winw_alloc != winw || winh_alloc != winh) {
	freevideo();
	nbuf = (REFRESH_STRING *)zshcalloc((winh + 1) * sizeof(*nbuf));
	obuf = (REFRESH_STRING *)zshcalloc((winh + 1) * sizeof(*obuf));
	nbuf[0] = (REFRESH_STRING)zalloc((winw + 2) * sizeof(**nbuf));
	obuf[0] = (REFRESH_STRING)zalloc((winw + 2) * sizeof(**obuf));

#ifdef MULTIBYTE_SUPPORT
	nmw_size = DEF_MWBUF_ALLOC;
	nmw_ind = 1;
	nmwbuf = (REFRESH_CHAR *)zalloc(nmw_size * sizeof(*nmwbuf));

	omw_size = DEF_MWBUF_ALLOC;
	omwbuf = (REFRESH_CHAR *)zalloc(omw_size * sizeof(*omwbuf));
#endif

	winw_alloc = winw;
	winh_alloc = winh;
    }
    for (ln = 0; ln != winh + 1; ln++) {
	if (nbuf[ln]) {
	    nbuf[ln][0] = zr_nl;
	    nbuf[ln][1] = zr_zr;
	}
	if (obuf[ln]) {
	    obuf[ln][0] = zr_nl;
	    obuf[ln][1] = zr_zr;
	}
    }

    /*
     * countprompt() now correctly handles multibyte input.
     */
    countprompt(lpromptbuf, &lpromptwof, &lprompth, 1);
    countprompt(rpromptbuf, &rpromptw, &rprompth, 0);
    if (lpromptwof != winw)
	lpromptw = lpromptwof;
    else {
	lpromptw = 0;
	lprompth++;
    }

    if (lpromptw) {
    	ZR_memset(nbuf[0], zr_sp, lpromptw);
	ZR_memset(obuf[0], zr_sp, lpromptw);
	nbuf[0][lpromptw] = obuf[0][lpromptw] = zr_zr;
    }

    vcs = lpromptw;
    olnct = nlnct = 0;
    if (showinglist > 0)
	showinglist = -2;
    trashedzle = 0;
}

/*
 * Nov 96: <mason> changed to single line scroll
 */

/**/
static void
scrollwindow(int tline)
{
    int t0;
    REFRESH_STRING s;

    s = nbuf[tline];
    for (t0 = tline; t0 < winh - 1; t0++)
	nbuf[t0] = nbuf[t0 + 1];
    nbuf[winh - 1] = s;
    if (!tline)
	more_start = 1;
    return;
}

/*
 * Parameters in zrefresh used for communicating with next-line functions.
 */
struct rparams {
    int canscroll;		/* number of lines we are allowed to scroll */
    int ln;			/* current line we're working on */
    int more_status;		/* more stuff in status line */
    int nvcs;			/* video cursor column */
    int nvln;			/* video cursor line */
    int tosln;			/* tmp in statusline stuff */
    REFRESH_STRING s;		/* pointer into the video buffer */
    REFRESH_STRING sen;		/* pointer to end of the video buffer (eol) */
};
typedef struct rparams *Rparams;

static int cleareol,		/* clear to end-of-line (if can't cleareod) */
    clearf,			/* alwayslastprompt used immediately before */
    put_rpmpt,			/* whether we should display right-prompt   */
    oput_rpmpt,			/* whether displayed right-prompt last time */
    oxtabs,			/* oxtabs - tabs expand to spaces if set    */
    numscrolls, onumscrolls;

/*
 * Go to the next line in the main display area.  Return 1 if we should abort
 * processing the line loop at this point, else 0.
 *
 * If wrapped is non-zero, text wrapped, so output newline.
 * Otherwise, text not wrapped, so output null.
 */
static int
nextline(Rparams rpms, int wrapped)
{
    nbuf[rpms->ln][winw+1] = wrapped ? zr_nl : zr_zr;
    *rpms->s = zr_zr;
    if (rpms->ln != winh - 1)
	rpms->ln++;
    else {
	if (!rpms->canscroll)	{
	    if (rpms->nvln != -1 && rpms->nvln != winh - 1
		&& (numscrolls != onumscrolls - 1
		    || rpms->nvln <= winh / 2))
		return 1;
	    numscrolls++;
	    rpms->canscroll = winh / 2;
	}
	rpms->canscroll--;
	scrollwindow(0);
	if (rpms->nvln != -1)
	    rpms->nvln--;
    }
    if (!nbuf[rpms->ln])
	nbuf[rpms->ln] = (REFRESH_STRING)zalloc((winw + 2) * sizeof(**nbuf));
    rpms->s = nbuf[rpms->ln];
    rpms->sen = rpms->s + winw;

    return 0;
}


/*
 * Go to the next line in the status area.
 */
static void
snextline(Rparams rpms)
{
    *rpms->s = zr_zr;
    if (rpms->ln != winh - 1)
	rpms->ln++;
    else
	if (rpms->tosln > rpms->ln) {
	    rpms->tosln--;
	    if (rpms->nvln > 1) {
		scrollwindow(0);
		rpms->nvln--;
	    } else
		more_end = 1;
	} else if (rpms->tosln > 2 && rpms->nvln > 1) {
	    rpms->tosln--;
	    if (rpms->tosln <= rpms->nvln) {
		scrollwindow(0);
		rpms->nvln--;
	    } else {
		scrollwindow(rpms->tosln);
		more_end = 1;
	    }
	} else {
	    rpms->more_status = 1;
	    scrollwindow(rpms->tosln + 1);
	}
    if (!nbuf[rpms->ln])
	nbuf[rpms->ln] = (REFRESH_STRING)zalloc((winw + 2) * sizeof(**nbuf));
    rpms->s = nbuf[rpms->ln];
    rpms->sen = rpms->s + winw;
}


/**/
static void
settextattributes(zattr atr)
{
    if (txtchangeisset(atr, TXTNOBOLDFACE))
	tsetcap(TCALLATTRSOFF, 0);
    if (txtchangeisset(atr, TXTNOSTANDOUT))
	tsetcap(TCSTANDOUTEND, 0);
    if (txtchangeisset(atr, TXTNOUNDERLINE))
	tsetcap(TCUNDERLINEEND, 0);
    if (txtchangeisset(atr, TXTBOLDFACE))
	tsetcap(TCBOLDFACEBEG, 0);
    if (txtchangeisset(atr, TXTSTANDOUT))
	tsetcap(TCSTANDOUTBEG, 0);
    if (txtchangeisset(atr, TXTUNDERLINE))
	tsetcap(TCUNDERLINEBEG, 0);
    if (txtchangeisset(atr, TXTFGCOLOUR|TXTNOFGCOLOUR))
	set_colour_attribute(atr, COL_SEQ_FG, 0);
    if (txtchangeisset(atr, TXTBGCOLOUR|TXTNOBGCOLOUR))
	set_colour_attribute(atr, COL_SEQ_BG, 0);
}

#ifdef MULTIBYTE_SUPPORT
/*
 * Add a multiword glyph at the screen location base.
 * tptr points to the source and there are ichars characters.
 */
static void
addmultiword(REFRESH_ELEMENT *base, ZLE_STRING_T tptr, int ichars)
{
    /* Number of characters needed in buffer incl. count */
    int iadd = ichars + 1, icnt;
    REFRESH_CHAR *nmwptr;
    base->atr |= TXT_MULTIWORD_MASK;
    /* check allocation */
    if (nmw_ind + iadd > nmw_size) {
	/* need more space in buffer */
	int mw_more = (iadd > DEF_MWBUF_ALLOC) ? iadd :
	    DEF_MWBUF_ALLOC;
	nmwbuf = (REFRESH_CHAR *)
	    zrealloc(nmwbuf, (nmw_size += mw_more) *
		     sizeof(*nmwbuf));
    }
    /* make buffer entry: count, then characters */
    nmwptr = nmwbuf + nmw_ind;
    *nmwptr++ = ichars;
    for (icnt = 0; icnt < ichars; icnt++)
	*nmwptr++ = tptr[icnt];
    /* save index and update */
    base->chr = (wint_t)nmw_ind;
    nmw_ind += iadd;
}
#endif


/*
 * Swap the old and new video buffers, plus any associated multiword
 * buffers.  The new buffer becomes the old one; the new buffer
 * will be filled with the command line next time.
 */
static void
bufswap(void)
{
    REFRESH_STRING	*qbuf;
#ifdef MULTIBYTE_SUPPORT
    REFRESH_CHAR *qmwbuf;
    int itmp;
#endif

    qbuf = nbuf;
    nbuf = obuf;
    obuf = qbuf;

#ifdef MULTIBYTE_SUPPORT
/* likewise multiword buffers */
    qmwbuf = nmwbuf;
    nmwbuf = omwbuf;
    omwbuf = qmwbuf;

    itmp = nmw_size;
    nmw_size = omw_size;
    omw_size = itmp;

    nmw_ind = 1;
#endif
}


/**/
mod_export void
zrefresh(void)
{
    static int inlist;		/* avoiding recursion			     */
    int iln;			/* current line as index in loops	     */
    int t0 = -1;		/* tmp					     */
    ZLE_STRING_T tmpline,	/* line with added pre/post text	     */
	t,			/* pointer into the real buffer		     */
	scs,			/* pointer to cursor position in real buffer */
	u;			/* pointer for status line stuff	     */
    int tmpcs, tmpll;		/* ditto cursor position and line length     */
    int tmppos;			/* t - tmpline				     */
    int tmpalloced;		/* flag to free tmpline when finished	     */
    int remetafy;		/* flag that zle line is metafied	     */
    zattr txtchange;		/* attributes set after prompts              */
    int rprompt_off = 1;	/* Offset of rprompt from right of screen    */
    struct rparams rpms;
#ifdef MULTIBYTE_SUPPORT
    int width;			/* width of wide character		     */
#endif


    /* If this is called from listmatches() (indirectly via trashzle()), and *
     * that was called from the end of zrefresh(), then we don't need to do  *
     * anything.  All this `inlist' code is actually unnecessary, but it     *
     * improves speed a little in a common case.                             */
    if (inlist)
	return;

    /*
     * zrefresh() is called from all over the place, so we can't
     * be sure if the line is metafied for completion or not.
     */
    if (zlemetaline != NULL) {
	remetafy = 1;
	unmetafy_line();
    }
    else
	remetafy = 0;

    if (predisplaylen || postdisplaylen) {
	/* There is extra text to display at the start or end of the line */
	tmpline = zalloc((zlell + predisplaylen + postdisplaylen)*sizeof(*tmpline));
	if (predisplaylen)
	    ZS_memcpy(tmpline, predisplay, predisplaylen);
	if (zlell)
	    ZS_memcpy(tmpline+predisplaylen, zleline, zlell);
	if (postdisplaylen)
	    ZS_memcpy(tmpline+predisplaylen+zlell, postdisplay,
		      postdisplaylen);

	tmpcs = zlecs + predisplaylen;
	tmpll = predisplaylen + zlell + postdisplaylen;
	tmpalloced = 1;
    } else {
	tmpline = zleline;
	tmpcs = zlecs;
	tmpll = zlell;
	tmpalloced = 0;
    }

    /* this will create region_highlights if it's still NULL */
    zle_set_highlight();

    DPUTS(!region_highlights, "region_highlights not created");

    /* check for region between point ($CURSOR) and mark ($MARK) */
    if (region_active) {
	if (zlecs <= mark) {
	    region_highlights[0].start = zlecs;
	    region_highlights[0].end = mark;
	} else {
	    region_highlights[0].start = mark;
	    region_highlights[0].end = zlecs;
	}
	if (region_active == 2) {
	    int origcs = zlecs;
	    zlecs = region_highlights[0].end;
	    region_highlights[0].end = findeol();
	    zlecs = region_highlights[0].start;
	    region_highlights[0].start = findbol();
	    zlecs = origcs;
	} else if (invicmdmode())
	    INCPOS(region_highlights[0].end);
    } else {
	region_highlights[0].start = region_highlights[0].end = -1;
    }
    /* check for isearch string to highlight */
    if (isearch_active) {
	region_highlights[1].start = isearch_startpos;
	region_highlights[1].end = isearch_endpos;
    } else {
	region_highlights[1].start = region_highlights[1].end = -1;
    }
    /* check for an active completion suffix */
    if (suffixlen) {
	region_highlights[2].start = zlecs - suffixlen;
	region_highlights[2].end = zlecs;
    } else {
	region_highlights[2].start = region_highlights[2].end = -1;
    }

    if (lastcmd & ZLE_YANK) {
	region_highlights[3].start = yankb;
	region_highlights[3].end = yanke;
    } else {
	region_highlights[3].start = region_highlights[3].end = -1;
    }

    if (clearlist && listshown > 0) {
	if (tccan(TCCLEAREOD)) {
	    int ovln = vln, ovcs = vcs;
	    REFRESH_STRING nb = nbuf[vln];

	    nbuf[vln] = obuf[vln];
	    moveto(nlnct, 0);
	    tcoutclear(TCCLEAREOD);
	    moveto(ovln, ovcs);
	    nbuf[vln] = nb;
	} else {
	    invalidatelist();
	    moveto(0, 0);
	    clearflag = 0;
	    resetneeded = 1;
	}
	listshown = lastlistlen = 0;
	if (showinglist != -2)
	    showinglist = 0;
    }
    clearlist = 0;

#ifdef HAVE_SELECT
    cost = 0;			/* reset */
#endif

/* Nov 96: <mason>  I haven't checked how complete this is.  sgtty stuff may
   or may not work */
#if defined(SGTABTYPE)
    oxtabs = ((SGTTYFLAG & SGTABTYPE) == SGTABTYPE);
#else
    oxtabs = 0;
#endif

    cleareol = 0;		/* unset */
    more_start = more_end = 0;	/* unset */
    if (isset(SINGLELINEZLE) || zterm_lines < 3
	|| (termflags & (TERM_NOUP | TERM_BAD | TERM_UNKNOWN)))
	termflags |= TERM_SHORT;
    else
	termflags &= ~TERM_SHORT;
    if (resetneeded) {
	onumscrolls = 0;
	zsetterm();
#ifdef TIOCGWINSZ
	if (winchanged) {
	    moveto(0, 0);
	    t0 = olnct;		/* this is to clear extra lines even when */
	    winchanged = 0;	/* the terminal cannot TCCLEAREOD	  */
	    listshown = 0;
	}
#endif
	/* we probably should only have explicitly set attributes */
	tsetcap(TCALLATTRSOFF, 0);
	tsetcap(TCSTANDOUTEND, 0);
	tsetcap(TCUNDERLINEEND, 0);
	txtattrmask = 0;

	if (trashedzle && !clearflag)
	    reexpandprompt(); 
	resetvideo();
	resetneeded = 0;	/* unset */
	oput_rpmpt = 0;		/* no right-prompt currently on screen */

	if (!clearflag) {
	    if (tccan(TCCLEAREOD))
		tcoutclear(TCCLEAREOD);
	    else
		cleareol = 1;   /* request: clear to end of line */
	    if (listshown > 0)
		listshown = 0;
	}
	if (t0 > -1)
	    olnct = (t0 < winh) ? t0 : winh;
	if (termflags & TERM_SHORT)
	    vcs = 0;
	else if (!clearflag && lpromptbuf[0]) {
	    zputs(lpromptbuf, shout);
	    if (lpromptwof == winw)
		zputs("\n", shout);	/* works with both hasam and !hasam */
	} else {
	    txtchange = pmpt_attr;
	    settextattributes(txtchange);
	}
	if (clearflag) {
	    zputc(&zr_cr);
	    vcs = 0;
	    moveto(0, lpromptw);
	}
	fflush(shout);
	clearf = clearflag;
    } else if (winw != zterm_columns || rwinh != zterm_lines)
	resetvideo();

/* now winw equals columns and winh equals lines 
   width comparisons can be made with winw, height comparisons with winh */

    if (termflags & TERM_SHORT) {
	singlerefresh(tmpline, tmpll, tmpcs);
	goto singlelineout;
    }

    if (tmpcs < 0) {
#ifdef DEBUG
	fprintf(stderr, "BUG: negative cursor position\n");
	fflush(stderr); 
#endif
	tmpcs = 0;
    }
    scs = tmpline + tmpcs;
    numscrolls = 0;

/* first, we generate the video line buffers so we know what to put on
   the screen - also determine final cursor position (nvln, nvcs) */

    /* Deemed necessary by PWS 1995/05/15 due to kill-line problems */
    if (!*nbuf)
	*nbuf = (REFRESH_STRING)zalloc((winw + 2) * sizeof(**nbuf));

    memset(&rpms, 0, sizeof(rpms));
    rpms.nvln = -1;

    rpms.s = nbuf[rpms.ln = 0] + lpromptw;
    rpms.sen = *nbuf + winw;
    for (t = tmpline, tmppos = 0; tmppos < tmpll; t++, tmppos++) {
	unsigned ireg;
	zattr base_atr_on = default_atr_on, base_atr_off = 0;
	zattr all_atr_on, all_atr_off;
	struct region_highlight *rhp;
	/*
	 * Calculate attribute based on region.
	 */
	for (ireg = 0, rhp = region_highlights;
	     ireg < n_region_highlights;
	     ireg++, rhp++) {
	    int offset;
	    if (rhp->flags & ZRH_PREDISPLAY)
		offset = 0;	/* include predisplay in start end */
	    else
		offset = predisplaylen; /* increment over it */
	    if (rhp->start + offset <= tmppos &&
		tmppos < rhp->end + offset) {
		if (rhp->atr & (TXTFGCOLOUR|TXTBGCOLOUR)) {
		    /* override colour with later entry */
		    base_atr_on = (base_atr_on & ~TXT_ATTR_ON_VALUES_MASK) |
			rhp->atr;
		} else {
		    /* no colour set yet */
		    base_atr_on |= rhp->atr;
		}
		if (tmppos == rhp->end + offset - 1 ||
		    tmppos == tmpll - 1)
		    base_atr_off |= TXT_ATTR_OFF_FROM_ON(rhp->atr);
	    }
	}
	if (special_atr_on & (TXTFGCOLOUR|TXTBGCOLOUR)) {
	    /* keep colours from special attributes */
	    all_atr_on = special_atr_on |
		(base_atr_on & ~TXT_ATTR_COLOUR_ON_MASK);
	} else {
	    /* keep colours from standard attributes */
	    all_atr_on = special_atr_on | base_atr_on;
	}
	all_atr_off = TXT_ATTR_OFF_FROM_ON(all_atr_on);

	if (t == scs)			/* if cursor is here, remember it */
	    rpms.nvcs = rpms.s - nbuf[rpms.nvln = rpms.ln];

	if (*t == ZWC('\n')){		/* newline */
	    /* text not wrapped */
	    if (nextline(&rpms, 0))
		break;
	} else if (*t == ZWC('\t')) {		/* tab */
	    t0 = rpms.s - nbuf[rpms.ln];
	    if ((t0 | 7) + 1 >= winw) {
		/* text wrapped */
		if (nextline(&rpms, 1))
		    break;
	    } else {
		do {
		    rpms.s->chr = ZWC(' ');
		    rpms.s->atr = base_atr_on;
		    rpms.s++;
		} while ((++t0) & 7);
		rpms.s[-1].atr |= base_atr_off;
	    }
	}
#ifdef MULTIBYTE_SUPPORT
	else if (
#ifdef __STDC_ISO_10646__
		 !ZSH_INVALID_WCHAR_TEST(*t) &&
#endif
		 WC_ISPRINT(*t) && (width = WCWIDTH(*t)) > 0) {
	    int ichars;
	    if (width > rpms.sen - rpms.s) {
		int started = 0;
		/*
		 * Too wide to fit.  Insert spaces to end of current line.
		 */
		do {
		    rpms.s->chr = ZWC(' ');
		    if (!started)
			started = 1;
		    rpms.s->atr = all_atr_on;
		    rpms.s++;
		} while (rpms.s < rpms.sen);
		if (started)
		    rpms.s[-1].atr |= all_atr_off;
		if (nextline(&rpms, 1))
		    break;
		if (t == scs) {
		    /* Update cursor to this point */
		    rpms.nvcs = rpms.s - nbuf[rpms.nvln = rpms.ln];
		}
	    }
	    if (isset(COMBININGCHARS) && IS_BASECHAR(*t)) {
		/*
		 * Look for combining characters.
		 */
		for (ichars = 1; tmppos + ichars < tmpll; ichars++) {
		    if (!IS_COMBINING(t[ichars]))
			break;
		}
	    } else
		ichars = 1;
	    if (width > rpms.sen - rpms.s || width == 0) {
		/*
		 * The screen width is too small to fit even one
		 * occurrence.
		 */
		rpms.s->chr = ZWC('?');
		rpms.s->atr = all_atr_on | all_atr_off;
		rpms.s++;
	    } else {
		/* We can fit it without reaching the end of the line. */
		/*
		 * As we don't actually output the WEOF, we attach
		 * any off attributes to the character itself.
		 */
		rpms.s->atr = base_atr_on | base_atr_off;
		if (ichars > 1) {
		    /*
		     * Glyph includes combining characters.
		     * Write these into the multiword buffer and put
		     * the index into the value at the screen location.
		     */
		    addmultiword(rpms.s, t, ichars);
		} else {
		    /* Single wide character */
		    rpms.s->chr = *t;
		}
		rpms.s++;
		while (--width > 0) {
		    rpms.s->chr = WEOF;
		    /* Not used, but be consistent... */
		    rpms.s->atr = base_atr_on | base_atr_off;
		    rpms.s++;
		}
	    }
	    if (ichars > 1) {
		/* allow for normal increment */
		tmppos += ichars - 1;
		t += ichars - 1;
	    }
	}
#endif
	else if (ZC_icntrl(*t)
#ifdef MULTIBYTE_SUPPORT
		 && (unsigned)*t <= 0xffU
#endif
	    ) {	/* other control character */
	    rpms.s->chr = ZWC('^');
	    rpms.s->atr = all_atr_on;
	    rpms.s++;
	    if (rpms.s == rpms.sen) {
		/* text wrapped */
		rpms.s[-1].atr |= all_atr_off;
		if (nextline(&rpms, 1))
		    break;
	    }
	    rpms.s->chr = (((unsigned int)*t & ~0x80u) > 31) ?
		ZWC('?') : (*t | ZWC('@'));
	    rpms.s->atr = all_atr_on | all_atr_off;
	    rpms.s++;
	}
#ifdef MULTIBYTE_SUPPORT
	else {
	    /*
	     * Not printable or zero width.
	     * Resort to hackery.
	     */
	    char dispchars[11];
	    char *dispptr = dispchars;
	    wchar_t wc;
	    int started = 0;

#ifdef __STDC_ISO_10646__
	    if (ZSH_INVALID_WCHAR_TEST(*t)) {
		int c = ZSH_INVALID_WCHAR_TO_INT(*t);
		sprintf(dispchars, "<%.02x>", c);
	    } else
#endif
	    if ((unsigned)*t > 0xffffU) {
		sprintf(dispchars, "<%.08x>", (unsigned)*t);
	    } else {
		sprintf(dispchars, "<%.04x>", (unsigned)*t);
	    }
	    while (*dispptr) {
		if (mbtowc(&wc, dispptr, 1) == 1 /* paranoia */)
		{
		    rpms.s->chr = wc;
		    if (!started)
			started = 1;
		    rpms.s->atr = all_atr_on;
		    rpms.s++;
		    if (rpms.s == rpms.sen) {
			/* text wrapped */
			if (started) {
			    rpms.s[-1].atr |= all_atr_off;
			    started = 0;
			}
			if (nextline(&rpms, 1))
			    break;
		    }
		}
		dispptr++;
	    }
	    if (started)
		rpms.s[-1].atr |= all_atr_off;
	    if (*dispptr) /* nextline said stop processing */
		break;
	}
#else
	else {			/* normal character */
	    rpms.s->chr = *t;
	    rpms.s->atr = base_atr_on | base_atr_off;
	    rpms.s++;
	}
#endif
	if (rpms.s == rpms.sen) {
	    /* text wrapped */
	    if (nextline(&rpms, 1))
		break;
	}
    }

/* if we're really on the next line, don't fake it; do everything properly */
    if (t == scs &&
	(rpms.nvcs = rpms.s - (nbuf[rpms.nvln = rpms.ln])) == winw) {
	/* text wrapped */
	(void)nextline(&rpms, 1);
	*rpms.s = zr_zr;
	rpms.nvcs = 0;
	rpms.nvln++;
    }

    if (t != tmpline + tmpll)
	more_end = 1;

    if (statusline) {
	int outll, outsz;
	zattr all_atr_on, all_atr_off;
	char *statusdup = ztrdup(statusline);
	ZLE_STRING_T outputline =
	    stringaszleline(statusdup, 0, &outll, &outsz, NULL); 

	all_atr_on = special_atr_on;
	all_atr_off = TXT_ATTR_OFF_FROM_ON(all_atr_on);

	rpms.tosln = rpms.ln + 1;
	nbuf[rpms.ln][winw + 1] = zr_zr;	/* text not wrapped */
	snextline(&rpms);
	u = outputline;
	for (; u < outputline + outll; u++) {
#ifdef MULTIBYTE_SUPPORT
	    if (WC_ISPRINT(*u)) {
		int width = WCWIDTH(*u);
		/* Handle wide characters as above */
		if (width > rpms.sen - rpms.s) {
		    do {
			*rpms.s++ = zr_sp;
		    } while (rpms.s < rpms.sen);
		    nbuf[rpms.ln][winw + 1] = zr_nl;
		    snextline(&rpms);
		}
		if (width > rpms.sen - rpms.s) {
		    rpms.s->chr = ZWC('?');
		    rpms.s->atr = all_atr_on | all_atr_off;
		    rpms.s++;
		} else {
		    rpms.s->chr = *u;
		    rpms.s->atr = 0;
		    rpms.s++;
		    while (--width > 0) {
			rpms.s->chr = WEOF;
			rpms.s->atr = 0;
			rpms.s++;
		    }
		}
	    }
	    else
#endif
	    if (ZC_icntrl(*u)) { /* simplified processing in the status line */
		rpms.s->chr = ZWC('^');
		rpms.s->atr = all_atr_on;
		rpms.s++;
		if (rpms.s == rpms.sen) {
		    nbuf[rpms.ln][winw + 1] = zr_nl;/* text wrapped */
		    snextline(&rpms);
		}
		rpms.s->chr = (((unsigned int)*u & ~0x80u) > 31)
		    ? ZWC('?') : (*u | ZWC('@'));
		rpms.s->atr = all_atr_on | all_atr_off;
		rpms.s++;
	    } else {
		rpms.s->chr = *u;
		rpms.s->atr = 0;
		rpms.s++;
	    }
	    if (rpms.s == rpms.sen) {
		nbuf[rpms.ln][winw + 1] = zr_nl;	/* text wrapped */
		snextline(&rpms);
	    }
	}
	if (rpms.s == rpms.sen) {
	    /*
	     * I suppose we don't modify nbuf[rpms.ln][winw+1] here
	     * since we're right at the end?
	     */
	    snextline(&rpms);
	}
	zfree(outputline, outsz);
	free(statusdup);
    }
    *rpms.s = zr_zr;

/* insert <.... at end of last line if there is more text past end of screen */
    if (more_end) {
#ifdef MULTIBYTE_SUPPORT
	int extra_ellipsis = 0;
#endif
	if (!statusline)
	    rpms.tosln = winh;
	rpms.s = nbuf[rpms.tosln - 1];
	rpms.sen = rpms.s + winw - 7;
	for (; rpms.s < rpms.sen; rpms.s++) {
	    if (rpms.s->chr == ZWC('\0')) {
		ZR_memset(rpms.s, zr_sp, rpms.sen - rpms.s);
		/* make sure we don't trigger the WEOF test */
		rpms.sen->chr = ZWC('\0');
		break;
	    }
	}
	/* rpms.s is no longer needed */
#ifdef MULTIBYTE_SUPPORT
	/*
	 * Ensure we don't start overwriting in the middle of a wide
	 * character.
	 */
	while(rpms.sen > nbuf[rpms.tosln - 1] && rpms.sen->chr == WEOF) {
	    extra_ellipsis++;
	    rpms.sen--;
	}
#endif
	ZR_memcpy(rpms.sen, zr_end_ellipsis, ZR_END_ELLIPSIS_SIZE);
#ifdef MULTIBYTE_SUPPORT
	/* Extend to the end if we backed off for a wide character */
	if (extra_ellipsis) {
	    rpms.sen += ZR_END_ELLIPSIS_SIZE;
	    ZR_memset(rpms.sen, zr_dt, extra_ellipsis);
	}
#endif
	nbuf[rpms.tosln - 1][winw] = nbuf[rpms.tosln - 1][winw + 1] = zr_zr;
    }

/* insert <....> at end of first status line if status is too big */
    if (rpms.more_status) {
#ifdef MULTIBYTE_SUPPORT
	int extra_ellipsis = 0;
#endif
	rpms.s = nbuf[rpms.tosln];
	rpms.sen = rpms.s + winw - 8;
	for (; rpms.s < rpms.sen; rpms.s++) {
	    if (rpms.s->chr == ZWC('\0')) {
		ZR_memset(rpms.s, zr_sp, rpms.sen - rpms.s);
		break;
	    }
	}
	/* rpms.s is no longer needed */
#ifdef MULTIBYTE_SUPPORT
	/*
	 * Ensure we don't start overwriting in the middle of a wide
	 * character.
	 */
	while(rpms.sen > nbuf[rpms.tosln - 1] && rpms.sen->chr == WEOF) {
	    extra_ellipsis++;
	    rpms.sen--;
	}
#endif
	ZR_memcpy(rpms.sen, zr_mid_ellipsis1, ZR_MID_ELLIPSIS1_SIZE);
	rpms.sen += ZR_MID_ELLIPSIS1_SIZE;
#ifdef MULTIBYTE_SUPPORT
	/* Extend if we backed off for a wide character */
	if (extra_ellipsis) {
	    ZR_memset(rpms.sen, zr_dt, extra_ellipsis);
	    rpms.sen += extra_ellipsis;
	}
#endif
	ZR_memcpy(rpms.sen, zr_mid_ellipsis2, ZR_MID_ELLIPSIS2_SIZE);
	nbuf[rpms.tosln][winw] = nbuf[rpms.tosln][winw + 1] = zr_zr;
    }

    nlnct = rpms.ln + 1;
    for (iln = nlnct; iln < winh; iln++) {
	zfree(nbuf[iln], (winw + 2) * sizeof(**nbuf));
	nbuf[iln] = NULL;
    }

/* determine whether the right-prompt exists and can fit on the screen */
    if (!more_start) {
	if (trashedzle && opts[TRANSIENTRPROMPT])
	    put_rpmpt = 0;
	else {
	    put_rpmpt = rprompth == 1 && rpromptbuf[0] &&
		!strchr(rpromptbuf, '\t');
	    if (put_rpmpt)
	    {
	      rprompt_off = rprompt_indent;
	      /* sanity to avoid horrible things happening */
	      if (rprompt_off < 0)
		rprompt_off = 0;
	      put_rpmpt =
		(int)ZR_strlen(nbuf[0]) + rpromptw < winw - rprompt_off;
	    }
	}
    } else {
/* insert >.... on first line if there is more text before start of screen */
	ZR_memset(nbuf[0], zr_sp, lpromptw);
	t0 = winw - lpromptw;
	t0 = t0 > ZR_START_ELLIPSIS_SIZE ? ZR_START_ELLIPSIS_SIZE : t0;
	ZR_memcpy(nbuf[0] + lpromptw, zr_start_ellipsis, t0);
	ZR_memset(nbuf[0] + lpromptw + t0, zr_sp, winw - t0 - lpromptw);
	nbuf[0][winw] = nbuf[0][winw + 1] = zr_zr;
    }

    for (iln = 0; iln < nlnct; iln++) {
	/* if we have more lines than last time, clear the newly-used lines */
	if (iln >= olnct)
	    cleareol = 1;

    /* if old line and new line are different,
       see if we can insert/delete a line to speed up update */

	if (!clearf && iln > 0 && iln < olnct - 1 &&
	    !(hasam && vcs == winw) &&
	    nbuf[iln] && obuf[iln] &&
	    ZR_strncmp(obuf[iln], nbuf[iln], 16)) {
	    if (tccan(TCDELLINE) && obuf[iln + 1] &&
		obuf[iln + 1][0].chr && nbuf[iln] &&
		!ZR_strncmp(obuf[iln + 1], nbuf[iln], 16)) {
		moveto(iln, 0);
		tcout(TCDELLINE);
		zfree(obuf[iln], (winw + 2) * sizeof(**obuf));
		for (t0 = iln; t0 != olnct; t0++)
		    obuf[t0] = obuf[t0 + 1];
		obuf[--olnct] = NULL;
	    }
	/* don't try to insert a line if olnct = vmaxln (vmaxln is the number
	   of lines that have been displayed by this routine) so that we don't
	   go off the end of the screen. */

	    else if (tccan(TCINSLINE) && olnct < vmaxln && nbuf[iln + 1] &&
		     obuf[iln] && !ZR_strncmp(obuf[iln], nbuf[iln + 1], 16)) {
		moveto(iln, 0);
		tcout(TCINSLINE);
		for (t0 = olnct; t0 != iln; t0--)
		    obuf[t0] = obuf[t0 - 1];
		obuf[iln] = NULL;
		olnct++;
	    }
	}

    /* update the single line */
	refreshline(iln);

    /* output the right-prompt if appropriate */
	if (put_rpmpt && !iln && !oput_rpmpt) {
	    zattr attrchange;

	    moveto(0, winw - rprompt_off - rpromptw);
	    zputs(rpromptbuf, shout);
	    if (rprompt_off) {
		vcs = winw - rprompt_off;
	    } else {
		zputc(&zr_cr);
		vcs = 0;
	    }
	/* reset character attributes to that set by the main prompt */
	    txtchange = pmpt_attr;
	    /*
	     * Keep attributes that have actually changed,
	     * which are ones off in rpmpt_attr and on in
	     * pmpt_attr, and vice versa.
	     */
	    attrchange = txtchange &
		(TXT_ATTR_OFF_FROM_ON(rpmpt_attr) |
		 TXT_ATTR_ON_FROM_OFF(rpmpt_attr));
	    /*
	     * Careful in case the colour changed.
	     */
	    if (txtchangeisset(txtchange, TXTFGCOLOUR) &&
		(!txtchangeisset(rpmpt_attr, TXTFGCOLOUR) ||
		 ((txtchange ^ rpmpt_attr) & TXT_ATTR_FG_COL_MASK)))
	    {
		attrchange |=
		    txtchange & (TXTFGCOLOUR | TXT_ATTR_FG_COL_MASK);
	    }
	    if (txtchangeisset(txtchange, TXTBGCOLOUR) &&
		(!txtchangeisset(rpmpt_attr, TXTBGCOLOUR) ||
		 ((txtchange ^ rpmpt_attr) & TXT_ATTR_BG_COL_MASK)))
	    {
		attrchange |=
		    txtchange & (TXTBGCOLOUR | TXT_ATTR_BG_COL_MASK);
	    }
	    /*
	     * Now feed these changes into the usual function,
	     * if necessary.
	     */
	    if (attrchange)
		settextattributes(attrchange);
	}
    }

/* if old buffer had extra lines, set them to be cleared and refresh them
individually */

    if (olnct > nlnct) {
	cleareol = 1;
	for (iln = nlnct; iln < olnct; iln++)
	    refreshline(iln);
    }

/* reset character attributes */
    if (clearf && postedit) {
	if ((txtchange = pmpt_attr ? pmpt_attr : rpmpt_attr))
	    settextattributes(txtchange);
    }
    clearf = 0;
    oput_rpmpt = put_rpmpt;

/* move to the new cursor position */
    moveto(rpms.nvln, rpms.nvcs);

/* swap old and new buffers - better than freeing/allocating every time */
    bufswap();

/* store current values so we can use them next time */
    ovln = rpms.nvln;
    olnct = nlnct;
    onumscrolls = numscrolls;
    if (nlnct > vmaxln)
	vmaxln = nlnct;
singlelineout:
    fflush(shout);		/* make sure everything is written out */

    if (tmpalloced)
	zfree(tmpline, tmpll * sizeof(*tmpline));

    zle_free_highlight();

    /* if we have a new list showing, note it; if part of the list has been
    overwritten, redisplay it. We have to metafy line back before calling
    completion code */
    if (showinglist == -2 || (showinglist > 0 && showinglist < nlnct)) {
	if (remetafy) {
	    metafy_line();
	    remetafy = 0;
	}
	inlist = 1;
	listmatches();
	inlist = 0;
	if (!errflag)
	    zrefresh();
    }
    if (showinglist == -1)
	showinglist = nlnct;

    if (remetafy)
	metafy_line();
}

#define tcinscost(X)   (tccan(TCMULTINS) ? tclen[TCMULTINS] : (X)*tclen[TCINS])
#define tcdelcost(X)   (tccan(TCMULTDEL) ? tclen[TCMULTDEL] : (X)*tclen[TCDEL])
#define tc_delchars(X)	(void) tcmultout(TCDEL, TCMULTDEL, (X))
#define tc_inschars(X)	(void) tcmultout(TCINS, TCMULTINS, (X))
#define tc_upcurs(X)	(void) tcmultout(TCUP, TCMULTUP, (X))
#define tc_leftcurs(X)	(void) tcmultout(TCLEFT, TCMULTLEFT, (X))

/*
 * Once again, in the multibyte case the arguments must be in the
 * order:  element of old video array, element of new video array.
 */
static int
wpfxlen(const REFRESH_ELEMENT *olds, const REFRESH_ELEMENT *news)
{
    int i = 0;

    while (olds->chr && ZR_equal(*olds, *news))
	olds++, news++, i++;
    return i;
}

/* refresh one line, using whatever speed-up tricks are provided by the tty */

/**/
static void
refreshline(int ln)
{
    REFRESH_STRING nl, ol, p1;	/* line buffer pointers			 */
    int ccs = 0,		/* temporary count for cursor position	 */
	char_ins = 0,		/* number of characters inserted/deleted */
	col_cleareol,		/* clear to end-of-line from this column */
	i, j,			/* tmp					 */
	ins_last,		/* insert pushed last character off line */
	nllen, ollen,		/* new and old line buffer lengths	 */
	rnllen;			/* real new line buffer length		 */

/* 0: setup */
    nl = nbuf[ln];
    rnllen = nllen = nl ? ZR_strlen(nl) : 0;
    if (ln < olnct && obuf[ln]) {
	ol = obuf[ln];
	ollen = ZR_strlen(ol);
    }
    else {
	static REFRESH_ELEMENT nullchr = { ZWC('\0'), 0 };
	ol = &nullchr;
	ollen = 0;
    }

/* optimisation: can easily happen for clearing old lines.  If the terminal has
   the capability, then this is the easiest way to skip unnecessary stuff */
    if (cleareol && !nllen && !(hasam && ln < nlnct - 1)
	&& tccan(TCCLEAREOL)) {
	moveto(ln, 0);
	tcoutclear(TCCLEAREOL);
	return;	
    }

/* 1: pad out the new buffer with spaces to contain _all_ of the characters
      which need to be written. do this now to allow some pre-processing */

    if (cleareol 		/* request to clear to end of line */
	|| (!nllen && (ln != 0 || !put_rpmpt))	/* no line buffer given */
	|| (ln == 0 && (put_rpmpt != oput_rpmpt))) {	/* prompt changed */
	p1 = zhalloc((winw + 2) * sizeof(*p1));
	if (nllen)
	    ZR_memcpy(p1, nl, nllen);
	ZR_memset(p1 + nllen, zr_sp, winw - nllen);
	p1[winw] = zr_zr;
	if (nllen < winw)
	    p1[winw + 1] = zr_zr;
	else
	    p1[winw + 1] = nl[winw + 1];
	if (ln && nbuf[ln])
	    ZR_memcpy(nl, p1, winw + 2);	/* next time obuf will be up-to-date */
	else
	    nl = p1;		/* don't keep the padding for prompt line */
	nllen = winw;
    } else if (ollen > nllen) { /* make new line at least as long as old */
	p1 = zhalloc((ollen + 1) * sizeof(*p1));
	ZR_memcpy(p1, nl, nllen);
	ZR_memset(p1 + nllen, zr_sp, ollen - nllen);
	p1[ollen] = zr_zr;
	nl = p1;
	nllen = ollen;
    }

/* 2: see if we can clear to end-of-line, and if it's faster, work out where
   to do it from - we can normally only do so if there's no right-prompt.
   With automatic margins, we shouldn't do it if there is another line, in
   case it messes up cut and paste. */

    if (hasam && ln < nlnct - 1 && rnllen == winw)
	col_cleareol = -2;	/* clearing eol would be evil so don't */
    else {
	col_cleareol = -1;
	if (tccan(TCCLEAREOL) && (nllen == winw || put_rpmpt != oput_rpmpt)) {
	    for (i = nllen; i && ZR_equal(zr_sp, nl[i - 1]); i--)
		;
	    for (j = ollen; j && ZR_equal(ol[j - 1], zr_sp); j--)
		;
	    if ((j > i + tclen[TCCLEAREOL])	/* new buf has enough spaces */
		|| (nllen == winw && ZR_equal(zr_sp, nl[winw - 1])))
		col_cleareol = i;
	}
    }

/* 2b: first a new trick for automargin niceness - good for cut and paste */

    if (hasam && vcs == winw) {
	if (nbuf[vln] && nbuf[vln][vcs + 1].chr == ZWC('\n')) {
	    vln++, vcs = 1;
	    if (nbuf[vln]  && nbuf[vln]->chr) {
		zputc(nbuf[vln]);
	    } else
		zputc(&zr_sp);  /* I don't think this should happen */
	    if (ln == vln) {	/* better safe than sorry */
		nl++;
		if (ol->chr)
		    ol++;
		ccs = 1;
	    }			/* else  hmmm... I wonder what happened */
	} else {
	    vln++, vcs = 0;
	    zputc(&zr_nl);
	}
    }
    ins_last = 0;

/* 2c: if we're on the first line, start checking at the end of the prompt;
   we shouldn't be doing anything within the prompt */

    if (ln == 0 && lpromptw) {
	i = lpromptw - ccs;
	j = ZR_strlen(ol);
	nl += i;
	ol += (i > j ? j : i);	/* if ol is too short, point it to '\0' */
	ccs = lpromptw;
    }

#ifdef MULTIBYTE_SUPPORT
    /*
     * Realign to a real character after any jiggery pokery at
     * the start of the line.
     */
    while (nl->chr == WEOF) {
	nl++, ccs++, vcs++;
	if (ol->chr)
	    ol++;
    }
#endif

/* 3: main display loop - write out the buffer using whatever tricks we can */

    for (;;) {
	zattr now_off;

#ifdef MULTIBYTE_SUPPORT
	if ((!nl->chr || nl->chr != WEOF) && (!ol->chr || ol->chr != WEOF)) {
#endif
	    if (nl->chr && ol->chr && ZR_equal(ol[1], nl[1])) {
		/* skip only if second chars match */
#ifdef MULTIBYTE_SUPPORT
		int ccs_was = ccs;
#endif
		/* skip past all matching characters */
		for (; nl->chr && ZR_equal(*ol, *nl); nl++, ol++, ccs++)
		    ;
#ifdef MULTIBYTE_SUPPORT
		/* Make sure ol and nl are pointing to real characters */
		while ((nl->chr == WEOF || ol->chr == WEOF) && ccs > ccs_was) {
		    nl--;
		    ol--;
		    ccs--;
		}
#endif
	    }

	    if (!nl->chr) {
		if (ccs == winw && hasam && char_ins > 0 && ins_last
		    && vcs != winw) {
		    nl--;	   /* we can assume we can go back here */
		    moveto(ln, winw - 1);
		    zputc(nl);
		    vcs++;
		    return;	 /* write last character in line */
		}
		if ((char_ins <= 0) || (ccs >= winw))    /* written everything */
		    return;
		if (tccan(TCCLEAREOL) && (char_ins >= tclen[TCCLEAREOL])
		    && col_cleareol != -2)
		    /* we've got junk on the right yet to clear */
		    col_cleareol = 0;	/* force a clear to end of line */
	    }

	    moveto(ln, ccs);	/* move to where we do all output from */

	    /* if we can finish quickly, do so */
	    if ((col_cleareol >= 0) && (ccs >= col_cleareol)) {
		tcoutclear(TCCLEAREOL);
		return;
	    }

	    /* we've written out the new but yet to clear rubbish due to inserts */
	    if (!nl->chr) {
		i = (winw - ccs < char_ins) ? (winw - ccs) : char_ins;
		if (tccan(TCDEL) && (tcdelcost(i) <= i + 1))
		    tc_delchars(i);
		else {
		    vcs += i;
		    while (i-- > 0)
			zputc(&zr_sp);
		}
		return;
	    }

	    /* if we've reached the end of the old buffer, then there are few tricks
	       we can do, so we just dump out what we must and clear if we can */
	    if (!ol->chr) {
		i = (col_cleareol >= 0) ? col_cleareol : nllen;
		i -= vcs;
		if (i < 0) {
		    /*
		     * This shouldn't be necessary, but it's better
		     * than a crash if there's a bug somewhere else,
		     * so report in debug mode.
		     */
		    DPUTS(1, "BUG: badly calculated old line width in refresh");
		    i = 0;
		}
		zwrite(nl, i);
		vcs += i;
		if (col_cleareol >= 0)
		    tcoutclear(TCCLEAREOL);
		return;
	    }

	    /* inserting & deleting chars: we can if there's no right-prompt */
	    if ((ln || !put_rpmpt || !oput_rpmpt) 
#ifdef MULTIBYTE_SUPPORT
		&& ol->chr != WEOF && nl->chr != WEOF
#endif
		&& nl[1].chr && ol[1].chr && !ZR_equal(ol[1], nl[1])) { 

		/* deleting characters - see if we can find a match series that
		   makes it cheaper to delete intermediate characters
		   eg. oldline: hifoobar \ hopefully cheaper here to delete two
		   newline: foobar	 / characters, then we have six matches */
		if (tccan(TCDEL)) {
		    int first = 1;
		    for (i = 1; ol[i].chr; i++) {
			if (tcdelcost(i) < wpfxlen(ol + i, nl)) {
			    /*
			     * Some terminals will output the current
			     * attributes into cells added at the end by
			     * deletions, so turn off text attributes.
			     */
			    if (first) {
				clearattributes();
				first = 0;
			    }
			    tc_delchars(i);
			    ol += i;
			    char_ins -= i;
#ifdef MULTIBYTE_SUPPORT
			    while (ol->chr == WEOF) {
				ol++;
				char_ins--;
			    }
#endif
			    i = 0;
			    break;
			}
		    }
		    if (!i)
			continue;
		}
		/*
		 * inserting characters - characters pushed off the right
		 * should be annihilated, but we don't do this if we're on the
		 * last line lest undesired scrolling occurs due to `illegal'
		 * characters on screen
		 */ 
		if (tccan(TCINS) && (vln != zterm_lines - 1)) {
		    /* not on last line */
		    for (i = 1; nl[i].chr; i++) {
			if (tcinscost(i) < wpfxlen(ol, nl + i)) {
			    tc_inschars(i);
			    zwrite(nl, i);
			    nl += i;
#ifdef MULTIBYTE_SUPPORT
			    while (nl->chr == WEOF) {
				nl++;
				i++;
			    }
#endif
			    char_ins += i;
			    ccs = (vcs += i);
			    /*
			     * if we've pushed off the right, truncate
			     * oldline
			     */
			    for (i = 0; ol[i].chr && i < winw - ccs; i++)
				;
#ifdef MULTIBYTE_SUPPORT
			    while (ol[i].chr == WEOF)
				i++;
			    if (i >= winw - ccs) {
				/*
				 * Yes, we're over the right.
				 * Make sure we truncate at the real
				 * character, not a WEOF added to
				 * make up the width.
				 */
				while (ol[i-1].chr == WEOF)
				    i--;
				ol[i] = zr_zr;
				ins_last = 1;
			    }
#else
			    if (i >= winw - ccs) {
				ol[i] = zr_zr;
				ins_last = 1;
			    }
#endif
			    i = 0;
			    break;
			}
		    }
		    if (!i)
			continue;
		}
	    }
#ifdef MULTIBYTE_SUPPORT
	}
#endif
    /* we can't do any fancy tricks, so just dump the single character
       and keep on trying */
#ifdef MULTIBYTE_SUPPORT
	/*
	 * in case we were tidying up a funny-width character when we
	 * reached the end of the new line...
	 */
	if (!nl->chr)
	    break;
	do {
#endif
	    /*
	     * If an attribute was on here but isn't any more,
	     * output the sequence to turn it off.
	     */
	    now_off = ol->atr & ~nl->atr & TXT_ATTR_ON_MASK;
	    if (now_off)
		settextattributes(TXT_ATTR_OFF_FROM_ON(now_off));

	    /*
	     * This is deliberately called if nl->chr is WEOF
	     * in order to keep text attributes consistent.
	     * We check for WEOF inside.
	     */
	    zputc(nl);
	    nl++;
	    if (ol->chr)
	      ol++;
	    ccs++, vcs++;
#ifdef MULTIBYTE_SUPPORT
	    /*
	     * Make sure we always overwrite the complete width of
	     * a character that was there before.
	     */
	} while ((ol->chr == WEOF && nl->chr) ||
		 (nl->chr == WEOF && ol->chr));
#endif
    }
}

/* move the cursor to line ln (relative to the prompt line),
   absolute column cl; update vln, vcs - video line and column */

/**/
void
moveto(int ln, int cl)
{
    const REFRESH_ELEMENT *rep;

    if (vcs == winw) {
	vln++, vcs = 0;
	if (!hasam) {
	    zputc(&zr_cr);
	    zputc(&zr_nl);
	} else {
	    if ((vln < nlnct) && nbuf[vln] && nbuf[vln]->chr)
		rep = nbuf[vln];
	    else
		rep = &zr_sp;
	    zputc(rep);
	    zputc(&zr_cr);
	    if ((vln < olnct) && obuf[vln] && obuf[vln]->chr)
		*obuf[vln] = *rep;
	}
    }

    if (ln == vln && cl == vcs)
	return;

/* move up */
    if (ln < vln) {
	tc_upcurs(vln - ln);
	vln = ln;
    }
/* move down; if we might go off the end of the screen, use newlines
   instead of TCDOWN */

    while (ln > vln) {
	if (vln < vmaxln - 1) {
	    if (ln > vmaxln - 1) {
		if (tc_downcurs(vmaxln - 1 - vln))
		    vcs = 0;
		vln = vmaxln - 1;
	    } else {
		if (tc_downcurs(ln - vln))
		    vcs = 0;
		vln = ln;
		continue;
	    }
	}
	zputc(&zr_cr), vcs = 0; /* safety precaution */
	while (ln > vln) {
	    zputc(&zr_nl);
	    vln++;
	}
    }

    if (cl != vcs)
	singmoveto(cl);
}

/**/
mod_export int
tcmultout(int cap, int multcap, int ct)
{
    if (tccan(multcap) && (!tccan(cap) || tclen[multcap] <= tclen[cap] * ct)) {
	tcoutarg(multcap, ct);
	return 1;
    } else if (tccan(cap)) {
	while (ct--)
	    tcout(cap);
	return 1;
    }
    return 0;
}

/* ct: number of characters to move across */
/**/
static void
tc_rightcurs(int ct)
{
    int cl,			/* ``desired'' absolute horizontal position */
	i = vcs,		/* cursor position after initial movements  */
	j;
    REFRESH_STRING t;

    cl = ct + vcs;

/* do a multright if we can - it's the most reliable */
    if (tccan(TCMULTRIGHT)) {
	tcoutarg(TCMULTRIGHT, ct);
	return;
    }

/* do an absolute horizontal position if we can */
    if (tccan(TCHORIZPOS)) {
	tcoutarg(TCHORIZPOS, cl);
	return;
    }

/* XXX: should really check "it" in termcap and use / and % */
/* try tabs if tabs are non destructive and multright is not possible */
    if (!oxtabs && tccan(TCNEXTTAB) && ((vcs | 7) < cl)) {
	i = (vcs | 7) + 1;
	tcout(TCNEXTTAB);
	for ( ; i + 8 <= cl; i += 8)
	    tcout(TCNEXTTAB);
	if ((ct = cl - i) == 0) /* number of chars still to move across */
	    return;
    }

/* otherwise _carefully_ write the contents of the video buffer.
   if we're anywhere in the prompt, goto the left column and write the whole
   prompt out.

   If strlen(lpromptbuf) == lpromptw, we can cheat and output
   the appropriate chunk of the string.  This test relies on the
   fact that any funny business will always make the length of
   the string larger than the printing width, so if they're the same
   we have only ASCII characters or a single-byte extension of ASCII.
   Unfortunately this trick won't work if there are potentially
   characters occupying more than one column.  We could flag that
   this has happened (since it's not that common to have characters
   wider than one column), but for now it's easier not to use the
   trick if we are using WCWIDTH() on the prompt.  It's not that
   common to be editing in the middle of the prompt anyway, I would
   think.
   */
    if (vln == 0 && i < lpromptw && !(termflags & TERM_SHORT)) {
#ifndef MULTIBYTE_SUPPORT
	if ((int)strlen(lpromptbuf) == lpromptw)
	    fputs(lpromptbuf + i, shout);
	else 
#endif
	if (tccan(TCRIGHT) && (tclen[TCRIGHT] * ct <= ztrlen(lpromptbuf)))
	    /* it is cheaper to send TCRIGHT than reprint the whole prompt */
	    for (ct = lpromptw - i; ct--; )
		tcout(TCRIGHT);
	else {
	    if (i != 0)
		zputc(&zr_cr);
	    tc_upcurs(lprompth - 1);
	    zputs(lpromptbuf, shout);
	    if (lpromptwof == winw)
		zputs("\n", shout);	/* works with both hasam and !hasam */
	}
	i = lpromptw;
	ct = cl - i;
    }

    if (nbuf[vln]) {
	for (j = 0, t = nbuf[vln]; t->chr && (j < i); j++, t++);
	if (j == i)
	    for ( ; t->chr && ct; ct--, t++)
		zputc(t);
    }
    while (ct--)
	zputc(&zr_sp);	/* not my fault your terminal can't go right */
}

/**/
mod_export int
tc_downcurs(int ct)
{
    int ret = 0;

    if (ct && !tcmultout(TCDOWN, TCMULTDOWN, ct)) {
	while (ct--)
	    zputc(&zr_nl);
	zputc(&zr_cr), ret = -1;
    }
    return ret;
}

/*
 * Output a termcap value using a function defined by "zle -T tc".
 * Loosely inspired by subst_string_by_func().
 *
 * cap is the internal index for the capability; it will be looked up
 * in the table and the string passed to the function.
 *
 * arg is eithr an argument to the capability or -1 if there is none;
 * if it is not -1 it will be passed as an additional argument to the
 * function.
 *
 * outc is the output function; currently this is always putshout
 * but in principle it may be used to output to a string.
 */

/**/
static void
tcout_via_func(int cap, int arg, int (*outc)(int))
{
    Shfunc tcout_func;
    int osc, osm, old_incompfunc;

    osc = sfcontext;
    osm = stopmsg;
    old_incompfunc = incompfunc;

    sfcontext = SFC_SUBST;
    incompfunc = 0;

    if ((tcout_func = getshfunc(tcout_func_name))) {
	LinkList l = newlinklist();
	char buf[DIGBUFSIZE], *str;

	addlinknode(l, tcout_func_name);
	addlinknode(l, tccap_get_name(cap));

	if (arg != -1) {
	    sprintf(buf, "%d", arg);
	    addlinknode(l, buf);
	}

	(void)doshfunc(tcout_func, l, 1);

	str = getsparam("REPLY");
	if (str) {
	    while (*str) {
		int chr;
		if (*str == Meta) {
		    chr = str[1] ^ 32;
		    str += 2;
		} else {
		    chr = *str++;
		}
		(void)outc(chr);
	    }
	}
    }

    sfcontext = osc;
    stopmsg = osm;
    incompfunc = old_incompfunc;
}

/**/
mod_export void
tcout(int cap)
{
    if (tcout_func_name) {
	tcout_via_func(cap, -1, putshout);
    } else {
	tputs(tcstr[cap], 1, putshout);
    }
    SELECT_ADD_COST(tclen[cap]);
}

/**/
static void
tcoutarg(int cap, int arg)
{
    char *result;

    result = tgoto(tcstr[cap], arg, arg);
    if (tcout_func_name) {
	tcout_via_func(cap, arg, putshout);
    } else {
	tputs(result, 1, putshout);
    }
    SELECT_ADD_COST(strlen(result));
}

/**/
mod_export int
clearscreen(UNUSED(char **args))
{
    tcoutclear(TCCLEARSCREEN);
    resetneeded = 1;
    clearflag = 0;
    reexpandprompt();
    return 0;
}

/**/
mod_export int
redisplay(UNUSED(char **args))
{
    moveto(0, 0);
    zputc(&zr_cr);		/* extra care */
    tc_upcurs(lprompth - 1);
    resetneeded = 1;
    clearflag = 0;
    return 0;
}

/*
 * Show as much of the line buffer as we can in single line mode.
 * TBD: all termcap effects are turned off in this mode, so
 * there's no point in using character attributes.  We should
 * decide what we're going to do and either remove the handling
 * from here or enable it in tsetcap().
 */

/**/
static void
singlerefresh(ZLE_STRING_T tmpline, int tmpll, int tmpcs)
{
    REFRESH_STRING vbuf, vp,	/* video buffer and pointer    */
	refreshop;	        /* pointer to old video buffer */
    int t0,			/* tmp			       */
	vsiz,			/* size of new video buffer    */
	nvcs = 0,		/* new video cursor column     */
	owinpos = winpos,	/* previous window position    */
	owinprompt = winprompt;	/* previous winprompt          */
#ifdef MULTIBYTE_SUPPORT
    int width;			/* width of multibyte character */
#endif

    nlnct = 1;
/* generate the new line buffer completely */
    for (vsiz = 1 + lpromptw, t0 = 0; t0 != tmpll; t0++) {
	if (tmpline[t0] == ZWC('\t'))
	    vsiz = (vsiz | 7) + 2;
#ifdef MULTIBYTE_SUPPORT
	else if (WC_ISPRINT(tmpline[t0]) && ((width = WCWIDTH(tmpline[t0])) > 0)) {
	    vsiz += width;
	    if (isset(COMBININGCHARS) && IS_BASECHAR(tmpline[t0])) {
		while (t0 < tmpll-1 && IS_COMBINING(tmpline[t0+1]))
		    t0++;
	    }
	}
#endif
	else if (ZC_icntrl(tmpline[t0])
#ifdef MULTIBYTE_SUPPORT
		 && (unsigned)tmpline[t0] <= 0xffU
#endif
		 )
	    vsiz += 2;
#ifdef MULTIBYTE_SUPPORT
	else
	    vsiz += 10;
#else
	else
	    vsiz++;
#endif
    }
    vbuf = (REFRESH_STRING)zalloc(vsiz * sizeof(*vbuf));

    if (tmpcs < 0) {
#ifdef DEBUG
	fprintf(stderr, "BUG: negative cursor position\n");
	fflush(stderr);
#endif
	tmpcs = 0;
    }

    /* prompt is not directly copied into the video buffer */
    ZR_memset(vbuf, zr_sp, lpromptw);
    vp = vbuf + lpromptw;
    *vp = zr_zr;

    for (t0 = 0; t0 < tmpll; t0++) {
	unsigned ireg;
	zattr base_atr_on = 0, base_atr_off = 0;
	zattr all_atr_on, all_atr_off;
	struct region_highlight *rhp;
	/*
	 * Calculate attribute based on region.
	 */
	for (ireg = 0, rhp = region_highlights;
	     ireg < n_region_highlights;
	     ireg++, rhp++) {
	    int offset;
	    if (rhp->flags & ZRH_PREDISPLAY)
		offset = 0;	/* include predisplay in start end */
	    else
		offset = predisplaylen; /* increment over it */
	    if (rhp->start + offset <= t0 &&
		t0 < rhp->end + offset) {
		if (base_atr_on & (TXTFGCOLOUR|TXTBGCOLOUR)) {
		    /* keep colour already set */
		    base_atr_on |= rhp->atr & ~TXT_ATTR_COLOUR_ON_MASK;
		} else {
		    /* no colour set yet */
		    base_atr_on |= rhp->atr;
		}
		if (t0 == rhp->end + offset - 1 ||
		    t0 == tmpll - 1)
		    base_atr_off |= TXT_ATTR_OFF_FROM_ON(rhp->atr);
	    }
	}
	if (special_atr_on & (TXTFGCOLOUR|TXTBGCOLOUR)) {
	    /* keep colours from special attributes */
	    all_atr_on = special_atr_on |
		(base_atr_on & ~TXT_ATTR_COLOUR_ON_MASK);
	} else {
	    /* keep colours from standard attributes */
	    all_atr_on = special_atr_on | base_atr_on;
	}
	all_atr_off = TXT_ATTR_OFF_FROM_ON(all_atr_on);

	if (tmpline[t0] == ZWC('\t')) {
	    for (*vp++ = zr_sp; (vp - vbuf) & 7; )
		*vp++ = zr_sp;
	    vp[-1].atr |= base_atr_off;
	} else if (tmpline[t0] == ZWC('\n')) {
	    vp->chr = ZWC('\\');
	    vp->atr = all_atr_on;
	    vp++;
	    vp->chr = ZWC('n');
	    vp->atr = all_atr_on | all_atr_off;
	    vp++;
#ifdef MULTIBYTE_SUPPORT
	} else if (WC_ISPRINT(tmpline[t0]) &&
		   (width = WCWIDTH(tmpline[t0])) > 0) {
	    int ichars;
	    if (isset(COMBININGCHARS) && IS_BASECHAR(tmpline[t0])) {
		/*
		 * Look for combining characters.
		 */
		for (ichars = 1; t0 + ichars < tmpll; ichars++) {
		    if (!IS_COMBINING(tmpline[t0+ichars]))
			break;
		}
	    } else
		ichars = 1;
	    vp->atr = base_atr_on | base_atr_off;
	    if (ichars > 1)
		addmultiword(vp, tmpline+t0, ichars);
	    else
		vp->chr = tmpline[t0];
	    vp++;
	    while (--width > 0) {
		vp->chr = WEOF;
		vp->atr = base_atr_on | base_atr_off;
		vp++;
	    }
	    t0 += ichars - 1;
#endif
	} else if (ZC_icntrl(tmpline[t0])
#ifdef MULTIBYTE_SUPPORT
		   && (unsigned)tmpline[t0] <= 0xffU
#endif
		   ) {
	    ZLE_INT_T t = tmpline[++t0];

	    vp->chr = ZWC('^');
	    vp->atr = all_atr_on;
	    vp++;
	    vp->chr = (((unsigned int)t & ~0x80u) > 31) ?
		ZWC('?') : (t | ZWC('@'));
	    vp->atr = all_atr_on | all_atr_off;
	    vp++;
	}
#ifdef MULTIBYTE_SUPPORT
	else {
	    char dispchars[11];
	    char *dispptr = dispchars;
	    wchar_t wc;
	    int started = 0;

	    if ((unsigned)tmpline[t0] > 0xffffU) {
		sprintf(dispchars, "<%.08x>", (unsigned)tmpline[t0]);
	    } else {
		sprintf(dispchars, "<%.04x>", (unsigned)tmpline[t0]);
	    }
	    while (*dispptr) {
		if (mbtowc(&wc, dispptr, 1) == 1 /* paranoia */) {
		    vp->chr = wc;
		    if (!started)
			started = 1;
		    vp->atr = all_atr_on;
		    vp++;
		}
		dispptr++;
	    }
	    if (started)
		vp[-1].atr |= all_atr_off;
	}
#else
	else {
	    vp->chr = tmpline[t0];
	    vp->atr = base_atr_on | base_atr_off;
	    vp++;
	}
#endif
	if (t0 == tmpcs)
	    nvcs = vp - vbuf - 1;
    }
    if (t0 == tmpcs)
	nvcs = vp - vbuf;
    *vp = zr_zr;

/* determine which part of the new line buffer we want for the display */
    if (winpos == -1)
	winpos = 0;
    if ((winpos && nvcs < winpos + 1) || (nvcs > winpos + winw - 2)) {
	if ((winpos = nvcs - ((winw - hasam) / 2)) < 0)
	    winpos = 0;
    }
    if (winpos) {
	vbuf[winpos].chr = ZWC('<');	/* line continues to the left */
	vbuf[winpos].atr = 0;
    }
    if ((int)ZR_strlen(vbuf + winpos) > (winw - hasam)) {
	vbuf[winpos + winw - hasam - 1].chr = ZWC('>');	/* line continues to right */
	vbuf[winpos + winw - hasam - 1].atr = 0;
	vbuf[winpos + winw - hasam] = zr_zr;
    }
    ZR_strcpy(nbuf[0], vbuf + winpos);
    zfree(vbuf, vsiz * sizeof(*vbuf));
    nvcs -= winpos;

    if (winpos < lpromptw) {
	/* skip start of buffer corresponding to prompt */
	winprompt = lpromptw - winpos;
    } else {
	/* don't */
	winprompt = 0;
    }
    if (winpos != owinpos && winprompt) {
	char *pptr;
	int skipping = 0, skipchars = winpos;
	/*
	 * Need to output such part of the left prompt as fits.
	 * Skip the first winpos characters, outputting
	 * any characters marked with %{...%}.
	 */
	singmoveto(0);
	MB_METACHARINIT();
	for (pptr = lpromptbuf; *pptr; ) {
	    if (*pptr == Inpar) {
		skipping = 1;
		pptr++;
	    } else if (*pptr == Outpar) {
		skipping = 0;
		pptr++;
	    } else {
		convchar_t cc;
		int mblen = MB_METACHARLENCONV(pptr, &cc);
		if (skipping || skipchars == 0)
		{
		    while (mblen) {
#ifdef MULTIBYTE_SUPPORT
			if (cc == WEOF)
			    fputc('?', shout);
			else
#endif
			    if (*pptr == Meta) {
				mblen--;
				fputc(*++pptr ^ 32, shout);
			    } else {
				fputc(*pptr, shout);
			    }
			pptr++;
			mblen--;
		    }
		} else {
		    skipchars--;
		    pptr += mblen;
		}
	    }
	}
	vcs = winprompt;
    }

/* display the `visible' portion of the line buffer */
    t0 = winprompt;
    vp = *nbuf + winprompt;
    refreshop = *obuf + winprompt;
    for (;;) {
	/*
	 * Skip past all matching characters, but if there used
	 * to be a prompt here be careful since all manner of
	 * nastiness may be around.
	 */
	if (vp - *nbuf >= owinprompt)
	    for (; vp->chr && ZR_equal(*refreshop, *vp);
		 t0++, vp++, refreshop++)
		;

	if (!vp->chr && !refreshop->chr)
	    break;

	singmoveto(t0);		/* move to where we do all output from */

	if (!refreshop->chr) {
	    if ((t0 = ZR_strlen(vp)))
		zwrite(vp, t0);
	    vcs += t0;
	    break;
	}
	if (!vp->chr) {
	    if (tccan(TCCLEAREOL))
		tcoutclear(TCCLEAREOL);
	    else
		for (; refreshop++->chr; vcs++)
		    zputc(&zr_sp);
	    break;
	}
	zputc(vp);
	vcs++, t0++;
	vp++, refreshop++;
    }
/* move to the new cursor position */
    singmoveto(nvcs);

    bufswap();
}

/**/
static void
singmoveto(int pos)
{
    if (pos == vcs)
	return;

/* choose cheapest movements for ttys without multiple movement capabilities -
   do this now because it's easier (to code) */

    if ((!tccan(TCMULTLEFT) || pos == 0) && (pos <= vcs / 2)) {
	zputc(&zr_cr);
	vcs = 0;
    }

    if (pos < vcs)
	tc_leftcurs(vcs - pos);
    else if (pos > vcs)
	tc_rightcurs(pos - vcs);

    vcs = pos;
}

/* Provided for loading the module in a modular fashion */

/**/
void
zle_refresh_boot(void)
{
}

/* Provided for unloading the module in a modular fashion */

/**/
void
zle_refresh_finish(void)
{
    freevideo();

    if (region_highlights)
    {
	free_region_highlights_memos();
	zfree(region_highlights,
	      sizeof(struct region_highlight) * n_region_highlights);
	region_highlights = NULL;
	n_region_highlights = 0;
    }
}
