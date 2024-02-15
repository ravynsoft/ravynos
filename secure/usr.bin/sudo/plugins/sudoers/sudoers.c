/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 1993-1996, 1998-2023 Todd C. Miller <Todd.Miller@sudo.ws>
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
 *
 * Sponsored in part by the Defense Advanced Research Projects
 * Agency (DARPA) and Air Force Research Laboratory, Air Force
 * Materiel Command, USAF, under agreement number F39502-99-1-0512.
 */

/*
 * This is an open source non-commercial project. Dear PVS-Studio, please check it.
 * PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
 */

#ifdef __TANDEM
# include <floss.h>
#endif

#include <config.h>

#include <sys/types.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <netdb.h>
#ifdef HAVE_LOGIN_CAP_H
# include <login_cap.h>
# ifndef LOGIN_DEFROOTCLASS
#  define LOGIN_DEFROOTCLASS	"daemon"
# endif
# ifndef LOGIN_SETENV
#  define LOGIN_SETENV	0
# endif
#endif
#ifdef HAVE_SELINUX
# include <selinux/selinux.h>
#endif
#include <ctype.h>
#ifndef HAVE_GETADDRINFO
# include <compat/getaddrinfo.h>
#endif

#include <sudoers.h>
#include <timestamp.h>
#include <sudo_iolog.h>

/*
 * Prototypes
 */
static int set_cmnd(struct sudoers_context *ctx);
static bool init_vars(struct sudoers_context *ctx, char * const *);
static bool set_loginclass(struct sudoers_context *);
static bool set_runaspw(struct sudoers_context *ctx, const char *, bool);
static bool set_runasgr(struct sudoers_context *ctx, const char *, bool);

/*
 * Globals
 */
static char *prev_user;
static struct sudoers_context sudoers_ctx = SUDOERS_CONTEXT_INITIALIZER;
static struct sudo_nss_list *snl;
static bool unknown_runas_uid;
static bool unknown_runas_gid;
static int cmnd_status = NOT_FOUND_ERROR;
static struct defaults_list initial_defaults = TAILQ_HEAD_INITIALIZER(initial_defaults);

#ifdef __linux__
static struct rlimit nproclimit;
#endif

#ifdef SUDOERS_LOG_CLIENT
# define remote_iologs	(!SLIST_EMPTY(&def_log_servers))
#else
# define remote_iologs	0
#endif

/*
 * Unlimit the number of processes since Linux's setuid() will
 * apply resource limits when changing uid and return EAGAIN if
 * nproc would be exceeded by the uid switch.
 */
static void
unlimit_nproc(void)
{
#ifdef __linux__
    struct rlimit rl;
    debug_decl(unlimit_nproc, SUDOERS_DEBUG_UTIL);

    if (getrlimit(RLIMIT_NPROC, &nproclimit) != 0)
	    sudo_warn("getrlimit(RLIMIT_NPROC)");
    rl.rlim_cur = rl.rlim_max = RLIM_INFINITY;
    if (setrlimit(RLIMIT_NPROC, &rl) != 0) {
	rl.rlim_cur = rl.rlim_max = nproclimit.rlim_max;
	if (setrlimit(RLIMIT_NPROC, &rl) != 0)
	    sudo_warn("setrlimit(RLIMIT_NPROC)");
    }
    debug_return;
#endif /* __linux__ */
}

/*
 * Restore saved value of RLIMIT_NPROC.
 */
static void
restore_nproc(void)
{
#ifdef __linux__
    debug_decl(restore_nproc, SUDOERS_DEBUG_UTIL);

    if (setrlimit(RLIMIT_NPROC, &nproclimit) != 0)
	sudo_warn("setrlimit(RLIMIT_NPROC)");

    debug_return;
#endif /* __linux__ */
}

/*
 * Re-initialize Defaults settings.
 * We do not warn, log or send mail for errors when reinitializing,
 * this would have already been done the first time through.
 */
static bool
sudoers_reinit_defaults(struct sudoers_context *ctx)
{
    struct sudo_nss *nss, *nss_next;
    sudoers_logger_t logger = sudoers_error_hook;
    debug_decl(sudoers_reinit_defaults, SUDOERS_DEBUG_PLUGIN);

    if (!init_defaults()) {
	sudo_warnx("%s", U_("unable to initialize sudoers default values"));
	debug_return_bool(false);
    }

    /* It should not be possible for the initial defaults to fail to apply. */
    if (!update_defaults(ctx, NULL, &initial_defaults,
	    SETDEF_GENERIC|SETDEF_HOST|SETDEF_USER|SETDEF_RUNAS, false))
	debug_return_bool(false);

    /* Disable error logging while re-processing defaults. */
    sudoers_error_hook = NULL;

    TAILQ_FOREACH_SAFE(nss, snl, entries, nss_next) {
	/* Missing/invalid defaults is not a fatal error. */
	if (nss->getdefs(ctx, nss) != -1) {
	    (void)update_defaults(ctx, nss->parse_tree, NULL,
		SETDEF_GENERIC|SETDEF_HOST|SETDEF_USER|SETDEF_RUNAS, true);
	}
    }

    /* Restore error logging. */
    sudoers_error_hook = logger;

    /* No need to check the admin flag file multiple times. */
    if (ISSET(ctx->mode, MODE_POLICY_INTERCEPTED)) {
	free(def_admin_flag);
	def_admin_flag = NULL;
    }

    debug_return_bool(true);
}

