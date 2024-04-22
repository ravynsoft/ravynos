/*
 * fontconfig/src/fccharset.c
 *
 * Copyright © 2001 Keith Packard
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of the author(s) not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  The authors make no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * THE AUTHOR(S) DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE AUTHOR(S) BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include "fcint.h"
#include <stdlib.h>

/* #define CHECK */

FcCharSet *
FcCharSetCreate (void)
{
    FcCharSet	*fcs;

    fcs = (FcCharSet *) malloc (sizeof (FcCharSet));
    if (!fcs)
	return 0;
    FcRefInit (&fcs->ref, 1);
    fcs->num = 0;
    fcs->leaves_offset = 0;
    fcs->numbers_offset = 0;
    return fcs;
}

FcCharSet *
FcCharSetPromote (FcValuePromotionBuffer *vbuf)
{
    FcCharSet *fcs = (FcCharSet *) vbuf;

    FC_ASSERT_STATIC (sizeof (FcCharSet) <= sizeof (FcValuePromotionBuffer));

    FcRefSetConst (&fcs->ref);
    fcs->num = 0;
    fcs->leaves_offset = 0;
    fcs->numbers_offset = 0;

    return fcs;
}

FcCharSet *
FcCharSetNew (void)
{
    return FcCharSetCreate ();
}

void
FcCharSetDestroy (FcCharSet *fcs)
{
    int i;

    if (fcs)
    {
	if (FcRefIsConst (&fcs->ref))
	{
	    FcCacheObjectDereference (fcs);
	    return;
	}
	if (FcRefDec (&fcs->ref) != 1)
	    return;
	for (i = 0; i < fcs->num; i++)
	    free (FcCharSetLeaf (fcs, i));
	if (fcs->num)
	{
	    free (FcCharSetLeaves (fcs));
	    free (FcCharSetNumbers (fcs));
	}
	free (fcs);
    }
}

/*
 * Search for the leaf containing with the specified num.
 * Return its index if it exists, otherwise return negative of
 * the (position + 1) where it should be inserted
 */


static int
FcCharSetFindLeafForward (const FcCharSet *fcs, int start, FcChar16 num)
{
    FcChar16		*numbers = FcCharSetNumbers(fcs);
    FcChar16		page;
    int			low = start;
    int			high = fcs->num - 1;

    if (!numbers)
	return -1;
    while (low <= high)
    {
	int mid = (low + high) >> 1;
	page = numbers[mid];
	if (page == num)
	    return mid;
	if (page < num)
	    low = mid + 1;
	else
	    high = mid - 1;
    }
    if (high < 0 || (high < fcs->num && numbers[high] < num))
	high++;
    return -(high + 1);
}

/*
 * Locate the leaf containing the specified char, return
 * its index if it exists, otherwise return negative of
 * the (position + 1) where it should be inserted
 */

static int
FcCharSetFindLeafPos (const FcCharSet *fcs, FcChar32 ucs4)
{
    return FcCharSetFindLeafForward (fcs, 0, ucs4 >> 8);
}

static FcCharLeaf *
FcCharSetFindLeaf (const FcCharSet *fcs, FcChar32 ucs4)
{
    int	pos = FcCharSetFindLeafPos (fcs, ucs4);
    if (pos >= 0)
	return FcCharSetLeaf(fcs, pos);
    return 0;
}

#define FC_IS_ZERO_OR_POWER_OF_TWO(x) (!((x) & ((x)-1)))

static FcBool
FcCharSetPutLeaf (FcCharSet	*fcs,
		  FcChar32	ucs4,
		  FcCharLeaf	*leaf,
		  int		pos)
{
    intptr_t	*leaves = FcCharSetLeaves (fcs);
    FcChar16	*numbers = FcCharSetNumbers (fcs);

    ucs4 >>= 8;
    if (ucs4 >= 0x10000)
	return FcFalse;

    if (FC_IS_ZERO_OR_POWER_OF_TWO (fcs->num))
    {
      if (!fcs->num)
      {
        unsigned int alloced = 8;
	leaves = malloc (alloced * sizeof (*leaves));
	numbers = malloc (alloced * sizeof (*numbers));
	if (!leaves || !numbers)
	{
	    if (leaves)
		free (leaves);
	    if (numbers)
		free (numbers);
	    return FcFalse;
	}
      }
      else
      {
	int i;
        unsigned int alloced = fcs->num;
	intptr_t *new_leaves;
	ptrdiff_t distance;

	alloced *= 2;
	numbers = realloc (numbers, alloced * sizeof (*numbers));
	if (!numbers)
	    return FcFalse;
	new_leaves = realloc (leaves, alloced * sizeof (*leaves));
	if (!new_leaves)
	{
	    /*
	     * Revert the reallocation of numbers. We update numbers_offset
	     * first in case realloc() fails.
	     */
	    fcs->numbers_offset = FcPtrToOffset (fcs, numbers);
	    numbers = realloc (numbers, (alloced / 2) * sizeof (*numbers));
	    /* unlikely to fail though */
	    if (!numbers)
		return FcFalse;
	    fcs->numbers_offset = FcPtrToOffset (fcs, numbers);
	    return FcFalse;
	}
	distance = (char *) new_leaves - (char *) leaves;
	for (i = 0; i < fcs->num; i++) {
	    new_leaves[i] -= distance;
	}
	leaves = new_leaves;
      }

      fcs->leaves_offset = FcPtrToOffset (fcs, leaves);
      fcs->numbers_offset = FcPtrToOffset (fcs, numbers);
    }

    memmove (leaves + pos + 1, leaves + pos,
	     (fcs->num - pos) * sizeof (*leaves));
    memmove (numbers + pos + 1, numbers + pos,
	     (fcs->num - pos) * sizeof (*numbers));
    numbers[pos] = (FcChar16) ucs4;
    leaves[pos] = FcPtrToOffset (leaves, leaf);
    fcs->num++;
    return FcTrue;
}

