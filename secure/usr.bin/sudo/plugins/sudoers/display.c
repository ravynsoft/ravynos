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

#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <pwd.h>

#include <sudoers.h>
#include <sudo_lbuf.h>
#include <gram.h>

static int
display_priv_short(const struct sudoers_parse_tree *parse_tree,
    const struct passwd *pw, const struct userspec *us, struct sudo_lbuf *lbuf)
{
    struct privilege *priv;
    int nfound = 0;
    debug_decl(display_priv_short, SUDOERS_DEBUG_PARSER);

    TAILQ_FOREACH(priv, &us->privileges, entries) {
	struct cmndspec *cs;
	struct cmndtag tags;

	if (hostlist_matches(parse_tree, pw, &priv->hostlist) != ALLOW)
	    continue;

	sudoers_defaults_list_to_tags(&priv->defaults, &tags);
	TAILQ_FOREACH(cs, &priv->cmndlist, entries) {
	    struct cmndspec *prev_cs = TAILQ_PREV(cs, cmndspec_list, entries);

	    if (prev_cs == NULL || RUNAS_CHANGED(cs, prev_cs)) {
		struct member *m;

		/* Start new line, first entry or RunAs changed. */
		if (prev_cs != NULL)
		    sudo_lbuf_append(lbuf, "\n");
		sudo_lbuf_append(lbuf, "    (");
		if (cs->runasuserlist != NULL) {
		    TAILQ_FOREACH(m, cs->runasuserlist, entries) {
			if (m != TAILQ_FIRST(cs->runasuserlist))
			    sudo_lbuf_append(lbuf, ", ");
			sudoers_format_member(lbuf, parse_tree, m, ", ",
			    RUNASALIAS);
		    }
		} else if (cs->runasgrouplist == NULL) {
		    sudo_lbuf_append(lbuf, "%s", def_runas_default);
		} else {
		    sudo_lbuf_append(lbuf, "%s", pw->pw_name);
		}
		if (cs->runasgrouplist != NULL) {
		    sudo_lbuf_append(lbuf, " : ");
		    TAILQ_FOREACH(m, cs->runasgrouplist, entries) {
			if (m != TAILQ_FIRST(cs->runasgrouplist))
			    sudo_lbuf_append(lbuf, ", ");
			sudoers_format_member(lbuf, parse_tree, m, ", ",
			    RUNASALIAS);
		    }
		}
		sudo_lbuf_append(lbuf, ") ");
		sudoers_format_cmndspec(lbuf, parse_tree, cs, NULL,
		    tags, true);
	    } else {
		/* Continue existing line. */
		sudo_lbuf_append(lbuf, ", ");
		sudoers_format_cmndspec(lbuf, parse_tree, cs, prev_cs,
		    tags, true);
	    }
	    nfound++;
	}
	sudo_lbuf_append(lbuf, "\n");
    }
    debug_return_int(nfound);
}

/*
 * Compare the current cmndspec with the previous one to determine
 * whether we need to start a new long entry for "sudo -ll".
 * Returns true if we should start a new long entry, else false.
 */
static bool
new_long_entry(const struct cmndspec *cs, const struct cmndspec *prev_cs)
{
    debug_decl(new_long_entry, SUDOERS_DEBUG_PARSER);

    if (prev_cs == NULL)
	debug_return_bool(true);
    if (RUNAS_CHANGED(cs, prev_cs) || TAGS_CHANGED(prev_cs->tags, cs->tags))
	debug_return_bool(true);
#ifdef HAVE_PRIV_SET
    if (cs->privs && (!prev_cs->privs || strcmp(cs->privs, prev_cs->privs) != 0))
	debug_return_bool(true);
    if (cs->limitprivs && (!prev_cs->limitprivs || strcmp(cs->limitprivs, prev_cs->limitprivs) != 0))
	debug_return_bool(true);
#endif /* HAVE_PRIV_SET */
#ifdef HAVE_SELINUX
    if (cs->role && (!prev_cs->role || strcmp(cs->role, prev_cs->role) != 0))
	debug_return_bool(true);
    if (cs->type && (!prev_cs->type || strcmp(cs->type, prev_cs->type) != 0))
	debug_return_bool(true);
#endif /* HAVE_SELINUX */
#ifdef HAVE_APPARMOR
    if (cs->apparmor_profile && (!prev_cs->apparmor_profile || strcmp(cs->apparmor_profile, prev_cs->apparmor_profile) != 0))
	debug_return_bool(true);
#endif /* HAVE_APPARMOR */
    if (cs->runchroot && (!prev_cs->runchroot || strcmp(cs->runchroot, prev_cs->runchroot) != 0))
	debug_return_bool(true);
    if (cs->runcwd && (!prev_cs->runcwd || strcmp(cs->runcwd, prev_cs->runcwd) != 0))
	debug_return_bool(true);
    if (cs->timeout != prev_cs->timeout)
	debug_return_bool(true);
    if (cs->notbefore != prev_cs->notbefore)
	debug_return_bool(true);
    if (cs->notafter != prev_cs->notafter)
	debug_return_bool(true);
    debug_return_bool(false);
}

