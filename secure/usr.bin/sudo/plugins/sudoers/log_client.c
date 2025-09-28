/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2019-2022 Todd C. Miller <Todd.Miller@sudo.ws>
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

#ifdef SUDOERS_LOG_CLIENT

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <netdb.h>
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
#include <pwd.h>
#include <grp.h>
#ifndef HAVE_GETADDRINFO
# include <compat/getaddrinfo.h>
#endif

#if defined(HAVE_OPENSSL)
# if defined(HAVE_WOLFSSL)
#  include <wolfssl/options.h>
# endif
# include <openssl/ssl.h>
# include <openssl/err.h>
# include <openssl/x509v3.h>
#endif /* HAVE_OPENSSL */

#define NEED_INET_NTOP		/* to expose sudo_inet_ntop in sudo_compat.h */

#include <sudoers.h>
#include <sudo_event.h>
#include <sudo_eventlog.h>
#include <sudo_iolog.h>
#include <hostcheck.h>
#include <log_client.h>
#include <strlist.h>

/* Shared between iolog.c and audit.c */
struct client_closure *client_closure;

/* Server callback may redirect to client callback for TLS. */
static void client_msg_cb(int fd, int what, void *v);
static void server_msg_cb(int fd, int what, void *v);

static void
connect_cb(int sock, int what, void *v)
{
    int optval, ret, *errnump = v;
    socklen_t optlen = sizeof(optval);
    debug_decl(connect_cb, SUDOERS_DEBUG_UTIL);

    if (what == SUDO_PLUGIN_EV_TIMEOUT) {
	*errnump = ETIMEDOUT;
    } else {
	ret = getsockopt(sock, SOL_SOCKET, SO_ERROR, &optval, &optlen);
	*errnump = ret == 0 ? optval : errno;
    }

    debug_return;
}

/*
 * Like connect(2) but with a timeout.
 */
static int
timed_connect(int sock, const struct sockaddr *addr, socklen_t addrlen,
    const struct timespec *timeout)
{
    struct sudo_event_base *evbase = NULL;
    struct sudo_event *connect_event = NULL;
    int ret, errnum = 0;
    debug_decl(timed_connect, SUDOERS_DEBUG_UTIL);

    ret = connect(sock, addr, addrlen);
    if (ret == -1 && errno == EINPROGRESS) {
	evbase = sudo_ev_base_alloc();
	connect_event = sudo_ev_alloc(sock, SUDO_PLUGIN_EV_WRITE, connect_cb,
	    &errnum);
	if (evbase == NULL || connect_event == NULL) {
	    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	    goto done;
	}
	if (sudo_ev_add(evbase, connect_event, timeout, false) == -1) {
	    sudo_warnx("%s", U_("unable to add event to queue"));
	    goto done;
	}
	if (sudo_ev_dispatch(evbase) == -1) {
	    sudo_warn("%s", U_("error in event loop"));
	    goto done;
	}
	if (errnum == 0)
	    ret = 0;
	else
	    errno = errnum;
    }

done:
    sudo_ev_base_free(evbase);
    sudo_ev_free(connect_event);

    debug_return_int(ret);
}

#if defined(HAVE_OPENSSL)
static int
verify_peer_identity(int preverify_ok, X509_STORE_CTX *ctx)
{
    HostnameValidationResult result;
    struct client_closure *closure;
    SSL *ssl;
    X509 *current_cert;
    X509 *peer_cert;
    debug_decl(verify_peer_identity, SUDOERS_DEBUG_UTIL);

    /* if pre-verification of the cert failed, just propagate that result back */
    if (preverify_ok != 1) {
        debug_return_int(0);
    }

    /* since this callback is called for each cert in the chain,
     * check that current cert is the peer's certificate
     */
    current_cert = X509_STORE_CTX_get_current_cert(ctx);
    peer_cert = X509_STORE_CTX_get0_cert(ctx);

    if (current_cert != peer_cert) {
        debug_return_int(1);
    }

    /* read out the attached object (closure) from the ssl connection object */
    ssl = X509_STORE_CTX_get_ex_data(ctx, SSL_get_ex_data_X509_STORE_CTX_idx());
    closure = SSL_get_ex_data(ssl, 1);

    result = validate_hostname(peer_cert, closure->server_name,
	closure->server_ip, 0);

    switch(result)
    {
        case MatchFound:
            debug_return_int(1);
        default:
            debug_return_int(0);
    }
}

static bool
tls_init(struct client_closure *closure)
{
    const char *errstr;
    debug_decl(tls_init, SUDOERS_DEBUG_PLUGIN);

    /* Only attempt to initialize TLS once, the parameters don't change. */
    if (closure->ssl_initialized) {
        if (closure->ssl == NULL)
            debug_return_bool(false);
        SSL_clear(closure->ssl);
        debug_return_bool(true);
    }

    closure->ssl_initialized = true;
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    
    /* Create the ssl context and enforce TLS 1.2 or higher. */
    if ((closure->ssl_ctx = SSL_CTX_new(TLS_method())) == NULL) {
        errstr = ERR_reason_error_string(ERR_get_error());
        sudo_warnx(U_("Creation of new SSL_CTX object failed: %s"),
	    errstr ? errstr : strerror(errno));
        goto bad;
    }
#ifdef HAVE_SSL_CTX_SET_MIN_PROTO_VERSION
    if (!SSL_CTX_set_min_proto_version(closure->ssl_ctx, TLS1_2_VERSION)) {
        errstr = ERR_reason_error_string(ERR_get_error());
        sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
            "unable to restrict min. protocol version: %s",
	    errstr ? errstr : strerror(errno));
        goto bad;
    }
#else
    SSL_CTX_set_options(closure->ssl_ctx,
        SSL_OP_NO_SSLv2|SSL_OP_NO_SSLv3|SSL_OP_NO_TLSv1|SSL_OP_NO_TLSv1_1);
#endif

    /* Enable server cert verification if log_server_verify is set in sudoers */
    if (closure->log_details->verify_server) {
        if (closure->log_details->ca_bundle != NULL) {
            if (SSL_CTX_load_verify_locations(closure->ssl_ctx,
                closure->log_details->ca_bundle, NULL) <= 0) {
                errstr = ERR_reason_error_string(ERR_get_error());
                sudo_warnx(U_("%s: %s"), closure->log_details->ca_bundle,
		    errstr ? errstr : strerror(errno));
                sudo_warnx(U_("unable to load certificate authority bundle %s"),
                    closure->log_details->ca_bundle);
                goto bad;
            }
        } else {
            if (!SSL_CTX_set_default_verify_paths(closure->ssl_ctx)) {
                errstr = ERR_reason_error_string(ERR_get_error());
                sudo_warnx("SSL_CTX_set_default_verify_paths: %s",
		    errstr ? errstr : strerror(errno));
                goto bad;
            }
        }
        SSL_CTX_set_verify(closure->ssl_ctx, SSL_VERIFY_PEER, verify_peer_identity);
    }

    /* Load the client certificate file if it is set in sudoers. */
    if (closure->log_details->cert_file != NULL) {
        if (!SSL_CTX_use_certificate_chain_file(closure->ssl_ctx,
                closure->log_details->cert_file)) {
            errstr = ERR_reason_error_string(ERR_get_error());
	    sudo_warnx(U_("%s: %s"), closure->log_details->cert_file,
		errstr ? errstr : strerror(errno));
	    sudo_warnx(U_("unable to load certificate %s"),
		closure->log_details->cert_file);
            goto bad;
        }
        if (closure->log_details->key_file == NULL) {
            /* No explicit key file set, try to use the cert file. */
            closure->log_details->key_file = closure->log_details->cert_file;
        }
        if (!SSL_CTX_use_PrivateKey_file(closure->ssl_ctx,
                closure->log_details->key_file, SSL_FILETYPE_PEM) ||
                !SSL_CTX_check_private_key(closure->ssl_ctx)) {
            errstr = ERR_reason_error_string(ERR_get_error());
	    sudo_warnx(U_("%s: %s"), closure->log_details->key_file,
		errstr ? errstr : strerror(errno));
	    sudo_warnx(U_("unable to load private key %s"),
		closure->log_details->key_file);
            goto bad;
        }
    }

    /* Create the SSL object and attach the closure. */
    if ((closure->ssl = SSL_new(closure->ssl_ctx)) == NULL) {
        errstr = ERR_reason_error_string(ERR_get_error());
        sudo_warnx(U_("Unable to allocate ssl object: %s"),
	    errstr ? errstr : strerror(errno));
        goto bad;
    }
    if (SSL_set_ex_data(closure->ssl, 1, closure) <= 0) {
        errstr = ERR_reason_error_string(ERR_get_error());
        sudo_warnx(U_("Unable to attach user data to the ssl object: %s"),
	    errstr ? errstr : strerror(errno));
        goto bad;
    }

    debug_return_bool(true);

