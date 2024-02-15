/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2011-2023 Todd C. Miller <Todd.Miller@sudo.ws>
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

#ifdef __TANDEM
# include <floss.h>
#endif

#include <config.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pwd.h>
#include <grp.h>
#include <netdb.h>
#ifndef HAVE_GETADDRINFO
# include <compat/getaddrinfo.h>
#endif

#include <sudoers.h>
#include <timestamp.h>
#include <sudo_iolog.h>

#ifndef AI_FQDN
# define AI_FQDN AI_CANONNAME
#endif

static bool override_umask;

/*
 * Look up the fully qualified domain name of host.
 * Use AI_FQDN if available since "canonical" is not always the same as fqdn.
 * Returns 0 on success, setting longp and shortp.
 * Returns non-zero on failure, longp and shortp are unchanged.
 * See gai_strerror() for the list of error return codes.
 */
static int
resolve_host(const char *host, char **longp, char **shortp)
{
    struct addrinfo *res0, hint;
    char *cp, *lname, *sname;
    int ret;
    debug_decl(resolve_host, SUDOERS_DEBUG_PLUGIN);

    memset(&hint, 0, sizeof(hint));
    hint.ai_family = PF_UNSPEC;
    hint.ai_flags = AI_FQDN;

    if ((ret = getaddrinfo(host, NULL, &hint, &res0)) != 0)
	debug_return_int(ret);
    if ((lname = strdup(res0->ai_canonname)) == NULL) {
	freeaddrinfo(res0);
	debug_return_int(EAI_MEMORY);
    }
    if ((cp = strchr(lname, '.')) != NULL) {
	sname = strndup(lname, (size_t)(cp - lname));
	if (sname == NULL) {
	    free(lname);
	    freeaddrinfo(res0);
	    debug_return_int(EAI_MEMORY);
	}
    } else {
	sname = lname;
    }
    freeaddrinfo(res0);
    *longp = lname;
    *shortp = sname;

    debug_return_int(0);
}

/*
 * Look up the fully qualified domain name of user and runas hosts.
 * Sets ctx->user.host, ctx->user.shost, ctx->runas.host and ctx->runas.shost.
 */
static bool
cb_fqdn(struct sudoers_context *ctx, const char *file,
    int line, int column, const union sudo_defs_val *sd_un, int op)
{
    bool remote;
    int rc;
    char *lhost, *shost;
    debug_decl(cb_fqdn, SUDOERS_DEBUG_PLUGIN);

    /* Nothing to do if fqdn flag is disabled. */
    if (sd_un != NULL && !sd_un->flag)
	debug_return_bool(true);

    /* If the -h flag was given we need to resolve both host names. */
    remote = strcmp(ctx->runas.host, ctx->user.host) != 0;

    /* First resolve ctx->user.host, setting host and shost. */
    if (resolve_host(ctx->user.host, &lhost, &shost) != 0) {
	if ((rc = resolve_host(ctx->runas.host, &lhost, &shost)) != 0) {
	    gai_log_warning(ctx, SLOG_PARSE_ERROR|SLOG_RAW_MSG, rc,
		N_("unable to resolve host %s"), ctx->user.host);
	    debug_return_bool(false);
	}
    }
    if (ctx->user.shost != ctx->user.host)
	free(ctx->user.shost);
    free(ctx->user.host);
    ctx->user.host = lhost;
    ctx->user.shost = shost;

    /* Next resolve ctx->runas.host, setting host and shost in ctx->runas. */
    lhost = shost = NULL;
    if (remote) {
	if ((rc = resolve_host(ctx->runas.host, &lhost, &shost)) != 0) {
	    gai_log_warning(ctx, SLOG_NO_LOG|SLOG_RAW_MSG, rc,
		N_("unable to resolve host %s"), ctx->runas.host);
	    debug_return_bool(false);
	}
    } else {
	/* Not remote, just use ctx->user.host. */
	if ((lhost = strdup(ctx->user.host)) != NULL) {
	    if (ctx->user.shost != ctx->user.host)
		shost = strdup(ctx->user.shost);
	    else
		shost = lhost;
	}
	if (lhost == NULL || shost == NULL) {
	    free(lhost);
	    if (lhost != shost)
		free(shost);
	    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	    debug_return_bool(false);
	}
    }
    if (lhost != NULL && shost != NULL) {
	if (ctx->runas.shost != ctx->runas.host)
	    free(ctx->runas.shost);
	free(ctx->runas.host);
	ctx->runas.host = lhost;
	ctx->runas.shost = shost;
    }

    sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
	"host %s, shost %s, runas host %s, runas shost %s",
	ctx->user.host, ctx->user.shost, ctx->runas.host, ctx->runas.shost);
    debug_return_bool(true);
}