/*
 * Locate the leaf containing the specified char, creating it
 * if desired
 */

FcCharLeaf *
FcCharSetFindLeafCreate (FcCharSet *fcs, FcChar32 ucs4)
{
    int			pos;
    FcCharLeaf		*leaf;

    pos = FcCharSetFindLeafPos (fcs, ucs4);
    if (pos >= 0)
	return FcCharSetLeaf(fcs, pos);

    leaf = calloc (1, sizeof (FcCharLeaf));
    if (!leaf)
	return 0;

    pos = -pos - 1;
    if (!FcCharSetPutLeaf (fcs, ucs4, leaf, pos))
    {
	free (leaf);
	return 0;
    }
    return leaf;
}

static FcBool
FcCharSetInsertLeaf (FcCharSet *fcs, FcChar32 ucs4, FcCharLeaf *leaf)
{
    int		    pos;

    pos = FcCharSetFindLeafPos (fcs, ucs4);
    if (pos >= 0)
    {
	free (FcCharSetLeaf (fcs, pos));
	FcCharSetLeaves(fcs)[pos] = FcPtrToOffset (FcCharSetLeaves(fcs),
						   leaf);
	return FcTrue;
    }
    pos = -pos - 1;
    return FcCharSetPutLeaf (fcs, ucs4, leaf, pos);
}

FcBool
FcCharSetAddChar (FcCharSet *fcs, FcChar32 ucs4)
{
    FcCharLeaf	*leaf;
    FcChar32	*b;

    if (fcs == NULL || FcRefIsConst (&fcs->ref))
	return FcFalse;
    leaf = FcCharSetFindLeafCreate (fcs, ucs4);
    if (!leaf)
	return FcFalse;
    b = &leaf->map[(ucs4 & 0xff) >> 5];
    *b |= (1U << (ucs4 & 0x1f));
    return FcTrue;
}

FcBool
FcCharSetDelChar (FcCharSet *fcs, FcChar32 ucs4)
{
    FcCharLeaf	*leaf;
    FcChar32	*b;

    if (fcs == NULL || FcRefIsConst (&fcs->ref))
	return FcFalse;
    leaf = FcCharSetFindLeaf (fcs, ucs4);
    if (!leaf)
	return FcTrue;
    b = &leaf->map[(ucs4 & 0xff) >> 5];
    *b &= ~(1U << (ucs4 & 0x1f));
    /* We don't bother removing the leaf if it's empty */
    return FcTrue;
}

/*
 * An iterator for the leaves of a charset
 */

typedef struct _fcCharSetIter {
    FcCharLeaf	    *leaf;
    FcChar32	    ucs4;
    int		    pos;
} FcCharSetIter;

/*
 * Set iter->leaf to the leaf containing iter->ucs4 or higher
 */

static void
FcCharSetIterSet (const FcCharSet *fcs, FcCharSetIter *iter)
{
    int		    pos = FcCharSetFindLeafPos (fcs, iter->ucs4);

    if (pos < 0)
    {
	pos = -pos - 1;
	if (pos == fcs->num)
	{
	    iter->ucs4 = ~0;
	    iter->leaf = 0;
	    return;
	}
        iter->ucs4 = (FcChar32) FcCharSetNumbers(fcs)[pos] << 8;
    }
    iter->leaf = FcCharSetLeaf(fcs, pos);
    iter->pos = pos;
}

static void
FcCharSetIterNext (const FcCharSet *fcs, FcCharSetIter *iter)
{
    int	pos = iter->pos + 1;
    if (pos >= fcs->num)
    {
	iter->ucs4 = ~0;
	iter->leaf = 0;
    }
    else
    {
	iter->ucs4 = (FcChar32) FcCharSetNumbers(fcs)[pos] << 8;
	iter->leaf = FcCharSetLeaf(fcs, pos);
	iter->pos = pos;
    }
}


static void
FcCharSetIterStart (const FcCharSet *fcs, FcCharSetIter *iter)
{
    iter->ucs4 = 0;
    iter->pos = 0;
    FcCharSetIterSet (fcs, iter);
}

FcCharSet *
FcCharSetCopy (FcCharSet *src)
{
    if (src)
    {
	if (!FcRefIsConst (&src->ref))
	    FcRefInc (&src->ref);
	else
	    FcCacheObjectReference (src);
    }
    return src;
}

