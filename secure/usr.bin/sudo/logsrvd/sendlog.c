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
#include <sys/time.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <netdb.h>
#ifdef HAVE_STDBOOL_H
# include <stdbool.h>
#else
# include <compat/stdbool.h>
#endif /* HAVE_STDBOOL_H */
#if defined(HAVE_STDINT_H)
# include <stdint.h>
#elif defined(HAVE_INTTYPES_H)
# include <inttypes.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#ifndef HAVE_GETADDRINFO
# include <compat/getaddrinfo.h>
#endif
#ifdef HAVE_GETOPT_LONG
# include <getopt.h>
# else
# include <compat/getopt.h>
#endif /* HAVE_GETOPT_LONG */

#include <sudo_compat.h>
#include <sudo_conf.h>
#include <sudo_debug.h>
#include <sudo_event.h>
#include <sudo_eventlog.h>
#include <sudo_fatal.h>
#include <sudo_gettext.h>
#include <sudo_iolog.h>
#include <sudo_util.h>

#include "sendlog.h"
#include <hostcheck.h>

#if defined(HAVE_OPENSSL)
# define TLS_HANDSHAKE_TIMEO_SEC 10
#endif

TAILQ_HEAD(connection_list, client_closure);
static struct connection_list connections = TAILQ_HEAD_INITIALIZER(connections);

static struct peer_info server_info = { "localhost" };
static char *iolog_dir;
static bool testrun = false;
static int nr_of_conns = 1;
static int finished_transmissions = 0;

#if defined(HAVE_OPENSSL)
static SSL_CTX *ssl_ctx = NULL;
static const char *ca_bundle = NULL;
static const char *cert = NULL;
static const char *key = NULL;
static bool verify_server = true;
#endif

/* Server callback may redirect to client callback for TLS. */
static void client_msg_cb(int fd, int what, void *v);
static void server_msg_cb(int fd, int what, void *v);

