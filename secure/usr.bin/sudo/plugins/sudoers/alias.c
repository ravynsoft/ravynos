/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2004-2005, 2007-2021, 2023
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
 * This is an open source non-commercial project. Dear PVS-Studio, please check it.
 * PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sudoers.h>
#include <redblack.h>
#include <gram.h>

/*
 * Comparison function for the red-black tree.
 * Aliases are sorted by name with the type used as a tie-breaker.
 */
static int
alias_compare(const void *v1, const void *v2)
{
    const struct alias *a1 = (const struct alias *)v1;
    const struct alias *a2 = (const struct alias *)v2;
    int res;
    debug_decl(alias_compare, SUDOERS_DEBUG_ALIAS);

    if (a1 == NULL)
	res = -1;
    else if (a2 == NULL)
	res = 1;
    else if ((res = strcmp(a1->name, a2->name)) == 0)
	res = a1->type - a2->type;
    debug_return_int(res);
}

/*
 * Search the tree for an alias with the specified name and type.
 * Returns a pointer to the alias structure or NULL if not found.
 * Caller is responsible for calling alias_put() on the returned
 * alias to mark it as unused.
 */
struct alias *
alias_get(const struct sudoers_parse_tree *parse_tree, const char *name,
    short type)
{
    struct alias key;
    struct rbnode *node;
    struct alias *a = NULL;
    debug_decl(alias_get, SUDOERS_DEBUG_ALIAS);

    if (parse_tree->aliases == NULL)
	debug_return_ptr(NULL);

    key.name = (char *)name;
    key.type = type;
    if ((node = rbfind(parse_tree->aliases, &key)) != NULL) {
	/*
	 * Check whether this alias is already in use.
	 * If so, we've detected a loop.  If not, set the flag,
	 * which the caller should clear with a call to alias_put().
	 */
	a = node->data;
	if (a->used) {
	    errno = ELOOP;
	    debug_return_ptr(NULL);
	}
	a->used = true;
    } else {
	errno = ENOENT;
    }
    debug_return_ptr(a);
}

/*
 * Clear the "used" flag in an alias once the caller is done with it.
 */
void
alias_put(struct alias *a)
{
    debug_decl(alias_put, SUDOERS_DEBUG_ALIAS);
    a->used = false;
    debug_return;
}

/*
 * Add an alias to the aliases redblack tree.
 * Note that "file" must be a reference-counted string.
 * Returns true on success and false on failure, setting errno.
 */
bool
alias_add(struct sudoers_parse_tree *parse_tree, char *name,
    short type, char *file, int line, int column,
    struct member *members)
{
    struct alias *a;
    debug_decl(alias_add, SUDOERS_DEBUG_ALIAS);

    if (parse_tree->aliases == NULL) {
	if ((parse_tree->aliases = alloc_aliases()) == NULL)
	    debug_return_bool(false);
    }

    a = calloc(1, sizeof(*a));
    if (a == NULL)
	debug_return_bool(false);

    /* Only set elements used by alias_compare() in case there is a dupe. */
    a->name = name;
    a->type = type;
    switch (rbinsert(parse_tree->aliases, a, NULL)) {
    case 1:
	free(a);
	errno = EEXIST;
	debug_return_bool(false);
    case -1:
	free(a);
	debug_return_bool(false);
    }

    /*
     * It is now safe to fill in the rest of the alias.  We do this last
     * since it modifies "file" (adds a ref) and "members" (tailq conversion).
     */
    /* a->used = false; */
    a->file = sudo_rcstr_addref(file);
    a->line = line;
    a->column = column;
    HLTQ_TO_TAILQ(&a->members, members, entries);
    debug_return_bool(true);
}

/*
 * Closure to adapt 2-arg rbapply() to 3-arg alias_apply().
 */
struct alias_apply_closure {
    struct sudoers_parse_tree *parse_tree;
    int (*func)(struct sudoers_parse_tree *, struct alias *, void *);
    void *cookie;
};

/* Adapt rbapply() to alias_apply() calling convention. */
static int
alias_apply_func(void *v1, void *v2)
{
    struct alias *a = v1;
    struct alias_apply_closure *closure = v2;

    return closure->func(closure->parse_tree, a, closure->cookie);
}

/*
 * Apply a function to each alias entry and pass in a cookie.
 */
void
alias_apply(struct sudoers_parse_tree *parse_tree,
    int (*func)(struct sudoers_parse_tree *, struct alias *, void *),
    void *cookie)
{
    struct alias_apply_closure closure;
    debug_decl(alias_apply, SUDOERS_DEBUG_ALIAS);

    if (parse_tree->aliases != NULL) {
	closure.parse_tree = parse_tree;
	closure.func = func;
	closure.cookie = cookie;
	rbapply(parse_tree->aliases, alias_apply_func, &closure, inorder);
    }

    debug_return;
}

/*
 * Returns true if there are no aliases in the parse_tree, else false.
 */
bool
no_aliases(const struct sudoers_parse_tree *parse_tree)
{
    debug_decl(no_aliases, SUDOERS_DEBUG_ALIAS);
    debug_return_bool(parse_tree->aliases == NULL ||
	rbisempty(parse_tree->aliases));
}

/*
 * Free memory used by an alias struct and its members.
 */