FcBool
FcCharSetEqual (const FcCharSet *a, const FcCharSet *b)
{
    FcCharSetIter   ai, bi;
    int		    i;

    if (a == b)
	return FcTrue;
    if (!a || !b)
	return FcFalse;
    for (FcCharSetIterStart (a, &ai), FcCharSetIterStart (b, &bi);
	 ai.leaf && bi.leaf;
	 FcCharSetIterNext (a, &ai), FcCharSetIterNext (b, &bi))
    {
	if (ai.ucs4 != bi.ucs4)
	    return FcFalse;
	for (i = 0; i < 256/32; i++)
	    if (ai.leaf->map[i] != bi.leaf->map[i])
		return FcFalse;
    }
    return ai.leaf == bi.leaf;
}

static FcBool
FcCharSetAddLeaf (FcCharSet	*fcs,
		  FcChar32	ucs4,
		  FcCharLeaf	*leaf)
{
    FcCharLeaf   *new = FcCharSetFindLeafCreate (fcs, ucs4);
    if (!new)
	return FcFalse;
    *new = *leaf;
    return FcTrue;
}

static FcCharSet *
FcCharSetOperate (const FcCharSet   *a,
		  const FcCharSet   *b,
		  FcBool	    (*overlap) (FcCharLeaf	    *result,
						const FcCharLeaf    *al,
						const FcCharLeaf    *bl),
		  FcBool	aonly,
		  FcBool	bonly)
{
    FcCharSet	    *fcs;
    FcCharSetIter   ai, bi;

    if (!a || !b)
	goto bail0;
    fcs = FcCharSetCreate ();
    if (!fcs)
	goto bail0;
    FcCharSetIterStart (a, &ai);
    FcCharSetIterStart (b, &bi);
    while ((ai.leaf || (bonly && bi.leaf)) && (bi.leaf || (aonly && ai.leaf)))
    {
	if (ai.ucs4 < bi.ucs4)
	{
	    if (aonly)
	    {
		if (!FcCharSetAddLeaf (fcs, ai.ucs4, ai.leaf))
		    goto bail1;
		FcCharSetIterNext (a, &ai);
	    }
	    else
	    {
		ai.ucs4 = bi.ucs4;
		FcCharSetIterSet (a, &ai);
	    }
	}
	else if (bi.ucs4 < ai.ucs4 )
	{
	    if (bonly)
	    {
		if (!FcCharSetAddLeaf (fcs, bi.ucs4, bi.leaf))
		    goto bail1;
		FcCharSetIterNext (b, &bi);
	    }
	    else
	    {
		bi.ucs4 = ai.ucs4;
		FcCharSetIterSet (b, &bi);
	    }
	}
	else
	{
	    FcCharLeaf  leaf;

	    if ((*overlap) (&leaf, ai.leaf, bi.leaf))
	    {
		if (!FcCharSetAddLeaf (fcs, ai.ucs4, &leaf))
		    goto bail1;
	    }
	    FcCharSetIterNext (a, &ai);
	    FcCharSetIterNext (b, &bi);
	}
    }
    return fcs;
bail1:
    FcCharSetDestroy (fcs);
bail0:
    return 0;
}

static FcBool
FcCharSetIntersectLeaf (FcCharLeaf *result,
			const FcCharLeaf *al,
			const FcCharLeaf *bl)
{
    int	    i;
    FcBool  nonempty = FcFalse;

    for (i = 0; i < 256/32; i++)
	if ((result->map[i] = al->map[i] & bl->map[i]))
	    nonempty = FcTrue;
    return nonempty;
}

FcCharSet *
FcCharSetIntersect (const FcCharSet *a, const FcCharSet *b)
{
    return FcCharSetOperate (a, b, FcCharSetIntersectLeaf, FcFalse, FcFalse);
}

static FcBool
FcCharSetUnionLeaf (FcCharLeaf *result,
		    const FcCharLeaf *al,
		    const FcCharLeaf *bl)
{
    int	i;

    for (i = 0; i < 256/32; i++)
	result->map[i] = al->map[i] | bl->map[i];
    return FcTrue;
}

FcCharSet *
FcCharSetUnion (const FcCharSet *a, const FcCharSet *b)
{
    return FcCharSetOperate (a, b, FcCharSetUnionLeaf, FcTrue, FcTrue);
}

FcBool
FcCharSetMerge (FcCharSet *a, const FcCharSet *b, FcBool *changed)
{
    int		ai = 0, bi = 0;
    FcChar16	an, bn;

    if (!a || !b)
	return FcFalse;

    if (FcRefIsConst (&a->ref)) {
	if (changed)
	    *changed = FcFalse;
	return FcFalse;
    }

    if (changed) {
	*changed = !FcCharSetIsSubset(b, a);
	if (!*changed)
	    return FcTrue;
    }

    while (bi < b->num)
    {
	an = ai < a->num ? FcCharSetNumbers(a)[ai] : ~0;
	bn = FcCharSetNumbers(b)[bi];

	if (an < bn)
	{
	    ai = FcCharSetFindLeafForward (a, ai + 1, bn);
	    if (ai < 0)
		ai = -ai - 1;
	}
	else
	{
	    FcCharLeaf *bl = FcCharSetLeaf(b, bi);
	    if (bn < an)
	    {
		if (!FcCharSetAddLeaf (a, bn << 8, bl))
		    return FcFalse;
	    }
	    else
	    {
		FcCharLeaf *al = FcCharSetLeaf(a, ai);
		FcCharSetUnionLeaf (al, al, bl);
	    }

	    ai++;
	    bi++;
	}
    }

    return FcTrue;
}

