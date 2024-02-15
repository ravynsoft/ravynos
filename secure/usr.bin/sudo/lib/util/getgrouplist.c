/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2010, 2011, 2013-2021
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

/*
 * This is an open source non-commercial project. Dear PVS-Studio, please check it.
 * PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
 */

#include <config.h>

#include <stdlib.h>
#include <string.h>
#include <grp.h>
#include <limits.h>
#include <unistd.h>
#ifdef HAVE_NSS_SEARCH
# include <errno.h>
# include <limits.h>
# include <nsswitch.h>
# ifdef HAVE_NSS_DBDEFS_H
#  include <nss_dbdefs.h>
# else
#  include <compat/nss_dbdefs.h>
# endif
#endif

#include <sudo_compat.h>
#include <sudo_debug.h>
#include <sudo_util.h>

#ifndef HAVE_GETGROUPLIST
int
sudo_getgrouplist(const char *name, GETGROUPS_T basegid, GETGROUPS_T *groups,
    int *ngroupsp)
{
    return sudo_getgrouplist2(name, basegid, &groups, ngroupsp);
}
#endif /* HAVE_GETGROUPLIST */

#if defined(HAVE_GETGROUPLIST)

#if defined(HAVE_GETGROUPLIST_2) && !HAVE_DECL_GETGROUPLIST_2
int getgrouplist_2(const char *name, GETGROUPS_T basegid, GETGROUPS_T **groups);
#endif /* HAVE_GETGROUPLIST_2 && !HAVE_DECL_GETGROUPLIST_2 */

/*
 * Extended getgrouplist(3) using getgrouplist(3) and getgrouplist_2(3)
 */
int
sudo_getgrouplist2_v1(const char *name, GETGROUPS_T basegid,
    GETGROUPS_T **groupsp, int *ngroupsp)
{
#ifdef __APPLE__
    int *groups = (int *)*groupsp;
#else
    GETGROUPS_T *groups = *groupsp;
#endif
    int ngroups;
#ifndef HAVE_GETGROUPLIST_2
    long grpsize;
    int tries;
#endif
    debug_decl(sudo_getgrouplist2, SUDO_DEBUG_UTIL);

    /* For static group vector, just use getgrouplist(3). */
    if (groups != NULL)
	debug_return_int(getgrouplist(name, basegid, groups, ngroupsp));

#ifdef HAVE_GETGROUPLIST_2
    if ((ngroups = getgrouplist_2(name, basegid, groupsp)) == -1)
	debug_return_int(-1);
    *ngroupsp = ngroups;
    debug_return_int(0);
#else
    grpsize = sysconf(_SC_NGROUPS_MAX);
    if (grpsize < 0 || grpsize > INT_MAX)
	grpsize = NGROUPS_MAX;
    grpsize++;	/* include space for the primary gid */
    /*
     * It is possible to belong to more groups in the group database
     * than NGROUPS_MAX.
     */
    for (tries = 0; tries < 10; tries++) {
	free(groups);
	groups = reallocarray(NULL, (size_t)grpsize, sizeof(*groups));
	if (groups == NULL)
	    debug_return_int(-1);
	ngroups = (int)grpsize;
	if (getgrouplist(name, basegid, groups, &ngroups) != -1) {
	    *groupsp = groups;
	    *ngroupsp = ngroups;
	    debug_return_int(0);
	}
	if (ngroups == grpsize) {
	    /* Failed for some reason other than ngroups too small. */
	    break;
	}
	/* getgrouplist(3) set ngroups to the required length, use it. */
	grpsize = ngroups;
    }
    free(groups);
    debug_return_int(-1);
#endif /* HAVE_GETGROUPLIST_2 */
}

#elif defined(HAVE_GETGRSET)

/*
 * Extended getgrouplist(3) using AIX getgrset(3)
 */
