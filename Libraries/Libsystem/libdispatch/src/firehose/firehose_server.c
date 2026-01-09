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

#include <servers/bootstrap.h>
#include <sys/ioctl.h>
#include <sys/ttycom.h>
#include <sys/uio.h>
#include "internal.h"
#include "firehoseServer.h" // MiG
#include "firehose_reply.h" // MiG

#if __has_feature(c_static_assert)
_Static_assert(offsetof(struct firehose_client_s, fc_mem_sent_flushed_pos)
		% 8 == 0, "Make sure atomic fields are properly aligned");
#endif

typedef struct fs_client_queue_s {
	struct firehose_client_s *volatile fs_client_head;
	struct firehose_client_s *volatile fs_client_tail;
} fs_client_queue_s, *fs_client_queue_t;

static struct firehose_server_s {
	mach_port_t			fs_bootstrap_port;
	dispatch_mach_t		fs_mach_channel;
	dispatch_queue_t	fs_snapshot_gate_queue;
	dispatch_queue_t	fs_io_drain_queue;
	dispatch_workloop_t	fs_io_wl;
	dispatch_queue_t	fs_mem_drain_queue;
	firehose_handler_t	fs_handler;

	firehose_snapshot_t fs_snapshot;
	firehose_client_t	fs_kernel_client;
	int					fs_kernel_fd;

	mach_port_t         fs_prefs_cache_entry;
	size_t              fs_prefs_cache_size;
	void               *fs_prefs_cache;

	TAILQ_HEAD(, firehose_client_s) fs_clients;
	os_unfair_lock      fs_clients_lock;
	fs_client_queue_s	fs_queues[4];
	dispatch_source_t	fs_sources[4];
} server_config = {
	.fs_clients = TAILQ_HEAD_INITIALIZER(server_config.fs_clients),
	.fs_clients_lock = OS_UNFAIR_LOCK_INIT,
	.fs_kernel_fd = -1,
};

OS_ALWAYS_INLINE
static inline void
fs_clients_lock(void)
{
	os_unfair_lock_lock_with_options(&server_config.fs_clients_lock,
			OS_UNFAIR_LOCK_DATA_SYNCHRONIZATION);
}

OS_ALWAYS_INLINE
static inline void
fs_clients_unlock(void)
{
	os_unfair_lock_unlock(&server_config.fs_clients_lock);
}

static void firehose_client_cancel(firehose_client_t fc);
static void firehose_client_snapshot_finish(firehose_client_t fc,
		firehose_snapshot_t snapshot, bool for_io);
static void firehose_client_handle_death(void *ctxt);

static const struct mig_subsystem *const firehose_subsystems[] = {
	(mig_subsystem_t)&firehose_server_firehose_subsystem,
};

#pragma mark -
#pragma mark firehose client enqueueing

OS_ALWAYS_INLINE
static inline bool
fs_idx_is_for_io(size_t idx)
{
	return idx & 1;
}

OS_ALWAYS_INLINE
static inline bool
fs_queue_is_for_io(fs_client_queue_t q)
{
	return (q - server_config.fs_queues) & 1;
}

OS_ALWAYS_INLINE
static inline bool
fs_queue_is_for_quarantined(fs_client_queue_t q)
{
	return (q - server_config.fs_queues) & 2;
}

OS_ALWAYS_INLINE
static inline fs_client_queue_t
fs_queue(bool quarantined, bool for_io)
{
	return &server_config.fs_queues[quarantined * 2 + for_io];
}

OS_ALWAYS_INLINE
static inline dispatch_source_t
fs_source(bool quarantined, bool for_io)
{
	return server_config.fs_sources[quarantined * 2 + for_io];
}

OS_ALWAYS_INLINE
static inline void
firehose_client_push(firehose_client_t fc, bool quarantined, bool for_io)
{
	fs_client_queue_t queue = fs_queue(quarantined, for_io);
	if (fc && os_mpsc_push_item(os_mpsc(queue, fs_client),
			fc, fc_next[for_io])) {
		dispatch_source_merge_data(fs_source(quarantined, for_io), 1);
	}
}

OS_ALWAYS_INLINE
static inline bool
firehose_client_wakeup(firehose_client_t fc, bool for_io)
{
	uint16_t canceled_bit = FC_STATE_CANCELED(for_io);
	uint16_t enqueued_bit = FC_STATE_ENQUEUED(for_io);
	uint16_t old_state, new_state;

	os_atomic_rmw_loop(&fc->fc_state, old_state, new_state, relaxed, {
		if (old_state & canceled_bit) {
			os_atomic_rmw_loop_give_up(return false);
		}
		if (old_state & enqueued_bit) {
			os_atomic_rmw_loop_give_up(break);
		}
		new_state = old_state | enqueued_bit;
	});
	firehose_client_push(old_state & enqueued_bit ? NULL : fc,
			fc->fc_quarantined, for_io);
	return true;
}

OS_ALWAYS_INLINE
static inline void
firehose_client_start_cancel(firehose_client_t fc, bool for_io)
{
	uint16_t canceling_bit = FC_STATE_CANCELING(for_io);
	uint16_t canceled_bit = FC_STATE_CANCELED(for_io);
	uint16_t enqueued_bit = FC_STATE_ENQUEUED(for_io);
	uint16_t old_state, new_state;

	os_atomic_rmw_loop(&fc->fc_state, old_state, new_state, relaxed, {
		if (old_state & (canceled_bit | canceling_bit)) {
			os_atomic_rmw_loop_give_up(return);
		}
		new_state = old_state | enqueued_bit | canceling_bit;
	});
	firehose_client_push(old_state & enqueued_bit ? NULL : fc,
			fc->fc_quarantined, for_io);
}

OS_ALWAYS_INLINE
static inline bool
firehose_client_dequeue(firehose_client_t fc, bool for_io)
{
	uint16_t canceling_bit = FC_STATE_CANCELING(for_io);
	uint16_t canceled_bit = FC_STATE_CANCELED(for_io);
	uint16_t enqueued_bit = FC_STATE_ENQUEUED(for_io);
	uint16_t old_state, new_state;

	os_atomic_rmw_loop(&fc->fc_state, old_state, new_state, relaxed, {
		new_state = old_state & ~(canceling_bit | enqueued_bit);
		if (old_state & canceling_bit) {
			new_state |= canceled_bit;
		}
	});

	if (((old_state ^ new_state) & FC_STATE_CANCELED_MASK) &&
			(new_state & FC_STATE_CANCELED_MASK) == FC_STATE_CANCELED_MASK) {
		dispatch_async_f(server_config.fs_io_drain_queue, fc,
				firehose_client_handle_death);
	}
	return !(new_state & canceled_bit);
}

#pragma mark -
#pragma mark firehose client state machine

