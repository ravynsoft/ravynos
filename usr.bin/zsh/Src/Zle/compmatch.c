/*
 * compmatch.c - the complete module, completion matching code
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

#include "complete.mdh"
#include "compmatch.pro"

/*
 * This compares two cpattern lists and returns non-zero if they are
 * equal (N.B. opposite sense to usual *cmp()).
 *
 * The old version of this didn't worry about whether the lists
 * were the same length.  This one does.  It's hard to see how
 * that can be wrong even if it's unnecessary.
 */

/**/
static int
cpatterns_same(Cpattern a, Cpattern b)
{
    while (a) {
	if (!b)
	    return 0;
	if (a->tp != b->tp)
	    return 0;
	switch (a->tp) {
	case CPAT_CCLASS:
	case CPAT_NCLASS:
	case CPAT_EQUIV:
	    /*
	     * Patterns can actually match the same even if
	     * the range strings don't compare differently, but
	     * I don't think we need to handle that subtlety.
	     */
	    if (strcmp(a->u.str, b->u.str) != 0)
		return 0;
	    break;

	case CPAT_CHAR:
	    if (a->u.chr != b->u.chr)
		return 0;
	    break;

	default:
	    /* here to silence compiler */
	    break;
	}

	a = a->next;
	b = b->next;
    }
    return !b;
}

/* This compares two cmatchers and returns non-zero if they are equal. */

/**/
static int
cmatchers_same(Cmatcher a, Cmatcher b)
{
    return (a == b ||
	    (a->flags == b->flags &&
	     a->llen == b->llen && a->wlen == b->wlen &&
	     (!a->llen || cpatterns_same(a->line, b->line)) &&
	     (a->wlen <= 0 || cpatterns_same(a->word, b->word)) &&
	     (!(a->flags & (CMF_LEFT | CMF_RIGHT)) ||
	      (a->lalen == b->lalen && a->ralen == b->ralen &&
	       (!a->lalen || cpatterns_same(a->left, b->left)) &&
	       (!a->ralen || cpatterns_same(a->right, b->right))))));
}

/* Add the given matchers to the bmatcher list. */

/**/
mod_export void
add_bmatchers(Cmatcher m)
{
    Cmlist old = bmatchers, *q = &bmatchers, n;

    for (; m; m = m->next) {
	if ((!m->flags && m->wlen > 0 && m->llen > 0) ||
	    (m->flags == CMF_RIGHT && m->wlen < 0 && !m->llen)) {
	    *q = n = (Cmlist) zhalloc(sizeof(struct cmlist));
	    n->matcher = m;
	    q = &(n->next);
	}
    }
    *q = old;
}

/* This is called when the matchers in the mstack have changed to
 * ensure that the bmatchers list contains no matchers not in mstack. */

/**/
mod_export void
update_bmatchers(void)
{
    Cmlist p = bmatchers, ms;
    Cmatcher mp;
    int t;

    while (p) {
	t = 0;
	for (ms = mstack; ms && !t; ms = ms->next)
	    for (mp = ms->matcher; mp && !t; mp = mp->next)
		t = cmatchers_same(mp, p->matcher);

	p = p->next;
	if (!t) {
	    bmatchers = p;
	}
    }
}

/* This returns a new Cline structure. */

/**/
Cline
get_cline(char *l, int ll, char *w, int wl, char *o, int ol, int fl)
{
    Cline r;

    /* Prefer to take it from the buffer list (freecl), if there
     * is none, allocate a new one. */

    if ((r = freecl))
	freecl = r->next;
    else
	r = (Cline) zhalloc(sizeof(*r));

    r->next = NULL;
    r->line = l; r->llen = ll;
    r->word = w; r->wlen = wl;
    DPUTS(wl > 0 && !*w, "Bad word");
    r->orig = o; r->olen = ol;
    r->slen = 0;
    r->flags = fl;
    r->prefix = r->suffix = NULL;
    r->min = r->max = 0;
    return r;
}

/* This frees a cline list. */

/**/
void
free_cline(Cline l)
{
    Cline n;

    while (l) {
	n = l->next;
	l->next = freecl;
	freecl = l;
	free_cline(l->prefix);
	free_cline(l->suffix);
	l = n;
    }
}

/* Copy a cline list. */

/**/
Cline
cp_cline(Cline l, int deep)
{
    Cline r = NULL, *p = &r, t, lp = NULL;

    while (l) {
	if ((t = freecl))
	    freecl = t->next;
	else
	    t = (Cline) zhalloc(sizeof(*t));
	memcpy(t, l, sizeof(*t));
	if (deep) {
	    if (t->prefix)
		t->prefix = cp_cline(t->prefix, 0);
	    if (t->suffix)
		t->suffix = cp_cline(t->suffix, 0);
	}
	*p = lp = t;
	p = &(t->next);
	l = l->next;
    }
    *p = NULL;

    return r;
}

/* Calculate the length of a cline and its sub-lists. */

/**/
int
cline_sublen(Cline l)
{
    int len = ((l->flags & CLF_LINE) ? l->llen : l->wlen);

    if (l->olen && !((l->flags & CLF_SUF) ? l->suffix : l->prefix))
	len += l->olen;
    else {
	Cline p;

	for (p = l->prefix; p; p = p->next)
	    len += ((p->flags & CLF_LINE) ? p->llen : p->wlen);
	for (p = l->suffix; p; p = p->next)
	    len += ((p->flags & CLF_LINE) ? p->llen : p->wlen);
    }
    return len;
}

/* Set the lengths in the cline lists. */

/**/
void
cline_setlens(Cline l, int both)
{
    while (l) {
	l->min = cline_sublen(l);
	if (both)
	    l->max = l->min;
	l = l->next;
    }
}

/* This sets the CLF_MATCHED flag in the given clines. */

/**/
void
cline_matched(Cline p)
{
    while (p) {
	p->flags |= CLF_MATCHED;
	cline_matched(p->prefix);
	cline_matched(p->suffix);

	p = p->next;
    }
}

/* This reverts the order of the elements of the given cline list and
 * returns a pointer to the new head. */

/**/
Cline
revert_cline(Cline p)
{
    Cline r = NULL, n;

    while (p) {
	n = p->next;
	p->next = r;
	r = p;
	p = n;
    }
    return r;
}

/* Global variables used during matching: a char-buffer for the string to
 * use for the match, and two cline lists for the two levels we use. */

/**/
char *matchbuf = NULL;
/**/
int matchbuflen = 0, matchbufadded;

/**/
Cline matchparts, matchlastpart;
/**/
Cline matchsubs, matchlastsub;

/* This initialises the variables above. */

/**/
static void
start_match(void)
{
    if (matchbuf)
	*matchbuf = '\0';
    matchbufadded = 0;
    matchparts = matchlastpart = matchsubs = matchlastsub = NULL;
}

/* This aborts a matching, freeing the cline lists build. */

/**/
static void
abort_match(void)
{
    free_cline(matchparts);
    free_cline(matchsubs);
    matchparts = matchsubs = NULL;
}

/* This adds a new string in the static char buffer. The arguments are
 * the matcher used (if any), the strings from the line and the word
 * and the length of the string from the word. The last argument is
 * non-zero if we are matching a suffix (where the given string has to 
 * be prepended to the contents of the buffer). */

/**/
static void
add_match_str(Cmatcher m, char *l, char *w, int wl, int sfx)
{
    /* Get the string and length to insert: either from the line 
     * or from the match. */
    if (m && (m->flags & CMF_LINE)) {
	wl = m->llen; w = l;
    }
    if (wl) {
	/* Probably resize the buffer. */
	if (matchbuflen - matchbufadded <= wl) {
	    int blen = matchbuflen + wl + 20;
	    char *buf;

	    buf = (char *) zalloc(blen);
	    if (matchbuf) {
		memcpy(buf, matchbuf, matchbuflen);
		zfree(matchbuf, matchbuflen);
	    }
#ifdef DEBUG
	    else {
		DPUTS(matchbuflen, "matchbuflen with no matchbuf");
	    }
#endif
	    matchbuf = buf;
	    matchbuflen = blen;
	}
	/* Insert the string. */
	if (sfx) {
	    memmove(matchbuf + wl, matchbuf, matchbufadded + 1);
	    memcpy(matchbuf, w, wl);
	} else
	    memcpy(matchbuf + matchbufadded, w, wl);
	matchbufadded += wl;
	matchbuf[matchbufadded] = '\0';
    }
}

/* This adds a cline for a word-part during matching. Arguments are the
 * matcher used, pointers to the line and word strings for the anchor,
 * a pointer to the original line string for the whole part, the string
 * before (or after) the anchor that has not yet been added, the length
 * of the line-string for that, and a flag saying if we are matching a 
 * suffix. */

/**/
static void
add_match_part(Cmatcher m, char *l, char *w, int wl,
	       char *o, int ol, char *s, int sl, int osl, int sfx)
{
    Cline p, lp, lprem;

    /* If the anchors are equal, we keep only one. */

    if (l && !strncmp(l, w, wl))
	l = NULL;

    /*
     * Split the new part into parts and turn the last one into a
     * `suffix' if we have a left anchor---don't do this if the last one
     * came from a right anchor before the end of the part we're
     * splitting.
     */

    p = bld_parts(s, sl, osl, &lp, &lprem);

    if (lprem && m && (m->flags & CMF_LEFT)) {
	lprem->flags |= CLF_SUF;
	lprem->suffix = lprem->prefix;
	lprem->prefix = NULL;
    }
    /* cline lists for suffixes are sorted from back to front, so we have
     * to revert the list we got. */
    if (sfx)
	p = revert_cline(lp = p);
    /* Now add the sub-clines we already had. */
    if (matchsubs) {
	if (sfx) {
	    Cline q;

	    if ((q = lp->prefix)) {
		while (q->next)
		    q = q->next;
		q->next = matchsubs;
	    } else
		lp->prefix = matchsubs;

	    matchlastsub->next = NULL;
	} else {
	    matchlastsub->next = p->prefix;
	    p->prefix = matchsubs;
	}
	matchsubs = matchlastsub = NULL;
    }
    /* Store the arguments in the last part-cline. */
    if (lp->llen || lp->wlen) {
	lp->next = get_cline(l, wl, w, wl, o, ol, CLF_NEW);
	lp = lp->next;
    } else {
	lp->line = l; lp->llen = wl;
	lp->word = w; lp->wlen = wl;
	DPUTS(wl > 0 && !*w, "Bad word");
	lp->orig = o; lp->olen = ol;
    }
    if (o || ol)
	lp->flags &= ~CLF_NEW;

    /* Finally, put the new parts on the list. */
    if (matchlastpart)
	matchlastpart->next = p;
    else
	matchparts = p;
    matchlastpart = lp;
}

