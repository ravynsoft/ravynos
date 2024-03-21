/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2023 Todd C. Miller <Todd.Miller@sudo.ws>
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

#ifndef SUDO_SSL_COMPAT_H
#define SUDO_SSL_COMPAT_H

# ifdef HAVE_OPENSSL

/*
 * Compatibility defines for OpenSSL 1.0.2 (not needed for 1.1.x)
 */
#  ifndef HAVE_WOLFSSL
#   ifndef HAVE_X509_STORE_CTX_GET0_CERT
#    define X509_STORE_CTX_get0_cert(x)   ((x)->cert)
#   endif
#   ifndef HAVE_TLS_METHOD
#    define TLS_method()                  SSLv23_method()
#   endif
#  endif /* !HAVE_WOLFSSL */

/*
 * SSL_read_ex() and SSL_write_ex() were added in OpenSSL 1.1.1.
 */
#  ifndef HAVE_SSL_READ_EX
int SSL_read_ex(SSL *, void *, size_t, size_t *);
int SSL_write_ex(SSL *, const void *, size_t, size_t *);
#  endif /* HAVE_SSL_READ_EX */

# endif /* HAVE_OPENSSL */

#endif /* SUDO_SSL_COMPAT_H */
