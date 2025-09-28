/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2021-2022 Todd C. Miller <Todd.Miller@sudo.ws>
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
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>

#include <sudoers.h>
#include <redblack.h>
#include <cvtsudoers.h>
#include <gram.h>

static struct member *
new_member(const char *name, short type)
{
    struct member *m;
    debug_decl(digest_list_equivalent, SUDOERS_DEBUG_PARSER);

    m = calloc(1, sizeof(struct member));
    if (m == NULL)
	sudo_fatalx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
    if (name != NULL) {
	m->name = strdup(name);
	if (m->name == NULL) {
	    sudo_fatalx(U_("%s: %s"), __func__,
		U_("unable to allocate memory"));
	}
    }
    m->type = type;

    debug_return_ptr(m);
}

/*
 * Compare two digest lists.
 * Returns true if they are the same, else false.
 * XXX - should not care about order
 */
static bool
digest_list_equivalent(struct command_digest_list *cdl1,
    struct command_digest_list *cdl2)
{
    struct command_digest *cd1 = TAILQ_FIRST(cdl1);
    struct command_digest *cd2 = TAILQ_FIRST(cdl2);
    debug_decl(digest_list_equivalent, SUDOERS_DEBUG_PARSER);

    while (cd1 != NULL && cd2 != NULL) {
	if (cd1->digest_type != cd2->digest_type)
	    debug_return_bool(false);
	if (strcmp(cd1->digest_str, cd2->digest_str) != 0)
	    debug_return_bool(false);
	cd1 = TAILQ_NEXT(cd1, entries);
	cd2 = TAILQ_NEXT(cd2, entries);
    }

    if (cd1 != NULL || cd2 != NULL)
	debug_return_bool(false);
    debug_return_bool(true);
}

/*
 * Compare two members.
 * Returns true if they are the same, else false.
 */
static bool
member_equivalent(struct member *m1, struct member *m2)
{
    debug_decl(member_equivalent, SUDOERS_DEBUG_PARSER);

    if (m1->type != m2->type || m1->negated != m2->negated)
	debug_return_bool(false);

    if (m1->type == COMMAND) {
	struct sudo_command *c1 = (struct sudo_command *)m1->name;
	struct sudo_command *c2 = (struct sudo_command *)m2->name;
	if (c1->cmnd != NULL && c2->cmnd != NULL) {
	    if (strcmp(c1->cmnd, c2->cmnd) != 0)
		debug_return_bool(false);
	} else if (c1->cmnd != c2->cmnd) {
	    debug_return_bool(false);
	}

	if (c1->args != NULL && c2->args != NULL) {
	    if (strcmp(c1->args, c2->args) != 0)
		debug_return_bool(false);
	} else if (c1->args != c2->args) {
	    debug_return_bool(false);
	}

	if (!digest_list_equivalent(&c1->digests, &c2->digests)) {
	    debug_return_bool(false);
	}
    } else {
	if (m1->name != NULL && m2->name != NULL) {
	    if (strcmp(m1->name, m2->name) != 0)
		debug_return_bool(false);
	} else if (m1->name != m2->name) {
	    debug_return_bool(false);
	}
    }

    debug_return_bool(true);
}

/*
 * Compare two members, m1 and m2.
 * Returns true if m2 overrides m1, else false.
 */
static bool
member_overridden(struct member *m1, struct member *m2, bool check_negated)
{
    debug_decl(member_overridden, SUDOERS_DEBUG_PARSER);

    if (check_negated && m1->negated != m2->negated)
	debug_return_bool(false);

    /* "ALL" always wins (modulo digest). */
    if (m2->type == ALL) {
	if (m2->name != NULL) {
	    struct sudo_command *c1 = (struct sudo_command *)m1->name;
	    struct sudo_command *c2 = (struct sudo_command *)m2->name;
	    debug_return_bool(digest_list_equivalent(&c1->digests, &c2->digests));
	}
	debug_return_bool(true);
    }

    if (m1->type != m2->type)
	debug_return_bool(false);

    if (m1->type == COMMAND) {
	struct sudo_command *c1 = (struct sudo_command *)m1->name;
	struct sudo_command *c2 = (struct sudo_command *)m2->name;
	if (strcmp(c1->cmnd, c2->cmnd) != 0)
	    debug_return_bool(false);

	if (c1->args != NULL && c2->args != NULL) {
	    if (strcmp(c1->args, c2->args) != 0)
		debug_return_bool(false);
	} else if (c1->args != c2->args) {
	    debug_return_bool(false);
	}

	if (!digest_list_equivalent(&c1->digests, &c2->digests)) {
	    debug_return_bool(false);
	}
    } else {
	if (strcmp(m1->name, m2->name) != 0)
	    debug_return_bool(false);
    }

    debug_return_bool(true);
}

/*
 * Given two member lists, ml1 and ml2.
 * Returns true if the every element of ml1 is overridden by ml2, else false.
 */
