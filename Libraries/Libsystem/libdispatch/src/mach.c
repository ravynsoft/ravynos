/*
 * Copyright (c) 2008-2016 Apple Inc. All rights reserved.
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

#include "internal.h"
#if HAVE_MACH

#define DISPATCH_MACH_RETURN_IMMEDIATE_SEND_RESULT 0x1
#define DISPATCH_MACH_REGISTER_FOR_REPLY 0x2
#define DISPATCH_MACH_WAIT_FOR_REPLY 0x4
#define DISPATCH_MACH_OPTIONS_MASK 0xffff

#define DM_SEND_STATUS_SUCCESS 0x1
#define DM_SEND_STATUS_RETURNING_IMMEDIATE_SEND_RESULT 0x2

#define DM_CHECKIN_CANCELED ((dispatch_mach_msg_t)~0ul)

DISPATCH_OPTIONS(dispatch_mach_send_invoke_flags, uint32_t,
	DM_SEND_INVOKE_NONE            = 0x0,
	DM_SEND_INVOKE_MAKE_DIRTY      = 0x1,
	DM_SEND_INVOKE_NEEDS_BARRIER   = 0x2,
	DM_SEND_INVOKE_CAN_RUN_BARRIER = 0x4,
	DM_SEND_INVOKE_IMMEDIATE_SEND  = 0x8,
);
#define DM_SEND_INVOKE_IMMEDIATE_SEND_MASK \
		((dispatch_mach_send_invoke_flags_t)DM_SEND_INVOKE_IMMEDIATE_SEND)

static inline mach_msg_option_t _dispatch_mach_checkin_options(void);
static mach_port_t _dispatch_mach_msg_get_remote_port(dispatch_object_t dou);
static mach_port_t _dispatch_mach_msg_get_reply_port(dispatch_object_t dou);
static void _dispatch_mach_msg_disconnected(dispatch_mach_t dm,
		mach_port_t local_port, mach_port_t remote_port);
static inline void _dispatch_mach_msg_reply_received(dispatch_mach_t dm,
		dispatch_mach_reply_wait_refs_t dwr, mach_port_t local_port);
static dispatch_mach_msg_t _dispatch_mach_msg_create_reply_disconnected(
		dispatch_object_t dou, dispatch_mach_reply_refs_t dmr,
		dispatch_mach_reason_t reason);
static bool _dispatch_mach_reconnect_invoke(dispatch_mach_t dm,
		dispatch_object_t dou);
static inline mach_msg_header_t* _dispatch_mach_msg_get_msg(
		dispatch_mach_msg_t dmsg);
static void _dispatch_mach_send_push(dispatch_mach_t dm, dispatch_object_t dou,
		dispatch_qos_t qos);
static void _dispatch_mach_push_send_barrier_drain(dispatch_mach_t dm,
		dispatch_qos_t qos);
static void _dispatch_mach_handle_or_push_received_msg(dispatch_mach_t dm,
		dispatch_mach_msg_t dmsg, pthread_priority_t pp);
static void _dispatch_mach_push_async_reply_msg(dispatch_mach_t dm,
		dispatch_mach_msg_t dmsg, dispatch_queue_t drq);
static dispatch_queue_t _dispatch_mach_msg_context_async_reply_queue(
		dispatch_mach_t dm,
		void *ctxt);
static dispatch_continuation_t _dispatch_mach_msg_async_reply_wrap(
		dispatch_mach_msg_t dmsg, dispatch_mach_t dm);
static void _dispatch_mach_notification_kevent_unregister(dispatch_mach_t dm);
static void _dispatch_mach_notification_kevent_register(dispatch_mach_t dm,
		mach_port_t send);

// For tests only.
DISPATCH_EXPORT void _dispatch_mach_hooks_install_default(void);

#pragma mark -
#pragma mark dispatch to XPC callbacks

void
dispatch_mach_hooks_install_4libxpc(dispatch_mach_xpc_hooks_t hooks)
{
	if (hooks->version < DISPATCH_MACH_XPC_MIN_HOOKS_VERSION) {
		DISPATCH_CLIENT_CRASH(hooks,
				"trying to install hooks with unsupported version");
	}
	if (!os_atomic_cmpxchg(&_dispatch_mach_xpc_hooks,
			&_dispatch_mach_xpc_hooks_default, hooks, relaxed)) {
		DISPATCH_CLIENT_CRASH(_dispatch_mach_xpc_hooks,
				"dispatch_mach_hooks_install_4libxpc called twice");
	}
}

void
_dispatch_mach_hooks_install_default(void)
{
	os_atomic_store(&_dispatch_mach_xpc_hooks,
			&_dispatch_mach_xpc_hooks_default, relaxed);
}

#pragma mark -
#pragma mark dispatch_mach_t

DISPATCH_OPTIONS(dispatch_mach_create_flags, unsigned,
	DMCF_NONE             = 0x00000000,
	DMCF_HANDLER_IS_BLOCK = 0x00000001,
	DMCF_IS_XPC           = 0x00000002,
	DMCF_USE_STRICT_REPLY = 0x00000004,
);

static dispatch_mach_t
_dispatch_mach_create(const char *label, dispatch_queue_t q, void *context,
		dispatch_mach_handler_function_t handler,
		dispatch_mach_create_flags_t dmcf)
{
	dispatch_mach_recv_refs_t dmrr;
	dispatch_mach_send_refs_t dmsr;
	dispatch_mach_t dm;

	dm = _dispatch_queue_alloc(mach, DQF_MUTABLE, 1,
			DISPATCH_QUEUE_INACTIVE | DISPATCH_QUEUE_ROLE_INNER)._dm;
	dm->dq_label = label;
	dm->dm_is_xpc = (bool)(dmcf & DMCF_IS_XPC);
	dm->dm_strict_reply = (bool)(dmcf & DMCF_USE_STRICT_REPLY);

	dmrr = dux_create(&_dispatch_mach_type_recv, 0, 0)._dmrr;
	dispatch_assert(dmrr->du_is_direct);
	dmrr->du_owner_wref = _dispatch_ptr2wref(dm);
	dmrr->dmrr_handler_func = handler;
	dmrr->dmrr_handler_ctxt = context;
	dmrr->dmrr_handler_is_block = (bool)(dmcf & DMCF_HANDLER_IS_BLOCK);
	if (dm->dm_strict_reply) {
		dmrr->du_fflags |= MACH_MSG_STRICT_REPLY;
	}
	dm->dm_recv_refs = dmrr;

	dmsr = dux_create(&_dispatch_mach_type_send, 0,
			DISPATCH_MACH_SEND_POSSIBLE|DISPATCH_MACH_SEND_DEAD)._dmsr;
	dmsr->du_owner_wref = _dispatch_ptr2wref(dm);
	dm->dm_send_refs = dmsr;

	if (unlikely(!q)) {
		q = _dispatch_get_default_queue(true);
	} else {
		_dispatch_retain(q);
	}
	dm->do_targetq = q;
	_dispatch_object_debug(dm, "%s", __func__);
	return dm;
}

dispatch_mach_t
dispatch_mach_create(const char *label, dispatch_queue_t q,
		dispatch_mach_handler_t handler)
{
	dispatch_block_t bb = _dispatch_Block_copy((void*)handler);
	return _dispatch_mach_create(label, q, bb,
			(dispatch_mach_handler_function_t)_dispatch_Block_invoke(bb),
			DMCF_HANDLER_IS_BLOCK);
}

dispatch_mach_t
dispatch_mach_create_f(const char *label, dispatch_queue_t q, void *context,
		dispatch_mach_handler_function_t handler)
{
	return _dispatch_mach_create(label, q, context, handler, DMCF_NONE);
}

dispatch_mach_t
dispatch_mach_create_4libxpc(const char *label, dispatch_queue_t q,
		void *context, dispatch_mach_handler_function_t handler)
{
	return _dispatch_mach_create(label, q, context, handler, DMCF_IS_XPC | DMCF_USE_STRICT_REPLY);
}

void
_dispatch_mach_dispose(dispatch_mach_t dm, bool *allow_free)
{
	_dispatch_object_debug(dm, "%s", __func__);
	_dispatch_unote_dispose(dm->dm_recv_refs);
	dm->dm_recv_refs = NULL;
	_dispatch_unote_dispose(dm->dm_send_refs);
	dm->dm_send_refs = NULL;
	if (dm->dm_xpc_term_refs) {
		_dispatch_unote_dispose(dm->dm_xpc_term_refs);
		dm->dm_xpc_term_refs = NULL;
	}
	_dispatch_lane_class_dispose(dm, allow_free);
}

void
dispatch_mach_request_no_senders(dispatch_mach_t dm)
{
	dm->dm_arm_no_senders = true;
	_dispatch_queue_setter_assert_inactive(dm);
}

void
dispatch_mach_set_flags(dispatch_mach_t dm, dispatch_mach_flags_t flags)
{
	dm->dm_strict_reply = !!(flags & DMF_USE_STRICT_REPLY);
	dm->dm_arm_no_senders = !!(flags & DMF_REQUEST_NO_SENDERS);

	_dispatch_queue_setter_assert_inactive(dm);
}

static void
_dispatch_mach_arm_no_senders(dispatch_mach_t dm, bool allow_previous)
{
	mach_port_t recvp = (mach_port_t)dm->dm_recv_refs->du_ident;
	mach_port_t previous = MACH_PORT_NULL;
	kern_return_t kr;

	if (MACH_PORT_VALID(recvp)) {
		kr = mach_port_request_notification(mach_task_self(), recvp,
				MACH_NOTIFY_NO_SENDERS, 0, recvp,
				MACH_MSG_TYPE_MAKE_SEND_ONCE, &previous);
		DISPATCH_VERIFY_MIG(kr);
		dispatch_assume_zero(kr);
	}
	if (unlikely(previous)) {
		if (!allow_previous) {
			DISPATCH_CLIENT_CRASH(previous, "Mach port notification collision");
		}
		kr = mach_port_deallocate(mach_task_self(), previous);
		DISPATCH_VERIFY_MIG(kr);
		dispatch_assume_zero(kr);
	}
}

void
dispatch_mach_connect(dispatch_mach_t dm, mach_port_t receive,
		mach_port_t send, dispatch_mach_msg_t checkin)
{
	dispatch_mach_send_refs_t dmsr = dm->dm_send_refs;

	if (MACH_PORT_VALID(receive)) {
		dm->dm_recv_refs->du_ident = receive;
	}
	dmsr->dmsr_send = send;
	if (MACH_PORT_VALID(send)) {
		if (checkin) {
			dispatch_mach_msg_t dmsg = checkin;
			dispatch_retain(dmsg);
			dmsg->dmsg_options = _dispatch_mach_checkin_options();
			dmsr->dmsr_checkin_port = _dispatch_mach_msg_get_remote_port(dmsg);
		}
		dmsr->dmsr_checkin = checkin;
	}

	if (dm->dm_arm_no_senders && !dmsr->dmsr_checkin) {
		_dispatch_mach_arm_no_senders(dm, false);
	}

	uint32_t disconnect_cnt = os_atomic_and_orig2o(dmsr, dmsr_disconnect_cnt,
			~DISPATCH_MACH_NEVER_CONNECTED, relaxed);
	if (unlikely(!(disconnect_cnt & DISPATCH_MACH_NEVER_CONNECTED))) {
		DISPATCH_CLIENT_CRASH(disconnect_cnt, "Channel already connected");
	}
	_dispatch_object_debug(dm, "%s", __func__);
	return dispatch_activate(dm);
}

static inline void
_dispatch_mach_reply_list_insert(dispatch_mach_send_refs_t dmsr,
		dispatch_mach_reply_refs_t dmr)
{
	_dispatch_unfair_lock_lock(&dmsr->dmsr_replies_lock);
	dispatch_assert(!_LIST_IS_ENQUEUED(dmr, dmr_list));
	LIST_INSERT_HEAD(&dmsr->dmsr_replies, dmr, dmr_list);
	_dispatch_unfair_lock_unlock(&dmsr->dmsr_replies_lock);
}

static inline void
_dispatch_mach_reply_list_remove_locked(dispatch_mach_reply_refs_t dmr)
{
	dispatch_assert(_LIST_IS_ENQUEUED(dmr, dmr_list));
	LIST_REMOVE(dmr, dmr_list);
	_LIST_MARK_NOT_ENQUEUED(dmr, dmr_list);
}

static inline bool
_dispatch_mach_reply_list_tryremove(dispatch_mach_send_refs_t dmsr,
		dispatch_mach_reply_refs_t dmr)
{
	bool removed;
	_dispatch_unfair_lock_lock(&dmsr->dmsr_replies_lock);
	if ((removed = _LIST_IS_ENQUEUED(dmr, dmr_list))) {
		_dispatch_mach_reply_list_remove_locked(dmr);
	}
	_dispatch_unfair_lock_unlock(&dmsr->dmsr_replies_lock);
	return removed;
}

#define DMRU_DELETE_ACK     DUU_DELETE_ACK
#define DMRU_PROBE          DUU_PROBE
#define DMRU_MUST_SUCCEED   DUU_MUST_SUCCEED
#define DMRU_DUU_MASK       0x0f
#define DMRU_DISCONNECTED   0x10
#define DMRU_REMOVE         0x20
#define DMRU_ASYNC_MERGE    0x40
#define DMRU_CANCEL         0x80

DISPATCH_NOINLINE
static void
_dispatch_mach_reply_unregister(dispatch_mach_t dm,
		dispatch_mach_reply_refs_t dmr, uint32_t options)
{
	// - async waiters have a dmr of type &_dispatch_mach_type_reply
	//   heap-allocated in _dispatch_mach_reply_kevent_register().
	//
	// - sync waiters have a dmr of type DISPATCH_MACH_TYPE_WAITER,
	//   stack-allocated in _dispatch_mach_send_and_wait_for_reply().
	bool sync_waiter = (dux_type(dmr) == DISPATCH_MACH_TYPE_WAITER);
	dispatch_mach_send_refs_t dmsr = dm->dm_send_refs;
	bool disconnected = (options & DMRU_DISCONNECTED);
	bool wakeup = false;

	_dispatch_debug("machport[0x%08x]: unregistering for%s reply%s, ctxt %p",
			(mach_port_t)dmr->du_ident, sync_waiter ? " sync" : "",
			(options & DMRU_CANCEL) ? " (canceled)" :
			disconnected ? " (disconnected)" : "", dmr->dmr_ctxt);

	if (options & DMRU_REMOVE) {
		_dispatch_unfair_lock_lock(&dmsr->dmsr_replies_lock);
		_dispatch_mach_reply_list_remove_locked(dmr);
		if (LIST_EMPTY(&dmsr->dmsr_replies) && dmsr->dmsr_disconnect_cnt) {
			wakeup = true;
		}
		_dispatch_unfair_lock_unlock(&dmsr->dmsr_replies_lock);
	}

	if (_dispatch_unote_registered(dmr) &&
			!_dispatch_unote_unregister(dmr, options & DMRU_DUU_MASK)) {
		dispatch_assert(!sync_waiter); // sync waiters never use kevent
		if (options & DMRU_CANCEL) {
			// when canceling, failed unregistrations are put back in the list
			// the caller has the lock held
			LIST_INSERT_HEAD(&dmsr->dmsr_replies, dmr, dmr_list);
		}
		return;
	}

	dispatch_mach_msg_t dmsgr = NULL;
	dispatch_queue_t drq = NULL;
	if (disconnected) {
		if (dm->dm_is_xpc && dmr->dmr_ctxt) {
			drq = _dispatch_mach_msg_context_async_reply_queue(dm, dmr->dmr_ctxt);
		}
		dmsgr = _dispatch_mach_msg_create_reply_disconnected(NULL, dmr,
				drq ? DISPATCH_MACH_ASYNC_WAITER_DISCONNECTED
				: DISPATCH_MACH_DISCONNECTED);
		// _dispatch_mach_msg_create_reply_disconnected() consumes the voucher
		dispatch_assert(dmr->dmr_voucher == NULL);
	} else if (dmr->dmr_voucher) {
		_voucher_release(dmr->dmr_voucher);
		dmr->dmr_voucher = NULL;
	}
	if (!sync_waiter) {
		_dispatch_unote_dispose(dmr);
	}

	if (dmsgr) {
		if (drq) {
			_dispatch_mach_push_async_reply_msg(dm, dmsgr, drq);
		} else {
			_dispatch_mach_handle_or_push_received_msg(dm, dmsgr, 0);
		}
	}
	if (options & DMRU_ASYNC_MERGE) {
		if (wakeup) {
			return dx_wakeup(dm, 0,
					DISPATCH_WAKEUP_CONSUME_2 | DISPATCH_WAKEUP_MAKE_DIRTY);
		}
		return _dispatch_release_2_tailcall(dm);
	}
}

DISPATCH_NOINLINE
static void
_dispatch_mach_reply_waiter_register(dispatch_mach_t dm,
		dispatch_mach_reply_wait_refs_t dwr, mach_port_t reply_port,
		dispatch_mach_msg_t dmsg)
{
	dispatch_mach_reply_refs_t dmr = &dwr->dwr_refs;
	dmr->du_owner_wref = _dispatch_ptr2wref(dm);
	dmr->du_filter = EVFILT_MACHPORT;
	dmr->du_ident = reply_port;
	if (!dmr->dmr_reply_port_owned) {
		if (dmsg->dmsg_voucher) {
			dmr->dmr_voucher = _voucher_retain(dmsg->dmsg_voucher);
		}
		dmr->dmr_priority = dmsg->dmsg_priority;
		// make reply context visible to leaks rdar://11777199
		dmr->dmr_ctxt = dmsg->do_ctxt;
	}

	_dispatch_debug("machport[0x%08x]: registering for sync reply, ctxt %p",
			reply_port, dmsg->do_ctxt);
	_dispatch_mach_reply_list_insert(dm->dm_send_refs, dmr);
}

DISPATCH_NOINLINE
static void
_dispatch_mach_reply_kevent_register(dispatch_mach_t dm, mach_port_t reply_port,
		dispatch_mach_msg_t dmsg)
{
	dispatch_mach_reply_refs_t dmr;
	dispatch_priority_t mpri, pri, overcommit;
	dispatch_qos_t fallback;
	dispatch_wlh_t wlh;

	dmr = dux_create(&_dispatch_mach_type_reply, reply_port, 0)._dmr;
	dispatch_assert(dmr->du_is_direct);
	dmr->du_owner_wref = _dispatch_ptr2wref(dm);
	if (dmsg->dmsg_voucher) {
		dmr->dmr_voucher = _voucher_retain(dmsg->dmsg_voucher);
	}
	dmr->dmr_priority = dmsg->dmsg_priority;
	// make reply context visible to leaks rdar://11777199
	dmr->dmr_ctxt = dmsg->do_ctxt;

	dispatch_queue_t drq = NULL;
	if (dm->dm_is_xpc && dmsg->do_ctxt) {
		drq = _dispatch_mach_msg_context_async_reply_queue(dm, dmsg->do_ctxt);
	}
	if (dm->dm_strict_reply) {
		dmr->du_fflags |= MACH_MSG_STRICT_REPLY;
	}
	if (unlikely((!drq || drq == dm->_as_dq) &&
			_dispatch_unote_wlh(dm->dm_recv_refs))) {
		wlh = _dispatch_unote_wlh(dm->dm_recv_refs);
		pri = dm->dq_priority;
	} else if (dx_hastypeflag(drq, QUEUE_ROOT)) {
		wlh = DISPATCH_WLH_ANON;
		if (_dispatch_is_in_root_queues_array(drq)) {
			pri = drq->dq_priority;
		} else {
			pri = DISPATCH_PRIORITY_FLAG_MANAGER;
		}
	} else if (!(pri = _dispatch_queue_compute_priority_and_wlh(drq, &wlh))) {
		wlh = DISPATCH_WLH_ANON;
		pri = drq->dq_priority;
	}
	mpri = _dispatch_priority_from_pp_strip_flags(dmsg->dmsg_priority);
	overcommit = pri & DISPATCH_PRIORITY_FLAG_OVERCOMMIT;
	fallback = _dispatch_priority_fallback_qos(pri);
	if (pri & DISPATCH_PRIORITY_REQUESTED_MASK) {
		pri &= DISPATCH_PRIORITY_REQUESTED_MASK;
		if (pri < mpri) pri = mpri;
		pri |= overcommit;
	} else if (fallback && mpri) {
		pri = mpri | overcommit;
	} else if (fallback && !mpri) {
		pri = _dispatch_priority_make(fallback, 0) | overcommit;
	} else {
		pri = DISPATCH_PRIORITY_FLAG_MANAGER;
		wlh = DISPATCH_WLH_ANON;
	}

	_dispatch_debug("machport[0x%08x]: registering for reply, ctxt %p",
			reply_port, dmsg->do_ctxt);
	_dispatch_mach_reply_list_insert(dm->dm_send_refs, dmr);

	if (!_dispatch_unote_register(dmr, wlh, pri)) {
		uint32_t options = DMRU_MUST_SUCCEED | DMRU_REMOVE | DMRU_DISCONNECTED;
		_dispatch_mach_reply_unregister(dm, dmr, options);
	}
}

#pragma mark -
#pragma mark dispatch_mach_msg

DISPATCH_ALWAYS_INLINE DISPATCH_CONST
static inline bool
_dispatch_use_mach_special_reply_port(void)
{
#if DISPATCH_USE_MACH_SEND_SYNC_OVERRIDE
	return true;
#else
#define thread_get_special_reply_port() ({__builtin_trap(); MACH_PORT_NULL;})
	return false;
#endif
}

static void
_dispatch_destruct_reply_port(mach_port_t reply_port,
		enum thread_destruct_special_reply_port_rights rights)
{
	kern_return_t kr = KERN_SUCCESS;

	if (_dispatch_use_mach_special_reply_port()) {
		kr = thread_destruct_special_reply_port(reply_port, rights);
	} else if (rights == THREAD_SPECIAL_REPLY_PORT_ALL ||
			rights == THREAD_SPECIAL_REPLY_PORT_RECEIVE_ONLY) {
		kr = mach_port_destruct(mach_task_self(), reply_port, 0, 0);
	}
	DISPATCH_VERIFY_MIG(kr);
	dispatch_assume_zero(kr);
}

static mach_port_t
_dispatch_get_thread_reply_port(void)
{
	mach_port_t reply_port, mrp;
	if (_dispatch_use_mach_special_reply_port()) {
		mrp = _dispatch_get_thread_special_reply_port();
	} else {
		mrp = _dispatch_get_thread_mig_reply_port();
	}
	if (mrp) {
		reply_port = mrp;
		_dispatch_debug("machport[0x%08x]: borrowed thread sync reply port",
				reply_port);
	} else {
		if (_dispatch_use_mach_special_reply_port()) {
			reply_port = thread_get_special_reply_port();
			_dispatch_set_thread_special_reply_port(reply_port);
		} else {
			reply_port = mach_reply_port();
			_dispatch_set_thread_mig_reply_port(reply_port);
		}
		if (unlikely(!MACH_PORT_VALID(reply_port))) {
			DISPATCH_CLIENT_CRASH(_dispatch_use_mach_special_reply_port(),
				"Unable to allocate reply port, possible port leak");
		}
		_dispatch_debug("machport[0x%08x]: allocated thread sync reply port",
				reply_port);
	}
	_dispatch_debug_machport(reply_port);
	return reply_port;
}

static void
_dispatch_clear_thread_reply_port(mach_port_t reply_port)
{
	mach_port_t mrp;
	if (_dispatch_use_mach_special_reply_port()) {
		mrp = _dispatch_get_thread_special_reply_port();
	} else {
		mrp = _dispatch_get_thread_mig_reply_port();
	}
	if (reply_port != mrp) {
		if (mrp) {
			_dispatch_debug("machport[0x%08x]: did not clear thread sync reply "
					"port (found 0x%08x)", reply_port, mrp);
		}
		return;
	}
	if (_dispatch_use_mach_special_reply_port()) {
		_dispatch_set_thread_special_reply_port(MACH_PORT_NULL);
	} else {
		_dispatch_set_thread_mig_reply_port(MACH_PORT_NULL);
	}
	_dispatch_debug_machport(reply_port);
	_dispatch_debug("machport[0x%08x]: cleared thread sync reply port",
			reply_port);
}

static void
_dispatch_set_thread_reply_port(mach_port_t reply_port)
{
	_dispatch_debug_machport(reply_port);
	mach_port_t mrp;
	if (_dispatch_use_mach_special_reply_port()) {
		mrp = _dispatch_get_thread_special_reply_port();
	} else {
		mrp = _dispatch_get_thread_mig_reply_port();
	}
	if (mrp) {
		_dispatch_destruct_reply_port(reply_port,
				THREAD_SPECIAL_REPLY_PORT_ALL);
		_dispatch_debug("machport[0x%08x]: deallocated sync reply port "
				"(found 0x%08x)", reply_port, mrp);
	} else {
		if (_dispatch_use_mach_special_reply_port()) {
			_dispatch_set_thread_special_reply_port(reply_port);
		} else {
			_dispatch_set_thread_mig_reply_port(reply_port);
		}
		_dispatch_debug("machport[0x%08x]: restored thread sync reply port",
				reply_port);
	}
}

static inline mach_port_t
_dispatch_mach_msg_get_remote_port(dispatch_object_t dou)
{
	mach_msg_header_t *hdr = _dispatch_mach_msg_get_msg(dou._dmsg);
	mach_port_t remote = hdr->msgh_remote_port;
	return remote;
}

static inline mach_port_t
_dispatch_mach_msg_get_reply_port(dispatch_object_t dou)
{
	mach_msg_header_t *hdr = _dispatch_mach_msg_get_msg(dou._dmsg);
	mach_port_t local = hdr->msgh_local_port;
	if (!MACH_PORT_VALID(local) || MACH_MSGH_BITS_LOCAL(hdr->msgh_bits) !=
			MACH_MSG_TYPE_MAKE_SEND_ONCE) return MACH_PORT_NULL;
	return local;
}

static inline void
_dispatch_mach_msg_set_reason(dispatch_mach_msg_t dmsg, mach_error_t err,
		unsigned long reason)
{
	dispatch_assert_zero(reason & ~(unsigned long)code_emask);
	dmsg->dmsg_error = ((err || !reason) ? err :
			 err_local|err_sub(0x3e0)|(mach_error_t)reason);
}

static inline unsigned long
_dispatch_mach_msg_get_reason(dispatch_mach_msg_t dmsg, mach_error_t *err_ptr)
{
	mach_error_t err = dmsg->dmsg_error;

	if ((err & system_emask) == err_local && err_get_sub(err) == 0x3e0) {
		*err_ptr = 0;
		return err_get_code(err);
	}
	*err_ptr = err;
	return err ? DISPATCH_MACH_MESSAGE_SEND_FAILED : DISPATCH_MACH_MESSAGE_SENT;
}

static inline dispatch_mach_msg_t
_dispatch_mach_msg_create_recv(mach_msg_header_t *hdr, mach_msg_size_t siz,
		dispatch_mach_reply_refs_t dmr, uint32_t flags, pthread_priority_t pp)
{
	dispatch_mach_msg_destructor_t destructor;
	dispatch_mach_msg_t dmsg;
	voucher_t voucher;

	if (dmr) {
		_voucher_mach_msg_clear(hdr, false); // deallocate reply message voucher
		pp = dmr->dmr_priority;
		voucher = dmr->dmr_voucher;
		dmr->dmr_voucher = NULL; // transfer reference
	} else {
		voucher = voucher_create_with_mach_msg(hdr);
		pp = _dispatch_priority_compute_propagated(pp, 0);
	}

	destructor = (flags & DISPATCH_EV_MSG_NEEDS_FREE) ?
			DISPATCH_MACH_MSG_DESTRUCTOR_FREE :
			DISPATCH_MACH_MSG_DESTRUCTOR_DEFAULT;
	dmsg = dispatch_mach_msg_create(hdr, siz, destructor, NULL);
	if (!(flags & DISPATCH_EV_MSG_NEEDS_FREE)) {
		_dispatch_ktrace2(DISPATCH_MACH_MSG_hdr_move,
				(uint64_t)hdr, (uint64_t)dmsg->dmsg_buf);
	}
	dmsg->dmsg_voucher = voucher;
	dmsg->dmsg_priority = pp;
	dmsg->do_ctxt = dmr ? dmr->dmr_ctxt : NULL;
	_dispatch_mach_msg_set_reason(dmsg, 0, DISPATCH_MACH_MESSAGE_RECEIVED);
	_dispatch_voucher_debug("mach-msg[%p] create", voucher, dmsg);
	_dispatch_voucher_ktrace_dmsg_push(dmsg);
	return dmsg;
}

DISPATCH_NOINLINE
static void
_dispatch_mach_no_senders_invoke(dispatch_mach_t dm)
{
	if (!(_dispatch_queue_atomic_flags(dm) & DSF_CANCELED)) {
		dispatch_mach_recv_refs_t dmrr = dm->dm_recv_refs;
		_dispatch_client_callout4(dmrr->dmrr_handler_ctxt,
				DISPATCH_MACH_NO_SENDERS, NULL, 0, dmrr->dmrr_handler_func);
	}
	_dispatch_perfmon_workitem_inc();
}

void
_dispatch_mach_merge_msg(dispatch_unote_t du, uint32_t flags,
		mach_msg_header_t *hdr, mach_msg_size_t siz,
		pthread_priority_t msg_pp, pthread_priority_t ovr_pp)

{
	if (flags & EV_VANISHED) {
		DISPATCH_CLIENT_CRASH(du._du->du_ident,
				"Unexpected EV_VANISHED (do not destroy random mach ports)");
	}

	_dispatch_debug_machport(hdr->msgh_remote_port);
	_dispatch_debug("machport[0x%08x]: received msg id 0x%x, reply on 0x%08x",
			hdr->msgh_local_port, hdr->msgh_id, hdr->msgh_remote_port);

	dispatch_mach_t dm = _dispatch_wref2ptr(du._dmrr->du_owner_wref);
	if (unlikely(_dispatch_queue_atomic_flags(dm) & DSF_CANCELED)) {
		_dispatch_debug("machport[0x%08x]: drop msg id 0x%x, reply on 0x%08x",
				hdr->msgh_local_port, hdr->msgh_id, hdr->msgh_remote_port);
		mach_msg_destroy(hdr);
		if (flags & DISPATCH_EV_MSG_NEEDS_FREE) {
			free(hdr);
		}
	} else if (hdr->msgh_id == MACH_NOTIFY_NO_SENDERS && dm->dm_arm_no_senders){
		if (dispatch_assume(_dispatch_mach_msg_sender_is_kernel(hdr))) {
			dispatch_continuation_t dc = _dispatch_continuation_alloc();
			(void)_dispatch_continuation_init_f(dc, dm, dm,
					(dispatch_function_t)_dispatch_mach_no_senders_invoke,
					DISPATCH_BLOCK_HAS_PRIORITY | DISPATCH_BLOCK_NO_VOUCHER,
					DC_FLAG_CONSUME);
			_dispatch_continuation_async(dm, dc, 0, dc->dc_flags);
		}
		mach_msg_destroy(hdr);
		if (flags & DISPATCH_EV_MSG_NEEDS_FREE) {
			free(hdr);
		}
	} else {
		// Once the mach channel disarming is visible, cancellation will switch
		// to immediately destroy messages.  If we're preempted here, then the
		// whole cancellation sequence may be complete by the time we really
		// enqueue the message.
		//
		// _dispatch_mach_msg_invoke_with_mach() is responsible for filtering it
		// out to keep the promise that DISPATCH_MACH_DISCONNECTED is the last
		// event sent.
		dispatch_mach_msg_t dmsg;
		dmsg = _dispatch_mach_msg_create_recv(hdr, siz, NULL, flags, msg_pp);
		_dispatch_mach_handle_or_push_received_msg(dm, dmsg, ovr_pp);
	}

	// Note: it is ok to do a relaxed load of the dq_state_bits as we only care
	// about bits that are in the top bits of the 64bit dq_state.
	// This avoids expensive CAS on 32bit acrhictures.
	if (unlikely(_dispatch_unote_needs_delete(du) ||
			_dq_state_is_activating((uint64_t)dm->dq_state_bits << 32))) {
		return dx_wakeup(dm, 0, DISPATCH_WAKEUP_EVENT |
				DISPATCH_WAKEUP_CLEAR_ACTIVATING |
				DISPATCH_WAKEUP_CONSUME_2 | DISPATCH_WAKEUP_MAKE_DIRTY);
	}
	return _dispatch_release_2_tailcall(dm);
}

void
_dispatch_mach_reply_merge_msg(dispatch_unote_t du, uint32_t flags,
		mach_msg_header_t *hdr, mach_msg_size_t siz,
		pthread_priority_t msg_pp, pthread_priority_t ovr_pp)
{
	dispatch_mach_reply_refs_t dmr = du._dmr;
	dispatch_mach_t dm = _dispatch_wref2ptr(dmr->du_owner_wref);
	bool canceled = (_dispatch_queue_atomic_flags(dm) & DSF_CANCELED);
	dispatch_mach_msg_t dmsg = NULL;

	_dispatch_debug_machport(hdr->msgh_remote_port);
	_dispatch_debug("machport[0x%08x]: received msg id 0x%x, reply on 0x%08x",
			hdr->msgh_local_port, hdr->msgh_id, hdr->msgh_remote_port);

	if (!canceled) {
		dmsg = _dispatch_mach_msg_create_recv(hdr, siz, dmr, flags, msg_pp);
	}

	if (dmsg) {
		dispatch_queue_t drq = NULL;
		if (dm->dm_is_xpc && dmsg->do_ctxt) {
			drq = _dispatch_mach_msg_context_async_reply_queue(dm, dmsg->do_ctxt);
		}
		if (drq) {
			_dispatch_mach_push_async_reply_msg(dm, dmsg, drq);
		} else {
			_dispatch_mach_handle_or_push_received_msg(dm, dmsg, ovr_pp);
		}
	} else {
		_dispatch_debug("machport[0x%08x]: drop msg id 0x%x, reply on 0x%08x",
				hdr->msgh_local_port, hdr->msgh_id, hdr->msgh_remote_port);
		mach_msg_destroy(hdr);
		if (flags & DISPATCH_EV_MSG_NEEDS_FREE) {
			free(hdr);
		}
	}

	uint32_t options = DMRU_ASYNC_MERGE | DMRU_REMOVE;
	options |= DMRU_MUST_SUCCEED | DMRU_DELETE_ACK;
	if (canceled) options |= DMRU_DISCONNECTED;
	dispatch_assert(_dispatch_unote_needs_delete(dmr));
	_dispatch_mach_reply_unregister(dm, dmr, options); // consumes the +2
}

DISPATCH_ALWAYS_INLINE
static void
_dispatch_mach_stack_probe(void *addr, size_t size)
{
#if __has_feature(stack_check)
	(void)addr; (void)size;
#else
	for (mach_vm_address_t p = mach_vm_trunc_page(addr + vm_page_size);
			p < (mach_vm_address_t)addr + size; p += vm_page_size) {
		*(char*)p = 0; // ensure alloca buffer doesn't overlap with stack guard
	}
#endif
}

DISPATCH_ALWAYS_INLINE
static inline dispatch_mach_msg_t
_dispatch_mach_msg_reply_recv(dispatch_mach_t dm,
		dispatch_mach_reply_wait_refs_t dwr, mach_port_t reply_port,
		mach_port_t send)
{
	if (unlikely(!MACH_PORT_VALID(reply_port))) {
		DISPATCH_CLIENT_CRASH(reply_port, "Invalid reply port");
	}
	void *ctxt = dwr->dwr_refs.dmr_ctxt;
	mach_msg_header_t *hdr, *hdr2 = NULL;
	void *hdr_copyout_addr;
	mach_msg_size_t siz, msgsiz = 0;
	mach_msg_return_t kr;
	mach_msg_option_t options;
	mach_port_t notify = MACH_PORT_NULL;
	siz = mach_vm_round_page(DISPATCH_MACH_RECEIVE_MAX_INLINE_MESSAGE_SIZE +
			DISPATCH_MACH_TRAILER_SIZE);
	hdr = alloca(siz);
	_dispatch_mach_stack_probe(hdr, siz);
	options = DISPATCH_MACH_RCV_OPTIONS & (~MACH_RCV_VOUCHER);
	if (MACH_PORT_VALID(send)) {
		notify = send;
		options |= MACH_RCV_SYNC_WAIT;
	}
	if (dm->dm_strict_reply) {
		options |= MACH_MSG_STRICT_REPLY;
	}

retry:
	_dispatch_debug_machport(reply_port);
	_dispatch_debug("machport[0x%08x]: MACH_RCV_MSG %s", reply_port,
			(options & MACH_RCV_TIMEOUT) ? "poll" : "wait");
	kr = mach_msg(hdr, options, 0, siz, reply_port, MACH_MSG_TIMEOUT_NONE,
			notify);
	hdr_copyout_addr = hdr;
	_dispatch_debug_machport(reply_port);
	_dispatch_debug("machport[0x%08x]: MACH_RCV_MSG (size %u, opts 0x%x) "
			"returned: %s - 0x%x", reply_port, siz, options,
			mach_error_string(kr), kr);
	switch (kr) {
	case MACH_RCV_TOO_LARGE:
		if (unlikely(hdr->msgh_size > UINT_MAX - DISPATCH_MACH_TRAILER_SIZE)) {
			DISPATCH_CLIENT_CRASH(hdr->msgh_size, "Overlarge message");
		}
		if (options & MACH_RCV_LARGE) {
			msgsiz = hdr->msgh_size + DISPATCH_MACH_TRAILER_SIZE;
			hdr2 = malloc(msgsiz);
			if (dispatch_assume(hdr2)) {
				hdr = hdr2;
				siz = msgsiz;
			}
			options |= MACH_RCV_TIMEOUT;
			options &= ~MACH_RCV_LARGE;
			goto retry;
		}
		_dispatch_log("BUG in libdispatch client: "
				"dispatch_mach_send_and_wait_for_reply: dropped message too "
				"large to fit in memory: id = 0x%x, size = %u", hdr->msgh_id,
				hdr->msgh_size);
		break;
	case MACH_RCV_INVALID_NAME: // rdar://problem/21963848
	case MACH_RCV_PORT_CHANGED: // rdar://problem/21885327
	case MACH_RCV_PORT_DIED:
		// channel was disconnected/canceled and reply port destroyed
		_dispatch_debug("machport[0x%08x]: sync reply port destroyed, ctxt %p: "
				"%s - 0x%x", reply_port, ctxt, mach_error_string(kr), kr);
		if (dwr->dwr_refs.dmr_reply_port_owned) {
			_dispatch_destruct_reply_port(reply_port,
					THREAD_SPECIAL_REPLY_PORT_SEND_ONLY);
		}
		goto out;
	case MACH_MSG_SUCCESS:
		if (hdr->msgh_remote_port) {
			_dispatch_debug_machport(hdr->msgh_remote_port);
		}
		_dispatch_debug("machport[0x%08x]: received msg id 0x%x, size = %u, "
				"reply on 0x%08x", hdr->msgh_local_port, hdr->msgh_id,
				hdr->msgh_size, hdr->msgh_remote_port);
		siz = hdr->msgh_size + DISPATCH_MACH_TRAILER_SIZE;
		if (hdr2 && siz < msgsiz) {
			void *shrink = realloc(hdr2, msgsiz);
			if (shrink) hdr = hdr2 = shrink;
		}
		break;
	case MACH_RCV_INVALID_NOTIFY:
	default:
		DISPATCH_INTERNAL_CRASH(kr, "Unexpected error from mach_msg_receive");
		break;
	}
	_dispatch_mach_msg_reply_received(dm, dwr, hdr->msgh_local_port);
	hdr->msgh_local_port = MACH_PORT_NULL;
	if (unlikely((dm->dq_atomic_flags & DSF_CANCELED) || kr)) {
		if (!kr) mach_msg_destroy(hdr);
		goto out;
	}
	dispatch_mach_msg_t dmsg;
	dispatch_mach_msg_destructor_t destructor = (!hdr2) ?
			DISPATCH_MACH_MSG_DESTRUCTOR_DEFAULT :
			DISPATCH_MACH_MSG_DESTRUCTOR_FREE;
	dmsg = dispatch_mach_msg_create(hdr, siz, destructor, NULL);
	if (!hdr2 || hdr != hdr_copyout_addr) {
		_dispatch_ktrace2(DISPATCH_MACH_MSG_hdr_move,
				(uint64_t)hdr_copyout_addr,
				(uint64_t)_dispatch_mach_msg_get_msg(dmsg));
	}
	dmsg->do_ctxt = ctxt;
	return dmsg;
out:
	free(hdr2);
	return NULL;
}

static inline void
_dispatch_mach_msg_reply_received(dispatch_mach_t dm,
		dispatch_mach_reply_wait_refs_t dwr, mach_port_t local_port)
{
	dispatch_mach_reply_refs_t dmr = &dwr->dwr_refs;
	bool removed = _dispatch_mach_reply_list_tryremove(dm->dm_send_refs, dmr);
	mach_port_t reply_port = (mach_port_t)dmr->du_ident;

	if (removed) {
		_dispatch_debug("machport[0x%08x]: unregistered for sync reply, ctxt %p",
				reply_port, dmr->dmr_ctxt);
	}

	if (dmr->dmr_reply_port_owned) {
		if (local_port != reply_port &&
				(removed || MACH_PORT_VALID(local_port))) {
			DISPATCH_CLIENT_CRASH(local_port,
					"Reply received on unexpected port");
		}
		if (removed) {
			_dispatch_set_thread_reply_port(reply_port);
		} else {
			_dispatch_destruct_reply_port(reply_port,
					THREAD_SPECIAL_REPLY_PORT_SEND_ONLY);
		}
		return;
	}

	if (!MACH_PORT_VALID(local_port) || !removed) {
		// port moved/destroyed during receive, or reply waiter was never
		// registered or already removed (disconnected)
		return;
	}

	mach_msg_header_t *hdr;
	dispatch_mach_msg_t dmsg;
	dmsg = dispatch_mach_msg_create(NULL, sizeof(mach_msg_header_t),
			DISPATCH_MACH_MSG_DESTRUCTOR_DEFAULT, &hdr);
	hdr->msgh_local_port = local_port;
	dmsg->dmsg_voucher = dmr->dmr_voucher;
	dmr->dmr_voucher = NULL;  // transfer reference
	dmsg->dmsg_priority = dmr->dmr_priority;
	dmsg->do_ctxt = dmr->dmr_ctxt;
	_dispatch_mach_msg_set_reason(dmsg, 0, DISPATCH_MACH_REPLY_RECEIVED);
	return _dispatch_mach_handle_or_push_received_msg(dm, dmsg, 0);
}

static inline void
_dispatch_mach_msg_disconnected(dispatch_mach_t dm, mach_port_t local_port,
		mach_port_t remote_port)
{
	mach_msg_header_t *hdr;
	dispatch_mach_msg_t dmsg;
	dmsg = dispatch_mach_msg_create(NULL, sizeof(mach_msg_header_t),
			DISPATCH_MACH_MSG_DESTRUCTOR_DEFAULT, &hdr);
	if (local_port) hdr->msgh_local_port = local_port;
	if (remote_port) hdr->msgh_remote_port = remote_port;
	_dispatch_mach_msg_set_reason(dmsg, 0, DISPATCH_MACH_DISCONNECTED);
	_dispatch_debug("machport[0x%08x]: %s right disconnected", local_port ?
			local_port : remote_port, local_port ? "receive" : "send");
	return _dispatch_mach_handle_or_push_received_msg(dm, dmsg, 0);
}

static inline dispatch_mach_msg_t
_dispatch_mach_msg_create_reply_disconnected(dispatch_object_t dou,
		dispatch_mach_reply_refs_t dmr, dispatch_mach_reason_t reason)
{
	dispatch_mach_msg_t dmsg = dou._dmsg, dmsgr;
	mach_port_t reply_port = dmsg ? dmsg->dmsg_reply :(mach_port_t)dmr->du_ident;

	if (!reply_port) {
		if (!dmsg && dmr->dmr_voucher) {
			_voucher_release(dmr->dmr_voucher);
			dmr->dmr_voucher = NULL;
		}
		return NULL;
	}

	if (dmr && !_dispatch_unote_registered(dmr) && dmr->dmr_reply_port_owned) {
		if (dmr->dmr_voucher) {
			_voucher_release(dmr->dmr_voucher);
			dmr->dmr_voucher = NULL;
		}
		// deallocate owned reply port to break _dispatch_mach_msg_reply_recv
		// out of waiting in mach_msg(MACH_RCV_MSG).
		//
		// after this call, dmr can become invalid
		_dispatch_destruct_reply_port(reply_port,
				THREAD_SPECIAL_REPLY_PORT_RECEIVE_ONLY);
		return NULL;
	}

	mach_msg_header_t *hdr;
	dmsgr = dispatch_mach_msg_create(NULL, sizeof(mach_msg_header_t),
			DISPATCH_MACH_MSG_DESTRUCTOR_DEFAULT, &hdr);
	hdr->msgh_local_port = reply_port;
	if (dmsg) {
		dmsgr->dmsg_priority = dmsg->dmsg_priority;
		dmsgr->do_ctxt = dmsg->do_ctxt;
		dmsgr->dmsg_voucher = dmsg->dmsg_voucher;
		if (dmsgr->dmsg_voucher) _voucher_retain(dmsgr->dmsg_voucher);
	} else {
		dmsgr->dmsg_priority = dmr->dmr_priority;
		dmsgr->do_ctxt = dmr->dmr_ctxt;
		dmsgr->dmsg_voucher = dmr->dmr_voucher;
		dmr->dmr_voucher = NULL; // transfer reference
	}
	_dispatch_mach_msg_set_reason(dmsgr, 0, reason);
	_dispatch_debug("machport[0x%08x]: reply disconnected, ctxt %p",
			hdr->msgh_local_port, dmsgr->do_ctxt);
	return dmsgr;
}

DISPATCH_NOINLINE
static void
_dispatch_mach_msg_not_sent(dispatch_mach_t dm, dispatch_object_t dou,
		dispatch_mach_reply_wait_refs_t dwr)
{
	dispatch_mach_msg_t dmsg = dou._dmsg, dmsgr;
	dispatch_queue_t drq = NULL;
	mach_msg_header_t *msg = _dispatch_mach_msg_get_msg(dmsg);
	mach_msg_option_t msg_opts = dmsg->dmsg_options;
	_dispatch_debug("machport[0x%08x]: not sent msg id 0x%x, ctxt %p, "
			"msg_opts 0x%x, kvoucher 0x%08x, reply on 0x%08x",
			msg->msgh_remote_port, msg->msgh_id, dmsg->do_ctxt,
			msg_opts, msg->msgh_voucher_port, dmsg->dmsg_reply);
	unsigned long reason = (msg_opts & DISPATCH_MACH_REGISTER_FOR_REPLY) ?
			0 : DISPATCH_MACH_MESSAGE_NOT_SENT;
	if (dm->dm_is_xpc && dmsg->do_ctxt) {
		drq = _dispatch_mach_msg_context_async_reply_queue(dm, dmsg->do_ctxt);
	}
	dmsgr = _dispatch_mach_msg_create_reply_disconnected(dmsg,
			dwr ? &dwr->dwr_refs : NULL,
			drq ? DISPATCH_MACH_ASYNC_WAITER_DISCONNECTED
			: DISPATCH_MACH_DISCONNECTED);
	_dispatch_mach_msg_set_reason(dmsg, 0, reason);
	_dispatch_mach_handle_or_push_received_msg(dm, dmsg, 0);
	if (dmsgr) {
		if (drq) {
			_dispatch_mach_push_async_reply_msg(dm, dmsgr, drq);
		} else {
			_dispatch_mach_handle_or_push_received_msg(dm, dmsgr, 0);
		}
	}
}

DISPATCH_NOINLINE
static uint32_t
_dispatch_mach_msg_send(dispatch_mach_t dm, dispatch_object_t dou,
		dispatch_mach_reply_wait_refs_t dwr, dispatch_qos_t qos,
		dispatch_mach_send_invoke_flags_t send_flags)
{
	dispatch_mach_send_refs_t dsrr = dm->dm_send_refs;
	dispatch_mach_msg_t dmsg = dou._dmsg, dmsgr = NULL;
	voucher_t voucher = dmsg->dmsg_voucher;
	dispatch_queue_t drq = NULL;
	mach_voucher_t ipc_kvoucher = MACH_VOUCHER_NULL;
	uint32_t send_status = 0;
	bool clear_voucher = false, kvoucher_move_send = false;
	mach_msg_header_t *msg = _dispatch_mach_msg_get_msg(dmsg);
	bool is_reply = (MACH_MSGH_BITS_REMOTE(msg->msgh_bits) ==
			MACH_MSG_TYPE_MOVE_SEND_ONCE);
	mach_port_t reply_port = dmsg->dmsg_reply;
	if (!is_reply) {
		dm->dm_needs_mgr = 0;
		if (unlikely(dsrr->dmsr_checkin && dmsg != dsrr->dmsr_checkin)) {
			// send initial checkin message
			if (unlikely(_dispatch_unote_registered(dsrr) &&
					_dispatch_queue_get_current() != _dispatch_mgr_q._as_dq)) {
				// send kevent must be uninstalled on the manager queue
				dm->dm_needs_mgr = true;
				goto out;
			}
			if (unlikely(!_dispatch_mach_msg_send(dm,
					dsrr->dmsr_checkin, NULL, qos, DM_SEND_INVOKE_NONE))) {
				goto out;
			}
			if (dm->dm_arm_no_senders) {
				_dispatch_mach_arm_no_senders(dm, true);
			}
			dsrr->dmsr_checkin = NULL;
		}
	}
	mach_msg_return_t kr = 0;
	mach_msg_option_t opts = 0, msg_opts = dmsg->dmsg_options;
	if (!(msg_opts & DISPATCH_MACH_REGISTER_FOR_REPLY)) {
		mach_msg_priority_t msg_priority = MACH_MSG_PRIORITY_UNSPECIFIED;
		opts = MACH_SEND_MSG | (msg_opts & ~DISPATCH_MACH_OPTIONS_MASK);
		if (!is_reply) {
			if (dmsg != dsrr->dmsr_checkin) {
				msg->msgh_remote_port = dsrr->dmsr_send;
			}
			if (_dispatch_queue_get_current() == _dispatch_mgr_q._as_dq) {
				if (unlikely(!_dispatch_unote_registered(dsrr))) {
					_dispatch_mach_notification_kevent_register(dm,
							msg->msgh_remote_port);
					dispatch_assert(_dispatch_unote_registered(dsrr));
				}
				if (os_atomic_load(&dsrr->dmsr_notification_armed, relaxed)) {
					goto out;
				}
				opts |= MACH_SEND_NOTIFY;
			}
			opts |= MACH_SEND_TIMEOUT;
			if (dmsg->dmsg_priority != _voucher_get_priority(voucher)) {
				ipc_kvoucher = _voucher_create_mach_voucher_with_priority(
						voucher, dmsg->dmsg_priority);
			}
			_dispatch_voucher_debug("mach-msg[%p] msg_set", voucher, dmsg);
			if (ipc_kvoucher) {
				kvoucher_move_send = true;
				clear_voucher = _voucher_mach_msg_set_mach_voucher(msg,
						ipc_kvoucher, kvoucher_move_send);
			} else {
				clear_voucher = _voucher_mach_msg_set(msg, voucher);
			}
			if (qos) {
				opts |= MACH_SEND_OVERRIDE;
				msg_priority = (mach_msg_priority_t)
						_dispatch_priority_compute_propagated(
						_dispatch_qos_to_pp(qos), 0);
			}
			if (reply_port && dm->dm_strict_reply) {
				opts |= MACH_MSG_STRICT_REPLY;
			}
		}
		_dispatch_debug_machport(msg->msgh_remote_port);
		if (reply_port) _dispatch_debug_machport(reply_port);
		if (msg_opts & DISPATCH_MACH_WAIT_FOR_REPLY) {
			if (dwr->dwr_refs.dmr_reply_port_owned) {
				if (_dispatch_use_mach_special_reply_port()) {
					opts |= MACH_SEND_SYNC_OVERRIDE;
				}
				_dispatch_clear_thread_reply_port(reply_port);
			}
			_dispatch_mach_reply_waiter_register(dm, dwr, reply_port, dmsg);
		}
		kr = mach_msg(msg, opts, msg->msgh_size, 0, MACH_PORT_NULL, 0,
				msg_priority);
		_dispatch_debug("machport[0x%08x]: sent msg id 0x%x, ctxt %p, "
				"opts 0x%x, msg_opts 0x%x, kvoucher 0x%08x, reply on 0x%08x: "
				"%s - 0x%x", msg->msgh_remote_port, msg->msgh_id, dmsg->do_ctxt,
				opts, msg_opts, msg->msgh_voucher_port, reply_port,
				mach_error_string(kr), kr);
		if (unlikely(kr && (msg_opts & DISPATCH_MACH_WAIT_FOR_REPLY))) {
			uint32_t options = DMRU_MUST_SUCCEED | DMRU_REMOVE;
			dispatch_assert(dwr);
			_dispatch_mach_reply_unregister(dm, &dwr->dwr_refs, options);
		}
		if (clear_voucher) {
			if (kr == MACH_SEND_INVALID_VOUCHER && msg->msgh_voucher_port) {
				DISPATCH_CLIENT_CRASH(kr, "Voucher port corruption");
			}
			mach_voucher_t kv;
			kv = _voucher_mach_msg_clear(msg, kvoucher_move_send);
			if (kvoucher_move_send) ipc_kvoucher = kv;
		}
	}
	if (kr == MACH_SEND_TIMED_OUT && (opts & MACH_SEND_TIMEOUT)) {
		if (opts & MACH_SEND_NOTIFY) {
			_dispatch_mach_notification_set_armed(dsrr);
		} else {
			// send kevent must be installed on the manager queue
			dm->dm_needs_mgr = true;
		}
		if (ipc_kvoucher) {
			_dispatch_kvoucher_debug("reuse on re-send", ipc_kvoucher);
			voucher_t ipc_voucher;
			ipc_voucher = _voucher_create_with_priority_and_mach_voucher(
					voucher, dmsg->dmsg_priority, ipc_kvoucher);
			_dispatch_voucher_debug("mach-msg[%p] replace voucher[%p]",
					ipc_voucher, dmsg, voucher);
			if (dmsg->dmsg_voucher) _voucher_release(dmsg->dmsg_voucher);
			dmsg->dmsg_voucher = ipc_voucher;
		}
		goto out;
	} else if (ipc_kvoucher && (kr || !kvoucher_move_send)) {
		_voucher_dealloc_mach_voucher(ipc_kvoucher);
	}
	dispatch_mach_recv_refs_t dmrr = dm->dm_recv_refs;
	if (!(msg_opts & DISPATCH_MACH_WAIT_FOR_REPLY) && !kr && reply_port &&
			!(_dispatch_unote_registered(dmrr) &&
			dmrr->du_ident == reply_port)) {
		_dispatch_mach_reply_kevent_register(dm, reply_port, dmsg);
	}
	if (unlikely(!is_reply && dmsg == dsrr->dmsr_checkin &&
			_dispatch_unote_registered(dsrr))) {
		_dispatch_mach_notification_kevent_unregister(dm);
	}
	if (unlikely(kr)) {
		// Send failed, so reply was never registered <rdar://problem/14309159>
		if (dm->dm_is_xpc && dmsg->do_ctxt) {
			drq = _dispatch_mach_msg_context_async_reply_queue(dm, dmsg->do_ctxt);
		}
		dmsgr = _dispatch_mach_msg_create_reply_disconnected(dmsg,
				dwr ? &dwr->dwr_refs : NULL,
				drq ? DISPATCH_MACH_ASYNC_WAITER_DISCONNECTED
				: DISPATCH_MACH_DISCONNECTED);
	}
	_dispatch_mach_msg_set_reason(dmsg, kr, 0);
	if ((send_flags & DM_SEND_INVOKE_IMMEDIATE_SEND) &&
			(msg_opts & DISPATCH_MACH_RETURN_IMMEDIATE_SEND_RESULT)) {
		// Return sent message synchronously <rdar://problem/25947334>
		send_status |= DM_SEND_STATUS_RETURNING_IMMEDIATE_SEND_RESULT;
	} else {
		_dispatch_mach_handle_or_push_received_msg(dm, dmsg, 0);
	}
	if (dmsgr) {
		if (drq) {
			_dispatch_mach_push_async_reply_msg(dm, dmsgr, drq);
		} else {
			_dispatch_mach_handle_or_push_received_msg(dm, dmsgr, 0);
		}
	}
	send_status |= DM_SEND_STATUS_SUCCESS;
out:
	return send_status;
}

#pragma mark -
#pragma mark dispatch_mach_send_refs_t

#define _dmsr_state_needs_lock_override(dq_state, qos) \
		unlikely(qos < _dq_state_max_qos(dq_state))

DISPATCH_ALWAYS_INLINE
static inline dispatch_qos_t
_dmsr_state_max_qos(uint64_t dmsr_state)
{
	return _dq_state_max_qos(dmsr_state);
}

DISPATCH_ALWAYS_INLINE
static inline bool
_dmsr_state_needs_override(uint64_t dmsr_state, dispatch_qos_t qos)
{
	dmsr_state &= DISPATCH_MACH_STATE_MAX_QOS_MASK;
	return dmsr_state < _dq_state_from_qos(qos);
}

DISPATCH_ALWAYS_INLINE
static inline uint64_t
_dmsr_state_merge_override(uint64_t dmsr_state, dispatch_qos_t qos)
{
	if (_dmsr_state_needs_override(dmsr_state, qos)) {
		dmsr_state &= ~DISPATCH_MACH_STATE_MAX_QOS_MASK;
		dmsr_state |= _dq_state_from_qos(qos);
		dmsr_state |= DISPATCH_MACH_STATE_DIRTY;
		dmsr_state |= DISPATCH_MACH_STATE_RECEIVED_OVERRIDE;
	}
	return dmsr_state;
}

#define _dispatch_mach_send_push_update_tail(dmsr, tail) \
		os_mpsc_push_update_tail(os_mpsc(dmsr, dmsr), tail, do_next)
#define _dispatch_mach_send_push_update_prev(dmsr, prev, head) \
		os_mpsc_push_update_prev(os_mpsc(dmsr, dmsr), prev, head, do_next)
#define _dispatch_mach_send_get_head(dmsr) \
		os_mpsc_get_head(os_mpsc(dmsr, dmsr))
#define _dispatch_mach_send_undo_pop_head(dmsr, dc, dc_next) \
		os_mpsc_undo_pop_head(os_mpsc(dmsr, dmsr), dc, dc_next, do_next)
#define _dispatch_mach_send_pop_head(dmsr, head) \
		os_mpsc_pop_head(os_mpsc(dmsr, dmsr), head, do_next)

#define dm_push(dm, dc, qos) \
		_dispatch_lane_push(dm, dc, qos)

DISPATCH_NOINLINE
static bool
_dispatch_mach_send_drain(dispatch_mach_t dm, dispatch_invoke_flags_t flags,
		dispatch_mach_send_invoke_flags_t send_flags)
{
	dispatch_mach_send_refs_t dmsr = dm->dm_send_refs;
	dispatch_mach_reply_wait_refs_t dwr;
	dispatch_mach_msg_t dmsg;
	struct dispatch_object_s *dc = NULL, *next_dc = NULL;
	dispatch_qos_t qos = _dmsr_state_max_qos(dmsr->dmsr_state);
	uint64_t old_state, new_state;
	uint32_t send_status;
	bool returning_send_result = false;
	dispatch_wakeup_flags_t wflags = 0;

again:
	while (dmsr->dmsr_tail) {
		dc = _dispatch_mach_send_get_head(dmsr);
		do {
			dispatch_mach_send_invoke_flags_t sf = send_flags;
			// Only request immediate send result for the first message
			send_flags &= ~DM_SEND_INVOKE_IMMEDIATE_SEND_MASK;
			next_dc = _dispatch_mach_send_pop_head(dmsr, dc);
			if (_dispatch_object_has_type(dc,
					DISPATCH_CONTINUATION_TYPE(MACH_SEND_BARRIER))) {
				if (!(send_flags & DM_SEND_INVOKE_CAN_RUN_BARRIER)) {
					goto partial_drain;
				}
				_dispatch_continuation_pop(dc, NULL, flags, dm);
				continue;
			}
			if (_dispatch_object_is_sync_waiter(dc)) {
				dmsg = ((dispatch_continuation_t)dc)->dc_data;
				dwr = ((dispatch_continuation_t)dc)->dc_other;
			} else if (_dispatch_object_has_vtable(dc)) {
				dmsg = (dispatch_mach_msg_t)dc;
				dwr = NULL;
			} else {
				if (_dispatch_unote_registered(dmsr) &&
						(_dispatch_queue_get_current() != _dispatch_mgr_q._as_dq)) {
					// send kevent must be uninstalled on the manager queue
					dm->dm_needs_mgr = true;
					wflags |= DISPATCH_WAKEUP_MAKE_DIRTY;
					goto partial_drain;
				}
				if (unlikely(!_dispatch_mach_reconnect_invoke(dm, dc))) {
					goto partial_drain;
				}
				_dispatch_perfmon_workitem_inc();
				continue;
			}
			_dispatch_voucher_ktrace_dmsg_pop(dmsg);
			if (unlikely(dmsr->dmsr_disconnect_cnt ||
					(dm->dq_atomic_flags & DSF_CANCELED))) {
				_dispatch_mach_msg_not_sent(dm, dmsg, dwr);
				_dispatch_perfmon_workitem_inc();
				continue;
			}
			send_status = _dispatch_mach_msg_send(dm, dmsg, dwr, qos, sf);
			if (unlikely(!send_status)) {
				if (dm->dm_needs_mgr) wflags |= DISPATCH_WAKEUP_MAKE_DIRTY;
				goto partial_drain;
			}
			if (send_status & DM_SEND_STATUS_RETURNING_IMMEDIATE_SEND_RESULT) {
				returning_send_result = true;
			}
			_dispatch_perfmon_workitem_inc();
		} while ((dc = next_dc));
	}

	os_atomic_rmw_loop2o(dmsr, dmsr_state, old_state, new_state, release, {
		if (old_state & DISPATCH_MACH_STATE_DIRTY) {
			new_state = old_state;
			new_state &= ~DISPATCH_MACH_STATE_DIRTY;
			new_state &= ~DISPATCH_MACH_STATE_RECEIVED_OVERRIDE;
			new_state &= ~DISPATCH_MACH_STATE_PENDING_BARRIER;
		} else {
			// unlock
			new_state = 0;
		}
	});
	goto out;

partial_drain:
	// if this is not a complete drain, we must undo some things
	_dispatch_mach_send_undo_pop_head(dmsr, dc, next_dc);

	if (_dispatch_object_has_type(dc,
			DISPATCH_CONTINUATION_TYPE(MACH_SEND_BARRIER))) {
		os_atomic_rmw_loop2o(dmsr, dmsr_state, old_state, new_state, release, {
			new_state = old_state;
			new_state |= DISPATCH_MACH_STATE_DIRTY;
			new_state |= DISPATCH_MACH_STATE_PENDING_BARRIER;
			new_state &= ~DISPATCH_MACH_STATE_UNLOCK_MASK;
			new_state &= ~DISPATCH_MACH_STATE_RECEIVED_OVERRIDE;
		});
	} else {
		os_atomic_rmw_loop2o(dmsr, dmsr_state, old_state, new_state, release, {
			new_state = old_state;
			if (old_state & (DISPATCH_MACH_STATE_DIRTY |
					DISPATCH_MACH_STATE_RECEIVED_OVERRIDE)) {
				new_state &= ~DISPATCH_MACH_STATE_DIRTY;
				new_state &= ~DISPATCH_MACH_STATE_RECEIVED_OVERRIDE;
				new_state &= ~DISPATCH_MACH_STATE_PENDING_BARRIER;
			} else {
				new_state |= DISPATCH_MACH_STATE_DIRTY;
				new_state &= ~DISPATCH_MACH_STATE_UNLOCK_MASK;
			}
		});
	}

out:
	if (old_state & DISPATCH_MACH_STATE_RECEIVED_OVERRIDE) {
		// Ensure that the root queue sees that this thread was overridden.
		_dispatch_set_basepri_override_qos(_dmsr_state_max_qos(old_state));
	}

	qos = _dmsr_state_max_qos(new_state);
	if (unlikely(new_state & DISPATCH_MACH_STATE_UNLOCK_MASK)) {
		os_atomic_thread_fence(dependency);
		dmsr = os_atomic_force_dependency_on(dmsr, new_state);
		goto again;
	}

	if (new_state & DISPATCH_MACH_STATE_PENDING_BARRIER) {
		// we don't need to wakeup the mach channel with DISPATCH_WAKEUP_EVENT
		// because a push on the receive queue always causes a wakeup even
		// wen DSF_NEEDS_EVENT is set.
		_dispatch_mach_push_send_barrier_drain(dm, qos);
		return returning_send_result;
	}

	if (new_state == 0 && dm->dm_disconnected && !dm->dm_cancel_handler_called){
		// cancelation waits for the send queue to be empty
		// so when we know cancelation is pending, and we empty the queue,
		// force an EVENT wakeup.
		wflags |= DISPATCH_WAKEUP_EVENT | DISPATCH_WAKEUP_MAKE_DIRTY;
	}
	if ((old_state ^ new_state) & DISPATCH_MACH_STATE_ENQUEUED) {
		if (wflags) {
			wflags |= DISPATCH_WAKEUP_CONSUME_2;
		} else {
			// <rdar://problem/26734097> Note that after this release
			// the mach channel may be gone.
			_dispatch_release_2(dm);
		}
	}
	if (wflags) {
		dx_wakeup(dm, dm->dm_needs_mgr ? qos : 0, wflags);
	}
	return returning_send_result;
}

DISPATCH_NOINLINE
static void
_dispatch_mach_send_invoke(dispatch_mach_t dm, dispatch_invoke_flags_t flags,
		dispatch_mach_send_invoke_flags_t send_flags)
{
	dispatch_mach_send_refs_t dmsr = dm->dm_send_refs;
	dispatch_lock owner_self = _dispatch_lock_value_for_self();
	uint64_t old_state, new_state;

	uint64_t canlock_mask = DISPATCH_MACH_STATE_UNLOCK_MASK;
	uint64_t canlock_state = 0;

	if (send_flags & DM_SEND_INVOKE_NEEDS_BARRIER) {
		canlock_mask |= DISPATCH_MACH_STATE_PENDING_BARRIER;
		canlock_state = DISPATCH_MACH_STATE_PENDING_BARRIER;
	} else if (!(send_flags & DM_SEND_INVOKE_CAN_RUN_BARRIER)) {
		canlock_mask |= DISPATCH_MACH_STATE_PENDING_BARRIER;
	}

	dispatch_qos_t oq_floor = _dispatch_get_basepri_override_qos_floor();
retry:
	os_atomic_rmw_loop2o(dmsr, dmsr_state, old_state, new_state, acquire, {
		new_state = old_state;
		if (unlikely((old_state & canlock_mask) != canlock_state)) {
			if (!(send_flags & DM_SEND_INVOKE_MAKE_DIRTY)) {
				os_atomic_rmw_loop_give_up(break);
			}
			new_state |= DISPATCH_MACH_STATE_DIRTY;
		} else {
			if (_dmsr_state_needs_lock_override(old_state, oq_floor)) {
				os_atomic_rmw_loop_give_up({
					oq_floor = _dispatch_queue_override_self(old_state);
					goto retry;
				});
			}
			new_state |= owner_self;
			new_state &= ~DISPATCH_MACH_STATE_DIRTY;
			new_state &= ~DISPATCH_MACH_STATE_RECEIVED_OVERRIDE;
			new_state &= ~DISPATCH_MACH_STATE_PENDING_BARRIER;
		}
	});

	if (unlikely((old_state & canlock_mask) != canlock_state)) {
		return;
	}
	_dispatch_mach_send_drain(dm, flags, send_flags);
}

DISPATCH_NOINLINE
void
_dispatch_mach_send_barrier_drain_invoke(dispatch_continuation_t dc,
		DISPATCH_UNUSED dispatch_invoke_context_t dic,
		dispatch_invoke_flags_t flags)
{
	dispatch_mach_t dm = upcast(_dispatch_queue_get_current())._dm;
	uintptr_t dc_flags = DC_FLAG_CONSUME | DC_FLAG_NO_INTROSPECTION;
	dispatch_thread_frame_s dtf;

	DISPATCH_COMPILER_CAN_ASSUME(dc->dc_priority == DISPATCH_NO_PRIORITY);
	DISPATCH_COMPILER_CAN_ASSUME(dc->dc_voucher == DISPATCH_NO_VOUCHER);
	// hide the mach channel (see _dispatch_mach_barrier_invoke comment)
	_dispatch_thread_frame_stash(&dtf);
	_dispatch_continuation_pop_forwarded(dc, dc_flags, dm, {
		_dispatch_mach_send_invoke(dm, flags,
				DM_SEND_INVOKE_NEEDS_BARRIER | DM_SEND_INVOKE_CAN_RUN_BARRIER);
	});
	_dispatch_thread_frame_unstash(&dtf);
}

DISPATCH_NOINLINE
static void
_dispatch_mach_push_send_barrier_drain(dispatch_mach_t dm, dispatch_qos_t qos)
{
	dispatch_continuation_t dc = _dispatch_continuation_alloc();

	dc->do_vtable = DC_VTABLE(MACH_SEND_BARRRIER_DRAIN);
	dc->dc_func = NULL;
	dc->dc_ctxt = NULL;
	dc->dc_voucher = DISPATCH_NO_VOUCHER;
	dc->dc_priority = DISPATCH_NO_PRIORITY;
	dm_push(dm, dc, qos);
}

DISPATCH_NOINLINE
static void
_dispatch_mach_send_push(dispatch_mach_t dm, dispatch_object_t dou,
		dispatch_qos_t qos)
{
	dispatch_mach_send_refs_t dmsr = dm->dm_send_refs;
	uint64_t old_state, new_state, state_flags = 0;
	struct dispatch_object_s *prev;
	dispatch_wakeup_flags_t wflags = 0;
	bool is_send_barrier = (dou._dc->do_vtable == DC_VTABLE(MACH_SEND_BARRIER));
	dispatch_tid owner;

	// <rdar://problem/25896179&26266265> the send queue needs to retain
	// the mach channel if not empty, for the whole duration of this call
	//
	// When we may add the ENQUEUED bit, we need to reserve 2 more that we will
	// transfer to _dispatch_mach_send_drain().
	prev = _dispatch_mach_send_push_update_tail(dmsr, dou._do);
	_dispatch_retain_n_unsafe(dm, os_mpsc_push_was_empty(prev) ? 4 : 2);
	_dispatch_mach_send_push_update_prev(dmsr, prev, dou._do);

	if (unlikely(os_mpsc_push_was_empty(prev))) {
		state_flags = DISPATCH_MACH_STATE_DIRTY | DISPATCH_MACH_STATE_ENQUEUED;
		wflags |= DISPATCH_WAKEUP_MAKE_DIRTY;
		if (is_send_barrier) {
			state_flags |= DISPATCH_MACH_STATE_PENDING_BARRIER;
		}

		os_atomic_rmw_loop2o(dmsr, dmsr_state, old_state, new_state, release, {
			new_state = _dmsr_state_merge_override(old_state, qos);
			new_state |= state_flags;
		});
		if ((old_state ^ new_state) & DISPATCH_MACH_STATE_ENQUEUED) {
			// +2 transfered to the ENQUEUED state, _dispatch_mach_send_drain
			// will consume it when clearing the bit.
		} else {
			_dispatch_release_2_no_dispose(dm);
		}
	} else {
		os_atomic_rmw_loop2o(dmsr, dmsr_state, old_state, new_state, relaxed, {
			new_state = _dmsr_state_merge_override(old_state, qos);
			if (old_state == new_state) {
				os_atomic_rmw_loop_give_up(break);
			}
		});
	}


	qos = _dmsr_state_max_qos(new_state);
	owner = _dispatch_lock_owner((dispatch_lock)old_state);
	if (owner) {
		if (_dmsr_state_needs_override(old_state, qos)) {
			_dispatch_wqthread_override_start_check_owner(owner, qos,
					&dmsr->dmsr_state_lock.dul_lock);
		}
	} else if (state_flags & DISPATCH_MACH_STATE_PENDING_BARRIER) {
		_dispatch_mach_push_send_barrier_drain(dm, qos);
	} else if (wflags || dmsr->dmsr_disconnect_cnt ||
			(dm->dq_atomic_flags & DSF_CANCELED)) {
		return dx_wakeup(dm, qos, wflags | DISPATCH_WAKEUP_CONSUME_2);
	} else if (old_state & DISPATCH_MACH_STATE_PENDING_BARRIER) {
		return dx_wakeup(dm, qos, DISPATCH_WAKEUP_CONSUME_2);
	}

	return _dispatch_release_2_tailcall(dm);
}

DISPATCH_NOINLINE
static bool
_dispatch_mach_send_push_and_trydrain(dispatch_mach_t dm,
		dispatch_object_t dou, dispatch_qos_t qos,
		dispatch_mach_send_invoke_flags_t send_flags)
{
	dispatch_mach_send_refs_t dmsr = dm->dm_send_refs;
	dispatch_lock owner_self = _dispatch_lock_value_for_self();
	uint64_t old_state, new_state, canlock_mask, state_flags = 0;
	dispatch_wakeup_flags_t wflags = 0;
	dispatch_tid owner;
	struct dispatch_object_s *prev;

	prev = _dispatch_mach_send_push_update_tail(dmsr, dou._do);
	if (os_mpsc_push_was_empty(prev)) {
		// <rdar://problem/25896179&26266265> the send queue needs to retain
		// the mach channel if not empty.
		_dispatch_retain_2(dm);
		state_flags = DISPATCH_MACH_STATE_DIRTY | DISPATCH_MACH_STATE_ENQUEUED;
		wflags = DISPATCH_WAKEUP_CONSUME_2 | DISPATCH_WAKEUP_MAKE_DIRTY;
	}
	_dispatch_mach_send_push_update_prev(dmsr, prev, dou._do);

	if (unlikely(dmsr->dmsr_disconnect_cnt ||
			(dm->dq_atomic_flags & DSF_CANCELED))) {
		os_atomic_rmw_loop2o(dmsr, dmsr_state, old_state, new_state, release, {
			new_state = _dmsr_state_merge_override(old_state, qos);
			new_state |= state_flags;
		});
		if ((old_state ^ new_state) & DISPATCH_MACH_STATE_ENQUEUED) {
			wflags &= ~(dispatch_wakeup_flags_t)DISPATCH_WAKEUP_CONSUME_2;
		}
		dx_wakeup(dm, qos, wflags);
		return false;
	}

	canlock_mask = DISPATCH_MACH_STATE_UNLOCK_MASK |
			DISPATCH_MACH_STATE_PENDING_BARRIER;
	if (state_flags) {
		os_atomic_rmw_loop2o(dmsr, dmsr_state, old_state, new_state, seq_cst, {
			new_state = _dmsr_state_merge_override(old_state, qos);
			new_state |= state_flags;
			if (likely((old_state & canlock_mask) == 0)) {
				new_state |= owner_self;
				new_state &= ~DISPATCH_MACH_STATE_DIRTY;
				new_state &= ~DISPATCH_MACH_STATE_RECEIVED_OVERRIDE;
				new_state &= ~DISPATCH_MACH_STATE_PENDING_BARRIER;
			}
		});
		if ((old_state ^ new_state) & DISPATCH_MACH_STATE_ENQUEUED) {
			wflags &= ~(dispatch_wakeup_flags_t)DISPATCH_WAKEUP_CONSUME_2;
		}
	} else {
		os_atomic_rmw_loop2o(dmsr, dmsr_state, old_state, new_state, acquire, {
			new_state = _dmsr_state_merge_override(old_state, qos);
			if (new_state == old_state) {
				os_atomic_rmw_loop_give_up(return false);
			}
			if (likely((old_state & canlock_mask) == 0)) {
				new_state |= owner_self;
				new_state &= ~DISPATCH_MACH_STATE_DIRTY;
				new_state &= ~DISPATCH_MACH_STATE_RECEIVED_OVERRIDE;
				new_state &= ~DISPATCH_MACH_STATE_PENDING_BARRIER;
			}
		});
	}

	owner = _dispatch_lock_owner((dispatch_lock)old_state);
	if (owner) {
		if (_dmsr_state_needs_override(old_state, qos)) {
			_dispatch_wqthread_override_start_check_owner(owner, qos,
					&dmsr->dmsr_state_lock.dul_lock);
		}
		if (wflags & DISPATCH_WAKEUP_CONSUME_2) _dispatch_release_2(dm);
		return false;
	}

	if (old_state & DISPATCH_MACH_STATE_PENDING_BARRIER) {
		dx_wakeup(dm, qos, wflags);
		return false;
	}

	// Ensure our message is still at the head of the queue and has not already
	// been dequeued by another thread that raced us to the send queue lock.
	// A plain load of the head and comparison against our object pointer is
	// sufficient.
	if (unlikely(!(wflags && dou._do == dmsr->dmsr_head))) {
		// Don't request immediate send result for messages we don't own
		send_flags &= ~DM_SEND_INVOKE_IMMEDIATE_SEND_MASK;
	}
	if (wflags & DISPATCH_WAKEUP_CONSUME_2) _dispatch_release_2_no_dispose(dm);
	return _dispatch_mach_send_drain(dm, DISPATCH_INVOKE_NONE, send_flags);
}

#pragma mark -
#pragma mark dispatch_mach

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_mach_notification_kevent_unregister(dispatch_mach_t dm)
{
	uint32_t duu_options = DUU_DELETE_ACK | DUU_MUST_SUCCEED;
	DISPATCH_ASSERT_ON_MANAGER_QUEUE();
	_dispatch_unote_unregister(dm->dm_send_refs, duu_options);
	dm->dm_send_refs->du_ident = 0;
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_mach_notification_kevent_register(dispatch_mach_t dm,mach_port_t send)
{
	DISPATCH_ASSERT_ON_MANAGER_QUEUE();
	dm->dm_send_refs->du_ident = send;
	dispatch_assume(_dispatch_unote_register(dm->dm_send_refs,
			DISPATCH_WLH_ANON, DISPATCH_PRIORITY_FLAG_MANAGER));
}

void
_dispatch_mach_notification_merge_evt(dispatch_unote_t du,
		uint32_t flags DISPATCH_UNUSED, uintptr_t data,
		pthread_priority_t pp DISPATCH_UNUSED)
{
	dispatch_mach_send_refs_t dmsr = du._dmsr;
	dispatch_mach_t dm = _dispatch_wref2ptr(dmsr->du_owner_wref);

	if (data & dmsr->du_fflags) {
		_dispatch_mach_send_invoke(dm, DISPATCH_INVOKE_MANAGER_DRAIN,
				DM_SEND_INVOKE_MAKE_DIRTY);
	}
	_dispatch_release_2_tailcall(dm);
}

DISPATCH_NOINLINE
static void
_dispatch_mach_handle_or_push_received_msg(dispatch_mach_t dm,
		dispatch_mach_msg_t dmsg, pthread_priority_t pp)
{
	mach_error_t error;
	dispatch_mach_reason_t reason = _dispatch_mach_msg_get_reason(dmsg, &error);
	dispatch_qos_t qos;

	if (reason == DISPATCH_MACH_MESSAGE_RECEIVED || !dm->dm_is_xpc ||
			!_dispatch_mach_xpc_hooks->dmxh_direct_message_handler(
			dm->dm_recv_refs->dmrr_handler_ctxt, reason, dmsg, error)) {
		// Not XPC client or not a message that XPC can handle inline - push
		// it onto the channel queue.
		_dispatch_trace_item_push(dm, dmsg);
		qos = _dispatch_qos_from_pp(pp);
		if (!qos) qos = _dispatch_priority_qos(dm->dq_priority);
		dm_push(dm, dmsg, qos);
	} else {
		// XPC handled the message inline. Do the cleanup that would otherwise
		// have happened in _dispatch_mach_msg_invoke(), leaving out steps that
		// are not required in this context.
		dmsg->do_next = DISPATCH_OBJECT_LISTLESS;
		dispatch_release(dmsg);
	}
}

DISPATCH_ALWAYS_INLINE
static void
_dispatch_mach_push_async_reply_msg(dispatch_mach_t dm,
		dispatch_mach_msg_t dmsg, dispatch_queue_t drq)
{
	// Push the message onto the given queue. This function is only used for
	// replies to messages sent by
	// dispatch_mach_send_with_result_and_async_reply_4libxpc().
	dispatch_continuation_t dc = _dispatch_mach_msg_async_reply_wrap(dmsg, dm);
	_dispatch_trace_item_push(drq, dc);
	dx_push(drq, dc, _dispatch_qos_from_pp(dmsg->dmsg_priority));
}

#pragma mark -
#pragma mark dispatch_mach_t

static inline mach_msg_option_t
_dispatch_mach_checkin_options(void)
{
	mach_msg_option_t options = 0;
#if DISPATCH_USE_CHECKIN_NOIMPORTANCE
	options = MACH_SEND_NOIMPORTANCE; // <rdar://problem/16996737>
#endif
	return options;
}



static inline mach_msg_option_t
_dispatch_mach_send_options(void)
{
	mach_msg_option_t options = 0;
	return options;
}

DISPATCH_ALWAYS_INLINE
static inline mach_msg_option_t
_dispatch_mach_send_msg_prepare(dispatch_mach_t dm,
		dispatch_mach_msg_t dmsg, mach_msg_option_t options)
{
#if DISPATCH_DEBUG
	if (dm->dm_is_xpc && (options & DISPATCH_MACH_WAIT_FOR_REPLY) == 0 &&
			_dispatch_mach_msg_get_reply_port(dmsg)) {
		dispatch_assert(
				_dispatch_mach_msg_context_async_reply_queue(dm, dmsg->do_ctxt));
	}
#else
	(void)dm;
#endif
	if (DISPATCH_USE_NOIMPORTANCE_QOS && (options & MACH_SEND_NOIMPORTANCE)) {
		dmsg->dmsg_priority = 0;
	} else {
		unsigned int flags = DISPATCH_PRIORITY_PROPAGATE_CURRENT;
		if ((options & DISPATCH_MACH_WAIT_FOR_REPLY) &&
				_dispatch_use_mach_special_reply_port()) {
			// TODO: remove QoS contribution of sync IPC messages to send queue
			// rdar://31848737
			flags |= DISPATCH_PRIORITY_PROPAGATE_FOR_SYNC_IPC;
		}
		dmsg->dmsg_priority = _dispatch_priority_compute_propagated(0, flags);
	}
	dmsg->dmsg_voucher = _voucher_copy();
	_dispatch_voucher_debug("mach-msg[%p] set", dmsg->dmsg_voucher, dmsg);
	options |= _dispatch_mach_send_options();
	dmsg->dmsg_options = options;
	return options;
}

DISPATCH_NOINLINE
static bool
_dispatch_mach_send_msg(dispatch_mach_t dm, dispatch_mach_msg_t dmsg,
		dispatch_continuation_t dc_wait, mach_msg_option_t options)
{
	dispatch_mach_send_refs_t dmsr = dm->dm_send_refs;
	if (unlikely(dmsg->do_next != DISPATCH_OBJECT_LISTLESS)) {
		DISPATCH_CLIENT_CRASH(dmsg->do_next, "Message already enqueued");
	}
	options = _dispatch_mach_send_msg_prepare(dm, dmsg, options);
	dispatch_retain(dmsg);
	dispatch_qos_t qos = _dispatch_qos_from_pp(dmsg->dmsg_priority);
	mach_msg_header_t *msg = _dispatch_mach_msg_get_msg(dmsg);
	dmsg->dmsg_reply = _dispatch_mach_msg_get_reply_port(dmsg);
	bool is_reply = (MACH_MSGH_BITS_REMOTE(msg->msgh_bits) ==
			MACH_MSG_TYPE_MOVE_SEND_ONCE);

	uint32_t send_status;
	bool returning_send_result = false;
	dispatch_mach_send_invoke_flags_t send_flags = DM_SEND_INVOKE_NONE;
	if (options & DISPATCH_MACH_RETURN_IMMEDIATE_SEND_RESULT) {
		send_flags = DM_SEND_INVOKE_IMMEDIATE_SEND;
	}
	if (is_reply && !dmsg->dmsg_reply && !dmsr->dmsr_disconnect_cnt &&
			!(dm->dq_atomic_flags & DSF_CANCELED)) {
		// replies are sent to a send-once right and don't need the send queue
		dispatch_assert(!dc_wait);
		send_status = _dispatch_mach_msg_send(dm, dmsg, NULL, 0, send_flags);
		dispatch_assert(send_status);
		returning_send_result = !!(send_status &
				DM_SEND_STATUS_RETURNING_IMMEDIATE_SEND_RESULT);
	} else {
		_dispatch_voucher_ktrace_dmsg_push(dmsg);
		dispatch_object_t dou = { ._dmsg = dmsg };
		if (dc_wait) dou._dc = dc_wait;
		returning_send_result = _dispatch_mach_send_push_and_trydrain(dm, dou,
				qos, send_flags);
	}
	if (returning_send_result) {
		_dispatch_voucher_debug("mach-msg[%p] clear", dmsg->dmsg_voucher, dmsg);
		if (dmsg->dmsg_voucher) _voucher_release(dmsg->dmsg_voucher);
		dmsg->dmsg_voucher = NULL;
		dmsg->do_next = DISPATCH_OBJECT_LISTLESS;
		dispatch_release(dmsg);
	}
	return returning_send_result;
}

DISPATCH_NOINLINE
void
dispatch_mach_send(dispatch_mach_t dm, dispatch_mach_msg_t dmsg,
		mach_msg_option_t options)
{
	dispatch_assert_zero(options & DISPATCH_MACH_OPTIONS_MASK);
	options &= ~DISPATCH_MACH_OPTIONS_MASK;
	bool returned_send_result = _dispatch_mach_send_msg(dm, dmsg, NULL, options);
	dispatch_assert(!returned_send_result);
}

DISPATCH_NOINLINE
void
dispatch_mach_send_with_result(dispatch_mach_t dm, dispatch_mach_msg_t dmsg,
		mach_msg_option_t options, dispatch_mach_send_flags_t send_flags,
		dispatch_mach_reason_t *send_result, mach_error_t *send_error)
{
	if (unlikely(send_flags != DISPATCH_MACH_SEND_DEFAULT)) {
		DISPATCH_CLIENT_CRASH(send_flags, "Invalid send flags");
	}
	dispatch_assert_zero(options & DISPATCH_MACH_OPTIONS_MASK);
	options &= ~DISPATCH_MACH_OPTIONS_MASK;
	options |= DISPATCH_MACH_RETURN_IMMEDIATE_SEND_RESULT;
	bool returned_send_result = _dispatch_mach_send_msg(dm, dmsg, NULL,options);
	unsigned long reason = DISPATCH_MACH_NEEDS_DEFERRED_SEND;
	mach_error_t err = 0;
	if (returned_send_result) {
		reason = _dispatch_mach_msg_get_reason(dmsg, &err);
	}
	*send_result = reason;
	*send_error = err;
}

static inline
dispatch_mach_msg_t
_dispatch_mach_send_and_wait_for_reply(dispatch_mach_t dm,
		dispatch_mach_msg_t dmsg, mach_msg_option_t options,
		bool *returned_send_result)
{
	struct dispatch_mach_reply_wait_refs_s dwr_buf = {
		.dwr_refs = {
			.du_type = DISPATCH_MACH_TYPE_WAITER,
			.dmr_ctxt = dmsg->do_ctxt,
		},
		.dwr_waiter_tid = _dispatch_tid_self(),
	};
	dispatch_mach_reply_wait_refs_t dwr = &dwr_buf;
	mach_port_t send = MACH_PORT_NULL;
	mach_port_t reply_port = _dispatch_mach_msg_get_reply_port(dmsg);

	if (likely(!reply_port)) {
		// use per-thread mach reply port <rdar://24597802>
		reply_port = _dispatch_get_thread_reply_port();
		mach_msg_header_t *hdr = _dispatch_mach_msg_get_msg(dmsg);
		dispatch_assert(MACH_MSGH_BITS_LOCAL(hdr->msgh_bits) ==
				MACH_MSG_TYPE_MAKE_SEND_ONCE);
		hdr->msgh_local_port = reply_port;
		dwr->dwr_refs.dmr_reply_port_owned = true;
	}
	options |= DISPATCH_MACH_WAIT_FOR_REPLY;

#if DISPATCH_DEBUG
	dwr = _dispatch_calloc(1, sizeof(*dwr));
	*dwr = dwr_buf;
#endif
	struct dispatch_continuation_s dc_wait = {
		.dc_flags = DC_FLAG_SYNC_WAITER,
		.dc_data = dmsg,
		.dc_other = &dwr->dwr_refs,
		.dc_priority = DISPATCH_NO_PRIORITY,
		.dc_voucher = DISPATCH_NO_VOUCHER,
	};
	*returned_send_result = _dispatch_mach_send_msg(dm, dmsg, &dc_wait,options);
	if (dwr->dwr_refs.dmr_reply_port_owned) {
		_dispatch_clear_thread_reply_port(reply_port);
		if (_dispatch_use_mach_special_reply_port()) {
			// link special reply port to send right for remote receive right
			// TODO: extend to pre-connect phase <rdar://problem/31823384>
			send = dm->dm_send_refs->dmsr_send;
		}
	}
	dmsg = _dispatch_mach_msg_reply_recv(dm, dwr, reply_port, send);
#if DISPATCH_DEBUG
	free(dwr);
#endif
	return dmsg;
}

DISPATCH_NOINLINE
dispatch_mach_msg_t
dispatch_mach_send_and_wait_for_reply(dispatch_mach_t dm,
		dispatch_mach_msg_t dmsg, mach_msg_option_t options)
{
	bool returned_send_result;
	dispatch_mach_msg_t reply;
	dispatch_assert_zero(options & DISPATCH_MACH_OPTIONS_MASK);
	options &= ~DISPATCH_MACH_OPTIONS_MASK;
	reply = _dispatch_mach_send_and_wait_for_reply(dm, dmsg, options,
			&returned_send_result);
	dispatch_assert(!returned_send_result);
	return reply;
}

DISPATCH_NOINLINE
dispatch_mach_msg_t
dispatch_mach_send_with_result_and_wait_for_reply(dispatch_mach_t dm,
		dispatch_mach_msg_t dmsg, mach_msg_option_t options,
		dispatch_mach_send_flags_t send_flags,
		dispatch_mach_reason_t *send_result, mach_error_t *send_error)
{
	if (unlikely(send_flags != DISPATCH_MACH_SEND_DEFAULT)) {
		DISPATCH_CLIENT_CRASH(send_flags, "Invalid send flags");
	}
	bool returned_send_result;
	dispatch_mach_msg_t reply;
	dispatch_assert_zero(options & DISPATCH_MACH_OPTIONS_MASK);
	options &= ~DISPATCH_MACH_OPTIONS_MASK;
	options |= DISPATCH_MACH_RETURN_IMMEDIATE_SEND_RESULT;
	reply = _dispatch_mach_send_and_wait_for_reply(dm, dmsg, options,
			&returned_send_result);
	unsigned long reason = DISPATCH_MACH_NEEDS_DEFERRED_SEND;
	mach_error_t err = 0;
	if (returned_send_result) {
		reason = _dispatch_mach_msg_get_reason(dmsg, &err);
	}
	*send_result = reason;
	*send_error = err;
	return reply;
}

DISPATCH_NOINLINE
void
dispatch_mach_send_with_result_and_async_reply_4libxpc(dispatch_mach_t dm,
		dispatch_mach_msg_t dmsg, mach_msg_option_t options,
		dispatch_mach_send_flags_t send_flags,
		dispatch_mach_reason_t *send_result, mach_error_t *send_error)
{
	if (unlikely(send_flags != DISPATCH_MACH_SEND_DEFAULT)) {
		DISPATCH_CLIENT_CRASH(send_flags, "Invalid send flags");
	}
	if (unlikely(!dm->dm_is_xpc)) {
		DISPATCH_CLIENT_CRASH(0,
			"dispatch_mach_send_with_result_and_wait_for_reply is XPC only");
	}

	dispatch_assert_zero(options & DISPATCH_MACH_OPTIONS_MASK);
	options &= ~DISPATCH_MACH_OPTIONS_MASK;
	options |= DISPATCH_MACH_RETURN_IMMEDIATE_SEND_RESULT;
	mach_port_t reply_port = _dispatch_mach_msg_get_reply_port(dmsg);
	if (!reply_port) {
		DISPATCH_CLIENT_CRASH(0, "Reply port needed for async send with reply");
	}
	bool returned_send_result = _dispatch_mach_send_msg(dm, dmsg, NULL,options);
	unsigned long reason = DISPATCH_MACH_NEEDS_DEFERRED_SEND;
	mach_error_t err = 0;
	if (returned_send_result) {
		reason = _dispatch_mach_msg_get_reason(dmsg, &err);
	}
	*send_result = reason;
	*send_error = err;
}

DISPATCH_NOINLINE
static bool
_dispatch_mach_cancel(dispatch_mach_t dm)
{
	bool uninstalled = dm->dm_disconnected;
	if (dm->dm_send_refs->dmsr_disconnect_cnt) {
		uninstalled = false; // <rdar://problem/31233110>
	}

	_dispatch_object_debug(dm, "%s", __func__);

	uint32_t duu_options = DMRU_DELETE_ACK;
	if (!(_dispatch_queue_atomic_flags(dm) & DSF_NEEDS_EVENT)) {
		duu_options |= DMRU_PROBE;
	}

	dispatch_xpc_term_refs_t dxtr = dm->dm_xpc_term_refs;
	if (dxtr && !_dispatch_unote_unregister(dxtr, duu_options)) {
		uninstalled = false;
	}

	dispatch_mach_recv_refs_t dmrr = dm->dm_recv_refs;
	mach_port_t local_port = (mach_port_t)dmrr->du_ident;
	if (local_port) {
		// handle the deferred delete case properly, similar to what
		// _dispatch_source_invoke2() does
		if (_dispatch_unote_unregister(dmrr, duu_options)) {
			_dispatch_mach_msg_disconnected(dm, local_port, MACH_PORT_NULL);
			dmrr->du_ident = 0;
		} else {
			uninstalled = false;
		}
	}

	if (uninstalled) {
		dispatch_queue_flags_t dqf;
		dqf = _dispatch_queue_atomic_flags_set_and_clear_orig(dm,
				DSF_DELETED, DSF_NEEDS_EVENT);
		if (unlikely(dqf & (DSF_DELETED | DSF_CANCEL_WAITER))) {
			DISPATCH_CLIENT_CRASH(dqf, "Corrupt channel state");
		}
		_dispatch_release_no_dispose(dm); // see _dispatch_queue_alloc()
	} else {
		_dispatch_queue_atomic_flags_set(dm, DSF_NEEDS_EVENT);
	}
	return uninstalled;
}

DISPATCH_NOINLINE
static bool
_dispatch_mach_reconnect_invoke(dispatch_mach_t dm, dispatch_object_t dou)
{
	_dispatch_object_debug(dm, "%s", __func__);

	// 1. handle the send-possible notification and checkin message

	dispatch_mach_send_refs_t dmsr = dm->dm_send_refs;
	if (_dispatch_unote_registered(dmsr)) {
		_dispatch_mach_notification_kevent_unregister(dm);
	}
	if (MACH_PORT_VALID(dmsr->dmsr_send)) {
		_dispatch_mach_msg_disconnected(dm, MACH_PORT_NULL, dmsr->dmsr_send);
		dmsr->dmsr_send = MACH_PORT_NULL;
	}
	if (dmsr->dmsr_checkin) {
		_dispatch_mach_msg_not_sent(dm, dmsr->dmsr_checkin, NULL);
		dmsr->dmsr_checkin = NULL;
	}
	dm->dm_needs_mgr = 0;

	// 2. cancel all pending replies and break out synchronous waiters

	dispatch_mach_reply_refs_t dmr, tmp;
	LIST_HEAD(, dispatch_mach_reply_refs_s) replies =
			LIST_HEAD_INITIALIZER(replies);
	bool disconnected;

	// _dispatch_mach_reply_merge_msg is the one passing DMRU_DELETE_ACK
	uint32_t dmru_options = DMRU_CANCEL | DMRU_DISCONNECTED;
	if (!(_dispatch_queue_atomic_flags(dm) & DSF_NEEDS_EVENT)) {
		dmru_options |= DMRU_PROBE;
	}

	_dispatch_unfair_lock_lock(&dmsr->dmsr_replies_lock);
	LIST_SWAP(&replies, &dmsr->dmsr_replies,
			dispatch_mach_reply_refs_s, dmr_list);
	LIST_FOREACH_SAFE(dmr, &replies, dmr_list, tmp) {
		_LIST_MARK_NOT_ENQUEUED(dmr, dmr_list);
		_dispatch_mach_reply_unregister(dm, dmr, dmru_options);
	}
	// any unote unregistration that fails is put back on the reply list
	disconnected = LIST_EMPTY(&dmsr->dmsr_replies);
	_dispatch_unfair_lock_unlock(&dmsr->dmsr_replies_lock);

	// 3. if no reply is left pending deferred deletion, finish reconnecting

	if (disconnected) {
		mach_port_t dmsr_send = (mach_port_t)dou._dc->dc_other;
		dispatch_mach_msg_t dmsr_checkin = dou._dc->dc_data;

		_dispatch_continuation_free(dou._dc);
		if (dmsr_checkin == DM_CHECKIN_CANCELED) {
			dm->dm_disconnected = true;
			dmsr_checkin = NULL;
		}
		if (dm->dm_disconnected) {
			if (MACH_PORT_VALID(dmsr_send)) {
				_dispatch_mach_msg_disconnected(dm, MACH_PORT_NULL, dmsr_send);
			}
			if (dmsr_checkin) {
				_dispatch_mach_msg_not_sent(dm, dmsr_checkin, NULL);
			}
		} else {
			dmsr->dmsr_send = dmsr_send;
			dmsr->dmsr_checkin = dmsr_checkin;
		}
		(void)os_atomic_dec2o(dmsr, dmsr_disconnect_cnt, relaxed);
	}
	return disconnected;
}

DISPATCH_NOINLINE
void
dispatch_mach_reconnect(dispatch_mach_t dm, mach_port_t send,
		dispatch_mach_msg_t checkin)
{
	dispatch_mach_send_refs_t dmsr = dm->dm_send_refs;
	(void)os_atomic_inc2o(dmsr, dmsr_disconnect_cnt, relaxed);
	if (MACH_PORT_VALID(send) && checkin) {
		dispatch_mach_msg_t dmsg = checkin;
		dispatch_retain(dmsg);
		dmsg->dmsg_options = _dispatch_mach_checkin_options();
		dmsr->dmsr_checkin_port = _dispatch_mach_msg_get_remote_port(dmsg);
	} else {
		if (checkin != DM_CHECKIN_CANCELED) checkin = NULL;
		dmsr->dmsr_checkin_port = MACH_PORT_NULL;
	}
	dispatch_continuation_t dc = _dispatch_continuation_alloc();
	dc->dc_flags = DC_FLAG_CONSUME | DC_FLAG_ALLOCATED;
	// actually called manually in _dispatch_mach_send_drain
	dc->dc_func = (void*)_dispatch_mach_reconnect_invoke;
	dc->dc_ctxt = dc;
	dc->dc_data = checkin;
	dc->dc_other = (void*)(uintptr_t)send;
	dc->dc_voucher = DISPATCH_NO_VOUCHER;
	dc->dc_priority = DISPATCH_NO_PRIORITY;
	return _dispatch_mach_send_push(dm, dc, 0);
}

DISPATCH_NOINLINE
mach_port_t
dispatch_mach_get_checkin_port(dispatch_mach_t dm)
{
	dispatch_mach_send_refs_t dmsr = dm->dm_send_refs;
	if (unlikely(dm->dq_atomic_flags & DSF_CANCELED)) {
		return MACH_PORT_DEAD;
	}
	return dmsr->dmsr_checkin_port;
}

DISPATCH_NOINLINE
static void
_dispatch_mach_connect_invoke(dispatch_mach_t dm)
{
	dispatch_mach_recv_refs_t dmrr = dm->dm_recv_refs;
	_dispatch_client_callout4(dmrr->dmrr_handler_ctxt,
			DISPATCH_MACH_CONNECTED, NULL, 0, dmrr->dmrr_handler_func);
	dm->dm_connect_handler_called = 1;
	_dispatch_perfmon_workitem_inc();
}

typedef struct dispatch_ipc_handoff_s {
	struct dispatch_continuation_s dih_dc;
	uint64_t _Atomic dih_wlh;
	int32_t dih_refcnt;
} dispatch_ipc_handoff_s, *dispatch_ipc_handoff_t;

typedef struct _dispatch_ipc_handoff_context_s {
	dispatch_thread_context_s dihc_dtc;
	dispatch_queue_t dihc_dq;
	dispatch_qos_t dihc_qos;
} _dispatch_ipc_handoff_context_s, *_dispatch_ipc_handoff_ctxt_t;

static char const * const
_dispatch_mach_msg_context_key = "mach_msg";

static _dispatch_ipc_handoff_ctxt_t
_dispatch_mach_handoff_context(mach_port_t port)
{
	dispatch_thread_context_t dtc;
	_dispatch_ipc_handoff_ctxt_t dihc = NULL;
	dispatch_ipc_handoff_t dih;

	dtc = _dispatch_thread_context_find(_dispatch_mach_msg_context_key);
	if (dtc && dtc->dtc_dmsg) {
		/*
		 * We need one refcount per async() done,
		 * and one for the whole chain.
		 */
		dihc = (_dispatch_ipc_handoff_ctxt_t)dtc;
		if (dx_type(dtc->dtc_dmsg) == DISPATCH_MACH_MSG_TYPE) {
			dtc->dtc_dih = _dispatch_calloc(1, sizeof(dispatch_ipc_handoff_s));
			dih = dtc->dtc_dih;
			os_atomic_store(&dih->dih_refcnt, 1, relaxed);
		} else {
			dih = dtc->dtc_dih;
			os_atomic_inc(&dih->dih_refcnt, relaxed);
		}
		if (dih->dih_dc.dc_other) {
			DISPATCH_CLIENT_CRASH(0, "Calling dispatch_mach_handoff_reply "
					"multiple times from the same context");
		}
	} else  {
		DISPATCH_CLIENT_CRASH(0, "Trying to handoff IPC from non IPC context");
	}

	dih->dih_dc.dc_other = (void *)(uintptr_t)port;
	return dihc;
}

