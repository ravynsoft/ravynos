/*
 * Copyright (c) 2015 Apple Inc. All rights reserved.
 *
 * @APPLE_APACHE_LICENSE_HEADER_START@
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
 * @APPLE_APACHE_LICENSE_HEADER_END@
 */

#ifndef __FIREHOSE_BUFFER_INTERNAL__
#define __FIREHOSE_BUFFER_INTERNAL__

#if BYTE_ORDER != LITTLE_ENDIAN
#error unsupported byte order
#endif

#ifndef KERNEL
#include <os/lock_private.h>
#endif

// firehose buffer is CHUNK_COUNT * CHUNK_SIZE big == 256k
#define FIREHOSE_BUFFER_CHUNK_COUNT					64ul
#ifndef KERNEL
#define FIREHOSE_BUFFER_CHUNK_PREALLOCATED_COUNT	4
#define FIREHOSE_BUFFER_MADVISE_CHUNK_COUNT			4
#endif

#define FIREHOSE_RING_POS_GEN_INC		((uint16_t)(FIREHOSE_BUFFER_CHUNK_COUNT))
#define FIREHOSE_RING_POS_IDX_MASK		((uint16_t)(FIREHOSE_RING_POS_GEN_INC - 1))
#define FIREHOSE_RING_POS_GEN_MASK		((uint16_t)~FIREHOSE_RING_POS_IDX_MASK)

#if __has_feature(c_static_assert)
_Static_assert(FIREHOSE_RING_POS_IDX_MASK < 0xff,
		"firehose chunk ref fits in its type with space for PRISTINE");
#endif

typedef uint8_t firehose_chunk_ref_t;

static const unsigned long firehose_stream_uses_io_bank =
	(1UL << firehose_stream_persist) |
	(1UL << firehose_stream_special) |
	(1UL << firehose_stream_signpost);

typedef union {
#define FIREHOSE_BANK_SHIFT(bank)			(16 * (bank))
#define FIREHOSE_BANK_INC(bank)				(1ULL << FIREHOSE_BANK_SHIFT(bank))
	uint64_t fbs_atomic_state;
	struct {
		union {
			struct {
				uint16_t fbs_mem_bank;
				uint16_t fbs_io_bank;
			};
			uint16_t fbs_banks[2];
		};
		firehose_chunk_ref_t fbs_max_ref;
		uint8_t fbs_unused1;
		uint16_t fbs_unused2;
	};
} firehose_bank_state_u;

#if __has_feature(c_static_assert)
_Static_assert(8 * offsetof(firehose_bank_state_u, fbs_mem_bank)
		== FIREHOSE_BANK_SHIFT(0), "mem bank shift");
_Static_assert(8 * offsetof(firehose_bank_state_u, fbs_io_bank)
		== FIREHOSE_BANK_SHIFT(1), "mem bank shift");
#endif

typedef struct firehose_buffer_bank_s {
	firehose_bank_state_u volatile fbb_state;
	uint64_t volatile fbb_metadata_bitmap;
	uint64_t volatile fbb_mem_flushed;
	uint64_t volatile fbb_mem_notifs;
	uint64_t volatile fbb_mem_sync_pushes;
	uint64_t volatile fbb_io_flushed;
	uint64_t volatile fbb_io_notifs;
	uint64_t volatile fbb_io_sync_pushes;
#define FIREHOSE_BUFFER_BANK_FLAG_LOW_MEMORY	(1UL << 0)
#define FIREHOSE_BUFFER_BANK_FLAG_HIGH_RATE		(1UL << 1)
	unsigned long volatile fbb_flags;

	uint64_t fbb_bitmap; // protected by fbb_lock
	firehose_bank_state_u fbb_limits; // protected by fbb_lock
#ifdef KERNEL
	uint32_t _fbb_unused;
#else
	dispatch_unfair_lock_s fbb_lock;
#endif
} OS_ALIGNED(64) *firehose_buffer_bank_t;

typedef union {
	uint64_t fss_atomic_state;
	dispatch_gate_s fss_gate;
	struct {
#define FIREHOSE_GATE_RELIABLE_WAITERS_BIT 		0x00000001UL
#define FIREHOSE_GATE_UNRELIABLE_WAITERS_BIT 	0x00000002UL
#define FIREHOSE_GATE_WAITERS_MASK 				0x00000003UL
		uint32_t fss_allocator;
#define FIREHOSE_STREAM_STATE_PRISTINE		0xff
		firehose_chunk_ref_t fss_current;
		uint8_t fss_loss : FIREHOSE_LOSS_COUNT_WIDTH;
		uint8_t fss_timestamped : 1;
		uint8_t fss_waiting_for_logd : 1;

		/*
		 * We use a generation counter to prevent a theoretical ABA problem: a
		 * thread could try to acquire a tracepoint in a chunk, fail to do so,
		 * mark it as to be pushed, enqueue it, and then be preempted.  It
		 * sleeps for a long time, and then tries to acquire the allocator bit
		 * and uninstall the chunk. Succeeds in doing so, but because the chunk
		 * actually happened to have cycled all the way back to being installed.
		 * That thread would effectively hide that unflushed chunk and leak it.
		 * Having a generation counter prevents the uninstallation of the chunk
		 * to spuriously succeed when it was a re-incarnation of it.
		 */
		uint16_t fss_generation;
	};
} firehose_stream_state_u;

typedef struct firehose_buffer_stream_s {
	firehose_stream_state_u fbs_state;
	uint64_t fbs_loss_start; // protected by fss_gate
} OS_ALIGNED(128) *firehose_buffer_stream_t;

typedef union {
	uint64_t frp_atomic_tail;
	struct {
		uint16_t frp_mem_tail;
		uint16_t frp_mem_flushed;
		uint16_t frp_io_tail;
		uint16_t frp_io_flushed;
	};
} firehose_ring_tail_u;

OS_ENUM(firehose_buffer_pushport, uint8_t,
	FIREHOSE_BUFFER_PUSHPORT_MEM,
	FIREHOSE_BUFFER_PUSHPORT_IO,
	FIREHOSE_BUFFER_NPUSHPORTS,
);

/*
 * Rings are circular buffers with CHUNK_COUNT entries, with 3 important markers
 *
 * +--------+-------------------------+------------+---------------------------+
 * |xxxxxxxx|                         |............|xxxxxxxxxxxxxxxxxxxxxxxxxxx|
 * +--------+-------------------------+------------+---------------------------+
 *          ^                         ^            ^
 *        head                       tail       flushed
 *
 * A ring position is a uint16_t made of a generation (see GEN_MASK) and an
 * index (see IDX_MASK). Slots of that ring hold tagged page references. These
 * are made from a generation (see GEN_MASK) and a page reference.
 *
 * A generation is how many times the head wrapped around.
 *
 * These conditions hold:
 *   (uint16_t)(flushed - tail) < FIREHOSE_BUFFER_CHUNK_COUNT
 *   (uint16_t)(head - flushed) < FIREHOSE_BUFFER_CHUNK_COUNT
 * which really means, on the circular buffer, tail <= flushed <= head.
 *
 * Page references span from 1 to (CHUNK_COUNT - 1). 0 is an invalid page
 * (corresponds to the buffer header) and means "unused".
 *
 *
 * - Entries situated between tail and flushed hold references to pages that
 *   the firehose consumer (logd) has flushed, and can be reused.
 *
 * - Entries situated between flushed and head are references to pages waiting
 *   to be flushed.
 *
 * - Entries not situated between tail and head are either slots being modified
 *   or that should be set to Empty. Empty is the 0 page reference associated
 *   with the generation count the head will have the next time it will go over
 *   that slot.
 */
typedef struct firehose_buffer_header_s {
	uint16_t volatile				fbh_mem_ring[FIREHOSE_BUFFER_CHUNK_COUNT];
	uint16_t volatile				fbh_io_ring[FIREHOSE_BUFFER_CHUNK_COUNT];

	firehose_ring_tail_u volatile	fbh_ring_tail OS_ALIGNED(64);
	uint32_t						fbh_spi_version;
	uint16_t volatile				fbh_ring_mem_head OS_ALIGNED(64);
	uint16_t volatile				fbh_ring_io_head OS_ALIGNED(64);
	struct firehose_buffer_bank_s	fbh_bank;
	struct firehose_buffer_stream_s fbh_stream[_firehose_stream_max];

	uint64_t						fbh_uniquepid;
	pid_t							fbh_pid;
	mach_port_t						fbh_logd_port;
	mach_port_t volatile			fbh_sendp[FIREHOSE_BUFFER_NPUSHPORTS];
	mach_port_t						fbh_recvp;

	// past that point fields may be aligned differently between 32 and 64bits
#ifndef KERNEL
	dispatch_unfair_lock_s			fbh_logd_lock;
#define FBH_QUARANTINE_NONE		0
#define FBH_QUARANTINE_PENDING	1
#define FBH_QUARANTINE_STARTED	2
	uint8_t volatile				fbh_quarantined_state;
	bool							fbh_quarantined;
#endif
	uint64_t						fbh_unused[0];
} OS_ALIGNED(FIREHOSE_CHUNK_SIZE) *firehose_buffer_header_t;

union firehose_buffer_u {
	struct firehose_buffer_header_s fb_header;
	struct firehose_chunk_s fb_chunks[FIREHOSE_BUFFER_CHUNK_COUNT];
};

// used to let the compiler pack these values in 1 or 2 registers
typedef struct firehose_tracepoint_query_s {
	uint64_t stamp;
	uint16_t pubsize;
	uint16_t privsize;
	firehose_stream_t stream;
	bool	 is_bank_ok;
	bool	 for_io : 1;
	bool	 quarantined : 1;
	bool	 reliable : 1;
} *firehose_tracepoint_query_t;

#ifndef FIREHOSE_SERVER

firehose_buffer_t
firehose_buffer_create(mach_port_t logd_port, uint64_t unique_pid,
		unsigned long bank_flags);

firehose_tracepoint_t
firehose_buffer_tracepoint_reserve_slow(firehose_buffer_t fb,
		firehose_tracepoint_query_t ask, uint8_t **privptr);

void *
firehose_buffer_get_logging_prefs(firehose_buffer_t fb, size_t *size);

bool
firehose_buffer_should_send_strings(firehose_buffer_t fb);

void
firehose_buffer_update_limits(firehose_buffer_t fb);

void
firehose_buffer_ring_enqueue(firehose_buffer_t fb, firehose_chunk_ref_t ref);

void
firehose_buffer_force_connect(firehose_buffer_t fb);

#endif

#endif // __FIREHOSE_BUFFER_INTERNAL__