static bool
cb_tty_tickets(struct sudoers_context *ctx, const char *file,
    int line, int column, const union sudo_defs_val *sd_un, int op)
{
    debug_decl(cb_tty_tickets, SUDOERS_DEBUG_PLUGIN);

    /* Convert tty_tickets -> timestamp_type */
    if (sd_un->flag)
	def_timestamp_type = tty;
    else
	def_timestamp_type = global;
    debug_return_bool(true);
}

static bool
cb_umask(struct sudoers_context *ctx, const char *file,
    int line, int column, const union sudo_defs_val *sd_un, int op)
{
    debug_decl(cb_umask, SUDOERS_DEBUG_PLUGIN);

    /* Override umask if explicitly set in sudoers. */
    override_umask = sd_un->mode != ACCESSPERMS;

    debug_return_bool(true);
}

static bool
cb_runchroot(struct sudoers_context *ctx, const char *file,
    int line, int column, const union sudo_defs_val *sd_un, int op)
{
    debug_decl(cb_runchroot, SUDOERS_DEBUG_PLUGIN);

    sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
	"def_runchroot now %s", sd_un->str);
    if (ctx->user.cmnd != NULL) {
	/* Update ctx->user.cmnd and cmnd_status based on the new chroot. */
	set_cmnd_status(ctx, sd_un->str);
	sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
	    "ctx->user.cmnd now %s", ctx->user.cmnd);
    }

    debug_return_bool(true);
}

static bool
cb_logfile(struct sudoers_context *ctx, const char *file,
    int line, int column, const union sudo_defs_val *sd_un, int op)
{
    int logtype = def_syslog ? EVLOG_SYSLOG : EVLOG_NONE;
    debug_decl(cb_logfile, SUDOERS_DEBUG_PLUGIN);

    if (sd_un->str != NULL)
	SET(logtype, EVLOG_FILE);
    eventlog_set_type(logtype);
    eventlog_set_logpath(sd_un->str);

    debug_return_bool(true);
}

static bool
cb_log_format(struct sudoers_context *ctx, const char *file,
    int line, int column, const union sudo_defs_val *sd_un, int op)
{
    debug_decl(cb_log_format, SUDOERS_DEBUG_PLUGIN);

    eventlog_set_format(sd_un->tuple == sudo ? EVLOG_SUDO : EVLOG_JSON);

    debug_return_bool(true);
}

static bool
cb_syslog(struct sudoers_context *ctx, const char *file,
    int line, int column, const union sudo_defs_val *sd_un, int op)
{
    int logtype = def_logfile ? EVLOG_FILE : EVLOG_NONE;
    debug_decl(cb_syslog, SUDOERS_DEBUG_PLUGIN);

    if (sd_un->str != NULL)
	SET(logtype, EVLOG_SYSLOG);
    eventlog_set_type(logtype);

    debug_return_bool(true);
}

static bool
cb_syslog_goodpri(struct sudoers_context *ctx, const char *file,
    int line, int column, const union sudo_defs_val *sd_un, int op)
{
    debug_decl(cb_syslog_goodpri, SUDOERS_DEBUG_PLUGIN);

    eventlog_set_syslog_acceptpri(sd_un->ival);

    debug_return_bool(true);
}

static bool
cb_syslog_badpri(struct sudoers_context *ctx, const char *file,
    int line, int column, const union sudo_defs_val *sd_un, int op)
{
    debug_decl(cb_syslog_badpri, SUDOERS_DEBUG_PLUGIN);

    eventlog_set_syslog_rejectpri(sd_un->ival);
    eventlog_set_syslog_alertpri(sd_un->ival);

    debug_return_bool(true);
}

