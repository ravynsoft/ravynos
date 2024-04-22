/*
 * fontconfig/src/fclist.c
 *
 * Copyright © 2000 Keith Packard
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

FcObjectSet *
FcObjectSetCreate (void)
{
    FcObjectSet    *os;

    os = (FcObjectSet *) malloc (sizeof (FcObjectSet));
    if (!os)
	return 0;
    os->nobject = 0;
    os->sobject = 0;
    os->objects = 0;
    return os;
}

FcBool
FcObjectSetAdd (FcObjectSet *os, const char *object)
{
    int		s;
    const char	**objects;
    int		high, low, mid, c;

    if (os->nobject == os->sobject)
    {
	s = os->sobject + 4;
	if (os->objects)
	    objects = (const char **) realloc ((void *) os->objects,
					       s * sizeof (const char *));
	else
	    objects = (const char **) malloc (s * sizeof (const char *));
	if (!objects)
	    return FcFalse;
	os->objects = objects;
	os->sobject = s;
    }
    high = os->nobject - 1;
    low = 0;
    mid = 0;
    c = 1;
    object = strdup (object);
    while (low <= high)
    {
	mid = (low + high) >> 1;
	c = os->objects[mid] - object;
	if (c == 0)
	{
	    FcFree (object);
	    return FcTrue;
	}
	if (c < 0)
	    low = mid + 1;
	else
	    high = mid - 1;
    }
    if (c < 0)
	mid++;
    memmove (os->objects + mid + 1, os->objects + mid,
	     (os->nobject - mid) * sizeof (const char *));
    os->objects[mid] = object;
    os->nobject++;
    return FcTrue;
}

void
FcObjectSetDestroy (FcObjectSet *os)
{
    int i;

    if (os)
    {
	if (os->objects)
	{
	    for (i = 0; i < os->nobject; i++)
		FcFree (os->objects[i]);

	    free ((void *) os->objects);
	}
	free (os);
    }
}

FcObjectSet *
FcObjectSetVaBuild (const char *first, va_list va)
{
    FcObjectSet    *ret;

    FcObjectSetVapBuild (ret, first, va);
    return ret;
}

FcObjectSet *
FcObjectSetBuild (const char *first, ...)
{
    va_list	    va;
    FcObjectSet    *os;

    va_start (va, first);
    FcObjectSetVapBuild (os, first, va);
    va_end (va);
    return os;
}

/*
 * Font must have a containing value for every value in the pattern
 */
static FcBool
FcListValueListMatchAny (FcValueListPtr patOrig,	    /* pattern */
			 FcValueListPtr fntOrig)	    /* font */
{
    FcValueListPtr	 pat, fnt;

    for (pat = patOrig; pat != NULL; pat = FcValueListNext(pat))
    {
	for (fnt = fntOrig; fnt != NULL; fnt = FcValueListNext(fnt))
	{
	    /*
	     * make sure the font 'contains' the pattern.
	     * (OpListing is OpContains except for strings
	     *  where it requires an exact match)
	     */
	    if (FcConfigCompareValue (&fnt->value,
				      FC_OP (FcOpListing, FcOpFlagIgnoreBlanks),
				      &pat->value))
		break;
	}
	if (fnt == NULL)
	    return FcFalse;
    }
    return FcTrue;
}

static FcBool
FcListValueListEqual (FcValueListPtr v1orig,
		      FcValueListPtr v2orig)
{
    FcValueListPtr	    v1, v2;

    for (v1 = v1orig; v1 != NULL; v1 = FcValueListNext(v1))
    {
	for (v2 = v2orig; v2 != NULL; v2 = FcValueListNext(v2))
	    if (FcValueEqual (FcValueCanonicalize(&(v1)->value),
			      FcValueCanonicalize(&(v2)->value)))
		break;
	if (v2 == NULL)
	    return FcFalse;
    }
    for (v2 = v2orig; v2 != NULL; v2 = FcValueListNext(v2))
    {
	for (v1 = v1orig; v1 != NULL; v1 = FcValueListNext(v1))
	    if (FcValueEqual (FcValueCanonicalize(&v1->value),
			      FcValueCanonicalize(&v2->value)))
		break;
	if (v1 == NULL)
	    return FcFalse;
    }
    return FcTrue;
}

