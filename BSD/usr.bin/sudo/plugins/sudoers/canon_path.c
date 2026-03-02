/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2023 Todd C. Miller <Todd.Miller@sudo.ws>
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

#include <sys/stat.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sudoers.h>
#include <redblack.h>

static struct rbtree *canon_cache;

/*
 * A cache_item includes storage for both the original path and the
 * resolved path.  The resolved path is directly embedded into the
 * struct so that we can find the start of the struct cache_item
 * given the value of resolved.  Storage for pathname is embedded
 * at the end with resolved.
 */
struct cache_item {
    unsigned int refcnt;
    char *pathname;
    char resolved[];
};

/*
 * Compare function for canon_cache.
 * v1 is the key to find or data to insert, v2 is in-tree data.
 */
static int
compare(const void *v1, const void *v2)
{
    const struct cache_item *ci1 = (const struct cache_item *)v1;
    const struct cache_item *ci2 = (const struct cache_item *)v2;
    return strcmp(ci1->pathname, ci2->pathname);
}

/* Convert a pointer returned by canon_path() to a struct cache_item *. */
#define resolved_to_item(_r) ((struct cache_item *)((_r) - offsetof(struct cache_item, resolved)))

/*
 * Delete a ref from item and free if the refcount reaches 0.
 */
static void
canon_path_free_item(void *v)
{
    struct cache_item *item = v;
    debug_decl(canon_path_free_item, SUDOERS_DEBUG_UTIL);

    if (--item->refcnt == 0)
	free(item);

    debug_return;
}

/*
 * Delete a ref from the item containing "resolved" and free if
 * the refcount reaches 0.
 */
void
canon_path_free(char *resolved)
{
    debug_decl(canon_path_free, SUDOERS_DEBUG_UTIL);
    if (resolved != NULL)
	canon_path_free_item(resolved_to_item(resolved));
    debug_return;
}

/*
 * Free canon_cache.
 * This only removes the reference for that the cache owns.
 * Other references remain valid until canon_path_free() is called.
 */
void
canon_path_free_cache(void)
{
    debug_decl(canon_path_free_cache, SUDOERS_DEBUG_UTIL);

    if (canon_cache != NULL) {
	rbdestroy(canon_cache, canon_path_free_item);
	canon_cache = NULL;
    }

    debug_return;
}

/*
 * Like realpath(3) but caches the result.  Returns an entry from the
 * cache on success (with an added reference) or NULL on failure.
 */
char *
canon_path(const char *inpath)
{
    size_t item_size, inlen, reslen = 0;
    char *resolved, resbuf[PATH_MAX];
    struct cache_item key, *item;
    struct rbnode *node = NULL;
    debug_decl(canon_path, SUDOERS_DEBUG_UTIL);

    if (canon_cache == NULL) {
	canon_cache = rbcreate(compare);
	if (canon_cache == NULL) {
	    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	    debug_return_str(NULL);
	}
    } else {
	/* Check cache. */
	key.pathname = (char *)inpath;
	if ((node = rbfind(canon_cache, &key)) != NULL) {
	    item = node->data;
	    goto done;
	}
    }

    /*
     * Not cached, call realpath(3).
     * Older realpath() doesn't support passing a NULL buffer.
     * We special-case the empty string to resolve to "/".
     * XXX - warn on errors other than ENOENT?
     */
    if (*inpath == '\0')
	resolved = (char *)"/";
    else
	resolved = realpath(inpath, resbuf);

    inlen = strlen(inpath);
    /* one for NULL terminator of resolved, one for NULL terminator of pathname */
    item_size = sizeof(*item) + inlen + 2;
    if (resolved != NULL) {
	reslen = strlen(resolved);
	item_size += reslen;
    }
    item = malloc(item_size);
    if (item == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	debug_return_str(NULL);
    }
    if (resolved != NULL)
	memcpy(item->resolved, resolved, reslen);
    item->resolved[reslen] = '\0';
    item->pathname = item->resolved + reslen + 1;
    memcpy(item->pathname, inpath, inlen);
    item->pathname[inlen] = '\0';
    item->refcnt = 1;
    switch (rbinsert(canon_cache, item, NULL)) {
    case 1:
	/* should not happen */
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
	    "path \"%s\" already exists in the cache", inpath);
	item->refcnt = 0;
	break;
    case -1:
	/* can't cache item, just return it */
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
	    "can't cache path \"%s\"", inpath);
	item->refcnt = 0;
	break;
    }
done:
    if (item->refcnt != 0) {
        sudo_debug_printf(SUDO_DEBUG_DEBUG,
            "%s: path %s -> %s (%s)", __func__, inpath,
	    item->resolved[0] ? item->resolved : "NULL",
            node ? "cache hit" : "cached");
    }
    if (item->resolved[0] == '\0') {
	/* negative result, free item if not cached */
	if (item->refcnt == 0)
	    free(item);
	debug_return_str(NULL);
    }
    item->refcnt++;
    debug_return_str(item->resolved);
}
