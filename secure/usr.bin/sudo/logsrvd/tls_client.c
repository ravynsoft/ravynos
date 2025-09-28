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

#include <errno.h>
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

#include <sudo_compat.h>
#include <sudo_debug.h>
#include <sudo_event.h>
#include <sudo_fatal.h>
#include <sudo_gettext.h>
#include <sudo_util.h>

#include "logsrv_util.h"
#include <tls_common.h>
#include <hostcheck.h>

#if defined(HAVE_OPENSSL)

/*
 * Check that the server's certificate is valid that it contains the
 * server name or IP address.
 * Returns 0 if the cert is invalid, else 1.
 */
static int
verify_peer_identity(int preverify_ok, X509_STORE_CTX *ctx)
{
    HostnameValidationResult result;
    struct peer_info *peer_info;
    SSL *ssl;
    X509 *current_cert;
    X509 *peer_cert;
    debug_decl(verify_peer_identity, SUDO_DEBUG_UTIL);

    /* if pre-verification of the cert failed, just propagate that result back */
    if (preverify_ok != 1) {
        debug_return_int(0);
    }

    /*
     * Since this callback is called for each cert in the chain,
     * check that current cert is the peer's certificate
     */
    current_cert = X509_STORE_CTX_get_current_cert(ctx);
    peer_cert = X509_STORE_CTX_get0_cert(ctx);
    if (current_cert != peer_cert) {
        debug_return_int(1);
    }

    /* Fetch the attached peer_info from the ssl connection object. */
    ssl = X509_STORE_CTX_get_ex_data(ctx, SSL_get_ex_data_X509_STORE_CTX_idx());
    peer_info = SSL_get_ex_data(ssl, 1);

    /*
     * Validate the cert based on the host name and IP address.
     * If host name is not known, validate_hostname() can resolve it.
     */
    result = validate_hostname(peer_cert,
	peer_info->name ? peer_info->name : peer_info->ipaddr,
	peer_info->ipaddr, peer_info->name ? 0 : 1);

    debug_return_int(result == MatchFound);
}

void
tls_connect_cb(int sock, int what, void *v)
{
    struct tls_client_closure *tls_client = v;
    struct sudo_event_base *evbase = tls_client->evbase;
    const struct timespec *timeout = &tls_client->connect_timeout;
    const char *errstr;
    int con_stat;
    debug_decl(tls_connect_cb, SUDO_DEBUG_UTIL);

    if (what == SUDO_EV_TIMEOUT) {
        sudo_warnx("%s", U_("TLS handshake timeout occurred"));
        goto bad;
    }

    con_stat = SSL_connect(tls_client->ssl);

    if (con_stat == 1) {
	sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
	    "SSL_connect successful");
        tls_client->tls_connect_state = true;
    } else {
        switch (SSL_get_error(tls_client->ssl, con_stat)) {
            /* TLS handshake is not finished, reschedule event */
            case SSL_ERROR_WANT_READ:
		sudo_debug_printf(SUDO_DEBUG_NOTICE|SUDO_DEBUG_LINENO,
		    "SSL_connect returns SSL_ERROR_WANT_READ");
		if (what != SUDO_EV_READ) {
		    if (sudo_ev_set(tls_client->tls_connect_ev,
			    SSL_get_fd(tls_client->ssl), SUDO_EV_READ,
			    tls_connect_cb, tls_client) == -1) {
			sudo_warnx("%s", U_("unable to set event"));
			goto bad;
		    }
		}
                if (sudo_ev_add(evbase, tls_client->tls_connect_ev, timeout, false) == -1) {
                    sudo_warnx("%s", U_("unable to add event to queue"));
		    goto bad;
                }
		break;
            case SSL_ERROR_WANT_WRITE:
		sudo_debug_printf(SUDO_DEBUG_NOTICE|SUDO_DEBUG_LINENO,
		    "SSL_connect returns SSL_ERROR_WANT_WRITE");
		if (what != SUDO_EV_WRITE) {
		    if (sudo_ev_set(tls_client->tls_connect_ev,
			    SSL_get_fd(tls_client->ssl), SUDO_EV_WRITE,
			    tls_connect_cb, tls_client) == -1) {
			sudo_warnx("%s", U_("unable to set event"));
			goto bad;
		    }
		}
                if (sudo_ev_add(evbase, tls_client->tls_connect_ev, timeout, false) == -1) {
                    sudo_warnx("%s", U_("unable to add event to queue"));
		    goto bad;
                }
		break;
	    case SSL_ERROR_SYSCALL:
                sudo_warnx(U_("TLS connection failed: %s"), strerror(errno));
		goto bad;
            default:
		errstr = ERR_reason_error_string(ERR_get_error());
                sudo_warnx(U_("TLS connection failed: %s"),
		    errstr ? errstr : strerror(errno));
                goto bad;
        }
    }

    if (tls_client->tls_connect_state) {
	sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
	    "TLS version: %s, negotiated cipher suite: %s",
	    SSL_get_version(tls_client->ssl), SSL_get_cipher(tls_client->ssl));

	/* Done with TLS connect, send ClientHello */
	sudo_ev_free(tls_client->tls_connect_ev);
	tls_client->tls_connect_ev = NULL;
	if (!tls_client->start_fn(tls_client))
	    goto bad;
    }

    debug_return;

