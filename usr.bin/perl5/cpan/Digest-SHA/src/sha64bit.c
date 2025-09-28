/*
 * sha64bit.c: routines to compute SHA-384/512 digests
 *
 * Ref: NIST FIPS PUB 180-4 Secure Hash Standard
 *
 * Copyright (C) 2003-2023 Mark Shelor, All Rights Reserved
 *
 * Version: 6.04
 * Sat Feb 25 12:00:50 PM MST 2023
 *
 */

#ifdef SHA_384_512

#undef sha_384_512
#undef W64
#undef sha512
#undef H0384
#undef H0512
#undef H0512224
#undef H0512256

#define sha_384_512	1

#define W64		SHA64		/* useful abbreviations */
#define C64		SHA64_CONST
#define SR64		SHA64_SHR
#define SL64		SHA64_SHL

#define ROTRQ(x, n)	(SR64(x, n) | SL64(x, 64-(n)))
#define SIGMAQ0(x)	(ROTRQ(x, 28) ^ ROTRQ(x, 34) ^ ROTRQ(x, 39))
#define SIGMAQ1(x)	(ROTRQ(x, 14) ^ ROTRQ(x, 18) ^ ROTRQ(x, 41))
#define sigmaQ0(x)	(ROTRQ(x,  1) ^ ROTRQ(x,  8) ^ SR64(x,  7))
#define sigmaQ1(x)	(ROTRQ(x, 19) ^ ROTRQ(x, 61) ^ SR64(x,  6))

static const W64 K512[80] =		/* SHA-384/512 constants */
{
C64(0x428a2f98d728ae22), C64(0x7137449123ef65cd), C64(0xb5c0fbcfec4d3b2f),
C64(0xe9b5dba58189dbbc), C64(0x3956c25bf348b538), C64(0x59f111f1b605d019),
C64(0x923f82a4af194f9b), C64(0xab1c5ed5da6d8118), C64(0xd807aa98a3030242),
C64(0x12835b0145706fbe), C64(0x243185be4ee4b28c), C64(0x550c7dc3d5ffb4e2),
C64(0x72be5d74f27b896f), C64(0x80deb1fe3b1696b1), C64(0x9bdc06a725c71235),
C64(0xc19bf174cf692694), C64(0xe49b69c19ef14ad2), C64(0xefbe4786384f25e3),
C64(0x0fc19dc68b8cd5b5), C64(0x240ca1cc77ac9c65), C64(0x2de92c6f592b0275),
C64(0x4a7484aa6ea6e483), C64(0x5cb0a9dcbd41fbd4), C64(0x76f988da831153b5),
C64(0x983e5152ee66dfab), C64(0xa831c66d2db43210), C64(0xb00327c898fb213f),
C64(0xbf597fc7beef0ee4), C64(0xc6e00bf33da88fc2), C64(0xd5a79147930aa725),
C64(0x06ca6351e003826f), C64(0x142929670a0e6e70), C64(0x27b70a8546d22ffc),
C64(0x2e1b21385c26c926), C64(0x4d2c6dfc5ac42aed), C64(0x53380d139d95b3df),
C64(0x650a73548baf63de), C64(0x766a0abb3c77b2a8), C64(0x81c2c92e47edaee6),
C64(0x92722c851482353b), C64(0xa2bfe8a14cf10364), C64(0xa81a664bbc423001),
C64(0xc24b8b70d0f89791), C64(0xc76c51a30654be30), C64(0xd192e819d6ef5218),
C64(0xd69906245565a910), C64(0xf40e35855771202a), C64(0x106aa07032bbd1b8),
C64(0x19a4c116b8d2d0c8), C64(0x1e376c085141ab53), C64(0x2748774cdf8eeb99),
C64(0x34b0bcb5e19b48a8), C64(0x391c0cb3c5c95a63), C64(0x4ed8aa4ae3418acb),
C64(0x5b9cca4f7763e373), C64(0x682e6ff3d6b2b8a3), C64(0x748f82ee5defb2fc),
C64(0x78a5636f43172f60), C64(0x84c87814a1f0ab72), C64(0x8cc702081a6439ec),
C64(0x90befffa23631e28), C64(0xa4506cebde82bde9), C64(0xbef9a3f7b2c67915),
C64(0xc67178f2e372532b), C64(0xca273eceea26619c), C64(0xd186b8c721c0c207),
C64(0xeada7dd6cde0eb1e), C64(0xf57d4f7fee6ed178), C64(0x06f067aa72176fba),
C64(0x0a637dc5a2c898a6), C64(0x113f9804bef90dae), C64(0x1b710b35131c471b),
C64(0x28db77f523047d84), C64(0x32caab7b40c72493), C64(0x3c9ebe0a15c9bebc),
C64(0x431d67c49c100d4c), C64(0x4cc5d4becb3e42b6), C64(0x597f299cfc657e2a),
C64(0x5fcb6fab3ad6faec), C64(0x6c44198c4a475817)
};