int
sudoers_init(void *info, sudoers_logger_t logger, char * const envp[])
{
    struct sudo_nss *nss, *nss_next;
    int oldlocale, sources = 0;
    static int ret = -1;
    debug_decl(sudoers_init, SUDOERS_DEBUG_PLUGIN);

    /* Only initialize once. */
    if (snl != NULL)
	debug_return_int(ret);

    bindtextdomain("sudoers", LOCALEDIR);

    /* Hook up logging function for parse errors. */
    sudoers_error_hook = logger;

    /* Register fatal/fatalx callback. */
    sudo_fatal_callback_register(sudoers_cleanup);

    /* Initialize environment functions (including replacements). */
    if (!env_init(envp))
	debug_return_int(-1);

    /* Setup defaults data structures. */
    if (!init_defaults()) {
	sudo_warnx("%s", U_("unable to initialize sudoers default values"));
	debug_return_int(-1);
    }

    /* Parse info from front-end. */
    sudoers_ctx.mode = sudoers_policy_deserialize_info(&sudoers_ctx, info,
	&initial_defaults);
    if (ISSET(sudoers_ctx.mode, MODE_ERROR))
	debug_return_int(-1);

    if (!init_vars(&sudoers_ctx, envp))
	debug_return_int(-1);

    /* Parse nsswitch.conf for sudoers order. */
    snl = sudo_read_nss();

    /* LDAP or NSS may modify the euid so we need to be root for the open. */
    if (!set_perms(NULL, PERM_ROOT))
	debug_return_int(-1);

    /* Use the C locale unless another is specified in sudoers. */
    sudoers_setlocale(SUDOERS_LOCALE_SUDOERS, &oldlocale);
    sudo_warn_set_locale_func(sudoers_warn_setlocale);

    /* Update defaults set by front-end. */
    if (!update_defaults(&sudoers_ctx, NULL, &initial_defaults,
	    SETDEF_GENERIC|SETDEF_HOST|SETDEF_USER|SETDEF_RUNAS, false)) {
	goto cleanup;
    }

    /* Open and parse sudoers, set global defaults.  */
    TAILQ_FOREACH_SAFE(nss, snl, entries, nss_next) {
	if (nss->open(&sudoers_ctx, nss) == -1 || (nss->parse_tree = nss->parse(&sudoers_ctx, nss)) == NULL) {
	    TAILQ_REMOVE(snl, nss, entries);
	    continue;
	}
	sources++;

	/* Missing/invalid defaults is not a fatal error. */
	if (nss->getdefs(&sudoers_ctx, nss) == -1) {
	    log_warningx(&sudoers_ctx, SLOG_PARSE_ERROR|SLOG_NO_STDERR,
		N_("unable to get defaults from %s"), nss->source);
	} else {
	    (void)update_defaults(&sudoers_ctx, nss->parse_tree, NULL,
		SETDEF_GENERIC|SETDEF_HOST|SETDEF_USER|SETDEF_RUNAS, false);
	}
    }
    if (sources == 0) {
	/* Display an extra warning if there are multiple sudoers sources. */
	if (TAILQ_FIRST(snl) != TAILQ_LAST(snl, sudo_nss_list))
	    sudo_warnx("%s", U_("no valid sudoers sources found, quitting"));
	goto cleanup;
    }

    /* Set login class if applicable (after sudoers is parsed). */
    if (set_loginclass(&sudoers_ctx))
	ret = true;

cleanup:
    mail_parse_errors(&sudoers_ctx);

    if (!restore_perms())
	ret = -1;

    /* Restore user's locale. */
    sudo_warn_set_locale_func(NULL);
    sudoers_setlocale(oldlocale, NULL);

    debug_return_int(ret);
}

/*
 * Expand I/O log dir and file into a full path.
 * Returns the full I/O log path prefixed with "iolog_path=".
 * Sets ctx->iolog_file and ctx->iolog_path as a side effect.
 */
static char *
format_iolog_path(struct sudoers_context *ctx)
{
    char dir[PATH_MAX], file[PATH_MAX];
    char *iolog_path = NULL;
    int oldlocale;
    bool ok;
    debug_decl(format_iolog_path, SUDOERS_DEBUG_PLUGIN);

    /* Use sudoers locale for strftime() */
    sudoers_setlocale(SUDOERS_LOCALE_SUDOERS, &oldlocale);
    ok = expand_iolog_path(def_iolog_dir, dir, sizeof(dir),
	&sudoers_iolog_path_escapes[1], ctx);
    if (ok) {
	ctx->iolog_dir = dir;
	ok = expand_iolog_path(def_iolog_file, file, sizeof(file),
	    &sudoers_iolog_path_escapes[0], ctx);
	ctx->iolog_dir = NULL;
    }
    sudoers_setlocale(oldlocale, NULL);
    if (!ok)
	goto done;

    if (asprintf(&iolog_path, "iolog_path=%s/%s", dir, file) == -1) {
	iolog_path = NULL;
	goto done;
    }

    /* Stash pointer to the I/O log for the event log. */
    ctx->iolog_path = iolog_path + sizeof("iolog_path=") - 1;
    ctx->iolog_file = ctx->iolog_path + 1 + strlen(dir);

done:
    debug_return_str(iolog_path);
}

static void
cb_lookup(const struct sudoers_parse_tree *parse_tree,
    const struct userspec *us, int user_match, const struct privilege *priv,
    int host_match, const struct cmndspec *cs, int date_match, int runas_match,
    int cmnd_match, void *closure)
{
    struct sudoers_match_info *info = closure;

    if (cmnd_match != UNSPEC) {
	info->us = us;
	info->priv = priv;
	info->cs = cs;
    }
}

/*
 * Find the command, perform a sudoers lookup, ask for a password as
 * needed, and perform post-lokup checks.  Logs success/failure.
 * This is used by the check, list and validate plugin methods.
 *
 * Returns true if allowed, false if denied, -1 on error and
 * -2 for usage error.
 */
static int
sudoers_check_common(struct sudoers_context *ctx, int pwflag)
{
    struct sudoers_match_info match_info = { NULL };
    int oldlocale, ret = -1;
    unsigned int validated;
    time_t now;
    debug_decl(sudoers_check_common, SUDOERS_DEBUG_PLUGIN);

    /* If given the -P option, set the "preserve_groups" flag. */
    if (ISSET(ctx->mode, MODE_PRESERVE_GROUPS))
	def_preserve_groups = true;

    /* Find command in path and apply per-command Defaults. */
    cmnd_status = set_cmnd(ctx);
    if (cmnd_status == NOT_FOUND_ERROR)
	goto done;

    /* Is root even allowed to run sudo? */
    if (ctx->user.uid == 0 && !def_root_sudo) {
	/* Not an audit event (should it be?). */
	sudo_warnx("%s",
	    U_("sudoers specifies that root is not allowed to sudo"));
	ret = false;
	goto done;
    }

    /* Check for -C overriding def_closefrom. */
    if (ctx->user.closefrom >= 0 && ctx->user.closefrom != def_closefrom) {
	if (!def_closefrom_override) {
	    log_warningx(ctx, SLOG_NO_STDERR|SLOG_AUDIT,
		N_("user not allowed to override closefrom limit"));
	    sudo_warnx("%s", U_("you are not permitted to use the -C option"));
	    goto bad;
	}
	def_closefrom = ctx->user.closefrom;
    }

    /*
     * Check sudoers sources, using the locale specified in sudoers.
     */
    time(&now);
    sudoers_setlocale(SUDOERS_LOCALE_SUDOERS, &oldlocale);
    validated = sudoers_lookup(snl, ctx, now, cb_lookup, &match_info,
	&cmnd_status, pwflag);
    sudoers_setlocale(oldlocale, NULL);
    if (ISSET(validated, VALIDATE_ERROR)) {
	/* The lookup function should have printed an error. */
	goto done;
    }

    if (match_info.us != NULL && match_info.us->file != NULL) {
	free(ctx->source);
	if (match_info.us->line != 0) {
	    if (asprintf(&ctx->source, "%s:%d:%d", match_info.us->file,
		    match_info.us->line, match_info.us->column) == -1)
		ctx->source = NULL;
	} else {
	    ctx->source = strdup(match_info.us->file);
	}
	if (ctx->source == NULL) {
	    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	    goto done;
	}
    }

    if (ctx->runas.cmnd == NULL) {
	if ((ctx->runas.cmnd = strdup(ctx->user.cmnd)) == NULL) {
	    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	    goto done;
	}
    }

    /* Defer uid/gid checks until after defaults have been updated. */
    if (unknown_runas_uid && !def_runas_allow_unknown_id) {
	log_warningx(ctx, SLOG_AUDIT, N_("unknown user %s"),
	    ctx->runas.pw->pw_name);
	goto done;
    }
    if (ctx->runas.gr != NULL) {
	if (unknown_runas_gid && !def_runas_allow_unknown_id) {
	    log_warningx(ctx, SLOG_AUDIT, N_("unknown group %s"),
		ctx->runas.gr->gr_name);
	    goto done;
	}
    }

    /* If no command line args and "shell_noargs" is not set, error out. */
    if (ISSET(ctx->mode, MODE_IMPLIED_SHELL) && !def_shell_noargs) {
	/* Not an audit event. */
	ret = -2; /* usage error */
	goto done;
    }

    /* Bail if a tty is required and we don't have one. */
    if (def_requiretty && !sudoers_tty_present(ctx)) {
	log_warningx(ctx, SLOG_NO_STDERR|SLOG_AUDIT, N_("no tty"));
	sudo_warnx("%s", U_("sorry, you must have a tty to run sudo"));
	goto bad;
    }

    /* Check runas user's shell if running (or checking) a command. */
    if (ISSET(ctx->mode, MODE_RUN|MODE_CHECK)) {
	if (!user_shell_valid(ctx->runas.pw)) {
	    log_warningx(ctx, SLOG_RAW_MSG|SLOG_AUDIT,
		N_("invalid shell for user %s: %s"),
		ctx->runas.pw->pw_name, ctx->runas.pw->pw_shell);
	    goto bad;
	}
    }

    /*
     * We don't reset the environment for sudoedit or if the user
     * specified the -E command line flag and they have setenv privs.
     */
    if (ISSET(ctx->mode, MODE_EDIT) ||
	(ISSET(ctx->mode, MODE_PRESERVE_ENV) && def_setenv))
	def_env_reset = false;

    /* Build a new environment that avoids any nasty bits. */
    if (!rebuild_env(ctx))
	goto bad;

    /* Require a password if sudoers says so.  */
    switch (check_user(ctx, validated, ctx->mode)) {
    case AUTH_SUCCESS:
	/* user authenticated successfully. */
	break;
    case AUTH_FAILURE:
	/* Note: log_denial() calls audit for us. */
	if (!ISSET(validated, VALIDATE_SUCCESS)) {
	    /* Only display a denial message if no password was read. */
	    if (!log_denial(ctx, validated, def_passwd_tries <= 0))
		goto done;
	}
	goto bad;
    default:
	/* some other error, ret is -1. */
	goto done;
    }

    /* Check whether ctx->runas.chroot is permitted (if specified). */
    switch (check_user_runchroot(ctx->runas.chroot)) {
    case true:
	break;
    case false:
	log_warningx(ctx, SLOG_NO_STDERR|SLOG_AUDIT,
	    N_("user not allowed to change root directory to %s"),
	    ctx->runas.chroot);
	sudo_warnx(U_("you are not permitted to use the -R option with %s"),
	    ctx->user.cmnd);
	goto bad;
    default:
	goto done;
    }

    /* Check whether ctx->runas.cwd is permitted (if specified). */
    switch (check_user_runcwd(ctx->runas.cwd)) {
    case true:
	break;
    case false:
	log_warningx(ctx, SLOG_NO_STDERR|SLOG_AUDIT,
	    N_("user not allowed to change directory to %s"), ctx->runas.cwd);
	sudo_warnx(U_("you are not permitted to use the -D option with %s"),
	    ctx->user.cmnd);
	goto bad;
    default:
	goto done;
    }

    /* If run as root with SUDO_USER set, set ctx->user.pw to that user. */
    /* XXX - causes confusion when root is not listed in sudoers */
    if (ISSET(ctx->mode, MODE_RUN|MODE_EDIT) && prev_user != NULL) {
	if (ctx->user.uid == 0 && strcmp(prev_user, "root") != 0) {
	    struct passwd *pw;

	    if ((pw = sudo_getpwnam(prev_user)) != NULL) {
		    if (ctx->user.pw != NULL)
			sudo_pw_delref(ctx->user.pw);
		    ctx->user.pw = pw;
	    }
	}
    }

    /* If the user was not allowed to run the command we are done. */
    if (!ISSET(validated, VALIDATE_SUCCESS)) {
	/* Note: log_failure() calls audit for us. */
	if (!log_failure(ctx, validated, cmnd_status))
	    goto done;
	goto bad;
    }

    /*
     * Check if the user is trying to run a setid binary in intercept mode.
     * For the DSO intercept_type, we reject attempts to run setid binaries
     * by default since the dynamic loader will clear LD_PRELOAD, defeating
     * intercept.
     */
    if (def_intercept || ISSET(ctx->mode, MODE_POLICY_INTERCEPTED)) {
	if (!def_intercept_allow_setid && ctx->user.cmnd_stat != NULL) {
	    if (ISSET(ctx->user.cmnd_stat->st_mode, S_ISUID|S_ISGID)) {
		CLR(validated, VALIDATE_SUCCESS);
		if (!log_denial(ctx, validated|FLAG_INTERCEPT_SETID, true))
		    goto done;
		goto bad;
	    }
	}
    }

    /* Create Ubuntu-style dot file to indicate sudo was successful. */
    if (create_admin_success_flag(ctx) == -1)
	goto done;

    /* Finally tell the user if the command did not exist. */
    if (cmnd_status == NOT_FOUND_DOT) {
	audit_failure(ctx, ctx->runas.argv, N_("command in current directory"));
	sudo_warnx(U_("ignoring \"%s\" found in '.'\nUse \"sudo ./%s\" if this is the \"%s\" you wish to run."), ctx->user.cmnd, ctx->user.cmnd, ctx->user.cmnd);
	goto bad;
    } else if (cmnd_status == NOT_FOUND) {
	if (ISSET(ctx->mode, MODE_CHECK)) {
	    audit_failure(ctx, ctx->runas.argv, N_("%s: command not found"),
		ctx->runas.argv[1]);
	    sudo_warnx(U_("%s: command not found"), ctx->runas.argv[1]);
	} else {
	    audit_failure(ctx, ctx->runas.argv, N_("%s: command not found"),
		ctx->user.cmnd);
	    sudo_warnx(U_("%s: command not found"), ctx->user.cmnd);
	    if (strncmp(ctx->user.cmnd, "cd", 2) == 0 && (ctx->user.cmnd[2] == '\0' ||
		    isblank((unsigned char)ctx->user.cmnd[2]))) {
		sudo_warnx("%s",
		    U_("\"cd\" is a shell built-in command, it cannot be run directly."));
		sudo_warnx("%s",
		    U_("the -s option may be used to run a privileged shell."));
		sudo_warnx("%s",
		    U_("the -D option may be used to run a command in a specific directory."));
	    }
	}
	goto bad;
    }

    /* If user specified a timeout make sure sudoers allows it. */
    if (!def_user_command_timeouts && ctx->user.timeout > 0) {
	log_warningx(ctx, SLOG_NO_STDERR|SLOG_AUDIT,
	    N_("user not allowed to set a command timeout"));
	sudo_warnx("%s",
	    U_("sorry, you are not allowed set a command timeout"));
	goto bad;
    }

    /* If user specified env vars make sure sudoers allows it. */
    if (ISSET(ctx->mode, MODE_RUN) && !def_setenv) {
	if (ISSET(ctx->mode, MODE_PRESERVE_ENV)) {
	    log_warningx(ctx, SLOG_NO_STDERR|SLOG_AUDIT,
		N_("user not allowed to preserve the environment"));
	    sudo_warnx("%s",
		U_("sorry, you are not allowed to preserve the environment"));
	    goto bad;
	} else {
	    if (!validate_env_vars(ctx, ctx->user.env_add))
		goto bad;
	}
    }

    ret = true;
    goto done;

bad:
    ret = false;
done:
    debug_return_int(ret);
}

static bool need_reinit;

/*
 * Check whether the user is allowed to run the specified command.
 * Returns true if allowed, false if denied, -1 on error and
 * -2 for usage error.
 */
int
sudoers_check_cmnd(int argc, char * const argv[], char *env_add[],
    void *closure)
{
    char *iolog_path = NULL;
    mode_t cmnd_umask = ACCESSPERMS;
    int ret = -1;
    debug_decl(sudoers_check_cmnd, SUDOERS_DEBUG_PLUGIN);

    sudo_warn_set_locale_func(sudoers_warn_setlocale);

    if (argc == 0) {
	sudo_warnx("%s", U_("no command specified"));
	debug_return_int(-1);
    }

    if (need_reinit) {
	/* Was previous command intercepted? */
	if (ISSET(sudoers_ctx.mode, MODE_RUN) && def_intercept)
	    SET(sudoers_ctx.mode, MODE_POLICY_INTERCEPTED);

	/* Only certain mode flags are legal for intercepted commands. */
	if (ISSET(sudoers_ctx.mode, MODE_POLICY_INTERCEPTED))
	    sudoers_ctx.mode &= MODE_INTERCEPT_MASK;

	/* Re-initialize defaults if we are called multiple times. */
	if (!sudoers_reinit_defaults(&sudoers_ctx))
	    debug_return_int(-1);
    }
    need_reinit = true;

    unlimit_nproc();

    if (!set_perms(&sudoers_ctx, PERM_INITIAL))
	goto bad;

    /* Environment variables specified on the command line. */
    if (env_add != NULL && env_add[0] != NULL)
	sudoers_ctx.user.env_add = env_add;

    /*
     * Make a local copy of argc/argv, with special handling for the
     * '-i' option.  We also allocate an extra slot for bash's --login.
     */
    if (sudoers_ctx.runas.argv != NULL && sudoers_ctx.runas.argv != sudoers_ctx.runas.argv_saved) {
	sudoers_gc_remove(GC_PTR, sudoers_ctx.runas.argv);
	free(sudoers_ctx.runas.argv);
    }
    sudoers_ctx.runas.argv = reallocarray(NULL, (size_t)argc + 2, sizeof(char *));
    if (sudoers_ctx.runas.argv == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	goto error;
    }
    sudoers_gc_add(GC_PTR, sudoers_ctx.runas.argv);
    memcpy(sudoers_ctx.runas.argv, argv, (size_t)argc * sizeof(char *));
    sudoers_ctx.runas.argc = argc;
    sudoers_ctx.runas.argv[sudoers_ctx.runas.argc] = NULL;
    if (ISSET(sudoers_ctx.mode, MODE_LOGIN_SHELL) && sudoers_ctx.runas.pw != NULL) {
	sudoers_ctx.runas.argv[0] = strdup(sudoers_ctx.runas.pw->pw_shell);
	if (sudoers_ctx.runas.argv[0] == NULL) {
	    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	    goto error;
	}
	sudoers_gc_add(GC_PTR, sudoers_ctx.runas.argv[0]);
    }

    ret = sudoers_check_common(&sudoers_ctx, 0);
    if (ret != true)
	goto done;

    if (!remote_iologs) {
	if (iolog_enabled && def_iolog_file && def_iolog_dir) {
	    if ((iolog_path = format_iolog_path(&sudoers_ctx)) == NULL) {
		if (!def_ignore_iolog_errors)
		    goto error;
		/* Unable to expand I/O log path, disable I/O logging. */
		def_log_input = false;
		def_log_output = false;
		def_log_stdin = false;
		def_log_stdout = false;
		def_log_stderr = false;
		def_log_ttyin = false;
		def_log_ttyout = false;
	    }
	}
    }

    /*
     * Set umask based on sudoers.
     * If user's umask is more restrictive, OR in those bits too
     * unless umask_override is set.
     */
    if (def_umask != ACCESSPERMS) {
	cmnd_umask = def_umask;
	if (!def_umask_override)
	    cmnd_umask |= sudoers_ctx.user.umask;
    }

    if (ISSET(sudoers_ctx.mode, MODE_LOGIN_SHELL)) {
	char *p;

	/* Convert /bin/sh -> -sh so shell knows it is a login shell */
	if ((p = strrchr(sudoers_ctx.runas.argv[0], '/')) == NULL)
	    p = sudoers_ctx.runas.argv[0];
	*p = '-';
	sudoers_ctx.runas.argv[0] = p;

	/*
	 * Newer versions of bash require the --login option to be used
	 * in conjunction with the -c option even if the shell name starts
	 * with a '-'.  Unfortunately, bash 1.x uses -login, not --login
	 * so this will cause an error for that.
	 */
	if (sudoers_ctx.runas.argc > 1 && strcmp(sudoers_ctx.runas.argv[0], "-bash") == 0 &&
	    strcmp(sudoers_ctx.runas.argv[1], "-c") == 0) {
	    /* We allocated extra space for the --login above. */
	    memmove(&sudoers_ctx.runas.argv[2], &sudoers_ctx.runas.argv[1],
		(size_t)sudoers_ctx.runas.argc * sizeof(char *));
	    sudoers_ctx.runas.argv[1] = (char *)"--login";
	    sudoers_ctx.runas.argc++;
	}

#ifdef _PATH_ENVIRONMENT
	/* Insert system-wide environment variables. */
	if (!read_env_file(&sudoers_ctx, _PATH_ENVIRONMENT, true, false))
	    sudo_warn("%s", _PATH_ENVIRONMENT);
#endif
#ifdef HAVE_LOGIN_CAP_H
	/* Set environment based on login class. */
	if (sudoers_ctx.runas.class) {
	    login_cap_t *lc = login_getclass(sudoers_ctx.runas.class);
	    if (lc != NULL) {
		setusercontext(lc, sudoers_ctx.runas.pw,
		    sudoers_ctx.runas.pw->pw_uid, LOGIN_SETPATH|LOGIN_SETENV);
		login_close(lc);
	    }
	}
#endif /* HAVE_LOGIN_CAP_H */
    }

    /* Insert system-wide environment variables. */
    if (def_restricted_env_file) {
	if (!read_env_file(&sudoers_ctx, def_restricted_env_file, false, true))
	    sudo_warn("%s", def_restricted_env_file);
    }
    if (def_env_file) {
	if (!read_env_file(&sudoers_ctx, def_env_file, false, false))
	    sudo_warn("%s", def_env_file);
    }

    /* Insert user-specified environment variables. */
    if (!insert_env_vars(sudoers_ctx.user.env_add)) {
	sudo_warnx("%s",
	    U_("error setting user-specified environment variables"));
	goto error;
    }

    /* Note: must call audit before uid change. */
    if (ISSET(sudoers_ctx.mode, MODE_EDIT)) {
	const char *env_editor = NULL;
	char **edit_argv;
	int edit_argc;

	sudoers_ctx.sudoedit_nfiles = sudoers_ctx.runas.argc - 1;
	free(sudoers_ctx.runas.cmnd);
	sudoers_ctx.runas.cmnd = find_editor(sudoers_ctx.sudoedit_nfiles,
	    sudoers_ctx.runas.argv + 1, &edit_argc, &edit_argv, NULL, &env_editor);
	if (sudoers_ctx.runas.cmnd == NULL) {
	    switch (errno) {
	    case ENOENT:
		audit_failure(&sudoers_ctx, sudoers_ctx.runas.argv,
		    N_("%s: command not found"),
		    env_editor ? env_editor : def_editor);
		sudo_warnx(U_("%s: command not found"),
		    env_editor ? env_editor : def_editor);
		goto error;
	    case EINVAL:
		if (def_env_editor && env_editor != NULL) {
		    /* User tried to do something funny with the editor. */
		    log_warningx(&sudoers_ctx,
			SLOG_NO_STDERR|SLOG_AUDIT|SLOG_SEND_MAIL,
			"invalid user-specified editor: %s", env_editor);
		    goto error;
		}
		FALLTHROUGH;
	    default:
		goto error;
	    }
	}
	/* find_editor() already g/c'd edit_argv[] */
	if (sudoers_ctx.runas.argv != sudoers_ctx.runas.argv_saved) {
	    sudoers_gc_remove(GC_PTR, sudoers_ctx.runas.argv);
	    free(sudoers_ctx.runas.argv);
	}
	sudoers_ctx.runas.argv = edit_argv;
	sudoers_ctx.runas.argc = edit_argc;

	/* We want to run the editor with the unmodified environment. */
	env_swap_old();
    }

    /* Save the initial command and argv so we have it for exit logging. */
    if (sudoers_ctx.runas.cmnd_saved == NULL) {
	sudoers_ctx.runas.cmnd_saved = strdup(sudoers_ctx.runas.cmnd);
	if (sudoers_ctx.runas.cmnd_saved == NULL) {
	    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	    goto error;
	}
	sudoers_ctx.runas.argv_saved = sudoers_ctx.runas.argv;
    }

    ret = true;
    goto done;

bad:
    ret = false;
    goto done;

error:
    ret = -1;

done:
    mail_parse_errors(&sudoers_ctx);

    if (def_group_plugin)
	group_plugin_unload();
    reset_parser();

    if (ret == -1) {
	/* Free locally-allocated strings. */
	free(iolog_path);
    } else {
	/* Store settings to pass back to front-end. */
	if (!sudoers_policy_store_result(&sudoers_ctx, ret,
	    sudoers_ctx.runas.argv, env_get(), cmnd_umask, iolog_path, closure))
	    ret = -1;
    }

    /* Zero out stashed copy of environment, it is owned by the front-end. */
    (void)env_init(NULL);

    if (!rewind_perms())
	ret = -1;

    restore_nproc();

    sudo_warn_set_locale_func(NULL);

    debug_return_int(ret);
}

