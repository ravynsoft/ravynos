/* verify_mbedtls.c
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
 * DNS SIG(0) signature verification for DNSSD SRP using mbedtls.
 *
 * Provides functions for generating a public key validating context based on SIG(0) KEY RR data, and
 * validating a signature using a context generated with that public key.  Currently only ECDSASHA256 is
 * supported.
 */

#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>

#include "srp.h"
#define SRP_CRYPTO_MBEDTLS_INTERNAL
#include "dns-msg.h"
#include "srp-crypto.h"


// Given a DNS message, a signature, and a public key, validate the message
bool
srp_sig0_verify(dns_wire_t *message, dns_rr_t *key, dns_rr_t *signature)
{
    mbedtls_ecp_point pubkey;
    mbedtls_ecp_group group;
    mbedtls_sha256_context sha;
    int status;
    char errbuf[128];
    uint8_t hash[ECDSA_SHA256_HASH_SIZE];
    mbedtls_mpi r, s;
    uint8_t *rdata;
    size_t rdlen;

    // The key algorithm and the signature algorithm have to match or we can't validate the signature.
    if (key->data.key.algorithm != signature->data.sig.algorithm) {
        return false;
    }

    // Key must be the right length (DNS ECDSA KEY isn't compressed).
    if (key->data.key.len != ECDSA_KEY_SIZE) {
        return false;
    }

    // Currently only support ecdsa
    if (signature->data.sig.algorithm != dnssec_keytype_ecdsa) {
        return false;
    }

    // Make sure the signature is the right size.
    if (signature->data.sig.len != ECDSA_SHA256_SIG_SIZE) {
        return false;
    }

    // Take the KEY RR and turn it into a public key we can use to check the signature.
    // Initialize the ECP group (SECP256).
    mbedtls_ecp_point_init(&pubkey);
    mbedtls_ecp_group_init(&group);
    mbedtls_ecp_group_load(&group, MBEDTLS_ECP_DP_SECP256R1);
    mbedtls_mpi_init(&r);
    mbedtls_mpi_init(&s);
    mbedtls_sha256_init(&sha);
    memset(hash, 0, sizeof hash);
    
    if ((status = mbedtls_mpi_read_binary(&pubkey.X, key->data.key.key, ECDSA_KEY_PART_SIZE)) != 0 ||
        (status = mbedtls_mpi_read_binary(&pubkey.Y, key->data.key.key + ECDSA_KEY_PART_SIZE, ECDSA_KEY_PART_SIZE)) != 0) {
        mbedtls_strerror(status, errbuf, sizeof errbuf);
        ERROR("mbedtls_mpi_read_binary: reading key: %s", errbuf);
    }
    mbedtls_mpi_lset(&pubkey.Z, 1);

    if ((status = mbedtls_mpi_read_binary(&r, signature->data.sig.signature, ECDSA_SHA256_SIG_PART_SIZE)) != 0 ||
        (status = mbedtls_mpi_read_binary(&s, signature->data.sig.signature + ECDSA_SHA256_SIG_PART_SIZE,
                                          ECDSA_SHA256_SIG_PART_SIZE)) != 0) {
        mbedtls_strerror(status, errbuf, sizeof errbuf);
        ERROR("mbedtls_mpi_read_binary: reading signature: %s", errbuf);
    }
    
    // The hash is across the message _before_ the SIG RR is added, so we have to decrement arcount before
 	// computing it.
    message->arcount = htons(ntohs(message->arcount) - 1);

    // And the SIG RRDATA that we hash includes the canonical version of the name, not whatever bits
    // are in the actual wire format message, so we have to just make a copy of it.
    rdlen = SIG_STATIC_RDLEN + dns_name_wire_length(signature->data.sig.signer);
    rdata = malloc(rdlen);
    if (rdata == NULL) {
        ERROR("no memory for SIG RR canonicalization");
        return 0;
    }
    memcpy(rdata, &message->data[signature->data.sig.start + SIG_HEADERLEN], SIG_STATIC_RDLEN);
    if (!dns_name_to_wire_canonical(rdata + SIG_STATIC_RDLEN, rdlen - SIG_STATIC_RDLEN,
                                    signature->data.sig.signer)) {
        // Should never happen.
        ERROR("dns_name_wire_length and dns_name_to_wire_canonical got different lengths!");
        return 0;
    }

    // First compute the hash across the SIG RR, then hash the message up to the SIG RR
    if ((status = mbedtls_sha256_starts_ret(&sha, 0)) != 0 ||
        (status = srp_mbedtls_sha256_update_ret(&sha, rdata, rdlen)) != 0 ||
        (status = srp_mbedtls_sha256_update_ret(&sha, (uint8_t *)message,
                                                signature->data.sig.start +
                                                (sizeof *message) - DNS_DATA_SIZE)) != 0 ||
        (status = srp_mbedtls_sha256_finish_ret(&sha, hash)) != 0) {
        // Put it back
        message->arcount = htons(ntohs(message->arcount) + 1);
        mbedtls_strerror(status, errbuf, sizeof errbuf);
        ERROR("mbedtls_sha_256 hash failed: %s", errbuf);
        return 0;
    }
    message->arcount = htons(ntohs(message->arcount) + 1);
    free(rdata);
    
    // Now check the signature against the hash
    status = mbedtls_ecdsa_verify(&group, hash, sizeof hash, &pubkey, &r, &s);
    if (status != 0) {
        mbedtls_strerror(status, errbuf, sizeof errbuf);
        ERROR("mbedtls_ecdsa_verify failed: %s", errbuf);
        return 0;
    }
    return 1;
}

// Function to copy out the public key as binary data
void
srp_print_key(srp_key_t *key)
{
    mbedtls_ecp_keypair *ecp = mbedtls_pk_ec(key->key);
    char errbuf[64];
    uint8_t buf[ECDSA_KEY_SIZE];
    uint8_t b64buf[((ECDSA_KEY_SIZE * 4) / 3) + 6];
    size_t b64len;
    int status;

    // Currently ECP only.
    if ((status = mbedtls_mpi_write_binary(&ecp->Q.X, buf, ECDSA_KEY_PART_SIZE)) != 0 ||
        (status = mbedtls_mpi_write_binary(&ecp->Q.Y, buf + ECDSA_KEY_PART_SIZE, ECDSA_KEY_PART_SIZE)) != 0) {
        mbedtls_strerror(status, errbuf, sizeof errbuf);
        ERROR("mbedtls_mpi_write_binary: %s", errbuf);
        return;
    }

    status = mbedtls_base64_encode(b64buf, sizeof b64buf, &b64len, buf, ECDSA_KEY_SIZE);
    if (status != 0) {
        mbedtls_strerror(status, errbuf, sizeof errbuf);
        ERROR("mbedtls_mpi_write_binary: %s", errbuf);
        return;
    }
    fputs("thread-demo.default.service.arpa. IN KEY 513 3 13 ", stdout);
    fwrite(b64buf, b64len, 1, stdout);
    putc('\n', stdout);
}

// Local Variables:
// mode: C
// tab-width: 4
// c-file-style: "bsd"
// c-basic-offset: 4
// fill-column: 108
// indent-tabs-mode: nil
// End:
