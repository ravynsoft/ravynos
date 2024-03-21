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
#include <unistd.h>
#include <pwd.h>

#include <sudoers.h>
#include <gram.h>

static int
runas_matches_pw(struct sudoers_parse_tree *parse_tree,
    const struct cmndspec *cs, const struct passwd *pw)
{
    debug_decl(runas_matches_pw, SUDOERS_DEBUG_PARSER);

    if (cs->runasuserlist != NULL)
	debug_return_int(userlist_matches(parse_tree, pw, cs->runasuserlist));

    if (cs->runasgrouplist == NULL) {
	/* No explicit runas user or group, use default. */
	if (userpw_matches(def_runas_default, pw->pw_name, pw) == ALLOW)
	    debug_return_int(ALLOW);
    }
    debug_return_int(UNSPEC);
}

/*
 * Look up the user in the sudoers parse tree for pseudo-commands like
 * list, verify and kill.
 */
static unsigned int
sudoers_lookup_pseudo(struct sudo_nss_list *snl, struct sudoers_context *ctx,
    time_t now, sudoers_lookup_callback_fn_t callback, void *cb_data,
    int pwflag)
{
    char *saved_runchroot;
    struct passwd *root_pw = NULL;
    struct sudo_nss *nss;
    struct cmndspec *cs;
    struct privilege *priv;
    struct userspec *us;
    struct defaults *def;
    int nopass, match = UNSPEC;
    unsigned int validated = 0;
    enum def_tuple pwcheck;
    debug_decl(sudoers_lookup_pseudo, SUDOERS_DEBUG_PARSER);

    pwcheck = (pwflag == -1) ? never : sudo_defs_table[pwflag].sd_un.tuple;
    nopass = (pwcheck == never || pwcheck == all) ? true : false;

    if (ctx->runas.list_pw != NULL) {
	root_pw = sudo_getpwuid(ROOT_UID);
	if (root_pw == NULL)
	    sudo_warnx(U_("unknown uid %u"), ROOT_UID);
    } else {
	SET(validated, FLAG_NO_CHECK);
    }

    /* Don't use chroot setting for pseudo-commands. */
    saved_runchroot = def_runchroot;
    def_runchroot = NULL;

    TAILQ_FOREACH(nss, snl, entries) {
	if (nss->query(ctx, nss, ctx->user.pw) == -1) {
	    /* The query function should have printed an error message. */
	    SET(validated, VALIDATE_ERROR);
	    break;
	}

	/*
	 * We have to traverse the policy forwards, not in reverse,
	 * to support the "pwcheck == all" case.
	 */
	TAILQ_FOREACH(us, &nss->parse_tree->userspecs, entries) {
	    const int user_match = userlist_matches(nss->parse_tree,
		ctx->user.pw, &us->users);
	    if (user_match != ALLOW) {
		if (callback != NULL && user_match == DENY) {
		    callback(nss->parse_tree, us, user_match, NULL, UNSPEC,
			NULL, UNSPEC, UNSPEC, UNSPEC, cb_data);
		}
		continue;
	    }
	    TAILQ_FOREACH(priv, &us->privileges, entries) {
		int priv_nopass = UNSPEC;
		const int host_match = hostlist_matches(nss->parse_tree,
		    ctx->user.pw, &priv->hostlist);
		if (host_match != ALLOW) {
		    if (callback != NULL) {
			callback(nss->parse_tree, us, user_match, priv,
			    host_match, NULL, UNSPEC, UNSPEC, UNSPEC, cb_data);
		    }
		    continue;
		}
		TAILQ_FOREACH(def, &priv->defaults, entries) {
		    if (strcmp(def->var, "authenticate") == 0) {
			priv_nopass = !def->op;
			break;
		    }
		}
		TAILQ_FOREACH(cs, &priv->cmndlist, entries) {
		    int cmnd_match = UNSPEC;
		    int date_match = UNSPEC;
		    int runas_match = UNSPEC;

		    if (pwcheck == any) {
			if (cs->tags.nopasswd == true || priv_nopass == true)
			    nopass = true;
		    } else if (pwcheck == all) {
			if (cs->tags.nopasswd != true && priv_nopass != true)
			    nopass = false;
		    }

		    if (cs->notbefore != UNSPEC) {
			date_match = now < cs->notbefore ? DENY : ALLOW;
		    }
		    if (cs->notafter != UNSPEC) {
			date_match = now > cs->notafter ? DENY : ALLOW;
		    }
		    /*
		     * Root can list any user's privileges.
		     * A user may always list their own privileges.
		     */
		    if (ctx->user.uid == 0 || ctx->runas.list_pw == NULL ||
			    ctx->user.uid == ctx->runas.list_pw->pw_uid) {
			cmnd_match = ALLOW;
			runas_match = ALLOW;
		    } else if (date_match != DENY) {
			/*
			 * To list another user's prilileges, the runas
			 * user must match the list user or root.
			 */
			runas_match = runas_matches_pw(nss->parse_tree, cs,
			    ctx->runas.list_pw);
			switch (runas_match) {
			case DENY:
			    break;
			case ALLOW:
			    /*
			     * RunAs user matches list user.
			     * Match on command "list" or ALL.
			     */
			    cmnd_match = cmnd_matches(nss->parse_tree,
				cs->cmnd, cs->runchroot, NULL);
			    break;
			default:
			    /*
			     * RunAs user doesn't match list user.
			     * Only allow listing if the user has
			     * "sudo ALL" for root.
			     */
			    if (root_pw != NULL &&
				    runas_matches_pw(nss->parse_tree, cs,
				    root_pw) == ALLOW) {
				runas_match = ALLOW;
				cmnd_match = cmnd_matches_all(nss->parse_tree,
				    cs->cmnd, cs->runchroot, NULL);
			    }
			    break;
			}
		    }
		    if (callback != NULL) {
			callback(nss->parse_tree, us, user_match, priv,
			    host_match, cs, date_match, runas_match,
			    cmnd_match, cb_data);
		    }
		    if (SPECIFIED(cmnd_match)) {
			/*
			 * We take the last match but must process
			 * the entire policy for pwcheck == all.
			 */
			match = cmnd_match;
		    }
		}
	    }
	}
	if (!sudo_nss_can_continue(nss, match))
	    break;
    }
    if (root_pw != NULL)
	sudo_pw_delref(root_pw);
    if (match == ALLOW || ctx->user.uid == 0) {
	/* User has an entry for this host. */
	SET(validated, VALIDATE_SUCCESS);
    } else {
	/* No entry or user is not allowed to list other users. */
	SET(validated, VALIDATE_FAILURE);
    }
    if (pwcheck == always && def_authenticate)
	SET(validated, FLAG_CHECK_USER);
    else if (nopass == true)
	def_authenticate = false;

    /* Restore original def_runchroot. */
    def_runchroot = saved_runchroot;

    debug_return_uint(validated);
}

