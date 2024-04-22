/*
 * Copyright © 2002 Keith Packard
 * Copyright © 2010 Behdad Esfahbod
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
#include "fcarch.h"
#include <stdio.h>

FC_ASSERT_STATIC (1 == sizeof (char));
FC_ASSERT_STATIC (2 == sizeof (FcChar16));
FC_ASSERT_STATIC (4 == sizeof (int));
FC_ASSERT_STATIC (4 == sizeof (FcChar32));
FC_ASSERT_STATIC (4 == sizeof (FcObject));
FC_ASSERT_STATIC (4 == sizeof (FcValueBinding));
FC_ASSERT_STATIC (8 == sizeof (FcAlign));
FC_ASSERT_STATIC (0x20 == sizeof (FcCharLeaf));

FC_ASSERT_STATIC (SIZEOF_VOID_P == sizeof (intptr_t));
FC_ASSERT_STATIC (SIZEOF_VOID_P == sizeof (FcPatternEltPtr));
FC_ASSERT_STATIC (SIZEOF_VOID_P == sizeof (FcValueListPtr));
FC_ASSERT_STATIC (SIZEOF_VOID_P == sizeof (char *));
FC_ASSERT_STATIC (SIZEOF_VOID_P == sizeof (struct FcPatternElt *));
FC_ASSERT_STATIC (SIZEOF_VOID_P == sizeof (FcValueList *));
FC_ASSERT_STATIC (SIZEOF_VOID_P == sizeof (FcStrSet *));
FC_ASSERT_STATIC (SIZEOF_VOID_P == sizeof (FcCharLeaf **));
FC_ASSERT_STATIC (SIZEOF_VOID_P == sizeof (FcChar16 *));

FC_ASSERT_STATIC (0x08 + 1*FC_MAX(SIZEOF_VOID_P,ALIGNOF_DOUBLE) == sizeof (FcValue));
FC_ASSERT_STATIC (0x00 + 2*SIZEOF_VOID_P == sizeof (FcPatternElt));
FC_ASSERT_STATIC (0x08 + 2*SIZEOF_VOID_P == sizeof (FcPattern));
FC_ASSERT_STATIC (0x08 + 2*SIZEOF_VOID_P == sizeof (FcCharSet));
FC_ASSERT_STATIC (0x10 + 6*SIZEOF_VOID_P == sizeof (FcCache));


int
main (int argc FC_UNUSED, char **argv FC_UNUSED)
{
    printf ("%s\n", FC_ARCHITECTURE);
    return 0;
}
