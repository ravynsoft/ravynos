/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 1999-2005, 2007-2023
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
 *
 * Sponsored in part by the Defense Advanced Research Projects
 * Agency (DARPA) and Air Force Research Laboratory, Air Force
 * Materiel Command, USAF, under agreement number F39502-99-1-0512.
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
#include <errno.h>
#include <limits.h>
#include <syslog.h>

#include <sudoers.h>
#include <sudo_eventlog.h>
#include <sudo_iolog.h>
#include <gram.h>

static struct early_default early_defaults[] = {
    { I_IGNORE_UNKNOWN_DEFAULTS },
#ifdef FQDN
    { I_FQDN, true },
#else
    { I_FQDN },
#endif
    { I_MATCH_GROUP_BY_GID },
    { I_GROUP_PLUGIN },
    { I_RUNAS_DEFAULT },
    { I_SUDOERS_LOCALE },
    { -1 }
};

/*
 * Local prototypes.
 */
static bool store_int(const char *str, struct sudo_defs_types *def);
static bool store_list(const char *str, struct sudo_defs_types *def, int op);
static bool store_mode(const char *str, struct sudo_defs_types *def);
static int  store_str(const char *str, struct sudo_defs_types *def);
static bool store_syslogfac(const char *str, struct sudo_defs_types *def);
static bool store_syslogpri(const char *str, struct sudo_defs_types *def);
static bool store_timeout(const char *str, struct sudo_defs_types *def);
static bool store_tuple(const char *str, struct sudo_defs_types *def, int op);
static bool store_uint(const char *str, struct sudo_defs_types *def);
static bool store_timespec(const char *str, struct sudo_defs_types *def);
static bool store_rlimit(const char *str, struct sudo_defs_types *def);
static bool store_plugin(const char *str, struct sudo_defs_types *def, int op);
static bool list_op(const char *str, size_t, struct list_members *list, enum list_ops op);
static bool valid_path(const struct sudoers_context *ctx, struct sudo_defs_types *def, const char *val, const char *file, int line, int column, bool quiet);

/*
 * Table describing compile-time and run-time options.
 */
#include <def_data.c>

/*
 * Print version and configure info.
 */
void
dump_defaults(void)
{
    struct sudo_defs_types *cur;
    struct list_member *item;
    struct def_values *def;
    const char *desc;
    debug_decl(dump_defaults, SUDOERS_DEBUG_DEFAULTS);

    for (cur = sudo_defs_table; cur->name; cur++) {
	if (cur->desc) {
	    desc = _(cur->desc);
	    switch (cur->type & T_MASK) {
		case T_FLAG:
		    if (cur->sd_un.flag)
			sudo_printf(SUDO_CONV_INFO_MSG, "%s\n", desc);
		    break;
		case T_STR:
		case T_RLIMIT:
		    if (cur->sd_un.str) {
			sudo_printf(SUDO_CONV_INFO_MSG, desc, cur->sd_un.str);
			sudo_printf(SUDO_CONV_INFO_MSG, "\n");
		    }
		    break;
		case T_LOGFAC:
		    if (cur->sd_un.ival) {
			sudo_printf(SUDO_CONV_INFO_MSG, desc,
			    sudo_logfac2str(cur->sd_un.ival));
			sudo_printf(SUDO_CONV_INFO_MSG, "\n");
		    }
		    break;
		case T_LOGPRI:
		    if (cur->sd_un.ival) {
			sudo_printf(SUDO_CONV_INFO_MSG, desc,
			    sudo_logpri2str(cur->sd_un.ival));
			sudo_printf(SUDO_CONV_INFO_MSG, "\n");
		    }
		    break;
		case T_INT:
		    sudo_printf(SUDO_CONV_INFO_MSG, desc, cur->sd_un.ival);
		    sudo_printf(SUDO_CONV_INFO_MSG, "\n");
		    break;
		case T_UINT:
		    sudo_printf(SUDO_CONV_INFO_MSG, desc, cur->sd_un.uival);
		    sudo_printf(SUDO_CONV_INFO_MSG, "\n");
		    break;
		case T_TIMESPEC: {
		    /* display timespec in minutes as a double */
		    double d = (double)cur->sd_un.tspec.tv_sec +
			((double)cur->sd_un.tspec.tv_nsec / 1000000000.0);
		    sudo_printf(SUDO_CONV_INFO_MSG, desc, d / 60.0);
		    sudo_printf(SUDO_CONV_INFO_MSG, "\n");
		    break;
		}
		case T_MODE:
		    sudo_printf(SUDO_CONV_INFO_MSG, desc, cur->sd_un.mode);
		    sudo_printf(SUDO_CONV_INFO_MSG, "\n");
		    break;
		case T_LIST:
		    if (!SLIST_EMPTY(&cur->sd_un.list)) {
			sudo_printf(SUDO_CONV_INFO_MSG, "%s\n", desc);
			SLIST_FOREACH(item, &cur->sd_un.list, entries) {
			    sudo_printf(SUDO_CONV_INFO_MSG,
				"\t%s\n", item->value);
			}
		    }
		    break;
		case T_TIMEOUT:
		    if (cur->sd_un.ival) {
			sudo_printf(SUDO_CONV_INFO_MSG, desc,
			    cur->sd_un.ival);
			sudo_printf(SUDO_CONV_INFO_MSG, "\n");
		    }
		    break;
		case T_TUPLE:
		    for (def = cur->values; def->sval; def++) {
			if (cur->sd_un.tuple == def->nval) {
			    sudo_printf(SUDO_CONV_INFO_MSG, desc, def->sval);
			    break;
			}
		    }
		    sudo_printf(SUDO_CONV_INFO_MSG, "\n");
		    break;
	    }
	}
    }
    debug_return;
}

