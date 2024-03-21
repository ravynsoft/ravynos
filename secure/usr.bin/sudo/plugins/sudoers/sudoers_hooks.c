/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2000-2005, 2007-2019
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sudoers.h>

/*
 * Similar to setenv(3) but operates on a private copy of the environment.
 * Does not include warnings or debugging to avoid recursive calls.
 */
static int
sudo_setenv_nodebug(const char *var, const char *val, int overwrite)
{
    char *ep, *estring = NULL;
    const char *cp;
    size_t esize;
    int ret = -1;

    if (var == NULL || *var == '\0') {
	errno = EINVAL;
	goto done;
    }

    /*
     * POSIX says a var name with '=' is an error but BSD
     * just ignores the '=' and anything after it.
     */
    for (cp = var; *cp && *cp != '='; cp++)
	continue;
    esize = (size_t)(cp - var) + 2;
    if (val) {
	esize += strlen(val);	/* glibc treats a NULL val as "" */
    }

    /* Allocate and fill in estring. */
    if ((estring = ep = malloc(esize)) == NULL)
	goto done;
    for (cp = var; *cp && *cp != '='; cp++)
	*ep++ = *cp;
    *ep++ = '=';
    if (val) {
	for (cp = val; *cp; cp++)
	    *ep++ = *cp;
    }
    *ep = '\0';

    ret = sudo_putenv_nodebug(estring, true, overwrite);
done:
    if (ret == -1)
	free(estring);
    else
	sudoers_gc_add(GC_PTR, estring);
    return ret;
}

int
sudoers_hook_getenv(const char *name, char **value, void *closure)
{
    static bool in_progress = false; /* avoid recursion */

    if (in_progress || env_get() == NULL)
	return SUDO_HOOK_RET_NEXT;

    in_progress = true;

    /* Hack to make GNU gettext() find the sudoers locale when needed. */
    if (*name == 'L' && sudoers_getlocale() == SUDOERS_LOCALE_SUDOERS) {
	if (strcmp(name, "LANGUAGE") == 0 || strcmp(name, "LANG") == 0) {
	    *value = NULL;
	    goto done;
	}
	if (strcmp(name, "LC_ALL") == 0 || strcmp(name, "LC_MESSAGES") == 0) {
	    *value = def_sudoers_locale;
	    goto done;
	}
    }

    *value = sudo_getenv_nodebug(name);
done:
    in_progress = false;
    return SUDO_HOOK_RET_STOP;
}

int
sudoers_hook_putenv(char *string, void *closure)
{
    static bool in_progress = false; /* avoid recursion */

    if (in_progress || env_get() == NULL)
	return SUDO_HOOK_RET_NEXT;

    in_progress = true;
    sudo_putenv_nodebug(string, true, true);
    in_progress = false;
    return SUDO_HOOK_RET_STOP;
}

int
sudoers_hook_setenv(const char *name, const char *value, int overwrite, void *closure)
{
    static bool in_progress = false; /* avoid recursion */

    if (in_progress || env_get() == NULL)
	return SUDO_HOOK_RET_NEXT;

    in_progress = true;
    sudo_setenv_nodebug(name, value, overwrite);
    in_progress = false;
    return SUDO_HOOK_RET_STOP;
}

int
sudoers_hook_unsetenv(const char *name, void *closure)
{
    static bool in_progress = false; /* avoid recursion */

    if (in_progress || env_get() == NULL)
	return SUDO_HOOK_RET_NEXT;

    in_progress = true;
    sudo_unsetenv_nodebug(name);
    in_progress = false;
    return SUDO_HOOK_RET_STOP;
}