static bool
member_list_override(struct member_list *ml1, struct member_list *ml2,
    bool check_negated)
{
    struct member *m1, *m2;
    debug_decl(member_list_override, SUDOERS_DEBUG_PARSER);

    /* An empty member_list only overrides another empty list. */
    if (TAILQ_EMPTY(ml2)) {
	debug_return_bool(TAILQ_EMPTY(ml1));
    }

    /* Check whether each element of ml1 is also covered by ml2. */
    TAILQ_FOREACH_REVERSE(m1, ml1, member_list, entries) {
	bool overridden = false;
	TAILQ_FOREACH_REVERSE(m2, ml2, member_list, entries) {
	    if (member_overridden(m1, m2, check_negated)) {
		overridden = true;
		break;
	    }
	}
	if (!overridden)
	    debug_return_bool(false);
    }

    debug_return_bool(true);
}

/*
 * Compare two member lists.
 * Returns true if they are the same, else false.
 * XXX - should not care about order if things are not negated.
 */
static bool
member_list_equivalent(struct member_list *ml1, struct member_list *ml2)
{
    struct member *m1 = TAILQ_FIRST(ml1);
    struct member *m2 = TAILQ_FIRST(ml2);
    debug_decl(member_list_equivalent, SUDOERS_DEBUG_PARSER);

    while (m1 != NULL && m2 != NULL) {
	if (!member_equivalent(m1, m2))
	    debug_return_bool(false);
	m1 = TAILQ_NEXT(m1, entries);
	m2 = TAILQ_NEXT(m2, entries);
    }

    if (m1 != NULL || m2 != NULL)
	debug_return_bool(false);
    debug_return_bool(true);
}

/*
 * Attempt to simplify a host list.
 * If a host list contains all hosts in bound_hosts, replace them with
 * "ALL".  Also prune hosts on either side of "ALL" when possible.
 */
static void
simplify_host_list(struct member_list *hosts, const char *file, int line,
    int column, struct member_list *bound_hosts)
{
    struct member *m, *n, *next;
    bool logged = false;
    debug_decl(simplify_host_list, SUDOERS_DEBUG_PARSER);

    /*
     * If all sudoers sources have an associated host, replace a
     * list of those hosts with "ALL".
     */
    if (!TAILQ_EMPTY(bound_hosts)) {
	TAILQ_FOREACH_REVERSE(n, bound_hosts, member_list, entries) {
	    TAILQ_FOREACH_REVERSE(m, hosts, member_list, entries) {
		if (m->negated) {
		    /* Don't try to handled negated entries. */
		    m = NULL;
		    break;
		}
		if (m->type == n->type && strcmp(m->name, n->name) == 0) {
		    /* match */
		    break;
		}
	    }
	    if (m == NULL) {
		/* no match */
		break;
	    }
	}
	if (n == NULL) {
	    /* found all hosts */
	    log_warnx(U_("%s:%d:%d: converting host list to ALL"),
		file, line, column);
	    logged = true;

	    TAILQ_FOREACH_REVERSE(n, bound_hosts, member_list, entries) {
		TAILQ_FOREACH_REVERSE_SAFE(m, hosts, member_list, entries, next) {
		    if (m->negated) {
			/* Don't try to handled negated entries. */
			m = NULL;
			break;
		    }
		    if (m->type == n->type && strcmp(m->name, n->name) == 0) {
			/* remove matching host */
			TAILQ_REMOVE(hosts, m, entries);
			free_member(m);
			break;
		    }
		}
	    }
	    m = new_member(NULL, ALL);
	    TAILQ_INSERT_TAIL(hosts, m, entries);
	}
    }

    /*
     * A host list that contains ALL with no negated entries past it
     * is equivalent to a list containing just "ALL".
     */
    TAILQ_FOREACH_REVERSE(m, hosts, member_list, entries) {
	if (m->negated) {
	    /* Don't try to handled negated entries. */
	    break;
	}
	if (m->type == ALL) {
	    /* Replace member list with a single ALL entry. */
	    if (!logged) {
		log_warnx(U_("%s:%d:%d: converting host list to ALL"),
		    file, line, column);
	    }
	    TAILQ_REMOVE(hosts, m, entries);
	    free_members(hosts);
	    TAILQ_INSERT_TAIL(hosts, m, entries);
	    break;
	}
    }

    debug_return;
}

/*
 * Generate a unique name from old_name that is not used in parse_tree,
 * subsequent parse_trees or merged_tree.
 */
static char *
alias_make_unique(const char *old_name, short type,
    struct sudoers_parse_tree *parse_tree0,
    struct sudoers_parse_tree *merged_tree)
{
    struct sudoers_parse_tree *parse_tree;
    char *cp, *new_name = NULL;
    struct alias *a;
    long long suffix;
    size_t namelen;
    debug_decl(alias_make_unique, SUDOERS_DEBUG_ALIAS);

    /* If old_name already has a suffix, increment it, else start with "_1". */
    suffix = 0;
    namelen = strlen(old_name);
    cp = strrchr(old_name, '_');
    if (cp != NULL && isdigit((unsigned char)cp[1])) {
	suffix = sudo_strtonum(cp + 1, 0, LLONG_MAX, NULL);
	if (suffix != 0) {
	    namelen = (size_t)(cp - old_name);
	}
    }

    for (;;) {
	suffix++;
	free(new_name);
	if (asprintf(&new_name, "%.*s_%lld", (int)namelen, old_name, suffix) == -1)
	    sudo_fatalx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	/* Make sure new_name is not already in use. */
	a = alias_get(merged_tree, new_name, type);
	if (a != NULL) {
	    alias_put(a);
	    continue;
	}
	parse_tree = parse_tree0;
	while ((parse_tree = TAILQ_NEXT(parse_tree, entries)) != NULL) {
	    a = alias_get(parse_tree, new_name, type);
	    if (a != NULL) {
		alias_put(a);
		break;
	    }
	}
	if (a == NULL) {
	    /* Must be unique. */
	    break;
	}
    }

