/*
 * zle_hist.c - history editing
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
#include "zle_hist.pro"

/* Column position of vi ideal cursor.  -1 if it is unknown -- most *
 * movements and changes do this.                                   */

/**/
int lastcol;

/* current history line number */

/**/
int histline;

/* Previous search string use in an incremental search */

/**/
char *previous_search = NULL;

/**/
int previous_search_len;

/* Previous aborted search string use in an incremental search */

/**/
char *previous_aborted_search = NULL;

/* Local keymap in isearch mode */

/**/
Keymap isearch_keymap;

/*** History text manipulation utilities ***/

/*
 * Text for the line:  anything previously modified within zle since
 * the last time the line editor was started, else what was originally
 * put in the history.
 */
#define GETZLETEXT(ent)	((ent)->zle_text ? (ent)->zle_text : (ent)->node.nam)

/**/
void
remember_edits(void)
{
    Histent ent = quietgethist(histline);
    if (ent) {
	char *line =
	    zlemetaline ? zlemetaline :
	    zlelineasstring(zleline, zlell, 0, NULL, NULL, 0);
	if (!ent->zle_text || strcmp(line, ent->zle_text) != 0) {
	    if (ent->zle_text)
		free(ent->zle_text);
	    ent->zle_text = zlemetaline ? ztrdup(line) : line;
	} else if (!zlemetaline)
	    free(line);
    }
}

/**/
void
forget_edits(void)
{
    Histent he;

    for (he = hist_ring; he; he = up_histent(he)) {
	if (he->zle_text) {
	    free(he->zle_text);
	    he->zle_text = NULL;
	}
    }
}


/*** Search utilities ***/


/*
 * Return zero if the ZLE string histp length histl and the ZLE string
 * inputp length inputl are the same.  Return -1 if inputp is a prefix
 * of histp.  Return 1 if inputp is the lowercase version of histp.
 * Return 2 if inputp is the lowercase prefix of histp and return 3
 * otherwise.
 */

static int
zlinecmp(const char *histp, const char *inputp)
{
    const char *hptr = histp, *iptr = inputp;
#ifdef MULTIBYTE_SUPPORT
    mbstate_t hstate, istate;
#endif

    while (*iptr && *hptr == *iptr) {
	hptr++;
	iptr++;
    }

    if (!*iptr) {
	if (!*hptr) {
	    /* strings are the same */
	    return 0;
	} else {
	    /* inputp is a prefix */
	    return -1;
	}
    }

#ifdef MULTIBYTE_SUPPORT
    memset(&hstate, 0, sizeof(hstate));
    memset(&istate, 0, sizeof(istate));
#endif

    /* look for lower case versions */
    while (*histp && *inputp) {
#ifdef MULTIBYTE_SUPPORT
	wint_t hwc, iwc;
	int hlen, ilen;

	hlen = mb_metacharlenconv_r(histp, &hwc, &hstate);
	ilen = mb_metacharlenconv_r(inputp, &iwc, &istate);

	if (hwc == WEOF || iwc == WEOF) {
	    /* can't convert, compare input characters */
	    if (ilen != hlen || memcmp(histp, inputp, hlen) != 0)
		return 3;
	} else if (towlower(hwc) != iwc)
	    return 3;

	histp += hlen;
	inputp += ilen;
#else
    	if (tulower(*histp++) != *inputp++)
	    return 3;
#endif
    }
    if (!*inputp) {
	/* one string finished, if it's the input... */
	if (!*histp)
	    return 1;		/* ...same, else */
	else
	    return 2;		/* ...prefix */
    }
    /* Different */
    return 3;
}


/*
 * Search for needle in haystack.  Haystack and needle are metafied strings.
 * Start the search at position pos in haystack.
 * Search forward if dir > 0, otherwise search backward.
 * sens is used to test against the return value of linecmp.
 *
 * Return the pointer to the location in haystack found, else NULL.
 *
 * We assume we'll only find needle at some sensible position in a multibyte
 * string, so we don't bother calculating multibyte character lengths for
 * incrementing and decrementing the search position.
 */

static char *
zlinefind(char *haystack, int pos, char *needle, int dir, int sens)
{
    char *s = haystack + pos;

    if (dir > 0) {
	while (*s) {
	    if (zlinecmp(s, needle) < sens)
		return s;
	    s++;
	}
    } else {
	for (;;) {
	    if (zlinecmp(s, needle) < sens)
		return s;
	    if (s == haystack)
		break;
	    s--;
	}
    }

    return NULL;
}


/*** Widgets ***/


/**/
int
uphistory(UNUSED(char **args))
{
    int nodups = isset(HISTIGNOREDUPS);
    if (!zle_goto_hist(histline, -zmult, nodups) && isset(HISTBEEP))
	return 1;
    return 0;
}

/**/
int
upline(char **args)
{
    int n = zmult;

    if (n < 0) {
	zmult = -zmult;
	n = -downline(args);
	zmult = -zmult;
	return n;
    }
    if (lastcol == -1)
	lastcol = zlecs - findbol();
    zlecs = findbol();
    while (n) {
	if (!zlecs)
	    break;
	zlecs--;
	zlecs = findbol();
	n--;
    }
    if (!n) {
	int x = findeol();

	if ((zlecs += lastcol) >= x) {
	    zlecs = x;
	    if (zlecs > findbol() && invicmdmode())
		DECCS();
	}
#ifdef MULTIBYTE_SUPPORT
	else
	    CCRIGHT();
#endif
	    
    }
    return n;
}

/**/
int
uplineorhistory(char **args)
{
    int ocs = zlecs;
    int n = upline(args);
    if (n) {
	int m = zmult, ret;

	zlecs = ocs;
	if (virangeflag || !(zlereadflags & ZLRF_HISTORY))
	    return 1;
	zmult = n;
	ret = uphistory(args);
	zmult = m;
	return ret;
    }
    return 0;
}

/**/
int
viuplineorhistory(char **args)
{
    int col = lastcol;
    uplineorhistory(args);
    lastcol = col;
    return vifirstnonblank(args);
}

/**/
int
uplineorsearch(char **args)
{
    int ocs = zlecs;
    int n = upline(args);
    if (n) {
	int m = zmult, ret;

	zlecs = ocs;
	if (virangeflag || !(zlereadflags & ZLRF_HISTORY))
	    return 1;
	zmult = n;
	ret = historysearchbackward(args);
	zmult = m;
	return ret;
    }
    return 0;
}

/**/
int
downline(char **args)
{
    int n = zmult;

    if (n < 0) {
	zmult = -zmult;
	n = -upline(args);
	zmult = -zmult;
	return n;
    }
    if (lastcol == -1)
	lastcol = zlecs - findbol();
    while (n) {
	int x = findeol();

	if (x == zlell)
	    break;
	zlecs = x + 1;
	n--;
    }
    if (!n) {
	int x = findeol();

	if ((zlecs += lastcol) >= x) {
	    zlecs = x;
	    if (zlecs > findbol() && invicmdmode())
		DECCS();
	}
#ifdef MULTIBYTE_SUPPORT
	else
	    CCRIGHT();
#endif
    }
    return n;
}

/**/
int
downlineorhistory(char **args)
{
    int ocs = zlecs;
    int n = downline(args);
    if (n) {
	int m = zmult, ret;

	zlecs = ocs;
	if (virangeflag || !(zlereadflags & ZLRF_HISTORY))
	    return 1;
	zmult = n;
	ret = downhistory(args);
	zmult = m;
	return ret;
    }
    return 0;
}

