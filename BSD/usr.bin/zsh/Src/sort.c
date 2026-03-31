/*
 * sort.c - comparison and sorting of strings
 *
 * This file is part of zsh, the Z shell.
 *
 * Copyright (c) 1992-2007 Paul Falstad
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
#include "sort.pro"

/* Flag for direction of sort: 1 forwards, -1 reverse */
static int sortdir;

/* Flag that sort ignores backslashes */
static int sortnobslash;

/* Flag that sort is numeric */
static int sortnumeric;

/**/
static int
eltpcmp(const void *a, const void *b)
{
    const SortElt ae = *(const SortElt *)a;
    const SortElt be = *(const SortElt *)b;
    const char *as = ae->cmp, *bs = be->cmp;
    const char *ao = as;
    int cmp;

    if (ae->len != -1 || be->len != -1) {
	/*
	 * Length recorded.  We only do that if there are embedded
	 * nulls we need to treat as regular characters.
	 *
	 * Since we don't know where multibyte characters start,
	 * but do know that a null character can't occur inside
	 * one (we are relying on multibyte characters being extensions
	 * of ASCII), we can compare starting from after the last
	 * null character that occurred in both strings.
	 */
	const char *cmpa, *cmpb;
	const char *laststarta = as;
	int len;
	if (ae->len != -1) {
	    len = ae->len;
	    if (be->len != -1 && len > be->len)
		len = be->len;
	}
	else
	    len = be->len;

	for (cmpa = as, cmpb = bs; *cmpa == *cmpb && len--; cmpa++, cmpb++) {
	    if (!*cmpa) {
		/*
		 * If either string doesn't have a length, we've reached
		 * the end.  This is covered in the test below.
		 */
		if (ae->len == -1 || be->len == -1)
		    break;
		laststarta = cmpa + 1;
	    }
	}
	if (*cmpa == *cmpb && ae->len != be->len) {
	    /*
	     * Special case: one of the strings has finished, but
	     * another one continues after the NULL.  The string
	     * that's finished sorts below the other.  We need
	     * to handle this here since strcoll() or strcmp()
	     * will just compare the strings as equal.
	     */
	    if (ae->len != -1) {
		if (be->len != -1) {
		    /*
		     * if a is shorter it's earlier, so return -1 and
		     * vice versa 
		     */
		    return (ae->len - be->len) * sortdir;
		} else {
		    /*
		     * a has a length and so continues, hence
		     * b sorts lower.
		     */
		    return sortdir;
		}
	    } else {
		/*
		 * b must continue because it has a recorded length,
		 * so a sorts lower.
		 */
		return - sortdir;
	    }
	}

	bs += (laststarta - as);
	as += (laststarta - as);
    }

   if (sortnobslash) {
	while (*as && *bs) {
	    if (*as == '\\')
		as++;
	    if (*bs == '\\')
		bs++;
	    if (*as != *bs || !*as)
		break;
	    as++;
	    bs++;
	}
    }

#ifdef HAVE_STRCOLL
    cmp = strcoll(as, bs);
#endif

    if (sortnumeric) {
	int mul = 0;
	for (; *as == *bs && *as; as++, bs++);
#ifndef HAVE_STRCOLL
	cmp = (int)STOUC(*as) - (int)STOUC(*bs);
#endif
	if (sortnumeric < 0) {
	    if (*as == '-' && idigit(as[1]) && idigit(*bs)) {
		cmp = -1;
		mul = 1;
	    } else if (*bs == '-' && idigit(bs[1]) && idigit(*as)) {
		cmp = 1;
		mul = 1;
	    }
	}
	if (!mul && (idigit(*as) || idigit(*bs))) {
	    for (; as > ao && idigit(as[-1]); as--, bs--);
	    mul = (sortnumeric < 0 && as > ao && as[-1] == '-') ? -1 : 1;
	    if (idigit(*as) && idigit(*bs)) {
		while (*as == '0')
		    as++;
		while (*bs == '0')
		    bs++;
		for (; idigit(*as) && *as == *bs; as++, bs++);
		if (idigit(*as) || idigit(*bs)) {
		    cmp = mul * ((int)STOUC(*as) - (int)STOUC(*bs));
		    while (idigit(*as) && idigit(*bs))
			as++, bs++;
		    if (idigit(*as) && !idigit(*bs))
			return mul * sortdir;
		    if (idigit(*bs) && !idigit(*as))
			return -mul * sortdir;
		}
	    }
	}
    }
#ifndef HAVE_STRCOLL
    else
	cmp = strcmp(as, bs);
#endif

    return sortdir * cmp;
}