static void
firehose_client_notify(firehose_client_t fc, mach_port_t reply_port)
{
	firehose_push_reply_t push_reply = {
		.fpr_mem_flushed_pos = os_atomic_load2o(fc, fc_mem_flushed_pos,relaxed),
		.fpr_io_flushed_pos = os_atomic_load2o(fc, fc_io_flushed_pos, relaxed),
	};
	kern_return_t kr;

	firehose_atomic_max2o(fc, fc_mem_sent_flushed_pos,
			push_reply.fpr_mem_flushed_pos, relaxed);
	firehose_atomic_max2o(fc, fc_io_sent_flushed_pos,
			push_reply.fpr_io_flushed_pos, relaxed);

	if (!fc->fc_pid) {
		if (ioctl(server_config.fs_kernel_fd, LOGFLUSHED, &push_reply) < 0) {
			dispatch_assume_zero(errno);
		}
	} else {
		if (reply_port == fc->fc_sendp) {
			kr = firehose_send_push_notify_async(reply_port, push_reply,
					fc->fc_quarantined, 0);
		} else {
			kr = firehose_send_push_reply(reply_port, KERN_SUCCESS, push_reply,
					fc->fc_quarantined);
		}
		if (kr != MACH_SEND_INVALID_DEST) {
			DISPATCH_VERIFY_MIG(kr);
			dispatch_assume_zero(kr);
		}
	}
}

OS_ALWAYS_INLINE
static inline uint16_t
firehose_client_acquire_head(firehose_buffer_t fb, bool for_io)
{
	uint16_t head;
	if (for_io) {
		head = os_atomic_load2o(&fb->fb_header, fbh_ring_io_head, acquire);
	} else {
		head = os_atomic_load2o(&fb->fb_header, fbh_ring_mem_head, acquire);
	}
	return head;
}

OS_NOINLINE OS_COLD
static void
firehose_client_mark_corrupted(firehose_client_t fc, mach_port_t reply_port)
{
	// this client is really confused, do *not* answer to asyncs anymore
	fc->fc_memory_corrupted = true;

	// XXX: do not cancel the data sources or a corrupted client could
	// prevent snapshots from being taken if unlucky with ordering

	if (reply_port) {
		kern_return_t kr = firehose_send_push_reply(reply_port, 0,
				FIREHOSE_PUSH_REPLY_CORRUPTED, false);
		DISPATCH_VERIFY_MIG(kr);
		dispatch_assume_zero(kr);
	}
}

OS_ALWAYS_INLINE
static inline void
firehose_client_snapshot_mark_done(firehose_client_t fc,
		firehose_snapshot_t snapshot, bool for_io)
{
	if (for_io) {
		fc->fc_needs_io_snapshot = false;
	} else {
		fc->fc_needs_mem_snapshot = false;
	}
	dispatch_group_leave(snapshot->fs_group);
}

#define DRAIN_BATCH_SIZE  4
#define FIREHOSE_DRAIN_FOR_IO 0x1
#define FIREHOSE_DRAIN_POLL 0x2

OS_NOINLINE
static void
firehose_client_drain_one(firehose_client_t fc, mach_port_t port, uint32_t flags)
{
	firehose_buffer_t fb = fc->fc_buffer;
	firehose_chunk_t fbc;
	firehose_event_t evt;
	firehose_snapshot_event_t sevt;
	uint16_t volatile *fbh_ring;
	uint16_t flushed, count = 0;
	firehose_chunk_ref_t ref;
	uint16_t client_head, client_flushed, sent_flushed;
	firehose_snapshot_t snapshot = NULL;
	bool for_io = (flags & FIREHOSE_DRAIN_FOR_IO);

	if (for_io) {
		evt = FIREHOSE_EVENT_IO_BUFFER_RECEIVED;
		sevt = FIREHOSE_SNAPSHOT_EVENT_IO_BUFFER;
		_Static_assert(FIREHOSE_EVENT_IO_BUFFER_RECEIVED ==
				FIREHOSE_SNAPSHOT_EVENT_IO_BUFFER, "");
		fbh_ring = fb->fb_header.fbh_io_ring;
		sent_flushed = (uint16_t)fc->fc_io_sent_flushed_pos;
		flushed = (uint16_t)fc->fc_io_flushed_pos;
		if (fc->fc_needs_io_snapshot) snapshot = server_config.fs_snapshot;
	} else {
		evt = FIREHOSE_EVENT_MEM_BUFFER_RECEIVED;
		sevt = FIREHOSE_SNAPSHOT_EVENT_MEM_BUFFER;
		_Static_assert(FIREHOSE_EVENT_MEM_BUFFER_RECEIVED ==
				FIREHOSE_SNAPSHOT_EVENT_MEM_BUFFER, "");
		fbh_ring = fb->fb_header.fbh_mem_ring;
		sent_flushed = (uint16_t)fc->fc_mem_sent_flushed_pos;
		flushed = (uint16_t)fc->fc_mem_flushed_pos;
		if (fc->fc_needs_mem_snapshot) snapshot = server_config.fs_snapshot;
	}

	if (unlikely(fc->fc_memory_corrupted)) {
		goto corrupt;
	}

	client_head = flushed;
	do {
		if ((uint16_t)(flushed + count) == client_head) {
			client_head = firehose_client_acquire_head(fb, for_io);
			if ((uint16_t)(flushed + count) == client_head) {
				break;
			}
			if ((uint16_t)(client_head - sent_flushed) >=
					FIREHOSE_BUFFER_CHUNK_COUNT) {
				goto corrupt;
			}
		}

		// see firehose_buffer_ring_enqueue
		do {
			ref = (flushed + count) & FIREHOSE_RING_POS_IDX_MASK;
			ref = (firehose_chunk_ref_t)os_atomic_load(&fbh_ring[ref], relaxed);
			ref &= FIREHOSE_RING_POS_IDX_MASK;
		} while (!fc->fc_pid && !ref);
		count++;
		if (!ref) {
			_dispatch_debug("Ignoring invalid page reference in ring: %d", ref);
			continue;
		}

		fbc = firehose_buffer_ref_to_chunk(fb, ref);
		firehose_chunk_pos_u fc_pos = fbc->fc_pos;
		if (fc_pos.fcp_stream == firehose_stream_metadata) {
			// serialize with firehose_client_metadata_stream_peek
			os_unfair_lock_lock(&fc->fc_lock);
		}
		server_config.fs_handler(fc, evt, fbc, fc_pos);
		if (unlikely(snapshot)) {
			snapshot->handler(fc, sevt, fbc, fc_pos);
		}
		if (fc_pos.fcp_stream == firehose_stream_metadata) {
			os_unfair_lock_unlock(&fc->fc_lock);
		}
		// clients not using notifications (single threaded) always drain fully
		// because they use all their limit, always
	} while (count < DRAIN_BATCH_SIZE || snapshot);

	if (count) {
		// we don't load the full fbh_ring_tail because that is a 64bit quantity
		// and we only need 16bits from it. and on 32bit arm, there's no way to
		// perform an atomic load of a 64bit quantity on read-only memory.
		if (for_io) {
			os_atomic_add2o(fc, fc_io_flushed_pos, count, relaxed);
			client_flushed = os_atomic_load2o(&fb->fb_header,
				fbh_ring_tail.frp_io_flushed, relaxed);
		} else {
			os_atomic_add2o(fc, fc_mem_flushed_pos, count, relaxed);
			client_flushed = os_atomic_load2o(&fb->fb_header,
				fbh_ring_tail.frp_mem_flushed, relaxed);
		}
		if (!fc->fc_pid) {
			// will fire firehose_client_notify() because port is MACH_PORT_DEAD
			port = fc->fc_sendp;
		} else if (!port && client_flushed == sent_flushed) {
			port = fc->fc_sendp;
		}
	}

	if (unlikely(snapshot)) {
		firehose_client_snapshot_finish(fc, snapshot, for_io);
		firehose_client_snapshot_mark_done(fc, snapshot, for_io);
	}
	if (port) {
		firehose_client_notify(fc, port);
	}
	if (!fc->fc_pid) {
		if (!(flags & FIREHOSE_DRAIN_POLL)) {
			// see firehose_client_kernel_source_handle_event
			dispatch_resume(fc->fc_kernel_source);
		}
	} else {
		if (count >= DRAIN_BATCH_SIZE) {
			// if we hit the drain batch size, the client probably logs a lot
			// and there's more to drain, so optimistically schedule draining
			// again this is cheap since the queue is hot, and is fair for other
			// clients
			firehose_client_wakeup(fc, for_io);
		}
		if (count && server_config.fs_kernel_client) {
			// the kernel is special because it can drop messages, so if we're
			// draining, poll the kernel each time while we're bound to a thread
			firehose_client_drain_one(server_config.fs_kernel_client,
					MACH_PORT_NULL, flags | FIREHOSE_DRAIN_POLL);
		}
	}
	return;

corrupt:
	if (snapshot) {
		firehose_client_snapshot_mark_done(fc, snapshot, for_io);
	}
	firehose_client_mark_corrupted(fc, port);
	// from now on all IO/mem drains depending on `for_io` will be no-op
	// (needs_<for_io>_snapshot: false, memory_corrupted: true). we can safely
	// silence the corresponding source of drain wake-ups.
	if (fc->fc_pid) {
		firehose_client_start_cancel(fc, for_io);
	}
}

