/*
 * fontconfig/src/fcname.c
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
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static const FcObjectType FcObjects[] = {
#define FC_OBJECT(NAME, Type, Cmp) { FC_##NAME, Type },
#include "fcobjs.h"
#undef FC_OBJECT
};

#define NUM_OBJECT_TYPES ((int) (sizeof FcObjects / sizeof FcObjects[0]))

static const FcObjectType *
FcObjectFindById (FcObject object)
{
    if (1 <= object && object <= NUM_OBJECT_TYPES)
	return &FcObjects[object - 1];
    return FcObjectLookupOtherTypeById (object);
}

FcBool
FcNameRegisterObjectTypes (const FcObjectType *types, int ntypes)
{
    /* Deprecated. */
    return FcFalse;
}

FcBool
FcNameUnregisterObjectTypes (const FcObjectType *types, int ntypes)
{
    /* Deprecated. */
    return FcFalse;
}

const FcObjectType *
FcNameGetObjectType (const char *object)
{
    int id = FcObjectLookupBuiltinIdByName (object);

    if (!id)
	return FcObjectLookupOtherTypeByName (object);

    return &FcObjects[id - 1];
}

FcBool
FcObjectValidType (FcObject object, FcType type)
{
    const FcObjectType    *t = FcObjectFindById (object);

    if (t) {
	switch ((int) t->type) {
	case FcTypeUnknown:
	    return FcTrue;
	case FcTypeDouble:
	case FcTypeInteger:
	    if (type == FcTypeDouble || type == FcTypeInteger)
		return FcTrue;
	    break;
	case FcTypeLangSet:
	    if (type == FcTypeLangSet || type == FcTypeString)
		return FcTrue;
	    break;
	case FcTypeRange:
	    if (type == FcTypeRange ||
		type == FcTypeDouble ||
		type == FcTypeInteger)
		return FcTrue;
	    break;
	default:
	    if (type == t->type)
		return FcTrue;
	    break;
	}
	return FcFalse;
    }
    return FcTrue;
}

FcObject
FcObjectFromName (const char * name)
{
    return FcObjectLookupIdByName (name);
}

FcObjectSet *
FcObjectGetSet (void)
{
    int		i;
    FcObjectSet	*os = NULL;


    os = FcObjectSetCreate ();
    for (i = 0; i < NUM_OBJECT_TYPES; i++)
	FcObjectSetAdd (os, FcObjects[i].object);

    return os;
}

const char *
FcObjectName (FcObject object)
{
    const FcObjectType   *o = FcObjectFindById (object);

    if (o)
	return o->object;

    return FcObjectLookupOtherNameById (object);
}