/**/
int
vidownlineorhistory(char **args)
{
    int col = lastcol;
    downlineorhistory(args);
    lastcol = col;
    return vifirstnonblank(zlenoargs);
}

/**/
int
downlineorsearch(char **args)
{
    int ocs = zlecs;
    int n = downline(args);
    if (n) {
	int m = zmult, ret;

	zlecs = ocs;
	if (virangeflag || !(zlereadflags & ZLRF_HISTORY))
	    return 1;
	zmult = n;
	ret = historysearchforward(args);
	zmult = m;
	return ret;
    }
    return 0;
}

/**/
int
acceptlineanddownhistory(UNUSED(char **args))
{
    Histent he = quietgethist(histline);

    if (he && (he = movehistent(he, 1, HIST_FOREIGN))) {
	zpushnode(bufstack, ztrdup(he->node.nam));
	stackhist = he->histnum;
    }
    done = 1;
    return 0;
}

/**/
int
downhistory(UNUSED(char **args))
{
    int nodups = isset(HISTIGNOREDUPS);
    if (!zle_goto_hist(histline, zmult, nodups) && isset(HISTBEEP))
	return 1;
    return 0;
}

/*
 * Values remembered for history searches to enable repetition.
 * srch_hl remembers the old value of histline, to see if it's changed
 *   since the last search.
 * srch_cs remembers the old value of zlecs for the same purpose (it is
 *   not use for any other purpose, i.e. does not need to be a valid
 *   index into anything).
 * srch_str is the metafied search string, as extracted from the start
 *   of zleline.
 */
static int histpos, srch_hl, srch_cs = -1;
static char *srch_str;

/**/
int
historysearchbackward(char **args)
{
    Histent he;
    int n = zmult;
    char *str;
    char *zt;

    if (zmult < 0) {
	int ret;
	zmult = -n;
	ret = historysearchforward(args);
	zmult = n;
	return ret;
    }
    if (*args) {
	str = *args;
    } else {
	char *line = zlelineasstring(zleline, zlell, 0, NULL, NULL, 0);
	if (histline == curhist || histline != srch_hl || zlecs != srch_cs ||
	    mark != 0 || strncmp(srch_str, line, histpos) != 0) {
	    free(srch_str);
	    for (histpos = 0; histpos < zlell && !ZC_iblank(zleline[histpos]);
		 histpos++)
		;
	    if (histpos < zlell)
		histpos++;
	    /* ensure we're not on a combining character */
	    CCRIGHTPOS(histpos);
	    /* histpos from now on is an index into the metafied string */
	    srch_str = zlelineasstring(zleline, histpos, 0, NULL, NULL, 0);
	}
	free(line);
	str = srch_str;
    }
    if (!(he = quietgethist(histline)))
	return 1;

    metafy_line();
    while ((he = movehistent(he, -1, hist_skip_flags))) {
	if (isset(HISTFINDNODUPS) && he->node.flags & HIST_DUP)
	    continue;
	zt = GETZLETEXT(he);
	if (zlinecmp(zt, str) < 0 &&
	    (*args || strcmp(zt, zlemetaline) != 0)) {
	    if (--n <= 0) {
		unmetafy_line();
		zle_setline(he);
		srch_hl = histline;
		srch_cs = zlecs;
		return 0;
	    }
	}
    }
    unmetafy_line();
    return 1;
}

/**/
int
historysearchforward(char **args)
{
    Histent he;
    int n = zmult;
    char *str;
    char *zt;

    if (zmult < 0) {
	int ret;
	zmult = -n;
	ret = historysearchbackward(args);
	zmult = n;
	return ret;
    }
    if (*args) {
	str = *args;
    } else {
	char *line = zlelineasstring(zleline, zlell, 0, NULL, NULL, 0);
	if (histline == curhist || histline != srch_hl || zlecs != srch_cs ||
	    mark != 0 || strncmp(srch_str, line, histpos) != 0) {
	    free(srch_str);
	    for (histpos = 0; histpos < zlell && !ZC_iblank(zleline[histpos]);
		 histpos++)
		;
	    if (histpos < zlell)
		histpos++;
	    CCRIGHT();
	    srch_str = zlelineasstring(zleline, histpos, 0, NULL, NULL, 0);
	}
	free(line);
	str = srch_str;
    }
    if (!(he = quietgethist(histline)))
	return 1;

    metafy_line();
    while ((he = movehistent(he, 1, hist_skip_flags))) {
	if (isset(HISTFINDNODUPS) && he->node.flags & HIST_DUP)
	    continue;
	zt = GETZLETEXT(he);
	if (zlinecmp(zt, str) < (he->histnum == curhist) &&
	    (*args || strcmp(zt, zlemetaline) != 0)) {
	    if (--n <= 0) {
		unmetafy_line();
		zle_setline(he);
		srch_hl = histline;
		srch_cs = zlecs;
		return 0;
	    }
	}
    }
    unmetafy_line();
    return 1;
}

/**/
int
beginningofbufferorhistory(char **args)
{
    if (findbol())
	zlecs = 0;
    else
	return beginningofhistory(args);
    return 0;
}

/**/
int
beginningofhistory(UNUSED(char **args))
{
    if (!zle_goto_hist(firsthist(), 0, 0) && isset(HISTBEEP))
	return 1;
    return 0;
}

/**/
int
endofbufferorhistory(char **args)
{
    if (findeol() != zlell)
	zlecs = zlell;
    else
	return endofhistory(args);
    return 0;
}

/**/
int
endofhistory(UNUSED(char **args))
{
    zle_goto_hist(curhist, 0, 0);
    return 0;
}

