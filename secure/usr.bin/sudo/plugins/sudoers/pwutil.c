/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 1996, 1998-2005, 2007-2018
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

#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_STRINGS_H
# include <strings.h>		/* strcasecmp */
#endif
#ifdef HAVE_SETAUTHDB
# include <usersec.h>
#endif /* HAVE_SETAUTHDB */
#include <errno.h>
#include <pwd.h>
#include <grp.h>

#include <sudoers.h>
#include <redblack.h>
#include <pwutil.h>

/*
 * The passwd and group caches.
 */
static struct rbtree *pwcache_byuid, *pwcache_byname;
static struct rbtree *grcache_bygid, *grcache_byname;
static struct rbtree *gidlist_cache, *grlist_cache;

static int  cmp_pwuid(const void *, const void *);
static int  cmp_pwnam(const void *, const void *);
static int  cmp_grgid(const void *, const void *);

static int max_groups;

/*
 * Default functions for building cache items.
 */
static sudo_make_pwitem_t make_pwitem = sudo_make_pwitem;
static sudo_make_gritem_t make_gritem = sudo_make_gritem;
static sudo_make_gidlist_item_t make_gidlist_item = sudo_make_gidlist_item;
static sudo_make_grlist_item_t make_grlist_item = sudo_make_grlist_item;
static sudo_valid_shell_t valid_shell = sudo_valid_shell;

#define cmp_grnam	cmp_pwnam

/*
 * AIX has the concept of authentication registries (files, NIS, LDAP, etc).
 * This allows you to have separate ID <-> name mappings based on which
 * authentication registries the user was looked up in.
 * We store the registry as part of the key and use it when matching.
 */
#ifdef HAVE_SETAUTHDB
# define getauthregistry(u, r)	aix_getauthregistry((u), (r))
#else
# define getauthregistry(u, r)	((r)[0] = '\0')
#endif

/*
 * Change the default pwutil backend functions.
 * The default functions query the password and group databases.
 */
void
sudo_pwutil_set_backend(sudo_make_pwitem_t pwitem, sudo_make_gritem_t gritem,
    sudo_make_gidlist_item_t gidlist_item, sudo_make_grlist_item_t grlist_item,
    sudo_valid_shell_t check_shell)
{
    debug_decl(sudo_pwutil_set_backend, SUDOERS_DEBUG_NSS);

    if (pwitem != NULL)
	make_pwitem = pwitem;
    if (gritem != NULL)
	make_gritem = gritem;
    if (gidlist_item != NULL)
	make_gidlist_item = gidlist_item;
    if (grlist_item != NULL)
	make_grlist_item = grlist_item;
    if (check_shell != NULL)
	valid_shell = check_shell;

    debug_return;
}

/* Get the max number of user groups if set, or 0 if not set. */
int
sudo_pwutil_get_max_groups(void)
{
    return max_groups;
}

/* Set the max number of user groups (negative values ignored). */
void
sudo_pwutil_set_max_groups(int n)
{
    max_groups = n > 0 ? n : 0;
}

/*
 * Compare by user-ID.
 * v1 is the key to find or data to insert, v2 is in-tree data.
 */
static int
cmp_pwuid(const void *v1, const void *v2)
{
    const struct cache_item *ci1 = (const struct cache_item *) v1;
    const struct cache_item *ci2 = (const struct cache_item *) v2;
    if (ci1->k.uid == ci2->k.uid)
	return strcmp(ci1->registry, ci2->registry);
    if (ci1->k.uid < ci2->k.uid)
	return -1;
    return 1;
}

/*
 * Compare by user/group name.
 * v1 is the key to find or data to insert, v2 is in-tree data.
 */
static int
cmp_pwnam(const void *v1, const void *v2)
{
    const struct cache_item *ci1 = (const struct cache_item *) v1;
    const struct cache_item *ci2 = (const struct cache_item *) v2;
    int ret = strcmp(ci1->k.name, ci2->k.name);
    if (ret == 0)
	ret = strcmp(ci1->registry, ci2->registry);
    return ret;
}

/*
 * Compare by user name, taking into account the source type.
 * Need to differentiate between group-IDs received from the front-end
 * (via getgroups()) and groups IDs queried from the group database.
 * v1 is the key to find or data to insert, v2 is in-tree data.
 */
static int
cmp_gidlist(const void *v1, const void *v2)
{
    const struct cache_item *ci1 = (const struct cache_item *) v1;
    const struct cache_item *ci2 = (const struct cache_item *) v2;
    int ret = strcmp(ci1->k.name, ci2->k.name);
    if (ret == 0) {
	if (ci1->type == ENTRY_TYPE_ANY || ci1->type == ci2->type)
	    return strcmp(ci1->registry, ci2->registry);
	if (ci1->type < ci2->type)
	    return -1;
	return 1;
    }
    return ret;
}