static FcBool
FcCharSetSubtractLeaf (FcCharLeaf *result,
		       const FcCharLeaf *al,
		       const FcCharLeaf *bl)
{
    int	    i;
    FcBool  nonempty = FcFalse;

    for (i = 0; i < 256/32; i++)
	if ((result->map[i] = al->map[i] & ~bl->map[i]))
	    nonempty = FcTrue;
    return nonempty;
}

FcCharSet *
FcCharSetSubtract (const FcCharSet *a, const FcCharSet *b)
{
    return FcCharSetOperate (a, b, FcCharSetSubtractLeaf, FcTrue, FcFalse);
}

FcBool
FcCharSetHasChar (const FcCharSet *fcs, FcChar32 ucs4)
{
    FcCharLeaf	*leaf;

    if (!fcs)
	return FcFalse;
    leaf = FcCharSetFindLeaf (fcs, ucs4);
    if (!leaf)
	return FcFalse;
    return (leaf->map[(ucs4 & 0xff) >> 5] & (1U << (ucs4 & 0x1f))) != 0;
}

static FcChar32
FcCharSetPopCount (FcChar32 c1)
{
#if __GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 4)
    return __builtin_popcount (c1);
#else
    /* hackmem 169 */
    FcChar32	c2 = (c1 >> 1) & 033333333333;
    c2 = c1 - c2 - ((c2 >> 1) & 033333333333);
    return (((c2 + (c2 >> 3)) & 030707070707) % 077);
#endif
}

FcChar32
FcCharSetIntersectCount (const FcCharSet *a, const FcCharSet *b)
{
    FcCharSetIter   ai, bi;
    FcChar32	    count = 0;

    if (a && b)
    {
	FcCharSetIterStart (a, &ai);
	FcCharSetIterStart (b, &bi);
	while (ai.leaf && bi.leaf)
	{
	    if (ai.ucs4 == bi.ucs4)
	    {
		FcChar32	*am = ai.leaf->map;
		FcChar32	*bm = bi.leaf->map;
		int		i = 256/32;
		while (i--)
		    count += FcCharSetPopCount (*am++ & *bm++);
		FcCharSetIterNext (a, &ai);
	    }
	    else if (ai.ucs4 < bi.ucs4)
	    {
		ai.ucs4 = bi.ucs4;
		FcCharSetIterSet (a, &ai);
	    }
	    if (bi.ucs4 < ai.ucs4)
	    {
		bi.ucs4 = ai.ucs4;
		FcCharSetIterSet (b, &bi);
	    }
	}
    }
    return count;
}

FcChar32
FcCharSetCount (const FcCharSet *a)
{
    FcCharSetIter   ai;
    FcChar32	    count = 0;

    if (a)
    {
	for (FcCharSetIterStart (a, &ai); ai.leaf; FcCharSetIterNext (a, &ai))
	{
	    int		    i = 256/32;
	    FcChar32	    *am = ai.leaf->map;

	    while (i--)
		count += FcCharSetPopCount (*am++);
	}
    }
    return count;
}

FcChar32
FcCharSetSubtractCount (const FcCharSet *a, const FcCharSet *b)
{
    FcCharSetIter   ai, bi;
    FcChar32	    count = 0;

    if (a && b)
    {
	FcCharSetIterStart (a, &ai);
	FcCharSetIterStart (b, &bi);
	while (ai.leaf)
	{
	    if (ai.ucs4 <= bi.ucs4)
	    {
		FcChar32	*am = ai.leaf->map;
		int		i = 256/32;
		if (ai.ucs4 == bi.ucs4)
		{
		    FcChar32	*bm = bi.leaf->map;
		    while (i--)
			count += FcCharSetPopCount (*am++ & ~*bm++);
		}
		else
		{
		    while (i--)
			count += FcCharSetPopCount (*am++);
		}
		FcCharSetIterNext (a, &ai);
	    }
	    else if (bi.leaf)
	    {
		bi.ucs4 = ai.ucs4;
		FcCharSetIterSet (b, &bi);
	    }
	}
    }
    return count;
}

/*
 * return FcTrue iff a is a subset of b
 */
