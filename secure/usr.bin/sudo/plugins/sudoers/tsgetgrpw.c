/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2005, 2008, 2010-2015, 2022
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

/*
 * Trivial replacements for the libc getgrent() and getpwent() family
 * of functions for use by testsudoers in the sudo test harness.
 * We need our own since many platforms don't provide set{pw,gr}file().
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <limits.h>
#include <unistd.h>

#include <tsgetgrpw.h>
#include <sudoers.h>

#undef GRMEM_MAX
#define GRMEM_MAX 200

#ifndef UID_MAX
# define UID_MAX 0xffffffffU
#endif

#ifndef GID_MAX
# define GID_MAX UID_MAX
#endif

static FILE *pwf;
static const char *pwfile = "/etc/passwd";
static int pw_stayopen;

static FILE *grf;
static const char *grfile = "/etc/group";
static int gr_stayopen;

void
testsudoers_setpwfile(const char *file)
{
    pwfile = file;
    if (pwf != NULL)
	testsudoers_endpwent();
}

static int
open_passwd(int reset)
{
    if (pwf == NULL) {
	pwf = fopen(pwfile, "r");
	if (pwf != NULL) {
	    if (fcntl(fileno(pwf), F_SETFD, FD_CLOEXEC) == -1) {
		fclose(pwf);
		pwf = NULL;
	    }
	}
	if (pwf == NULL)
	    return 0;
    } else if (reset) {
	rewind(pwf);
    }
    return 1;
}

int
testsudoers_setpassent(int stayopen)
{
    if (!open_passwd(1))
	return 0;
    pw_stayopen = stayopen;
    return 1;
}

void
testsudoers_setpwent(void)
{
    testsudoers_setpassent(0);
}

void
testsudoers_endpwent(void)
{
    if (pwf != NULL) {
	fclose(pwf);
	pwf = NULL;
    }
    pw_stayopen = 0;
}

struct passwd *
testsudoers_getpwent(void)
{
    static struct passwd pw;
    static char pwbuf[LINE_MAX];
    size_t len;
    id_t id;
    char *cp, *colon;
    const char *errstr;

    if (!open_passwd(0))
	return NULL;

next_entry:
    if ((colon = fgets(pwbuf, sizeof(pwbuf), pwf)) == NULL)
	return NULL;

    memset(&pw, 0, sizeof(pw));
    if ((colon = strchr(cp = colon, ':')) == NULL)
	goto next_entry;
    *colon++ = '\0';
    pw.pw_name = cp;
    if ((colon = strchr(cp = colon, ':')) == NULL)
	goto next_entry;
    *colon++ = '\0';
    pw.pw_passwd = cp;
    if ((colon = strchr(cp = colon, ':')) == NULL)
	goto next_entry;
    *colon++ = '\0';
    id = sudo_strtoid(cp, &errstr);
    if (errstr != NULL)
	goto next_entry;
    pw.pw_uid = (uid_t)id;
    if ((colon = strchr(cp = colon, ':')) == NULL)
	goto next_entry;
    *colon++ = '\0';
    id = sudo_strtoid(cp, &errstr);
    if (errstr != NULL)
	goto next_entry;
    pw.pw_gid = (gid_t)id;
    if ((colon = strchr(cp = colon, ':')) == NULL)
	goto next_entry;
    *colon++ = '\0';
    pw.pw_gecos = cp;
    if ((colon = strchr(cp = colon, ':')) == NULL)
	goto next_entry;
    *colon++ = '\0';
    pw.pw_dir = cp;
    pw.pw_shell = colon;
    len = strlen(colon);
    if (len > 0 && colon[len - 1] == '\n')
	colon[len - 1] = '\0';
    return &pw;
}

struct passwd *
testsudoers_getpwnam(const char *name)
{
    struct passwd *pw;

    if (!open_passwd(1))
	return NULL;
    while ((pw = testsudoers_getpwent()) != NULL) {
	if (strcmp(pw->pw_name, name) == 0)
	    break;
    }
    if (!pw_stayopen) {
	fclose(pwf);
	pwf = NULL;
    }
    return pw;
}

struct passwd *
testsudoers_getpwuid(uid_t uid)
{
    struct passwd *pw;

    if (!open_passwd(1))
	return NULL;
    while ((pw = testsudoers_getpwent()) != NULL) {
	if (pw->pw_uid == uid)
	    break;
    }
    if (!pw_stayopen) {
	fclose(pwf);
	pwf = NULL;
    }
    return pw;
}

void
testsudoers_setgrfile(const char *file)
{
    grfile = file;
    if (grf != NULL)
	testsudoers_endgrent();
}

static int
open_group(int reset)
{
    if (grf == NULL) {
	grf = fopen(grfile, "r");
	if (grf != NULL) {
	    if (fcntl(fileno(grf), F_SETFD, FD_CLOEXEC) == -1) {
		fclose(grf);
		grf = NULL;
	    }
	}
	if (grf == NULL)
	    return 0;
    } else if (reset) {
	rewind(grf);
    }
    return 1;
}

int
testsudoers_setgroupent(int stayopen)
{
    if (!open_group(1))
	return 0;
    gr_stayopen = stayopen;
    return 1;
}

void
testsudoers_setgrent(void)
{
    testsudoers_setgroupent(0);
}

void
testsudoers_endgrent(void)
{
    if (grf != NULL) {
	fclose(grf);
	grf = NULL;
    }
    gr_stayopen = 0;
}

struct group *
testsudoers_getgrent(void)
{
    static struct group gr;
    static char grbuf[LINE_MAX], *gr_mem[GRMEM_MAX+1];
    size_t len;
    id_t id;
    char *cp, *colon;
    const char *errstr;
    int n;

    if (!open_group(0))
	return NULL;

next_entry:
    if ((colon = fgets(grbuf, sizeof(grbuf), grf)) == NULL)
	return NULL;

    memset(&gr, 0, sizeof(gr));
    if ((colon = strchr(cp = colon, ':')) == NULL)
	goto next_entry;
    *colon++ = '\0';
    gr.gr_name = cp;
    if ((colon = strchr(cp = colon, ':')) == NULL)
	goto next_entry;
    *colon++ = '\0';
    gr.gr_passwd = cp;
    if ((colon = strchr(cp = colon, ':')) == NULL)
	goto next_entry;
    *colon++ = '\0';
    id = sudo_strtoid(cp, &errstr);
    if (errstr != NULL)
	goto next_entry;
    gr.gr_gid = (gid_t)id;
    len = strlen(colon);
    if (len > 0 && colon[len - 1] == '\n')
	colon[len - 1] = '\0';
    if (*colon != '\0') {
	char *last;

	gr.gr_mem = gr_mem;
	cp = strtok_r(colon, ",", &last);
	for (n = 0; cp != NULL && n < GRMEM_MAX; n++) {
	    gr.gr_mem[n] = cp;
	    cp = strtok_r(NULL, ",", &last);
	}
	gr.gr_mem[n] = NULL;
    } else
	gr.gr_mem = NULL;
    return &gr;
}

struct group *
testsudoers_getgrnam(const char *name)
{
    struct group *gr;

    if (!open_group(1))
	return NULL;
    while ((gr = testsudoers_getgrent()) != NULL) {
	if (strcmp(gr->gr_name, name) == 0)
	    break;
    }
    if (!gr_stayopen) {
	fclose(grf);
	grf = NULL;
    }
    return gr;
}

struct group *
testsudoers_getgrgid(gid_t gid)
{
    struct group *gr;

    if (!open_group(1))
	return NULL;
    while ((gr = testsudoers_getgrent()) != NULL) {
	if (gr->gr_gid == gid)
	    break;
    }
    if (!gr_stayopen) {
	fclose(grf);
	grf = NULL;
    }
    return gr;
}

/*
 * Copied from getgrouplist.c
 */
int
testsudoers_getgrouplist2(const char *name, GETGROUPS_T basegid,
    GETGROUPS_T **groupsp, int *ngroupsp)
{
    GETGROUPS_T *groups = *groupsp;
    int i, ngroups = 1;
    long grpsize;
    int ret = -1;
    struct group *grp;

    if (groups == NULL) {
	/* Dynamically-sized group vector. */
	grpsize = sysconf(_SC_NGROUPS_MAX);
	if (grpsize < 0)
	    grpsize = NGROUPS_MAX;
	groups = reallocarray(NULL, (size_t)grpsize, 4 * sizeof(*groups));
	if (groups == NULL)
	    return -1;
	grpsize <<= 2;
    } else {
	/* Static group vector. */
	if ((grpsize = *ngroupsp) < 1)
	    return -1;
    }

    /* We support BSD semantics where the first element is the base gid */
    groups[0] = basegid;

    testsudoers_setgrent();
    while ((grp = testsudoers_getgrent()) != NULL) {
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
		tmp = reallocarray(groups, (size_t)grpsize, 2 * sizeof(*groups));
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
    testsudoers_endgrent();
    *groupsp = groups;
    *ngroupsp = ngroups;

    return ret;
}