static bool
defaults_warnx(const struct sudoers_context *ctx, const char *file, int line,
    int column, bool quiet, const char * restrict fmt, ...)
{
    va_list ap;
    bool ret;
    debug_decl(defaults_warnx, SUDOERS_DEBUG_DEFAULTS);

    va_start(ap, fmt);
    ret = parser_vwarnx(ctx, file, line, column, true, quiet, fmt, ap);
    va_end(ap);

    debug_return_bool(ret);
}

/*
 * Find the index of the specified Defaults name in sudo_defs_table[]
 * On success, returns the matching index or -1 on failure.
 */
static int
find_default(const struct sudoers_context *ctx, const char *name,
    const char *file, int line, int column, bool quiet)
{
    int i;
    debug_decl(find_default, SUDOERS_DEBUG_DEFAULTS);

    for (i = 0; sudo_defs_table[i].name != NULL; i++) {
	if (strcmp(name, sudo_defs_table[i].name) == 0)
	    debug_return_int(i);
    }
    if (!def_ignore_unknown_defaults) {
	defaults_warnx(ctx, file, line, column, quiet,
	    N_("unknown defaults entry \"%s\""), name);
    }
    debug_return_int(-1);
}

/*
 * Parse a defaults entry, storing the parsed entry in sd_un.
 * Returns true on success or false on failure.
 */
static bool
parse_default_entry(const struct sudoers_context *ctx,
    struct sudo_defs_types *def, const char *val, int op,
    const char *file, int line, int column, bool quiet)
{
    int rc;
    debug_decl(parse_default_entry, SUDOERS_DEBUG_DEFAULTS);

    if (file == NULL)
	file = "front-end";

    sudo_debug_printf(SUDO_DEBUG_INFO, "%s: %s:%d:%d: %s=%s op=%d",
	__func__, file, line, column, def->name, val ? val : "", op);

    /*
     * If no value specified, the boolean flag must be set for non-flags.
     * Only flags and tuples support boolean "true".
     */
    if (val == NULL) {
	switch (def->type & T_MASK) {
	case T_LOGFAC:
	    if (op == true) {
		/* Use default syslog facility if none specified. */
		val = LOGFAC;
	    }
	    break;
	case T_FLAG:
	    break;
	case T_TUPLE:
	    if (ISSET(def->type, T_BOOL))
		break;
	    FALLTHROUGH;
	default:
	    if (!ISSET(def->type, T_BOOL) || op != false) {
		defaults_warnx(ctx, file, line, column, quiet,
		    N_("no value specified for \"%s\""), def->name);
		debug_return_bool(false);
	    }
	}
    }

    /* Only lists support append/remove. */
    if ((op == '+' || op == '-') && (def->type & T_MASK) != T_LIST) {
	defaults_warnx(ctx, file, line, column, quiet,
	    N_("invalid operator \"%c=\" for \"%s\""), op, def->name);
	debug_return_bool(false);
    }

    switch (def->type & T_MASK) {
	case T_LOGFAC:
	    rc = store_syslogfac(val, def);
	    break;
	case T_LOGPRI:
	    rc = store_syslogpri(val, def);
	    break;
	case T_STR:
	    if (val != NULL && ISSET(def->type, T_PATH|T_CHPATH)) {
		if (!valid_path(ctx, def, val, file, line, column, quiet)) {
		    rc = -1;
		    break;
		}
	    }
	    rc = store_str(val, def);
	    break;
	case T_INT:
	    rc = store_int(val, def);
	    break;
	case T_UINT:
	    rc = store_uint(val, def);
	    break;
	case T_MODE:
	    rc = store_mode(val, def);
	    break;
	case T_FLAG:
	    if (val != NULL) {
		defaults_warnx(ctx, file, line, column, quiet,
		    N_("option \"%s\" does not take a value"), def->name);
		rc = -1;
		break;
	    }
	    def->sd_un.flag = (bool)op;
	    rc = true;
	    break;
	case T_LIST:
	    rc = store_list(val, def, op);
	    break;
	case T_TIMEOUT:
	    rc = store_timeout(val, def);
	    break;
	case T_TUPLE:
	    rc = store_tuple(val, def, op);
	    break;
	case T_TIMESPEC:
	    rc = store_timespec(val, def);
	    break;
	case T_PLUGIN:
	    rc = store_plugin(val, def, op);
	    break;
	case T_RLIMIT:
	    rc = store_rlimit(val, def);
	    break;
	default:
	    defaults_warnx(ctx, file, line, column, quiet,
		N_("invalid Defaults type 0x%x for option \"%s\""),
		def->type, def->name);
	    rc = -1;
	    break;
    }
    if (rc == false) {
	defaults_warnx(ctx, file, line, column, quiet,
	    N_("value \"%s\" is invalid for option \"%s\""), val, def->name);
    }

    debug_return_bool(rc == true);
}