    debug_return_ptr(new_name);
}

struct alias_rename_closure {
    const char *old_name;
    const char *new_name;
    int type;
};

static int
alias_rename_members(struct sudoers_parse_tree *parse_tree, struct alias *a,
    void *v)
{
    struct alias_rename_closure *closure = v;
    struct member *m;
    debug_decl(alias_rename_members, SUDOERS_DEBUG_ALIAS);

    if (a->type != closure->type)
	debug_return_int(0);

    /* Replace old_name in member list, if present. */
    TAILQ_FOREACH(m, &a->members, entries) {
	if (m->type == ALIAS && strcmp(m->name, closure->old_name) == 0) {
	    char *copy = strdup(closure->new_name);
	    if (copy == NULL)
		sudo_fatalx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	    free(m->name);
	    m->name = copy;
	}
    }

    debug_return_int(0);
}

static void
alias_rename_defaults(const char *old_name, const char *new_name,
    short alias_type, struct defaults_list *defaults)
{
    struct defaults *def, *def_next;
    struct member *m;
    debug_decl(alias_rename_defaults, SUDOERS_DEBUG_ALIAS);

    TAILQ_FOREACH_SAFE(def, defaults, entries, def_next) {
	/* Consecutive Defaults can share the same binding. */
	if (def_next != NULL && def->binding == def_next->binding)
	    continue;

	switch (def->type) {
	case DEFAULTS_USER:
	    if (alias_type != USERALIAS)
		continue;
	    break;
	case DEFAULTS_RUNAS:
	    if (alias_type != RUNASALIAS)
		continue;
	    break;
	case DEFAULTS_HOST:
	    if (alias_type != HOSTALIAS)
		continue;
	    break;
	default:
	    continue;
	}

	/* Rename matching aliases in the binding's member_list. */
	TAILQ_FOREACH(m, &def->binding->members, entries) {
	    if (m->type != ALIAS)
		continue;
	    if (strcmp(m->name, old_name) == 0) {
		char *copy = strdup(new_name);
		if (copy == NULL) {
		    sudo_fatalx(U_("%s: %s"), __func__,
			U_("unable to allocate memory"));
		}
		free(m->name);
		m->name = copy;
	    }
	}
    }

    debug_return;
}

static void
alias_rename_member(const char *old_name, const char *new_name,
    struct member *m)
{
    debug_decl(alias_rename_member, SUDOERS_DEBUG_ALIAS);

    if (m->type == ALIAS && strcmp(m->name, old_name) == 0) {
	char *copy = strdup(new_name);
	if (copy == NULL) {
	    sudo_fatalx(U_("%s: %s"), __func__,
		U_("unable to allocate memory"));
	}
	free(m->name);
	m->name = copy;
    }

    debug_return;
}

static void
alias_rename_member_list(const char *old_name, const char *new_name,
    struct member_list *members)
{
    struct member *m;
    debug_decl(alias_rename_member_list, SUDOERS_DEBUG_ALIAS);

    TAILQ_FOREACH(m, members, entries) {
	alias_rename_member(old_name, new_name, m);
    }

    debug_return;
}

static bool
alias_rename_userspecs(const char *old_name, const char *new_name,
    short alias_type, struct userspec_list *userspecs)
{
    struct privilege *priv;
    struct cmndspec *cs;
    struct userspec *us;
    bool ret = true;
    debug_decl(alias_rename_userspecs, SUDOERS_DEBUG_ALIAS);

    TAILQ_FOREACH(us, userspecs, entries) {
	if (alias_type == USERALIAS) {
	    alias_rename_member_list(old_name, new_name, &us->users);
	}
	TAILQ_FOREACH(priv, &us->privileges, entries) {
	    alias_rename_defaults(old_name, new_name, alias_type, &priv->defaults);
	    if (alias_type == HOSTALIAS) {
		alias_rename_member_list(old_name, new_name, &priv->hostlist);
		continue;
	    }
	    TAILQ_FOREACH(cs, &priv->cmndlist, entries) {
		if (alias_type == CMNDALIAS) {
		    alias_rename_member(old_name, new_name, cs->cmnd);
		    continue;
		}
		if (alias_type == RUNASALIAS) {
		    if (cs->runasuserlist != NULL) {
			alias_rename_member_list(old_name, new_name, cs->runasuserlist);
		    }
		    if (cs->runasgrouplist != NULL) {
			alias_rename_member_list(old_name, new_name, cs->runasgrouplist);
		    }
		}
	    }
	}
    }

    debug_return_bool(ret);
}

/*
 * Rename an alias in parse_tree and all the places where it is used.
 */
