/*
 * zle_utils.c - miscellaneous line editor utilities
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
#include "zle_utils.pro"

/* Primary cut buffer */

/**/
struct cutbuffer cutbuf;

/* Emacs-style kill buffer ring */

/**/
struct cutbuffer *kring;
/**/
int kringsize, kringnum;

/* Vi named cut buffers.  0-25 are the named buffers "a to "z, and *
 * 26-35 are the numbered buffer stack "0 to "9.                   */

/**/
struct cutbuffer vibuf[36];

/* the line before last mod (for undo purposes) */

/**/
ZLE_STRING_T lastline;
/**/
int lastlinesz, lastll, lastcs;

/* size of line buffer */

/**/
int linesz;

/* make sure that the line buffer has at least sz chars */

/**/
void
sizeline(int sz)
{
    int cursz = (zlemetaline != NULL) ? metalinesz : linesz;

    while (sz > cursz) {
	if (cursz < 256)
	    cursz = 256;
	else
	    cursz *= 4;

	if (zlemetaline != NULL) {
	    /* One spare character for the NULL */
	    zlemetaline = realloc(zlemetaline, cursz + 1);
	} else {
	    /* One spare character for the NULL, one for the newline */
	    zleline =
		(ZLE_STRING_T)realloc(zleline,
				      (cursz + 2) * ZLE_CHAR_SIZE);
	}
    }

    if (zlemetaline != NULL)
	metalinesz = cursz;
    else
	linesz = cursz;
}

/*
 * Insert a character, called from main shell.
 * Note this always operates on the metafied multibyte version of the
 * line.
 */

/**/
mod_export void
zleaddtoline(int chr)
{
    spaceinline(1);
    zlemetaline[zlemetacs++] = chr;
}

/*
 * Convert a line editor character to a possibly multibyte character
 * in a metafied string.  To be safe buf should have space for at least
 * 2 * MB_CUR_MAX chars for multibyte mode and 2 otherwise.  Returns the
 * length of the string added.
 */

/**/
int
zlecharasstring(ZLE_CHAR_T inchar, char *buf)
{
#ifdef MULTIBYTE_SUPPORT
    int ret;
    char *ptr;

#ifdef __STDC_ISO_10646__
    if (ZSH_INVALID_WCHAR_TEST(inchar)) {
	buf[0] = ZSH_INVALID_WCHAR_TO_CHAR(inchar);
	ret = 1;
    } else
#endif
    {
	ret = wctomb(buf, inchar);
	if (ret <= 0) {
	    /* Ick. */
	    buf[0] = '?';
	    return 1;
	}
    }
    ptr = buf + ret - 1;
    for (;;) {
	if (imeta(*ptr)) {
	    char *ptr2 = buf + ret - 1;
	    for (;;) {
		ptr2[1] = ptr2[0];
		if (ptr2 == ptr)
		    break;
		ptr2--;
	    }
	    *ptr = Meta;
	    ptr[1] ^= 32;
	    ret++;
	}

	if (ptr == buf)
	    return ret;
	ptr--;
    }
#else
    if (imeta(inchar)) {
	buf[0] = Meta;
	buf[1] = inchar ^ 32;
	return 2;
    } else {
	buf[0] = inchar;
	return 1;
    }
#endif
}

/*
 * Input: a line in internal zle format, possibly using wide characters,
 * possibly not, together with its length and the cursor position.
 * The length must be accurate and includes all characters (no NULL
 * termination is expected).  The input cursor position is only
 * significant if outcs is non-NULL.
 *
 * Output: an ordinary NULL-terminated string, using multibyte characters
 * instead of wide characters where appropriate and with the contents
 * metafied.
 *
 * If outllp is non-NULL, assign the new length.  This is the conventional
 * string length, without the NULL byte.
 *
 * If outcsp is non-NULL, assign the new character position.
 * If outcsp is &zlemetacs, update the positions in the region_highlight
 * array, too.  This is a bit of a hack.
 *
 * If useheap is 1, memory is returned from the heap, else is allocated
 * for later freeing.
 */

