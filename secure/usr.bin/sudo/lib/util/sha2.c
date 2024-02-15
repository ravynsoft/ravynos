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
 * This is an open source non-commercial project. Dear PVS-Studio, please check it.
 * PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
 */

/*
 * Implementation of SHA-224, SHA-256, SHA-384 and SHA-512
 * as per FIPS 180-4: Secure Hash Standard (SHS)
 * http://csrc.nist.gov/publications/fips/fips180-4/fips-180-4.pdf
 *
 * Derived from the public domain SHA-1 and SHA-2 implementations
 * by Steve Reid and Wei Dai respectively.
 */

#include <config.h>
#include <string.h>
#if defined(HAVE_STDINT_H)
# include <stdint.h>
#elif defined(HAVE_INTTYPES_H)
# include <inttypes.h>
#endif
#if defined(HAVE_ENDIAN_H)
# include <endian.h>
#elif defined(HAVE_SYS_ENDIAN_H)
# include <sys/endian.h>
#elif defined(HAVE_MACHINE_ENDIAN_H)
# include <machine/endian.h>
#else
# include <compat/endian.h>
#endif

#include <sudo_compat.h>
#include <compat/sha2.h>

/*
 * SHA-2 operates on 32-bit and 64-bit words in big endian byte order.
 * The following macros convert between character arrays and big endian words.
 */
#define BE8TO32(x, y) do {				\
	(x) = (((uint32_t)((y)[0] & 255) << 24) |	\
	       ((uint32_t)((y)[1] & 255) << 16) |	\
	       ((uint32_t)((y)[2] & 255) << 8)  |	\
	       ((uint32_t)((y)[3] & 255)));		\
} while (0)

#define BE8TO64(x, y) do {				\
	(x) = (((uint64_t)((y)[0] & 255) << 56) |	\
	       ((uint64_t)((y)[1] & 255) << 48) |	\
	       ((uint64_t)((y)[2] & 255) << 40) |	\
	       ((uint64_t)((y)[3] & 255) << 32) |	\
	       ((uint64_t)((y)[4] & 255) << 24) |	\
	       ((uint64_t)((y)[5] & 255) << 16) |	\
	       ((uint64_t)((y)[6] & 255) << 8)  |	\
	       ((uint64_t)((y)[7] & 255)));		\
} while (0)

#define BE32TO8(x, y) do {			\
	(x)[0] = (uint8_t)(((y) >> 24) & 255);	\
	(x)[1] = (uint8_t)(((y) >> 16) & 255);	\
	(x)[2] = (uint8_t)(((y) >> 8) & 255);	\
	(x)[3] = (uint8_t)((y) & 255);		\
} while (0)

#define BE64TO8(x, y) do {			\
	(x)[0] = (uint8_t)(((y) >> 56) & 255);	\
	(x)[1] = (uint8_t)(((y) >> 48) & 255);	\
	(x)[2] = (uint8_t)(((y) >> 40) & 255);	\
	(x)[3] = (uint8_t)(((y) >> 32) & 255);	\
	(x)[4] = (uint8_t)(((y) >> 24) & 255);	\
	(x)[5] = (uint8_t)(((y) >> 16) & 255);	\
	(x)[6] = (uint8_t)(((y) >> 8) & 255);	\
	(x)[7] = (uint8_t)((y) & 255);		\
} while (0)

#define rotrFixed(x,y) (y ? ((x>>y) | (x<<(sizeof(x)*8-y))) : x)

#define blk0(i) (W[i])
#define blk2(i) (W[i&15]+=s1(W[(i-2)&15])+W[(i-7)&15]+s0(W[(i-15)&15]))

#define Ch(x,y,z) (z^(x&(y^z)))
#define Maj(x,y,z) (y^((x^y)&(y^z)))

#define a(i) T[(0-i)&7]
#define b(i) T[(1-i)&7]
#define c(i) T[(2-i)&7]
#define d(i) T[(3-i)&7]
#define e(i) T[(4-i)&7]
#define f(i) T[(5-i)&7]
#define g(i) T[(6-i)&7]
#define h(i) T[(7-i)&7]

void
SHA224Init(SHA2_CTX *ctx)
{
	memset(ctx, 0, sizeof(*ctx));
	ctx->state.st32[0] = 0xc1059ed8UL;
	ctx->state.st32[1] = 0x367cd507UL;
	ctx->state.st32[2] = 0x3070dd17UL;
	ctx->state.st32[3] = 0xf70e5939UL;
	ctx->state.st32[4] = 0xffc00b31UL;
	ctx->state.st32[5] = 0x68581511UL;
	ctx->state.st32[6] = 0x64f98fa7UL;
	ctx->state.st32[7] = 0xbefa4fa4UL;
}