bad:
    SSL_free(closure->ssl);
    closure->ssl = NULL;
    SSL_CTX_free(closure->ssl_ctx);
    closure->ssl_ctx = NULL;
    debug_return_bool(false);
}

struct tls_connect_closure {
    bool tls_conn_status;
    SSL *ssl;
    const char *host;
    const char *port;
    const struct timespec *timeout;
    struct sudo_event_base *evbase;
    struct sudo_event *tls_connect_ev;
};

static void
tls_connect_cb(int sock, int what, void *v)
{
    struct tls_connect_closure *closure = v;
    const struct timespec *timeout = closure->timeout;
    int tls_con;
    debug_decl(tls_connect_cb, SUDOERS_DEBUG_UTIL);

    if (what == SUDO_PLUGIN_EV_TIMEOUT) {
        sudo_warnx("%s", U_("TLS handshake timeout occurred"));
        goto bad;
    }

    tls_con = SSL_connect(closure->ssl);

    if (tls_con == 1) {
        sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
            "TLS version: %s, negotiated cipher suite: %s",
            SSL_get_version(closure->ssl), SSL_get_cipher(closure->ssl));
        closure->tls_conn_status = true;
    } else {
	const char *errstr;

        switch (SSL_get_error(closure->ssl, tls_con)) {
	    /* TLS handshake is not finished, reschedule event */
            case SSL_ERROR_WANT_READ:
		sudo_debug_printf(SUDO_DEBUG_NOTICE|SUDO_DEBUG_LINENO,
		    "SSL_connect returns SSL_ERROR_WANT_READ");
		if (what != SUDO_EV_READ) {
		    if (sudo_ev_set(closure->tls_connect_ev, sock,
			    SUDO_EV_READ, tls_connect_cb, closure) == -1) {
			sudo_warnx("%s", U_("unable to set event"));
			goto bad;
		    }
		}
		if (sudo_ev_add(closure->evbase, closure->tls_connect_ev,
			timeout, false) == -1) {
                    sudo_warnx("%s", U_("unable to add event to queue"));
		    goto bad;
                }
		break;
            case SSL_ERROR_WANT_WRITE:
		sudo_debug_printf(SUDO_DEBUG_NOTICE|SUDO_DEBUG_LINENO,
		    "SSL_connect returns SSL_ERROR_WANT_WRITE");
		if (what != SUDO_EV_WRITE) {
		    if (sudo_ev_set(closure->tls_connect_ev, sock,
			    SUDO_EV_WRITE, tls_connect_cb, closure) == -1) {
			sudo_warnx("%s", U_("unable to set event"));
			goto bad;
		    }
		}
		if (sudo_ev_add(closure->evbase, closure->tls_connect_ev,
			timeout, false) == -1) {
                    sudo_warnx("%s", U_("unable to add event to queue"));
		    goto bad;
                }
                break;
	    case SSL_ERROR_SYSCALL:
		sudo_warnx(U_("TLS connection to %s:%s failed: %s"),
		    closure->host, closure->port, strerror(errno));
		goto bad;
            default:
                errstr = ERR_reason_error_string(ERR_get_error());
		sudo_warnx(U_("TLS connection to %s:%s failed: %s"),
		    closure->host, closure->port,
		    errstr ? errstr : strerror(errno));
                goto bad;
        }
    }

    debug_return;

bad:
    /* Break out of tls connect event loop with an error. */
    sudo_ev_loopbreak(closure->evbase);

    debug_return;
}

static bool
tls_timed_connect(SSL *ssl, const char *host, const char *port,
    const struct timespec *timeout)
{
    struct tls_connect_closure closure;
    debug_decl(tls_timed_connect, SUDOERS_DEBUG_UTIL);

    memset(&closure, 0, sizeof(closure));
    closure.ssl = ssl;
    closure.host = host;
    closure.port = port;
    closure.timeout = timeout;
    closure.evbase = sudo_ev_base_alloc();
    closure.tls_connect_ev = sudo_ev_alloc(SSL_get_fd(ssl),
        SUDO_PLUGIN_EV_WRITE, tls_connect_cb, &closure);

    if (closure.evbase == NULL || closure.tls_connect_ev == NULL) {
        sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	goto done;
    }

    if (sudo_ev_add(closure.evbase, closure.tls_connect_ev, timeout, false) == -1) {
	sudo_warnx("%s", U_("unable to add event to queue"));
	goto done;
    }

    if (sudo_ev_dispatch(closure.evbase) == -1) {
	sudo_warnx("%s", U_("error in event loop"));
	goto done;
    }

done:
    if (closure.tls_connect_ev != NULL)
	sudo_ev_free(closure.tls_connect_ev);
    sudo_ev_base_free(closure.evbase);

    debug_return_bool(closure.tls_conn_status);
}
#endif /* HAVE_OPENSSL */

/*
 * Connect to specified host:port
 * If host has multiple addresses, the first one that connects is used.
 * Returns open socket or -1 on error.
 */
static int
connect_server(const char *host, const char *port, bool tls,
    struct client_closure *closure, const char **reason)
{
    const struct timespec *timeout = &closure->log_details->server_timeout;
    struct addrinfo hints, *res, *res0;
    const char *addr, *cause = NULL;
    int error, sock = -1;
    debug_decl(connect_server, SUDOERS_DEBUG_UTIL);

#if !defined(HAVE_OPENSSL)
    if (tls) {
        errno = EPROTONOSUPPORT;
        sudo_warn("%s:%s(tls)", host, port);
        debug_return_int(-1);
    }
#endif

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    error = getaddrinfo(host, port, &hints, &res0);
    if (error != 0) {
	sudo_warnx(U_("unable to look up %s:%s: %s"), host, port,
	    gai_strerror(error));
	debug_return_int(-1);
    }