static struct early_default *
is_early_default(const char *name)
{
    struct early_default *early;
    debug_decl(is_early_default, SUDOERS_DEBUG_DEFAULTS);

    for (early = early_defaults; early->idx != -1; early++) {
	if (strcmp(name, sudo_defs_table[early->idx].name) == 0)
	    debug_return_ptr(early);
    }
    debug_return_ptr(NULL);
}

static bool
run_callback(struct sudoers_context *ctx, const char *file, int line,
    int column, struct sudo_defs_types *def, int op)
{
    debug_decl(run_callback, SUDOERS_DEBUG_DEFAULTS);

    if (def->callback == NULL)
	debug_return_bool(true);
    debug_return_bool(def->callback(ctx, file, line, column, &def->sd_un, op));
}

/*
 * Sets/clears an entry in the defaults structure.
 * Runs the callback if present on success.
 */
bool
set_default(struct sudoers_context *ctx, const char *var, const char *val,
    int op, const char *file, int line, int column, bool quiet)
{
    int idx;
    debug_decl(set_default, SUDOERS_DEBUG_DEFAULTS);

    sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
	"%s: setting Defaults %s -> %s", __func__, var, val ? val : "false");

    idx = find_default(ctx, var, file, line, column, quiet);
    if (idx != -1) {
	/* Set parsed value in sudo_defs_table and run callback (if any). */
	struct sudo_defs_types *def = &sudo_defs_table[idx];
	if (parse_default_entry(ctx, def, val, op, file, line, column, quiet))
	    debug_return_bool(run_callback(ctx, file, line, column, def, op));
    }
    debug_return_bool(false);
}

/*
 * Like set_default() but stores the matching default value
 * and does not run callbacks.
 */
static bool
set_early_default(const struct sudoers_context *ctx, const char *var,
    const char *val, int op, const char *file, int line, int column,
    bool quiet, struct early_default *early)
{
    int idx;
    debug_decl(set_early_default, SUDOERS_DEBUG_DEFAULTS);

    idx = find_default(ctx, var, file, line, column, quiet);
    if (idx != -1) {
	/* Set parsed value in sudo_defs_table but defer callback (if any). */
	struct sudo_defs_types *def = &sudo_defs_table[idx];
	if (parse_default_entry(ctx, def, val, op, file, line, column, quiet)) {
	    if (early->file != NULL)
		sudo_rcstr_delref(early->file);
	    early->file = sudo_rcstr_addref(file);
	    early->line = line;
	    early->column = column;
	    early->run_callback = true;
	    debug_return_bool(true);
	}
    }
    debug_return_bool(false);
}

/*
 * Run callbacks for early defaults.
 */
static bool
run_early_defaults(struct sudoers_context *ctx)
{
    struct early_default *early;
    bool ret = true;
    debug_decl(run_early_defaults, SUDOERS_DEBUG_DEFAULTS);

    for (early = early_defaults; early->idx != -1; early++) {
	if (early->run_callback) {
	    if (!run_callback(ctx, early->file, early->line, early->column,
		    &sudo_defs_table[early->idx], true))
		ret = false;
	    early->run_callback = false;
	}
    }
    debug_return_bool(ret);
}

static void
free_defs_val(int type, union sudo_defs_val *sd_un)
{
    switch (type & T_MASK) {
	case T_STR:
	case T_RLIMIT:
	    free(sd_un->str);
	    break;
	case T_LIST:
	    (void)list_op(NULL, 0, &sd_un->list, freeall);
	    break;
    }
    memset(sd_un, 0, sizeof(*sd_un));
}

static bool
init_passprompt_regex(void)
{
    struct list_member *lm;
    debug_decl(init_passprompt_regex, SUDOERS_DEBUG_DEFAULTS);

    /* Add initial defaults setting. */
    lm = calloc(1, sizeof(struct list_member));
    if (lm == NULL || (lm->value = strdup(PASSPROMPT_REGEX)) == NULL) {
	free(lm);
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	debug_return_bool(false);
    }
    SLIST_INSERT_HEAD(&def_passprompt_regex, lm, entries);

    debug_return_bool(true);
}

/*
 * Set default options to compiled-in values.
 * Any of these may be overridden at runtime by a "Defaults" file.
 */
