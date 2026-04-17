//
//  sha256.c
//  corecrypto_kernel
//
//  Created by William Kent on 10/15/19.
//  Copyright © 2019 William Kent. All rights reserved.
//
//  adapted from botan/src/lib/hash/sha2_32/sha2_32.cpp
//  Botan  http://botan.randombit.net
//  License :  https://github.com/randombit/botan/blob/master/doc/license.txt

#include <sys/types.h>
__BEGIN_DECLS
#include <corecrypto/ccdigest.h>
#include <corecrypto/ccsha2.h>
#include "pdcrypto_digest_final.h"
#if KERNEL
#include <libkern/crypto/md5.h>
#include <sys/systm.h>
#else
#include <stdlib.h>
#endif
__END_DECLS

template<size_t ROT, typename T>
inline constexpr T rotl(T input)
{
	static_assert(ROT > 0 && ROT < 8*sizeof(T), "Invalid rotation constant");
	return static_cast<T>((input << ROT) | (input >> (8*sizeof(T) - ROT)));
}

template<size_t ROT, typename T>
inline constexpr T rotr(T input)
{
	static_assert(ROT > 0 && ROT < 8*sizeof(T), "Invalid rotation constant");
	return static_cast<T>((input >> ROT) | (input << (8*sizeof(T) - ROT)));
}

/*
 * SHA-256 F1 Function
 *
 * Use a macro as many compilers won't inline a function this big,
 * even though it is much faster if inlined.
 */
#define SHA2_32_F(A, B, C, D, E, F, G, H, M1, M2, M3, M4, magic) do { \
	uint32_t A_rho = rotr<2>(A) ^ rotr<13>(A) ^ rotr<22>(A); \
	uint32_t E_rho = rotr<6>(E) ^ rotr<11>(E) ^ rotr<25>(E); \
	uint32_t M2_sigma = rotr<17>(M2) ^ rotr<19>(M2) ^ (M2 >> 10); \
	uint32_t M4_sigma = rotr<7>(M4) ^ rotr<18>(M4) ^ (M4 >> 3); \
	H += magic + E_rho + ((E & F) ^ (~E & G)) + M1; \
	D += H; \
	H += A_rho + ((A & B) | ((A | B) & C)); \
	M1 += M2_sigma + M3 + M4_sigma; \
	} while(0);

static inline uint32_t load_be(const uint8_t in[], size_t off) {
	in += off * sizeof(uint32_t);
	uint32_t out = 0;
	for(size_t i = 0; i != sizeof(uint32_t); ++i)
		out = (out << 8) | in[i];
	return out;
}


