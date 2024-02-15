/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2020-2023 Todd C. Miller <Todd.Miller@sudo.ws>
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
#ifdef HAVE_STDBOOL_H
# include <stdbool.h>
#else
# include <compat/stdbool.h>
#endif /* HAVE_STDBOOL_H */

#include <sudo_compat.h>
#include <sudo_debug.h>
#include <sudo_eventlog.h>
#include <sudo_iolog.h>

bool
iolog_parse_loginfo_json(FILE *fp, const char *iolog_dir, struct eventlog *evlog)
{
    struct eventlog_json_object *root;
    bool ret = false;
    debug_decl(iolog_parse_loginfo_json, SUDO_DEBUG_UTIL);

    root = eventlog_json_read(fp, iolog_dir);
    if (root != NULL) {
	/* Parse the JSON into an eventlog. */
	ret = eventlog_json_parse(root, evlog);

	/* Cleanup. */
	eventlog_json_free(root);
    }

    debug_return_bool(ret);
}