static FcBool
FcListPatternEqual (FcPattern	*p1,
		    FcPattern	*p2,
		    FcObjectSet	*os)
{
    int		    i;
    FcPatternElt    *e1, *e2;

    for (i = 0; i < os->nobject; i++)
    {
	e1 = FcPatternObjectFindElt (p1, FcObjectFromName (os->objects[i]));
	e2 = FcPatternObjectFindElt (p2, FcObjectFromName (os->objects[i]));
	if (!e1 && !e2)
	    continue;
	if (!e1 || !e2)
	    return FcFalse;
	if (!FcListValueListEqual (FcPatternEltValues(e1),
				   FcPatternEltValues(e2)))
	    return FcFalse;
    }
    return FcTrue;
}

/*
 * FcTrue iff all objects in "p" match "font"
 */

FcBool
FcListPatternMatchAny (const FcPattern *p,
		       const FcPattern *font)
{
    int		    i;

    if (!p)
	return FcFalse;
    for (i = 0; i < p->num; i++)
    {
	FcPatternElt	*pe = &FcPatternElts(p)[i];
	FcPatternElt	*fe;

	if (pe->object == FC_NAMELANG_OBJECT)
	{
	    /* "namelang" object is the alias object to change "familylang",
	     * "stylelang" and "fullnamelang" object all together. it won't be
	     * available on the font pattern. so checking its availability
	     * causes no results. we should ignore it here.
	     */
	    continue;
	}
	fe = FcPatternObjectFindElt (font, pe->object);
	if (!fe)
	    return FcFalse;
	if (!FcListValueListMatchAny (FcPatternEltValues(pe),    /* pat elts */
				      FcPatternEltValues(fe)))   /* font elts */
	    return FcFalse;
    }
    return FcTrue;
}

static FcChar32
FcListMatrixHash (const FcMatrix *m)
{
    int	    xx = (int) (m->xx * 100),
	    xy = (int) (m->xy * 100),
	    yx = (int) (m->yx * 100),
	    yy = (int) (m->yy * 100);

    return ((FcChar32) xx) ^ ((FcChar32) xy) ^ ((FcChar32) yx) ^ ((FcChar32) yy);
}

static FcChar32
FcListValueHash (FcValue    *value)
{
    FcValue v = FcValueCanonicalize(value);
    switch (v.type) {
    case FcTypeUnknown:
    case FcTypeVoid:
	return 0;
    case FcTypeInteger:
	return (FcChar32) v.u.i;
    case FcTypeDouble:
	return (FcChar32) (int) v.u.d;
    case FcTypeString:
	return FcStrHashIgnoreCase (v.u.s);
    case FcTypeBool:
	return (FcChar32) v.u.b;
    case FcTypeMatrix:
	return FcListMatrixHash (v.u.m);
    case FcTypeCharSet:
	return FcCharSetCount (v.u.c);
    case FcTypeFTFace:
	return (intptr_t) v.u.f;
    case FcTypeLangSet:
	return FcLangSetHash (v.u.l);
    case FcTypeRange:
	return FcRangeHash (v.u.r);
    }
    return 0;
}

static FcChar32
FcListValueListHash (FcValueListPtr list)
{
    FcChar32	h = 0;

    while (list != NULL)
    {
	h = h ^ FcListValueHash (&list->value);
	list = FcValueListNext(list);
    }
    return h;
}

static FcChar32
FcListPatternHash (FcPattern	*font,
		   FcObjectSet	*os)
{
    int		    n;
    FcPatternElt    *e;
    FcChar32	    h = 0;

    for (n = 0; n < os->nobject; n++)
    {
	e = FcPatternObjectFindElt (font, FcObjectFromName (os->objects[n]));
	if (e)
	    h = h ^ FcListValueListHash (FcPatternEltValues(e));
    }
    return h;
}

typedef struct _FcListBucket {
    struct _FcListBucket    *next;
    FcChar32		    hash;
    FcPattern		    *pattern;
} FcListBucket;

#define FC_LIST_HASH_SIZE   4099

