/*
 * fontconfig/src/fclang.c
 *
 * Copyright © 2002 Keith Packard
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
#include "fcftint.h"

/* Objects MT-safe for readonly access. */

typedef struct {
    const FcChar8    	lang[16];
    const FcCharSet	charset;
} FcLangCharSet;

typedef struct {
    int begin;
    int end;
} FcLangCharSetRange;

#include "../fc-lang/fclang.h"

struct _FcLangSet {
    FcStrSet	*extra;
    FcChar32    map_size;
    FcChar32	map[NUM_LANG_SET_MAP];
};

static int FcLangSetIndex (const FcChar8 *lang);


static void
FcLangSetBitSet (FcLangSet    *ls,
		 unsigned int  id)
{
  unsigned int bucket;

  id = fcLangCharSetIndices[id];
  bucket = id >> 5;
  if (bucket >= ls->map_size)
    return; /* shouldn't happen really */

  ls->map[bucket] |= ((FcChar32) 1U << (id & 0x1f));
}

static FcBool
FcLangSetBitGet (const FcLangSet *ls,
		 unsigned int     id)
{
  unsigned int bucket;

  id = fcLangCharSetIndices[id];
  bucket = id >> 5;
  if (bucket >= ls->map_size)
    return FcFalse;

  return ((ls->map[bucket] >> (id & 0x1f)) & 1) ? FcTrue : FcFalse;
}

static void
FcLangSetBitReset (FcLangSet    *ls,
		   unsigned int  id)
{
  unsigned int bucket;

  id = fcLangCharSetIndices[id];
  bucket = id >> 5;
  if (bucket >= ls->map_size)
    return; /* shouldn't happen really */

  ls->map[bucket] &= ~((FcChar32) 1U << (id & 0x1f));
}

FcLangSet *
FcFreeTypeLangSet (const FcCharSet  *charset,
		   const FcChar8    *exclusiveLang)
{
    int		    i, j;
    FcChar32	    missing;
    const FcCharSet *exclusiveCharset = 0;
    FcLangSet	    *ls;

    if (exclusiveLang)
	exclusiveCharset = FcLangGetCharSet (exclusiveLang);
    ls = FcLangSetCreate ();
    if (!ls)
	return 0;
    if (FcDebug() & FC_DBG_LANGSET)
    {
	printf ("font charset");
	FcCharSetPrint (charset);
	printf ("\n");
    }
    for (i = 0; i < NUM_LANG_CHAR_SET; i++)
    {
	if (FcDebug() & FC_DBG_LANGSET)
	{
	    printf ("%s charset", fcLangCharSets[i].lang);
	    FcCharSetPrint (&fcLangCharSets[i].charset);
	    printf ("\n");
	}
	
	/*
	 * Check for Han charsets to make fonts
	 * which advertise support for a single language
	 * not support other Han languages
	 */
	if (exclusiveCharset &&
	    FcFreeTypeIsExclusiveLang (fcLangCharSets[i].lang))
	{
	    if (fcLangCharSets[i].charset.num != exclusiveCharset->num)
		continue;

	    for (j = 0; j < fcLangCharSets[i].charset.num; j++)
		if (FcCharSetLeaf(&fcLangCharSets[i].charset, j) !=
		    FcCharSetLeaf(exclusiveCharset, j))
		    continue;
	}
	missing = FcCharSetSubtractCount (&fcLangCharSets[i].charset, charset);
        if (FcDebug() & FC_DBG_SCANV)
	{
	    if (missing && missing < 10)
	    {
		FcCharSet   *missed = FcCharSetSubtract (&fcLangCharSets[i].charset,
							 charset);
		FcChar32    ucs4;
		FcChar32    map[FC_CHARSET_MAP_SIZE];
		FcChar32    next;

		printf ("\n%s(%u) ", fcLangCharSets[i].lang, missing);
		printf ("{");
		for (ucs4 = FcCharSetFirstPage (missed, map, &next);
		     ucs4 != FC_CHARSET_DONE;
		     ucs4 = FcCharSetNextPage (missed, map, &next))
		{
		    int	    i, j;
		    for (i = 0; i < FC_CHARSET_MAP_SIZE; i++)
			if (map[i])
			{
			    for (j = 0; j < 32; j++)
				if (map[i] & (1U << j))
				    printf (" %04x", ucs4 + i * 32 + j);
			}
		}
		printf (" }\n\t");
		FcCharSetDestroy (missed);
	    }
	    else
		printf ("%s(%u) ", fcLangCharSets[i].lang, missing);
	}
	if (!missing)
	    FcLangSetBitSet (ls, i);
    }

    if (FcDebug() & FC_DBG_SCANV)
	printf ("\n");


    return ls;
}

