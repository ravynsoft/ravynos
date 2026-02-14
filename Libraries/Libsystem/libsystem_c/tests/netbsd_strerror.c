/* $NetBSD: t_strerror.c,v 1.3 2011/05/10 06:55:27 jruoho Exp $ */

/*-
 * Copyright (c) 2011 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Jukka Ruohonen.
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
#include <sys/cdefs.h>
__RCSID("$NetBSD: t_strerror.c,v 1.3 2011/05/10 06:55:27 jruoho Exp $");

#include <errno.h>
#include <limits.h>
#include <locale.h>
#include <string.h>
#include <stdio.h>

#include "darwintest.h"

T_DECL(netbsd_strerror_basic, "A basic test of strerror(3)")
{
	int i;

	for (i = 1; i < sys_nerr; i++)
		T_EXPECT_NULL(strstr(strerror(i), "Unknown error:"), NULL);

	for (; i < sys_nerr + 10; i++)
		T_EXPECT_NOTNULL(strstr(strerror(i), "Unknown error:"), NULL);
}

T_DECL(netbsd_strerror_err, "Test errors from strerror(3)")
{
	(void)setlocale(LC_ALL, "C");

	errno = 0;

	T_ASSERT_NOTNULL(strstr(strerror(INT_MAX), "Unknown error:"), NULL);
	T_ASSERT_EQ(errno, EINVAL, NULL);

	errno = 0;

	T_ASSERT_NOTNULL(strstr(strerror(INT_MIN), "Unknown error:"), NULL);
	T_ASSERT_EQ(errno, EINVAL, NULL);
}

T_DECL(netbsd_strerror_r_basic, "A basic test of strerror_r(3)")
{
	char buf[512];
	int i;

	(void)setlocale(LC_ALL, "C");

	for (i = 1; i < sys_nerr; i++) {
		T_ASSERT_EQ(strerror_r(i, buf, sizeof(buf)), 0, NULL);
		T_ASSERT_NULL(strstr(buf, "Unknown error:"), NULL);
	}

	for (; i < sys_nerr + 10; i++) {
		T_ASSERT_EQ(strerror_r(i, buf, sizeof(buf)), EINVAL, NULL);
		T_ASSERT_NOTNULL(strstr(buf, "Unknown error:"), NULL);
	}
}

T_DECL(netbsd_strerror_r_err, "Test errors from strerror_r(3)")
{
	char buf[512];
	int rv;

	(void)setlocale(LC_ALL, "C");

	rv = strerror_r(EPERM, buf, 1);
	T_ASSERT_EQ(rv, ERANGE, NULL);

	rv = strerror_r(INT_MAX, buf, sizeof(buf));

	T_ASSERT_EQ(rv, EINVAL, NULL);
	T_ASSERT_NOTNULL(strstr(buf, "Unknown error:"), NULL);

	rv = strerror_r(INT_MIN, buf, sizeof(buf));

	T_ASSERT_EQ(rv, EINVAL, NULL);
	T_ASSERT_NOTNULL(strstr(buf, "Unknown error:"), NULL);
}
