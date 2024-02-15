/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2004-2005, 2007-2021, 2023 Todd C. Miller <Todd.Miller@sudo.ws>
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
#include <time.h>

#include <sudoers.h>
#include <sudo_lbuf.h>
#include <gram.h>

/*
 * Write a privilege to lbuf in sudoers format.
 */
bool
sudoers_format_privilege(struct sudo_lbuf *lbuf,
    const struct sudoers_parse_tree *parse_tree, const struct privilege *priv,
    bool expand_aliases)
{
    const struct cmndspec *cs, *prev_cs;
    const struct member *m;
    struct cmndtag tags;
    debug_decl(sudoers_format_privilege, SUDOERS_DEBUG_UTIL);

    /* Convert per-privilege defaults to tags. */
    sudoers_defaults_list_to_tags(&priv->defaults, &tags);

    /* Print hosts list. */
    TAILQ_FOREACH(m, &priv->hostlist, entries) {
	if (m != TAILQ_FIRST(&priv->hostlist))
	    sudo_lbuf_append(lbuf, ", ");
	sudoers_format_member(lbuf, parse_tree, m, ", ",
	    expand_aliases ? HOSTALIAS : UNSPEC);
    }

    /* Print commands. */
    sudo_lbuf_append(lbuf, " = ");
    prev_cs = NULL;
    TAILQ_FOREACH(cs, &priv->cmndlist, entries) {
	if (prev_cs == NULL || RUNAS_CHANGED(cs, prev_cs)) {
	    if (cs != TAILQ_FIRST(&priv->cmndlist))
		sudo_lbuf_append(lbuf, ", ");
	    if (cs->runasuserlist != NULL || cs->runasgrouplist != NULL)
		sudo_lbuf_append(lbuf, "(");
	    if (cs->runasuserlist != NULL) {
		TAILQ_FOREACH(m, cs->runasuserlist, entries) {
		    if (m != TAILQ_FIRST(cs->runasuserlist))
			sudo_lbuf_append(lbuf, ", ");
		    sudoers_format_member(lbuf, parse_tree, m, ", ",
			expand_aliases ? RUNASALIAS : UNSPEC);
		}
	    }
	    if (cs->runasgrouplist != NULL) {
		sudo_lbuf_append(lbuf, " : ");
		TAILQ_FOREACH(m, cs->runasgrouplist, entries) {
		    if (m != TAILQ_FIRST(cs->runasgrouplist))
			sudo_lbuf_append(lbuf, ", ");
		    sudoers_format_member(lbuf, parse_tree, m, ", ",
			expand_aliases ? RUNASALIAS : UNSPEC);
		}
	    }
	    if (cs->runasuserlist != NULL || cs->runasgrouplist != NULL)
		sudo_lbuf_append(lbuf, ") ");
	} else if (cs != TAILQ_FIRST(&priv->cmndlist)) {
	    sudo_lbuf_append(lbuf, ", ");
	}
	sudoers_format_cmndspec(lbuf, parse_tree, cs, prev_cs, tags,
	    expand_aliases);
	prev_cs = cs;
    }

    debug_return_bool(!sudo_lbuf_error(lbuf));
}

/*
 * Write a userspec to lbuf in sudoers format.
 */
bool
sudoers_format_userspec(struct sudo_lbuf *lbuf,
    const struct sudoers_parse_tree *parse_tree,
    const struct userspec *us, bool expand_aliases)
{
    const struct sudoers_comment *comment;
    const struct privilege *priv;
    const struct member *m;
    debug_decl(sudoers_format_userspec, SUDOERS_DEBUG_UTIL);

    /* Print comments (if any). */
    STAILQ_FOREACH(comment, &us->comments, entries) {
	sudo_lbuf_append(lbuf, "# %s\n", comment->str);
    }

    /* Print users list. */
    TAILQ_FOREACH(m, &us->users, entries) {
	if (m != TAILQ_FIRST(&us->users))
	    sudo_lbuf_append(lbuf, ", ");
	sudoers_format_member(lbuf, parse_tree, m, ", ",
	    expand_aliases ? USERALIAS : UNSPEC);
    }

    TAILQ_FOREACH(priv, &us->privileges, entries) {
	if (priv != TAILQ_FIRST(&us->privileges))
	    sudo_lbuf_append(lbuf, " : ");
	else
	    sudo_lbuf_append(lbuf, " ");
	if (!sudoers_format_privilege(lbuf, parse_tree, priv, expand_aliases))
	    break;
    }
    sudo_lbuf_append(lbuf, "\n");

    debug_return_bool(!sudo_lbuf_error(lbuf));
}