static const FcConstant _FcBaseConstants[] = {
    { (FcChar8 *) "thin",	    "weight",   FC_WEIGHT_THIN, },
    { (FcChar8 *) "extralight",	    "weight",   FC_WEIGHT_EXTRALIGHT, },
    { (FcChar8 *) "ultralight",	    "weight",   FC_WEIGHT_EXTRALIGHT, },
    { (FcChar8 *) "demilight",	    "weight",   FC_WEIGHT_DEMILIGHT, },
    { (FcChar8 *) "semilight",	    "weight",   FC_WEIGHT_DEMILIGHT, },
    { (FcChar8 *) "light",	    "weight",   FC_WEIGHT_LIGHT, },
    { (FcChar8 *) "book",	    "weight",	FC_WEIGHT_BOOK, },
    { (FcChar8 *) "regular",	    "weight",   FC_WEIGHT_REGULAR, },
    { (FcChar8 *) "normal",	    "weight",	FC_WEIGHT_NORMAL, },
    { (FcChar8 *) "medium",	    "weight",   FC_WEIGHT_MEDIUM, },
    { (FcChar8 *) "demibold",	    "weight",   FC_WEIGHT_DEMIBOLD, },
    { (FcChar8 *) "semibold",	    "weight",   FC_WEIGHT_DEMIBOLD, },
    { (FcChar8 *) "bold",	    "weight",   FC_WEIGHT_BOLD, },
    { (FcChar8 *) "extrabold",	    "weight",   FC_WEIGHT_EXTRABOLD, },
    { (FcChar8 *) "ultrabold",	    "weight",   FC_WEIGHT_EXTRABOLD, },
    { (FcChar8 *) "black",	    "weight",   FC_WEIGHT_BLACK, },
    { (FcChar8 *) "heavy",	    "weight",	FC_WEIGHT_HEAVY, },
    { (FcChar8 *) "extrablack",     "weight",	FC_WEIGHT_EXTRABLACK, },
    { (FcChar8 *) "ultrablack",     "weight",	FC_WEIGHT_ULTRABLACK, },

    { (FcChar8 *) "roman",	    "slant",    FC_SLANT_ROMAN, },
    { (FcChar8 *) "italic",	    "slant",    FC_SLANT_ITALIC, },
    { (FcChar8 *) "oblique",	    "slant",    FC_SLANT_OBLIQUE, },

    { (FcChar8 *) "ultracondensed", "width",	FC_WIDTH_ULTRACONDENSED },
    { (FcChar8 *) "extracondensed", "width",	FC_WIDTH_EXTRACONDENSED },
    { (FcChar8 *) "condensed",	    "width",	FC_WIDTH_CONDENSED },
    { (FcChar8 *) "semicondensed",  "width",	FC_WIDTH_SEMICONDENSED },
    { (FcChar8 *) "normal",	    "width",	FC_WIDTH_NORMAL },
    { (FcChar8 *) "semiexpanded",   "width",	FC_WIDTH_SEMIEXPANDED },
    { (FcChar8 *) "expanded",	    "width",	FC_WIDTH_EXPANDED },
    { (FcChar8 *) "extraexpanded",  "width",	FC_WIDTH_EXTRAEXPANDED },
    { (FcChar8 *) "ultraexpanded",  "width",	FC_WIDTH_ULTRAEXPANDED },

    { (FcChar8 *) "proportional",   "spacing",  FC_PROPORTIONAL, },
    { (FcChar8 *) "dual",	    "spacing",  FC_DUAL, },
    { (FcChar8 *) "mono",	    "spacing",  FC_MONO, },
    { (FcChar8 *) "charcell",	    "spacing",  FC_CHARCELL, },

    { (FcChar8 *) "unknown",	    "rgba",	    FC_RGBA_UNKNOWN },
    { (FcChar8 *) "rgb",	    "rgba",	    FC_RGBA_RGB, },
    { (FcChar8 *) "bgr",	    "rgba",	    FC_RGBA_BGR, },
    { (FcChar8 *) "vrgb",	    "rgba",	    FC_RGBA_VRGB },
    { (FcChar8 *) "vbgr",	    "rgba",	    FC_RGBA_VBGR },
    { (FcChar8 *) "none",	    "rgba",	    FC_RGBA_NONE },

    { (FcChar8 *) "hintnone",	    "hintstyle",   FC_HINT_NONE },
    { (FcChar8 *) "hintslight",	    "hintstyle",   FC_HINT_SLIGHT },
    { (FcChar8 *) "hintmedium",	    "hintstyle",   FC_HINT_MEDIUM },
    { (FcChar8 *) "hintfull",	    "hintstyle",   FC_HINT_FULL },

    { (FcChar8 *) "antialias",	    "antialias",    FcTrue },
    { (FcChar8 *) "hinting",	    "hinting",	    FcTrue },
    { (FcChar8 *) "verticallayout", "verticallayout",	FcTrue },
    { (FcChar8 *) "autohint",	    "autohint",	    FcTrue },
    { (FcChar8 *) "globaladvance",  "globaladvance",	FcTrue }, /* deprecated */
    { (FcChar8 *) "outline",	    "outline",	    FcTrue },
    { (FcChar8 *) "scalable",	    "scalable",	    FcTrue },
    { (FcChar8 *) "minspace",	    "minspace",	    FcTrue },
    { (FcChar8 *) "embolden",	    "embolden",	    FcTrue },
    { (FcChar8 *) "embeddedbitmap", "embeddedbitmap",	FcTrue },
    { (FcChar8 *) "decorative",	    "decorative",   FcTrue },
    { (FcChar8 *) "lcdnone",	    "lcdfilter",    FC_LCD_NONE },
    { (FcChar8 *) "lcddefault",	    "lcdfilter",    FC_LCD_DEFAULT },
    { (FcChar8 *) "lcdlight",	    "lcdfilter",    FC_LCD_LIGHT },
    { (FcChar8 *) "lcdlegacy",	    "lcdfilter",    FC_LCD_LEGACY },
};