/**/
int
insertlastword(char **args)
{
    int n, nwords, histstep = -1, wordpos = 0, deleteword = 0, len;
    char *s, *t;
    Histent he = NULL;
    LinkList l = NULL;
    LinkNode node;
    ZLE_STRING_T zs;

    static char *lastinsert;
    static int lasthist, lastpos, lastlen;
    int evhist;

    /*
     * If we have at least one argument, the first is the history
     * step.  The default is -1 (go back).  Repeated calls take
     * a step in this direction.  A value of 0 is allowed and doesn't
     * move the line.
     *
     * If we have two arguments, the second is the position of
     * the word to extract, 1..N.  The default is to use the
     * numeric argument, or the last word if that is not set.
     *
     * If we have three arguments, we reset the history pointer to
     * the current history event before applying the history step.
     */
    if (*args)
    {
	histstep = (int)zstrtol(*args, NULL, 10);
	if (*++args)
	{
	    wordpos = (int)zstrtol(*args, NULL, 10);
	    if (*++args)
		lasthist = curhist;
	}
    }

    fixsuffix();
    metafy_line();
    if (lastinsert && lastlen &&
	lastpos <= zlemetacs &&
	lastlen == zlemetacs - lastpos &&
	memcmp(lastinsert, &zlemetaline[lastpos], lastlen) == 0)
	deleteword = 1;
    else
	lasthist = curhist;
    evhist = histstep ? addhistnum(lasthist, histstep, HIST_FOREIGN) :
	lasthist;

    if (evhist == curhist) {
	/*
	 * The line we are currently editing.  If we are going to
	 * replace an existing word, delete the old one now to avoid
	 * confusion.
	 */
	if (deleteword) {
	    int pos = zlemetacs;
	    zlemetacs = lastpos;
	    foredel(pos - zlemetacs, CUT_RAW);
	    /*
	     * Mark that this has been deleted.
	     * For consistency with history lines, we really ought to
	     * insert it back if the current command later fails. But
	     * - we can't be bothered
	     * - the problem that this can screw up going to other
	     *   lines in the history because we don't update
	     *   the history line isn't really relevant
	     * - you can see what you're copying, dammit, so you
	     *   shouldn't make errors.
	     * Of course, I could have implemented it in the time
	     * it took to say why I haven't.
	     */
	    deleteword = 0;
	}
	/*
	 * Can only happen fail if the line is empty, I hope.
	 * In that case, we don't need to worry about restoring
	 * a deleted word, because that can only have come
	 * from a non-empty line.  I think.
	 */
	if (!(l = bufferwords(NULL, NULL, NULL, 0))) {
	    unmetafy_line();
	    return 1;
	}
	nwords = countlinknodes(l);
    } else {
	/* Some stored line.  By default, search for a non-empty line. */
	while ((he = quietgethist(evhist)) && histstep == -1 && !*args) {
	    if (he->nwords)
		break;
	    evhist = addhistnum(evhist, histstep, HIST_FOREIGN);
	}
	if (!he || !he->nwords) {
	    unmetafy_line();
	    return 1;
	}
	nwords = he->nwords;
    }
    if (wordpos) {
	n = (wordpos > 0) ? wordpos : nwords + wordpos + 1;
    } else if (zmult > 0) {
	n = nwords - (zmult - 1);
    } else {
	n = 1 - zmult;
    }
    if (n < 1 || n > nwords) {
	/*
	 * We can't put in the requested word, but we did find the
	 * history entry, so we remember the position in the history
	 * list.  This avoids getting stuck on a history line with
	 * fewer words than expected.  The cursor location cs
	 * has not changed, and lastinsert is still valid.
	 */
	lasthist = evhist;
	unmetafy_line();
	return 1;
    }
    /*
     * Only remove the old word from the command line if we have
     * successfully found a new one to insert.
     */
    if (deleteword > 0) {
	int pos = zlemetacs;
	zlemetacs = lastpos;
	foredel(pos - zlemetacs, CUT_RAW);
    }
    if (lastinsert) {
	zfree(lastinsert, lastlen);
	lastinsert = NULL;
    }
    if (l) {
	for (node = firstnode(l); --n; incnode(node))
	    ;
	s = (char *)getdata(node);
	t = s + strlen(s);
    } else {
	s = he->node.nam + he->words[2*n-2];
	t = he->node.nam + he->words[2*n-1];
    }

    lasthist = evhist;
    lastpos = zlemetacs;
    /* ignore trailing whitespace */
    lastlen = t - s;
    lastinsert = zalloc(t - s);
    memcpy(lastinsert, s, lastlen);
    n = zmult;
    zmult = 1;

    unmetafy_line();

    zs = stringaszleline(dupstrpfx(s, t - s), 0, &len, NULL, NULL);
    doinsert(zs, len);
    free(zs);
    zmult = n;
    return 0;
}

/**/
void
zle_setline(Histent he)
{
    int remetafy;
    if (zlemetaline) {
	unmetafy_line();
	remetafy = 1;
    } else
	remetafy = 0;
    remember_edits();
    mkundoent();
    histline = he->histnum;

    setline(GETZLETEXT(he), ZSL_COPY|ZSL_TOEND);
    zlecallhook("zle-history-line-set", NULL);
    setlastline();
    clearlist = 1;
    if (remetafy)
	metafy_line();
}

/**/
int
setlocalhistory(UNUSED(char **args))
{
    if (zmod.flags & MOD_MULT) {
	hist_skip_flags = zmult? HIST_FOREIGN : 0;
    } else {
	hist_skip_flags ^= HIST_FOREIGN;
    }
    return 0;
}

/**/
int
zle_goto_hist(int ev, int n, int skipdups)
{
    Histent he = quietgethist(ev);
    char *line = zlelineasstring(zleline, zlell, 0, NULL, NULL, 1);

    if (!he || !(he = movehistent(he, n, hist_skip_flags)))
	return 1;
    if (skipdups && n) {
	n = n < 0? -1 : 1;
	while (he) {
	    int ret;

	    ret = zlinecmp(GETZLETEXT(he), line);
	    if (ret)
		break;
	    he = movehistent(he, n, hist_skip_flags);
	}
    }
    if (!he)
	return 0;
    zle_setline(he);
    return 1;
}

/**/
int
pushline(UNUSED(char **args))
{
    int n = zmult;

    if (n < 0)
	return 1;
    zpushnode(bufstack, zlelineasstring(zleline, zlell, 0, NULL, NULL, 0));
    while (--n)
	zpushnode(bufstack, ztrdup(""));
    if (invicmdmode())
	INCCS();
    stackcs = zlecs;
    *zleline = ZWC('\0');
    zlell = zlecs = 0;
    clearlist = 1;
    return 0;
}

/**/
int
pushlineoredit(char **args)
{
    int ics, ret;
    ZLE_STRING_T s;
    char *hline = hgetline();

    if (zmult < 0)
	return 1;
    if (hline && *hline) {
	ZLE_STRING_T zhline = stringaszleline(hline, 0, &ics, NULL, NULL);

	sizeline(ics + zlell + 1);
	/* careful of overlapping copy */
	for (s = zleline + zlell; --s >= zleline; s[ics] = *s)
	    ;
	ZS_memcpy(zleline, zhline, ics);
	zlell += ics;
	zlecs += ics;
	free(zhline);
    }
    ret = pushline(args);
    if (!isfirstln) {
	errflag |= ERRFLAG_ERROR|ERRFLAG_INT;
	done = 1;
    }
    clearlist = 1;
    return ret;
}

/**/
int
pushinput(char **args)
{
    int i, ret;

    if (zmult < 0)
	return 1;
    zmult += i = !isfirstln;
    ret = pushlineoredit(args);
    zmult -= i;
    return ret;
}

/* Renamed to avoid clash with library function */
/**/
int
zgetline(UNUSED(char **args))
{
    char *s = getlinknode(bufstack);

    if (!s) {
	return 1;
    } else {
	int cc;
	ZLE_STRING_T lineadd = stringaszleline(s, 0, &cc, NULL, NULL);

	spaceinline(cc);
	ZS_memcpy(zleline + zlecs, lineadd, cc);
	zlecs += cc;
	free(s);
	free(lineadd);
	clearlist = 1;
	/* not restoring stackhist as we're inserting into current line */
	stackhist = -1;
    }
    return 0;
}

/**/
int
historyincrementalsearchbackward(char **args)
{
    return doisearch(args, -1, 0);
}

/**/
int
historyincrementalsearchforward(char **args)
{
    return doisearch(args, 1, 0);
}

/**/
int
historyincrementalpatternsearchbackward(char **args)
{
    return doisearch(args, -1, 1);
}

/**/
int
historyincrementalpatternsearchforward(char **args)
{
    return doisearch(args, 1, 1);
}

static struct isrch_spot {
    int hl;			/* This spot's histline */
    int pat_hl;			/* histline where pattern search started */
    unsigned short pos;		/* The search position in our metafied str */
    unsigned short pat_pos;     /* pos where pattern search started */
    unsigned short end_pos;	/* The position of the end of the matched str */
    unsigned short cs;		/* The visible search position to the user */
    unsigned short len;		/* The search string's length */
    unsigned short flags;	/* This spot's flags */
#define ISS_FORWARD	1
#define ISS_NOMATCH_SHIFT 1
} *isrch_spots;

static int max_spot = 0;

/**/
void
free_isrch_spots(void)
{
    zfree(isrch_spots, max_spot * sizeof(*isrch_spots));
    max_spot = 0;
    isrch_spots = NULL;
}

