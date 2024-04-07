/*
 * sha.c: routines to compute SHA-1/224/256/384/512 digests
 *
 * Ref: NIST FIPS PUB 180-4 Secure Hash Standard
 *
 * Copyright (C) 2003-2023 Mark Shelor, All Rights Reserved
 *
 * Version: 6.04
 * Sat Feb 25 12:00:50 PM MST 2023
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>
#include "sha.h"
#include "sha64bit.h"

#define W32	SHA32			/* useful abbreviations */
#define C32	SHA32_CONST
#define SR32	SHA32_SHR
#define SL32	SHA32_SHL
#define LO32	SHA_LO32
#define UCHR	unsigned char
#define UINT	unsigned int
#define ULNG	unsigned long
#define VP	void *

#define ROTR(x, n)	(SR32(x, n) | SL32(x, 32-(n)))
#define ROTL(x, n)	(SL32(x, n) | SR32(x, 32-(n)))

#define Ch(x, y, z)	((z) ^ ((x) & ((y) ^ (z))))
#define Pa(x, y, z)	((x) ^ (y) ^ (z))
#define Ma(x, y, z)	(((x) & (y)) | ((z) & ((x) | (y))))

#define SIGMA0(x)	(ROTR(x,  2) ^ ROTR(x, 13) ^ ROTR(x, 22))
#define SIGMA1(x)	(ROTR(x,  6) ^ ROTR(x, 11) ^ ROTR(x, 25))
#define sigma0(x)	(ROTR(x,  7) ^ ROTR(x, 18) ^ SR32(x,  3))
#define sigma1(x)	(ROTR(x, 17) ^ ROTR(x, 19) ^ SR32(x, 10))

#define K1	C32(0x5a827999)		/* SHA-1 constants */
#define K2	C32(0x6ed9eba1)
#define K3	C32(0x8f1bbcdc)
#define K4	C32(0xca62c1d6)

static const W32 K256[64] =		/* SHA-224/256 constants */
{
	C32(0x428a2f98), C32(0x71374491), C32(0xb5c0fbcf), C32(0xe9b5dba5),
	C32(0x3956c25b), C32(0x59f111f1), C32(0x923f82a4), C32(0xab1c5ed5),
	C32(0xd807aa98), C32(0x12835b01), C32(0x243185be), C32(0x550c7dc3),
	C32(0x72be5d74), C32(0x80deb1fe), C32(0x9bdc06a7), C32(0xc19bf174),
	C32(0xe49b69c1), C32(0xefbe4786), C32(0x0fc19dc6), C32(0x240ca1cc),
	C32(0x2de92c6f), C32(0x4a7484aa), C32(0x5cb0a9dc), C32(0x76f988da),
	C32(0x983e5152), C32(0xa831c66d), C32(0xb00327c8), C32(0xbf597fc7),
	C32(0xc6e00bf3), C32(0xd5a79147), C32(0x06ca6351), C32(0x14292967),
	C32(0x27b70a85), C32(0x2e1b2138), C32(0x4d2c6dfc), C32(0x53380d13),
	C32(0x650a7354), C32(0x766a0abb), C32(0x81c2c92e), C32(0x92722c85),
	C32(0xa2bfe8a1), C32(0xa81a664b), C32(0xc24b8b70), C32(0xc76c51a3),
	C32(0xd192e819), C32(0xd6990624), C32(0xf40e3585), C32(0x106aa070),
	C32(0x19a4c116), C32(0x1e376c08), C32(0x2748774c), C32(0x34b0bcb5),
	C32(0x391c0cb3), C32(0x4ed8aa4a), C32(0x5b9cca4f), C32(0x682e6ff3),
	C32(0x748f82ee), C32(0x78a5636f), C32(0x84c87814), C32(0x8cc70208),
	C32(0x90befffa), C32(0xa4506ceb), C32(0xbef9a3f7), C32(0xc67178f2)
};

static const W32 H01[8] =		/* SHA-1 initial hash value */
{
	C32(0x67452301), C32(0xefcdab89), C32(0x98badcfe), C32(0x10325476),
	C32(0xc3d2e1f0), C32(0x00000000), C32(0x00000000), C32(0x00000000)
};

static const W32 H0224[8] =		/* SHA-224 initial hash value */
{
	C32(0xc1059ed8), C32(0x367cd507), C32(0x3070dd17), C32(0xf70e5939),
	C32(0xffc00b31), C32(0x68581511), C32(0x64f98fa7), C32(0xbefa4fa4)
};

static const W32 H0256[8] =		/* SHA-256 initial hash value */
{
	C32(0x6a09e667), C32(0xbb67ae85), C32(0x3c6ef372), C32(0xa54ff53a),
	C32(0x510e527f), C32(0x9b05688c), C32(0x1f83d9ab), C32(0x5be0cd19)
};

static void sha1(SHA *s, UCHR *block)		/* SHA-1 transform */
{
	W32 a, b, c, d, e;
	W32 W[16];
	W32 *wp = W;
	W32 *H = s->H32;

	SHA32_SCHED(W, block);

/*
 * Use SHA-1 alternate method from FIPS PUB 180-4 (ref. 6.1.3)
 *
 * To improve performance, unroll the loop and consolidate assignments
 * by changing the roles of variables "a" through "e" at each step.
 * Note that the variable "T" is no longer needed.
 */

#define M1(a, b, c, d, e, f, k, w)		\
	e += ROTL(a, 5) + f(b, c, d) + k + w;	\
	b =  ROTL(b, 30)

#define M11(f, k, w)	M1(a, b, c, d, e, f, k, w);
#define M12(f, k, w)	M1(e, a, b, c, d, f, k, w);
#define M13(f, k, w)	M1(d, e, a, b, c, f, k, w);
#define M14(f, k, w)	M1(c, d, e, a, b, f, k, w);
#define M15(f, k, w)	M1(b, c, d, e, a, f, k, w);

#define W11(s)	W[(s+ 0) & 0xf]
#define W12(s)	W[(s+13) & 0xf]
#define W13(s)	W[(s+ 8) & 0xf]
#define W14(s)	W[(s+ 2) & 0xf]

#define A1(s)	(W11(s) = ROTL(W11(s) ^ W12(s) ^ W13(s) ^ W14(s), 1))

	a = H[0]; b = H[1]; c = H[2]; d = H[3]; e = H[4];

	M11(Ch, K1,  *wp++); M12(Ch, K1,  *wp++); M13(Ch, K1,  *wp++);
	M14(Ch, K1,  *wp++); M15(Ch, K1,  *wp++); M11(Ch, K1,  *wp++);
	M12(Ch, K1,  *wp++); M13(Ch, K1,  *wp++); M14(Ch, K1,  *wp++);
	M15(Ch, K1,  *wp++); M11(Ch, K1,  *wp++); M12(Ch, K1,  *wp++);
	M13(Ch, K1,  *wp++); M14(Ch, K1,  *wp++); M15(Ch, K1,  *wp++);
	M11(Ch, K1,  *wp  ); M12(Ch, K1, A1( 0)); M13(Ch, K1, A1( 1));
	M14(Ch, K1, A1( 2)); M15(Ch, K1, A1( 3)); M11(Pa, K2, A1( 4));
	M12(Pa, K2, A1( 5)); M13(Pa, K2, A1( 6)); M14(Pa, K2, A1( 7));
	M15(Pa, K2, A1( 8)); M11(Pa, K2, A1( 9)); M12(Pa, K2, A1(10));
	M13(Pa, K2, A1(11)); M14(Pa, K2, A1(12)); M15(Pa, K2, A1(13));
	M11(Pa, K2, A1(14)); M12(Pa, K2, A1(15)); M13(Pa, K2, A1( 0));
	M14(Pa, K2, A1( 1)); M15(Pa, K2, A1( 2)); M11(Pa, K2, A1( 3));
	M12(Pa, K2, A1( 4)); M13(Pa, K2, A1( 5)); M14(Pa, K2, A1( 6));
	M15(Pa, K2, A1( 7)); M11(Ma, K3, A1( 8)); M12(Ma, K3, A1( 9));
	M13(Ma, K3, A1(10)); M14(Ma, K3, A1(11)); M15(Ma, K3, A1(12));
	M11(Ma, K3, A1(13)); M12(Ma, K3, A1(14)); M13(Ma, K3, A1(15));
	M14(Ma, K3, A1( 0)); M15(Ma, K3, A1( 1)); M11(Ma, K3, A1( 2));
	M12(Ma, K3, A1( 3)); M13(Ma, K3, A1( 4)); M14(Ma, K3, A1( 5));
	M15(Ma, K3, A1( 6)); M11(Ma, K3, A1( 7)); M12(Ma, K3, A1( 8));
	M13(Ma, K3, A1( 9)); M14(Ma, K3, A1(10)); M15(Ma, K3, A1(11));
	M11(Pa, K4, A1(12)); M12(Pa, K4, A1(13)); M13(Pa, K4, A1(14));
	M14(Pa, K4, A1(15)); M15(Pa, K4, A1( 0)); M11(Pa, K4, A1( 1));
	M12(Pa, K4, A1( 2)); M13(Pa, K4, A1( 3)); M14(Pa, K4, A1( 4));
	M15(Pa, K4, A1( 5)); M11(Pa, K4, A1( 6)); M12(Pa, K4, A1( 7));
	M13(Pa, K4, A1( 8)); M14(Pa, K4, A1( 9)); M15(Pa, K4, A1(10));
	M11(Pa, K4, A1(11)); M12(Pa, K4, A1(12)); M13(Pa, K4, A1(13));
	M14(Pa, K4, A1(14)); M15(Pa, K4, A1(15));

	H[0] += a; H[1] += b; H[2] += c; H[3] += d; H[4] += e;
}