FcChar8 *
FcLangNormalize (const FcChar8 *lang)
{
    FcChar8 *result = NULL, *s, *orig;
    char *territory, *encoding, *modifier;
    size_t llen, tlen = 0, mlen = 0;

    if (!lang || !*lang)
	return NULL;

    /* might be called without initialization */
    FcInitDebug ();

    if (FcStrCmpIgnoreCase (lang, (const FcChar8 *)"C") == 0 ||
	FcStrCmpIgnoreCase (lang, (const FcChar8 *)"C.UTF-8") == 0 ||
	FcStrCmpIgnoreCase (lang, (const FcChar8 *)"C.utf8") == 0 ||
	FcStrCmpIgnoreCase (lang, (const FcChar8 *)"POSIX") == 0)
    {
	result = FcStrCopy ((const FcChar8 *)"en");
	goto bail;
    }

    s = FcStrCopy (lang);
    if (!s)
	goto bail;

    /* from the comments in glibc:
     *
     * LOCALE can consist of up to four recognized parts for the XPG syntax:
     *
     *            language[_territory[.codeset]][@modifier]
     *
     * Beside the first all of them are allowed to be missing.  If the
     * full specified locale is not found, the less specific one are
     * looked for.  The various part will be stripped off according to
     * the following order:
     *            (1) codeset
     *            (2) normalized codeset
     *            (3) territory
     *            (4) modifier
     *
     * So since we don't take care of the codeset part here, what patterns
     * we need to deal with is:
     *
     *   1. language_territory@modifier
     *   2. language@modifier
     *   3. language
     *
     * then. and maybe no need to try language_territory here.
     */
    modifier = strchr ((const char *) s, '@');
    if (modifier)
    {
	*modifier = 0;
	modifier++;
	mlen = strlen (modifier);
    }
    encoding = strchr ((const char *) s, '.');
    if (encoding)
    {
	*encoding = 0;
	encoding++;
	if (modifier)
	{
	    memmove (encoding, modifier, mlen + 1);
	    modifier = encoding;
	}
    }
    territory = strchr ((const char *) s, '_');
    if (!territory)
	territory = strchr ((const char *) s, '-');
    if (territory)
    {
	*territory = 0;
	territory++;
	tlen = strlen (territory);
    }
    llen = strlen ((const char *) s);
    if (llen < 2 || llen > 3)
    {
	fprintf (stderr, "Fontconfig warning: ignoring %s: not a valid language tag\n",
		 lang);
	goto bail0;
    }
    if (territory && (tlen < 2 || tlen > 3) &&
	!(territory[0] == 'z' && tlen < 5))
    {
	fprintf (stderr, "Fontconfig warning: ignoring %s: not a valid region tag\n",
		 lang);
	goto bail0;
    }
    if (territory)
	territory[-1] = '-';
    if (modifier)
	modifier[-1] = '@';
    orig = FcStrDowncase (s);
    if (!orig)
	goto bail0;
    if (territory)
    {
	if (FcDebug () & FC_DBG_LANGSET)
	    printf("Checking the existence of %s.orth\n", s);
	if (FcLangSetIndex (s) < 0)
	{
	    memmove (territory - 1, territory + tlen, (mlen > 0 ? mlen + 1 : 0) + 1);
	    if (modifier)
		modifier = territory;
	}
	else
	{
	    result = s;
	    /* we'll miss the opportunity to reduce the correct size
	     * of the allocated memory for the string after that.
	     */
	    s = NULL;
	    goto bail1;
	}
    }
    if (modifier)
    {
	if (FcDebug () & FC_DBG_LANGSET)
	    printf("Checking the existence of %s.orth\n", s);
	if (FcLangSetIndex (s) < 0)
	    modifier[-1] = 0;
	else
	{
	    result = s;
	    /* we'll miss the opportunity to reduce the correct size
	     * of the allocated memory for the string after that.
	     */
	    s = NULL;
	    goto bail1;
	}
    }
    if (FcDebug () & FC_DBG_LANGSET)
	printf("Checking the existence of %s.orth\n", s);
    if (FcLangSetIndex (s) < 0)
    {
	/* there seems no languages matched in orth.
	 * add the language as is for fallback.
	 */
	result = orig;
	orig = NULL;
    }
    else
    {
	result = s;
	/* we'll miss the opportunity to reduce the correct size
	 * of the allocated memory for the string after that.
	 */
	s = NULL;
    }
  bail1:
    if (orig)
	FcStrFree (orig);
  bail0:
    if (s)
	free (s);
  bail:
    if (FcDebug () & FC_DBG_LANGSET)
    {
	if (result)
	    printf ("normalized: %s -> %s\n", lang, result);
	else
	    printf ("Unable to normalize %s\n", lang);
    }

    return result;
}