int
sudo_getgrouplist2_v1(const char *name, GETGROUPS_T basegid,
    GETGROUPS_T **groupsp, int *ngroupsp)
{
    GETGROUPS_T *groups = *groupsp;
    char *cp, *last, *grset = NULL;
    const char *errstr;
    int ngroups = 1;
    int grpsize = *ngroupsp;
    int ret = -1;
    gid_t gid;
    debug_decl(sudo_getgrouplist2, SUDO_DEBUG_UTIL);

#ifdef HAVE_SETAUTHDB
    aix_setauthdb((char *) name, NULL);
#endif
    if ((grset = getgrset(name)) == NULL)
	goto done;

    if (groups == NULL) {
	/* Dynamically-sized group vector, count groups and alloc. */
	grpsize = 1;	/* reserve one for basegid */
	if (*grset != '\0') {
	    grpsize++;	/* at least one supplementary group */
	    for (cp = grset; *cp != '\0'; cp++) {
		if (*cp == ',')
		    grpsize++;
	    }
	}
	groups = reallocarray(NULL, grpsize, sizeof(*groups));
	if (groups == NULL)
	    debug_return_int(-1);
    } else {
	/* Static group vector. */
	if (grpsize < 1)
	    debug_return_int(-1);
    }

    /* We support BSD semantics where the first element is the base gid */
    groups[0] = basegid;

    for (cp = strtok_r(grset, ",", &last); cp != NULL; cp = strtok_r(NULL, ",", &last)) {
	gid = sudo_strtoid(cp, &errstr);
	if (errstr == NULL && gid != basegid) {
	    if (ngroups == grpsize)
		goto done;
	    groups[ngroups++] = gid;
	}
    }
    ret = 0;

done:
    free(grset);
#ifdef HAVE_SETAUTHDB
    aix_restoreauthdb();
#endif
    *groupsp = groups;
    *ngroupsp = ngroups;

    debug_return_int(ret);
}

#elif defined(HAVE_NSS_SEARCH)

#ifndef ALIGNBYTES
# define ALIGNBYTES	(sizeof(long) - 1L)
#endif
#ifndef ALIGN
# define ALIGN(p)	(((unsigned long)(p) + ALIGNBYTES) & ~ALIGNBYTES)
#endif

#if defined(HAVE__NSS_INITF_GROUP) || defined(HAVE___NSS_INITF_GROUP)
extern void _nss_initf_group(nss_db_params_t *params);
#else
static void
_nss_initf_group(nss_db_params_t *params)
{
    params->name = NSS_DBNAM_GROUP;
    params->default_config = NSS_DEFCONF_GROUP;
}
#endif

/*
 * Convert a groups file string (instr) to a struct group (ent) using
 * buf for storage.  
 */
static int
str2grp(const char *instr, int inlen, void *ent, char *buf, int buflen)
{
    struct group *grp = ent;
    char *cp, *fieldsep = buf;
    char **gr_mem, **gr_end;
    const char *errstr;
    int yp = 0;
    id_t id;
    debug_decl(str2grp, SUDO_DEBUG_UTIL);

    /* Must at least have space to copy instr -> buf. */
    if (inlen >= buflen)
	debug_return_int(NSS_STR_PARSE_ERANGE);
    
    /* Paranoia: buf and instr should be distinct. */
    if (buf != instr) {
	memmove(buf, instr, inlen);
	buf[inlen] = '\0';
    }

    if ((fieldsep = strchr(cp = fieldsep, ':')) == NULL)
	debug_return_int(NSS_STR_PARSE_PARSE);
    *fieldsep++ = '\0';
    grp->gr_name = cp;

    /* Check for YP inclusion/exclusion entries. */
    if (*cp == '+' || *cp == '-') {
	/* Only the name is required for YP inclusion/exclusion entries. */
	grp->gr_passwd = (char *)"";
	grp->gr_gid = 0;
	grp->gr_mem = NULL;
	yp = 1;
    }

    if ((fieldsep = strchr(cp = fieldsep, ':')) == NULL)
	debug_return_int(yp ? NSS_STR_PARSE_SUCCESS : NSS_STR_PARSE_PARSE);
    *fieldsep++ = '\0';
    grp->gr_passwd = cp;

    if ((fieldsep = strchr(cp = fieldsep, ':')) == NULL)
	debug_return_int(yp ? NSS_STR_PARSE_SUCCESS : NSS_STR_PARSE_PARSE);
    *fieldsep++ = '\0';
    id = sudo_strtoid(cp, &errstr);
    if (errstr != NULL) {
	/*
	 * A range error is always a fatal error, but ignore garbage
	 * at the end of YP entries since it has no meaning.
	 */
	if (errno == ERANGE)
	    debug_return_int(NSS_STR_PARSE_ERANGE);
	debug_return_int(yp ? NSS_STR_PARSE_SUCCESS : NSS_STR_PARSE_PARSE);
    }
#ifdef GID_NOBODY
    /* Negative gids get mapped to nobody on Solaris. */
    if (*cp == '-' && id != 0)
	grp->gr_gid = GID_NOBODY;
    else
#endif
	grp->gr_gid = (gid_t)id;

    /* Store group members, taking care to use proper alignment. */
    grp->gr_mem = NULL;
    if (*fieldsep != '\0') {
	grp->gr_mem = gr_mem = (char **)ALIGN(buf + inlen + 1);
	gr_end = (char **)((unsigned long)(buf + buflen) & ~ALIGNBYTES) - 1;
	for (;;) {
	    if (gr_mem >= gr_end)
		debug_return_int(NSS_STR_PARSE_ERANGE);	/* out of space! */
	    *gr_mem++ = cp;
	    if (fieldsep == NULL)
		break;
	    if ((fieldsep = strchr(cp = fieldsep, ',')) != NULL)
		*fieldsep++ = '\0';
	}
	*gr_mem = NULL;
    }
    debug_return_int(NSS_STR_PARSE_SUCCESS);
}