bad:
    sudo_ev_loopbreak(evbase);
    debug_return;
}

bool
tls_ctx_client_setup(SSL_CTX *ssl_ctx, int sock,
    struct tls_client_closure *closure)
{
    const char *errstr;
    bool ret = false;
    debug_decl(tls_ctx_client_setup, SUDO_DEBUG_UTIL);

    if ((closure->ssl = SSL_new(ssl_ctx)) == NULL) {
	errstr = ERR_reason_error_string(ERR_get_error());
        sudo_warnx(U_("unable to allocate ssl object: %s"),
	    errstr ? errstr : strerror(errno));
        goto done;
    }

    if (SSL_set_ex_data(closure->ssl, 1, closure->peer_name) <= 0) {
	errstr = ERR_reason_error_string(ERR_get_error());
	sudo_warnx(U_("Unable to attach user data to the ssl object: %s"),
	    errstr ? errstr : strerror(errno));
	goto done;
    }

    if (SSL_set_fd(closure->ssl, sock) <= 0) {
	errstr = ERR_reason_error_string(ERR_get_error());
        sudo_warnx(U_("Unable to attach socket to the ssl object: %s"),
	    errstr ? errstr : strerror(errno));
        goto done;
    }

    if (sudo_ev_add(closure->evbase, closure->tls_connect_ev, NULL, false) == -1) {
	sudo_warnx("%s", U_("unable to add event to queue"));
	goto done;
    }

    ret = true;

done:
    debug_return_bool(ret);
}

bool
tls_client_setup(int sock, const char *ca_bundle_file, const char *cert_file,
    const char *key_file, const char *dhparam_file, const char *ciphers_v12,
    const char *ciphers_v13, bool verify_server, bool check_peer,
    struct tls_client_closure *closure)
{
    SSL_CTX *ssl_ctx;
    debug_decl(tls_client_setup, SUDO_DEBUG_UTIL);

    ssl_ctx = init_tls_context(ca_bundle_file, cert_file, key_file,
	dhparam_file, ciphers_v12, ciphers_v13, verify_server);
    if (ssl_ctx == NULL) {
        sudo_warnx("%s", U_("unable to initialize TLS context"));
	debug_return_bool(false);
    }

    if (check_peer) {
	/* Verify server cert during the handshake. */
	SSL_CTX_set_verify(ssl_ctx,
	    SSL_VERIFY_PEER|SSL_VERIFY_FAIL_IF_NO_PEER_CERT,
	    verify_peer_identity);
    }

    debug_return_bool(tls_ctx_client_setup(ssl_ctx, sock, closure));
}
#endif /* HAVE_OPENSSL */