FcBool
FcCharSetIsSubset (const FcCharSet *a, const FcCharSet *b)
{
    int		ai, bi;
    FcChar16	an, bn;

    if (a == b)
	return FcTrue;
    if (!a || !b)
	return FcFalse;
    bi = 0;
    ai = 0;
    while (ai < a->num && bi < b->num)
    {
	an = FcCharSetNumbers(a)[ai];
	bn = FcCharSetNumbers(b)[bi];
	/*
	 * Check matching pages
	 */
	if (an == bn)
	{
	    FcChar32	*am = FcCharSetLeaf(a, ai)->map;
	    FcChar32	*bm = FcCharSetLeaf(b, bi)->map;
	
	    if (am != bm)
	    {
		int	i = 256/32;
		/*
		 * Does am have any bits not in bm?
		 */
		while (i--)
		    if (*am++ & ~*bm++)
			return FcFalse;
	    }
	    ai++;
	    bi++;
	}
	/*
	 * Does a have any pages not in b?
	 */
	else if (an < bn)
	    return FcFalse;
	else
	{
	    bi = FcCharSetFindLeafForward (b, bi + 1, an);
	    if (bi < 0)
		bi = -bi - 1;
	}
    }
    /*
     * did we look at every page?
     */
    return ai >= a->num;
}

/*
 * These two functions efficiently walk the entire charmap for
 * other software (like pango) that want their own copy
 */

FcChar32
FcCharSetNextPage (const FcCharSet  *a,
		   FcChar32	    map[FC_CHARSET_MAP_SIZE],
		   FcChar32	    *next)
{
    FcCharSetIter   ai;
    FcChar32	    page;

    if (!a)
	return FC_CHARSET_DONE;
    ai.ucs4 = *next;
    FcCharSetIterSet (a, &ai);
    if (!ai.leaf)
	return FC_CHARSET_DONE;

    /*
     * Save current information
     */
    page = ai.ucs4;
    memcpy (map, ai.leaf->map, sizeof (ai.leaf->map));
    /*
     * Step to next page
     */
    FcCharSetIterNext (a, &ai);
    *next = ai.ucs4;

    return page;
}

FcChar32
FcCharSetFirstPage (const FcCharSet *a,
		    FcChar32	    map[FC_CHARSET_MAP_SIZE],
		    FcChar32	    *next)
{
    *next = 0;
    return FcCharSetNextPage (a, map, next);
}

/*
 * old coverage API, rather hard to use correctly
 */

FcChar32
FcCharSetCoverage (const FcCharSet *a, FcChar32 page, FcChar32 *result)
{
    FcCharSetIter   ai;

    ai.ucs4 = page;
    FcCharSetIterSet (a, &ai);
    if (!ai.leaf)
    {
	memset (result, '\0', 256 / 8);
	page = 0;
    }
    else
    {
	memcpy (result, ai.leaf->map, sizeof (ai.leaf->map));
	FcCharSetIterNext (a, &ai);
	page = ai.ucs4;
    }
    return page;
}

static FcBool
FcNameParseRange (FcChar8 **string, FcChar32 *pfirst, FcChar32 *plast)
{
	char *s = (char *) *string;
	char *t;
	long first, last;

	while (isspace((unsigned char) *s))
	    s++;
	t = s;
	errno = 0;
	first = last = strtol (s, &s, 16);
	if (errno)
	    return FcFalse;
	while (isspace((unsigned char) *s))
	    s++;
	if (*s == '-')
	{
	    s++;
	    errno = 0;
	    last = strtol (s, &s, 16);
	    if (errno)
		return FcFalse;
	}

	if (s == t || first < 0 || last < 0 || last < first || last > 0x10ffff)
	     return FcFalse;

	*string = (FcChar8 *) s;
	*pfirst = first;
	*plast = last;
	return FcTrue;
}

FcCharSet *
FcNameParseCharSet (FcChar8 *string)
{
    FcCharSet	*c;
    FcChar32	first, last;

    c = FcCharSetCreate ();
    if (!c)
	goto bail0;
    while (*string)
    {
	FcChar32 u;

	if (!FcNameParseRange (&string, &first, &last))
		goto bail1;

	for (u = first; u < last + 1; u++)
	    FcCharSetAddChar (c, u);
    }
    return c;
bail1:
    FcCharSetDestroy (c);
bail0:
    return NULL;
}

static void
FcNameUnparseUnicode (FcStrBuf *buf, FcChar32 u)
{
    FcChar8	    buf_static[64];
    snprintf ((char *) buf_static, sizeof (buf_static), "%x", u);
    FcStrBufString (buf, buf_static);
}

