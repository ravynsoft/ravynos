/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2019-2023 Todd C. Miller <Todd.Miller@sudo.ws>
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
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <limits.h>
#include <netdb.h>
#ifdef HAVE_STDBOOL_H
# include <stdbool.h>
#else
# include <compat/stdbool.h>
#endif
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>
#include <grp.h>
#include <pwd.h>
#ifndef HAVE_GETADDRINFO
# include <compat/getaddrinfo.h>
#endif

#include <pathnames.h>
#include <sudo_compat.h>
#include <sudo_debug.h>
#include <sudo_eventlog.h>
#include <sudo_fatal.h>
#include <sudo_gettext.h>
#include <sudo_iolog.h>
#include <sudo_util.h>

#include <logsrvd.h>

#if defined(HAVE_OPENSSL)
# define DEFAULT_CA_CERT_PATH       "/etc/ssl/sudo/cacert.pem"
# define DEFAULT_SERVER_CERT_PATH   "/etc/ssl/sudo/certs/logsrvd_cert.pem"
# define DEFAULT_SERVER_KEY_PATH    "/etc/ssl/sudo/private/logsrvd_key.pem"

/* Evaluates to true if at least one TLS field is set, else false. */
# define TLS_CONFIGURED(_s)						\
    ((_s).tls_key_path != NULL || (_s).tls_cert_path != NULL ||		\
     (_s).tls_cacert_path != NULL || (_s).tls_dhparams_path != NULL ||	\
     (_s).tls_ciphers_v12 != NULL || (_s).tls_ciphers_v13 != NULL ||	\
     (_s).tls_verify != -1)

/* Evaluates to the relay-specific TLS setting, falling back to server. */
# define TLS_RELAY_STR(_c, _f)	\
    ((_c)->relay._f != NULL ? (_c)->relay._f : (_c)->server._f)

# define TLS_RELAY_INT(_c, _f)	\
    ((_c)->relay._f != -1 ? (_c)->relay._f : (_c)->server._f)
#endif

enum server_log_type {
    SERVER_LOG_NONE,
    SERVER_LOG_STDERR,
    SERVER_LOG_SYSLOG,
    SERVER_LOG_FILE
};

struct logsrvd_config;
typedef bool (*logsrvd_conf_cb_t)(struct logsrvd_config *, const char *, size_t);

struct logsrvd_config_entry {
    const char *conf_str;
    logsrvd_conf_cb_t setter;
    size_t offset;
};

struct logsrvd_config_section {
    const char *name;
    struct logsrvd_config_entry *entries;
};

struct address_list_container {
    unsigned int refcnt;
    struct server_address_list addrs;
};

static struct logsrvd_config {
    struct logsrvd_config_server {
        struct address_list_container addresses;
        struct timespec timeout;
        bool tcp_keepalive;
	enum server_log_type log_type;
	FILE *log_stream;
	char *log_file;
	char *pid_file;
#if defined(HAVE_OPENSSL)
	char *tls_key_path;
	char *tls_cert_path;
	char *tls_cacert_path;
	char *tls_dhparams_path;
	char *tls_ciphers_v12;
	char *tls_ciphers_v13;
	int tls_check_peer;
	int tls_verify;
	SSL_CTX *ssl_ctx;
#endif
    } server;
    struct logsrvd_config_relay {
        struct address_list_container relays;
        struct timespec connect_timeout;
        struct timespec timeout;
	time_t retry_interval;
	char *relay_dir;
        bool tcp_keepalive;
	bool store_first;
#if defined(HAVE_OPENSSL)
	char *tls_key_path;
	char *tls_cert_path;
	char *tls_cacert_path;
	char *tls_dhparams_path;
	char *tls_ciphers_v12;
	char *tls_ciphers_v13;
	int tls_check_peer;
	int tls_verify;
	SSL_CTX *ssl_ctx;
#endif
    } relay;
    struct logsrvd_config_iolog {
	bool compress;
	bool flush;
	bool gid_set;
	bool log_passwords;
	uid_t uid;
	gid_t gid;
	mode_t mode;
	unsigned int maxseq;
	char *iolog_dir;
	char *iolog_file;
	void *passprompt_regex;
    } iolog;
    struct logsrvd_config_eventlog {
	int log_type;
        bool log_exit;
	enum eventlog_format log_format;
    } eventlog;
    struct logsrvd_config_syslog {
	unsigned int maxlen;
	int server_facility;
	int facility;
	int acceptpri;
	int rejectpri;
	int alertpri;
    } syslog;
    struct logsrvd_config_logfile {
	char *path;
	char *time_format;
	FILE *stream;
    } logfile;
} *logsrvd_config;

static bool logsrvd_warn_enable_stderr = true;

/* eventlog getters */
bool
logsrvd_conf_log_exit(void)
{
    return logsrvd_config->eventlog.log_exit;
}

/* iolog getters */
uid_t
logsrvd_conf_iolog_uid(void)
{
    return logsrvd_config->iolog.uid;
}

gid_t
logsrvd_conf_iolog_gid(void)
{
    return logsrvd_config->iolog.gid;
}

mode_t
logsrvd_conf_iolog_mode(void)
{
    return logsrvd_config->iolog.mode;
}

const char *
logsrvd_conf_iolog_dir(void)
{
    return logsrvd_config->iolog.iolog_dir;
}

const char *
logsrvd_conf_iolog_file(void)
{
    return logsrvd_config->iolog.iolog_file;
}

bool
logsrvd_conf_iolog_log_passwords(void)
{
    return logsrvd_config->iolog.log_passwords;
}

void *
logsrvd_conf_iolog_passprompt_regex(void)
{
    return logsrvd_config->iolog.passprompt_regex;
}

/* server getters */
struct server_address_list *
logsrvd_conf_server_listen_address(void)
{
    return &logsrvd_config->server.addresses.addrs;
}

bool
logsrvd_conf_server_tcp_keepalive(void)
{
    return logsrvd_config->server.tcp_keepalive;
}

const char *
logsrvd_conf_pid_file(void)
{
    return logsrvd_config->server.pid_file;
}

struct timespec *
logsrvd_conf_server_timeout(void)
{
    if (sudo_timespecisset(&logsrvd_config->server.timeout)) {
        return &logsrvd_config->server.timeout;
    }

    return NULL;
}

#if defined(HAVE_OPENSSL)
SSL_CTX *
logsrvd_server_tls_ctx(void)
{
    return logsrvd_config->server.ssl_ctx;
}

bool
logsrvd_conf_server_tls_check_peer(void)
{
    return logsrvd_config->server.tls_check_peer;
}
#endif

/* relay getters */
struct server_address_list *
logsrvd_conf_relay_address(void)
{
    return &logsrvd_config->relay.relays.addrs;
}

const char *
logsrvd_conf_relay_dir(void)
{
    return logsrvd_config->relay.relay_dir;
}

bool
logsrvd_conf_relay_store_first(void)
{
    return logsrvd_config->relay.store_first;
}

bool
logsrvd_conf_relay_tcp_keepalive(void)
{
    return logsrvd_config->relay.tcp_keepalive;
}

struct timespec *
logsrvd_conf_relay_timeout(void)
{
    if (sudo_timespecisset(&logsrvd_config->relay.timeout)) {
        return &logsrvd_config->relay.timeout;
    }

    return NULL;
}