static void
display_cmndspec_long(const struct sudoers_parse_tree *parse_tree,
    const struct passwd *pw, const struct userspec *us,
    const struct privilege *priv, const struct cmndspec *cs,
    const struct cmndspec *prev_cs, struct sudo_lbuf *lbuf)
{
    const struct defaults *d;
    const struct member *m;
    debug_decl(display_cmndspec_long, SUDOERS_DEBUG_PARSER);

    if (new_long_entry(cs, prev_cs)) {
	unsigned int olen;

	if (prev_cs != NULL)
	    sudo_lbuf_append(lbuf, "\n");
	if (priv->ldap_role != NULL) {
	    sudo_lbuf_append(lbuf, _("LDAP Role: %s\n"),
		priv->ldap_role);
	} else {
	    sudo_lbuf_append(lbuf, _("Sudoers entry: %s\n"),
		us->file);
	}
	sudo_lbuf_append(lbuf, "%s", _("    RunAsUsers: "));
	if (cs->runasuserlist != NULL) {
	    TAILQ_FOREACH(m, cs->runasuserlist, entries) {
		if (m != TAILQ_FIRST(cs->runasuserlist))
		    sudo_lbuf_append(lbuf, ", ");
		sudoers_format_member(lbuf, parse_tree, m, ", ",
		    RUNASALIAS);
	    }
	} else if (cs->runasgrouplist == NULL) {
	    sudo_lbuf_append(lbuf, "%s", def_runas_default);
	} else {
	    sudo_lbuf_append(lbuf, "%s", pw->pw_name);
	}
	sudo_lbuf_append(lbuf, "\n");
	if (cs->runasgrouplist != NULL) {
	    sudo_lbuf_append(lbuf, "%s", _("    RunAsGroups: "));
	    TAILQ_FOREACH(m, cs->runasgrouplist, entries) {
		if (m != TAILQ_FIRST(cs->runasgrouplist))
		    sudo_lbuf_append(lbuf, ", ");
		sudoers_format_member(lbuf, parse_tree, m, ", ",
		    RUNASALIAS);
	    }
	    sudo_lbuf_append(lbuf, "\n");
	}
	olen = lbuf->len;
	sudo_lbuf_append(lbuf, "%s", _("    Options: "));
	TAILQ_FOREACH(d, &priv->defaults, entries) {
	    sudoers_format_default(lbuf, d);
	    sudo_lbuf_append(lbuf, ", ");
	}
	if (TAG_SET(cs->tags.setenv))
	    sudo_lbuf_append(lbuf, "%ssetenv, ", cs->tags.setenv ? "" : "!");
	if (TAG_SET(cs->tags.noexec))
	    sudo_lbuf_append(lbuf, "%snoexec, ", cs->tags.noexec ? "" : "!");
	if (TAG_SET(cs->tags.intercept))
	    sudo_lbuf_append(lbuf, "%sintercept, ", cs->tags.intercept ? "" : "!");
	if (TAG_SET(cs->tags.nopasswd))
	    sudo_lbuf_append(lbuf, "%sauthenticate, ", cs->tags.nopasswd ? "!" : "");
	if (TAG_SET(cs->tags.log_input))
	    sudo_lbuf_append(lbuf, "%slog_input, ", cs->tags.log_input ? "" : "!");
	if (TAG_SET(cs->tags.log_output))
	    sudo_lbuf_append(lbuf, "%slog_output, ", cs->tags.log_output ? "" : "!");
	if (lbuf->buf[lbuf->len - 2] == ',') {
	    lbuf->len -= 2;	/* remove trailing ", " */
	    sudo_lbuf_append(lbuf, "\n");
	} else {
	    lbuf->len = olen;	/* no options */
	}
#ifdef HAVE_PRIV_SET
	if (cs->privs)
	    sudo_lbuf_append(lbuf, "    Privs: %s\n", cs->privs);
	if (cs->limitprivs)
	    sudo_lbuf_append(lbuf, "    Limitprivs: %s\n", cs->limitprivs);
#endif /* HAVE_PRIV_SET */
#ifdef HAVE_SELINUX
	if (cs->role)
	    sudo_lbuf_append(lbuf, "    Role: %s\n", cs->role);
	if (cs->type)
	    sudo_lbuf_append(lbuf, "    Type: %s\n", cs->type);
#endif /* HAVE_SELINUX */
	if (cs->runchroot != NULL)
	    sudo_lbuf_append(lbuf, "    Chroot: %s\n", cs->runchroot);
	if (cs->runcwd != NULL)
	    sudo_lbuf_append(lbuf, "    Cwd: %s\n", cs->runcwd);
	if (cs->timeout > 0) {
	    char numbuf[STRLEN_MAX_SIGNED(int) + 1];
	    (void)snprintf(numbuf, sizeof(numbuf), "%d", cs->timeout);
	    sudo_lbuf_append(lbuf, "    Timeout: %s\n", numbuf);
	}
	if (cs->notbefore != UNSPEC) {
	    char buf[sizeof("CCYYMMDDHHMMSSZ")] = "";
	    struct tm gmt;
	    size_t len;
	    if (gmtime_r(&cs->notbefore, &gmt) != NULL) {
		len = strftime(buf, sizeof(buf), "%Y%m%d%H%M%SZ", &gmt);
		if (len != 0 && buf[sizeof(buf) - 1] == '\0')
		    sudo_lbuf_append(lbuf, "    NotBefore: %s\n", buf);
	    }
	}
	if (cs->notafter != UNSPEC) {
	    char buf[sizeof("CCYYMMDDHHMMSSZ")] = "";
	    struct tm gmt;
	    size_t len;
	    if (gmtime_r(&cs->notafter, &gmt) != NULL) {
		len = strftime(buf, sizeof(buf), "%Y%m%d%H%M%SZ", &gmt);
		if (len != 0 && buf[sizeof(buf) - 1] == '\0')
		    sudo_lbuf_append(lbuf, "    NotAfter: %s\n", buf);
	    }
	}
	sudo_lbuf_append(lbuf, "%s", _("    Commands:\n"));
    }
    sudo_lbuf_append(lbuf, "\t");
    sudoers_format_member(lbuf, parse_tree, cs->cmnd, "\n\t",
	CMNDALIAS);
    sudo_lbuf_append(lbuf, "\n");

    debug_return;
}