static bool
alias_rename(const char *old_name, const char *new_name,
    short alias_type, struct sudoers_parse_tree *parse_tree)
{
    struct alias_rename_closure closure = { old_name, new_name, alias_type };
    struct alias *a;
    debug_decl(alias_rename, SUDOERS_DEBUG_ALIAS);

    /* Remove under old name and add via new to maintain tree properties. */
    a = alias_remove(parse_tree, old_name, alias_type);
    if (a == NULL) {
	/* Should not happen. */
	sudo_warnx(U_("unable to find alias %s"), old_name);
	debug_return_bool(false);
    }
    log_warnx(U_("%s:%d:%d: renaming alias %s to %s"),
	a->file, a->line, a->column, a->name, new_name);
    free(a->name);
    a->name = strdup(new_name);
    if (a->name == NULL)
	sudo_fatalx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
    switch (rbinsert(parse_tree->aliases, a, NULL)) {
    case 0:
	/* success */
	break;
    case 1:
	/* Already present, should not happen. */
	errno = EEXIST;
	sudo_warn(U_("%s: %s"), __func__, a->name);
	break;
    default:
	sudo_fatalx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
    }

    /* Rename it in the aliases tree itself (aliases can be nested). */
    alias_apply(parse_tree, alias_rename_members, &closure);

    /* Rename it in the Defaults list. */
    alias_rename_defaults(old_name, new_name, alias_type, &parse_tree->defaults);

    /* Rename it in the userspecs list. */
    alias_rename_userspecs(old_name, new_name, alias_type, &parse_tree->userspecs);

    debug_return_bool(true);
}

static int
alias_resolve_conflicts(struct sudoers_parse_tree *parse_tree0, struct alias *a,
    void *v)
{
    struct sudoers_parse_tree *parse_tree = parse_tree0;
    struct sudoers_parse_tree *merged_tree = v;
    char *new_name;
    int ret;
    debug_decl(alias_resolve_conflicts, SUDOERS_DEBUG_ALIAS);

    /*
     * Check for conflicting alias names in the subsequent sudoers files.
     * Duplicates are removed and conflicting aliases are renamed.
     * We cannot modify the alias tree that we are traversing.
     */
    while ((parse_tree = TAILQ_NEXT(parse_tree, entries)) != NULL) {
	struct alias *b = alias_get(parse_tree, a->name, a->type);
	if (b == NULL)
	    continue;

	/* If alias 'b' is equivalent, remove it. */
	alias_put(b);
	if (member_list_equivalent(&a->members, &b->members)) {
	    sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
		"removing duplicate alias %s from %p", a->name, parse_tree);
	    b = alias_remove(parse_tree, a->name, a->type);
	    log_warnx(U_("%s:%d:%d: removing duplicate alias %s"),
		b->file, b->line, b->column, b->name);
	    alias_free(b);
	    continue;
	}

	/* Rename alias 'b' to avoid a naming conflict. */
	new_name = alias_make_unique(a->name, a->type, parse_tree, merged_tree);
	alias_rename(a->name, new_name, a->type, parse_tree);
	free(new_name);
    }

    /*
     * The alias will exist in both the original and merged trees.
     * This is not a problem as the caller will delete the old trees
     * (without freeing the data).
     */
    ret = rbinsert(merged_tree->aliases, a, NULL);
    switch (ret) {
    case 0:
	/* success */
	break;
    case 1:
	/* already present, should not happen. */
	errno = EEXIST;
	sudo_warn(U_("%s: %s"), __func__, a->name);
	break;
    default:
	sudo_fatalx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
    }

    debug_return_int(0);
}

static bool
merge_aliases(struct sudoers_parse_tree_list *parse_trees,
    struct sudoers_parse_tree *merged_tree)
{
    struct sudoers_parse_tree *parse_tree;
    debug_decl(merge_aliases, SUDOERS_DEBUG_ALIAS);

    /*
     * For each parse_tree, check for collisions with alias names
     * in subsequent parse trees.  On collision, add a numbered
     * suffix (e.g. ALIAS_1) to make the name unique and rename
     * any uses of that alias in the affected parse_tree.
     */
    TAILQ_FOREACH(parse_tree, parse_trees, entries) {
	if (parse_tree->aliases == NULL)
	    continue;

	/*
	 * Resolve any conflicts in alias names, renaming aliases as
	 * needed and eliminating duplicates.
	 */
	alias_apply(parse_tree, alias_resolve_conflicts, merged_tree);

	/*
	 * Destroy the old alias tree without freeing the alias data
	 * which has been copied to merged_tree.
	 */
	rbdestroy(parse_tree->aliases, NULL);
	parse_tree->aliases = NULL;
    }

    debug_return_bool(true);
}

/*
 * Compare two defaults structs but not their actual value.
 * Returns true if they refer to the same Defaults variable and binding.
 * Also sets mergeable if they only differ in the binding.
 */
static bool
defaults_var_matches(struct defaults *d1, struct defaults *d2,
    bool *mergeable)
{
    debug_decl(defaults_var_matches, SUDOERS_DEBUG_DEFAULTS);

    if (strcmp(d1->var, d2->var) != 0)
	debug_return_bool(false);
    if (d1->type != d2->type) {
	if ((d1->type == DEFAULTS && d2->type == DEFAULTS_HOST) ||
	    (d1->type == DEFAULTS_HOST && d2->type == DEFAULTS)) {
	    /* We can merge host and global bindings. */
	    if (mergeable != NULL)
		*mergeable = true;
	}
	debug_return_bool(false);
    }
    if (d1->type != DEFAULTS) {
	if (!member_list_equivalent(&d1->binding->members, &d2->binding->members)) {
	    if (mergeable != NULL)
		*mergeable = true;
	    debug_return_bool(false);
	}
    }

    debug_return_bool(true);
}