/**/
mod_export char *
zlelineasstring(ZLE_STRING_T instr, int inll, int incs, int *outllp,
		int *outcsp, int useheap)
{
    int outcs, outll, sub;
    struct region_highlight *rhp;

#ifdef MULTIBYTE_SUPPORT
    char *s;
    int i, j;
    size_t mb_len = 0;
    mbstate_t mbs;

    s = zalloc(inll * MB_CUR_MAX + 1);

    outcs = 0;
    memset(&mbs, 0, sizeof(mbs));
    for (i=0; i < inll; i++) {
	if (incs == 0)
	    outcs = mb_len;
	incs--;
	if (region_highlights && outcsp == &zlemetacs) {
	    for (rhp = region_highlights + N_SPECIAL_HIGHLIGHTS;
		 rhp < region_highlights + n_region_highlights;
		 rhp++) {
		if (rhp->flags & ZRH_PREDISPLAY)
		    sub = predisplaylen;
		else
		    sub = 0;
		if (rhp->start - sub == 0)
		    rhp->start_meta = sub + mb_len;
		rhp->start--;
		if (rhp->end - sub == 0)
		    rhp->end_meta = sub + mb_len;
		rhp->end--;
	    }
	}
#ifdef __STDC_ISO_10646__
	if (ZSH_INVALID_WCHAR_TEST(instr[i])) {
	    s[mb_len++] = ZSH_INVALID_WCHAR_TO_CHAR(instr[i]);
	} else
#endif
	{
	    j = wcrtomb(s + mb_len, instr[i], &mbs);
	    if (j == -1) {
		/* invalid char */
		s[mb_len++] = ZWC('?');
		memset(&mbs, 0, sizeof(mbs));
	    } else {
		mb_len += j;
	    }
	}
    }
    if (incs == 0)
	outcs = mb_len;
    if (region_highlights && outcsp == &zlemetacs) {
	for (rhp = region_highlights + N_SPECIAL_HIGHLIGHTS;
	     rhp < region_highlights + n_region_highlights;
	     rhp++) {
	    if (rhp->flags & ZRH_PREDISPLAY)
		sub = predisplaylen;
	    else
		sub = 0;
	    if (rhp->start - sub == 0)
		rhp->start_meta = sub + mb_len;
	    if (rhp->end - sub == 0)
		rhp->end_meta = sub + mb_len;
	}
    }
    s[mb_len] = '\0';

    outll = mb_len;
#else
    outll = inll;
    outcs = incs;
    if (region_highlights && outcsp == &zlemetacs) {
	for (rhp = region_highlights + N_SPECIAL_HIGHLIGHTS;
	     rhp < region_highlights + n_region_highlights;
	     rhp++) {
	    rhp->start_meta = rhp->start;
	    rhp->end_meta = rhp->end;
	}
    }
#endif

    /*
     * *outcsp and *outllp are to be indexes into the final string,
     * not character offsets, so we need to take account of any
     * metafiable characters.
     */
    if (outcsp != NULL || outllp != NULL) {
#ifdef MULTIBYTE_SUPPORT
	char *strp = s;
#else
	char *strp = instr;
#endif
	char *stopcs = strp + outcs;
	char *stopll = strp + outll;
	char *startp = strp;

	if (region_highlights && outcsp == &zlemetacs) {
	    for (rhp = region_highlights + N_SPECIAL_HIGHLIGHTS;
		 rhp < region_highlights + n_region_highlights;
		 rhp++) {
		/* Used as temporary storage */
		rhp->start = rhp->start_meta;
		rhp->end = rhp->end_meta;
	    }
	}
	while (strp < stopll) {
	    if (imeta(*strp)) {
		if (strp < stopcs)
		    outcs++;
		if (region_highlights && outcsp == &zlemetacs) {
		    for (rhp = region_highlights + N_SPECIAL_HIGHLIGHTS;
			 rhp < region_highlights + n_region_highlights;
			 rhp++) {
			if (rhp->flags & ZRH_PREDISPLAY)
			    sub = predisplaylen;
			else
			    sub = 0;
			if (strp < startp + rhp->start - sub) {
			    rhp->start_meta++;
			}
			if (strp < startp + rhp->end - sub) {
			    rhp->end_meta++;
			}
		    }
		}
		outll++;
	    }
	    strp++;
	}
	if (outcsp != NULL)
	    *outcsp = outcs;
	if (outllp != NULL)
	    *outllp = outll;
    }

#ifdef MULTIBYTE_SUPPORT
    if (useheap) {
	char *ret = metafy(s, mb_len, META_HEAPDUP);

	zfree(s, inll * MB_CUR_MAX + 1);

	return ret;
    }
    return metafy(s, mb_len, META_REALLOC);
#else
    return metafy(instr, inll, useheap ? META_HEAPDUP : META_DUP);
#endif
}


/*
 * Input a NULL-terminated metafied string instr.
 * Output a line in internal zle format, together with its length
 * in the appropriate character units.  Note that outll may not be NULL.
 *
 * If outsz is non-NULL, the number of allocated characters in the
 * string is written there.  For compatibility with use of the linesz
 * variable (allocate size of zleline), at least two characters are
 * allocated more than needed for immediate use.  (The extra characters
 * may take a newline and a null at a later stage.)  These are not
 * included in *outsz.
 *
 * If outcs is non-NULL, the character position in the original
 * string incs (a standard string offset, i.e. incremented 2 for
 * each metafied character) is converted into the corresponding
 * character position in *outcs.
 *
 * If, further, outcs is &zlecs, we update the positions in the
 * region_highlight array, too.  (This is a bit of a hack.)
 *
 * Note that instr is modified in place, hence should be copied
 * first if necessary;
 *
 * Memory for the returned string is permanently allocated.  *outsz may
 * be longer than the *outll returned.  Hence it should be freed with
 * zfree(outstr, *outsz) or free(outstr), not zfree(outstr, *outll).
 */

