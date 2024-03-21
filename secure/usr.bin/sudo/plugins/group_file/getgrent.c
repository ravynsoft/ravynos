/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2005,2008,2010-2015,2022 Todd C. Miller <Todd.Miller@sudo.ws>
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
 * Trivial replacements for the libc getgrent() family of functions.
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <limits.h>
#include <grp.h>

#include <sudo_compat.h>
#include <sudo_util.h>

#undef GRMEM_MAX
#define GRMEM_MAX 200

static FILE *grf;
static const char *grfile = "/etc/group";
static int gr_stayopen;

void mysetgrfile(const char *);
void mysetgrent(void);
void myendgrent(void);
int mysetgroupent(int);
struct group *mygetgrent(void);
struct group *mygetgrnam(const char *);
struct group *mygetgrgid(gid_t);

void
mysetgrfile(const char *file)
{
    grfile = file;
    if (grf != NULL)
	myendgrent();
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
mysetgroupent(int stayopen)
{
    if (!open_group(1))
	return 0;
    gr_stayopen = stayopen;
    return 1;
}

void
mysetgrent(void)
{
    mysetgroupent(0);
}

void
myendgrent(void)
{
    if (grf != NULL) {
	fclose(grf);
	grf = NULL;
    }
    gr_stayopen = 0;
}

struct group *
mygetgrent(void)
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
mygetgrnam(const char *name)
{
    struct group *gr;

    if (!open_group(1))
	return NULL;
    while ((gr = mygetgrent()) != NULL) {
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
mygetgrgid(gid_t gid)
{
    struct group *gr;

    if (!open_group(1))
	return NULL;
    while ((gr = mygetgrent()) != NULL) {
	if (gr->gr_gid == gid)
	    break;
    }
    if (!gr_stayopen) {
	fclose(grf);
	grf = NULL;
    }
    return gr;
}
