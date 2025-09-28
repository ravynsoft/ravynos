/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2017-2018 Todd C. Miller <Todd.Miller@sudo.ws>
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

#ifdef HAVE_STDBOOL_H
# include <stdbool.h>
#else
# include <compat/stdbool.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <gcrypt.h>

#include <sudo_compat.h>
#include <sudo_debug.h>
#include <sudo_digest.h>

#define SUDO_LIBGCRYPT_VERSION_MIN "1.3.0"

struct sudo_digest {
    int gcry_digest_type;
    unsigned int digest_len;
    gcry_md_hd_t ctx;
};

/* Map sudo digest type to gcrypt digest type. */
static int
sudo_digest_type_to_gcry(unsigned int digest_type)
{
    switch (digest_type) {
    case SUDO_DIGEST_SHA224:
	return GCRY_MD_SHA224;
    case SUDO_DIGEST_SHA256:
	return GCRY_MD_SHA256;
    case SUDO_DIGEST_SHA384:
	return GCRY_MD_SHA384;
    case SUDO_DIGEST_SHA512:
	return GCRY_MD_SHA512;
    default:
	return -1;
    }
}

struct sudo_digest *
sudo_digest_alloc_v1(unsigned int digest_type)
{
    debug_decl(sudo_digest_alloc, SUDO_DEBUG_UTIL);
    static bool initialized = false;
    struct sudo_digest *dig;
    int gcry_digest_type;

    if (!initialized) {
	if (!gcry_check_version(SUDO_LIBGCRYPT_VERSION_MIN)) {
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		"libgcrypt too old (need %s, have %s)",
		SUDO_LIBGCRYPT_VERSION_MIN, gcry_check_version(NULL));
	    debug_return_ptr(NULL);
	}
	gcry_control(GCRYCTL_INITIALIZATION_FINISHED, 0);
	initialized = true;
    }

    gcry_digest_type = sudo_digest_type_to_gcry(digest_type);
    if (gcry_digest_type == -1) {
	errno = EINVAL;
	debug_return_ptr(NULL);
    }

    if ((dig = malloc(sizeof(*dig))) == NULL)
	debug_return_ptr(NULL);
    dig->gcry_digest_type = gcry_digest_type;
    dig->digest_len = gcry_md_get_algo_dlen(gcry_digest_type);

    if (gcry_md_open(&dig->ctx, gcry_digest_type, 0) != 0) {
	free(dig);
	debug_return_ptr(NULL);
    }

    debug_return_ptr(dig);
}

void
sudo_digest_free_v1(struct sudo_digest *dig)
{
    debug_decl(sudo_digest_free, SUDO_DEBUG_UTIL);

    if (dig != NULL) {
	gcry_md_close(dig->ctx);
	free(dig);
    }

    debug_return;
}

void
sudo_digest_reset_v1(struct sudo_digest *dig)
{
    debug_decl(sudo_digest_reset, SUDO_DEBUG_UTIL);

    gcry_md_reset(dig->ctx);

    debug_return;
}

size_t
sudo_digest_getlen_v2(unsigned int digest_type)
{
    debug_decl(sudo_digest_getlen, SUDO_DEBUG_UTIL);
    int gcry_digest_type;

    gcry_digest_type = sudo_digest_type_to_gcry(digest_type);
    if (gcry_digest_type == -1)
	debug_return_size_t(0);

    debug_return_size_t(gcry_md_get_algo_dlen(gcry_digest_type));
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

    gcry_md_write(dig->ctx, data, len);

    debug_return;
}

void
sudo_digest_final_v1(struct sudo_digest *dig, unsigned char *md)
{
    debug_decl(sudo_digest_final, SUDO_DEBUG_UTIL);

    gcry_md_final(dig->ctx);
    memcpy(md, gcry_md_read(dig->ctx, 0), dig->digest_len);

    debug_return;
}