static void
firehose_client_drain(void *ctxt)
{
	fs_client_queue_t queue = ctxt;
	bool for_io = fs_queue_is_for_io(queue);
	bool quarantined = fs_queue_is_for_quarantined(queue);
	firehose_client_t fc, fc_next;
	size_t clients = 0;

	while (queue->fs_client_tail) {
		fc = os_mpsc_get_head(os_mpsc(queue, fs_client));
		do {
			fc_next = os_mpsc_pop_head(os_mpsc(queue, fs_client),
					fc, fc_next[for_io]);
			if (firehose_client_dequeue(fc, for_io)) {
				firehose_client_drain_one(fc, MACH_PORT_NULL,
						for_io ? FIREHOSE_DRAIN_FOR_IO : 0);
			}
			// process quarantined clients 4 times as slow as the other ones
			// also reasyncing every 4 clients allows for discovering
			// quarantined suspension faster
			if (++clients == (quarantined ? 1 : 4)) {
				dispatch_source_merge_data(fs_source(quarantined, for_io), 1);
				return;
			}
		} while ((fc = fc_next));
	}
}

OS_NOINLINE
static void
firehose_client_finalize(firehose_client_t fc OS_OBJECT_CONSUMED)
{
	firehose_snapshot_t snapshot = server_config.fs_snapshot;
	firehose_buffer_t fb = fc->fc_buffer;

	dispatch_assert_queue(server_config.fs_io_drain_queue);

	// if a client dies between phase 1 and 2 of starting the snapshot
	// (see firehose_snapshot_start)) there's no handler to call, but the
	// dispatch group has to be adjusted for this client going away.
	if (fc->fc_needs_io_snapshot) {
		dispatch_group_leave(snapshot->fs_group);
		fc->fc_needs_io_snapshot = false;
	}
	if (fc->fc_needs_mem_snapshot) {
		dispatch_group_leave(snapshot->fs_group);
		fc->fc_needs_mem_snapshot = false;
	}
	if (fc->fc_memory_corrupted) {
		server_config.fs_handler(fc, FIREHOSE_EVENT_CLIENT_CORRUPTED,
				&fb->fb_chunks[0], (firehose_chunk_pos_u){ .fcp_pos = 0 });
	}
	server_config.fs_handler(fc, FIREHOSE_EVENT_CLIENT_DIED, NULL,
			(firehose_chunk_pos_u){ .fcp_pos = 0 });

	fs_clients_lock();
	TAILQ_REMOVE(&server_config.fs_clients, fc, fc_entry);
	fs_clients_unlock();

	for (int i = 0; i < FIREHOSE_BUFFER_NPUSHPORTS; i++) {
		dispatch_release(fc->fc_mach_channel[i]);
		fc->fc_mach_channel[i] = NULL;
	}
	fc->fc_entry.tqe_next = DISPATCH_OBJECT_LISTLESS;
	fc->fc_entry.tqe_prev = DISPATCH_OBJECT_LISTLESS;
	_os_object_release_without_xref_dispose(&fc->fc_as_os_object);
}