#define FcLangEnd(c)	((c) == '-' || (c) == '\0')

FcLangResult
FcLangCompare (const FcChar8 *s1, const FcChar8 *s2)
{
    FcChar8	    c1, c2;
    FcLangResult    result = FcLangDifferentLang;
    const FcChar8  *s1_orig = s1;
    FcBool	    is_und;

    is_und = FcToLower (s1[0]) == 'u' &&
	     FcToLower (s1[1]) == 'n' &&
	     FcToLower (s1[2]) == 'd' &&
	     FcLangEnd (s1[3]);

    for (;;)
    {
	c1 = *s1++;
	c2 = *s2++;
	
	c1 = FcToLower (c1);
	c2 = FcToLower (c2);
	if (c1 != c2)
	{
	    if (!is_und && FcLangEnd (c1) && FcLangEnd (c2))
		result = FcLangDifferentTerritory;
	    return result;
	}
	else if (!c1)
	{
	    return is_und ? result : FcLangEqual;
	}
	else if (c1 == '-')
	{
	    if (!is_und)
		result = FcLangDifferentTerritory;
	}

	/* If we parsed past "und-", then do not consider it undefined anymore,
	 * as there's *something* specified. */
	if (is_und && s1 - s1_orig == 4)
	    is_und = FcFalse;
    }
}

/*
 * Return FcTrue when super contains sub.
 *
 * super contains sub if super and sub have the same
 * language and either the same country or one
 * is missing the country
 */

static FcBool
FcLangContains (const FcChar8 *super, const FcChar8 *sub)
{
    FcChar8	    c1, c2;

    for (;;)
    {
	c1 = *super++;
	c2 = *sub++;
	
	c1 = FcToLower (c1);
	c2 = FcToLower (c2);
	if (c1 != c2)
	{
	    /* see if super has a country while sub is missing one */
	    if (c1 == '-' && c2 == '\0')
		return FcTrue;
	    /* see if sub has a country while super is missing one */
	    if (c1 == '\0' && c2 == '-')
		return FcTrue;
	    return FcFalse;
	}
	else if (!c1)
	    return FcTrue;
    }
}

const FcCharSet *
FcLangGetCharSet (const FcChar8 *lang)
{
    int		i;
    int		country = -1;

    for (i = 0; i < NUM_LANG_CHAR_SET; i++)
    {
	switch (FcLangCompare (lang, fcLangCharSets[i].lang)) {
	case FcLangEqual:
	    return &fcLangCharSets[i].charset;
	case FcLangDifferentTerritory:
	    if (country == -1)
		country = i;
	case FcLangDifferentLang:
	default:
	    break;
	}
    }
    if (country == -1)
	return 0;
    return &fcLangCharSets[country].charset;
}

FcStrSet *
FcGetLangs (void)
{
    FcStrSet *langs;
    int	i;

    langs = FcStrSetCreate();
    if (!langs)
	return 0;

    for (i = 0; i < NUM_LANG_CHAR_SET; i++)
	FcStrSetAdd (langs, fcLangCharSets[i].lang);

    return langs;
}

FcLangSet *
FcLangSetCreate (void)
{
    FcLangSet	*ls;

    ls = malloc (sizeof (FcLangSet));
    if (!ls)
	return 0;
    memset (ls->map, '\0', sizeof (ls->map));
    ls->map_size = NUM_LANG_SET_MAP;
    ls->extra = 0;
    return ls;
}

void
FcLangSetDestroy (FcLangSet *ls)
{
    if (!ls)
	return;

    if (ls->extra)
	FcStrSetDestroy (ls->extra);
    free (ls);
}