/*
 * Compare the values of two defaults structs, which must be of the same type.
 * Returns true if the value and operator match, else false.
 */
static bool
defaults_val_matches(struct defaults *d1, struct defaults *d2)
{
    debug_decl(defaults_val_matches, SUDOERS_DEBUG_DEFAULTS);

    /* XXX - what about list operators? */
    if (d1->op != d2->op)
	debug_return_bool(false);

    /* Either both must be NULL or both non-NULL _and_ matching. */
    if (d1->val != NULL && d2->val != NULL) {
	if (strcmp(d1->val, d2->val) != 0)
	    debug_return_bool(false);
    } else {
	if (d1->val != NULL || d2->val != NULL)
	    debug_return_bool(false);
    }

    debug_return_bool(true);
}

/*
 * Returns true if d1 is equivalent to d2, else false.
 */
static bool
defaults_equivalent(struct defaults *d1, struct defaults *d2)
{
    debug_decl(defaults_equivalent, SUDOERS_DEBUG_DEFAULTS);

    if (!defaults_var_matches(d1, d2, NULL))
	debug_return_bool(false);
    debug_return_bool(defaults_val_matches(d1, d2));
}

/*
 * Returns true if dl1 is equivalent to dl2, else false.
 */
static bool
defaults_list_equivalent(struct defaults_list *dl1, struct defaults_list *dl2)
{
    struct defaults *d1 = TAILQ_FIRST(dl1);
    struct defaults *d2 = TAILQ_FIRST(dl2);
    debug_decl(defaults_list_equivalent, SUDOERS_DEBUG_DEFAULTS);

    while (d1 != NULL && d2 != NULL) {
	if (!defaults_equivalent(d1, d2))
	    debug_return_bool(false);
	d1 = TAILQ_NEXT(d1, entries);
	d2 = TAILQ_NEXT(d2, entries);
    }

    if (d1 != NULL || d2 != NULL)
	debug_return_bool(false);
    debug_return_bool(true);
}

enum cvtsudoers_conflict {
    CONFLICT_NONE,
    CONFLICT_RESOLVED,
    CONFLICT_UNRESOLVED
};

/*
 * Check for duplicate and conflicting Defaults entries in later sudoers files.
 * Returns true if we find a conflict or duplicate, else false.
 */
static enum cvtsudoers_conflict
defaults_check_conflict(struct defaults *def,
    struct sudoers_parse_tree *parse_tree0)
{
    struct sudoers_parse_tree *parse_tree = parse_tree0;
    struct defaults *d;
    debug_decl(defaults_check_conflict, SUDOERS_DEBUG_DEFAULTS);

    while ((parse_tree = TAILQ_NEXT(parse_tree, entries)) != NULL) {
	TAILQ_FOREACH_REVERSE(d, &parse_tree->defaults, defaults_list, entries) {
	    bool mergeable = false;

	    /*
	     * We currently only merge host-based Defaults but could do
	     * others as well.  Lists in Defaults entries can be harder
	     * to read, especially command lists.
	     */
	    if (!defaults_var_matches(def, d, &mergeable)) {
		if (!mergeable || (def->type != DEFAULTS && def->type != DEFAULTS_HOST))
		    continue;
	    }
	    if (defaults_val_matches(def, d)) {
		/* Duplicate Defaults entry (may need to merge binding). */
		if (mergeable) {
		    if (d->type != def->type &&
			    (d->type == DEFAULTS || def->type == DEFAULTS)) {
			/*
			 * To be able to merge two Defaults, they both must
			 * have the same binding type.  Convert a global
			 * Defaults to one bound to single "ALL" member.
			 */
			if (d->type == DEFAULTS) {
			    struct member *m = new_member(NULL, ALL);
			    TAILQ_INSERT_TAIL(&d->binding->members, m, entries);
			    d->type = def->type;
			}
			if (def->type == DEFAULTS) {
			    struct member *m = new_member(NULL, ALL);
			    TAILQ_INSERT_TAIL(&def->binding->members, m, entries);
			    def->type = d->type;
			}
		    }

		    /* Prepend def binding to d (hence double concat). */
		    TAILQ_CONCAT(&def->binding->members, &d->binding->members, entries);
		    TAILQ_CONCAT(&d->binding->members, &def->binding->members, entries);
		}
		debug_return_int(CONFLICT_RESOLVED);
	    }
	    /*
	     * If the value doesn't match but the Defaults name did we don't
	     * consider that a conflict.
	     */
	    if (!mergeable) {
		log_warnx(U_("%s:%d:%d: conflicting Defaults entry \"%s\" host-specific in %s:%d:%d"),
		    def->file, def->line, def->column, def->var,
		    d->file, d->line, d->column);
		debug_return_int(CONFLICT_UNRESOLVED);
	    }
	}
    }

    debug_return_int(CONFLICT_NONE);
}

/*
 * Merge Defaults entries in parse_trees and store the result in
 * merged_tree.  If a hostname was specified with the sudoers source,
 * create a host-specific Defaults entry where possible.
 * Returns true on success, else false.
 */