void
sudo_pw_addref(struct passwd *pw)
{
    debug_decl(sudo_pw_addref, SUDOERS_DEBUG_NSS);
    ptr_to_item(pw)->refcnt++;
    debug_return;
}

static void
sudo_pw_delref_item(void *v)
{
    struct cache_item *item = v;
    debug_decl(sudo_pw_delref_item, SUDOERS_DEBUG_NSS);

    if (--item->refcnt == 0)
	free(item);

    debug_return;
}

void
sudo_pw_delref(struct passwd *pw)
{
    debug_decl(sudo_pw_delref, SUDOERS_DEBUG_NSS);
    sudo_pw_delref_item(ptr_to_item(pw));
    debug_return;
}

/*
 * Get a password entry by uid and allocate space for it.
 */
struct passwd *
sudo_getpwuid(uid_t uid)
{
    struct cache_item key, *item;
    struct rbnode *node;
    debug_decl(sudo_getpwuid, SUDOERS_DEBUG_NSS);

    if (pwcache_byuid == NULL) {
	pwcache_byuid = rbcreate(cmp_pwuid);
	if (pwcache_byuid == NULL) {
	    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	    debug_return_ptr(NULL);
	}
    }

    key.k.uid = uid;
    getauthregistry(IDtouser(uid), key.registry);
    if ((node = rbfind(pwcache_byuid, &key)) != NULL) {
	item = node->data;
	goto done;
    }
    /*
     * Cache passwd db entry if it exists or a negative response if not.
     */
#ifdef HAVE_SETAUTHDB
    aix_setauthdb(IDtouser(uid), key.registry);
#endif
    item = make_pwitem(uid, NULL);
#ifdef HAVE_SETAUTHDB
    aix_restoreauthdb();
#endif
    if (item == NULL) {
	if (errno != ENOENT || (item = calloc(1, sizeof(*item))) == NULL) {
	    sudo_warn(U_("unable to cache uid %u"), (unsigned int) uid);
	    /* cppcheck-suppress memleak */
	    debug_return_ptr(NULL);
	}
	item->refcnt = 1;
	item->k.uid = uid;
	/* item->d.pw = NULL; */
    }
    strlcpy(item->registry, key.registry, sizeof(item->registry));
    switch (rbinsert(pwcache_byuid, item, NULL)) {
    case 1:
	/* should not happen */
	sudo_warnx(U_("unable to cache uid %u, already exists"),
	    (unsigned int) uid);
	item->refcnt = 0;
	break;
    case -1:
	/* can't cache item, just return it */
	sudo_warn(U_("unable to cache uid %u"), (unsigned int) uid);
	item->refcnt = 0;
	break;
    }
done:
    if (item->refcnt != 0) {
	sudo_debug_printf(SUDO_DEBUG_DEBUG,
	    "%s: uid %u [%s] -> user %s [%s] (%s)", __func__,
	    (unsigned int)uid, key.registry,
	    item->d.pw ? item->d.pw->pw_name : "unknown",
	    item->registry, node ? "cache hit" : "cached");
    }
    if (item->d.pw != NULL)
	item->refcnt++;
    debug_return_ptr(item->d.pw);
}

/*
 * Get a password entry by name and allocate space for it.
 */
struct passwd *
sudo_getpwnam(const char *name)
{
    struct cache_item key, *item;
    struct rbnode *node;
    debug_decl(sudo_getpwnam, SUDOERS_DEBUG_NSS);

    if (pwcache_byname == NULL) {
	pwcache_byname = rbcreate(cmp_pwnam);
	if (pwcache_byname == NULL) {
	    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	    debug_return_ptr(NULL);
	}
    }

    key.k.name = (char *) name;
    getauthregistry((char *) name, key.registry);
    if ((node = rbfind(pwcache_byname, &key)) != NULL) {
	item = node->data;
	goto done;
    }
    /*
     * Cache passwd db entry if it exists or a negative response if not.
     */
#ifdef HAVE_SETAUTHDB
    aix_setauthdb((char *) name, key.registry);
#endif
    item = make_pwitem((uid_t)-1, name);
#ifdef HAVE_SETAUTHDB
    aix_restoreauthdb();
#endif
    if (item == NULL) {
	const size_t len = strlen(name) + 1;
	if (errno != ENOENT || (item = calloc(1, sizeof(*item) + len)) == NULL) {
	    sudo_warn(U_("unable to cache user %s"), name);
	    /* cppcheck-suppress memleak */
	    debug_return_ptr(NULL);
	}
	item->refcnt = 1;
	item->k.name = (char *) item + sizeof(*item);
	memcpy(item->k.name, name, len);
	/* item->d.pw = NULL; */
    }
    strlcpy(item->registry, key.registry, sizeof(item->registry));
    switch (rbinsert(pwcache_byname, item, NULL)) {
    case 1:
	/* should not happen */
	sudo_warnx(U_("unable to cache user %s, already exists"), name);
	item->refcnt = 0;
	break;
    case -1:
	/* can't cache item, just return it */
	sudo_warn(U_("unable to cache user %s"), name);
	item->refcnt = 0;
	break;
    }
done:
    if (item->refcnt != 0) {
	sudo_debug_printf(SUDO_DEBUG_DEBUG,
	    "%s: user %s [%s] -> uid %d [%s] (%s)", __func__, name,
	    key.registry, item->d.pw ? (int)item->d.pw->pw_uid : -1,
	    item->registry, node ? "cache hit" : "cached");
    }
    if (item->d.pw != NULL)
	item->refcnt++;
    debug_return_ptr(item->d.pw);
}

