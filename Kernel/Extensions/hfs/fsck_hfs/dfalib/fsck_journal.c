/*
 * Copyright (c) 2010-2012 Apple Inc. All rights reserved.
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

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/disk.h>
#include <sys/param.h>

#include "../fsck_hfs.h"
#include "fsck_journal.h"

#define DEBUG_JOURNAL 0

extern char debug;

#include <hfs/hfs_format.h>
#include <libkern/OSByteOrder.h>

typedef struct SwapType {
	const char *name;
	uint16_t (^swap16)(uint16_t);
	uint32_t (^swap32)(uint32_t);
	uint64_t (^swap64)(uint64_t);
} swapper_t;

static swapper_t nativeEndian = {
	"native endian",
	^(uint16_t x) { return x; },
	^(uint32_t x) { return x; },
	^(uint64_t x) { return x; }
};

static swapper_t swappedEndian = {
	"swapped endian",
	^(uint16_t x) { return OSSwapInt16(x); },
	^(uint32_t x) { return OSSwapInt32(x); },
	^(uint64_t x) { return OSSwapInt64(x); }
};

typedef int (^journal_write_block_t)(off_t, void *, size_t);

//
// this isn't a great checksum routine but it will do for now.
// we use it to checksum the journal header and the block list
// headers that are at the start of each transaction.
//
static uint32_t
calc_checksum(char *ptr, int len)
{
	int i;
	uint32_t cksum = 0;

	// this is a lame checksum but for now it'll do
	for(i = 0; i < len; i++, ptr++) {
		cksum = (cksum << 8) ^ (cksum + *(unsigned char *)ptr);
	}

	return (~cksum);
}

typedef struct JournalIOInfo {
	int		jfd;	// File descriptor for journal buffer
	int		wrapCount;	// Incremented when it wraps around.
	size_t		bSize;	// Block size.  I/O needs to be done in that amount.
	uint64_t	base;	// Base offset of journal buffer, past the header
	uint64_t	size;	// Size of the journal, minus the header size
	uint64_t	end;	// End of the journal (initially the "end" field from the journal header)
	uint64_t	current;	// Current offset; starts at "start"
} JournalIOInfo_t;

/*
 * Attempt to read <length> bytes from the journal buffer.
 * Since this is a wrapped buffer, it may have to start at the
 * beginning.  info->{base, size, end} are read-only; info->current
 * is updated with the current offset.  It returns the number of bytes
 * it read, or -1 on error.
 */
static ssize_t
journalRead(JournalIOInfo_t *info, uint8_t *buffer, size_t length)
{
	size_t nread = 0;
	uint8_t *ptr = buffer;

//	fprintf(stderr, "%s(%p, %p, %zu)\n", __FUNCTION__, info, buffer, length);
	if (info->wrapCount > 1) {
		fplog(stderr, "%s(%p, %p, %zu):  journal buffer wrap count = %d\n", __FUNCTION__, info, buffer, length, info->wrapCount);
		return -1;
	}
	while (nread < length) {
		off_t end;
		size_t amt;
		ssize_t n;

		if (info->end < info->current) {
			// It wraps, so we max out at bse+size
			end = info->base + info->size;
		} else {
			end = info->end;
		}
		amt = MIN((length - nread), (end - info->current));
		if (amt == 0) {
			if (debug) {
				fplog(stderr, "Journal read amount is 0, is that right?\n");
			}
			goto done;
		}

		n = pread(info->jfd, ptr, amt, info->current);
		if (n == -1) {
			warn("pread(%d, %p, %zu, %llu)", info->jfd, ptr, amt, info->current);
			goto done;
		}
		if (n != amt) {
			if (debug) {
				fplog(stderr, "%s(%d):  Wanted to read %zu, but only read %zd\n", __FUNCTION__, __LINE__, amt, n);
			}
		}
		nread += n;
		ptr += n;
		info->current += n;
		if (info->current == (info->base + info->size)) {
			info->current = info->base;
			info->wrapCount++;
		}
	}
done:
	return nread;
}

/*
 * Read a transaction from the journal buffer.
 * A transaction is a list of block_list_headers, and their
 * associated data.  It needs to read all of the block_lists in
 * a transaction, or it fails.  It returns NULL if there are
 * no transactions, and on error.  (Maybe that should change?)
 */