/* This adds a new sub-cline. Arguments are the matcher and the strings from
 * the line and the word. */

/**/
static void
add_match_sub(Cmatcher m, char *l, int ll, char *w, int wl)
{
    int flags;
    Cline n;

    /* Check if we are interested only in the string from the line. */
    if (m && (m->flags & CMF_LINE)) {
	w = NULL; wl = 0;
	flags = CLF_LINE;
    } else
	flags = 0;

    /* And add the cline. */
    if (wl || ll) {
	Cline p, lp;

	if ((p = n = bld_parts(w, wl, ll, &lp, NULL)) && n != lp) {
	    for (; p->next != lp; p = p->next);

	    if (matchsubs) {
		matchlastsub->next = n->prefix;
		n->prefix = matchsubs;
	    }
	    matchsubs = matchlastsub = lp;

	    if (matchlastpart)
		matchlastpart->next = n;
	    else
		matchparts = n;
	    p->next = 0;
	    matchlastpart = p;
	} else {
	    n = get_cline(l, ll, w, wl, NULL, 0,
			  flags | ((m && m->wlen == -2) ? CLF_SKIP : 0));
	    if (matchlastsub)
		matchlastsub->next = n;
	    else
		matchsubs = n;
	    matchlastsub = n;
	}
    }
}

/* This tests if the string from the line l matches the word w. In *bpp
 * the offset for the brace is returned, in rwlp the length of the
 * matched prefix or suffix, not including the stuff before or after
 * the last anchor is given. When sfx is non-zero matching is done from
 * the ends of the strings backward, if test is zero, the global variables
 * above are used to build the string for the match and the cline. If
 * part is non-zero, we are satisfied if only a part of the line-string
 * is used (and return the length used). */

