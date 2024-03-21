/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 1994-1996, 1998-2023 Todd C. Miller <Todd.Miller@sudo.ws>
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

#include <sys/stat.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifdef HAVE_NL_LANGINFO
# include <langinfo.h>
#endif /* HAVE_NL_LANGINFO */
#include <netdb.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <syslog.h>
#ifndef HAVE_GETADDRINFO
# include <compat/getaddrinfo.h>
#endif

#include <sudoers.h>
#ifdef SUDOERS_LOG_CLIENT
# include <log_client.h>
# include <strlist.h>
#endif

struct parse_error {
    STAILQ_ENTRY(parse_error) entries;
    char *errstr;
};
STAILQ_HEAD(parse_error_list, parse_error);
static struct parse_error_list parse_error_list =
    STAILQ_HEAD_INITIALIZER(parse_error_list);

static bool should_mail(const struct sudoers_context *ctx, unsigned int);
static bool warned = false;

#ifdef SUDOERS_LOG_CLIENT
/*
 * Convert a defaults-style list to a stringlist.
 */
static struct sudoers_str_list *
list_to_strlist(struct list_members *list)
{
    struct sudoers_str_list *strlist;
    struct sudoers_string *str;
    struct list_member *item;
    debug_decl(slist_to_strlist, SUDOERS_DEBUG_LOGGING);

    if ((strlist = str_list_alloc()) == NULL)
	goto oom;

    SLIST_FOREACH(item, list, entries) {
	if ((str = sudoers_string_alloc(item->value)) == NULL)
	    goto oom;
	/* List is in reverse order, insert at head to fix that. */
	STAILQ_INSERT_HEAD(strlist, str, entries);
    }

    debug_return_ptr(strlist);
oom:
    str_list_free(strlist);
    debug_return_ptr(NULL);
}

bool
init_log_details(struct log_details *details, struct eventlog *evlog)
{
    struct sudoers_str_list *log_servers = NULL;
    debug_decl(init_log_details, SUDOERS_DEBUG_LOGGING);

    memset(details, 0, sizeof(*details));

    if ((log_servers = list_to_strlist(&def_log_servers)) == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	debug_return_bool(false);
    }

    details->evlog = evlog;
    details->ignore_log_errors = def_ignore_logfile_errors;
    details->log_servers = log_servers;
    details->server_timeout.tv_sec = def_log_server_timeout;
    details->keepalive = def_log_server_keepalive;
#if defined(HAVE_OPENSSL)
    details->ca_bundle = def_log_server_cabundle;
    details->cert_file = def_log_server_peer_cert;
    details->key_file = def_log_server_peer_key;
    details->verify_server = def_log_server_verify;
#endif /* HAVE_OPENSSL */

    debug_return_bool(true);
}

bool
log_server_reject(const struct sudoers_context *ctx, struct eventlog *evlog,
    const char *message)
{
    bool ret = false;
    debug_decl(log_server_reject, SUDOERS_DEBUG_LOGGING);

    if (SLIST_EMPTY(&def_log_servers))
	debug_return_bool(true);

    if (ISSET(ctx->mode, MODE_POLICY_INTERCEPTED)) {
	/* Older servers don't support multiple commands per session. */
	if (!client_closure->subcommands)
	    debug_return_bool(true);

	/* Use existing client closure. */
        if (fmt_reject_message(client_closure, evlog)) {
            if (client_closure->write_ev->add(client_closure->write_ev,
                    &client_closure->log_details->server_timeout) == -1) {
                sudo_warn("%s", U_("unable to add event to queue"));
                goto done;
            }
            ret = true;
        }
    } else {
	struct log_details details;

	if (!init_log_details(&details, evlog))
	    debug_return_bool(false);

	/* Open connection to log server, send hello and reject messages. */
	client_closure = log_server_open(&details, &evlog->submit_time,
	    false, SEND_REJECT, message);
	if (client_closure != NULL) {
	    client_closure_free(client_closure);
	    client_closure = NULL;
	    ret = true;
	}

	/* Only the log_servers string list is dynamically allocated. */
	str_list_free(details.log_servers);
    }

done:
    debug_return_bool(ret);
}

bool
log_server_alert(const struct sudoers_context *ctx, struct eventlog *evlog,
    struct timespec *now, const char *message, const char *errstr)
{
    struct log_details details;
    char *emessage = NULL;
    bool ret = false;
    debug_decl(log_server_alert, SUDOERS_DEBUG_LOGGING);

    if (SLIST_EMPTY(&def_log_servers))
	debug_return_bool(true);

    if (errstr != NULL) {
	if (asprintf(&emessage, _("%s: %s"), message, errstr) == -1) {
	    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	    goto done;
	}
    }

