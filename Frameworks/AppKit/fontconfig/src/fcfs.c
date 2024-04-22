/*
 * fontconfig/src/fcfs.c
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

FcFontSet *
FcFontSetCreate (void)
{
    FcFontSet	*s;

    s = (FcFontSet *) malloc (sizeof (FcFontSet));
    if (!s)
	return 0;
    s->nfont = 0;
    s->sfont = 0;
    s->fonts = 0;
    return s;
}

void
FcFontSetDestroy (FcFontSet *s)
{
    if (s)
    {
	int	    i;

	for (i = 0; i < s->nfont; i++)
	    FcPatternDestroy (s->fonts[i]);
	if (s->fonts)
	    free (s->fonts);
	free (s);
    }
}

FcBool
FcFontSetAdd (FcFontSet *s, FcPattern *font)
{
    FcPattern	**f;
    int		sfont;

    if (s->nfont == s->sfont)
    {
	sfont = s->sfont + 32;
	if (s->fonts)
	    f = (FcPattern **) realloc (s->fonts, sfont * sizeof (FcPattern *));
	else
	    f = (FcPattern **) malloc (sfont * sizeof (FcPattern *));
	if (!f)
	    return FcFalse;
	s->sfont = sfont;
	s->fonts = f;
    }
    s->fonts[s->nfont++] = font;
    return FcTrue;
}

FcBool
FcFontSetSerializeAlloc (FcSerialize *serialize, const FcFontSet *s)
{
    int i;

    if (!FcSerializeAlloc (serialize, s, sizeof (FcFontSet)))
	return FcFalse;
    if (!FcSerializeAlloc (serialize, s->fonts, s->nfont * sizeof (FcPattern *)))
	return FcFalse;
    for (i = 0; i < s->nfont; i++)
    {
	if (!FcPatternSerializeAlloc (serialize, s->fonts[i]))
	    return FcFalse;
    }
    return FcTrue;
}

FcFontSet *
FcFontSetSerialize (FcSerialize *serialize, const FcFontSet * s)
{
    int		i;
    FcFontSet	*s_serialize;
    FcPattern	**fonts_serialize;
    FcPattern	*p_serialize;

    s_serialize = FcSerializePtr (serialize, s);
    if (!s_serialize)
	return NULL;
    *s_serialize = *s;
    s_serialize->sfont = s_serialize->nfont;

    fonts_serialize = FcSerializePtr (serialize, s->fonts);
    if (!fonts_serialize)
	return NULL;
    s_serialize->fonts = FcPtrToEncodedOffset (s_serialize,
					       fonts_serialize, FcPattern *);

    for (i = 0; i < s->nfont; i++)
    {
	p_serialize = FcPatternSerialize (serialize, s->fonts[i]);
	if (!p_serialize)
	    return NULL;
	fonts_serialize[i] = FcPtrToEncodedOffset (s_serialize,
						   p_serialize,
						   FcPattern);
    }

    return s_serialize;
}

FcFontSet *
FcFontSetDeserialize (const FcFontSet *set)
{
    int i;
    FcFontSet *new = FcFontSetCreate ();

    if (!new)
	return NULL;
    for (i = 0; i < set->nfont; i++)
    {
	if (!FcFontSetAdd (new, FcPatternDuplicate (FcFontSetFont (set, i))))
	    goto bail;
    }

    return new;
bail:
    FcFontSetDestroy (new);

    return NULL;
}

#define __fcfs__
#include "fcaliastail.h"
#undef __fcfs__
