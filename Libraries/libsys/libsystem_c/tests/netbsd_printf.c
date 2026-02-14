/* $NetBSD: t_printf.c,v 1.8 2012/04/11 16:21:42 jruoho Exp $ */

/*-
 * Copyright (c) 2010 The NetBSD Foundation, Inc.
 * All rights reserved.
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

#include <sys/types.h>
#include <sys/resource.h>
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#include "darwintest.h"

T_DECL(netbsd_snprintf_c99, "Test printf(3) C99 conformance (PR lib/22019)")
{
	char s[4];

	(void)memset(s, '\0', sizeof(s));
	(void)snprintf(s, sizeof(s), "%#.o", 0);
	(void)printf("printf = %#.o\n", 0);
	(void)fprintf(stderr, "snprintf = %s", s);

	T_ASSERT_EQ(strlen(s), (size_t)1, NULL);
	T_ASSERT_EQ(s[0], '0', NULL);
}

T_DECL(netbsd_snprintf_dotzero, "PR lib/32951: %%.0f formats (0.0,0.5] to \"0.\"")
{
	char s[4];

	T_WITH_ERRNO; T_EXPECT_EQ(snprintf(s, sizeof(s), "%.0f", 0.1), 1, NULL);
	T_EXPECT_EQ_STR(s, "0", NULL);
}

T_DECL(netbsd_snprintf_float, "test that floating conversions don't leak memory")
{
	union {
		double d;
		uint64_t bits;
	} u;
	uint32_t ul, uh;
	time_t now;
	char buf[1000];
	struct rlimit rl;

	rl.rlim_cur = rl.rlim_max = 1 * 1024 * 1024;
	T_WITH_ERRNO; T_EXPECT_NE(setrlimit(RLIMIT_AS, &rl), -1, NULL);
	rl.rlim_cur = rl.rlim_max = 1 * 1024 * 1024;
	T_EXPECT_POSIX_SUCCESS(setrlimit(RLIMIT_DATA, &rl), NULL);

	time(&now);
	srand((unsigned int)now);
	for (size_t i = 0; i < 10000; i++) {
		ul = (uint32_t)rand();
		uh = (uint32_t)rand();
		u.bits = (uint64_t)uh << 32 | ul;
		T_EXPECT_POSIX_SUCCESS(snprintf(buf, sizeof buf, " %.2f", u.d), NULL);
	}
}

T_DECL(netbsd_sprintf_zeropad, "Test output format zero padding (PR lib/44113)")
{
	char str[1024];

	T_WITH_ERRNO; T_EXPECT_EQ(sprintf(str, "%010f", 0.0), 10, NULL);
	T_EXPECT_EQ_STR(str, "000.000000", NULL);

	/* ieeefp */
#ifndef __vax__
	/* printf(3) should ignore zero padding for nan/inf */
	T_WITH_ERRNO; T_EXPECT_EQ(sprintf(str, "%010f", NAN), 10, NULL);
	T_EXPECT_EQ_STR(str, "       nan", NULL);
	T_WITH_ERRNO; T_EXPECT_EQ(sprintf(str, "%010f", INFINITY), 10, NULL);
	T_EXPECT_EQ_STR(str, "       inf", NULL);
#endif
}