static void sha256(SHA *s, UCHR *block)		/* SHA-224/256 transform */
{
	W32 a, b, c, d, e, f, g, h, T1;
	W32 W[16];
	const W32 *kp = K256;
	W32 *wp = W;
	W32 *H = s->H32;

	SHA32_SCHED(W, block);

/*
 * Use same technique as in sha1()
 *
 * To improve performance, unroll the loop and consolidate assignments
 * by changing the roles of variables "a" through "h" at each step.
 * Note that the variable "T2" is no longer needed.
 */

#define M2(a, b, c, d, e, f, g, h, w)				\
	T1 = h  + SIGMA1(e) + Ch(e, f, g) + (*kp++) + w;	\
	h  = T1 + SIGMA0(a) + Ma(a, b, c); d += T1;

#define W21(s)	W[(s+ 0) & 0xf]
#define W22(s)	W[(s+14) & 0xf]
#define W23(s)	W[(s+ 9) & 0xf]
#define W24(s)	W[(s+ 1) & 0xf]

#define A2(s)	(W21(s) += sigma1(W22(s)) + W23(s) + sigma0(W24(s)))

#define M21(w)	M2(a, b, c, d, e, f, g, h, w)
#define M22(w)	M2(h, a, b, c, d, e, f, g, w)
#define M23(w)	M2(g, h, a, b, c, d, e, f, w)
#define M24(w)	M2(f, g, h, a, b, c, d, e, w)
#define M25(w)	M2(e, f, g, h, a, b, c, d, w)
#define M26(w)	M2(d, e, f, g, h, a, b, c, w)
#define M27(w)	M2(c, d, e, f, g, h, a, b, w)
#define M28(w)	M2(b, c, d, e, f, g, h, a, w)

	a = H[0]; b = H[1]; c = H[2]; d = H[3];
	e = H[4]; f = H[5]; g = H[6]; h = H[7];

	M21( *wp++); M22( *wp++); M23( *wp++); M24( *wp++);
	M25( *wp++); M26( *wp++); M27( *wp++); M28( *wp++);
	M21( *wp++); M22( *wp++); M23( *wp++); M24( *wp++);
	M25( *wp++); M26( *wp++); M27( *wp++); M28( *wp  );
	M21(A2( 0)); M22(A2( 1)); M23(A2( 2)); M24(A2( 3));
	M25(A2( 4)); M26(A2( 5)); M27(A2( 6)); M28(A2( 7));
	M21(A2( 8)); M22(A2( 9)); M23(A2(10)); M24(A2(11));
	M25(A2(12)); M26(A2(13)); M27(A2(14)); M28(A2(15));
	M21(A2( 0)); M22(A2( 1)); M23(A2( 2)); M24(A2( 3));
	M25(A2( 4)); M26(A2( 5)); M27(A2( 6)); M28(A2( 7));
	M21(A2( 8)); M22(A2( 9)); M23(A2(10)); M24(A2(11));
	M25(A2(12)); M26(A2(13)); M27(A2(14)); M28(A2(15));
	M21(A2( 0)); M22(A2( 1)); M23(A2( 2)); M24(A2( 3));
	M25(A2( 4)); M26(A2( 5)); M27(A2( 6)); M28(A2( 7));
	M21(A2( 8)); M22(A2( 9)); M23(A2(10)); M24(A2(11));
	M25(A2(12)); M26(A2(13)); M27(A2(14)); M28(A2(15));

	H[0] += a; H[1] += b; H[2] += c; H[3] += d;
	H[4] += e; H[5] += f; H[6] += g; H[7] += h;
}

