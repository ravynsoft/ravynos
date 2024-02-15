/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2010, 2012-2016 Todd C. Miller <Todd.Miller@sudo.ws>
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
#include <errno.h>

#include <sudo.h>
#include <sudo_plugin.h>
#include <sudo_dso.h>

extern char **environ;		/* global environment pointer */
static char **priv_environ;	/* private environment pointer */

/*
 * NOTE: we don't use dlsym() to find the libc getenv()
 *	 since this may allocate memory on some systems (glibc)
 *	 which leads to a hang if malloc() calls getenv (jemalloc).
 */
char *
getenv_unhooked(const char *name)
{
    char **ep, *val = NULL;
    size_t namelen = 0;

    /* For BSD compatibility, treat '=' in name like end of string. */
    while (name[namelen] != '\0' && name[namelen] != '=')
	namelen++;
    for (ep = environ; *ep != NULL; ep++) {
	if (strncmp(*ep, name, namelen) == 0 && (*ep)[namelen] == '=') {
	    val = *ep + namelen + 1;
	    break;
	}
    }
    return val;
}

sudo_dso_public char *getenv(const char *name);

char *
getenv(const char *name)
{
    char *val = NULL;

    switch (process_hooks_getenv(name, &val)) {
	case SUDO_HOOK_RET_STOP:
	    return val;
	case SUDO_HOOK_RET_ERROR:
	    return NULL;
	default:
	    return getenv_unhooked(name);
    }
}

static int
rpl_putenv(PUTENV_CONST char *string)
{
    char **ep;
    const char *equal;
    size_t len;
    bool found = false;

    /* Some putenv(3) implementations check for NULL. */
    if (string == NULL) {
	errno = EINVAL;
	return -1;
    }

    /* The string must contain a '=' char but not start with one. */
    equal = strchr(string, '=');
    if (equal == NULL || equal == string) {
	errno = EINVAL;
	return -1;
    }

    /* Look for existing entry. */
    len = (size_t)(equal - string) + 1;
    for (ep = environ; *ep != NULL; ep++) {
	if (strncmp(string, *ep, len) == 0) {
	    *ep = (char *)string;
	    found = true;
	    break;
	}
    }
    /* Prune out duplicate variables. */
    if (found) {
	while (*ep != NULL) {
	    if (strncmp(string, *ep, len) == 0) {
		char **cur = ep;
		while ((*cur = *(cur + 1)) != NULL)
		    cur++;
	    } else {
		ep++;
	    }
	}
    }

    /* Append at the end if not already found. */
    if (!found) {
	size_t env_len = (size_t)(ep - environ);
	char **envp = reallocarray(priv_environ, env_len + 2, sizeof(char *));
	if (envp == NULL)
	    return -1;
	if (environ != priv_environ)
	    memcpy(envp, environ, env_len * sizeof(char *));
	envp[env_len++] = (char *)string;
	envp[env_len] = NULL;
	priv_environ = environ = envp;
    }
    return 0;
}

typedef int (*sudo_fn_putenv_t)(PUTENV_CONST char *);

static int
putenv_unhooked(PUTENV_CONST char *string)
{
    sudo_fn_putenv_t fn;

    fn = (sudo_fn_putenv_t)sudo_dso_findsym(SUDO_DSO_NEXT, "putenv");
    if (fn != NULL)
	return fn(string);
    return rpl_putenv(string);
}

sudo_dso_public int putenv(PUTENV_CONST char *string);

int
putenv(PUTENV_CONST char *string)
{
    switch (process_hooks_putenv((char *)string)) {
	case SUDO_HOOK_RET_STOP:
	    return 0;
	case SUDO_HOOK_RET_ERROR:
	    return -1;
	default:
	    return putenv_unhooked(string);
    }
}