/**/
static void
set_isrch_spot(int num, int hl, int pos, int pat_hl, int pat_pos,
	       int end_pos, int cs, int len, int dir, int nomatch)
{
    if (num >= max_spot) {
	if (!isrch_spots) {
	    isrch_spots = (struct isrch_spot*)
			    zalloc((max_spot = 64) * sizeof *isrch_spots);
	} else {
	    isrch_spots = (struct isrch_spot*)realloc((char*)isrch_spots,
			    (max_spot += 64) * sizeof *isrch_spots);
	}
    }

    isrch_spots[num].hl = hl;
    isrch_spots[num].pos = (unsigned short)pos;
    isrch_spots[num].pat_hl = pat_hl;
    isrch_spots[num].pat_pos = (unsigned short)pat_pos;
    isrch_spots[num].end_pos = (unsigned short)end_pos;
    isrch_spots[num].cs = (unsigned short)cs;
    isrch_spots[num].len = (unsigned short)len;
    isrch_spots[num].flags = (dir > 0? ISS_FORWARD : 0)
			   + (nomatch << ISS_NOMATCH_SHIFT);
}

/**/
static void
get_isrch_spot(int num, int *hlp, int *posp, int *pat_hlp, int *pat_posp,
	       int *end_posp, int *csp, int *lenp, int *dirp, int *nomatch)
{
    *hlp = isrch_spots[num].hl;
    *posp = (int)isrch_spots[num].pos;
    *pat_hlp = isrch_spots[num].pat_hl;
    *pat_posp = (int)isrch_spots[num].pat_pos;
    *end_posp = (int)isrch_spots[num].end_pos;
    *csp = (int)isrch_spots[num].cs;
    *lenp = (int)isrch_spots[num].len;
    *dirp = (isrch_spots[num].flags & ISS_FORWARD)? 1 : -1;
    *nomatch = (int)(isrch_spots[num].flags >> ISS_NOMATCH_SHIFT);
}

/*
 * In pattern search mode, look through the list for a match at, or
 * before or after the given position, according to the direction.
 * Return new position or -1.
 *
 * Note this handles curpos out of range correctly, i.e. curpos < 0
 * never matches when searching backwards and curpos > length of string
 * never matches when searching forwards.
 */
static int
isearch_newpos(LinkList matchlist, int curpos, int dir,
	       int *endmatchpos)
{
    LinkNode node;

    if (dir < 0) {
	for (node = lastnode(matchlist);
	     node != (LinkNode)matchlist; decnode(node)) {
	    Repldata rdata = (Repldata)getdata(node);
	    if (rdata->b <= curpos) {
		*endmatchpos = rdata->e;
		return rdata->b;
	    }
	}
    } else {
	for (node = firstnode(matchlist);
	     node; incnode(node)) {
	    Repldata rdata = (Repldata)getdata(node);
	    if (rdata->b >= curpos) {
		*endmatchpos = rdata->e;
		return rdata->b;
	    }
	}
    }

    return -1;
}

/*
 * Save an isearch buffer from sbuf to sbuf+sbptr
 * into the string *search with length *searchlen.
 * searchlen may be NULL; the string is a NULL-terminated metafied string.
 */
static void
save_isearch_buffer(char *sbuf, int sbptr,
		    char **search, int *searchlen)
{
    if (*search)
	free(*search);
    *search = zalloc(sbptr+1);
    memcpy(*search, sbuf, sbptr);
    if (searchlen)
	*searchlen = sbptr;
    (*search)[sbptr] = '\0';
}

#define ISEARCH_PROMPT		"XXXXXXX XXX-i-search: "
#define FAILING_TEXT		"failing"
#define INVALID_TEXT		"invalid"
#define BAD_TEXT_LEN		7
#define NORM_PROMPT_POS		(BAD_TEXT_LEN+1)
#define FIRST_SEARCH_CHAR	(NORM_PROMPT_POS + 14)

/**/
int isearch_active, isearch_startpos, isearch_endpos;