    if (ISSET(ctx->mode, MODE_POLICY_INTERCEPTED)) {
	/* Older servers don't support multiple commands per session. */
	if (!client_closure->subcommands) {
            ret = true;
	    goto done;
	}

	/* Use existing client closure. */
        if (fmt_reject_message(client_closure, evlog)) {
            if (client_closure->write_ev->add(client_closure->write_ev,
                    &client_closure->log_details->server_timeout) == -1) {
                sudo_warn("%s", U_("unable to add event to queue"));
                goto done;
            }
            ret = true;
        }
    } else {
	if (!init_log_details(&details, evlog))
	    goto done;

	/* Open connection to log server, send hello and alert messages. */
	client_closure = log_server_open(&details, now, false,
	    SEND_ALERT, emessage ? emessage : message);
	if (client_closure != NULL) {
	    client_closure_free(client_closure);
	    client_closure = NULL;
	    ret = true;
	}

	/* Only the log_servers string list is dynamically allocated. */
	str_list_free(details.log_servers);
    }

done:
    free(emessage);
    debug_return_bool(ret);
}
#else
bool
log_server_reject(const struct sudoers_context *ctx, struct eventlog *evlog,
    const char *message)
{
    return true;
}

bool
log_server_alert(const struct sudoers_context *ctx, struct eventlog *evlog,
    struct timespec *now, const char *message, const char *errstr)
{
    return true;
}
#endif /* SUDOERS_LOG_CLIENT */

/*
 * Log a reject event to syslog, a log file, sudo_logsrvd and/or email.
 */
static bool
log_reject(const struct sudoers_context *ctx, const char *message,
    bool logit, bool mailit)
{
    const char *uuid_str = NULL;
    struct eventlog evlog;
    int evl_flags = 0;
    bool ret;
    debug_decl(log_reject, SUDOERS_DEBUG_LOGGING);

    if (!ISSET(ctx->mode, MODE_POLICY_INTERCEPTED))
	uuid_str = ctx->uuid_str;

    if (mailit) {
	SET(evl_flags, EVLOG_MAIL);
	if (!logit)
	    SET(evl_flags, EVLOG_MAIL_ONLY);
    }
    sudoers_to_eventlog(ctx, &evlog, ctx->runas.cmnd, ctx->runas.argv,
	NULL, uuid_str);
    ret = eventlog_reject(&evlog, evl_flags, message, NULL, NULL);
    if (!log_server_reject(ctx, &evlog, message))
	ret = false;

    debug_return_bool(ret);
}

/*
 * Log, audit and mail the denial message, optionally informing the user.
 */
bool
log_denial(const struct sudoers_context *ctx, unsigned int status,
    bool inform_user)
{
    const char *message;
    int oldlocale;
    bool mailit, ret = true;
    debug_decl(log_denial, SUDOERS_DEBUG_LOGGING);

    /* Send mail based on status. */
    mailit = should_mail(ctx, status);

    /* Set error message. */
    if (ISSET(status, FLAG_NO_USER))
	message = N_("user NOT in sudoers");
    else if (ISSET(status, FLAG_NO_HOST))
	message = N_("user NOT authorized on host");
    else if (ISSET(status, FLAG_INTERCEPT_SETID))
	message = N_("setid command rejected in intercept mode");
    else
	message = N_("command not allowed");

    /* Do auditing first (audit_failure() handles the locale itself). */
    audit_failure(ctx, ctx->runas.argv, "%s", message);

    if (def_log_denied || mailit) {
	/* Log and mail messages should be in the sudoers locale. */
	sudoers_setlocale(SUDOERS_LOCALE_SUDOERS, &oldlocale);

	if (!log_reject(ctx, message, def_log_denied, mailit))
	    ret = false;

	/* Restore locale. */
	sudoers_setlocale(oldlocale, NULL);
    }

    /* Inform the user of the failure (in their locale).  */
    if (inform_user) {
	sudoers_setlocale(SUDOERS_LOCALE_USER, &oldlocale);

	if (ISSET(status, FLAG_NO_USER)) {
	    sudo_printf(SUDO_CONV_ERROR_MSG, _("%s is not in the sudoers "
		"file.\n"), ctx->user.name);
	} else if (ISSET(status, FLAG_NO_HOST)) {
	    sudo_printf(SUDO_CONV_ERROR_MSG, _("%s is not allowed to run sudo "
		"on %s.\n"), ctx->user.name, ctx->runas.shost);
	} else if (ISSET(status, FLAG_INTERCEPT_SETID)) {
	    sudo_printf(SUDO_CONV_ERROR_MSG, _("%s: %s\n"), getprogname(),
		_("setid commands are not permitted in intercept mode"));
	} else if (ISSET(status, FLAG_NO_CHECK)) {
	    sudo_printf(SUDO_CONV_ERROR_MSG, _("Sorry, user %s may not run "
		"sudo on %s.\n"), ctx->user.name, ctx->runas.shost);
	} else {
	    const struct passwd *runas_pw =
		ctx->runas.list_pw ? ctx->runas.list_pw : ctx->runas.pw;
	    const char *cmnd1 = ctx->user.cmnd;
	    const char *cmnd2 = "";

	    if (ISSET(ctx->mode, MODE_CHECK)) {
		/* For "sudo -l command" the command run is in runas.argv[1]. */
		cmnd1 = "list ";
		cmnd2 = ctx->runas.argv[1];
	    }
	    sudo_printf(SUDO_CONV_ERROR_MSG, _("Sorry, user %s is not allowed "
		"to execute '%s%s%s%s' as %s%s%s on %s.\n"),
		ctx->user.name, cmnd1, cmnd2, ctx->user.cmnd_args ? " " : "",
		ctx->user.cmnd_args ? ctx->user.cmnd_args : "",
		runas_pw ? runas_pw->pw_name : ctx->user.name,
		ctx->runas.gr ? ":" : "",
		ctx->runas.gr ? ctx->runas.gr->gr_name : "",
		ctx->user.host);
	}
	if (mailit) {
	    sudo_printf(SUDO_CONV_ERROR_MSG, "%s",
		_("This incident has been reported to the administrator.\n"));
	}
	sudoers_setlocale(oldlocale, NULL);
    }
    debug_return_bool(ret);
}