/*
 * Take a user, uid, gid, home and shell and return a faked up passwd struct.
 * If home or shell are NULL default values will be used.
 */
struct passwd *
sudo_mkpwent(const char *user, uid_t uid, gid_t gid, const char *home,
    const char *shell)
{
    struct cache_item_pw *pwitem;
    struct cache_item *item;
    struct passwd *pw;
    size_t len, name_len, home_len, shell_len;
    unsigned int i;
    debug_decl(sudo_mkpwent, SUDOERS_DEBUG_NSS);

    if (pwcache_byuid == NULL)
	pwcache_byuid = rbcreate(cmp_pwuid);
    if (pwcache_byname == NULL)
	pwcache_byname = rbcreate(cmp_pwnam);
    if (pwcache_byuid == NULL || pwcache_byname == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	debug_return_ptr(NULL);
    }

    /* Optional arguments. */
    if (home == NULL)
	home = "/";
    if (shell == NULL)
	shell = _PATH_BSHELL;

    sudo_debug_printf(SUDO_DEBUG_DEBUG,
	"%s: creating and caching passwd struct for %s:%u:%u:%s:%s", __func__,
	user, (unsigned int)uid, (unsigned int)gid, home, shell);

    name_len = strlen(user);
    home_len = strlen(home);
    shell_len = strlen(shell);
    len = sizeof(*pwitem) + name_len + 1 /* pw_name */ +
	sizeof("*") /* pw_passwd */ + sizeof("") /* pw_gecos */ +
	home_len + 1 /* pw_dir */ + shell_len + 1 /* pw_shell */;

    for (i = 0; i < 2; i++) {
	struct rbtree *pwcache;
	struct rbnode *node;

	pwitem = calloc(1, len);
	if (pwitem == NULL) {
	    sudo_warn(U_("unable to cache user %s"), user);
	    debug_return_ptr(NULL);
	}
	pw = &pwitem->pw;
	pw->pw_uid = uid;
	pw->pw_gid = gid;
	pw->pw_name = (char *)(pwitem + 1);
	memcpy(pw->pw_name, user, name_len + 1);
	pw->pw_passwd = pw->pw_name + name_len + 1;
	memcpy(pw->pw_passwd, "*", 2);
	pw->pw_gecos = pw->pw_passwd + 2;
	pw->pw_gecos[0] = '\0';
	pw->pw_dir = pw->pw_gecos + 1;
	memcpy(pw->pw_dir, home, home_len + 1);
	pw->pw_shell = pw->pw_dir + home_len + 1;
	memcpy(pw->pw_shell, shell, shell_len + 1);

	item = &pwitem->cache;
	item->refcnt = 1;
	item->d.pw = pw;
	if (i == 0) {
	    /* Store by uid. */
	    item->k.uid = pw->pw_uid;
	    pwcache = pwcache_byuid;
	} else {
	    /* Store by name. */
	    item->k.name = pw->pw_name;
	    pwcache = pwcache_byname;
	}
	getauthregistry(NULL, item->registry);
	switch (rbinsert(pwcache, item, &node)) {
	case 1:
	    /* Already exists. */
	    item = node->data;
	    if (item->d.pw == NULL) {
		/* Negative cache entry, replace with ours. */
		sudo_pw_delref_item(item);
		item = node->data = &pwitem->cache;
	    } else {
		/* Good entry, discard our fake one. */
		free(pwitem);
	    }
	    break;
	case -1:
	    /* can't cache item, just return it */
	    sudo_warn(U_("unable to cache user %s"), user);
	    item->refcnt = 0;
	    break;
	}
    }
    item->refcnt++;
    debug_return_ptr(item->d.pw);
}

/*
 * Take a uid in string form "#123" and return a faked up passwd struct.
 */
struct passwd *
sudo_fakepwnam(const char *user, gid_t gid)
{
    const char *errstr;
    uid_t uid;
    debug_decl(sudo_fakepwnam, SUDOERS_DEBUG_NSS);

    uid = (uid_t) sudo_strtoid(user + 1, &errstr);
    if (errstr != NULL) {
	sudo_debug_printf(SUDO_DEBUG_DIAG|SUDO_DEBUG_LINENO,
	    "uid %s %s", user, errstr);
	debug_return_ptr(NULL);
    }
    debug_return_ptr(sudo_mkpwent(user, uid, gid, NULL, NULL));
}