    for (res = res0; res; res = res->ai_next) {
	int flags, save_errno;

	sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (sock == -1) {
	    cause = "socket";
	    continue;
	}
	flags = fcntl(sock, F_GETFL, 0);
	if (flags == -1 || fcntl(sock, F_SETFL, flags | O_NONBLOCK) == -1) {
	    cause = "fcntl(O_NONBLOCK)";
	    save_errno = errno;
	    close(sock);
	    errno = save_errno;
	    sock = -1;
	    continue;
	}
        if (fcntl(sock, F_SETFD, FD_CLOEXEC) == -1) {
	    cause = "fcntl(FD_CLOEXEC)";
	    save_errno = errno;
	    close(sock);
	    errno = save_errno;
	    sock = -1;
	    continue;
        }
        if (closure->log_details->keepalive) {
            flags = 1;
            if (setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &flags,
                    sizeof(flags)) == -1) {
                cause = "setsockopt(SO_KEEPALIVE)";
                save_errno = errno;
                close(sock);
                errno = save_errno;
                sock = -1;
                continue;
            }
        }
	if (timed_connect(sock, res->ai_addr, res->ai_addrlen, timeout) == -1) {
	    /* No need to set cause, caller's error message is sufficient. */
	    save_errno = errno;
	    close(sock);
	    errno = save_errno;
	    sock = -1;
	    continue;
	}
	switch (res->ai_family) {
	case AF_INET:
	    addr = (char *)&((struct sockaddr_in *)res->ai_addr)->sin_addr;
	    break;
#ifdef HAVE_STRUCT_IN6_ADDR
	case AF_INET6:
	    addr = (char *)&((struct sockaddr_in6 *)res->ai_addr)->sin6_addr;
	    break;
#endif
	default:
            cause = "ai_family";
	    save_errno = EAFNOSUPPORT;
	    shutdown(sock, SHUT_RDWR);
	    close(sock);
	    errno = save_errno;
	    sock = -1;
	    continue;
	}
        if (inet_ntop(res->ai_family, addr, closure->server_ip,
                sizeof(closure->server_ip)) == NULL) {
            cause = "inet_ntop";
	    save_errno = errno;
	    shutdown(sock, SHUT_RDWR);
	    close(sock);
	    errno = save_errno;
	    sock = -1;
	    continue;
        }
	free(closure->server_name);
	if ((closure->server_name = strdup(host)) == NULL) {
	    cause = "strdup";
	    save_errno = errno;
	    shutdown(sock, SHUT_RDWR);
	    close(sock);
	    errno = save_errno;
	    sock = -1;
	    continue;
	}

#if defined(HAVE_OPENSSL)
        if (tls) {
            if (!tls_init(closure) || !SSL_set_fd(closure->ssl, sock)) {
                cause = U_("TLS initialization was unsuccessful");
                save_errno = errno;
		shutdown(sock, SHUT_RDWR);
                close(sock);
                errno = save_errno;
                sock = -1;
                continue;
            }
            /* Perform TLS handshake. */
            if (!tls_timed_connect(closure->ssl, host, port, timeout)) {
                cause = U_("TLS handshake was unsuccessful");
                save_errno = errno;
		shutdown(sock, SHUT_RDWR);
                close(sock);
                errno = save_errno;
                sock = -1;
                continue;
            }
        } else {
            /* No TLS for this connection, make sure it is not initialized. */
            SSL_free(closure->ssl);
            closure->ssl = NULL;
            SSL_CTX_free(closure->ssl_ctx);
            closure->ssl_ctx = NULL;
        }
#endif /* HAVE_OPENSSL */
	break;	/* success */
    }
    freeaddrinfo(res0);

    if (sock == -1)
	*reason = cause;

    debug_return_int(sock);
}

/*
 * Connect to the first server in the list.
 * Stores socket in closure with O_NONBLOCK and close-on-exec flags set.
 * Returns true on success, else false.
 */
bool
log_server_connect(struct client_closure *closure)
{
    struct sudoers_string *server;
    char *host, *port, *copy = NULL;
    const char *cause = NULL;
    int sock;
    bool tls, ret = false;
    debug_decl(log_server_connect, SUDOERS_DEBUG_UTIL);

    STAILQ_FOREACH(server, closure->log_details->log_servers, entries) {
        free(copy);
	if ((copy = strdup(server->str)) == NULL)
                break;
	if (!iolog_parse_host_port(copy, &host, &port, &tls, DEFAULT_PORT,
		DEFAULT_PORT_TLS)) {
            sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
                "unable to parse %s", copy);
	    continue;
	}
        sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
            "connecting to %s port %s%s", host, port, tls ? " (tls)" : "");
	sock = connect_server(host, port, tls, closure, &cause);
	if (sock != -1) {
            if (closure->read_ev->set(closure->read_ev, sock,
                    SUDO_PLUGIN_EV_READ|SUDO_PLUGIN_EV_PERSIST,
                    server_msg_cb, closure) == -1) {
                cause = (U_("unable to add event to queue"));
                break;
            }

            if (closure->write_ev->set(closure->write_ev, sock,
                    SUDO_PLUGIN_EV_WRITE|SUDO_PLUGIN_EV_PERSIST,
                    client_msg_cb, closure) == -1) {
                cause = (U_("unable to add event to queue"));
                break;
            }

            /* success */
            closure->sock = sock;
            ret = true;
            break;
	}
    }
    free(copy);

    if (!ret && cause != NULL)
        sudo_warn("%s", cause);

    debug_return_bool(ret);
}

/*
 * Free client closure and contents, not including log details.
 */
void
client_closure_free(struct client_closure *closure)
{
    struct connection_buffer *buf;
    debug_decl(client_closure_free, SUDOERS_DEBUG_UTIL);

    if (closure == NULL)
        debug_return;

#if defined(HAVE_OPENSSL)
    /* Shut down the TLS connection cleanly and free SSL data. */
    if (closure->ssl != NULL) {
	if (SSL_shutdown(closure->ssl) == 0)
	    SSL_shutdown(closure->ssl);
	SSL_free(closure->ssl);
    }
    SSL_CTX_free(closure->ssl_ctx);
#endif

    if (closure->sock != -1) {
	shutdown(closure->sock, SHUT_RDWR);
	close(closure->sock);
    }
    free(closure->server_name);
    while ((buf = TAILQ_FIRST(&closure->write_bufs)) != NULL) {
	TAILQ_REMOVE(&closure->write_bufs, buf, entries);
	free(buf->data);
	free(buf);
    }
    while ((buf = TAILQ_FIRST(&closure->free_bufs)) != NULL) {
	TAILQ_REMOVE(&closure->free_bufs, buf, entries);
	free(buf->data);
	free(buf);
    }
    if (closure->read_ev != NULL)
	closure->read_ev->free(closure->read_ev);
    if (closure->write_ev != NULL)
	closure->write_ev->free(closure->write_ev);
    free(closure->read_buf.data);
    free(closure->iolog_id);

    free(closure);

    debug_return;
}

static struct connection_buffer *
get_free_buf(struct client_closure *closure)
{
    struct connection_buffer *buf;
    debug_decl(get_free_buf, SUDOERS_DEBUG_UTIL);

    buf = TAILQ_FIRST(&closure->free_bufs);
    if (buf != NULL)
	TAILQ_REMOVE(&closure->free_bufs, buf, entries);
    else
	buf = calloc(1, sizeof(*buf));

    debug_return_ptr(buf);
}

/*
 * Format a ClientMessage.
 * Appends the wire format message to the closure's write queue.
 * Returns true on success, false on failure.
 */
bool
fmt_client_message(struct client_closure *closure, ClientMessage *msg)
{
    struct connection_buffer *buf;
    uint32_t msg_len;
    bool ret = false;
    size_t len;
    debug_decl(fmt_client_message, SUDOERS_DEBUG_UTIL);

    if ((buf = get_free_buf(closure)) == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	goto done;
    }

    len = client_message__get_packed_size(msg);
    if (len > MESSAGE_SIZE_MAX) {
    	sudo_warnx(U_("client message too large: %zu"), len);
        goto done;
    }
    /* Wire message size is used for length encoding, precedes message. */
    msg_len = htonl((uint32_t)len);
    len += sizeof(msg_len);

    sudo_debug_printf(SUDO_DEBUG_INFO, "%s: new ClientMessage, %zu bytes",
	__func__, len);

    /* Resize buffer as needed. */
    if (len > buf->size) {
	const size_t new_size = sudo_pow2_roundup(len);
	if (new_size < len) {
	    /* overflow */
	    errno = ENOMEM;
	    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	    goto done;
	}
	free(buf->data);
	if ((buf->data = malloc(new_size)) == NULL) {
	    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	    goto done;
	}
	buf->size = new_size;
    }

    memcpy(buf->data, &msg_len, sizeof(msg_len));
    client_message__pack(msg, buf->data + sizeof(msg_len));
    buf->len = len;
    TAILQ_INSERT_TAIL(&closure->write_bufs, buf, entries);
    buf = NULL;

    ret = true;

done:
    if (buf != NULL) {
	free(buf->data);
	free(buf);
    }
    debug_return_bool(ret);
}