void
SHA224Transform(uint32_t state[8], const uint8_t buffer[SHA224_BLOCK_LENGTH])
{
	SHA256Transform(state, buffer);
}

void
SHA224Update(SHA2_CTX *ctx, const uint8_t *data, size_t len)
{
	SHA256Update(ctx, data, len);
}

void
SHA224Pad(SHA2_CTX *ctx)
{
	SHA256Pad(ctx);
}

void
SHA224Final(uint8_t digest[SHA224_DIGEST_LENGTH], SHA2_CTX *ctx)
{
	SHA256Pad(ctx);
	if (digest != NULL) {
#if BYTE_ORDER == BIG_ENDIAN
		memcpy(digest, ctx->state.st32, SHA224_DIGEST_LENGTH);
#else
		unsigned int i;

		for (i = 0; i < 7; i++)
			BE32TO8(digest + (i * 4), ctx->state.st32[i]);
#endif
		memset(ctx, 0, sizeof(*ctx));
	}
}

static const uint32_t SHA256_K[64] = {
	0x428a2f98UL, 0x71374491UL, 0xb5c0fbcfUL, 0xe9b5dba5UL,
	0x3956c25bUL, 0x59f111f1UL, 0x923f82a4UL, 0xab1c5ed5UL,
	0xd807aa98UL, 0x12835b01UL, 0x243185beUL, 0x550c7dc3UL,
	0x72be5d74UL, 0x80deb1feUL, 0x9bdc06a7UL, 0xc19bf174UL,
	0xe49b69c1UL, 0xefbe4786UL, 0x0fc19dc6UL, 0x240ca1ccUL,
	0x2de92c6fUL, 0x4a7484aaUL, 0x5cb0a9dcUL, 0x76f988daUL,
	0x983e5152UL, 0xa831c66dUL, 0xb00327c8UL, 0xbf597fc7UL,
	0xc6e00bf3UL, 0xd5a79147UL, 0x06ca6351UL, 0x14292967UL,
	0x27b70a85UL, 0x2e1b2138UL, 0x4d2c6dfcUL, 0x53380d13UL,
	0x650a7354UL, 0x766a0abbUL, 0x81c2c92eUL, 0x92722c85UL,
	0xa2bfe8a1UL, 0xa81a664bUL, 0xc24b8b70UL, 0xc76c51a3UL,
	0xd192e819UL, 0xd6990624UL, 0xf40e3585UL, 0x106aa070UL,
	0x19a4c116UL, 0x1e376c08UL, 0x2748774cUL, 0x34b0bcb5UL,
	0x391c0cb3UL, 0x4ed8aa4aUL, 0x5b9cca4fUL, 0x682e6ff3UL,
	0x748f82eeUL, 0x78a5636fUL, 0x84c87814UL, 0x8cc70208UL,
	0x90befffaUL, 0xa4506cebUL, 0xbef9a3f7UL, 0xc67178f2UL
};

void
SHA256Init(SHA2_CTX *ctx)
{
	memset(ctx, 0, sizeof(*ctx));
	ctx->state.st32[0] = 0x6a09e667UL;
	ctx->state.st32[1] = 0xbb67ae85UL;
	ctx->state.st32[2] = 0x3c6ef372UL;
	ctx->state.st32[3] = 0xa54ff53aUL;
	ctx->state.st32[4] = 0x510e527fUL;
	ctx->state.st32[5] = 0x9b05688cUL;
	ctx->state.st32[6] = 0x1f83d9abUL;
	ctx->state.st32[7] = 0x5be0cd19UL;
}

/* Round macros for SHA256 */
#define R(i) do {							     \
	h(i)+=S1(e(i))+Ch(e(i),f(i),g(i))+SHA256_K[i+j]+(j?blk2(i):blk0(i)); \
	d(i)+=h(i);							     \
	h(i)+=S0(a(i))+Maj(a(i),b(i),c(i));				     \
} while (0)

#define S0(x) (rotrFixed(x,2)^rotrFixed(x,13)^rotrFixed(x,22))
#define S1(x) (rotrFixed(x,6)^rotrFixed(x,11)^rotrFixed(x,25))
#define s0(x) (rotrFixed(x,7)^rotrFixed(x,18)^(x>>3))
#define s1(x) (rotrFixed(x,17)^rotrFixed(x,19)^(x>>10))