/*
 * Validate the user and update their timestamp file entry.
 * Returns true if allowed, false if denied, -1 on error and
 * -2 for usage error.
 */
int
sudoers_validate_user(void)
{
    int ret = -1;
    debug_decl(sudoers_validate_user, SUDOERS_DEBUG_PLUGIN);

    sudo_warn_set_locale_func(sudoers_warn_setlocale);

    unlimit_nproc();

    if (!set_perms(&sudoers_ctx, PERM_INITIAL))
	goto done;

    sudoers_ctx.runas.argv = reallocarray(NULL, 2, sizeof(char *));
    if (sudoers_ctx.runas.argv == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	goto done;
    }
    sudoers_gc_add(GC_PTR, sudoers_ctx.runas.argv);
    sudoers_ctx.runas.argv[0] = (char *)"validate";
    sudoers_ctx.runas.argv[1] = NULL;
    sudoers_ctx.runas.argc = 2;

    ret = sudoers_check_common(&sudoers_ctx, I_VERIFYPW);

done:
    mail_parse_errors(&sudoers_ctx);

    if (def_group_plugin)
	group_plugin_unload();
    reset_parser();
    env_init(NULL);

    if (!rewind_perms())
	ret = -1;

    restore_nproc();

    sudo_warn_set_locale_func(NULL);

    debug_return_int(ret);
}

/*
 * List a user's privileges or check whether a specific command may be run.
 * Returns true if allowed, false if denied, -1 on error and
 * -2 for usage error.
 */
int
sudoers_list(int argc, char * const argv[], const char *list_user, int verbose)
{
    struct passwd *pw;
    int ret = -1;
    debug_decl(sudoers_list, SUDOERS_DEBUG_PLUGIN);

    sudo_warn_set_locale_func(sudoers_warn_setlocale);

    unlimit_nproc();

    if (!set_perms(&sudoers_ctx, PERM_INITIAL))
	goto done;

    if (list_user) {
	if (sudoers_ctx.runas.list_pw != NULL)
	    sudo_pw_delref(sudoers_ctx.runas.list_pw);
	sudoers_ctx.runas.list_pw = sudo_getpwnam(list_user);
	if (sudoers_ctx.runas.list_pw == NULL) {
	    sudo_warnx(U_("unknown user %s"), list_user);
	    goto done;
	}
    }

    sudoers_ctx.runas.argv = reallocarray(NULL, (size_t)argc + 2, sizeof(char *));
    if (sudoers_ctx.runas.argv == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	goto done;
    }
    sudoers_gc_add(GC_PTR, sudoers_ctx.runas.argv);
    sudoers_ctx.runas.argv[0] = (char *)"list";
    if (argc != 0)
	memcpy(sudoers_ctx.runas.argv + 1, argv, (size_t)argc * sizeof(char *));
    sudoers_ctx.runas.argc = argc + 1;
    sudoers_ctx.runas.argv[sudoers_ctx.runas.argc] = NULL;

    ret = sudoers_check_common(&sudoers_ctx, I_LISTPW);
    if (ret != true)
	goto done;

    pw = sudoers_ctx.runas.list_pw ? sudoers_ctx.runas.list_pw : sudoers_ctx.user.pw;
    if (ISSET(sudoers_ctx.mode, MODE_CHECK))
	ret = display_cmnd(&sudoers_ctx, snl, pw, verbose);
    else
	ret = display_privs(&sudoers_ctx, snl, pw, verbose);

done:
    mail_parse_errors(&sudoers_ctx);

    if (def_group_plugin)
	group_plugin_unload();
    reset_parser();
    env_init(NULL);

    if (!rewind_perms())
	ret = -1;

    restore_nproc();

    sudo_warn_set_locale_func(NULL);

    debug_return_int(ret);
}

/*
 * Initialize timezone and fill in ctx->user.
 */
static bool
init_vars(struct sudoers_context *ctx, char * const envp[])
{
    char * const * ep;
    bool unknown_user = false;
    debug_decl(init_vars, SUDOERS_DEBUG_PLUGIN);

    if (!sudoers_initlocale(setlocale(LC_ALL, NULL), def_sudoers_locale)) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	debug_return_bool(false);
    }

#define MATCHES(s, v)	\
    (strncmp((s), (v), sizeof(v) - 1) == 0 && (s)[sizeof(v) - 1] != '\0')

    ctx->user.envp = envp;
    for (ep = ctx->user.envp; *ep; ep++) {
	switch (**ep) {
	    case 'K':
		if (MATCHES(*ep, "KRB5CCNAME="))
		    ctx->user.ccname = *ep + sizeof("KRB5CCNAME=") - 1;
		break;
	    case 'P':
		if (MATCHES(*ep, "PATH="))
		    ctx->user.path = *ep + sizeof("PATH=") - 1;
		break;
	    case 'S':
		if (MATCHES(*ep, "SUDO_PROMPT=")) {
		    /* Don't override "sudo -p prompt" */
		    if (ctx->user.prompt == NULL)
			ctx->user.prompt = *ep + sizeof("SUDO_PROMPT=") - 1;
		    break;
		}
		if (MATCHES(*ep, "SUDO_USER="))
		    prev_user = *ep + sizeof("SUDO_USER=") - 1;
		break;
	    }
    }
#undef MATCHES

    if (ctx->user.pw == NULL) {
	/* Fake a struct passwd for the call to log_warningx(). */
	ctx->user.pw = sudo_mkpwent(ctx->user.name, ctx->user.uid,
	    ctx->user.gid, NULL, NULL);
	unknown_user = true;
    }
    if (ctx->user.gid_list == NULL)
	ctx->user.gid_list = sudo_get_gidlist(ctx->user.pw, ENTRY_TYPE_ANY);

    /* Store initialize permissions so we can restore them later. */
    if (!set_perms(ctx, PERM_INITIAL))
	debug_return_bool(false);

    /* Set parse callbacks */
    set_callbacks();

    /* It is now safe to use log_warningx() and set_perms() */
    if (unknown_user) {
	log_warningx(ctx, SLOG_SEND_MAIL, N_("unknown user %s"), ctx->user.name);
	debug_return_bool(false);
    }

    /*
     * Set runas passwd/group entries based on command line or sudoers.
     * Note that if runas_group was specified without runas_user we
     * run the command as the invoking user.
     */
    if (ctx->runas.group != NULL) {
	if (!set_runasgr(ctx, ctx->runas.group, false))
	    debug_return_bool(false);
	if (!set_runaspw(ctx, ctx->runas.user ?
		ctx->runas.user : ctx->user.name, false))
	    debug_return_bool(false);
    } else {
	if (!set_runaspw(ctx, ctx->runas.user ?
		ctx->runas.user : def_runas_default, false))
	    debug_return_bool(false);
    }

    debug_return_bool(true);
}

/*
 * Fill in ctx->user.cmnd and ctx->user.cmnd_stat variables.
 * Does not fill in ctx->user.cmnd_base.
 */
int
set_cmnd_path(struct sudoers_context *ctx, const char *runchroot)
{
    struct sudoers_pivot pivot_state = SUDOERS_PIVOT_INITIALIZER;
    const char *cmnd_in;
    char *cmnd_out = NULL;
    char *path = ctx->user.path;
    int ret;
    debug_decl(set_cmnd_path, SUDOERS_DEBUG_PLUGIN);

    cmnd_in = ISSET(ctx->mode, MODE_CHECK) ?
	ctx->runas.argv[1] : ctx->runas.argv[0];

    free(ctx->user.cmnd_list);
    ctx->user.cmnd_list = NULL;
    free(ctx->user.cmnd);
    ctx->user.cmnd = NULL;
    canon_path_free(ctx->user.cmnd_dir);
    ctx->user.cmnd_dir = NULL;
    if (def_secure_path && !user_is_exempt(ctx))
	path = def_secure_path;

    /* Pivot root. */
    if (runchroot != NULL) {
	if (!pivot_root(runchroot, &pivot_state))
	    goto error;
    }

    ret = resolve_cmnd(ctx, cmnd_in, &cmnd_out, path);
    if (ret == FOUND) {
	char *slash = strrchr(cmnd_out, '/');
	if (slash != NULL) {
	    *slash = '\0';
	    ctx->user.cmnd_dir = canon_path(cmnd_out);
	    if (ctx->user.cmnd_dir == NULL && errno == ENOMEM)
		goto error;
	    *slash = '/';
	}
    }

    if (ISSET(ctx->mode, MODE_CHECK))
	ctx->user.cmnd_list = cmnd_out;
    else
	ctx->user.cmnd = cmnd_out;

    /* Restore root. */
    if (runchroot != NULL)
	(void)unpivot_root(&pivot_state);

    debug_return_int(ret);
error:
    if (runchroot != NULL)
	(void)unpivot_root(&pivot_state);
    free(cmnd_out);
    debug_return_int(NOT_FOUND_ERROR);
}

/*
 * Fill in ctx->user.cmnd, ctx->user.cmnd_stat and cmnd_status variables.
 * Does not fill in ctx->user.cmnd_base.
 */
void
set_cmnd_status(struct sudoers_context *ctx, const char *runchroot)
{
    cmnd_status = set_cmnd_path(ctx, runchroot);
}

/*
 * Fill in ctx->user.cmnd, ctx->user.cmnd_args, ctx->user.cmnd_base and
 * ctx->user.cmnd_stat variables and apply any command-specific defaults entries.
 */