void
sudo_freepwcache(void)
{
    debug_decl(sudo_freepwcache, SUDOERS_DEBUG_NSS);

    if (pwcache_byuid != NULL) {
	rbdestroy(pwcache_byuid, sudo_pw_delref_item);
	pwcache_byuid = NULL;
    }
    if (pwcache_byname != NULL) {
	rbdestroy(pwcache_byname, sudo_pw_delref_item);
	pwcache_byname = NULL;
    }

    debug_return;
}

/*
 * Compare by group-ID.
 * v1 is the key to find or data to insert, v2 is in-tree data.
 */
static int
cmp_grgid(const void *v1, const void *v2)
{
    const struct cache_item *ci1 = (const struct cache_item *) v1;
    const struct cache_item *ci2 = (const struct cache_item *) v2;
    if (ci1->k.gid == ci2->k.gid)
	return strcmp(ci1->registry, ci2->registry);
    if (ci1->k.gid < ci2->k.gid)
	return -1;
    return 1;
}

void
sudo_gr_addref(struct group *gr)
{
    debug_decl(sudo_gr_addref, SUDOERS_DEBUG_NSS);
    ptr_to_item(gr)->refcnt++;
    debug_return;
}

static void
sudo_gr_delref_item(void *v)
{
    struct cache_item *item = v;
    debug_decl(sudo_gr_delref_item, SUDOERS_DEBUG_NSS);

    if (--item->refcnt == 0)
	free(item);

    debug_return;
}

void
sudo_gr_delref(struct group *gr)
{
    debug_decl(sudo_gr_delref, SUDOERS_DEBUG_NSS);
    sudo_gr_delref_item(ptr_to_item(gr));
    debug_return;
}

/*
 * Get a group entry by gid and allocate space for it.
 */
struct group *
sudo_getgrgid(gid_t gid)
{
    struct cache_item key, *item;
    struct rbnode *node;
    debug_decl(sudo_getgrgid, SUDOERS_DEBUG_NSS);

    if (grcache_bygid == NULL) {
	grcache_bygid = rbcreate(cmp_grgid);
	if (grcache_bygid == NULL) {
	    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	    debug_return_ptr(NULL);
	}
    }

    key.k.gid = gid;
    getauthregistry(NULL, key.registry);
    if ((node = rbfind(grcache_bygid, &key)) != NULL) {
	item = node->data;
	goto done;
    }
    /*
     * Cache group db entry if it exists or a negative response if not.
     */
    item = make_gritem(gid, NULL);
    if (item == NULL) {
	if (errno != ENOENT || (item = calloc(1, sizeof(*item))) == NULL) {
	    sudo_warn(U_("unable to cache gid %u"), (unsigned int) gid);
	    /* cppcheck-suppress memleak */
	    debug_return_ptr(NULL);
	}
	item->refcnt = 1;
	item->k.gid = gid;
	/* item->d.gr = NULL; */
    }
    strlcpy(item->registry, key.registry, sizeof(item->registry));
    switch (rbinsert(grcache_bygid, item, NULL)) {
    case 1:
	/* should not happen */
	sudo_warnx(U_("unable to cache gid %u, already exists"),
	    (unsigned int) gid);
	item->refcnt = 0;
	break;
    case -1:
	/* can't cache item, just return it */
	sudo_warn(U_("unable to cache gid %u"), (unsigned int) gid);
	item->refcnt = 0;
	break;
    }
done:
    if (item->refcnt != 0) {
	sudo_debug_printf(SUDO_DEBUG_DEBUG,
	    "%s: gid %u [%s] -> group %s [%s] (%s)", __func__,
	    (unsigned int)gid, key.registry,
	    item->d.gr ? item->d.gr->gr_name : "unknown",
	    item->registry, node ? "cache hit" : "cached");
    }
    if (item->d.gr != NULL)
	item->refcnt++;
    debug_return_ptr(item->d.gr);
}

/*
 * Get a group entry by name and allocate space for it.
 */