#define NUM_FC_CONSTANTS   (sizeof _FcBaseConstants/sizeof _FcBaseConstants[0])

FcBool
FcNameRegisterConstants (const FcConstant *consts, int nconsts)
{
    /* Deprecated. */
    return FcFalse;
}

FcBool
FcNameUnregisterConstants (const FcConstant *consts, int nconsts)
{
    /* Deprecated. */
    return FcFalse;
}

const FcConstant *
FcNameGetConstant (const FcChar8 *string)
{
    unsigned int	    i;

    for (i = 0; i < NUM_FC_CONSTANTS; i++)
	if (!FcStrCmpIgnoreCase (string, _FcBaseConstants[i].name))
	    return &_FcBaseConstants[i];

    return 0;
}

const FcConstant *
FcNameGetConstantFor (const FcChar8 *string, const char *object)
{
    unsigned int	    i;

    for (i = 0; i < NUM_FC_CONSTANTS; i++)
	if (!FcStrCmpIgnoreCase (string, _FcBaseConstants[i].name) &&
	    !FcStrCmpIgnoreCase ((const FcChar8 *)object, (const FcChar8 *)_FcBaseConstants[i].object))
	    return &_FcBaseConstants[i];

    return 0;
}

FcBool
FcNameConstant (const FcChar8 *string, int *result)
{
    const FcConstant	*c;

    if ((c = FcNameGetConstant(string)))
    {
	*result = c->value;
	return FcTrue;
    }
    return FcFalse;
}

FcBool
FcNameConstantWithObjectCheck (const FcChar8 *string, const char *object, int *result)
{
    const FcConstant	*c;

    if ((c = FcNameGetConstantFor(string, object)))
    {
	*result = c->value;
	return FcTrue;
    }
    else if ((c = FcNameGetConstant(string)))
    {
	if (strcmp (c->object, object) != 0)
	{
	    fprintf (stderr, "Fontconfig error: Unexpected constant name `%s' used for object `%s': should be `%s'\n", string, object, c->object);
	    return FcFalse;
	}
	/* Unlikely to reach out */
	*result = c->value;
	return FcTrue;
    }
    return FcFalse;
}

FcBool
FcNameBool (const FcChar8 *v, FcBool *result)
{
    char    c0, c1;

    c0 = *v;
    c0 = FcToLower (c0);
    if (c0 == 't' || c0 == 'y' || c0 == '1')
    {
	*result = FcTrue;
	return FcTrue;
    }
    if (c0 == 'f' || c0 == 'n' || c0 == '0')
    {
	*result = FcFalse;
	return FcTrue;
    }
    if (c0 == 'd' || c0 == 'x' || c0 == '2')
    {
	*result = FcDontCare;
	return FcTrue;
    }
    if (c0 == 'o')
    {
	c1 = v[1];
	c1 = FcToLower (c1);
	if (c1 == 'n')
	{
	    *result = FcTrue;
	    return FcTrue;
	}
	if (c1 == 'f')
	{
	    *result = FcFalse;
	    return FcTrue;
	}
	if (c1 == 'r')
	{
	    *result = FcDontCare;
	    return FcTrue;
	}
    }
    return FcFalse;
}