static const W64 H0384[8] =	/* SHA-384 initial hash value */
{
C64(0xcbbb9d5dc1059ed8), C64(0x629a292a367cd507), C64(0x9159015a3070dd17),
C64(0x152fecd8f70e5939), C64(0x67332667ffc00b31), C64(0x8eb44a8768581511),
C64(0xdb0c2e0d64f98fa7), C64(0x47b5481dbefa4fa4)
};

static const W64 H0512[8] =	/* SHA-512 initial hash value */
{
C64(0x6a09e667f3bcc908), C64(0xbb67ae8584caa73b), C64(0x3c6ef372fe94f82b),
C64(0xa54ff53a5f1d36f1), C64(0x510e527fade682d1), C64(0x9b05688c2b3e6c1f),
C64(0x1f83d9abfb41bd6b), C64(0x5be0cd19137e2179)
};

static const W64 H0512224[8] =	/* SHA-512/224 initial hash value */
{
C64(0x8c3d37c819544da2), C64(0x73e1996689dcd4d6), C64(0x1dfab7ae32ff9c82),
C64(0x679dd514582f9fcf), C64(0x0f6d2b697bd44da8), C64(0x77e36f7304c48942),
C64(0x3f9d85a86a1d36c8), C64(0x1112e6ad91d692a1)
};

static const W64 H0512256[8] =	/* SHA-512/256 initial hash value */
{
C64(0x22312194fc2bf72c), C64(0x9f555fa3c84c64c2), C64(0x2393b86b6f53b151),
C64(0x963877195940eabd), C64(0x96283ee2a88effe3), C64(0xbe5e1e2553863992),
C64(0x2b0199fc2c85b8aa), C64(0x0eb72ddc81c52ca2)
};

static void sha512(SHA *s, unsigned char *block) /* SHA-384/512 transform */
{
	W64 a, b, c, d, e, f, g, h, T1, T2;
	W64 W[80];
	W64 *H = s->H64;
	int t;

	SHA64_SCHED(W, block);
	for (t = 16; t < 80; t++)
		W[t] = sigmaQ1(W[t-2]) + W[t-7] + sigmaQ0(W[t-15]) + W[t-16];
	a = H[0]; b = H[1]; c = H[2]; d = H[3];
	e = H[4]; f = H[5]; g = H[6]; h = H[7];
	for (t = 0; t < 80; t++) {
		T1 = h + SIGMAQ1(e) + Ch(e, f, g) + K512[t] + W[t];
		T2 = SIGMAQ0(a) + Ma(a, b, c);
		h = g; g = f; f = e; e = d + T1;
		d = c; c = b; b = a; a = T1 + T2;
	}
	H[0] += a; H[1] += b; H[2] += c; H[3] += d;
	H[4] += e; H[5] += f; H[6] += g; H[7] += h;
}

#endif	/* #ifdef SHA_384_512 */