static int
set_cmnd(struct sudoers_context *ctx)
{
    struct sudo_nss *nss;
    int ret = FOUND;
    debug_decl(set_cmnd, SUDOERS_DEBUG_PLUGIN);

    /* Allocate ctx->user.cmnd_stat for find_path() and match functions. */
    free(ctx->user.cmnd_stat);
    ctx->user.cmnd_stat = calloc(1, sizeof(struct stat));
    if (ctx->user.cmnd_stat == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	debug_return_int(NOT_FOUND_ERROR);
    }

    /* Re-initialize for when we are called multiple times. */
    free(ctx->runas.cmnd);
    ctx->runas.cmnd = NULL;

    if (ISSET(ctx->mode, MODE_RUN|MODE_EDIT|MODE_CHECK)) {
	if (!ISSET(ctx->mode, MODE_EDIT)) {
	    const char *runchroot = ctx->runas.chroot;
	    if (runchroot == NULL && def_runchroot != NULL &&
		    strcmp(def_runchroot, "*") != 0)
		runchroot = def_runchroot;

	    ret = set_cmnd_path(ctx, runchroot);
	    if (ret == NOT_FOUND_ERROR) {
		if (errno == ENAMETOOLONG) {
		    audit_failure(ctx, ctx->runas.argv, N_("command too long"));
		}
		log_warning(ctx, 0, "%s", ctx->runas.argv[0]);
		debug_return_int(ret);
	    }
	}

	/* set ctx->user.cmnd_args */
	free(ctx->user.cmnd_args);
	ctx->user.cmnd_args = NULL;
	if (ISSET(ctx->mode, MODE_CHECK)) {
	    if (ctx->runas.argc > 2) {
		/* Skip the command being listed in ctx->runas.argv[1]. */
		ctx->user.cmnd_args = strvec_join(ctx->runas.argv + 2, ' ', NULL);
		if (ctx->user.cmnd_args == NULL)
		    debug_return_int(NOT_FOUND_ERROR);
	    }
	} else if (ctx->runas.argc > 1) {
	    if (ISSET(ctx->mode, MODE_SHELL|MODE_LOGIN_SHELL) &&
		    ISSET(ctx->mode, MODE_RUN)) {
		/*
		 * When running a command via a shell, the sudo front-end
		 * escapes potential meta chars.  We unescape non-spaces
		 * for sudoers matching and logging purposes.
		 * TODO: move escaping to the policy plugin instead
		 */
		ctx->user.cmnd_args = strvec_join(ctx->runas.argv + 1, ' ',
		    strlcpy_unescape);
	    } else {
		ctx->user.cmnd_args = strvec_join(ctx->runas.argv + 1, ' ',
		    NULL);
	    }
	    if (ctx->user.cmnd_args == NULL)
		debug_return_int(NOT_FOUND_ERROR);
	}
    }
    if (ctx->user.cmnd == NULL) {
	ctx->user.cmnd = strdup(ctx->runas.argv[0]);
	if (ctx->user.cmnd == NULL)
	    debug_return_int(NOT_FOUND_ERROR);
    }
    ctx->user.cmnd_base = sudo_basename(ctx->user.cmnd);

    /* Convert "sudo sudoedit" -> "sudoedit" */
    if (ISSET(ctx->mode, MODE_RUN) && strcmp(ctx->user.cmnd_base, "sudoedit") == 0) {
	char *new_cmnd;

	CLR(ctx->mode, MODE_RUN);
	SET(ctx->mode, MODE_EDIT);
	sudo_warnx("%s", U_("sudoedit doesn't need to be run via sudo"));
	if ((new_cmnd = strdup("sudoedit")) == NULL) {
	    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	    debug_return_int(NOT_FOUND_ERROR);
	}
	free(ctx->user.cmnd);
	ctx->user.cmnd_base = ctx->user.cmnd = new_cmnd;
    }

    TAILQ_FOREACH(nss, snl, entries) {
	/* Missing/invalid defaults is not a fatal error. */
	(void)update_defaults(ctx, nss->parse_tree, NULL, SETDEF_CMND, false);
    }

    debug_return_int(ret);
}

static int
open_file(const char *path, int flags)
{
    int fd;
    debug_decl(open_file, SUDOERS_DEBUG_PLUGIN);

    if (!set_perms(NULL, PERM_SUDOERS))
	debug_return_int(-1);

    fd = open(path, flags);
    if (fd == -1 && errno == EACCES && geteuid() != ROOT_UID) {
	/*
	 * If we tried to open sudoers as non-root but got EACCES,
	 * try again as root.
	 */
	int serrno = errno;
	if (restore_perms() && set_perms(NULL, PERM_ROOT))
	    fd = open(path, flags);
	errno = serrno;
    }
    if (!restore_perms()) {
	/* unable to change back to root */
	if (fd != -1) {
	    close(fd);
	    fd = -1;
	}
    }

    debug_return_int(fd);
}

/*
 * Open sudoers file and check mode/owner/type.
 * Returns a handle to the sudoers file or NULL on error.
 */
FILE *
open_sudoers(const char *path, char **outfile, bool doedit, bool *keepopen)
{
    char fname[PATH_MAX];
    FILE *fp = NULL;
    struct stat sb;
    int error, fd;
    debug_decl(open_sudoers, SUDOERS_DEBUG_PLUGIN);

    fd = sudo_open_conf_path(path, fname, sizeof(fname), open_file);
    if (sudoers_ctx.parser_conf.ignore_perms) {
	/* Skip sudoers security checks when ignore_perms is set. */
	if (fd == -1 || fstat(fd, &sb) == -1)
	    error = SUDO_PATH_MISSING;
	else
	    error = SUDO_PATH_SECURE;
    } else {
	error = sudo_secure_fd(fd, S_IFREG, sudoers_file_uid(),
	    sudoers_file_gid(), &sb);
    }
    switch (error) {
    case SUDO_PATH_SECURE:
	/*
	 * Make sure we can read the file so we can present the
	 * user with a reasonable error message (unlike the lexer).
	 */
	if ((fp = fdopen(fd, "r")) == NULL) {
	    log_warning(&sudoers_ctx, SLOG_PARSE_ERROR,
		N_("unable to open %s"), fname);
	} else {
	    fd = -1;
	    if (sb.st_size != 0 && fgetc(fp) == EOF) {
		log_warning(&sudoers_ctx, SLOG_PARSE_ERROR,
		    N_("unable to read %s"), fname);
		fclose(fp);
		fp = NULL;
	    } else {
		/* Rewind fp and set close on exec flag. */
		rewind(fp);
		(void)fcntl(fileno(fp), F_SETFD, 1);
		if (outfile != NULL) {
                    *outfile = sudo_rcstr_dup(fname);
		    if (*outfile == NULL) {
			sudo_warnx(U_("%s: %s"), __func__,
			    U_("unable to allocate memory"));
			fclose(fp);
			fp = NULL;
		    }
		}
	    }
	}
	break;
    case SUDO_PATH_MISSING:
	log_warning(&sudoers_ctx, SLOG_PARSE_ERROR,
	    N_("unable to open %s"), path);
	break;
    case SUDO_PATH_BAD_TYPE:
	log_warningx(&sudoers_ctx, SLOG_PARSE_ERROR,
	    N_("%s is not a regular file"), fname);
	break;
    case SUDO_PATH_WRONG_OWNER:
	log_warningx(&sudoers_ctx, SLOG_PARSE_ERROR,
	    N_("%s is owned by uid %u, should be %u"), fname,
	    (unsigned int)sb.st_uid, (unsigned int)sudoers_file_uid());
	break;
    case SUDO_PATH_WORLD_WRITABLE:
	log_warningx(&sudoers_ctx, SLOG_PARSE_ERROR,
	    N_("%s is world writable"), fname);
	break;
    case SUDO_PATH_GROUP_WRITABLE:
	log_warningx(&sudoers_ctx, SLOG_PARSE_ERROR,
	    N_("%s is owned by gid %u, should be %u"), fname,
	    (unsigned int)sb.st_gid, (unsigned int)sudoers_file_gid());
	break;
    default:
	sudo_warnx("%s: internal error, unexpected error %d", __func__, error);
	break;
    }

    if (fp == NULL && fd != -1)
	close(fd);

    debug_return_ptr(fp);
}