static int
sudoers_lookup_check(struct sudo_nss *nss, struct sudoers_context *ctx,
    unsigned int *validated, struct cmnd_info *info, time_t now,
    sudoers_lookup_callback_fn_t callback, void *cb_data,
    struct cmndspec **matching_cs, struct defaults_list **defs)
{
    struct cmndspec *cs;
    struct privilege *priv;
    struct userspec *us;
    struct member *matching_user;
    debug_decl(sudoers_lookup_check, SUDOERS_DEBUG_PARSER);

    memset(info, 0, sizeof(*info));

    TAILQ_FOREACH_REVERSE(us, &nss->parse_tree->userspecs, userspec_list, entries) {
	const int user_match = userlist_matches(nss->parse_tree, ctx->user.pw,
	    &us->users);
	if (user_match != ALLOW) {
	    if (callback != NULL && user_match == DENY) {
		callback(nss->parse_tree, us, user_match, NULL, UNSPEC, NULL,
		    UNSPEC, UNSPEC, UNSPEC, cb_data);
	    }
	    continue;
	}
	CLR(*validated, FLAG_NO_USER);
	TAILQ_FOREACH_REVERSE(priv, &us->privileges, privilege_list, entries) {
	    const int host_match = hostlist_matches(nss->parse_tree,
		ctx->user.pw, &priv->hostlist);
	    if (host_match == ALLOW) {
		CLR(*validated, FLAG_NO_HOST);
	    } else {
		if (callback != NULL) {
		    callback(nss->parse_tree, us, user_match, priv, host_match,
			NULL, UNSPEC, UNSPEC, UNSPEC, cb_data);
		}
		continue;
	    }
	    TAILQ_FOREACH_REVERSE(cs, &priv->cmndlist, cmndspec_list, entries) {
		int cmnd_match = UNSPEC;
		int date_match = UNSPEC;
		int runas_match = UNSPEC;

		if (cs->notbefore != UNSPEC) {
		    date_match = now < cs->notbefore ? DENY : ALLOW;
		}
		if (cs->notafter != UNSPEC) {
		    date_match = now > cs->notafter ? DENY : ALLOW;
		}
		if (date_match != DENY) {
		    matching_user = NULL;
		    runas_match = runaslist_matches(nss->parse_tree,
			cs->runasuserlist, cs->runasgrouplist, &matching_user,
			NULL);
		    if (runas_match == ALLOW) {
			cmnd_match = cmnd_matches(nss->parse_tree, cs->cmnd,
			    cs->runchroot, info);
		    }
		}
		if (callback != NULL) {
		    callback(nss->parse_tree, us, user_match, priv, host_match,
			cs, date_match, runas_match, cmnd_match, cb_data);
		}

		if (SPECIFIED(cmnd_match)) {
		    /*
		     * If user is running command as themselves,
		     * set ctx->runas.pw = ctx->user.pw.
		     * XXX - hack, want more general solution
		     */
		    if (matching_user && matching_user->type == MYSELF) {
			sudo_pw_delref(ctx->runas.pw);
			sudo_pw_addref(ctx->user.pw);
			ctx->runas.pw = ctx->user.pw;
		    }
		    *matching_cs = cs;
		    *defs = &priv->defaults;
		    sudo_debug_printf(SUDO_DEBUG_DEBUG|SUDO_DEBUG_LINENO,
			"userspec matched @ %s:%d:%d: %s",
			us->file ? us->file : "???", us->line, us->column,
			cmnd_match ? "allowed" : "denied");
		    debug_return_int(cmnd_match);
		}
		free(info->cmnd_path);
		memset(info, 0, sizeof(*info));
	    }
	}
    }
    debug_return_int(UNSPEC);
}