#include "sha64bit.c"

#define BITSET(s, pos)	s[(pos) >> 3] &  (UCHR)  (0x01 << (7 - (pos) % 8))
#define SETBIT(s, pos)	s[(pos) >> 3] |= (UCHR)  (0x01 << (7 - (pos) % 8))
#define CLRBIT(s, pos)	s[(pos) >> 3] &= (UCHR) ~(0x01 << (7 - (pos) % 8))
#define NBYTES(nbits)	(((nbits) + 7) >> 3)
#define HEXLEN(nbytes)	((nbytes) << 1)
#define B64LEN(nbytes)	(((nbytes) % 3 == 0) ? ((nbytes) / 3) * 4 \
			: ((nbytes) / 3) * 4 + ((nbytes) % 3) + 1)

/* w32mem: writes 32-bit word to memory in big-endian order */
static UCHR *w32mem(UCHR *mem, W32 w32)
{
	int i;

	for (i = 0; i < 4; i++)
		*mem++ = (UCHR) (SR32(w32, 24-i*8) & 0xff);
	return(mem);
}

/* memw32: returns 32-bit word from memory written in big-endian order */
static W32 memw32(UCHR *mem)
{
	int i;
	W32 w = 0;

	for (i = 0; i < 4; i++)
		w = (w << 8) + *mem++;
	return(w);
}

/* digcpy: writes current state to digest buffer */
static UCHR *digcpy(SHA *s)
{
	int i;
	UCHR *d = s->digest;
	W32 *p32 = s->H32;
	W64 *p64 = s->H64;

	if (s->alg <= SHA256)
		for (i = 0; i < 8; i++, d += 4)
			w32mem(d, *p32++);
	else
		for (i = 0; i < 8; i++, d += 8) {
			w32mem(d, (W32) ((*p64 >> 16) >> 16));
			w32mem(d+4, (W32) (*p64++ & SHA32_MAX));
		}
	return(s->digest);
}

/* statecpy: writes buffer to current state (opposite of digcpy) */
static UCHR *statecpy(SHA *s, UCHR *buf)
{
	int i;
	W32 *p32 = s->H32;
	W64 *p64 = s->H64;

	if (s->alg <= SHA256)
		for (i = 0; i < 8; i++, buf += 4)
			*p32++ = memw32(buf);
	else
		for (i = 0; i < 8; i++, buf += 8)
			*p64++ = (((W64)memw32(buf) << 16) << 16) +
					memw32(buf+4);
	return(buf);
}