/*
 * Build and format a ClientHello wrapped in a ClientMessage.
 * Appends the wire format message to the closure's write queue.
 * Returns true on success, false on failure.
 */
static bool
fmt_client_hello(struct client_closure *closure)
{
    ClientMessage client_msg = CLIENT_MESSAGE__INIT;
    ClientHello hello_msg = CLIENT_HELLO__INIT;
    bool ret = false;
    debug_decl(fmt_client_hello, SUDOERS_DEBUG_UTIL);

    sudo_debug_printf(SUDO_DEBUG_INFO, "%s: sending ClientHello", __func__);

    /* Client name + version */
    hello_msg.client_id = (char *)"sudoers " PACKAGE_VERSION;

    /* Schedule ClientMessage */
    client_msg.u.hello_msg = &hello_msg;
    client_msg.type_case = CLIENT_MESSAGE__TYPE_HELLO_MSG;
    ret = fmt_client_message(closure, &client_msg);

    debug_return_bool(ret);
}

/*
 * Free an array of InfoMessage.
 * Does not free the actual contents, other than strlistval arrays.
 */
static void
free_info_messages(InfoMessage **info_msgs, size_t n)
{
    debug_decl(free_info_messages, SUDOERS_DEBUG_UTIL);

    if (info_msgs != NULL) {
	while (n-- != 0) {
	    /* A strlist array is dynamically allocated. */
	    if (info_msgs[n]->value_case == INFO_MESSAGE__VALUE_STRLISTVAL) {
		free(info_msgs[n]->u.strlistval);
	    }
	    free(info_msgs[n]);
	}
	free(info_msgs);
    }

    debug_return;
}