bool
init_defaults(void)
{
    static bool firsttime = true;
    struct sudo_defs_types *def;
    debug_decl(init_defaults, SUDOERS_DEBUG_DEFAULTS);

    /* Clear any old settings. */
    if (!firsttime) {
	for (def = sudo_defs_table; def->name != NULL; def++)
	    free_defs_val(def->type, &def->sd_un);
    }

    /* First initialize the flags. */
#ifdef LONG_OTP_PROMPT
    def_long_otp_prompt = true;
#endif
#ifdef IGNORE_DOT_PATH
    def_ignore_dot = true;
#endif
#ifdef ALWAYS_SEND_MAIL
    def_mail_always = true;
#endif
#ifdef SEND_MAIL_WHEN_NO_USER
    def_mail_no_user = true;
#endif
#ifdef SEND_MAIL_WHEN_NO_HOST
    def_mail_no_host = true;
#endif
#ifdef SEND_MAIL_WHEN_NOT_OK
    def_mail_no_perms = true;
#endif
#ifndef NO_LECTURE
    def_lecture = once;
#endif
#ifndef NO_AUTHENTICATION
    def_authenticate = true;
#endif
#ifndef NO_ROOT_SUDO
    def_root_sudo = true;
#endif
#ifdef HOST_IN_LOG
    def_log_host = true;
#endif
#ifdef SHELL_IF_NO_ARGS
    def_shell_noargs = true;
#endif
#ifdef SHELL_SETS_HOME
    def_set_home = true;
#endif
#ifndef DONT_LEAK_PATH_INFO
    def_path_info = true;
#endif
#ifdef USE_INSULTS
    def_insults = true;
#endif
#ifdef FQDN
    def_fqdn = true;
#endif
#ifdef ENV_EDITOR
    def_env_editor = true;
#endif
#ifdef UMASK_OVERRIDE
    def_umask_override = true;
#endif
#ifdef SUDOERS_NAME_MATCH
    def_fast_glob = true;
    def_fdexec = never;
#else
    def_fdexec = digest_only;
#endif
    def_timestamp_type = TIMESTAMP_TYPE;
    if ((def_iolog_file = strdup(IOLOG_FILE)) == NULL)
	goto oom;
    if ((def_iolog_dir = strdup(_PATH_SUDO_IO_LOGDIR)) == NULL)
	goto oom;
    if ((def_sudoers_locale = strdup("C")) == NULL)
	goto oom;
    def_env_reset = ENV_RESET;
    def_set_logname = true;
    def_closefrom = STDERR_FILENO + 1;
    def_pam_ruser = true;
#ifdef __sun__
    def_pam_rhost = true;
#endif
    if ((def_pam_service = strdup("sudo")) == NULL)
	goto oom;
#ifdef HAVE_PAM_LOGIN
    if ((def_pam_login_service = strdup("sudo-i")) == NULL)
	goto oom;
#else
    if ((def_pam_login_service = strdup("sudo")) == NULL)
	goto oom;
#endif
#ifdef NO_PAM_SESSION
    def_pam_session = false;
#else
    def_pam_session = true;
#endif
#ifdef HAVE_SELINUX
    def_selinux = true;
#endif
#ifdef _PATH_SUDO_ADMIN_FLAG
    if ((def_admin_flag = strdup(_PATH_SUDO_ADMIN_FLAG)) == NULL)
	goto oom;
#endif
    if ((def_rlimit_core = strdup("0,0")) == NULL)
	goto oom;
    def_intercept_type = dso;
    def_intercept_verify = true;
    def_use_netgroups = true;
    def_netgroup_tuple = false;
    def_sudoedit_checkdir = true;
    def_iolog_mode = S_IRUSR|S_IWUSR;
    def_log_allowed = true;
    def_log_denied = true;
    def_log_format = sudo;
    def_runas_allow_unknown_id = false;
    def_noninteractive_auth = false;
    def_use_pty = true;

    /* Syslog options need special care since they both strings and ints */
#if (LOGGING & SLOG_SYSLOG)
    (void) store_syslogfac(LOGFAC, &sudo_defs_table[I_SYSLOG]);
    (void) store_syslogpri(PRI_SUCCESS, &sudo_defs_table[I_SYSLOG_GOODPRI]);
    (void) store_syslogpri(PRI_FAILURE, &sudo_defs_table[I_SYSLOG_BADPRI]);
#endif

    /* Password flags also have a string and integer component. */
    (void) store_tuple("any", &sudo_defs_table[I_LISTPW], 0);
    (void) store_tuple("all", &sudo_defs_table[I_VERIFYPW], 0);

    /* Then initialize the int-like things. */
#ifdef SUDO_UMASK
    def_umask = SUDO_UMASK;
#else
    def_umask = ACCESSPERMS;
#endif
    def_loglinelen = MAXLOGFILELEN;
    def_timestamp_timeout.tv_sec = TIMEOUT * 60;
    def_passwd_timeout.tv_sec = PASSWORD_TIMEOUT * 60;
    def_passwd_tries = TRIES_FOR_PASSWORD;
#ifdef HAVE_ZLIB_H
    def_compress_io = true;
#endif
    def_ignore_audit_errors = true;
    def_ignore_iolog_errors = false;
    def_ignore_logfile_errors = true;
    def_log_passwords = true;
#ifdef SUDOERS_LOG_CLIENT
    def_log_server_timeout = 30;
    def_log_server_verify = true;
    def_log_server_keepalive = true;
#endif

    /* Now do the strings */
    if ((def_mailto = strdup(MAILTO)) == NULL)
	goto oom;
    if ((def_mailsub = strdup(N_(MAILSUBJECT))) == NULL)
	goto oom;
    if ((def_badpass_message = strdup(_(INCORRECT_PASSWORD))) == NULL)
	goto oom;
#ifdef _PATH_SUDO_LECTURE_DIR
    if ((def_lecture_status_dir = strdup(_PATH_SUDO_LECTURE_DIR)) == NULL)
	goto oom;
#endif
#ifdef _PATH_SUDO_TIMEDIR
    if ((def_timestampdir = strdup(_PATH_SUDO_TIMEDIR)) == NULL)
	goto oom;
#endif
    if ((def_passprompt = strdup(_(PASSPROMPT))) == NULL)
	goto oom;
    if ((def_runas_default = strdup(RUNAS_DEFAULT)) == NULL)
	goto oom;
#ifdef _PATH_SUDO_SENDMAIL
    if ((def_mailerpath = strdup(_PATH_SUDO_SENDMAIL)) == NULL)
	goto oom;
#endif
    if ((def_mailerflags = strdup("-t")) == NULL)
	goto oom;
#if (LOGGING & SLOG_FILE)
    if ((def_logfile = strdup(_PATH_SUDO_LOGFILE)) == NULL)
	goto oom;
#endif
#ifdef EXEMPTGROUP
    if ((def_exempt_group = strdup(EXEMPTGROUP)) == NULL)
	goto oom;
#endif
#ifdef SECURE_PATH
    if ((def_secure_path = strdup(SECURE_PATH)) == NULL)
	goto oom;
#endif
    if ((def_editor = strdup(EDITOR)) == NULL)
	goto oom;
    def_set_utmp = true;
    def_pam_acct_mgmt = true;
    def_pam_setcred = true;
    def_syslog_maxlen = MAXSYSLOGLEN;
    def_case_insensitive_user = true;
    def_case_insensitive_group = true;

    /* Reset the locale. */
    if (!firsttime) {
	if (!sudoers_initlocale(NULL, def_sudoers_locale))
	    goto oom;
    }

    /* Finally do the lists (currently just environment tables). */
    if (!init_envtables())
	goto oom;

    /* Init eventlog config. */
    init_eventlog_config();

    /* Initial iolog password prompt regex. */
    if (!init_passprompt_regex())
	debug_return_bool(false);

    firsttime = false;

    debug_return_bool(true);
oom:
    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
    debug_return_bool(false);
}