/*
 * Write a userspec_list to lbuf in sudoers format.
 */
bool
sudoers_format_userspecs(struct sudo_lbuf *lbuf,
    const struct sudoers_parse_tree *parse_tree, const char *separator,
    bool expand_aliases, bool flush)
{
    const struct userspec *us;
    debug_decl(sudoers_format_userspecs, SUDOERS_DEBUG_UTIL);

    TAILQ_FOREACH(us, &parse_tree->userspecs, entries) {
	if (separator != NULL && us != TAILQ_FIRST(&parse_tree->userspecs))
	    sudo_lbuf_append(lbuf, "%s", separator);
	if (!sudoers_format_userspec(lbuf, parse_tree, us, expand_aliases))
	    break;
	sudo_lbuf_print(lbuf);
    }

    debug_return_bool(!sudo_lbuf_error(lbuf));
}

/*
 * Format and append a defaults line to the specified lbuf.
 * If next, is specified, it must point to the next defaults
 * entry in the list; this is used to print multiple defaults
 * entries with the same binding on a single line.
 */
bool
sudoers_format_default_line(struct sudo_lbuf *lbuf,
    const struct sudoers_parse_tree *parse_tree, const struct defaults *d,
    struct defaults **next, bool expand_aliases)
{
    const struct member *m;
    short alias_type;
    debug_decl(sudoers_format_default_line, SUDOERS_DEBUG_UTIL);

    /* Print Defaults type and binding (if present) */
    switch (d->type) {
	case DEFAULTS_HOST:
	    sudo_lbuf_append(lbuf, "Defaults@");
	    alias_type = expand_aliases ? HOSTALIAS : UNSPEC;
	    break;
	case DEFAULTS_USER:
	    sudo_lbuf_append(lbuf, "Defaults:");
	    alias_type = expand_aliases ? USERALIAS : UNSPEC;
	    break;
	case DEFAULTS_RUNAS:
	    sudo_lbuf_append(lbuf, "Defaults>");
	    alias_type = expand_aliases ? RUNASALIAS : UNSPEC;
	    break;
	case DEFAULTS_CMND:
	    sudo_lbuf_append(lbuf, "Defaults!");
	    alias_type = expand_aliases ? CMNDALIAS : UNSPEC;
	    break;
	default:
	    sudo_lbuf_append(lbuf, "Defaults");
	    alias_type = UNSPEC;
	    break;
    }
    TAILQ_FOREACH(m, &d->binding->members, entries) {
	if (m != TAILQ_FIRST(&d->binding->members))
	    sudo_lbuf_append(lbuf, ", ");
	sudoers_format_member(lbuf, parse_tree, m, ", ", alias_type);
    }

    sudo_lbuf_append(lbuf, " ");
    sudoers_format_default(lbuf, d);

    if (next != NULL) {
	/* Merge Defaults with the same binding, there may be multiple. */
	struct defaults *n;
	while ((n = TAILQ_NEXT(d, entries)) && d->binding == n->binding) {
	    sudo_lbuf_append(lbuf, ", ");
	    sudoers_format_default(lbuf, n);
	    d = n;
	}
	*next = n;
    }
    sudo_lbuf_append(lbuf, "\n");

    debug_return_bool(!sudo_lbuf_error(lbuf));
}