/*
 * Log and audit that user was not allowed to run the command.
 */
bool
log_failure(const struct sudoers_context *ctx, unsigned int status,
    int cmnd_status)
{
    bool ret, inform_user = true;
    debug_decl(log_failure, SUDOERS_DEBUG_LOGGING);

    /* The user doesn't always get to see the log message (path info). */
    if (!ISSET(status, FLAG_NO_USER | FLAG_NO_HOST) &&
	    ctx->runas.list_pw == NULL && def_path_info &&
	    (cmnd_status == NOT_FOUND_DOT || cmnd_status == NOT_FOUND))
	inform_user = false;
    ret = log_denial(ctx, status, inform_user);

    if (!inform_user) {
	const char *cmnd = ctx->user.cmnd;
	if (ISSET(ctx->mode, MODE_CHECK))
	    cmnd = ctx->user.cmnd_list ? ctx->user.cmnd_list : ctx->runas.argv[1];

	/*
	 * We'd like to not leak path info at all here, but that can
	 * *really* confuse the users.  To really close the leak we'd
	 * have to say "not allowed to run foo" even when the problem
	 * is just "no foo in path" since the user can trivially set
	 * their path to just contain a single dir.
	 */
	if (cmnd_status == NOT_FOUND)
	    sudo_warnx(U_("%s: command not found"), cmnd);
	else if (cmnd_status == NOT_FOUND_DOT)
	    sudo_warnx(U_("ignoring \"%s\" found in '.'\nUse \"sudo ./%s\" if this is the \"%s\" you wish to run."), cmnd, cmnd, cmnd);
    }

    debug_return_bool(ret);
}

/*
 * Format an authentication failure message, using either
 * authfail_message from sudoers or a locale-specific message.
 */
static char *
fmt_authfail_message(unsigned int tries)
{
    char numbuf[STRLEN_MAX_UNSIGNED(unsigned int) + 1];
    char *dst, *dst_end, *ret = NULL;
    const char *src;
    size_t len;
    debug_decl(fmt_authfail_message, SUDOERS_DEBUG_LOGGING);

    if (def_authfail_message == NULL) {
	if (asprintf(&ret, ngettext("%u incorrect password attempt",
		"%u incorrect password attempts", tries), tries) == -1)
	    goto oom;
	debug_return_ptr(ret);
    }

    len = (size_t)snprintf(numbuf, sizeof(numbuf), "%u", tries);
    if (len >= sizeof(numbuf))
	goto overflow;

    src = def_authfail_message;
    len = strlen(src) + 1;
    while (*src != '\0') {
	if (src[0] == '%') {
	    switch (src[1]) {
	    case '%':
		len--;
		src++;
		break;
	    case 'd':
		len -= 2;
		len += strlen(numbuf);
		src++;
		break;
	    default:
		/* pass through as-is */
		break;
	    }
	}
	src++;
    }

    if ((ret = malloc(len)) == NULL)
	goto oom;
    dst = ret;
    dst_end = ret + len;

    src = def_authfail_message;
    while (*src != '\0') {
	/* Always leave space for the terminating NUL. */
	if (dst + 1 >= dst_end)
	    goto overflow;
	if (src[0] == '%') {
	    switch (src[1]) {
	    case '%':
		src++;
		break;
	    case 'd':
		len = strlcpy(dst, numbuf, (size_t)(dst_end - dst));
		if (len >= (size_t)(dst_end - dst))
		    goto overflow;
		dst += len;
		src += 2;
		continue;
	    default:
		/* pass through as-is */
		break;
	    }
	}
	*dst++ = *src++;
    }
    *dst = '\0';

    debug_return_ptr(ret);

oom:
    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
    debug_return_ptr(NULL);

overflow:
    sudo_warnx(U_("internal error, %s overflow"), __func__);
    free(ret);
    errno = ERANGE;
    debug_return_ptr(NULL);
}