/*
 * Check whether a defaults entry matches the specified type.
 * Returns true if it matches, else false.
 */
static bool
default_type_matches(struct defaults *d, int what)
{
    debug_decl(default_type_matches, SUDOERS_DEBUG_DEFAULTS);

    switch (d->type) {
    case DEFAULTS:
	if (ISSET(what, SETDEF_GENERIC))
	    debug_return_bool(true);
	break;
    case DEFAULTS_USER:
	if (ISSET(what, SETDEF_USER))
	    debug_return_bool(true);
	break;
    case DEFAULTS_RUNAS:
	if (ISSET(what, SETDEF_RUNAS))
	    debug_return_bool(true);
	break;
    case DEFAULTS_HOST:
	if (ISSET(what, SETDEF_HOST))
	    debug_return_bool(true);
	break;
    case DEFAULTS_CMND:
	if (ISSET(what, SETDEF_CMND))
	    debug_return_bool(true);
	break;
    }
    debug_return_bool(false);
}

/*
 * Check whether a defaults entry's binding matches.
 * Returns true if it matches, else false.
 */
static bool
default_binding_matches(const struct sudoers_context *ctx,
    struct sudoers_parse_tree *parse_tree, struct defaults *d, int what)
{
    debug_decl(default_binding_matches, SUDOERS_DEBUG_DEFAULTS);

    switch (d->type) {
    case DEFAULTS:
	debug_return_bool(true);
    case DEFAULTS_USER:
	if (userlist_matches(parse_tree, ctx->user.pw, &d->binding->members) == ALLOW)
	    debug_return_bool(true);
	break;
    case DEFAULTS_RUNAS:
	if (runaslist_matches(parse_tree, &d->binding->members, NULL, NULL, NULL) == ALLOW)
	    debug_return_bool(true);
	break;
    case DEFAULTS_HOST:
	if (hostlist_matches(parse_tree, ctx->user.pw, &d->binding->members) == ALLOW)
	    debug_return_bool(true);
	break;
    case DEFAULTS_CMND:
	if (cmndlist_matches(parse_tree, &d->binding->members, NULL, NULL) == ALLOW)
	    debug_return_bool(true);
	break;
    }
    debug_return_bool(false);
}

/*
 * Update the global defaults based on the given defaults list.
 * Pass in an OR'd list of which default types to update.
 */
bool
update_defaults(struct sudoers_context *ctx,
    struct sudoers_parse_tree *parse_tree,
    struct defaults_list *defs, int what, bool quiet)
{
    struct defaults *d;
    bool global_defaults = false;
    bool ret = true;
    debug_decl(update_defaults, SUDOERS_DEBUG_DEFAULTS);

    sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
	"what: 0x%02x", what);

    /* If no defaults list specified, use the global one in the parse tree. */
    if (defs == NULL) {
	defs = &parse_tree->defaults;
	global_defaults = true;
    }

    /*
     * If using the global defaults list, apply Defaults values marked as early.
     */
    if (global_defaults) {
	TAILQ_FOREACH(d, defs, entries) {
	    struct early_default *early = is_early_default(d->var);
	    if (early == NULL)
		continue;

	    /* Defaults type and binding must match. */
	    if (!default_type_matches(d, what) ||
		!default_binding_matches(ctx, parse_tree, d, what))
		continue;

	    /* Copy the value to sudo_defs_table and mark as early. */
	    if (!set_early_default(ctx, d->var, d->val, d->op, d->file, d->line,
		d->column, quiet, early))
		ret = false;
	}

	/* Run callbacks for early defaults (if any) */
	if (!run_early_defaults(ctx))
	    ret = false;
    }

    /*
     * Set the rest of the defaults and run their callbacks, if any.
     */
    TAILQ_FOREACH(d, defs, entries) {
	if (global_defaults) {
	    /* Skip Defaults marked as early, we already did them. */
	    if (is_early_default(d->var))
		continue;
	}

	/* Defaults type and binding must match. */
	if (!default_type_matches(d, what) ||
	    !default_binding_matches(ctx, parse_tree, d, what))
	    continue;

	/* Copy the value to sudo_defs_table and run callback (if any) */
	if (!set_default(ctx, d->var, d->val, d->op, d->file, d->line, d->column, quiet))
	    ret = false;
    }

    debug_return_bool(ret);
}

/*
 * Check all defaults entries without actually setting them.
 */
bool
check_defaults(const struct sudoers_parse_tree *parse_tree, bool quiet)
{
    struct defaults *d;
    bool ret = true;
    int idx;
    debug_decl(check_defaults, SUDOERS_DEBUG_DEFAULTS);

    TAILQ_FOREACH(d, &parse_tree->defaults, entries) {
	idx = find_default(parse_tree->ctx, d->var, d->file, d->line,
	    d->column, quiet);
	if (idx != -1) {
	    struct sudo_defs_types def = sudo_defs_table[idx];
	    memset(&def.sd_un, 0, sizeof(def.sd_un));
	    if (parse_default_entry(parse_tree->ctx, &def, d->val, d->op,
		    d->file, d->line, d->column, quiet)) {
		free_defs_val(def.type, &def.sd_un);
		continue;
	    }
	}
	/* There was an error in the entry. */
	ret = false;
    }
    debug_return_bool(ret);
}

static bool
store_int(const char *str, struct sudo_defs_types *def)
{
    const char *errstr;
    int i;
    debug_decl(store_int, SUDOERS_DEBUG_DEFAULTS);

    if (str == NULL) {
	def->sd_un.ival = 0;
    } else {
	i = (int)sudo_strtonum(str, INT_MIN, INT_MAX, &errstr);
	if (errstr != NULL) {
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		"%s: %s", str, errstr);
	    debug_return_bool(false);
	}
	def->sd_un.ival = i;
    }
    debug_return_bool(true);
}

static bool
store_uint(const char *str, struct sudo_defs_types *def)
{
    const char *errstr;
    unsigned int u;
    debug_decl(store_uint, SUDOERS_DEBUG_DEFAULTS);

    if (str == NULL) {
	def->sd_un.uival = 0;
    } else {
	u = (unsigned int)sudo_strtonum(str, 0, UINT_MAX, &errstr);
	if (errstr != NULL) {
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		"%s: %s", str, errstr);
	    debug_return_bool(false);
	}
	def->sd_un.uival = u;
    }
    debug_return_bool(true);
}

/* Check resource limit syntax, does not save as rlim_t. */
static bool
check_rlimit(const char *str, bool soft)
{
    const size_t inflen = sizeof("infinity") - 1;
    debug_decl(check_rlimit, SUDOERS_DEBUG_DEFAULTS);

    if (isdigit((unsigned char)*str)) {
	unsigned long long ullval;
	char *ep;

	errno = 0;
#ifdef HAVE_STRTOULL
	ullval = strtoull(str, &ep, 10);
	if (str == ep || (errno == ERANGE && ullval == ULLONG_MAX))
	    debug_return_bool(false);
#else
	ullval = strtoul(str, &ep, 10);
	if (str == ep || (errno == ERANGE && ullval == ULONG_MAX))
	    debug_return_bool(false);
#endif
	if (*ep == '\0' || (soft && *ep == ','))
	    debug_return_bool(true);
	debug_return_bool(false);
    }
    if (strncmp(str, "infinity", inflen) == 0) {
	if (str[inflen] == '\0' || (soft && str[inflen] == ','))
	    debug_return_bool(true);
    }
    debug_return_bool(false);
}

static bool
store_rlimit(const char *str, struct sudo_defs_types *def)
{
    debug_decl(store_rlimit, SUDOERS_DEBUG_DEFAULTS);

    /* The special values "user" and "default" are not compound. */
    if (str != NULL && strcmp(str, "user") != 0 && strcmp(str, "default") != 0) {
	const char *hard, *soft = str;
	/*
	 * Expect a limit in the form "soft,hard" or "limit" (both soft+hard).
	 */
	hard = strchr(str, ',');
	if (hard != NULL)
	    hard++;
	else
	    hard = soft;

	if (!check_rlimit(soft, true))
	    debug_return_bool(false);
	if (!check_rlimit(hard, false))
	    debug_return_bool(false);
    }

    /* Store as string, front-end will parse it as a limit. */
    debug_return_bool(store_str(str, def));
}