OS_NOINLINE
static void
firehose_client_handle_death(void *ctxt)
{
	firehose_client_t fc = ctxt;
	firehose_buffer_t fb = fc->fc_buffer;
	firehose_buffer_header_t fbh = &fb->fb_header;
	uint64_t mem_bitmap = 0, bitmap;

	if (fc->fc_memory_corrupted) {
		return firehose_client_finalize(fc);
	}

	dispatch_assert_queue(server_config.fs_io_drain_queue);

	// acquire to match release barriers from threads that died
	os_atomic_thread_fence(acquire);

	bitmap = fbh->fbh_bank.fbb_bitmap & ~1ULL;
	for (int for_io = 0; for_io < 2; for_io++) {
		uint16_t volatile *fbh_ring;
		uint16_t tail, flushed;

		if (for_io) {
			fbh_ring = fbh->fbh_io_ring;
			tail = fbh->fbh_ring_tail.frp_io_tail;
			flushed = (uint16_t)fc->fc_io_flushed_pos;
		} else {
			fbh_ring = fbh->fbh_mem_ring;
			tail = fbh->fbh_ring_tail.frp_mem_tail;
			flushed = (uint16_t)fc->fc_mem_flushed_pos;
		}
		if ((uint16_t)(flushed - tail) >= FIREHOSE_BUFFER_CHUNK_COUNT) {
			fc->fc_memory_corrupted = true;
			return firehose_client_finalize(fc);
		}

		// remove the pages that we flushed already from the bitmap
		for (; tail != flushed; tail++) {
			uint16_t ring_pos = tail & FIREHOSE_RING_POS_IDX_MASK;
			firehose_chunk_ref_t ref =
					fbh_ring[ring_pos] & FIREHOSE_RING_POS_IDX_MASK;

			bitmap &= ~(1ULL << ref);
		}
	}

	firehose_snapshot_t snapshot = server_config.fs_snapshot;

	// Then look at all the allocated pages not seen in the ring
	while (bitmap) {
		firehose_chunk_ref_t ref = firehose_bitmap_first_set(bitmap);
		firehose_chunk_t fbc = firehose_buffer_ref_to_chunk(fb, ref);
		firehose_chunk_pos_u fc_pos = fbc->fc_pos;
		uint16_t fbc_length = fc_pos.fcp_next_entry_offs;

		bitmap &= ~(1ULL << ref);
		if (fbc->fc_start + fbc_length <= fbc->fc_data) {
			// this page has its "recycle-requeue" done, but hasn't gone
			// through "recycle-reuse", or it has no data, ditch it
			continue;
		}
		if (!((firehose_tracepoint_t)fbc->fc_data)->ft_length) {
			// this thing has data, but the first tracepoint is unreadable
			// so also just ditch it
			continue;
		}
		if (!fc_pos.fcp_flag_io) {
			mem_bitmap |= 1ULL << ref;
			continue;
		}
		server_config.fs_handler(fc, FIREHOSE_EVENT_IO_BUFFER_RECEIVED, fbc,
				fc_pos);
		if (fc->fc_needs_io_snapshot) {
			snapshot->handler(fc, FIREHOSE_SNAPSHOT_EVENT_IO_BUFFER, fbc,
					fc_pos);
		}
	}

	if (!mem_bitmap) {
		return firehose_client_finalize(fc);
	}

	dispatch_async(server_config.fs_mem_drain_queue, ^{
		uint64_t mem_bitmap_copy = mem_bitmap;

		while (mem_bitmap_copy) {
			firehose_chunk_ref_t ref = firehose_bitmap_first_set(mem_bitmap_copy);
			firehose_chunk_t fbc = firehose_buffer_ref_to_chunk(fb, ref);
			firehose_chunk_pos_u fc_pos = fbc->fc_pos;

			mem_bitmap_copy &= ~(1ULL << ref);
			server_config.fs_handler(fc, FIREHOSE_EVENT_MEM_BUFFER_RECEIVED,
					fbc, fc_pos);
			if (fc->fc_needs_mem_snapshot) {
				snapshot->handler(fc, FIREHOSE_SNAPSHOT_EVENT_MEM_BUFFER,
						fbc, fc_pos);
			}
		}

		dispatch_async_f(server_config.fs_io_drain_queue, fc,
				(dispatch_function_t)firehose_client_finalize);
	});
}

static void
firehose_client_handle_mach_event(void *ctx, dispatch_mach_reason_t reason,
		dispatch_mach_msg_t dmsg, mach_error_t error OS_UNUSED)
{
	mach_msg_header_t *msg_hdr = NULL;
	firehose_client_t fc = ctx;
	mach_port_t port;

	switch (reason) {
	case DISPATCH_MACH_MESSAGE_RECEIVED:
		if (dispatch_mach_mig_demux(fc, firehose_subsystems,
				countof(firehose_subsystems), dmsg)) {
			break;
		}

		mach_msg_destroy(dispatch_mach_msg_get_msg(dmsg, NULL));
		break;

	case DISPATCH_MACH_NO_SENDERS:
		_dispatch_debug("FIREHOSE NO_SENDERS (unique_pid: 0x%llx)",
				firehose_client_get_unique_pid(fc, NULL));
		for (int i = 0; i < FIREHOSE_BUFFER_NPUSHPORTS; i++) {
			dispatch_mach_cancel(fc->fc_mach_channel[i]);
		}
		break;

	case DISPATCH_MACH_DISCONNECTED:
		msg_hdr = dispatch_mach_msg_get_msg(dmsg, NULL);
		port = msg_hdr->msgh_local_port;
		if (MACH_PORT_VALID(port)) {
			int i;
			for (i = 0; i < FIREHOSE_BUFFER_NPUSHPORTS; i++) {
				if (fc->fc_recvp[i] == port) {
					break;
				}
			}
			if (i == FIREHOSE_BUFFER_NPUSHPORTS) {
				DISPATCH_INTERNAL_CRASH(port, "Unknown recv-right");
			}
			firehose_mach_port_recv_dispose(fc->fc_recvp[i], &fc->fc_recvp[i]);
			fc->fc_recvp[i] = MACH_PORT_NULL;
		}
		break;

	case DISPATCH_MACH_CANCELED:
		if (!_os_atomic_refcnt_sub2o(fc, fc_mach_channel_refcnt, 1)) {
			_os_atomic_refcnt_dispose_barrier2o(fc, fc_mach_channel_refcnt);

			firehose_mach_port_send_release(fc->fc_sendp);
			fc->fc_sendp = MACH_PORT_NULL;
			for (int i = 0; i < FIREHOSE_BUFFER_NPUSHPORTS; i++) {
				if (MACH_PORT_VALID(fc->fc_recvp[i])) {
					DISPATCH_INTERNAL_CRASH(fc->fc_recvp[i], "recv-right leak");
				}
			}

			firehose_client_cancel(fc);
		}
		break;
	default:
		break;
	}
}

#if !TARGET_OS_SIMULATOR
static void
firehose_client_kernel_source_handle_event(void *ctxt)
{
	firehose_client_t fc = ctxt;

	// resumed in firehose_client_drain for both memory and I/O
	dispatch_suspend(fc->fc_kernel_source);
	dispatch_suspend(fc->fc_kernel_source);
	firehose_client_wakeup(fc, false);
	firehose_client_wakeup(fc, true);
}
#endif

static inline void
firehose_client_resume(firehose_client_t fc,
		const struct firehose_client_connected_info_s *fcci)
{
	dispatch_assert_queue(server_config.fs_mem_drain_queue);

	fs_clients_lock();
	TAILQ_INSERT_TAIL(&server_config.fs_clients, fc, fc_entry);
	fs_clients_unlock();

	server_config.fs_handler(fc, FIREHOSE_EVENT_CLIENT_CONNECTED, (void *)fcci,
			(firehose_chunk_pos_u){ .fcp_pos = 0 });
	if (!fc->fc_pid) {
		dispatch_activate(fc->fc_kernel_source);
	} else {
		for (int i = 0; i < FIREHOSE_BUFFER_NPUSHPORTS; i++) {
			dispatch_mach_connect(fc->fc_mach_channel[i],
					fc->fc_recvp[i], MACH_PORT_NULL, NULL);
		}
	}
}

static void
firehose_client_cancel(firehose_client_t fc)
{
	_dispatch_debug("client died (unique_pid: 0x%llx",
			firehose_client_get_unique_pid(fc, NULL));

	dispatch_assert(fc->fc_sendp == MACH_PORT_NULL);
	for (int i = 0; i < FIREHOSE_BUFFER_NPUSHPORTS; i++) {
		dispatch_assert(fc->fc_recvp[i] == MACH_PORT_NULL);
	}
	firehose_client_start_cancel(fc, false);
	firehose_client_start_cancel(fc, true);
}

