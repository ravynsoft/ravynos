/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2013-2021 Todd C. Miller <Todd.Miller@sudo.ws>
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

#include <stdlib.h>
#include <errno.h>

#if defined(HAVE_WOLFSSL)
# include <wolfssl/options.h>
#endif
#include <openssl/evp.h>

#include <sudo_compat.h>
#include <sudo_debug.h>
#include <sudo_digest.h>

struct sudo_digest {
    EVP_MD_CTX *ctx;
    const EVP_MD *md;
};

static const EVP_MD *
sudo_digest_type_to_md(unsigned int digest_type)
{
    const EVP_MD *md = NULL;
    debug_decl(sudo_digest_type_to_md, SUDO_DEBUG_UTIL);

    switch (digest_type) {
    case SUDO_DIGEST_SHA224:
	md = EVP_sha224();
	break;
    case SUDO_DIGEST_SHA256:
	md = EVP_sha256();
	break;
    case SUDO_DIGEST_SHA384:
	md = EVP_sha384();
	break;
    case SUDO_DIGEST_SHA512:
	md = EVP_sha512();
	break;
    default:
	errno = EINVAL;
	break;
    }
    debug_return_const_ptr(md);
}

struct sudo_digest *
sudo_digest_alloc_v1(unsigned int digest_type)
{
    struct sudo_digest *dig;
    EVP_MD_CTX *mdctx = NULL;
    const EVP_MD *md;
    debug_decl(sudo_digest_alloc, SUDO_DEBUG_UTIL);

    md = sudo_digest_type_to_md(digest_type);
    if (md == NULL)
	goto bad;

    mdctx = EVP_MD_CTX_new();
    if (mdctx == NULL || !EVP_DigestInit_ex(mdctx, md, NULL))
	goto bad;

    if ((dig = malloc(sizeof(*dig))) == NULL)
	goto bad;
    dig->md = md;
    dig->ctx = mdctx;

    debug_return_ptr(dig);
bad:
    EVP_MD_CTX_free(mdctx);
    debug_return_ptr(NULL);
}

void
sudo_digest_free_v1(struct sudo_digest *dig)
{
    debug_decl(sudo_digest_free, SUDO_DEBUG_UTIL);

    if (dig != NULL) {
	EVP_MD_CTX_free(dig->ctx);
	free(dig);
    }

    debug_return;
}

void
sudo_digest_reset_v1(struct sudo_digest *dig)
{
    debug_decl(sudo_digest_reset, SUDO_DEBUG_UTIL);

    /* These cannot fail. */
    EVP_MD_CTX_reset(dig->ctx);
    EVP_DigestInit_ex(dig->ctx, dig->md, NULL);

    debug_return;
}

size_t
sudo_digest_getlen_v2(unsigned int digest_type)
{
    const EVP_MD *md;
    debug_decl(sudo_digest_getlen, SUDO_DEBUG_UTIL);

    md = sudo_digest_type_to_md(digest_type);
    if (md == NULL)
	debug_return_size_t(0);

    debug_return_size_t((size_t)EVP_MD_size(md));
}

int
sudo_digest_getlen_v1(unsigned int digest_type)
{
    size_t len = sudo_digest_getlen_v2(digest_type);
    return len ? (int)len : -1;
}

void
sudo_digest_update_v1(struct sudo_digest *dig, const void *data, size_t len)
{
    debug_decl(sudo_digest_update, SUDO_DEBUG_UTIL);

    EVP_DigestUpdate(dig->ctx, data, len);

    debug_return;
}

void
sudo_digest_final_v1(struct sudo_digest *dig, unsigned char *md)
{
    debug_decl(sudo_digest_final, SUDO_DEBUG_UTIL);

    EVP_DigestFinal_ex(dig->ctx, md, NULL);

    debug_return;
}
