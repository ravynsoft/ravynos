//
//  sha384_impl.cpp
//  corecrypto_kernel
//
//  Created by William Kent on 5/28/20.
//  Copyright © 2020 William Kent. All rights reserved.
//
//  adapted from botan/src/lib/hash/sha2_64/sha2_64.cpp
//  Botan  http://botan.randombit.net
//  License :  https://github.com/randombit/botan/blob/master/doc/license.txt
//

#include <sys/types.h>
__BEGIN_DECLS
#include <corecrypto/ccdigest.h>
#include <corecrypto/ccsha2.h>
#include "pdcrypto_digest_final.h"
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
 * SHA-512 F1 Function
 *
 * Use a macro as many compilers won't inline a function this big,
 * even though it is much faster if inlined.
 */
#define SHA2_64_F(A, B, C, D, E, F, G, H, M1, M2, M3, M4, magic)         \
   do {                                                                  \
      const uint64_t E_rho = rotr<14>(E) ^ rotr<18>(E) ^ rotr<41>(E);    \
      const uint64_t A_rho = rotr<28>(A) ^ rotr<34>(A) ^ rotr<39>(A);    \
      const uint64_t M2_sigma = rotr<19>(M2) ^ rotr<61>(M2) ^ (M2 >> 6); \
      const uint64_t M4_sigma = rotr<1>(M4) ^ rotr<8>(M4) ^ (M4 >> 7);   \
      H += magic + E_rho + ((E & F) ^ (~E & G)) + M1;                    \
      D += H;                                                            \
      H += A_rho + ((A & B) | ((A | B) & C));                            \
      M1 += M2_sigma + M3 + M4_sigma;                                    \
   } while(0);

template<typename T>
static inline T load_be(const uint8_t in[], size_t off) {
	in += off * sizeof(T);
	T out = 0;
	for(size_t i = 0; i != sizeof(T); ++i)
		out = (out << 8) | in[i];
	return out;
}

