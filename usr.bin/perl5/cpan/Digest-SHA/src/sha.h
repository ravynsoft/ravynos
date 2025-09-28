/*
 * sha.h: header file for SHA-1/224/256/384/512 routines
 *
 * Ref: NIST FIPS PUB 180-4 Secure Hash Standard
 *
 * Copyright (C) 2003-2023 Mark Shelor, All Rights Reserved
 *
 * Version: 6.04
 * Sat Feb 25 12:00:50 PM MST 2023
 *
 */

#ifndef _INCLUDE_SHA_H_
#define _INCLUDE_SHA_H_

#include <limits.h>

#define SHA32_MAX	4294967295U

#define SHA32_SHR(x, n)	((x) >> (n))
#define SHA32_SHL(x, n)	((x) << (n))

#define SHA64_SHR(x, n)	((x) >> (n))
#define SHA64_SHL(x, n)	((x) << (n))

#define SHA32_ALIGNED
#define SHA64_ALIGNED

#define SHA_LO32(x)	(x)

#if USHRT_MAX == SHA32_MAX
	#define SHA32	unsigned short
	#define SHA32_CONST(c)	c ## U
#elif UINT_MAX == SHA32_MAX
	#define SHA32	unsigned int
	#define SHA32_CONST(c)	c ## U
#elif ULONG_MAX == SHA32_MAX
	#define SHA32	unsigned long
	#define SHA32_CONST(c)	c ## UL
#else
	#undef  SHA32_ALIGNED
	#undef  SHA_LO32
	#define SHA_LO32(x)	((x) & SHA32_MAX)
	#undef  SHA32_SHR
	#define SHA32_SHR(x, n)	(SHA_LO32(x) >> (n))
	#define SHA32	unsigned long
	#define SHA32_CONST(c)	c ## UL
#endif

#if defined(ULONG_LONG_MAX) || defined(ULLONG_MAX) || defined(HAS_LONG_LONG)
	#define SHA_ULL_EXISTS
#endif

#if (((ULONG_MAX >> 16) >> 16) >> 16) >> 15 == 1UL
	#define SHA64	unsigned long
	#define SHA64_CONST(c)	c ## UL
#elif defined(SHA_ULL_EXISTS) && defined(LONGLONGSIZE) && LONGLONGSIZE == 8
	#define SHA64	unsigned long long
	#define SHA64_CONST(c)	c ## ULL
#elif defined(SHA_ULL_EXISTS)
	#undef  SHA64_ALIGNED
	#undef  SHA64_SHR
	#define SHA64_MAX	18446744073709551615ULL
	#define SHA64_SHR(x, n)	(((x) & SHA64_MAX) >> (n))
	#define SHA64	unsigned long long
	#define SHA64_CONST(c)	c ## ULL

	/* The following cases detect compilers that
	 * support 64-bit types in a non-standard way */

#elif defined(_MSC_VER)					/* Microsoft C */
	#define SHA64	unsigned __int64
	#define SHA64_CONST(c)	(SHA64) c
#endif

#if defined(SHA64) && !defined(NO_SHA_384_512)
	#define SHA_384_512
#endif

#if defined(BYTEORDER) && (BYTEORDER & 0xffff) == 0x4321
	#if defined(SHA32_ALIGNED)
		#define SHA32_SCHED(W, b)	Copy(b, W, 64, char)
	#endif
	#if defined(SHA64) && defined(SHA64_ALIGNED)
		#define SHA64_SCHED(W, b)	Copy(b, W, 128, char)
	#endif
#endif

#if !defined(SHA32_SCHED)
	#define SHA32_SCHED(W, b) { int t; SHA32 *q = W;		\
		for (t = 0; t < 16; t++, b += 4) *q++ =			\
			(SHA32) b[0] << 24 | (SHA32) b[1] << 16 |	\
			(SHA32) b[2] <<  8 | (SHA32) b[3]; }
#endif

#if defined(SHA64) && !defined(SHA64_SCHED)
	#define SHA64_SCHED(W, b) { int t; SHA64 *q = W;		\
		for (t = 0; t < 16; t++, b += 8) *q++ =			\
			(SHA64) b[0] << 56 | (SHA64) b[1] << 48 |	\
			(SHA64) b[2] << 40 | (SHA64) b[3] << 32 |	\
			(SHA64) b[4] << 24 | (SHA64) b[5] << 16 |	\
			(SHA64) b[6] <<  8 | (SHA64) b[7]; }
#endif

#define SHA1		1
#define SHA224		224
#define SHA256		256
#define SHA384		384
#define SHA512		512
#define SHA512224	512224
#define SHA512256	512256

#define SHA1_BLOCK_BITS		512
#define SHA224_BLOCK_BITS	SHA1_BLOCK_BITS
#define SHA256_BLOCK_BITS	SHA1_BLOCK_BITS
#define SHA384_BLOCK_BITS	1024
#define SHA512_BLOCK_BITS	SHA384_BLOCK_BITS
#define SHA512224_BLOCK_BITS	SHA512_BLOCK_BITS
#define SHA512256_BLOCK_BITS	SHA512_BLOCK_BITS

#define SHA1_DIGEST_BITS	160
#define SHA224_DIGEST_BITS	224
#define SHA256_DIGEST_BITS	256
#define SHA384_DIGEST_BITS	384
#define SHA512_DIGEST_BITS	512
#define SHA512224_DIGEST_BITS	224
#define SHA512256_DIGEST_BITS	256

#define SHA_MAX_BLOCK_BITS	SHA512_BLOCK_BITS
#define SHA_MAX_DIGEST_BITS	SHA512_DIGEST_BITS
#define SHA_MAX_HEX_LEN		(SHA_MAX_DIGEST_BITS / 4)
#define SHA_MAX_BASE64_LEN	(1 + (SHA_MAX_DIGEST_BITS / 6))

#if !defined(SHA64)
	#define SHA64	SHA32
#endif

typedef struct SHA {
	int alg;
	void (*sha)(struct SHA *, unsigned char *);
	SHA32 H32[8];
	SHA64 H64[8];
	unsigned char block[SHA_MAX_BLOCK_BITS/8];
	unsigned int blockcnt;
	unsigned int blocksize;
	SHA32 lenhh, lenhl, lenlh, lenll;
	unsigned char digest[SHA_MAX_DIGEST_BITS/8];
	unsigned int digestlen;
	char hex[SHA_MAX_HEX_LEN+1];
	char base64[SHA_MAX_BASE64_LEN+1];
} SHA;

typedef struct {
	SHA isha;
	SHA osha;
	unsigned int digestlen;
	unsigned char key[SHA_MAX_BLOCK_BITS/8];
} HMAC;

#endif	/* _INCLUDE_SHA_H_ */