/**/
int
match_str(char *l, char *w, Brinfo *bpp, int bc, int *rwlp,
	  const int sfx, int test, int part)
{
    /* How many characters from the line string and from the word string are
     * yet to be matched. */
    int ll = strlen(l), lw = strlen(w);
    /* Number of characters from the line string and word string matched. */
    int il = 0, iw = 0;
    /* How many characters were matched exactly in the line and in the word. */
    int exact = 0, wexact = 0;
    int he = 0;
    int bslash;
    char *ow;
    Cmlist ms; /* loop variable */
    Cmatcher mp, lm = NULL;
    Brinfo bp = NULL;
    const int obc = bc;
    const int ind = (sfx ? -1 : 0);
    const int add = (sfx ? -1 : 1);
    const int original_ll = ll, original_lw = lw;

    /* INVARIANT: il+ll == original_ll; iw+lw == original_lw */

    if (!test) {
	start_match();
	bp = *bpp;
    }
    /* Adjust the pointers and get the values for subscripting and
     * incrementing. */

    if (sfx) {
	l += ll; w += lw;
    }
    /* ow will always point to the beginning (or end) of that sub-string
     * in w that wasn't put in the match-variables yet. */

    ow = w;

    /* If the brace is at the beginning, we have to treat it now. */

    if (!test && bp && bc >= bp->pos) {
	bp->curpos = bc;
	bp = bp->next;
    }
    /*** This once was: `while (ll && lw)', but then ignored characters at
     *   the end were not, well, ignored. */
    while (ll) {

	/* Hm, we unconditionally first tried the matchers for the cases
	 * where the beginnings of the line and word patterns match the
	 * same character(s).
	 * But first testing if the characters are the same is sooo
	 * much faster...
	 * Maybe it's OK to make same-character matching be preferred in
	 * recursive calls. At least, it /seems/ to work.
	 *
	 * Let's try.
	 *
	 * Update: this once tested `test && ...' to check for exact
	 * character matches only in recursive calls.  But then one
	 * can't complete `nom<TAB>' to `nomatch' with a match spec
	 * of `B:[nN][oO]=' because that will eat the `no'.
	 * But that would break completion of strings like `nonomatch'
	 * because the `B:[nN][oO]=' doesn't match the second `no'.
	 * For this we added the code below that can remove already
	 * accepted exact characters and try again, preferring match
	 * specs.
	 */

	bslash = 0;
	if (!sfx && lw && (!part || test) &&
	    (l[ind] == w[ind] ||
	     (bslash = (lw > 1 && w[ind] == '\\' && w[ind+1] == l[0])))) {
	    /* No matcher could be used, but the strings have the same
	     * character here, skip over it. */
	    l += add; w += (bslash ? (add + add) : add);
	    il++; iw += 1 + bslash;
	    ll--; lw -= 1 + bslash;
	    bc++;
	    exact++;
	    wexact += 1 + bslash;
	    if (!test)
		while (bp && bc >= (useqbr ? bp->qpos : bp->pos)) {
		    bp->curpos = matchbufadded + (w - ow) + obc;
		    bp = bp->next;
		}
	    lm = NULL;
	    he = 0;

	    continue;
	}
    retry:
	/* First try the matchers. Err... see above. */
	for (mp = NULL, ms = mstack; !mp && ms; ms = ms->next) {
	    for (mp = ms->matcher; mp; mp = mp->next) {
		if ((lm && lm == mp) ||
		    ((original_ll == ll || original_lw == lw) &&
		     (test == 1 || (test && !mp->left && !mp->right)) &&
		     mp->wlen < 0))
		    /* If we were called recursively, don't use `*' patterns
		     * at the beginning (avoiding infinite recursion). */
		    continue;

		if (mp->wlen < 0) {
		    /* `*'-pattern. */
		    /*
		     * Similar to the identically-named variable in the 'else'
		     * block.
		     */
		    int t;
		    /* 
		     * 1 iff the anchor and the word are on the same side of
		     * the line pattern; that is: if either
		     * - the anchor is on the left and we are matching
		     *   a prefix; or
		     * - the anchor is on the right and we are matching
		     *   a suffix.
		     */
		    int both;
		    /*
		     * Offset from the line pattern pointer ('l') to the start
		     * of the line pattern.
		     */
		    int loff;
		    /*
		     * Offset from the line pattern pointer ('l') to the start
		     * of the anchor.
		     */
		    int aoff;
		    /*
		     * The length of the line pattern.
		     */
		    int llen;
		    /*
		     * The length of the anchor.
		     *
		     * SEE: ap; aol, aop
		     */
		    int alen;
		    /*
		     * ### Related to 'zoff', which was removed in 2016.
		     */
		    int moff;
		    /*
		     * ### These two are related.
		     *
		     * ### They may have a relation similar to that of lw/iw
		     * ### (q.v.), at least during the 'for' loop.  They may be
		     * ### overloaded/repurposed after it.
		     */
		    int ct, ict;
		    /*
		     * The length of the OTHER anchor: the left anchor when
		     * we're anchored on the right, and of the right anchor
		     * when we're anchored on the left.
		     */
		    int aol;
		    /*
		     * LOST: Documentation comment.  Last seen 10 years ago in
		     * the temporal lobe.  Reward promised for its safe return.
		     * Contact zsh-workers@zsh.org.
		     */
		    char *tp;
		    /* 
		     * Temporary variable.  Used as temporary storage for a
		     *
		     *     {
		     *         () {
		     *           local foo="$foo"
		     *           foo[1]=bar
		     *           ...
		     *         }
		     *         (use original $foo here)
		     *     }
		     *
		     * operation.  Similar to savw.
		     */
		    char savl = 0;
		    /*
		     * The anchor on this end.
		     */
		    Cpattern ap;
		    /*
		     * The anchor on the other end.
		     */
		    Cpattern aop;

		    /* This is for `*' patterns, first initialise some
		     * local variables. */
		    llen = mp->llen;
		    if (mp->flags & CMF_LEFT) {
			alen = mp->lalen; aol = mp->ralen;
		    } else {
			alen = mp->ralen; aol = mp->lalen;
		    }
		    /* Give up if we don't have enough characters for the
		     * line-string and the anchor, or for both anchors in
		     * the case of the trial completion word. */
		    if (ll < llen + alen || lw < alen + aol)
			continue;

		    if (mp->flags & CMF_LEFT) {
			ap = mp->left; moff = alen; aop = mp->right;
			if (sfx) {
			    both = 0; loff = -llen; aoff = -(llen + alen);
			} else {
			    both = 1; loff = alen; aoff = 0;
			}
		    } else {
			ap = mp->right; moff = 0; aop = mp->left;
			if (sfx) {
			    both = 1; loff = -(llen + alen); aoff = -alen;
			} else {
			    both = 0; loff = 0; aoff = llen;
			}
		    }
		    /* Try to match the line pattern and the anchor. */
		    if (!pattern_match(mp->line, l + loff, NULL, NULL))
			continue;
		    if (ap) {
			if (!pattern_match(ap, l + aoff, NULL, NULL) ||
			    (both &&
			     (!pattern_match(ap, w + aoff, NULL, NULL) ||
			      (aol && aol <= aoff + iw &&
			       !pattern_match(aop, w + aoff - aol,
					      NULL, NULL)) ||
			      !match_parts(l + aoff, w + aoff, alen, part))))
				continue;
		    } else if (!both || ((mp->flags & CMF_INTER) ?
					 ((mp->flags & CMF_LINE) ? iw : il) :
					 (il || iw)))
			continue;

		    /* Fine, now we call ourselves recursively to find the
		     * string matched by the `*'. */
		    if (sfx && (savl = l[-(llen + alen)]))
			l[-(llen + alen)] = '\0';
		    for (t = 0, tp = w, ct = 0, ict = lw - alen + 1;
			 ict;
			 tp += add, ct++, ict--) {
			if ((both &&
			     (!ap || !test ||
			      !pattern_match(ap, tp + aoff, NULL, NULL) ||
			      (aol && aol <= aoff + ct + iw &&
			       !pattern_match(aop, tp + aoff - aol,
					      NULL, NULL)))) ||
			    (!both &&
			     pattern_match(ap, tp - moff, NULL, NULL) &&
			     (!aol || (aol <= iw + ct - moff &&
				       pattern_match(aop, tp - moff - aol,
						     NULL, NULL))) &&
			     (mp->wlen == -1 ||
			      match_parts(l + aoff , tp - moff,
						      alen, part)))) {
			    if (!both && mp->wlen == -1 &&
				!match_parts(l + aoff , tp - moff, alen, part))
				break;
			    if (sfx) {
				/* Call ourselves recursively with the
				 * anchor removed. */
				char savw;
				if ((savw = tp[-alen]))
				    tp[-alen] = '\0';
				t = match_str(l - ll, w - lw,
					      NULL, 0, NULL, sfx, 2, part);
				if (savw)
				    tp[-alen] = savw;
			    } else
				t = match_str(l + llen + moff, tp + moff,
					      NULL, 0, NULL, sfx, 1, part);
			    if (t || (mp->wlen == -1 && !both))
				break;
			}
		    }
		    ict = ct;
		    if (sfx && savl)
			l[-(llen + alen)] = savl;

		    /* Have we found a position in w where the rest of l
		     * matches? */
		    if (!t)
			continue;

		    /* Yes, add the strings and clines if this is a 
		     * top-level call. */
		    if (!test && (!he || (llen + alen))) {
			char *op, *lp, *map, *wap, *wmp;
			int ol;

			if (sfx) {
			    op = w; ol = ow - w; lp = l - (llen + alen);
			    map = tp - alen;
			    if (mp->flags & CMF_LEFT) {
				wap = tp - alen; wmp = tp;
			    } else {
				wap = w - alen; wmp = tp - alen;
			    }
			} else {
			    op = ow; ol = w - ow; lp = l;
			    map = ow;
			    if (mp->flags & CMF_LEFT) {
				wap = w; wmp = w + alen;
			    } else {
				wap = tp; wmp = ow;
			    }
			}
			/* If the matcher says that we are only interested
			 * in the line pattern, we just add that and the
			 * anchor and the string not added yet. Otherwise
			 * we add a new part. */
			if (mp->flags & CMF_LINE) {
			    add_match_str(NULL, NULL, op, ol, sfx);
			    add_match_str(NULL, NULL, lp, llen + alen, sfx);
			    add_match_sub(NULL, NULL, ol, op, ol);
			    add_match_sub(NULL, NULL, llen + alen,
					  lp, llen + alen);
			} else if (sfx) {
			    add_match_str(NULL, NULL,
					  map, ct + ol + alen, sfx);
			    add_match_part(mp, l + aoff, wap, alen,
					   l + loff, llen, op, ol, ol, sfx);
			    add_match_sub(NULL, NULL, 0, wmp, ct);
			} else {
			    add_match_str(NULL, NULL,
					  map, ct + ol + alen, sfx);
			    if (both) {
				add_match_sub(NULL, NULL, ol, op, ol);
				ol = -1;
			    } else
				ct += ol;
			    add_match_part(mp, l + aoff, wap, alen,
					   l + loff, llen, wmp, ct, ol, sfx);
			}
		    }
		    /* Now skip over the matched portion and the anchor. */
		    llen += alen; alen += ict;
		    if (sfx) {
			l -= llen; w -= alen;
		    } else {
			l += llen; w += alen;
		    }
		    ll -= llen; il += llen;
		    lw -= alen; iw += alen;
		    bc += llen;
		    exact = 0;

		    if (!test) {
			int bpc;
			while (bp &&
			       bc >= (bpc = (useqbr ? bp->qpos : bp->pos))) {
			    bp->curpos = matchbufadded + bpc - bc + obc;
			    bp = bp->next;
			}
		    }
		    ow = w;

		    if (!llen && !alen) {
			lm = mp;
			if (he) {
			    /* Signal the outer for loop to continue. */
			    mp = NULL;
			}
			else
			    he = 1;
		    } else {
			lm = NULL; he = 0;
		    }
		    break;
		} else if (ll >= mp->llen && lw >= mp->wlen) {
		    /* Non-`*'-pattern. */
		    /*
		     * Similar to the identically-named variable in the 'if'
		     * block.
		     */
		    int t = 1;
		    char *tl, *tw;
		    int tll, tlw, til, tiw;

		    /* We do this only if the line- and word-substrings
		     * are not equal. */
		    if (!(mp->flags & (CMF_LEFT | CMF_RIGHT)) &&
			mp->llen == mp->wlen &&
			!(sfx ? strncmp(l - mp->llen, w - mp->wlen, mp->llen) :
			  strncmp(l, w, mp->llen)))
			continue;

		    /* Using local variables to make the following
		     * independent of whether we match a prefix or a
		     * suffix. */
		    if (sfx) {
			tl = l - mp->llen; tw = w - mp->wlen;
			til = ll - mp->llen; tiw = lw - mp->wlen;
			tll = il + mp->llen; tlw = iw + mp->wlen;
		    } else {
			tl = l; tw = w;
			til = il; tiw = iw;
			tll = ll; tlw = lw;
		    }
		    if (mp->flags & CMF_LEFT) {
			/* Try to match the left anchor, if any. */
			if (til < mp->lalen || tiw < mp->lalen + mp->ralen)
			    continue;
			else if (mp->left)
			    t = pattern_match(mp->left, tl - mp->lalen,
					      NULL, NULL) &&
				pattern_match(mp->left, tw - mp->lalen,
					      NULL, NULL) &&
				(!mp->ralen ||
				 pattern_match(mp->right,
					       tw - mp->lalen - mp->ralen,
					       NULL, NULL));
			else
			    t = (!sfx && !((mp->flags & CMF_INTER) ?
					   ((mp->flags & CMF_LINE) ? iw : il) :
					   (il || iw)));
		    }
		    if (mp->flags & CMF_RIGHT) {
			/* Try to match the right anchor, if any. */
			if (tll < mp->llen + mp->ralen ||
			    tlw < mp->wlen + mp->ralen + mp->lalen)
			    continue;
			else if (mp->right)
			    t = pattern_match(mp->right,
					      /* tl + mp->llen - mp->ralen, */
					      tl + mp->llen,
					      NULL, NULL) &&
				pattern_match(mp->right,
					      /* tw + mp->wlen - mp->ralen, */
					      tw + mp->wlen,
					      NULL, NULL) &&
				(!mp->lalen ||
				 pattern_match(mp->left, tw + mp->wlen -
					       mp->ralen - mp->lalen,
					       NULL, NULL));
			else
			    t = (sfx && !((mp->flags & CMF_INTER) ?
					  ((mp->flags & CMF_LINE) ? iw : il) :
					  (il || iw)));
		    }
		    /* Now try to match the line and word patterns. */
		    if (!t || !pattern_match(mp->line, tl, mp->word, tw))
			continue;

		    /* Probably add the matched strings. */
		    if (!test) {
			if (sfx)
			{
			    if (ow >= w)
				add_match_str(NULL, NULL, w, ow - w, sfx);
			}
			else
			{
			    if (w >= ow)
				add_match_str(NULL, NULL, ow, w - ow, sfx);
			}
			add_match_str(mp, tl, tw, mp->wlen, sfx);
			if (sfx)
			{
			    if (ow >= w)
				add_match_sub(NULL, NULL, 0, w, ow - w);
			}
			else
			{
			    if (w >= ow)
				add_match_sub(NULL, NULL, 0, ow, w - ow);
			}
			add_match_sub(mp, tl, mp->llen, tw, mp->wlen);
		    }
		    if (sfx) {
			l = tl;	w = tw;
		    } else {
			l += mp->llen; w += mp->wlen;
		    }
		    il += mp->llen; iw += mp->wlen;
		    ll -= mp->llen; lw -= mp->wlen;
		    bc += mp->llen;
		    exact = 0;

		    if (!test) {
			int bpc;
			while (bp &&
			       bc >= (bpc = (useqbr ? bp->qpos : bp->pos))) {
			    bp->curpos = matchbufadded + bpc - bc + obc;
			    bp = bp->next;
			}
		    }
		    ow = w;
		    lm = NULL;
		    he = 0;
		    break;
		}
	    }
	}
	if (mp)
	    continue;

	/* Same code as at the beginning, used in top-level calls. */

	bslash = 0;
	if ((!test || sfx) && lw &&
	    (l[ind] == w[ind] ||
	     (bslash = (lw > 1 && w[ind] == '\\' && w[ind+1] == l[0])))) {
	    /* No matcher could be used, but the strings have the same
	     * character here, skip over it. */
	    l += add; w += (bslash ? (add + add ) : add);
	    il++; iw += 1 + bslash;
	    ll--; lw -= 1 + bslash;
	    bc++;
	    if (!test)
		while (bp && bc >= (useqbr ? bp->qpos : bp->pos)) {
		    bp->curpos = matchbufadded + (sfx ? (ow - w) : (w - ow)) + obc;
		    bp = bp->next;
		}
	    lm = NULL;
	    he = 0;
	} else {

	    if (!lw)
		break;

	    if (exact && !part) {
		/* If we just accepted some characters directly (at the
		 * beginning of the loop) and now can't match any further,
		 * we go back to before those characters and try again,
		 * preferring match specs this time. */

		il -= exact; iw -= wexact;
		ll += exact; lw += wexact;
		bc -= exact;
		l -= add * exact; w -= add * wexact;

		exact = wexact = 0;

		goto retry;
	    }
	    /* No matcher and different characters: l does not match w. */
	    if (test)
		return 0;

	    abort_match();

	    return -1;
	}
    }
    /* If this is a recursive call, we just return if l matched w or not. */
    if (test)
	return (part || !ll);

    /* In top-level calls, if ll is non-zero (unmatched portion in l),
     * we have to free the collected clines. */
    if (!part && ll) {
	abort_match();

	return -1;
    }
    if (rwlp)
	*rwlp = iw - (sfx ? ow - w : w - ow);

    /* If we matched a suffix, the anchors stored in the top-clines
     * will be in the wrong clines: shifted by one. Adjust this. */
    if (sfx && matchparts) {
	Cline t, tn, s;

	if (matchparts->prefix || matchparts->suffix) {
	    t = get_cline(NULL, 0, NULL, 0, NULL, 0, 0);
	    t->next = matchparts;
	    if (matchparts->prefix)
		t->prefix = (Cline) 1;
	    else
		t->suffix = (Cline) 1;
	    matchparts = t;
	}
	for (t = matchparts; (tn = t->next); t = tn) {
	    s = (tn->prefix ? tn->prefix : tn->suffix);
	    if (t->suffix)
		t->suffix = s;
	    else
		t->prefix = s;
	}
	t->prefix = t->suffix = NULL;
    }
    /* Finally, return the number of matched characters. */

    *bpp = bp;
    return (part ? il : iw);
}

/* Wrapper for match_str(), only for a certain length and only doing
 * the test. */

/**/
static int
match_parts(char *l, char *w, int n, int part)
{
    char lsav = l[n], wsav = w[n];
    int ret;

    if (lsav)
	l[n] = '\0';
    if (wsav)
	w[n] = '\0';
    ret = match_str(l, w, NULL, 0, NULL, 0, 1, part);
    if (lsav)
	l[n] = lsav;
    if (wsav)
	w[n] = wsav;

    return ret;
}

