/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 1996, 1998-2005, 2007-2019
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

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_STRINGS_H
# include <strings.h>
#endif /* HAVE_STRINGS_H */
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <pwd.h>
#include <grp.h>

#include <sudoers.h>
#include <cvtsudoers.h>
#include <pwutil.h>

#ifndef LOGIN_NAME_MAX
# ifdef _POSIX_LOGIN_NAME_MAX
#  define LOGIN_NAME_MAX _POSIX_LOGIN_NAME_MAX
# else
#  define LOGIN_NAME_MAX 9
# endif
#endif /* LOGIN_NAME_MAX */

#define FIELD_SIZE(src, name, size)			\
do {							\
	if ((src)->name) {				\
		size = strlen((src)->name) + 1;		\
		total += size;				\
	} else {                                        \
		size = 0;				\
	}                                               \
} while (0)

#define FIELD_COPY(src, dst, name, size)		\
do {							\
	if ((src)->name) {				\
		memcpy(cp, (src)->name, size);		\
		(dst)->name = cp;			\
		cp += size;				\
	}						\
} while (0)

/*
 * Dynamically allocate space for a struct item plus the key and data
 * elements.  If name is non-NULL it is used as the key, else the
 * uid is the key.  Fills in datum from the users filter.
 * Returns NULL on calloc error or unknown name/id, setting errno
 * to ENOMEM or ENOENT respectively.
 */
struct cache_item *
cvtsudoers_make_pwitem(uid_t uid, const char *name)
{
    char *cp, uidstr[STRLEN_MAX_UNSIGNED(uid_t) + 2];
    size_t nsize, psize, gsize, dsize, ssize, total;
#ifdef HAVE_LOGIN_CAP_H
    size_t csize;
#endif
    struct cache_item_pw *pwitem;
    struct passwd pw, *newpw;
    struct sudoers_string *s = NULL;
    debug_decl(cvtsudoers_make_pwitem, SUDOERS_DEBUG_NSS);

    /* Look up name or uid in filter list. */
    if (name != NULL) {
	STAILQ_FOREACH(s, &filters->users, entries) {
	    if (strcasecmp(name, s->str) == 0) {
		uid = (uid_t)-1;
		break;
	    }
	}
    } else {
	STAILQ_FOREACH(s, &filters->users, entries) {
	    const char *errstr;
	    uid_t filter_uid;

	    if (s->str[0] != '#')
		continue;

	    filter_uid = sudo_strtoid(s->str + 1, &errstr);
	    if (errstr == NULL) {
		if (uid != filter_uid)
		    continue;
		(void)snprintf(uidstr, sizeof(uidstr), "#%u",
		    (unsigned int)uid);
		break;
	    }
	}
    }
    if (s == NULL) {
	errno = ENOENT;
	debug_return_ptr(NULL);
    }

    /* Fake up a passwd struct. */
    memset(&pw, 0, sizeof(pw));
    pw.pw_name = name ? s->str : uidstr;
    pw.pw_passwd = (char *)"*";
    pw.pw_uid = uid;
    pw.pw_gid = (gid_t)-1;
    pw.pw_shell = (char *)_PATH_BSHELL;
    pw.pw_dir = (char *)"/";

    /* Allocate in one big chunk for easy freeing. */
    total = sizeof(*pwitem);
    FIELD_SIZE(&pw, pw_name, nsize);
    FIELD_SIZE(&pw, pw_passwd, psize);
#ifdef HAVE_LOGIN_CAP_H
    FIELD_SIZE(&pw, pw_class, csize);
#endif
    FIELD_SIZE(&pw, pw_gecos, gsize);
    FIELD_SIZE(&pw, pw_dir, dsize);
    FIELD_SIZE(&pw, pw_shell, ssize);
    if (name != NULL)
	total += strlen(name) + 1;

    /* Allocate space for struct item, struct passwd and the strings. */
    if ((pwitem = calloc(1, total)) == NULL) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
	    "unable to allocate memory");
	debug_return_ptr(NULL);
    }
    newpw = &pwitem->pw;

    /*
     * Copy in passwd contents and make strings relative to space
     * at the end of the struct.
     */
    memcpy(newpw, &pw, sizeof(pw));
    cp = (char *)(pwitem + 1);
    FIELD_COPY(&pw, newpw, pw_name, nsize);
    FIELD_COPY(&pw, newpw, pw_passwd, psize);
#ifdef HAVE_LOGIN_CAP_H
    FIELD_COPY(&pw, newpw, pw_class, csize);
#endif
    FIELD_COPY(&pw, newpw, pw_gecos, gsize);
    FIELD_COPY(&pw, newpw, pw_dir, dsize);
    FIELD_COPY(&pw, newpw, pw_shell, ssize);

    /* Set key and datum. */
    if (name != NULL) {
	memcpy(cp, name, strlen(name) + 1);
	pwitem->cache.k.name = cp;
    } else {
	pwitem->cache.k.uid = pw.pw_uid;
    }
    pwitem->cache.d.pw = newpw;
    pwitem->cache.refcnt = 1;

    debug_return_ptr(&pwitem->cache);
}