static bool
merge_defaults(struct sudoers_parse_tree_list *parse_trees,
    struct sudoers_parse_tree *merged_tree, struct member_list *bound_hosts)
{
    struct sudoers_parse_tree *parse_tree;
    struct defaults *def;
    struct member *m;
    debug_decl(merge_defaults, SUDOERS_DEBUG_DEFAULTS);

    TAILQ_FOREACH(parse_tree, parse_trees, entries) {
	/*
	 * If parse_tree has a host name associated with it,
	 * try to make the Defaults setting host-specific.
	 */
	TAILQ_FOREACH(def, &parse_tree->defaults, entries) {
	    if (parse_tree->lhost != NULL && def->type == DEFAULTS) {
		m = new_member(parse_tree->lhost, WORD);
		log_warnx(U_("%s:%d:%d: made Defaults \"%s\" specific to host %s"),
		    def->file, def->line, def->column, def->var,
		    parse_tree->lhost);
		TAILQ_INSERT_TAIL(&def->binding->members, m, entries);
		def->type = DEFAULTS_HOST;
	    }
	}
    }

    TAILQ_FOREACH(parse_tree, parse_trees, entries) {
	while ((def = TAILQ_FIRST(&parse_tree->defaults)) != NULL) {
	    /*
	     * Only add Defaults entry if not overridden by subsequent sudoers.
	     */
	    TAILQ_REMOVE(&parse_tree->defaults, def, entries);
	    switch (defaults_check_conflict(def, parse_tree)) {
	    case CONFLICT_NONE:
		if (def->type != DEFAULTS_HOST) {
		    log_warnx(U_("%s:%d:%d: unable to make Defaults \"%s\" host-specific"),
			def->file, def->line, def->column, def->var);
		}
		TAILQ_INSERT_TAIL(&merged_tree->defaults, def, entries);
		break;
	    case CONFLICT_RESOLVED:
		/* Duplicate or merged into a subsequent Defaults setting. */
		free_default(def);
		break;
	    case CONFLICT_UNRESOLVED:
		log_warnx(U_("%s:%d:%d: removing Defaults \"%s\" overridden by subsequent entries"),
		    def->file, def->line, def->column, def->var);
		free_default(def);
		break;
	    }
	}
    }

    /*
     * Simplify host lists in the merged Defaults.
     */
    TAILQ_FOREACH(def, &merged_tree->defaults, entries) {
	/* TODO: handle refcnt != 1 */
	if (def->type == DEFAULTS_HOST && def->binding->refcnt == 1) {
	    simplify_host_list(&def->binding->members, def->file, def->line,
		def->column, bound_hosts);
	    m = TAILQ_FIRST(&def->binding->members);
	    if (m->type == ALL && !m->negated) {
		if (TAILQ_NEXT(m, entries) == NULL) {
		    /* Convert Defaults@ALL -> Defaults */
		    def->type = DEFAULTS;
		    free_members(&def->binding->members);
		    TAILQ_INIT(&def->binding->members);
		}
	    }
	}
    }

    debug_return_bool(true);
}

/*
 * Returns true if cs1 is equivalent to cs2, else false.
 */
static bool
cmndspec_equivalent(struct cmndspec *cs1, struct cmndspec *cs2, bool check_negated)
{
    debug_decl(cmndspec_equivalent, SUDOERS_DEBUG_PARSER);

    if (cs1->runasuserlist != NULL && cs2->runasuserlist != NULL) {
	if (!member_list_override(cs1->runasuserlist, cs2->runasuserlist, check_negated))
	    debug_return_bool(false);
    } else if (cs1->runasuserlist != cs2->runasuserlist) {
	debug_return_bool(false);
    }
    if (cs1->runasgrouplist != NULL && cs2->runasgrouplist != NULL) {
	if (!member_list_override(cs1->runasgrouplist, cs2->runasgrouplist, check_negated))
	    debug_return_bool(false);
    } else if (cs1->runasgrouplist != cs2->runasgrouplist) {
	debug_return_bool(false);
    }
    if (!member_equivalent(cs1->cmnd, cs2->cmnd))
	debug_return_bool(false);
    if (TAGS_CHANGED(cs1->tags, cs2->tags))
	debug_return_bool(false);
    if (cs1->timeout != cs2->timeout)
	debug_return_bool(false);
    if (cs1->notbefore != cs2->notbefore)
	debug_return_bool(false);
    if (cs1->notafter != cs2->notafter)
	debug_return_bool(false);
    if (cs1->runcwd != NULL && cs2->runcwd != NULL) {
	if (strcmp(cs1->runcwd, cs2->runcwd) != 0)
	    debug_return_bool(false);
    } else if (cs1->runcwd != cs2->runcwd) {
	debug_return_bool(false);
    }
    if (cs1->runchroot != NULL && cs2->runchroot != NULL) {
	if (strcmp(cs1->runchroot, cs2->runchroot) != 0)
	    debug_return_bool(false);
    } else if (cs1->runchroot != cs2->runchroot) {
	debug_return_bool(false);
    }
#ifdef HAVE_SELINUX
    if (cs1->role != NULL && cs2->role != NULL) {
	if (strcmp(cs1->role, cs2->role) != 0)
	    debug_return_bool(false);
    } else if (cs1->role != cs2->role) {
	debug_return_bool(false);
    }
    if (cs1->type != NULL && cs2->type != NULL) {
	if (strcmp(cs1->type, cs2->type) != 0)
	    debug_return_bool(false);
    } else if (cs1->type != cs2->type) {
	debug_return_bool(false);
    }
#endif
#ifdef HAVE_APPARMOR
    if (cs1->apparmor_profile != NULL && cs2->apparmor_profile != NULL) {
	if (strcmp(cs1->apparmor_profile, cs2->apparmor_profile) != 0)
	    debug_return_bool(false);
    } else if (cs1->apparmor_profile != cs2->apparmor_profile) {
	debug_return_bool(false);
    }
#endif
#ifdef HAVE_PRIV_SET
    if (cs1->privs != NULL && cs2->privs != NULL) {
	if (strcmp(cs1->privs, cs2->privs) != 0)
	    debug_return_bool(false);
    } else if (cs1->privs != cs2->privs) {
	debug_return_bool(false);
    }
    if (cs1->limitprivs != NULL && cs2->limitprivs != NULL) {
	if (strcmp(cs1->limitprivs, cs2->limitprivs) != 0)
	    debug_return_bool(false);
    } else if (cs1->limitprivs != cs2->limitprivs) {
	debug_return_bool(false);
    }
#endif

    debug_return_bool(true);
}

