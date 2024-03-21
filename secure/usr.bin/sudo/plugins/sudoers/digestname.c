/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2017 Todd C. Miller <Todd.Miller@sudo.ws>
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

#include <sudo_compat.h>
#include <sudo_digest.h>
#include <sudoers_debug.h>
#include <parse.h>

const char *
digest_type_to_name(unsigned int digest_type)
{
    const char *digest_name;
    debug_decl(digest_type_to_name, SUDOERS_DEBUG_UTIL);

    switch (digest_type) {
    case SUDO_DIGEST_SHA224:
	digest_name = "sha224";
	break;
    case SUDO_DIGEST_SHA256:
	digest_name = "sha256";
	break;
    case SUDO_DIGEST_SHA384:
	digest_name = "sha384";
	break;
    case SUDO_DIGEST_SHA512:
	digest_name = "sha512";
	break;
    default:
	digest_name = "unknown digest";
	break;
    }
    debug_return_const_str(digest_name);
}