/*
 * Apply cmndspec-specific settings including SELinux role/type,
 * Solaris privs, and command tags.
 */
static bool
apply_cmndspec(struct sudoers_context *ctx, struct cmndspec *cs)
{
    debug_decl(apply_cmndspec, SUDOERS_DEBUG_PARSER);

    if (cs != NULL) {
#ifdef HAVE_SELINUX
	/* Set role and type if not specified on command line. */
	if (ctx->runas.role == NULL) {
	    if (cs->role != NULL) {
		ctx->runas.role = strdup(cs->role);
		if (ctx->runas.role == NULL) {
		    sudo_warnx(U_("%s: %s"), __func__,
			U_("unable to allocate memory"));
		    debug_return_bool(false);
		}
	    } else {
		ctx->runas.role = def_role;
		def_role = NULL;
	    }
	    if (ctx->runas.role != NULL) {
		sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
		    "ctx->runas.role -> %s", ctx->runas.role);
	    }
	}
	if (ctx->runas.type == NULL) {
	    if (cs->type != NULL) {
		ctx->runas.type = strdup(cs->type);
		if (ctx->runas.type == NULL) {
		    sudo_warnx(U_("%s: %s"), __func__,
			U_("unable to allocate memory"));
		    debug_return_bool(false);
		}
	    } else {
		ctx->runas.type = def_type;
		def_type = NULL;
	    }
	    if (ctx->runas.type != NULL) {
		sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
		    "ctx->runas.type -> %s", ctx->runas.type);
	    }
	}