static void
display_usage(FILE *fp)
{
#if defined(HAVE_OPENSSL)
    fprintf(fp, "usage: %s [-AnV] [-b ca_bundle] [-c cert_file] [-h host] "
	"[-i iolog-id] [-k key_file] [-p port] "
#else
    fprintf(fp, "usage: %s [-AnV] [-h host] [-i iolog-id] [-p port] "
#endif
	"[-r restart-point] [-R reject-reason] [-s stop-point] [-t number] /path/to/iolog\n",
        getprogname());
}

sudo_noreturn static void
usage(void)
{
    display_usage(stderr);
    exit(EXIT_FAILURE);
}

sudo_noreturn static void
help(void)
{
    printf("%s - %s\n\n", getprogname(),
	_("send sudo I/O log to remote server"));
    display_usage(stdout);
    printf("\n%s\n", _("Options:"));
    printf("      --help            %s\n",
	_("display help message and exit"));
    printf("  -A, --accept          %s\n",
	_("only send an accept event (no I/O)"));
#if defined(HAVE_OPENSSL)
    printf("  -b, --ca-bundle       %s\n",
	_("certificate bundle file to verify server's cert against"));
    printf("  -c, --cert            %s\n",
	_("certificate file for TLS handshake"));
#endif
    printf("  -h, --host            %s\n",
	_("host to send logs to"));
    printf("  -i, --iolog_id        %s\n",
	_("remote ID of I/O log to be resumed"));
#if defined(HAVE_OPENSSL)
    printf("  -k, --key             %s\n",
	_("private key file"));
    printf("  -n, --no-verify       %s\n",
	_("do not verify server certificate"));
#endif
    printf("  -p, --port            %s\n",
	_("port to use when connecting to host"));
    printf("  -r, --restart         %s\n",
	_("restart previous I/O log transfer"));
    printf("  -R, --reject          %s\n",
	_("reject the command with the given reason"));
    printf("  -s, --stop-after        %s\n",
	_("stop transfer after reaching this time"));
    printf("  -t, --test            %s\n",
	_("test audit server by sending selected I/O log n times in parallel"));
    printf("  -V, --version         %s\n",
	_("display version information and exit"));
    putchar('\n');
    exit(EXIT_SUCCESS);
}

/*
 * Connect to specified host:port
 * If host has multiple addresses, the first one that connects is used.
 * Returns open socket or -1 on error.
 */
static int
connect_server(struct peer_info *server, const char *port)
{
    struct addrinfo hints, *res, *res0;
    const char *addr, *cause = "getaddrinfo";
    int error, sock, save_errno;
    debug_decl(connect_server, SUDO_DEBUG_UTIL);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    error = getaddrinfo(server->name, port, &hints, &res0);
    if (error != 0) {
	sudo_warnx(U_("unable to look up %s:%s: %s"), server->name, port,
	    gai_strerror(error));
	debug_return_int(-1);
    }

    sock = -1;
    for (res = res0; res; res = res->ai_next) {
	sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (sock == -1) {
	    cause = "socket";
	    continue;
	}
	if (connect(sock, res->ai_addr, res->ai_addrlen) == -1) {
	    cause = "connect";
	    save_errno = errno;
	    close(sock);
	    errno = save_errno;
	    sock = -1;
	    continue;
	}
	if (server->ipaddr[0] == '\0') {
	    switch (res->ai_family) {
	    case AF_INET:
		addr = (char *)&((struct sockaddr_in *)res->ai_addr)->sin_addr;
		break;
	    case AF_INET6:
		addr = (char *)&((struct sockaddr_in6 *)res->ai_addr)->sin6_addr;
		break;
	    default:
		cause = "ai_family";
		save_errno = EAFNOSUPPORT;
		close(sock);
		errno = save_errno;
		sock = -1;
		continue;
	    }
	    if (inet_ntop(res->ai_family, addr, server->ipaddr,
		    sizeof(server->ipaddr)) == NULL) {
		sudo_warnx("%s", U_("unable to get server IP addr"));
	    }
	}
	break;	/* success */
    }
    freeaddrinfo(res0);

    if (sock != -1) {
	int flags = fcntl(sock, F_GETFL, 0);
	if (flags == -1 || fcntl(sock, F_SETFL, flags | O_NONBLOCK) == -1) {
	    cause = "fcntl(O_NONBLOCK)";
	    save_errno = errno;
	    close(sock);
	    errno = save_errno;
	    sock = -1;
	}
    }
    if (sock == -1)
	sudo_warn("%s", cause);

    debug_return_int(sock);
}

/*
 * Get a buffer from the free list if possible, else allocate a new one.
 */
static struct connection_buffer *
get_free_buf(size_t len, struct client_closure *closure)
{
    struct connection_buffer *buf;
    debug_decl(get_free_buf, SUDO_DEBUG_UTIL);

    buf = TAILQ_FIRST(&closure->free_bufs);
    if (buf != NULL) {
        TAILQ_REMOVE(&closure->free_bufs, buf, entries);
    } else {
        if ((buf = calloc(1, sizeof(*buf))) == NULL) {
	    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	    debug_return_ptr(NULL);
	}
    }

    if (len > buf->size) {
	free(buf->data);
	buf->size = sudo_pow2_roundup(len);
	if (buf->size < len || (buf->data = malloc(buf->size)) == NULL) {
	    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	    free(buf);
	    buf = NULL;
	}
    }

    debug_return_ptr(buf);
}

/*
 * Read the next I/O buffer as described by closure->timing.
 */
static bool
read_io_buf(struct client_closure *closure)
{
    struct timing_closure *timing = &closure->timing;
    const char *errstr = NULL;
    size_t nread;
    debug_decl(read_io_buf, SUDO_DEBUG_UTIL);

    if (!closure->iolog_files[timing->event].enabled) {
	errno = ENOENT;
	sudo_warn("%s/%s", iolog_dir, iolog_fd_to_name(timing->event));
	debug_return_bool(false);
    }

    /* Expand buf as needed. */
    if (timing->u.nbytes > closure->bufsize) {
	const size_t new_size = sudo_pow2_roundup(timing->u.nbytes);
	if (new_size < timing->u.nbytes) {
	    /* overflow */
	    errno = ENOMEM;
	    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	    timing->u.nbytes = 0;
	    debug_return_bool(false);
	}
	free(closure->buf);
	if ((closure->buf = malloc(new_size)) == NULL) {
	    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	    closure->bufsize = 0;
	    timing->u.nbytes = 0;
	    debug_return_bool(false);
	}
	closure->bufsize = new_size;
    }

    nread = (size_t)iolog_read(&closure->iolog_files[timing->event],
	closure->buf, timing->u.nbytes, &errstr);
    if (nread == (size_t)-1) {
	sudo_warnx(U_("unable to read %s/%s: %s"), iolog_dir,
	    iolog_fd_to_name(timing->event), errstr);
	debug_return_bool(false);
    }
    debug_return_bool(true);
}

/*
 * Format a ClientMessage and store the wire format message in buf.
 * Returns true on success, false on failure.
 */
static bool
fmt_client_message(struct client_closure *closure, ClientMessage *msg)
{
    struct connection_buffer *buf = NULL;
    uint32_t msg_len;
    bool ret = false;
    size_t len;
    debug_decl(fmt_client_message, SUDO_DEBUG_UTIL);

    len = client_message__get_packed_size(msg);
    if (len > MESSAGE_SIZE_MAX) {
    	sudo_warnx(U_("client message too large: %zu"), len);
        goto done;
    }
    /* Wire message size is used for length encoding, precedes message. */
    msg_len = htonl((uint32_t)len);
    len += sizeof(msg_len);

    if (!TAILQ_EMPTY(&closure->write_bufs)) {
	buf = TAILQ_FIRST(&closure->write_bufs);
	if (len > buf->size - buf->len) {
	    /* Too small. */
	    buf = NULL;
	}
    }
    if (buf == NULL) {
	if ((buf = get_free_buf(len, closure)) == NULL) {
	    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	    goto done;
	}
	TAILQ_INSERT_TAIL(&closure->write_bufs, buf, entries);
    }

    memcpy(buf->data + buf->len, &msg_len, sizeof(msg_len));
    client_message__pack(msg, buf->data + buf->len + sizeof(msg_len));
    buf->len += len;

    ret = true;

done:
    debug_return_bool(ret);
}

static bool
fmt_client_hello(struct client_closure *closure)
{
    ClientMessage client_msg = CLIENT_MESSAGE__INIT;
    ClientHello hello_msg = CLIENT_HELLO__INIT;
    bool ret = false;
    debug_decl(fmt_client_hello, SUDO_DEBUG_UTIL);

    sudo_debug_printf(SUDO_DEBUG_INFO, "%s: sending ClientHello", __func__);
    hello_msg.client_id = (char *)"Sudo Sendlog " PACKAGE_VERSION;

    /* Schedule ClientMessage */
    client_msg.u.hello_msg = &hello_msg;
    client_msg.type_case = CLIENT_MESSAGE__TYPE_HELLO_MSG;
    ret = fmt_client_message(closure, &client_msg);
    if (ret) {
	if (sudo_ev_add(closure->evbase, closure->read_ev, NULL, false) == -1)
	    ret = false;
	if (sudo_ev_add(closure->evbase, closure->write_ev, NULL, false) == -1)
	    ret = false;
    }

    debug_return_bool(ret);
}

#if defined(HAVE_OPENSSL)
/* Wrapper for fmt_client_hello() called via tls_connect_cb() */
static bool
tls_start_fn(struct tls_client_closure *tls_client)
{
    return fmt_client_hello(tls_client->parent_closure);
}
#endif /* HAVE_OPENSSL */

static void
free_info_messages(InfoMessage **info_msgs, size_t n_info_msgs)
{
    debug_decl(free_info_messages, SUDO_DEBUG_UTIL);

    if (info_msgs != NULL) {
	while (n_info_msgs) {
	    if (info_msgs[--n_info_msgs]->value_case == INFO_MESSAGE__VALUE_STRLISTVAL) {
		/* Only strlistval was dynamically allocated */
		free(info_msgs[n_info_msgs]->u.strlistval->strings);
		free(info_msgs[n_info_msgs]->u.strlistval);
	    }
	    free(info_msgs[n_info_msgs]);
	}
	free(info_msgs);
    }

    debug_return;
}

/*
 * Convert a NULL-terminated string vector (argv, envp) to a
 * StringList with an associated size.
 * Performs a shallow copy of the strings (copies pointers).
 */
static InfoMessage__StringList *
vec_to_stringlist(char * const *vec)
{
    InfoMessage__StringList *strlist;
    size_t len;
    debug_decl(vec_to_stringlist, SUDO_DEBUG_UTIL);

    strlist = malloc(sizeof(*strlist));
    if (strlist == NULL)
	goto done;
    info_message__string_list__init(strlist);

    /* Convert vec into a StringList. */
    for (len = 0; vec[len] != NULL; len++) {
	continue;
    }
    strlist->strings = reallocarray(NULL, len, sizeof(char *));
    if (strlist->strings == NULL) {
	free(strlist);
	strlist = NULL;
	goto done;
    }
    strlist->n_strings = len;
    for (len = 0; vec[len] != NULL; len++) {
	strlist->strings[len] = vec[len];
    }

done:
    debug_return_ptr(strlist);
}

/*
 * Split command + args separated by whitespace into a StringList.
 * Returns a StringList containing command and args, reusing the contents
 * of "command", which is modified.
 */
static InfoMessage__StringList *
command_to_stringlist(char *command)
{
    InfoMessage__StringList *strlist;
    char *cp;
    size_t len;
    debug_decl(command_to_stringlist, SUDO_DEBUG_UTIL);

    strlist = malloc(sizeof(*strlist));
    if (strlist == NULL)
	debug_return_ptr(NULL);
    info_message__string_list__init(strlist);

    for (cp = command, len = 0;;) {
	len++;
	if ((cp = strchr(cp, ' ')) == NULL)
	    break;
	cp++;
    }
    strlist->strings = reallocarray(NULL, len, sizeof(char *));
    if (strlist->strings == NULL) {
	free(strlist);
	debug_return_ptr(NULL);
    }
    strlist->n_strings = len;

    for (cp = command, len = 0;;) {
	strlist->strings[len++] = cp;
	if ((cp = strchr(cp, ' ')) == NULL)
	    break;
	*cp++ = '\0';
    }

    debug_return_ptr(strlist);
}

/*
 * Build runargv StringList using either argv or command in evlog.
 * Truncated command in evlog after first space as a side effect.
 */
static InfoMessage__StringList *
fmt_runargv(const struct eventlog *evlog)
{
    InfoMessage__StringList *runargv;
    debug_decl(fmt_runargv, SUDO_DEBUG_UTIL);

    /* We may have runargv from the log.json file. */
    if (evlog->runargv != NULL && evlog->runargv[0] != NULL) {
	/* Convert evlog->runargv into a StringList. */
	runargv = vec_to_stringlist(evlog->runargv);
	if (runargv != NULL) {
	    /* Make sure command doesn't include arguments. */
	    char *cp = strchr(evlog->command, ' ');
	    if (cp != NULL)
		*cp = '\0';
	}
    } else {
	/* No log.json file, split command into a StringList. */
	runargv = command_to_stringlist(evlog->command);
    }

    debug_return_ptr(runargv);
}

/*
 * Build runenv StringList from env in evlog, if present.
 */
static InfoMessage__StringList *
fmt_runenv(const struct eventlog *evlog)
{
    debug_decl(fmt_runenv, SUDO_DEBUG_UTIL);

    /* Only present in log.json. */
    if (evlog->runenv == NULL || evlog->runenv[0] == NULL)
	debug_return_ptr(NULL);

    debug_return_ptr(vec_to_stringlist(evlog->runenv));
}

/*
 * Build submitenv StringList from env in evlog, if present.
 */
static InfoMessage__StringList *
fmt_submitenv(const struct eventlog *evlog)
{
    debug_decl(fmt_submitenv, SUDO_DEBUG_UTIL);

    /* Only present in log.json. */
    if (evlog->submitenv == NULL || evlog->submitenv[0] == NULL)
	debug_return_ptr(NULL);

    debug_return_ptr(vec_to_stringlist(evlog->submitenv));
}

static InfoMessage **
fmt_info_messages(const struct eventlog *evlog, char *hostname,
    size_t *n_info_msgs)
{
    InfoMessage **info_msgs = NULL;
    InfoMessage__StringList *runargv = NULL;
    InfoMessage__StringList *runenv = NULL;
    InfoMessage__StringList *submitenv = NULL;
    size_t info_msgs_size, n = 0;
    debug_decl(fmt_info_messages, SUDO_DEBUG_UTIL);

    runargv = fmt_runargv(evlog);
    if (runargv == NULL)
	goto oom;

    /* runenv and submitenv are only present in log.json */
    runenv = fmt_runenv(evlog);
    submitenv = fmt_submitenv(evlog);

    /* The sudo I/O log info file has limited info. */
    info_msgs_size = 15;
    info_msgs = calloc(info_msgs_size, sizeof(InfoMessage *));
    if (info_msgs == NULL)
	goto oom;
    for (n = 0; n < info_msgs_size; n++) {
	info_msgs[n] = malloc(sizeof(InfoMessage));
	if (info_msgs[n] == NULL)
            goto oom;
	info_message__init(info_msgs[n]);
    }

#define fill_str(_n, _v) do { \
    info_msgs[n]->key = (char *)(_n); \
    info_msgs[n]->u.strval = (_v); \
    info_msgs[n]->value_case = INFO_MESSAGE__VALUE_STRVAL; \
    n++; \
} while (0)

#define fill_strlist(_n, _v) do { \
    info_msgs[n]->key = (char *)(_n); \
    info_msgs[n]->u.strlistval = (_v); \
    info_msgs[n]->value_case = INFO_MESSAGE__VALUE_STRLISTVAL; \
    n++; \
} while (0)

