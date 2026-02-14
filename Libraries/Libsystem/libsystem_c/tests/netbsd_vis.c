/*	$NetBSD: t_vis.c,v 1.7 2014/09/08 19:01:03 christos Exp $	*/

/*-
 * Copyright (c) 2002 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code was contributed to The NetBSD Foundation by Christos Zoulas.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <string.h>
#include <stdlib.h>
#include <err.h>
#include <vis.h>

#include <darwintest.h>

static int styles[] = {
	VIS_OCTAL,
	VIS_CSTYLE,
	VIS_SP,
	VIS_TAB,
	VIS_NL,
	VIS_WHITE,
	VIS_SAFE,
#if 0	/* Not reversible */
	VIS_NOSLASH,
#endif
	VIS_HTTP1808,
	VIS_MIMESTYLE,
#if 0	/* Not supported by vis(3) */
	VIS_HTTP1866,
#endif
};

#define SIZE	256

T_DECL(strvis_basic, "strvis(3)")
{
	char *srcbuf, *dstbuf, *visbuf;
	unsigned int i, j;

	T_ASSERT_NOTNULL((srcbuf = malloc(SIZE)), NULL);
	T_ASSERT_NOTNULL((dstbuf = malloc(SIZE + 1)), NULL);
	T_ASSERT_NOTNULL((visbuf = malloc(SIZE * 4 + 1)), NULL);

	for (i = 0; i < SIZE; i++)
		srcbuf[i] = (char)i;

	for (i = 0; i < sizeof(styles)/sizeof(styles[0]); i++) {
		T_ASSERT_GT(strsvisx(visbuf, srcbuf, SIZE, styles[i], ""), 0, NULL);
		memset(dstbuf, 0, SIZE);
		T_ASSERT_GT(strunvisx(dstbuf, visbuf, 
		    styles[i] & (VIS_HTTP1808|VIS_MIMESTYLE)), 0, NULL);
		for (j = 0; j < SIZE; j++)
			if (dstbuf[j] != (char)j)
				T_FAIL("Failed for style %x, char %d [%d]", styles[i], j, dstbuf[j]);
		if (dstbuf[SIZE] != '\0')
			T_FAIL("Failed for style %x, the result must be null-terminated [%d]", styles[i], dstbuf[SIZE]);
	}
	free(dstbuf);
	free(srcbuf);
	free(visbuf);
}

T_DECL(strvis_null, "strvis(3) NULL")
{
	char dst[] = "fail";
	strvis(dst, NULL, VIS_SAFE);
	T_ASSERT_EQ(dst[0], '\0', NULL);
	T_ASSERT_EQ(dst[1], 'a', NULL);
}

T_DECL(strvis_empty, "strvis(3) empty")
{
	char dst[] = "fail";
	strvis(dst, "", VIS_SAFE);
	T_ASSERT_EQ(dst[0], '\0', NULL);
	T_ASSERT_EQ(dst[1], 'a', NULL);
}

T_DECL(strunvis_hex, "strunvis(3) \\Xxx")
{
	static const struct {
		const char *e;
		const char *d;
		int error;
	} ed[] = {
		{ "\\xff", "\xff", 1 },
		{ "\\x1", "\x1", 1 },
		{ "\\x1\\x02", "\x1\x2", 2 },
		{ "\\x1x", "\x1x", 2 },
		{ "\\xx", "", -1 },
	};
	char uv[10];

	for (size_t i = 0; i < sizeof(ed)/sizeof(ed[0]); i++) {
		T_ASSERT_EQ(strunvis(uv, ed[i].e), ed[i].error, NULL);
		if (ed[i].error > 0)
			T_ASSERT_EQ(memcmp(ed[i].d, uv, (unsigned long)ed[i].error), 0, NULL);
	}
}