struct timespec *
logsrvd_conf_relay_connect_timeout(void)
{
    if (sudo_timespecisset(&logsrvd_config->relay.connect_timeout)) {
        return &logsrvd_config->relay.connect_timeout;
    }

    return NULL;
}

time_t
logsrvd_conf_relay_retry_interval(void)
{
    return logsrvd_config->relay.retry_interval;
}

#if defined(HAVE_OPENSSL)
SSL_CTX *
logsrvd_relay_tls_ctx(void)
{
    if (logsrvd_config->relay.ssl_ctx != NULL)
	return logsrvd_config->relay.ssl_ctx;
    return logsrvd_config->server.ssl_ctx;
}

bool
logsrvd_conf_relay_tls_check_peer(void)
{
    if (logsrvd_config->relay.tls_check_peer != -1)
	return logsrvd_config->relay.tls_check_peer;
    return logsrvd_config->server.tls_check_peer;
}
#endif

/* I/O log callbacks */
static bool
cb_iolog_dir(struct logsrvd_config *config, const char *path, size_t offset)
{
    debug_decl(cb_iolog_dir, SUDO_DEBUG_UTIL);

    free(config->iolog.iolog_dir);
    if ((config->iolog.iolog_dir = strdup(path)) == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	debug_return_bool(false);
    }
    debug_return_bool(true);
}

static bool
cb_iolog_file(struct logsrvd_config *config, const char *path, size_t offset)
{
    debug_decl(cb_iolog_file, SUDO_DEBUG_UTIL);

    free(config->iolog.iolog_file);
    if ((config->iolog.iolog_file = strdup(path)) == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	debug_return_bool(false);
    }
    debug_return_bool(true);
}

static bool
cb_iolog_compress(struct logsrvd_config *config, const char *str, size_t offset)
{
    int val;
    debug_decl(cb_iolog_compress, SUDO_DEBUG_UTIL);

    if ((val = sudo_strtobool(str)) == -1)
	debug_return_bool(false);

    config->iolog.compress = val;
    debug_return_bool(true);
}

static bool
cb_iolog_log_passwords(struct logsrvd_config *config, const char *str, size_t offset)
{
    int val;
    debug_decl(cb_iolog_log_passwords, SUDO_DEBUG_UTIL);

    if ((val = sudo_strtobool(str)) == -1)
	debug_return_bool(false);

    config->iolog.log_passwords = val;
    debug_return_bool(true);
}

static bool
cb_iolog_flush(struct logsrvd_config *config, const char *str, size_t offset)
{
    int val;
    debug_decl(cb_iolog_flush, SUDO_DEBUG_UTIL);

    if ((val = sudo_strtobool(str)) == -1)
	debug_return_bool(false);

    config->iolog.flush = val;
    debug_return_bool(true);
}

static bool
cb_iolog_user(struct logsrvd_config *config, const char *user, size_t offset)
{
    struct passwd *pw;
    debug_decl(cb_iolog_user, SUDO_DEBUG_UTIL);

    if ((pw = getpwnam(user)) == NULL) {
	sudo_warnx(U_("unknown user %s"), user);
	debug_return_bool(false);
    }
    config->iolog.uid = pw->pw_uid;
    if (!config->iolog.gid_set)
	config->iolog.gid = pw->pw_gid;

    debug_return_bool(true);
}

static bool
cb_iolog_group(struct logsrvd_config *config, const char *group, size_t offset)
{
    struct group *gr;
    debug_decl(cb_iolog_group, SUDO_DEBUG_UTIL);

    if ((gr = getgrnam(group)) == NULL) {
	sudo_warnx(U_("unknown group %s"), group);
	debug_return_bool(false);
    }
    config->iolog.gid = gr->gr_gid;
    config->iolog.gid_set = true;

    debug_return_bool(true);
}

static bool
cb_iolog_mode(struct logsrvd_config *config, const char *str, size_t offset)
{
    const char *errstr;
    mode_t mode;
    debug_decl(cb_iolog_mode, SUDO_DEBUG_UTIL);

    mode = sudo_strtomode(str, &errstr);
    if (errstr != NULL) {
	sudo_warnx(U_("unable to parse iolog mode %s"), str);
	debug_return_bool(false);
    }
    config->iolog.mode = mode;
    debug_return_bool(true);
}

static bool
cb_iolog_maxseq(struct logsrvd_config *config, const char *str, size_t offset)
{
    const char *errstr;
    unsigned int value;
    debug_decl(cb_iolog_maxseq, SUDO_DEBUG_UTIL);

    value = (unsigned int)sudo_strtonum(str, 0, SESSID_MAX, &errstr);
    if (errstr != NULL) {
        if (errno != ERANGE) {
	    sudo_warnx(U_("invalid value for %s: %s"), "maxseq", errstr);
            debug_return_bool(false);
        }
        /* Out of range, clamp to SESSID_MAX as documented. */
        value = SESSID_MAX;
    }
    config->iolog.maxseq = value;
    debug_return_bool(true);
}

static bool
cb_iolog_passprompt_regex(struct logsrvd_config *config, const char *str, size_t offset)
{
    debug_decl(cb_iolog_passprompt_regex, SUDO_DEBUG_UTIL);

    if (config->iolog.passprompt_regex == NULL) {
	/* Lazy alloc of the passprompt regex handle. */
	config->iolog.passprompt_regex = iolog_pwfilt_alloc();
	if (config->iolog.passprompt_regex == NULL)
	    debug_return_bool(false);
    }
    debug_return_bool(iolog_pwfilt_add(config->iolog.passprompt_regex, str));
}

/* Server callbacks */
static bool
append_address(struct server_address_list *addresses, const char *str,
    bool allow_wildcard)
{
    struct addrinfo hints, *res, *res0 = NULL;
    char *sa_str = NULL, *sa_host = NULL;
    char *copy, *host, *port;
    bool tls, ret = false;
    int error;
    debug_decl(append_address, SUDO_DEBUG_UTIL);

    if ((copy = strdup(str)) == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	debug_return_bool(false);
    }

    /* Parse host[:port] */
    if (!iolog_parse_host_port(copy, &host, &port, &tls, DEFAULT_PORT,
	    DEFAULT_PORT_TLS))
	goto done;
    if (host[0] == '*' && host[1] == '\0') {
	if (!allow_wildcard)
	    goto done;
	host = NULL;
    }

#if !defined(HAVE_OPENSSL)
    if (tls) {
	sudo_warnx("%s", U_("TLS not supported"));
	goto done;
    }
#endif

    /* Only make a single copy of the string + host for all addresses. */
    if ((sa_str = sudo_rcstr_dup(str)) == NULL)	{
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	goto done;
    }
    if (host != NULL && (sa_host = sudo_rcstr_dup(host)) == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	goto done;
    }

    /* Resolve host (and port if it is a service). */
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    error = getaddrinfo(host, port, &hints, &res0);
    if (error != 0) {
	sudo_gai_warn(error, U_("%s:%s"), host ? host : "*", port);
	goto done;
    }
    for (res = res0; res != NULL; res = res->ai_next) {
	struct server_address *addr;

	if ((addr = malloc(sizeof(*addr))) == NULL) {
	    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	    goto done;
	}
	addr->sa_str = sudo_rcstr_addref(sa_str);
	addr->sa_host = sudo_rcstr_addref(sa_host);

	memcpy(&addr->sa_un, res->ai_addr, res->ai_addrlen);
	addr->sa_size = res->ai_addrlen;
	addr->tls = tls;
	TAILQ_INSERT_TAIL(addresses, addr, entries);
    }

    ret = true;
done:
    sudo_rcstr_delref(sa_str);
    sudo_rcstr_delref(sa_host);
    if (res0 != NULL)
	freeaddrinfo(res0);
    free(copy);
    debug_return_bool(ret);
}

static bool
cb_server_listen_address(struct logsrvd_config *config, const char *str, size_t offset)
{
    return append_address(&config->server.addresses.addrs, str, true);
}

static bool
cb_server_timeout(struct logsrvd_config *config, const char *str, size_t offset)
{
    time_t timeout;
    const char *errstr;
    debug_decl(cb_server_timeout, SUDO_DEBUG_UTIL);

    timeout = (time_t)sudo_strtonum(str, 0, TIME_T_MAX, &errstr);
    if (errstr != NULL)
	debug_return_bool(false);

    config->server.timeout.tv_sec = timeout;

    debug_return_bool(true);
}

static bool
cb_server_keepalive(struct logsrvd_config *config, const char *str, size_t offset)
{
    int val;
    debug_decl(cb_server_keepalive, SUDO_DEBUG_UTIL);

    if ((val = sudo_strtobool(str)) == -1)
	debug_return_bool(false);

    config->server.tcp_keepalive = val;
    debug_return_bool(true);
}

static bool
cb_server_pid_file(struct logsrvd_config *config, const char *str, size_t offset)
{
    char *copy = NULL;
    debug_decl(cb_server_pid_file, SUDO_DEBUG_UTIL);

    /* An empty value means to disable the pid file. */
    if (*str != '\0') {
	if (*str != '/') {
	    sudo_warnx(U_("%s: not a fully qualified path"), str);
	    debug_return_bool(false);
	}
	if ((copy = strdup(str)) == NULL) {
	    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	    debug_return_bool(false);
	}
    }

    free(config->server.pid_file);
    config->server.pid_file = copy;

    debug_return_bool(true);
}

static bool
cb_server_log(struct logsrvd_config *config, const char *str, size_t offset)
{
    char *copy = NULL;
    enum server_log_type log_type = SERVER_LOG_NONE;
    debug_decl(cb_server_log, SUDO_DEBUG_UTIL);

    /* An empty value means to disable the server log. */
    if (*str != '\0') {
	if (*str == '/') {
	    log_type = SERVER_LOG_FILE;
	    if ((copy = strdup(str)) == NULL) {
		sudo_warnx(U_("%s: %s"), __func__,
		    U_("unable to allocate memory"));
		debug_return_bool(false);
	    }
	} else if (strcmp(str, "stderr") == 0) {
	    log_type = SERVER_LOG_STDERR;
	} else if (strcmp(str, "syslog") == 0) {
	    log_type = SERVER_LOG_SYSLOG;
	} else {
	    debug_return_bool(false);
	}
    }

    free(config->server.log_file);
    config->server.log_file = copy;
    config->server.log_type = log_type;

    debug_return_bool(true);
}

#if defined(HAVE_OPENSSL)
static bool
cb_tls_key(struct logsrvd_config *config, const char *path, size_t offset)
{
    char **p = (char **)((char *)config + offset);
    debug_decl(cb_tls_key, SUDO_DEBUG_UTIL);

    free(*p);
    if ((*p = strdup(path)) == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
        debug_return_bool(false);
    }
    debug_return_bool(true);
}

static bool
cb_tls_cacert(struct logsrvd_config *config, const char *path, size_t offset)
{
    char **p = (char **)((char *)config + offset);
    debug_decl(cb_tls_cacert, SUDO_DEBUG_UTIL);

    free(*p);
    if ((*p = strdup(path)) == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
        debug_return_bool(false);
    }
    debug_return_bool(true);
}

static bool
cb_tls_cert(struct logsrvd_config *config, const char *path, size_t offset)
{
    char **p = (char **)((char *)config + offset);
    debug_decl(cb_tls_cert, SUDO_DEBUG_UTIL);

    free(*p);
    if ((*p = strdup(path)) == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
        debug_return_bool(false);
    }
    debug_return_bool(true);
}

static bool
cb_tls_dhparams(struct logsrvd_config *config, const char *path, size_t offset)
{
    char **p = (char **)((char *)config + offset);
    debug_decl(cb_tls_dhparams, SUDO_DEBUG_UTIL);

    free(*p);
    if ((*p = strdup(path)) == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
        debug_return_bool(false);
    }
    debug_return_bool(true);
}

static bool
cb_tls_ciphers12(struct logsrvd_config *config, const char *str, size_t offset)
{
    char **p = (char **)((char *)config + offset);
    debug_decl(cb_tls_ciphers12, SUDO_DEBUG_UTIL);

    free(*p);
    if ((*p = strdup(str)) == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
        debug_return_bool(false);
    }
    debug_return_bool(true);
}

static bool
cb_tls_ciphers13(struct logsrvd_config *config, const char *str, size_t offset)
{
    char **p = (char **)((char *)config + offset);
    debug_decl(cb_tls_ciphers13, SUDO_DEBUG_UTIL);

    free(*p);
    if ((*p = strdup(str)) == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
        debug_return_bool(false);
    }
    debug_return_bool(true);
}

static bool
cb_tls_verify(struct logsrvd_config *config, const char *str, size_t offset)
{
    int *p = (int *)((char *)config + offset);
    int val;
    debug_decl(cb_tls_verify, SUDO_DEBUG_UTIL);

    if ((val = sudo_strtobool(str)) == -1)
	debug_return_bool(false);

    *p = val;
    debug_return_bool(true);
}

static bool
cb_tls_checkpeer(struct logsrvd_config *config, const char *str, size_t offset)
{
    int *p = (int *)((char *)config + offset);
    int val;
    debug_decl(cb_tls_checkpeer, SUDO_DEBUG_UTIL);

    if ((val = sudo_strtobool(str)) == -1)
	debug_return_bool(false);

    *p = val;
    debug_return_bool(true);
}
#endif

/* relay callbacks */
static bool
cb_relay_host(struct logsrvd_config *config, const char *str, size_t offset)
{
    return append_address(&config->relay.relays.addrs, str, false);
}

static bool
cb_relay_timeout(struct logsrvd_config *config, const char *str, size_t offset)
{
    time_t timeout;
    const char *errstr;
    debug_decl(cb_relay_timeout, SUDO_DEBUG_UTIL);

    timeout = (time_t)sudo_strtonum(str, 0, TIME_T_MAX, &errstr);
    if (errstr != NULL)
	debug_return_bool(false);

    config->server.timeout.tv_sec = timeout;

    debug_return_bool(true);
}

static bool
cb_relay_connect_timeout(struct logsrvd_config *config, const char *str, size_t offset)
{
    time_t timeout;
    const char *errstr;
    debug_decl(cb_relay_connect_timeout, SUDO_DEBUG_UTIL);

    timeout = (time_t)sudo_strtonum(str, 0, TIME_T_MAX, &errstr);
    if (errstr != NULL)
	debug_return_bool(false);

    config->relay.connect_timeout.tv_sec = timeout;

    debug_return_bool(true);
}