void
SHA256Transform(uint32_t state[8], const uint8_t data[SHA256_BLOCK_LENGTH])
{
	uint32_t W[16];
	uint32_t T[8];
	unsigned int j;

	/* Copy context state to working vars. */
	memcpy(T, state, sizeof(T));
	/* Copy data to W in big endian format. */
#if BYTE_ORDER == BIG_ENDIAN
	memcpy(W, data, sizeof(W));
#else
	for (j = 0; j < 16; j++) {
	    BE8TO32(W[j], data);
	    data += 4;
	}
#endif
	/* 64 operations, partially loop unrolled. */
	for (j = 0; j < 64; j += 16)
	{
		R( 0); R( 1); R( 2); R( 3);
		R( 4); R( 5); R( 6); R( 7);
		R( 8); R( 9); R(10); R(11);
		R(12); R(13); R(14); R(15);
	}
	/* Add the working vars back into context state. */
	state[0] += a(0);
	state[1] += b(0);
	state[2] += c(0);
	state[3] += d(0);
	state[4] += e(0);
	state[5] += f(0);
	state[6] += g(0);
	state[7] += h(0);
	/* Cleanup */
	explicit_bzero(T, sizeof(T));
	explicit_bzero(W, sizeof(W));
}

#undef S0
#undef S1
#undef s0
#undef s1
#undef R

void
SHA256Update(SHA2_CTX *ctx, const uint8_t *data, size_t len)
{
	size_t i = 0, j;

	j = (size_t)((ctx->count[0] >> 3) & (SHA256_BLOCK_LENGTH - 1));
	ctx->count[0] += ((uint64_t)len << 3);
	if ((j + len) > SHA256_BLOCK_LENGTH - 1) {
		memcpy(&ctx->buffer[j], data, (i = SHA256_BLOCK_LENGTH - j));
		SHA256Transform(ctx->state.st32, ctx->buffer);
		for ( ; i + SHA256_BLOCK_LENGTH - 1 < len; i += SHA256_BLOCK_LENGTH)
			SHA256Transform(ctx->state.st32, (uint8_t *)&data[i]);
		j = 0;
	}
	memcpy(&ctx->buffer[j], &data[i], len - i);
}

void
SHA256Pad(SHA2_CTX *ctx)
{
	uint8_t finalcount[8];

	/* Store unpadded message length in bits in big endian format. */
	BE64TO8(finalcount, ctx->count[0]);

	/* Append a '1' bit (0x80) to the message. */
	SHA256Update(ctx, (uint8_t *)"\200", 1);

	/* Pad message such that the resulting length modulo 512 is 448. */
	while ((ctx->count[0] & 511) != 448)
		SHA256Update(ctx, (uint8_t *)"\0", 1);

	/* Append length of message in bits and do final SHA256Transform(). */
	SHA256Update(ctx, finalcount, sizeof(finalcount));
}

void
SHA256Final(uint8_t digest[SHA256_DIGEST_LENGTH], SHA2_CTX *ctx)
{
	SHA256Pad(ctx);
	if (digest != NULL) {
#if BYTE_ORDER == BIG_ENDIAN
		memcpy(digest, ctx->state.st32, SHA256_DIGEST_LENGTH);
#else
		unsigned int i;

		for (i = 0; i < 8; i++)
			BE32TO8(digest + (i * 4), ctx->state.st32[i]);
#endif
		memset(ctx, 0, sizeof(*ctx));
	}
}

void
SHA384Init(SHA2_CTX *ctx)
{
	memset(ctx, 0, sizeof(*ctx));
	ctx->state.st64[0] = 0xcbbb9d5dc1059ed8ULL;
	ctx->state.st64[1] = 0x629a292a367cd507ULL;
	ctx->state.st64[2] = 0x9159015a3070dd17ULL;
	ctx->state.st64[3] = 0x152fecd8f70e5939ULL;
	ctx->state.st64[4] = 0x67332667ffc00b31ULL;
	ctx->state.st64[5] = 0x8eb44a8768581511ULL;
	ctx->state.st64[6] = 0xdb0c2e0d64f98fa7ULL;
	ctx->state.st64[7] = 0x47b5481dbefa4fa4ULL;
}

void
SHA384Transform(uint64_t state[8], const uint8_t data[SHA384_BLOCK_LENGTH])
{
	SHA512Transform(state, data);
}

void
SHA384Update(SHA2_CTX *ctx, const uint8_t *data, size_t len)
{
	SHA512Update(ctx, data, len);
}

void
SHA384Pad(SHA2_CTX *ctx)
{
	SHA512Pad(ctx);
}

