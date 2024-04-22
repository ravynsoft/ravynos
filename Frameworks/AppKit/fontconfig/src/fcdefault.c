/*
 * fontconfig/src/fcdefault.c
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
#include <limits.h>
#include <string.h>

/* MT-safe */

static const struct {
    FcObject	field;
    FcBool	value;
} FcBoolDefaults[] = {
    { FC_HINTING_OBJECT,	   FcTrue	},  /* !FT_LOAD_NO_HINTING */
    { FC_VERTICAL_LAYOUT_OBJECT,   FcFalse	},  /* FC_LOAD_VERTICAL_LAYOUT */
    { FC_AUTOHINT_OBJECT,	   FcFalse	},  /* FC_LOAD_FORCE_AUTOHINT */
    { FC_GLOBAL_ADVANCE_OBJECT,    FcTrue	},  /* !FC_LOAD_IGNORE_GLOBAL_ADVANCE_WIDTH */
    { FC_EMBEDDED_BITMAP_OBJECT,   FcTrue 	},  /* !FC_LOAD_NO_BITMAP */
    { FC_DECORATIVE_OBJECT,	   FcFalse	},
    { FC_SYMBOL_OBJECT,		   FcFalse	},
    { FC_VARIABLE_OBJECT,	   FcFalse	},
};

#define NUM_FC_BOOL_DEFAULTS	(int) (sizeof FcBoolDefaults / sizeof FcBoolDefaults[0])

FcStrSet *default_langs;

FcStrSet *
FcGetDefaultLangs (void)
{
    FcStrSet *result;
retry:
    result = (FcStrSet *) fc_atomic_ptr_get (&default_langs);
    if (!result)
    {
	char *langs;

	result = FcStrSetCreate ();

	langs = getenv ("FC_LANG");
	if (!langs || !langs[0])
	    langs = getenv ("LC_ALL");
	if (!langs || !langs[0])
        {
            langs = getenv ("LC_CTYPE");
            // On some macOS systems, LC_CTYPE is set to "UTF-8", which doesn't
            // give any languge information. In this case, ignore LC_CTYPE and
            // continue the search with LANG.
            if (langs && (FcStrCmpIgnoreCase((const FcChar8 *) langs,
                                             (const FcChar8 *)"UTF-8") == 0))
            {
                langs = NULL;
            }
        }
	if (!langs || !langs[0])
	    langs = getenv ("LANG");
	if (langs && langs[0])
	{
	    if (!FcStrSetAddLangs (result, langs))
		FcStrSetAdd (result, (const FcChar8 *) "en");
	}
	else
	    FcStrSetAdd (result, (const FcChar8 *) "en");

	FcRefSetConst (&result->ref);
	if (!fc_atomic_ptr_cmpexch (&default_langs, NULL, result)) {
	    FcRefInit (&result->ref, 1);
	    FcStrSetDestroy (result);
	    goto retry;
	}
    }

    return result;
}

static FcChar8 *default_lang; /* MT-safe */

FcChar8 *
FcGetDefaultLang (void)
{
    FcChar8 *lang;
retry:
    lang = fc_atomic_ptr_get (&default_lang);
    if (!lang)
    {
	FcStrSet *langs = FcGetDefaultLangs ();
	lang = FcStrdup (langs->strs[0]);

	if (!fc_atomic_ptr_cmpexch (&default_lang, NULL, lang)) {
	    free (lang);
	    goto retry;
	}
    }

    return lang;
}

static FcChar8 *default_prgname;

FcChar8 *
FcGetPrgname (void)
{
    FcChar8 *prgname;
retry:
    prgname = fc_atomic_ptr_get (&default_prgname);
    if (!prgname)
    {
#ifdef _WIN32
	char buf[MAX_PATH+1];

	/* TODO This is ASCII-only; fix it. */
	if (GetModuleFileNameA (GetModuleHandle (NULL), buf, sizeof (buf) / sizeof (buf[0])) > 0)
	{
	    char *p;
	    unsigned int len;

	    p = strrchr (buf, '\\');
	    if (p)
		p++;
	    else
		p = buf;

	    len = strlen (p);

	    if (len > 4 && 0 == strcmp (p + len - 4, ".exe"))
	    {
		len -= 4;
		buf[len] = '\0';
	    }

	    prgname = FcStrdup (p);
	}
#elif defined (HAVE_GETPROGNAME)
	const char *q = getprogname ();
	if (q)
	    prgname = FcStrdup (q);
	else
	    prgname = FcStrdup ("");
#else
# if defined (HAVE_GETEXECNAME)
	char *p = FcStrdup(getexecname ());
# elif defined (HAVE_READLINK)
	size_t size = FC_PATH_MAX;
	char *p = NULL;

	while (1)
	{
	    char *buf = malloc (size);
	    ssize_t len;

	    if (!buf)
		break;

	    len = readlink ("/proc/self/exe", buf, size - 1);
	    if (len < 0)
	    {
		free (buf);
		break;
	    }
	    if (len < size - 1)
	    {
		buf[len] = 0;
		p = buf;
		break;
	    }

	    free (buf);
	    size *= 2;
	}
# else
	char *p = NULL;
# endif
	if (p)
	{
	    char *r = strrchr (p, '/');
	    if (r)
		r++;
	    else
		r = p;

	    prgname = FcStrdup (r);
	}

	if (!prgname)
	    prgname = FcStrdup ("");

	if (p)
	    free (p);
#endif

	if (!fc_atomic_ptr_cmpexch (&default_prgname, NULL, prgname)) {
	    free (prgname);
	    goto retry;
	}
    }

    if (prgname && !prgname[0])
	return NULL;

    return prgname;
}

static FcChar8 *default_desktop_name;

FcChar8 *
FcGetDesktopName (void)
{
    FcChar8 *desktop_name;
retry:
    desktop_name = fc_atomic_ptr_get (&default_desktop_name);
    if (!desktop_name)
    {
	char *s = getenv ("XDG_CURRENT_DESKTOP");

	if (!s)
	    desktop_name = FcStrdup ("");
	else
	    desktop_name = FcStrdup (s);
	if (!desktop_name)
	{
	    fprintf (stderr, "Fontconfig error: out of memory in %s\n",
		     __FUNCTION__);
	    return NULL;
	}

	if (!fc_atomic_ptr_cmpexch(&default_desktop_name, NULL, desktop_name))
	{
	    free (desktop_name);
	    goto retry;
	}
    }
    if (desktop_name && !desktop_name[0])
	return NULL;

    return desktop_name;
}

void
FcDefaultFini (void)
{
    FcChar8  *lang;
    FcStrSet *langs;
    FcChar8  *prgname;
    FcChar8  *desktop;

    lang = fc_atomic_ptr_get (&default_lang);
    if (lang && fc_atomic_ptr_cmpexch (&default_lang, lang, NULL))
    {
	free (lang);
    }

    langs = fc_atomic_ptr_get (&default_langs);
    if (langs && fc_atomic_ptr_cmpexch (&default_langs, langs, NULL))
    {
	FcRefInit (&langs->ref, 1);
	FcStrSetDestroy (langs);
    }

    prgname = fc_atomic_ptr_get (&default_prgname);
    if (prgname && fc_atomic_ptr_cmpexch (&default_prgname, prgname, NULL))
    {
	free (prgname);
    }

    desktop = fc_atomic_ptr_get (&default_desktop_name);
    if (desktop && fc_atomic_ptr_cmpexch(&default_desktop_name, desktop, NULL))
    {
	free (desktop);
    }
}

void
FcDefaultSubstitute (FcPattern *pattern)
{
    FcPatternIter iter;
    FcValue v, namelang, v2;
    int	    i;
    double	dpi, size, scale, pixelsize;

    if (!FcPatternFindObjectIter (pattern, &iter, FC_WEIGHT_OBJECT))
	FcPatternObjectAddInteger (pattern, FC_WEIGHT_OBJECT, FC_WEIGHT_NORMAL);

    if (!FcPatternFindObjectIter (pattern, &iter, FC_SLANT_OBJECT))
	FcPatternObjectAddInteger (pattern, FC_SLANT_OBJECT, FC_SLANT_ROMAN);

    if (!FcPatternFindObjectIter (pattern, &iter, FC_WIDTH_OBJECT))
	FcPatternObjectAddInteger (pattern, FC_WIDTH_OBJECT, FC_WIDTH_NORMAL);

    for (i = 0; i < NUM_FC_BOOL_DEFAULTS; i++)
	if (!FcPatternFindObjectIter (pattern, &iter, FcBoolDefaults[i].field))
	    FcPatternObjectAddBool (pattern, FcBoolDefaults[i].field, FcBoolDefaults[i].value);

    if (FcPatternObjectGetDouble (pattern, FC_SIZE_OBJECT, 0, &size) != FcResultMatch)
    {
	FcRange *r;
	double b, e;
	if (FcPatternObjectGetRange (pattern, FC_SIZE_OBJECT, 0, &r) == FcResultMatch && FcRangeGetDouble (r, &b, &e))
	    size = (b + e) * .5;
	else
	    size = 12.0L;
    }
    if (FcPatternObjectGetDouble (pattern, FC_SCALE_OBJECT, 0, &scale) != FcResultMatch)
	scale = 1.0;
    if (FcPatternObjectGetDouble (pattern, FC_DPI_OBJECT, 0, &dpi) != FcResultMatch)
	dpi = 75.0;

    if (!FcPatternFindObjectIter (pattern, &iter, FC_PIXEL_SIZE_OBJECT))
    {
	(void) FcPatternObjectDel (pattern, FC_SCALE_OBJECT);
	FcPatternObjectAddDouble (pattern, FC_SCALE_OBJECT, scale);
	pixelsize = size * scale;
	(void) FcPatternObjectDel (pattern, FC_DPI_OBJECT);
	FcPatternObjectAddDouble (pattern, FC_DPI_OBJECT, dpi);
	pixelsize *= dpi / 72.0;
	FcPatternObjectAddDouble (pattern, FC_PIXEL_SIZE_OBJECT, pixelsize);
    }
    else
    {
	FcPatternIterGetValue(pattern, &iter, 0, &v, NULL);
	size = v.u.d;
	size = size / dpi * 72.0 / scale;
    }
    (void) FcPatternObjectDel (pattern, FC_SIZE_OBJECT);
    FcPatternObjectAddDouble (pattern, FC_SIZE_OBJECT, size);

    if (!FcPatternFindObjectIter (pattern, &iter, FC_FONTVERSION_OBJECT))
	FcPatternObjectAddInteger (pattern, FC_FONTVERSION_OBJECT, 0x7fffffff);

    if (!FcPatternFindObjectIter (pattern, &iter, FC_HINT_STYLE_OBJECT))
	FcPatternObjectAddInteger (pattern, FC_HINT_STYLE_OBJECT, FC_HINT_FULL);

    if (!FcPatternFindObjectIter (pattern, &iter, FC_NAMELANG_OBJECT))
	FcPatternObjectAddString (pattern, FC_NAMELANG_OBJECT, FcGetDefaultLang ());

    /* shouldn't be failed. */
    FcPatternObjectGet (pattern, FC_NAMELANG_OBJECT, 0, &namelang);
    /* Add a fallback to ensure the english name when the requested language
     * isn't available. this would helps for the fonts that have non-English
     * name at the beginning.
     */
    /* Set "en-us" instead of "en" to avoid giving higher score to "en".
     * This is a hack for the case that the orth is not like ll-cc, because,
     * if no namelang isn't explicitly set, it will has something like ll-cc
     * according to current locale. which may causes FcLangDifferentTerritory
     * at FcLangCompare(). thus, the English name is selected so that
     * exact matched "en" has higher score than ll-cc.
     */
    v2.type = FcTypeString;
    v2.u.s = (FcChar8 *) "en-us";
    if (!FcPatternFindObjectIter (pattern, &iter, FC_FAMILYLANG_OBJECT))
    {
	FcPatternObjectAdd (pattern, FC_FAMILYLANG_OBJECT, namelang, FcTrue);
	FcPatternObjectAddWithBinding (pattern, FC_FAMILYLANG_OBJECT, v2, FcValueBindingWeak, FcTrue);
    }
    if (!FcPatternFindObjectIter (pattern, &iter, FC_STYLELANG_OBJECT))
    {
	FcPatternObjectAdd (pattern, FC_STYLELANG_OBJECT, namelang, FcTrue);
	FcPatternObjectAddWithBinding (pattern, FC_STYLELANG_OBJECT, v2, FcValueBindingWeak, FcTrue);
    }
    if (!FcPatternFindObjectIter (pattern, &iter, FC_FULLNAMELANG_OBJECT))
    {
	FcPatternObjectAdd (pattern, FC_FULLNAMELANG_OBJECT, namelang, FcTrue);
	FcPatternObjectAddWithBinding (pattern, FC_FULLNAMELANG_OBJECT, v2, FcValueBindingWeak, FcTrue);
    }

    if (FcPatternObjectGet (pattern, FC_PRGNAME_OBJECT, 0, &v) == FcResultNoMatch)
    {
	FcChar8 *prgname = FcGetPrgname ();
	if (prgname)
	    FcPatternObjectAddString (pattern, FC_PRGNAME_OBJECT, prgname);
    }

    if (FcPatternObjectGet (pattern, FC_DESKTOP_NAME_OBJECT, 0, &v) == FcResultNoMatch)
    {
	FcChar8 *desktop = FcGetDesktopName ();
	if (desktop)
	    FcPatternObjectAddString (pattern, FC_DESKTOP_NAME_OBJECT, desktop);
    }

    if (!FcPatternFindObjectIter (pattern, &iter, FC_ORDER_OBJECT))
	FcPatternObjectAddInteger (pattern, FC_ORDER_OBJECT, 0);
}
#define __fcdefault__
#include "fcaliastail.h"
#undef __fcdefault__