/*
 * Front-end to eltpcmp() to compare strings.
 * TODO: it would be better to eliminate this altogether by
 * making the calling function call into the sort code
 * at a higher level.
 */

/**/
mod_export int
zstrcmp(const char *as, const char *bs, int sortflags)
{
    struct sortelt ae, be, *aeptr, *beptr;
    int oldsortdir = sortdir;
    int oldsortnobslash = sortnobslash;
    int oldsortnumeric = sortnumeric;
    int ret;

    ae.cmp = as;
    be.cmp = bs;
    ae.len = -1;
    be.len = -1;

    aeptr = &ae;
    beptr = &be;

    sortdir = 1;
    sortnobslash = (sortflags & SORTIT_IGNORING_BACKSLASHES) ? 1 : 0;
    sortnumeric = (sortflags & SORTIT_NUMERICALLY_SIGNED) ? -1 :
	(sortflags & SORTIT_NUMERICALLY) ? 1 : 0;

    ret = eltpcmp(&aeptr, &beptr);

    /* Paranoia: I don't think we ever need to restore these. */
    sortnobslash = oldsortnobslash;
    sortnumeric = oldsortnumeric;
    sortdir = oldsortdir;

    return ret;
}


/*
 * Sort an array of metafied strings.  Use an "or" of bit flags
 * to decide how to sort.  See the SORTIT_* flags in zsh.h.
 *
 * If unmetalenp is not NULL, the strings in array are already
 * unmetafied and unmetalenp is an array containing the corresponding
 * lengths.
 */