struct group *
sudo_getgrnam(const char *name)
{
    struct cache_item key, *item;
    struct rbnode *node;
    debug_decl(sudo_getgrnam, SUDOERS_DEBUG_NSS);

    if (grcache_byname == NULL) {
	grcache_byname = rbcreate(cmp_grnam);
	if (grcache_byname == NULL) {
	    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	    debug_return_ptr(NULL);
	}
    }

    key.k.name = (char *) name;
    getauthregistry(NULL, key.registry);
    if ((node = rbfind(grcache_byname, &key)) != NULL) {
	item = node->data;
	goto done;
    }
    /*
     * Cache group db entry if it exists or a negative response if not.
     */
    item = make_gritem((gid_t)-1, name);
    if (item == NULL) {
	const size_t len = strlen(name) + 1;
	if (errno != ENOENT || (item = calloc(1, sizeof(*item) + len)) == NULL) {
	    sudo_warn(U_("unable to cache group %s"), name);
	    /* cppcheck-suppress memleak */
	    debug_return_ptr(NULL);
	}
	item->refcnt = 1;
	item->k.name = (char *) item + sizeof(*item);
	memcpy(item->k.name, name, len);
	/* item->d.gr = NULL; */
    }
    strlcpy(item->registry, key.registry, sizeof(item->registry));
    switch (rbinsert(grcache_byname, item, NULL)) {
    case 1:
	/* should not happen */
	sudo_warnx(U_("unable to cache group %s, already exists"), name);
	item->refcnt = 0;
	break;
    case -1:
	/* can't cache item, just return it */
	sudo_warn(U_("unable to cache group %s"), name);
	item->refcnt = 0;
	break;
    }
done:
    if (item->refcnt != 0) {
	sudo_debug_printf(SUDO_DEBUG_DEBUG,
	    "%s: group %s [%s] -> gid %d [%s] (%s)", __func__, name,
	    key.registry, item->d.gr ? (int)item->d.gr->gr_gid : -1,
	    item->registry, node ? "cache hit" : "cached");
    }
    if (item->d.gr != NULL)
	item->refcnt++;
    debug_return_ptr(item->d.gr);
}

/*
 * Take a group name, ID, members and return a faked up group struct.
 */
struct group *
sudo_mkgrent(const char *group, gid_t gid, ...)
{
    struct cache_item_gr *gritem;
    struct cache_item *item;
    struct group *gr;
    size_t nmem, nsize, total;
    char *cp, *mem;
    va_list ap;
    unsigned int i;
    debug_decl(sudo_mkgrent, SUDOERS_DEBUG_NSS);

    if (grcache_bygid == NULL)
	grcache_bygid = rbcreate(cmp_grgid);
    if (grcache_byname == NULL)
	grcache_byname = rbcreate(cmp_grnam);
    if (grcache_bygid == NULL || grcache_byname == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	debug_return_ptr(NULL);
    }

    /* Allocate in one big chunk for easy freeing. */
    nsize = strlen(group) + 1;
    total = sizeof(*gritem) + nsize;
    va_start(ap, gid);
    for (nmem = 1; (mem = va_arg(ap, char *)) != NULL; nmem++) {
	total += strlen(mem) + 1;
    }
    va_end(ap);
    total += sizeof(char *) * nmem;

    for (i = 0; i < 2; i++) {
	struct rbtree *grcache;
	struct rbnode *node;

	/*
	 * Fill in group contents and make strings relative to space
	 * at the end of the buffer.  Note that gr_mem must come
	 * immediately after struct group to guarantee proper alignment.
	 */
	gritem = calloc(1, total);
	if (gritem == NULL) {
	    sudo_warn(U_("unable to cache group %s"), group);
	    debug_return_ptr(NULL);
	}
	gr = &gritem->gr;
	gr->gr_gid = gid;
	gr->gr_passwd = (char *)"*";
	cp = (char *)(gritem + 1);
	gr->gr_mem = (char **)cp;
	cp += sizeof(char *) * nmem;
	va_start(ap, gid);
	for (nmem = 0; (mem = va_arg(ap, char *)) != NULL; nmem++) {
	    size_t len = strlen(mem) + 1;
	    memcpy(cp, mem, len);
	    gr->gr_mem[nmem] = cp;
	    cp += len;
	}
	va_end(ap);
	gr->gr_mem[nmem] = NULL;
	gr->gr_name = cp;
	memcpy(gr->gr_name, group, nsize);

	item = &gritem->cache;
	item->refcnt = 1;
	item->d.gr = gr;
	if (i == 0) {
	    /* Store by gid if it doesn't already exist. */
	    item->k.gid = gr->gr_gid;
	    grcache = grcache_bygid;
	} else {
	    /* Store by name, overwriting cached version. */
	    gritem->cache.k.name = gr->gr_name;
	    grcache = grcache_byname;
	}
	getauthregistry(NULL, item->registry);
	switch (rbinsert(grcache, item, &node)) {
	case 1:
	    /* Already exists. */
	    item = node->data;
	    if (item->d.gr == NULL) {
		/* Negative cache entry, replace with ours. */
		sudo_gr_delref_item(item);
		item = node->data = &gritem->cache;
	    } else {
		/* Good entry, discard our fake one. */
		free(gritem);
	    }
	    break;
	case -1:
	    /* can't cache item, just return it */
	    sudo_warn(U_("unable to cache group %s"), group);
	    item->refcnt = 0;
	    break;
	}
    }
    if (item->d.gr != NULL)
	item->refcnt++;
    debug_return_ptr(item->d.gr);
}

/*
 * Take a gid in string form "#123" and return a faked up group struct.
 */