/**/
mod_export ZLE_STRING_T
stringaszleline(char *instr, int incs, int *outll, int *outsz, int *outcs)
{
    ZLE_STRING_T outstr;
    int ll, sz, sub;
    struct region_highlight *rhp;
#ifdef MULTIBYTE_SUPPORT
    mbstate_t mbs;
#endif

    if (outcs) {
	/*
	 * Take account of Meta characters in the input string
	 * before we unmetafy it.  This does not yet take account
	 * of multibyte characters.  If there are none, this
	 * is all the processing required to calculate outcs.
	 */
	char *inptr = instr, *cspos = instr + incs;
	if (region_highlights && outcs == &zlecs) {
	    for (rhp = region_highlights + N_SPECIAL_HIGHLIGHTS;
		 rhp < region_highlights + n_region_highlights;
		 rhp++) {
		rhp->start = rhp->start_meta;
		rhp->end = rhp->end_meta;
	    }
	}
	while (*inptr) {
	    if (*inptr == Meta) {
		if (inptr < cspos) {
		    incs--;
		}
		if (region_highlights && outcs == &zlecs) {
		    for (rhp = region_highlights + N_SPECIAL_HIGHLIGHTS;
			 rhp < region_highlights + n_region_highlights;
			 rhp++) {
			if (rhp->flags & ZRH_PREDISPLAY)
			    sub = predisplaylen;
			else
			    sub = 0;
			if (inptr - instr < rhp->start - sub) {
			    rhp->start_meta--;
			}
			if (inptr - instr < rhp->end - sub) {
			    rhp->end_meta--;
			}
		    }
		}
		inptr++;
	    }
	    inptr++;
	}
    }
    unmetafy(instr, &ll);

    /*
     * ll is the maximum number of characters there can be in
     * the output string; the closer to ASCII the string, the
     * better the guess.  For the 2 see above.
     */
    sz = (ll + 2) * ZLE_CHAR_SIZE;
    if (outsz)
	*outsz = ll;
    outstr = (ZLE_STRING_T)zalloc(sz);

#ifdef MULTIBYTE_SUPPORT
    if (ll) {
	char *inptr = instr;
	wchar_t *outptr = outstr;

	/* Reset shift state to input complete string */
	memset(&mbs, '\0', sizeof mbs);

	while (ll > 0) {
	    size_t cnt = mbrtowc(outptr, inptr, ll, &mbs);

#ifdef __STDC_ISO_10646__
	    if (cnt == MB_INCOMPLETE || cnt == MB_INVALID) {
		/* Use private encoding for invalid single byte */
		*outptr = ZSH_CHAR_TO_INVALID_WCHAR(*inptr);
		cnt = 1;
	    }
#else
	    /*
	     * At this point we don't handle either incomplete (-2) or
	     * invalid (-1) multibyte sequences.  Use the current length
	     * and return.
	     */
	    if (cnt == MB_INCOMPLETE || cnt == MB_INVALID)
		break;
#endif

	    if (cnt == 0) {
		/* Converting '\0' returns 0, but a '\0' is a real
		 * character for us, so we should consume 1 byte
		 * (certainly true for Unicode and unlikely to be false
		 * in any non-pathological multibyte representation). */
		cnt = 1;
	    } else if (cnt > (size_t)ll) {
		/*
		 * Some multibyte implementations return the
		 * full length of a previous incomplete character
		 * instead of the remaining length.
		 * This is paranoia: it only applies if we start
		 * midway through a multibyte character, which
		 * presumably can't happen.
		 */
		cnt = ll;
	    }

	    if (outcs) {
		int offs = inptr - instr;
		if (offs <= incs && incs < offs + (int)cnt)
		    *outcs = outptr - outstr;
		if (region_highlights && outcs == &zlecs) {
		    for (rhp = region_highlights + N_SPECIAL_HIGHLIGHTS;
			 rhp < region_highlights + n_region_highlights;
			 rhp++) {
			if (rhp->flags & ZRH_PREDISPLAY)
			    sub = predisplaylen;
			else
			    sub = 0;
			if (offs <= rhp->start_meta - sub &&
			    rhp->start_meta - sub < offs + (int)cnt) {
			    rhp->start = outptr - outstr + sub;
			}
			if (offs <= rhp->end_meta - sub &&
			    rhp->end_meta - sub < offs + (int)cnt) {
			    rhp->end = outptr - outstr + sub;
			}
		    }
		}
	    }

	    inptr += cnt;
	    outptr++;
	    ll -= cnt;
	}
	if (outcs && inptr <= instr + incs)
	    *outcs = outptr - outstr;
	*outll = outptr - outstr;
    } else {
	*outll = 0;
	if (outcs)
	    *outcs = 0;
    }
#else
    memcpy(outstr, instr, ll);
    *outll = ll;
    if (outcs)
	*outcs = incs;
    if (region_highlights && outcs == &zlecs) {
	for (rhp = region_highlights + N_SPECIAL_HIGHLIGHTS;
	     rhp < region_highlights + n_region_highlights;
	     rhp++) {
	    rhp->start = rhp->start_meta;
	    rhp->end = rhp->end_meta;
	}
    }
#endif

    return outstr;
}

/*
 * This function is called when we are playing very nasty tricks
 * indeed: see bufferwords in hist.c.  Consequently we can make
 * absolutely no assumption about the state whatsoever, except
 * that it has one.
 */

/**/
mod_export char *
zlegetline(int *ll, int *cs)
{
    if (zlemetaline != NULL) {
	*ll = zlemetall;
	*cs = zlemetacs;
	return ztrdup(zlemetaline);
    }
    if (zleline)
	return zlelineasstring(zleline, zlell, zlecs, ll, cs, 0);
    *ll = *cs = 0;
    return ztrdup("");
}


/*
 * free() the 'memo' elements of region_highlights.
 */

/**/
void
free_region_highlights_memos(void)
{
    struct region_highlight *rhp;
    for (rhp = region_highlights;
	 rhp < region_highlights + n_region_highlights;
	 rhp++) {
	zfree((char*) rhp->memo, 0);
    }
}

/* Forward reference */
struct zle_region;

/* A non-special entry in region_highlight */
struct zle_region  {
    struct zle_region *next;
    /* Entries of region_highlight, as needed */
    int atr;
    int start;
    int end;
    int flags;
    const char *memo;
};

/* Forward reference */
struct zle_position;

/* A saved set of position information */
struct zle_position {
    /* Link pointer */
    struct zle_position *next;
    /* Cursor position */
    int cs;
    /* Mark */
    int mk;
    /* Line length */
    int ll;
    struct zle_region *regions;
};

/* LIFO stack of positions */
static struct zle_position *zle_positions;

/*
 * Save positions including cursor, end-of-line and
 * (non-special) region highlighting.
 *
 * Must be matched by a subsequent zle_restore_positions().
 */

/**/
mod_export void
zle_save_positions(void)
{
    struct region_highlight *rhp;
    struct zle_position *newpos;
    struct zle_region **newrhpp, *newrhp;

    newpos = (struct zle_position *)zalloc(sizeof(*newpos));

    newpos->mk = mark;
    if (zlemetaline) {
	/* Use metafied information */
	newpos->cs = zlemetacs;
	newpos->ll = zlemetall;
    } else {
	/* Use unmetafied information */
	newpos->cs = zlecs;
	newpos->ll = zlell;

    }

    newrhpp = &newpos->regions;
    *newrhpp = NULL;
    if (region_highlights) {
	for (rhp = region_highlights + N_SPECIAL_HIGHLIGHTS;
	     rhp < region_highlights + n_region_highlights;
	     rhp++) {
	    /*
	     * This is a FIFO stack, so we preserve the order
	     * of entries when we restore region_highlights.
	     */
	    newrhp = *newrhpp = (struct zle_region *)zalloc(sizeof(**newrhpp));
	    newrhp->next = NULL;
	    newrhp->atr = rhp->atr;
	    newrhp->flags = rhp->flags;
	    newrhp->memo = ztrdup(rhp->memo);
	    if (zlemetaline) {
		newrhp->start = rhp->start_meta;
		newrhp->end = rhp->end_meta;
	    } else {
		newrhp->start = rhp->start;
		newrhp->end = rhp->end;
	    }
	    newrhpp = &newrhp->next;
	}
    }

    newpos->next = zle_positions;
    zle_positions = newpos;
}

