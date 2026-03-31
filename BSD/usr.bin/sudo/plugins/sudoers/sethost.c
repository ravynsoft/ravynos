/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2018-2023 Todd C. Miller <Todd.Miller@sudo.ws>
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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sudoers.h>

/*
 * Set ctx->user.host. ctx->user.shost, ctx->runas.host and ctx->runas.shost
 * based on the local and remote host names.  If host is NULL, the local
 * host name is used.  If remhost is NULL, the same value as host is used.
 */
bool
sudoers_sethost(struct sudoers_context *ctx, const char *host,
    const char *remhost)
{
    char *cp;
    debug_decl(sudoers_sethost, SUDOERS_DEBUG_UTIL);

    if (ctx->user.shost != ctx->user.host)
	free(ctx->user.shost);
    free(ctx->user.host);
    ctx->user.host = NULL;
    ctx->user.shost = NULL;

    if (host == NULL) {
	ctx->user.host = sudo_gethostname();
	if (ctx->user.host == NULL && errno != ENOMEM)
	    ctx->user.host = strdup("localhost");
    } else {
	ctx->user.host = strdup(host);
    }
    if (ctx->user.host == NULL)
	    goto oom;
    if ((cp = strchr(ctx->user.host, '.')) != NULL) {
	ctx->user.shost = strndup(ctx->user.host,
	    (size_t)(cp - ctx->user.host));
	if (ctx->user.shost == NULL)
	    goto oom;
    } else {
	ctx->user.shost = ctx->user.host;
    }

    if (ctx->runas.shost != ctx->runas.host)
	free(ctx->runas.shost);
    free(ctx->runas.host);
    ctx->runas.host = NULL;
    ctx->runas.shost = NULL;

    ctx->runas.host = strdup(remhost ? remhost : ctx->user.host);
    if (ctx->runas.host == NULL)
	goto oom;
    if ((cp = strchr(ctx->runas.host, '.')) != NULL) {
	ctx->runas.shost = strndup(ctx->runas.host,
	    (size_t)(cp - ctx->runas.host));
	if (ctx->runas.shost == NULL)
	    goto oom;
    } else {
	ctx->runas.shost = ctx->runas.host;
    }

    debug_return_bool(true);
oom:
    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
    debug_return_bool(false);
}
