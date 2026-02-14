/*
 * Based on the OpenBSD test
 * Copyright (c) 2011 Martin Pieuchot <mpi@openbsd.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <sys/cdefs.h>
__RCSID("$NetBSD: t_open_memstream.c,v 1.2 2014/10/19 11:17:43 justin Exp $");

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <darwintest.h>

#define OFFSET 16384

static const char start[] = "start";
static const char hello[] = "hello";

T_DECL(netbsd_open_memstream_test_open_memstream, "")
{
	FILE	*fp;
	char	*buf = (char *)0xff;
	size_t	 size = 0;
	off_t	 off;
	int	 i;

	fp = open_memstream(&buf, &size);
	T_ASSERT_NOTNULL(fp, NULL);

	off = ftello(fp);
	T_EXPECT_EQ(off, 0LL, NULL);

	T_EXPECT_POSIX_ZERO(fflush(fp), NULL);
	T_EXPECT_EQ(size, 0UL, NULL);
	T_EXPECT_NE((void*)buf, (void *)0xff, NULL);
	T_EXPECT_EQ(fseek(fp, -6, SEEK_SET), -1, NULL);
	T_EXPECT_POSIX_ZERO(fseek(fp, OFFSET, SEEK_SET), NULL);
	T_EXPECT_NE(fprintf(fp, hello), EOF, NULL);
	T_EXPECT_NE(fflush(fp), EOF, NULL);
	T_EXPECT_EQ(size, OFFSET + sizeof(hello)-1, NULL);
	T_EXPECT_POSIX_ZERO(fseek(fp, 0, SEEK_SET), NULL);
	T_EXPECT_NE(fprintf(fp, start), EOF, NULL);
	T_EXPECT_NE(fflush(fp), EOF, NULL);
	T_EXPECT_EQ(size, sizeof(start)-1, NULL);

	/* Needed for sparse files */
	T_EXPECT_EQ(strncmp(buf, start, sizeof(start)-1), 0, NULL);
	for (i = sizeof(start)-1; i < OFFSET; i++)
		T_EXPECT_EQ(buf[i], '\0', NULL);

	T_EXPECT_EQ(memcmp(buf + OFFSET, hello, sizeof(hello)-1), 0, NULL);

	/* verify that simply seeking past the end doesn't increase the size */
	T_EXPECT_POSIX_ZERO(fseek(fp, 100, SEEK_END), NULL);
	T_EXPECT_NE(fflush(fp), EOF, NULL);
	T_EXPECT_EQ(size, OFFSET + sizeof(hello)-1, NULL);
	T_EXPECT_POSIX_ZERO(fseek(fp, 8, SEEK_SET), NULL);
	T_EXPECT_EQ(ftell(fp), 8L, NULL);

	/* Try to seek backward */
	T_EXPECT_POSIX_ZERO(fseek(fp, -1, SEEK_CUR), NULL);
	T_EXPECT_EQ(ftell(fp), 7L, NULL);
	T_EXPECT_POSIX_ZERO(fseek(fp, 5, SEEK_CUR), NULL);
	T_EXPECT_NE(fclose(fp), EOF, NULL);
	T_EXPECT_EQ(size, 12UL, NULL);

	free(buf);
}