FcLangSet *
FcLangSetCopy (const FcLangSet *ls)
{
    FcLangSet	*new;

    if (!ls)
	return NULL;

    new = FcLangSetCreate ();
    if (!new)
	goto bail0;
    memset (new->map, '\0', sizeof (new->map));
    memcpy (new->map, ls->map, FC_MIN (sizeof (new->map), ls->map_size * sizeof (ls->map[0])));
    if (ls->extra)
    {
	FcStrList	*list;
	FcChar8		*extra;
	
	new->extra = FcStrSetCreate ();
	if (!new->extra)
	    goto bail1;

	list = FcStrListCreate (ls->extra);	
	if (!list)
	    goto bail1;
	
	while ((extra = FcStrListNext (list)))
	    if (!FcStrSetAdd (new->extra, extra))
	    {
		FcStrListDone (list);
		goto bail1;
	    }
	FcStrListDone (list);
    }
    return new;
bail1:
    FcLangSetDestroy (new);
bail0:
    return 0;
}

/* When the language isn't found, the return value r is such that:
 *  1) r < 0
 *  2) -r -1 is the index of the first language in fcLangCharSets that comes
 *     after the 'lang' argument in lexicographic order.
 *
 *  The -1 is necessary to avoid problems with language id 0 (otherwise, we
 *  wouldn't be able to distinguish between “language found, id is 0” and
 *  “language not found, sorts right before the language with id 0”).
 */
static int
FcLangSetIndex (const FcChar8 *lang)
{
    int	    low, high, mid = 0;
    int	    cmp = 0;
    FcChar8 firstChar = FcToLower(lang[0]);
    FcChar8 secondChar = firstChar ? FcToLower(lang[1]) : '\0';

    if (firstChar < 'a')
    {
	low = 0;
	high = fcLangCharSetRanges[0].begin;
    }
    else if(firstChar > 'z')
    {
	low = fcLangCharSetRanges[25].begin;
	high = NUM_LANG_CHAR_SET - 1;
    }
    else
    {
	low = fcLangCharSetRanges[firstChar - 'a'].begin;
	high = fcLangCharSetRanges[firstChar - 'a'].end;
	/* no matches */
	if (low > high)
	    return -(low+1); /* one past next entry after where it would be */
    }

    while (low <= high)
    {
	mid = (high + low) >> 1;
	if(fcLangCharSets[mid].lang[0] != firstChar)
	    cmp = FcStrCmpIgnoreCase(fcLangCharSets[mid].lang, lang);
	else
	{   /* fast path for resolving 2-letter languages (by far the most common) after
	     * finding the first char (probably already true because of the hash table) */
	    cmp = fcLangCharSets[mid].lang[1] - secondChar;
	    if (cmp == 0 &&
		(fcLangCharSets[mid].lang[2] != '\0' ||
		 lang[2] != '\0'))
	    {
		cmp = FcStrCmpIgnoreCase(fcLangCharSets[mid].lang+2,
					 lang+2);
	    }
	}
	if (cmp == 0)
	    return mid;
	if (cmp < 0)
	    low = mid + 1;
	else
	    high = mid - 1;
    }
    if (cmp < 0)
	mid++;
    return -(mid + 1);
}

FcBool
FcLangSetAdd (FcLangSet *ls, const FcChar8 *lang)
{
    int	    id;

    id = FcLangSetIndex (lang);
    if (id >= 0)
    {
	FcLangSetBitSet (ls, id);
	return FcTrue;
    }
    if (!ls->extra)
    {
	ls->extra = FcStrSetCreate ();
	if (!ls->extra)
	    return FcFalse;
    }
    return FcStrSetAdd (ls->extra, lang);
}

FcBool
FcLangSetDel (FcLangSet *ls, const FcChar8 *lang)
{
    int	id;

    id = FcLangSetIndex (lang);
    if (id >= 0)
    {
	FcLangSetBitReset (ls, id);
    }
    else if (ls->extra)
    {
	FcStrSetDel (ls->extra, lang);
    }
    return FcTrue;
}