static firehose_client_t
_firehose_client_create(firehose_buffer_t fb)
{
	firehose_client_t fc;

	fc = (firehose_client_t)_os_object_alloc_realized(FIREHOSE_CLIENT_CLASS,
			sizeof(struct firehose_client_s));
	fc->fc_buffer = fb;
	fc->fc_mem_flushed_pos = fb->fb_header.fbh_bank.fbb_mem_flushed;
	fc->fc_mem_sent_flushed_pos = fc->fc_mem_flushed_pos;
	fc->fc_io_flushed_pos = fb->fb_header.fbh_bank.fbb_io_flushed;
	fc->fc_io_sent_flushed_pos = fc->fc_io_flushed_pos;
	return fc;
}

#pragma pack(4)
typedef struct firehose_token_s {
	uid_t auid;
	uid_t euid;
	gid_t egid;
	uid_t ruid;
	gid_t rgid;
	pid_t pid;
	au_asid_t asid;
	dev_t execcnt;
} *firehose_token_t;
#pragma pack()

static firehose_client_t
firehose_client_create(firehose_buffer_t fb, firehose_token_t token,
		mach_port_t comm_mem_recvp, mach_port_t comm_io_recvp,
		mach_port_t comm_sendp)
{
	uint64_t unique_pid = fb->fb_header.fbh_uniquepid;
	firehose_client_t fc = _firehose_client_create(fb);
	dispatch_mach_t dm;

	fc->fc_pid = token->pid ? token->pid : ~0;
	fc->fc_euid = token->euid;
	fc->fc_pidversion = token->execcnt;

	_dispatch_debug("FIREHOSE_REGISTER (unique_pid: 0x%llx)", unique_pid);
	mach_port_t recvp[] = { comm_mem_recvp, comm_io_recvp };
	dispatch_queue_t fsqs[] = {
		server_config.fs_mem_drain_queue,
		server_config.fs_io_drain_queue
	};
	fc->fc_mach_channel_refcnt = FIREHOSE_BUFFER_NPUSHPORTS;
	for (int i = 0; i < FIREHOSE_BUFFER_NPUSHPORTS; i++) {
		fc->fc_recvp[i] = recvp[i];
		firehose_mach_port_guard(fc->fc_recvp[i], true, &fc->fc_recvp[i]);
		dm = dispatch_mach_create_f("com.apple.firehose.peer", fsqs[i], fc,
				firehose_client_handle_mach_event);
		fc->fc_mach_channel[i] = dm;
	}

	fc->fc_sendp = comm_sendp;
	return fc;
}

static void
firehose_kernel_client_create(void)
{
#if !TARGET_OS_SIMULATOR
	struct firehose_server_s *fs = &server_config;
	firehose_buffer_map_info_t fb_map;
	firehose_client_t fc;
	dispatch_source_t ds;
	int fd;

	while ((fd = open("/dev/oslog", O_RDWR)) < 0) {
		if (errno == EINTR) {
			continue;
		}
		if (errno == ENOENT) {
			return;
		}
		DISPATCH_INTERNAL_CRASH(errno, "Unable to open /dev/oslog");
	}

	while (ioctl(fd, LOGBUFFERMAP, &fb_map) < 0) {
		if (errno == EINTR) {
			continue;
		}
		DISPATCH_INTERNAL_CRASH(errno, "Unable to map kernel buffer");
	}
	if ((fb_map.fbmi_size < FIREHOSE_BUFFER_KERNEL_MIN_CHUNK_COUNT * FIREHOSE_CHUNK_SIZE) ||
		(fb_map.fbmi_size > FIREHOSE_BUFFER_KERNEL_MAX_CHUNK_COUNT * FIREHOSE_CHUNK_SIZE)) {
		DISPATCH_INTERNAL_CRASH(fb_map.fbmi_size, "Unexpected kernel buffer size");
	}

	fc = _firehose_client_create((firehose_buffer_t)(uintptr_t)fb_map.fbmi_addr);
	ds = dispatch_source_create(DISPATCH_SOURCE_TYPE_READ, (uintptr_t)fd, 0,
			fs->fs_mem_drain_queue);
	dispatch_set_qos_class_floor(ds, QOS_CLASS_USER_INITIATED, 0);
	dispatch_set_context(ds, fc);
	dispatch_source_set_event_handler_f(ds,
			firehose_client_kernel_source_handle_event);
	fc->fc_kernel_source = ds;
	fc->fc_sendp = MACH_PORT_DEAD; // causes drain() to call notify

	fs->fs_kernel_fd = fd;
	fs->fs_kernel_client = fc;
#endif
}

void
_firehose_client_dispose(firehose_client_t fc)
{
	vm_deallocate(mach_task_self(), (vm_address_t)fc->fc_buffer,
			sizeof(*fc->fc_buffer));
	fc->fc_buffer = NULL;
	server_config.fs_handler(fc, FIREHOSE_EVENT_CLIENT_FINALIZE, NULL,
			(firehose_chunk_pos_u){ .fcp_pos = 0 });
}

uint64_t
firehose_client_get_unique_pid(firehose_client_t fc, pid_t *pid_out)
{
	firehose_buffer_header_t fbh = &fc->fc_buffer->fb_header;
	if (pid_out) *pid_out = fc->fc_pid;
	if (!fc->fc_pid) return 0;
	return fbh->fbh_uniquepid ? fbh->fbh_uniquepid : ~0ull;
}

uid_t
firehose_client_get_euid(firehose_client_t fc)
{
	return fc->fc_euid;
}

int
firehose_client_get_pid_version(firehose_client_t fc)
{
	return fc->fc_pidversion;
}

void *
firehose_client_get_metadata_buffer(firehose_client_t client, size_t *size)
{
	firehose_buffer_header_t fbh = &client->fc_buffer->fb_header;

	*size = FIREHOSE_BUFFER_LIBTRACE_HEADER_SIZE;
	return (void *)((uintptr_t)(fbh + 1) - *size);
}

void *
firehose_client_get_context(firehose_client_t fc)
{
	return os_atomic_load2o(fc, fc_ctxt, relaxed);
}

void
firehose_client_set_strings_cached(firehose_client_t fc)
{
	fc->fc_strings_cached = true;
}

void *
firehose_client_set_context(firehose_client_t fc, void *ctxt)
{
	return os_atomic_xchg2o(fc, fc_ctxt, ctxt, relaxed);
}

void
firehose_client_initiate_quarantine(firehose_client_t fc)
{
	fc->fc_quarantined = true;
}

#pragma mark -
#pragma mark firehose server

static void
firehose_server_handle_mach_event(void *ctx,
		dispatch_mach_reason_t reason, dispatch_mach_msg_t dmsg,
		mach_error_t error OS_UNUSED)
{
	if (reason == DISPATCH_MACH_MESSAGE_RECEIVED) {
		if (!dispatch_mach_mig_demux(ctx, firehose_subsystems,
				countof(firehose_subsystems), dmsg)) {
			mach_msg_destroy(dispatch_mach_msg_get_msg(dmsg, NULL));
		}
	}
}