/*
 * Restore positions previously saved.
 * Relies on zlemetaline being restored correctly beforehand,
 * so that it can tell whether to use metafied positions or not.
 */

/**/
mod_export void
zle_restore_positions(void)
{
    struct zle_position *oldpos = zle_positions;
    struct zle_region *oldrhp;
    struct region_highlight *rhp;
    int nreg;

    zle_positions = oldpos->next;

    mark = oldpos->mk;
    if (zlemetaline) {
	/* Use metafied information */
	zlemetacs = oldpos->cs;
	zlemetall = oldpos->ll;
    } else {
	/* Use unmetafied information */
	zlecs = oldpos->cs;
	zlell = oldpos->ll;
    }

    if (oldpos->regions) {
	/* Count number of regions and see if the array needs resizing */
	for (nreg = 0, oldrhp = oldpos->regions;
	     oldrhp;
	     nreg++, oldrhp = oldrhp->next)
	    ;
	if (nreg + N_SPECIAL_HIGHLIGHTS != n_region_highlights) {
	    free_region_highlights_memos();
	    n_region_highlights = nreg + N_SPECIAL_HIGHLIGHTS;
	    region_highlights = (struct region_highlight *)
		zrealloc(region_highlights,
			 sizeof(struct region_highlight) * n_region_highlights);
	}
	oldrhp = oldpos->regions;
	rhp = region_highlights + N_SPECIAL_HIGHLIGHTS;
	while (oldrhp) {
	    struct zle_region *nextrhp = oldrhp->next;

	    rhp->atr = oldrhp->atr;
	    rhp->flags = oldrhp->flags;
	    rhp->memo = oldrhp->memo; /* transferring ownership of the permanently-allocated memory */
	    if (zlemetaline) {
		rhp->start_meta = oldrhp->start;
		rhp->end_meta = oldrhp->end;
	    } else {
		rhp->start = oldrhp->start;
		rhp->end = oldrhp->end;
	    }

	    zfree(oldrhp, sizeof(*oldrhp));
	    oldrhp = nextrhp;
	    rhp++;
	}
    } else if (region_highlights) {
	free_region_highlights_memos();
	zfree(region_highlights, sizeof(struct region_highlight) *
	      n_region_highlights);
	region_highlights  = NULL;
	n_region_highlights = 0;
    }

    zfree(oldpos, sizeof(*oldpos));
}

/*
 * Discard positions previously saved, the line has been updated.
 */

/**/
mod_export void
zle_free_positions(void)
{
    struct zle_position *oldpos = zle_positions;
    struct zle_region *oldrhp;

    zle_positions = oldpos->next;
    oldrhp = oldpos->regions;
    while (oldrhp) {
	struct zle_region *nextrhp = oldrhp->next;
	zfree(oldrhp, sizeof(*oldrhp));
	oldrhp = nextrhp;
    }
    zfree(oldpos, sizeof(*oldpos));
}

/*
 * Basic utility functions for adding to line or removing from line.
 * At this level the counts supplied are raw character counts, so
 * the calling code must be aware of combining characters where
 * necessary, e.g. if we want to delete a + combing grave forward
 * from the cursor, then shiftchars() gets the count 2 (not 1).
 *
 * This is necessary because these utility functions don't know about
 * zlecs, and we need to count combined characters from there.
 */

/* insert space for ct chars at cursor position */

/**/
mod_export void
spaceinline(int ct)
{
    int i, sub;
    struct region_highlight *rhp;

    if (zlemetaline) {
	sizeline(ct + zlemetall);
	for (i = zlemetall; --i >= zlemetacs;)
	    zlemetaline[i + ct] = zlemetaline[i];
	zlemetall += ct;
	zlemetaline[zlemetall] = '\0';

	if (mark > zlemetacs)
	    mark += ct;

	if (region_highlights) {
	    for (rhp = region_highlights + N_SPECIAL_HIGHLIGHTS;
		 rhp < region_highlights + n_region_highlights;
		 rhp++) {
		if (rhp->flags & ZRH_PREDISPLAY)
		    sub = predisplaylen;
		else
		    sub = 0;
		if (rhp->start_meta - sub >= zlemetacs) {
		    rhp->start_meta += ct;
		}
		if (rhp->end_meta - sub >= zlemetacs) {
		    rhp->end_meta += ct;
		}
	    }
	}
    } else {
	sizeline(ct + zlell);
	for (i = zlell; --i >= zlecs;)
	    zleline[i + ct] = zleline[i];
	zlell += ct;
	zleline[zlell] = ZWC('\0');

	if (mark > zlecs)
	    mark += ct;
	if (viinsbegin > zlecs)
	    viinsbegin = 0;

	if (region_highlights) {
	    for (rhp = region_highlights + N_SPECIAL_HIGHLIGHTS;
		 rhp < region_highlights + n_region_highlights;
		 rhp++) {
		if (rhp->flags & ZRH_PREDISPLAY)
		    sub = predisplaylen;
		else
		    sub = 0;
		if (rhp->start - sub >= zlecs) {
		    rhp->start += ct;
		}
		if (rhp->end - sub >= zlecs) {
		    rhp->end += ct;
		}
	    }
	}
    }
    region_active = 0;
}

/*
 * Within the ZLE line, cut the "cnt" characters from position "to".
 */