FcLangResult
FcLangSetHasLang (const FcLangSet *ls, const FcChar8 *lang)
{
    int		    id;
    FcLangResult    best, r;
    int		    i;

    id = FcLangSetIndex (lang);
    if (id < 0)
	id = -id - 1;
    else if (FcLangSetBitGet (ls, id))
	return FcLangEqual;
    best = FcLangDifferentLang;
    for (i = id - 1; i >= 0; i--)
    {
	r = FcLangCompare (lang, fcLangCharSets[i].lang);
	if (r == FcLangDifferentLang)
	    break;
	if (FcLangSetBitGet (ls, i) && r < best)
	    best = r;
    }
    for (i = id; i < NUM_LANG_CHAR_SET; i++)
    {
	r = FcLangCompare (lang, fcLangCharSets[i].lang);
	if (r == FcLangDifferentLang)
	    break;
	if (FcLangSetBitGet (ls, i) && r < best)
	    best = r;
    }
    if (ls->extra)
    {
	FcStrList	*list = FcStrListCreate (ls->extra);
	FcChar8		*extra;
	
	if (list)
	{
	    while (best > FcLangEqual && (extra = FcStrListNext (list)))
	    {
		r = FcLangCompare (lang, extra);
		if (r < best)
		    best = r;
	    }
	    FcStrListDone (list);
	}
    }
    return best;
}

static FcLangResult
FcLangSetCompareStrSet (const FcLangSet *ls, FcStrSet *set)
{
    FcStrList	    *list = FcStrListCreate (set);
    FcLangResult    r, best = FcLangDifferentLang;
    FcChar8	    *extra;

    if (list)
    {
	while (best > FcLangEqual && (extra = FcStrListNext (list)))
	{
	    r = FcLangSetHasLang (ls, extra);
	    if (r < best)
		best = r;
	}
	FcStrListDone (list);
    }
    return best;
}

FcLangResult
FcLangSetCompare (const FcLangSet *lsa, const FcLangSet *lsb)
{
    int		    i, j, count;
    FcLangResult    best, r;
    FcChar32 aInCountrySet, bInCountrySet;

    count = FC_MIN (lsa->map_size, lsb->map_size);
    count = FC_MIN (NUM_LANG_SET_MAP, count);
    for (i = 0; i < count; i++)
	if (lsa->map[i] & lsb->map[i])
	    return FcLangEqual;
    best = FcLangDifferentLang;
    for (j = 0; j < NUM_COUNTRY_SET; j++)
    {
	aInCountrySet = 0;
	bInCountrySet = 0;

	for (i = 0; i < count; i++)
	{
	    aInCountrySet |= lsa->map[i] & fcLangCountrySets[j][i];
	    bInCountrySet |= lsb->map[i] & fcLangCountrySets[j][i];

	    if (aInCountrySet && bInCountrySet)
	    {
		best = FcLangDifferentTerritory;
		break;
	    }
	}
    }
    if (lsa->extra)
    {
	r = FcLangSetCompareStrSet (lsb, lsa->extra);
	if (r < best)
	    best = r;
    }
    if (best > FcLangEqual && lsb->extra)
    {
	r = FcLangSetCompareStrSet (lsa, lsb->extra);
	if (r < best)
	    best = r;
    }
    return best;
}

/*
 * Used in computing values -- mustn't allocate any storage
 */
FcLangSet *
FcLangSetPromote (const FcChar8 *lang, FcValuePromotionBuffer *vbuf)
{
    int		id;
    typedef struct {
	FcLangSet  ls;
	FcStrSet   strs;
	FcChar8   *str;
    } FcLangSetPromotionBuffer;
    FcLangSetPromotionBuffer *buf = (FcLangSetPromotionBuffer *) vbuf;

    FC_ASSERT_STATIC (sizeof (FcLangSetPromotionBuffer) <= sizeof (FcValuePromotionBuffer));

    memset (buf->ls.map, '\0', sizeof (buf->ls.map));
    buf->ls.map_size = NUM_LANG_SET_MAP;
    buf->ls.extra = 0;
    if (lang)
    {
	id = FcLangSetIndex (lang);
	if (id >= 0)
	{
	    FcLangSetBitSet (&buf->ls, id);
	}
	else
	{
	    buf->ls.extra = &buf->strs;
	    buf->strs.num = 1;
	    buf->strs.size = 1;
	    buf->strs.strs = &buf->str;
	    FcRefInit (&buf->strs.ref, 1);
	    buf->str = (FcChar8 *) lang;
	}
    }
    return &buf->ls;
}

FcChar32
FcLangSetHash (const FcLangSet *ls)
{
    FcChar32	h = 0;
    int		i, count;

    count = FC_MIN (ls->map_size, NUM_LANG_SET_MAP);
    for (i = 0; i < count; i++)
	h ^= ls->map[i];
    if (ls->extra)
	h ^= ls->extra->num;
    return h;
}