/*
 * Dynamically allocate space for a struct item plus the key and data
 * elements.  If name is non-NULL it is used as the key, else the
 * gid is the key.  Fills in datum from the groups filter.
 * Returns NULL on calloc error or unknown name/id, setting errno
 * to ENOMEM or ENOENT respectively.
 */
struct cache_item *
cvtsudoers_make_gritem(gid_t gid, const char *name)
{
    char *cp, gidstr[STRLEN_MAX_UNSIGNED(gid_t) + 2];
    size_t nsize, psize, total, len, nmem = 0;
    struct cache_item_gr *gritem;
    struct group gr, *newgr;
    struct sudoers_string *s = NULL;
    debug_decl(cvtsudoers_make_gritem, SUDOERS_DEBUG_NSS);

    /* Look up name or gid in filter list. */
    if (name != NULL) {
	STAILQ_FOREACH(s, &filters->groups, entries) {
	    if (strcasecmp(name, s->str) == 0) {
		gid = (gid_t)-1;
		break;
	    }
	}
    } else {
	STAILQ_FOREACH(s, &filters->groups, entries) {
	    const char *errstr;
	    gid_t filter_gid;

	    if (s->str[0] != '#')
		continue;

	    filter_gid = sudo_strtoid(s->str + 1, &errstr);
	    if (errstr == NULL) {
		if (gid != filter_gid)
		    continue;
		(void)snprintf(gidstr, sizeof(gidstr), "#%u",
		    (unsigned int)gid);
		break;
	    }
	}
    }
    if (s == NULL) {
	errno = ENOENT;
	debug_return_ptr(NULL);
    }

    /* Fake up a group struct with all filter users as members. */
    memset(&gr, 0, sizeof(gr));
    gr.gr_name = name ? s->str : gidstr;
    gr.gr_gid = gid;

    /* Allocate in one big chunk for easy freeing. */
    total = sizeof(*gritem);
    FIELD_SIZE(&gr, gr_name, nsize);
    FIELD_SIZE(&gr, gr_passwd, psize);
    if (!STAILQ_EMPTY(&filters->users)) {
	STAILQ_FOREACH(s, &filters->users, entries) {
	    total += strlen(s->str) + 1;
	    nmem++;
	}
	total += sizeof(char *) * nmem;
    }
    if (name != NULL)
	total += strlen(name) + 1;

    if ((gritem = calloc(1, total)) == NULL) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
	    "unable to allocate memory");
	debug_return_ptr(NULL);
    }

    /*
     * Copy in group contents and make strings relative to space
     * at the end of the buffer.  Note that gr_mem must come
     * immediately after struct group to guarantee proper alignment.
     */
    newgr = &gritem->gr;
    memcpy(newgr, &gr, sizeof(gr));
    cp = (char *)(gritem + 1);
    if (nmem != 0) {
	newgr->gr_mem = (char **)cp;
	cp += sizeof(char *) * nmem;
	nmem = 0;
	STAILQ_FOREACH(s, &filters->users, entries) {
	    len = strlen(s->str) + 1;
	    memcpy(cp, s->str, len);
	    newgr->gr_mem[nmem++] = cp;
	    cp += len;
	}
	newgr->gr_mem[nmem] = NULL;
    }
    FIELD_COPY(&gr, newgr, gr_passwd, psize);
    FIELD_COPY(&gr, newgr, gr_name, nsize);

    /* Set key and datum. */
    if (name != NULL) {
	memcpy(cp, name, strlen(name) + 1);
	gritem->cache.k.name = cp;
    } else {
	gritem->cache.k.gid = gr.gr_gid;
    }
    gritem->cache.d.gr = newgr;
    gritem->cache.refcnt = 1;

    debug_return_ptr(&gritem->cache);
}

static struct cache_item_gidlist *gidlist_item;

/*
 * Dynamically allocate space for a struct item plus the key and data
 * elements.  Fills in datum from the groups filter.
 */
