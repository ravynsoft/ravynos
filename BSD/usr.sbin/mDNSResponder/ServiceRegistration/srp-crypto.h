/* srp-key.h
 *
 * Copyright (c) 2018 Apple Computer, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * DNS SIG(0) signature generation for DNSSD SRP using mbedtls.
 *
 * Functions required for loading, saving, and generating public/private keypairs, extracting the public key
 * into KEY RR data, and computing signatures.
 */

#ifndef __SRP_CRYPTO_H
#define __SRP_CRYPTO_H
// Anonymous key structure, depends on the target.
typedef struct srp_key srp_key_t;

#ifdef SRP_CRYPTO_MBEDTLS_INTERNAL
#include <mbedtls/error.h>
#include <mbedtls/pk.h>
#include <mbedtls/ecp.h>
#include <mbedtls/ecdsa.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/sha256.h>
#include <mbedtls/base64.h>

// The SRP key includes both the ecdsa key and the pseudo-random number generator context, so that we can
// use the PRNG for signing as well as generating keys.   The PRNG is seeded with a high-entropy data source.
// This structure assumes that we are just using this one key; if we want to support multiple keys then
// the entropy source and PRNG should be shared by all keys (of course, that's not thread-safe, so...)
struct srp_key {
    mbedtls_pk_context key;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr;
};    

#ifdef DEBUG_SHA256
int srp_mbedtls_sha256_update_ret(mbedtls_sha256_context *NONNULL sha, uint8_t *NONNULL message, size_t msglen);
int srp_mbedtls_sha256_finish_ret(mbedtls_sha256_context *NONNULL sha, uint8_t *NONNULL hash);
#else
#define srp_mbedtls_sha256_update_ret mbedtls_sha256_update_ret
#define srp_mbedtls_sha256_finish_ret mbedtls_sha256_finish_ret
#endif // DEBUG_SHA256
#endif // SRP_CRYPTO_MBEDTLS_INTERNAL

#define ECDSA_KEY_SIZE 64
#define ECDSA_KEY_PART_SIZE 32
#define ECDSA_SHA256_HASH_SIZE 32
#define ECDSA_SHA256_SIG_SIZE 64
#define ECDSA_SHA256_SIG_PART_SIZE 32

#define SIG_HEADERLEN 11
#define SIG_STATIC_RDLEN 18


#define dnssec_keytype_ecdsa  13

// sign_*.c:
void srp_keypair_free(srp_key_t *NONNULL key);
srp_key_t *NULLABLE srp_load_keypair(const char *NONNULL file);
srp_key_t *NULLABLE srp_generate_key(void);
int srp_write_key_to_file(const char *NONNULL file, srp_key_t *NONNULL key);
int srp_key_algorithm(srp_key_t *NONNULL key);
size_t srp_pubkey_length(srp_key_t *NONNULL key);
size_t srp_signature_length(srp_key_t *NONNULL key);
int srp_pubkey_copy(uint8_t *NONNULL buf, size_t max, srp_key_t *NONNULL key);
int srp_sign(uint8_t *NONNULL output, size_t max,
	     uint8_t *NONNULL message, size_t msglen, uint8_t *NONNULL rdata, size_t rdlen, srp_key_t *NONNULL key);

// verify_*.c:
bool srp_sig0_verify(dns_wire_t *NONNULL message, dns_rr_t *NONNULL key, dns_rr_t *NONNULL signature);
void srp_print_key(srp_key_t *NONNULL key);

#endif // __SRP_CRYPTO_H

// Local Variables:
// mode: C
// tab-width: 4
// c-file-style: "bsd"
// c-basic-offset: 4
// fill-column: 108
// indent-tabs-mode: nil
// End:
