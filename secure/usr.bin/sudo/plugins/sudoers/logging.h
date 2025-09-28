/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 1999-2005, 2009-2022
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

#ifndef SUDOERS_LOGGING_H
#define SUDOERS_LOGGING_H

#include <stdarg.h>

struct sudoers_str_list;
struct log_details {
    struct eventlog *evlog;
    struct sudoers_str_list *log_servers;
    struct timespec server_timeout;
# if defined(HAVE_OPENSSL)
    char *ca_bundle;
    char *cert_file;
    char *key_file;
# endif /* HAVE_OPENSSL */
    bool keepalive;
    bool verify_server;
    bool ignore_log_errors;
};

/*
 * Values for sudoers_setlocale()
 */
#define SUDOERS_LOCALE_USER     0
#define SUDOERS_LOCALE_SUDOERS  1

/* Logging types */
#define SLOG_SYSLOG		0x01
#define SLOG_FILE		0x02
#define SLOG_BOTH		0x03

/* Flags for log_warning()/log_warningx() */
#define SLOG_USE_ERRNO		0x01	/* internal use only */
#define SLOG_GAI_ERRNO		0x02	/* internal use only */
#define SLOG_RAW_MSG		0x04	/* do not format msg before logging */
#define SLOG_SEND_MAIL		0x08	/* log via mail */
#define SLOG_NO_STDERR		0x10	/* do not log via stderr */
#define SLOG_NO_LOG		0x20	/* do not log via file or syslog */
#define SLOG_AUDIT		0x40	/* send message to audit as well */
#define SLOG_PARSE_ERROR	0x80	/* format as a parse error */

struct sudoers_context;
typedef bool (*sudoers_logger_t)(const struct sudoers_context *ctx, const char *file, int line, int column, const char * restrict fmt, va_list args);

/* XXX - needed for auditing */
extern char *audit_msg;

union sudo_defs_val;
struct sudo_plugin_event;
struct log_details;

bool sudoers_warn_setlocale(bool restore, int *cookie);
bool sudoers_setlocale(int locale_type, int *prev_locale);
int sudoers_getlocale(void);
int audit_failure(const struct sudoers_context *ctx, char *const argv[], char const * restrict const fmt, ...) sudo_printflike(3, 4);
int vaudit_failure(const struct sudoers_context *ctx, char *const argv[], char const * restrict const fmt, va_list ap) sudo_printflike(3, 0);
bool log_allowed(const struct sudoers_context *ctx, struct eventlog *evlog);
bool log_exit_status(const struct sudoers_context *ctx, int exit_status);
bool log_auth_failure(const struct sudoers_context *ctx, unsigned int status, unsigned int tries);
bool log_denial(const struct sudoers_context *ctx, unsigned int status, bool inform_user);
bool log_failure(const struct sudoers_context *ctx, unsigned int status, int flags);
bool log_server_alert(const struct sudoers_context *ctx, struct eventlog *evlog, struct timespec *now, const char *message, const char *errstr);
bool log_server_reject(const struct sudoers_context *ctx, struct eventlog *evlog, const char *message);
bool log_warning(const struct sudoers_context *ctx, unsigned int flags, const char * restrict fmt, ...) sudo_printflike(3, 4);
bool log_warningx(const struct sudoers_context *ctx, unsigned int flags, const char * restrict fmt, ...) sudo_printflike(3, 4);
bool gai_log_warning(const struct sudoers_context *ctx, unsigned int flags, int errnum, const char * restrict fmt, ...) sudo_printflike(4, 5);
bool sudoers_initlocale(const char *ulocale, const char *slocale);
bool sudoers_locale_callback(struct sudoers_context *ctx, const char *file, int line, int column, const union sudo_defs_val *sd_un, int op);
void sudoers_to_eventlog(const struct sudoers_context *ctx, struct eventlog *evlog, const char *cmnd, char * const runargv[], char *const runenv[], const char *uuid_str);
void init_eventlog_config(void);
bool init_log_details(struct log_details *details, struct eventlog *evlog);
bool log_parse_error(const struct sudoers_context *ctx, const char *file, int line, int column, const char * restrict fmt, va_list ap) sudo_printf0like(5, 0);
bool mail_parse_errors(const struct sudoers_context *ctx);

#endif /* SUDOERS_LOGGING_H */