struct cache_item *
cvtsudoers_make_gidlist_item(const struct passwd *pw, int unused1,
    GETGROUPS_T *unused2, char * const *unused3, unsigned int type)
{
    char *cp;
    size_t nsize, total;
    struct cache_item_gidlist *glitem;
    struct sudoers_string *s;
    struct gid_list *gidlist;
    GETGROUPS_T *gids = NULL;
    int i, ngids = 0;
    debug_decl(cvtsudoers_make_gidlist_item, SUDOERS_DEBUG_NSS);

    /*
     * There's only a single gid list.
     */
    if (gidlist_item != NULL) {
	gidlist_item->cache.refcnt++;
	debug_return_ptr(&gidlist_item->cache);
    }

    /* Count number of possible gids in the filter. */
    STAILQ_FOREACH(s, &filters->groups, entries) {
	if (s->str[0] == '#')
		ngids++;
    }

    /* Allocate gids[] array and fill it with parsed gids. */
    if (ngids != 0) {
	gids = reallocarray(NULL, (size_t)ngids, sizeof(GETGROUPS_T));
	if (gids == NULL) {
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		"unable to allocate memory");
	    debug_return_ptr(NULL);
	}
	ngids = 0;
	STAILQ_FOREACH(s, &filters->groups, entries) {
	    if (s->str[0] == '#') {
		const char *errstr;
		gid_t gid = sudo_strtoid(s->str + 1, &errstr);
		if (errstr == NULL) {
		    /* Valid gid. */
		    gids[ngids++] = gid;
		}
	    }
	}
    }
    if (ngids == 0) {
	free(gids);
	errno = ENOENT;
	debug_return_ptr(NULL);
    }

    /* Allocate in one big chunk for easy freeing. */
    nsize = strlen(pw->pw_name) + 1;
    total = sizeof(*glitem) + nsize;
    total += sizeof(gid_t *) * (size_t)ngids;

    if ((glitem = calloc(1, total)) == NULL) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
	    "unable to allocate memory");
	free(gids);
	debug_return_ptr(NULL);
    }

    /*
     * Copy in group list and make pointers relative to space
     * at the end of the buffer.  Note that the groups array must come
     * immediately after struct group to guarantee proper alignment.
     */
    gidlist = &glitem->gidlist;
    cp = (char *)(glitem + 1);
    gidlist->gids = (gid_t *)cp;
    cp += sizeof(gid_t) * (size_t)ngids;

    /* Set key and datum. */
    memcpy(cp, pw->pw_name, nsize);
    glitem->cache.k.name = cp;
    glitem->cache.d.gidlist = gidlist;
    glitem->cache.refcnt = 1;
    glitem->cache.type = type;

    /*
     * Store group IDs.
     */
    for (i = 0; i < ngids; i++)
	gidlist->gids[i] = gids[i];
    gidlist->ngids = ngids;
    free(gids);

    debug_return_ptr(&glitem->cache);
}

static struct cache_item_gidlist *grlist_item;

/*
 * Dynamically allocate space for a struct item plus the key and data
 * elements.  Fills in group names from the groups filter.
 */
struct cache_item *
cvtsudoers_make_grlist_item(const struct passwd *pw, char * const *unused1)
{
    char *cp;
    size_t nsize, ngroups, total, len;
    struct cache_item_grlist *grlitem;
    struct sudoers_string *s;
    struct group_list *grlist;
    size_t groupname_len;
    debug_decl(cvtsudoers_make_grlist_item, SUDOERS_DEBUG_NSS);

    /*
     * There's only a single group list.
     */
    if (grlist_item != NULL) {
	grlist_item->cache.refcnt++;
	debug_return_ptr(&grlist_item->cache);
    }

    /* Count number of groups in the filter. */
    ngroups = 0;
    STAILQ_FOREACH(s, &filters->groups, entries) {
	ngroups++;
    }

#ifdef _SC_LOGIN_NAME_MAX
    groupname_len = (size_t)MAX(sysconf(_SC_LOGIN_NAME_MAX), 32);
#else
    groupname_len = MAX(LOGIN_NAME_MAX, 32);
#endif

    /* Allocate in one big chunk for easy freeing. */
    nsize = strlen(pw->pw_name) + 1;
    total = sizeof(*grlitem) + nsize;
    total += groupname_len * ngroups;

again:
    if ((grlitem = calloc(1, total)) == NULL) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
	    "unable to allocate memory");
	debug_return_ptr(NULL);
    }

    /*
     * Copy in group list and make pointers relative to space
     * at the end of the buffer.  Note that the groups array must come
     * immediately after struct group to guarantee proper alignment.
     */
    grlist = &grlitem->grlist;
    cp = (char *)(grlitem + 1);
    grlist->groups = (char **)cp;
    cp += sizeof(char *) * ngroups;

    /* Set key and datum. */
    memcpy(cp, pw->pw_name, nsize);
    grlitem->cache.k.name = cp;
    grlitem->cache.d.grlist = grlist;
    grlitem->cache.refcnt = 1;
    cp += nsize;

    /*
     * Copy groups from filter.
     */
    ngroups = 0;
    STAILQ_FOREACH(s, &filters->groups, entries) {
	if (s->str[0] == '#') {
	    const char *errstr;
	    sudo_strtoid(s->str + 1, &errstr);
	    if (errstr == NULL) {
		/* Group ID not name, ignore it. */
		continue;
	    }
	}
	len = strlen(s->str) + 1;
	if ((size_t)(cp - (char *)grlitem) + len > total) {
	    total += len + groupname_len;
	    free(grlitem);
	    goto again;
	}
	memcpy(cp, s->str, len);
	grlist->groups[ngroups++] = cp;
	cp += len;
    }
    grlist->ngroups = (int)ngroups;

    debug_return_ptr(&grlitem->cache);
}