static int
display_priv_long(const struct sudoers_parse_tree *parse_tree,
    const struct passwd *pw, const struct userspec *us, struct sudo_lbuf *lbuf)
{
    const struct privilege *priv;
    int nfound = 0;
    debug_decl(display_priv_long, SUDOERS_DEBUG_PARSER);

    TAILQ_FOREACH(priv, &us->privileges, entries) {
	const struct cmndspec *cs, *prev_cs;

	if (hostlist_matches(parse_tree, pw, &priv->hostlist) != ALLOW)
	    continue;
	prev_cs = NULL;
	sudo_lbuf_append(lbuf, "\n");
	TAILQ_FOREACH(cs, &priv->cmndlist, entries) {
	    display_cmndspec_long(parse_tree, pw, us, priv, cs, prev_cs,
		lbuf);
	    prev_cs = cs;
	    nfound++;
	}
    }
    debug_return_int(nfound);
}

static int
sudo_display_userspecs(struct sudoers_parse_tree *parse_tree,
    const struct passwd *pw, struct sudo_lbuf *lbuf, bool verbose)
{
    const struct userspec *us;
    int nfound = 0;
    debug_decl(sudo_display_userspecs, SUDOERS_DEBUG_PARSER);

    TAILQ_FOREACH(us, &parse_tree->userspecs, entries) {
	if (userlist_matches(parse_tree, pw, &us->users) != ALLOW)
	    continue;

	if (verbose)
	    nfound += display_priv_long(parse_tree, pw, us, lbuf);
	else
	    nfound += display_priv_short(parse_tree, pw, us, lbuf);
    }
    if (sudo_lbuf_error(lbuf))
	debug_return_int(-1);
    debug_return_int(nfound);
}