FcBool
FcNameUnparseCharSet (FcStrBuf *buf, const FcCharSet *c)
{
    FcCharSetIter   ci;
    FcChar32	    first, last;
    int		    i;
#ifdef CHECK
    int		    len = buf->len;
#endif

    first = last = 0x7FFFFFFF;

    for (FcCharSetIterStart (c, &ci);
	 ci.leaf;
	 FcCharSetIterNext (c, &ci))
    {
	for (i = 0; i < 256/32; i++)
	{
	    FcChar32 bits = ci.leaf->map[i];
	    FcChar32 u = ci.ucs4 + i * 32;

	    while (bits)
	    {
		if (bits & 1)
		{
			if (u != last + 1)
			{
			    if (last != first)
			    {
				FcStrBufChar (buf, '-');
				FcNameUnparseUnicode (buf, last);
			    }
			    if (last != 0x7FFFFFFF)
				FcStrBufChar (buf, ' ');
			    /* Start new range. */
			    first = u;
			    FcNameUnparseUnicode (buf, u);
			}
			last = u;
		}
		bits >>= 1;
		u++;
	    }
	}
    }
    if (last != first)
    {
	FcStrBufChar (buf, '-');
	FcNameUnparseUnicode (buf, last);
    }
#ifdef CHECK
    {
	FcCharSet	*check;
	FcChar32	missing;
	FcCharSetIter	ci, checki;
	
	/* null terminate for parser */
	FcStrBufChar (buf, '\0');
	/* step back over null for life after test */
	buf->len--;
	check = FcNameParseCharSet (buf->buf + len);
	FcCharSetIterStart (c, &ci);
	FcCharSetIterStart (check, &checki);
	while (ci.leaf || checki.leaf)
	{
	    if (ci.ucs4 < checki.ucs4)
	    {
		printf ("Missing leaf node at 0x%x\n", ci.ucs4);
		FcCharSetIterNext (c, &ci);
	    }
	    else if (checki.ucs4 < ci.ucs4)
	    {
		printf ("Extra leaf node at 0x%x\n", checki.ucs4);
		FcCharSetIterNext (check, &checki);
	    }
	    else
	    {
		int	    i = 256/32;
		FcChar32    *cm = ci.leaf->map;
		FcChar32    *checkm = checki.leaf->map;

		for (i = 0; i < 256; i += 32)
		{
		    if (*cm != *checkm)
			printf ("Mismatching sets at 0x%08x: 0x%08x != 0x%08x\n",
				ci.ucs4 + i, *cm, *checkm);
		    cm++;
		    checkm++;
		}
		FcCharSetIterNext (c, &ci);
		FcCharSetIterNext (check, &checki);
	    }
	}
	if ((missing = FcCharSetSubtractCount (c, check)))
	    printf ("%d missing in reparsed result\n", missing);
	if ((missing = FcCharSetSubtractCount (check, c)))
	    printf ("%d extra in reparsed result\n", missing);
	FcCharSetDestroy (check);
    }
#endif

    return FcTrue;
}

typedef struct _FcCharLeafEnt FcCharLeafEnt;

struct _FcCharLeafEnt {
    FcCharLeafEnt   *next;
    FcChar32	    hash;
    FcCharLeaf	    leaf;
};

#define FC_CHAR_LEAF_BLOCK	(4096 / sizeof (FcCharLeafEnt))
#define FC_CHAR_LEAF_HASH_SIZE	257

typedef struct _FcCharSetEnt FcCharSetEnt;

struct _FcCharSetEnt {
    FcCharSetEnt	*next;
    FcChar32		hash;
    FcCharSet		set;
};

typedef struct _FcCharSetOrigEnt FcCharSetOrigEnt;

struct _FcCharSetOrigEnt {
    FcCharSetOrigEnt	*next;
    const FcCharSet    	*orig;
    const FcCharSet    	*frozen;
};

#define FC_CHAR_SET_HASH_SIZE    67

struct _FcCharSetFreezer {
    FcCharLeafEnt   *leaf_hash_table[FC_CHAR_LEAF_HASH_SIZE];
    FcCharLeafEnt   **leaf_blocks;
    int		    leaf_block_count;
    FcCharSetEnt    *set_hash_table[FC_CHAR_SET_HASH_SIZE];
    FcCharSetOrigEnt	*orig_hash_table[FC_CHAR_SET_HASH_SIZE];
    FcCharLeafEnt   *current_block;
    int		    leaf_remain;
    int		    leaves_seen;
    int		    charsets_seen;
    int		    leaves_allocated;
    int		    charsets_allocated;
};

static FcCharLeafEnt *
FcCharLeafEntCreate (FcCharSetFreezer *freezer)
{
    if (!freezer->leaf_remain)
    {
	FcCharLeafEnt **newBlocks;

	freezer->leaf_block_count++;
	newBlocks = realloc (freezer->leaf_blocks, freezer->leaf_block_count * sizeof (FcCharLeafEnt *));
	if (!newBlocks)
	    return 0;
	freezer->leaf_blocks = newBlocks;
	freezer->current_block = freezer->leaf_blocks[freezer->leaf_block_count-1] = malloc (FC_CHAR_LEAF_BLOCK * sizeof (FcCharLeafEnt));
	if (!freezer->current_block)
	    return 0;
	freezer->leaf_remain = FC_CHAR_LEAF_BLOCK;
    }
    freezer->leaf_remain--;
    freezer->leaves_allocated++;
    return freezer->current_block++;
}

static FcChar32
FcCharLeafHash (FcCharLeaf *leaf)
{
    FcChar32	hash = 0;
    int		i;

    for (i = 0; i < 256/32; i++)
	hash = ((hash << 1) | (hash >> 31)) ^ leaf->map[i];
    return hash;
}