/*
 * Log and audit that user was not able to authenticate themselves.
 */
bool
log_auth_failure(const struct sudoers_context *ctx, unsigned int status,
    unsigned int tries)
{
    char *message = NULL;
    int oldlocale;
    bool ret = true;
    bool mailit = false;
    bool logit = true;
    debug_decl(log_auth_failure, SUDOERS_DEBUG_LOGGING);

    /* Do auditing first (audit_failure() handles the locale itself). */
    audit_failure(ctx, ctx->runas.argv, "%s", N_("authentication failure"));

    /* If sudoers denied the command we'll log that separately. */
    if (!ISSET(status, FLAG_BAD_PASSWORD|FLAG_NO_USER_INPUT))
	logit = false;

    /*
     * Do we need to send mail?
     * We want to avoid sending multiple messages for the same command
     * so if we are going to send an email about the denial, that takes
     * precedence.
     */
    if (ISSET(status, VALIDATE_SUCCESS)) {
	/* Command allowed, auth failed; do we need to send mail? */
	if (def_mail_badpass || def_mail_always)
	    mailit = true;
	if (!def_log_denied)
	    logit = false;
    } else {
	/* Command denied, auth failed; make sure we don't send mail twice. */
	if (def_mail_badpass && !should_mail(ctx, status))
	    mailit = true;
	/* Don't log the bad password message, we'll log a denial instead. */
	logit = false;
    }

    if (logit || mailit) {
	/* Log and mail messages should be in the sudoers locale. */
	sudoers_setlocale(SUDOERS_LOCALE_SUDOERS, &oldlocale);

	if (ISSET(status, FLAG_BAD_PASSWORD)) {
	    message = fmt_authfail_message(tries);
	    if (message == NULL) {
		ret = false;
	    } else {
		ret = log_reject(ctx, message, logit, mailit);
		free(message);
	    }
	} else {
	    ret = log_reject(ctx, _("a password is required"), logit, mailit);
	}

	/* Restore locale. */
	sudoers_setlocale(oldlocale, NULL);
    }

    /* Inform the user if they failed to authenticate (in their locale).  */
    sudoers_setlocale(SUDOERS_LOCALE_USER, &oldlocale);

    if (ISSET(status, FLAG_BAD_PASSWORD)) {
	message = fmt_authfail_message(tries);
	if (message == NULL) {
	    ret = false;
	} else {
	    sudo_warnx("%s", message);
	    free(message);
	}
    } else {
	sudo_warnx("%s", _("a password is required"));
    }

    sudoers_setlocale(oldlocale, NULL);

    debug_return_bool(ret);
}

/*
 * Log and potentially mail the allowed command.
 */
bool
log_allowed(const struct sudoers_context *ctx, struct eventlog *evlog)
{
    int oldlocale;
    int evl_flags = 0;
    bool mailit, ret = true;
    debug_decl(log_allowed, SUDOERS_DEBUG_LOGGING);

    /* Send mail based on status. */
    mailit = should_mail(ctx, VALIDATE_SUCCESS);

    if (def_log_allowed || mailit) {
	/* Log and mail messages should be in the sudoers locale. */
	sudoers_setlocale(SUDOERS_LOCALE_SUDOERS, &oldlocale);

	if (mailit) {
	    SET(evl_flags, EVLOG_MAIL);
	    if (!def_log_allowed)
		SET(evl_flags, EVLOG_MAIL_ONLY);
	}
	if (!eventlog_accept(evlog, evl_flags, NULL, NULL))
	    ret = false;

	sudoers_setlocale(oldlocale, NULL);
    }

    debug_return_bool(ret);
}

