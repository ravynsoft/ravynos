/*	$NetBSD: t_getcwd.c,v 1.3 2011/07/27 05:04:11 jruoho Exp $ */

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
__RCSID("$NetBSD: t_getcwd.c,v 1.3 2011/07/27 05:04:11 jruoho Exp $");

#include <sys/param.h>
#include <sys/stat.h>

#include <errno.h>
#include <fts.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>

#include <darwintest.h>

T_DECL(getcwd_err, "Test error conditions in getcwd(3)")
{
	char buf[MAXPATHLEN];

	errno = 0;

	T_ASSERT_NULL(getcwd(buf, 0), NULL);
	T_ASSERT_EQ(errno, EINVAL, NULL);
}

T_DECL(getcwd_fts, "A basic test of getcwd(3)")
{
	char buf[MAXPATHLEN];
	char *args[2] = {"/System", NULL};
	FTSENT *ftse;
	FTS *fts;
	int ops;
	short depth;

	/*
	 * Do not traverse too deep
	 */
	depth = 2;

	/*
	 * Test that getcwd(3) works with basic
	 * system directories. Note that having
	 * no FTS_NOCHDIR specified should ensure
	 * that the current directory is visited.
	 */
	ops = FTS_PHYSICAL | FTS_NOSTAT;
	fts = fts_open(args, ops, NULL);

	T_ASSERT_NOTNULL(fts, NULL);

	while ((ftse = fts_read(fts)) != NULL) {

		if (ftse->fts_level < 1)
			continue;

		if (ftse->fts_level > depth) {
			(void)fts_set(fts, ftse, FTS_SKIP);
			continue;
		}

		switch(ftse->fts_info) {

		case FTS_DP:
			(void)memset(buf, 0, sizeof(buf));
			T_WITH_ERRNO;
			T_ASSERT_NOTNULL(getcwd(buf, sizeof(buf)), NULL);
			T_LOG("ftse->fts_path: %s", ftse->fts_path);
			T_ASSERT_NOTNULL(strstr(ftse->fts_path, buf), NULL);
			break;

		default:
			break;
		}
	}

	(void)fts_close(fts);
}