static FcValue
FcNameConvert (FcType type, const char *object, FcChar8 *string)
{
    FcValue	v;
    FcMatrix	m;
    double	b, e;
    char	*p;

    v.type = type;
    switch ((int) v.type) {
    case FcTypeInteger:
	if (!FcNameConstantWithObjectCheck (string, object, &v.u.i))
	    v.u.i = atoi ((char *) string);
	break;
    case FcTypeString:
	v.u.s = FcStrdup (string);
	if (!v.u.s)
	    v.type = FcTypeVoid;
	break;
    case FcTypeBool:
	if (!FcNameBool (string, &v.u.b))
	    v.u.b = FcFalse;
	break;
    case FcTypeDouble:
	v.u.d = strtod ((char *) string, 0);
	break;
    case FcTypeMatrix:
	FcMatrixInit (&m);
	sscanf ((char *) string, "%lg %lg %lg %lg", &m.xx, &m.xy, &m.yx, &m.yy);
	v.u.m = FcMatrixCopy (&m);
	break;
    case FcTypeCharSet:
	v.u.c = FcNameParseCharSet (string);
	if (!v.u.c)
	    v.type = FcTypeVoid;
	break;
    case FcTypeLangSet:
	v.u.l = FcNameParseLangSet (string);
	if (!v.u.l)
	    v.type = FcTypeVoid;
	break;
    case FcTypeRange:
	if (sscanf ((char *) string, "[%lg %lg]", &b, &e) != 2)
	{
	    char *sc, *ec;
	    size_t len = strlen ((const char *) string);
	    int si, ei;

	    sc = malloc (len + 1);
	    ec = malloc (len + 1);
	    if (sc && ec && sscanf ((char *) string, "[%s %[^]]]", sc, ec) == 2)
	    {
		if (FcNameConstantWithObjectCheck ((const FcChar8 *) sc, object, &si) &&
		    FcNameConstantWithObjectCheck ((const FcChar8 *) ec, object, &ei))
		    v.u.r =  FcRangeCreateDouble (si, ei);
		else
		    goto bail1;
	    }
	    else
	    {
	    bail1:
		v.type = FcTypeDouble;
		if (FcNameConstantWithObjectCheck (string, object, &si))
		{
		    v.u.d = (double) si;
		} else {
		    v.u.d = strtod ((char *) string, &p);
		    if (p != NULL && p[0] != 0)
			v.type = FcTypeVoid;
		}
	    }
	    if (sc)
		free (sc);
	    if (ec)
		free (ec);
	}
	else
	    v.u.r = FcRangeCreateDouble (b, e);
	break;
    default:
	break;
    }
    return v;
}

static const FcChar8 *
FcNameFindNext (const FcChar8 *cur, const char *delim, FcChar8 *save, FcChar8 *last)
{
    FcChar8    c;

    while ((c = *cur))
    {
	if (!isspace (c))
	    break;
	++cur;
    }
    while ((c = *cur))
    {
	if (c == '\\')
	{
	    ++cur;
	    if (!(c = *cur))
		break;
	}
	else if (strchr (delim, c))
	    break;
	++cur;
	*save++ = c;
    }
    *save = 0;
    *last = *cur;
    if (*cur)
	cur++;
    return cur;
}