#define fill_num(_n, _v) do { \
    info_msgs[n]->key = (char *)(_n); \
    info_msgs[n]->u.numval = (_v); \
    info_msgs[n]->value_case = INFO_MESSAGE__VALUE_NUMVAL; \
    n++; \
} while (0)

    /* Fill in info_msgs */
    n = 0;
    fill_num("columns", evlog->columns);
    fill_str("command", evlog->command);
    fill_num("lines", evlog->lines);
    fill_strlist("runargv", runargv);
    runargv = NULL;
    if (submitenv != NULL) {
	fill_strlist("submitenv", submitenv);
	submitenv = NULL;
    }
    if (runenv != NULL) {
	fill_strlist("runenv", runenv);
	runenv = NULL;
    }
    if (evlog->rungid != (gid_t)-1) {
	fill_num("rungid", evlog->rungid);
    }
    if (evlog->rungroup != NULL) {
	fill_str("rungroup", evlog->rungroup);
    }
    if (evlog->runuid != (uid_t)-1) {
	fill_num("runuid", evlog->runuid);
    }
    fill_str("runuser", evlog->runuser);
    fill_str("source", evlog->source);
    fill_str("submitcwd", evlog->cwd);
    fill_str("submithost", hostname);
    fill_str("submituser", evlog->submituser);
    fill_str("ttyname", evlog->ttyname);

    /* Update n_info_msgs. */
    *n_info_msgs = n;

    /* Avoid leaking unused info_msg structs. */
    while (n < info_msgs_size) {
        free(info_msgs[n++]);
    }

    debug_return_ptr(info_msgs);

oom:
    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
    free_info_messages(info_msgs, n);
    if (runargv != NULL) {
        free(runargv->strings);
        free(runargv);
    }
    if (runenv != NULL) {
        free(runenv->strings);
        free(runenv);
    }
    if (submitenv != NULL) {
        free(submitenv->strings);
        free(submitenv);
    }
    *n_info_msgs = 0;
    debug_return_ptr(NULL);
}

/*
 * Build and format a RejectMessage wrapped in a ClientMessage.
 * Stores the wire format message in the closure's write buffer.
 * Returns true on success, false on failure.
 */
static bool
fmt_reject_message(struct client_closure *closure)
{
    ClientMessage client_msg = CLIENT_MESSAGE__INIT;
    RejectMessage reject_msg = REJECT_MESSAGE__INIT;
    TimeSpec tv = TIME_SPEC__INIT;
    size_t n_info_msgs;
    bool ret = false;
    char *hostname;
    debug_decl(fmt_reject_message, SUDO_DEBUG_UTIL);

    /*
     * Fill in RejectMessage and add it to ClientMessage.
     */
    if ((hostname = sudo_gethostname()) == NULL) {
	sudo_warn("gethostname");
	debug_return_bool(false);
    }

    /* Sudo I/O logs only store start time in seconds. */
    tv.tv_sec = (int64_t)closure->evlog->submit_time.tv_sec;
    tv.tv_nsec = (int32_t)closure->evlog->submit_time.tv_nsec;
    reject_msg.submit_time = &tv;

    /* Why the command was rejected. */
    reject_msg.reason = closure->reject_reason;

    reject_msg.info_msgs = fmt_info_messages(closure->evlog, hostname,
        &n_info_msgs);
    if (reject_msg.info_msgs == NULL)
	goto done;

    /* Update n_info_msgs. */
    reject_msg.n_info_msgs = n_info_msgs;

    sudo_debug_printf(SUDO_DEBUG_INFO,
	"%s: sending RejectMessage, array length %zu", __func__, n_info_msgs);

    /* Schedule ClientMessage */
    client_msg.u.reject_msg = &reject_msg;
    client_msg.type_case = CLIENT_MESSAGE__TYPE_REJECT_MSG;
    ret = fmt_client_message(closure, &client_msg);
    if (ret) {
	if (sudo_ev_add(closure->evbase, closure->write_ev, NULL, false) == -1)
	    ret = false;
    }

done:
    free_info_messages(reject_msg.info_msgs, n_info_msgs);
    free(hostname);

    debug_return_bool(ret);
}

/*
 * Build and format an AcceptMessage wrapped in a ClientMessage.
 * Stores the wire format message in the closure's write buffer.
 * Returns true on success, false on failure.
 */
static bool
fmt_accept_message(struct client_closure *closure)
{
    ClientMessage client_msg = CLIENT_MESSAGE__INIT;
    AcceptMessage accept_msg = ACCEPT_MESSAGE__INIT;
    TimeSpec tv = TIME_SPEC__INIT;
    size_t n_info_msgs;
    bool ret = false;
    char *hostname;
    debug_decl(fmt_accept_message, SUDO_DEBUG_UTIL);

    /*
     * Fill in AcceptMessage and add it to ClientMessage.
     */
    if ((hostname = sudo_gethostname()) == NULL) {
	sudo_warn("gethostname");
	debug_return_bool(false);
    }

    /* Sudo I/O logs only store start time in seconds. */
    tv.tv_sec = (int64_t)closure->evlog->submit_time.tv_sec;
    tv.tv_nsec = (int32_t)closure->evlog->submit_time.tv_nsec;
    accept_msg.submit_time = &tv;

    /* Client will send IoBuffer messages. */
    accept_msg.expect_iobufs = !closure->accept_only;

    accept_msg.info_msgs = fmt_info_messages(closure->evlog, hostname,
        &n_info_msgs);
    if (accept_msg.info_msgs == NULL)
	goto done;

    /* Update n_info_msgs. */
    accept_msg.n_info_msgs = n_info_msgs;

    sudo_debug_printf(SUDO_DEBUG_INFO,
	"%s: sending AcceptMessage, array length %zu", __func__, n_info_msgs);

    /* Schedule ClientMessage */
    client_msg.u.accept_msg = &accept_msg;
    client_msg.type_case = CLIENT_MESSAGE__TYPE_ACCEPT_MSG;
    ret = fmt_client_message(closure, &client_msg);
    if (ret) {
	if (sudo_ev_add(closure->evbase, closure->write_ev, NULL, false) == -1)
	    ret = false;
    }

done:
    free_info_messages(accept_msg.info_msgs, n_info_msgs);
    free(hostname);

    debug_return_bool(ret);
}

/*
 * Build and format a RestartMessage wrapped in a ClientMessage.
 * Stores the wire format message in the closure's write buffer.
 * Returns true on success, false on failure.
 */