static bool
cb_relay_dir(struct logsrvd_config *config, const char *str, size_t offset)
{
    char *copy = NULL;
    debug_decl(cb_relay_dir, SUDO_DEBUG_UTIL);

    if ((copy = strdup(str)) == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	debug_return_bool(false);
    }

    free(config->relay.relay_dir);
    config->relay.relay_dir = copy;

    debug_return_bool(true);
}

static bool
cb_retry_interval(struct logsrvd_config *config, const char *str, size_t offset)
{
    time_t interval;
    const char *errstr;
    debug_decl(cb_retry_interval, SUDO_DEBUG_UTIL);

    interval = (time_t)sudo_strtonum(str, 0, TIME_T_MAX, &errstr);
    if (errstr != NULL)
	debug_return_bool(false);

    config->relay.retry_interval = interval;

    debug_return_bool(true);
}

static bool
cb_relay_store_first(struct logsrvd_config *config, const char *str, size_t offset)
{
    int val;
    debug_decl(cb_relay_store_first, SUDO_DEBUG_UTIL);

    if ((val = sudo_strtobool(str)) == -1)
	debug_return_bool(false);

    config->relay.store_first = val;
    debug_return_bool(true);
}

static bool
cb_relay_keepalive(struct logsrvd_config *config, const char *str, size_t offset)
{
    int val;
    debug_decl(cb_relay_keepalive, SUDO_DEBUG_UTIL);

    if ((val = sudo_strtobool(str)) == -1)
	debug_return_bool(false);

    config->relay.tcp_keepalive = val;
    debug_return_bool(true);
}

/* eventlog callbacks */
static bool
cb_eventlog_type(struct logsrvd_config *config, const char *str, size_t offset)
{
    debug_decl(cb_eventlog_type, SUDO_DEBUG_UTIL);

    if (strcmp(str, "none") == 0)
	config->eventlog.log_type = EVLOG_NONE;
    else if (strcmp(str, "syslog") == 0)
	config->eventlog.log_type = EVLOG_SYSLOG;
    else if (strcmp(str, "logfile") == 0)
	config->eventlog.log_type = EVLOG_FILE;
    else
	debug_return_bool(false);

    debug_return_bool(true);
}

static bool
cb_eventlog_format(struct logsrvd_config *config, const char *str, size_t offset)
{
    debug_decl(cb_eventlog_format, SUDO_DEBUG_UTIL);

    if (strcmp(str, "json") == 0)
	config->eventlog.log_format = EVLOG_JSON;
    else if (strcmp(str, "sudo") == 0)
	config->eventlog.log_format = EVLOG_SUDO;
    else
	debug_return_bool(false);

    debug_return_bool(true);
}

static bool
cb_eventlog_exit(struct logsrvd_config *config, const char *str, size_t offset)
{
    int val;
    debug_decl(cb_eventlog_exit, SUDO_DEBUG_UTIL);

    if ((val = sudo_strtobool(str)) == -1)
	debug_return_bool(false);

    config->eventlog.log_exit = val;
    debug_return_bool(true);
}

/* syslog callbacks */
static bool
cb_syslog_maxlen(struct logsrvd_config *config, const char *str, size_t offset)
{
    unsigned int maxlen;
    const char *errstr;
    debug_decl(cb_syslog_maxlen, SUDO_DEBUG_UTIL);

    maxlen = (unsigned int)sudo_strtonum(str, 1, UINT_MAX, &errstr);
    if (errstr != NULL)
	debug_return_bool(false);

    config->syslog.maxlen = maxlen;

    debug_return_bool(true);
}

static bool
cb_syslog_server_facility(struct logsrvd_config *config, const char *str, size_t offset)
{
    int logfac;
    debug_decl(cb_syslog_server_facility, SUDO_DEBUG_UTIL);

    if (!sudo_str2logfac(str, &logfac)) {
	sudo_warnx(U_("unknown syslog facility %s"), str);
	debug_return_bool(false);
    }

    config->syslog.server_facility = logfac;

    debug_return_bool(true);
}

static bool
cb_syslog_facility(struct logsrvd_config *config, const char *str, size_t offset)
{
    int logfac;
    debug_decl(cb_syslog_facility, SUDO_DEBUG_UTIL);

    if (!sudo_str2logfac(str, &logfac)) {
	sudo_warnx(U_("unknown syslog facility %s"), str);
	debug_return_bool(false);
    }

    config->syslog.facility = logfac;

    debug_return_bool(true);
}

static bool
cb_syslog_acceptpri(struct logsrvd_config *config, const char *str, size_t offset)
{
    int logpri;
    debug_decl(cb_syslog_acceptpri, SUDO_DEBUG_UTIL);

    if (!sudo_str2logpri(str, &logpri)) {
	sudo_warnx(U_("unknown syslog priority %s"), str);
	debug_return_bool(false);
    }

    config->syslog.acceptpri = logpri;

    debug_return_bool(true);
}

static bool
cb_syslog_rejectpri(struct logsrvd_config *config, const char *str, size_t offset)
{
    int logpri;
    debug_decl(cb_syslog_rejectpri, SUDO_DEBUG_UTIL);

    if (!sudo_str2logpri(str, &logpri)) {
	sudo_warnx(U_("unknown syslog priority %s"), str);
	debug_return_bool(false);
    }

    config->syslog.rejectpri = logpri;

    debug_return_bool(true);
}

static bool
cb_syslog_alertpri(struct logsrvd_config *config, const char *str, size_t offset)
{
    int logpri;
    debug_decl(cb_syslog_alertpri, SUDO_DEBUG_UTIL);

    if (!sudo_str2logpri(str, &logpri)) {
	sudo_warnx(U_("unknown syslog priority %s"), str);
	debug_return_bool(false);
    }

    config->syslog.alertpri = logpri;

    debug_return_bool(true);
}

/* logfile callbacks */
static bool
cb_logfile_path(struct logsrvd_config *config, const char *str, size_t offset)
{
    char *copy = NULL;
    debug_decl(cb_logfile_path, SUDO_DEBUG_UTIL);

    if (*str != '/') {
	sudo_warnx(U_("%s: not a fully qualified path"), str);
	debug_return_bool(false);
    }
    if ((copy = strdup(str)) == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	debug_return_bool(false);
    }

    free(config->logfile.path);
    config->logfile.path = copy;

    debug_return_bool(true);
}

static bool
cb_logfile_time_format(struct logsrvd_config *config, const char *str, size_t offset)
{
    char *copy = NULL;
    debug_decl(cb_logfile_time_format, SUDO_DEBUG_UTIL);

    if ((copy = strdup(str)) == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	debug_return_bool(false);
    }

    free(config->logfile.time_format);
    config->logfile.time_format = copy;

    debug_return_bool(true);
}

void
address_list_addref(struct server_address_list *al)
{
    struct address_list_container *container =
	__containerof(al, struct address_list_container, addrs);
    container->refcnt++;
}

void
address_list_delref(struct server_address_list *al)
{
    struct address_list_container *container =
	__containerof(al, struct address_list_container, addrs);
    if (--container->refcnt == 0) {
	struct server_address *addr;
	while ((addr = TAILQ_FIRST(al))) {
	    TAILQ_REMOVE(al, addr, entries);
	    sudo_rcstr_delref(addr->sa_str);
	    sudo_rcstr_delref(addr->sa_host);
	    free(addr);
	}
    }
}

static struct logsrvd_config_entry server_conf_entries[] = {
    { "listen_address", cb_server_listen_address },
    { "timeout", cb_server_timeout },
    { "tcp_keepalive", cb_server_keepalive },
    { "pid_file", cb_server_pid_file },
    { "server_log", cb_server_log },
#if defined(HAVE_OPENSSL)
    { "tls_key", cb_tls_key, offsetof(struct logsrvd_config, server.tls_key_path) },
    { "tls_cacert", cb_tls_cacert, offsetof(struct logsrvd_config, server.tls_cacert_path) },
    { "tls_cert", cb_tls_cert, offsetof(struct logsrvd_config, server.tls_cert_path) },
    { "tls_dhparams", cb_tls_dhparams, offsetof(struct logsrvd_config, server.tls_dhparams_path) },
    { "tls_ciphers_v12", cb_tls_ciphers12, offsetof(struct logsrvd_config, server.tls_ciphers_v12) },
    { "tls_ciphers_v13", cb_tls_ciphers13, offsetof(struct logsrvd_config, server.tls_ciphers_v13) },
    { "tls_checkpeer", cb_tls_checkpeer, offsetof(struct logsrvd_config, server.tls_check_peer) },
    { "tls_verify", cb_tls_verify, offsetof(struct logsrvd_config, server.tls_verify) },
#endif
    { NULL }
};

static struct logsrvd_config_entry relay_conf_entries[] = {
    { "relay_host", cb_relay_host },
    { "timeout", cb_relay_timeout },
    { "connect_timeout", cb_relay_connect_timeout },
    { "relay_dir", cb_relay_dir },
    { "retry_interval", cb_retry_interval },
    { "store_first", cb_relay_store_first },
    { "tcp_keepalive", cb_relay_keepalive },
#if defined(HAVE_OPENSSL)
    { "tls_key", cb_tls_key, offsetof(struct logsrvd_config, relay.tls_key_path) },
    { "tls_cacert", cb_tls_cacert, offsetof(struct logsrvd_config, relay.tls_cacert_path) },
    { "tls_cert", cb_tls_cert, offsetof(struct logsrvd_config, relay.tls_cert_path) },
    { "tls_dhparams", cb_tls_dhparams, offsetof(struct logsrvd_config, relay.tls_dhparams_path) },
    { "tls_ciphers_v12", cb_tls_ciphers12, offsetof(struct logsrvd_config, relay.tls_ciphers_v12) },
    { "tls_ciphers_v13", cb_tls_ciphers13, offsetof(struct logsrvd_config, relay.tls_ciphers_v13) },
    { "tls_checkpeer", cb_tls_checkpeer, offsetof(struct logsrvd_config, relay.tls_check_peer) },
    { "tls_verify", cb_tls_verify, offsetof(struct logsrvd_config, relay.tls_verify) },
#endif
    { NULL }
};

static struct logsrvd_config_entry iolog_conf_entries[] = {
    { "iolog_dir", cb_iolog_dir },
    { "iolog_file", cb_iolog_file },
    { "iolog_flush", cb_iolog_flush },
    { "iolog_compress", cb_iolog_compress },
    { "iolog_user", cb_iolog_user },
    { "iolog_group", cb_iolog_group },
    { "iolog_mode", cb_iolog_mode },
    { "log_passwords", cb_iolog_log_passwords },
    { "maxseq", cb_iolog_maxseq },
    { "passprompt_regex", cb_iolog_passprompt_regex },
    { NULL }
};

static struct logsrvd_config_entry eventlog_conf_entries[] = {
    { "log_type", cb_eventlog_type },
    { "log_format", cb_eventlog_format },
    { "log_exit", cb_eventlog_exit },
    { NULL }
};

static struct logsrvd_config_entry syslog_conf_entries[] = {
    { "maxlen", cb_syslog_maxlen },
    { "server_facility", cb_syslog_server_facility },
    { "facility", cb_syslog_facility },
    { "reject_priority", cb_syslog_rejectpri },
    { "accept_priority", cb_syslog_acceptpri },
    { "alert_priority", cb_syslog_alertpri },
    { NULL }
};

static struct logsrvd_config_entry logfile_conf_entries[] = {
    { "path", cb_logfile_path },
    { "time_format", cb_logfile_time_format },
    { NULL }
};

static struct logsrvd_config_section logsrvd_config_sections[] = {
    { "server", server_conf_entries },
    { "relay", relay_conf_entries },
    { "iolog", iolog_conf_entries },
    { "eventlog", eventlog_conf_entries },
    { "syslog", syslog_conf_entries },
    { "logfile", logfile_conf_entries },
    { NULL }
};

static bool
logsrvd_conf_parse(struct logsrvd_config *config, FILE *fp, const char *path)
{
    struct logsrvd_config_section *conf_section = NULL;
    unsigned int lineno = 0;
    size_t linesize = 0;
    char *line = NULL;
    bool ret = false;
    debug_decl(logsrvd_conf_parse, SUDO_DEBUG_UTIL);

    while (sudo_parseln(&line, &linesize, &lineno, fp, 0) != -1) {
	struct logsrvd_config_entry *entry;
	char *ep, *val;

	/* Skip blank, comment or invalid lines. */
	if (*line == '\0' || *line == ';')
	    continue;

	/* New section */
	if (line[0] == '[') {
	    char *cp, *section_name = line + 1;

	    if ((ep = strchr(section_name, ']')) == NULL) {
		sudo_warnx(U_("%s:%d unmatched '[': %s"),
		    path, lineno, line);
		goto done;
	    }
	    for (cp = ep + 1; *cp != '\0'; cp++) {
		if (!isspace((unsigned char)*cp)) {
		    sudo_warnx(U_("%s:%d garbage after ']': %s"),
			path, lineno, line);
		    goto done;
		}
	    }
	    *ep = '\0';
	    for (conf_section = logsrvd_config_sections; conf_section->name != NULL;
		    conf_section++) {
		if (strcasecmp(section_name, conf_section->name) == 0)
		    break;
	    }
	    if (conf_section->name == NULL) {
		sudo_warnx(U_("%s:%d invalid config section: %s"),
		    path, lineno, section_name);
		goto done;
	    }
	    continue;
	}

	if ((ep = strchr(line, '=')) == NULL) {
	    sudo_warnx(U_("%s:%d invalid configuration line: %s"),
		path, lineno, line);
	    goto done;
	}

	if (conf_section == NULL) {
	    sudo_warnx(U_("%s:%d expected section name: %s"),
		path, lineno, line);
	    goto done;
	}

	val = ep + 1;
	while (isspace((unsigned char)*val))
	    val++;
	while (ep > line && isspace((unsigned char)ep[-1]))
	    ep--;
	*ep = '\0';
	for (entry = conf_section->entries; entry->conf_str != NULL; entry++) {
	    if (strcasecmp(line, entry->conf_str) == 0) {
		if (!entry->setter(config, val, entry->offset)) {
		    sudo_warnx(U_("invalid value for %s: %s"),
			entry->conf_str, val);
		    goto done;
		}
		break;
	    }
	}
	if (entry->conf_str == NULL) {
	    sudo_warnx(U_("%s:%d [%s] illegal key: %s"), path, lineno,
		conf_section->name, line);
	    goto done;
	}
    }
    ret = true;

done:
    free(line);
    debug_return_bool(ret);
}