/**/
static int
doisearch(char **args, int dir, int pattern)
{
    /* The full search buffer, including space for all prompts */
    char *ibuf = zhalloc(80);
    /*
     * The part of the search buffer with the search string.
     * This is a normal metafied string.
     */
    char *sbuf = ibuf + FIRST_SEARCH_CHAR;
    /* The previous line shown to the user */
    char *last_line = NULL;
    /* Text of the history line being examined */
    char *zt;
    /*
     * sbptr: index into sbuf.
     * top_spot: stack index into the "isrch_spot" stack.
     * sibuf: allocation size for ibuf
     */
    int sbptr = 0, top_spot = 0, sibuf = 80;
    /*
     * nomatch = 1: failing isearch
     * nomatch = 2: invalid pattern
     * skip_line: finished with current line, skip to next
     * skip_pos: keep current line but try before/after current position.
     */
    int nomatch = 0, skip_line = 0, skip_pos = 0;
    /*
     * odir: original search direction
     * sens: limit for zlinecmp to allow (3) or disallow (1) lower case
     *       matching upper case.
     */
    int odir = dir, sens = zmult == 1 ? 3 : 1;
    /*
     * hl: the number of the history line we are looking at
     * pos: the character position into it.  On backward matches the
     *      cursor will be set to this; on forward matches to the end
     *      of the matched string
     */
    int hl = histline, pos;
    /*
     * The value of hl and pos at which the last pattern match
     * search started.  We need to record these because there's
     * a pathology with pattern matching.  Here's an example.  Suppose
     * the history consists of:
     *  echo '*OH NO*'
     *  echo '\n'
     *  echo "*WHAT?*"
     *  <...backward pattern search starts here...>
     * The user types "\".  As there's nothing after it it's treated
     * literally (and I certainly don't want to change that).  This
     * goes to the second line.  Then the user types "*".  This
     * ought to match the "*" in the line immediately before where the
     * search started.  However, unless we return to that line for the
     * new search it will instead carry on to the first line.  This is
     * different from straight string matching where we never have
     * to backtrack.
     *
     * I think these need resetting to the current hl and pos when
     * we start a new search or repeat a search.  It seems to work,
     * anyway.
     *
     * We could optimize this more, but I don't think there's a lot
     * of point.  (Translation:  it's difficult.)
     */
    int pat_hl = hl, pat_pos;
    /*
     * This is the flag that we need to revert the positions to
     * the above for the next pattern search.
     */
    int revert_patpos = 0;
    /*
     * Another nasty feature related to the above.  When
     * we revert the position, we might advance the search to
     * the same line again.  When we do this the test for ignoring
     * duplicates may trigger.  This flag indicates that in this
     * case it's OK.
     */
    int dup_ok = 0;
    /*
     * End position of the match.
     * When forward matching, this is the position for the cursor.
     * When backward matching, the cursor position is pos.
     */
    int end_pos = 0;
    /*
     * savekeys records the unget buffer, so that if we have arguments
     * they don't pollute the input.
     * feep indicates we should feep.  This is a well-known word
     * meaning "to indicate an error in the zsh line editor".
     */
    int savekeys = -1, feep = 0;
    /* Flag that we are at an old position, no need to search again */
    int nosearch = 0;
    /* Command read as input:  we don't read characters directly. */
    Thingy cmd;
    /* Save the keymap if necessary */
    char *okeymap;
    /* The current history entry, corresponding to hl */
    Histent he;
    /* When pattern matching, the compiled pattern */
    Patprog patprog = NULL;
    /* When pattern matching, the list of match positions */
    LinkList matchlist = NULL;
    /*
     * When we exit isearching this may be a zle command to
     * execute.  We save it and execute it after unmetafying the
     * command line.
     */
    ZleIntFunc exitfn = (ZleIntFunc)0;
    /*
     * Flag that the search was aborted.
     */
    int aborted = 0;

    if (!(he = quietgethist(hl)))
	return 1;

    selectlocalmap(isearch_keymap);

    clearlist = 1;

    if (*args) {
	int len;
	char *arg;
	savekeys = kungetct;
	arg = getkeystring(*args, &len, GETKEYS_BINDKEY, NULL);
	ungetbytes(arg, len);
    }

    strcpy(ibuf, ISEARCH_PROMPT);
    /* careful with fwd/bck: we don't want the NULL copied */
    memcpy(ibuf + NORM_PROMPT_POS, (dir == 1) ? "fwd" : "bck", 3);
    okeymap = ztrdup(curkeymapname);
    selectkeymap("main", 1);

    metafy_line();
    remember_edits();
    zt = GETZLETEXT(he);
    pat_pos = pos = zlemetacs;
    for (;;) {
	/* Remember the current values in case search fails (doesn't push). */
	set_isrch_spot(top_spot, hl, pos, pat_hl, pat_pos, end_pos,
		       zlemetacs, sbptr, dir, nomatch);
	if (sbptr == 1 && sbuf[0] == '^') {
	    zlemetacs = 0;
    	    nomatch = 0;
	    statusline = ibuf + NORM_PROMPT_POS;
	} else if (sbptr > 0) {
	    /* The matched text, used as flag that we matched */
	    char *t = NULL;
	    last_line = zt;

	    sbuf[sbptr] = '\0';
	    if (pattern && !patprog && !nosearch) {
		/* avoid too much heap use, can get heavy round here... */
		char *patbuf = ztrdup(sbuf);
		char *patstring;
		/*
		 * Do not use static pattern buffer (PAT_STATIC) since we
		 * call zle hooks, which might call other pattern
		 * functions.  Use PAT_ZDUP because we re-use the pattern
		 * in subsequent loops, so we can't pushheap/popheap.
		 * Use PAT_NOANCH because we don't need the match anchored
		 * to the end, even if it is at the start.
		 */
		int patflags = PAT_ZDUP|PAT_NOANCH;
		if (sbuf[0] == '^') {
		    /*
		     * We'll handle the anchor later when
		     * we call into the globbing code.
		     */
		    patstring = patbuf + 1;
		} else {
		    /* Scanning for multiple matches per line */
		    patflags |= PAT_SCAN;
		    patstring = patbuf;
		}
		if (sens == 3)
		    patflags |= PAT_LCMATCHUC;
		tokenize(patstring);
		remnulargs(patstring);
		patprog = patcompile(patstring, patflags, NULL);
		free(patbuf);
		if (matchlist) {
		    freematchlist(matchlist);
		    matchlist = NULL;
		}
		if (patprog) {
		    revert_patpos = 1;
		    skip_pos = 0;
		} else {
		    if (nomatch != 2) {
			handlefeep(zlenoargs);
			nomatch = 2;
		    }
		    /* indicate "invalid" in status line */
		    memcpy(ibuf, INVALID_TEXT, BAD_TEXT_LEN);
		    statusline = ibuf;
		}
	    }
	    /*
	     * skip search if pattern compilation failed, or
	     * if we back somewhere we already searched.
	     */
	    while ((!pattern || patprog) && !nosearch) {
		if (patprog) {
		    if (revert_patpos) {
			/*
			 * Search from where the previous
			 * search started; see note above.
			 * This is down here within the loop because of
			 * the "nosearch" optimisation.
			 */
			revert_patpos = 0;
			dup_ok = 1;
			he = quietgethist(hl = pat_hl);
			zt = GETZLETEXT(he);
			pos = pat_pos;
		    }
		    /*
		     * We are pattern matching against the current
		     * line.  If anchored at the start, this is
		     * easy; a single test suffices.
		     *
		     * Otherwise, our strategy is to retrieve a linked
		     * list of all matches within the current line and
		     * scan through it as appropriate.  This isn't
		     * actually significantly more efficient, but
		     * it is algorithmically easier since we just
		     * need a single one-off line-matching interface
		     * to the pattern code.  We use a variant of
		     * the code used for replacing within parameters
		     * which for historical reasons is in glob.c rather
		     * than pattern.c.
		     *
		     * The code for deciding whether to skip something
		     * is a bit icky but that sort of code always is.
		     */
		    if (!skip_line) {
			if (sbuf[0] == '^') {
			    /*
			     * skip_pos applies to the whole line in
			     * this mode.
			     */
			    if (!skip_pos &&
				pattryrefs(patprog, zt, -1, -1, NULL, 0,
					   NULL, NULL, &end_pos))
				t = zt;
			} else {
			    if (!matchlist && !skip_pos) {
				if (!getmatchlist(zt, patprog, &matchlist) ||
				    !firstnode(matchlist)) {
				    if (matchlist) {
					freematchlist(matchlist);
					matchlist = NULL;
				    }
				}
			    }
			    if (matchlist) {
				int newpos;
				if (!skip_pos) {
				    /* OK to match at current pos */
				    newpos = pos;
				} else {
				    if (dir < 0)
					newpos = pos - 1;
				    else
					newpos = pos + 1;
				}
				newpos = isearch_newpos(matchlist, newpos,
							dir, &end_pos);
				/* need a new list next time if off the end */
				if (newpos < 0) {
				    freematchlist(matchlist);
				    matchlist = NULL;
				} else {
				    t = zt + newpos;
				}
			    }
			}
		    }
		    skip_pos = 0;
		} else {
		    /*
		     * If instructed, move past a match position:
		     * backwards if searching backwards (skipping
		     * the line if we're at the start), forwards
		     * if searching forwards (skipping a line if we're
		     * at the end).
		     */
		    if (skip_pos) {
			if (dir < 0) {
			    if (pos == 0)
				skip_line = 1;
			    else
				pos = backwardmetafiedchar(zlemetaline,
							   zlemetaline + pos,
							   NULL) - zlemetaline;
			} else if (sbuf[0] != '^') {
			    if (pos >= (int)strlen(zt) - 1)
				skip_line = 1;
			    else
				pos += 1;
			} else
			    skip_line = 1;
			skip_pos = 0;
		    }
		    /*
		     * First search for a(nother) match within the
		     * current line, unless we've been told to skip it.
		     */
		    if (!skip_line) {
			if (sbuf[0] == '^') {
			    if (zlinecmp(zt, sbuf + 1) < sens)
				t = zt;
			} else
			    t = zlinefind(zt, pos, sbuf, dir, sens);
			if (t)
			    end_pos = (t - zt) + sbptr - (sbuf[0] == '^');
		    }
		}
		if (t) {
		    pos = t - zt;
		    break;
		}
		/*
		 * If not found within that line, move through
		 * the history to try again.
		 */
		if (!(zlereadflags & ZLRF_HISTORY)
		 || !(he = movehistent(he, dir, hist_skip_flags))) {
		    if (sbptr == (int)isrch_spots[top_spot-1].len
		     && (isrch_spots[top_spot-1].flags >> ISS_NOMATCH_SHIFT))
			top_spot--;
		    get_isrch_spot(top_spot, &hl, &pos, &pat_hl, &pat_pos,
				   &end_pos, &zlemetacs, &sbptr, &dir,
				   &nomatch);
		    if (nomatch != 1) {
			feep = 1;
			nomatch = 1;
		    }
		    he = quietgethist(hl);
		    zt = GETZLETEXT(he);
		    skip_line = 0;
		    /* indicate "failing" in status line */
		    memcpy(ibuf, nomatch == 2 ? INVALID_TEXT :FAILING_TEXT,
			   BAD_TEXT_LEN);
		    statusline = ibuf;
		    break;
		}
		hl = he->histnum;
		zt = GETZLETEXT(he);
		pos = (dir == 1) ? 0 : strlen(zt);
		if (dup_ok)
		    skip_line = 0;
		else
		    skip_line = isset(HISTFINDNODUPS)
			? !!(he->node.flags & HIST_DUP)
			: !strcmp(zt, last_line);
	    }
	    dup_ok = 0;
	    /*
	     * If we matched above (t set), set the new line.
	     * If we didn't, but are here because we are on a previous
	     * match (nosearch set and nomatch not, set the line again).
	     */
	    if (t || (nosearch && !nomatch)) {
		zle_setline(he);
		if (dir == 1)
		    zlemetacs = end_pos;
		else
		    zlemetacs = pos;
		statusline = ibuf + NORM_PROMPT_POS;
		nomatch = 0;
	    }
	} else {
	    top_spot = 0;
    	    nomatch = 0;
	    statusline = ibuf + NORM_PROMPT_POS;
	}
	nosearch = 0;
	if (feep) {
	    handlefeep(zlenoargs);
	    feep = 0;
	}
	sbuf[sbptr] = '_';
	sbuf[sbptr+1] = '\0';
	if (!nomatch && sbptr && (sbptr > 1 || sbuf[0] != '^')) {
#ifdef MULTIBYTE_SUPPORT
	    int charpos = 0, charcount = 0, ret;
	    wint_t wc;
	    mbstate_t mbs;

	    /*
	     * Count unmetafied character positions for the
	     * start and end of the match for the benefit of
	     * highlighting.
	     */
	    memset(&mbs, 0, sizeof(mbs));
	    while (charpos < end_pos) {
		ret = mb_metacharlenconv_r(zlemetaline + charpos, &wc, &mbs);
		if (ret <= 0) /* Unrecognised, treat as single char */
		    ret = 1;
		if (charpos <= pos && pos < charpos + ret)
		    isearch_startpos = charcount;
		charcount++;
		charpos += ret;
	    }
	    isearch_endpos = charcount;
#else
	    isearch_startpos = ztrsub(zlemetaline + pos, zlemetaline);
	    isearch_endpos = ztrsub(zlemetaline + end_pos,
				    zlemetaline);
#endif
	    isearch_active = 1;
	} else
	    isearch_active = 0;
    ref:
	zlecallhook("zle-isearch-update", NULL);
	redrawhook();
	zrefresh();
	if (!(cmd = getkeycmd()) || cmd == Th(z_sendbreak)) {
	    int i;
	    aborted = 1;
	    save_isearch_buffer(sbuf, sbptr,
				&previous_aborted_search, NULL);
	    get_isrch_spot(0, &hl, &pos, &pat_hl, &pat_pos, &end_pos,
			   &i, &sbptr, &dir, &nomatch);
	    he = quietgethist(hl);
	    zle_setline(he);
	    zt = GETZLETEXT(he);
	    zlemetacs = i;
	    break;
	}
	if(cmd == Th(z_clearscreen)) {
	    clearscreen(zlenoargs);
	    goto ref;
	} else if(cmd == Th(z_redisplay)) {
	    redisplay(zlenoargs);
	    goto ref;
	} else if(cmd == Th(z_vicmdmode)) {
	    if(selectkeymap(invicmdmode() ? "main" : "vicmd", 0))
		feep = 1;
	    goto ref;
       } else if (cmd == Th(z_vibackwarddeletechar) ||
		  cmd == Th(z_backwarddeletechar) ||
		  cmd == Th(z_vibackwardkillword) ||
		  cmd == Th(z_backwardkillword) ||
		  cmd == Th(z_backwarddeleteword)) {
	    int only_one = (cmd == Th(z_vibackwarddeletechar) ||
			    cmd == Th(z_backwarddeletechar));
	    int old_sbptr = sbptr;
	    if (top_spot) {
		for (;;) {
		    get_isrch_spot(--top_spot, &hl, &pos, &pat_hl,
				   &pat_pos,  &end_pos, &zlemetacs,
				   &sbptr, &dir, &nomatch);
		    if (only_one || !top_spot || old_sbptr != sbptr)
			break;
		}
		freepatprog(patprog);
		patprog = NULL;
		nosearch = 1;
		skip_pos = 0;
	    } else
		feep = 1;
	    if (nomatch) {
		memcpy(ibuf, nomatch == 2 ? INVALID_TEXT : FAILING_TEXT,
		       BAD_TEXT_LEN);
		statusline = ibuf;
		skip_pos = 1;
	    }
	    he = quietgethist(hl);
	    zt = GETZLETEXT(he);
	    /*
	     * Set the line for the cases where we won't go past
	     * the usual line-setting logic:  if we're not on a match,
	     * or if we don't have enough to search for.
	     */
	    if (nomatch || !sbptr || (sbptr == 1 && sbuf[0] == '^')) {
		int i = zlemetacs;
		zle_setline(he);
		zlemetacs = i;
	    }
	    memcpy(ibuf + NORM_PROMPT_POS,
		   (dir == 1) ? "fwd" : "bck", 3);
	    continue;
	} else if(cmd == Th(z_acceptandhold)) {
	    exitfn = acceptandhold;
	    break;
	} else if(cmd == Th(z_acceptandinfernexthistory)) {
	    exitfn = acceptandinfernexthistory;
	    break;
	} else if(cmd == Th(z_acceptlineanddownhistory)) {
	    exitfn = acceptlineanddownhistory;
	    break;
	} else if(cmd == Th(z_acceptline)) {
	    exitfn = acceptline;
	    break;
	} else if(cmd == Th(z_historyincrementalsearchbackward) ||
		  cmd == Th(z_historyincrementalpatternsearchbackward)) {
	    pat_hl = hl;
	    pat_pos = pos;
	    set_isrch_spot(top_spot++, hl, pos, pat_hl, pat_pos, end_pos,
			   zlemetacs, sbptr, dir, nomatch);
	    if (dir != -1)
		dir = -1;
	    else
		skip_pos = 1;
	    goto rpt;
	} else if(cmd == Th(z_historyincrementalsearchforward) ||
		  cmd == Th(z_historyincrementalpatternsearchforward)) {
	    pat_hl = hl;
	    pat_pos = pos;
	    set_isrch_spot(top_spot++, hl, pos, pat_hl, pat_pos, end_pos,
			   zlemetacs, sbptr, dir, nomatch);
	    if (dir != 1)
		dir = 1;
	    else
		skip_pos = 1;
	    goto rpt;
	} else if(cmd == Th(z_virevrepeatsearch)) {
	    pat_hl = hl;
	    pat_pos = pos;
	    set_isrch_spot(top_spot++, hl, pos, pat_hl, pat_pos, end_pos,
			   zlemetacs, sbptr, dir, nomatch);
	    dir = -odir;
	    skip_pos = 1;
	    goto rpt;
	} else if(cmd == Th(z_virepeatsearch)) {
	    pat_hl = hl;
	    pat_pos = pos;
	    set_isrch_spot(top_spot++, hl, pos, pat_hl, pat_pos, end_pos,
			   zlemetacs, sbptr, dir, nomatch);
	    dir = odir;
	    skip_pos = 1;
	rpt:
	    if (!sbptr && previous_search_len && dir == odir) {
		if (previous_search_len > sibuf - FIRST_SEARCH_CHAR - 2) {
		    ibuf = hrealloc((char *)ibuf, sibuf,
				    (sibuf + previous_search_len));
		    sbuf = ibuf + FIRST_SEARCH_CHAR;
		    sibuf += previous_search_len;
		}
		memcpy(sbuf, previous_search, sbptr = previous_search_len);
	    }
	    memcpy(ibuf + NORM_PROMPT_POS, (dir == 1) ? "fwd" : "bck", 3);
	    continue;
	} else if(cmd == Th(z_viquotedinsert) ||
	    	cmd == Th(z_quotedinsert)) {
	    if(cmd == Th(z_viquotedinsert)) {
		sbuf[sbptr] = '^';
		sbuf[sbptr+1] = '\0';
		zrefresh();
	    }
	    if (getfullchar(0) == ZLEEOF)
		feep = 1;
	    else
		goto ins;
	} else if (cmd == Th(z_bracketedpaste)) {
	    char *paste = bracketedstring();
	    set_isrch_spot(top_spot++, hl, pos, pat_hl, pat_pos, end_pos,
			   zlemetacs, sbptr, dir, nomatch);
	    size_t pastelen = strlen(paste);
	    if (sbptr + pastelen >= sibuf - FIRST_SEARCH_CHAR - 2) {
		int oldsize = sibuf;
		sibuf += (pastelen >= sibuf) ? pastelen + 1 : sibuf;
		ibuf = hrealloc(ibuf, oldsize, sibuf);
		sbuf = ibuf + FIRST_SEARCH_CHAR;
	    }
	    strcpy(sbuf + sbptr, paste);
	    sbptr += pastelen;
	    freepatprog(patprog);
	    patprog = NULL;
	    free(paste);
	} else if (cmd == Th(z_acceptsearch)) {
	    break;
	} else {
	    if(cmd == Th(z_selfinsertunmeta)) {
		fixunmeta();
	    } else if (cmd == Th(z_magicspace)) {
		fixmagicspace();
	    } else if (cmd == Th(z_selfinsert)) {
#ifdef MULTIBYTE_SUPPORT
		if (!lastchar_wide_valid)
		    if (getrestchar(lastchar, NULL, NULL) == WEOF) {
			handlefeep(zlenoargs);
			continue;
		    }
#else
		;
#endif
	    } else {
		ungetkeycmd();
		if (cmd == Th(z_sendbreak)) {
		    aborted = 1;
		    save_isearch_buffer(sbuf, sbptr,
					&previous_aborted_search, NULL);
		    sbptr = 0;
		}
		break;
	    }
	ins:
	    if (sbptr == PATH_MAX) {
		feep = 1;
		continue;
	    }
	    set_isrch_spot(top_spot++, hl, pos, pat_hl, pat_pos, end_pos,
			   zlemetacs, sbptr, dir, nomatch);
	    if (sbptr >= sibuf - FIRST_SEARCH_CHAR - 2 
#ifdef MULTIBYTE_SUPPORT
		- 2 * (int)MB_CUR_MAX
#endif
		) {
		ibuf = hrealloc(ibuf, sibuf, sibuf * 2);
		sbuf = ibuf + FIRST_SEARCH_CHAR;
		sibuf *= 2;
	    }
	    /*
	     * We've supposedly arranged above that lastchar_wide is
	     * always valid at this point.
	     */
	    sbptr += zlecharasstring(LASTFULLCHAR, sbuf + sbptr);
	    freepatprog(patprog);
	    patprog = NULL;
	}
	if (feep)
	    handlefeep(zlenoargs);
	feep = 0;
    }
    if (sbptr) {
	save_isearch_buffer(sbuf, sbptr,
			    &previous_search, &previous_search_len);
    }
    statusline = NULL;
    unmetafy_line();
    zlecallhook("zle-isearch-exit", NULL);
    redrawhook();
    if (exitfn)
	exitfn(zlenoargs);
    selectkeymap(okeymap, 1);
    zsfree(okeymap);
    if (matchlist)
	freematchlist(matchlist);
    freepatprog(patprog);
    isearch_active = 0;
    /*
     * Don't allow unused characters provided as a string to the
     * widget to overflow and be used as separated commands.
     */
    if (savekeys >= 0 && kungetct > savekeys)
	kungetct = savekeys;

    selectlocalmap(NULL);

    return aborted ? 3 : nomatch;
}