static FcCharLeaf *
FcCharSetFreezeLeaf (FcCharSetFreezer *freezer, FcCharLeaf *leaf)
{
    FcChar32			hash = FcCharLeafHash (leaf);
    FcCharLeafEnt		**bucket = &freezer->leaf_hash_table[hash % FC_CHAR_LEAF_HASH_SIZE];
    FcCharLeafEnt		*ent;

    for (ent = *bucket; ent; ent = ent->next)
    {
	if (ent->hash == hash && !memcmp (&ent->leaf, leaf, sizeof (FcCharLeaf)))
	    return &ent->leaf;
    }

    ent = FcCharLeafEntCreate(freezer);
    if (!ent)
	return 0;
    ent->leaf = *leaf;
    ent->hash = hash;
    ent->next = *bucket;
    *bucket = ent;
    return &ent->leaf;
}

static FcChar32
FcCharSetHash (FcCharSet *fcs)
{
    FcChar32	hash = 0;
    int		i;

    /* hash in leaves */
    for (i = 0; i < fcs->num; i++)
	hash = ((hash << 1) | (hash >> 31)) ^ FcCharLeafHash (FcCharSetLeaf(fcs,i));
    /* hash in numbers */
    for (i = 0; i < fcs->num; i++)
	hash = ((hash << 1) | (hash >> 31)) ^ FcCharSetNumbers(fcs)[i];
    return hash;
}

static FcBool
FcCharSetFreezeOrig (FcCharSetFreezer *freezer, const FcCharSet *orig, const FcCharSet *frozen)
{
    FcCharSetOrigEnt	**bucket = &freezer->orig_hash_table[((uintptr_t) orig) % FC_CHAR_SET_HASH_SIZE];
    FcCharSetOrigEnt	*ent;

    ent = malloc (sizeof (FcCharSetOrigEnt));
    if (!ent)
	return FcFalse;
    ent->orig = orig;
    ent->frozen = frozen;
    ent->next = *bucket;
    *bucket = ent;
    return FcTrue;
}

static FcCharSet *
FcCharSetFreezeBase (FcCharSetFreezer *freezer, FcCharSet *fcs)
{
    FcChar32		hash = FcCharSetHash (fcs);
    FcCharSetEnt	**bucket = &freezer->set_hash_table[hash % FC_CHAR_SET_HASH_SIZE];
    FcCharSetEnt	*ent;
    int			size;
    int			i;

    for (ent = *bucket; ent; ent = ent->next)
    {
	if (ent->hash == hash &&
	    ent->set.num == fcs->num &&
	    !memcmp (FcCharSetNumbers(&ent->set),
		     FcCharSetNumbers(fcs),
		     fcs->num * sizeof (FcChar16)))
	{
	    FcBool ok = FcTrue;
	    int i;

	    for (i = 0; i < fcs->num; i++)
		if (FcCharSetLeaf(&ent->set, i) != FcCharSetLeaf(fcs, i))
		    ok = FcFalse;
	    if (ok)
		return &ent->set;
	}
    }

    size = (sizeof (FcCharSetEnt) +
	    fcs->num * sizeof (FcCharLeaf *) +
	    fcs->num * sizeof (FcChar16));
    ent = malloc (size);
    if (!ent)
	return 0;

    freezer->charsets_allocated++;

    FcRefSetConst (&ent->set.ref);
    ent->set.num = fcs->num;
    if (fcs->num)
    {
	intptr_t    *ent_leaves;

	ent->set.leaves_offset = sizeof (ent->set);
	ent->set.numbers_offset = (ent->set.leaves_offset +
				   fcs->num * sizeof (intptr_t));

	ent_leaves = FcCharSetLeaves (&ent->set);
	for (i = 0; i < fcs->num; i++)
	    ent_leaves[i] = FcPtrToOffset (ent_leaves,
					   FcCharSetLeaf (fcs, i));
	memcpy (FcCharSetNumbers (&ent->set),
		FcCharSetNumbers (fcs),
		fcs->num * sizeof (FcChar16));
    }
    else
    {
	ent->set.leaves_offset = 0;
	ent->set.numbers_offset = 0;
    }

    ent->hash = hash;
    ent->next = *bucket;
    *bucket = ent;

    return &ent->set;
}

static const FcCharSet *
FcCharSetFindFrozen (FcCharSetFreezer *freezer, const FcCharSet *orig)
{
    FcCharSetOrigEnt    **bucket = &freezer->orig_hash_table[((uintptr_t) orig) % FC_CHAR_SET_HASH_SIZE];
    FcCharSetOrigEnt	*ent;

    for (ent = *bucket; ent; ent = ent->next)
	if (ent->orig == orig)
	    return ent->frozen;
    return NULL;
}

const FcCharSet *
FcCharSetFreeze (FcCharSetFreezer *freezer, const FcCharSet *fcs)
{
    FcCharSet	    *b;
    const FcCharSet *n = 0;
    FcCharLeaf	    *l;
    int		    i;

    b = FcCharSetCreate ();
    if (!b)
	goto bail0;
    for (i = 0; i < fcs->num; i++)
    {
	l = FcCharSetFreezeLeaf (freezer, FcCharSetLeaf(fcs, i));
	if (!l)
	    goto bail1;
	if (!FcCharSetInsertLeaf (b, FcCharSetNumbers(fcs)[i] << 8, l))
	    goto bail1;
    }
    n = FcCharSetFreezeBase (freezer, b);
    if (!FcCharSetFreezeOrig (freezer, fcs, n))
    {
	n = NULL;
	goto bail1;
    }
    freezer->charsets_seen++;
    freezer->leaves_seen += fcs->num;
bail1:
    if (b->num)
	free (FcCharSetLeaves (b));
    if (b->num)
	free (FcCharSetNumbers (b));
    free (b);
bail0:
    return n;
}