/*
 * Display matching Defaults entries for the given user on this host.
 */
static int
display_defaults(const struct sudoers_parse_tree *parse_tree,
    const struct passwd *pw, struct sudo_lbuf *lbuf)
{
    const struct defaults *d;
    const char *prefix;
    int nfound = 0;
    debug_decl(display_defaults, SUDOERS_DEBUG_PARSER);

    if (lbuf->len == 0 || isspace((unsigned char)lbuf->buf[lbuf->len - 1]))
	prefix = "    ";
    else
	prefix = ", ";

    TAILQ_FOREACH(d, &parse_tree->defaults, entries) {
	switch (d->type) {
	    case DEFAULTS_HOST:
		if (hostlist_matches(parse_tree, pw, &d->binding->members) != ALLOW)
		    continue;
		break;
	    case DEFAULTS_USER:
		if (userlist_matches(parse_tree, pw, &d->binding->members) != ALLOW)
		    continue;
		break;
	    case DEFAULTS_RUNAS:
	    case DEFAULTS_CMND:
		continue;
	}
	sudo_lbuf_append(lbuf, "%s", prefix);
	sudoers_format_default(lbuf, d);
	prefix = ", ";
	nfound++;
    }
    if (sudo_lbuf_error(lbuf))
	debug_return_int(-1);
    debug_return_int(nfound);
}

/*
 * Display Defaults entries of the given type.
 */
static int
display_bound_defaults_by_type(const struct sudoers_parse_tree *parse_tree,
    int deftype, struct sudo_lbuf *lbuf)
{
    const struct defaults *d;
    const struct defaults_binding *binding = NULL;
    const struct member *m;
    const char *dsep;
    short atype;
    int nfound = 0;
    debug_decl(display_bound_defaults_by_type, SUDOERS_DEBUG_PARSER);

    switch (deftype) {
	case DEFAULTS_HOST:
	    atype = HOSTALIAS;
	    dsep = "@";
	    break;
	case DEFAULTS_USER:
	    atype = USERALIAS;
	    dsep = ":";
	    break;
	case DEFAULTS_RUNAS:
	    atype = RUNASALIAS;
	    dsep = ">";
	    break;
	case DEFAULTS_CMND:
	    atype = CMNDALIAS;
	    dsep = "!";
	    break;
	default:
	    debug_return_int(-1);
    }
    TAILQ_FOREACH(d, &parse_tree->defaults, entries) {
	if (d->type != deftype)
	    continue;

	nfound++;
	if (binding != d->binding) {
	    binding = d->binding;
	    if (nfound != 1)
		sudo_lbuf_append(lbuf, "\n");
	    sudo_lbuf_append(lbuf, "    Defaults%s", dsep);
	    TAILQ_FOREACH(m, &binding->members, entries) {
		if (m != TAILQ_FIRST(&binding->members))
		    sudo_lbuf_append(lbuf, ", ");
		sudoers_format_member(lbuf, parse_tree, m, ", ", atype);
	    }
	    sudo_lbuf_append(lbuf, " ");
	} else
	    sudo_lbuf_append(lbuf, ", ");
	sudoers_format_default(lbuf, d);
    }

    if (sudo_lbuf_error(lbuf))
	debug_return_int(-1);
    debug_return_int(nfound);
}

/*
 * Display Defaults entries that are per-runas or per-command
 */
static int
display_bound_defaults(const struct sudoers_parse_tree *parse_tree,
    const struct passwd *pw, struct sudo_lbuf *lbuf)
{
    int nfound = 0;
    debug_decl(display_bound_defaults, SUDOERS_DEBUG_PARSER);

    /* XXX - should only print ones that match what the user can do. */
    nfound += display_bound_defaults_by_type(parse_tree, DEFAULTS_RUNAS,
	lbuf);
    nfound += display_bound_defaults_by_type(parse_tree, DEFAULTS_CMND,
	lbuf);

    if (sudo_lbuf_error(lbuf))
	debug_return_int(-1);
    debug_return_int(nfound);
}