static Histent
infernexthist(Histent he, UNUSED(char **args))
{
    metafy_line();
    for (he = movehistent(he, -2, HIST_FOREIGN);
	 he; he = movehistent(he, -1, HIST_FOREIGN)) {
	if (!zlinecmp(GETZLETEXT(he), zlemetaline)) {
	    unmetafy_line();
	    return movehistent(he, 1, HIST_FOREIGN);
	}
    }
    unmetafy_line();
    return NULL;
}

/**/
int
acceptandinfernexthistory(char **args)
{
    Histent he;

    if (!(he = infernexthist(hist_ring, args)))
	return 1;
    zpushnode(bufstack, ztrdup(he->node.nam));
    done = 1;
    stackhist = he->histnum;
    return 0;
}

/**/
int
infernexthistory(char **args)
{
    Histent he = quietgethist(histline);

    if (!he || !(he = infernexthist(he, args)))
	return 1;
    zle_setline(he);
    return 0;
}

/**/
int
vifetchhistory(UNUSED(char **args))
{
    if (zmult < 0)
	return 1;
    if (histline == curhist) {
	if (!(zmod.flags & MOD_MULT)) {
	    zlecs = zlell;
	    zlecs = findbol();
	    return 0;
	}
    }
    if (!zle_goto_hist((zmod.flags & MOD_MULT) ? zmult : curhist, 0, 0) &&
	isset(HISTBEEP)) {
	return 1;
    }
    return 0;
}