struct group *
sudo_fakegrnam(const char *group)
{
    const char *errstr;
    gid_t gid;
    debug_decl(sudo_fakegrnam, SUDOERS_DEBUG_NSS);

    gid = (gid_t) sudo_strtoid(group + 1, &errstr);
    if (errstr != NULL) {
	sudo_debug_printf(SUDO_DEBUG_DIAG|SUDO_DEBUG_LINENO,
	    "gid %s %s", group, errstr);
	debug_return_ptr(NULL);
    }

    debug_return_ptr(sudo_mkgrent(group, gid, (char *)NULL));
}

void
sudo_gidlist_addref(struct gid_list *gidlist)
{
    debug_decl(sudo_gidlist_addref, SUDOERS_DEBUG_NSS);
    ptr_to_item(gidlist)->refcnt++;
    debug_return;
}

static void
sudo_gidlist_delref_item(void *v)
{
    struct cache_item *item = v;
    debug_decl(sudo_gidlist_delref_item, SUDOERS_DEBUG_NSS);

    if (--item->refcnt == 0)
	free(item);

    debug_return;
}

void
sudo_gidlist_delref(struct gid_list *gidlist)
{
    debug_decl(sudo_gidlist_delref, SUDOERS_DEBUG_NSS);
    sudo_gidlist_delref_item(ptr_to_item(gidlist));
    debug_return;
}

void
sudo_grlist_addref(struct group_list *grlist)
{
    debug_decl(sudo_grlist_addref, SUDOERS_DEBUG_NSS);
    ptr_to_item(grlist)->refcnt++;
    debug_return;
}

static void
sudo_grlist_delref_item(void *v)
{
    struct cache_item *item = v;
    debug_decl(sudo_grlist_delref_item, SUDOERS_DEBUG_NSS);

    if (--item->refcnt == 0)
	free(item);

    debug_return;
}

void
sudo_grlist_delref(struct group_list *grlist)
{
    debug_decl(sudo_grlist_delref, SUDOERS_DEBUG_NSS);
    sudo_grlist_delref_item(ptr_to_item(grlist));
    debug_return;
}

void
sudo_freegrcache(void)
{
    debug_decl(sudo_freegrcache, SUDOERS_DEBUG_NSS);

    if (grcache_bygid != NULL) {
	rbdestroy(grcache_bygid, sudo_gr_delref_item);
	grcache_bygid = NULL;
    }
    if (grcache_byname != NULL) {
	rbdestroy(grcache_byname, sudo_gr_delref_item);
	grcache_byname = NULL;
    }
    if (grlist_cache != NULL) {
	rbdestroy(grlist_cache, sudo_grlist_delref_item);
	grlist_cache = NULL;
    }
    if (gidlist_cache != NULL) {
	rbdestroy(gidlist_cache, sudo_gidlist_delref_item);
	gidlist_cache = NULL;
    }

    debug_return;
}

struct group_list *
sudo_get_grlist(const struct passwd *pw)
{
    struct cache_item key, *item;
    struct rbnode *node;
    debug_decl(sudo_get_grlist, SUDOERS_DEBUG_NSS);

    sudo_debug_printf(SUDO_DEBUG_DEBUG, "%s: looking up group names for %s",
	__func__, pw->pw_name);

    if (grlist_cache == NULL) {
	grlist_cache = rbcreate(cmp_pwnam);
	if (grlist_cache == NULL) {
	    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	    debug_return_ptr(NULL);
	}
    }

    key.k.name = pw->pw_name;
    getauthregistry(pw->pw_name, key.registry);
    if ((node = rbfind(grlist_cache, &key)) != NULL) {
	item = node->data;
	goto done;
    }
    /*
     * Cache group db entry if it exists or a negative response if not.
     */
    item = make_grlist_item(pw, NULL);
    if (item == NULL) {
	/* Out of memory? */
	debug_return_ptr(NULL);
    }
    strlcpy(item->registry, key.registry, sizeof(item->registry));
    switch (rbinsert(grlist_cache, item, NULL)) {
    case 1:
	/* should not happen */
	sudo_warnx(U_("unable to cache group list for %s, already exists"),
	    pw->pw_name);
	item->refcnt = 0;
	break;
    case -1:
	/* can't cache item, just return it */
	sudo_warn(U_("unable to cache group list for %s"), pw->pw_name);
	item->refcnt = 0;
	break;
    }
    if (item->d.grlist != NULL) {
	int i;
	for (i = 0; i < item->d.grlist->ngroups; i++) {
	    sudo_debug_printf(SUDO_DEBUG_DEBUG,
		"%s: user %s is a member of group %s", __func__,
		pw->pw_name, item->d.grlist->groups[i]);
	}
    }
done:
    if (item->d.grlist != NULL)
	item->refcnt++;
    debug_return_ptr(item->d.grlist);
}

