/*
 * Copyright © 2019 Red Hat, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <config.h>

#pragma once

#include <unistd.h>
#include "util-strings.h"

/**
 * Try to figure out the directory we're executing from and if it matches
 * the builddir, return that directory. Otherwise, return NULL.
 */
static inline char *
builddir_lookup(void)
{
	char execdir[PATH_MAX];
	char *pathsep;
	ssize_t nread;

	/* In the case of release builds, the builddir is
	   the empty string */
	if (streq(MESON_BUILD_ROOT, ""))
		return NULL;

	nread = readlink("/proc/self/exe", execdir, sizeof(execdir) - 1);
	if (nread <= 0 || nread == sizeof(execdir) - 1)
		return NULL;

	/* readlink doesn't terminate the string and readlink says
	   anything past sz is undefined */
	execdir[nread] = '\0';

	pathsep = strrchr(execdir, '/');
	if (!pathsep)
		return NULL;

	*pathsep = '\0';
	if (!streq(execdir, MESON_BUILD_ROOT))
		return NULL;

	return safe_strdup(execdir);
}
