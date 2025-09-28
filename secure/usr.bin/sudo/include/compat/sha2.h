/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2013-2015 Todd C. Miller <Todd.Miller@sudo.ws>
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
 * Derived from the public domain SHA-1 and SHA-2 implementations
 * by Steve Reid and Wei Dai respectively.
 */

#ifndef COMPAT_SHA2_H
#define COMPAT_SHA2_H

#define	SHA224_BLOCK_LENGTH		64
#define	SHA224_DIGEST_LENGTH		28
#define	SHA224_DIGEST_STRING_LENGTH	(SHA224_DIGEST_LENGTH * 2 + 1)

#define	SHA256_BLOCK_LENGTH		64
#define	SHA256_DIGEST_LENGTH		32
#define	SHA256_DIGEST_STRING_LENGTH	(SHA256_DIGEST_LENGTH * 2 + 1)

#define	SHA384_BLOCK_LENGTH		128
#define	SHA384_DIGEST_LENGTH		48
#define	SHA384_DIGEST_STRING_LENGTH	(SHA384_DIGEST_LENGTH * 2 + 1)

#define	SHA512_BLOCK_LENGTH		128
#define	SHA512_DIGEST_LENGTH		64
#define	SHA512_DIGEST_STRING_LENGTH	(SHA512_DIGEST_LENGTH * 2 + 1)

typedef struct {
    union {
	uint32_t st32[8];	/* sha224 and sha256 */
	uint64_t st64[8];	/* sha384 and sha512 */
    } state;
    uint64_t count[2];
    uint8_t buffer[SHA512_BLOCK_LENGTH];
} SHA2_CTX;

sudo_dso_public void sudo_SHA224Init(SHA2_CTX *ctx);
sudo_dso_public void sudo_SHA224Pad(SHA2_CTX *ctx);
sudo_dso_public void sudo_SHA224Transform(uint32_t state[8], const uint8_t buffer[SHA224_BLOCK_LENGTH]);
sudo_dso_public void sudo_SHA224Update(SHA2_CTX *ctx, const uint8_t *data, size_t len);
sudo_dso_public void sudo_SHA224Final(uint8_t digest[SHA224_DIGEST_LENGTH], SHA2_CTX *ctx);

#define SHA224Init		sudo_SHA224Init
#define SHA224Pad		sudo_SHA224Pad
#define SHA224Transform		sudo_SHA224Transform
#define SHA224Update		sudo_SHA224Update
#define SHA224Final		sudo_SHA224Final

sudo_dso_public void sudo_SHA256Init(SHA2_CTX *ctx);
sudo_dso_public void sudo_SHA256Pad(SHA2_CTX *ctx);
sudo_dso_public void sudo_SHA256Transform(uint32_t state[8], const uint8_t buffer[SHA256_BLOCK_LENGTH]);
sudo_dso_public void sudo_SHA256Update(SHA2_CTX *ctx, const uint8_t *data, size_t len);
sudo_dso_public void sudo_SHA256Final(uint8_t digest[SHA256_DIGEST_LENGTH], SHA2_CTX *ctx);

#define SHA256Init		sudo_SHA256Init
#define SHA256Pad		sudo_SHA256Pad
#define SHA256Transform		sudo_SHA256Transform
#define SHA256Update		sudo_SHA256Update
#define SHA256Final		sudo_SHA256Final

sudo_dso_public void sudo_SHA384Init(SHA2_CTX *ctx);
sudo_dso_public void sudo_SHA384Pad(SHA2_CTX *ctx);
sudo_dso_public void sudo_SHA384Transform(uint64_t state[8], const uint8_t buffer[SHA384_BLOCK_LENGTH]);
sudo_dso_public void sudo_SHA384Update(SHA2_CTX *ctx, const uint8_t *data, size_t len);
sudo_dso_public void sudo_SHA384Final(uint8_t digest[SHA384_DIGEST_LENGTH], SHA2_CTX *ctx);

#define SHA384Init		sudo_SHA384Init
#define SHA384Pad		sudo_SHA384Pad
#define SHA384Transform		sudo_SHA384Transform
#define SHA384Update		sudo_SHA384Update
#define SHA384Final		sudo_SHA384Final

sudo_dso_public void sudo_SHA512Init(SHA2_CTX *ctx);
sudo_dso_public void sudo_SHA512Pad(SHA2_CTX *ctx);
sudo_dso_public void sudo_SHA512Transform(uint64_t state[8], const uint8_t buffer[SHA512_BLOCK_LENGTH]);
sudo_dso_public void sudo_SHA512Update(SHA2_CTX *ctx, const uint8_t *data, size_t len);
sudo_dso_public void sudo_SHA512Final(uint8_t digest[SHA512_DIGEST_LENGTH], SHA2_CTX *ctx);

#define SHA512Init		sudo_SHA512Init
#define SHA512Pad		sudo_SHA512Pad
#define SHA512Transform		sudo_SHA512Transform
#define SHA512Update		sudo_SHA512Update
#define SHA512Final		sudo_SHA512Final

#endif /* COMPAT_SHA2_H */
