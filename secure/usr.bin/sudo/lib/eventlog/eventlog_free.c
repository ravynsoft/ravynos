/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2020 Todd C. Miller <Todd.Miller@sudo.ws>
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
 *
 * Sponsored in part by the Defense Advanced Research Projects
 * Agency (DARPA) and Air Force Research Laboratory, Air Force
 * Materiel Command, USAF, under agreement number F39502-99-1-0512.
 */

/*
 * This is an open source non-commercial project. Dear PVS-Studio, please check it.
 * PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>

#include <sudo_compat.h>
#include <sudo_debug.h>
#include <sudo_eventlog.h>
#include <sudo_util.h>

/*
 * Free the strings in a struct eventlog.
 */
void
eventlog_free(struct eventlog *evlog)
{
    size_t i;
    debug_decl(eventlog_free, SUDO_DEBUG_UTIL);

    if (evlog != NULL) {
	free(evlog->iolog_path);
	free(evlog->command);
	free(evlog->cwd);
	free(evlog->runchroot);
	free(evlog->runcwd);
	free(evlog->rungroup);
	free(evlog->runuser);
	free(evlog->peeraddr);
	free(evlog->signal_name);
	free(evlog->source);
	if (evlog->submitenv != NULL) {
	    for (i = 0; evlog->submitenv[i] != NULL; i++)
		free(evlog->submitenv[i]);
	    free(evlog->submitenv);
	}
	free(evlog->submithost);
	free(evlog->submituser);
	free(evlog->submitgroup);
	free(evlog->ttyname);
	if (evlog->runargv != NULL) {
	    for (i = 0; evlog->runargv[i] != NULL; i++)
		free(evlog->runargv[i]);
	    free(evlog->runargv);
	}
	if (evlog->runenv != NULL) {
	    for (i = 0; evlog->runenv[i] != NULL; i++)
		free(evlog->runenv[i]);
	    free(evlog->runenv);
	}
	if (evlog->env_add != NULL) {
	    for (i = 0; evlog->env_add[i] != NULL; i++)
		free(evlog->env_add[i]);
	    free(evlog->env_add);
	}
	free(evlog);
    }

    debug_return;
}