FcPattern *
FcNameParse (const FcChar8 *name)
{
    FcChar8		*save;
    FcPattern		*pat;
    double		d;
    FcChar8		*e;
    FcChar8		delim;
    FcValue		v;
    const FcObjectType	*t;
    const FcConstant	*c;

    /* freed below */
    save = malloc (strlen ((char *) name) + 1);
    if (!save)
	goto bail0;
    pat = FcPatternCreate ();
    if (!pat)
	goto bail1;

    for (;;)
    {
	name = FcNameFindNext (name, "-,:", save, &delim);
	if (save[0])
	{
	    if (!FcPatternObjectAddString (pat, FC_FAMILY_OBJECT, save))
		goto bail2;
	}
	if (delim != ',')
	    break;
    }
    if (delim == '-')
    {
	for (;;)
	{
	    name = FcNameFindNext (name, "-,:", save, &delim);
	    d = strtod ((char *) save, (char **) &e);
	    if (e != save)
	    {
		if (!FcPatternObjectAddDouble (pat, FC_SIZE_OBJECT, d))
		    goto bail2;
	    }
	    if (delim != ',')
		break;
	}
    }
    while (delim == ':')
    {
	name = FcNameFindNext (name, "=_:", save, &delim);
	if (save[0])
	{
	    if (delim == '=' || delim == '_')
	    {
		t = FcNameGetObjectType ((char *) save);
		for (;;)
		{
		    name = FcNameFindNext (name, ":,", save, &delim);
		    if (t)
		    {
			v = FcNameConvert (t->type, t->object, save);
			if (!FcPatternAdd (pat, t->object, v, FcTrue))
			{
			    FcValueDestroy (v);
			    goto bail2;
			}
			FcValueDestroy (v);
		    }
		    if (delim != ',')
			break;
		}
	    }
	    else
	    {
		if ((c = FcNameGetConstant (save)))
		{
		    t = FcNameGetObjectType ((char *) c->object);
		    if (t == NULL)
			goto bail2;
		    switch ((int) t->type) {
		    case FcTypeInteger:
		    case FcTypeDouble:
			if (!FcPatternAddInteger (pat, c->object, c->value))
			    goto bail2;
			break;
		    case FcTypeBool:
			if (!FcPatternAddBool (pat, c->object, c->value))
			    goto bail2;
			break;
		    case FcTypeRange:
			if (!FcPatternAddInteger (pat, c->object, c->value))
			    goto bail2;
			break;
		    default:
			break;
		    }
		}
	    }
	}
    }

    free (save);
    return pat;

bail2:
    FcPatternDestroy (pat);
bail1:
    free (save);
bail0:
    return 0;
}
static FcBool
FcNameUnparseString (FcStrBuf	    *buf,
		     const FcChar8  *string,
		     const FcChar8  *escape)
{
    FcChar8 c;
    while ((c = *string++))
    {
	if (escape && strchr ((char *) escape, (char) c))
	{
	    if (!FcStrBufChar (buf, escape[0]))
		return FcFalse;
	}
	if (!FcStrBufChar (buf, c))
	    return FcFalse;
    }
    return FcTrue;
}

FcBool
FcNameUnparseValue (FcStrBuf	*buf,
		    FcValue	*v0,
		    FcChar8	*escape)
{
    FcChar8	temp[1024];
    FcValue v = FcValueCanonicalize(v0);

    switch (v.type) {
    case FcTypeUnknown:
    case FcTypeVoid:
	return FcTrue;
    case FcTypeInteger:
	sprintf ((char *) temp, "%d", v.u.i);
	return FcNameUnparseString (buf, temp, 0);
    case FcTypeDouble:
	sprintf ((char *) temp, "%g", v.u.d);
	return FcNameUnparseString (buf, temp, 0);
    case FcTypeString:
	return FcNameUnparseString (buf, v.u.s, escape);
    case FcTypeBool:
	return FcNameUnparseString (buf,
				    v.u.b == FcTrue  ? (FcChar8 *) "True" :
				    v.u.b == FcFalse ? (FcChar8 *) "False" :
				                       (FcChar8 *) "DontCare", 0);
    case FcTypeMatrix:
	sprintf ((char *) temp, "%g %g %g %g",
		 v.u.m->xx, v.u.m->xy, v.u.m->yx, v.u.m->yy);
	return FcNameUnparseString (buf, temp, 0);
    case FcTypeCharSet:
	return FcNameUnparseCharSet (buf, v.u.c);
    case FcTypeLangSet:
	return FcNameUnparseLangSet (buf, v.u.l);
    case FcTypeFTFace:
	return FcTrue;
    case FcTypeRange:
	sprintf ((char *) temp, "[%g %g]", v.u.r->begin, v.u.r->end);
	return FcNameUnparseString (buf, temp, 0);
    }
    return FcFalse;
}