#define SHA_INIT(s, algo, transform, state, state_t)			\
	do {								\
		Zero(s, 1, SHA);					\
		s->alg = algo; s->sha = sha ## transform;		\
		Copy(H0 ## algo, s->state, 8, state_t);			\
		s->blocksize = SHA ## algo ## _BLOCK_BITS;		\
		s->digestlen = SHA ## algo ## _DIGEST_BITS >> 3;	\
	} while (0)

/* sharewind: resets digest object */
static void sharewind(SHA *s)
{
	if      (s->alg == SHA1)   SHA_INIT(s, 1, 1, H32, SHA32);
	else if (s->alg == SHA224) SHA_INIT(s, 224, 256, H32, SHA32);
	else if (s->alg == SHA256) SHA_INIT(s, 256, 256, H32, SHA32);
	else if (s->alg == SHA384) SHA_INIT(s, 384, 512, H64, SHA64);
	else if (s->alg == SHA512) SHA_INIT(s, 512, 512, H64, SHA64);
	else if (s->alg == SHA512224) SHA_INIT(s, 512224, 512, H64, SHA64);
	else if (s->alg == SHA512256) SHA_INIT(s, 512256, 512, H64, SHA64);
}

/* shainit: initializes digest object */
static int shainit(SHA *s, int alg)
{
	if (alg >= SHA384 && !sha_384_512)
		return 0;
	if (alg != SHA1 && alg != SHA224 && alg != SHA256 &&
		alg != SHA384    && alg != SHA512 &&
		alg != SHA512224 && alg != SHA512256)
		return 0;
	s->alg = alg;
	sharewind(s);
	return 1;
}

/* shadirect: updates state directly (w/o going through s->block) */
static ULNG shadirect(UCHR *bitstr, ULNG bitcnt, SHA *s)
{
	ULNG savecnt = bitcnt;

	while (bitcnt >= s->blocksize) {
		s->sha(s, bitstr);
		bitstr += (s->blocksize >> 3);
		bitcnt -= s->blocksize;
	}
	if (bitcnt > 0) {
		Copy(bitstr, s->block, NBYTES(bitcnt), char);
		s->blockcnt = bitcnt;
	}
	return(savecnt);
}

/* shabytes: updates state for byte-aligned data in s->block */
static ULNG shabytes(UCHR *bitstr, ULNG bitcnt, SHA *s)
{
	UINT offset;
	UINT nbits;
	ULNG savecnt = bitcnt;

	offset = s->blockcnt >> 3;
	if (s->blockcnt + bitcnt >= s->blocksize) {
		nbits = s->blocksize - s->blockcnt;
		Copy(bitstr, s->block+offset, nbits>>3, char);
		bitcnt -= nbits;
		bitstr += (nbits >> 3);
		s->sha(s, s->block), s->blockcnt = 0;
		shadirect(bitstr, bitcnt, s);
	}
	else {
		Copy(bitstr, s->block+offset, NBYTES(bitcnt), char);
		s->blockcnt += bitcnt;
	}
	return(savecnt);
}

/* shabits: updates state for bit-aligned data in s->block */
static ULNG shabits(UCHR *bitstr, ULNG bitcnt, SHA *s)
{
	ULNG i;

	for (i = 0UL; i < bitcnt; i++) {
		if (BITSET(bitstr, i))
			SETBIT(s->block, s->blockcnt);
		else
			CLRBIT(s->block, s->blockcnt);
		if (++s->blockcnt == s->blocksize)
			s->sha(s, s->block), s->blockcnt = 0;
	}
	return(bitcnt);
}

/* shawrite: triggers a state update using data in bitstr/bitcnt */
static ULNG shawrite(UCHR *bitstr, ULNG bitcnt, SHA *s)
{
	if (!bitcnt)
		return(0);
	if (SHA_LO32(s->lenll += bitcnt) < bitcnt)
		if (SHA_LO32(++s->lenlh) == 0)
			if (SHA_LO32(++s->lenhl) == 0)
				s->lenhh++;
	if (s->blockcnt == 0)
		return(shadirect(bitstr, bitcnt, s));
	else if (s->blockcnt % 8 == 0)
		return(shabytes(bitstr, bitcnt, s));
	else
		return(shabits(bitstr, bitcnt, s));
}

/* shafinish: pads remaining block(s) and computes final digest state */
static void shafinish(SHA *s)
{
	UINT lenpos, lhpos, llpos;

	lenpos = s->blocksize == SHA1_BLOCK_BITS ? 448 : 896;
	lhpos  = s->blocksize == SHA1_BLOCK_BITS ?  56 : 120;
	llpos  = s->blocksize == SHA1_BLOCK_BITS ?  60 : 124;
	SETBIT(s->block, s->blockcnt), s->blockcnt++;
	while (s->blockcnt > lenpos)
		if (s->blockcnt < s->blocksize)
			CLRBIT(s->block, s->blockcnt), s->blockcnt++;
		else
			s->sha(s, s->block), s->blockcnt = 0;
	while (s->blockcnt < lenpos)
		CLRBIT(s->block, s->blockcnt), s->blockcnt++;
	if (s->blocksize > SHA1_BLOCK_BITS) {
		w32mem(s->block + 112, s->lenhh);
		w32mem(s->block + 116, s->lenhl);
	}
	w32mem(s->block + lhpos, s->lenlh);
	w32mem(s->block + llpos, s->lenll);
	s->sha(s, s->block);
}

#define shadigest(state)	digcpy(state)

/* xmap: translation map for hexadecimal encoding */
static const char xmap[] =
	"0123456789abcdef";

/* shahex: returns pointer to current digest (hexadecimal) */
static char *shahex(SHA *s)
{
	UINT i;
	char *h;
	UCHR *d;

	d = digcpy(s);
	s->hex[0] = '\0';
	if (HEXLEN((size_t) s->digestlen) >= sizeof(s->hex))
		return(s->hex);
	for (i = 0, h = s->hex; i < s->digestlen; i++) {
		*h++ = xmap[(*d >> 4) & 0x0f];
		*h++ = xmap[(*d++   ) & 0x0f];
	}
	*h = '\0';
	return(s->hex);
}

/* bmap: translation map for Base 64 encoding */
static const char bmap[] =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/* encbase64: encodes input (0 to 3 bytes) into Base 64 */
static void encbase64(UCHR *in, UINT n, char *out)
{
	UCHR byte[3] = {0, 0, 0};

	out[0] = '\0';
	if (n < 1 || n > 3)
		return;
	Copy(in, byte, n, UCHR);
	out[0] = bmap[byte[0] >> 2];
	out[1] = bmap[((byte[0] & 0x03) << 4) | (byte[1] >> 4)];
	out[2] = bmap[((byte[1] & 0x0f) << 2) | (byte[2] >> 6)];
	out[3] = bmap[byte[2] & 0x3f];
	out[n+1] = '\0';
}

/* shabase64: returns pointer to current digest (Base 64) */
static char *shabase64(SHA *s)
{
	UINT n;
	UCHR *q;
	char out[5];

	q = digcpy(s);
	s->base64[0] = '\0';
	if (B64LEN((size_t) s->digestlen) >= sizeof(s->base64))
		return(s->base64);
	for (n = s->digestlen; n > 3; n -= 3, q += 3) {
		encbase64(q, 3, out);
		strcat(s->base64, out);
	}
	encbase64(q, n, out);
	strcat(s->base64, out);
	return(s->base64);
}

/* hmacinit: initializes HMAC-SHA digest object */
static HMAC *hmacinit(HMAC *h, int alg, UCHR *key, UINT keylen)
{
	UINT i;
	SHA ksha;

	Zero(h, 1, HMAC);
	if (!shainit(&h->isha, alg))
		return(NULL);
	if (!shainit(&h->osha, alg))
		return(NULL);
	if (keylen <= h->osha.blocksize / 8)
		Copy(key, h->key, keylen, char);
	else {
		if (!shainit(&ksha, alg))
			return(NULL);
		shawrite(key, keylen * 8, &ksha);
		shafinish(&ksha);
		Copy(digcpy(&ksha), h->key, ksha.digestlen, char);
	}
	h->digestlen = h->osha.digestlen;
	for (i = 0; i < h->osha.blocksize / 8; i++)
		h->key[i] ^= 0x5c;
	shawrite(h->key, h->osha.blocksize, &h->osha);
	for (i = 0; i < h->isha.blocksize / 8; i++)
		h->key[i] ^= (0x5c ^ 0x36);
	shawrite(h->key, h->isha.blocksize, &h->isha);
	Zero(h->key, sizeof(h->key), char);
	return(h);
}

/* hmacwrite: triggers a state update using data in bitstr/bitcnt */
static ULNG hmacwrite(UCHR *bitstr, ULNG bitcnt, HMAC *h)
{
	return(shawrite(bitstr, bitcnt, &h->isha));
}

/* hmacfinish: computes final digest state */
static void hmacfinish(HMAC *h)
{
	shafinish(&h->isha);
	shawrite(digcpy(&h->isha), h->isha.digestlen * 8, &h->osha);
	shafinish(&h->osha);
}

#define hmacdigest(h)	digcpy(&(h)->osha)

/* hmachex: returns pointer to digest (hexadecimal) */
static char *hmachex(HMAC *h)
{
	return(shahex(&h->osha));
}

/* hmacbase64: returns pointer to digest (Base 64) */
static char *hmacbase64(HMAC *h)
{
	return(shabase64(&h->osha));
}