bool
log_exit_status(const struct sudoers_context *ctx, int status)
{
    struct eventlog evlog;
    int evl_flags = 0;
    int exit_value = 0;
    int oldlocale;
    struct timespec run_time;
    char sigbuf[SIG2STR_MAX];
    char *signal_name = NULL;
    bool dumped_core = false;
    bool ret = true;
    debug_decl(log_exit_status, SUDOERS_DEBUG_LOGGING);

    if (def_log_exit_status || def_mail_always) {
	if (sudo_gettime_real(&run_time) == -1) {
	    sudo_warn("%s", U_("unable to get time of day"));
	    ret = false;
	    goto done;
	}
	sudo_timespecsub(&run_time, &ctx->submit_time, &run_time);

        if (WIFEXITED(status)) {
	    exit_value = WEXITSTATUS(status);
        } else if (WIFSIGNALED(status)) {
            int signo = WTERMSIG(status);
            if (signo <= 0 || sig2str(signo, sigbuf) == -1)
                (void)snprintf(sigbuf, sizeof(sigbuf), "%d", signo);
	    signal_name = sigbuf;
	    exit_value = signo | 128;
	    dumped_core = WCOREDUMP(status);
        } else {
            sudo_warnx("invalid exit status 0x%x", status);
	    ret = false;
	    goto done;
        }

	/* Log and mail messages should be in the sudoers locale. */
	sudoers_setlocale(SUDOERS_LOCALE_SUDOERS, &oldlocale);

	sudoers_to_eventlog(ctx, &evlog, ctx->runas.cmnd_saved,
	    ctx->runas.argv_saved, NULL, ctx->uuid_str);
	if (def_mail_always) {
	    SET(evl_flags, EVLOG_MAIL);
	    if (!def_log_exit_status)
		SET(evl_flags, EVLOG_MAIL_ONLY);
	}
	evlog.run_time = run_time;
	evlog.exit_value = exit_value;
	evlog.signal_name = signal_name;
	evlog.dumped_core = dumped_core;
	if (!eventlog_exit(&evlog, evl_flags))
	    ret = false;

	sudoers_setlocale(oldlocale, NULL);
    }

done:
    debug_return_bool(ret);
}

/*
 * Add message to the parse error journal, which takes ownership of it.
 * The message will be freed once the journal is processed.
 * Returns true if message was journaled (and consumed), else false.
 */
static bool
journal_parse_error(char *message)
{
    struct parse_error *pe;
    debug_decl(journal_parse_error, SUDOERS_DEBUG_LOGGING);

    pe = malloc(sizeof(*pe));
    if (pe == NULL)
	debug_return_bool(false);
    pe->errstr = message;
    STAILQ_INSERT_TAIL(&parse_error_list, pe, entries);
    debug_return_bool(true);
}

/*
 * Perform logging for log_warning()/log_warningx().
 */
static bool
vlog_warning(const struct sudoers_context *ctx, unsigned int flags,
    int errnum, const char * restrict fmt, va_list ap)
{
    struct eventlog evlog;
    struct timespec now;
    const char *errstr = NULL;
    char *message;
    bool ret = true;
    int len, oldlocale;
    int evl_flags = 0;
    va_list ap2;
    debug_decl(vlog_warning, SUDOERS_DEBUG_LOGGING);

    /* Do auditing first (audit_failure() handles the locale itself). */
    if (ISSET(flags, SLOG_AUDIT)) {
	va_copy(ap2, ap);
	vaudit_failure(ctx, ctx->runas.argv, fmt, ap2);
	va_end(ap2);
    }

    /* Need extra copy of ap for sudo_vwarn()/sudo_vwarnx() below. */
    va_copy(ap2, ap);

    /* Log messages should be in the sudoers locale. */
    sudoers_setlocale(SUDOERS_LOCALE_SUDOERS, &oldlocale);

    /* Expand printf-style format + args. */
    len = vasprintf(&message, _(fmt), ap);
    if (len == -1) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	ret = false;
	goto done;
    }

    if (ISSET(flags, SLOG_USE_ERRNO))
	errstr = strerror(errnum);
    else if (ISSET(flags, SLOG_GAI_ERRNO))
	errstr = gai_strerror(errnum);

    /* Log to debug file. */
    if (errstr != NULL) {
	sudo_debug_printf2(NULL, NULL, 0,
	    SUDO_DEBUG_WARN|sudo_debug_subsys, "%s: %s", message, errstr);
    } else {
	sudo_debug_printf2(NULL, NULL, 0,
	    SUDO_DEBUG_WARN|sudo_debug_subsys, "%s", message);
    }

    if (ISSET(flags, SLOG_SEND_MAIL) || !ISSET(flags, SLOG_NO_LOG)) {
	if (sudo_gettime_real(&now) == -1) {
	    sudo_warn("%s", U_("unable to get time of day"));
	    goto done;
	}
	if (ISSET(flags, SLOG_RAW_MSG))
	    SET(evl_flags, EVLOG_RAW);
	if (ISSET(flags, SLOG_SEND_MAIL)) {
	    SET(evl_flags, EVLOG_MAIL);
	    if (ISSET(flags, SLOG_NO_LOG))
		SET(evl_flags, EVLOG_MAIL_ONLY);
	}
	sudoers_to_eventlog(ctx, &evlog, ctx->runas.cmnd, ctx->runas.argv,
	    NULL, ctx->uuid_str);
	if (!eventlog_alert(&evlog, evl_flags, &now, message, errstr))
	    ret = false;
	if (!log_server_alert(ctx, &evlog, &now, message, errstr))
	    ret = false;
    }

    if (ISSET(flags, SLOG_PARSE_ERROR)) {
	char *copy;

	/* Journal parse error for later mailing. */
	if (errstr != NULL) {
	    if (asprintf(&copy, U_("%s: %s"), message, errstr) == -1)
		copy = NULL;
	} else {
	    copy = strdup(message);
	}
	if (copy != NULL) {
	    /* journal_parse_error() takes ownership of copy on success. */
	    if (!journal_parse_error(copy)) {
		free(copy);
		ret = false;
	    }
	}
    }

    /*
     * Tell the user (in their locale).
     */
    if (!ISSET(flags, SLOG_NO_STDERR)) {
	sudoers_setlocale(SUDOERS_LOCALE_USER, NULL);
	if (ISSET(flags, SLOG_USE_ERRNO)) {
	    errno = errnum;
	    sudo_vwarn_nodebug(_(fmt), ap2);
	} else if (ISSET(flags, SLOG_GAI_ERRNO)) {
	    sudo_gai_vwarn_nodebug(errnum, _(fmt), ap2);
	} else {
	    sudo_vwarnx_nodebug(_(fmt), ap2);
	}
    }