static bool
fmt_restart_message(struct client_closure *closure)
{
    ClientMessage client_msg = CLIENT_MESSAGE__INIT;
    RestartMessage restart_msg = RESTART_MESSAGE__INIT;
    TimeSpec tv = TIME_SPEC__INIT;
    bool ret = false;
    debug_decl(fmt_restart_message, SUDO_DEBUG_UTIL);

    sudo_debug_printf(SUDO_DEBUG_INFO,
	"%s: sending RestartMessage, [%lld, %ld]", __func__,
	(long long)closure->restart.tv_sec, closure->restart.tv_nsec);

    tv.tv_sec = (int64_t)closure->restart.tv_sec;
    tv.tv_nsec = (int32_t)closure->restart.tv_nsec;
    restart_msg.resume_point = &tv;
    restart_msg.log_id = (char *)closure->iolog_id;

    /* Schedule ClientMessage */
    client_msg.u.restart_msg = &restart_msg;
    client_msg.type_case = CLIENT_MESSAGE__TYPE_RESTART_MSG;
    ret = fmt_client_message(closure, &client_msg);
    if (ret) {
	if (sudo_ev_add(closure->evbase, closure->write_ev, NULL, false) == -1)
	    ret = false;
    }

    debug_return_bool(ret);
}

/*
 * Build and format an ExitMessage wrapped in a ClientMessage.
 * Stores the wire format message in the closure's write buffer list.
 * Returns true on success, false on failure.
 */
static bool
fmt_exit_message(struct client_closure *closure)
{
    ClientMessage client_msg = CLIENT_MESSAGE__INIT;
    ExitMessage exit_msg = EXIT_MESSAGE__INIT;
    TimeSpec run_time = TIME_SPEC__INIT;
    struct eventlog *evlog = closure->evlog;
    bool ret = false;
    debug_decl(fmt_exit_message, SUDO_DEBUG_UTIL);

    if (evlog->exit_value != -1)
	exit_msg.exit_value = evlog->exit_value;
    if (sudo_timespecisset(&evlog->run_time)) {
	run_time.tv_sec = (int64_t)evlog->run_time.tv_sec;
	run_time.tv_nsec = (int32_t)evlog->run_time.tv_nsec;
	exit_msg.run_time = &run_time;
    }
    if (evlog->signal_name != NULL) {
	exit_msg.signal = evlog->signal_name;
	exit_msg.dumped_core = evlog->dumped_core;
    }

    if (evlog->signal_name != NULL) {
	sudo_debug_printf(SUDO_DEBUG_INFO,
	    "%s: sending ExitMessage, signal %s, run_time [%lld, %ld]",
	    __func__, evlog->signal_name, (long long)evlog->run_time.tv_sec,
	    evlog->run_time.tv_nsec);
    } else {
	sudo_debug_printf(SUDO_DEBUG_INFO,
	    "%s: sending ExitMessage, exit value %d, run_time [%lld, %ld]",
	    __func__, evlog->exit_value, (long long)evlog->run_time.tv_sec,
	    evlog->run_time.tv_nsec);
    }

    /* Send ClientMessage */
    client_msg.u.exit_msg = &exit_msg;
    client_msg.type_case = CLIENT_MESSAGE__TYPE_EXIT_MSG;
    if (!fmt_client_message(closure, &client_msg))
	goto done;

    ret = true;

done:
    debug_return_bool(ret);
}

/*
 * Build and format an IoBuffer wrapped in a ClientMessage.
 * Stores the wire format message in the closure's write buffer list.
 * Returns true on success, false on failure.
 */
static bool
fmt_io_buf(int type, struct client_closure *closure)
{
    ClientMessage client_msg = CLIENT_MESSAGE__INIT;
    IoBuffer iobuf_msg = IO_BUFFER__INIT;
    TimeSpec delay = TIME_SPEC__INIT;
    bool ret = false;
    debug_decl(fmt_io_buf, SUDO_DEBUG_UTIL);

    if (!read_io_buf(closure))
	goto done;

    /* Fill in IoBuffer. */
    /* TODO: split buffer if it is too large */
    delay.tv_sec = (int64_t)closure->timing.delay.tv_sec;
    delay.tv_nsec = (int32_t)closure->timing.delay.tv_nsec;
    iobuf_msg.delay = &delay;
    iobuf_msg.data.data = (void *)closure->buf;
    iobuf_msg.data.len = closure->timing.u.nbytes;

    sudo_debug_printf(SUDO_DEBUG_INFO,
	"%s: sending IoBuffer length %zu, type %d, size %zu", __func__,
	iobuf_msg.data.len, type, io_buffer__get_packed_size(&iobuf_msg));

    /* Send ClientMessage, it doesn't matter which IoBuffer we set. */
    client_msg.u.ttyout_buf = &iobuf_msg;
    client_msg.type_case = type;
    if (!fmt_client_message(closure, &client_msg))
        goto done;

    ret = true;

done:
    debug_return_bool(ret);
}

/*
 * Build and format a ChangeWindowSize message wrapped in a ClientMessage.
 * Stores the wire format message in the closure's write buffer list.
 * Returns true on success, false on failure.
 */
static bool
fmt_winsize(struct client_closure *closure)
{
    ClientMessage client_msg = CLIENT_MESSAGE__INIT;
    ChangeWindowSize winsize_msg = CHANGE_WINDOW_SIZE__INIT;
    TimeSpec delay = TIME_SPEC__INIT;
    struct timing_closure *timing = &closure->timing;
    bool ret = false;
    debug_decl(fmt_winsize, SUDO_DEBUG_UTIL);

    /* Fill in ChangeWindowSize message. */
    delay.tv_sec = (int64_t)timing->delay.tv_sec;
    delay.tv_nsec = (int32_t)timing->delay.tv_nsec;
    winsize_msg.delay = &delay;
    winsize_msg.rows = timing->u.winsize.lines;
    winsize_msg.cols = timing->u.winsize.cols;

    sudo_debug_printf(SUDO_DEBUG_INFO, "%s: sending ChangeWindowSize, %dx%d",
	__func__, winsize_msg.rows, winsize_msg.cols);

    /* Send ClientMessage */
    client_msg.u.winsize_event = &winsize_msg;
    client_msg.type_case = CLIENT_MESSAGE__TYPE_WINSIZE_EVENT;
    if (!fmt_client_message(closure, &client_msg))
        goto done;

    ret = true;

done:
    debug_return_bool(ret);
}

/*
 * Build and format a CommandSuspend message wrapped in a ClientMessage.
 * Stores the wire format message in the closure's write buffer list.
 * Returns true on success, false on failure.
 */
static bool
fmt_suspend(struct client_closure *closure)
{
    ClientMessage client_msg = CLIENT_MESSAGE__INIT;
    CommandSuspend suspend_msg = COMMAND_SUSPEND__INIT;
    TimeSpec delay = TIME_SPEC__INIT;
    struct timing_closure *timing = &closure->timing;
    bool ret = false;
    debug_decl(fmt_suspend, SUDO_DEBUG_UTIL);

    /* Fill in CommandSuspend message. */
    delay.tv_sec = (int64_t)timing->delay.tv_sec;
    delay.tv_nsec = (int32_t)timing->delay.tv_nsec;
    suspend_msg.delay = &delay;
    if (sig2str(timing->u.signo, closure->buf) == -1)
	goto done;
    suspend_msg.signal = closure->buf;

    sudo_debug_printf(SUDO_DEBUG_INFO,
    	"%s: sending CommandSuspend, SIG%s", __func__, suspend_msg.signal);

    /* Send ClientMessage */
    client_msg.u.suspend_event = &suspend_msg;
    client_msg.type_case = CLIENT_MESSAGE__TYPE_SUSPEND_EVENT;
    if (!fmt_client_message(closure, &client_msg))
        goto done;

    ret = true;

done:
    debug_return_bool(ret);
}

/*
 * Read the next entry for the I/O log timing file and format a ClientMessage.
 * Stores the wire format message in the closure's write buffer list.
 * Returns true on success, false on failure.
 */ 
static bool
fmt_next_iolog(struct client_closure *closure)
{
    struct timing_closure *timing = &closure->timing;
    bool ret = false;
    debug_decl(fmt_next_iolog, SUDO_DEBUG_UTIL);

    for (;;) {
	const int timing_status = iolog_read_timing_record(
	    &closure->iolog_files[IOFD_TIMING], timing);
	switch (timing_status) {
	case 0:
	    /* OK */
	    break;
	case 1:
	    /* no more IO buffers */
	    closure->state = SEND_EXIT;
	    debug_return_bool(fmt_exit_message(closure));
	case -1:
	default:
	    debug_return_bool(false);
	}

	/* Track elapsed time for comparison with commit points. */
	sudo_timespecadd(&closure->elapsed, &timing->delay, &closure->elapsed);

	/* If there is a stopping point, make sure we haven't reached it. */
	if (sudo_timespecisset(&closure->stop_after)) {
	    if (sudo_timespeccmp(&closure->elapsed, &closure->stop_after, >)) {
		/* Reached limit, force premature end. */
		sudo_timespecsub(&closure->elapsed, &timing->delay,
		    &closure->elapsed);
		debug_return_bool(false);
	    }
	}

	/* If we have a restart point, ignore records until we hit it. */
	if (sudo_timespecisset(&closure->restart)) {
	    if (sudo_timespeccmp(&closure->restart, &closure->elapsed, >=))
		continue;
	    sudo_timespecclear(&closure->restart);	/* caught up */
	}

	switch (timing->event) {
	case IO_EVENT_STDIN:
	    ret = fmt_io_buf(CLIENT_MESSAGE__TYPE_STDIN_BUF, closure);
	    break;
	case IO_EVENT_STDOUT:
	    ret = fmt_io_buf(CLIENT_MESSAGE__TYPE_STDOUT_BUF, closure);
	    break;
	case IO_EVENT_STDERR:
	    ret = fmt_io_buf(CLIENT_MESSAGE__TYPE_STDERR_BUF, closure);
	    break;
	case IO_EVENT_TTYIN:
	    ret = fmt_io_buf(CLIENT_MESSAGE__TYPE_TTYIN_BUF, closure);
	    break;
	case IO_EVENT_TTYOUT:
	    ret = fmt_io_buf(CLIENT_MESSAGE__TYPE_TTYOUT_BUF, closure);
	    break;
	case IO_EVENT_WINSIZE:
	    ret = fmt_winsize(closure);
	    break;
	case IO_EVENT_SUSPEND:
	    ret = fmt_suspend(closure);
	    break;
	default:
	    sudo_warnx(U_("unexpected I/O event %d"), timing->event);
	    break;
	}

	/* Keep filling write buffer as long as we only have one of them. */
	if (!ret)
	    break;
	if (TAILQ_NEXT(TAILQ_FIRST(&closure->write_bufs), entries) != NULL)
	    break;
    }

    debug_return_bool(ret);
}

/*
 * Additional work to do after a ClientMessage was sent to the server.
 * Advances state and formats the next ClientMessage (if any).
 */
static bool
client_message_completion(struct client_closure *closure)
{
    debug_decl(client_message_completion, SUDO_DEBUG_UTIL);

    switch (closure->state) {
    case RECV_HELLO:
	/* Wait for ServerHello, nothing to write until then. */
	sudo_ev_del(closure->evbase, closure->write_ev);
	break;
    case SEND_ACCEPT:
	if (closure->accept_only) {
	    closure->state = SEND_EXIT;
	    debug_return_bool(fmt_exit_message(closure));
	}
	FALLTHROUGH;
    case SEND_RESTART:
	closure->state = SEND_IO;
	FALLTHROUGH;
    case SEND_IO:
	/* fmt_next_iolog() will advance state on EOF. */
	if (!fmt_next_iolog(closure))
	    debug_return_bool(false);
	break;
    case SEND_REJECT:
	/* Done writing, wait for server to close connection. */
	sudo_ev_del(closure->evbase, closure->write_ev);
	closure->state = FINISHED;
	break;
    case SEND_EXIT:
	/* Done writing, wait for final commit point if sending I/O. */
	sudo_ev_del(closure->evbase, closure->write_ev);
	closure->state = closure->accept_only ? FINISHED : CLOSING;
	break;
    default:
	sudo_warnx(U_("%s: unexpected state %d"), __func__, closure->state);
	debug_return_bool(false);
    }
    debug_return_bool(true);
}

/*
 * Respond to a ServerHello message from the server.
 * Returns true on success, false on error.
 */
static bool
handle_server_hello(ServerHello *msg, struct client_closure *closure)
{
    size_t n;
    debug_decl(handle_server_hello, SUDO_DEBUG_UTIL);

    if (closure->state != RECV_HELLO) {
	sudo_warnx(U_("%s: unexpected state %d"), __func__, closure->state);
	debug_return_bool(false);
    }

    /* Check that ServerHello is valid. */
    if (msg->server_id == NULL || msg->server_id[0] == '\0') {
	sudo_warnx("%s", U_("invalid ServerHello"));
	debug_return_bool(false);
    }

    if (!testrun) {
        printf("Server ID: %s\n", msg->server_id);
        /* TODO: handle redirect */
        if (msg->redirect != NULL && msg->redirect[0] != '\0')
            printf("Redirect: %s\n", msg->redirect);
        for (n = 0; n < msg->n_servers; n++) {
            printf("Server %u: %s\n", (unsigned int)n + 1, msg->servers[n]);
        }
    }

    debug_return_bool(true);
}

/*
 * Respond to a CommitPoint message from the server.
 * Returns true on success, false on error.
 */
static bool
handle_commit_point(TimeSpec *commit_point, struct client_closure *closure)
{
    debug_decl(handle_commit_point, SUDO_DEBUG_UTIL);

    /* Only valid after we have sent an IO buffer. */
    if (closure->state < SEND_IO) {
	sudo_warnx(U_("%s: unexpected state %d"), __func__, closure->state);
	debug_return_bool(false);
    }

    sudo_debug_printf(SUDO_DEBUG_INFO, "%s: commit point: [%lld, %d]",
	__func__, (long long)commit_point->tv_sec, commit_point->tv_nsec);
    closure->committed.tv_sec = (time_t)commit_point->tv_sec;
    closure->committed.tv_nsec = (long)commit_point->tv_nsec;

    debug_return_bool(true);
}

/*
 * Respond to a LogId message from the server.
 * Always returns true.
 */
static bool
handle_log_id(char *id, struct client_closure *closure)
{
    debug_decl(handle_log_id, SUDO_DEBUG_UTIL);

    if (!testrun)
        printf("Remote log ID: %s\n", id);

    debug_return_bool(true);
}

/*
 * Respond to a ServerError message from the server.
 * Always returns false.
 */
static bool
handle_server_error(char *errmsg, struct client_closure *closure)
{
    debug_decl(handle_server_error, SUDO_DEBUG_UTIL);

    sudo_warnx(U_("error message received from server: %s"), errmsg);
    debug_return_bool(false);
}

/*
 * Respond to a ServerAbort message from the server.
 * Always returns false.
 */
static bool
handle_server_abort(char *errmsg, struct client_closure *closure)
{
    debug_decl(handle_server_abort, SUDO_DEBUG_UTIL);

    sudo_warnx(U_("abort message received from server: %s"), errmsg);
    debug_return_bool(false);
}

/*
 * Respond to a ServerMessage from the server.
 * Returns true on success, false on error.
 */
static bool
handle_server_message(uint8_t *buf, size_t len,
    struct client_closure *closure)
{
    ServerMessage *msg;
    bool ret = false;
    debug_decl(handle_server_message, SUDO_DEBUG_UTIL);

    sudo_debug_printf(SUDO_DEBUG_INFO, "%s: unpacking ServerMessage", __func__);
    msg = server_message__unpack(NULL, len, buf);
    if (msg == NULL) {
	sudo_warnx(U_("unable to unpack %s size %zu"), "ServerMessage", len);
	debug_return_bool(false);
    }

    switch (msg->type_case) {
    case SERVER_MESSAGE__TYPE_HELLO:
	if ((ret = handle_server_hello(msg->u.hello, closure))) {
	    if (sudo_timespecisset(&closure->restart)) {
		closure->state = SEND_RESTART;
		ret = fmt_restart_message(closure);
	    } else if (closure->reject_reason != NULL) {
		closure->state = SEND_REJECT;
		ret = fmt_reject_message(closure);
            } else {
		closure->state = SEND_ACCEPT;
		ret = fmt_accept_message(closure);
	    }
	}
	break;
    case SERVER_MESSAGE__TYPE_COMMIT_POINT:
	ret = handle_commit_point(msg->u.commit_point, closure);
	if (sudo_timespeccmp(&closure->elapsed, &closure->committed, ==)) {
	    sudo_ev_del(closure->evbase, closure->read_ev);
	    closure->state = FINISHED;
	    if (++finished_transmissions == nr_of_conns)
	        sudo_ev_loopexit(closure->evbase);
	}
	break;
    case SERVER_MESSAGE__TYPE_LOG_ID:
	ret = handle_log_id(msg->u.log_id, closure);
	break;
    case SERVER_MESSAGE__TYPE_ERROR:
	ret = handle_server_error(msg->u.error, closure);
	closure->state = ERROR;
	break;
    case SERVER_MESSAGE__TYPE_ABORT:
	ret = handle_server_abort(msg->u.abort, closure);
	closure->state = ERROR;
	break;
    default:
	sudo_warnx(U_("%s: unexpected type_case value %d"),
	    __func__, msg->type_case);
	break;
    }

    server_message__free_unpacked(msg, NULL);
    debug_return_bool(ret);
}

/*
 * Read and unpack a ServerMessage (read callback).
 */
static void
server_msg_cb(int fd, int what, void *v)
{
    struct client_closure *closure = v;
    struct connection_buffer *buf = &closure->read_buf;
    size_t nread;
    uint32_t msg_len;
    debug_decl(server_msg_cb, SUDO_DEBUG_UTIL);

    /* For TLS we may need to read as part of SSL_write_ex(). */
    if (closure->write_instead_of_read) {
	closure->write_instead_of_read = false;
        client_msg_cb(fd, what, v);
        debug_return;
    }

    if (what == SUDO_EV_TIMEOUT) {
        sudo_warnx("%s", U_("timeout reading from server"));
        goto bad;
    }

#if defined(HAVE_OPENSSL)
    if (cert != NULL) {
	SSL *ssl = closure->tls_client.ssl;
	int result;
	sudo_debug_printf(SUDO_DEBUG_INFO, "%s: reading ServerMessage (TLS)", __func__);
        result = SSL_read_ex(ssl, buf->data + buf->len, buf->size - buf->len,
	    &nread);
        if (result <= 0) {
	    unsigned long errcode;
	    const char *errstr;

            switch (SSL_get_error(ssl, result)) {
		case SSL_ERROR_ZERO_RETURN:
		    /* ssl connection shutdown cleanly */
		    nread = 0;
		    break;
                case SSL_ERROR_WANT_READ:
                    /* ssl wants to read more, read event is always active */
		    sudo_debug_printf(SUDO_DEBUG_NOTICE|SUDO_DEBUG_LINENO,
			"SSL_read_ex returns SSL_ERROR_WANT_READ");
                    debug_return;
                case SSL_ERROR_WANT_WRITE:
                    /* ssl wants to write, schedule a write if not pending */
		    sudo_debug_printf(SUDO_DEBUG_NOTICE|SUDO_DEBUG_LINENO,
			"SSL_read_ex returns SSL_ERROR_WANT_WRITE");
		    if (!sudo_ev_pending(closure->write_ev, SUDO_EV_WRITE, NULL)) {
			/* Enable a temporary write event. */
			if (sudo_ev_add(closure->evbase, closure->write_ev, NULL, false) == -1) {
			    sudo_warnx("%s", U_("unable to add event to queue"));
			    goto bad;
			}
			closure->temporary_write_event = true;
		    }
		    /* Redirect write event to finish SSL_read_ex() */
		    closure->read_instead_of_write = true;
                    debug_return;
                case SSL_ERROR_SSL:
                    /*
                     * For TLS 1.3, if the cert verify function on the server
                     * returns an error, OpenSSL will send an internal error
                     * alert when we read ServerHello.  Convert to a more useful
                     * message and hope that no actual internal error occurs.
                     */
                    errcode = ERR_get_error();
#if !defined(HAVE_WOLFSSL)
                    if (closure->state == RECV_HELLO &&
                        ERR_GET_REASON(errcode) == SSL_R_TLSV1_ALERT_INTERNAL_ERROR) {
                        errstr = U_("host name does not match certificate");
                    } else
#endif
		    {
                        errstr = ERR_reason_error_string(errcode);
                    }
                    sudo_warnx("%s", errstr ? errstr : strerror(errno));
                    goto bad;
                case SSL_ERROR_SYSCALL:
                    sudo_warn("SSL_read_ex");
                    goto bad;
                default:
                    errstr = ERR_reason_error_string(ERR_get_error());
                    sudo_warnx("SSL_read_ex: %s",
			errstr ? errstr : strerror(errno));
                    goto bad;
            }
        }
    } else
#endif
    {
	sudo_debug_printf(SUDO_DEBUG_INFO, "%s: reading ServerMessage", __func__);
	nread = (size_t)read(fd, buf->data + buf->len, buf->size - buf->len);
    }
    sudo_debug_printf(SUDO_DEBUG_INFO, "%s: received %zd bytes from server",
	__func__, nread);
    switch (nread) {
    case (size_t)-1:
	if (errno == EAGAIN || errno == EINTR)
	    debug_return;
	sudo_warn("read");
	goto bad;
    case 0:
	if (closure->state != FINISHED)
	    sudo_warnx("%s", U_("premature EOF"));
	goto bad;
    default:
	break;
    }
    buf->len += nread;

    while (buf->len - buf->off >= sizeof(msg_len)) {
	/* Read wire message size (uint32_t in network byte order). */
	memcpy(&msg_len, buf->data + buf->off, sizeof(msg_len));
	msg_len = ntohl(msg_len);

	if (msg_len > MESSAGE_SIZE_MAX) {
	    sudo_warnx(U_("server message too large: %u"), msg_len);
	    goto bad;
	}

	if (msg_len + sizeof(msg_len) > buf->len - buf->off) {
	    /* Incomplete message, we'll read the rest next time. */
	    if (!expand_buf(buf, msg_len + sizeof(msg_len)))
		    goto bad;
	    debug_return;
	}

	/* Parse ServerMessage, could be zero bytes. */
	sudo_debug_printf(SUDO_DEBUG_INFO,
	    "%s: parsing ServerMessage, size %u", __func__, msg_len);
	buf->off += sizeof(msg_len);
	if (!handle_server_message(buf->data + buf->off, msg_len, closure))
	    goto bad;
	buf->off += msg_len;
    }
    buf->len -= buf->off;
    buf->off = 0;
    debug_return;
bad:
    sudo_ev_del(closure->evbase, closure->read_ev);
    debug_return;
}

/*
 * Send a ClientMessage to the server (write callback).
 */
static void
client_msg_cb(int fd, int what, void *v)
{
    struct client_closure *closure = v;
    struct connection_buffer *buf;
    size_t nwritten;
    debug_decl(client_msg_cb, SUDO_DEBUG_UTIL);

    if ((buf = TAILQ_FIRST(&closure->write_bufs)) == NULL) {
	sudo_warnx(U_("missing write buffer for client %s"), "localhost");
	goto bad;
    }

    /* For TLS we may need to write as part of SSL_read_ex(). */
    if (closure->read_instead_of_write) {
	closure->read_instead_of_write = false;
        /* Delete write event if it was only due to SSL_read_ex(). */
        if (closure->temporary_write_event) {
            closure->temporary_write_event = false;
            sudo_ev_del(closure->evbase, closure->write_ev);
        }
        server_msg_cb(fd, what, v);
        debug_return;
    }

    if (what == SUDO_EV_TIMEOUT) {
        sudo_warnx("%s", U_("timeout writing to server"));
        goto bad;
    }

    sudo_debug_printf(SUDO_DEBUG_INFO,
    	"%s: sending %zu bytes to server", __func__, buf->len - buf->off);

#if defined(HAVE_OPENSSL)
    if (cert != NULL) {
	SSL *ssl = closure->tls_client.ssl;
        const int result = SSL_write_ex(ssl, buf->data + buf->off,
	    buf->len - buf->off, &nwritten);
        if (result <= 0) {
	    const char *errstr;

            switch (SSL_get_error(ssl, result)) {
		case SSL_ERROR_ZERO_RETURN:
		    /* ssl connection shutdown */
		    goto bad;
                case SSL_ERROR_WANT_READ:
                    /* ssl wants to read, read event always active */
		    sudo_debug_printf(SUDO_DEBUG_NOTICE|SUDO_DEBUG_LINENO,
			"SSL_write_ex returns SSL_ERROR_WANT_READ");
		    /* Redirect read event to finish SSL_write_ex() */
		    closure->write_instead_of_read = true;
                    debug_return;
                case SSL_ERROR_WANT_WRITE:
		    /* ssl wants to write more, write event remains active */
		    sudo_debug_printf(SUDO_DEBUG_NOTICE|SUDO_DEBUG_LINENO,
			"SSL_write_ex returns SSL_ERROR_WANT_WRITE");
                    debug_return;
                case SSL_ERROR_SYSCALL:
                    sudo_warn("SSL_write_ex");
                    goto bad;
                default:
		    errstr = ERR_reason_error_string(ERR_get_error());
		    sudo_warnx("SSL_write_ex: %s",
			errstr ? errstr : strerror(errno));
                    goto bad;
            }
        }
    } else
#endif
    {
	nwritten = (size_t)write(fd, buf->data + buf->off, buf->len - buf->off);
    }
    if (nwritten == (size_t)-1) {
	if (errno == EAGAIN || errno == EINTR)
	    debug_return;
	sudo_warn("write");
	goto bad;
    }
    buf->off += nwritten;

    if (buf->off == buf->len) {
	/* sent entire message */
	sudo_debug_printf(SUDO_DEBUG_INFO,
	    "%s: finished sending %zu bytes to server", __func__, buf->len);
	buf->off = 0;
	buf->len = 0;
	TAILQ_REMOVE(&closure->write_bufs, buf, entries);
	TAILQ_INSERT_TAIL(&closure->free_bufs, buf, entries);
	if (TAILQ_EMPTY(&closure->write_bufs)) {
	    /* Write queue empty, check state. */
	    if (!client_message_completion(closure))
		goto bad;
	}
    }
    debug_return;

bad:
    sudo_ev_del(closure->evbase, closure->read_ev);
    sudo_ev_del(closure->evbase, closure->write_ev);
    debug_return;
}

/*
 * Parse a timespec on the command line of the form
 * seconds[,nanoseconds]
 */
static bool
parse_timespec(struct timespec *ts, char *strval)
{
    const char *errstr;
    char *nsecstr;
    debug_decl(parse_timespec, SUDO_DEBUG_UTIL);

    if ((nsecstr = strchr(strval, ',')) != NULL)
	*nsecstr++ = '\0';

    ts->tv_nsec = 0;
    ts->tv_sec = (time_t)sudo_strtonum(strval, 0, TIME_T_MAX, &errstr);
    if (errstr != NULL) {
	sudo_warnx(U_("%s: %s"), strval, U_(errstr));
	debug_return_bool(false);
    }

    if (nsecstr != NULL) {
	ts->tv_nsec = (long)sudo_strtonum(nsecstr, 0, LONG_MAX, &errstr);
	if (errstr != NULL) {
	    sudo_warnx(U_("%s: %s"), nsecstr, U_(errstr));
	    debug_return_bool(false);
	}
    }

    sudo_debug_printf(SUDO_DEBUG_INFO, "%s: parsed timespec [%lld, %ld]",
	__func__, (long long)ts->tv_sec, ts->tv_nsec);
    debug_return_bool(true);
}

/*
 * Free client closure contents.
 */
static void
client_closure_free(struct client_closure *closure)
{
    struct connection_buffer *buf;
    debug_decl(connection_closure_free, SUDO_DEBUG_UTIL);

    if (closure != NULL) {
	TAILQ_REMOVE(&connections, closure, entries);
#if defined(HAVE_OPENSSL)
        if (closure->tls_client.ssl != NULL) {
            if (SSL_shutdown(closure->tls_client.ssl) == 0)
		SSL_shutdown(closure->tls_client.ssl);
            SSL_free(closure->tls_client.ssl);
        }
	sudo_ev_free(closure->tls_client.tls_connect_ev);
#endif
        sudo_ev_free(closure->read_ev);
        sudo_ev_free(closure->write_ev);
        free(closure->read_buf.data);
        free(closure->buf);
	while ((buf = TAILQ_FIRST(&closure->write_bufs)) != NULL) {
	    sudo_debug_printf(SUDO_DEBUG_WARN|SUDO_DEBUG_LINENO,
		"discarding write buffer %p, len %zu", buf, buf->len - buf->off);
	    TAILQ_REMOVE(&closure->write_bufs, buf, entries);
	    free(buf->data);
	    free(buf);
	}
	while ((buf = TAILQ_FIRST(&closure->free_bufs)) != NULL) {
	    TAILQ_REMOVE(&closure->free_bufs, buf, entries);
	    free(buf->data);
	    free(buf);
	}
	shutdown(closure->sock, SHUT_RDWR);
        close(closure->sock);
        free(closure);
    }

    debug_return;
}

/*
 * Initialize a new client closure
 */
static struct client_closure *
client_closure_alloc(int sock, struct sudo_event_base *base,
    struct timespec *restart, struct timespec *stop_after, const char *iolog_id,
    char *reject_reason, bool accept_only, struct eventlog *evlog)
{
    struct connection_buffer *buf;
    struct client_closure *closure;
    debug_decl(client_closure_alloc, SUDO_DEBUG_UTIL);

    if ((closure = calloc(1, sizeof(*closure))) == NULL)
	debug_return_ptr(NULL);

    closure->sock = sock;
    closure->evbase = base;
    TAILQ_INIT(&closure->write_bufs);
    TAILQ_INIT(&closure->free_bufs);

    TAILQ_INSERT_TAIL(&connections, closure, entries);

    closure->state = RECV_HELLO;
    closure->accept_only = accept_only;
    closure->reject_reason = reject_reason;
    closure->evlog = evlog;

    closure->restart.tv_sec = restart->tv_sec;
    closure->restart.tv_nsec = restart->tv_nsec;
    closure->stop_after.tv_sec = stop_after->tv_sec;
    closure->stop_after.tv_nsec = stop_after->tv_nsec;

    closure->iolog_id = iolog_id;

    closure->read_buf.size = 8 * 1024;
    closure->read_buf.data = malloc(closure->read_buf.size);
    if (closure->read_buf.data == NULL)
	goto bad;

    closure->read_ev = sudo_ev_alloc(sock, SUDO_EV_READ|SUDO_EV_PERSIST,
	server_msg_cb, closure);
    if (closure->read_ev == NULL)
	goto bad;

    buf = get_free_buf(64 * 1024, closure);
    if (buf == NULL)
	goto bad;
    TAILQ_INSERT_TAIL(&closure->free_bufs, buf, entries);

    closure->write_ev = sudo_ev_alloc(sock, SUDO_EV_WRITE|SUDO_EV_PERSIST,
	client_msg_cb, closure);
    if (closure->write_ev == NULL)
	goto bad;

#if defined(HAVE_OPENSSL)
    if (cert != NULL) {
	closure->tls_client.tls_connect_ev = sudo_ev_alloc(sock, SUDO_EV_WRITE,
	    tls_connect_cb, &closure->tls_client);
	if (closure->tls_client.tls_connect_ev == NULL)
	    goto bad;
	closure->tls_client.evbase = base;
	closure->tls_client.parent_closure = closure;
	closure->tls_client.peer_name = &server_info;
	closure->tls_client.connect_timeout.tv_sec = TLS_HANDSHAKE_TIMEO_SEC;
	closure->tls_client.start_fn = tls_start_fn;
    }
#endif

    debug_return_ptr(closure);
bad:
    client_closure_free(closure);
    debug_return_ptr(NULL);
}