static bool
cb_syslog_maxlen(struct sudoers_context *ctx, const char *file,
    int line, int column, const union sudo_defs_val *sd_un, int op)
{
    debug_decl(cb_syslog_maxlen, SUDOERS_DEBUG_PLUGIN);

    eventlog_set_syslog_maxlen((size_t)sd_un->ival);

    debug_return_bool(true);
}

static bool
cb_loglinelen(struct sudoers_context *ctx, const char *file,
    int line, int column, const union sudo_defs_val *sd_un, int op)
{
    debug_decl(cb_loglinelen, SUDOERS_DEBUG_PLUGIN);

    eventlog_set_file_maxlen((size_t)sd_un->ival);

    debug_return_bool(true);
}

static bool
cb_log_year(struct sudoers_context *ctx, const char *file,
    int line, int column, const union sudo_defs_val *sd_un, int op)
{
    debug_decl(cb_syslog_maxlen, SUDOERS_DEBUG_PLUGIN);

    eventlog_set_time_fmt(sd_un->flag ? "%h %e %T %Y" : "%h %e %T");

    debug_return_bool(true);
}

static bool
cb_log_host(struct sudoers_context *ctx, const char *file,
    int line, int column, const union sudo_defs_val *sd_un, int op)
{
    debug_decl(cb_syslog_maxlen, SUDOERS_DEBUG_PLUGIN);

    eventlog_set_omit_hostname(!sd_un->flag);

    debug_return_bool(true);
}

static bool
cb_mailerpath(struct sudoers_context *ctx, const char *file,
    int line, int column, const union sudo_defs_val *sd_un, int op)
{
    debug_decl(cb_mailerpath, SUDOERS_DEBUG_PLUGIN);

    eventlog_set_mailerpath(sd_un->str);

    debug_return_bool(true);
}

static bool
cb_mailerflags(struct sudoers_context *ctx, const char *file,
    int line, int column, const union sudo_defs_val *sd_un, int op)
{
    debug_decl(cb_mailerflags, SUDOERS_DEBUG_PLUGIN);

    eventlog_set_mailerflags(sd_un->str);

    debug_return_bool(true);
}

static bool
cb_mailfrom(struct sudoers_context *ctx, const char *file,
    int line, int column, const union sudo_defs_val *sd_un, int op)
{
    debug_decl(cb_mailfrom, SUDOERS_DEBUG_PLUGIN);

    eventlog_set_mailfrom(sd_un->str);

    debug_return_bool(true);
}

static bool
cb_mailto(struct sudoers_context *ctx, const char *file,
    int line, int column, const union sudo_defs_val *sd_un, int op)
{
    debug_decl(cb_mailto, SUDOERS_DEBUG_PLUGIN);

    eventlog_set_mailto(sd_un->str);

    debug_return_bool(true);
}

static bool
cb_mailsub(struct sudoers_context *ctx, const char *file,
    int line, int column, const union sudo_defs_val *sd_un, int op)
{
    debug_decl(cb_mailsub, SUDOERS_DEBUG_PLUGIN);

    eventlog_set_mailsub(sd_un->str);

    debug_return_bool(true);
}

static bool
cb_intercept_type(struct sudoers_context *ctx, const char *file,
    int line, int column, const union sudo_defs_val *sd_un, int op)
{
    debug_decl(cb_intercept_type, SUDOERS_DEBUG_PLUGIN);

    if (op != -1) {
	/* Set explicitly in sudoers. */
	if (sd_un->tuple == dso) {
	    /* Reset intercept_allow_setid default value. */
	    if (!ISSET(ctx->settings.flags, USER_INTERCEPT_SETID))
		def_intercept_allow_setid = false;
	}
    }

    debug_return_bool(true);
}

static bool
cb_intercept_allow_setid(struct sudoers_context *ctx, const char *file,
    int line, int column, const union sudo_defs_val *sd_un, int op)
{
    debug_decl(cb_intercept_allow_setid, SUDOERS_DEBUG_PLUGIN);

    /* Operator will be -1 if set by front-end. */
    if (op != -1) {
	/* Set explicitly in sudoers. */
	SET(ctx->settings.flags, USER_INTERCEPT_SETID);
    }

    debug_return_bool(true);
}