done:
    va_end(ap2);
    sudoers_setlocale(oldlocale, NULL);

    debug_return_bool(ret);
}

bool
log_warning(const struct sudoers_context *ctx, unsigned int flags,
    const char * restrict fmt, ...)
{
    va_list ap;
    bool ret;
    debug_decl(log_warning, SUDOERS_DEBUG_LOGGING);

    /* Log the error. */
    va_start(ap, fmt);
    ret = vlog_warning(ctx, flags|SLOG_USE_ERRNO, errno, fmt, ap);
    va_end(ap);

    debug_return_bool(ret);
}

bool
log_warningx(const struct sudoers_context *ctx, unsigned int flags,
    const char * restrict fmt, ...)
{
    va_list ap;
    bool ret;
    debug_decl(log_warningx, SUDOERS_DEBUG_LOGGING);

    /* Log the error. */
    va_start(ap, fmt);
    ret = vlog_warning(ctx, flags, 0, fmt, ap);
    va_end(ap);

    debug_return_bool(ret);
}

bool
gai_log_warning(const struct sudoers_context *ctx, unsigned int flags,
    int errnum, const char * restrict fmt, ...)
{
    va_list ap;
    bool ret;
    debug_decl(gai_log_warning, SUDOERS_DEBUG_LOGGING);

    /* Log the error. */
    va_start(ap, fmt);
    ret = vlog_warning(ctx, flags|SLOG_GAI_ERRNO, errnum, fmt, ap);
    va_end(ap);

    debug_return_bool(ret);
}

/*
 * Send mail about accumulated parser errors.
 * Frees the list of parse errors when done.
 */
bool
mail_parse_errors(const struct sudoers_context *ctx)
{
    const int evl_flags = EVLOG_RAW;
    struct parse_error *pe;
    struct eventlog evlog;
    char **errors = NULL;
    struct timespec now;
    bool ret = false;
    size_t n;
    debug_decl(mail_parse_errors, SUDOERS_DEBUG_LOGGING);

    if (STAILQ_EMPTY(&parse_error_list))
	debug_return_bool(true);

    if (sudo_gettime_real(&now) == -1) {
	sudo_warn("%s", U_("unable to get time of day"));
	goto done;
    }
    sudoers_to_eventlog(ctx, &evlog, ctx->runas.cmnd, ctx->runas.argv,
	NULL, ctx->uuid_str);

    /* Convert parse_error_list to a string vector. */
    n = 0;
    STAILQ_FOREACH(pe, &parse_error_list, entries) {
	n++;
    }
    errors = reallocarray(NULL, n + 1, sizeof(char *));
    if (errors == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	goto done;
    }
    n = 0;
    STAILQ_FOREACH(pe, &parse_error_list, entries) {
	errors[n++] = _(pe->errstr);
    }
    errors[n] = NULL;

    ret = eventlog_mail(&evlog, evl_flags, &now, _("problem parsing sudoers"),
	NULL, errors);

done:
    free(errors);
    while ((pe = STAILQ_FIRST(&parse_error_list)) != NULL) {
	STAILQ_REMOVE_HEAD(&parse_error_list, entries);
	free(pe->errstr);
	free(pe);
    }
    debug_return_bool(ret);
}

