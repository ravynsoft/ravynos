/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2018 Todd C. Miller <Todd.Miller@sudo.ws>
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
 *
 * Sponsored in part by the Defense Advanced Research Projects
 * Agency (DARPA) and Air Force Research Laboratory, Air Force
 * Materiel Command, USAF, under agreement number F39502-99-1-0512.
 */

#ifndef SUDO_RAND_H
#define SUDO_RAND_H

#include <stdlib.h>	/* For arc4random() on systems that have it */

/*
 * All libc replacements are prefixed with "sudo_" to avoid namespace issues.
 */

#ifndef HAVE_ARC4RANDOM
sudo_dso_public uint32_t sudo_arc4random(void);
# undef arc4random
# define arc4random() sudo_arc4random()
#endif /* ARC4RANDOM */

#ifndef HAVE_ARC4RANDOM_BUF
sudo_dso_public void sudo_arc4random_buf(void *buf, size_t n);
# undef arc4random_buf
# define arc4random_buf(a, b) sudo_arc4random_buf((a), (b))
#endif /* ARC4RANDOM_BUF */

#ifndef HAVE_ARC4RANDOM_UNIFORM
sudo_dso_public uint32_t sudo_arc4random_uniform(uint32_t upper_bound);
# undef arc4random_uniform
# define arc4random_uniform(_a) sudo_arc4random_uniform((_a))
#endif /* ARC4RANDOM_UNIFORM */

#ifndef HAVE_GETENTROPY
/* Note: not exported by libutil. */
int sudo_getentropy(void *buf, size_t buflen);
# undef getentropy
# define getentropy(_a, _b) sudo_getentropy((_a), (_b))
#endif /* HAVE_GETENTROPY */

#endif /* SUDO_RAND_H */