void
alias_free(void *v)
{
    struct alias *a = (struct alias *)v;
    debug_decl(alias_free, SUDOERS_DEBUG_ALIAS);

    if (a != NULL) {
	free(a->name);
	sudo_rcstr_delref(a->file);
	free_members(&a->members);
	free(a);
    }

    debug_return;
}

/*
 * Find the named alias, remove it from the tree and return it.
 */
struct alias *
alias_remove(struct sudoers_parse_tree *parse_tree, const char *name,
    short type)
{
    struct rbnode *node;
    struct alias key;
    debug_decl(alias_remove, SUDOERS_DEBUG_ALIAS);

    if (parse_tree->aliases != NULL) {
	key.name = (char *)name;
	key.type = type;
	if ((node = rbfind(parse_tree->aliases, &key)) != NULL)
	    debug_return_ptr(rbdelete(parse_tree->aliases, node));
    }
    errno = ENOENT;
    debug_return_ptr(NULL);
}

struct rbtree *
alloc_aliases(void)
{
    debug_decl(alloc_aliases, SUDOERS_DEBUG_ALIAS);

    debug_return_ptr(rbcreate(alias_compare));
}

void
free_aliases(struct rbtree *aliases)
{
    debug_decl(free_aliases, SUDOERS_DEBUG_ALIAS);

    if (aliases != NULL)
	rbdestroy(aliases, alias_free);
}

const char *
alias_type_to_string(short alias_type)
{
    return alias_type == HOSTALIAS ? "Host_Alias" :
	alias_type == CMNDALIAS ? "Cmnd_Alias" :
	alias_type == USERALIAS ? "User_Alias" :
	alias_type == RUNASALIAS ? "Runas_Alias" :
	"Invalid_Alias";
}

/*
 * Remove the alias of the specified type as well as any other aliases
 * referenced by that alias.  Stores removed aliases in a freelist.
 */
static bool
alias_remove_recursive(struct sudoers_parse_tree *parse_tree, char *name,
    short type, struct rbtree *freelist)
{
    struct member *m;
    struct alias *a;
    bool ret = true;
    debug_decl(alias_remove_recursive, SUDOERS_DEBUG_ALIAS);

    if ((a = alias_remove(parse_tree, name, type)) != NULL) {
	TAILQ_FOREACH(m, &a->members, entries) {
	    if (m->type == ALIAS) {
		if (!alias_remove_recursive(parse_tree, m->name, type, freelist))
		    ret = false;
	    }
	}
	if (rbinsert(freelist, a, NULL) != 0)
	    ret = false;
    }
    debug_return_bool(ret);
}

static int
alias_find_used_members(struct sudoers_parse_tree *parse_tree,
    struct member_list *members, short atype, struct rbtree *used_aliases)
{
    struct member *m;
    int errors = 0;
    debug_decl(alias_find_used_members, SUDOERS_DEBUG_ALIAS);

    if (members != NULL) {
	TAILQ_FOREACH(m, members, entries) {
	    if (m->type != ALIAS)
		continue;
	    if (!alias_remove_recursive(parse_tree, m->name, atype, used_aliases))
		errors++;
	}
    }

    debug_return_int(errors);
}

/*
 * Move all aliases referenced by userspecs to used_aliases.
 */
bool
alias_find_used(struct sudoers_parse_tree *parse_tree, struct rbtree *used_aliases)
{
    struct privilege *priv;
    struct userspec *us;
    struct cmndspec *cs;
    struct defaults *d;
    struct member *m;
    int errors = 0;
    debug_decl(alias_find_used, SUDOERS_DEBUG_ALIAS);

    /* Move referenced aliases to used_aliases. */
    TAILQ_FOREACH(us, &parse_tree->userspecs, entries) {
	errors += alias_find_used_members(parse_tree, &us->users,
	    USERALIAS, used_aliases);
	TAILQ_FOREACH(priv, &us->privileges, entries) {
	    errors += alias_find_used_members(parse_tree, &priv->hostlist,
		HOSTALIAS, used_aliases);
	    TAILQ_FOREACH(cs, &priv->cmndlist, entries) {
		errors += alias_find_used_members(parse_tree, cs->runasuserlist,
		    RUNASALIAS, used_aliases);
		errors += alias_find_used_members(parse_tree, cs->runasgrouplist,
		    RUNASALIAS, used_aliases);
		if ((m = cs->cmnd)->type == ALIAS) {
		    if (!alias_remove_recursive(parse_tree, m->name, CMNDALIAS,
			used_aliases))
			errors++;
		}
	    }
	}
    }
    TAILQ_FOREACH(d, &parse_tree->defaults, entries) {
	switch (d->type) {
	    case DEFAULTS_HOST:
		errors += alias_find_used_members(parse_tree,
		    &d->binding->members, HOSTALIAS, used_aliases);
		break;
	    case DEFAULTS_USER:
		errors += alias_find_used_members(parse_tree,
		    &d->binding->members, USERALIAS, used_aliases);
		break;
	    case DEFAULTS_RUNAS:
		errors += alias_find_used_members(parse_tree,
		    &d->binding->members, RUNASALIAS, used_aliases);
		break;
	    case DEFAULTS_CMND:
		errors += alias_find_used_members(parse_tree,
		    &d->binding->members, CMNDALIAS, used_aliases);
		break;
	    default:
		break;
	}
    }

    debug_return_bool(errors ? false : true);
}