FcBool
FcNameUnparseValueList (FcStrBuf	*buf,
			FcValueListPtr	v,
			FcChar8		*escape)
{
    while (v)
    {
	if (!FcNameUnparseValue (buf, &v->value, escape))
	    return FcFalse;
	if ((v = FcValueListNext(v)) != NULL)
	    if (!FcNameUnparseString (buf, (FcChar8 *) ",", 0))
		return FcFalse;
    }
    return FcTrue;
}

#define FC_ESCAPE_FIXED    "\\-:,"
#define FC_ESCAPE_VARIABLE "\\=_:,"

FcChar8 *
FcNameUnparse (FcPattern *pat)
{
    return FcNameUnparseEscaped (pat, FcTrue);
}

FcChar8 *
FcNameUnparseEscaped (FcPattern *pat, FcBool escape)
{
    FcStrBuf		    buf, buf2;
    FcChar8		    buf_static[8192], buf2_static[256];
    int			    i;
    FcPatternElt	    *e;

    FcStrBufInit (&buf, buf_static, sizeof (buf_static));
    FcStrBufInit (&buf2, buf2_static, sizeof (buf2_static));
    e = FcPatternObjectFindElt (pat, FC_FAMILY_OBJECT);
    if (e)
    {
        if (!FcNameUnparseValueList (&buf, FcPatternEltValues(e), escape ? (FcChar8 *) FC_ESCAPE_FIXED : 0))
	    goto bail0;
    }
    e = FcPatternObjectFindElt (pat, FC_SIZE_OBJECT);
    if (e)
    {
	FcChar8 *p;

	if (!FcNameUnparseString (&buf2, (FcChar8 *) "-", 0))
	    goto bail0;
	if (!FcNameUnparseValueList (&buf2, FcPatternEltValues(e), escape ? (FcChar8 *) FC_ESCAPE_FIXED : 0))
	    goto bail0;
	p = FcStrBufDoneStatic (&buf2);
	FcStrBufDestroy (&buf2);
	if (strlen ((const char *)p) > 1)
	    if (!FcStrBufString (&buf, p))
		goto bail0;
    }
    for (i = 0; i < NUM_OBJECT_TYPES; i++)
    {
	FcObject id = i + 1;
	const FcObjectType	    *o;
	o = &FcObjects[i];
	if (!strcmp (o->object, FC_FAMILY) ||
	    !strcmp (o->object, FC_SIZE))
	    continue;

	e = FcPatternObjectFindElt (pat, id);
	if (e)
	{
	    if (!FcNameUnparseString (&buf, (FcChar8 *) ":", 0))
		goto bail0;
	    if (!FcNameUnparseString (&buf, (FcChar8 *) o->object, escape ? (FcChar8 *) FC_ESCAPE_VARIABLE : 0))
		goto bail0;
	    if (!FcNameUnparseString (&buf, (FcChar8 *) "=", 0))
		goto bail0;
	    if (!FcNameUnparseValueList (&buf, FcPatternEltValues(e), escape ?
					 (FcChar8 *) FC_ESCAPE_VARIABLE : 0))
		goto bail0;
	}
    }
    return FcStrBufDone (&buf);
bail0:
    FcStrBufDestroy (&buf);
    return 0;
}
#define __fcname__
#include "fcaliastail.h"
#undef __fcname__