#ifdef HAVE_LOGIN_CAP_H
static bool
set_loginclass(struct sudoers_context *ctx)
{
    const struct passwd *pw = ctx->runas.pw ? ctx->runas.pw : ctx->user.pw;
    const unsigned int errflags = SLOG_RAW_MSG;
    login_cap_t *lc;
    bool ret = true;
    debug_decl(set_loginclass, SUDOERS_DEBUG_PLUGIN);

    if (!def_use_loginclass)
	goto done;

    if (ctx->runas.class && strcmp(ctx->runas.class, "-") != 0) {
	if (ctx->user.uid != 0 && pw->pw_uid != 0) {
	    sudo_warnx(U_("only root can use \"-c %s\""), ctx->runas.class);
	    ret = false;
	    goto done;
	}
    } else {
	ctx->runas.class = pw->pw_class;
	if (!ctx->runas.class || !*ctx->runas.class)
	    ctx->runas.class = (char *)
		((pw->pw_uid == 0) ? LOGIN_DEFROOTCLASS : LOGIN_DEFCLASS);
    }

    /* Make sure specified login class is valid. */
    lc = login_getclass(ctx->runas.class);
    if (!lc || !lc->lc_class || strcmp(lc->lc_class, ctx->runas.class) != 0) {
	/*
	 * Don't make it an error if the user didn't specify the login
	 * class themselves.  We do this because if login.conf gets
	 * corrupted we want the admin to be able to use sudo to fix it.
	 */
	log_warningx(ctx, errflags, N_("unknown login class %s"),
	    ctx->runas.class);
	def_use_loginclass = false;
	if (ctx->runas.class)
	    ret = false;
    }
    login_close(lc);
done:
    debug_return_bool(ret);
}
#else
static bool
set_loginclass(struct sudoers_context *ctx)
{
    return true;
}
#endif /* HAVE_LOGIN_CAP_H */

/*
 * Get passwd entry for the user we are going to run commands as
 * and store it in ctx->runas.pw.  By default, commands run as "root".
 */
static bool
set_runaspw(struct sudoers_context *ctx, const char *user, bool quiet)
{
    struct passwd *pw = NULL;
    debug_decl(set_runaspw, SUDOERS_DEBUG_PLUGIN);

    unknown_runas_uid = false;
    if (*user == '#') {
	const char *errstr;
	uid_t uid = sudo_strtoid(user + 1, &errstr);
	if (errstr == NULL) {
	    if ((pw = sudo_getpwuid(uid)) == NULL) {
		unknown_runas_uid = true;
		pw = sudo_fakepwnam(user, ctx->user.gid);
	    }
	}
    }
    if (pw == NULL) {
	if ((pw = sudo_getpwnam(user)) == NULL) {
	    if (!quiet)
		log_warningx(ctx, SLOG_AUDIT, N_("unknown user %s"), user);
	    debug_return_bool(false);
	}
    }
    if (ctx->runas.pw != NULL)
	sudo_pw_delref(ctx->runas.pw);
    ctx->runas.pw = pw;
    debug_return_bool(true);
}

/*
 * Get group entry for the group we are going to run commands as
 * and store it in ctx->runas.gr.
 */
static bool
set_runasgr(struct sudoers_context *ctx, const char *group, bool quiet)
{
    struct group *gr = NULL;
    debug_decl(set_runasgr, SUDOERS_DEBUG_PLUGIN);

    unknown_runas_gid = false;
    if (*group == '#') {
	const char *errstr;
	gid_t gid = sudo_strtoid(group + 1, &errstr);
	if (errstr == NULL) {
	    if ((gr = sudo_getgrgid(gid)) == NULL) {
		unknown_runas_gid = true;
		gr = sudo_fakegrnam(group);
	    }
	}
    }
    if (gr == NULL) {
	if ((gr = sudo_getgrnam(group)) == NULL) {
	    if (!quiet)
		log_warningx(ctx, SLOG_AUDIT, N_("unknown group %s"), group);
	    debug_return_bool(false);
	}
    }
    if (ctx->runas.gr != NULL)
	sudo_gr_delref(ctx->runas.gr);
    ctx->runas.gr = gr;
    debug_return_bool(true);
}

/*
 * Callback for runas_default sudoers setting.
 */
bool
cb_runas_default(struct sudoers_context *ctx, const char *file, int line,
    int column, const union sudo_defs_val *sd_un, int op)
{
    debug_decl(cb_runas_default, SUDOERS_DEBUG_PLUGIN);

    /* Only reset runaspw if user didn't specify one. */
    if (ctx->runas.user == NULL && ctx->runas.group == NULL)
	debug_return_bool(set_runaspw(ctx, sd_un->str, true));
    debug_return_bool(true);
}

/*
 * Cleanup hook for sudo_fatal()/sudo_fatalx()
 * Also called at policy close time.
 */
void
sudoers_cleanup(void)
{
    struct sudo_nss *nss;
    struct defaults *def;
    debug_decl(sudoers_cleanup, SUDOERS_DEBUG_PLUGIN);

    if (snl != NULL) {
	TAILQ_FOREACH(nss, snl, entries) {
	    nss->close(&sudoers_ctx, nss);
	}
	snl = NULL;
	reset_parser();
    }
    while ((def = TAILQ_FIRST(&initial_defaults)) != NULL) {
	TAILQ_REMOVE(&initial_defaults, def, entries);
	free(def->var);
	free(def->val);
	free(def);
    }
    need_reinit = false;
    if (def_group_plugin)
	group_plugin_unload();
    sudoers_ctx_free(&sudoers_ctx);
    sudo_freepwcache();
    sudo_freegrcache();
    canon_path_free_cache();

    /* We must free the cached environment before running g/c. */
    env_free();

    /* Run garbage collector. */
    sudoers_gc_run();

    /* Clear globals */
    prev_user = NULL;

    debug_return;
}

bool
sudoers_set_mode(unsigned int flags, unsigned int mask)
{
    SET(sudoers_ctx.mode, flags);
    return ((sudoers_ctx.mode & mask) == sudoers_ctx.mode);
}

const struct sudoers_context *
sudoers_get_context(void)
{
    return &sudoers_ctx;
}
