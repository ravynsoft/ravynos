/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2010-2013, 2015-2017 Todd C. Miller <Todd.Miller@sudo.ws>
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

#ifndef SUDOERS_PWUTIL_H
#define SUDOERS_PWUTIL_H

#define ptr_to_item(p) ((struct cache_item *)((char *)p - offsetof(struct cache_item_##p, p)))

/*
 * Generic cache element.
 */
struct cache_item {
    unsigned int refcnt;
    unsigned int type;	/* only used for gidlist */
    char registry[16];	/* AIX-specific, empty otherwise */
    /* key */
    union {
	uid_t uid;
	gid_t gid;
	char *name;
    } k;
    /* datum */
    union {
	struct passwd *pw;
	struct group *gr;
	struct group_list *grlist;
	struct gid_list *gidlist;
    } d;
};

/*
 * Container structs to simpify size and offset calculations and guarantee
 * proper alignment of struct passwd, group, gid_list and group_list.
 */
struct cache_item_pw {
    struct cache_item cache;
    struct passwd pw;
};

struct cache_item_gr {
    struct cache_item cache;
    struct group gr;
};

struct cache_item_grlist {
    struct cache_item cache;
    struct group_list grlist;
    /* actually bigger */
};

struct cache_item_gidlist {
    struct cache_item cache;
    struct gid_list gidlist;
    /* actually bigger */
};

struct cache_item *sudo_make_gritem(gid_t gid, const char *group);
struct cache_item *sudo_make_grlist_item(const struct passwd *pw, char * const *groups);
struct cache_item *sudo_make_gidlist_item(const struct passwd *pw, int ngids, GETGROUPS_T *gids, char * const *gidstrs, unsigned int type);
struct cache_item *sudo_make_pwitem(uid_t uid, const char *user);
bool sudo_valid_shell(const char *shell);

#endif /* SUDOERS_PWUTIL_H */
