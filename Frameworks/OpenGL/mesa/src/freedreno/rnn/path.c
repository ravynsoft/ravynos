/*
 * Copyright (C) 2011 Marcin Kościelnicki <koriakin@0x04.net>
 * All Rights Reserved.
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
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "util.h"
#include <string.h>

FILE *find_in_path(const char *name, const char *path, char **pfullname) {
	if (!path)
		return 0;
	while (path) {
		const char *npath = strchr(path, ':');
		size_t plen;
		if (npath) {
			plen = npath - path;
			npath++;
		} else {
			plen = strlen(path);
		}
		if (plen) {
			/* also look for .gz compressed xml: */
			const char *exts[] = { "", ".gz" };
			for (int i = 0; i < ARRAY_SIZE(exts); i++) {
				char *fullname;

				int ret = asprintf(&fullname, "%.*s/%s%s", (int)plen, path, name, exts[i]);
				if (ret < 0)
					return NULL;

				FILE *file = fopen(fullname, "r");
				if (file) {
					if (pfullname)
						*pfullname = fullname;
					else
						free(fullname);
					return file;
				}
				free(fullname);
			}
		}
		path = npath;
	}
	return 0;
}