static void pdcsha512_compress(ccdigest_state_t state_t, unsigned long nblocks, const void *data) {
	const uint8_t *input = (const uint8_t *)data;
	uint64_t *state = ccdigest_u64(state_t);

	uint64_t A = state[0], B = state[1], C = state[2], D = state[3];
	uint64_t E = state[4], F = state[5], G = state[6], H = state[7];

	for(size_t i = 0; i != nblocks; ++i)
	{
	   uint64_t W00 = load_be<uint64_t>(input,  0);
	   uint64_t W01 = load_be<uint64_t>(input,  1);
	   uint64_t W02 = load_be<uint64_t>(input,  2);
	   uint64_t W03 = load_be<uint64_t>(input,  3);
	   uint64_t W04 = load_be<uint64_t>(input,  4);
	   uint64_t W05 = load_be<uint64_t>(input,  5);
	   uint64_t W06 = load_be<uint64_t>(input,  6);
	   uint64_t W07 = load_be<uint64_t>(input,  7);
	   uint64_t W08 = load_be<uint64_t>(input,  8);
	   uint64_t W09 = load_be<uint64_t>(input,  9);
	   uint64_t W10 = load_be<uint64_t>(input, 10);
	   uint64_t W11 = load_be<uint64_t>(input, 11);
	   uint64_t W12 = load_be<uint64_t>(input, 12);
	   uint64_t W13 = load_be<uint64_t>(input, 13);
	   uint64_t W14 = load_be<uint64_t>(input, 14);
	   uint64_t W15 = load_be<uint64_t>(input, 15);

	   SHA2_64_F(A, B, C, D, E, F, G, H, W00, W14, W09, W01, 0x428A2F98D728AE22);
	   SHA2_64_F(H, A, B, C, D, E, F, G, W01, W15, W10, W02, 0x7137449123EF65CD);
	   SHA2_64_F(G, H, A, B, C, D, E, F, W02, W00, W11, W03, 0xB5C0FBCFEC4D3B2F);
	   SHA2_64_F(F, G, H, A, B, C, D, E, W03, W01, W12, W04, 0xE9B5DBA58189DBBC);
	   SHA2_64_F(E, F, G, H, A, B, C, D, W04, W02, W13, W05, 0x3956C25BF348B538);
	   SHA2_64_F(D, E, F, G, H, A, B, C, W05, W03, W14, W06, 0x59F111F1B605D019);
	   SHA2_64_F(C, D, E, F, G, H, A, B, W06, W04, W15, W07, 0x923F82A4AF194F9B);
	   SHA2_64_F(B, C, D, E, F, G, H, A, W07, W05, W00, W08, 0xAB1C5ED5DA6D8118);
	   SHA2_64_F(A, B, C, D, E, F, G, H, W08, W06, W01, W09, 0xD807AA98A3030242);
	   SHA2_64_F(H, A, B, C, D, E, F, G, W09, W07, W02, W10, 0x12835B0145706FBE);
	   SHA2_64_F(G, H, A, B, C, D, E, F, W10, W08, W03, W11, 0x243185BE4EE4B28C);
	   SHA2_64_F(F, G, H, A, B, C, D, E, W11, W09, W04, W12, 0x550C7DC3D5FFB4E2);
	   SHA2_64_F(E, F, G, H, A, B, C, D, W12, W10, W05, W13, 0x72BE5D74F27B896F);
	   SHA2_64_F(D, E, F, G, H, A, B, C, W13, W11, W06, W14, 0x80DEB1FE3B1696B1);
	   SHA2_64_F(C, D, E, F, G, H, A, B, W14, W12, W07, W15, 0x9BDC06A725C71235);
	   SHA2_64_F(B, C, D, E, F, G, H, A, W15, W13, W08, W00, 0xC19BF174CF692694);
	   SHA2_64_F(A, B, C, D, E, F, G, H, W00, W14, W09, W01, 0xE49B69C19EF14AD2);
	   SHA2_64_F(H, A, B, C, D, E, F, G, W01, W15, W10, W02, 0xEFBE4786384F25E3);
	   SHA2_64_F(G, H, A, B, C, D, E, F, W02, W00, W11, W03, 0x0FC19DC68B8CD5B5);
	   SHA2_64_F(F, G, H, A, B, C, D, E, W03, W01, W12, W04, 0x240CA1CC77AC9C65);
	   SHA2_64_F(E, F, G, H, A, B, C, D, W04, W02, W13, W05, 0x2DE92C6F592B0275);
	   SHA2_64_F(D, E, F, G, H, A, B, C, W05, W03, W14, W06, 0x4A7484AA6EA6E483);
	   SHA2_64_F(C, D, E, F, G, H, A, B, W06, W04, W15, W07, 0x5CB0A9DCBD41FBD4);
	   SHA2_64_F(B, C, D, E, F, G, H, A, W07, W05, W00, W08, 0x76F988DA831153B5);
	   SHA2_64_F(A, B, C, D, E, F, G, H, W08, W06, W01, W09, 0x983E5152EE66DFAB);
	   SHA2_64_F(H, A, B, C, D, E, F, G, W09, W07, W02, W10, 0xA831C66D2DB43210);
	   SHA2_64_F(G, H, A, B, C, D, E, F, W10, W08, W03, W11, 0xB00327C898FB213F);
	   SHA2_64_F(F, G, H, A, B, C, D, E, W11, W09, W04, W12, 0xBF597FC7BEEF0EE4);
	   SHA2_64_F(E, F, G, H, A, B, C, D, W12, W10, W05, W13, 0xC6E00BF33DA88FC2);
	   SHA2_64_F(D, E, F, G, H, A, B, C, W13, W11, W06, W14, 0xD5A79147930AA725);
	   SHA2_64_F(C, D, E, F, G, H, A, B, W14, W12, W07, W15, 0x06CA6351E003826F);
	   SHA2_64_F(B, C, D, E, F, G, H, A, W15, W13, W08, W00, 0x142929670A0E6E70);
	   SHA2_64_F(A, B, C, D, E, F, G, H, W00, W14, W09, W01, 0x27B70A8546D22FFC);
	   SHA2_64_F(H, A, B, C, D, E, F, G, W01, W15, W10, W02, 0x2E1B21385C26C926);
	   SHA2_64_F(G, H, A, B, C, D, E, F, W02, W00, W11, W03, 0x4D2C6DFC5AC42AED);
	   SHA2_64_F(F, G, H, A, B, C, D, E, W03, W01, W12, W04, 0x53380D139D95B3DF);
	   SHA2_64_F(E, F, G, H, A, B, C, D, W04, W02, W13, W05, 0x650A73548BAF63DE);
	   SHA2_64_F(D, E, F, G, H, A, B, C, W05, W03, W14, W06, 0x766A0ABB3C77B2A8);
	   SHA2_64_F(C, D, E, F, G, H, A, B, W06, W04, W15, W07, 0x81C2C92E47EDAEE6);
	   SHA2_64_F(B, C, D, E, F, G, H, A, W07, W05, W00, W08, 0x92722C851482353B);
	   SHA2_64_F(A, B, C, D, E, F, G, H, W08, W06, W01, W09, 0xA2BFE8A14CF10364);
	   SHA2_64_F(H, A, B, C, D, E, F, G, W09, W07, W02, W10, 0xA81A664BBC423001);
	   SHA2_64_F(G, H, A, B, C, D, E, F, W10, W08, W03, W11, 0xC24B8B70D0F89791);
	   SHA2_64_F(F, G, H, A, B, C, D, E, W11, W09, W04, W12, 0xC76C51A30654BE30);
	   SHA2_64_F(E, F, G, H, A, B, C, D, W12, W10, W05, W13, 0xD192E819D6EF5218);
	   SHA2_64_F(D, E, F, G, H, A, B, C, W13, W11, W06, W14, 0xD69906245565A910);
	   SHA2_64_F(C, D, E, F, G, H, A, B, W14, W12, W07, W15, 0xF40E35855771202A);
	   SHA2_64_F(B, C, D, E, F, G, H, A, W15, W13, W08, W00, 0x106AA07032BBD1B8);
	   SHA2_64_F(A, B, C, D, E, F, G, H, W00, W14, W09, W01, 0x19A4C116B8D2D0C8);
	   SHA2_64_F(H, A, B, C, D, E, F, G, W01, W15, W10, W02, 0x1E376C085141AB53);
	   SHA2_64_F(G, H, A, B, C, D, E, F, W02, W00, W11, W03, 0x2748774CDF8EEB99);
	   SHA2_64_F(F, G, H, A, B, C, D, E, W03, W01, W12, W04, 0x34B0BCB5E19B48A8);
	   SHA2_64_F(E, F, G, H, A, B, C, D, W04, W02, W13, W05, 0x391C0CB3C5C95A63);
	   SHA2_64_F(D, E, F, G, H, A, B, C, W05, W03, W14, W06, 0x4ED8AA4AE3418ACB);
	   SHA2_64_F(C, D, E, F, G, H, A, B, W06, W04, W15, W07, 0x5B9CCA4F7763E373);
	   SHA2_64_F(B, C, D, E, F, G, H, A, W07, W05, W00, W08, 0x682E6FF3D6B2B8A3);
	   SHA2_64_F(A, B, C, D, E, F, G, H, W08, W06, W01, W09, 0x748F82EE5DEFB2FC);
	   SHA2_64_F(H, A, B, C, D, E, F, G, W09, W07, W02, W10, 0x78A5636F43172F60);
	   SHA2_64_F(G, H, A, B, C, D, E, F, W10, W08, W03, W11, 0x84C87814A1F0AB72);
	   SHA2_64_F(F, G, H, A, B, C, D, E, W11, W09, W04, W12, 0x8CC702081A6439EC);
	   SHA2_64_F(E, F, G, H, A, B, C, D, W12, W10, W05, W13, 0x90BEFFFA23631E28);
	   SHA2_64_F(D, E, F, G, H, A, B, C, W13, W11, W06, W14, 0xA4506CEBDE82BDE9);
	   SHA2_64_F(C, D, E, F, G, H, A, B, W14, W12, W07, W15, 0xBEF9A3F7B2C67915);
	   SHA2_64_F(B, C, D, E, F, G, H, A, W15, W13, W08, W00, 0xC67178F2E372532B);
	   SHA2_64_F(A, B, C, D, E, F, G, H, W00, W14, W09, W01, 0xCA273ECEEA26619C);
	   SHA2_64_F(H, A, B, C, D, E, F, G, W01, W15, W10, W02, 0xD186B8C721C0C207);
	   SHA2_64_F(G, H, A, B, C, D, E, F, W02, W00, W11, W03, 0xEADA7DD6CDE0EB1E);
	   SHA2_64_F(F, G, H, A, B, C, D, E, W03, W01, W12, W04, 0xF57D4F7FEE6ED178);
	   SHA2_64_F(E, F, G, H, A, B, C, D, W04, W02, W13, W05, 0x06F067AA72176FBA);
	   SHA2_64_F(D, E, F, G, H, A, B, C, W05, W03, W14, W06, 0x0A637DC5A2C898A6);
	   SHA2_64_F(C, D, E, F, G, H, A, B, W06, W04, W15, W07, 0x113F9804BEF90DAE);
	   SHA2_64_F(B, C, D, E, F, G, H, A, W07, W05, W00, W08, 0x1B710B35131C471B);
	   SHA2_64_F(A, B, C, D, E, F, G, H, W08, W06, W01, W09, 0x28DB77F523047D84);
	   SHA2_64_F(H, A, B, C, D, E, F, G, W09, W07, W02, W10, 0x32CAAB7B40C72493);
	   SHA2_64_F(G, H, A, B, C, D, E, F, W10, W08, W03, W11, 0x3C9EBE0A15C9BEBC);
	   SHA2_64_F(F, G, H, A, B, C, D, E, W11, W09, W04, W12, 0x431D67C49C100D4C);
	   SHA2_64_F(E, F, G, H, A, B, C, D, W12, W10, W05, W13, 0x4CC5D4BECB3E42B6);
	   SHA2_64_F(D, E, F, G, H, A, B, C, W13, W11, W06, W14, 0x597F299CFC657E2A);
	   SHA2_64_F(C, D, E, F, G, H, A, B, W14, W12, W07, W15, 0x5FCB6FAB3AD6FAEC);
	   SHA2_64_F(B, C, D, E, F, G, H, A, W15, W13, W08, W00, 0x6C44198C4A475817);

	   A = (state[0] += A);
	   B = (state[1] += B);
	   C = (state[2] += C);
	   D = (state[3] += D);
	   E = (state[4] += E);
	   F = (state[5] += F);
	   G = (state[6] += G);
	   H = (state[7] += H);

	   input += 128;
	}
}