static FILE *
logsrvd_open_log_file(const char *path, int flags)
{
    mode_t oldmask;
    FILE *fp = NULL;
    const char *omode;
    int fd;
    debug_decl(logsrvd_open_log_file, SUDO_DEBUG_UTIL);

    if (ISSET(flags, O_APPEND)) {
	omode = "a";
    } else {
	omode = "w";
    }
    oldmask = umask(S_IRWXG|S_IRWXO);
    fd = open(path, flags, S_IRUSR|S_IWUSR);
    (void)umask(oldmask);
    if (fd == -1 || (fp = fdopen(fd, omode)) == NULL) {
	sudo_warn(U_("unable to open log file %s"), path);
	if (fd != -1)
	    close(fd);
    }

    debug_return_ptr(fp);
}

static FILE *
logsrvd_open_eventlog(struct logsrvd_config *config)
{
    int flags;
    debug_decl(logsrvd_open_eventlog, SUDO_DEBUG_UTIL);

    /* Cannot append to a JSON file. */
    if (config->eventlog.log_format == EVLOG_JSON) {
	flags = O_RDWR|O_CREAT;
    } else {
	flags = O_WRONLY|O_APPEND|O_CREAT;
    }
    debug_return_ptr(logsrvd_open_log_file(config->logfile.path, flags));
}

static FILE *
logsrvd_stub_open_log(int type, const char *logfile)
{
    /* Actual open already done by logsrvd_open_eventlog() */
    return logsrvd_config->logfile.stream;
}

static void
logsrvd_stub_close_log(int type, FILE *fp)
{
    return;
}

/* Set eventlog configuration settings from logsrvd config. */
static void
logsrvd_conf_eventlog_setconf(struct logsrvd_config *config)
{
    debug_decl(logsrvd_conf_eventlog_setconf, SUDO_DEBUG_UTIL);

    eventlog_set_type(config->eventlog.log_type);
    eventlog_set_format(config->eventlog.log_format);
    eventlog_set_syslog_acceptpri(config->syslog.acceptpri); 
    eventlog_set_syslog_rejectpri(config->syslog.rejectpri); 
    eventlog_set_syslog_alertpri(config->syslog.alertpri); 
    eventlog_set_syslog_maxlen(config->syslog.maxlen); 
    eventlog_set_logpath(config->logfile.path);
    eventlog_set_time_fmt(config->logfile.time_format);
    eventlog_set_open_log(logsrvd_stub_open_log);
    eventlog_set_close_log(logsrvd_stub_close_log);

    debug_return;
}

/* Set I/O log configuration settings from logsrvd config. */
static void
logsrvd_conf_iolog_setconf(struct logsrvd_config *config)
{
    debug_decl(logsrvd_conf_iolog_setconf, SUDO_DEBUG_UTIL);

    iolog_set_defaults();
    iolog_set_compress(config->iolog.compress);
    iolog_set_flush(config->iolog.flush);
    iolog_set_owner(config->iolog.uid, config->iolog.gid);
    iolog_set_mode(config->iolog.mode);
    iolog_set_maxseq(config->iolog.maxseq);

    debug_return;
}

/*
 * Conversation function for use by sudo_warn/sudo_fatal.
 * Logs to stdout/stderr.
 */
static int
logsrvd_conv_stderr(int num_msgs, const struct sudo_conv_message msgs[],
    struct sudo_conv_reply replies[], struct sudo_conv_callback *callback)
{
    int i;
    debug_decl(logsrvd_conv_stderr, SUDO_DEBUG_UTIL);

    for (i = 0; i < num_msgs; i++) {
	if (fputs(msgs[i].msg, stderr) == EOF)
	    debug_return_int(-1);
    }

    debug_return_int(0);
}

/*
 * Conversation function for use by sudo_warn/sudo_fatal.
 * Acts as a no-op log sink.
 */
static int
logsrvd_conv_none(int num_msgs, const struct sudo_conv_message msgs[],
    struct sudo_conv_reply replies[], struct sudo_conv_callback *callback)
{
    /* Also write to stderr if still in the foreground. */
    if (logsrvd_warn_enable_stderr) {
	(void)logsrvd_conv_stderr(num_msgs, msgs, replies, callback);
    }

    return 0;
}

/*
 * Conversation function for use by sudo_warn/sudo_fatal.
 * Logs to syslog.
 */
static int
logsrvd_conv_syslog(int num_msgs, const struct sudo_conv_message msgs[],
    struct sudo_conv_reply replies[], struct sudo_conv_callback *callback)
{
    char *buf = NULL, *cp = NULL;
    const char *progname;
    size_t proglen, bufsize = 0;
    int i;
    debug_decl(logsrvd_conv_syslog, SUDO_DEBUG_UTIL);

    if (logsrvd_config == NULL) {
	debug_return_int(logsrvd_conv_stderr(num_msgs, msgs, replies, callback));
    }

    /* Also write to stderr if still in the foreground. */
    if (logsrvd_warn_enable_stderr) {
	(void)logsrvd_conv_stderr(num_msgs, msgs, replies, callback);
    }

    /*
     * Concat messages into a flag string that we can syslog.
     */
    progname = getprogname();
    proglen = strlen(progname);
    for (i = 0; i < num_msgs; i++) {
	const char *msg = msgs[i].msg;
	size_t len = strlen(msg);
	size_t used = (size_t)(cp - buf);

	/* Strip leading "sudo_logsrvd: " prefix. */
	if (strncmp(msg, progname, proglen) == 0) {
	    msg += proglen;
	    len -= proglen;
	    if (len == 0) {
		/* Skip over ": " string that follows program name. */
		if (i + 1 < num_msgs && strcmp(msgs[i + 1].msg, ": ") == 0) {
		    i++;
		    continue;
		}
	    } else if (msg[0] == ':' && msg[1] == ' ') {
		/* Handle "progname: " */
		msg += 2;
		len -= 2;
	    }
	}

	/* Strip off trailing newlines. */
	while (len > 1 && msg[len - 1] == '\n')
	    len--;
	if (len == 0)
	    continue;

	if (len >= bufsize - used) {
	    bufsize += 1024;
	    char *tmp = realloc(buf, bufsize);
	    if (tmp == NULL) {
		sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
		free(buf);
		debug_return_int(-1);
	    }
	    buf = tmp;
	    cp = tmp + used;
	}
	memcpy(cp, msg, len);
	cp[len] = '\0';
	cp += len;
    }
    if (buf != NULL) {
	openlog(progname, 0, logsrvd_config->syslog.server_facility);
	syslog(LOG_ERR, "%s", buf);
	free(buf);

	/* Restore old syslog settings. */
	if (logsrvd_config->eventlog.log_type == EVLOG_SYSLOG)
	    openlog("sudo", 0, logsrvd_config->syslog.facility);
    }

    debug_return_int(0);
}

/*
 * Conversation function for use by sudo_warn/sudo_fatal.
 * Logs to an already-open log file.
 */
static int
logsrvd_conv_logfile(int num_msgs, const struct sudo_conv_message msgs[],
    struct sudo_conv_reply replies[], struct sudo_conv_callback *callback)
{
    const char *progname;
    size_t proglen;
    int i;
    debug_decl(logsrvd_conv_logfile, SUDO_DEBUG_UTIL);

    if (logsrvd_config == NULL) {
	debug_return_int(logsrvd_conv_stderr(num_msgs, msgs, replies, callback));
    }

    /* Also write to stderr if still in the foreground. */
    if (logsrvd_warn_enable_stderr) {
	(void)logsrvd_conv_stderr(num_msgs, msgs, replies, callback);
    }