static int
output(const char *buf)
{
    struct sudo_conv_message msg;
    struct sudo_conv_reply repl;
    debug_decl(output, SUDOERS_DEBUG_NSS);

    /* Call conversation function */
    memset(&msg, 0, sizeof(msg));
    msg.msg_type = SUDO_CONV_INFO_MSG;
    msg.msg = buf;
    memset(&repl, 0, sizeof(repl));
    if (sudo_conv(1, &msg, &repl, NULL) == -1)
	debug_return_int(0);
    debug_return_int((int)strlen(buf));
}

/*
 * Print out privileges for the specified user.
 * Returns true on success or -1 on error.
 */
int
display_privs(struct sudoers_context *ctx, const struct sudo_nss_list *snl,
    struct passwd *pw, int verbose)
{
    const struct sudo_nss *nss;
    struct sudo_lbuf def_buf, priv_buf;
    int cols, count, n;
    unsigned int olen;
    struct stat sb;
    debug_decl(display_privs, SUDOERS_DEBUG_PARSER);

    if (verbose < 0) {
	/* Nothing to display. */
	debug_return_int(true);
    }

    cols = ctx->user.cols;
    if (fstat(STDOUT_FILENO, &sb) == 0 && S_ISFIFO(sb.st_mode))
	cols = 0;
    sudo_lbuf_init(&def_buf, output, 4, NULL, cols);
    sudo_lbuf_init(&priv_buf, output, 8, NULL, cols);

    sudo_lbuf_append(&def_buf, _("Matching Defaults entries for %s on %s:\n"),
	pw->pw_name, ctx->runas.shost);
    count = 0;
    TAILQ_FOREACH(nss, snl, entries) {
	n = display_defaults(nss->parse_tree, pw, &def_buf);
	if (n == -1)
	    goto bad;
	count += n;
    }
    if (count != 0) {
	sudo_lbuf_append(&def_buf, "\n\n");
    } else {
	/* Undo Defaults header. */
	def_buf.len = 0;
    }

    /* Display Runas and Cmnd-specific defaults. */
    olen = def_buf.len;
    sudo_lbuf_append(&def_buf, _("Runas and Command-specific defaults for %s:\n"),
	pw->pw_name);
    count = 0;
    TAILQ_FOREACH(nss, snl, entries) {
	n = display_bound_defaults(nss->parse_tree, pw, &def_buf);
	if (n == -1)
	    goto bad;
	count += n;
    }
    if (count != 0) {
	sudo_lbuf_append(&def_buf, "\n\n");
    } else {
	/* Undo Defaults header. */
	def_buf.len = olen;
    }

    /* Display privileges from all sources. */
    sudo_lbuf_append(&priv_buf,
	_("User %s may run the following commands on %s:\n"),
	pw->pw_name, ctx->runas.shost);
    count = 0;
    TAILQ_FOREACH(nss, snl, entries) {
	if (nss->query(ctx, nss, pw) != -1) {
	    n = sudo_display_userspecs(nss->parse_tree, pw, &priv_buf,
		verbose);
	    if (n == -1)
		goto bad;
	    count += n;
	}
    }
    if (count == 0) {
	def_buf.len = 0;
	priv_buf.len = 0;
	sudo_lbuf_append(&priv_buf,
	    _("User %s is not allowed to run sudo on %s.\n"),
	    pw->pw_name, ctx->runas.shost);
    }
    if (sudo_lbuf_error(&def_buf) || sudo_lbuf_error(&priv_buf))
	goto bad;

    sudo_lbuf_print(&def_buf);
    sudo_lbuf_print(&priv_buf);

    sudo_lbuf_destroy(&def_buf);
    sudo_lbuf_destroy(&priv_buf);

    debug_return_int(true);
bad:
    sudo_lbuf_destroy(&def_buf);
    sudo_lbuf_destroy(&priv_buf);

    debug_return_int(-1);
}

