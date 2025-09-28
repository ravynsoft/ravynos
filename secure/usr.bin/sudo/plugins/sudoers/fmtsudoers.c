/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2004-2005, 2007-2023 Todd C. Miller <Todd.Miller@sudo.ws>
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
#include <pwd.h>
#include <time.h>

#include <sudoers.h>
#include <sudo_lbuf.h>
#include <gram.h>

/*
 * Write the contents of a struct member to the lbuf.
 * If alias_type is not UNSPEC, expand aliases using that type with
 * the specified separator (which must not be NULL in the UNSPEC case).
 */
static bool
sudoers_format_member_int(struct sudo_lbuf *lbuf,
    const struct sudoers_parse_tree *parse_tree, const char *name, int type,
    bool negated, const char *separator, short alias_type)
{
    const struct sudoers_context *ctx = parse_tree->ctx;
    struct alias *a;
    const struct member *m;
    const struct sudo_command *c;
    const struct command_digest *digest;
    debug_decl(sudoers_format_member_int, SUDOERS_DEBUG_UTIL);

    switch (type) {
	case MYSELF:
	    sudo_lbuf_append(lbuf, "%s%s", negated ? "!" : "",
		ctx->runas.list_pw ? ctx->runas.list_pw->pw_name :
		(ctx->user.name ? ctx->user.name : ""));
	    break;
	case ALL:
	    if (name == NULL) {
		sudo_lbuf_append(lbuf, "%sALL", negated ? "!" : "");
		break;
	    }
	    FALLTHROUGH;
	case COMMAND:
	    c = (struct sudo_command *) name;
	    TAILQ_FOREACH(digest, &c->digests, entries) {
		sudo_lbuf_append(lbuf, "%s:%s%s ",
		    digest_type_to_name(digest->digest_type),
		    digest->digest_str, TAILQ_NEXT(digest, entries) ? "," : "");
	    }
	    if (negated)
		sudo_lbuf_append(lbuf, "!");
	    if (c->cmnd == NULL || c->cmnd[0] == '^') {
		/* No additional quoting of characters inside a regex. */
		sudo_lbuf_append(lbuf, "%s", c->cmnd ? c->cmnd : "ALL");
	    } else {
		sudo_lbuf_append_quoted(lbuf, SUDOERS_QUOTED_CMD, "%s",
		    c->cmnd);
	    }
	    if (c->args != NULL) {
		sudo_lbuf_append(lbuf, " ");
		if (c->args[0] == '^') {
		    /* No additional quoting of characters inside a regex. */
		    sudo_lbuf_append(lbuf, "%s", c->args);
		} else {
		    sudo_lbuf_append_quoted(lbuf, SUDOERS_QUOTED_ARG, "%s",
			c->args);
		}
	    }
	    break;
	case USERGROUP:
	    /* Special case for %#gid, %:non-unix-group, %:#non-unix-gid */
	    if (strpbrk(name, " \t") == NULL) {
		if (*++name == ':') {
		    name++;
		    sudo_lbuf_append(lbuf, "%s", "%:");
		} else {
		    sudo_lbuf_append(lbuf, "%s", "%");
		}
	    }
	    goto print_word;
	case ALIAS:
	    if (alias_type != UNSPEC) {
		if ((a = alias_get(parse_tree, name, alias_type)) != NULL) {
		    TAILQ_FOREACH(m, &a->members, entries) {
			if (m != TAILQ_FIRST(&a->members))
			    sudo_lbuf_append(lbuf, "%s", separator);
			sudoers_format_member_int(lbuf, parse_tree,
			    m->name, m->type,
			    negated ? !m->negated : m->negated,
			    separator, alias_type);
		    }
		    alias_put(a);
		    break;
		}
	    }
	    FALLTHROUGH;
	default:
	print_word:
	    /* Do not quote UID/GID, all others get quoted. */
	    if (name[0] == '#' &&
		name[strspn(name + 1, "0123456789") + 1] == '\0') {
		sudo_lbuf_append(lbuf, "%s%s", negated ? "!" : "", name);
	    } else {
		if (strpbrk(name, " \t") != NULL) {
		    sudo_lbuf_append(lbuf, "%s\"", negated ? "!" : "");
		    sudo_lbuf_append_quoted(lbuf, "\"", "%s", name);
		    sudo_lbuf_append(lbuf, "\"");
		} else {
		    sudo_lbuf_append_quoted(lbuf, SUDOERS_QUOTED, "%s%s",
			negated ? "!" : "", name);
		}
	    }
	    break;
    }
    debug_return_bool(!sudo_lbuf_error(lbuf));
}