void
firehose_server_init(mach_port_t comm_port, firehose_handler_t handler)
{
	struct firehose_server_s *fs = &server_config;
	dispatch_queue_attr_t attr = DISPATCH_QUEUE_SERIAL_WITH_AUTORELEASE_POOL;
	dispatch_queue_attr_t attr_inactive =
			dispatch_queue_attr_make_initially_inactive(attr);
	dispatch_mach_t dm;
	dispatch_source_t ds;

	// just reference the string so that it's captured
	(void)os_atomic_load(&__libfirehose_serverVersionString[0], relaxed);

	fs->fs_snapshot_gate_queue = dispatch_queue_create_with_target(
			"com.apple.firehose.snapshot-gate", attr, NULL);

	fs->fs_io_wl = dispatch_workloop_create_inactive("com.apple.firehose.io-wl");
	dispatch_set_qos_class_fallback(fs->fs_io_wl, QOS_CLASS_UTILITY);
	dispatch_activate(fs->fs_io_wl);

	fs->fs_io_drain_queue = dispatch_queue_create_with_target(
			"com.apple.firehose.drain-io", attr, (dispatch_queue_t)fs->fs_io_wl);

	fs->fs_mem_drain_queue = dispatch_queue_create_with_target(
			"com.apple.firehose.drain-mem", attr_inactive, NULL);
	dispatch_set_qos_class_fallback(fs->fs_mem_drain_queue, QOS_CLASS_UTILITY);
	dispatch_activate(fs->fs_mem_drain_queue);

	dm = dispatch_mach_create_f("com.apple.firehose.listener",
			fs->fs_mem_drain_queue, NULL, firehose_server_handle_mach_event);
	fs->fs_bootstrap_port = comm_port;
	fs->fs_mach_channel = dm;
	fs->fs_handler = _Block_copy(handler);
	firehose_kernel_client_create();

	for (size_t i = 0; i < countof(fs->fs_sources); i++) {
		ds = dispatch_source_create(DISPATCH_SOURCE_TYPE_DATA_OR, 0, 0,
				fs_idx_is_for_io(i) ? server_config.fs_io_drain_queue :
				server_config.fs_mem_drain_queue);
		dispatch_set_context(ds, &fs->fs_queues[i]);
		dispatch_source_set_event_handler_f(ds, firehose_client_drain);
		fs->fs_sources[i] = ds;
	}
}

void
firehose_server_assert_spi_version(uint32_t spi_version)
{
	if (spi_version != OS_FIREHOSE_SPI_VERSION) {
		DISPATCH_CLIENT_CRASH(spi_version, "firehose server version mismatch ("
				OS_STRINGIFY(OS_FIREHOSE_SPI_VERSION) ")");
	}
	if (_firehose_spi_version != OS_FIREHOSE_SPI_VERSION) {
		DISPATCH_CLIENT_CRASH(_firehose_spi_version,
				"firehose libdispatch version mismatch ("
				OS_STRINGIFY(OS_FIREHOSE_SPI_VERSION) ")");

	}
}

bool
firehose_server_has_ever_flushed_pages(void)
{
	// Use the IO pages flushed count from the kernel client as an
	// approximation for whether the firehose has ever flushed pages during
	// this boot. logd uses this detect the first time it starts after a
	// fresh boot.
	firehose_client_t fhc = server_config.fs_kernel_client;
	return !fhc || fhc->fc_io_flushed_pos > 0;
}

void
firehose_server_resume(void)
{
	struct firehose_server_s *fs = &server_config;

	if (fs->fs_kernel_client) {
		dispatch_async(fs->fs_mem_drain_queue, ^{
			struct firehose_client_connected_info_s fcci = {
				.fcci_version = FIREHOSE_CLIENT_CONNECTED_INFO_VERSION,
			};
			firehose_client_resume(fs->fs_kernel_client, &fcci);
		});
	}
	dispatch_mach_connect(fs->fs_mach_channel, fs->fs_bootstrap_port,
			MACH_PORT_NULL, NULL);
	for (size_t i = 0; i < countof(fs->fs_sources); i++) {
		dispatch_activate(fs->fs_sources[i]);
	}
}

void
firehose_server_cancel(void)
{
	firehose_client_t fc;

	dispatch_mach_cancel(server_config.fs_mach_channel);

	fs_clients_lock();
	TAILQ_FOREACH(fc, &server_config.fs_clients, fc_entry) {
		if (fc->fc_pid) {
			for (int i = 0; i < FIREHOSE_BUFFER_NPUSHPORTS; i++) {
				dispatch_mach_cancel(fc->fc_mach_channel[i]);
			}
		}
	}
	fs_clients_unlock();
}

void
firehose_server_set_logging_prefs(void *pointer, size_t length, os_block_t block)
{
	dispatch_async(server_config.fs_mem_drain_queue, ^{
		kern_return_t kr;
		memory_object_size_t size = (memory_object_size_t)length;
		if (server_config.fs_prefs_cache_entry) {
			kr = mach_port_deallocate(mach_task_self(),
					server_config.fs_prefs_cache_entry);
			DISPATCH_VERIFY_MIG(kr);
			dispatch_assume_zero(kr);
		}
		if (server_config.fs_prefs_cache) {
			munmap(server_config.fs_prefs_cache,
					server_config.fs_prefs_cache_size);
		}

		server_config.fs_prefs_cache = pointer;
		server_config.fs_prefs_cache_size = length;
		server_config.fs_prefs_cache_entry = MACH_PORT_NULL;
		if (pointer) {
			kr = mach_make_memory_entry_64(mach_task_self(), &size,
					(mach_vm_address_t)pointer, VM_PROT_READ | MAP_MEM_VM_SHARE,
					&server_config.fs_prefs_cache_entry, MACH_PORT_NULL);
			DISPATCH_VERIFY_MIG(kr);
			dispatch_assume_zero(kr);
		}
		if (block) block();
	});
}

dispatch_queue_t
firehose_server_copy_queue(firehose_server_queue_t which)
{
	dispatch_queue_t dq;
	switch (which) {
	case FIREHOSE_SERVER_QUEUE_IO:
		dq = server_config.fs_io_drain_queue;
		break;
	case FIREHOSE_SERVER_QUEUE_MEMORY:
		dq = server_config.fs_mem_drain_queue;
		break;
	case FIREHOSE_SERVER_QUEUE_IO_WL:
		dq = (dispatch_queue_t)server_config.fs_io_wl;
		break;
	default:
		DISPATCH_INTERNAL_CRASH(which, "Invalid firehose server queue type");
	}
	dispatch_retain(dq);
	return dq;
}

void
firehose_server_quarantined_suspend(firehose_server_queue_t which)
{
	switch (which) {
	case FIREHOSE_SERVER_QUEUE_IO:
		dispatch_suspend(fs_source(true, true));
		break;
	case FIREHOSE_SERVER_QUEUE_MEMORY:
		dispatch_suspend(fs_source(true, false));
		break;
	default:
		DISPATCH_INTERNAL_CRASH(which, "Invalid firehose server queue type");
	}
}