FcLangSet *
FcNameParseLangSet (const FcChar8 *string)
{
    FcChar8	    lang[32], c = 0;
    int i;
    FcLangSet	    *ls;

    ls = FcLangSetCreate ();
    if (!ls)
	goto bail0;

    for(;;)
    {
	for(i = 0; i < 31;i++)
	{
	    c = *string++;
	    if(c == '\0' || c == '|')
		break; /* end of this code */
	    lang[i] = c;
	}
	lang[i] = '\0';
	if (!FcLangSetAdd (ls, lang))
	    goto bail1;
	if(c == '\0')
	    break;
    }
    return ls;
bail1:
    FcLangSetDestroy (ls);
bail0:
    return 0;
}

FcBool
FcNameUnparseLangSet (FcStrBuf *buf, const FcLangSet *ls)
{
    int		i, bit, count;
    FcChar32	bits;
    FcBool	first = FcTrue;

    count = FC_MIN (ls->map_size, NUM_LANG_SET_MAP);
    for (i = 0; i < count; i++)
    {
	if ((bits = ls->map[i]))
	{
	    for (bit = 0; bit <= 31; bit++)
		if (bits & (1U << bit))
		{
		    int id = (i << 5) | bit;
		    if (!first)
			if (!FcStrBufChar (buf, '|'))
			    return FcFalse;
		    if (!FcStrBufString (buf, fcLangCharSets[fcLangCharSetIndicesInv[id]].lang))
			return FcFalse;
		    first = FcFalse;
		}
	}
    }
    if (ls->extra)
    {
	FcStrList   *list = FcStrListCreate (ls->extra);
	FcChar8	    *extra;

	if (!list)
	    return FcFalse;
	while ((extra = FcStrListNext (list)))
	{
	    if (!first)
		if (!FcStrBufChar (buf, '|'))
                {
                    FcStrListDone (list);
		    return FcFalse;
                }
	    if (!FcStrBufString (buf, extra))
                {
                    FcStrListDone (list);
                    return FcFalse;
                }
	    first = FcFalse;
	}
        FcStrListDone (list);
    }
    return FcTrue;
}

FcBool
FcLangSetEqual (const FcLangSet *lsa, const FcLangSet *lsb)
{
    int	    i, count;

    count = FC_MIN (lsa->map_size, lsb->map_size);
    count = FC_MIN (NUM_LANG_SET_MAP, count);
    for (i = 0; i < count; i++)
    {
	if (lsa->map[i] != lsb->map[i])
	    return FcFalse;
    }
    if (!lsa->extra && !lsb->extra)
	return FcTrue;
    if (lsa->extra && lsb->extra)
	return FcStrSetEqual (lsa->extra, lsb->extra);
    return FcFalse;
}

static FcBool
FcLangSetContainsLang (const FcLangSet *ls, const FcChar8 *lang)
{
    int		    id;
    int		    i;

    id = FcLangSetIndex (lang);
    if (id < 0)
	id = -id - 1;
    else if (FcLangSetBitGet (ls, id))
	return FcTrue;
    /*
     * search up and down among equal languages for a match
     */
    for (i = id - 1; i >= 0; i--)
    {
	if (FcLangCompare (fcLangCharSets[i].lang, lang) == FcLangDifferentLang)
	    break;
	if (FcLangSetBitGet (ls, i) &&
	    FcLangContains (fcLangCharSets[i].lang, lang))
	    return FcTrue;
    }
    for (i = id; i < NUM_LANG_CHAR_SET; i++)
    {
	if (FcLangCompare (fcLangCharSets[i].lang, lang) == FcLangDifferentLang)
	    break;
	if (FcLangSetBitGet (ls, i) &&
	    FcLangContains (fcLangCharSets[i].lang, lang))
	    return FcTrue;
    }
    if (ls->extra)
    {
	FcStrList	*list = FcStrListCreate (ls->extra);
	FcChar8		*extra;
	
	if (list)
	{
	    while ((extra = FcStrListNext (list)))
	    {
		if (FcLangContains (extra, lang))
		    break;
	    }
	    FcStrListDone (list);
    	    if (extra)
		return FcTrue;
	}
    }
    return FcFalse;
}

/*
 * return FcTrue if lsa contains every language in lsb
 */
FcBool
FcLangSetContains (const FcLangSet *lsa, const FcLangSet *lsb)
{
    int		    i, j, count;
    FcChar32	    missing;

    if (FcDebug() & FC_DBG_MATCHV)
    {
	printf ("FcLangSet "); FcLangSetPrint (lsa);
	printf (" contains "); FcLangSetPrint (lsb);
	printf ("\n");
    }
    /*
     * check bitmaps for missing language support
     */
    count = FC_MIN (lsa->map_size, lsb->map_size);
    count = FC_MIN (NUM_LANG_SET_MAP, count);
    for (i = 0; i < count; i++)
    {
	missing = lsb->map[i] & ~lsa->map[i];
	if (missing)
	{
	    for (j = 0; j < 32; j++)
		if (missing & (1U << j))
		{
		    if (!FcLangSetContainsLang (lsa,
						fcLangCharSets[fcLangCharSetIndicesInv[i*32 + j]].lang))
		    {
			if (FcDebug() & FC_DBG_MATCHV)
			    printf ("\tMissing bitmap %s\n", fcLangCharSets[fcLangCharSetIndicesInv[i*32+j]].lang);
			return FcFalse;
		    }
		}
	}
    }
    if (lsb->extra)
    {
	FcStrList   *list = FcStrListCreate (lsb->extra);
	FcChar8	    *extra;

	if (list)
	{
	    while ((extra = FcStrListNext (list)))
	    {
		if (!FcLangSetContainsLang (lsa, extra))
		{
		    if (FcDebug() & FC_DBG_MATCHV)
			printf ("\tMissing string %s\n", extra);
		    break;
		}
	    }
	    FcStrListDone (list);
	    if (extra)
		return FcFalse;
	}
    }
    return FcTrue;
}

FcBool
FcLangSetSerializeAlloc (FcSerialize *serialize, const FcLangSet *l)
{
    if (!FcSerializeAlloc (serialize, l, sizeof (FcLangSet)))
	return FcFalse;
    return FcTrue;
}

FcLangSet *
FcLangSetSerialize(FcSerialize *serialize, const FcLangSet *l)
{
    FcLangSet	*l_serialize = FcSerializePtr (serialize, l);

    if (!l_serialize)
	return NULL;
    memset (l_serialize->map, '\0', sizeof (l_serialize->map));
    memcpy (l_serialize->map, l->map, FC_MIN (sizeof (l_serialize->map), l->map_size * sizeof (l->map[0])));
    l_serialize->map_size = NUM_LANG_SET_MAP;
    l_serialize->extra = NULL; /* We don't serialize ls->extra */
    return l_serialize;
}

FcStrSet *
FcLangSetGetLangs (const FcLangSet *ls)
{
    FcStrSet *langs;
    int	      i;

    langs = FcStrSetCreate();
    if (!langs)
	return 0;

    for (i = 0; i < NUM_LANG_CHAR_SET; i++)
	if (FcLangSetBitGet (ls, i))
	    FcStrSetAdd (langs, fcLangCharSets[i].lang);

    if (ls->extra)
    {
	FcStrList	*list = FcStrListCreate (ls->extra);
	FcChar8		*extra;

	if (list)
	{
	    while ((extra = FcStrListNext (list)))
		FcStrSetAdd (langs, extra);

	    FcStrListDone (list);
	}
    }

    return langs;
}

static FcLangSet *
FcLangSetOperate(const FcLangSet	*a,
		 const FcLangSet	*b,
		 FcBool			(*func) (FcLangSet 	*ls,
						 const FcChar8	*s))
{
    FcLangSet	*langset = FcLangSetCopy (a);
    FcStrSet	*set = FcLangSetGetLangs (b);
    FcStrList	*sl = FcStrListCreate (set);
    FcChar8	*str;

    FcStrSetDestroy (set);
    while ((str = FcStrListNext (sl)))
    {
	func (langset, str);
    }
    FcStrListDone (sl);

    return langset;
}

FcLangSet *
FcLangSetUnion (const FcLangSet *a, const FcLangSet *b)
{
    return FcLangSetOperate(a, b, FcLangSetAdd);
}

FcLangSet *
FcLangSetSubtract (const FcLangSet *a, const FcLangSet *b)
{
    return FcLangSetOperate(a, b, FcLangSetDel);
}

#define __fclang__
#include "fcaliastail.h"
#include "fcftaliastail.h"
#undef __fclang__