bool
sudoers_format_member(struct sudo_lbuf *lbuf,
    const struct sudoers_parse_tree *parse_tree, const struct member *m,
    const char *separator, short alias_type)
{
    return sudoers_format_member_int(lbuf, parse_tree, m->name, m->type,
	m->negated, separator, alias_type);
}

/*
 * Store a defaults entry as a command tag.
 */
bool
sudoers_defaults_to_tags(const char *var, const char *val, int op,
    struct cmndtag *tags)
{
    bool ret = true;
    debug_decl(sudoers_defaults_to_tags, SUDOERS_DEBUG_UTIL);

    if (op == true || op == false) {
	if (strcmp(var, "authenticate") == 0) {
	    tags->nopasswd = op == false;
	} else if (strcmp(var, "sudoedit_follow") == 0) {
	    tags->follow = op == true;
	} else if (strcmp(var, "log_input") == 0) {
	    tags->log_input = op == true;
	} else if (strcmp(var, "log_output") == 0) {
	    tags->log_output = op == true;
	} else if (strcmp(var, "noexec") == 0) {
	    tags->noexec = op == true;
	} else if (strcmp(var, "intercept") == 0) {
	    tags->intercept = op == true;
	} else if (strcmp(var, "setenv") == 0) {
	    tags->setenv = op == true;
	} else if (strcmp(var, "mail_all_cmnds") == 0 ||
	    strcmp(var, "mail_always") == 0 ||
	    strcmp(var, "mail_no_perms") == 0) {
	    tags->send_mail = op == true;
	} else {
	    ret = false;
	}
    } else {
	ret = false;
    }
    debug_return_bool(ret);
}

/*
 * Convert a defaults list to command tags.
 */
bool
sudoers_defaults_list_to_tags(const struct defaults_list *defs,
    struct cmndtag *tags)
{
    const struct defaults *d;
    bool ret = true;
    debug_decl(sudoers_defaults_list_to_tags, SUDOERS_DEBUG_UTIL);

    TAGS_INIT(tags);
    if (defs != NULL) {
	TAILQ_FOREACH(d, defs, entries) {
	    if (!sudoers_defaults_to_tags(d->var, d->val, d->op, tags)) {
		if (d->val != NULL) {
		    sudo_debug_printf(SUDO_DEBUG_WARN,
			"unable to convert defaults to tag: %s%s%s", d->var,
			d->op == '+' ? "+=" : d->op == '-' ? "-=" : "=", d->val);
		} else {
		    sudo_debug_printf(SUDO_DEBUG_WARN,
			"unable to convert defaults to tag: %s%s%s",
			d->op == false ? "!" : "", d->var, "");
		}
		ret = false;
	    }
	}
    }
    debug_return_bool(ret);
}

#define	FIELD_CHANGED(ocs, ncs, fld) \
	((ocs) == NULL || (ncs)->fld != (ocs)->fld)

#define	TAG_CHANGED(ocs, ncs, t, tt) \
	(TAG_SET((t).tt) && FIELD_CHANGED(ocs, ncs, tags.tt))

/*
 * Write a cmndspec to lbuf in sudoers format.
 */