void
firehose_server_quarantined_resume(firehose_server_queue_t which)
{
	switch (which) {
	case FIREHOSE_SERVER_QUEUE_IO:
		dispatch_resume(fs_source(true, true));
		break;
	case FIREHOSE_SERVER_QUEUE_MEMORY:
		dispatch_resume(fs_source(true, false));
		break;
	default:
		DISPATCH_INTERNAL_CRASH(which, "Invalid firehose server queue type");
	}
}


#pragma mark -
#pragma mark firehose snapshot and peeking

void
firehose_client_metadata_stream_peek(firehose_client_t fc,
		OS_UNUSED firehose_event_t context, bool (^peek_should_start)(void),
		bool (^peek)(firehose_chunk_t fbc))
{
	os_unfair_lock_lock(&fc->fc_lock);

	if (peek_should_start && peek_should_start()) {
		firehose_buffer_t fb = fc->fc_buffer;
		firehose_buffer_header_t fbh = &fb->fb_header;
		uint64_t bitmap = fbh->fbh_bank.fbb_metadata_bitmap;

		while (bitmap) {
			firehose_chunk_ref_t ref = firehose_bitmap_first_set(bitmap);
			firehose_chunk_t fbc = firehose_buffer_ref_to_chunk(fb, ref);
			uint16_t fbc_length = fbc->fc_pos.fcp_next_entry_offs;

			bitmap &= ~(1ULL << ref);
			if (fbc->fc_start + fbc_length <= fbc->fc_data) {
				// this page has its "recycle-requeue" done, but hasn't gone
				// through "recycle-reuse", or it has no data, ditch it
				continue;
			}
			if (!((firehose_tracepoint_t)fbc->fc_data)->ft_length) {
				// this thing has data, but the first tracepoint is unreadable
				// so also just ditch it
				continue;
			}
			if (fbc->fc_pos.fcp_stream != firehose_stream_metadata) {
				continue;
			}
			if (!peek(fbc)) {
				break;
			}
		}
	}

	os_unfair_lock_unlock(&fc->fc_lock);
}

OS_NOINLINE OS_COLD
static void
firehose_client_snapshot_finish(firehose_client_t fc,
		firehose_snapshot_t snapshot, bool for_io)
{
	firehose_buffer_t fb = fc->fc_buffer;
	firehose_buffer_header_t fbh = &fb->fb_header;
	firehose_snapshot_event_t evt;
	uint16_t volatile *fbh_ring;
	uint16_t tail, flushed;
	uint64_t bitmap;

	bitmap = ~1ULL;

	if (for_io) {
		fbh_ring = fbh->fbh_io_ring;
		tail = fbh->fbh_ring_tail.frp_io_tail;
		flushed = (uint16_t)fc->fc_io_flushed_pos;
		evt = FIREHOSE_SNAPSHOT_EVENT_IO_BUFFER;
	} else {
		fbh_ring = fbh->fbh_mem_ring;
		tail = fbh->fbh_ring_tail.frp_mem_tail;
		flushed = (uint16_t)fc->fc_mem_flushed_pos;
		evt = FIREHOSE_SNAPSHOT_EVENT_MEM_BUFFER;
	}
	if ((uint16_t)(flushed - tail) >= FIREHOSE_BUFFER_CHUNK_COUNT) {
		fc->fc_memory_corrupted = true;
		return;
	}

	// remove the pages that we flushed already from the bitmap
	for (; tail != flushed; tail++) {
		uint16_t idx = tail & FIREHOSE_RING_POS_IDX_MASK;
		firehose_chunk_ref_t ref = fbh_ring[idx] & FIREHOSE_RING_POS_IDX_MASK;

		bitmap &= ~(1ULL << ref);
	}

	// Remove pages that are free by AND-ing with the allocating bitmap.
	// The load of fbb_bitmap may not be atomic, but it's ok because bits
	// being flipped are pages we don't care about snapshotting. The worst thing
	// that can happen is that we go peek at an unmapped page and we fault it in
	bitmap &= fbh->fbh_bank.fbb_bitmap;

	// Then look at all the allocated pages not seen in the ring
	while (bitmap) {
		firehose_chunk_ref_t ref = firehose_bitmap_first_set(bitmap);
		firehose_chunk_t fbc = firehose_buffer_ref_to_chunk(fb, ref);
		firehose_chunk_pos_u fc_pos = fbc->fc_pos;
		uint16_t fbc_length = fc_pos.fcp_next_entry_offs;

		bitmap &= ~(1ULL << ref);
		if (fbc->fc_start + fbc_length <= fbc->fc_data) {
			// this page has its "recycle-requeue" done, but hasn't gone
			// through "recycle-reuse", or it has no data, ditch it
			continue;
		}
		if (!((firehose_tracepoint_t)fbc->fc_data)->ft_length) {
			// this thing has data, but the first tracepoint is unreadable
			// so also just ditch it
			continue;
		}
		if (fc_pos.fcp_flag_io != for_io) {
			continue;
		}
		snapshot->handler(fc, evt, fbc, fc_pos);
	}
}

static void
firehose_snapshot_tickle_clients(firehose_snapshot_t fs, bool for_io)
{
	firehose_client_t fc;
	uint32_t n = 0;

	fs_clients_lock();
	TAILQ_FOREACH(fc, &server_config.fs_clients, fc_entry) {
		if (unlikely(fc->fc_memory_corrupted)) {
			continue;
		}
		if (!fc->fc_pid) {
#if TARGET_OS_SIMULATOR
			continue;
#endif
		} else if (!firehose_client_wakeup(fc, for_io)) {
			continue;
		}
		n++;
		if (for_io) {
			fc->fc_needs_io_snapshot = true;
		} else {
			fc->fc_needs_mem_snapshot = true;
		}
	}
	fs_clients_unlock();

	// cheating: equivalent to dispatch_group_enter() n times
	// without the acquire barriers that we don't need
	if (n) {
		os_atomic_sub2o(fs->fs_group, dg_bits,
				n * DISPATCH_GROUP_VALUE_INTERVAL, relaxed);
	}
}

static void
firehose_snapshot_finish(void *ctxt)
{
	firehose_snapshot_t fs = ctxt;

	fs->handler(NULL, FIREHOSE_SNAPSHOT_EVENT_COMPLETE, NULL,
			(firehose_chunk_pos_u){ .fcp_pos = 0 });
	server_config.fs_snapshot = NULL;

	dispatch_release(fs->fs_group);
	Block_release(fs->handler);
	free(fs);

	// resume the snapshot gate queue to maybe handle the next snapshot
	dispatch_resume(server_config.fs_snapshot_gate_queue);
}