static void
sudo_debug_group_list(const char *user, char * const *groups,
    unsigned int level)
{
    size_t i, len = 0;
    debug_decl(sudo_debug_group_list, SUDOERS_DEBUG_NSS);

    if (groups == NULL || !sudo_debug_needed(level))
	debug_return;

    for (i = 0; groups[i] != NULL; i++) {
	len += strlen(groups[i]) + 1;
    }
    if (len != 0) {
	char *groupstr = malloc(len);
	if (groupstr != NULL) {
	    char *cp = groupstr;
	    for (i = 0; groups[i] != NULL; i++) {
		size_t n = (size_t)snprintf(cp, len, "%s%s", i ? "," : "",
		    groups[i]);
		if (n >= len)
		    break;
		cp += n;
		len -= n;
	    }
	    sudo_debug_printf(level, "%s: %s", user, groupstr);
	    free(groupstr);
	}
    }
    debug_return;
}

int
sudo_set_grlist(struct passwd *pw, char * const *groups)
{
    struct cache_item key, *item;
    debug_decl(sudo_set_grlist, SUDOERS_DEBUG_NSS);

    sudo_debug_printf(SUDO_DEBUG_DEBUG, "%s: setting group names for %s",
	__func__, pw->pw_name);

    sudo_debug_group_list(pw->pw_name, groups, SUDO_DEBUG_DEBUG);

    if (grlist_cache == NULL) {
	grlist_cache = rbcreate(cmp_pwnam);
	if (grlist_cache == NULL) {
	    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	    debug_return_int(-1);
	}
    }

    /*
     * Cache group db entry if it doesn't already exist
     */
    key.k.name = pw->pw_name;
    getauthregistry(pw->pw_name, key.registry);
    if (rbfind(grlist_cache, &key) == NULL) {
	if ((item = make_grlist_item(pw, groups)) == NULL) {
	    sudo_warnx(U_("unable to parse groups for %s"), pw->pw_name);
	    debug_return_int(-1);
	}
	strlcpy(item->registry, key.registry, sizeof(item->registry));
	switch (rbinsert(grlist_cache, item, NULL)) {
	case 1:
	    sudo_warnx(U_("unable to cache group list for %s, already exists"),
		pw->pw_name);
	    sudo_grlist_delref_item(item);
	    break;
	case -1:
	    sudo_warn(U_("unable to cache group list for %s"), pw->pw_name);
	    sudo_grlist_delref_item(item);
	    debug_return_int(-1);
	}
    } else {
	sudo_debug_printf(SUDO_DEBUG_WARN|SUDO_DEBUG_LINENO,
	    "groups for user %s are already cached", pw->pw_name);
    }

    debug_return_int(0);
}

struct gid_list *
sudo_get_gidlist(const struct passwd *pw, unsigned int type)
{
    struct cache_item key, *item;
    struct rbnode *node;
    debug_decl(sudo_get_gidlist, SUDOERS_DEBUG_NSS);

    sudo_debug_printf(SUDO_DEBUG_DEBUG, "%s: looking up group-IDs for %s",
	__func__, pw->pw_name);

    if (gidlist_cache == NULL) {
	gidlist_cache = rbcreate(cmp_gidlist);
	if (gidlist_cache == NULL) {
	    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	    debug_return_ptr(NULL);
	}
    }

    key.k.name = pw->pw_name;
    key.type = type;
    getauthregistry(pw->pw_name, key.registry);
    if ((node = rbfind(gidlist_cache, &key)) != NULL) {
	item = node->data;
	goto done;
    }
    /*
     * Cache group db entry if it exists or a negative response if not.
     */
    item = make_gidlist_item(pw, -1, NULL, NULL, type);
    if (item == NULL) {
	/* Out of memory? */
	debug_return_ptr(NULL);
    }
    strlcpy(item->registry, key.registry, sizeof(item->registry));
    switch (rbinsert(gidlist_cache, item, NULL)) {
    case 1:
	/* should not happen */
	sudo_warnx(U_("unable to cache group list for %s, already exists"),
	    pw->pw_name);
	item->refcnt = 0;
	break;
    case -1:
	/* can't cache item, just return it */
	sudo_warn(U_("unable to cache group list for %s"), pw->pw_name);
	item->refcnt = 0;
	break;
    }
    if (item->d.gidlist != NULL) {
	int i;
	for (i = 0; i < item->d.gidlist->ngids; i++) {
	    sudo_debug_printf(SUDO_DEBUG_DEBUG,
		"%s: user %s has supplementary gid %u", __func__,
		pw->pw_name, (unsigned int)item->d.gidlist->gids[i]);
	}
    }
done:
    if (item->d.gidlist != NULL)
	item->refcnt++;
    debug_return_ptr(item->d.gidlist);
}