static bool
store_timespec(const char *str, struct sudo_defs_types *def)
{
    struct timespec ts;
    char sign = '+';
    long i;
    debug_decl(store_timespec, SUDOERS_DEBUG_DEFAULTS);

    sudo_timespecclear(&ts);
    if (str != NULL) {
	/* Convert from minutes to seconds. */
	if (*str == '+' || *str == '-')
	    sign = *str++;
	while (*str != '\0' && *str != '.') {
		if (!isdigit((unsigned char)*str))
		    debug_return_bool(false);	/* invalid number */

		/* Verify (ts.tv_sec * 10) + (digit * 60) <= TIME_T_MAX. */
		i = (*str++ - '0') * 60L;
		if (ts.tv_sec > (TIME_T_MAX - i) / 10)
		    debug_return_bool(false);	/* overflow */
		ts.tv_sec *= 10;
		ts.tv_sec += i;
	}
	if (*str++ == '.') {
	    long long nsec = 0;

	    /* Convert optional fractional component to seconds and nanosecs. */
	    for (i = 100000000; i > 0; i /= 10) {
		if (*str == '\0')
		    break;
		if (!isdigit((unsigned char)*str))
		    debug_return_bool(false);	/* invalid number */
		nsec += i * (*str++ - '0') * 60LL;
	    }
	    while (nsec >= 1000000000) {
		if (ts.tv_sec == TIME_T_MAX)
		    debug_return_bool(false);	/* overflow */
		ts.tv_sec++;
		nsec -= 1000000000;
	    }
	    ts.tv_nsec = (long)nsec;
	}
    }
    if (sign == '-') {
	def->sd_un.tspec.tv_sec = -ts.tv_sec;
	def->sd_un.tspec.tv_nsec = -ts.tv_nsec;
    } else {
	def->sd_un.tspec.tv_sec = ts.tv_sec;
	def->sd_un.tspec.tv_nsec = ts.tv_nsec;
    }
    debug_return_bool(true);
}

static bool
store_tuple(const char *str, struct sudo_defs_types *def, int op)
{
    struct def_values *v;
    debug_decl(store_tuple, SUDOERS_DEBUG_DEFAULTS);

    /*
     * Look up tuple value by name to find enum def_tuple value.
     * A tuple must have at least two possible values.
     */
    if (str == NULL) {
	/*
	 * Boolean context: true maps to values[1], false maps to values[0].
	 */
	if (op == true) {
	    v = &def->values[1];
	    def->sd_un.ival = v->nval;
	} else if (op == false) {
	    v = &def->values[0];
	    def->sd_un.ival = v->nval;
	} else {
	    debug_return_bool(false);
	}
    } else {
	for (v = def->values; v->sval != NULL; v++) {
	    if (strcmp(v->sval, str) == 0) {
		def->sd_un.tuple = v->nval;
		break;
	    }
	}
	if (v->sval == NULL)
	    debug_return_bool(false);
    }
    debug_return_bool(true);
}

static int
store_str(const char *str, struct sudo_defs_types *def)
{
    debug_decl(store_str, SUDOERS_DEBUG_DEFAULTS);

    free(def->sd_un.str);
    if (str == NULL) {
	def->sd_un.str = NULL;
    } else {
	if ((def->sd_un.str = strdup(str)) == NULL) {
	    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	    debug_return_int(-1);
	}
    }
    debug_return_int(true);
}

static bool
store_list(const char *str, struct sudo_defs_types *def, int op)
{
    debug_decl(store_list, SUDOERS_DEBUG_DEFAULTS);

    /* Remove all old members. */
    if (op == false || op == true)
	(void)list_op(NULL, 0, &def->sd_un.list, freeall);

    /* Split str into multiple space-separated words and act on each one. */
    if (str != NULL) {
	const char *cp, *ep;
	const char *end = str + strlen(str);
	const enum list_ops lop = op == '-' ? delete : add;

	if (ISSET(def->type, T_SPACE)) {
	    if (!list_op(str, strlen(str), &def->sd_un.list, lop))
		debug_return_bool(false);
	} else {
	    for (cp = sudo_strsplit(str, end, " \t", &ep); cp != NULL;
		cp = sudo_strsplit(NULL, end, " \t", &ep)) {
		if (!list_op(cp, (size_t)(ep - cp), &def->sd_un.list, lop))
		    debug_return_bool(false);
	    }
	}
    }
    debug_return_bool(true);
}

static bool
store_plugin(const char *str, struct sudo_defs_types *def, int op)
{
    const enum list_ops lop = op == '-' ? delete : add;
    debug_decl(store_plugin, SUDOERS_DEBUG_DEFAULTS);

    /* Remove all old members. */
    if (op == false || op == true)
	(void)list_op(NULL, 0, &def->sd_un.list, freeall);

    if (str != NULL) {
	if (!list_op(str, strlen(str), &def->sd_un.list, lop))
	    debug_return_bool(false);
    }

    debug_return_bool(true);
}

static bool
store_syslogfac(const char *str, struct sudo_defs_types *def)
{
    debug_decl(store_syslogfac, SUDOERS_DEBUG_DEFAULTS);

    if (str == NULL) {
	def->sd_un.ival = false;
	debug_return_bool(true);
    }
    debug_return_bool(sudo_str2logfac(str, &def->sd_un.ival));
}

static bool
store_syslogpri(const char *str, struct sudo_defs_types *def)
{
    debug_decl(store_syslogpri, SUDOERS_DEBUG_DEFAULTS);

    if (str == NULL) {
	def->sd_un.ival = -1;
	debug_return_bool(true);
    }
    debug_return_bool(sudo_str2logpri(str, &def->sd_un.ival));
}