/*
 * Returns true if csl1 is equivalent to csl2, else false.
 */
static bool
cmndspec_list_equivalent(struct cmndspec_list *csl1, struct cmndspec_list *csl2,
    bool check_negated)
{
    struct cmndspec *cs1 = TAILQ_FIRST(csl1);
    struct cmndspec *cs2 = TAILQ_FIRST(csl2);
    debug_decl(cmndspec_list_equivalent, SUDOERS_DEBUG_PARSER);

    while (cs1 != NULL && cs2 != NULL) {
	if (!cmndspec_equivalent(cs1, cs2, check_negated))
	    debug_return_bool(false);
	cs1 = TAILQ_NEXT(cs1, entries);
	cs2 = TAILQ_NEXT(cs2, entries);
    }

    if (cs1 != NULL || cs2 != NULL)
	debug_return_bool(false);
    debug_return_bool(true);
}

/*
 * Check whether userspec us1 is overridden by another sudoers file entry.
 * If us1 and another userspec differ only in their host lists, merges
 * the hosts from us1 into that userspec.
 * Returns true if overridden, else false.
 * TODO: merge privs
 */
static enum cvtsudoers_conflict
userspec_overridden(struct userspec *us1,
    struct sudoers_parse_tree *parse_tree, bool check_negated)
{
    struct userspec *us2;
    bool hosts_differ = false;
    debug_decl(userspec_overridden, SUDOERS_DEBUG_PARSER);

    if (TAILQ_EMPTY(&parse_tree->userspecs))
	debug_return_int(CONFLICT_NONE);

    /* Sudoers rules are applied in reverse order (last match wins). */
    TAILQ_FOREACH_REVERSE(us2, &parse_tree->userspecs, userspec_list, entries) {
	struct privilege *priv1, *priv2;

	if (!member_list_override(&us1->users, &us2->users, check_negated))
	    continue;

	/* XXX - order should not matter */
	priv1 = TAILQ_LAST(&us1->privileges, privilege_list);
	priv2 = TAILQ_LAST(&us2->privileges, privilege_list);
	while (priv1 != NULL && priv2 != NULL) {
	    if (!defaults_list_equivalent(&priv1->defaults, &priv2->defaults))
		break;
	    if (!cmndspec_list_equivalent(&priv1->cmndlist, &priv2->cmndlist, check_negated))
		break;

	    if (!member_list_override(&priv1->hostlist, &priv2->hostlist, check_negated))
		hosts_differ = true;

	    priv1 = TAILQ_PREV(priv1, privilege_list, entries);
	    priv2 = TAILQ_PREV(priv2, privilege_list, entries);
	}
	if (priv1 != NULL || priv2 != NULL) {
	    /* mismatch */
	    continue;
	}

	/*
	 * If we have a match of everything except the host list,
	 * merge the differing host lists.
	 */
	if (hosts_differ) {
	    priv1 = TAILQ_LAST(&us1->privileges, privilege_list);
	    priv2 = TAILQ_LAST(&us2->privileges, privilege_list);
	    while (priv1 != NULL && priv2 != NULL) {
		if (!member_list_override(&priv1->hostlist, &priv2->hostlist, check_negated)) {
		    /*
		     * Priv matches but hosts differ, prepend priv1 hostlist
		     * to into priv2 hostlist (hence the double concat).
		     */
		    TAILQ_CONCAT(&priv1->hostlist, &priv2->hostlist, entries);
		    TAILQ_CONCAT(&priv2->hostlist, &priv1->hostlist, entries);
		    log_warnx(U_("%s:%d:%d: merging userspec into %s:%d:%d"),
			us1->file, us1->line, us1->column,
			us2->file, us2->line, us2->column);
		}
		priv1 = TAILQ_PREV(priv1, privilege_list, entries);
		priv2 = TAILQ_PREV(priv2, privilege_list, entries);
	    }
	    debug_return_int(CONFLICT_RESOLVED);
	}
	debug_return_int(CONFLICT_UNRESOLVED);
    }

    debug_return_int(CONFLICT_NONE);
}

/*
 * Check whether userspec us1 is overridden by another sudoers file entry.
 * If us1 and another userspec differ only in their host lists, merges
 * the hosts from us1 into that userspec.
 * Returns true if overridden, else false.
 */