#if defined(HAVE_OPENSSL)
static const char short_opts[] = "Ah:i:np:r:R:s:t:b:c:k:V";
#else
static const char short_opts[] = "Ah:i:Ip:r:R:t:s:V";
#endif
static struct option long_opts[] = {
    { "accept",		no_argument,		NULL,	'A' },
    { "help",		no_argument,		NULL,	1 },
    { "host",		required_argument,	NULL,	'h' },
    { "iolog-id",	required_argument,	NULL,	'i' },
    { "port",		required_argument,	NULL,	'p' },
    { "restart",	required_argument,	NULL,	'r' },
    { "reject",		required_argument,	NULL,	'R' },
    { "stop-after",	required_argument,	NULL,	's' },
    { "test",	    	optional_argument,	NULL,	't' },
#if defined(HAVE_OPENSSL)
    { "ca-bundle",	required_argument,	NULL,	'b' },
    { "cert",		required_argument,	NULL,	'c' },
    { "key",		required_argument,	NULL,	'k' },
    { "no-verify",	no_argument,		NULL,	'n' },
#endif
    { "version",	no_argument,		NULL,	'V' },
    { NULL,		no_argument,		NULL,	0 },
};

sudo_dso_public int main(int argc, char *argv[]);

int
main(int argc, char *argv[])
{
    struct client_closure *closure = NULL;
    struct sudo_event_base *evbase;
    struct eventlog *evlog;
    const char *port = NULL;
    struct timespec restart = { 0, 0 };
    struct timespec stop_after = { 0, 0 };
    bool accept_only = false;
    char *reject_reason = NULL;
    const char *iolog_id = NULL;
    const char *open_mode = "r";
    const char *errstr;
    int ch, sock, iolog_dir_fd, finished;
    debug_decl_vars(main, SUDO_DEBUG_MAIN);

#if defined(SUDO_DEVEL) && defined(__OpenBSD__)
    {
	extern char *malloc_options;
	malloc_options = "S";
    }
#endif

    signal(SIGPIPE, SIG_IGN);

    initprogname(argc > 0 ? argv[0] : "sudo_sendlog");
    setlocale(LC_ALL, "");
    bindtextdomain("sudo", LOCALEDIR); /* XXX - add logsrvd domain */
    textdomain("sudo");

    /* Read sudo.conf and initialize the debug subsystem. */
    if (sudo_conf_read(NULL, SUDO_CONF_DEBUG) == -1)
        return EXIT_FAILURE;
    sudo_debug_register(getprogname(), NULL, NULL,
        sudo_conf_debug_files(getprogname()), -1);

    if (protobuf_c_version_number() < 1003000)
	sudo_fatalx("%s", U_("Protobuf-C version 1.3 or higher required"));

    while ((ch = getopt_long(argc, argv, short_opts, long_opts, NULL)) != -1) {
	switch (ch) {
	case 'A':
	    accept_only = true;
	    break;
	case 'h':
	    server_info.name = optarg;
	    break;
	case 'i':
	    iolog_id = optarg;
	    break;
	case 'p':
	    port = optarg;
	    break;
	case 'R':
	    reject_reason = optarg;
	    break;
	case 'r':
	    if (!parse_timespec(&restart, optarg))
		goto bad;
	    open_mode = "r+";
	    break;
	case 's':
	    if (!parse_timespec(&stop_after, optarg))
		goto bad;
	    break;
	case 't':
	    nr_of_conns = (int)sudo_strtonum(optarg, 1, INT_MAX, &errstr);
	    if (errstr != NULL) {
		sudo_warnx(U_("%s: %s"), optarg, U_(errstr));
		goto bad;
	    }
	    testrun = true;
	    break;
	case 1:
	    help();
	    /* NOTREACHED */
#if defined(HAVE_OPENSSL)
	case 'b':
	    ca_bundle = optarg;
	    break;
	case 'c':
	    cert = optarg;
	    break;
	case 'k':
	    key = optarg;
	    break;
	case 'n':
	    verify_server = false;
	    break;
#endif
	case 'V':
	    (void)printf(_("%s version %s\n"), getprogname(),
		PACKAGE_VERSION);
	    return 0;
	default:
	    usage();
	    /* NOTREACHED */
	}
    }
    argc -= optind;
    argv += optind;

#if defined(HAVE_OPENSSL)
    /* if no key file is given explicitly, try to load the key from the cert */
    if (cert != NULL) {
	if (key == NULL)
	    key = cert;
	if (port == NULL)
	    port = DEFAULT_PORT_TLS;
    }
#endif
    if (port == NULL)
	port = DEFAULT_PORT;

    if (sudo_timespecisset(&restart) != (iolog_id != NULL)) {
	sudo_warnx("%s", U_("both restart point and iolog ID must be specified"));
	usage();
    }
    if (sudo_timespecisset(&restart) && (accept_only || reject_reason)) {
	sudo_warnx("%s", U_("a restart point may not be set when no I/O is sent"));
	usage();
    }

    /* Remaining arg should be to I/O log dir to send. */
    if (argc != 1)
	usage();
    iolog_dir = argv[0];
    if ((iolog_dir_fd = open(iolog_dir, O_RDONLY)) == -1) {
	sudo_warn("%s", iolog_dir);
	goto bad;
    }

    /* Parse I/O log info file. */
    if ((evlog = iolog_parse_loginfo(iolog_dir_fd, iolog_dir)) == NULL)
	goto bad;

    if ((evbase = sudo_ev_base_alloc()) == NULL)
	sudo_fatal(U_("%s: %s"), __func__, U_("unable to allocate memory"));

    if (testrun)
        printf("connecting clients...\n");

    for (int i = 0; i < nr_of_conns; i++) {
        sock = connect_server(&server_info, port);
        if (sock == -1)
            goto bad;
        
        if (!testrun)
            printf("Connected to %s:%s\n", server_info.name, port);

        closure = client_closure_alloc(sock, evbase, &restart, &stop_after,
	    iolog_id, reject_reason, accept_only, evlog);
        if (closure == NULL)
            goto bad;

        /* Open the I/O log files and seek to restart point if there is one. */
        if (!iolog_open_all(iolog_dir_fd, iolog_dir, closure->iolog_files, open_mode))
            goto bad;
        if (sudo_timespecisset(&closure->restart)) {
            if (!iolog_seekto(iolog_dir_fd, iolog_dir, closure->iolog_files,
		    &closure->elapsed, &closure->restart))
                goto bad;
        }

#if defined(HAVE_OPENSSL)
	if (cert != NULL) {
	    if (!tls_client_setup(closure->sock, ca_bundle, cert, key, NULL,
		    NULL, NULL, verify_server, false, &closure->tls_client))
		goto bad;
	} else
#endif
	{
	    /* No TLS, send ClientHello */
	    if (!fmt_client_hello(closure))
		goto bad;
	}
    }  

    if (testrun)
        puts("sending logs...");

    struct timespec t_start, t_end, t_result;
    sudo_gettime_real(&t_start);

    sudo_ev_dispatch(evbase);
    sudo_ev_base_free(evbase);

    sudo_gettime_real(&t_end);
    sudo_timespecsub(&t_end, &t_start, &t_result);

    finished = 0;
    while ((closure = TAILQ_FIRST(&connections)) != NULL) {
        if (closure->state == FINISHED) {
	    finished++;
	} else {
            sudo_warnx(U_("exited prematurely with state %d"), closure->state);
            sudo_warnx(U_("elapsed time sent to server [%lld, %ld]"),
                (long long)closure->elapsed.tv_sec, closure->elapsed.tv_nsec);
            sudo_warnx(U_("commit point received from server [%lld, %ld]"),
                (long long)closure->committed.tv_sec, closure->committed.tv_nsec);
        }
        client_closure_free(closure);
    }
    eventlog_free(evlog);
#if defined(HAVE_OPENSSL)
    SSL_CTX_free(ssl_ctx);
#endif

    if (finished != 0) {
        printf("%d I/O log%s transmitted successfully in %lld.%.9ld seconds\n",
	    finished, nr_of_conns > 1 ? "s" : "",
            (long long)t_result.tv_sec, t_result.tv_nsec);
        debug_return_int(EXIT_SUCCESS);
    }

bad:
    debug_return_int(EXIT_FAILURE);
}