#endif /* HAVE_SELINUX */
#ifdef HAVE_APPARMOR
	/* Set AppArmor profile, if specified */
	if (cs->apparmor_profile != NULL) {
	    ctx->runas.apparmor_profile = strdup(cs->apparmor_profile);
	    if (ctx->runas.apparmor_profile == NULL) {
		sudo_warnx(U_("%s: %s"), __func__,
		    U_("unable to allocate memory"));
		debug_return_bool(false);
	    }
	} else {
	    ctx->runas.apparmor_profile = def_apparmor_profile;
	    def_apparmor_profile = NULL;
	}
	if (ctx->runas.apparmor_profile != NULL) {
	    sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
		"ctx->runas.apparmor_profile -> %s", ctx->runas.apparmor_profile);
	}
#endif
#ifdef HAVE_PRIV_SET
	/* Set Solaris privilege sets */
	if (ctx->runas.privs == NULL) {
	    if (cs->privs != NULL) {
		ctx->runas.privs = strdup(cs->privs);
		if (ctx->runas.privs == NULL) {
		    sudo_warnx(U_("%s: %s"), __func__,
			U_("unable to allocate memory"));
		    debug_return_bool(false);
		}
	    } else {
		ctx->runas.privs = def_privs;
		def_privs = NULL;
	    }
	    if (ctx->runas.privs != NULL) {
		sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
		    "ctx->runas.privs -> %s", ctx->runas.privs);
	    }
	}
	if (ctx->runas.limitprivs == NULL) {
	    if (cs->limitprivs != NULL) {
		ctx->runas.limitprivs = strdup(cs->limitprivs);
		if (ctx->runas.limitprivs == NULL) {
		    sudo_warnx(U_("%s: %s"), __func__,
			U_("unable to allocate memory"));
		    debug_return_bool(false);
		}
	    } else {
		ctx->runas.limitprivs = def_limitprivs;
		def_limitprivs = NULL;
	    }
	    if (ctx->runas.limitprivs != NULL) {
		sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
		    "ctx->runas.limitprivs -> %s", ctx->runas.limitprivs);
	    }
	}
#endif /* HAVE_PRIV_SET */
	if (cs->timeout > 0) {
	    def_command_timeout = cs->timeout;
	    sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
		"def_command_timeout -> %d", def_command_timeout);
	}
	if (cs->runcwd != NULL) {
	    free(def_runcwd);
	    def_runcwd = strdup(cs->runcwd);
	    if (def_runcwd == NULL) {
		sudo_warnx(U_("%s: %s"), __func__,
		    U_("unable to allocate memory"));
		debug_return_bool(false);
	    }
	    sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
		"def_runcwd -> %s", def_runcwd);
	}
	if (cs->runchroot != NULL) {
	    free(def_runchroot);
	    def_runchroot = strdup(cs->runchroot);
	    if (def_runchroot == NULL) {
		sudo_warnx(U_("%s: %s"), __func__,
		    U_("unable to allocate memory"));
		debug_return_bool(false);
	    }
	    sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
		"def_runchroot -> %s", def_runchroot);
	}
	if (cs->tags.nopasswd != UNSPEC) {
	    def_authenticate = !cs->tags.nopasswd;
	    sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
		"def_authenticate -> %s", def_authenticate ? "true" : "false");
	}
	if (cs->tags.noexec != UNSPEC) {
	    def_noexec = cs->tags.noexec;
	    sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
		"def_noexec -> %s", def_noexec ? "true" : "false");
	}
	if (cs->tags.intercept != UNSPEC) {
	    def_intercept = cs->tags.intercept;
	    sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
		"def_intercept -> %s", def_intercept ? "true" : "false");
	}
	if (cs->tags.setenv != UNSPEC) {
	    def_setenv = cs->tags.setenv;
	    sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
		"def_setenv -> %s", def_setenv ? "true" : "false");
	}
	if (cs->tags.log_input != UNSPEC) {
	    def_log_input = cs->tags.log_input;
	    cb_log_input(ctx, NULL, 0, 0, NULL, cs->tags.log_input);
	    sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
		"def_log_input -> %s", def_log_input ? "true" : "false");
	}
	if (cs->tags.log_output != UNSPEC) {
	    def_log_output = cs->tags.log_output;
	    cb_log_output(ctx, NULL, 0, 0, NULL, cs->tags.log_output);
	    sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
		"def_log_output -> %s", def_log_output ? "true" : "false");
	}
	if (cs->tags.send_mail != UNSPEC) {
	    if (cs->tags.send_mail) {
		def_mail_all_cmnds = true;
		sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
		    "def_mail_all_cmnds -> true");
	    } else {
		def_mail_all_cmnds = false;
		def_mail_always = false;
		def_mail_no_perms = false;
		sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
		    "def_mail_all_cmnds -> false, def_mail_always -> false, "
		    "def_mail_no_perms -> false");
	    }
	}
	if (cs->tags.follow != UNSPEC) {
	    def_sudoedit_follow = cs->tags.follow;
	    sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
		"def_sudoedit_follow -> %s", def_sudoedit_follow ? "true" : "false");
	}
    }

    debug_return_bool(true);
}