/*
 * Log a parse error using log_warningx().
 * Journals the message to be mailed after parsing is complete.
 * Does not write the message to stderr.
 */
bool
log_parse_error(const struct sudoers_context *ctx, const char *file,
    int line, int column, const char * restrict fmt, va_list args)
{
    const unsigned int flags = SLOG_RAW_MSG|SLOG_NO_STDERR;
    char *message, *tofree = NULL;
    const char *errstr;
    bool ret;
    int len;
    debug_decl(log_parse_error, SUDOERS_DEBUG_LOGGING);

    if (fmt == NULL) {
	errstr = _("syntax error");
    } else if (strcmp(fmt, "%s") == 0) {
	/* Optimize common case, a single string. */
	errstr = _(va_arg(args, char *));
    } else {
	if (vasprintf(&tofree, _(fmt), args) == -1)
	    debug_return_bool(false);
	errstr = tofree;
    }

    if (line > 0) {
	ret = log_warningx(ctx, flags, N_("%s:%d:%d: %s"), file, line, column,
	    errstr);
    } else {
	ret = log_warningx(ctx, flags, N_("%s: %s"), file, errstr);
    }

    /* Journal parse error for later mailing. */
    if (line > 0) {
	len = asprintf(&message, _("%s:%d:%d: %s"), file, line, column, errstr);
    } else {
	len = asprintf(&message, _("%s: %s"), file, errstr);
    }
    if (len != -1) {
	if (!journal_parse_error(message)) {
	    free(message);
	    ret = false;
	}
    } else {
	ret = false;
    }

    free(tofree);

    debug_return_bool(ret);
}

/*
 * Determine whether we should send mail based on "status" and defaults options.
 */
static bool
should_mail(const struct sudoers_context *ctx, unsigned int status)
{
    debug_decl(should_mail, SUDOERS_DEBUG_LOGGING);

    if (!def_mailto || !def_mailerpath || access(def_mailerpath, X_OK) == -1)
	debug_return_bool(false);

    debug_return_bool(def_mail_always || ISSET(status, VALIDATE_ERROR) ||
	(def_mail_all_cmnds && ISSET(ctx->mode, (MODE_RUN|MODE_EDIT))) ||
	(def_mail_no_user && ISSET(status, FLAG_NO_USER)) ||
	(def_mail_no_host && ISSET(status, FLAG_NO_HOST)) ||
	(def_mail_no_perms && !ISSET(status, VALIDATE_SUCCESS)));
}

/*
 * Build a struct eventlog from sudoers data.
 * The values in the resulting eventlog struct should not be freed.
 */
void
sudoers_to_eventlog(const struct sudoers_context *ctx, struct eventlog *evlog,
    const char *cmnd, char * const runargv[], char * const runenv[],
    const char *uuid_str)
{
    struct group *grp;
    debug_decl(sudoers_to_eventlog, SUDOERS_DEBUG_LOGGING);

    /* We rely on the reference held by the group cache. */
    if ((grp = sudo_getgrgid(ctx->user.pw->pw_gid)) != NULL)
	sudo_gr_delref(grp);

    memset(evlog, 0, sizeof(*evlog));
    evlog->iolog_file = ctx->iolog_file;
    evlog->iolog_path = ctx->iolog_path;
    evlog->command = cmnd ? (char *)cmnd : (runargv ? runargv[0] : NULL);
    evlog->cwd = ctx->user.cwd;
    if (def_runchroot != NULL && strcmp(def_runchroot, "*") != 0) {
	evlog->runchroot = def_runchroot;
    }
    if (def_runcwd && strcmp(def_runcwd, "*") != 0) {
	evlog->runcwd = def_runcwd;
    } else if (ISSET(ctx->mode, MODE_LOGIN_SHELL) && ctx->runas.pw != NULL) {
	evlog->runcwd = ctx->runas.pw->pw_dir;
    } else {
	evlog->runcwd = ctx->user.cwd;
    }
    evlog->rungroup = ctx->runas.gr ? ctx->runas.gr->gr_name : ctx->runas.group;
    evlog->source = ctx->source;
    evlog->submithost = ctx->user.host;
    evlog->submituser = ctx->user.name;
    if (grp != NULL)
	evlog->submitgroup = grp->gr_name;
    evlog->ttyname = ctx->user.ttypath;
    evlog->runargv = (char **)runargv;
    evlog->env_add = (char **)ctx->user.env_add;
    evlog->runenv = (char **)runenv;
    evlog->submitenv = (char **)ctx->user.envp;
    evlog->submit_time = ctx->submit_time;
    evlog->lines = ctx->user.lines;
    evlog->columns = ctx->user.cols;
    if (ctx->runas.pw != NULL) {
	evlog->rungid = ctx->runas.pw->pw_gid;
	evlog->runuid = ctx->runas.pw->pw_uid;
	evlog->runuser = ctx->runas.pw->pw_name;
    } else {
	evlog->rungid = (gid_t)-1;
	evlog->runuid = (uid_t)-1;
	evlog->runuser = ctx->runas.user;
    }
    if (uuid_str == NULL) {
	unsigned char uuid[16];

	sudo_uuid_create(uuid);
	if (sudo_uuid_to_string(uuid, evlog->uuid_str, sizeof(evlog->uuid_str)) == NULL)
	    sudo_warnx("%s", U_("unable to generate UUID"));
    } else {
	strlcpy(evlog->uuid_str, uuid_str, sizeof(evlog->uuid_str));
    }
    if (ISSET(ctx->mode, MODE_POLICY_INTERCEPTED)) {
	struct timespec now;
	if (sudo_gettime_real(&now) == -1) {
	    sudo_warn("%s", U_("unable to get time of day"));
	} else {
	    sudo_timespecsub(&now, &ctx->submit_time, &evlog->iolog_offset);
	}
    }

    debug_return;
}

