/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2020 Todd C. Miller <Todd.Miller@sudo.ws>
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

#define __STDC_WANT_LIB_EXT1__ 1	/* for memset_s() */

#include <string.h>
#ifdef HAVE_STRINGS_H
# include <strings.h>
#endif /* HAVE_STRINGS_H */

#include <sudo_compat.h>

#ifndef HAVE_EXPLICIT_BZERO

# if defined(HAVE_EXPLICIT_MEMSET)
void
sudo_explicit_bzero(void *s, size_t n)
{
    explicit_memset(s, 0, n);
}
# elif defined(HAVE_MEMSET_EXPLICIT)
void
sudo_explicit_bzero(void *s, size_t n)
{
    memset_explicit(s, 0, n);
}
# elif defined(HAVE_MEMSET_S)
void
sudo_explicit_bzero(void *s, size_t n)
{
    (void)memset_s(s, n, 0, n);
}
# elif defined(HAVE_BZERO)
/* Jumping through a volatile function pointer should not be optimized away. */
void (* volatile sudo_explicit_bzero_impl)(void *, size_t) =
    (void (*)(void *, size_t))bzero;

void
sudo_explicit_bzero(void *s, size_t n)
{
    sudo_explicit_bzero_impl(s, n);
}
# else
void
sudo_explicit_bzero(void *v, size_t n)
{
    volatile unsigned char *s = v;

    /* Updating through a volatile pointer should not be optimized away. */
    while (n--)
	*s++ = 0;
}
# endif /* HAVE_BZERO */

#endif /* HAVE_EXPLICIT_BZERO */