/**/
void
shiftchars(int to, int cnt)
{
    struct region_highlight *rhp;
    int sub;

    if (mark >= to + cnt)
	mark -= cnt;
    else if (mark > to)
	mark = to;

    if (zlemetaline) {
	/* before to is updated... */
	if (region_highlights) {
	    for (rhp = region_highlights + N_SPECIAL_HIGHLIGHTS;
		 rhp < region_highlights + n_region_highlights;
		 rhp++) {
		if (rhp->flags & ZRH_PREDISPLAY)
		    sub = predisplaylen;
		else
		    sub = 0;
		if (rhp->start_meta - sub > to) {
		    if (rhp->start_meta - sub > to + cnt)
			rhp->start_meta -= cnt;
		    else
			rhp->start_meta = to;
		}
		if (rhp->end_meta - sub > to) {
		    if (rhp->end_meta - sub > to + cnt)
			rhp->end_meta -= cnt;
		    else
			rhp->end_meta = to;
		}
	    }
	}

	while (to + cnt < zlemetall) {
	    zlemetaline[to] = zlemetaline[to + cnt];
	    to++;
	}
	zlemetaline[zlemetall = to] = '\0';
    } else {
	/* before to is updated... */
	if (region_highlights) {
	    for (rhp = region_highlights + N_SPECIAL_HIGHLIGHTS;
		 rhp < region_highlights + n_region_highlights;
		 rhp++) {
		if (rhp->flags & ZRH_PREDISPLAY)
		    sub = predisplaylen;
		else
		    sub = 0;
		if (rhp->start - sub > to) {
		    if (rhp->start - sub > to + cnt)
			rhp->start -= cnt;
		    else
			rhp->start = to;
		}
		if (rhp->end - sub > to) {
		    if (rhp->end - sub > to + cnt)
			rhp->end -= cnt;
		    else
			rhp->end = to;
		}
	    }
	}

	while (to + cnt < zlell) {
	    zleline[to] = zleline[to + cnt];
	    to++;
	}
	zleline[zlell = to] = ZWC('\0');
    }
    region_active = 0;
}

/*
 * Put the ct characters starting at zleline + i into the
 * cutbuffer, circling the kill ring if necessary (it's
 * not if we're dealing with vi buffers, which is detected
 * internally).  The text is not removed from zleline.
 *
 * dir indicates how the text is to be added to the cutbuffer,
 * if the cutbuffer wasn't zeroed (this depends on the last
 * command being a kill).  If dir is 1, the new text goes
 * to the front of the cut buffer.  If dir is -1, the cutbuffer
 * is completely overwritten.
 */

/**/
void
cut(int i, int ct, int flags)
{
  cuttext(zleline + i, ct, flags);
}

/*
 * As cut, but explicitly supply the text together with its length.
 */

/**/
void
cuttext(ZLE_STRING_T line, int ct, int flags)
{
    if (!(ct || vilinerange) ||  zmod.flags & MOD_NULL)
	return;

    UNMETACHECK();
    if (zmod.flags & MOD_VIBUF) {
	struct cutbuffer *b = &vibuf[zmod.vibuf];

	if (!(zmod.flags & MOD_VIAPP) || !b->buf) {
	    free(b->buf);
	    b->buf = (ZLE_STRING_T)zalloc(ct * ZLE_CHAR_SIZE);
	    ZS_memcpy(b->buf, line, ct);
	    b->len = ct;
	    b->flags = vilinerange ? CUTBUFFER_LINE : 0;
	} else {
	    int len = b->len;

	    if(vilinerange)
		b->flags |= CUTBUFFER_LINE;
	    b->buf = (ZLE_STRING_T)
		realloc((char *)b->buf,
			(ct + len + !!(b->flags & CUTBUFFER_LINE))
			* ZLE_CHAR_SIZE);
	    if (b->flags & CUTBUFFER_LINE)
		b->buf[len++] = ZWC('\n');
	    ZS_memcpy(b->buf + len, line, ct);
	    b->len = len + ct;
	}
    } else if (flags & CUT_YANK) {
	/* Save in "0 */
	free(vibuf[26].buf);
	vibuf[26].buf = (ZLE_STRING_T)zalloc(ct * ZLE_CHAR_SIZE);
	ZS_memcpy(vibuf[26].buf, line, ct);
	vibuf[26].len = ct;
	vibuf[26].flags = vilinerange ? CUTBUFFER_LINE : 0;
    } else {
	/* Save in "1, shifting "1-"8 along to "2-"9 */
	int n;
	free(vibuf[35].buf);
	for(n=35; n>27; n--)
	    vibuf[n] = vibuf[n-1];
	vibuf[27].buf = (ZLE_STRING_T)zalloc(ct * ZLE_CHAR_SIZE);
	ZS_memcpy(vibuf[27].buf, line, ct);
	vibuf[27].len = ct;
	vibuf[27].flags = vilinerange ? CUTBUFFER_LINE : 0;
    }
    if (!cutbuf.buf) {
	cutbuf.buf = (ZLE_STRING_T)zalloc(ZLE_CHAR_SIZE);
	cutbuf.buf[0] = ZWC('\0');
	cutbuf.len = cutbuf.flags = 0;
    } else if (!(lastcmd & ZLE_KILL) || (flags & CUT_REPLACE)) {
	Cutbuffer kptr;
	if (!kring) {
	    kringsize = KRINGCTDEF;
	    kring = (Cutbuffer)zshcalloc(kringsize * sizeof(struct cutbuffer));
	} else
	    kringnum = (kringnum + 1) % kringsize;
	kptr = kring + kringnum;
	if (kptr->buf)
	    free(kptr->buf);
	*kptr = cutbuf;
	cutbuf.buf = (ZLE_STRING_T)zalloc(ZLE_CHAR_SIZE);
	cutbuf.buf[0] = ZWC('\0');
	cutbuf.len = cutbuf.flags = 0;
    }
    if (flags & (CUT_FRONT|CUT_REPLACE)) {
	ZLE_STRING_T s = (ZLE_STRING_T)zalloc((cutbuf.len + ct)*ZLE_CHAR_SIZE);

	ZS_memcpy(s, line, ct);
	ZS_memcpy(s + ct, cutbuf.buf, cutbuf.len);
	free(cutbuf.buf);
	cutbuf.buf = s;
	cutbuf.len += ct;
    } else {
	/* don't alloc 0 bytes; length 0 occurs for blank lines in vi mode */
	cutbuf.buf = realloc((char *)cutbuf.buf,
			     (cutbuf.len + (ct ? ct : 1)) * ZLE_CHAR_SIZE);
	ZS_memcpy(cutbuf.buf + cutbuf.len, line, ct);
	cutbuf.len += ct;
    }
    if(vilinerange)
	cutbuf.flags |= CUTBUFFER_LINE;
    else
	cutbuf.flags &= ~CUTBUFFER_LINE;
}

/*
 * Now we're back in the world of zlecs where we need to keep
 * track of whether we're on a combining character.
 */

/**/
mod_export void
backkill(int ct, int flags)
{
    UNMETACHECK();
    if (flags & CUT_RAW) {
	zlecs -= ct;
    } else {
	int origcs = zlecs;
	while (ct--)
	    DECCS();
	ct = origcs - zlecs;
    }

    cut(zlecs, ct, flags);
    shiftchars(zlecs, ct);
    CCRIGHT();
}

/**/
mod_export void
forekill(int ct, int flags)
{
    int i = zlecs;

    UNMETACHECK();
    if (!(flags & CUT_RAW)) {
	int n = ct;
	while (n--)
	    INCCS();
	ct = zlecs - i;
	zlecs = i;
    }

    cut(i, ct, flags);
    shiftchars(i, ct);
    CCRIGHT();
}

/**/
mod_export void
backdel(int ct, int flags)
{
    if (flags & CUT_RAW) {
	if (zlemetaline != NULL) {
	    shiftchars(zlemetacs -= ct, ct);
	} else {
	    shiftchars(zlecs -= ct, ct);
	    CCRIGHT();
	}
    } else {
	int n = ct, origcs = zlecs;
	DPUTS(zlemetaline != NULL, "backdel needs CUT_RAW when metafied");
	while (n--)
	    DECCS();
	shiftchars(zlecs, origcs - zlecs);
	CCRIGHT();
    }
}

/**/
mod_export void
foredel(int ct, int flags)
{
    if (flags & CUT_RAW) {
	if (zlemetaline != NULL) {
	    shiftchars(zlemetacs, ct);
	} else if (flags & CUT_RAW) {
	    shiftchars(zlecs, ct);
	    CCRIGHT();
	}
    } else {
	int origcs = zlecs;
	int n = ct;
	DPUTS(zlemetaline != NULL, "foredel needs CUT_RAW when metafied");
	while (n--)
	    INCCS();
	ct = zlecs - origcs;
	zlecs = origcs;
	shiftchars(zlecs, ct);
	CCRIGHT();
    }
}

/**/
void
setline(char *s, int flags)
{
    char *scp;

    UNMETACHECK();
    if (flags & ZSL_COPY)
	scp = ztrdup(s);
    else
	scp = s;
    /*
     * TBD: we could make this more efficient by passing the existing
     * allocated line to stringaszleline.
     */
    free(zleline);

    viinsbegin = 0;
    zleline = stringaszleline(scp, 0, &zlell, &linesz, NULL);

    if ((flags & ZSL_TOEND) && (zlecs = zlell) && invicmdmode())
	DECCS();
    else if (zlecs > zlell)
	zlecs = zlell;
    CCRIGHT();
    if (flags & ZSL_COPY)
	free(scp);
}

/**/
int
findbol(void)
{
    int x = zlecs;

    while (x > 0 && zleline[x - 1] != ZWC('\n'))
	x--;
    return x;
}

/**/
int
findeol(void)
{
    int x = zlecs;

    while (x != zlell && zleline[x] != ZWC('\n'))
	x++;
    return x;
}

/**/
void
findline(int *a, int *b)
{
    *a = findbol();
    *b = findeol();
}

/*
 * Query the user, and return 1 for yes, 0 for no.  The question is assumed to
 * have been printed already, and the cursor is left immediately after the
 * response echoed.  (Might cause a problem if this takes it onto the next
 * line.)  <Tab> is interpreted as 'y'; any other control character is
 * interpreted as 'n'.  If there are any characters in the buffer, this is
 * taken as a negative response, and no characters are read.  Case is folded.
 */

/**/
mod_export int
getzlequery(void)
{
    ZLE_INT_T c;
#ifdef FIONREAD
    int val;

    /* check for typeahead, which is treated as a negative response */
    ioctl(SHTTY, FIONREAD, (char *)&val);
    if (val) {
	putc('n', shout);
	return 0;
    }
#endif

    /* get a character from the tty and interpret it */
    c = getfullchar(0);
    /*
     * We'll interpret an interruption here as only interrupting the
     * query, not the line editor.
     */
    errflag &= ~ERRFLAG_INT;
    if (c == ZWC('\t'))
	c = ZWC('y');
    else if (ZC_icntrl(c) || c == ZLEEOF)
	c = ZWC('n');
    else
	c = ZC_tolower(c);
    /* echo response and return */
    if (c != ZWC('\n')) {
	REFRESH_ELEMENT re;
	re.chr = c;
	re.atr = 0;
	zwcputc(&re, NULL);
    }
    return c == ZWC('y');
}

/* Format a string, keybinding style. */

/**/
char *
bindztrdup(char *str)
{
    int c, len = 1;
    char *buf, *ptr, *ret;

    for(ptr = str; *ptr; ptr++) {
	c = *ptr == Meta ? STOUC(*++ptr) ^ 32 : STOUC(*ptr);
	if(c & 0x80) {
	    len += 3;
	    c &= 0x7f;
	}
	if(c < 32 || c == 0x7f) {
	    len++;
	    c ^= 64;
	}
	len += c == '\\' || c == '^';
	len++;
    }
    ptr = buf = zalloc(len);
    for(; *str; str++) {
	c = *str == Meta ? STOUC(*++str) ^ 32 : STOUC(*str);
	if(c & 0x80) {
	    *ptr++ = '\\';
	    *ptr++ = 'M';
	    *ptr++ = '-';
	    c &= 0x7f;
	}
	if(c < 32 || c == 0x7f) {
	    *ptr++ = '^';
	    c ^= 64;
	}
	if(c == '\\' || c == '^')
	    *ptr++ = '\\';
	*ptr++ = c;
    }
    *ptr = 0;
    ret = dquotedztrdup(buf);
    zsfree(buf);
    return ret;
}

/* Display a metafied string, keybinding-style. */

/**/
int
printbind(char *str, FILE *stream)
{
    char *b = bindztrdup(str);
    int ret = zputs(b, stream);

    zsfree(b);
    return ret;
}

/*
 * Display a message where the completion list normally goes.
 * The message must be metafied.
 *
 * TODO: there's some advantage in using a ZLE_STRING_T array here,
 * together with improvements in other places, but messages don't
 * need to be particularly efficient.
 */

