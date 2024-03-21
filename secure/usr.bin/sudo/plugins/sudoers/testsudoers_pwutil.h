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

#ifndef TESTSUDOERS_PWUTIL_H
#define TESTSUDOERS_PWUTIL_H

#include <config.h>

#ifdef HAVE_STDBOOL_H
# include <stdbool.h>
#else
# include <compat/stdbool.h>
#endif /* HAVE_STDBOOL_H */
#include <grp.h>
#include <pwd.h>

struct cache_item *testsudoers_make_gritem(gid_t gid, const char *group);
struct cache_item *testsudoers_make_grlist_item(const struct passwd *pw, char * const *groups);
struct cache_item *testsudoers_make_gidlist_item(const struct passwd *pw, int ngids, GETGROUPS_T *gids, char * const *gidstrs, unsigned int type);
struct cache_item *testsudoers_make_pwitem(uid_t uid, const char *user);
bool testsudoers_valid_shell(const char *shell);

#endif /* TESTSUDOERS_PWUTIL_H */