    if (logsrvd_config->server.log_stream == NULL) {
	errno = EBADF;
	debug_return_int(-1);
    }

    progname = getprogname();
    proglen = strlen(progname);
    for (i = 0; i < num_msgs; i++) {
	const char *msg = msgs[i].msg;
	size_t len = strlen(msg);

	/* Strip leading "sudo_logsrvd: " prefix. */
	if (strncmp(msg, progname, proglen) == 0) {
	    msg += proglen;
	    len -= proglen;
	    if (len == 0) {
		/* Skip over ": " string that follows program name. */
		if (i + 1 < num_msgs && strcmp(msgs[i + 1].msg, ": ") == 0) {
		    i++;
		    continue;
		}
	    } else if (msg[0] == ':' && msg[1] == ' ') {
		/* Handle "progname: " */
		msg += 2;
		len -= 2;
	    }
	}

	if (fwrite(msg, len, 1, logsrvd_config->server.log_stream) != 1)
	    debug_return_int(-1);
    }

    debug_return_int(0);
}

/* Free the specified struct logsrvd_config and its contents. */
static void
logsrvd_conf_free(struct logsrvd_config *config)
{
    debug_decl(logsrvd_conf_free, SUDO_DEBUG_UTIL);

    if (config == NULL)
	debug_return;

    /* struct logsrvd_config_server */
    address_list_delref(&config->server.addresses.addrs);
    free(config->server.pid_file);
    free(config->server.log_file);
    if (config->server.log_stream != NULL)
	fclose(config->server.log_stream);
#if defined(HAVE_OPENSSL)
    free(config->server.tls_key_path);
    free(config->server.tls_cert_path);
    free(config->server.tls_cacert_path);
    free(config->server.tls_dhparams_path);
    free(config->server.tls_ciphers_v12);
    free(config->server.tls_ciphers_v13);

    if (config->server.ssl_ctx != NULL)
	SSL_CTX_free(config->server.ssl_ctx);
#endif

    /* struct logsrvd_config_relay */
    address_list_delref(&config->relay.relays.addrs);
    free(config->relay.relay_dir);
#if defined(HAVE_OPENSSL)
    free(config->relay.tls_key_path);
    free(config->relay.tls_cert_path);
    free(config->relay.tls_cacert_path);
    free(config->relay.tls_dhparams_path);
    free(config->relay.tls_ciphers_v12);
    free(config->relay.tls_ciphers_v13);

    if (config->relay.ssl_ctx != NULL)
	SSL_CTX_free(config->relay.ssl_ctx);
#endif

    /* struct logsrvd_config_iolog */
    free(config->iolog.iolog_dir);
    free(config->iolog.iolog_file);
    iolog_pwfilt_free(config->iolog.passprompt_regex);

    /* struct logsrvd_config_logfile */
    free(config->logfile.path);
    free(config->logfile.time_format);
    if (config->logfile.stream != NULL)
	fclose(config->logfile.stream);

    free(config);

    debug_return;
}

/* Allocate a new struct logsrvd_config and set default values. */
static struct logsrvd_config *
logsrvd_conf_alloc(void)
{
    struct logsrvd_config *config;
    debug_decl(logsrvd_conf_alloc, SUDO_DEBUG_UTIL);

    if ((config = calloc(1, sizeof(*config))) == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	debug_return_ptr(NULL);
    }

    /* Relay defaults */
    TAILQ_INIT(&config->relay.relays.addrs);
    config->relay.relays.refcnt = 1;
    config->relay.timeout.tv_sec = DEFAULT_SOCKET_TIMEOUT_SEC;
    config->relay.connect_timeout.tv_sec = DEFAULT_SOCKET_TIMEOUT_SEC;
    config->relay.tcp_keepalive = true;
    config->relay.retry_interval = 30;
    if (!cb_relay_dir(config, _PATH_SUDO_RELAY_DIR, 0))
	goto bad;
#if defined(HAVE_OPENSSL)
    config->relay.tls_verify = -1;
    config->relay.tls_check_peer = -1;
#endif

    /* Server defaults */
    TAILQ_INIT(&config->server.addresses.addrs);
    config->server.addresses.refcnt = 1;
    config->server.timeout.tv_sec = DEFAULT_SOCKET_TIMEOUT_SEC;
    config->server.tcp_keepalive = true;
    config->server.log_type = SERVER_LOG_SYSLOG;
    config->server.pid_file = strdup(_PATH_SUDO_LOGSRVD_PID);
    if (config->server.pid_file == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	goto bad;
    }

#if defined(HAVE_OPENSSL)
    /*
     * Only set default CA and cert paths if the files actually exist.
     * This ensures we don't enable TLS by default when it is not configured.
     */
    if (access(DEFAULT_CA_CERT_PATH, R_OK) == 0) {
	config->server.tls_cacert_path = strdup(DEFAULT_CA_CERT_PATH);
	if (config->server.tls_cacert_path == NULL) {
	    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	    goto bad;
	}
    }
    if (access(DEFAULT_SERVER_CERT_PATH, R_OK) == 0) {
	config->server.tls_cert_path = strdup(DEFAULT_SERVER_CERT_PATH);
	if (config->server.tls_cert_path == NULL) {
	    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	    goto bad;
	}
    }
    config->server.tls_key_path = strdup(DEFAULT_SERVER_KEY_PATH);
    if (config->server.tls_key_path == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	goto bad;
    }
    config->server.tls_verify = true;
    config->server.tls_check_peer = false;
#endif

    /* I/O log defaults */
    config->iolog.compress = false;
    config->iolog.flush = true;
    config->iolog.mode = S_IRUSR|S_IWUSR;
    config->iolog.maxseq = SESSID_MAX;
    if (!cb_iolog_dir(config, _PATH_SUDO_IO_LOGDIR, 0))
	goto bad;
    if (!cb_iolog_file(config, "%{seq}", 0))
	goto bad;
    config->iolog.uid = ROOT_UID;
    config->iolog.gid = ROOT_GID;
    config->iolog.gid_set = false;
    config->iolog.log_passwords = true;

    /* Event log defaults */
    config->eventlog.log_type = EVLOG_SYSLOG;
    config->eventlog.log_format = EVLOG_SUDO;
    config->eventlog.log_exit = false;

    /* Syslog defaults */
    config->syslog.maxlen = 960;
    config->syslog.server_facility = LOG_DAEMON;
    if (!cb_syslog_facility(config, LOGFAC, 0)) {
	sudo_warnx(U_("unknown syslog facility %s"), LOGFAC);
	goto bad;
    }
    if (!cb_syslog_acceptpri(config, PRI_SUCCESS, 0)) {
	sudo_warnx(U_("unknown syslog priority %s"), PRI_SUCCESS);
	goto bad;
    }
    if (!cb_syslog_rejectpri(config, PRI_FAILURE, 0)) {
	sudo_warnx(U_("unknown syslog priority %s"), PRI_FAILURE);
	goto bad;
    }
    if (!cb_syslog_alertpri(config, PRI_FAILURE, 0)) {
	sudo_warnx(U_("unknown syslog priority %s"), PRI_FAILURE);
	goto bad;
    }

    /* Log file defaults */
    if (!cb_logfile_time_format(config, "%h %e %T", 0))
	goto bad;
    if (!cb_logfile_path(config, _PATH_SUDO_LOGFILE, 0))
	goto bad;

    debug_return_ptr(config);
bad:
    logsrvd_conf_free(config);
    debug_return_ptr(NULL);
}