static block_list_header *
getJournalTransaction(JournalIOInfo_t *jinfo, swapper_t *swap)
{
	block_list_header *retval = NULL;
	uint8_t block[jinfo->bSize];
	block_list_header *hdr = (void*)&block;
	ssize_t nread;
	ssize_t amt;

	memset(block, 0, sizeof(block));
	nread = journalRead(jinfo, block, sizeof(block));
	if (nread == -1 ||
	    (size_t)nread != sizeof(block)) {
		if (debug)
			plog("%s:  wanted %zd, got %zd\n", __FUNCTION__, sizeof(block), nread);
		return NULL;
	}
	if (swap->swap32(hdr->num_blocks) == 0) {
		/*
		 * Either there really are no blocks, or this is not a valid
		 * transaction.  Either way, there's nothing for us to do here.
		 */
#if DEBUG_JOURNAL
		if (debug)
			fplog(stderr, "%s(%d):  hdr->num_blocks == 0\n", __FUNCTION__, __LINE__);
#endif
		return NULL;
	}
	/*
	 * Now we check the checksum to see if this is a valid header.
	 * Note that we verify the checksum before reading any more -- if
	 * it's not a valid header, we don't want to read more than a block
	 * size.
	 */
	uint32_t tmpChecksum = swap->swap32(hdr->checksum);
	uint32_t compChecksum;
	hdr->checksum = 0;
	compChecksum = calc_checksum((void*)hdr, sizeof(*hdr));
	hdr->checksum = swap->swap32(tmpChecksum);

	if (compChecksum != tmpChecksum) {
		if (debug)
			fplog(stderr, "%s(%d):  hdr has bad checksum, returning NULL\n", __FUNCTION__, __LINE__);
		return NULL;
	}

	if (swap->swap32(hdr->bytes_used) < sizeof(block)) {
#if DEBUG_JOURNAL
		if (debug) {
			fplog(stderr, "%s(%d):  hdr has bytes_used (%u) less than sizeof block (%zd)\n",
			      __FUNCTION__, __LINE__, swap->swap32(hdr->bytes_used), sizeof(block));
		}
#endif
		return NULL;
	}

	retval = malloc(swap->swap32(hdr->bytes_used));
	if (retval == NULL)
		return NULL;

	memset(retval, 0, swap->swap32(hdr->bytes_used));
	memcpy(retval, block, sizeof(block));
	amt = swap->swap32(hdr->bytes_used) - sizeof(block);
	nread = journalRead(jinfo, ((uint8_t*)retval) + sizeof(block), amt);
	if (nread != amt) {
		free(retval);
		return NULL;
	}

	return retval;
}

/*
 * Replay a transaction.
 * Transactions have a blockListSize amount of block_list_header, and
 * are then followed by data.  We read it in, verify the checksum, and
 * if it's good, we call the block that was passed in to do something
 * with it.  Maybe write it out.  Maybe laugh about it.
 *
 * It returns -1 if there was an error before it wrote anything out,
 * and -2 if there was an error after it wrote something out.
 *
 * The arguments are:
 * txn	-- a block_list_header pointer, which has the description and data
 * 	to be replayed.
 * blSize	-- the size of the block_list for this journal.  (The data
 *		are after the block_list, but part of the same buffer.)
 * blkSize	-- The block size used to convert block numbers to offsets.  This
 *		is defined to be the size of the journal header.
 * swap	-- A pointer to a swapper_t used to swap journal data structure elements.
 * writer	-- A block-of-code that does writing.
 *
 * "writer" should return -1 to stop the replay (this propagates an error up).
 */