static nss_status_t
process_cstr(const char *instr, int inlen, struct nss_groupsbymem *gbm,
    int dynamic)
{
    const char *user = gbm->username;
    nss_status_t ret = NSS_NOTFOUND;
    nss_XbyY_buf_t *buf;
    struct group *grp;
    char **gr_mem;
    int	error, i;
    debug_decl(process_cstr, SUDO_DEBUG_UTIL);

    sudo_debug_printf(SUDO_DEBUG_INFO, "%s: parsing %.*s", __func__,
	inlen, instr);

    /* Hack to let us check whether the query was handled by nscd or us. */
    if (gbm->force_slow_way != 0)
	gbm->force_slow_way = 2;

    buf = _nss_XbyY_buf_alloc(sizeof(struct group), NSS_BUFLEN_GROUP);
    if (buf == NULL)
	debug_return_int(NSS_UNAVAIL);

    /* Parse groups file string -> struct group. */
    grp = buf->result;
    error = (*gbm->str2ent)(instr, inlen, grp, buf->buffer, buf->buflen);
    if (error != NSS_STR_PARSE_SUCCESS || grp->gr_mem == NULL)
	goto done;

    for (gr_mem = grp->gr_mem; *gr_mem != NULL; gr_mem++) {
	if (strcmp(*gr_mem, user) == 0) {
	    const int numgids = MIN(gbm->numgids, gbm->maxgids);

	    /* Append to gid_array unless gr_gid is a dupe. */
	    for (i = 0; i < numgids; i++) {
		if (gbm->gid_array[i] == grp->gr_gid)
		    goto done;			/* already present */
	    }
	    if (i == gbm->maxgids && dynamic) {
		GETGROUPS_T *tmp = reallocarray(gbm->gid_array, gbm->maxgids,
		    2 * sizeof(GETGROUPS_T));
		if (tmp == NULL) {
		    /* Out of memory, just return what we have. */
		    dynamic = 0;
		} else {
		    gbm->gid_array = tmp;
		    gbm->maxgids <<= 1;
		}
	    }
	    /* Store gid if there is space. */
	    if (i < gbm->maxgids)
		gbm->gid_array[i] = grp->gr_gid;
	    /* Always increment numgids so we can detect when out of space. */
	    gbm->numgids++;
	    goto done;
	}
    }
done:
    _nss_XbyY_buf_free(buf);
    debug_return_int(ret);
}

static nss_status_t
process_cstr_static(const char *instr, int inlen, struct nss_groupsbymem *gbm)
{
    return process_cstr(instr, inlen, gbm, 0);
}

static nss_status_t
process_cstr_dynamic(const char *instr, int inlen, struct nss_groupsbymem *gbm)
{
    return process_cstr(instr, inlen, gbm, 1);
}

/*
 * Extended getgrouplist(3) using nss_search(3)
 */