/* Check if the word w is matched by the strings in pfx and sfx (the prefix
 * and the suffix from the line) or the pattern cp. In clp a cline list for
 * w is returned.
 * qu is non-zero if the words has to be quoted before processed any
 * further: the value 2 indicates file quoting.
 * bpl and bsl are used to report the positions where the brace-strings in
 * the prefix and the suffix have to be re-inserted if this match is inserted
 * in the line.
 * The return value is the string to use as a completion or NULL if the prefix
 * and the suffix don't match the word w. */

/**/
mod_export char *
comp_match(char *pfx, char *sfx, char *w, Patprog cp, Cline *clp, int qu,
	   Brinfo *bpl, int bcp, Brinfo *bsl, int bcs, int *exact)
{
    char *r = NULL;
    int onoerrs = noerrs;

    if (cp) {
	/* We have a globcomplete-like pattern, just use that. */
	int wl;
	char *teststr;

	r = w;
	if (!qu) {
	    /*
	     * If we're not quoting the strings, that means they're
	     * already quoted (?) and so when we test them against
	     * a pattern we have to remove the quotes else we will
	     * end up trying to match against the quote characters.
	     *
	     * Almost certainly this fails in some complicated cases
	     * but it should catch the basic ones.
	     */
	    teststr = dupstring(r);
	    tokenize(teststr);
	    noerrs = 1;
	    if (parse_subst_string(teststr))
		teststr = r;
	    else {
		remnulargs(teststr);
		untokenize(teststr);
	    }
	    noerrs = onoerrs;
	} else
	    teststr = r;
	if (!pattry(cp, teststr))
	    return NULL;
    
	r = (qu == 2 ? tildequote(r, 0) : multiquote(r, !qu));

	/* We still break it into parts here, trying to build a sensible
	 * cline list for these matches, too. */
	w = dupstring(w);
	wl = strlen(w);
	*clp = bld_parts(w, wl, wl, NULL, NULL);
	*exact = 0;
    } else {
	Cline pli, plil;
	int mpl, rpl, wl;

	w = (qu == 2 ? tildequote(w, 0) : multiquote(w, !qu));
	wl = strlen(w);

	/* Always try to match the prefix. */

	useqbr = qu;
	if ((mpl = match_str(pfx, w, bpl, bcp, &rpl, 0, 0, 0)) < 0)
	    return NULL;

	if (sfx && *sfx) {
	    int wpl = matchbufadded, msl, rsl;
	    VARARR(char, wpfx, wpl);
	    Cline mli, mlil;

	    /* We also have a suffix to match, so first save the
	     * contents of the global matching variables. */
	    memcpy(wpfx, matchbuf, wpl);
	    if (matchsubs) {
		Cline tmp = get_cline(NULL, 0, NULL, 0, NULL, 0, 0);

		tmp->prefix = matchsubs;
		if (matchlastpart)
		    matchlastpart->next = tmp;
		else
		    matchparts = tmp;
	    }
	    pli = matchparts;
	    plil = matchlastpart;

	    /* The try to match the suffix. */

	    if ((msl = match_str(sfx, w + mpl, bsl, bcs, &rsl, 1, 0, 0)) < 0) {
		free_cline(pli);

		return NULL;
	    }
	    /* Matched, so add the string in the middle and the saved
	     * string for the prefix, and build a combined cline list
	     * for the prefix and the suffix. */
	    if (matchsubs) {
		Cline tmp = get_cline(NULL, 0, NULL, 0, NULL, 0, CLF_SUF);

		tmp->suffix = matchsubs;
		if (matchlastpart)
		    matchlastpart->next = tmp;
		else
		    matchparts = tmp;
	    }
	    add_match_str(NULL, NULL, w + rpl, wl - rpl - rsl, 1);
	    add_match_str(NULL, NULL, wpfx, wpl, 1);

	    mli = bld_parts(w + rpl, wl - rpl - rsl,
			    (mpl - rpl) + (msl - rsl), &mlil, NULL);
	    mlil->flags |= CLF_MID;
	    mlil->slen = msl - rsl;
	    mlil->next = revert_cline(matchparts);

	    if (plil)
		plil->next = mli;
	    else
		pli = mli;
	} else {
	    /* Only a prefix, add the string and a part-cline for it. */
	    add_match_str(NULL, NULL, w + rpl, wl - rpl, 0);

	    add_match_part(NULL, NULL, NULL, 0, NULL, 0, w + rpl, wl - rpl,
			   mpl - rpl, 0);
	    pli = matchparts;
	}
	r = dupstring(matchbuf ? matchbuf : "");

	*clp = pli;

	/* Test if the string built is equal to the one from the line. */
	if (sfx && *sfx) {
	    int pl = strlen(pfx);

	    *exact = (!strncmp(pfx, w, pl) && !strcmp(sfx, w + pl));
	} else
	    *exact = !strcmp(pfx, w);
    }
    if (!qu)
	hasunqu = 1;

    return r;
}


/*
 * Guts of a single pattern for pattern_match().
 * Return non-zero if match successful.
 * If the class was an equivalence, return 1 + the index into
 * the equivalence class (see pattern.c for how this is calculated).
 */

/**/
mod_export convchar_t
pattern_match1(Cpattern p, convchar_t c, int *mtp)
{
    convchar_t ind;

    *mtp = 0;
    switch (p->tp) {
    case CPAT_CCLASS:
	return PATMATCHRANGE(p->u.str, CONVCAST(c), NULL, NULL);

    case CPAT_NCLASS:
	return !PATMATCHRANGE(p->u.str, CONVCAST(c), NULL, NULL);

    case CPAT_EQUIV:
	if (PATMATCHRANGE(p->u.str, CONVCAST(c), &ind, mtp))
	    return ind + 1;
	else
	    return 0;

    case CPAT_ANY:
	return 1;

    case CPAT_CHAR:
	return (p->u.chr == c);

    default:
	DPUTS(1, "bad matcher pattern type");
	return 0;
    }
}


/*
 * Use an equivalence to deduce the line character from the word, or
 * vice versa.  (If vice versa, then "line" and "word" are reversed
 * in what follows.  The logic is symmetric.)
 * lp is the line pattern.
 * wind is the index returned by a pattern match on the word pattern,
 * with type wmtp.
 * wchr is the word character.
 * Return CHR_INVALID if no matching character, else the character.
 *
 * Only makes sense if lp->tp == CPAT_EQUIV and the (unseen) word
 * pattern also has that type.
 */

/**/
mod_export convchar_t
pattern_match_equivalence(Cpattern lp, convchar_t wind, int wmtp,
			  convchar_t wchr)
{
    convchar_t lchr;
    int lmtp;

    if (!PATMATCHINDEX(lp->u.str, wind, &lchr, &lmtp)) {
	/*
	 * No equivalent.  No possible match; give up.
	 */
	return CHR_INVALID;
    }
    /*
     * If we matched an exact character rather than a range
     * type, return it.
     */
    if (lchr != CHR_INVALID)
	return lchr;

    /*
     * Check the match types.  We may want a case-changed
     * version of the word character.
     */
    if (wmtp == PP_UPPER && lmtp == PP_LOWER)
	return ZC_tolower(wchr);
    else if (wmtp == PP_LOWER && lmtp == PP_UPPER)
	return ZC_toupper(wchr);
    else if (wmtp == lmtp) {
	/*
	 * Be lenient and allow identical replacements
	 * for character classes, although in fact this
	 * doesn't give special functionality for equivalence
	 * classes.
	 */
	return wchr;
    } else {
	/*
	 * Non-matching generic types; this can't work.
	 */
	return CHR_INVALID;
    }
}

/*
 * Check if the given pattern matches the given string.
 * p is either an anchor or line pattern and string;
 * wp and wsc are word (candidate) pattern and string
 *
 * Check that characters match for {...} classes by comparing positions in the
 * strings.
 *
 * prestrict is a chain of patterns at least as long
 * as the line string.  In this case we are still assembling the line at
 * newline (which has been allocated but doesn't yet contain anything useful)
 * and must continue to do so as we go along; prestrict gives
 * restrictions on the line character to be applied along side the other
 * patterns.  In the simple case a restriction is a character to be put
 * in place; otherwise it is a set of possible characters and we have to
 * deduce an actual matching character.  Note prestrict is never an
 * equivalence class.  In extreme cases we can't deduce a unique
 * character; then the match fails.
 *
 * If prestrict is not NULL, s will be NULL.
 */

/**/
static int
pattern_match_restrict(Cpattern p, Cpattern wp, convchar_t *wsc, int wsclen,  
		       Cpattern prestrict, ZLE_STRING_T new_line)
{
    convchar_t c;
    convchar_t ind, wind;
    int mt, wmt;

    while (p && wp && wsclen && prestrict) {
	/* First test the word character */
	wind = pattern_match1(wp, *wsc, &wmt);
	if (!wind)
	    return 0;

	/*
	 * Now the line character; deal with the case where
	 * we don't yet have it, only a restriction on it.
	 */
	if (prestrict->tp == CPAT_CHAR) {
	    /*
	     * Easy case: restricted to an exact character on
	     * the line.  Proceed as normal.
	     */
	    c = prestrict->u.chr;
	} else {
	    if (p->tp == CPAT_CHAR) {
		/*
		 * Normal line pattern is an exact character:  as
		 * long as this matches prestrict, we can proceed
		 * as usual.
		 */
		c = p->u.chr;
	    } else if (p->tp == CPAT_EQUIV) {
		/*
		 * An equivalence, so we can deduce the character
		 * backwards from the word pattern and see if it
		 * matches prestrict.
		 */
		if ((c = pattern_match_equivalence(p, wind, wmt, *wsc)) ==
		    CHR_INVALID)
		    return 0;
	    } else {
		/*
		 * Not an equivalence, so that means we must match
		 * the word (not just the word pattern), so grab it
		 * and make sure it fulfills our needs.  I think.
		 * Not 100% sure about that, but what else can
		 * we do?  We haven't actually been passed a string
		 * from the command line.
		 */
		c = *wsc;
	    }
	    /* Character so deduced must match the restriction. */
	    if (!pattern_match1(prestrict, c, &mt))
		return 0;
	}

	/*
	 * If either is "?", they match each other; no further tests.
	 * Apply this even if the character wasn't convertible;
	 * there's no point trying to be clever in that case.
	 */
	if (p->tp != CPAT_ANY || wp->tp != CPAT_ANY)
	{
	    ind = pattern_match1(p, c, &mt);
	    if (!ind)
		return 0;
	    if (ind != wind)
		return 0;
	    if (mt != wmt) {
		/*
		 * Special case if matching lower vs. upper or
		 * vice versa.  The transformed characters must match.
		 * We don't need to check the transformation is
		 * the appropriate one for each character separately,
		 * since that was done in pattern_match1(), so just
		 * compare lower-cased versions of both.
		 */
		if ((mt == PP_LOWER || mt == PP_UPPER) &&
		    (wmt == PP_LOWER || wmt == PP_UPPER)) {
		    if (ZC_tolower(c) != ZC_tolower(*wsc))
			return 0;
		} else {
		    /* Other different classes can't match. */
		    return 0;
		}
	    }
	}

	/* We need to assemble the line */
	*new_line++ = (ZLE_CHAR_T)c;
	prestrict = prestrict->next;
	wsc++;
	wsclen--;
	p = p->next;
	wp = wp->next;
    }

    while (p && prestrict) {
	/*
	 * As above, but with even less info to go on.
	 * (Can this happen?)  At least handle the cases where
	 * one of our patterns has given us a specific character.
	 */
	if (prestrict->tp == CPAT_CHAR) {
	    c = prestrict->u.chr;
	} else {
	    if (p->tp == CPAT_CHAR) {
		c = p->u.chr;
	    } else {
		/*
		 * OK.  Here we are in a function with just a line
		 * pattern and another pattern to restrict the
		 * characters that can go on the line, and no actual
		 * characters.  We're matching two patterns against
		 * one another to generate a character to insert.
		 * This is a bit too psychedelic, so I'm going to
		 * bail out now.  See you on the ground.
		 */
		return 0;
	    }
	    if (!pattern_match1(prestrict, c, &mt))
		return 0;
	}
	if (!pattern_match1(p, c, &mt))
	    return 0;
	p = p->next;
	*new_line++ = (ZLE_CHAR_T)c;
	prestrict = prestrict->next;
    }

    if (prestrict) {
	/* Restriction with nothing to match */
	return 0;
    }

    while (wp && wsclen) {
	/* No funny business when we only have the word pattern. */
	if (!pattern_match1(wp, *wsc, &wmt))
	    return 0;
	wp = wp->next;
	wsc++;
	wsclen--;
    }

    return 1;
}


/*
 * The usual version of pattern matching, without the line string
 * being handled by restriction.
 *
 * Check if the given pattern matches the given string.
 * p and  s are either anchor or line pattern and string;
 * wp and ws are word (candidate) pattern and string
 *
 * If only one pattern is given, we just check if characters match.
 * If both line and word are given, we check that characters match
 * for {...} classes by comparing positions in the strings.
 *
 * Patterns and strings are always passed in pairs, so it is enough
 * to check for non-NULL wp. p should always be present.
 */
/**/
mod_export int
pattern_match(Cpattern p, char *s, Cpattern wp, char *ws)
{
    convchar_t c, wc;
    convchar_t ind, wind;
    int len = 0, wlen = 0, mt, wmt;

    while (p && wp && *s && *ws) {
	/* First test the word character */
	wc = unmeta_one(ws, &wlen);
	wind = pattern_match1(wp, wc, &wmt);
	if (!wind)
	    return 0;

	/*
	 * Now the line character.
	 */
	c = unmeta_one(s, &len);
	/*
	 * If either is "?", they match each other; no further tests.
	 * Apply this even if the character wasn't convertible;
	 * there's no point trying to be clever in that case.
	 */
	if (p->tp != CPAT_ANY || wp->tp != CPAT_ANY)
	{
	    ind = pattern_match1(p, c, &mt);
	    if (!ind)
		return 0;
	    if (ind != wind)
		return 0;
	    if (mt != wmt) {
		/*
		 * Special case if matching lower vs. upper or
		 * vice versa.  The transformed characters must match.
		 * We don't need to check the transformation is
		 * the appropriate one for each character separately,
		 * since that was done in pattern_match1(), so just
		 * compare lower-cased versions of both.
		 */
		if ((mt == PP_LOWER || mt == PP_UPPER) &&
		    (wmt == PP_LOWER || wmt == PP_UPPER)) {
		    if (ZC_tolower(c) != ZC_tolower(wc))
			return 0;
		} else {
		    /* Other different classes can't match. */
		    return 0;
		}
	    }
	}

	s += len;
	ws += wlen;
	p = p->next;
	wp = wp->next;
    }

    while (p && *s) {
	c = unmeta_one(s, &len);
	if (!pattern_match1(p, c, &mt))
	    return 0;
	p = p->next;
	s += len;
    }

    while (wp && *ws) {
	wc = unmeta_one(ws, &wlen);
	if (!pattern_match1(wp, wc, &wmt))
	    return 0;
	wp = wp->next;
	ws += wlen;
    }

    return 1;
}


/* This splits the given string into a list of cline structs, separated
 * at those places where one of the anchors of an `*' pattern was found.
 * plen gives the number of characters on the line that matched this
 * string.
 *
 * In *lp, if lp is not NULL, we return a pointer to the last cline struct we
 * build.
 *
 * In *lprem, if lprem is not NULL, we return a pointer to the last
 * cline struct we build if it came from the remainder of the
 * line rather than from a right anchor match, else NULL.
 */

/**/
Cline
bld_parts(char *str, int len, int plen, Cline *lp, Cline *lprem)
{
    Cline ret = NULL, *q = &ret, n = NULL;
    Cmlist ms;
    Cmatcher mp;
    int t, op = plen;
    char *p = str, *os = str;

    while (len) {
	for (t = 0, ms = bmatchers; ms && !t; ms = ms->next) {
	    mp = ms->matcher;
	    if (mp && mp->flags == CMF_RIGHT && mp->wlen < 0 && mp->ralen &&
		!mp->llen && len >= mp->ralen && (str - os) >= mp->lalen &&
		pattern_match(mp->right, str, NULL, NULL) &&
		(!mp->lalen ||
		 ((str - os) >= mp->lalen &&
		  pattern_match(mp->left, str - mp->lalen, NULL, NULL)))) {
		int olen = str - p, llen;

		/* We found an anchor, create a new cline. The NEW flag
		 * is set if the characters before the anchor were not
		 * on the line. */
		*q = n = get_cline(NULL, mp->ralen, str, mp->ralen, NULL, 0,
				   ((plen <= 0) ? CLF_NEW : 0));

		/* If there were any characters before the anchor, add
		 * them as a cline struct. */

		if (p != str) {
		    llen = (op < 0 ? 0 : op);

		    if (llen > olen)
			llen = olen;
		    n->prefix = get_cline(NULL, llen, p, olen, NULL, 0, 0);
		}
		q = &(n->next);
		str += mp->ralen; len -= mp->ralen;
		plen -= mp->ralen;
		op -= olen;
		p = str;
		t = 1;
	    }
	}
	if (!t) {
	    /* No anchor was found here, skip. */
	    str++; len--;
	    plen--;
	}
    }
    /* This is the cline struct for the remaining string at the end. */

    if (p != str) {
	int olen = str - p, llen = (op < 0 ? 0 : op);

        *q = n = get_cline(NULL, 0, NULL, 0, NULL, 0, (plen <= 0 ? CLF_NEW : 0));

	if (llen > olen)
	    llen = olen;
	n->prefix = get_cline(NULL, llen, p, olen, NULL, 0, 0);
	if (lprem)
	    *lprem = n;
    }
    else if (!ret) {
        *q = n =
	    get_cline(NULL, 0, NULL, 0, NULL, 0, (plen <= 0 ? CLF_NEW : 0));
	if (lprem)
	    *lprem = n;
    } else if (lprem) {
	*lprem = NULL;
    }

    if (n)
        n->next = NULL;

    if (lp)
	*lp = n;

    return ret;
}


/*
 * This builds all the possible line patterns for the pattern pat in the
 * buffer line.  Then we test if this line matches the string given by
 * wlen and word.
 *
 * The matcher  ) wpat, containing pattern that matched previously
 *   mp gives   ) lpat, containing the pattern for line we build
 * line is the line we are assembling; it is initially empty
 * mword is a string that matched wpat before
 * word is string that we try to match now
 *
 * The return value is the length of the string matched in the word, it
 * is zero if we couldn't build a line that matches the word.
 */

