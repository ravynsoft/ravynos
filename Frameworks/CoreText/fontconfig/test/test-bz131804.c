/*
 * fontconfig/test/test-bz89617.c
 *
 * Copyright © 2000 Keith Packard
 * Copyright © 2015 Akira TAGOH
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
#include <stdio.h>
#include <fontconfig/fontconfig.h>

FcLangResult
comp(const FcChar8 *l1, const FcChar8 *l2)
{
    FcLangSet *ls1, *ls2;
    FcLangResult result;

    ls1 = FcLangSetCreate();
    ls2 = FcLangSetCreate();
    FcLangSetAdd(ls1, l1);
    FcLangSetAdd(ls2, l2);

    result = FcLangSetCompare(ls1, ls2);
    FcLangSetDestroy(ls1);
    FcLangSetDestroy(ls2);

    return result;
}

int
main(void)
{
    int i = 1;

    /* 1 */
    if (comp((const FcChar8 *)"ku-am", (const FcChar8 *)"ku-iq") != FcLangDifferentTerritory)
	return i;
    i++;
    /* 2 */
    if (comp((const FcChar8 *)"ku-am", (const FcChar8 *)"ku-ir") != FcLangDifferentTerritory)
	return i;
    i++;
    /* 3 */
    if (comp((const FcChar8 *)"ku-am", (const FcChar8 *)"ku-tr") != FcLangDifferentTerritory)
	return i;
    i++;
    /* 4 */
    if (comp((const FcChar8 *)"ku-iq", (const FcChar8 *)"ku-ir") != FcLangDifferentTerritory)
	return i;
    i++;
    /* 5 */
    if (comp((const FcChar8 *)"ku-iq", (const FcChar8 *)"ku-tr") != FcLangDifferentTerritory)
	return i;
    i++;
    /* 6 */
    if (comp((const FcChar8 *)"ku-ir", (const FcChar8 *)"ku-tr") != FcLangDifferentTerritory)
	return i;
    i++;
    /* 7 */
    if (comp((const FcChar8 *)"ps-af", (const FcChar8 *)"ps-pk") != FcLangDifferentTerritory)
	return i;
    i++;
    /* 8 */
    if (comp((const FcChar8 *)"ti-er", (const FcChar8 *)"ti-et") != FcLangDifferentTerritory)
	return i;
    i++;
    /* 9 */
    if (comp((const FcChar8 *)"zh-cn", (const FcChar8 *)"zh-hk") != FcLangDifferentTerritory)
	return i;
    i++;
    /* 10 */
    if (comp((const FcChar8 *)"zh-cn", (const FcChar8 *)"zh-mo") != FcLangDifferentTerritory)
	return i;
    i++;
    /* 11 */
    if (comp((const FcChar8 *)"zh-cn", (const FcChar8 *)"zh-sg") != FcLangDifferentTerritory)
	return i;
    i++;
    /* 12 */
    if (comp((const FcChar8 *)"zh-cn", (const FcChar8 *)"zh-tw") != FcLangDifferentTerritory)
	return i;
    i++;
    /* 13 */
    if (comp((const FcChar8 *)"zh-hk", (const FcChar8 *)"zh-mo") != FcLangDifferentTerritory)
	return i;
    i++;
    /* 14 */
    if (comp((const FcChar8 *)"zh-hk", (const FcChar8 *)"zh-sg") != FcLangDifferentTerritory)
	return i;
    i++;
    /* 15 */
    if (comp((const FcChar8 *)"zh-hk", (const FcChar8 *)"zh-tw") != FcLangDifferentTerritory)
	return i;
    i++;
    /* 16 */
    if (comp((const FcChar8 *)"zh-mo", (const FcChar8 *)"zh-sg") != FcLangDifferentTerritory)
	return i;
    i++;
    /* 17 */
    if (comp((const FcChar8 *)"zh-mo", (const FcChar8 *)"zh-tw") != FcLangDifferentTerritory)
	return i;
    i++;
    /* 18 */
    if (comp((const FcChar8 *)"zh-sg", (const FcChar8 *)"zh-tw") != FcLangDifferentTerritory)
	return i;
    i++;
    /* 19 */
    if (comp((const FcChar8 *)"mn-mn", (const FcChar8 *)"mn-cn") != FcLangDifferentTerritory)
	return i;
    i++;
    /* 20 */
    if (comp((const FcChar8 *)"pap-an", (const FcChar8 *)"pap-aw") != FcLangDifferentTerritory)
	return i;
    i++;

    return 0;
}


