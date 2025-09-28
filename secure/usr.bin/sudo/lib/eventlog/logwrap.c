/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2011, 2014-2020 Todd C. Miller <Todd.Miller@sudo.ws>
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

/*
 * This is an open source non-commercial project. Dear PVS-Studio, please check it.
 * PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sudo_compat.h>
#include <sudo_debug.h>
#include <sudo_util.h>
#include <sudo_eventlog.h>

size_t
eventlog_writeln(FILE * restrict fp, char * restrict line, size_t linelen, size_t maxlen)
{
    const char *indent = "";
    char *beg = line;
    char *end;
    int len;
    size_t outlen = 0;
    debug_decl(eventlog_writeln, SUDO_DEBUG_UTIL);

    if (maxlen < sizeof(EVENTLOG_INDENT)) {
	/* Maximum length too small, disable wrapping. */
	outlen = fwrite(line, 1, linelen, fp);
	if (outlen != linelen)
	    debug_return_size_t((size_t)-1);
	if (fputc('\n', fp) == EOF)
	    debug_return_size_t((size_t)-1);
	debug_return_size_t(outlen + 1);
    }

    /*
     * Print out line with word wrap around maxlen characters.
     */
    while (linelen > maxlen) {
	end = beg + maxlen;
	while (end != beg && *end != ' ')
	    end--;
	if (beg == end) {
	    /* Unable to find word break within maxlen, look beyond. */
	    end = strchr(beg + maxlen, ' ');
	    if (end == NULL)
		break;	/* no word break */
	}
	len = fprintf(fp, "%s%.*s\n", indent, (int)(end - beg), beg);
	if (len < 0)
	    debug_return_size_t((size_t)-1);
	outlen += (size_t)len;
	while (*end == ' ')
	    end++;
	linelen -= (size_t)(end - beg);
	beg = end;
	if (indent[0] == '\0') {
	    indent = EVENTLOG_INDENT;
	    maxlen -= sizeof(EVENTLOG_INDENT) - 1;
	}
    }
    /* Print remainder, if any. */
    if (linelen) {
	len = fprintf(fp, "%s%s\n", indent, beg);
	if (len < 0)
	    debug_return_size_t((size_t)-1);
	outlen += (size_t)len;
    }

    debug_return_size_t(outlen);
}