int
sudo_getgrouplist2_v1(const char *name, GETGROUPS_T basegid,
    GETGROUPS_T **groupsp, int *ngroupsp)
{
    struct nss_groupsbymem gbm;
    static DEFINE_NSS_DB_ROOT(db_root);
    debug_decl(sudo_getgrouplist2, SUDO_DEBUG_UTIL);

    memset(&gbm, 0, sizeof(gbm));
    gbm.username = name;
    gbm.gid_array = *groupsp;
    gbm.maxgids = *ngroupsp;
    gbm.numgids = 1; /* for basegid */
    gbm.force_slow_way = 1;
    gbm.str2ent = str2grp;

    if (gbm.gid_array == NULL) {
	/* Dynamically-sized group vector. */
	gbm.maxgids = (int)sysconf(_SC_NGROUPS_MAX);
	if (gbm.maxgids < 0)
	    gbm.maxgids = NGROUPS_MAX;
	gbm.gid_array = reallocarray(NULL, gbm.maxgids, 4 * sizeof(GETGROUPS_T));
	if (gbm.gid_array == NULL)
	    debug_return_int(-1);
	gbm.maxgids <<= 2;
	gbm.process_cstr = process_cstr_dynamic;
    } else {
	/* Static group vector. */
	if (gbm.maxgids <= 0)
	    debug_return_int(-1);
	gbm.process_cstr = process_cstr_static;
    }

    /* We support BSD semantics where the first element is the base gid */
    gbm.gid_array[0] = basegid;

    /*
     * Can't use nss_search return value since it may return NSS_UNAVAIL
     * when no nsswitch.conf entry (e.g. compat mode).
     */
    for (;;) {
	GETGROUPS_T *tmp;

	(void)nss_search(&db_root, _nss_initf_group, NSS_DBOP_GROUP_BYMEMBER,
	    &gbm);

	/*
	 * If this was a statically-sized group vector or nscd was not used
	 * we are done.
	 */
	if (gbm.process_cstr != process_cstr_dynamic || gbm.force_slow_way == 2)
	    break;

	/*
	 * If gid_array is full and the query was handled by nscd, there
	 * may be more data, so double gid_array and try again.
	 */
	if (gbm.numgids != gbm.maxgids)
	    break;

	tmp = reallocarray(gbm.gid_array, gbm.maxgids, 2 * sizeof(GETGROUPS_T));
	if (tmp == NULL) {
	    free(gbm.gid_array);
	    debug_return_int(-1);
	}
	gbm.gid_array = tmp;
	gbm.maxgids <<= 1;
    }

    /* Note: we can only detect a too-small group list if nscd is not used. */
    *groupsp = gbm.gid_array;
    if (gbm.numgids <= gbm.maxgids) {
        *ngroupsp = gbm.numgids;
	debug_return_int(0);
    }
    *ngroupsp = gbm.maxgids;
    debug_return_int(-1);
}

#else /* !HAVE_GETGROUPLIST && !HAVE_GETGRSET && !HAVE__GETGROUPSBYMEMBER */

/*
 * Extended getgrouplist(3) using getgrent(3)
 */
int
sudo_getgrouplist2_v1(const char *name, GETGROUPS_T basegid,
    GETGROUPS_T **groupsp, int *ngroupsp)
{
    GETGROUPS_T *groups = *groupsp;
    int grpsize = *ngroupsp;
    int i, ngroups = 1;
    int ret = -1;
    struct group *grp;
    debug_decl(sudo_getgrouplist2, SUDO_DEBUG_UTIL);

    if (groups == NULL) {
	/* Dynamically-sized group vector. */
	grpsize = (int)sysconf(_SC_NGROUPS_MAX);
	if (grpsize < 0)
	    grpsize = NGROUPS_MAX;
	groups = reallocarray(NULL, grpsize, 4 * sizeof(*groups));
	if (groups == NULL)
	    debug_return_int(-1);
	grpsize <<= 2;
    } else {
	/* Static group vector. */
	if (grpsize < 1)
	    debug_return_int(-1);
    }

    /* We support BSD semantics where the first element is the base gid */
    groups[0] = basegid;

    setgrent();
    while ((grp = getgrent()) != NULL) {
	if (grp->gr_gid == basegid || grp->gr_mem == NULL)
	    continue;

	for (i = 0; grp->gr_mem[i] != NULL; i++) {
	    if (strcmp(name, grp->gr_mem[i]) == 0)
		break;
	}
	if (grp->gr_mem[i] == NULL)
	    continue; /* user not found */

	/* Only add if it is not the same as an existing gid */
	for (i = 0; i < ngroups; i++) {
	    if (grp->gr_gid == groups[i])
		break;
	}
	if (i == ngroups) {
	    if (ngroups == grpsize) {
		GETGROUPS_T *tmp;

		if (*groupsp != NULL) {
		    /* Static group vector. */
		    goto done;
		}
		tmp = reallocarray(groups, grpsize, 2 * sizeof(*groups));
		if (tmp == NULL) {
		    free(groups);
		    groups = NULL;
		    ngroups = 0;
		    goto done;
		}
		groups = tmp;
		grpsize <<= 1;
	    }
	    groups[ngroups++] = grp->gr_gid;
	}
    }
    ret = 0;

done:
    endgrent();
    *groupsp = groups;
    *ngroupsp = ngroups;

    debug_return_int(ret);
}
#endif /* !HAVE_GETGROUPLIST && !HAVE_GETGRSET && !HAVE__GETGROUPSBYMEMBER */
