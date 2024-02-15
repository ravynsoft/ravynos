/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2019-2020 Todd C. Miller <Todd.Miller@sudo.ws>
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
#include <sudo_debug.h>
#include <sudo_util.h>

/*
 * Round 32-bit unsigned length to the next highest power of two.
 * Always returns at least 64.
 */
unsigned int
sudo_pow2_roundup_v1(unsigned int len)
{
    if (len < 64)
	return 64;

#ifdef HAVE___BUILTIN_CLZ
    return 1U << (32 - __builtin_clz(len - 1));
#else
    len--;
    len |= len >> 1;
    len |= len >> 2;
    len |= len >> 4;
    len |= len >> 8;
    len |= len >> 16;
    len++;
    return len;
#endif
}

/*
 * Round a size_t length to the next highest power of two.
 * Always returns at least 64.
 */
size_t
sudo_pow2_roundup_v2(size_t len)
{
    if (len < 64)
	return 64;

#if defined(__LP64__) && defined(HAVE___BUILTIN_CLZL)
    return 1UL << (64 - __builtin_clzl(len - 1));
#elif !defined(__LP64__) && defined(HAVE___BUILTIN_CLZ)
    return 1U << (32 - __builtin_clz(len - 1));
#else
    len--;
    len |= len >> 1;
    len |= len >> 2;
    len |= len >> 4;
    len |= len >> 8;
    len |= len >> 16;
# ifdef __LP64__
    len |= len >> 32;
# endif
    len++;
    return len;
#endif
}