/**/
static int
bld_line(Cmatcher mp, ZLE_STRING_T line, char *mword, char *word,
	 int wlen, int sfx)
{
    Cpattern lpat = mp->line;
    Cpattern wpat = mp->word;
    Cpattern curgenpat;
    Cmlist ms;
    int llen, rl, l;
    convchar_t convchr, *wordcp;
    VARARR(convchar_t, wordchars, wlen);
    VARARR(struct cpattern, genpatarr, mp->llen);

    /*
     * We may need to start the "word" array from the end.  This
     * is much easier if we convert it to an array of (possibly wide)
     * characters.
     */
    MB_METACHARINIT();
    for (l = wlen, wordcp = wordchars; l; l--) {
	int charlen = MB_METACHARLENCONV(word, &convchr);
#ifdef MULTIBYTE_SUPPORT
	if (convchr == WEOF)
	    convchr = (*word == Meta) ? word[1] ^ 32 : *word;
#endif
	*wordcp++ = convchr;
	word += charlen;
    }

    /*
     * Loop over all characters.  At this stage, line is an empty
     * space of length llen (not counting the null byte) which we assemble as
     * we go along.
     *
     * However, first we need to know what characters can appear at each
     * point in the line.  For this we assemble an list genpatarr of the
     * same length as the line.  (It's convenient to store this as an
     * array but it's linked as a list, too.)  If there are equivalences
     * we use mword to derive the equivalent character; when we've
     * reached the end of mword, equivalences are treated just like
     * ordinary character classes.  For character classes we just attach
     * the class to the genpatarr list and apply it as a restriction
     * when we finally match the line against the set of matchers.
     */
    curgenpat = genpatarr;
    MB_METACHARINIT();
    while (lpat) {
	convchar_t wchr, wind;
	int wmtp, mwordlen;
	/*
	 * If the line pattern is an equivalence, query wpat to find the
	 * word part of the equivalence.  If we don't find one we don't try
	 * equivalencing but use lpat as an ordinary match.  (It's not
	 * entirely clear to me this is the correct behaviour on a
	 * failed character match within the equivalence, but that was
	 * the behaviour of the old logic that this replaces.)
	 */
	if (lpat->tp == CPAT_EQUIV && wpat && *mword) {
	    mwordlen = MB_METACHARLENCONV(mword, &wchr);
	    wind = pattern_match1(wpat, wchr, &wmtp);
	    wpat = wpat->next;
	    mword += mwordlen;
	} else
	    wind = 0;
	if (wind) {
	    /*
	     * Successful match for word side of equivalence.
	     * Find the line equivalent.
	     */
	    convchar_t lchr;
	    if ((lchr = pattern_match_equivalence(lpat, wind, wmtp, wchr))
		== CHR_INVALID) {
		/*
		 * No equivalent.  No possible match; give up.
		 */
		return 0;
	    }
	    /*
	     * We now have an exact character to match,
	     * so make up a pattern element for it.
	     */
	    curgenpat->tp = CPAT_CHAR;
	    curgenpat->u.chr = lchr;
	} else {
	    /*
	     * Not an equivalence class, so we just keep the
	     * test in the lpat as it is.
	     */
	    curgenpat->tp = lpat->tp;
	    if (lpat->tp == CPAT_CHAR)
		curgenpat->u.chr = lpat->u.chr;
	    else if (lpat->tp != CPAT_ANY) {
		/*
		 * The string isn't modified and is only needed in calls from
		 * this function, so we don't even need to copy it.
		 */
		curgenpat->u.str = lpat->u.str;
	    }
	}
	lpat = lpat->next;
	/*
	 * This linked list is defined above as an array.
	 * We could get away with just keeping it as an array
	 * and passing it down as such, but that's a bit icky
	 * since the generic linkage of Cpatterns is as a linked
	 * list and we should keep our local memory management
	 * problems to ourselvess.
	 */
	if (lpat)
	    curgenpat->next = curgenpat+1;
	else
	    curgenpat->next = NULL;
	curgenpat++;
    }

    /*
     * We now know how to match the word with the line patterns; let's
     * see if it does.  We will use the information in curgenpat if we
     * are successful to work out what character goes on the line.  This
     * is a bit hairy, as in "the Yeti is a creature that is a bit
     * hairy".
     */
    llen = mp->llen;
    rl = 0;

    if (sfx)
    {
	/*
	 * We need to work backwards from the end of both the
	 * word and the line strings.
	 */
	wordcp = wordchars + wlen;

	/*
	 * We construct the line from the end.
	 */
	line += llen;
	curgenpat = genpatarr + llen;
    } else {
	wordcp = wordchars;
	curgenpat = genpatarr;
    }

    /* we now reuse mp, lpat, wpat for the global matchers */
    MB_METACHARINIT();
    while (llen && wlen) {
	int wmtp;
	convchar_t *wp;
	Cpattern tmpgenpat;

	if (sfx) {
	    wp = wordcp - 1;
	    tmpgenpat = curgenpat - 1;
	} else {
	    tmpgenpat = curgenpat;
	    wp = wordcp;
	}
	if (pattern_match1(tmpgenpat, *wp, &wmtp))
	{
	    convchar_t lchr;
	    /*
	     * We can match the line character directly with the word
	     * character.  If the line character is a fixed one,
	     * keep it, since we went to all that trouble above,
	     * else if it's generic, keep the word character,
	     * since we have no choice.
	     */
	    if (tmpgenpat->tp == CPAT_CHAR)
		lchr = tmpgenpat->u.chr;
	    else
		lchr = *wp;

	    if (sfx)
		*--line = lchr;
	    else
		*line++ = lchr;

	    llen--;
	    wlen--;
	    rl++;

	    if (sfx) {
		wordcp = wp;
		curgenpat = tmpgenpat;
	    } else {
		if (llen)
		    curgenpat++;
		wordcp++;
	    }
	}
	else
	{
	    ZLE_CHAR_T *lp;
	    /*
	     * Need to loop over pattern matchers.
	     */
	    for (ms = bmatchers; ms; ms = ms->next) {
		mp = ms->matcher;
		/*
		 * This is the nightmare case: we have line and
		 * and word matchers and some pattern which restricts
		 * the value on the line without us knowing exactly
		 * what it is.  Dispatch to the special function
		 * for that.
		 */
		if (mp && !mp->flags && mp->wlen <= wlen &&
		    mp->llen <= llen)
		{
		    lp = line;
		    wp = wordcp;
		    tmpgenpat = curgenpat;

		    if (sfx) {
			lp -= mp->llen;
			wp -= mp->wlen;
			tmpgenpat -= mp->llen;
		    }

		    if (pattern_match_restrict(mp->line, mp->word, wp,
					       wlen - (wp - wordchars),
					       tmpgenpat, lp)) {
			/*
			 * Matched: advance over as many characters
			 * of the patterns and strings as
			 * we've done matches.
			 */
			if (sfx) {
			    line = lp;
			    wordcp = wp;
			    curgenpat = tmpgenpat;
			} else {
			    line += mp->llen;
			    wordcp += mp->wlen;
			    curgenpat += mp->llen;
			}
			llen -= mp->llen;
			wlen -= mp->wlen;
			rl += mp->wlen;
			break;
		    }
		}
	    }
	    if (!ms)
		return 0;	/* Didn't match, give up */
	}
    }
    if (!llen) {
	/* Unmatched portion in the line built, return matched length. */
	return rl;
    }
    return 0;
}

/* This builds a string that may be put on the line that fully matches the
 * given strings. The return value is NULL if no such string could be built
 * or that string in local static memory, dup it. */

/**/
static char *
join_strs(int la, char *sa, int lb, char *sb)
{
    static char *rs = NULL;
    static int rl = 0;

    Cmlist ms;
    Cmatcher mp;
    int t, bl;
    /** rr is the remaining length already allocated in rs */
    int rr = rl;
    /*
     * convlen is the length we need for the string converted to
     * char * (possibly multibyte).
     */
    int convlen;
    char *rp = rs;

    while (la && lb) {
	if (*sa != *sb) {
	    /* Different characters, try the matchers. */
	    for (t = 0, ms = bmatchers; ms && !t; ms = ms->next) {
		mp = ms->matcher;
		if (mp && !mp->flags && mp->wlen > 0 && mp->llen > 0 &&
		    mp->wlen <= la && mp->wlen <= lb) {
		    /* The pattern has no anchors and the word
		     * pattern fits, try it. */
		    if ((t = pattern_match(mp->word, sa, NULL, NULL)) ||
			pattern_match(mp->word, sb, NULL, NULL)) {
			/* It matched one of the strings, t says which one. */
			VARARR(ZLE_CHAR_T, line, mp->llen);
			char **ap, **bp;
			int *alp, *blp;

			if (t) {
			    ap = &sa;
			    alp = &la;

			    bp = &sb;
			    blp = &lb;
			} else {
			    ap = &sb;
			    alp = &lb;

			    bp = &sa;
			    blp = &la;
			}
			/* Now try to build a string that matches the other
			 * string. */
			if ((bl = bld_line(mp, line, *ap, *bp, *blp, 0))) {
			    /* Found one, put it into the return string. */
			    char *convstr =
				zlelineasstring(line, mp->llen, 0, &convlen,
						NULL, 0);
			    if (rr <= convlen) {
				char *or = rs;
				int alloclen = (convlen > 20) ? convlen : 20;

				rs = realloc(rs, (rl += alloclen));
				rr += alloclen;
				rp += rs - or;
			    }
			    memcpy(rp, convstr, convlen);
			    rp += convlen;
			    rr -= convlen;
			    /* HERE: multibyte chars */
			    *ap += mp->wlen;
			    *alp -= mp->wlen;

			    *bp += bl;
			    *blp -= bl;
			    t = 1;
			    free(convstr);
			} else
			    t = 0;
		    }
		}
	    }
	    if (!t)
		break;
	} else {
	    /* Same character, just take it. */
	    if (rr <= 1 /* HERE charlen */) {
		char *or = rs;

		rs = realloc(rs, (rl += 20));
		rr += 20;
		rp += rs - or;
	    }
	    /* HERE: multibyte char */
	    *rp++ = *sa;
	    rr--;
	    sa++;
	    sb++;
	    la--;
	    lb--;
	}
    }
    if (la || lb)
	return NULL;

    *rp = '\0';

    return rs;
}

/*
 * This compares the anchors stored in two top-level clines.
 * It returns 1 if the anchors are the same, 2 if they are
 * compatible (and have been combined in "o"), 0 otherwise.
 */

/**/
static int
cmp_anchors(Cline o, Cline n, int join)
{
    int line = 0;
    char *j;

    /* First try the exact strings. */
    if ((!(o->flags & CLF_LINE) && o->wlen == n->wlen &&
	 (!o->word || !strncmp(o->word, n->word, o->wlen))) ||
	(line = ((!o->line && !n->line && !o->wlen && !n->wlen) ||
		 (o->llen == n->llen && o->line && n->line &&
		  !strncmp(o->line, n->line, o->llen))))) {
	if (line) {
	    o->flags |= CLF_LINE;
	    o->word = NULL;
	    o->wlen = 0;
	}
	return 1;
    }
    /* Didn't work, try to build a string matching both anchors. */
    if (join && !(o->flags & CLF_JOIN) && o->word && n->word &&
	(j = join_strs(o->wlen, o->word, n->wlen, n->word))) {
	o->flags |= CLF_JOIN;
	o->wlen = strlen(j);
	o->word = dupstring(j);

	return 2;
    }
    return 0;
}

/* Below is the code to join two cline lists. This struct is used to walk
 * through a sub-list. */

typedef struct cmdata *Cmdata;

struct cmdata {
    Cline cl, pcl;
    char *str, *astr;
    int len, alen, olen, line;
};

/* This is used to ensure that a cmdata struct contains usable data.
 * The return value is non-zero if we reached the end. */

static int
check_cmdata(Cmdata md, int sfx)
{
    /* We will use the str and len fields to contain the next sub-string
     * in the list. If len is zero, we have to use the next cline. */
    if (!md->len) {
	/* If there is none, we reached the end. */
	if (!md->cl)
	    return 1;

	/* Otherwise, get the string. Only the line-string or both.
	 * We also have to adjust the pointer if this is for a suffix. */
	if (md->cl->flags & CLF_LINE) {
	    md->line = 1;
	    md->len = md->cl->llen;
	    md->str = md->cl->line;
	} else {
	    md->line = 0;
	    md->len = md->olen = md->cl->wlen;
	    /* HERE: multibyte */
	    if ((md->str = md->cl->word) && sfx)
		md->str += md->len;
	    md->alen = md->cl->llen;
	    /* HERE: multibyte */
	    if ((md->astr = md->cl->line) && sfx)
		md->astr += md->alen;
	}
	md->pcl = md->cl;
	md->cl = md->cl->next;
    }
    return 0;
}

/* This puts the not-yet-matched portion back into the last cline and 
 * returns that. */

static Cline
undo_cmdata(Cmdata md, int sfx)
{
    Cline r = md->pcl;

    if (md->line) {
	r->word = NULL;
	r->wlen = 0;
	r->flags |= CLF_LINE;
	r->llen = md->len;
	/* HERE: multibyte */
	r->line = md->str - (sfx ? md->len : 0);
    } else if (md->len != md->olen) {
	r->wlen = md->len;
	/* HERE: multibyte */
	r->word = md->str - (sfx ? md->len : 0);
	DPUTS(r->wlen > 0 && !*r->word, "Bad word");
    }
    return r;
}

