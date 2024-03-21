/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2023 Todd C. Miller <Todd.Miller@sudo.ws>
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

#include <sudoers.h>

/*
 * Free memory allocated for struct sudoers_context.
 */
void
sudoers_ctx_free(struct sudoers_context *ctx)
{
    debug_decl(sudoers_ctx_free, SUDOERS_DEBUG_PLUGIN);

    /* Free remaining references to password and group entries. */
    if (ctx->user.pw != NULL)
	sudo_pw_delref(ctx->user.pw);
    if (ctx->user.gid_list != NULL)
	sudo_gidlist_delref(ctx->user.gid_list);

    /* Free dynamic contents of user_ctx. */
    free(ctx->user.cwd);
    free(ctx->user.name);
    if (ctx->user.ttypath != NULL)
	free(ctx->user.ttypath);
    else
	free(ctx->user.tty);
    if (ctx->user.shost != ctx->user.host)
	    free(ctx->user.shost);
    free(ctx->user.host);
    free(ctx->user.cmnd);
    canon_path_free(ctx->user.cmnd_dir);
    free(ctx->user.cmnd_args);
    free(ctx->user.cmnd_list);
    free(ctx->user.cmnd_stat);

    /* Free remaining references to password and group entries. */
    if (ctx->runas.pw != NULL)
	sudo_pw_delref(ctx->runas.pw);
    if (ctx->runas.gr != NULL)
	sudo_gr_delref(ctx->runas.gr);
    if (ctx->runas.list_pw != NULL)
	sudo_pw_delref(ctx->runas.list_pw);

    /* Free dynamic contents of ctx->runas. */
    free(ctx->runas.cmnd);
    free(ctx->runas.cmnd_saved);
    if (ctx->runas.shost != ctx->runas.host)
	free(ctx->runas.shost);
    free(ctx->runas.host);
#ifdef HAVE_SELINUX
    free(ctx->runas.role);
    free(ctx->runas.type);
#endif
#ifdef HAVE_APPARMOR
    free(ctx->runas.apparmor_profile);
#endif
#ifdef HAVE_PRIV_SET
    free(ctx->runas.privs);
    free(ctx->runas.limitprivs);
#endif

    /* Free dynamic contents of ctx. */
    free(ctx->source);

    memset(ctx, 0, sizeof(*ctx));

    debug_return;
}