static void
firehose_snapshot_gate(void *ctxt)
{
	firehose_snapshot_t fs = ctxt;

	// prevent other snapshots from running until done

	dispatch_suspend(server_config.fs_snapshot_gate_queue);

	server_config.fs_snapshot = fs;
	dispatch_group_async(fs->fs_group, server_config.fs_mem_drain_queue, ^{
		// start the fs_mem_snapshot, this is what triggers the snapshot
		// logic from _drain() or handle_death()
		fs->handler(NULL, FIREHOSE_SNAPSHOT_EVENT_MEM_START, NULL,
				(firehose_chunk_pos_u){ .fcp_pos = 0 });
		firehose_snapshot_tickle_clients(fs, false);

		dispatch_group_async(fs->fs_group, server_config.fs_io_drain_queue, ^{
			// start the fs_io_snapshot, this is what triggers the snapshot
			// logic from _drain() or handle_death()
			// 29868879: must always happen after the memory snapshot started
			fs->handler(NULL, FIREHOSE_SNAPSHOT_EVENT_IO_START, NULL,
					(firehose_chunk_pos_u){ .fcp_pos = 0 });
			firehose_snapshot_tickle_clients(fs, true);

#if !TARGET_OS_SIMULATOR
			if (server_config.fs_kernel_client) {
				firehose_client_kernel_source_handle_event(
						server_config.fs_kernel_client);
			}
#endif
		});
	});

	dispatch_group_notify_f(fs->fs_group, server_config.fs_io_drain_queue,
			fs, firehose_snapshot_finish);
}

void
firehose_snapshot(firehose_snapshot_handler_t handler)
{
	firehose_snapshot_t snapshot = malloc(sizeof(struct firehose_snapshot_s));

	snapshot->handler = Block_copy(handler);
	snapshot->fs_group = dispatch_group_create();

	dispatch_async_f(server_config.fs_snapshot_gate_queue, snapshot,
			firehose_snapshot_gate);
}

#pragma mark -
#pragma mark MiG handler routines

kern_return_t
firehose_server_register(mach_port_t server_port OS_UNUSED,
		mach_port_t mem_port, mach_vm_size_t mem_size,
		mach_port_t comm_mem_recvp, mach_port_t comm_io_recvp,
		mach_port_t comm_sendp,
		mach_port_t extra_info_port, mach_vm_size_t extra_info_size,
		audit_token_t atoken)
{
	mach_vm_address_t base_addr = 0;
	firehose_client_t fc = NULL;
	kern_return_t kr;
	struct firehose_client_connected_info_s fcci = {
		.fcci_version = FIREHOSE_CLIENT_CONNECTED_INFO_VERSION,
	};

	fc = dispatch_mach_mig_demux_get_context();
	if (fc != NULL) {
		return KERN_FAILURE;
	}

	if (mem_size != sizeof(union firehose_buffer_u)) {
		return KERN_INVALID_VALUE;
	}

	/* Map the memory handle into the server address space */
	kr = mach_vm_map(mach_task_self(), &base_addr, mem_size, 0,
			VM_FLAGS_ANYWHERE, mem_port, 0, FALSE,
			VM_PROT_READ, VM_PROT_READ, VM_INHERIT_NONE);
	DISPATCH_VERIFY_MIG(kr);
	if (dispatch_assume_zero(kr)) {
		return KERN_NO_SPACE;
	}

	if (extra_info_port) {
		if (extra_info_size) {
			mach_vm_address_t addr = 0;
			kr = mach_vm_map(mach_task_self(), &addr, extra_info_size, 0,
					VM_FLAGS_ANYWHERE, extra_info_port, 0, TRUE,
					VM_PROT_READ, VM_PROT_READ, VM_INHERIT_NONE);
			if (dispatch_assume_zero(kr)) {
				mach_vm_deallocate(mach_task_self(), base_addr, mem_size);
				return KERN_NO_SPACE;
			}
			fcci.fcci_data = (void *)(uintptr_t)addr;
			fcci.fcci_size = (size_t)extra_info_size;
		}
		firehose_mach_port_send_release(extra_info_port);
	}

	firehose_mach_port_send_release(mem_port);

	fc = firehose_client_create((firehose_buffer_t)base_addr,
			(firehose_token_t)&atoken, comm_mem_recvp, comm_io_recvp,
			comm_sendp);
	/*
	 * Request a no senders notification for the memory channel.
	 * That should indicate the client going away.
	 */
	dispatch_mach_request_no_senders(
			fc->fc_mach_channel[FIREHOSE_BUFFER_PUSHPORT_MEM]);
	firehose_client_resume(fc, &fcci);

	if (fcci.fcci_size) {
		vm_deallocate(mach_task_self(), (vm_address_t)fcci.fcci_data,
				fcci.fcci_size);
	}

	return KERN_SUCCESS;
}

kern_return_t
firehose_server_push_async(mach_port_t server_port,
		qos_class_t qos DISPATCH_UNUSED)
{
	firehose_client_t fc = dispatch_mach_mig_demux_get_context();

	if (fc == NULL) {
		return KERN_FAILURE;
	}

	bool for_io = (server_port == fc->fc_recvp[FIREHOSE_BUFFER_PUSHPORT_IO]);

	_dispatch_debug("FIREHOSE_PUSH_ASYNC (unique_pid %llx)",
			firehose_client_get_unique_pid(fc, NULL));
	if (likely(!fc->fc_memory_corrupted)) {
		firehose_client_wakeup(fc, for_io);
	}
	return KERN_SUCCESS;
}

kern_return_t
firehose_server_push_and_wait(mach_port_t server_port,
		mach_port_t reply_port, firehose_push_reply_t *push_reply OS_UNUSED,
		boolean_t *quarantinedOut OS_UNUSED)
{
	firehose_client_t fc = dispatch_mach_mig_demux_get_context();

	if (fc == NULL) {
		return KERN_FAILURE;
	}

	bool for_io = (server_port == fc->fc_recvp[FIREHOSE_BUFFER_PUSHPORT_IO]);

	_dispatch_debug("FIREHOSE_PUSH (unique_pid %llx)",
			firehose_client_get_unique_pid(fc, NULL));

	if (unlikely(fc->fc_memory_corrupted)) {
		firehose_client_mark_corrupted(fc, reply_port);
		return MIG_NO_REPLY;
	}

	dispatch_queue_t q;
	if (for_io) {
		q = server_config.fs_io_drain_queue;
	} else {
		q = server_config.fs_mem_drain_queue;
	}
	dispatch_assert_queue(q);

	firehose_client_drain_one(fc, reply_port,
			for_io ? FIREHOSE_DRAIN_FOR_IO : 0);

	return MIG_NO_REPLY;
}

kern_return_t
firehose_server_get_logging_prefs(mach_port_t server_port OS_UNUSED,
		mach_port_t *mem_port, mach_vm_size_t *prefs_size)
{
	*mem_port = server_config.fs_prefs_cache_entry;
	*prefs_size = (mach_vm_size_t)server_config.fs_prefs_cache_size;
	return KERN_SUCCESS;
}

kern_return_t
firehose_server_should_send_strings(mach_port_t server_port OS_UNUSED,
		boolean_t *needs_strings)
{
	firehose_client_t fc = dispatch_mach_mig_demux_get_context();
	if (fc) {
		*needs_strings = !fc->fc_strings_cached;
		return KERN_SUCCESS;
	}
	return KERN_FAILURE;
}