/* This tries to build a string matching a sub-string in a sub-cline
 * that could not be matched otherwise. */

static Cline
join_sub(Cmdata md, char *str, int len, int *mlen, int sfx, int join)
{
    if (!check_cmdata(md, sfx)) {
	char *ow = str, *nw = md->str;
	int ol = len, nl = md->len;
	Cmlist ms;
	Cmatcher mp;
	int t;

	if (sfx) {
	    ow += ol; nw += nl;
	}
	for (t = 0, ms = bmatchers; ms && !t; ms = ms->next) {
	    mp = ms->matcher;
	    /* We use only those patterns that match a non-empty
	     * string in both the line and the word and that have
	     * no anchors. */
	    if (mp && !mp->flags && mp->wlen > 0 && mp->llen > 0) {
		/* We first test, if the old string matches already the
		 * new one. */
		if (mp->llen <= ol && mp->wlen <= nl &&
		    pattern_match(mp->line, ow - (sfx ? mp->llen : 0),
				  mp->word, nw - (sfx ? mp->wlen : 0))) {
		    /* It did, update the contents of the cmdata struct
		     * and return a cline for the matched part. */
		    if (sfx)
			md->str -= mp->wlen;
		    else
			md->str += mp->wlen;
		    md->len -= mp->wlen;
		    *mlen = mp->llen;

		    return get_cline(NULL, 0, ow - (sfx ? mp->llen : 0),
				     mp->llen, NULL, 0, 0);
		}
		/* Otherwise we will try to build a string that matches
		 * both strings. But try the pattern only if the word-
		 * pattern matches one of the strings. */
		if (join && mp->wlen <= ol && mp->wlen <= nl &&
		    ((t = pattern_match(mp->word, ow - (sfx ? mp->wlen : 0),
				       NULL, NULL)) ||
		     pattern_match(mp->word, nw - (sfx ? mp->wlen : 0),
				   NULL, NULL))) {
		    VARARR(ZLE_CHAR_T, line, mp->llen);
		    int bl;
		    char *mw;

		    /* Then build all the possible lines and see
		     * if one of them matches the other string. */
		    /* HERE: they're multibyte */
		    if (t)
			mw = ow - (sfx ? mp->wlen : 0);
		    else
			mw = nw - (sfx ? mp->wlen : 0);

		    if ((bl = bld_line(mp, line, mw, (t ? nw : ow),
				       (t ? nl : ol), sfx)))  {
			/* Yep, one of the lines matched the other
			 * string. */

			/* HERE: multibyte characters */
			if (t) {
			    ol = mp->wlen; nl = bl;
			} else {
			    ol = bl; nl = mp->wlen;
			}
			if (sfx)
			    md->str -= nl;
			else
			    md->str += nl;
			md->len -= nl;
			*mlen = ol;

			return get_cline(NULL, 0,
					 zlelineasstring(line, mp->llen,
							 0, NULL, NULL, 1),
					 mp->llen, NULL, 0, CLF_JOIN);
		    }
		}
	    }
	}
    }
    return NULL;
}

/* This is used to match a sub-string in a sub-cline. The length of the
 * matched portion is returned. This tests only for exact equality. */

static int
sub_match(Cmdata md, char *str, int len, int sfx)
{
    int ret = 0, l, ind, add;
    char *p, *q;
#ifdef MULTIBYTE_SUPPORT
    int fulllen = len;
    char *fullstr = str;
    mbstate_t mbs;
#endif

    if (sfx) {
	str += len;
	ind = -1; add = -1;
    } else {
	ind = 0; add = 1;
    }
    /* str and len describe the old string, in md we have the new one. */
    while (len) {
	if (check_cmdata(md, sfx))
	    return ret;

	/*
	 * Look for a common prefix.  If we do include metafied
	 * characters, at this stage we still need the overall length
	 * including Meta's as separate characters.
	 */
	for (l = 0, p = str, q = md->str;
	     l < len && l < md->len && p[ind] == q[ind];
	     l++, p += add, q += add) {}

	/* Make sure we don't end in the middle of a Meta sequence. */
	if (add == 1) {
	    if (l && p[-1] == Meta)
		l--;
	} else {
	    if (l && ((l < len && p[-1] == Meta)
		   || (l < md->len && q[-1] == Meta)))
		l--;
	}
#ifdef MULTIBYTE_SUPPORT
	/*
	 * Make sure we don't end in the middle of a multibyte character.
	 * Don't need to do this if the match ended at the start
	 * of the original string.
	 *
	 * Let q be the match point we've found.
	 */
	q = sfx ? str - l : str + l;
	if (q != fullstr) {
	    memset(&mbs, 0, sizeof mbs);
	    /*
	     * Otherwise read characters from the start of the original
	     * string until we reach or pass the match point.  This
	     * is rather inefficient, but in general only reading
	     * the full string can keep track of where we are in
	     * a character.  With a prefix we could be more efficient,
	     * but it's difficult with a suffix where the match point
	     * moves backwards.
	     */
	    for (p = fullstr; p < fullstr + fulllen; ) {
		wchar_t wc;
		/*
		 * ret must, in fact, be set by the current logic,
		 * but gcc doesn't realise (at least some versions don't).
		 */
		size_t cnt = MB_INVALID;
		int diff;
		char *p2;

		/*
		 * Because the string is metafied, we need to
		 * assembled wide characters a byte at a time.
		 */
		for (p2 = p; p2 < fullstr + fulllen; p2++) {
		    char curchar = (*p2 == Meta) ? (*++p2 ^ 32) : *p2;
		    cnt = mbrtowc(&wc, &curchar, 1, &mbs);
		    /* Continue while character is incomplete. */
		    if (cnt != MB_INCOMPLETE)
			break;
		}
		if (cnt == MB_INVALID || cnt == MB_INCOMPLETE) {
		    /* not a valid character, give up test */
		    break;
		}
		/* increment p2 for last byte read */
		diff = ++p2 - q;
		if (diff == 0) {
		    /*
		     * Prefix or suffix matches at end of multbyte character,
		     * so OK.
		     */
		    break;
		} else if (diff > 0) {
		    /*
		     * The prefix or suffix finishes in the middle
		     * of a character.  Shorten it until it doesn't.
		     */
		    if (sfx) {
			/*
			 * We need to remove the trailing part of
			 * the character from the suffix.
			 */
			l -= diff;
		    } else {
			/*
			 * We need to remove the initial part of
			 * the character from the prefix.
			 */
			l -= (q - p);
		    }
		    break;
		}
		/* Advance over full character */
		p = p2;
	    }
	}
#endif
	if (l) {
	    /* There was a common prefix, use it. */
	    md->len -= l; len -= l;
	    if (sfx) {
		md->str -= l; str -= l;
	    } else {
		md->str += l; str += l;
	    }
	    ret += l;
	} else if (md->line || md->len != md->olen || !md->astr)
	    return ret;
	else {
	    /* We still have the line string to try. */
	    md->line = 1;
	    md->len = md->alen;
	    md->str = md->astr;
	}
    }
    return ret;
}

/* This is used to build a common prefix or suffix sub-list. If requested
 * it returns the unmatched cline lists in orest and nrest. */

/**/
static void
join_psfx(Cline ot, Cline nt, Cline *orest, Cline *nrest, int sfx)
{
    Cline p = NULL, o, n;
    struct cmdata md, omd;
    char **sstr = NULL;
    int len, join = 0, line = 0, *slen = NULL;

    if (sfx) {
	o = ot->suffix; n = nt->suffix;
    } else {
	o = ot->prefix;	n = nt->prefix;
    }
    if (!o) {
	if (orest)
	    *orest = NULL;
	if (nrest)
	    *nrest = n;
	if (n && n->wlen)
	    ot->flags |= CLF_MISS;

	return;
    }
    if (!n) {
	if (sfx)
	    ot->suffix = NULL;
	else
	    ot->prefix = NULL;

	if (orest)
	    *orest = o;
	else
	    free_cline(o);
	if (nrest)
	    *nrest = NULL;
	return;
    }
    md.cl = n;
    md.len = 0;

    /* Walk through the old list. */
    while (o) {
	join = 0;
	memcpy(&omd, &md, sizeof(struct cmdata));

	/* We first get the length of the prefix equal in both strings. */
	if (o->flags & CLF_LINE) {
	    if ((len = sub_match(&md, o->line, o->llen, sfx)) != o->llen) {
		join = 1; line = 1; slen = &(o->llen); sstr = &(o->line);
	    }
	} else if ((len = sub_match(&md, o->word, o->wlen, sfx)) != o->wlen) {
	    if (o->line) {
		memcpy(&md, &omd, sizeof(struct cmdata));
		o->flags |= CLF_LINE | CLF_DIFF;

		continue;
	    }
	    o->llen = o->llen - ot->slen;
	    join = 1; line = 0; slen = &(o->wlen); sstr = &(o->word);
	}
	if (join) {
	    /* There is a rest that is different in the two lists,
	     * we try to build a new cline matching both strings. */
	    Cline joinl;
	    int jlen;

	    if ((joinl = join_sub(&md, *sstr + len, *slen - len,
				  &jlen, sfx, !(o->flags & CLF_JOIN)))) {
		/* We have one, insert it into the list. */
		joinl->flags |= CLF_DIFF;
		if (len + jlen != *slen) {
		    Cline rest;

		    rest = get_cline(NULL, 0, *sstr + (sfx ? 0 : len + jlen),
				     *slen - len - jlen, NULL, 0, 0);

		    rest->next = o->next;
		    joinl->next = rest;
		} else
		    joinl->next = o->next;

		if (len) {
		    if (sfx)
			*sstr += *slen - len;
		    *slen = len;
		    o->next = joinl;
		} else {
		    o->next = NULL;
		    free_cline(o);
		    if (p)
			p->next = joinl;
		    else if (sfx)
			ot->suffix = joinl;
		    else
			ot->prefix = joinl;
		}
		o = joinl;
		join = 0;
	    }
	}
	if (join) {
	    /* We couldn't build a cline for a common string, so we
	     * cut the list here. */
	    if (len) {
		Cline r;

		if (orest) {
		    if (line)
			r = get_cline(o->line + len, *slen - len,
				      NULL, 0, NULL, 0, o->flags);
		    else
			r = get_cline(NULL, 0, o->word + len, *slen - len,
				      NULL, 0, o->flags);

		    r->next = o->next;
		    *orest = r;

		    *slen = len;
		    o->next = NULL;
		} else {
		    if (sfx)
			*sstr += *slen - len;
		    *slen = len;
		    free_cline(o->next);
		    o->next = NULL;
		}
	    } else {
		if (p)
		    p->next = NULL;
		else if (sfx)
		    ot->suffix = NULL;
		else
		    ot->prefix = NULL;

		if (orest)
		    *orest = o;
		else
		    free_cline(o);
	    }
	    if (!orest || !nrest)
		ot->flags |= CLF_MISS;

	    if (nrest)
		*nrest = undo_cmdata(&md, sfx);

	    return;
	}
	p = o;
	o = o->next;
    }
    if (md.len || md.cl)
	ot->flags |= CLF_MISS;
    if (orest)
	*orest = NULL;
    if (nrest)
	*nrest = undo_cmdata(&md, sfx);
}