static void
_dispatch_ipc_handoff_release(dispatch_ipc_handoff_t dih)
{
	if (os_atomic_dec_orig(&dih->dih_refcnt, relaxed) == 0) {
		free(dih);
	}
}

static void
_dispatch_mach_handoff_set_wlh(dispatch_ipc_handoff_t dih, dispatch_queue_t dq)
{
	while (likely(dq->do_targetq)) {
		if (unlikely(_dispatch_queue_is_mutable(dq))) {
			_dispatch_queue_sidelock_lock(upcast(dq)._dl);
			_dispatch_queue_atomic_flags_clear(dq, DQF_MUTABLE);
			_dispatch_queue_sidelock_unlock(upcast(dq)._dl);
		}
		if (_dq_state_is_base_wlh(dq->dq_state)) {
			os_atomic_store(&dih->dih_wlh, (uint64_t)dq, relaxed);
			return;
		}
		dq = dq->do_targetq;
	}

	/* unsupported hierarchy */
	os_atomic_store(&dih->dih_wlh, 0, relaxed);
}

void
dispatch_mach_handoff_reply_f(dispatch_queue_t dq,
		mach_port_t port, void *ctxt, dispatch_function_t func)
{
	_dispatch_ipc_handoff_ctxt_t dihc = _dispatch_mach_handoff_context(port);
	dispatch_ipc_handoff_t dih = dihc->dihc_dtc.dtc_dih;
	dispatch_continuation_t dc = &dih->dih_dc;
	uintptr_t dc_flags = DC_FLAG_CONSUME;

	_dispatch_mach_handoff_set_wlh(dih, dq);
	_dispatch_retain(dq);
	dihc->dihc_dq = dq;
	dihc->dihc_qos = _dispatch_continuation_init_f(dc, dq, ctxt, func,
			0, dc_flags);
	dc->do_vtable = DC_VTABLE(MACH_IPC_HANDOFF);
}