typedef struct _FcListHashTable {
    int		    entries;
    FcListBucket    *buckets[FC_LIST_HASH_SIZE];
} FcListHashTable;

static void
FcListHashTableInit (FcListHashTable *table)
{
    table->entries = 0;
    memset (table->buckets, '\0', sizeof (table->buckets));
}

static void
FcListHashTableCleanup (FcListHashTable *table)
{
    int	i;
    FcListBucket    *bucket, *next;

    for (i = 0; i < FC_LIST_HASH_SIZE; i++)
    {
	for (bucket = table->buckets[i]; bucket; bucket = next)
	{
	    next = bucket->next;
	    FcPatternDestroy (bucket->pattern);
	    free (bucket);
	}
	table->buckets[i] = 0;
    }
    table->entries = 0;
}

static int
FcGetDefaultObjectLangIndex (FcPattern *font, FcObject object, const FcChar8 *lang)
{
    FcPatternElt   *e = FcPatternObjectFindElt (font, object);
    FcValueListPtr  v;
    FcValue         value;
    int             idx = -1;
    int             defidx = -1;
    int             i;

    if (e)
    {
	for (v = FcPatternEltValues(e), i = 0; v; v = FcValueListNext(v), ++i)
	{
	    value = FcValueCanonicalize (&v->value);

	    if (value.type == FcTypeString)
	    {
		FcLangResult res = FcLangCompare (value.u.s, lang);
		if (res == FcLangEqual)
		    return i;

		if (res == FcLangDifferentCountry && idx < 0)
		    idx = i;
		if (defidx < 0)
		{
		    /* workaround for fonts that has non-English value
		     * at the head of values.
		     */
		    res = FcLangCompare (value.u.s, (FcChar8 *)"en");
		    if (res == FcLangEqual)
			defidx = i;
		}
	    }
	}
    }

    return (idx > 0) ? idx : (defidx > 0) ? defidx : 0;
}

static FcBool
FcListAppend (FcListHashTable	*table,
	      FcPattern		*font,
	      FcObjectSet	*os,
	      const FcChar8	*lang)
{
    int		    o;
    FcPatternElt    *e;
    FcValueListPtr  v;
    FcChar32	    hash;
    FcListBucket    **prev, *bucket;
    int             familyidx = -1;
    int             fullnameidx = -1;
    int             styleidx = -1;
    int             defidx = 0;
    int             idx;

    hash = FcListPatternHash (font, os);
    for (prev = &table->buckets[hash % FC_LIST_HASH_SIZE];
	 (bucket = *prev); prev = &(bucket->next))
    {
	if (bucket->hash == hash &&
	    FcListPatternEqual (bucket->pattern, font, os))
	    return FcTrue;
    }
    bucket = (FcListBucket *) malloc (sizeof (FcListBucket));
    if (!bucket)
	goto bail0;
    bucket->next = 0;
    bucket->hash = hash;
    bucket->pattern = FcPatternCreate ();
    if (!bucket->pattern)
	goto bail1;

    for (o = 0; o < os->nobject; o++)
    {
	if (!strcmp (os->objects[o], FC_FAMILY) || !strcmp (os->objects[o], FC_FAMILYLANG))
	{
	    if (familyidx < 0)
		familyidx = FcGetDefaultObjectLangIndex (font, FC_FAMILYLANG_OBJECT, lang);
	    defidx = familyidx;
	}
	else if (!strcmp (os->objects[o], FC_FULLNAME) || !strcmp (os->objects[o], FC_FULLNAMELANG))
	{
	    if (fullnameidx < 0)
		fullnameidx = FcGetDefaultObjectLangIndex (font, FC_FULLNAMELANG_OBJECT, lang);
	    defidx = fullnameidx;
	}
	else if (!strcmp (os->objects[o], FC_STYLE) || !strcmp (os->objects[o], FC_STYLELANG))
	{
	    if (styleidx < 0)
		styleidx = FcGetDefaultObjectLangIndex (font, FC_STYLELANG_OBJECT, lang);
	    defidx = styleidx;
	}
	else
	    defidx = 0;

	e = FcPatternObjectFindElt (font, FcObjectFromName (os->objects[o]));
	if (e)
	{
	    for (v = FcPatternEltValues(e), idx = 0; v;
		 v = FcValueListNext(v), ++idx)
	    {
		if (!FcPatternAdd (bucket->pattern,
				   os->objects[o],
				   FcValueCanonicalize(&v->value), defidx != idx))
		    goto bail2;
	    }
	}
    }
    *prev = bucket;
    ++table->entries;

    return FcTrue;

bail2:
    FcPatternDestroy (bucket->pattern);
bail1:
    free (bucket);
bail0:
    return FcFalse;
}