/*
 * Look up the user in the sudoers parse tree and check to see if they are
 * allowed to run the specified command on this host as the target user.
 */
unsigned int
sudoers_lookup(struct sudo_nss_list *snl, struct sudoers_context *ctx,
    time_t now, sudoers_lookup_callback_fn_t callback, void *cb_data,
    int *cmnd_status, int pwflag)
{
    struct defaults_list *defs = NULL;
    struct sudoers_parse_tree *parse_tree = NULL;
    struct cmndspec *cs = NULL;
    struct sudo_nss *nss;
    struct cmnd_info info;
    unsigned int validated = FLAG_NO_USER | FLAG_NO_HOST;
    int m, match = UNSPEC;
    debug_decl(sudoers_lookup, SUDOERS_DEBUG_PARSER);

    /*
     * Special case checking the "validate", "list" and "kill" pseudo-commands.
     */
    if (pwflag) {
	debug_return_uint(sudoers_lookup_pseudo(snl, ctx, now, callback,
	    cb_data, pwflag));
    }

    /* Need to be runas user while stat'ing things. */
    if (!set_perms(ctx, PERM_RUNAS))
	debug_return_uint(validated);

    /* Query each sudoers source and check the user. */
    TAILQ_FOREACH(nss, snl, entries) {
	if (nss->query(ctx, nss, ctx->user.pw) == -1) {
	    /* The query function should have printed an error message. */
	    SET(validated, VALIDATE_ERROR);
	    break;
	}

	m = sudoers_lookup_check(nss, ctx, &validated, &info, now, callback,
	    cb_data, &cs, &defs);
	if (SPECIFIED(m)) {
	    match = m;
	    parse_tree = nss->parse_tree;
	}

	if (!sudo_nss_can_continue(nss, m))
	    break;
    }
    if (SPECIFIED(match)) {
	if (info.cmnd_path != NULL) {
	    /* Update cmnd, cmnd_stat, cmnd_status from matching entry. */
	    free(ctx->user.cmnd);
	    ctx->user.cmnd = info.cmnd_path;
	    if (ctx->user.cmnd_stat != NULL)
		*ctx->user.cmnd_stat = info.cmnd_stat;
	    *cmnd_status = info.status;
	}
	if (defs != NULL)
	    (void)update_defaults(ctx, parse_tree, defs, SETDEF_GENERIC, false);
	if (!apply_cmndspec(ctx, cs))
	    SET(validated, VALIDATE_ERROR);
	else if (match == ALLOW)
	    SET(validated, VALIDATE_SUCCESS);
	else
	    SET(validated, VALIDATE_FAILURE);
    }
    if (!restore_perms())
	SET(validated, VALIDATE_ERROR);
    debug_return_uint(validated);
}