void
dispatch_mach_handoff_reply(dispatch_queue_t dq,
		mach_port_t port, dispatch_block_t block)
{
	_dispatch_ipc_handoff_ctxt_t dihc = _dispatch_mach_handoff_context(port);
	dispatch_ipc_handoff_t dih = dihc->dihc_dtc.dtc_dih;
	dispatch_continuation_t dc = &dih->dih_dc;
	uintptr_t dc_flags = DC_FLAG_CONSUME;

	_dispatch_mach_handoff_set_wlh(dih, dq);
	_dispatch_retain(dq);
	dihc->dihc_dq = dq;
	dihc->dihc_qos = _dispatch_continuation_init(dc, dq, block, 0, dc_flags);
	dc->dc_data = (void *)dc->dc_flags;
	dc->do_vtable = DC_VTABLE(MACH_IPC_HANDOFF);
}

static void
_dispatch_mach_ipc_handoff_async(_dispatch_ipc_handoff_ctxt_t dihc)
{
	dispatch_ipc_handoff_t dih = dihc->dihc_dtc.dtc_dih;
	dispatch_continuation_t dc = &dih->dih_dc;
	mach_port_t port = (mach_port_t)(uintptr_t)dc->dc_other;
	uint64_t wlh = os_atomic_load(&dih->dih_wlh, relaxed);

	_dispatch_continuation_async(dihc->dihc_dq, dc, dihc->dihc_qos,
			(uintptr_t)dc->dc_data);

	if (wlh) {
		_dispatch_sync_ipc_handoff_begin((dispatch_wlh_t)wlh, port,
				&dih->dih_wlh);
		os_atomic_cmpxchg(&dih->dih_wlh, wlh, ~wlh, relaxed);
	}

	_dispatch_ipc_handoff_release(dih);
	_dispatch_release_tailcall(dihc->dihc_dq);
}