FcFontSet *
FcFontSetList (FcConfig	    *config,
	       FcFontSet    **sets,
	       int	    nsets,
	       FcPattern    *p,
	       FcObjectSet  *os)
{
    FcFontSet	    *ret;
    FcFontSet	    *s;
    int		    f;
    int		    set;
    FcListHashTable table;
    int		    i;
    FcListBucket    *bucket;
    int             destroy_os = 0;

    if (!config)
    {
	if (!FcInitBringUptoDate ())
	    goto bail0;
    }
    config = FcConfigReference (config);
    if (!config)
	goto bail0;
    FcListHashTableInit (&table);

    if (!os)
    {
	os = FcObjectGetSet ();
	destroy_os = 1;
    }

    /*
     * Walk all available fonts adding those that
     * match to the hash table
     */
    for (set = 0; set < nsets; set++)
    {
	s = sets[set];
	if (!s)
	    continue;
	for (f = 0; f < s->nfont; f++)
	    if (FcListPatternMatchAny (p,		/* pattern */
				       s->fonts[f]))	/* font */
	    {
		FcChar8 *lang;

		if (FcPatternObjectGetString (p, FC_NAMELANG_OBJECT, 0, &lang) != FcResultMatch)
		{
			lang = FcGetDefaultLang ();
		}
		if (!FcListAppend (&table, s->fonts[f], os, lang))
		    goto bail1;
	    }
    }
#if 0
    {
	int	max = 0;
	int	full = 0;
	int	ents = 0;
	int	len;
	for (i = 0; i < FC_LIST_HASH_SIZE; i++)
	{
	    if ((bucket = table.buckets[i]))
	    {
		len = 0;
		for (; bucket; bucket = bucket->next)
		{
		    ents++;
		    len++;
		}
		if (len > max)
		    max = len;
		full++;
	    }
	}
	printf ("used: %d max: %d avg: %g\n", full, max,
		(double) ents / FC_LIST_HASH_SIZE);
    }
#endif
    /*
     * Walk the hash table and build
     * a font set
     */
    ret = FcFontSetCreate ();
    if (!ret)
	goto bail1;
    for (i = 0; i < FC_LIST_HASH_SIZE; i++)
	while ((bucket = table.buckets[i]))
	{
	    if (!FcFontSetAdd (ret, bucket->pattern))
		goto bail2;
	    table.buckets[i] = bucket->next;
	    free (bucket);
	}

    if (destroy_os)
        FcObjectSetDestroy (os);
    FcConfigDestroy (config);

    return ret;

bail2:
    FcFontSetDestroy (ret);
bail1:
    FcListHashTableCleanup (&table);
    FcConfigDestroy (config);
bail0:
    if (destroy_os)
	FcObjectSetDestroy (os);
    return 0;
}

FcFontSet *
FcFontList (FcConfig	*config,
	    FcPattern	*p,
	    FcObjectSet *os)
{
    FcFontSet	*sets[2], *ret;
    int		nsets;

    if (!config)
    {
	if (!FcInitBringUptoDate ())
	    return 0;
    }
    config = FcConfigReference (config);
    if (!config)
	return NULL;
    nsets = 0;
    if (config->fonts[FcSetSystem])
	sets[nsets++] = config->fonts[FcSetSystem];
    if (config->fonts[FcSetApplication])
	sets[nsets++] = config->fonts[FcSetApplication];
    ret = FcFontSetList (config, sets, nsets, p, os);
    FcConfigDestroy (config);

    return ret;
}
#define __fclist__
#include "fcaliastail.h"
#undef __fclist__
