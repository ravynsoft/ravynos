//
// Copyright (c) 2009-2019 Apple Inc. All rights reserved.
//
// lf_cs_checksum.h - Defines for checksum handling for livefiles
//                    Apple_CoreStorage plugin.
//
// This header is copied from CoreStorage project.
//

#ifndef _LF_CS_CHECKSUM_H
#define _LF_CS_CHECKSUM_H

#include <stdint.h>
#include <string.h>
#include <stddef.h>
#include <stdbool.h>

//
// Checksum Algorithm
//
// Every metadata block has a checksum to check the data integrity.  Different
// checksum algorithms can be used by CoreStorage, and [[lvg_checksum_alg]] in
// [[lvg_info_t]] records what checksum algorithm to use.  For the first
// prototype, I am copying the CRC algorithm from the UDF project.
//
// The fletcher 2 algorithm used by ZFS has fundamental flaws.  And a variation
// of the fletcher 8 algorithm could be used, which is 2 times faster than
// CRC32 and 4 times faster than fletcher.
//
// <cksum definitions>=
//
#define MAX_CKSUM_NBYTES 8

enum cksum_alg {
	CKSUM_NONE,
	CKSUM_ALG_CRC_32, // CRC-32 algorithm copied from CRC32-C polynomial
};
typedef enum cksum_alg cksum_alg_t;


#define BITS_PER_BYTE 8L
#define META_CKSUM_BITS (MAX_CKSUM_NBYTES * BITS_PER_BYTE)

//
// Initialize checksum for a certain algorithm
//
bool cksum_init(cksum_alg_t alg, uint8_t cksum[MAX_CKSUM_NBYTES]);

//
// cksum - Calculate checksum of data for len bytes
//
// Existing value of cksum is also used to calculate the checksum.  In order to
// calculate cksums of different memory chunks, this function can be called
// several times:
//
//     cksum(alg, data1, len1, cksum);
//     cksum(alg, data2, len2, cksum);
//
// and the resultant cksum is calculated on two chunks
//
void cksum(cksum_alg_t alg, const void *data, size_t len,
		uint8_t cksum[MAX_CKSUM_NBYTES]);

#endif /* _LF_CS_CHECKSUM_H */
