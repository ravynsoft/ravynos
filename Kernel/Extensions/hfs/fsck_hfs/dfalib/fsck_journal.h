/*
 * Copyright (c) 2000-2012 Apple Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 * 
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */

#ifndef _FSCK_JOURNAL_H
#define _FSCK_JOURNAL_H

#include <sys/cdefs.h>

#include <sys/types.h>

/*
 * The guts of the journal:  a descriptor for which
 * block number on the data disk is to be written.
 */
typedef struct block_info {
	uint64_t	bnum;
	uint32_t	bsize;
	uint32_t	next;
} __attribute__((__packed__)) block_info;

/*
 * A "transaction," for want of a better word.
 * This contains a series of block_info, in the
 * binfo array, which are used to modify the
 * filesystem.
 */
typedef struct block_list_header {
	uint16_t	max_blocks;
	uint16_t	num_blocks;
	uint32_t	bytes_used;
	uint32_t	checksum;
	uint32_t	pad;
	block_info	binfo[1];
} __attribute__((__packed__)) block_list_header;

/*
 * This is written to block zero of the journal and it
 * maintains overall state about the journal.
 */
typedef struct journal_header {
    int32_t        magic;
    int32_t        endian;
    off_t	 start;         // zero-based byte offset of the start of the first transaction
    off_t	end;           // zero-based byte offset of where free space begins
    off_t          size;          // size in bytes of the entire journal
    int32_t        blhdr_size;    // size in bytes of each block_list_header in the journal
    int32_t        checksum;
    int32_t        jhdr_size;     // block size (in bytes) of the journal header
    uint32_t       sequence_num;  // NEW FIELD: a monotonically increasing value assigned to all txn's
} __attribute__((__packed__)) journal_header;

#define JOURNAL_HEADER_MAGIC	0x4a4e4c78   // 'JNLx'
#define	OLD_JOURNAL_HEADER_MAGIC	0x4a484452   // 'JHDR'
#define ENDIAN_MAGIC	0x12345678

//
// we only checksum the original size of the journal_header to remain
// backwards compatible.  the size of the original journal_header is
// everything up to the the sequence_num field, hence we use the
// offsetof macro to calculate the size.
//
#define JOURNAL_HEADER_CKSUM_SIZE  (offsetof(struct journal_header, sequence_num))

#define OLD_JOURNAL_HEADER_MAGIC  0x4a484452   // 'JHDR'

/*
 * The function used by fsck_hfs to replay the journal.
 * It's modeled on the kernel function.
 *
 * For the do_write_b block, the offset argument is in bytes --
 * the journal replay code will convert from journal block to
 * bytes.
 */

int	journal_open(int jdev,
		     off_t         offset,
		     off_t         journal_size,
		     size_t        min_fs_block_size,
		     uint32_t       flags,
		     const char	*jdev_name,
		     int (^do_write_b)(off_t, void *, size_t));

#endif /* !_FSCK_JOURNAL_H */