bool
cb_log_input(struct sudoers_context *ctx, const char *file,
    int line, int column, const union sudo_defs_val *sd_un, int op)
{
    debug_decl(cb_log_input, SUDOERS_DEBUG_PLUGIN);

    def_log_stdin = op;
    def_log_ttyin = op;

    debug_return_bool(true);
}

bool
cb_log_output(struct sudoers_context *ctx, const char *file,
    int line, int column, const union sudo_defs_val *sd_un, int op)
{
    debug_decl(cb_log_output, SUDOERS_DEBUG_PLUGIN);

    def_log_stdout = op;
    def_log_stderr = op;
    def_log_ttyout = op;

    debug_return_bool(true);
}

/*
 * Set parser Defaults callbacks.
 * We do this here instead in def_data.in so we don't have to
 * stub out the callbacks for visudo and testsudoers.
 */
void
set_callbacks(void)
{
    debug_decl(set_callbacks, SUDOERS_DEBUG_PLUGIN);

    /* Set fqdn callback. */
    sudo_defs_table[I_FQDN].callback = cb_fqdn;

    /* Set group_plugin callback. */
    sudo_defs_table[I_GROUP_PLUGIN].callback = cb_group_plugin;

    /* Set runas callback. */
    sudo_defs_table[I_RUNAS_DEFAULT].callback = cb_runas_default;

    /* Set locale callback. */
    sudo_defs_table[I_SUDOERS_LOCALE].callback = sudoers_locale_callback;

#ifdef SESSID_MAX
    /* Set maxseq callback. */
    sudo_defs_table[I_MAXSEQ].callback = cb_maxseq;

    /* Set iolog_user callback. */
    sudo_defs_table[I_IOLOG_USER].callback = cb_iolog_user;

    /* Set iolog_group callback. */
    sudo_defs_table[I_IOLOG_GROUP].callback = cb_iolog_group;

    /* Set iolog_mode callback. */
    sudo_defs_table[I_IOLOG_MODE].callback = cb_iolog_mode;
#endif /* SESSID_MAX */

    /* Set timestampowner callback. */
    sudo_defs_table[I_TIMESTAMPOWNER].callback = cb_timestampowner;

    /* Set tty_tickets callback. */
    sudo_defs_table[I_TTY_TICKETS].callback = cb_tty_tickets;

    /* Set umask callback. */
    sudo_defs_table[I_UMASK].callback = cb_umask;

    /* Set runchroot callback. */
    sudo_defs_table[I_RUNCHROOT].callback = cb_runchroot;

    /* eventlog callbacks */
    sudo_defs_table[I_SYSLOG].callback = cb_syslog;
    sudo_defs_table[I_SYSLOG_GOODPRI].callback = cb_syslog_goodpri;
    sudo_defs_table[I_SYSLOG_BADPRI].callback = cb_syslog_badpri;
    sudo_defs_table[I_SYSLOG_MAXLEN].callback = cb_syslog_maxlen;
    sudo_defs_table[I_LOGLINELEN].callback = cb_loglinelen;
    sudo_defs_table[I_LOG_HOST].callback = cb_log_host;
    sudo_defs_table[I_LOGFILE].callback = cb_logfile;
    sudo_defs_table[I_LOG_FORMAT].callback = cb_log_format;
    sudo_defs_table[I_LOG_YEAR].callback = cb_log_year;
    sudo_defs_table[I_MAILERPATH].callback = cb_mailerpath;
    sudo_defs_table[I_MAILERFLAGS].callback = cb_mailerflags;
    sudo_defs_table[I_MAILFROM].callback = cb_mailfrom;
    sudo_defs_table[I_MAILTO].callback = cb_mailto;
    sudo_defs_table[I_MAILSUB].callback = cb_mailsub;
    sudo_defs_table[I_PASSPROMPT_REGEX].callback = cb_passprompt_regex;
    sudo_defs_table[I_INTERCEPT_TYPE].callback = cb_intercept_type;
    sudo_defs_table[I_INTERCEPT_ALLOW_SETID].callback = cb_intercept_allow_setid;
    sudo_defs_table[I_LOG_INPUT].callback = cb_log_input;
    sudo_defs_table[I_LOG_OUTPUT].callback = cb_log_output;

    debug_return;
}

bool
sudoers_override_umask(void)
{
    return override_umask;
}