static InfoMessage **
fmt_info_messages(struct client_closure *closure, struct eventlog *evlog,
    size_t *n_info_msgs)
{
    InfoMessage__StringList *runargv = NULL;
    InfoMessage__StringList *runenv = NULL;
    InfoMessage__StringList *submitenv = NULL;
    InfoMessage **info_msgs = NULL;
    size_t info_msgs_size, n = 0;
    debug_decl(fmt_info_messages, SUDOERS_DEBUG_UTIL);

    /* Convert NULL-terminated vectors to StringList. */
    if (evlog->submitenv != NULL) {
	if ((submitenv = malloc(sizeof(*submitenv))) == NULL)
	    goto bad;
	info_message__string_list__init(submitenv);
	submitenv->strings = evlog->submitenv;
	while (submitenv->strings[submitenv->n_strings] != NULL)
	    submitenv->n_strings++;
    }

    if (evlog->runargv != NULL) {
	if ((runargv = malloc(sizeof(*runargv))) == NULL)
	    goto bad;
	info_message__string_list__init(runargv);
	runargv->strings = evlog->runargv;
	while (runargv->strings[runargv->n_strings] != NULL)
	    runargv->n_strings++;
    }

    if (evlog->runenv != NULL) {
	if ((runenv = malloc(sizeof(*runenv))) == NULL)
	    goto bad;
	info_message__string_list__init(runenv);
	runenv->strings = evlog->runenv;
	while (runenv->strings[runenv->n_strings] != NULL)
	    runenv->n_strings++;
    }

    /* XXX - realloc as needed instead of preallocating */
    info_msgs_size = 24;
    info_msgs = calloc(info_msgs_size, sizeof(InfoMessage *));
    if (info_msgs == NULL)
	goto bad;
    for (n = 0; n < info_msgs_size; n++) {
	info_msgs[n] = malloc(sizeof(InfoMessage));
	if (info_msgs[n] == NULL)
	    goto bad;
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

    /* TODO: clientargv (not currently supported by API) */
    /* TODO: clientpid */
    /* TODO: clientppid */
    /* TODO: clientsid */
    fill_num("columns", evlog->columns);
    fill_str("command", evlog->command);
    fill_num("lines", evlog->lines);
    if (runargv != NULL) {
	fill_strlist("runargv", runargv);
	runargv = NULL;
    }
    if (evlog->runchroot != NULL) {
	fill_str("runchroot", evlog->runchroot);
    }
    if (evlog->runcwd != NULL) {
	fill_str("runcwd", evlog->runcwd);
    }
    if (runenv != NULL) {
        fill_strlist("runenv", runenv);
        runenv = NULL;
    }
    if (evlog->rungroup != NULL) {
        fill_num("rungid", evlog->rungid);
        fill_str("rungroup", evlog->rungroup);
    }
    /* TODO - rungids */
    /* TODO - rungroups */
    fill_num("runuid", evlog->runuid);
    fill_str("runuser", evlog->runuser);
    if (evlog->source != NULL) {
	fill_str("source", evlog->source);
    }
    if (evlog->cwd != NULL) {
	fill_str("submitcwd", evlog->cwd);
    }
    if (submitenv != NULL) {
        fill_strlist("submitenv", submitenv);
        submitenv = NULL;
    }
    /* TODO - submitgid */
    /* TODO - submitgids */
    /* TODO - submitgroup */
    /* TODO - submitgroups */
    fill_str("submithost", evlog->submithost);
    /* TODO - submituid */
    fill_str("submituser", evlog->submituser);
    if (evlog->ttyname != NULL) {
	fill_str("ttyname", evlog->ttyname);
    }

    /* Free unused structs. */
    while (info_msgs_size > n)
	free(info_msgs[--info_msgs_size]);

    *n_info_msgs = n;
    debug_return_ptr(info_msgs);

bad:
    free_info_messages(info_msgs, n);
    free(runargv);
    free(runenv);
    free(submitenv);

    *n_info_msgs = 0;
    debug_return_ptr(NULL);
}

/*
 * Build and format an AcceptMessage wrapped in a ClientMessage.
 * Appends the wire format message to the closure's write queue.
 * Returns true on success, false on failure.
 */
bool
fmt_accept_message(struct client_closure *closure, struct eventlog *evlog)
{
    ClientMessage client_msg = CLIENT_MESSAGE__INIT;
    AcceptMessage accept_msg = ACCEPT_MESSAGE__INIT;
    TimeSpec ts = TIME_SPEC__INIT;
    struct timespec now;
    bool ret = false;
    debug_decl(fmt_accept_message, SUDOERS_DEBUG_UTIL);

    /*
     * Fill in AcceptMessage and add it to ClientMessage.
     */
    if (sudo_gettime_real(&now)) {
	sudo_warn("%s", U_("unable to get time of day"));
	debug_return_bool(false);
    }
    ts.tv_sec = (int64_t)now.tv_sec;
    ts.tv_nsec = (int32_t)now.tv_nsec;
    accept_msg.submit_time = &ts;

    /* Client will send IoBuffer messages. */
    accept_msg.expect_iobufs = closure->log_io;

    accept_msg.info_msgs = fmt_info_messages(closure, evlog,
	&accept_msg.n_info_msgs);
    if (accept_msg.info_msgs == NULL)
	goto done;

    sudo_debug_printf(SUDO_DEBUG_INFO,
	"%s: sending AcceptMessage, array length %zu", __func__,
	accept_msg.n_info_msgs);

    /* Schedule ClientMessage */
    client_msg.u.accept_msg = &accept_msg;
    client_msg.type_case = CLIENT_MESSAGE__TYPE_ACCEPT_MSG;
    ret = fmt_client_message(closure, &client_msg);

done:
    free_info_messages(accept_msg.info_msgs, accept_msg.n_info_msgs);

    debug_return_bool(ret);
}

/*
 * Build and format a RejectMessage wrapped in a ClientMessage.
 * Appends the wire format message to the closure's write queue.
 * Returns true on success, false on failure.
 */
bool
fmt_reject_message(struct client_closure *closure, struct eventlog *evlog)
{
    ClientMessage client_msg = CLIENT_MESSAGE__INIT;
    RejectMessage reject_msg = REJECT_MESSAGE__INIT;
    TimeSpec ts = TIME_SPEC__INIT;
    struct timespec now;
    bool ret = false;
    debug_decl(fmt_reject_message, SUDOERS_DEBUG_UTIL);

    /*
     * Fill in RejectMessage and add it to ClientMessage.
     */
    if (sudo_gettime_real(&now)) {
	sudo_warn("%s", U_("unable to get time of day"));
	debug_return_bool(false);
    }
    ts.tv_sec = (int64_t)now.tv_sec;
    ts.tv_nsec = (int32_t)now.tv_nsec;
    reject_msg.submit_time = &ts;

    /* Reason for rejecting the request. */
    reject_msg.reason = (char *)closure->reason;

    reject_msg.info_msgs = fmt_info_messages(closure, evlog,
	&reject_msg.n_info_msgs);
    if (reject_msg.info_msgs == NULL)
	goto done;

    sudo_debug_printf(SUDO_DEBUG_INFO,
	"%s: sending RejectMessage, array length %zu", __func__,
	reject_msg.n_info_msgs);

    /* Schedule ClientMessage */
    client_msg.u.reject_msg = &reject_msg;
    client_msg.type_case = CLIENT_MESSAGE__TYPE_REJECT_MSG;
    ret = fmt_client_message(closure, &client_msg);

done:
    free_info_messages(reject_msg.info_msgs, reject_msg.n_info_msgs);

    debug_return_bool(ret);
}

/*
 * Build and format an AlertMessage wrapped in a ClientMessage.
 * Appends the wire format message to the closure's write queue.
 * Returns true on success, false on failure.
 */
static bool
fmt_alert_message(struct client_closure *closure, struct eventlog *evlog)
{
    ClientMessage client_msg = CLIENT_MESSAGE__INIT;
    AlertMessage alert_msg = ALERT_MESSAGE__INIT;
    TimeSpec ts = TIME_SPEC__INIT;
    struct timespec now;
    bool ret = false;
    debug_decl(fmt_alert_message, SUDOERS_DEBUG_UTIL);

    /*
     * Fill in AlertMessage and add it to ClientMessage.
     */
    if (sudo_gettime_real(&now)) {
	sudo_warn("%s", U_("unable to get time of day"));
	debug_return_bool(false);
    }
    ts.tv_sec = (int64_t)now.tv_sec;
    ts.tv_nsec = (int32_t)now.tv_nsec;
    alert_msg.alert_time = &ts;

    /* Reason for the alert. */
    alert_msg.reason = (char *)closure->reason;

    alert_msg.info_msgs = fmt_info_messages(closure, evlog,
	&alert_msg.n_info_msgs);
    if (alert_msg.info_msgs == NULL)
	goto done;

    sudo_debug_printf(SUDO_DEBUG_INFO,
	"%s: sending AlertMessage, array length %zu", __func__,
	alert_msg.n_info_msgs);

    /* Schedule ClientMessage */
    client_msg.u.alert_msg = &alert_msg;
    client_msg.type_case = CLIENT_MESSAGE__TYPE_ALERT_MSG;
    ret = fmt_client_message(closure, &client_msg);

done:
    free_info_messages(alert_msg.info_msgs, alert_msg.n_info_msgs);

    debug_return_bool(ret);
}

/*
 * Build and format an AcceptMessage, RejectMessage or AlertMessage
 * (depending on initial_state) wrapped in a ClientMessage.
 * Appends the wire format message to the closure's write queue.
 * Returns true on success, false on failure.
 */
static bool
fmt_initial_message(struct client_closure *closure)
{
    bool ret = false;
    debug_decl(fmt_initial_message, SUDOERS_DEBUG_UTIL);

    closure->state = closure->initial_state;
    switch (closure->state) {
    case SEND_ACCEPT:
	/* Format and schedule AcceptMessage. */
	if ((ret = fmt_accept_message(closure, closure->log_details->evlog))) {
	    /*
	     * Move read/write events back to main sudo event loop.
	     * Server messages may occur at any time, so no timeout.
	     * Write event will be re-enabled later.
	     */
	    closure->read_ev->setbase(closure->read_ev, NULL);
	    if (closure->read_ev->add(closure->read_ev, NULL) == -1) {
		sudo_warn("%s", U_("unable to add event to queue"));
		ret = false;
	    }
	    closure->write_ev->setbase(closure->write_ev, NULL);
	}
	break;
    case SEND_REJECT:
	/* Format and schedule RejectMessage. */
	ret = fmt_reject_message(closure, closure->log_details->evlog);
	break;
    case SEND_ALERT:
	/* Format and schedule AlertMessage. */
	ret = fmt_alert_message(closure, closure->log_details->evlog);
	break;
    default:
	sudo_warnx(U_("%s: unexpected state %d"), __func__, closure->state);
	break;
    }
    debug_return_bool(ret);
}

#ifdef notyet
/*
 * Build and format a RestartMessage wrapped in a ClientMessage.
 * Appends the wire format message to the closure's write queue.
 * Returns true on success, false on failure.
 */
bool
fmt_restart_message(struct client_closure *closure)
{
    ClientMessage client_msg = CLIENT_MESSAGE__INIT;
    RestartMessage restart_msg = RESTART_MESSAGE__INIT;
    TimeSpec tv = TIME_SPEC__INIT;
    bool ret = false;
    debug_decl(fmt_restart_message, SUDOERS_DEBUG_UTIL);

    sudo_debug_printf(SUDO_DEBUG_INFO,
	"%s: sending RestartMessage, [%lld, %ld]", __func__,
	(long long)closure->restart->tv_sec, closure->restart->tv_nsec);

    tv.tv_sec = closure->restart->tv_sec;
    tv.tv_nsec = closure->restart->tv_nsec;
    restart_msg.resume_point = &tv;
    restart_msg.log_id = closure->iolog_id;

    /* Schedule ClientMessage */
    client_msg.restart_msg = &restart_msg;
    client_msg.type_case = CLIENT_MESSAGE__TYPE_RESTART_MSG;
    ret = fmt_client_message(closure, &client_msg);

    debug_return_bool(ret);
}
#endif

/*
 * Build and format an ExitMessage wrapped in a ClientMessage.
 * Appends the wire format message to the closure's write queue.
 * Returns true on success, false on failure.
 */
bool
fmt_exit_message(struct client_closure *closure, int exit_status, int error)
{
    ClientMessage client_msg = CLIENT_MESSAGE__INIT;
    ExitMessage exit_msg = EXIT_MESSAGE__INIT;
    TimeSpec ts = TIME_SPEC__INIT;
    char signame[SIG2STR_MAX];
    bool ret = false;
    struct timespec run_time;
    debug_decl(fmt_exit_message, SUDOERS_DEBUG_UTIL);

    if (sudo_gettime_awake(&run_time) == -1) {
	sudo_warn("%s", U_("unable to get time of day"));
	goto done;
    }
    sudo_timespecsub(&run_time, &closure->start_time, &run_time);

    ts.tv_sec = (int64_t)run_time.tv_sec;
    ts.tv_nsec = (int32_t)run_time.tv_nsec;
    exit_msg.run_time = &ts;

    if (error != 0) {
	/* Error executing the command. */
	exit_msg.error = strerror(error);
    } else {
	if (WIFEXITED(exit_status)) {
	    exit_msg.exit_value = WEXITSTATUS(exit_status);
	} else if (WIFSIGNALED(exit_status)) {
	    const int signo = WTERMSIG(exit_status);
	    if (signo <= 0 || sig2str(signo, signame) == -1) {
		sudo_warnx(U_("%s: internal error, invalid signal %d"),
		    __func__, signo);
		goto done;
	    }
	    exit_msg.signal = signame;
	    if (WCOREDUMP(exit_status))
		exit_msg.dumped_core = true;
	    exit_msg.exit_value = WTERMSIG(exit_status) | 128;
	} else if (WIFSTOPPED(exit_status)) {
	    const int signo = WSTOPSIG(exit_status);
	    sudo_warnx(U_("%s: internal error, invalid signal %d"),
		__func__, signo);
	    goto done;
	} else if (WIFCONTINUED(exit_status)) {
	    sudo_warnx(U_("%s: internal error, invalid signal %d"),
		__func__, SIGCONT);
	    goto done;
	} else {
	    sudo_warnx(U_("%s: internal error, invalid exit status %d"),
		__func__, exit_status);
	    goto done;
	}
    }

    sudo_debug_printf(SUDO_DEBUG_INFO,
	"%s: sending ExitMessage, exitval %d, error %s, signal %s, coredump %s",
	__func__, exit_msg.exit_value, exit_msg.error ? exit_msg.error : "",
	exit_msg.signal ? exit_msg.signal : "",
	exit_msg.dumped_core ? "yes" : "no");

    /* Send ClientMessage */
    client_msg.u.exit_msg = &exit_msg;
    client_msg.type_case = CLIENT_MESSAGE__TYPE_EXIT_MSG;
    if (!fmt_client_message(closure, &client_msg))
	goto done;

    closure->state = SEND_EXIT;
    ret = true;

done:
    debug_return_bool(ret);
}

/*
 * Build and format an IoBuffer wrapped in a ClientMessage.
 * Appends the wire format message to the closure's write queue.
 * Returns true on success, false on failure.
 */
bool
fmt_io_buf(struct client_closure *closure, int type, const char *buf,
    unsigned int len, struct timespec *delay)
{
    ClientMessage client_msg = CLIENT_MESSAGE__INIT;
    IoBuffer iobuf_msg = IO_BUFFER__INIT;
    TimeSpec ts = TIME_SPEC__INIT;
    bool ret = false;
    debug_decl(fmt_io_buf, SUDOERS_DEBUG_UTIL);

    /* Fill in IoBuffer. */
    ts.tv_sec = (int64_t)delay->tv_sec;
    ts.tv_nsec = (int32_t)delay->tv_nsec;
    iobuf_msg.delay = &ts;
    iobuf_msg.data.data = (void *)buf;
    iobuf_msg.data.len = len;

    sudo_debug_printf(SUDO_DEBUG_INFO,
	"%s: sending IoBuffer length %zu, type %d, size %zu", __func__,
	iobuf_msg.data.len, type, io_buffer__get_packed_size(&iobuf_msg));

    /* Schedule ClientMessage, it doesn't matter which IoBuffer we set. */
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
 * Appends the wire format message to the closure's write queue.
 * Returns true on success, false on failure.
 */
bool
fmt_winsize(struct client_closure *closure, unsigned int lines,
    unsigned int cols, struct timespec *delay)
{
    ClientMessage client_msg = CLIENT_MESSAGE__INIT;
    ChangeWindowSize winsize_msg = CHANGE_WINDOW_SIZE__INIT;
    TimeSpec ts = TIME_SPEC__INIT;
    bool ret = false;
    debug_decl(fmt_winsize, SUDOERS_DEBUG_UTIL);

    /* Fill in ChangeWindowSize message. */
    ts.tv_sec = (int64_t)delay->tv_sec;
    ts.tv_nsec = (int32_t)delay->tv_nsec;
    winsize_msg.delay = &ts;
    winsize_msg.rows = (int32_t)lines;
    winsize_msg.cols = (int32_t)cols;

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
 * Appends the wire format message to the closure's write queue.
 * Returns true on success, false on failure.
 */
bool
fmt_suspend(struct client_closure *closure, const char *signame, struct timespec *delay)
{
    ClientMessage client_msg = CLIENT_MESSAGE__INIT;
    CommandSuspend suspend_msg = COMMAND_SUSPEND__INIT;
    TimeSpec ts = TIME_SPEC__INIT;
    bool ret = false;
    debug_decl(fmt_suspend, SUDOERS_DEBUG_UTIL);

    /* Fill in CommandSuspend message. */
    ts.tv_sec = (int64_t)delay->tv_sec;
    ts.tv_nsec = (int32_t)delay->tv_nsec;
    suspend_msg.delay = &ts;
    suspend_msg.signal = (char *)signame;

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
 * Additional work to do after a ClientMessage was sent to the server.
 * Advances state and formats the next ClientMessage (if any).
 * XXX - better name
 */
static bool
client_message_completion(struct client_closure *closure)
{
    debug_decl(client_message_completion, SUDOERS_DEBUG_UTIL);

    sudo_debug_printf(SUDO_DEBUG_INFO, "%s: state %d", __func__, closure->state);

    switch (closure->state) {
    case RECV_HELLO:
	/* Waiting for ServerHello, nothing else to do. */
	break;
    case SEND_ALERT:
    case SEND_REJECT:
	/* Nothing else to send, we are done. */
	closure->write_ev->del(closure->write_ev);
	closure->read_ev->del(closure->read_ev);
	closure->state = FINISHED;
	break;
    case SEND_ACCEPT:
    case SEND_RESTART:
	closure->state = SEND_IO;
	break;
    case SEND_IO:
	/* Arbitrary number of I/O log buffers, no state change. */
	break;
    case SEND_EXIT:
	if (closure->log_io) {
	    /* Done writing, just waiting for final commit point. */
	    closure->write_ev->del(closure->write_ev);
	    closure->state = CLOSING;

	    /* Enable timeout while waiting for final commit point. */
	    if (closure->read_ev->add(closure->read_ev,
		    &closure->log_details->server_timeout) == -1) {
		sudo_warn("%s", U_("unable to add event to queue"));
		debug_return_bool(false);
	    }
	} else {
	    /* No commit point to wait for, we are done. */
	    closure->state = FINISHED;
	    closure->read_ev->del(closure->read_ev);
	}
	break;
    default:
	sudo_warnx(U_("%s: unexpected state %d"), __func__, closure->state);
	debug_return_bool(false);
    }
    debug_return_bool(true);
}

/*
 * Read the ServerHello message from the log server.
 * We do this synchronously, since we don't want the command to run
 * before the log server connection is completely established.
 */
bool
read_server_hello(struct client_closure *closure)
{
    struct sudo_event_base *evbase = NULL;
    bool ret = false;
    debug_decl(read_server_hello, SUDOERS_DEBUG_UTIL);

    /* Get new event base so we can read ServerHello synchronously. */
    evbase = sudo_ev_base_alloc();
    if (evbase == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	goto done;
    }

    /* Write ClientHello. */
    if (!fmt_client_hello(closure))
	goto done;
    closure->write_ev->setbase(closure->write_ev, evbase);
    if (closure->write_ev->add(closure->write_ev,
	    &closure->log_details->server_timeout) == -1) {
	sudo_warnx("%s", U_("unable to add event to queue"));
	goto done;
    }

    /* Read ServerHello. */
    closure->read_ev->setbase(closure->read_ev, evbase);
    if (closure->read_ev->add(closure->read_ev,
	    &closure->log_details->server_timeout) == -1) {
	sudo_warnx("%s", U_("unable to add event to queue"));
	goto done;
    }

    /* Read/write hello messages synchronously. */
    if (sudo_ev_dispatch(evbase) == -1) {
	sudo_warnx("%s", U_("error in event loop"));
	goto done;
    }

    if (!sudo_ev_got_break(evbase))
	ret = true;

done:
    sudo_ev_base_free(evbase);
    debug_return_bool(ret);
}

/*
 * Respond to a ServerHello message from the server.
 * Returns true on success, false on error.
 */
static bool
handle_server_hello(ServerHello *msg, struct client_closure *closure)
{
    size_t n;
    debug_decl(handle_server_hello, SUDOERS_DEBUG_UTIL);

    if (closure->state != RECV_HELLO) {
	sudo_warnx(U_("%s: unexpected state %d"), __func__, closure->state);
	debug_return_bool(false);
    }

    /* Check that ServerHello is valid. */
    if (msg->server_id == NULL || msg->server_id[0] == '\0') {
	sudo_warnx("%s", U_("invalid ServerHello"));
	debug_return_bool(false);
    }

    sudo_debug_printf(SUDO_DEBUG_INFO, "%s: server ID: %s",
	__func__, msg->server_id);
    /* TODO: handle redirect */
    if (msg->redirect != NULL && msg->redirect[0] != '\0') {
	sudo_debug_printf(SUDO_DEBUG_INFO, "%s: redirect: %s",
	    __func__, msg->redirect);
    }
    for (n = 0; n < msg->n_servers; n++) {
	sudo_debug_printf(SUDO_DEBUG_INFO, "%s: server %zu: %s",
	    __func__, n + 1, msg->servers[n]);
    }

    /* Does the server support logging sub-commands in a session? */
    closure->subcommands = msg->subcommands;

    debug_return_bool(true);
}

/*
 * Respond to a CommitPoint message from the server.
 * Returns true on success, false on error.
 */
static bool
handle_commit_point(TimeSpec *commit_point, struct client_closure *closure)
{
    debug_decl(handle_commit_point, SUDOERS_DEBUG_UTIL);

    /* Only valid after we have sent an IO buffer. */
    if (closure->state < SEND_IO) {
	sudo_warnx(U_("%s: unexpected state %d"), __func__, closure->state);
	debug_return_bool(false);
    }

    closure->committed.tv_sec = (time_t)commit_point->tv_sec;
    closure->committed.tv_nsec = (long)commit_point->tv_nsec;
    sudo_debug_printf(SUDO_DEBUG_INFO,
	"%s: received [%lld, %d], elapsed [%lld, %ld], committed [%lld, %ld]",
	__func__, (long long)commit_point->tv_sec, commit_point->tv_nsec,
	(long long)closure->elapsed.tv_sec, closure->elapsed.tv_nsec,
	(long long)closure->committed.tv_sec, closure->committed.tv_nsec);

    if (closure->state == CLOSING) {
	if (sudo_timespeccmp(&closure->elapsed, &closure->committed, ==)) {
	    /* Last commit point received, exit event loop. */
	    closure->state = FINISHED;
	    closure->read_ev->del(closure->read_ev);
	}
    }

    debug_return_bool(true);
}

/*
 * Respond to a LogId message from the server.
 * Always returns true.
 */
static bool
handle_log_id(char *id, struct client_closure *closure)
{
    debug_decl(handle_log_id, SUDOERS_DEBUG_UTIL);

    sudo_debug_printf(SUDO_DEBUG_INFO, "%s: remote log ID: %s", __func__, id);
    if (closure->iolog_id != NULL) {
	if ((closure->iolog_id = strdup(id)) == NULL)
	    sudo_fatal(U_("%s: %s"), __func__, U_("unable to allocate memory"));
    }
    debug_return_bool(true);
}

/*
 * Respond to a ServerError message from the server.
 * Always returns false.
 */
static bool
handle_server_error(char *errmsg, struct client_closure *closure)
{
    debug_decl(handle_server_error, SUDOERS_DEBUG_UTIL);

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
    debug_decl(handle_server_abort, SUDOERS_DEBUG_UTIL);

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
    debug_decl(handle_server_message, SUDOERS_DEBUG_UTIL);

    sudo_debug_printf(SUDO_DEBUG_INFO, "%s: unpacking ServerMessage", __func__);
    msg = server_message__unpack(NULL, len, buf);
    if (msg == NULL) {
	sudo_warnx(U_("unable to unpack %s size %zu"), "ServerMessage", len);
	debug_return_bool(false);
    }

    switch (msg->type_case) {
    case SERVER_MESSAGE__TYPE_HELLO:
	if (handle_server_hello(msg->u.hello, closure)) {
	    if ((ret = fmt_initial_message(closure))) {
		if (closure->write_ev->add(closure->write_ev,
			&closure->log_details->server_timeout) == -1) {
		    sudo_warn("%s", U_("unable to add event to queue"));
		    ret = false;
		}
	    }
	}
	break;
    case SERVER_MESSAGE__TYPE_COMMIT_POINT:
	ret = handle_commit_point(msg->u.commit_point, closure);
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
 * Expand buf as needed or just reset it.
 * XXX - share with logsrvd/sendlog
 */
static bool
expand_buf(struct connection_buffer *buf, size_t needed)
{
    void *newdata;
    debug_decl(expand_buf, SUDOERS_DEBUG_UTIL);

    if (buf->size < needed) {
	/* Expand buffer. */
	const size_t newsize = sudo_pow2_roundup(needed);
	if (newsize < needed) {
	    /* overflow */
	    errno = ENOMEM;
	    goto oom;
	}
	if ((newdata = malloc(needed)) == NULL)
	    goto oom;
	if (buf->off > 0)
	    memcpy(newdata, buf->data + buf->off, buf->len - buf->off);
	free(buf->data);
	buf->data = newdata;
	buf->size = newsize;
    } else {
	/* Just reset existing buffer. */
	if (buf->off > 0) {
	    memmove(buf->data, buf->data + buf->off,
		buf->len - buf->off);
	}
    }
    buf->len -= buf->off;
    buf->off = 0;

    debug_return_bool(true);
oom:
    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
    debug_return_bool(false);
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
    debug_decl(server_msg_cb, SUDOERS_DEBUG_UTIL);

    /* For TLS we may need to read as part of SSL_write_ex(). */
    if (closure->write_instead_of_read) {
	closure->write_instead_of_read = false;
        client_msg_cb(fd, what, v);
        debug_return;
    }

    if (what == SUDO_PLUGIN_EV_TIMEOUT) {
	sudo_debug_printf(SUDO_DEBUG_INFO, "%s: timed out reading from server",
	    __func__);
	goto bad;
    }

    sudo_debug_printf(SUDO_DEBUG_INFO, "%s: reading ServerMessage", __func__);
#if defined(HAVE_OPENSSL)
    if (closure->ssl != NULL) {
        const int result = SSL_read_ex(closure->ssl, buf->data + buf->len,
	    buf->size - buf->len, &nread);
        if (result <= 0) {
	    unsigned long errcode;
	    const char *errstr;

            switch (SSL_get_error(closure->ssl, result)) {
		case SSL_ERROR_ZERO_RETURN:
		    /* TLS connection shutdown cleanly */
		    sudo_debug_printf(SUDO_DEBUG_NOTICE|SUDO_DEBUG_LINENO,
			"TLS connection shut down cleanly");
		    nread = 0;
		    break;
                case SSL_ERROR_WANT_READ:
		    /* ssl wants to read more, read event is always active */
		    sudo_debug_printf(SUDO_DEBUG_NOTICE|SUDO_DEBUG_LINENO,
			"SSL_read_ex returns SSL_ERROR_WANT_READ");
                    debug_return;
                case SSL_ERROR_WANT_WRITE:
                    /* ssl wants to write, so schedule the write handler */
		    sudo_debug_printf(SUDO_DEBUG_NOTICE|SUDO_DEBUG_LINENO,
			"SSL_read_ex returns SSL_ERROR_WANT_WRITE");
		    if (!closure->write_ev->pending(closure->write_ev,
			    SUDO_PLUGIN_EV_WRITE, NULL)) {
			/* Enable a temporary write event. */
			if (closure->write_ev->add(closure->write_ev, NULL) == -1) {
			    sudo_warn("%s", U_("unable to add event to queue"));
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
		    if (nread == 0)
			sudo_warnx("%s", U_("lost connection to log server"));
		    else
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
#endif /* HAVE_OPENSSL */
    {
        nread = (size_t)read(fd, buf->data + buf->len, buf->size - buf->len);
    }
    sudo_debug_printf(SUDO_DEBUG_INFO, "%s: received %zd bytes from server",
	__func__, nread);
    switch (nread) {
    case (size_t)-1:
	if (errno == EAGAIN)
	    debug_return;
	sudo_warn("read");
	goto bad;
    case 0:
	sudo_warnx("%s", U_("lost connection to log server"));
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
    if (closure->log_details->ignore_log_errors) {
	/* Disable plugin, the command continues. */
	closure->disabled = true;
	closure->read_ev->del(closure->read_ev);
    } else {
	/* Break out of sudo event loop and kill the command. */
	closure->read_ev->loopbreak(closure->read_ev);
    }
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
    debug_decl(client_msg_cb, SUDOERS_DEBUG_UTIL);

    /* For TLS we may need to write as part of SSL_read_ex(). */
    if (closure->read_instead_of_write) {
	closure->read_instead_of_write = false;
	/* Delete write event if it was only due to SSL_read_ex(). */
	if (closure->temporary_write_event) {
            closure->temporary_write_event = false;
	    closure->write_ev->del(closure->write_ev);
	}
	server_msg_cb(fd, what, v);
	debug_return;
    }

    if (what == SUDO_PLUGIN_EV_TIMEOUT) {
	sudo_debug_printf(SUDO_DEBUG_INFO, "%s: timed out writing to server",
	    __func__);
	goto bad;
    }

    if ((buf = TAILQ_FIRST(&closure->write_bufs)) == NULL) {
	sudo_warnx("%s", U_("missing write buffer"));
	goto bad;
    }

    sudo_debug_printf(SUDO_DEBUG_INFO,
    	"%s: sending %zu bytes to server", __func__, buf->len - buf->off);

#if defined(HAVE_OPENSSL)
    if (closure->ssl != NULL) {
        const int result = SSL_write_ex(closure->ssl, buf->data + buf->off,
	    buf->len - buf->off, &nwritten);
        if (result <= 0) {
	    const char *errstr;

            switch (SSL_get_error(closure->ssl, result)) {
		case SSL_ERROR_ZERO_RETURN:
		    /* TLS connection shutdown cleanly */
		    sudo_debug_printf(SUDO_DEBUG_NOTICE|SUDO_DEBUG_LINENO,
			"TLS connection shut down cleanly");
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
                case SSL_ERROR_SSL:
                    errstr = ERR_reason_error_string(ERR_get_error());
                    sudo_warnx("%s", errstr ? errstr : strerror(errno));
                    goto bad;
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
#endif /* HAVE_OPENSSL */
    {
        nwritten = (size_t)write(fd, buf->data + buf->off, buf->len - buf->off);
    }

    if (nwritten == (size_t)-1) {
	sudo_warn("send");
	goto bad;
    }
    buf->off += nwritten;

    if (buf->off == buf->len) {
	/* sent entire message, move buf to free list */
	sudo_debug_printf(SUDO_DEBUG_INFO,
	    "%s: finished sending %zu bytes to server", __func__, buf->len);
	buf->off = 0;
	buf->len = 0;
	TAILQ_REMOVE(&closure->write_bufs, buf, entries);
	TAILQ_INSERT_TAIL(&closure->free_bufs, buf, entries);
	if (TAILQ_EMPTY(&closure->write_bufs)) {
	    /* Write queue empty, check for state change. */
	    closure->write_ev->del(closure->write_ev);
	    if (!client_message_completion(closure))
		goto bad;
	}
    }
    debug_return;

bad:
    if (closure->log_details->ignore_log_errors) {
	/* Disable plugin, the command continues. */
	closure->disabled = true;
	closure->write_ev->del(closure->read_ev);
	closure->write_ev->del(closure->write_ev);
    } else {
	/* Break out of sudo event loop and kill the command. */
	closure->write_ev->loopbreak(closure->write_ev);
    }
    debug_return;
}

/*
 * Allocate and initialize a new client closure
 */
static struct client_closure *
client_closure_alloc(struct log_details *details, struct timespec *now,
    bool log_io, enum client_state initial_state, const char *reason)
{
    struct client_closure *closure;
    debug_decl(client_closure_alloc, SUDOERS_DEBUG_UTIL);

    if (plugin_event_alloc == NULL) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
	    "plugin_event_alloc is not set");
	debug_return_ptr(NULL);
    }

    if ((closure = calloc(1, sizeof(*closure))) == NULL)
        goto oom;

    closure->sock = -1;
    closure->log_io = log_io;
    closure->reason = reason;
    closure->state = RECV_HELLO;
    closure->initial_state = initial_state;

    closure->start_time.tv_sec = now->tv_sec;
    closure->start_time.tv_nsec = now->tv_nsec;

    TAILQ_INIT(&closure->write_bufs);
    TAILQ_INIT(&closure->free_bufs);

    closure->read_buf.size = 64 * 1024;
    closure->read_buf.data = malloc(closure->read_buf.size);
    if (closure->read_buf.data == NULL)
	goto oom;

    if ((closure->read_ev = plugin_event_alloc()) == NULL)
	goto oom;

    if ((closure->write_ev = plugin_event_alloc()) == NULL)
	goto oom;

    closure->log_details = details;

    debug_return_ptr(closure);
oom:
    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
    client_closure_free(closure);
    debug_return_ptr(NULL);
}

struct client_closure *
log_server_open(struct log_details *details, struct timespec *now,
    bool log_io, enum client_state initial_state, const char *reason)
{
    struct client_closure *closure;
    static bool warned = false;
    debug_decl(log_server_open, SUDOERS_DEBUG_UTIL);

    closure = client_closure_alloc(details, now, log_io, initial_state,
	reason);
    if (closure == NULL)
	goto bad;

    /* Connect to log first available log server. */
    if (!log_server_connect(closure)) {
	/* TODO: support offline logs if server unreachable */
	if (!warned) {
	    sudo_warnx("%s", U_("unable to connect to log server"));
	    warned = true;
	}
	goto bad;
    }

    /* Read ServerHello synchronously or fail. */
    if (read_server_hello(closure))
	debug_return_ptr(closure);

bad:
    client_closure_free(closure);
    debug_return_ptr(NULL);
}

/*
 * Send ExitMessage, wait for final commit message and free closure.
 */
bool
log_server_close(struct client_closure *closure, int exit_status, int error)
{
    struct sudo_event_base *evbase = NULL;
    bool ret = false;
    debug_decl(log_server_close, SUDOERS_DEBUG_UTIL);

    if (closure->disabled)
	goto done;

    /* Format and append an ExitMessage to the write queue. */
    if (!fmt_exit_message(closure, exit_status, error))
	goto done;

    /*
     * Create private event base and reparent the read/write events.
     * We cannot use the main sudo event loop as it has already exited.
     */
    if ((evbase = sudo_ev_base_alloc()) == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	goto done;
    }

    /* Enable read event to receive server messages. */
    closure->read_ev->setbase(closure->read_ev, evbase);
    if (closure->read_ev->add(closure->read_ev,
	    &closure->log_details->server_timeout) == -1) {
	sudo_warn("%s", U_("unable to add event to queue"));
	goto done;
    }

    /* Enable the write event to write the ExitMessage. */
    closure->write_ev->setbase(closure->write_ev, evbase);
    if (closure->write_ev->add(closure->write_ev,
	    &closure->log_details->server_timeout) == -1) {
	sudo_warn("%s", U_("unable to add event to queue"));
	goto done;
    }

    /* Loop until queues are flushed and final commit point received. */
    sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
	"flushing buffers and waiting for final commit point");
    if (sudo_ev_dispatch(evbase) == -1 || sudo_ev_got_break(evbase)) {
	sudo_warnx("%s", U_("error in event loop"));
	goto done;
    }

    ret = true;

done:
    sudo_ev_base_free(evbase);
    client_closure_free(closure);
    debug_return_bool(ret);
}

#endif /* SUDOERS_LOG_CLIENT */