FcCharSetFreezer *
FcCharSetFreezerCreate (void)
{
    FcCharSetFreezer	*freezer;

    freezer = calloc (1, sizeof (FcCharSetFreezer));
    return freezer;
}

void
FcCharSetFreezerDestroy (FcCharSetFreezer *freezer)
{
    int i;

    if (FcDebug() & FC_DBG_CACHE)
    {
	printf ("\ncharsets %d -> %d leaves %d -> %d\n",
		freezer->charsets_seen, freezer->charsets_allocated,
		freezer->leaves_seen, freezer->leaves_allocated);
    }
    for (i = 0; i < FC_CHAR_SET_HASH_SIZE; i++)
    {
	FcCharSetEnt	*ent, *next;
	for (ent = freezer->set_hash_table[i]; ent; ent = next)
	{
	    next = ent->next;
	    free (ent);
	}
    }

    for (i = 0; i < FC_CHAR_SET_HASH_SIZE; i++)
    {
	FcCharSetOrigEnt	*ent, *next;
	for (ent = freezer->orig_hash_table[i]; ent; ent = next)
	{
	    next = ent->next;
	    free (ent);
	}
    }

    for (i = 0; i < freezer->leaf_block_count; i++)
	free (freezer->leaf_blocks[i]);

    free (freezer->leaf_blocks);
    free (freezer);
}

FcBool
FcCharSetSerializeAlloc (FcSerialize *serialize, const FcCharSet *cs)
{
    intptr_t	    *leaves;
    FcChar16	    *numbers;
    int		    i;

    if (!FcRefIsConst (&cs->ref))
    {
	if (!serialize->cs_freezer)
	{
	    serialize->cs_freezer = FcCharSetFreezerCreate ();
	    if (!serialize->cs_freezer)
		return FcFalse;
	}
	if (FcCharSetFindFrozen (serialize->cs_freezer, cs))
	    return FcTrue;

        cs = FcCharSetFreeze (serialize->cs_freezer, cs);
    }

    leaves = FcCharSetLeaves (cs);
    numbers = FcCharSetNumbers (cs);

    if (!FcSerializeAlloc (serialize, cs, sizeof (FcCharSet)))
	return FcFalse;
    if (!FcSerializeAlloc (serialize, leaves, cs->num * sizeof (intptr_t)))
	return FcFalse;
    if (!FcSerializeAlloc (serialize, numbers, cs->num * sizeof (FcChar16)))
	return FcFalse;
    for (i = 0; i < cs->num; i++)
	if (!FcSerializeAlloc (serialize, FcCharSetLeaf(cs, i),
			       sizeof (FcCharLeaf)))
	    return FcFalse;
    return FcTrue;
}

FcCharSet *
FcCharSetSerialize(FcSerialize *serialize, const FcCharSet *cs)
{
    FcCharSet	*cs_serialized;
    intptr_t	*leaves, *leaves_serialized;
    FcChar16	*numbers, *numbers_serialized;
    FcCharLeaf	*leaf, *leaf_serialized;
    int		i;

    if (!FcRefIsConst (&cs->ref) && serialize->cs_freezer)
    {
	cs = FcCharSetFindFrozen (serialize->cs_freezer, cs);
	if (!cs)
	    return NULL;
    }
		
    cs_serialized = FcSerializePtr (serialize, cs);
    if (!cs_serialized)
	return NULL;

    FcRefSetConst (&cs_serialized->ref);
    cs_serialized->num = cs->num;

    if (cs->num)
    {
	leaves = FcCharSetLeaves (cs);
	leaves_serialized = FcSerializePtr (serialize, leaves);
	if (!leaves_serialized)
	    return NULL;

	cs_serialized->leaves_offset = FcPtrToOffset (cs_serialized,
						      leaves_serialized);
	
	numbers = FcCharSetNumbers (cs);
	numbers_serialized = FcSerializePtr (serialize, numbers);
	if (!numbers)
	    return NULL;

	cs_serialized->numbers_offset = FcPtrToOffset (cs_serialized,
						       numbers_serialized);

	for (i = 0; i < cs->num; i++)
	{
	    leaf = FcCharSetLeaf (cs, i);
	    leaf_serialized = FcSerializePtr (serialize, leaf);
	    if (!leaf_serialized)
		return NULL;
	    *leaf_serialized = *leaf;
	    leaves_serialized[i] = FcPtrToOffset (leaves_serialized,
						  leaf_serialized);
	    numbers_serialized[i] = numbers[i];
	}
    }
    else
    {
	cs_serialized->leaves_offset = 0;
	cs_serialized->numbers_offset = 0;
    }

    return cs_serialized;
}
#define __fccharset__
#include "fcaliastail.h"
#undef __fccharset__