int
sudo_set_gidlist(struct passwd *pw, int ngids, GETGROUPS_T *gids,
    char * const *gidstrs, unsigned int type)
{
    struct cache_item key, *item;
    debug_decl(sudo_set_gidlist, SUDOERS_DEBUG_NSS);

    sudo_debug_printf(SUDO_DEBUG_DEBUG, "%s: setting group-IDs for %s",
	__func__, pw->pw_name);

    /* XXX - ngids/gids too */
    sudo_debug_group_list(pw->pw_name, gidstrs, SUDO_DEBUG_DEBUG);

    if (gidlist_cache == NULL) {
	gidlist_cache = rbcreate(cmp_gidlist);
	if (gidlist_cache == NULL) {
	    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	    debug_return_int(-1);
	}
    }

    /*
     * Cache group db entry if it doesn't already exist
     */
    key.k.name = pw->pw_name;
    key.type = type;
    getauthregistry(pw->pw_name, key.registry);
    if (rbfind(gidlist_cache, &key) == NULL) {
	if ((item = make_gidlist_item(pw, ngids, gids, gidstrs, type)) == NULL) {
	    sudo_warnx(U_("unable to parse gids for %s"), pw->pw_name);
	    debug_return_int(-1);
	}
	strlcpy(item->registry, key.registry, sizeof(item->registry));
	switch (rbinsert(gidlist_cache, item, NULL)) {
	case 1:
	    sudo_warnx(U_("unable to cache group list for %s, already exists"),
		pw->pw_name);
	    sudo_gidlist_delref_item(item);
	    break;
	case -1:
	    sudo_warn(U_("unable to cache group list for %s"), pw->pw_name);
	    sudo_gidlist_delref_item(item);
	    debug_return_int(-1);
	}
    } else {
	sudo_debug_printf(SUDO_DEBUG_WARN|SUDO_DEBUG_LINENO,
	    "gids for user %s are already cached", pw->pw_name);
    }

    debug_return_int(0);
}

bool
user_in_group(const struct passwd *pw, const char *group)
{
    struct group_list *grlist = NULL;
    struct gid_list *gidlist = NULL;
    struct group *grp = NULL;
    bool matched = false;
    int i;
    debug_decl(user_in_group, SUDOERS_DEBUG_NSS);

    /*
     * If it could be a sudo-style group-ID check gids first.
     */
    if (group[0] == '#') {
	const char *errstr;
	gid_t gid = (gid_t) sudo_strtoid(group + 1, &errstr);
	if (errstr != NULL) {
	    sudo_debug_printf(SUDO_DEBUG_DIAG|SUDO_DEBUG_LINENO,
		"gid %s %s", group, errstr);
	} else {
	    if (gid == pw->pw_gid) {
		matched = true;
		goto done;
	    }
	    if ((gidlist = sudo_get_gidlist(pw, ENTRY_TYPE_ANY)) != NULL) {
		for (i = 0; i < gidlist->ngids; i++) {
		    if (gid == gidlist->gids[i]) {
			matched = true;
			goto done;
		    }
		}
	    }
	}
    }

    /*
     * Next match the group name.  By default, sudoers resolves all the user's
     * group-IDs to names and matches by name.  If match_group_by_gid is
     * set, each group is sudoers is resolved and matching is by group-ID.
     */
    if (def_match_group_by_gid) {
	gid_t gid;

	/* Look up the ID of the group in sudoers. */
	if ((grp = sudo_getgrnam(group)) == NULL)
	    goto done;
	gid = grp->gr_gid;

	/* Check against user's primary (passwd file) group-ID. */
	if (gid == pw->pw_gid) {
	    matched = true;
	    goto done;
	}

	/* Check the supplementary group vector. */
	if (gidlist == NULL) {
	    if ((gidlist = sudo_get_gidlist(pw, ENTRY_TYPE_ANY)) != NULL) {
		for (i = 0; i < gidlist->ngids; i++) {
		    if (gid == gidlist->gids[i]) {
			matched = true;
			goto done;
		    }
		}
	    }
	}
    } else if ((grlist = sudo_get_grlist(pw)) != NULL) {
	int (*compare)(const char *, const char *);
	if (def_case_insensitive_group)
	    compare = strcasecmp;
	else
	    compare = strcmp;

	/* Check the user's group vector, which includes the primary group. */
	for (i = 0; i < grlist->ngroups; i++) {
	    if (compare(group, grlist->groups[i]) == 0) {
		matched = true;
		goto done;
	    }
	}
    }

done:
    if (grp != NULL)
	sudo_gr_delref(grp);
    if (grlist != NULL)
	sudo_grlist_delref(grlist);
    if (gidlist != NULL)
	sudo_gidlist_delref(gidlist);

    sudo_debug_printf(SUDO_DEBUG_DEBUG, "%s: user %s %sin group %s",
	__func__, pw->pw_name, matched ? "" : "NOT ", group);
    debug_return_bool(matched);
}

/*
 * Returns true if the user's shell is considered to be valid.
 */
bool
user_shell_valid(const struct passwd *pw)
{
    debug_decl(user_shell_valid, SUDOERS_DEBUG_NSS);

    if (!def_runas_check_shell)
	debug_return_bool(true);

    debug_return_bool(valid_shell(pw->pw_shell));
}