const uint64_t pdcsha384_initial_state[8] = {
	0xCBBB9D5DC1059ED8,
	0x629A292A367CD507,
	0x9159015A3070DD17,
	0x152FECD8F70E5939,
	0x67332667FFC00B31,
	0x8EB44A8768581511,
	0xDB0C2E0D64F98FA7,
	0x47B5481DBEFA4FA4
};

const uint64_t pdcsha512_initial_state[8] = {
	 0x6A09E667F3BCC908,
	 0xBB67AE8584CAA73B,
	 0x3C6EF372FE94F82B,
	 0xA54FF53A5F1D36F1,
	 0x510E527FADE682D1,
	 0x9B05688C2B3E6C1F,
	 0x1F83D9ABFB41BD6B,
	 0x5BE0CD19137E2179
};

const struct ccdigest_info ccsha384_ltc_di = {
	.output_size = CCSHA384_OUTPUT_SIZE,
	.state_size = CCSHA512_STATE_SIZE,
	.block_size = CCSHA512_BLOCK_SIZE,
	.oid_size = ccoid_sha384_len,
	.oid = (unsigned char *)CC_DIGEST_OID_SHA384,
	.initial_state = pdcsha384_initial_state,
	.compress = pdcsha512_compress,
	.final = pdcdigest_final_64be
};

const struct ccdigest_info ccsha512_ltc_di = {
	.output_size = CCSHA512_OUTPUT_SIZE,
	.state_size = CCSHA512_STATE_SIZE,
	.block_size = CCSHA512_BLOCK_SIZE,
	.oid_size = ccoid_sha512_len,
	.oid = (unsigned char *)CC_DIGEST_OID_SHA512,
	.initial_state = pdcsha512_initial_state,
	.compress = pdcsha512_compress,
	.final = pdcdigest_final_64be
};

const struct ccdigest_info *ccsha384_di(void) {
	return &ccsha384_ltc_di;
}

const struct ccdigest_info *ccsha512_di(void) {
	return &ccsha512_ltc_di;
}
