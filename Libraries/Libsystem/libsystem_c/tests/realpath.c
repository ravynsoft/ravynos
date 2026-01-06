/* $NetBSD: t_realpath.c,v 1.2 2012/03/27 07:54:58 njoly Exp $ */

/*-
 * Copyright (c) 2012 The NetBSD Foundation, Inc.
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
__RCSID("$NetBSD: t_realpath.c,v 1.2 2012/03/27 07:54:58 njoly Exp $");

#include <sys/param.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <darwintest.h>
#include <darwintest_utils.h>

struct testcase {
	const char *path;
	const char *result;
};

static const struct testcase paths[] = {
	{ "/",			"/"		},
	{ "///////",		"/"		},
	{ "",			NULL		},
	{ "       ",		NULL		},
	{ "/      ",		NULL		},
	{ "      /",		NULL		},
	{ "/usr/bin///",	"/usr/bin"	},
	{ "///////usr",		"/usr"		},
	{ "/a/b/c/d/e",		NULL		},
	{ "    /usr/bin	   ",	NULL		},
	{ "\\//////usr//bin",	NULL		},
	{ "//usr//bin//",	"/usr/bin"	},
	{ "//////usr//bin//",	"/usr/bin"	},
	{ "/usr/bin//////////", "/usr/bin"	},
};

T_DECL(realpath_basic, "Resolve various short directory paths")
{
	char buf[MAXPATHLEN];
	char *ptr;
	size_t i;

	size_t num_cases = sizeof(paths) / sizeof(struct testcase);
	for (i = 0; i < num_cases; i++) {

		(void)memset(buf, '\0', sizeof(buf));

		ptr = realpath(paths[i].path, buf);

		if (paths[i].result == NULL) {
			T_EXPECT_NULL(ptr, NULL);
		} else {
			T_EXPECT_EQ_STR(ptr, paths[i].result, "Real path matches expected path");
		}
	}
}

T_DECL(realpath_trailing, "Trailing . and .. should fail after non-directories")
{
	char result[MAXPATHLEN] = { 0 };

	T_EXPECT_NULL(realpath("/usr/null/.", result), NULL);
	T_EXPECT_NULL(realpath("/usr/null/..", result), NULL);
}

T_DECL(realpath_huge, "Test realpath with maximum path size")
{
	char result[MAXPATHLEN] = { 0 };
	char buffer[MAXPATHLEN] = { 0 };

	(void)memset(buffer, '/', sizeof(buffer) - 1);

	T_EXPECT_NOTNULL(realpath(buffer, result), NULL);
	T_EXPECT_EQ(strlen(result), (size_t) 1, NULL);
	T_EXPECT_EQ(result[0], '/', NULL);
}

T_DECL(realpath_symlink, "Resolve symlinked paths")
{
	char *resolved_tmpdir;
	char path[MAXPATHLEN] = { 0 };
	char slnk[MAXPATHLEN] = { 0 };
	char resb[MAXPATHLEN] = { 0 };
	int fd;

	// We have to use the realpath for this symlink test.
	// Otherwise, when trying to resolve a symlink, it won't match.
	resolved_tmpdir = realpath(dt_tmpdir(), NULL);
	snprintf(path, sizeof(path), "%s%s", resolved_tmpdir, "/real");
	snprintf(slnk, sizeof(path), "%s%s", resolved_tmpdir, "/sym");

	fd = open(path, O_RDONLY | O_CREAT, 0600);

	T_WITH_ERRNO;
	T_ASSERT_GE(fd, 0, NULL);
	T_ASSERT_POSIX_ZERO(symlink(path, slnk), NULL);
	T_ASSERT_POSIX_ZERO(close(fd), NULL);

	T_ASSERT_NOTNULL(realpath(slnk, resb), NULL);
	T_ASSERT_EQ_STR(resb, path, NULL);

	T_ASSERT_POSIX_ZERO(unlink(path), NULL);
	T_ASSERT_POSIX_ZERO(unlink(slnk), NULL);
	free(resolved_tmpdir);
}