static FILE *
sudoers_log_open(int type, const char *log_file)
{
    const char *omode;
    bool uid_changed;
    FILE *fp = NULL;
    mode_t oldmask;
    int fd, flags;
    debug_decl(sudoers_log_open, SUDOERS_DEBUG_LOGGING);

    switch (type) {
	case EVLOG_SYSLOG:
	    openlog("sudo", def_syslog_pid ? LOG_PID : 0, def_syslog);
	    break;
	case EVLOG_FILE:
	    /* Open log file as root, mode 0600 (cannot append to JSON). */
	    if (def_log_format == json) {
		flags = O_RDWR|O_CREAT;
		omode = "w";
	    } else {
		flags = O_WRONLY|O_APPEND|O_CREAT;
		omode = "a";
	    }
	    oldmask = umask(S_IRWXG|S_IRWXO);
	    uid_changed = set_perms(NULL, PERM_ROOT);
	    fd = open(log_file, flags, S_IRUSR|S_IWUSR);
	    if (uid_changed && !restore_perms()) {
		if (fd != -1) {
		    close(fd);
		    fd = -1;
		}
	    }
	    (void) umask(oldmask);
	    if (fd == -1 || (fp = fdopen(fd, omode)) == NULL) {
		if (!warned) {
		    warned = true;
		    sudo_warn(U_("unable to open log file %s"), log_file);
		}
		if (fd != -1)
		    close(fd);
	    }
	    break;
	default:
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		"unsupported log type %d", type);
	    break;
    }

    debug_return_ptr(fp);
}

static void
sudoers_log_close(int type, FILE *fp)
{
    debug_decl(sudoers_log_close, SUDOERS_DEBUG_LOGGING);

    switch (type) {
	case EVLOG_SYSLOG:
	    break;
	case EVLOG_FILE:
	    if (fp == NULL) {
		sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		    "tried to close NULL log stream");
		break;
	    }
	    (void)fflush(fp);
	    if (ferror(fp) && !warned) {
		warned = true;
		sudo_warn(U_("unable to write log file %s"), def_logfile);
	    }
	    fclose(fp);
	    break;
	default:
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		"unsupported log type %d", type);
	    break;
    }

    debug_return;
}

void
init_eventlog_config(void)
{
    int logtype = 0;
    debug_decl(init_eventlog_config, SUDOERS_DEBUG_LOGGING);

    if (def_syslog)
	logtype |= EVLOG_SYSLOG;
    if (def_logfile)
	logtype |= EVLOG_FILE;

    eventlog_set_type(logtype);
    eventlog_set_format(def_log_format == sudo ? EVLOG_SUDO : EVLOG_JSON);
    eventlog_set_syslog_acceptpri(def_syslog_goodpri);
    eventlog_set_syslog_rejectpri(def_syslog_badpri);
    eventlog_set_syslog_alertpri(def_syslog_badpri);
    eventlog_set_syslog_maxlen(def_syslog_maxlen);
    eventlog_set_file_maxlen(def_loglinelen);
    eventlog_set_mailuid(ROOT_UID);
    eventlog_set_omit_hostname(!def_log_host);
    eventlog_set_logpath(def_logfile);
    eventlog_set_time_fmt(def_log_year ? "%h %e %T %Y" : "%h %e %T");
    eventlog_set_mailerpath(def_mailerpath);
    eventlog_set_mailerflags(def_mailerflags);
    eventlog_set_mailfrom(def_mailfrom);
    eventlog_set_mailto(def_mailto);
    eventlog_set_mailsub(def_mailsub);
    eventlog_set_open_log(sudoers_log_open);
    eventlog_set_close_log(sudoers_log_close);

    debug_return;
}
