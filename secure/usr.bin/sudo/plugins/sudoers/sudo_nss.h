/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2007-2011, 2013-2015, 2017-2018
 *	Todd C. Miller <Todd.Miller@sudo.ws>
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

#ifndef SUDOERS_NSS_H
#define SUDOERS_NSS_H

struct passwd;
struct userspec_list;
struct defaults_list;
struct sudoers_context;

/* XXX - parse_tree, ret_if_found and ret_if_notfound should be private */
struct sudo_nss {
    TAILQ_ENTRY(sudo_nss) entries;
    const char *source;
    int (*open)(struct sudoers_context *ctx, struct sudo_nss *nss);
    int (*close)(struct sudoers_context *ctx, struct sudo_nss *nss);
    struct sudoers_parse_tree *(*parse)(struct sudoers_context *ctx, const struct sudo_nss *nss);
    int (*query)(struct sudoers_context *ctx, const struct sudo_nss *nss, struct passwd *pw);
    int (*getdefs)(struct sudoers_context *ctx, const struct sudo_nss *nss);
    int (*innetgr)(const struct sudo_nss *nss, const char *netgr,
	const char *host, const char *user, const char *domain);
    void *handle;
    struct sudoers_parse_tree *parse_tree;
    bool ret_if_found;
    bool ret_if_notfound;
};

TAILQ_HEAD(sudo_nss_list, sudo_nss);

struct sudo_nss_list *sudo_read_nss(void);
bool sudo_nss_can_continue(const struct sudo_nss *nss, int match);

#endif /* SUDOERS_NSS_H */