/**/
mod_export void
strmetasort(char **array, int sortwhat, int *unmetalenp)
{
    char **arrptr;
    /*
     * Array of elements containing stuff to sort.  Note sortptrarr
     * is an array of pointers, since that's more efficient
     * for qsort() to manipulate.  sortarr is the array of
     * structures.
     */
    SortElt *sortptrarr, *sortptrarrptr;
    SortElt sortarr, sortarrptr;
    int oldsortdir, oldsortnumeric, nsort;

    nsort = arrlen(array);
    if (nsort < 2)
	return;

    pushheap();

    sortptrarr = (SortElt *) zhalloc(nsort * sizeof(SortElt));
    sortarr = (SortElt) zhalloc(nsort * sizeof(struct sortelt));
    for (arrptr = array, sortptrarrptr = sortptrarr, sortarrptr = sortarr;
	 *arrptr; arrptr++, sortptrarrptr++, sortarrptr++) {
	char *metaptr;
	int needlen, needalloc;
	*sortptrarrptr = sortarrptr;
	sortarrptr->orig = *arrptr;

	if (unmetalenp) {
	    /*
	     * Already unmetafied.  We just need to check for
	     * embedded nulls.
	     */
	    int count = unmetalenp[arrptr-array];
	    /* Remember this length for sorted array */
	    sortarrptr->origlen = count;
	    for (metaptr = *arrptr; *metaptr != '\0' && count--; metaptr++)
		;
	    /* *metaptr must now be \0, even if we reached the end */
	    needlen = (count != 0);
	} else {
	    /*
	     * Not yet unmetafied.  See if it needs unmetafying.
	     * If it doesn't, there can't be any embedded nulls,
	     * since these are metafied.
	     */
	    needlen = 0;
	    for (metaptr = *arrptr; *metaptr && *metaptr != Meta;
		 metaptr++);
	}
	/*
	 * See if we need to do some special checking.
	 * Either we're going to need to copy it to transform it,
	 * or we need to unmetafy it.
	 */
	if ((needalloc = (sortwhat &
			  (SORTIT_IGNORING_CASE|SORTIT_IGNORING_BACKSLASHES)))
	    || *metaptr == Meta) {
	    char *s, *t, *src = *arrptr, *dst;
	    int len;
	    sortarrptr->cmp = dst =
		(char *)zhalloc(((sortwhat & SORTIT_IGNORING_CASE)?2:1)*strlen(src)+1);

	    if (unmetalenp) {
		/* Already unmetafied and we have the length. */
		len = unmetalenp[arrptr-array];
	    } else if (*metaptr != '\0') {
		/*
		 * Needs unmetafying.  We need to check for
		 * embedded nulls while we do this.
		 */
		char *t = dst + (metaptr - src);

		if (metaptr != src)
		    memcpy(dst, src, metaptr - src);
		while ((*t = *metaptr++)) {
		    if (*t++ == Meta) {
			if ((t[-1] = *metaptr++ ^ 32) == '\0')
			    needlen = 1;
		    }
		}
		len = t - dst;
		src = dst;
	    } else {
		/*
		 * Doesn't need unmetafying.
		 * This means metaptr is the NULL at the
		 * end of the string, so we have the length, and
		 * there are no embedded nulls, so we don't
		 * need the length later.
		 * We're only copying the string to transform it
		 * below.
		 */
		len = metaptr - src;
	    }
	    if (sortwhat & SORTIT_IGNORING_CASE) {
		char *send = src + len;
#ifdef MULTIBYTE_SUPPORT
		if (isset(MULTIBYTE)) {
		    /*
		     * Lower the case the hard way.  Convert to a wide
		     * character, process that, and convert back.  We
		     * don't assume the characters have the same
		     * multibyte length.  We can't use casemodify()
		     * because we have unmetafied data, which may have
		     * been passed down to use.
		     */
		    mbstate_t mbsin, mbsout;
		    int clen;
		    wchar_t wc;
		    memset(&mbsin, 0, sizeof(mbstate_t));
		    memset(&mbsout, 0, sizeof(mbstate_t));

		    for (s = src, t = dst; s < send; ) {
			clen = mbrtowc(&wc, s, send-s, &mbsin);
			if (clen < 0) {
			    /* invalid or unfinished: treat as single bytes */
			    while (s < send)
				*t++ = tulower(*s++);
			    break;
			}
			if (clen == 0) {
			    /* embedded null */
			    *t++ = '\0';
			    s++;
			    continue;
			}
			s += clen;
			wc = towlower(wc);
			clen = wcrtomb(t, wc, &mbsout);
			t += clen;
			DPUTS(clen < 0, "Bad conversion when lowering case");
		    }
		    *t = '\0';
		    len = t - dst;
		} else
#endif
		    for (s = src, t = dst; s < send; )
			*t++ = tulower(*s++);
		src = dst;
	    }
	    if (sortwhat & SORTIT_IGNORING_BACKSLASHES) {
                char *end = src + len + 1;
		/* copy null byte, so increment length */
		for (s = src, t = dst; s < end; ) {
		    if (*s == '\\') {
			s++;
			len--;
		    }
		    *t++ = *s++;
		}
	    }
	    /* Do we need to store the length (embedded null)? */
	    sortarrptr->len = needlen ? len : -1;
	} else {
	    /*
	     * We can use the string as is, although it's possible
	     * we still need to take account of an embedded null.
	     */
	    sortarrptr->cmp = *arrptr;
	    sortarrptr->len = needlen ? unmetalenp[arrptr-array] : -1;
	}
    }
    /*
     * We probably don't need to restore the following, but it's pretty cheap.
     */
    oldsortdir = sortdir;
    oldsortnumeric = sortnumeric;

    sortdir = (sortwhat & SORTIT_BACKWARDS) ? -1 : 1;
    sortnumeric = (sortwhat & SORTIT_NUMERICALLY_SIGNED) ? -1 :
	(sortwhat & SORTIT_NUMERICALLY) ? 1 : 0;

    qsort(sortptrarr, nsort, sizeof(SortElt), eltpcmp);

    sortnumeric = oldsortnumeric;
    sortdir = oldsortdir;
    for (arrptr = array, sortptrarrptr = sortptrarr; nsort--; ) {
	if (unmetalenp)
	    unmetalenp[arrptr-array] = (*sortptrarrptr)->origlen;
	*arrptr++ = (*sortptrarrptr++)->orig;
    }

    popheap();
}