/* This builds the common prefix and suffix for a mid-cline -- the one
 * describing the place where the prefix and the suffix meet. */

/**/
static void
join_mid(Cline o, Cline n)
{
    if (o->flags & CLF_JOIN) {
	/* The JOIN flag is set in the old cline struct if it was
	 * already joined with another one. In this case the suffix
	 * field contains the suffix from previous calls. */
	Cline nr;

	join_psfx(o, n, NULL, &nr, 0);

	n->suffix = revert_cline(nr);

	join_psfx(o, n, NULL, NULL, 1);
    } else {
	/* This is the first time for both structs, so the prefix field
	 * contains the whole sub-list. */
	Cline or, nr;

	o->flags |= CLF_JOIN;

	/* We let us give both rests and use them as the suffixes. */
	join_psfx(o, n, &or, &nr, 0);

	if (or)
	    or->llen = (o->slen > or->wlen ? or->wlen : o->slen);
	o->suffix = revert_cline(or);
	n->suffix = revert_cline(nr);

	join_psfx(o, n, NULL, NULL, 1);
    }
    n->suffix = NULL;
}

/* This turns the sequence of anchor cline structs from b to e into a
 * prefix sequence, puts it before the prefix of e and then tries to
 * join that with the prefix of a.
 * This is needed if some matches had a anchor match spec and others
 * didn't. */

/**/
static int
sub_join(Cline a, Cline b, Cline e, int anew)
{
    if (!e->suffix && a->prefix) {
	Cline op = e->prefix, n = NULL, *p = &n, t, ca;
	int min = 0, max = 0;

	for (; b != e; b = b->next) {
	    if ((*p = t = b->prefix)) {
		while (t->next)
		    t = t->next;
		p = &(t->next);
	    }
	    b->suffix = b->prefix = NULL;
	    b->flags &= ~CLF_SUF;
	    min += b->min;
	    max += b->max;
	    *p = b;
	    p = &(b->next);
	}
	*p = e->prefix;
	ca = a->prefix;

	while (n) {
	    e->prefix = cp_cline(n, 1);
	    a->prefix = cp_cline(ca, 1);

	    if (anew) {
		int f = e->flags;

		join_psfx(e, a, NULL, NULL, 0);
		e->flags = f;
		if (e->prefix)
		    return max - min;
	    } else {
		int f = e->flags;

		join_psfx(a, e, NULL, NULL, 0);
		e->flags = f;
		if (a->prefix)
		    return max - min;
	    }
	    min -= n->min;

	    if (n == op)
		break;
	    n = n->next;
	}
	return max - min;
    }
    return 0;
}

/* This simplifies the cline list given as the first argument so that
 * it also matches the second list. */

/**/
Cline
join_clines(Cline o, Cline n)
{
    cline_setlens(n, 1);

    /* First time called, just return the new list. On further invocations
     * we will get it as the first argument. */
    if (!o)
	return n;
    else {
	Cline oo = o, nn = n, po = NULL, pn = NULL, x;
	int diff;

	/* Walk through the lists. */
	while (o && n) {
	    /* If one of them describes a new part and the other one does
	     * not, synchronise them by searching an old part in the
	     * other list. */
	    if ((o->flags & CLF_NEW) && !(n->flags & CLF_NEW)) {
		Cline t, tn;

		for (t = o; (tn = t->next) &&
			 ((tn->flags & CLF_NEW) || !cmp_anchors(tn, n, 0));
		     t = tn);
		if (tn) {
		    diff = sub_join(n, o, tn, 1);

		    if (po)
			po->next = tn;
		    else
			oo = tn;

		    t->next = NULL;
		    free_cline(o);
		    x = o;
		    o = tn;

		    if (po && po->prefix && cmp_anchors(x, po, 0)) {
			po->flags |= CLF_MISS;
			po->max += diff;
		    } else {
			o->flags |= CLF_MISS;
			o->max += diff;
		    }
		    continue;
		}
	    }
	    if (!(o->flags & CLF_NEW) && (n->flags & CLF_NEW)) {
		Cline t, tn;

		for (t = n; (tn = t->next) &&
			 ((tn->flags & CLF_NEW) || !cmp_anchors(o, tn, 0));
		     t = tn);
		if (tn) {
		    int of = o->flags & CLF_MISS;

		    diff = sub_join(o, n, tn, 0);
		    o->flags = (o->flags & ~CLF_MISS) | of;

		    if (po && po->prefix && cmp_anchors(n, pn, 0)) {
			po->flags |= CLF_MISS;
			po->max += diff;
		    } else {
			o->flags |= CLF_MISS;
			o->max += diff;
		    }
		    n = tn;
		    continue;
		}
	    }
	    /* Almost the same as above, but for the case that they
	     * describe different types of parts (prefix, suffix, or mid). */
	    if ((o->flags & (CLF_SUF | CLF_MID)) !=
		(n->flags & (CLF_SUF | CLF_MID))) {
		Cline t, tn;

		for (t = n;
		     (tn = t->next) &&
			 (tn->flags & (CLF_SUF | CLF_MID)) !=
			 (o->flags  & (CLF_SUF | CLF_MID));
		     t = tn);
		if (tn && cmp_anchors(o, tn, 1)) {
		    sub_join(o, n, tn, 0);

		    n = tn;
		    continue;
		}
		for (t = o;
		     (tn = t->next) &&
			 (tn->flags & (CLF_SUF | CLF_MID)) !=
			 (n->flags  & (CLF_SUF | CLF_MID));
		     t = tn);
		if (tn && cmp_anchors(tn, n, 1)) {
		    sub_join(n, o, tn, 1);

		    if (po)
			po->next = tn;
		    else
			oo = tn;
		    t->next = NULL;
		    free_cline(o);
		    o = tn;
		    continue;
		}
		if (o->flags & CLF_MID) {
		    o->flags = (o->flags & ~CLF_MID) | (n->flags & CLF_SUF);
		    if (n->flags & CLF_SUF) {
			free_cline(o->prefix);
			o->prefix = NULL;
		    } else {
			free_cline(o->suffix);
			o->suffix = NULL;
		    }
		}
		break;
	    }
	    /* Now see if they have matching anchors. If not, cut the list. */
	    if (!(o->flags & CLF_MID) && !cmp_anchors(o, n, 1)) {
		Cline t, tn, tt, to = NULL;

		for (t = n; (tn = t->next); t = tn)
		    if (!(tn->flags & CLF_NEW) && (tn->flags & CLF_SKIP)) {
			for (tt = o; (to = tt->next); tt = to)
			    if (!(to->flags & CLF_NEW) && (to->flags & CLF_SKIP) &&
				cmp_anchors(tn, to, 1))
				break;
			if (to)
			    break;
		    }
		if (tn) {
		    if (po)
			po->next = to;
		    else
			oo = to;
		    o = to;

		    diff = sub_join(o, n, tn, 0);

		    o->flags |= CLF_MISS;
		    o->max += diff;

		    n = tn;
		    po = o;
		    o = o->next;
		    pn = n;
		    n = n->next;
		    continue;
		} else {
		    for (t = o; (to = t->next); t = to)
			if ((to->flags & CLF_SKIP) && cmp_anchors(n, to, 1))
			    break;

		    if (to) {
			diff = sub_join(n, o, to, 1);

			if (po)
			    po->next = to;
			else
			    oo = to;
			x = o;
			o = to;
			if (po && po->prefix && cmp_anchors(x, po, 0)) {
			    po->flags |= CLF_MISS;
			    po->max += diff;
			} else {
			    o->flags |= CLF_MISS;
			    o->max += diff;
			}
			continue;
		    } else {
			for (tt = NULL, t = n; (tn = t->next); t = tn) {
			    if (tn->flags & CLF_SKIP)
				for (tt = o; (to = tt->next); tt = to)
				    if ((to->flags & CLF_SKIP) &&
					cmp_anchors(tn, to, 1))
					break;
			    if (to)
				break;
			}
			if (to) {
			    diff = sub_join(n, o, to, 1);

			    if (po)
				po->next = to;
			    else
				oo = to;
			    x = o;
			    o = to;
			    if (po && po->prefix && cmp_anchors(x, po, 0)) {
				po->flags |= CLF_MISS;
				po->max += diff;
			    } else {
				o->flags |= CLF_MISS;
				o->max += diff;
			    }
			    continue;
			} else {
			    for (tn = n; tn; tn = tn->next)
				if ((tn->flags & CLF_NEW) ==
				    (o->flags & CLF_NEW) &&
				    cmp_anchors(tn, o, 1)) break;

			    if (tn) {
				int of = o->flags & CLF_MISS;

				if ((diff = sub_join(o, n, tn, 0))) {
				    o->flags = (o->flags & ~CLF_MISS) | of;
				    if (po && po->prefix) {
					po->flags |= CLF_MISS;
					po->max += diff;
				    }
				    else {
					o->flags |= CLF_MISS;
					o->max += diff;
				    }
				}
				n = tn;
				po = o;
				o = o->next;
				pn = n;
				n = n->next;
				continue;
			    }
			    if (o->flags & CLF_SUF)
				break;

			    o->word = o->line = o->orig = NULL;
			    o->wlen = 0;
			    free_cline(o->next);
			    o->next = NULL;
			    o->flags |= CLF_MISS;
			}
		    }
		}
	    }
	    /* Ok, they are equal, now copy the information about the
             * original string if needed, calculate minimum and maximum
	     * lengths, and join the sub-lists. */
	    if (!o->orig && !o->olen) {
		o->orig = n->orig;
		o->olen = n->olen;
	    }
	    if (n->min < o->min)
		o->min = n->min;
	    if (n->max > o->max)
		o->max = n->max;
	    if (o->flags & CLF_MID)
		join_mid(o, n);
	    else
		join_psfx(o, n, NULL, NULL, (o->flags & CLF_SUF));

	    po = o;
	    o = o->next;
	    pn = n;
	    n = n->next;
	}
	/* Free the rest of the old list. */
	if (o) {
	    if (po)
		po->next = NULL;
	    else
		oo = NULL;

	    free_cline(o);
	}
	free_cline(nn);

	return oo;
    }
}
