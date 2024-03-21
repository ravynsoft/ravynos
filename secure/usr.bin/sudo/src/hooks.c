/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2012-2016 Todd C. Miller <Todd.Miller@sudo.ws>
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
#include <sudo_plugin_int.h>

/* Singly linked hook list. */
struct sudo_hook_entry {
    SLIST_ENTRY(sudo_hook_entry) entries;
    union {
	sudo_hook_fn_t generic_fn;
	sudo_hook_fn_setenv_t setenv_fn;
	sudo_hook_fn_unsetenv_t unsetenv_fn;
	sudo_hook_fn_getenv_t getenv_fn;
	sudo_hook_fn_putenv_t putenv_fn;
    } u;
    void *closure;
};
SLIST_HEAD(sudo_hook_list, sudo_hook_entry);

/* Each hook type gets own hook list. */
static struct sudo_hook_list sudo_hook_setenv_list =
    SLIST_HEAD_INITIALIZER(sudo_hook_setenv_list);
static struct sudo_hook_list sudo_hook_unsetenv_list =
    SLIST_HEAD_INITIALIZER(sudo_hook_unsetenv_list);
static struct sudo_hook_list sudo_hook_getenv_list =
    SLIST_HEAD_INITIALIZER(sudo_hook_getenv_list);
static struct sudo_hook_list sudo_hook_putenv_list =
    SLIST_HEAD_INITIALIZER(sudo_hook_putenv_list);

/* NOTE: must not anything that might call setenv() */
int
process_hooks_setenv(const char *name, const char *value, int overwrite)
{
    struct sudo_hook_entry *hook;
    int rc = SUDO_HOOK_RET_NEXT;

    /* First process the hooks. */
    SLIST_FOREACH(hook, &sudo_hook_setenv_list, entries) {
	rc = hook->u.setenv_fn(name, value, overwrite, hook->closure);
	if (rc == SUDO_HOOK_RET_STOP || rc == SUDO_HOOK_RET_ERROR)
	    break;
    }
    return rc;
}

/* NOTE: must not anything that might call putenv() */
int
process_hooks_putenv(char *string)
{
    struct sudo_hook_entry *hook;
    int rc = SUDO_HOOK_RET_NEXT;

    /* First process the hooks. */
    SLIST_FOREACH(hook, &sudo_hook_putenv_list, entries) {
	rc = hook->u.putenv_fn(string, hook->closure);
	if (rc == SUDO_HOOK_RET_STOP || rc == SUDO_HOOK_RET_ERROR)
	    break;
    }
    return rc;
}

/* NOTE: must not anything that might call getenv() */
int
process_hooks_getenv(const char *name, char **value)
{
    struct sudo_hook_entry *hook;
    char *val = NULL;
    int rc = SUDO_HOOK_RET_NEXT;

    /* First process the hooks. */
    SLIST_FOREACH(hook, &sudo_hook_getenv_list, entries) {
	rc = hook->u.getenv_fn(name, &val, hook->closure);
	if (rc == SUDO_HOOK_RET_STOP || rc == SUDO_HOOK_RET_ERROR)
	    break;
    }
    if (val != NULL)
	*value = val;
    return rc;
}

/* NOTE: must not anything that might call unsetenv() */
int
process_hooks_unsetenv(const char *name)
{
    struct sudo_hook_entry *hook;
    int rc = SUDO_HOOK_RET_NEXT;

    /* First process the hooks. */
    SLIST_FOREACH(hook, &sudo_hook_unsetenv_list, entries) {
	rc = hook->u.unsetenv_fn(name, hook->closure);
	if (rc == SUDO_HOOK_RET_STOP || rc == SUDO_HOOK_RET_ERROR)
	    break;
    }
    return rc;
}

/* Hook registration internals. */
static int
register_hook_internal(struct sudo_hook_list *head,
    int (*hook_fn)(), void *closure)
{
    struct sudo_hook_entry *hook;
    debug_decl(register_hook_internal, SUDO_DEBUG_HOOKS);

    if ((hook = calloc(1, sizeof(*hook))) == NULL) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
	    "unable to allocate memory");
	debug_return_int(-1);
    }
    hook->u.generic_fn = hook_fn;
    hook->closure = closure;
    SLIST_INSERT_HEAD(head, hook, entries);

    debug_return_int(0);
}

/* Register the specified hook. */
int
register_hook(struct sudo_hook *hook)
{
    int ret;
    debug_decl(register_hook, SUDO_DEBUG_HOOKS);

    if (SUDO_API_VERSION_GET_MAJOR(hook->hook_version) != SUDO_HOOK_VERSION_MAJOR) {
	/* Major versions must match. */
	errno = EINVAL;
	ret = -1;
    } else {
	switch (hook->hook_type) {
	    case SUDO_HOOK_GETENV:
		ret = register_hook_internal(&sudo_hook_getenv_list,
		    hook->hook_fn, hook->closure);
		break;
	    case SUDO_HOOK_PUTENV:
		ret = register_hook_internal(&sudo_hook_putenv_list,
		    hook->hook_fn, hook->closure);
		break;
	    case SUDO_HOOK_SETENV:
		ret = register_hook_internal(&sudo_hook_setenv_list,
		    hook->hook_fn, hook->closure);
		break;
	    case SUDO_HOOK_UNSETENV:
		ret = register_hook_internal(&sudo_hook_unsetenv_list,
		    hook->hook_fn, hook->closure);
		break;
	    default:
		/* XXX - use define for unknown value */
		errno = ENOTSUP;
		ret = 1;
		break;
	}
    }

    debug_return_int(ret);
}

/* Hook deregistration internals. */
static void
deregister_hook_internal(struct sudo_hook_list *head,
    int (*hook_fn)(), void *closure)
{
    struct sudo_hook_entry *hook, *prev = NULL;
    debug_decl(deregister_hook_internal, SUDO_DEBUG_HOOKS);

    SLIST_FOREACH(hook, head, entries) {
	if (hook->u.generic_fn == hook_fn && hook->closure == closure) {
	    /* Remove from list and free. */
	    if (prev == NULL)
		SLIST_REMOVE_HEAD(head, entries);
	    else
		SLIST_REMOVE_AFTER(prev, entries);
	    free(hook);
	    break;
	}
	prev = hook;
    }

    debug_return;
}

/* Deregister the specified hook. */
int
deregister_hook(struct sudo_hook *hook)
{
    int ret = 0;
    debug_decl(deregister_hook, SUDO_DEBUG_HOOKS);

    if (SUDO_API_VERSION_GET_MAJOR(hook->hook_version) != SUDO_HOOK_VERSION_MAJOR) {
	/* Major versions must match. */
	ret = -1;
    } else {
	switch (hook->hook_type) {
	    case SUDO_HOOK_GETENV:
		deregister_hook_internal(&sudo_hook_getenv_list, hook->hook_fn,
		    hook->closure);
		break;
	    case SUDO_HOOK_PUTENV:
		deregister_hook_internal(&sudo_hook_putenv_list, hook->hook_fn,
		    hook->closure);
		break;
	    case SUDO_HOOK_SETENV:
		deregister_hook_internal(&sudo_hook_setenv_list, hook->hook_fn,
		    hook->closure);
		break;
	    case SUDO_HOOK_UNSETENV:
		deregister_hook_internal(&sudo_hook_unsetenv_list, hook->hook_fn,
		    hook->closure);
		break;
	    default:
		/* XXX - use define for unknown value */
		ret = 1;
		break;
	}
    }

    debug_return_int(ret);
}
