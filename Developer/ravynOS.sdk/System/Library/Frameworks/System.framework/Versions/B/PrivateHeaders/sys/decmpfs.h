/*
 * Copyright (c) 2008 Apple Inc. All rights reserved.
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
#ifndef _SYS_DECMPFS_H_
#define _SYS_DECMPFS_H_ 1

#include <stdbool.h>
#include <sys/kdebug.h>
#include <sys/kernel_types.h>
#include <sys/vnode.h>

/*
 * Please switch on @DECMPFS_ENABLE_KDEBUG_TRACES to enable tracepoints.
 * Tracepoints are compiled out by default to eliminate any overhead due to
 * kernel tracing.
 *
 * #define DECMPFS_ENABLE_KDEBUG_TRACES 1
 */
#if DECMPFS_ENABLE_KDEBUG_TRACES
#define DECMPFS_EMIT_TRACE_ENTRY(D, ...) \
	KDBG_FILTERED((D) | DBG_FUNC_START, ## __VA_ARGS__)
#define DECMPFS_EMIT_TRACE_RETURN(D, ...) \
	KDBG_FILTERED((D) | DBG_FUNC_END, ##__VA_ARGS__)
#else
#define DECMPFS_EMIT_TRACE_ENTRY(D, ...) do {} while (0)
#define DECMPFS_EMIT_TRACE_RETURN(D, ...) do {} while (0)
#endif /* DECMPFS_ENABLE_KDEBUG_TRACES */

/*
 * KERNEL_DEBUG related definitions for decmpfs.
 *
 * Please NOTE: The Class DBG_FSYSTEM = 3, and Subclass DBG_DECMP = 0x12, so
 * these debug codes are of the form 0x0312nnnn.
 */
#define DECMPDBG_CODE(code)  FSDBG_CODE(DBG_DECMP, code)

enum {
	DECMPDBG_DECOMPRESS_FILE            = DECMPDBG_CODE(0),/* 0x03120000 */
	DECMPDBG_FETCH_COMPRESSED_HEADER    = DECMPDBG_CODE(1),/* 0x03120004 */
	DECMPDBG_FETCH_UNCOMPRESSED_DATA    = DECMPDBG_CODE(2),/* 0x03120008 */
	DECMPDBG_FREE_COMPRESSED_DATA       = DECMPDBG_CODE(4),/* 0x03120010 */
	DECMPDBG_FILE_IS_COMPRESSED         = DECMPDBG_CODE(5),/* 0x03120014 */
};

#define MAX_DECMPFS_XATTR_SIZE 3802

/*
 *  NOTE:  decmpfs can only be used by thread-safe filesystems
 */

#define DECMPFS_MAGIC 0x636d7066 /* cmpf */

#define DECMPFS_XATTR_NAME "com.apple.decmpfs" /* extended attribute to use for decmpfs */

/*
 * This single field is to be interpreted differently depending on the
 * corresponding item type.
 * For regular files: it is a 64bits-encoded logical size
 * For directories: it is a 64bits-encoded number of children (ie st_nlink - 2)
 * For packages: it is 40bits encoded size and 24bits number of children at root
 */
typedef struct __attribute__((packed)) {
	uint64_t  value;
} decmpfs_raw_item_size;

#define DECMPFS_PKG_SIZE_MASK           0x000000ffffffffffULL
#define DECMPFS_PKG_COUNT_MASK          0xffffff
#define DECMPFS_PKG_CHLD_COUNT_SHIFT    40

#define DECMPFS_PKG_SIZE(x)             ((x).value & DECMPFS_PKG_SIZE_MASK)
#define DECMPFS_PKG_CHLD_COUNT(x)       ((uint32_t)(((x).value >> DECMPFS_PKG_CHLD_COUNT_SHIFT) & DECMPFS_PKG_COUNT_MASK))
#define DECMPFS_PKG_VALUE_FROM_SIZE_COUNT(size, count) \
	(((size) & DECMPFS_PKG_SIZE_MASK) | ((uint64_t)(count) << DECMPFS_PKG_CHLD_COUNT_SHIFT))

/* Dataless file or directory */
#define DATALESS_CMPFS_TYPE     0x80000001

/* Dataless package, with number of root children and total size encoded on disk */
#define DATALESS_PKG_CMPFS_TYPE 0x80000002


static inline bool
decmpfs_type_is_dataless(uint32_t cmp_type)
{
	return cmp_type == DATALESS_CMPFS_TYPE || cmp_type == DATALESS_PKG_CMPFS_TYPE;
}

typedef struct __attribute__((packed)) {
	/* this structure represents the xattr on disk; the fields below are little-endian */
	uint32_t compression_magic;
	uint32_t compression_type;   /* see the enum below */
	union {
		uint64_t uncompressed_size;  /* compatility accessor */
		decmpfs_raw_item_size _size;
	};
	unsigned char attr_bytes[0]; /* the bytes of the attribute after the header */
} decmpfs_disk_header;

typedef struct __attribute__((packed)) {
	/* this structure represents the xattr in memory; the fields below are host-endian */
	uint32_t attr_size;
	uint32_t compression_magic;
	uint32_t compression_type;
	union {
		/*
		 * although uncompressed_size remains available for backward-compatibility reasons
		 * the uncompressed size and nchildren should be accessed using the inline helpers
		 * below
		 */
		uint64_t uncompressed_size;
		decmpfs_raw_item_size _size;
	};
	unsigned char attr_bytes[0]; /* the bytes of the attribute after the header */
} decmpfs_header;

static inline uint64_t
decmpfs_get_uncompressed_size(const decmpfs_header *hdr)
{
	if (hdr->compression_magic == DECMPFS_MAGIC && hdr->compression_type == DATALESS_PKG_CMPFS_TYPE) {
		return DECMPFS_PKG_SIZE(hdr->_size);
	}

	return hdr->uncompressed_size;
}

static inline uint32_t
decmpfs_get_directory_entries(const decmpfs_header *hdr)
{
	if (hdr->compression_magic == DECMPFS_MAGIC && hdr->compression_type == DATALESS_PKG_CMPFS_TYPE) {
		return DECMPFS_PKG_CHLD_COUNT(hdr->_size);
	}

	return (uint32_t)hdr->uncompressed_size;
}

/* compression_type values */
enum {
	CMP_Type1       = 1,/* uncompressed data in xattr */

	/* additional types defined in AppleFSCompression project */

	CMP_MAX         = 255/* Highest compression_type supported */
};

typedef struct {
	void *buf;
	user_ssize_t size;
} decmpfs_vector;


#endif /* _SYS_DECMPFS_H_ */