bool
sudoers_format_cmndspec(struct sudo_lbuf *lbuf,
    const struct sudoers_parse_tree *parse_tree, const struct cmndspec *cs,
    const struct cmndspec *prev_cs, struct cmndtag tags, bool expand_aliases)
{
    debug_decl(sudoers_format_cmndspec, SUDOERS_DEBUG_UTIL);

    /* Merge privilege-level tags with cmndspec tags. */
    TAGS_MERGE(tags, cs->tags);

#ifdef HAVE_PRIV_SET
    if (cs->privs != NULL && FIELD_CHANGED(prev_cs, cs, privs))
	sudo_lbuf_append(lbuf, "PRIVS=\"%s\" ", cs->privs);
    if (cs->limitprivs != NULL && FIELD_CHANGED(prev_cs, cs, limitprivs))
	sudo_lbuf_append(lbuf, "LIMITPRIVS=\"%s\" ", cs->limitprivs);
#endif /* HAVE_PRIV_SET */
#ifdef HAVE_SELINUX
    if (cs->role != NULL && FIELD_CHANGED(prev_cs, cs, role))
	sudo_lbuf_append(lbuf, "ROLE=%s ", cs->role);
    if (cs->type != NULL && FIELD_CHANGED(prev_cs, cs, type))
	sudo_lbuf_append(lbuf, "TYPE=%s ", cs->type);
#endif /* HAVE_SELINUX */
#ifdef HAVE_APPARMOR
    if (cs->apparmor_profile != NULL && FIELD_CHANGED(prev_cs, cs, apparmor_profile))
	sudo_lbuf_append(lbuf, "APPARMOR_PROFILE=%s ", cs->apparmor_profile);
#endif /* HAVE_APPARMOR */
    if (cs->runchroot != NULL && FIELD_CHANGED(prev_cs, cs, runchroot))
	sudo_lbuf_append(lbuf, "CHROOT=%s ", cs->runchroot);
    if (cs->runcwd != NULL && FIELD_CHANGED(prev_cs, cs, runcwd))
	sudo_lbuf_append(lbuf, "CWD=%s ", cs->runcwd);
    if (cs->timeout > 0 && FIELD_CHANGED(prev_cs, cs, timeout)) {
	char numbuf[STRLEN_MAX_SIGNED(int) + 1];
	(void)snprintf(numbuf, sizeof(numbuf), "%d", cs->timeout);
	sudo_lbuf_append(lbuf, "TIMEOUT=%s ", numbuf);
    }
    if (cs->notbefore != UNSPEC && FIELD_CHANGED(prev_cs, cs, notbefore)) {
	char buf[sizeof("CCYYMMDDHHMMSSZ")] = "";
	struct tm gmt;
	if (gmtime_r(&cs->notbefore, &gmt) != NULL) {
	    size_t len = strftime(buf, sizeof(buf), "%Y%m%d%H%M%SZ", &gmt);
	    if (len != 0 && buf[sizeof(buf) - 1] == '\0')
		sudo_lbuf_append(lbuf, "NOTBEFORE=%s ", buf);
	}
    }
    if (cs->notafter != UNSPEC && FIELD_CHANGED(prev_cs, cs, notafter)) {
	char buf[sizeof("CCYYMMDDHHMMSSZ")] = "";
	struct tm gmt;
	if (gmtime_r(&cs->notafter, &gmt) != NULL) {
	    size_t len = strftime(buf, sizeof(buf), "%Y%m%d%H%M%SZ", &gmt);
	    if (len != 0 && buf[sizeof(buf) - 1] == '\0')
		sudo_lbuf_append(lbuf, "NOTAFTER=%s ", buf);
	}
    }
    if (TAG_CHANGED(prev_cs, cs, tags, setenv))
	sudo_lbuf_append(lbuf, tags.setenv ? "SETENV: " : "NOSETENV: ");
    if (TAG_CHANGED(prev_cs, cs, tags, intercept))
	sudo_lbuf_append(lbuf, tags.intercept ? "INTERCEPT: " : "NOINTERCEPT: ");
    if (TAG_CHANGED(prev_cs, cs, tags, noexec))
	sudo_lbuf_append(lbuf, tags.noexec ? "NOEXEC: " : "EXEC: ");
    if (TAG_CHANGED(prev_cs, cs, tags, nopasswd))
	sudo_lbuf_append(lbuf, tags.nopasswd ? "NOPASSWD: " : "PASSWD: ");
    if (TAG_CHANGED(prev_cs, cs, tags, log_input))
	sudo_lbuf_append(lbuf, tags.log_input ? "LOG_INPUT: " : "NOLOG_INPUT: ");
    if (TAG_CHANGED(prev_cs, cs, tags, log_output))
	sudo_lbuf_append(lbuf, tags.log_output ? "LOG_OUTPUT: " : "NOLOG_OUTPUT: ");
    if (TAG_CHANGED(prev_cs, cs, tags, send_mail))
	sudo_lbuf_append(lbuf, tags.send_mail ? "MAIL: " : "NOMAIL: ");
    if (TAG_CHANGED(prev_cs, cs, tags, follow))
	sudo_lbuf_append(lbuf, tags.follow ? "FOLLOW: " : "NOFOLLOW: ");
    sudoers_format_member(lbuf, parse_tree, cs->cmnd, ", ",
	expand_aliases ? CMNDALIAS : UNSPEC);
    debug_return_bool(!sudo_lbuf_error(lbuf));
}

/*
 * Format and append a defaults entry to the specified lbuf.
 */
bool
sudoers_format_default(struct sudo_lbuf *lbuf, const struct defaults *d)
{
    debug_decl(sudoers_format_default, SUDOERS_DEBUG_UTIL);

    if (d->val != NULL) {
	sudo_lbuf_append(lbuf, "%s%s", d->var,
	    d->op == '+' ? "+=" : d->op == '-' ? "-=" : "=");
	if (strpbrk(d->val, " \t") != NULL) {
	    sudo_lbuf_append(lbuf, "\"");
	    sudo_lbuf_append_quoted(lbuf, "\"", "%s", d->val);
	    sudo_lbuf_append(lbuf, "\"");
	} else
	    sudo_lbuf_append_quoted(lbuf, SUDOERS_QUOTED, "%s", d->val);
    } else {
	sudo_lbuf_append(lbuf, "%s%s", d->op == false ? "!" : "", d->var);
    }
    debug_return_bool(!sudo_lbuf_error(lbuf));
}