static int
rpl_setenv(const char *var, const char *val, int overwrite)
{
    char *envstr, *dst;
    const char *src;
    size_t esize;

    if (!var || *var == '\0') {
	errno = EINVAL;
	return -1;
    }

    /*
     * POSIX says a var name with '=' is an error but BSD
     * just ignores the '=' and anything after it.
     */
    for (src = var; *src != '\0' && *src != '='; src++)
	continue;
    esize = (size_t)(src - var) + 2;
    if (val) {
        esize += strlen(val);	/* glibc treats a NULL val as "" */
    }

    /* Allocate and fill in envstr. */
    if ((envstr = malloc(esize)) == NULL)
	return -1;
    for (src = var, dst = envstr; *src != '\0' && *src != '=';)
	*dst++ = *src++;
    *dst++ = '=';
    if (val) {
	for (src = val; *src != '\0';)
	    *dst++ = *src++;
    }
    *dst = '\0';

    if (!overwrite && getenv(var) != NULL) {
	free(envstr);
	return 0;
    }
    if (rpl_putenv(envstr) == -1) {
	free(envstr);
	return -1;
    }
    return 0;
}

typedef int (*sudo_fn_setenv_t)(const char *, const char *, int);

static int
setenv_unhooked(const char *var, const char *val, int overwrite)
{
    sudo_fn_setenv_t fn;

    fn = (sudo_fn_setenv_t)sudo_dso_findsym(SUDO_DSO_NEXT, "setenv");
    if (fn != NULL)
	return fn(var, val, overwrite);
    return rpl_setenv(var, val, overwrite);
}

sudo_dso_public int setenv(const char *var, const char *val, int overwrite);

int
setenv(const char *var, const char *val, int overwrite)
{
    switch (process_hooks_setenv(var, val, overwrite)) {
	case SUDO_HOOK_RET_STOP:
	    return 0;
	case SUDO_HOOK_RET_ERROR:
	    return -1;
	default:
	    return setenv_unhooked(var, val, overwrite);
    }
}

static int
rpl_unsetenv(const char *var)
{
    char **ep = environ;
    size_t len;

    if (var == NULL || *var == '\0' || strchr(var, '=') != NULL) {
	errno = EINVAL;
	return -1;
    }

    len = strlen(var);
    while (*ep != NULL) {
	if (strncmp(var, *ep, len) == 0 && (*ep)[len] == '=') {
	    /* Found it; shift remainder + NULL over by one. */
	    char **cur = ep;
	    while ((*cur = *(cur + 1)) != NULL)
		cur++;
	    /* Keep going, could be multiple instances of the var. */
	} else {
	    ep++;
	}
    }
    return 0;
}

#ifdef UNSETENV_VOID
typedef void (*sudo_fn_unsetenv_t)(const char *);
#else
typedef int (*sudo_fn_unsetenv_t)(const char *);
#endif

static int
unsetenv_unhooked(const char *var)
{
    int ret = 0;
    sudo_fn_unsetenv_t fn;

    fn = (sudo_fn_unsetenv_t)sudo_dso_findsym(SUDO_DSO_NEXT, "unsetenv");
    if (fn != NULL) {
# ifdef UNSETENV_VOID
	fn(var);
# else
	ret = fn(var);
# endif
    } else {
	ret = rpl_unsetenv(var);
    }
    return ret;
}

#ifdef UNSETENV_VOID
# define UNSETENV_RTYPE	void
#else
# define UNSETENV_RTYPE	int
#endif

sudo_dso_public UNSETENV_RTYPE unsetenv(const char *var);

UNSETENV_RTYPE
unsetenv(const char *var)
{
    int ret;

    switch (process_hooks_unsetenv(var)) {
	case SUDO_HOOK_RET_STOP:
	    ret = 0;
	    break;
	case SUDO_HOOK_RET_ERROR:
	    ret = -1;
	    break;
	default:
	    ret = unsetenv_unhooked(var);
	    break;
    }
#ifndef UNSETENV_VOID
    return ret;
#endif
}