void
_dispatch_mach_ipc_handoff_invoke(dispatch_continuation_t dc,
		dispatch_invoke_context_t dic DISPATCH_UNUSED,
		dispatch_invoke_flags_t flags)
{
	dispatch_ipc_handoff_t dih = (dispatch_ipc_handoff_t)dc;
	_dispatch_ipc_handoff_context_s dihc = { .dihc_dtc = {
		.dtc_key = _dispatch_mach_msg_context_key,
		.dtc_dih = dih,
	} };

	dispatch_queue_t cq = _dispatch_queue_get_current();
	uintptr_t dc_flags = (uintptr_t)dc->dc_data;
	mach_port_t port = (mach_port_t)(uintptr_t)dc->dc_other;
	uint64_t wlh = os_atomic_xchg(&dih->dih_wlh, 0, relaxed);

	if (wlh == 0) {
		/* not supported */
	} else if (wlh & 1) {
		/* _dispatch_mach_ipc_handoff_async finished its work */
		wlh = ~wlh;
	} else {
		/*
		 * Because this code may race with _dispatch_mach_ipc_handoff_async,
		 * Make sure that we have the push.
		 *
		 * Then mark the handoff as done, as the client callout below
		 * may consume the send once, and _dispatch_mach_ipc_handoff_async
		 * may be about an invalid port now.
		 */
		_dispatch_sync_ipc_handoff_begin((dispatch_wlh_t)wlh, port,
				&dih->dih_wlh);
	}

	dc->do_next = DISPATCH_OBJECT_LISTLESS;
	dc->dc_other = NULL;

	_dispatch_thread_context_push(&dihc.dihc_dtc);

	// DC_FLAG_CONSUME has been set, as we want the block and vouchers
	// to be consumed, however the continuation is not from the continuation
	// cache and its lifetime is managed explicitly by the handoff mechanism.
	DISPATCH_COMPILER_CAN_ASSUME(dc_flags & DC_FLAG_CONSUME);
	_dispatch_continuation_pop_forwarded_no_free(dc, dc_flags, cq, {
		dispatch_invoke_with_autoreleasepool(flags, {
			_dispatch_client_callout(dc->dc_ctxt, dc->dc_func);
		});
	});

	_dispatch_thread_context_pop(&dihc.dihc_dtc);

	if (dihc.dihc_dq) {
		/* a new handoff was started */
		_dispatch_mach_ipc_handoff_async(&dihc);
	} else {
		/* this was the last handoff in the chain, consume the last ref */
		_dispatch_ipc_handoff_release(dih);
	}

	if (wlh) {
		_dispatch_sync_ipc_handoff_end((dispatch_wlh_t)wlh, port);
	}
}