/**/
mod_export void
showmsg(char const *msg)
{
    char const *p;
    int up = 0, cc = 0;
    ZLE_CHAR_T c;
#ifdef MULTIBYTE_SUPPORT
    char *umsg;
    int ulen, eol = 0;
    size_t width;
    mbstate_t mbs;
#endif

    trashzle();
    clearflag = isset(USEZLE) && !termflags && isset(ALWAYSLASTPROMPT);

#ifdef MULTIBYTE_SUPPORT
    umsg = ztrdup(msg);
    p = unmetafy(umsg, &ulen);
    memset(&mbs, 0, sizeof mbs);

    mb_charinit();
    while (ulen > 0) {
	char const *n;
	if (*p == '\n') {
	    ulen--;
	    p++;

	    putc('\n', shout);
	    up += 1 + cc / zterm_columns;
	    cc = 0;
	} else {
	    /*
	     * Extract the next wide character from the multibyte string.
	     */
	    size_t cnt = eol ? MB_INVALID : mbrtowc(&c, p, ulen, &mbs);

	    switch (cnt) {
	    case MB_INCOMPLETE:
		eol = 1;
		/* FALL THROUGH */
	    case MB_INVALID:
		/*
		 * This really shouldn't be happening here, but...
		 * Treat it as a single byte character; it may get
		 * prettified.
		 */
		memset(&mbs, 0, sizeof mbs);
		n = nicechar(*p);
		cnt = 1;
		width = strlen(n);
		break;
	    case 0:
		cnt = 1;
		/* FALL THROUGH */
	    default:
		/*
		 * Paranoia: only needed if we start in the middle
		 * of a multibyte string and only in some implementations.
		 */
		if (cnt > (size_t)ulen)
		    cnt = ulen;
		n = wcs_nicechar(c, &width, NULL);
		break;
	    }
	    ulen -= cnt;
	    p += cnt;

	    zputs(n, shout);
	    cc += width;
	}
    }

    free(umsg);
#else
    for(p = msg; (c = *p); p++) {
	if(c == Meta)
	    c = *++p ^ 32;
	if(c == '\n') {
	    putc('\n', shout);
	    up += 1 + cc / zterm_columns;
	    cc = 0;
	} else {
	    char const *n = nicechar(c);
	    zputs(n, shout);
	    cc += strlen(n);
	}
    }
#endif
    up += cc / zterm_columns;

    if (clearflag) {
	putc('\r', shout);
	tcmultout(TCUP, TCMULTUP, up + nlnct);
    } else
	putc('\n', shout);
    showinglist = 0;
}

/* handle the error flag */

/**/
int
handlefeep(UNUSED(char **args))
{
    zbeep();
    return 0;
}

/* user control of auto-suffixes -- see iwidgets.list */

/**/
int
handlesuffix(UNUSED(char **args))
{
  return 0;
}

/***************/
/* undo system */
/***************/

/* head of the undo list, and the current position */

/**/
struct change *curchange;

static struct change *changes;

/* list of pending changes, not yet in the undo system */

static struct change *nextchanges, *endnextchanges;

/* incremented to provide a unique change number */

/**/
zlong undo_changeno;

/* If positive, don't undo beyond this point */

static zlong undo_limitno;

/**/
void
initundo(void)
{
    nextchanges = NULL;
    changes = curchange = zalloc(sizeof(*curchange));
    curchange->prev = curchange->next = NULL;
    curchange->del = curchange->ins = NULL;
    curchange->dell = curchange->insl = 0;
    curchange->changeno = undo_changeno = undo_limitno = 0;
    lastline = zalloc((lastlinesz = linesz) * ZLE_CHAR_SIZE);
    ZS_memcpy(lastline, zleline, (lastll = zlell));
    lastcs = zlecs;
}

/**/
void
freeundo(void)
{
    freechanges(changes);
    freechanges(nextchanges);
    zfree(lastline, lastlinesz);
    lastline = NULL;
    lastlinesz = 0;
}

/**/
static void
freechanges(struct change *p)
{
    struct change *n;

    for(; p; p = n) {
	n = p->next;
	free(p->del);
	free(p->ins);
	zfree(p, sizeof(*p));
    }
}

/* register pending changes in the undo system */

/**/
mod_export void
handleundo(void)
{
    int remetafy;

    /*
     * Yuk: we call this from within the completion system,
     * so we need to convert back to the form which can be
     * copied into undo entries.
     */
    if (zlemetaline != NULL) {
	unmetafy_line();
	remetafy = 1;
    } else
	remetafy = 0;

    mkundoent();
    if(nextchanges) {
	setlastline();
	if(curchange->next) {
	    freechanges(curchange->next);
	    curchange->next = NULL;
	    free(curchange->del);
	    free(curchange->ins);
	    curchange->del = curchange->ins = NULL;
	    curchange->dell = curchange->insl = 0;
	}
	nextchanges->prev = curchange->prev;
	if(curchange->prev)
	    curchange->prev->next = nextchanges;
	else
	    changes = nextchanges;
	curchange->prev = endnextchanges;
	endnextchanges->next = curchange;
	nextchanges = endnextchanges = NULL;
    }

    if (remetafy)
	metafy_line();
}

/* add an entry to the undo system, if anything has changed */