static int
replayTransaction(block_list_header *txn, size_t blSize, size_t blkSize, swapper_t *swap, journal_write_block_t writer)
{
	uint32_t i;
	uint8_t *endPtr = ((uint8_t*)txn) + swap->swap32(txn->bytes_used);
	uint8_t *dataPtr = ((uint8_t*)txn) + blSize;
	int retval = -1;
	for (i = 1; i < swap->swap32(txn->num_blocks); i++) {
#if DEBUG_JOURNAL
		if (debug)
			plog("\tBlock %d:  blkNum %llu, size %u, data offset = %zd\n", i, swap->swap64(txn->binfo[i].bnum), swap->swap32(txn->binfo[i].bsize), dataPtr - (uint8_t*)txn);
#endif
		/*
		 * XXX
		 * Check with security types on these checks.  Need to ensure
		 * that the fields don't take us off into the dark scary woods.
		 * It's mostly the second one that I am unsure about.
		 */
		if (dataPtr > endPtr) {
			if (debug)
				plog("\tData out of range for block_list_header\n");
			return retval;
		}
		if ((endPtr - dataPtr) < swap->swap32(txn->binfo[i].bsize)) {
			if (debug)
				plog("\tData size for block %d out of range for block_list_header\n", i);
			return retval;
		}
		if ((dataPtr + swap->swap32(txn->binfo[i].bsize)) > endPtr) {
			if (debug)
				plog("\tData end out of range for block_list_header\n");
			return retval;
		}
#if DEBUG_JOURNAL
		// Just for debugging
		if (debug) {
			if (swap->swap64(txn->binfo[i].bnum) == 2) {
				HFSPlusVolumeHeader *vp = (void*)dataPtr;
				plog("vp->signature = %#x, version = %#x\n", vp->signature, vp->version);
			}
		}
#endif
		// It's in the spec, and I saw it come up once on a live volume.
		if (swap->swap64(txn->binfo[i].bnum) == ~(uint64_t)0) {
#if DEBUG_JOURNAL
			if (debug)
				plog("\tSkipping this block due to magic skip number\n");
#endif
		} else {
			// Should we set retval to -2 here?
			if (writer) {
				if ((writer)(swap->swap64(txn->binfo[i].bnum) * blkSize, dataPtr, swap->swap32(txn->binfo[i].bsize)) == -1)
					return retval;
			}
		}
		dataPtr += swap->swap32(txn->binfo[i].bsize);
		retval = -2;
	}
	return 0;
}

/*
 * Read a journal header in from the journal device.
 */
static int
loadJournalHeader(int jfd, off_t offset, size_t blockSize, journal_header *jhp)
{
	uint8_t buffer[blockSize];
	ssize_t nread;

	nread = pread(jfd, buffer, sizeof(buffer), offset);
	if (nread == -1 ||
	    (size_t)nread != sizeof(buffer)) {
		warn("tried to read %zu for journal header buffer, got %zd", sizeof(buffer), nread);
		return -1;
	}
	*jhp = *(journal_header*)buffer;
	return 0;
}

/*
 * Replay a journal (called "journal_open" because you have to
 * to replay it as part of opening it).  At this point, all it
 * is useful for is replaying the journal.
 *
 * It is passed in:
 *     jfd	-- file descriptor for the journal device
 *     offset	-- offset (in bytes) of the journal on the journal device
 *     journal_size	-- size of the jorunal (in bytes)
 *     min_fs_blksize	-- Blocksize of the data filesystem
 *     flags	-- unused for now
 *     jdev_name	-- string name for the journal device.  used for logging.
 *     do_write_b	-- a block which does the actual writing.
 *
 * Currently, for fsck_hfs, the do_write_b block writes to the cache.  It could also
 * just print out the block numbers, or just check their integrity, as much as is
 * possible.
 * 
 * The function works by loading the journal header.  From there, it then starts
 * loading transactions, via block_list_header groups.  When it gets to the end
 * of the journal, it tries continuing, in case there were transactions that
 * didn't get updated in the header (this apparently happens).
 * 
 * It returns 0 on success, and -1 on error.  Note that there's not a lot
 * fsck_hfs can probably do in the event of error.
 *
 */