static enum cvtsudoers_conflict
userspec_check_conflict(struct userspec *us1,
    struct sudoers_parse_tree *parse_tree0)
{
    struct sudoers_parse_tree *parse_tree = parse_tree0;
    debug_decl(userspec_check_conflict, SUDOERS_DEBUG_PARSER);

    while ((parse_tree = TAILQ_NEXT(parse_tree, entries)) != NULL) {
	enum cvtsudoers_conflict ret =
	    userspec_overridden(us1, parse_tree, false);
	if (ret != CONFLICT_NONE)
	    debug_return_int(ret);
    }

    debug_return_int(CONFLICT_NONE);
}

/*
 * Merge userspecs in parse_trees and store the result in merged_tree.
 * If a hostname was specified with the sudoers source, make the
 * privilege host-specific where possible.
 * Returns true on success, else false.
 */
static bool
merge_userspecs(struct sudoers_parse_tree_list *parse_trees,
    struct sudoers_parse_tree *merged_tree, struct member_list *bound_hosts)
{
    struct sudoers_parse_tree *parse_tree;
    struct userspec *us;
    struct privilege *priv;
    struct member *m;
    debug_decl(merge_userspecs, SUDOERS_DEBUG_DEFAULTS);

    /*
     * If parse_tree has a host name associated with it,
     * try to make the privilege host-specific.
     */
    TAILQ_FOREACH(parse_tree, parse_trees, entries) {
	if (parse_tree->lhost == NULL)
	    continue;
	TAILQ_FOREACH(us, &parse_tree->userspecs, entries) {
	    TAILQ_FOREACH(priv, &us->privileges, entries) {
		TAILQ_FOREACH(m, &priv->hostlist, entries) {
		    /* We don't alter !ALL in a hostlist (XXX - should we?). */
		    if (m->type == ALL && !m->negated) {
			m->type = WORD;
			m->name = strdup(parse_tree->lhost);
			if (m->name == NULL) {
			    sudo_fatalx(U_("%s: %s"), __func__,
				U_("unable to allocate memory"));
			}
		    }
		}
	    }
	}
    }

    /*
     * Prune out duplicate userspecs after substituting hostname(s).
     * Traverse the list in reverse order--in sudoers last match wins.
     * XXX - do this at the privilege/cmndspec level instead.
     */
    TAILQ_FOREACH(parse_tree, parse_trees, entries) {
	while ((us = TAILQ_LAST(&parse_tree->userspecs, userspec_list)) != NULL) {
	    TAILQ_REMOVE(&parse_tree->userspecs, us, entries);
	    switch (userspec_check_conflict(us, parse_tree)) {
	    case CONFLICT_NONE:
		TAILQ_INSERT_HEAD(&merged_tree->userspecs, us, entries);
		break;
	    case CONFLICT_RESOLVED:
		free_userspec(us);
		break;
	    case CONFLICT_UNRESOLVED:
		log_warnx(U_("%s:%d:%d: removing userspec overridden by subsequent entries"),
		    us->file, us->line, us->column);
		free_userspec(us);
		break;
	    }
	}
    }

    /*
     * Simplify member lists in the merged tree.
     * Convert host lists with all hosts listed to "ALL" and
     * collapse other entries around "ALL".
     */
    TAILQ_FOREACH_REVERSE(us, &merged_tree->userspecs, userspec_list, entries) {
	TAILQ_FOREACH_REVERSE(priv, &us->privileges, privilege_list, entries) {
	    /* TODO: simplify other lists? */
	    simplify_host_list(&priv->hostlist, us->file, us->line, us->column,
		bound_hosts);
	}
    }

    debug_return_bool(true);
}

struct sudoers_parse_tree *
merge_sudoers(struct sudoers_parse_tree_list *parse_trees,
    struct sudoers_parse_tree *merged_tree)
{
    struct member_list bound_hosts = TAILQ_HEAD_INITIALIZER(bound_hosts);
    struct sudoers_parse_tree *parse_tree;
    debug_decl(merge_sudoers, SUDOERS_DEBUG_UTIL);

    /*
     * If all sudoers sources have a host associated with them, we
     * can replace a list of those hosts with "ALL" in Defaults
     * and userspecs.
     */
    TAILQ_FOREACH(parse_tree, parse_trees, entries) {
	if (parse_tree->lhost == NULL)
	    break;
    }
    if (parse_tree == NULL) {
	TAILQ_FOREACH(parse_tree, parse_trees, entries) {
	    struct member *m = new_member(parse_tree->lhost, WORD);
	    TAILQ_INSERT_TAIL(&bound_hosts, m, entries);
	}
    }

    if ((merged_tree->aliases = alloc_aliases()) == NULL)
	sudo_fatalx(U_("%s: %s"), __func__, U_("unable to allocate memory"));

    if (!merge_aliases(parse_trees, merged_tree))
	goto bad;

    if (!merge_defaults(parse_trees, merged_tree, &bound_hosts))
	goto bad;

    if (!merge_userspecs(parse_trees, merged_tree, &bound_hosts))
	goto bad;

    free_members(&bound_hosts);
    debug_return_ptr(merged_tree);
bad:
    free_members(&bound_hosts);
    debug_return_ptr(NULL);
}