static int
display_cmnd_check(struct sudoers_context *ctx,
    const struct sudoers_parse_tree *parse_tree, const struct passwd *pw,
    time_t now, struct sudoers_match_info *match_info)
{
    int host_match, runas_match, cmnd_match = UNSPEC;
    char *saved_user_cmnd, *saved_user_base;
    const struct privilege *priv;
    const struct userspec *us;
    const struct cmndspec *cs;
    debug_decl(display_cmnd_check, SUDOERS_DEBUG_PARSER);

    /*
     * For "sudo -l command", ctx->user.cmnd is "list" and the actual
     * command we are checking is in ctx->user.cmnd_list.
     */
    saved_user_cmnd = ctx->user.cmnd;
    saved_user_base = ctx->user.cmnd_base;
    ctx->user.cmnd = ctx->user.cmnd_list;
    ctx->user.cmnd_base = sudo_basename(ctx->user.cmnd);

    TAILQ_FOREACH_REVERSE(us, &parse_tree->userspecs, userspec_list, entries) {
	if (userlist_matches(parse_tree, pw, &us->users) != ALLOW)
	    continue;
	TAILQ_FOREACH_REVERSE(priv, &us->privileges, privilege_list, entries) {
	    host_match = hostlist_matches(parse_tree, pw, &priv->hostlist);
	    if (host_match != ALLOW)
		continue;
	    TAILQ_FOREACH_REVERSE(cs, &priv->cmndlist, cmndspec_list, entries) {
		if (cs->notbefore != UNSPEC) {
		    if (now < cs->notbefore)
			continue;
		}
		if (cs->notafter != UNSPEC) {
		    if (now > cs->notafter)
			continue;
		}
		runas_match = runaslist_matches(parse_tree, cs->runasuserlist,
		    cs->runasgrouplist, NULL, NULL);
		if (runas_match == ALLOW) {
		    cmnd_match = cmnd_matches(parse_tree, cs->cmnd,
			cs->runchroot, NULL);
		    if (cmnd_match != UNSPEC) {
			match_info->parse_tree = parse_tree;
			match_info->us = us;
			match_info->priv = priv;
			match_info->cs = cs;
			goto done;
		    }
		}
	    }
	}
    }
done:
    ctx->user.cmnd = saved_user_cmnd;
    ctx->user.cmnd_base = saved_user_base;
    debug_return_int(cmnd_match);
}

/*
 * Check ctx->user.cmnd against sudoers and print the matching entry if the
 * command is allowed.
 * Returns true if the command is allowed, false if not or -1 on error.
 */
int
display_cmnd(struct sudoers_context *ctx, const struct sudo_nss_list *snl,
    struct passwd *pw, int verbose)
{
    struct sudoers_match_info match_info = { NULL };
    struct sudo_lbuf lbuf;
    struct sudo_nss *nss;
    int m, match = UNSPEC;
    int ret = false;
    time_t now;
    debug_decl(display_cmnd, SUDOERS_DEBUG_PARSER);

    /* Iterate over each source, checking for the command. */
    time(&now);
    sudo_lbuf_init(&lbuf, output, 0, NULL, 0);
    TAILQ_FOREACH(nss, snl, entries) {
	if (nss->query(ctx, nss, pw) == -1) {
	    /* The query function should have printed an error message. */
	    debug_return_int(-1);
	}

	m = display_cmnd_check(ctx, nss->parse_tree, pw, now, &match_info);
	if (m != UNSPEC)
	    match = m;

	if (!sudo_nss_can_continue(nss, m))
	    break;
    }
    if (match == ALLOW) {
	if (verbose < 0) {
	    /* Nothing to display. */
	    debug_return_int(true);
	}
	if (verbose) {
	    /* Append matching sudoers rule (long form). */
	    display_cmndspec_long(match_info.parse_tree, pw, match_info.us,
		match_info.priv, match_info.cs, NULL, &lbuf);
	    sudo_lbuf_append(&lbuf, "    Matched: ");
	}
	sudo_lbuf_append(&lbuf, "%s%s%s\n", ctx->user.cmnd_list,
	    ctx->user.cmnd_args ? " " : "",
	    ctx->user.cmnd_args ? ctx->user.cmnd_args : "");
	sudo_lbuf_print(&lbuf);
	ret = sudo_lbuf_error(&lbuf) ? -1 : true;
	sudo_lbuf_destroy(&lbuf);
    }
    debug_return_int(ret);
}