void
SHA384Final(uint8_t digest[SHA384_DIGEST_LENGTH], SHA2_CTX *ctx)
{
	SHA384Pad(ctx);
	if (digest != NULL) {
#if BYTE_ORDER == BIG_ENDIAN
		memcpy(digest, ctx->state.st64, SHA384_DIGEST_LENGTH);
#else
		unsigned int i;

		for (i = 0; i < 6; i++)
			BE64TO8(digest + (i * 8), ctx->state.st64[i]);
#endif
		memset(ctx, 0, sizeof(*ctx));
	}
}

static const uint64_t SHA512_K[80] = {
	0x428a2f98d728ae22ULL, 0x7137449123ef65cdULL,
	0xb5c0fbcfec4d3b2fULL, 0xe9b5dba58189dbbcULL,
	0x3956c25bf348b538ULL, 0x59f111f1b605d019ULL,
	0x923f82a4af194f9bULL, 0xab1c5ed5da6d8118ULL,
	0xd807aa98a3030242ULL, 0x12835b0145706fbeULL,
	0x243185be4ee4b28cULL, 0x550c7dc3d5ffb4e2ULL,
	0x72be5d74f27b896fULL, 0x80deb1fe3b1696b1ULL,
	0x9bdc06a725c71235ULL, 0xc19bf174cf692694ULL,
	0xe49b69c19ef14ad2ULL, 0xefbe4786384f25e3ULL,
	0x0fc19dc68b8cd5b5ULL, 0x240ca1cc77ac9c65ULL,
	0x2de92c6f592b0275ULL, 0x4a7484aa6ea6e483ULL,
	0x5cb0a9dcbd41fbd4ULL, 0x76f988da831153b5ULL,
	0x983e5152ee66dfabULL, 0xa831c66d2db43210ULL,
	0xb00327c898fb213fULL, 0xbf597fc7beef0ee4ULL,
	0xc6e00bf33da88fc2ULL, 0xd5a79147930aa725ULL,
	0x06ca6351e003826fULL, 0x142929670a0e6e70ULL,
	0x27b70a8546d22ffcULL, 0x2e1b21385c26c926ULL,
	0x4d2c6dfc5ac42aedULL, 0x53380d139d95b3dfULL,
	0x650a73548baf63deULL, 0x766a0abb3c77b2a8ULL,
	0x81c2c92e47edaee6ULL, 0x92722c851482353bULL,
	0xa2bfe8a14cf10364ULL, 0xa81a664bbc423001ULL,
	0xc24b8b70d0f89791ULL, 0xc76c51a30654be30ULL,
	0xd192e819d6ef5218ULL, 0xd69906245565a910ULL,
	0xf40e35855771202aULL, 0x106aa07032bbd1b8ULL,
	0x19a4c116b8d2d0c8ULL, 0x1e376c085141ab53ULL,
	0x2748774cdf8eeb99ULL, 0x34b0bcb5e19b48a8ULL,
	0x391c0cb3c5c95a63ULL, 0x4ed8aa4ae3418acbULL,
	0x5b9cca4f7763e373ULL, 0x682e6ff3d6b2b8a3ULL,
	0x748f82ee5defb2fcULL, 0x78a5636f43172f60ULL,
	0x84c87814a1f0ab72ULL, 0x8cc702081a6439ecULL,
	0x90befffa23631e28ULL, 0xa4506cebde82bde9ULL,
	0xbef9a3f7b2c67915ULL, 0xc67178f2e372532bULL,
	0xca273eceea26619cULL, 0xd186b8c721c0c207ULL,
	0xeada7dd6cde0eb1eULL, 0xf57d4f7fee6ed178ULL,
	0x06f067aa72176fbaULL, 0x0a637dc5a2c898a6ULL,
	0x113f9804bef90daeULL, 0x1b710b35131c471bULL,
	0x28db77f523047d84ULL, 0x32caab7b40c72493ULL,
	0x3c9ebe0a15c9bebcULL, 0x431d67c49c100d4cULL,
	0x4cc5d4becb3e42b6ULL, 0x597f299cfc657e2aULL,
	0x5fcb6fab3ad6faecULL, 0x6c44198c4a475817ULL
};

void
SHA512Init(SHA2_CTX *ctx)
{
	memset(ctx, 0, sizeof(*ctx));
	ctx->state.st64[0] = 0x6a09e667f3bcc908ULL;
	ctx->state.st64[1] = 0xbb67ae8584caa73bULL;
	ctx->state.st64[2] = 0x3c6ef372fe94f82bULL;
	ctx->state.st64[3] = 0xa54ff53a5f1d36f1ULL;
	ctx->state.st64[4] = 0x510e527fade682d1ULL;
	ctx->state.st64[5] = 0x9b05688c2b3e6c1fULL;
	ctx->state.st64[6] = 0x1f83d9abfb41bd6bULL;
	ctx->state.st64[7] = 0x5be0cd19137e2179ULL;
}

