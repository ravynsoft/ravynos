/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2021 Todd C. Miller <Todd.Miller@sudo.ws>
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

#ifndef SUDO_TLS_COMMON_H
#define SUDO_TLS_COMMON_H

#include <config.h>

#if defined(HAVE_OPENSSL)
# if defined(HAVE_WOLFSSL)
#  include <wolfssl/options.h>
# endif
# include <openssl/ssl.h>
# include <openssl/err.h>
# include <sudo_ssl_compat.h>

struct tls_client_closure {
    SSL *ssl;
    void *parent_closure;
    struct sudo_event_base *evbase;	/* duplicated */
    struct sudo_event *tls_connect_ev;
    struct peer_info *peer_name;
    struct timespec connect_timeout;
    bool (*start_fn)(struct tls_client_closure *);
    bool tls_connect_state;
};

/* tls_client.c */
void tls_connect_cb(int sock, int what, void *v);
bool tls_client_setup(int sock, const char *ca_bundle_file, const char *cert_file, const char *key_file, const char *dhparam_file, const char *ciphers_v12, const char *ciphers_v13, bool verify_server, bool check_peer, struct tls_client_closure *closure);
bool tls_ctx_client_setup(SSL_CTX *ssl_ctx, int sock, struct tls_client_closure *closure);

/* tls_init.c */
SSL_CTX *init_tls_context(const char *ca_bundle_file, const char *cert_file, const char *key_file, const char *dhparam_file, const char *ciphers_v12, const char *ciphers_v13, bool verify_cert);

#endif /* HAVE_OPENSSL */

#endif /* SUDO_TLS_COMMON_H */
