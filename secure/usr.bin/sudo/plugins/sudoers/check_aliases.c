/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2004-2005, 2007-2018, 2021-2023
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sudoers.h>
#include <gram.h>

struct alias_warned {
    SLIST_ENTRY(alias_warned) entries;
    const char *name;
};
SLIST_HEAD(alias_warned_list, alias_warned);

static bool
alias_warned(struct alias_warned_list *warned, char *name)
{
    struct alias_warned *w;
    debug_decl(alias_warned, SUDOERS_DEBUG_ALIAS);

    SLIST_FOREACH(w, warned, entries) {
	if (strcmp(w->name, name) == 0)
	    debug_return_bool(true);
    }

    debug_return_bool(false);
}

static void
alias_warned_add(struct alias_warned_list *warned, char *name)
{
    struct alias_warned *w;
    debug_decl(alias_warned_add, SUDOERS_DEBUG_ALIAS);

    w = malloc(sizeof(*w));
    if (w != NULL) {
	w->name = name;
	SLIST_INSERT_HEAD(warned, w, entries);
    }

    debug_return;
}

static int
check_alias(struct sudoers_parse_tree *parse_tree,
    struct alias_warned_list *warned, char *name, short type,
    char *file, int line, int column, bool strict, bool quiet)
{
    struct member *m;
    struct alias *a;
    int errors = 0;
    debug_decl(check_alias, SUDOERS_DEBUG_ALIAS);

    if ((a = alias_get(parse_tree, name, type)) != NULL) {
	/* check alias contents */
	TAILQ_FOREACH(m, &a->members, entries) {
	    if (m->type != ALIAS)
		continue;
	    errors += check_alias(parse_tree, warned, m->name, type,
		a->file, a->line, a->column, strict, quiet);
	}
	alias_put(a);
    } else {
	if (!alias_warned(warned, name)) {
	    if (errno == ELOOP) {
		parser_warnx(parse_tree->ctx, file, line, column, strict, quiet,
		    N_("cycle in %s \"%s\""), alias_type_to_string(type), name);
	    } else {
		parser_warnx(parse_tree->ctx, file, line, column, strict, quiet,
		    N_("%s \"%s\" referenced but not defined"),
		    alias_type_to_string(type), name);
	    }
	    alias_warned_add(warned, name);
	}
	errors++;
    }

    debug_return_int(errors);
}

/*
 * Iterate through the sudoers datastructures looking for undefined
 * aliases or unused aliases.
 * In strict mode, returns the number of errors, else 0.
 */
int
check_aliases(struct sudoers_parse_tree *parse_tree, bool strict, bool quiet,
    int (*cb_unused)(struct sudoers_parse_tree *, struct alias *, void *))
{
    struct alias_warned_list warned = SLIST_HEAD_INITIALIZER(warned);
    struct rbtree *used_aliases;
    struct alias_warned *w;
    struct cmndspec *cs;
    struct member *m;
    struct privilege *priv;
    struct userspec *us;
    int errors = 0;
    debug_decl(check_aliases, SUDOERS_DEBUG_ALIAS);

    used_aliases = alloc_aliases();
    if (used_aliases == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	debug_return_int(-1);
    }

    /* Forward check. */
    TAILQ_FOREACH(us, &parse_tree->userspecs, entries) {
	TAILQ_FOREACH(m, &us->users, entries) {
	    if (m->type == ALIAS) {
		errors += check_alias(parse_tree, &warned, m->name, USERALIAS,
		    us->file, us->line, us->column, strict, quiet);
	    }
	}
	TAILQ_FOREACH(priv, &us->privileges, entries) {
	    TAILQ_FOREACH(m, &priv->hostlist, entries) {
		if (m->type == ALIAS) {
		    errors += check_alias(parse_tree, &warned, m->name, HOSTALIAS,
			us->file, us->line, us->column, strict, quiet);
		}
	    }
	    TAILQ_FOREACH(cs, &priv->cmndlist, entries) {
		if (cs->runasuserlist != NULL) {
		    TAILQ_FOREACH(m, cs->runasuserlist, entries) {
			if (m->type == ALIAS) {
			    errors += check_alias(parse_tree, &warned, m->name, RUNASALIAS,
				us->file, us->line, us->column, strict, quiet);
			}
		    }
		}
		if (cs->runasgrouplist != NULL) {
		    TAILQ_FOREACH(m, cs->runasgrouplist, entries) {
			if (m->type == ALIAS) {
			    errors += check_alias(parse_tree, &warned, m->name, RUNASALIAS,
				us->file, us->line, us->column, strict, quiet);
			}
		    }
		}
		if ((m = cs->cmnd)->type == ALIAS) {
		    errors += check_alias(parse_tree, &warned, m->name, CMNDALIAS,
			us->file, us->line, us->column, strict, quiet);
		}
	    }
	}
    }
    while ((w = SLIST_FIRST(&warned)) != NULL) {
	SLIST_REMOVE_HEAD(&warned, entries);
	free(w);
    }

    /* Reverse check (destructive) */
    if (!alias_find_used(parse_tree, used_aliases))
	errors++;
    free_aliases(used_aliases);

    /* If all aliases were referenced we will have an empty tree. */
    if (!no_aliases(parse_tree))
	alias_apply(parse_tree, cb_unused, &quiet);

    debug_return_int(strict ? errors : 0);
}