static bool
store_mode(const char *str, struct sudo_defs_types *def)
{
    mode_t mode;
    const char *errstr;
    debug_decl(store_mode, SUDOERS_DEBUG_DEFAULTS);

    if (str == NULL) {
	def->sd_un.mode = ACCESSPERMS;
    } else {
	mode = sudo_strtomode(str, &errstr);
	if (errstr != NULL) {
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		"%s is %s", str, errstr);
	    debug_return_bool(false);
	}
	def->sd_un.mode = mode;
    }
    debug_return_bool(true);
}

static bool
store_timeout(const char *str, struct sudo_defs_types *def)
{
    debug_decl(store_mode, SUDOERS_DEBUG_DEFAULTS);

    if (str == NULL) {
	def->sd_un.ival = 0;
    } else {
	int seconds = parse_timeout(str);
	if (seconds == -1) {
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_ERRNO|SUDO_DEBUG_LINENO,
		"%s", str);
	    debug_return_bool(false);
	}
	def->sd_un.ival = seconds;
    }
    debug_return_bool(true);
}

static bool
valid_path(const struct sudoers_context *ctx, struct sudo_defs_types *def,
    const char *val, const char *file, int line, int column, bool quiet)
{
    bool ret = true;
    debug_decl(valid_path, SUDOERS_DEBUG_DEFAULTS);

    if (strlen(val) >= PATH_MAX) {
	defaults_warnx(ctx, file, line, column, quiet,
	    N_("path name for \"%s\" too long"), def->name);
	ret = false;
    }
    if (ISSET(def->type, T_CHPATH)) {
	if (val[0] != '/' && val[0] != '~' && (val[0] != '*' || val[1] != '\0')) {
	    defaults_warnx(ctx, file, line, column, quiet,
		N_("values for \"%s\" must start with a '/', '~', or '*'"),
		def->name);
	    ret = false;
	}
    } else {
	if (val[0] != '/') {
	    defaults_warnx(ctx, file, line, column, quiet,
		N_("values for \"%s\" must start with a '/'"), def->name);
	    ret = false;
	}

    }
    debug_return_bool(ret);
}

static bool
list_op(const char *str, size_t len, struct list_members *list,
    enum list_ops op)
{
    struct list_member *cur, *prev = NULL;
    debug_decl(list_op, SUDOERS_DEBUG_DEFAULTS);

    if (op == freeall) {
	while ((cur = SLIST_FIRST(list)) != NULL) {
	    SLIST_REMOVE_HEAD(list, entries);
	    free(cur->value);
	    free(cur);
	}
	debug_return_bool(true);
    }

    SLIST_FOREACH(cur, list, entries) {
	if ((strncmp(cur->value, str, len) == 0 && cur->value[len] == '\0')) {

	    if (op == add)
		debug_return_bool(true); /* already exists */

	    /* Delete node */
	    if (prev == NULL)
		SLIST_REMOVE_HEAD(list, entries);
	    else
		SLIST_REMOVE_AFTER(prev, entries);
	    free(cur->value);
	    free(cur);
	    break;
	}
	prev = cur;
    }

    /* Add new node to the head of the list. */
    if (op == add) {
	cur = calloc(1, sizeof(struct list_member));
	if (cur == NULL || (cur->value = strndup(str, len)) == NULL) {
	    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	    free(cur);
	    debug_return_bool(false);
	}
	SLIST_INSERT_HEAD(list, cur, entries);
    }
    debug_return_bool(true);
}

bool
append_default(const char *var, const char *val, int op,
    char *source, struct defaults_list *defs)
{
    struct defaults *def;
    debug_decl(append_default, SUDOERS_DEBUG_DEFAULTS);

    if ((def = calloc(1, sizeof(*def))) == NULL)
	goto oom;

    def->type = DEFAULTS;
    def->op = op;
    if ((def->var = strdup(var)) == NULL) {
	goto oom;
    }
    if (val != NULL) {
	if ((def->val = strdup(val)) == NULL)
	    goto oom;
    }
    def->file = source;
    sudo_rcstr_addref(source);
    TAILQ_INSERT_TAIL(defs, def, entries);
    debug_return_bool(true);

oom:
    if (def != NULL) {
	free(def->var);
	free(def->val);
	free(def);
    }
    debug_return_bool(false);
}

bool
cb_passprompt_regex(struct sudoers_context *ctx, const char *file,
    int line, int column, const union sudo_defs_val *sd_un, int op)
{
    struct list_member *lm;
    const char *errstr;
    debug_decl(cb_passprompt_regex, SUDOERS_DEBUG_DEFAULTS);

    /* If adding one or more regexps, make sure they are valid. */
    if (op == '+' || op == true) {
	SLIST_FOREACH(lm, &sd_un->list, entries) {
	    if (!sudo_regex_compile(NULL, lm->value, &errstr)) {
		defaults_warnx(ctx, file, line, column, false,
		    U_("invalid regular expression \"%s\": %s"),
		    lm->value, U_(errstr));
		debug_return_bool(false);
	    }
	}
    }

    debug_return_bool(true);
}