/* the last vi search */

static char *visrchstr, *vipenultsrchstr;
static int visrchsense;

/**/
static int
getvisrchstr(void)
{
    char *sbuf = zhalloc(80);
    int sptr = 1, ret = 0, ssbuf = 80, feep = 0;
    Thingy cmd;
    char *okeymap = ztrdup(curkeymapname);

    if (vipenultsrchstr) {
	zsfree(vipenultsrchstr);
	vipenultsrchstr = NULL;
    }

    if (visrchstr) {
	vipenultsrchstr = visrchstr;
	visrchstr = NULL;
    }
    clearlist = 1;
    statusline = sbuf;
    sbuf[0] = (visrchsense == -1) ? '?' : '/';
    selectkeymap("main", 1);
    while (sptr) {
	sbuf[sptr] = '_';
	sbuf[sptr+1] = '\0';
	zrefresh();
	if (!(cmd = getkeycmd()) || cmd == Th(z_sendbreak)) {
	    ret = 0;
	    break;
	}
	if(cmd == Th(z_magicspace)) {
	    fixmagicspace();
	    cmd = Th(z_selfinsert);
	}
	if(cmd == Th(z_redisplay)) {
	    redisplay(zlenoargs);
	} else if(cmd == Th(z_clearscreen)) {
	    clearscreen(zlenoargs);
	} else if(cmd == Th(z_acceptline) ||
	    	cmd == Th(z_vicmdmode)) {
	    sbuf[sptr] = ZWC('\0');
	    visrchstr = ztrdup(sbuf+1);
	    if (!strlen(visrchstr)) {
	        zsfree(visrchstr);
		visrchstr = ztrdup(vipenultsrchstr);
	    }
	    ret = 1;
	    sptr = 0;
	} else if(cmd == Th(z_backwarddeletechar) ||
		  cmd == Th(z_vibackwarddeletechar)) {
	    sptr = backwardmetafiedchar(sbuf+1, sbuf+sptr, NULL) - sbuf;
	} else if(cmd == Th(z_backwardkillword) ||
		  cmd == Th(z_vibackwardkillword)) {
	    convchar_t cc;
	    char *newpos;
	    while (sptr != 1) {
		newpos = backwardmetafiedchar(sbuf+1, sbuf+sptr, &cc);
		if (!ZC_iblank(cc))
		    break;
		sptr = newpos - sbuf;
	    }
	    if (sptr > 1) {
		newpos = backwardmetafiedchar(sbuf+1, sbuf+sptr, &cc);
		if (ZC_iident(cc)) {
		    for (;;) {
			sptr = newpos - sbuf;
			if (sptr == 1)
			    break;
			newpos = backwardmetafiedchar(sbuf+1, sbuf+sptr, &cc);
			if (!ZC_iident(cc))
			    break;
		    }
		} else {
		    for (;;) {
			sptr = newpos - sbuf;
			if (sptr == 1)
			    break;
			newpos = backwardmetafiedchar(sbuf+1, sbuf+sptr, &cc);
			if (ZC_iident(cc) || ZC_iblank(cc))
			    break;
		    }
		}
	    }
	} else if(cmd == Th(z_viquotedinsert) || cmd == Th(z_quotedinsert)) {
	    if(cmd == Th(z_viquotedinsert)) {
		sbuf[sptr] = '^';
		zrefresh();
	    }
	    if (getfullchar(0) == ZLEEOF)
		feep = 1;
	    else
		goto ins;
	} else if(cmd == Th(z_selfinsertunmeta) || cmd == Th(z_selfinsert)) {
	    if(cmd == Th(z_selfinsertunmeta)) {
		fixunmeta();
	    } else {
#ifdef MULTIBYTE_SUPPORT
		if (!lastchar_wide_valid)
		    if (getrestchar(lastchar, NULL, NULL) == WEOF) {
			handlefeep(zlenoargs);
			continue;
		    }
#else
		;
#endif
	    }
	  ins:
	    if (sptr == ssbuf - 1) {
		char *newbuf = (char *)zhalloc((ssbuf *= 2));
		strcpy(newbuf, sbuf);
		statusline = sbuf = newbuf;
	    }
	    sptr += zlecharasstring(LASTFULLCHAR, sbuf + sptr);
	} else {
	    feep = 1;
	}
	if (feep)
	    handlefeep(zlenoargs);
	feep = 0;
    }
    statusline = NULL;
    selectkeymap(okeymap, 1);
    zsfree(okeymap);
    return ret;
}

/**/
int
vihistorysearchforward(char **args)
{
    if (*args) {
	int ose = visrchsense, ret;
	char *ost = visrchstr;

	visrchsense = 1;
	visrchstr = *args;
	ret = virepeatsearch(zlenoargs);
	visrchsense = ose;
	visrchstr = ost;
	return ret;
    }
    visrchsense = 1;
    if (getvisrchstr())
	return virepeatsearch(zlenoargs);
    return 1;
}

/**/
int
vihistorysearchbackward(char **args)
{
    if (*args) {
	int ose = visrchsense, ret;
	char *ost = visrchstr;

	visrchsense = -1;
	visrchstr = *args;
	ret = virepeatsearch(zlenoargs);
	visrchsense = ose;
	visrchstr = ost;
	return ret;
    }
    visrchsense = -1;
    if (getvisrchstr())
	return virepeatsearch(zlenoargs);
    return 1;
}

/**/
int
virepeatsearch(UNUSED(char **args))
{
    Histent he;
    int n = zmult;
    char *zt;

    if (!visrchstr)
	return 1;
    if (zmult < 0) {
	n = -n;
	visrchsense = -visrchsense;
    }
    if (!(he = quietgethist(histline)))
	return 1;
    metafy_line();
    while ((he = movehistent(he, visrchsense, hist_skip_flags))) {
	if (isset(HISTFINDNODUPS) && he->node.flags & HIST_DUP)
	    continue;
	zt = GETZLETEXT(he);
	if (zlinecmp(zt, zlemetaline) &&
	    (*visrchstr == '^' ? strpfx(visrchstr + 1, zt) :
	     zlinefind(zt, 0, visrchstr, 1, 1) != 0)) {
	    if (--n <= 0) {
		unmetafy_line();
		zle_setline(he);
		return 0;
	    }
	}
    }
    unmetafy_line();
    return 1;
}

/**/
int
virevrepeatsearch(char **args)
{
    int ret;
    visrchsense = -visrchsense;
    ret = virepeatsearch(args);
    visrchsense = -visrchsense;
    return ret;
}

/* Extra function added by A.R. Iano-Fletcher.	*/
/*The extern variable "zlecs" is the position of the cursor. */
/* history-beginning-search-backward */

/**/
int
historybeginningsearchbackward(char **args)
{
    Histent he;
    int cpos = zlecs;		/* save cursor position */
    int n = zmult;
    char *zt;

    if (zmult < 0) {
	int ret;
	zmult = -n;
	ret = historybeginningsearchforward(args);
	zmult = n;
	return ret;
    }
    if (!(he = quietgethist(histline)))
	return 1;
    metafy_line();
    while ((he = movehistent(he, -1, hist_skip_flags))) {
	int tst;
	char sav;
	if (isset(HISTFINDNODUPS) && he->node.flags & HIST_DUP)
	    continue;
	zt = GETZLETEXT(he);
	sav = zlemetaline[zlemetacs];
	zlemetaline[zlemetacs] = '\0';
	tst = zlinecmp(zt, zlemetaline);
	zlemetaline[zlemetacs] = sav;
	if (tst < 0 && zlinecmp(zt, zlemetaline)) {
	    if (--n <= 0) {
		unmetafy_line();
		zle_setline(he);
		zlecs = cpos;
		CCRIGHT();
		return 0;
	    }
	}
    }
    unmetafy_line();
    return 1;
}

/* Extra function added by A.R. Iano-Fletcher.	*/

/* history-beginning-search-forward */
/**/
int
historybeginningsearchforward(char **args)
{
    Histent he;
    int cpos = zlecs;		/* save cursor position */
    int n = zmult;
    char *zt;

    if (zmult < 0) {
	int ret;
	zmult = -n;
	ret = historybeginningsearchbackward(args);
	zmult = n;
	return ret;
    }
    if (!(he = quietgethist(histline)))
	return 1;
    metafy_line();
    while ((he = movehistent(he, 1, hist_skip_flags))) {
	char sav;
	int tst;
	if (isset(HISTFINDNODUPS) && he->node.flags & HIST_DUP)
	    continue;
	zt = GETZLETEXT(he);
	sav = zlemetaline[zlemetacs];
	zlemetaline[zlemetacs] = '\0';
	tst = zlinecmp(zt, zlemetaline) < (he->histnum == curhist);
	zlemetaline[zlemetacs] = sav;
	if (tst && zlinecmp(zt, zlemetaline)) {
	    if (--n <= 0) {
		unmetafy_line();
		zle_setline(he);
		zlecs = cpos;
		CCRIGHT();
		return 0;
	    }
	}
    }
    unmetafy_line();
    return 1;
}