/**/
void
mkundoent(void)
{
    int pre, suf;
    int sh = zlell < lastll ? zlell : lastll;
    struct change *ch;

    UNMETACHECK();
    if(lastll == zlell && lastlinesz >= zlell && !ZS_memcmp(lastline, zleline, zlell)) {
	lastcs = zlecs;
	return;
    }
    for(pre = 0; pre < sh && zleline[pre] == lastline[pre]; )
	pre++;
    for(suf = 0; suf < sh - pre &&
	zleline[zlell - 1 - suf] == lastline[lastll - 1 - suf]; )
	suf++;
    ch = zalloc(sizeof(*ch));
    ch->next = NULL;
    ch->hist = histline;
    ch->off = pre;
    ch->old_cs = lastcs;
    ch->new_cs = zlecs;
    if(suf + pre == lastll) {
	ch->del = NULL;
	ch->dell = 0;
    } else {
	ch->dell = lastll - pre - suf;
	ch->del = (ZLE_STRING_T)zalloc(ch->dell * ZLE_CHAR_SIZE);
	ZS_memcpy(ch->del, lastline + pre, ch->dell);
    }
    if(suf + pre == zlell) {
	ch->ins = NULL;
	ch->insl = 0;
    } else {
	ch->insl = zlell - pre - suf;
	ch->ins = (ZLE_STRING_T)zalloc(ch->insl * ZLE_CHAR_SIZE);
	ZS_memcpy(ch->ins, zleline + pre, ch->insl);
    }
    if(nextchanges) {
	ch->flags = CH_PREV;
	ch->prev = endnextchanges;
	endnextchanges->flags |= CH_NEXT;
	endnextchanges->next = ch;
    } else {
	nextchanges = ch;
	ch->flags = 0;
	ch->prev = NULL;
    }
    ch->changeno = ++undo_changeno;
    endnextchanges = ch;
}

/* set lastline to match line */

/**/
void
setlastline(void)
{
    UNMETACHECK();
    if(lastlinesz != linesz)
	lastline = realloc(lastline, (lastlinesz = linesz) * ZLE_CHAR_SIZE);
    ZS_memcpy(lastline, zleline, (lastll = zlell));
    lastcs = zlecs;
}

/* move backwards through the change list */

/**/
int
undo(char **args)
{
    zlong last_change;

    if (*args)
	last_change = zstrtol(*args, NULL, 0);
    else
	last_change = (zlong)-1;

    handleundo();
    do {
	struct change *prev = curchange->prev;
	if(!prev)
	    return 1;
	if (prev->changeno <= last_change)
	    break;
	if (prev->changeno <= undo_limitno && !*args)
	    return 1;
	if (!unapplychange(prev)) {
	    if (last_change >= 0) {
		unapplychange(prev);
		curchange = prev;
	    }
	} else {
	    curchange = prev;
	}
    } while (last_change >= (zlong)0 || (curchange->flags & CH_PREV));
    setlastline();
    return 0;
}

/**/
static int
unapplychange(struct change *ch)
{
    if(ch->hist != histline) {
	Histent he = quietgethist(ch->hist);
	DPUTS(he == NULL, "quietgethist(ch->hist) returned NULL");
	if(he == NULL)
	    return 1;
	zle_setline(he);
	zlecs = ch->new_cs;
	return 0;
    }
    zlecs = ch->off;
    if(ch->ins)
	foredel(ch->insl, CUT_RAW);
    if(ch->del) {
	spaceinline(ch->dell);
	ZS_memcpy(zleline + zlecs, ch->del, ch->dell);
	zlecs += ch->dell;
    }
    zlecs = ch->old_cs;
    return 1;
}

/* move forwards through the change list */

/**/
int
redo(UNUSED(char **args))
{
    handleundo();
    do {
	if(!curchange->next)
	    return 1;
	if (applychange(curchange))
	    curchange = curchange->next;
	else
	    break;
    } while(curchange->prev->flags & CH_NEXT);
    setlastline();
    return 0;
}

/**/
static int
applychange(struct change *ch)
{
    if(ch->hist != histline) {
	Histent he = quietgethist(ch->hist);
	DPUTS(he == NULL, "quietgethist(ch->hist) returned NULL");
	if(he == NULL)
	    return 1;
	zle_setline(he);
	zlecs = ch->old_cs;
	return 0;
    }
    zlecs = ch->off;
    if(ch->del)
	foredel(ch->dell, CUT_RAW);
    if(ch->ins) {
	spaceinline(ch->insl);
	ZS_memcpy(zleline + zlecs, ch->ins, ch->insl);
	zlecs += ch->insl;
    }
    zlecs = ch->new_cs;
    return 1;
}

/* vi undo: toggle between the end of the undo list and the preceding point */

/**/
int
viundochange(char **args)
{
    handleundo();
    if(curchange->next) {
	do {
	    applychange(curchange);
	    curchange = curchange->next;
	} while(curchange->next);
	setlastline();
	return 0;
    } else
	return undo(args);
}

/**/
int
splitundo(UNUSED(char **args))
{
    if (vistartchange >= 0) {
	mergeundo();
	vistartchange = undo_changeno;
    }
    handleundo();
    return 0;
}

/**/
void
mergeundo(void)
{
    struct change *current;
    for (current = curchange->prev;
	    current && current->prev && current->changeno > vistartchange+1;
	    current = current->prev) {
	current->flags |= CH_PREV;
	current->prev->flags |= CH_NEXT;
    }
    vistartchange = -1;
}

/*
 * Call a ZLE hook: a user-defined widget called at a specific point
 * within the line editor.
 *
 * A single argument arg is passed to the function (in addition to the
 * function name).  It may be NULL.
 */

/**/
void
zlecallhook(char *name, char *arg)
{
    Thingy thingy = rthingy_nocreate(name);
    int saverrflag, savretflag;
    char *args[2];

    if (!thingy)
	return;

    /* If anything here needs changing, see also redrawhook() */

    saverrflag = errflag;
    savretflag = retflag;

    args[0] = arg;
    args[1] = NULL;
    execzlefunc(thingy, args, 1, 0);
    unrefthingy(thingy);

    /* Retain any user interrupt error status */
    errflag = saverrflag | (errflag & ERRFLAG_INT);
    retflag = savretflag;
}

/*
 * Return the number corresponding to the last change made.
 */

/**/
zlong
get_undo_current_change(UNUSED(Param pm))
{
    int remetafy;

    /*
     * Yuk: we call this from within the completion system,
     * so we need to convert back to the form which can be
     * copied into undo entries.
     */
    if (zlemetaline != NULL) {
	unmetafy_line();
	remetafy = 1;
    } else
	remetafy = 0;

    /* add entry for any pending changes */
    mkundoent();
    setlastline();

    if (remetafy)
	metafy_line();

    return undo_changeno;
}

/**/
zlong
get_undo_limit_change(UNUSED(Param pm))
{
    return undo_limitno;
}

/**/
void
set_undo_limit_change(UNUSED(Param pm), zlong value)
{
    undo_limitno = value;
}