static void pdcsha256_compress(ccdigest_state_t state_t, unsigned long nblocks, const void *data) {
	const uint8_t *input = (const uint8_t *)data;
	uint32_t *state = ccdigest_u32(state_t);

	uint32_t A = state[0], B = state[1], C = state[2], D = state[3];
	uint32_t E = state[4], F = state[5], G = state[6], H = state[7];

	for (size_t i = 0; i != nblocks; ++i)
	{
		uint32_t W00 = load_be(input,  0);
		uint32_t W01 = load_be(input,  1);
		uint32_t W02 = load_be(input,  2);
		uint32_t W03 = load_be(input,  3);
		uint32_t W04 = load_be(input,  4);
		uint32_t W05 = load_be(input,  5);
		uint32_t W06 = load_be(input,  6);
		uint32_t W07 = load_be(input,  7);
		uint32_t W08 = load_be(input,  8);
		uint32_t W09 = load_be(input,  9);
		uint32_t W10 = load_be(input, 10);
		uint32_t W11 = load_be(input, 11);
		uint32_t W12 = load_be(input, 12);
		uint32_t W13 = load_be(input, 13);
		uint32_t W14 = load_be(input, 14);
		uint32_t W15 = load_be(input, 15);

		SHA2_32_F(A, B, C, D, E, F, G, H, W00, W14, W09, W01, 0x428A2F98);
		SHA2_32_F(H, A, B, C, D, E, F, G, W01, W15, W10, W02, 0x71374491);
		SHA2_32_F(G, H, A, B, C, D, E, F, W02, W00, W11, W03, 0xB5C0FBCF);
		SHA2_32_F(F, G, H, A, B, C, D, E, W03, W01, W12, W04, 0xE9B5DBA5);
		SHA2_32_F(E, F, G, H, A, B, C, D, W04, W02, W13, W05, 0x3956C25B);
		SHA2_32_F(D, E, F, G, H, A, B, C, W05, W03, W14, W06, 0x59F111F1);
		SHA2_32_F(C, D, E, F, G, H, A, B, W06, W04, W15, W07, 0x923F82A4);
		SHA2_32_F(B, C, D, E, F, G, H, A, W07, W05, W00, W08, 0xAB1C5ED5);
		SHA2_32_F(A, B, C, D, E, F, G, H, W08, W06, W01, W09, 0xD807AA98);
		SHA2_32_F(H, A, B, C, D, E, F, G, W09, W07, W02, W10, 0x12835B01);
		SHA2_32_F(G, H, A, B, C, D, E, F, W10, W08, W03, W11, 0x243185BE);
		SHA2_32_F(F, G, H, A, B, C, D, E, W11, W09, W04, W12, 0x550C7DC3);
		SHA2_32_F(E, F, G, H, A, B, C, D, W12, W10, W05, W13, 0x72BE5D74);
		SHA2_32_F(D, E, F, G, H, A, B, C, W13, W11, W06, W14, 0x80DEB1FE);
		SHA2_32_F(C, D, E, F, G, H, A, B, W14, W12, W07, W15, 0x9BDC06A7);
		SHA2_32_F(B, C, D, E, F, G, H, A, W15, W13, W08, W00, 0xC19BF174);

		SHA2_32_F(A, B, C, D, E, F, G, H, W00, W14, W09, W01, 0xE49B69C1);
		SHA2_32_F(H, A, B, C, D, E, F, G, W01, W15, W10, W02, 0xEFBE4786);
		SHA2_32_F(G, H, A, B, C, D, E, F, W02, W00, W11, W03, 0x0FC19DC6);
		SHA2_32_F(F, G, H, A, B, C, D, E, W03, W01, W12, W04, 0x240CA1CC);
		SHA2_32_F(E, F, G, H, A, B, C, D, W04, W02, W13, W05, 0x2DE92C6F);
		SHA2_32_F(D, E, F, G, H, A, B, C, W05, W03, W14, W06, 0x4A7484AA);
		SHA2_32_F(C, D, E, F, G, H, A, B, W06, W04, W15, W07, 0x5CB0A9DC);
		SHA2_32_F(B, C, D, E, F, G, H, A, W07, W05, W00, W08, 0x76F988DA);
		SHA2_32_F(A, B, C, D, E, F, G, H, W08, W06, W01, W09, 0x983E5152);
		SHA2_32_F(H, A, B, C, D, E, F, G, W09, W07, W02, W10, 0xA831C66D);
		SHA2_32_F(G, H, A, B, C, D, E, F, W10, W08, W03, W11, 0xB00327C8);
		SHA2_32_F(F, G, H, A, B, C, D, E, W11, W09, W04, W12, 0xBF597FC7);
		SHA2_32_F(E, F, G, H, A, B, C, D, W12, W10, W05, W13, 0xC6E00BF3);
		SHA2_32_F(D, E, F, G, H, A, B, C, W13, W11, W06, W14, 0xD5A79147);
		SHA2_32_F(C, D, E, F, G, H, A, B, W14, W12, W07, W15, 0x06CA6351);
		SHA2_32_F(B, C, D, E, F, G, H, A, W15, W13, W08, W00, 0x14292967);

		SHA2_32_F(A, B, C, D, E, F, G, H, W00, W14, W09, W01, 0x27B70A85);
		SHA2_32_F(H, A, B, C, D, E, F, G, W01, W15, W10, W02, 0x2E1B2138);
		SHA2_32_F(G, H, A, B, C, D, E, F, W02, W00, W11, W03, 0x4D2C6DFC);
		SHA2_32_F(F, G, H, A, B, C, D, E, W03, W01, W12, W04, 0x53380D13);
		SHA2_32_F(E, F, G, H, A, B, C, D, W04, W02, W13, W05, 0x650A7354);
		SHA2_32_F(D, E, F, G, H, A, B, C, W05, W03, W14, W06, 0x766A0ABB);
		SHA2_32_F(C, D, E, F, G, H, A, B, W06, W04, W15, W07, 0x81C2C92E);
		SHA2_32_F(B, C, D, E, F, G, H, A, W07, W05, W00, W08, 0x92722C85);
		SHA2_32_F(A, B, C, D, E, F, G, H, W08, W06, W01, W09, 0xA2BFE8A1);
		SHA2_32_F(H, A, B, C, D, E, F, G, W09, W07, W02, W10, 0xA81A664B);
		SHA2_32_F(G, H, A, B, C, D, E, F, W10, W08, W03, W11, 0xC24B8B70);
		SHA2_32_F(F, G, H, A, B, C, D, E, W11, W09, W04, W12, 0xC76C51A3);
		SHA2_32_F(E, F, G, H, A, B, C, D, W12, W10, W05, W13, 0xD192E819);
		SHA2_32_F(D, E, F, G, H, A, B, C, W13, W11, W06, W14, 0xD6990624);
		SHA2_32_F(C, D, E, F, G, H, A, B, W14, W12, W07, W15, 0xF40E3585);
		SHA2_32_F(B, C, D, E, F, G, H, A, W15, W13, W08, W00, 0x106AA070);

		SHA2_32_F(A, B, C, D, E, F, G, H, W00, W14, W09, W01, 0x19A4C116);
		SHA2_32_F(H, A, B, C, D, E, F, G, W01, W15, W10, W02, 0x1E376C08);
		SHA2_32_F(G, H, A, B, C, D, E, F, W02, W00, W11, W03, 0x2748774C);
		SHA2_32_F(F, G, H, A, B, C, D, E, W03, W01, W12, W04, 0x34B0BCB5);
		SHA2_32_F(E, F, G, H, A, B, C, D, W04, W02, W13, W05, 0x391C0CB3);
		SHA2_32_F(D, E, F, G, H, A, B, C, W05, W03, W14, W06, 0x4ED8AA4A);
		SHA2_32_F(C, D, E, F, G, H, A, B, W06, W04, W15, W07, 0x5B9CCA4F);
		SHA2_32_F(B, C, D, E, F, G, H, A, W07, W05, W00, W08, 0x682E6FF3);
		SHA2_32_F(A, B, C, D, E, F, G, H, W08, W06, W01, W09, 0x748F82EE);
		SHA2_32_F(H, A, B, C, D, E, F, G, W09, W07, W02, W10, 0x78A5636F);
		SHA2_32_F(G, H, A, B, C, D, E, F, W10, W08, W03, W11, 0x84C87814);
		SHA2_32_F(F, G, H, A, B, C, D, E, W11, W09, W04, W12, 0x8CC70208);
		SHA2_32_F(E, F, G, H, A, B, C, D, W12, W10, W05, W13, 0x90BEFFFA);
		SHA2_32_F(D, E, F, G, H, A, B, C, W13, W11, W06, W14, 0xA4506CEB);
		SHA2_32_F(C, D, E, F, G, H, A, B, W14, W12, W07, W15, 0xBEF9A3F7);
		SHA2_32_F(B, C, D, E, F, G, H, A, W15, W13, W08, W00, 0xC67178F2);

		A = (state[0] += A);
		B = (state[1] += B);
		C = (state[2] += C);
		D = (state[3] += D);
		E = (state[4] += E);
		F = (state[5] += F);
		G = (state[6] += G);
		H = (state[7] += H);

		input += 64;
	}
}

const uint32_t pdcsha256_initial_state[8] = {
	0x6A09E667UL, // A
	0xBB67AE85UL, // B
	0x3C6EF372UL, // C
	0xA54FF53AUL, // D
	0x510E527FUL, // E
	0x9B05688CUL, // F
	0x1F83D9ABUL, // G
	0x5BE0CD19UL  // H
};

const struct ccdigest_info ccsha256_ltc_di = {
	.output_size = CCSHA256_OUTPUT_SIZE,
	.state_size = CCSHA256_STATE_SIZE,
	.block_size = CCSHA256_BLOCK_SIZE,
	.oid_size = ccoid_sha256_len,
	.oid = (unsigned char *)CC_DIGEST_OID_SHA256,
	.initial_state = pdcsha256_initial_state,
	.compress = pdcsha256_compress,
	.final = pdcdigest_final_64be
};

const struct ccdigest_info *ccsha256_di(void) {
	return &ccsha256_ltc_di;
}
