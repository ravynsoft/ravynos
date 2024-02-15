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

/*
 * Stub versions of functions needed by the parser.
 * Required to link cvtsudoers and visudo.
 */

#include <config.h>

#include <stdio.h>
#include <string.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <sudoers.h>
#include <interfaces.h>

/* STUB */
bool
init_envtables(void)
{
    return true;
}

/* STUB */
bool
user_is_exempt(const struct sudoers_context *ctx)
{
    return false;
}

/* STUB */
void
sudo_setspent(void)
{
    return;
}

/* STUB */
void
sudo_endspent(void)
{
    return;
}

/* STUB */
int
group_plugin_query(const char *user, const char *group, const struct passwd *pw)
{
    return false;
}

/* STUB */
struct interface_list *
get_interfaces(void)
{
    static struct interface_list empty = SLIST_HEAD_INITIALIZER(interfaces);
    return &empty;
}

/* STUB */
int
set_cmnd_path(struct sudoers_context *ctx, const char *runchroot)
{
    /* Cannot return FOUND without also setting ctx->user.cmnd to a new value. */
    return NOT_FOUND;
}

/* STUB */
void
init_eventlog_config(void)
{
    return;
}

/* STUB */
bool
pivot_root(const char *new_root, struct sudoers_pivot *state)
{
    return true;
}

/* STUB */
bool
unpivot_root(struct sudoers_pivot *state)
{
    return true;
}
