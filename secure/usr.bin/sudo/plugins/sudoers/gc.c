/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2016 Todd C. Miller <Todd.Miller@sudo.ws>
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

#include <sudoers.h>

struct sudoers_gc_entry {
    SLIST_ENTRY(sudoers_gc_entry) entries;
    enum sudoers_gc_types type;
    union {
	char **vec;
	void *ptr;
    } u;
};
SLIST_HEAD(sudoers_gc_list, sudoers_gc_entry);
#ifdef NO_LEAKS
static struct sudoers_gc_list sudoers_gc_list =
    SLIST_HEAD_INITIALIZER(sudoers_gc_list);
#endif

bool
sudoers_gc_add(enum sudoers_gc_types type, void *v)
{
#ifdef NO_LEAKS
    struct sudoers_gc_entry *gc;
    debug_decl(sudoers_gc_add, SUDOERS_DEBUG_UTIL);

    if (v == NULL)
	debug_return_bool(false);

    gc = calloc(1, sizeof(*gc));
    if (gc == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	debug_return_bool(false);
    }
    switch (type) {
    case GC_PTR:
	gc->u.ptr = v;
	break;
    case GC_VECTOR:
	gc->u.vec = v;
	break;
    default:
	free(gc);
	sudo_warnx("unexpected garbage type %d", type);
	debug_return_bool(false);
    }
    gc->type = type;
    SLIST_INSERT_HEAD(&sudoers_gc_list, gc, entries);
    debug_return_bool(true);
#else
    return true;
#endif /* NO_LEAKS */
}

bool
sudoers_gc_remove(enum sudoers_gc_types type, void *v)
{
#ifdef NO_LEAKS
    struct sudoers_gc_entry *gc, *prev = NULL;
    debug_decl(sudoers_gc_remove, SUDOERS_DEBUG_UTIL);

    if (v == NULL)
	debug_return_bool(false);

    SLIST_FOREACH(gc, &sudoers_gc_list, entries) {
	switch (gc->type) {
	case GC_PTR:
	    if (gc->u.ptr == v)
	    	goto found;
	    break;
	case GC_VECTOR:
	    if (gc->u.vec == v)
	    	goto found;
	    break;
	default:
	    sudo_warnx("unexpected garbage type %d in %p", gc->type, gc);
	}
	prev = gc;
    }
    /* If this happens, there is a bug in the g/c code. */
    sudo_warnx("%s: unable to find %p, type %d", __func__, v, type);
#ifdef FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION
    abort();
#else
    debug_return_bool(false);
#endif
found:
    if (prev == NULL)
	SLIST_REMOVE_HEAD(&sudoers_gc_list, entries);
    else
	SLIST_REMOVE_AFTER(prev, entries);
    free(gc);

    debug_return_bool(true);
#else
    return true;
#endif /* NO_LEAKS */
}

void
sudoers_gc_run(void)
{
#ifdef NO_LEAKS
    struct sudoers_gc_entry *gc;
    char **cur;
    debug_decl(sudoers_gc_run, SUDOERS_DEBUG_UTIL);

    /* Collect garbage. */
    while ((gc = SLIST_FIRST(&sudoers_gc_list)) != NULL) {
	SLIST_REMOVE_HEAD(&sudoers_gc_list, entries);
	switch (gc->type) {
	case GC_PTR:
	    free(gc->u.ptr);
	    free(gc);
	    break;
	case GC_VECTOR:
	    for (cur = gc->u.vec; *cur != NULL; cur++)
		free(*cur);
	    free(gc->u.vec);
	    free(gc);
	    break;
	default:
	    sudo_warnx("unexpected garbage type %d", gc->type);
	}
    }

    debug_return;
#endif /* NO_LEAKS */
}
