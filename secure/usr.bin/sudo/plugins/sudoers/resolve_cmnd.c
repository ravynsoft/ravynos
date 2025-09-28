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

#include <sudoers.h>

/*
 * Calls find_path() first with PERM_RUNAS, falling back to PERM_USER.
 * Returns FOUND if the command was found, NOT_FOUND if it was not found,
 * NOT_FOUND_DOT if it would have been found but it is in '.' and
 * def_ignore_dot is set or NOT_FOUND_ERROR if an error occurred.
 * The caller is responsible for freeing the output file.
 */
int
resolve_cmnd(struct sudoers_context *ctx, const char *infile,
    char **outfile, const char *path)
{
    int ret = NOT_FOUND_ERROR;
    debug_decl(resolve_cmnd, SUDOERS_DEBUG_UTIL);

    if (!set_perms(ctx, PERM_RUNAS))
        goto done;
    ret = find_path(infile, outfile, ctx->user.cmnd_stat, path,
        def_ignore_dot, NULL);
    if (!restore_perms())
        goto done;
    if (ret == NOT_FOUND) {
        /* Failed as runas user, try as invoking user. */
        if (!set_perms(ctx, PERM_USER))
            goto done;
        ret = find_path(infile, outfile, ctx->user.cmnd_stat, path,
            def_ignore_dot, NULL);
        if (!restore_perms())
            goto done;
    }
done:
    debug_return_int(ret);
}