DISPATCH_ALWAYS_INLINE
static void
_dispatch_mach_msg_invoke_with_mach(dispatch_mach_msg_t dmsg,
		dispatch_invoke_flags_t flags, dispatch_mach_t dm)
{
	dispatch_mach_recv_refs_t dmrr;
	mach_error_t err;
	unsigned long reason = _dispatch_mach_msg_get_reason(dmsg, &err);
	dispatch_thread_set_self_t adopt_flags = DISPATCH_PRIORITY_ENFORCE|
			DISPATCH_VOUCHER_CONSUME|DISPATCH_VOUCHER_REPLACE;
	_dispatch_ipc_handoff_context_s dihc = { .dihc_dtc = {
		.dtc_key = _dispatch_mach_msg_context_key,
		.dtc_dmsg = dmsg,
	} };

	_dispatch_thread_context_push(&dihc.dihc_dtc);
	_dispatch_trace_item_pop(dm, dmsg);

	dmrr = dm->dm_recv_refs;
	dmsg->do_next = DISPATCH_OBJECT_LISTLESS;
	_dispatch_voucher_ktrace_dmsg_pop(dmsg);
	_dispatch_voucher_debug("mach-msg[%p] adopt", dmsg->dmsg_voucher, dmsg);
	(void)_dispatch_adopt_priority_and_set_voucher(dmsg->dmsg_priority,
			dmsg->dmsg_voucher, adopt_flags);
	dmsg->dmsg_voucher = NULL;
	dispatch_invoke_with_autoreleasepool(flags, {
		if (flags & DISPATCH_INVOKE_ASYNC_REPLY) {
			_dispatch_client_callout3(dmrr->dmrr_handler_ctxt, reason, dmsg,
					_dispatch_mach_xpc_hooks->dmxh_async_reply_handler);
		} else {
			if (unlikely(!dm->dm_connect_handler_called)) {
				_dispatch_mach_connect_invoke(dm);
			}
			if (reason == DISPATCH_MACH_MESSAGE_RECEIVED &&
					(_dispatch_queue_atomic_flags(dm) & DSF_CANCELED)) {
				// <rdar://problem/32184699> Do not deliver message received
				// after cancellation: _dispatch_mach_merge_msg can be preempted
				// for a long time right after disarming the unote but before
				// enqueuing the message, allowing for cancellation to complete,
				// and then the message event to be delivered.
				//
				// This makes XPC unhappy because some of these messages are
				// port-destroyed notifications that can cause it to try to
				// reconnect on a channel that is almost fully canceled
				mach_msg_header_t *hdr = _dispatch_mach_msg_get_msg(dmsg);
				_dispatch_debug("machport[0x%08x]: drop msg id 0x%x, reply on 0x%08x",
						hdr->msgh_local_port, hdr->msgh_id, hdr->msgh_remote_port);
				mach_msg_destroy(hdr);
			} else {
				_dispatch_client_callout4(dmrr->dmrr_handler_ctxt, reason, dmsg,
						err, dmrr->dmrr_handler_func);
			}
		}
		_dispatch_perfmon_workitem_inc();
	});
	_dispatch_trace_item_complete(dmsg);
	dispatch_release(dmsg);
	_dispatch_thread_context_pop(&dihc.dihc_dtc);

	if (dihc.dihc_dq) {
		_dispatch_mach_ipc_handoff_async(&dihc);
	}
}

