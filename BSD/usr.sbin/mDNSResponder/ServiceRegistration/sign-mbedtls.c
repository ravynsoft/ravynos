/* sign.c
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

#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/random.h>
#include <sys/errno.h>

#include "srp.h"
#include "dns-msg.h"
#define SRP_CRYPTO_MBEDTLS_INTERNAL
#include "srp-crypto.h"

// For debugging
#ifdef DEBUG_SHA256
int
srp_mbedtls_sha256_update_ret(mbedtls_sha256_context *sha, uint8_t *data, size_t len)
{
    int i;
    fprintf(stderr, "data %lu: ", (unsigned long)len);
    for (i = 0; i < len; i++) {
        fprintf(stderr, "%02x", data[i]);
    }
    fputs("\n", stderr);
    return mbedtls_sha256_update_ret(sha, data, len);
}

int
srp_mbedtls_sha256_finish_ret(mbedtls_sha256_context *sha, uint8_t *hash)
{
    int i;
    int status = mbedtls_sha256_finish_ret(sha, hash);
    fprintf(stderr, "hash:     ");
    for (i = 0; i < ECDSA_SHA256_HASH_SIZE; i++) {
        fprintf(stderr, "%02x", hash[i]);
    }
    fputs("\n", stderr);
    return status;
}
#endif

// Key is stored in an opaque data structure, for mbedtls this is an mbedtls_pk_context.
// Function to read a public key from a KEY record
// Function to validate a signature given some data and a public key (not required on client)

// Function to free a key
void
srp_keypair_free(srp_key_t *key)
{
    mbedtls_pk_free(&key->key);
    mbedtls_entropy_free(&key->entropy);
    mbedtls_ctr_drbg_free(&key->ctr);
    free(key);
}

// Needed to see the RNG with good entropy data.
static int
get_entropy(void *data, unsigned char *output, size_t len, size_t *outlen)
{
    int result = getentropy(output, len);
    (void)data;

    if (result != 0) {
        ERROR("getentropy returned %s", strerror(errno));
        return MBEDTLS_ERR_ENTROPY_SOURCE_FAILED;
    }
    *outlen = len;
    return 0;
}

static srp_key_t *
srp_key_setup(void)
{
    int status;
    srp_key_t *key = calloc(sizeof *key, 1);
    char errbuf[64];

    if (key == NULL) {
        return key;
    }
    
    mbedtls_pk_init(&key->key);
    mbedtls_entropy_init(&key->entropy);
    if ((status = mbedtls_entropy_add_source(&key->entropy, get_entropy,
                                             NULL, 1, MBEDTLS_ENTROPY_SOURCE_STRONG)) != 0) {
        mbedtls_strerror(status, errbuf, sizeof errbuf);
        ERROR("mbedtls_entropy_add_source failed: %s", errbuf);
    } else if ((status = mbedtls_ctr_drbg_seed(&key->ctr, mbedtls_entropy_func, &key->entropy, NULL, 0)) != 0) {
        mbedtls_strerror(status, errbuf, sizeof errbuf);
        ERROR("mbedtls_ctr_drbg_seed failed: %s", errbuf);
    } else {
        return key;
    }
    mbedtls_pk_free(&key->key);
    mbedtls_entropy_free(&key->entropy);
    free(key);
    return NULL;
}

// Function to read a keypair from a file
srp_key_t *
srp_load_keypair(const char *file)
{
    int fd = open(file, O_RDONLY);
    unsigned char buf[256];
    ssize_t rv;
    srp_key_t *key;
    int status;
    char errbuf[64];

    if (fd < 0) {
        if (errno != ENOENT) {
            ERROR("Unable to open srp.key: %s", strerror(errno));
            return NULL;
        }
        return NULL;
    }        

    // The key is of limited size, so there's no reason to get fancy.
    rv = read(fd, buf, sizeof buf);
    close(fd);
    if (rv == sizeof buf) {
        ERROR("key file is unreasonably large.");
        return NULL;
    }

    key = srp_key_setup();
    if (key == NULL) {
        return NULL;
    }

    if ((status = mbedtls_pk_parse_key(&key->key, buf, rv, NULL, 0)) != 0) {
        mbedtls_strerror(status, errbuf, sizeof errbuf);
        ERROR("mbedtls_pk_parse_key failed: %s", errbuf);
    } else if (!mbedtls_pk_can_do(&key->key, MBEDTLS_PK_ECDSA)) {
        ERROR("%s does not contain a usable ECDSA key.", file);
    } else {
        return key;
    }
    srp_keypair_free(key);
    return NULL;
}

// Function to generate a key
srp_key_t *
srp_generate_key(void)
{
    int status;
    char errbuf[64];
    srp_key_t *key = srp_key_setup();
    const mbedtls_pk_info_t *key_type = mbedtls_pk_info_from_type(MBEDTLS_PK_ECKEY);

    if (key == NULL || key_type == NULL) {
        return NULL;
    }
    
    if ((status = mbedtls_pk_setup(&key->key, key_type)) != 0) {
        mbedtls_strerror(status, errbuf, sizeof errbuf);
        ERROR("mbedtls_pk_setup failed: %s", errbuf);
    } else if ((status = mbedtls_ecdsa_genkey(mbedtls_pk_ec(key->key), MBEDTLS_ECP_DP_SECP256R1,
                                              mbedtls_ctr_drbg_random, &key->ctr)) != 0) {
        mbedtls_strerror(status, errbuf, sizeof errbuf);
        ERROR("mbedtls_ecdsa_genkey failed: %s", errbuf);
    } else {
        return key;
    }
    srp_keypair_free(key);
    return NULL;
}

// Function to write a keypair to a file
int
srp_write_key_to_file(const char *file, srp_key_t *key)
{
    int fd;
    unsigned char buf[256];
    ssize_t rv;
    int len;
    char errbuf[64];

    len = mbedtls_pk_write_key_der(&key->key, buf, sizeof buf);
    if (len <= 0) {
        mbedtls_strerror(len, errbuf, sizeof errbuf);
        ERROR("mbedtls_pk_write_key_der failed: %s", errbuf);
        return 0;
    }

#ifndef O_DIRECT
#define O_DIRECT 0
#endif
    fd = open(file, O_CREAT | O_EXCL | O_WRONLY | O_DIRECT, 0700);
    if (fd < 0) {
        ERROR("Unable to create srp.key: %s", strerror(errno));
        return 0;
    }        

    rv = write(fd, &buf[sizeof buf - len], len);
    close(fd);
    if (rv != len) {
        ERROR("key file write truncated.");
        unlink(file);
        return 0;
    }

    return 1;
}

// Function to get the length of the public key
size_t
srp_pubkey_length(srp_key_t *key)
{
    return ECDSA_KEY_SIZE;
}

int
srp_key_algorithm(srp_key_t *key)
{
    return dnssec_keytype_ecdsa;
}

size_t
srp_signature_length(srp_key_t *key)
{
    return ECDSA_KEY_SIZE;
}

// Function to copy out the public key as binary data
int
srp_pubkey_copy(uint8_t *buf, size_t max, srp_key_t *key)
{
    mbedtls_ecp_keypair *ecp = mbedtls_pk_ec(key->key);
    char errbuf[64];
    int status;

    if (max < ECDSA_KEY_SIZE) {
        return 0;
    }

    // Currently ECP only.
    if ((status = mbedtls_mpi_write_binary(&ecp->Q.X, buf, ECDSA_KEY_PART_SIZE)) != 0 ||
        (status = mbedtls_mpi_write_binary(&ecp->Q.Y, buf + ECDSA_KEY_PART_SIZE, ECDSA_KEY_PART_SIZE)) != 0) {
        mbedtls_strerror(status, errbuf, sizeof errbuf);
        ERROR("mbedtls_mpi_write_binary: %s", errbuf);
        return 0;
    }

#ifdef MBEDTLS_PUBKEY_DUMP
    int i;
    fprintf(stderr, "pubkey %d: ", ECDSA_KEY_SIZE);
    for (i = 0; i < ECDSA_KEY_SIZE; i++) {
        fprintf(stderr, "%02x", buf[i]);
    }
    putc('\n', stderr);
#endif

    return ECDSA_KEY_SIZE;
}

// Function to generate a signature given some data and a private key
int
srp_sign(uint8_t *output, size_t max, uint8_t *message, size_t msglen, uint8_t *rr, size_t rdlen, srp_key_t *key)
{
    int status;
    unsigned char hash[ECDSA_SHA256_HASH_SIZE];
    char errbuf[64];
    mbedtls_sha256_context sha;
    mbedtls_ecp_keypair *ecp = mbedtls_pk_ec(key->key);
    mbedtls_mpi r, s;

    if (max < ECDSA_SHA256_SIG_SIZE) {
        ERROR("srp_sign: not enough space in output buffer (%lu) for signature (%d).",
              (unsigned long)max, ECDSA_SHA256_SIG_SIZE);
        return 0;
    }

    mbedtls_sha256_init(&sha);
    memset(hash, 0, sizeof hash);
    mbedtls_mpi_init(&r);
    mbedtls_mpi_init(&s);

    // Calculate the hash across first the SIG RR (minus the signature) and then the message
    // up to but not including the SIG RR.
    if ((status = mbedtls_sha256_starts_ret(&sha, 0)) != 0 ||
        (status = srp_mbedtls_sha256_update_ret(&sha, rr, rdlen) != 0) ||
        (status = srp_mbedtls_sha256_update_ret(&sha, message, msglen)) != 0 ||
        (status = srp_mbedtls_sha256_finish_ret(&sha, hash)) != 0) {
        mbedtls_strerror(status, errbuf, sizeof errbuf);
        ERROR("mbedtls_sha_256 hash failed: %s", errbuf);
        return 0;
    }

    status = mbedtls_ecdsa_sign(&ecp->grp, &r, &s, &ecp->d, hash, sizeof hash,
                                mbedtls_ctr_drbg_random, &key->ctr);
    if (status != 0) {
        mbedtls_strerror(status, errbuf, sizeof errbuf);
        ERROR("mbedtls_ecdsa_sign failed: %s", errbuf);
        return 0;
    }

    if ((status = mbedtls_mpi_write_binary(&r, output, ECDSA_SHA256_SIG_PART_SIZE)) != 0 ||
        (status = mbedtls_mpi_write_binary(&s, output + ECDSA_SHA256_SIG_PART_SIZE,
                                           ECDSA_SHA256_SIG_PART_SIZE)) != 0) {
        mbedtls_strerror(status, errbuf, sizeof errbuf);
        ERROR("mbedtls_ecdsa_sign failed: %s", errbuf);
        return 0;
    }
    return 1;
}
    
// Local Variables:
// mode: C
// tab-width: 4
// c-file-style: "bsd"
// c-basic-offset: 4
// fill-column: 108
// indent-tabs-mode: nil
// End:
