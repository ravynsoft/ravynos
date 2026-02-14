/*-
 * Copyright (c) 2001 Wes Peters <wes@FreeBSD.org>
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD$
 */

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "darwintest.h"

static char buf[64];
static char *sret;
static int iret;

T_DECL(strerror, "")
{
	//T_DECL(strerror_unknown_error, "")
	errno = 0;
	sret = strerror(INT_MAX);
	snprintf(buf, sizeof(buf), "Unknown error: %d", INT_MAX);
	T_EXPECT_EQ_STR(sret, buf, NULL);
	T_EXPECT_EQ(errno, EINVAL, NULL);

	//T_DECL(strerror_no_error, "")
	errno = 0;
	sret = strerror(0);
	T_EXPECT_EQ_STR(sret, "Undefined error: 0", NULL);
	T_EXPECT_EQ(errno, 0, NULL);

	//T_DECL(strerror_EPERM_test, "")
	errno = 0;
	sret = strerror(EPERM);
	T_EXPECT_EQ_STR(sret, "Operation not permitted", NULL);
	T_EXPECT_EQ(errno, 0, NULL);

	//T_DECL(strerror_EPFNOSUPPORT_test, "")
	errno = 0;
	sret = strerror(EPFNOSUPPORT);
	T_EXPECT_EQ_STR(sret, "Protocol family not supported", NULL);
	T_EXPECT_EQ(errno, 0, NULL);

	//T_DECL(strerror_ELAST_test, "")
	errno = 0;
	sret = strerror(ELAST);
	T_EXPECT_EQ(errno, 0, NULL);
}

T_DECL(strerror_r, "")
{
	memset(buf, '*', sizeof(buf));
	iret = strerror_r(-1, buf, sizeof(buf));
	T_EXPECT_EQ_STR(buf, "Unknown error: -1", NULL);
	T_EXPECT_EQ(iret, EINVAL, NULL);

	//T_DECL(strerror_r__EPERM_one_byte_short, "")
	memset(buf, '*', sizeof(buf));
	/* One byte too short. */
	iret = strerror_r(EPERM, buf, strlen("Operation not permitted"));
	T_EXPECT_EQ_STR(buf, "Operation not permitte", NULL);
	T_EXPECT_EQ(iret, ERANGE, NULL);

	//T_DECL(strerror_r__EPERM_unknown_error_one_byte_short, "")
	memset(buf, '*', sizeof(buf));
	/* One byte too short. */
	iret = strerror_r(-1, buf, strlen("Unknown error: -1"));
	T_EXPECT_EQ_STR(buf, "Unknown error: -", NULL);
	T_EXPECT_EQ(iret, EINVAL, NULL);

	//T_DECL(strerror_r__EPERM_unknown_error_two_bytes_short, "")
	memset(buf, '*', sizeof(buf));
	/* Two bytes too short. */
	iret = strerror_r(-2, buf, strlen("Unknown error: -2") - 1);
	T_EXPECT_EQ_STR(buf, "Unknown error: ", NULL);
	T_EXPECT_EQ(iret, EINVAL, NULL);

	//T_DECL(strerror_r__EPERM_unknown_error_three_bytes_short, "")
	memset(buf, '*', sizeof(buf));
	/* Three bytes too short. */
	iret = strerror_r(-2, buf, strlen("Unknown error: -2") - 2);
	T_EXPECT_EQ_STR(buf, "Unknown error:", NULL);
	T_EXPECT_EQ(iret, EINVAL, NULL);

	//T_DECL(strerror_r__EPERM_unknown_error_12345_one_byte_short, "")
	memset(buf, '*', sizeof(buf));
	/* One byte too short. */
	iret = strerror_r(12345, buf, strlen("Unknown error: 12345"));
	T_EXPECT_EQ_STR(buf, "Unknown error: 1234", NULL);
	T_EXPECT_EQ(iret, EINVAL, NULL);

	//T_DECL(strerror_r__no_error, "")
	memset(buf, '*', sizeof(buf));
	iret = strerror_r(0, buf, sizeof(buf));
	T_EXPECT_EQ_STR(buf, "Undefined error: 0", NULL);
	T_EXPECT_EQ(iret, 0, NULL);

	//T_DECL(strerror_r__EDEADLK, "")
	memset(buf, '*', sizeof(buf));
	iret = strerror_r(EDEADLK, buf, sizeof(buf));
	T_EXPECT_EQ_STR(buf, "Resource deadlock avoided", NULL);
	T_EXPECT_EQ(iret, 0, NULL);

	//T_DECL(strerror_r__EPROCLIM, "")
	memset(buf, '*', sizeof(buf));
	iret = strerror_r(EPROCLIM, buf, sizeof(buf));
	T_EXPECT_EQ_STR(buf, "Too many processes", NULL);
	T_EXPECT_EQ(iret, 0, NULL);
}