DISPATCH_NOINLINE
void
_dispatch_mach_msg_invoke(dispatch_mach_msg_t dmsg,
		DISPATCH_UNUSED dispatch_invoke_context_t dic,
		dispatch_invoke_flags_t flags)
{
	dispatch_thread_frame_s dtf;

	// hide mach channel
	dispatch_mach_t dm = upcast(_dispatch_thread_frame_stash(&dtf))._dm;
	_dispatch_mach_msg_invoke_with_mach(dmsg, flags, dm);
	_dispatch_thread_frame_unstash(&dtf);
}

DISPATCH_NOINLINE
void
_dispatch_mach_barrier_invoke(dispatch_continuation_t dc,
		DISPATCH_UNUSED dispatch_invoke_context_t dic,
		dispatch_invoke_flags_t flags)
{
	dispatch_thread_frame_s dtf;
	dispatch_mach_t dm = dc->dc_other;
	dispatch_mach_recv_refs_t dmrr;
	uintptr_t dc_flags = (uintptr_t)dc->dc_data;
	unsigned long type = dc_type(dc);

	// hide mach channel from clients
	if (type == DISPATCH_CONTINUATION_TYPE(MACH_RECV_BARRIER)) {
		// on the send queue, the mach channel isn't the current queue
		// its target queue is the current one already
		_dispatch_thread_frame_stash(&dtf);
	}
	dmrr = dm->dm_recv_refs;
	DISPATCH_COMPILER_CAN_ASSUME(dc_flags & DC_FLAG_CONSUME);
	if (unlikely(!dm->dm_connect_handler_called)) {
		dispatch_invoke_with_autoreleasepool(flags, {
			// do not coalesce with the block below due to continuation reuse
			_dispatch_mach_connect_invoke(dm);
		});
	}
	_dispatch_continuation_pop_forwarded(dc, dc_flags, dm, {
		dispatch_invoke_with_autoreleasepool(flags, {
			_dispatch_client_callout(dc->dc_ctxt, dc->dc_func);
			_dispatch_client_callout4(dmrr->dmrr_handler_ctxt,
					DISPATCH_MACH_BARRIER_COMPLETED, NULL, 0,
					dmrr->dmrr_handler_func);
		});
	});
	if (type == DISPATCH_CONTINUATION_TYPE(MACH_RECV_BARRIER)) {
		_dispatch_thread_frame_unstash(&dtf);
	}
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_mach_barrier_set_vtable(dispatch_continuation_t dc,
		dispatch_mach_t dm, dispatch_continuation_vtable_t vtable)
{
	dc->dc_data = (void *)dc->dc_flags;
	dc->dc_other = dm;
	dc->do_vtable = vtable; // Must be after dc_flags load, dc_vtable aliases
}

DISPATCH_NOINLINE
void
dispatch_mach_send_barrier_f(dispatch_mach_t dm, void *context,
		dispatch_function_t func)
{
	dispatch_continuation_t dc = _dispatch_continuation_alloc();
	uintptr_t dc_flags = DC_FLAG_CONSUME;
	dispatch_qos_t qos;

	_dispatch_continuation_init_f(dc, dm, context, func, 0, dc_flags);
	_dispatch_mach_barrier_set_vtable(dc, dm, DC_VTABLE(MACH_SEND_BARRIER));
	_dispatch_trace_item_push(dm, dc);
	qos = _dispatch_qos_from_pp(dc->dc_priority);
	return _dispatch_mach_send_push(dm, dc, qos);
}

DISPATCH_NOINLINE
void
dispatch_mach_send_barrier(dispatch_mach_t dm, dispatch_block_t barrier)
{
	dispatch_continuation_t dc = _dispatch_continuation_alloc();
	uintptr_t dc_flags = DC_FLAG_CONSUME;
	dispatch_qos_t qos;

	_dispatch_continuation_init(dc, dm, barrier, 0, dc_flags);
	_dispatch_mach_barrier_set_vtable(dc, dm, DC_VTABLE(MACH_SEND_BARRIER));
	_dispatch_trace_item_push(dm, dc);
	qos = _dispatch_qos_from_pp(dc->dc_priority);
	return _dispatch_mach_send_push(dm, dc, qos);
}

DISPATCH_NOINLINE
void
dispatch_mach_receive_barrier_f(dispatch_mach_t dm, void *context,
		dispatch_function_t func)
{
	dispatch_continuation_t dc = _dispatch_continuation_alloc();
	uintptr_t dc_flags = DC_FLAG_CONSUME;
	dispatch_qos_t qos;

	qos = _dispatch_continuation_init_f(dc, dm, context, func, 0, dc_flags);
	_dispatch_mach_barrier_set_vtable(dc, dm, DC_VTABLE(MACH_RECV_BARRIER));
	return _dispatch_continuation_async(dm, dc, qos, dc_flags);
}

DISPATCH_NOINLINE
void
dispatch_mach_receive_barrier(dispatch_mach_t dm, dispatch_block_t barrier)
{
	dispatch_continuation_t dc = _dispatch_continuation_alloc();
	uintptr_t dc_flags = DC_FLAG_CONSUME;
	dispatch_qos_t qos;

	qos = _dispatch_continuation_init(dc, dm, barrier, 0, dc_flags);
	_dispatch_mach_barrier_set_vtable(dc, dm, DC_VTABLE(MACH_RECV_BARRIER));
	return _dispatch_continuation_async(dm, dc, qos, dc_flags);
}

DISPATCH_NOINLINE
static void
_dispatch_mach_cancel_invoke(dispatch_mach_t dm, dispatch_invoke_flags_t flags)
{
	dispatch_mach_recv_refs_t dmrr = dm->dm_recv_refs;

	dispatch_invoke_with_autoreleasepool(flags, {
		if (unlikely(!dm->dm_connect_handler_called)) {
			_dispatch_mach_connect_invoke(dm);
		}
		_dispatch_client_callout4(dmrr->dmrr_handler_ctxt,
				DISPATCH_MACH_CANCELED, NULL, 0, dmrr->dmrr_handler_func);
		_dispatch_perfmon_workitem_inc();
	});
	dm->dm_cancel_handler_called = 1;
}

DISPATCH_NOINLINE
void
dispatch_mach_cancel(dispatch_mach_t dm)
{
	dispatch_queue_flags_t dqf;

	_dispatch_object_debug(dm, "%s", __func__);
	// <rdar://problem/34849210> similar race to dispatch_source_cancel
	// Once we set the DSF_CANCELED bit, anyone can notice and finish the
	// unregistration causing use after free in dispatch_mach_reconnect() below.
	_dispatch_retain(dm);
	dqf = _dispatch_queue_atomic_flags_set_orig(dm, DSF_CANCELED);
	if (!(dqf & DSF_CANCELED)) {
		dispatch_mach_reconnect(dm, MACH_PORT_NULL, DM_CHECKIN_CANCELED);
	}
	_dispatch_release_tailcall(dm);
}

static void
_dispatch_mach_install(dispatch_mach_t dm, dispatch_wlh_t wlh,
		dispatch_priority_t pri)
{
	bool cancelled = (_dispatch_queue_atomic_flags(dm) & DSF_CANCELED);
	dispatch_mach_recv_refs_t dmrr = dm->dm_recv_refs;

	dispatch_assert(!dm->ds_is_installed);
	dm->ds_is_installed = true;

	uint32_t disconnect_cnt = os_atomic_load2o(dm->dm_send_refs,
			dmsr_disconnect_cnt, relaxed);
	if (unlikely(disconnect_cnt & DISPATCH_MACH_NEVER_CONNECTED)) {
		DISPATCH_CLIENT_CRASH(disconnect_cnt, "Channel never connected");
	}

	if (!dm->dq_priority) {
		// _dispatch_mach_reply_kevent_register assumes this has been done
		// which is unlike regular sources or queues, the FALLBACK flag
		// is used so that the priority of the channel doesn't act as
		// a QoS floor for incoming messages (26761457)
		dm->dq_priority = pri;
	}

	if (!cancelled && dm->dm_is_xpc &&
			_dispatch_mach_xpc_hooks->dmxh_enable_sigterm_notification(
			dmrr->dmrr_handler_ctxt)) {
		dispatch_xpc_term_refs_t _dxtr =
				dux_create(&_dispatch_xpc_type_sigterm, SIGTERM, 0)._dxtr;
		_dxtr->du_owner_wref = _dispatch_ptr2wref(dm);
		dm->dm_xpc_term_refs = _dxtr;
		_dispatch_unote_register(dm->dm_xpc_term_refs, wlh, pri);
	}

	if (!cancelled && dmrr->du_ident) {
		dispatch_assert(dmrr->du_is_direct);
		// rdar://45419440 this absolutely needs to be done last
		// as this can cause an event to be delivered
		// and to finish the activation concurrently
		(void)_dispatch_unote_register(dmrr, wlh, pri);
	}
}

void
_dispatch_mach_activate(dispatch_mach_t dm)
{
	dispatch_priority_t pri;
	dispatch_wlh_t wlh;

	// call "super"
	_dispatch_lane_activate(dm);

	if (!dm->ds_is_installed) {
		pri = _dispatch_queue_compute_priority_and_wlh(dm, &wlh);
		// rdar://45419440 this needs to be last
		if (pri) _dispatch_mach_install(dm, wlh, pri);
	}
}

DISPATCH_NOINLINE
static void
_dispatch_mach_handle_wlh_change(dispatch_mach_t dm)
{
	dispatch_queue_flags_t dqf;

	dqf = _dispatch_queue_atomic_flags_set_orig(dm, DSF_WLH_CHANGED);
	if (!(dqf & DQF_MUTABLE)) {
		if (dm->dm_is_xpc) {
			DISPATCH_CLIENT_CRASH(0, "Changing target queue "
					"hierarchy after xpc connection was activated");
		} else {
			DISPATCH_CLIENT_CRASH(0, "Changing target queue "
					"hierarchy after mach channel was connected");
		}
	}
	if (!(dqf & DSF_WLH_CHANGED)) {
		if (dm->dm_is_xpc) {
			_dispatch_bug_deprecated("Changing target queue "
					"hierarchy after xpc connection was activated");
		} else {
			_dispatch_bug_deprecated("Changing target queue "
					"hierarchy after mach channel was connected");
		}
	}
}

DISPATCH_ALWAYS_INLINE
static inline dispatch_queue_wakeup_target_t
_dispatch_mach_invoke2(dispatch_mach_t dm,
		dispatch_invoke_context_t dic, dispatch_invoke_flags_t flags,
		uint64_t *owned)
{
	dispatch_queue_wakeup_target_t retq = NULL;
	dispatch_queue_t dq = _dispatch_queue_get_current();
	dispatch_mach_send_refs_t dmsr = dm->dm_send_refs;
	dispatch_mach_recv_refs_t dmrr = dm->dm_recv_refs;
	dispatch_queue_flags_t dqf;

	if (unlikely(!(flags & DISPATCH_INVOKE_MANAGER_DRAIN) && dmrr &&
			_dispatch_unote_wlh_changed(dmrr, _dispatch_get_event_wlh()))) {
		_dispatch_mach_handle_wlh_change(dm);
	}

	// This function performs all mach channel actions. Each action is
	// responsible for verifying that it takes place on the appropriate queue.
	// If the current queue is not the correct queue for this action, the
	// correct queue will be returned and the invoke will be re-driven on that
	// queue.

	// The order of tests here in invoke and in wakeup should be consistent.

	if (unlikely(!dm->ds_is_installed)) {
		// The channel needs to be installed on the kevent queue.
		if (unlikely(flags & DISPATCH_INVOKE_MANAGER_DRAIN)) {
			return dm->do_targetq;
		}
		dispatch_priority_t pri = DISPATCH_PRIORITY_FLAG_MANAGER;
		if (likely(flags & DISPATCH_INVOKE_WORKER_DRAIN)) {
			pri = _dispatch_get_basepri();
		}
		_dispatch_mach_install(dm, _dispatch_get_event_wlh(), pri);
		_dispatch_perfmon_workitem_inc();
	}

	if (_dispatch_queue_class_probe(dm)) {
		if (dq == dm->do_targetq) {
drain:
			retq = _dispatch_lane_serial_drain(dm, dic, flags, owned);
		} else {
			retq = dm->do_targetq;
		}
	}

	dqf = _dispatch_queue_atomic_flags(dm);
	if (!retq && !(dqf & DSF_CANCELED) && _dispatch_unote_needs_rearm(dmrr)) {
		_dispatch_unote_resume(dmrr);
		if (dq == dm->do_targetq && !dq->do_targetq && !dmsr->dmsr_tail &&
				(dq->dq_priority & DISPATCH_PRIORITY_FLAG_OVERCOMMIT) &&
				_dispatch_wlh_should_poll_unote(dmrr)) {
			// try to redrive the drain from under the lock for channels
			// targeting an overcommit root queue to avoid parking
			// when the next message has already fired
			_dispatch_event_loop_drain(KEVENT_FLAG_IMMEDIATE);
			if (dm->dq_items_tail) goto drain;
		}
		dqf = _dispatch_queue_atomic_flags(dm);
	}

	if (dmsr->dmsr_tail) {
		if (!os_atomic_load(&dmsr->dmsr_notification_armed, relaxed) ||
				dmsr->dmsr_disconnect_cnt) {
			bool requires_mgr = dmsr->dmsr_disconnect_cnt ?
					_dispatch_unote_registered(dmsr) : dm->dm_needs_mgr;
			// The channel has pending messages to send.
			if (unlikely(requires_mgr && dq != _dispatch_mgr_q._as_dq)) {
				return retq ? retq : _dispatch_mgr_q._as_dq;
			}
			dispatch_mach_send_invoke_flags_t send_flags = DM_SEND_INVOKE_NONE;
			if (dq != _dispatch_mgr_q._as_dq) {
				send_flags |= DM_SEND_INVOKE_CAN_RUN_BARRIER;
			}
			_dispatch_mach_send_invoke(dm, flags, send_flags);
			if (!retq && dm->dq_items_tail) {
				retq = dm->do_targetq;
			}
		}
		if (!retq && dmsr->dmsr_tail) {
			retq = DISPATCH_QUEUE_WAKEUP_WAIT_FOR_EVENT;
		}
	}

	if (dqf & DSF_CANCELED) {
		// The channel has been cancelled and needs to be uninstalled from the
		// manager queue.
		if (!(dqf & DSF_DELETED) && !_dispatch_mach_cancel(dm)) {
			// waiting for the delivery of a deferred delete event
			return retq ? retq : DISPATCH_QUEUE_WAKEUP_WAIT_FOR_EVENT;
		}

		// After uninstallation, the cancellation handler needs to be delivered
		// to the target queue, but not before we drained all messages from the
		// receive queue.
		if (!dm->dm_cancel_handler_called) {
			if (dq != dm->do_targetq) {
				return retq ? retq : dm->do_targetq;
			}
			if (DISPATCH_QUEUE_IS_SUSPENDED(dm)) {
				return dm->do_targetq;
			}
			if (_dispatch_queue_class_probe(dm)) {
				goto drain;
			}
			_dispatch_mach_cancel_invoke(dm, flags);
		}
	}

	return retq;
}

DISPATCH_NOINLINE
void
_dispatch_mach_invoke(dispatch_mach_t dm,
		dispatch_invoke_context_t dic, dispatch_invoke_flags_t flags)
{
	_dispatch_queue_class_invoke(dm, dic, flags,
			DISPATCH_INVOKE_DISALLOW_SYNC_WAITERS, _dispatch_mach_invoke2);
}

void
_dispatch_mach_wakeup(dispatch_mach_t dm, dispatch_qos_t qos,
		dispatch_wakeup_flags_t flags)
{
	// This function determines whether the mach channel needs to be invoked.
	// The order of tests here in probe and in invoke should be consistent.

	dispatch_mach_send_refs_t dmsr = dm->dm_send_refs;
	dispatch_queue_wakeup_target_t tq = DISPATCH_QUEUE_WAKEUP_NONE;
	dispatch_queue_flags_t dqf = _dispatch_queue_atomic_flags(dm);

	if (!dm->ds_is_installed) {
		// The channel needs to be installed on the kevent queue.
		tq = DISPATCH_QUEUE_WAKEUP_TARGET;
		goto done;
	}

	if (_dispatch_queue_class_probe(dm)) {
		tq = DISPATCH_QUEUE_WAKEUP_TARGET;
		goto done;
	}

	if (dmsr->dmsr_tail) {
		if (_dispatch_lock_is_locked(dmsr->dmsr_state_lock.dul_lock)) {
			// Sending require the send lock, the channel will be woken up
			// when the lock is dropped <rdar://15132939&15203957>
			goto done;
		}

		if (!os_atomic_load(&dmsr->dmsr_notification_armed, relaxed) ||
				dmsr->dmsr_disconnect_cnt) {
			bool requires_mgr = dmsr->dmsr_disconnect_cnt ?
					_dispatch_unote_registered(dmsr) : dm->dm_needs_mgr;
			if (unlikely(requires_mgr)) {
				tq = DISPATCH_QUEUE_WAKEUP_MGR;
			} else {
				tq = DISPATCH_QUEUE_WAKEUP_TARGET;
			}
		}
	} else if ((dqf & DSF_CANCELED) && (dqf & DSF_NEEDS_EVENT) &&
			!(flags & DISPATCH_WAKEUP_EVENT)) {
		// waiting for the delivery of a deferred delete event
	} else if ((dqf & DSF_CANCELED) && !dm->dm_cancel_handler_called) {
		// The channel needs to be cancelled and the cancellation handler
		// needs to be delivered to the target queue.
		tq = DISPATCH_QUEUE_WAKEUP_TARGET;
	}

done:
	if ((tq == DISPATCH_QUEUE_WAKEUP_TARGET) &&
			dm->do_targetq == _dispatch_mgr_q._as_dq) {
		tq = DISPATCH_QUEUE_WAKEUP_MGR;
	}

	return _dispatch_queue_wakeup(dm, qos, flags, tq);
}

static void
_dispatch_mach_sigterm_invoke(void *ctx)
{
	dispatch_mach_t dm = ctx;
	uint32_t duu_options = DUU_DELETE_ACK | DUU_MUST_SUCCEED;
	_dispatch_unote_unregister(dm->dm_xpc_term_refs, duu_options);
	if (!(_dispatch_queue_atomic_flags(dm) & DSF_CANCELED)) {
		dispatch_mach_recv_refs_t dmrr = dm->dm_recv_refs;
		_dispatch_client_callout4(dmrr->dmrr_handler_ctxt,
				DISPATCH_MACH_SIGTERM_RECEIVED, NULL, 0,
				dmrr->dmrr_handler_func);
	}
}

void
_dispatch_xpc_sigterm_merge_evt(dispatch_unote_t du,
		uint32_t flags DISPATCH_UNUSED, uintptr_t data DISPATCH_UNUSED,
		pthread_priority_t pp)
{
	dispatch_mach_t dm = _dispatch_wref2ptr(du._du->du_owner_wref);

	_dispatch_barrier_async_detached_f(dm, dm, _dispatch_mach_sigterm_invoke);
	dx_wakeup(dm, _dispatch_qos_from_pp(pp), DISPATCH_WAKEUP_EVENT |
			DISPATCH_WAKEUP_CONSUME_2 | DISPATCH_WAKEUP_MAKE_DIRTY);
}

#pragma mark -
#pragma mark dispatch_mach_msg_t

dispatch_mach_msg_t
dispatch_mach_msg_create(mach_msg_header_t *msg, size_t size,
		dispatch_mach_msg_destructor_t destructor, mach_msg_header_t **msg_ptr)
{
	if (unlikely(size < sizeof(mach_msg_header_t) || (destructor && !msg))) {
		DISPATCH_CLIENT_CRASH(size, "Empty message");
	}

	dispatch_mach_msg_t dmsg;
	size_t msg_size = sizeof(struct dispatch_mach_msg_s);
	if (!destructor && os_add_overflow(msg_size,
			  (size - sizeof(dmsg->dmsg_msg)), &msg_size)) {
		DISPATCH_CLIENT_CRASH(size, "Message size too large");
	}

	dmsg = _dispatch_object_alloc(DISPATCH_VTABLE(mach_msg), msg_size);
	if (destructor) {
		dmsg->dmsg_msg = msg;
	} else if (msg) {
		memcpy(dmsg->dmsg_buf, msg, size);
	}
	dmsg->do_next = DISPATCH_OBJECT_LISTLESS;
	dmsg->do_targetq = _dispatch_get_default_queue(false);
	dmsg->dmsg_destructor = destructor;
	dmsg->dmsg_size = size;
	if (msg_ptr) {
		*msg_ptr = _dispatch_mach_msg_get_msg(dmsg);
	}
	return dmsg;
}

void
_dispatch_mach_msg_dispose(dispatch_mach_msg_t dmsg,
		DISPATCH_UNUSED bool *allow_free)
{
	if (dmsg->dmsg_voucher) {
		_voucher_release(dmsg->dmsg_voucher);
		dmsg->dmsg_voucher = NULL;
	}
	switch (dmsg->dmsg_destructor) {
	case DISPATCH_MACH_MSG_DESTRUCTOR_DEFAULT:
		break;
	case DISPATCH_MACH_MSG_DESTRUCTOR_FREE:
		free(dmsg->dmsg_msg);
		break;
	case DISPATCH_MACH_MSG_DESTRUCTOR_VM_DEALLOCATE: {
		mach_vm_size_t vm_size = dmsg->dmsg_size;
		mach_vm_address_t vm_addr = (uintptr_t)dmsg->dmsg_msg;
		(void)dispatch_assume_zero(mach_vm_deallocate(mach_task_self(),
				vm_addr, vm_size));
		break;
	}}
}

static inline mach_msg_header_t*
_dispatch_mach_msg_get_msg(dispatch_mach_msg_t dmsg)
{
	return dmsg->dmsg_destructor ? dmsg->dmsg_msg :
			(mach_msg_header_t*)dmsg->dmsg_buf;
}

mach_msg_header_t*
dispatch_mach_msg_get_msg(dispatch_mach_msg_t dmsg, size_t *size_ptr)
{
	if (size_ptr) {
		*size_ptr = dmsg->dmsg_size;
	}
	return _dispatch_mach_msg_get_msg(dmsg);
}

size_t
_dispatch_mach_msg_debug(dispatch_mach_msg_t dmsg, char* buf, size_t bufsiz)
{
	size_t offset = 0;
	offset += dsnprintf(&buf[offset], bufsiz - offset, "%s[%p] = { ",
			_dispatch_object_class_name(dmsg), dmsg);
	offset += _dispatch_object_debug_attr(dmsg, buf + offset, bufsiz - offset);
	offset += dsnprintf(&buf[offset], bufsiz - offset, "opts/err = 0x%x, "
			"msgh[%p] = { ", dmsg->dmsg_options, dmsg->dmsg_buf);
	mach_msg_header_t *hdr = _dispatch_mach_msg_get_msg(dmsg);
	if (hdr->msgh_id) {
		offset += dsnprintf(&buf[offset], bufsiz - offset, "id 0x%x, ",
				hdr->msgh_id);
	}
	if (hdr->msgh_size) {
		offset += dsnprintf(&buf[offset], bufsiz - offset, "size %u, ",
				hdr->msgh_size);
	}
	if (hdr->msgh_bits) {
		offset += dsnprintf(&buf[offset], bufsiz - offset, "bits <l %u, r %u",
				MACH_MSGH_BITS_LOCAL(hdr->msgh_bits),
				MACH_MSGH_BITS_REMOTE(hdr->msgh_bits));
		if (MACH_MSGH_BITS_OTHER(hdr->msgh_bits)) {
			offset += dsnprintf(&buf[offset], bufsiz - offset, ", o 0x%x",
					MACH_MSGH_BITS_OTHER(hdr->msgh_bits));
		}
		offset += dsnprintf(&buf[offset], bufsiz - offset, ">, ");
	}
	if (hdr->msgh_local_port && hdr->msgh_remote_port) {
		offset += dsnprintf(&buf[offset], bufsiz - offset, "local 0x%x, "
				"remote 0x%x", hdr->msgh_local_port, hdr->msgh_remote_port);
	} else if (hdr->msgh_local_port) {
		offset += dsnprintf(&buf[offset], bufsiz - offset, "local 0x%x",
				hdr->msgh_local_port);
	} else if (hdr->msgh_remote_port) {
		offset += dsnprintf(&buf[offset], bufsiz - offset, "remote 0x%x",
				hdr->msgh_remote_port);
	} else {
		offset += dsnprintf(&buf[offset], bufsiz - offset, "no ports");
	}
	offset += dsnprintf(&buf[offset], bufsiz - offset, " } }");
	return offset;
}

DISPATCH_ALWAYS_INLINE
static dispatch_queue_t
_dispatch_mach_msg_context_async_reply_queue(dispatch_mach_t dm,
		void *msg_context)
{
	dispatch_queue_t dq;
	dq = _dispatch_mach_xpc_hooks->dmxh_msg_context_reply_queue(msg_context);
	if (dq == DMXH_MSG_CONTEXT_REPLY_QUEUE_SELF) {
		dq = dm->_as_dq;
	}
	return dq;
}

static dispatch_continuation_t
_dispatch_mach_msg_async_reply_wrap(dispatch_mach_msg_t dmsg,
		dispatch_mach_t dm)
{
	_dispatch_retain(dm); // Released in _dispatch_mach_msg_async_reply_invoke()
	dispatch_continuation_t dc = _dispatch_continuation_alloc();
	dc->do_vtable = DC_VTABLE(MACH_ASYNC_REPLY);
	dc->dc_data = dmsg;
	dc->dc_other = dm;
	dc->dc_priority = DISPATCH_NO_PRIORITY;
	dc->dc_voucher = DISPATCH_NO_VOUCHER;
	return dc;
}

DISPATCH_NOINLINE
void
_dispatch_mach_msg_async_reply_invoke(dispatch_continuation_t dc,
		DISPATCH_UNUSED dispatch_invoke_context_t dic,
		dispatch_invoke_flags_t flags)
{
	// _dispatch_mach_msg_invoke_with_mach() releases the reference on dmsg
	// taken by _dispatch_mach_msg_async_reply_wrap() after handling it.
	dispatch_mach_msg_t dmsg = dc->dc_data;
	dispatch_mach_t dm = dc->dc_other;
	_dispatch_mach_msg_invoke_with_mach(dmsg,
			flags | DISPATCH_INVOKE_ASYNC_REPLY, dm);

	// Balances _dispatch_mach_msg_async_reply_wrap
	_dispatch_release(dc->dc_other);

	_dispatch_continuation_free(dc);
}

#pragma mark -
#pragma mark dispatch_mig_server

static inline kern_return_t
_dispatch_mig_return_code(mig_reply_error_t *msg)
{
	if (msg->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) {
		return KERN_SUCCESS;
	}
	return msg->RetCode;
}

static inline void
_dispatch_mig_consume_unsent_message(mach_msg_header_t *hdr)
{
	mach_port_t port = hdr->msgh_local_port;
	if (MACH_PORT_VALID(port)) {
		kern_return_t kr = KERN_SUCCESS;
		switch (MACH_MSGH_BITS_LOCAL(hdr->msgh_bits)) {
		case MACH_MSG_TYPE_MOVE_SEND:
		case MACH_MSG_TYPE_MOVE_SEND_ONCE:
			kr = mach_port_deallocate(mach_task_self(), port);
			break;
		case MACH_MSG_TYPE_MOVE_RECEIVE:
			kr = mach_port_mod_refs(mach_task_self(), port,
					MACH_PORT_RIGHT_RECEIVE, -1);
			break;
		}
		DISPATCH_VERIFY_MIG(kr);
		dispatch_assume_zero(kr);
	}
	mach_msg_destroy(hdr);
}

mach_msg_return_t
dispatch_mig_server(dispatch_source_t ds, size_t maxmsgsz,
		dispatch_mig_callback_t callback)
{
	mach_msg_options_t options = MACH_RCV_MSG | MACH_RCV_TIMEOUT
		| MACH_RCV_TRAILER_ELEMENTS(MACH_RCV_TRAILER_CTX)
		| MACH_RCV_TRAILER_TYPE(MACH_MSG_TRAILER_FORMAT_0) | MACH_RCV_VOUCHER;
	mach_msg_options_t tmp_options;
	mig_reply_error_t *bufTemp, *bufRequest, *bufReply;
	mach_msg_return_t kr = 0, skr;
	uint64_t assertion_token = 0;
	uint32_t cnt = 1000; // do not stall out serial queues
	boolean_t demux_success;
	bool received = false;
	size_t rcv_size = maxmsgsz + MAX_TRAILER_SIZE;
	dispatch_source_refs_t dr = ds->ds_refs;

	bufRequest = alloca(rcv_size);
	bufRequest->RetCode = 0;
	_dispatch_mach_stack_probe(bufRequest, rcv_size);

	bufReply = alloca(rcv_size);
	bufReply->Head.msgh_size = 0;
	_dispatch_mach_stack_probe(bufReply, rcv_size);

#if DISPATCH_DEBUG
	options |= MACH_RCV_LARGE; // rdar://problem/8422992
#endif
	tmp_options = options;
	// XXX FIXME -- change this to not starve out the target queue
	for (;;) {
		if (DISPATCH_QUEUE_IS_SUSPENDED(ds) || (--cnt == 0)) {
			options &= ~MACH_RCV_MSG;
			tmp_options &= ~MACH_RCV_MSG;

			if (!(tmp_options & MACH_SEND_MSG)) {
				goto out;
			}
		}
		kr = mach_msg(&bufReply->Head, tmp_options, bufReply->Head.msgh_size,
				(mach_msg_size_t)rcv_size, (mach_port_t)dr->du_ident, 0, 0);

		tmp_options = options;

		if (unlikely(kr)) {
			switch (kr) {
			case MACH_SEND_INVALID_DEST:
			case MACH_SEND_TIMED_OUT:
				_dispatch_mig_consume_unsent_message(&bufReply->Head);
				break;
			case MACH_RCV_TIMED_OUT:
				// Don't return an error if a message was sent this time or
				// a message was successfully received previously
				// rdar://problems/7363620&7791738
				if (bufReply->Head.msgh_remote_port || received) {
					kr = MACH_MSG_SUCCESS;
				}
				break;
			case MACH_RCV_INVALID_NAME:
				break;
#if DISPATCH_DEBUG
			case MACH_RCV_TOO_LARGE:
				// receive messages that are too large and log their id and size
				// rdar://problem/8422992
				tmp_options &= ~(MACH_RCV_LARGE | MACH_SEND_MSG);
				size_t large_size = bufReply->Head.msgh_size + MAX_TRAILER_SIZE;
				void *large_buf = malloc(large_size);
				if (large_buf) {
					rcv_size = large_size;
					bufReply = large_buf;
				}
				if (!mach_msg(&bufReply->Head, tmp_options, 0,
						(mach_msg_size_t)rcv_size,
						(mach_port_t)dr->du_ident, 0, 0)) {
					_dispatch_log("BUG in libdispatch client: "
							"dispatch_mig_server received message larger than "
							"requested size %zd: id = 0x%x, size = %d",
							maxmsgsz, bufReply->Head.msgh_id,
							bufReply->Head.msgh_size);
					mach_msg_destroy(&bufReply->Head);
				}
				if (large_buf) {
					free(large_buf);
				}
				// fall through
#endif
			default:
				_dispatch_bug_mach_client(
						"dispatch_mig_server: mach_msg() failed", kr);
				break;
			}
			goto out;
		}

		if (!(tmp_options & MACH_RCV_MSG)) {
			goto out;
		}

		if (assertion_token) {
#if DISPATCH_USE_IMPORTANCE_ASSERTION
			int r = proc_importance_assertion_complete(assertion_token);
			(void)dispatch_assume_zero(r);
#endif
			assertion_token = 0;
		}
		received = true;

		bufTemp = bufRequest;
		bufRequest = bufReply;
		bufReply = bufTemp;

#if DISPATCH_USE_IMPORTANCE_ASSERTION
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
		int r = proc_importance_assertion_begin_with_msg(&bufRequest->Head,
				NULL, &assertion_token);
		if (r && r != EIO) {
			(void)dispatch_assume_zero(r);
		}
#pragma clang diagnostic pop
#endif
		_voucher_replace(voucher_create_with_mach_msg(&bufRequest->Head));
		bufReply->Head = (mach_msg_header_t){ };
		demux_success = callback(&bufRequest->Head, &bufReply->Head);

		if (!demux_success) {
			skr = MIG_BAD_ID;
		} else {
			skr = _dispatch_mig_return_code(bufReply);
		}
		switch (skr) {
		case KERN_SUCCESS:
			break;
		case MIG_NO_REPLY:
			bufReply->Head.msgh_remote_port = MACH_PORT_NULL;
			break;
		default:
			// destroy the request - but not the reply port
			// (MIG moved it into the bufReply).
			bufRequest->Head.msgh_remote_port = 0;
			mach_msg_destroy(&bufRequest->Head);
		}

		if (bufReply->Head.msgh_remote_port) {
			tmp_options |= MACH_SEND_MSG;
			if (MACH_MSGH_BITS_REMOTE(bufReply->Head.msgh_bits) !=
					MACH_MSG_TYPE_MOVE_SEND_ONCE) {
				tmp_options |= MACH_SEND_TIMEOUT;
			}
		}
	}

out:
	if (assertion_token) {
#if DISPATCH_USE_IMPORTANCE_ASSERTION
		int r = proc_importance_assertion_complete(assertion_token);
		(void)dispatch_assume_zero(r);
#endif
	}

	return kr;
}

#pragma mark -
#pragma mark dispatch_mach_mig_demux

static char const * const
_dispatch_mach_mig_demux_context_key = "mach_mig_demux";

static const mig_routine_descriptor *
_dispatch_mach_mig_resolve(mach_msg_id_t msgid,
		const struct mig_subsystem *const subsystems[], size_t count)
{
	const mig_routine_descriptor *desc;

	for (size_t i = 0; i < count; i++) {
		if (subsystems[i]->start <= msgid && msgid < subsystems[i]->end) {
			desc = &subsystems[i]->routine[msgid - subsystems[i]->start];
			return desc->stub_routine ? desc : NULL;
		}
	}
	return NULL;
}

bool
dispatch_mach_mig_demux(void *context,
		const struct mig_subsystem *const subsystems[], size_t count,
		dispatch_mach_msg_t dmsg)
{
	dispatch_thread_context_s dmmd_ctx = {
		.dtc_key = _dispatch_mach_mig_demux_context_key,
		.dtc_mig_demux_ctx = context,
	};
	mach_msg_header_t *hdr = dispatch_mach_msg_get_msg(dmsg, NULL);
	mach_msg_id_t msgid = hdr->msgh_id;
	const mig_routine_descriptor *desc;
	mig_reply_error_t *bufReply;
	mach_msg_size_t reply_size;
	kern_return_t kr;

	desc = _dispatch_mach_mig_resolve(msgid, subsystems, count);
	if (!desc) return false;

	_dispatch_thread_context_push(&dmmd_ctx);

	reply_size = desc->max_reply_msg + MAX_TRAILER_SIZE;
	bufReply = alloca(reply_size);
	_dispatch_mach_stack_probe(bufReply, reply_size);
	bufReply->Head = (mach_msg_header_t){
		.msgh_bits = MACH_MSGH_BITS(MACH_MSGH_BITS_REMOTE(hdr->msgh_bits), 0),
		.msgh_remote_port = hdr->msgh_remote_port,
		.msgh_size = sizeof(mig_reply_error_t),
		.msgh_id = msgid + 100,
	};

	desc->stub_routine(hdr, &bufReply->Head);

	switch (_dispatch_mig_return_code(bufReply)) {
	case KERN_SUCCESS:
		break;
	case MIG_NO_REPLY:
		bufReply->Head.msgh_remote_port = MACH_PORT_NULL;
		break;
	default:
		// destroy the request - but not the reply port
		// (MIG moved it into the bufReply).
		hdr->msgh_remote_port = 0;
		mach_msg_destroy(hdr);
		break;
	}

	if (bufReply->Head.msgh_remote_port) {
		mach_msg_option_t options = MACH_SEND_MSG;
		if (MACH_MSGH_BITS_REMOTE(bufReply->Head.msgh_bits) !=
				MACH_MSG_TYPE_MOVE_SEND_ONCE) {
			options |= MACH_SEND_TIMEOUT;
		}
		kr = mach_msg(&bufReply->Head, options, bufReply->Head.msgh_size,
				0, MACH_PORT_NULL, 0, MACH_PORT_NULL);
		switch (kr) {
		case KERN_SUCCESS:
			break;
		case MACH_SEND_INVALID_DEST:
		case MACH_SEND_TIMED_OUT:
			_dispatch_mig_consume_unsent_message(&bufReply->Head);
			break;
		default:
			DISPATCH_VERIFY_MIG(kr);
			DISPATCH_CLIENT_CRASH(kr,
					"dispatch_mach_mig_demux: mach_msg(MACH_SEND_MSG) failed");
		}
	}

	_dispatch_thread_context_pop(&dmmd_ctx);
	return true;
}

void *
dispatch_mach_mig_demux_get_context(void)
{
	dispatch_thread_context_t dtc;
	dtc = _dispatch_thread_context_find(_dispatch_mach_mig_demux_context_key);
	if (unlikely(dtc == NULL)) {
		DISPATCH_CLIENT_CRASH(0, "dispatch_mach_mig_demux_get_context "
				"not called from dispatch_mach_mig_demux context");
	}
	return dtc->dtc_mig_demux_ctx;
}

#pragma mark -
#pragma mark dispatch_mach_debug

DISPATCH_COLD
static size_t
_dispatch_mach_debug_attr(dispatch_mach_t dm, char *buf, size_t bufsiz)
{
	dispatch_queue_t target = dm->do_targetq;
	dispatch_mach_send_refs_t dmsr = dm->dm_send_refs;
	dispatch_mach_recv_refs_t dmrr = dm->dm_recv_refs;

	return dsnprintf(buf, bufsiz, "target = %s[%p], receive = 0x%x, "
			"send = 0x%x, send-possible = 0x%x%s, checkin = 0x%x%s, "
			"send state = %016llx, disconnected = %d, canceled = %d ",
			target && target->dq_label ? target->dq_label : "", target,
			(mach_port_t)dmrr->du_ident, dmsr->dmsr_send,
			(mach_port_t)dmsr->du_ident,
			os_atomic_load(&dmsr->dmsr_notification_armed, relaxed) ? " (armed)" : "",
			dmsr->dmsr_checkin_port, dmsr->dmsr_checkin ? " (pending)" : "",
			dmsr->dmsr_state, dmsr->dmsr_disconnect_cnt,
			(bool)(dm->dq_atomic_flags & DSF_CANCELED));
}

size_t
_dispatch_mach_debug(dispatch_mach_t dm, char* buf, size_t bufsiz)
{
	size_t offset = 0;
	offset += dsnprintf(&buf[offset], bufsiz - offset, "%s[%p] = { ",
			dm->dq_label && !dm->dm_cancel_handler_called ? dm->dq_label :
			_dispatch_object_class_name(dm), dm);
	offset += _dispatch_object_debug_attr(dm, &buf[offset], bufsiz - offset);
	offset += _dispatch_mach_debug_attr(dm, &buf[offset], bufsiz - offset);
	offset += dsnprintf(&buf[offset], bufsiz - offset, "}");
	return offset;
}

#endif /* HAVE_MACH */
