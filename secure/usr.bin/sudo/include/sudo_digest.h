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
 */

#ifndef SUDO_DIGEST_H
#define SUDO_DIGEST_H

/* Digest types. */
#define SUDO_DIGEST_SHA224	0
#define SUDO_DIGEST_SHA256	1
#define SUDO_DIGEST_SHA384	2
#define SUDO_DIGEST_SHA512	3
#define SUDO_DIGEST_INVALID	4

struct sudo_digest;

/* Public functions. */
sudo_dso_public struct sudo_digest *sudo_digest_alloc_v1(unsigned int digest_type);
sudo_dso_public void sudo_digest_free_v1(struct sudo_digest *dig);
sudo_dso_public void sudo_digest_reset_v1(struct sudo_digest *dig);
sudo_dso_public int sudo_digest_getlen_v1(unsigned int digest_type);
sudo_dso_public size_t sudo_digest_getlen_v2(unsigned int digest_type);
sudo_dso_public void sudo_digest_update_v1(struct sudo_digest *dig, const void *data, size_t len);
sudo_dso_public void sudo_digest_final_v1(struct sudo_digest *dig, unsigned char *md);

#define sudo_digest_alloc(_a) sudo_digest_alloc_v1((_a))
#define sudo_digest_free(_a) sudo_digest_free_v1((_a))
#define sudo_digest_reset(_a) sudo_digest_reset_v1((_a))
#define sudo_digest_getlen(_a) sudo_digest_getlen_v2((_a))
#define sudo_digest_update(_a, _b, _c) sudo_digest_update_v1((_a), (_b), (_c))
#define sudo_digest_final(_a, _b) sudo_digest_final_v1((_a), (_b))

#endif /* SUDO_DIGEST_H */