/* Round macros for SHA512 */
#define R(i) do {							     \
	h(i)+=S1(e(i))+Ch(e(i),f(i),g(i))+SHA512_K[i+j]+(j?blk2(i):blk0(i)); \
	d(i)+=h(i);							     \
	h(i)+=S0(a(i))+Maj(a(i),b(i),c(i));				     \
} while (0)

#define S0(x) (rotrFixed(x,28)^rotrFixed(x,34)^rotrFixed(x,39))
#define S1(x) (rotrFixed(x,14)^rotrFixed(x,18)^rotrFixed(x,41))
#define s0(x) (rotrFixed(x,1)^rotrFixed(x,8)^(x>>7))
#define s1(x) (rotrFixed(x,19)^rotrFixed(x,61)^(x>>6))

void
SHA512Transform(uint64_t state[8], const uint8_t data[SHA512_BLOCK_LENGTH])
{
	uint64_t W[16];
	uint64_t T[8];
	unsigned int j;

	/* Copy context state to working vars. */
	memcpy(T, state, sizeof(T));
	/* Copy data to W in big endian format. */
#if BYTE_ORDER == BIG_ENDIAN
	memcpy(W, data, sizeof(W));
#else
	for (j = 0; j < 16; j++) {
	    BE8TO64(W[j], data);
	    data += 8;
	}
#endif
	/* 80 operations, partially loop unrolled. */
	for (j = 0; j < 80; j += 16)
	{
		R( 0); R( 1); R( 2); R( 3);
		R( 4); R( 5); R( 6); R( 7);
		R( 8); R( 9); R(10); R(11);
		R(12); R(13); R(14); R(15);
	}
	/* Add the working vars back into context state. */
	state[0] += a(0);
	state[1] += b(0);
	state[2] += c(0);
	state[3] += d(0);
	state[4] += e(0);
	state[5] += f(0);
	state[6] += g(0);
	state[7] += h(0);
	/* Cleanup. */
	explicit_bzero(T, sizeof(T));
	explicit_bzero(W, sizeof(W));
}

void
SHA512Update(SHA2_CTX *ctx, const uint8_t *data, size_t len)
{
	size_t i = 0, j;

	j = (size_t)((ctx->count[0] >> 3) & (SHA512_BLOCK_LENGTH - 1));
	ctx->count[0] += ((uint64_t)len << 3);
	if (ctx->count[0] < ((uint64_t)len << 3))
		ctx->count[1]++;
	if ((j + len) > SHA512_BLOCK_LENGTH - 1) {
		memcpy(&ctx->buffer[j], data, (i = SHA512_BLOCK_LENGTH - j));
		SHA512Transform(ctx->state.st64, ctx->buffer);
		for ( ; i + SHA512_BLOCK_LENGTH - 1 < len; i += SHA512_BLOCK_LENGTH)
			SHA512Transform(ctx->state.st64, (uint8_t *)&data[i]);
		j = 0;
	}
	memcpy(&ctx->buffer[j], &data[i], len - i);
}

void
SHA512Pad(SHA2_CTX *ctx)
{
	uint8_t finalcount[16];

	/* Store unpadded message length in bits in big endian format. */
	BE64TO8(finalcount, ctx->count[1]);
	BE64TO8(finalcount + 8, ctx->count[0]);

	/* Append a '1' bit (0x80) to the message. */
	SHA512Update(ctx, (uint8_t *)"\200", 1);

	/* Pad message such that the resulting length modulo 1024 is 896. */
	while ((ctx->count[0] & 1023) != 896)
		SHA512Update(ctx, (uint8_t *)"\0", 1);

	/* Append length of message in bits and do final SHA512Transform(). */
	SHA512Update(ctx, finalcount, sizeof(finalcount));
}

void
SHA512Final(uint8_t digest[SHA512_DIGEST_LENGTH], SHA2_CTX *ctx)
{
	SHA512Pad(ctx);
	if (digest != NULL) {
#if BYTE_ORDER == BIG_ENDIAN
		memcpy(digest, ctx->state.st64, SHA512_DIGEST_LENGTH);
#else
		unsigned int i;

		for (i = 0; i < 8; i++)
			BE64TO8(digest + (i * 8), ctx->state.st64[i]);
#endif
		memset(ctx, 0, sizeof(*ctx));
	}
}