static bool
logsrvd_conf_apply(struct logsrvd_config *config)
{
#if defined(HAVE_OPENSSL)
    struct server_address *addr;
#endif
    debug_decl(logsrvd_conf_apply, SUDO_DEBUG_UTIL);

    /* There can be multiple passprompt regular expressions. */
    if (config->iolog.passprompt_regex == NULL) {
	if (!cb_iolog_passprompt_regex(config, PASSPROMPT_REGEX, 0))
	    debug_return_bool(false);
    }

    /* There can be multiple addresses so we can't set a default earlier. */
    if (TAILQ_EMPTY(&config->server.addresses.addrs)) {
	/* Enable plaintext listender. */
	if (!cb_server_listen_address(config, "*:" DEFAULT_PORT, 0))
	    debug_return_bool(false);
#if defined(HAVE_OPENSSL)
	/* If a certificate was specified, enable the TLS listener too. */
	if (config->server.tls_cert_path != NULL) {
	    if (!cb_server_listen_address(config, "*:" DEFAULT_PORT_TLS "(tls)", 0))
		debug_return_bool(false);
	}
    } else {
	/* Check that TLS configuration is valid. */
	TAILQ_FOREACH(addr, &config->server.addresses.addrs, entries) {
	    if (!addr->tls)
		continue;
	    /*
	     * If a TLS listener was explicitly enabled but the cert path
	     * was not, use the default.
	     */
	    if (config->server.tls_cert_path == NULL) {
		config->server.tls_cert_path =
		    strdup(DEFAULT_SERVER_CERT_PATH);
		if (config->server.tls_cert_path == NULL) {
		    sudo_warnx(U_("%s: %s"), __func__,
			U_("unable to allocate memory"));
		    debug_return_bool(false);
		}
	    }
	    break;
	}
#endif /* HAVE_OPENSSL */
    }

#if defined(HAVE_OPENSSL)
    TAILQ_FOREACH(addr, &config->server.addresses.addrs, entries) {
	if (!addr->tls)
	    continue;
        /* Create a TLS context for the server. */
	config->server.ssl_ctx = init_tls_context(
	    config->server.tls_cacert_path, config->server.tls_cert_path,
	    config->server.tls_key_path, config->server.tls_dhparams_path,
	    config->server.tls_ciphers_v12, config->server.tls_ciphers_v13,
	    config->server.tls_verify);
	if (config->server.ssl_ctx == NULL) {
	    sudo_warnx("%s", U_("unable to initialize server TLS context"));
	    debug_return_bool(false);
	}
	break;
    }

    if (TLS_CONFIGURED(config->relay)) {
	TAILQ_FOREACH(addr, &config->relay.relays.addrs, entries) {
	    if (!addr->tls)
		continue;
	    /* Create a TLS context for the relay. */
	    config->relay.ssl_ctx = init_tls_context(
		TLS_RELAY_STR(config, tls_cacert_path),
		TLS_RELAY_STR(config, tls_cert_path),
		TLS_RELAY_STR(config, tls_key_path),
		TLS_RELAY_STR(config, tls_dhparams_path),
		TLS_RELAY_STR(config, tls_ciphers_v12),
		TLS_RELAY_STR(config, tls_ciphers_v13),
		TLS_RELAY_INT(config, tls_verify));
	    if (config->relay.ssl_ctx == NULL) {
		sudo_warnx("%s", U_("unable to initialize relay TLS context"));
		debug_return_bool(false);
	    }
	    break;
	}
    }
#endif /* HAVE_OPENSSL */

    /* Clear store_first if not relaying. */
    if (TAILQ_EMPTY(&config->relay.relays.addrs))
	config->relay.store_first = false;

    /* Open server log if specified. */
    switch (config->server.log_type) {
    case SERVER_LOG_SYSLOG:
	sudo_warn_set_conversation(logsrvd_conv_syslog);
	break;
    case SERVER_LOG_FILE:
	config->server.log_stream =
	    logsrvd_open_log_file(config->server.log_file, O_WRONLY|O_APPEND|O_CREAT);
	if (config->server.log_stream == NULL)
	    debug_return_bool(false);
	sudo_warn_set_conversation(logsrvd_conv_logfile);
	break;
    case SERVER_LOG_NONE:
	sudo_warn_set_conversation(logsrvd_conv_none);
	break;
    case SERVER_LOG_STDERR:
	/* Default is stderr. */
	sudo_warn_set_conversation(NULL);
	break;
    default:
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
	    "cannot open unknown log type %d", config->eventlog.log_type);
	break;
    }

    /* Open event log if specified. */
    switch (config->eventlog.log_type) {
    case EVLOG_SYSLOG:
	openlog("sudo", 0, config->syslog.facility);
	break;
    case EVLOG_FILE:
	config->logfile.stream = logsrvd_open_eventlog(config);
	if (config->logfile.stream == NULL)
	    debug_return_bool(false);
	break;
    case EVLOG_NONE:
	break;
    default:
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
	    "cannot open unknown log type %d", config->eventlog.log_type);
	break;
    }

    /*
     * Update event and I/O log library config and install the new
     * logsrvd config.  We must not fail past this point or the event
     * and I/O log config will be inconsistent with the logsrvd config.
     */
    logsrvd_conf_iolog_setconf(config);
    logsrvd_conf_eventlog_setconf(config);

    logsrvd_conf_free(logsrvd_config);
    logsrvd_config = config;

    debug_return_bool(true);
}

/*
 * Read .ini style logsrvd.conf file.
 * If path is NULL, use _PATH_SUDO_LOGSRVD_CONF.
 * Note that we use '#' not ';' for the comment character.
 */
bool
logsrvd_conf_read(const char *path)
{
    struct logsrvd_config *config;
    char conf_file[PATH_MAX];
    bool ret = false;
    FILE *fp = NULL;
    int fd = -1;
    debug_decl(logsrvd_conf_read, SUDO_DEBUG_UTIL);

    config = logsrvd_conf_alloc();

    if (path != NULL) {
       if (strlcpy(conf_file, path, sizeof(conf_file)) >= sizeof(conf_file))
            errno = ENAMETOOLONG;
	else
	    fd = open(conf_file, O_RDONLY);
    } else {
	fd = sudo_open_conf_path(_PATH_SUDO_LOGSRVD_CONF, conf_file,
	    sizeof(conf_file), NULL);
    }
    if (fd != -1)
	fp = fdopen(fd, "r");
    if (fp == NULL) {
	if (path != NULL || errno != ENOENT) {
	    sudo_warn("%s", conf_file);
	    goto done;
	}
    } else {
	if (!logsrvd_conf_parse(config, fp, conf_file))
	    goto done;
    }

    /* Install new config */
    if (logsrvd_conf_apply(config)) {
	config = NULL;
	ret = true;
    }

done:
    logsrvd_conf_free(config);
    if (fp != NULL)
	fclose(fp);
    debug_return_bool(ret);
}

void
logsrvd_conf_cleanup(void)
{
    debug_decl(logsrvd_conf_cleanup, SUDO_DEBUG_UTIL);

    logsrvd_conf_free(logsrvd_config);
    logsrvd_config = NULL;

    debug_return;
}

void
logsrvd_warn_stderr(bool enabled)
{
    logsrvd_warn_enable_stderr = enabled;
}