int
journal_open(int jfd,
	     off_t	offset,		// Offset of journal
	     off_t	journal_size,	// Size, in bytes, of the entire journal
	     size_t	min_fs_blksize,	// Blocksize of the data filesystem, journal blocksize must be at least this size
	     uint32_t	flags __unused,	// Not used in this implementation
	     const char	*jdev_name,	// The name of the journal device, for logging
	     int (^do_write_b)(off_t, void*, size_t))
{
	journal_header jhdr = { 0 };
	swapper_t	*jnlSwap;	// Used to swap fields of the journal
	uint32_t	tempCksum;	// Temporary checksum value
	uint32_t	jBlkSize = 0;

	if (ioctl(jfd, DKIOCGETBLOCKSIZE, &jBlkSize) == -1) {
		jBlkSize = (uint32_t)min_fs_blksize;
	} else {
		if (jBlkSize < min_fs_blksize) {
			fplog(stderr, "%s:  journal block size %u < min block size %zu for %s\n", __FUNCTION__, jBlkSize, min_fs_blksize, jdev_name);
			return -1;
		}
		if ((jBlkSize % min_fs_blksize) != 0) {
			fplog(stderr, "%s:  journal block size %u is not a multiple of fs block size %zu for %s\n", __FUNCTION__, jBlkSize, min_fs_blksize, jdev_name);
			return -1;
		}
	}
	if (loadJournalHeader(jfd, offset, jBlkSize, &jhdr) != 0) {
		fplog(stderr, "%s:  unable to load journal header from %s\n", __FUNCTION__, jdev_name);
		return -1;
	}

	/*
	 * Unlike the rest of the filesystem, the journal can be in native or
	 * non-native byte order.  Barring moving a filesystem from one host
	 * to another, it'll almost always be in native byte order.
	 */
	if (jhdr.endian == ENDIAN_MAGIC) {
		jnlSwap = &nativeEndian;
	} else if (OSSwapInt32(jhdr.endian) == ENDIAN_MAGIC) {
		jnlSwap = &swappedEndian;
	} else {
		fplog(stderr, "%s:  Unknown journal endian magic number %#x from %s\n", __FUNCTION__, jhdr.endian, jdev_name);
		return -1;
	}
	/*
	 * Two different magic numbers are valid.
	 * Do they mean different thigs, though?
	 */
	if (jnlSwap->swap32(jhdr.magic) != JOURNAL_HEADER_MAGIC &&
	    jnlSwap->swap32(jhdr.magic) != OLD_JOURNAL_HEADER_MAGIC) {
		fplog(stderr, "%s:  Unknown journal header magic number %#x from %s\n", __FUNCTION__, jhdr.magic, jdev_name);
		return -1;
	}

	/*
	 * Checksums have to be done with the checksum field set to 0.
	 * So we have to stash it aside for a bit, and set the field to
	 * 0, before we can compare.  Afterwards, if it compares correctly,
	 * we put the original (swapped, if necessary) value back, just
	 * in case.
	 */
	tempCksum = jnlSwap->swap32(jhdr.checksum);
	jhdr.checksum = 0;
	if (jnlSwap->swap32(jhdr.magic) == JOURNAL_HEADER_MAGIC &&
	    (calc_checksum((void*)&jhdr, JOURNAL_HEADER_CKSUM_SIZE) != tempCksum)) {
		fplog(stderr, "%s:  Invalid journal checksum from %s\n", __FUNCTION__, jdev_name);
		return -1;
	}
	jhdr.checksum = jnlSwap->swap32(tempCksum);

	/*
	 * Set up information about the journal which we use to do the I/O.
	 * The journal is a circular buffer.  However, the start of the journal
	 * buffer is past the journal header.  See the JournalIOInfo structure above.
	 */
	off_t startOffset = jnlSwap->swap64(jhdr.start);
	off_t endOffset =jnlSwap->swap64(jhdr.end);
	off_t journalStart = offset + jnlSwap->swap32(jhdr.jhdr_size);

	/*
	 * The journal code was updated to be able to read past the "end" of the journal,
	 * to see if there were any valid transactions there.  If we are peeking past the
	 * end, we don't care if we have checksum errors -- that just means they're not
	 * valid transactions.
	 *
	 */
	int into_the_weeds = 0;
	uint32_t last_sequence_number = 0;

	JournalIOInfo_t jinfo = { 0 };

#if DEBUG_JOURNAL
	if (debug)
		plog("Journal start sequence number = %u\n", jnlSwap->swap32(jhdr.sequence_num));
#endif

	/*
	 * Now set up the JournalIOInfo object with the file descriptor,
	 * the block size, start and end of the journal buffer, and where
	 * the journal pointer currently is.
	 */
	jinfo.jfd = jfd;
	jinfo.bSize = jnlSwap->swap32(jhdr.jhdr_size);
	jinfo.base = journalStart;
	jinfo.size = journal_size - jinfo.bSize;
	jinfo.end = offset + endOffset;
	jinfo.current = offset + startOffset;

	const char *state = "";
	int bad_journal = 0;
	block_list_header *txn = NULL;

	/*
	 * Loop while getting transactions.  We exit when we hit a checksum
	 * error, or when the sequence number for a transaction doesn't match
	 * what we expect it to.  (That's the trickiest part -- the into_the_weeds
	 * portion of the code.  It doesn't match the TN11150 documentation, so
	 * I've had to go by both my experience with real-world journals and by
	 * looking at the kernel code.)
	 */
	while (1) {
		int rv;

		if (jinfo.current == jinfo.end && into_the_weeds == 0) {
			/*
			 * This is a bit weird, but it works:  if current == end, but gone_into_weeds is 1,
			 * then this code will not execute.  If it does execute, it'll go to get a transaction.
			 * That will put the pointer past end.
			 */
			if (jhdr.sequence_num == 0) {
				/*
				 * XXX
				 * I am not sure about this; this behaviour is not in TN1150 at all,
				 * but I _think_ this is what the kernel is doing.
                 */
#if DEBUG_JOURNAL
                if (debug)
                    plog("Journal sequence number is 0, is going into the end okay?\n");
#endif
			}
			into_the_weeds = 1;
#if DEBUG_JOURNAL
			if (debug)
				plog("Attempting to read past stated end of journal\n");
#endif
			state = "tentative ";
			jinfo.end = (jinfo.base + startOffset - jinfo.bSize);
			continue;
		}
#if DEBUG_JOURNAL
		if (debug)
			plog("Before getting %stransaction:  jinfo.current = %llu\n", state, jinfo.current);
#endif
		/*
		 * Note that getJournalTransaction verifies the checksum on the block_list_header, so
		 * if it's bad, it'll return NULL.
		 */
		txn = getJournalTransaction(&jinfo, jnlSwap);
		if (txn == NULL) {
#if DEBUG_JOURNAL
			if (debug)
				plog("txn is NULL, jinfo.current = %llu\n", jinfo.current);
#endif
			if (into_the_weeds) {
#if DEBUG_JOURNAL
				if (debug)
					plog("\tBut we do not care, since it is past the end of the journal\n");
#endif
			} else {
				bad_journal = 1;
			}
			break;
		}
#if DEBUG_JOURNAL
		if (debug) {
			plog("After getting %stransaction:  jinfo.current = %llu\n", state, jinfo.current);
			plog("%stxn = { %u max_blocks, %u num_blocks, %u bytes_used, binfo[0].next = %u }\n", state, jnlSwap->swap32(txn->max_blocks), jnlSwap->swap32(txn->num_blocks), jnlSwap->swap32(txn->bytes_used), jnlSwap->swap32(txn->binfo[0].next));
		}
#endif
		if (into_the_weeds) {
			/*
			 * This seems to be what the kernel was checking:  if the
			 * last_sequence_number was set, and the txn sequence number
			 * is set, and the txn sequence number doesn't match either
			 * last_sequence_number _or_ an incremented version of it, then
			 * the transaction isn't worth looking at, and we've reached
			 * the end of the journal.
			 */
			if (last_sequence_number != 0 &&
			    txn->binfo[0].next != 0 &&
			    jnlSwap->swap32(txn->binfo[0].next) != last_sequence_number &&
			    jnlSwap->swap32(txn->binfo[0].next) != (last_sequence_number + 1)) {
				// Probably not a valid transaction
#if DEBUG_JOURNAL
				if (debug)
					plog("\tTentative txn sequence %u is not expected %u, stopping journal replay\n", jnlSwap->swap32(txn->binfo[0].next), last_sequence_number + 1);
#endif
				break;
			}
		}
		/*
		 * If we've got a valid transaction, then we replay it.
		 * If there was an error, we're done with the journal replay.
		 * (If the error occurred after the "end," then we don't care,
		 * and it's not a bad journal.)
		 */
		rv = replayTransaction(txn,
				       jnlSwap->swap32(jhdr.blhdr_size),
				       jnlSwap->swap32(jhdr.jhdr_size),
				       jnlSwap,
				       do_write_b);

		if (rv < 0) {
			if (debug)
				plog("\tTransaction replay failed, returned %d\n", rv);
			if (into_the_weeds) {
				if (debug)
					plog("\t\tAnd we don't care\n");
			} else {
				bad_journal = 1;
			}
			break;
		}
		last_sequence_number = jnlSwap->swap32(txn->binfo[0].next);
		free(txn);
		txn = NULL;
	}
	if (txn)
		free(txn);
	if (bad_journal) {
		if (debug)
			plog("Journal was bad, stopped replaying\n");
		return -1;
	}

	return 0;
}
