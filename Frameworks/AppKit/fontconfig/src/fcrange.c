/*
 * fontconfig/src/fcrange.c
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


FcRange *
FcRangeCreateDouble (double begin, double end)
{
    FcRange *ret = malloc (sizeof (FcRange));

    if (ret)
    {
	ret->begin = begin;
	ret->end = end;
    }

    return ret;
}

FcRange *
FcRangeCreateInteger (FcChar32 begin, FcChar32 end)
{
    FcRange *ret = malloc (sizeof (FcRange));

    if (ret)
    {
	ret->begin = begin;
	ret->end = end;
    }

    return ret;
}

void
FcRangeDestroy (FcRange *range)
{
    if (range)
	free (range);
}

FcRange *
FcRangeCopy (const FcRange *range)
{
    return FcRangeCreateDouble (range->begin, range->end);
}

FcBool
FcRangeGetDouble(const FcRange *range, double *begin, double *end)
{
    if (!range)
	return FcFalse;
    if (begin)
	*begin = range->begin;
    if (end)
	*end = range->end;

    return FcTrue;
}

FcRange *
FcRangePromote (double v, FcValuePromotionBuffer *vbuf)
{
    typedef struct {
	FcRange	r;
    } FcRangePromotionBuffer;
    FcRangePromotionBuffer *buf = (FcRangePromotionBuffer *) vbuf;

    FC_ASSERT_STATIC (sizeof (FcRangePromotionBuffer) <= sizeof (FcValuePromotionBuffer));
    buf->r.begin = v;
    buf->r.end = v;

    return &buf->r;
}

FcBool
FcRangeIsInRange (const FcRange *a, const FcRange *b)
{
    return a->begin >= b->begin && a->end <= b->end;
}

FcBool
FcRangeCompare (FcOp op, const FcRange *a, const FcRange *b)
{
    switch ((int) op) {
    case FcOpEqual:
	return a->begin == b->begin && a->end == b->end;
    case FcOpContains:
    case FcOpListing:
	return FcRangeIsInRange (a, b);
    case FcOpNotEqual:
	return a->begin != b->begin || a->end != b->end;
    case FcOpNotContains:
	return !FcRangeIsInRange (a, b);
    case FcOpLess:
	return a->end < b->begin;
    case FcOpLessEqual:
	return a->end <= b->begin;
    case FcOpMore:
	return a->begin > b->end;
    case FcOpMoreEqual:
	return a->begin >= b->end;
    default:
	break;
    }
    return FcFalse;
}

FcChar32
FcRangeHash (const FcRange *r)
{
    int b = (int) (r->begin * 100);
    int e = (int) (r->end * 100);

    return b ^ (b << 1) ^ (e << 9);
}

FcBool
FcRangeSerializeAlloc (FcSerialize *serialize, const FcRange *r)
{
    if (!FcSerializeAlloc (serialize, r, sizeof (FcRange)))
	return FcFalse;
    return FcTrue;
}

FcRange *
FcRangeSerialize (FcSerialize *serialize, const FcRange *r)
{
    FcRange *r_serialize = FcSerializePtr (serialize, r);

    if (!r_serialize)
	return NULL;
    memcpy (r_serialize, r, sizeof (FcRange));

    return r_serialize;
}

#define __fcrange__
#include "fcaliastail.h"
#undef __fcrange__
