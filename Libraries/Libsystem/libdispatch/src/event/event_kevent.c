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
#if DISPATCH_EVENT_BACKEND_KEVENT
#if HAVE_MACH
#include "protocol.h"
#include "protocolServer.h"
#endif

#if DISPATCH_USE_KEVENT_WORKQUEUE && !DISPATCH_USE_KEVENT_QOS
#error unsupported configuration
#endif

#define DISPATCH_KEVENT_MUXED_MARKER  1ul
#define DISPATCH_MACH_AUDIT_TOKEN_PID (5)

#define dispatch_kevent_udata_t  __typeof__(((dispatch_kevent_t)NULL)->udata)

typedef struct dispatch_muxnote_s {
	LIST_ENTRY(dispatch_muxnote_s) dmn_list;
	LIST_HEAD(, dispatch_unote_linkage_s) dmn_unotes_head;
	dispatch_kevent_s dmn_kev DISPATCH_ATOMIC64_ALIGN;
} *dispatch_muxnote_t;

LIST_HEAD(dispatch_muxnote_bucket_s, dispatch_muxnote_s);

DISPATCH_STATIC_GLOBAL(bool _dispatch_timers_force_max_leeway);
DISPATCH_STATIC_GLOBAL(dispatch_once_t _dispatch_kq_poll_pred);
DISPATCH_STATIC_GLOBAL(struct dispatch_muxnote_bucket_s _dispatch_sources[DSL_HASH_SIZE]);

#if defined(__APPLE__)
#define DISPATCH_NOTE_CLOCK_WALL      NOTE_NSECONDS | NOTE_MACH_CONTINUOUS_TIME
#define DISPATCH_NOTE_CLOCK_MONOTONIC NOTE_MACHTIME | NOTE_MACH_CONTINUOUS_TIME
#define DISPATCH_NOTE_CLOCK_UPTIME    NOTE_MACHTIME
#else
#define DISPATCH_NOTE_CLOCK_WALL      0
#define DISPATCH_NOTE_CLOCK_MONOTONIC 0
#define DISPATCH_NOTE_CLOCK_UPTIME    0
#endif

static const uint32_t _dispatch_timer_index_to_fflags[] = {
#define DISPATCH_TIMER_FFLAGS_INIT(kind, qos, note) \
	[DISPATCH_TIMER_INDEX(DISPATCH_CLOCK_##kind, DISPATCH_TIMER_QOS_##qos)] = \
			DISPATCH_NOTE_CLOCK_##kind | NOTE_ABSOLUTE | NOTE_LEEWAY | (note)
	DISPATCH_TIMER_FFLAGS_INIT(WALL, NORMAL, 0),
	DISPATCH_TIMER_FFLAGS_INIT(UPTIME, NORMAL, 0),
	DISPATCH_TIMER_FFLAGS_INIT(MONOTONIC, NORMAL, 0),
#if DISPATCH_HAVE_TIMER_QOS
	DISPATCH_TIMER_FFLAGS_INIT(WALL, CRITICAL, NOTE_CRITICAL),
	DISPATCH_TIMER_FFLAGS_INIT(UPTIME, CRITICAL, NOTE_CRITICAL),
	DISPATCH_TIMER_FFLAGS_INIT(MONOTONIC, CRITICAL, NOTE_CRITICAL),
	DISPATCH_TIMER_FFLAGS_INIT(WALL, BACKGROUND, NOTE_BACKGROUND),
	DISPATCH_TIMER_FFLAGS_INIT(UPTIME, BACKGROUND, NOTE_BACKGROUND),
	DISPATCH_TIMER_FFLAGS_INIT(MONOTONIC, BACKGROUND, NOTE_BACKGROUND),
#endif
#undef DISPATCH_TIMER_FFLAGS_INIT
};

static inline void _dispatch_kevent_timer_drain(dispatch_kevent_t ke);
#if DISPATCH_USE_KEVENT_WORKLOOP
static void _dispatch_kevent_workloop_poke_drain(dispatch_kevent_t ke);
#endif

#pragma mark -
#pragma mark kevent debug

DISPATCH_NOINLINE
static const char *
_evfiltstr(short filt)
{
	switch (filt) {
#define _evfilt2(f) case (f): return #f
	_evfilt2(EVFILT_READ);
	_evfilt2(EVFILT_WRITE);
	_evfilt2(EVFILT_SIGNAL);
	_evfilt2(EVFILT_TIMER);

#ifdef DISPATCH_EVENT_BACKEND_KEVENT
	_evfilt2(EVFILT_AIO);
	_evfilt2(EVFILT_VNODE);
	_evfilt2(EVFILT_PROC);
#if HAVE_MACH
	_evfilt2(EVFILT_MACHPORT);
	_evfilt2(DISPATCH_EVFILT_MACH_NOTIFICATION);
#endif
	_evfilt2(EVFILT_FS);
	_evfilt2(EVFILT_USER);
#ifdef EVFILT_SOCK
	_evfilt2(EVFILT_SOCK);
#endif
#ifdef EVFILT_MEMORYSTATUS
	_evfilt2(EVFILT_MEMORYSTATUS);
#endif
#if DISPATCH_USE_KEVENT_WORKLOOP
	_evfilt2(EVFILT_WORKLOOP);
#endif // DISPATCH_USE_KEVENT_WORKLOOP
#endif // DISPATCH_EVENT_BACKEND_KEVENT

	_evfilt2(DISPATCH_EVFILT_TIMER);
	_evfilt2(DISPATCH_EVFILT_TIMER_WITH_CLOCK);
	_evfilt2(DISPATCH_EVFILT_CUSTOM_ADD);
	_evfilt2(DISPATCH_EVFILT_CUSTOM_OR);
	_evfilt2(DISPATCH_EVFILT_CUSTOM_REPLACE);
	default:
		return "EVFILT_missing";
	}
}

#if DISPATCH_DEBUG
static const char *
_evflagstr2(uint16_t *flagsp)
{
#define _evflag2(f) \
	if ((*flagsp & (f)) == (f) && (f)) { \
		*flagsp &= ~(f); \
		return #f "|"; \
	}
	_evflag2(EV_ADD);
	_evflag2(EV_DELETE);
	_evflag2(EV_ENABLE);
	_evflag2(EV_DISABLE);
	_evflag2(EV_ONESHOT);
	_evflag2(EV_CLEAR);
	_evflag2(EV_RECEIPT);
	_evflag2(EV_DISPATCH);
	_evflag2(EV_UDATA_SPECIFIC);
#ifdef EV_POLL
	_evflag2(EV_POLL);
#endif
#ifdef EV_OOBAND
	_evflag2(EV_OOBAND);
#endif
	_evflag2(EV_ERROR);
	_evflag2(EV_EOF);
	_evflag2(EV_VANISHED);
	*flagsp = 0;
	return "EV_UNKNOWN ";
}

DISPATCH_NOINLINE
static const char *
_evflagstr(uint16_t flags, char *str, size_t strsize)
{
	str[0] = 0;
	while (flags) {
		strlcat(str, _evflagstr2(&flags), strsize);
	}
	size_t sz = strlen(str);
	if (sz) str[sz-1] = 0;
	return str;
}

DISPATCH_NOINLINE
static void
dispatch_kevent_debug(const char *verb, const dispatch_kevent_s *kev,
		int i, int n, const char *function, unsigned int line)
{
	char flagstr[256];
	char i_n[31];

	if (n > 1) {
		snprintf(i_n, sizeof(i_n), "%d/%d ", i + 1, n);
	} else {
		i_n[0] = '\0';
	}
	if (verb == NULL) {
		if (kev->flags & EV_DELETE) {
			verb = "deleting";
		} else if (kev->flags & EV_ADD) {
			verb = "adding";
		} else {
			verb = "updating";
		}
	}
#if DISPATCH_USE_KEVENT_QOS
	_dispatch_debug("%s kevent[%p] %s= { ident = 0x%llx, filter = %s, "
			"flags = %s (0x%x), fflags = 0x%x, data = 0x%llx, udata = 0x%llx, "
			"qos = 0x%x, ext[0] = 0x%llx, ext[1] = 0x%llx, ext[2] = 0x%llx, "
			"ext[3] = 0x%llx }: %s #%u", verb, kev, i_n,
			(unsigned long long)kev->ident, _evfiltstr(kev->filter),
			_evflagstr(kev->flags, flagstr, sizeof(flagstr)), kev->flags, kev->fflags,
			(unsigned long long)kev->data, (unsigned long long)kev->udata, kev->qos,
			kev->ext[0], kev->ext[1], kev->ext[2], kev->ext[3],
			function, line);
#else
	_dispatch_debug("%s kevent[%p] %s= { ident = 0x%llx, filter = %s, "
			"flags = %s (0x%x), fflags = 0x%x, data = 0x%llx, udata = 0x%llx}: "
			"%s #%u", verb, kev, i_n,
			(unsigned long long)kev->ident, _evfiltstr(kev->filter),
			_evflagstr(kev->flags, flagstr, sizeof(flagstr)), kev->flags,
			kev->fflags, (unsigned long long)kev->data,
			(unsigned long long)kev->udata,
			function, line);
#endif
}
#else
static inline void
dispatch_kevent_debug(const char *verb, const dispatch_kevent_s *kev,
		int i, int n, const char *function, unsigned int line)
{
	(void)verb; (void)kev; (void)i; (void)n; (void)function; (void)line;
}
#endif // DISPATCH_DEBUG
#define _dispatch_kevent_debug_n(verb, _kev, i, n) \
		dispatch_kevent_debug(verb, _kev, i, n, __FUNCTION__, __LINE__)
#define _dispatch_kevent_debug(verb, _kev) \
		_dispatch_kevent_debug_n(verb, _kev, 0, 0)
#if DISPATCH_MGR_QUEUE_DEBUG
#define _dispatch_kevent_mgr_debug(verb, kev) _dispatch_kevent_debug(verb, kev)
#else
#define _dispatch_kevent_mgr_debug(verb, kev) ((void)verb, (void)kev)
#endif // DISPATCH_MGR_QUEUE_DEBUG
#if DISPATCH_WLH_DEBUG
#define _dispatch_kevent_wlh_debug(verb, kev) _dispatch_kevent_debug(verb, kev)
#else
#define _dispatch_kevent_wlh_debug(verb, kev)  ((void)verb, (void)kev)
#endif // DISPATCH_WLH_DEBUG

#define _dispatch_du_debug(what, du) \
		_dispatch_debug("kevent-source[%p]: %s kevent[%p] " \
				"{ filter = %s, ident = 0x%x }", \
				_dispatch_wref2ptr((du)->du_owner_wref), what, \
				(du), _evfiltstr((du)->du_filter), (du)->du_ident)

#if DISPATCH_MACHPORT_DEBUG
#ifndef MACH_PORT_TYPE_SPREQUEST
#define MACH_PORT_TYPE_SPREQUEST 0x40000000
#endif

DISPATCH_NOINLINE
void
dispatch_debug_machport(mach_port_t name, const char* str)
{
	mach_port_type_t type;
	mach_msg_bits_t ns = 0, nr = 0, nso = 0, nd = 0;
	unsigned int dnreqs = 0, dnrsiz;
	kern_return_t kr = mach_port_type(mach_task_self(), name, &type);
	if (kr) {
		_dispatch_log("machport[0x%08x] = { error(0x%x) \"%s\" }: %s", name,
				kr, mach_error_string(kr), str);
		return;
	}
	if (type & MACH_PORT_TYPE_SEND) {
		(void)dispatch_assume_zero(mach_port_get_refs(mach_task_self(), name,
				MACH_PORT_RIGHT_SEND, &ns));
	}
	if (type & MACH_PORT_TYPE_SEND_ONCE) {
		(void)dispatch_assume_zero(mach_port_get_refs(mach_task_self(), name,
				MACH_PORT_RIGHT_SEND_ONCE, &nso));
	}
	if (type & MACH_PORT_TYPE_DEAD_NAME) {
		(void)dispatch_assume_zero(mach_port_get_refs(mach_task_self(), name,
				MACH_PORT_RIGHT_DEAD_NAME, &nd));
	}
	if (type & (MACH_PORT_TYPE_RECEIVE|MACH_PORT_TYPE_SEND)) {
		kr = mach_port_dnrequest_info(mach_task_self(), name, &dnrsiz, &dnreqs);
		if (kr != KERN_INVALID_RIGHT) (void)dispatch_assume_zero(kr);
	}
	if (type & MACH_PORT_TYPE_RECEIVE) {
		mach_port_status_t status = { .mps_pset = 0, };
		mach_msg_type_number_t cnt = MACH_PORT_RECEIVE_STATUS_COUNT;
		(void)dispatch_assume_zero(mach_port_get_refs(mach_task_self(), name,
				MACH_PORT_RIGHT_RECEIVE, &nr));
		(void)dispatch_assume_zero(mach_port_get_attributes(mach_task_self(),
				name, MACH_PORT_RECEIVE_STATUS, (void*)&status, &cnt));
		_dispatch_log("machport[0x%08x] = { R(%03u) S(%03u) SO(%03u) D(%03u) "
				"dnreqs(%03u) spreq(%s) nsreq(%s) pdreq(%s) srights(%s) "
				"sorights(%03u) qlim(%03u) msgcount(%03u) mkscount(%03u) "
				"seqno(%03u) }: %s", name, nr, ns, nso, nd, dnreqs,
				type & MACH_PORT_TYPE_SPREQUEST ? "Y":"N",
				status.mps_nsrequest ? "Y":"N", status.mps_pdrequest ? "Y":"N",
				status.mps_srights ? "Y":"N", status.mps_sorights,
				status.mps_qlimit, status.mps_msgcount, status.mps_mscount,
				status.mps_seqno, str);
	} else if (type & (MACH_PORT_TYPE_SEND|MACH_PORT_TYPE_SEND_ONCE|
			MACH_PORT_TYPE_DEAD_NAME)) {
		_dispatch_log("machport[0x%08x] = { R(%03u) S(%03u) SO(%03u) D(%03u) "
				"dnreqs(%03u) spreq(%s) }: %s", name, nr, ns, nso, nd, dnreqs,
				type & MACH_PORT_TYPE_SPREQUEST ? "Y":"N", str);
	} else {
		_dispatch_log("machport[0x%08x] = { type(0x%08x) }: %s", name, type,
				str);
	}
}
#endif

#pragma mark dispatch_kevent_t

#if HAVE_MACH

DISPATCH_STATIC_GLOBAL(dispatch_once_t _dispatch_mach_host_port_pred);
DISPATCH_STATIC_GLOBAL(mach_port_t _dispatch_mach_host_port);

static inline void*
_dispatch_kevent_mach_msg_buf(dispatch_kevent_t ke)
{
	return (void*)ke->ext[0];
}

static inline mach_msg_size_t
_dispatch_kevent_mach_msg_size(dispatch_kevent_t ke)
{
	// buffer size in the successful receive case, but message size (like
	// msgh_size) in the MACH_RCV_TOO_LARGE case, i.e. add trailer size.
	return (mach_msg_size_t)ke->ext[1];
}

static inline bool
_dispatch_kevent_has_machmsg_rcv_error(dispatch_kevent_t ke)
{
#define MACH_ERROR_RCV_SUB 0x4
	mach_error_t kr = (mach_error_t) ke->fflags;
	return (err_get_system(kr) == err_mach_ipc) &&
			(err_get_sub(kr) == MACH_ERROR_RCV_SUB);
#undef MACH_ERROR_RCV_SUB
}

static inline bool _dispatch_kevent_has_machmsg_rcv_error(dispatch_kevent_t ke);
static void _dispatch_kevent_mach_msg_drain(dispatch_kevent_t ke);
static inline void _dispatch_mach_host_calendar_change_register(void);

// DISPATCH_MACH_NOTIFICATION_ARMED are muxnotes that aren't registered with
// kevent for real, but with mach_port_request_notification()
//
// the kevent structure is used for bookkeeping:
// - ident, filter, flags and fflags have their usual meaning
// - data is used to monitor the actual state of the
//   mach_port_request_notification()
// - ext[0] is a boolean that trackes whether the notification is armed or not
#define DISPATCH_MACH_NOTIFICATION_ARMED(dmn) ((dmn)->dmn_kev.ext[0])
#endif

DISPATCH_ALWAYS_INLINE
static dispatch_muxnote_t
_dispatch_kevent_get_muxnote(dispatch_kevent_t ke)
{
	uintptr_t dmn_addr = (uintptr_t)ke->udata & ~DISPATCH_KEVENT_MUXED_MARKER;
	return (dispatch_muxnote_t)dmn_addr;
}

DISPATCH_ALWAYS_INLINE
static inline bool
_dispatch_kevent_unote_is_muxed(dispatch_kevent_t ke)
{
	return ((uintptr_t)ke->udata) & DISPATCH_KEVENT_MUXED_MARKER;
}

DISPATCH_ALWAYS_INLINE
static dispatch_unote_t
_dispatch_kevent_get_unote(dispatch_kevent_t ke)
{
	dispatch_assert(_dispatch_kevent_unote_is_muxed(ke) == false);
	return (dispatch_unote_t){ ._du = (dispatch_unote_class_t)ke->udata };
}

DISPATCH_NOINLINE
static void
_dispatch_kevent_print_error(dispatch_kevent_t ke)
{
	dispatch_unote_class_t du = NULL;
	_dispatch_debug("kevent[0x%llx]: handling error",
			(unsigned long long)ke->udata);
	if (ke->flags & EV_DELETE) {
		if (ke->flags & EV_UDATA_SPECIFIC) {
			if (ke->data == EINPROGRESS) {
				// deferred EV_DELETE
				return;
			}
		}
		// for EV_DELETE if the update was deferred we may have reclaimed
		// the udata already, and it is unsafe to dereference it now.
	} else if (_dispatch_kevent_unote_is_muxed(ke)) {
		ke->flags |= _dispatch_kevent_get_muxnote(ke)->dmn_kev.flags;
	} else if (ke->udata) {
		du = (dispatch_unote_class_t)(uintptr_t)ke->udata;
		if (!_dispatch_unote_registered(du)) {
			ke->flags |= EV_ADD;
		}
	}

	switch (ke->data) {
	case 0:
		return;
	case ERANGE: /* A broken QoS was passed to kevent_id() */
		DISPATCH_INTERNAL_CRASH(ke->qos, "Invalid kevent priority");
	default:
		// log the unexpected error
		_dispatch_bug_kevent_client("kevent", _evfiltstr(ke->filter),
				!ke->udata ? NULL :
				ke->flags & EV_DELETE ? "delete" :
				ke->flags & EV_ADD ? "add" :
				ke->flags & EV_ENABLE ? "enable" : "monitor",
				(int)ke->data, ke->ident, ke->udata, du);
	}
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_kevent_merge_ev_flags(dispatch_unote_t du, uint32_t flags)
{
	if (unlikely(!(flags & EV_UDATA_SPECIFIC) && (flags & EV_ONESHOT))) {
		_dispatch_unote_unregister(du, DUU_DELETE_ACK | DUU_MUST_SUCCEED);
		return;
	}

	if (flags & EV_DELETE) {
		// When a speculative deletion is requested by libdispatch,
		// and the kernel is about to deliver an event, it can acknowledge
		// our wish by delivering the event as a (EV_DELETE | EV_ONESHOT)
		// event and dropping the knote at once.
		_dispatch_unote_state_set(du, DU_STATE_NEEDS_DELETE);
	} else if (flags & (EV_ONESHOT | EV_VANISHED)) {
		// EV_VANISHED events if re-enabled will produce another EV_VANISHED
		// event. To avoid an infinite loop of such events, mark the unote
		// as needing deletion so that _dispatch_unote_needs_rearm()
		// eventually returns false.
		//
		// mach channels crash on EV_VANISHED, and dispatch sources stay
		// in a limbo until canceled (explicitly or not).
		dispatch_unote_state_t du_state = _dispatch_unote_state(du);
		du_state |= DU_STATE_NEEDS_DELETE;
		du_state &= ~DU_STATE_ARMED;
		_dispatch_unote_state_set(du, du_state);
	} else if (likely(flags & EV_DISPATCH)) {
		_dispatch_unote_state_clear_bit(du, DU_STATE_ARMED);
	} else {
		return;
	}

	_dispatch_du_debug((flags & EV_VANISHED) ? "vanished" :
			(flags & EV_DELETE) ? "deleted oneshot" :
			(flags & EV_ONESHOT) ? "oneshot" : "disarmed", du._du);
}

DISPATCH_NOINLINE
static void
_dispatch_kevent_merge(dispatch_unote_t du, dispatch_kevent_t ke)
{
	dispatch_unote_action_t action = dux_type(du._du)->dst_action;
	pthread_priority_t pp = 0;
	uintptr_t data;

	// once we modify the queue atomic flags below, it will allow concurrent
	// threads running _dispatch_source_invoke2 to dispose of the source,
	// so we can't safely borrow the reference we get from the muxnote udata
	// anymore, and need our own <rdar://20382435>
	_dispatch_retain_unote_owner(du);

	switch (action) {
	case DISPATCH_UNOTE_ACTION_PASS_DATA:
		data = (uintptr_t)ke->data;
		break;

	case DISPATCH_UNOTE_ACTION_PASS_FFLAGS:
		data = (uintptr_t)ke->fflags;
#if HAVE_MACH
		if (du._du->du_filter == EVFILT_MACHPORT) {
			data = DISPATCH_MACH_RECV_MESSAGE;
		}
#endif
		break;

	case DISPATCH_UNOTE_ACTION_SOURCE_SET_DATA:
		// ke->data is signed and "negative available data" makes no sense
		// zero bytes happens when EV_EOF is set
		dispatch_assert(ke->data >= 0l);
		data = (unsigned long)ke->data;
		os_atomic_store2o(du._dr, ds_pending_data, ~data, relaxed);
		break;

	case DISPATCH_UNOTE_ACTION_SOURCE_ADD_DATA:
		data = (unsigned long)ke->data;
		if (data) os_atomic_add2o(du._dr, ds_pending_data, data, relaxed);
		break;

	case DISPATCH_UNOTE_ACTION_SOURCE_OR_FFLAGS:
		data = ke->fflags & du._du->du_fflags;
		if (du._dr->du_has_extended_status) {
			uint64_t odata, ndata, value;
			uint32_t status = (uint32_t)ke->data;

			// We combine the data and status into a single 64-bit value.
			value = DISPATCH_SOURCE_COMBINE_DATA_AND_STATUS(data, status);
			os_atomic_rmw_loop2o(du._dr, ds_pending_data, odata, ndata, relaxed, {
				ndata = DISPATCH_SOURCE_GET_DATA(odata) | value;
			});
#if HAVE_MACH
		} else if (du._du->du_filter == EVFILT_MACHPORT) {
			data = DISPATCH_MACH_RECV_MESSAGE;
			os_atomic_store2o(du._dr, ds_pending_data, data, relaxed);
#endif
		} else {
			if (data) os_atomic_or2o(du._dr, ds_pending_data, data, relaxed);
		}
		break;

	default:
		DISPATCH_INTERNAL_CRASH(action, "Corrupt unote action");
	}

	_dispatch_kevent_merge_ev_flags(du, ke->flags);
#if DISPATCH_USE_KEVENT_QOS
	pp = ((pthread_priority_t)ke->qos) & ~_PTHREAD_PRIORITY_FLAGS_MASK;
#endif
	return dux_merge_evt(du._du, ke->flags, data, pp);
}

DISPATCH_NOINLINE
static void
_dispatch_kevent_merge_muxed(dispatch_kevent_t ke)
{
	dispatch_muxnote_t dmn = _dispatch_kevent_get_muxnote(ke);
	dispatch_unote_linkage_t dul, dul_next;

	if (ke->flags & (EV_ONESHOT | EV_DELETE)) {
		// tell _dispatch_unote_unregister_muxed() the kernel half is gone
		dmn->dmn_kev.flags |= EV_DELETE;
	}
	LIST_FOREACH_SAFE(dul, &dmn->dmn_unotes_head, du_link, dul_next) {
		_dispatch_kevent_merge(_dispatch_unote_linkage_get_unote(dul), ke);
	}
}

DISPATCH_NOINLINE
static void
_dispatch_kevent_drain(dispatch_kevent_t ke)
{
	if (ke->filter == EVFILT_USER) {
		_dispatch_kevent_mgr_debug("received", ke);
		return;
	}
#if DISPATCH_USE_KEVENT_WORKLOOP
	if (ke->filter == EVFILT_WORKLOOP) {
		return _dispatch_kevent_workloop_poke_drain(ke);
	}
#endif // DISPATCH_USE_KEVENT_WORKLOOP
	_dispatch_kevent_debug("received", ke);
	if (unlikely(ke->flags & EV_ERROR)) {
		if (ke->filter == EVFILT_PROC && ke->data == ESRCH) {
			// <rdar://problem/5067725&6626350> EVFILT_PROC may fail with ESRCH
			// when the process exists but is a zombie. As a workaround, we
			// simulate an exit event for any EVFILT_PROC with an invalid pid.
			ke->flags  = EV_UDATA_SPECIFIC | EV_ONESHOT | EV_DELETE;
			ke->fflags = NOTE_EXIT;
			ke->data   = 0;
			_dispatch_kevent_debug("synthetic NOTE_EXIT", ke);
		} else {
			return _dispatch_kevent_print_error(ke);
		}
	}
	if (ke->filter == EVFILT_TIMER) {
		return _dispatch_kevent_timer_drain(ke);
	}

#if HAVE_MACH
	if (ke->filter == EVFILT_MACHPORT && (_dispatch_kevent_mach_msg_size(ke) ||
			_dispatch_kevent_has_machmsg_rcv_error(ke))) {
		return _dispatch_kevent_mach_msg_drain(ke);
	}
#endif

	if (_dispatch_kevent_unote_is_muxed(ke)) {
		return _dispatch_kevent_merge_muxed(ke);
	}
	return _dispatch_kevent_merge(_dispatch_kevent_get_unote(ke), ke);
}

#pragma mark dispatch_kq

#if DISPATCH_USE_MGR_THREAD
DISPATCH_NOINLINE
static void
_dispatch_kq_create(intptr_t *fd_ptr)
{
	static const dispatch_kevent_s kev = {
		.ident = 1,
		.filter = EVFILT_USER,
		.flags = EV_ADD|EV_CLEAR,
		.udata = (dispatch_kevent_udata_t)DISPATCH_WLH_MANAGER,
	};
	int kqfd;

	_dispatch_fork_becomes_unsafe();
#if DISPATCH_USE_GUARDED_FD
	guardid_t guard = (uintptr_t)fd_ptr;
	kqfd = guarded_kqueue_np(&guard, GUARD_CLOSE | GUARD_DUP);
#else
	(void)guard_ptr;
	kqfd = kqueue();
#endif
	if (kqfd == -1) {
		int err = errno;
		switch (err) {
		case EMFILE:
			DISPATCH_CLIENT_CRASH(err, "kqueue() failure: "
					"process is out of file descriptors");
			break;
		case ENFILE:
			DISPATCH_CLIENT_CRASH(err, "kqueue() failure: "
					"system is out of file descriptors");
			break;
		case ENOMEM:
			DISPATCH_CLIENT_CRASH(err, "kqueue() failure: "
					"kernel is out of memory");
			break;
		default:
			DISPATCH_INTERNAL_CRASH(err, "kqueue() failure");
			break;
		}
	}
#if DISPATCH_USE_KEVENT_QOS
	dispatch_assume_zero(kevent_qos(kqfd, &kev, 1, NULL, 0, NULL, NULL, 0));
#else
	dispatch_assume_zero(kevent(kqfd, &kev, 1, NULL, 0, NULL));
#endif
	*fd_ptr = kqfd;
}
#endif

static inline int
_dispatch_kq_fd(void)
{
	return (int)(intptr_t)_dispatch_mgr_q.do_ctxt;
}

static void
_dispatch_kq_init(void *context)
{
	bool *kq_initialized = context;

	_dispatch_fork_becomes_unsafe();
	if (unlikely(getenv("LIBDISPATCH_TIMERS_FORCE_MAX_LEEWAY"))) {
		_dispatch_timers_force_max_leeway = true;
	}
	*kq_initialized = true;

#if DISPATCH_USE_KEVENT_WORKQUEUE
	_dispatch_kevent_workqueue_init();
	if (_dispatch_kevent_workqueue_enabled) {
		int r;
		int kqfd = _dispatch_kq_fd();
		const dispatch_kevent_s ke = {
			.ident = 1,
			.filter = EVFILT_USER,
			.flags = EV_ADD|EV_CLEAR,
			.qos = _PTHREAD_PRIORITY_EVENT_MANAGER_FLAG,
			.udata = (dispatch_kevent_udata_t)DISPATCH_WLH_MANAGER,
		};
retry:
		r = kevent_qos(kqfd, &ke, 1, NULL, 0, NULL, NULL,
				KEVENT_FLAG_WORKQ|KEVENT_FLAG_IMMEDIATE);
		if (unlikely(r == -1)) {
			int err = errno;
			switch (err) {
			case EINTR:
				goto retry;
			default:
				DISPATCH_CLIENT_CRASH(err,
						"Failed to initalize workqueue kevent");
				break;
			}
		}
		return;
	}
#endif // DISPATCH_USE_KEVENT_WORKQUEUE
#if DISPATCH_USE_MGR_THREAD
	_dispatch_kq_create((intptr_t *)&_dispatch_mgr_q.do_ctxt);
	_dispatch_trace_item_push(_dispatch_mgr_q.do_targetq, &_dispatch_mgr_q);
	dx_push(_dispatch_mgr_q.do_targetq, &_dispatch_mgr_q, 0);
#endif // DISPATCH_USE_MGR_THREAD
}

#if DISPATCH_USE_MEMORYPRESSURE_SOURCE
static void _dispatch_memorypressure_init(void);
#else
#define _dispatch_memorypressure_init() ((void)0)
#endif

DISPATCH_NOINLINE
static int
_dispatch_kq_poll(dispatch_wlh_t wlh, dispatch_kevent_t ke, int n,
		dispatch_kevent_t ke_out, int n_out, void *buf, size_t *avail,
		uint32_t flags)
{
	bool kq_initialized = false;
	int r = 0;

	dispatch_once_f(&_dispatch_kq_poll_pred, &kq_initialized, _dispatch_kq_init);
	if (unlikely(kq_initialized)) {
		// The calling thread was the one doing the initialization
		//
		// The event loop needs the memory pressure source and debug channel,
		// however creating these will recursively call _dispatch_kq_poll(),
		// so we can't quite initialize them under the dispatch once.
		_dispatch_memorypressure_init();
		_voucher_activity_debug_channel_init();
	}

#if !DISPATCH_USE_KEVENT_QOS
	if (flags & KEVENT_FLAG_ERROR_EVENTS) {
		// emulate KEVENT_FLAG_ERROR_EVENTS
		for (r = 0; r < n; r++) {
			ke[r].flags |= EV_RECEIPT;
		}
		n_out = n;
	}
#endif

retry:
	if (unlikely(wlh == NULL)) {
		DISPATCH_INTERNAL_CRASH(wlh, "Invalid wlh");
	} else if (wlh == DISPATCH_WLH_ANON) {
		int kqfd = _dispatch_kq_fd();
#if DISPATCH_USE_KEVENT_QOS
		if (_dispatch_kevent_workqueue_enabled) {
			flags |= KEVENT_FLAG_WORKQ;
		}
		r = kevent_qos(kqfd, ke, n, ke_out, n_out, buf, avail, flags);
#else
		(void)buf;
		(void)avail;
		const struct timespec timeout_immediately = {}, *timeout = NULL;
		if (flags & KEVENT_FLAG_IMMEDIATE) timeout = &timeout_immediately;
		r = kevent(kqfd, ke, n, ke_out, n_out, timeout);
#endif
#if DISPATCH_USE_KEVENT_WORKLOOP
	} else {
		flags |= KEVENT_FLAG_WORKLOOP;
		if (!(flags & KEVENT_FLAG_ERROR_EVENTS)) {
			flags |= KEVENT_FLAG_DYNAMIC_KQ_MUST_EXIST;
		}
		r = kevent_id((uintptr_t)wlh, ke, n, ke_out, n_out, buf, avail, flags);
#endif // DISPATCH_USE_KEVENT_WORKLOOP
	}
	if (unlikely(r == -1)) {
		int err = errno;
		switch (err) {
		case ENOMEM:
			_dispatch_temporary_resource_shortage();
			/* FALLTHROUGH */
		case EINTR:
			goto retry;
		case EBADF:
			DISPATCH_CLIENT_CRASH(err, "Do not close random Unix descriptors");
#if DISPATCH_USE_KEVENT_WORKLOOP
		case ENOENT:
			if ((flags & KEVENT_FLAG_ERROR_EVENTS) &&
					(flags & KEVENT_FLAG_DYNAMIC_KQ_MUST_EXIST)) {
				return 0;
			}
			/* FALLTHROUGH */
#endif // DISPATCH_USE_KEVENT_WORKLOOP
		default:
			DISPATCH_CLIENT_CRASH(err, "Unexpected error from kevent");
		}
	}
	return r;
}

DISPATCH_NOINLINE
static int
_dispatch_kq_drain(dispatch_wlh_t wlh, dispatch_kevent_t ke, int n,
		uint32_t flags)
{
	dispatch_kevent_s ke_out[DISPATCH_DEFERRED_ITEMS_EVENT_COUNT];
	bool poll_for_events = !(flags & KEVENT_FLAG_ERROR_EVENTS);
	int i, n_out = countof(ke_out), r = 0;
	size_t *avail = NULL;
	void *buf = NULL;

#if DISPATCH_USE_KEVENT_QOS
	size_t size;
	if (poll_for_events) {
		dispatch_assert(DISPATCH_MACH_RECEIVE_MAX_INLINE_MESSAGE_SIZE +
				DISPATCH_MACH_TRAILER_SIZE <= 32 << 10);
		size = 32 << 10; // match WQ_KEVENT_DATA_SIZE
		buf = alloca(size);
		avail = &size;
	}
#endif

#if DISPATCH_DEBUG
	for (r = 0; r < n; r++) {
		if (ke[r].filter != EVFILT_USER || DISPATCH_MGR_QUEUE_DEBUG) {
			_dispatch_kevent_debug_n(NULL, ke + r, r, n);
		}
	}
#endif

	if (poll_for_events) _dispatch_clear_return_to_kernel();
	n = _dispatch_kq_poll(wlh, ke, n, ke_out, n_out, buf, avail, flags);
	if (n == 0) {
		r = 0;
	} else if (flags & KEVENT_FLAG_ERROR_EVENTS) {
		for (i = 0, r = 0; i < n; i++) {
			if ((ke_out[i].flags & EV_ERROR) && ke_out[i].data) {
				_dispatch_kevent_drain(&ke_out[i]);
				r = (int)ke_out[i].data;
			}
		}
	} else {
#if DISPATCH_USE_KEVENT_WORKLOOP
		if (ke_out[0].flags & EV_ERROR) {
			// When kevent returns errors it doesn't process the kqueue
			// and doesn't rearm the return-to-kernel notification
			// We need to assume we have to go back.
			_dispatch_set_return_to_kernel();
		}
#endif // DISPATCH_USE_KEVENT_WORKLOOP
		for (i = 0, r = 0; i < n; i++) {
			_dispatch_kevent_drain(&ke_out[i]);
		}
	}
	return r;
}

DISPATCH_ALWAYS_INLINE
static inline int
_dispatch_kq_update_one(dispatch_wlh_t wlh, dispatch_kevent_t ke)
{
	return _dispatch_kq_drain(wlh, ke, 1,
			KEVENT_FLAG_IMMEDIATE | KEVENT_FLAG_ERROR_EVENTS);
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_kq_update_all(dispatch_wlh_t wlh, dispatch_kevent_t ke, int n)
{
	(void)_dispatch_kq_drain(wlh, ke, n,
			KEVENT_FLAG_IMMEDIATE | KEVENT_FLAG_ERROR_EVENTS);
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_kq_unote_set_kevent(dispatch_unote_t _du, dispatch_kevent_t dk,
		uint16_t action)
{
	dispatch_unote_class_t du = _du._du;
	dispatch_source_type_t dst = dux_type(du);
	uint16_t flags = dst->dst_flags | action;

	if ((flags & EV_VANISHED) && !(flags & EV_ADD)) {
		flags &= ~EV_VANISHED;
	}

	*dk = (dispatch_kevent_s){
		.ident  = du->du_ident,
		.filter = dst->dst_filter,
		.flags  = flags,
		.udata  = (dispatch_kevent_udata_t)du,
		.fflags = du->du_fflags | dst->dst_fflags,
		.data   = (__typeof__(dk->data))dst->dst_data,
#if DISPATCH_USE_KEVENT_QOS
		.qos    = (__typeof__(dk->qos))_dispatch_priority_to_pp_prefer_fallback(
				du->du_priority),
#endif
	};
}

DISPATCH_ALWAYS_INLINE
static inline int
_dispatch_kq_deferred_find_slot(dispatch_deferred_items_t ddi,
		int16_t filter, uint64_t ident, dispatch_kevent_udata_t udata)
{
	dispatch_kevent_t events = ddi->ddi_eventlist;
	int i;

	for (i = 0; i < ddi->ddi_nevents; i++) {
		if (events[i].filter == filter && events[i].ident == ident &&
				events[i].udata == udata) {
			break;
		}
	}
	return i;
}

DISPATCH_ALWAYS_INLINE
static inline dispatch_kevent_t
_dispatch_kq_deferred_reuse_slot(dispatch_wlh_t wlh,
		dispatch_deferred_items_t ddi, int slot)
{
	if (wlh != DISPATCH_WLH_ANON) _dispatch_set_return_to_kernel();
	if (unlikely(slot == ddi->ddi_maxevents)) {
		int nevents = ddi->ddi_nevents;
		ddi->ddi_nevents = 1;
		_dispatch_kq_update_all(wlh, ddi->ddi_eventlist, nevents);
		dispatch_assert(ddi->ddi_nevents == 1);
		slot = 0;
	} else if (slot == ddi->ddi_nevents) {
		ddi->ddi_nevents++;
	}
	return ddi->ddi_eventlist + slot;
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_kq_deferred_discard_slot(dispatch_deferred_items_t ddi, int slot)
{
	if (slot < ddi->ddi_nevents) {
		int last = --ddi->ddi_nevents;
		if (slot != last) {
			ddi->ddi_eventlist[slot] = ddi->ddi_eventlist[last];
		}
	}
}

DISPATCH_NOINLINE
static void
_dispatch_kq_deferred_update(dispatch_wlh_t wlh, dispatch_kevent_t ke)
{
	dispatch_deferred_items_t ddi = _dispatch_deferred_items_get();

	if (ddi && ddi->ddi_wlh == wlh && ddi->ddi_maxevents) {
		int slot = _dispatch_kq_deferred_find_slot(ddi, ke->filter, ke->ident,
				ke->udata);
		dispatch_kevent_t dk = _dispatch_kq_deferred_reuse_slot(wlh, ddi, slot);
		*dk = *ke;
		if (ke->filter != EVFILT_USER) {
			_dispatch_kevent_mgr_debug("deferred", ke);
		}
	} else {
		_dispatch_kq_update_one(wlh, ke);
	}
}

DISPATCH_NOINLINE
static int
_dispatch_kq_immediate_update(dispatch_wlh_t wlh, dispatch_kevent_t ke)
{
	dispatch_deferred_items_t ddi = _dispatch_deferred_items_get();
	if (ddi && ddi->ddi_wlh == wlh) {
		int slot = _dispatch_kq_deferred_find_slot(ddi, ke->filter, ke->ident,
				ke->udata);
		_dispatch_kq_deferred_discard_slot(ddi, slot);
	}
	return _dispatch_kq_update_one(wlh, ke);
}

#if HAVE_MACH
void
_dispatch_sync_ipc_handoff_begin(dispatch_wlh_t wlh, mach_port_t port,
		uint64_t _Atomic *addr)
{
#if DISPATCH_USE_WL_SYNC_IPC_HANDOFF
	dispatch_kevent_s ke = {
		.ident  = port,
		.filter = EVFILT_WORKLOOP,
		.flags  = EV_ADD | EV_DISABLE,
		.fflags = NOTE_WL_SYNC_IPC | NOTE_WL_IGNORE_ESTALE,
		.udata  = (uintptr_t)wlh,
		.ext[EV_EXTIDX_WL_ADDR]  = (uintptr_t)addr,
		.ext[EV_EXTIDX_WL_MASK]  = ~(uintptr_t)0,
		.ext[EV_EXTIDX_WL_VALUE] = (uintptr_t)wlh,
	};
	int rc = _dispatch_kq_immediate_update(wlh, &ke);
	if (unlikely(rc && rc != ENOENT)) {
		DISPATCH_INTERNAL_CRASH(rc, "Unexpected error from kevent");
	}
#else
	(void)wlh; (void)port; (void)addr;
#endif // DISPATCH_USE_WL_SYNC_IPC_HANDOFF
}

void
_dispatch_sync_ipc_handoff_end(dispatch_wlh_t wlh, mach_port_t port)
{
#if DISPATCH_USE_WL_SYNC_IPC_HANDOFF
	dispatch_kevent_s ke = {
		.ident  = port,
		.filter = EVFILT_WORKLOOP,
		.flags  = EV_ADD | EV_DELETE | EV_ENABLE,
		.fflags = NOTE_WL_SYNC_IPC,
		.udata  = (uintptr_t)wlh,
	};
	_dispatch_kq_deferred_update(wlh, &ke);
#else
	(void)wlh; (void)port;
#endif // DISPATCH_USE_WL_SYNC_IPC_HANDOFF
}
#endif

DISPATCH_NOINLINE
static bool
_dispatch_kq_unote_update(dispatch_wlh_t wlh, dispatch_unote_t _du,
		uint16_t action_flags)
{
	dispatch_deferred_items_t ddi = _dispatch_deferred_items_get();
	dispatch_unote_class_t du = _du._du;
	dispatch_kevent_t ke;
	int r = 0;

	if (action_flags & EV_ADD) {
		// as soon as we register we may get an event delivery and it has to
		// see du_state already set, else it will not unregister the kevent
		_dispatch_wlh_retain(wlh);
		_dispatch_unote_state_set(du, wlh, DU_STATE_ARMED);
	}

	if (ddi && ddi->ddi_wlh == wlh) {
		int slot = _dispatch_kq_deferred_find_slot(ddi,
				du->du_filter, du->du_ident, (dispatch_kevent_udata_t)du);
		if (slot < ddi->ddi_nevents) {
			// <rdar://problem/26202376> when deleting and an enable is pending,
			// we must merge EV_ENABLE to do an immediate deletion
			action_flags |= (ddi->ddi_eventlist[slot].flags & EV_ENABLE);
		}

		if (!(action_flags & EV_ADD) && (action_flags & EV_ENABLE)) {
			// can be deferred, so do it!
			ke = _dispatch_kq_deferred_reuse_slot(wlh, ddi, slot);
			_dispatch_kq_unote_set_kevent(du, ke, action_flags);
			_dispatch_kevent_debug("deferred", ke);
			goto done;
		}

		// get rid of the deferred item if any, we can't wait
		_dispatch_kq_deferred_discard_slot(ddi, slot);
	}

	if (action_flags) {
		dispatch_kevent_s dk;
		_dispatch_kq_unote_set_kevent(du, &dk, action_flags);
		r = _dispatch_kq_update_one(wlh, &dk);
	}

done:
	if (action_flags & EV_ADD) {
		if (unlikely(r)) {
			_dispatch_wlh_release(wlh);
			_dispatch_unote_state_set(du, DU_STATE_UNREGISTERED);
		} else {
			_dispatch_du_debug("installed", du);
		}
		return r == 0;
	}

	if (action_flags & EV_DELETE) {
		if (r == EINPROGRESS) {
			_dispatch_du_debug("deferred delete", du);
			return false;
		}
		_dispatch_wlh_release(wlh);
		_dispatch_unote_state_set(du, DU_STATE_UNREGISTERED);
		_dispatch_du_debug("deleted", du);
	} else if (action_flags & EV_ENABLE) {
		_dispatch_du_debug("rearmed", du);
	}

	dispatch_assume_zero(r);
	return true;
}

#pragma mark dispatch_muxnote_t

DISPATCH_ALWAYS_INLINE
static inline struct dispatch_muxnote_bucket_s *
_dispatch_muxnote_bucket(uint64_t ident, int16_t filter)
{
	switch (filter) {
#if HAVE_MACH
	case EVFILT_MACHPORT:
	case DISPATCH_EVFILT_MACH_NOTIFICATION:
		ident = MACH_PORT_INDEX(ident);
		break;
#endif
	case EVFILT_SIGNAL: // signo
	case EVFILT_PROC: // pid_t
	default: // fd
		break;
	}

	return &_dispatch_sources[DSL_HASH((uintptr_t)ident)];
}
#define _dispatch_unote_muxnote_bucket(du) \
	_dispatch_muxnote_bucket(du._du->du_ident, du._du->du_filter)

DISPATCH_ALWAYS_INLINE
static inline dispatch_muxnote_t
_dispatch_muxnote_find(struct dispatch_muxnote_bucket_s *dmb,
		uint64_t ident, int16_t filter)
{
	dispatch_muxnote_t dmn;
	LIST_FOREACH(dmn, dmb, dmn_list) {
		if (dmn->dmn_kev.ident == ident && dmn->dmn_kev.filter == filter) {
			break;
		}
	}
	return dmn;
}

#if HAVE_MACH
DISPATCH_ALWAYS_INLINE
static inline dispatch_muxnote_t
_dispatch_mach_muxnote_find(mach_port_t name, int16_t filter)
{
	struct dispatch_muxnote_bucket_s *dmb;
	dmb = _dispatch_muxnote_bucket(name, filter);
	return _dispatch_muxnote_find(dmb, name, filter);
}
#endif

bool
_dispatch_unote_register_muxed(dispatch_unote_t du)
{
	struct dispatch_muxnote_bucket_s *dmb = _dispatch_unote_muxnote_bucket(du);
	dispatch_muxnote_t dmn;
	bool installed = true;

	dmn = _dispatch_muxnote_find(dmb, du._du->du_ident, du._du->du_filter);
	if (dmn) {
		uint32_t flags = du._du->du_fflags & ~dmn->dmn_kev.fflags;
		if (flags) {
			dmn->dmn_kev.fflags |= flags;
			if (unlikely(dux_type(du._du)->dst_update_mux)) {
				installed = dux_type(du._du)->dst_update_mux(dmn);
			} else {
				installed = !_dispatch_kq_immediate_update(DISPATCH_WLH_ANON,
						&dmn->dmn_kev);
			}
			if (!installed) dmn->dmn_kev.fflags &= ~flags;
		}
	} else {
		dmn = _dispatch_calloc(1, sizeof(struct dispatch_muxnote_s));
		_dispatch_kq_unote_set_kevent(du, &dmn->dmn_kev, EV_ADD | EV_ENABLE);
#if DISPATCH_USE_KEVENT_QOS
		dmn->dmn_kev.qos = _PTHREAD_PRIORITY_EVENT_MANAGER_FLAG;
#endif
		dmn->dmn_kev.udata = (dispatch_kevent_udata_t)((uintptr_t)dmn |
				DISPATCH_KEVENT_MUXED_MARKER);
		if (unlikely(dux_type(du._du)->dst_update_mux)) {
			installed = dux_type(du._du)->dst_update_mux(dmn);
		} else {
			installed = !_dispatch_kq_immediate_update(DISPATCH_WLH_ANON,
					&dmn->dmn_kev);
		}
		if (installed) {
			dmn->dmn_kev.flags &= ~(EV_ADD | EV_VANISHED);
			LIST_INSERT_HEAD(dmb, dmn, dmn_list);
		} else {
			free(dmn);
		}
	}

	if (installed) {
		dispatch_unote_linkage_t dul = _dispatch_unote_get_linkage(du);
		LIST_INSERT_HEAD(&dmn->dmn_unotes_head, dul, du_link);
#if HAVE_MACH
		if (du._du->du_filter == DISPATCH_EVFILT_MACH_NOTIFICATION) {
			os_atomic_store2o(du._dmsr, dmsr_notification_armed,
					DISPATCH_MACH_NOTIFICATION_ARMED(dmn), relaxed);
		}
#endif
		dul->du_muxnote = dmn;
		_dispatch_unote_state_set(du, DISPATCH_WLH_ANON, DU_STATE_ARMED);
		_dispatch_du_debug("installed", du._du);
	}
	return installed;
}

void
_dispatch_unote_resume_muxed(dispatch_unote_t du)
{
	_dispatch_unote_state_set_bit(du, DU_STATE_ARMED);
	if (unlikely(dux_type(du._du)->dst_update_mux)) {
		dispatch_unote_linkage_t dul = _dispatch_unote_get_linkage(du);
		dux_type(du._du)->dst_update_mux(dul->du_muxnote);
	} else {
		dispatch_unote_linkage_t dul = _dispatch_unote_get_linkage(du);
		dispatch_muxnote_t dmn = dul->du_muxnote;
		_dispatch_kq_deferred_update(DISPATCH_WLH_ANON, &dmn->dmn_kev);
	}
}

bool
_dispatch_unote_unregister_muxed(dispatch_unote_t du)
{
	dispatch_unote_linkage_t dul = _dispatch_unote_get_linkage(du);
	dispatch_muxnote_t dmn = dul->du_muxnote;
	bool update = false, dispose = false;

#if HAVE_MACH
	if (dmn->dmn_kev.filter == DISPATCH_EVFILT_MACH_NOTIFICATION) {
		os_atomic_store2o(du._dmsr, dmsr_notification_armed, false, relaxed);
	}
#endif
	_dispatch_unote_state_set(du, DU_STATE_UNREGISTERED);
	LIST_REMOVE(dul, du_link);
	_LIST_TRASH_ENTRY(dul, du_link);
	dul->du_muxnote = NULL;

	if (LIST_EMPTY(&dmn->dmn_unotes_head)) {
		dispose = true;
		update = !(dmn->dmn_kev.flags & EV_DELETE);
		dmn->dmn_kev.flags |= EV_DELETE;
	} else {
		uint32_t fflags = dux_type(du._du)->dst_fflags;
		LIST_FOREACH(dul, &dmn->dmn_unotes_head, du_link) {
			du = _dispatch_unote_linkage_get_unote(dul);
			fflags |= du._du->du_fflags;
		}
		if (dmn->dmn_kev.fflags & ~fflags) {
			dmn->dmn_kev.fflags &= fflags;
			update = true;
		}
	}
	if (update) {
		if (unlikely(dux_type(du._du)->dst_update_mux)) {
			dispatch_assume(dux_type(du._du)->dst_update_mux(dmn));
		} else {
			_dispatch_kq_deferred_update(DISPATCH_WLH_ANON, &dmn->dmn_kev);
		}
	}
	if (dispose) {
		LIST_REMOVE(dmn, dmn_list);
		free(dmn);
	}
	_dispatch_du_debug("deleted", du._du);
	return true;
}

#if DISPATCH_HAVE_DIRECT_KNOTES
bool
_dispatch_unote_register_direct(dispatch_unote_t du, dispatch_wlh_t wlh)
{
	return _dispatch_kq_unote_update(wlh, du, EV_ADD | EV_ENABLE);
}

void
_dispatch_unote_resume_direct(dispatch_unote_t du)
{
	_dispatch_unote_state_set_bit(du, DU_STATE_ARMED);
	_dispatch_kq_unote_update(_dispatch_unote_wlh(du), du, EV_ENABLE);
}

bool
_dispatch_unote_unregister_direct(dispatch_unote_t du, uint32_t flags)
{
	dispatch_unote_state_t du_state = _dispatch_unote_state(du);
	dispatch_wlh_t du_wlh = _du_state_wlh(du_state);
	dispatch_deferred_items_t ddi = _dispatch_deferred_items_get();
	uint16_t action = EV_DELETE;
	if (likely(du_wlh != DISPATCH_WLH_ANON && ddi && ddi->ddi_wlh == du_wlh)) {
#if DISPATCH_USE_KEVENT_WORKLOOP
		// Workloops are special: event delivery and servicing a workloop
		// cannot race because the kernel can reason about these.
		// Unregistering from a workloop is always safe and should always
		// succeed immediately.
#endif
		action |= EV_ENABLE;
		flags |= DUU_DELETE_ACK | DUU_MUST_SUCCEED;
	}

	if (!_du_state_needs_delete(du_state) || (flags & DUU_DELETE_ACK)) {
		if (du_state == DU_STATE_NEEDS_DELETE) {
			// There is no knote to unregister anymore, just do it.
			_dispatch_unote_state_set(du, DU_STATE_UNREGISTERED);
			_dispatch_du_debug("acknowledged deleted oneshot", du._du);
			return true;
		}
		if (!_du_state_armed(du_state)) {
			action |= EV_ENABLE;
			flags |= DUU_MUST_SUCCEED;
		}
		if ((action & EV_ENABLE) || (flags & DUU_PROBE)) {
			if (_dispatch_kq_unote_update(du_wlh, du, action)) {
				return true;
			}
		}
	}
	if (flags & DUU_MUST_SUCCEED) {
		DISPATCH_INTERNAL_CRASH(0, "Unregistration failed");
	}
	return false;
}
#endif // DISPATCH_HAVE_DIRECT_KNOTES

#pragma mark -
#pragma mark dispatch_event_loop

enum {
	DISPATCH_WORKLOOP_ASYNC,
	DISPATCH_WORKLOOP_ASYNC_FROM_SYNC,
	DISPATCH_WORKLOOP_ASYNC_DISCOVER_SYNC,
	DISPATCH_WORKLOOP_ASYNC_QOS_UPDATE,
	DISPATCH_WORKLOOP_ASYNC_LEAVE,
	DISPATCH_WORKLOOP_ASYNC_LEAVE_FROM_SYNC,
	DISPATCH_WORKLOOP_ASYNC_LEAVE_FROM_TRANSFER,
	DISPATCH_WORKLOOP_ASYNC_FORCE_END_OWNERSHIP,
	DISPATCH_WORKLOOP_RETARGET,

	DISPATCH_WORKLOOP_SYNC_WAIT,
	DISPATCH_WORKLOOP_SYNC_WAKE,
	DISPATCH_WORKLOOP_SYNC_FAKE,
	DISPATCH_WORKLOOP_SYNC_END,
};

static char const * const _dispatch_workloop_actions[] = {
	[DISPATCH_WORKLOOP_ASYNC]                       = "async",
	[DISPATCH_WORKLOOP_ASYNC_FROM_SYNC]             = "async (from sync)",
	[DISPATCH_WORKLOOP_ASYNC_DISCOVER_SYNC]         = "discover sync",
	[DISPATCH_WORKLOOP_ASYNC_QOS_UPDATE]            = "qos update",
	[DISPATCH_WORKLOOP_ASYNC_LEAVE]                 = "leave",
	[DISPATCH_WORKLOOP_ASYNC_LEAVE_FROM_SYNC]       = "leave (from sync)",
	[DISPATCH_WORKLOOP_ASYNC_LEAVE_FROM_TRANSFER]   = "leave (from transfer)",
	[DISPATCH_WORKLOOP_ASYNC_FORCE_END_OWNERSHIP]   = "leave (forced)",
	[DISPATCH_WORKLOOP_RETARGET]                    = "retarget",

	[DISPATCH_WORKLOOP_SYNC_WAIT]                   = "sync-wait",
	[DISPATCH_WORKLOOP_SYNC_FAKE]                   = "sync-fake",
	[DISPATCH_WORKLOOP_SYNC_WAKE]                   = "sync-wake",
	[DISPATCH_WORKLOOP_SYNC_END]                    = "sync-end",
};

void
_dispatch_event_loop_atfork_child(void)
{
#if HAVE_MACH
	_dispatch_mach_host_port_pred = 0;
	_dispatch_mach_host_port = MACH_PORT_NULL;
#endif
}

#if DISPATCH_USE_KEVENT_WORKLOOP
#if DISPATCH_WLH_DEBUG
/*
 * Debug information for current thread & workloop:
 *
 * fflags:
 * - NOTE_WL_THREAD_REQUEST is set if there is a thread request knote
 * - NOTE_WL_SYNC_WAIT is set if there is at least one waiter
 *
 * ext[0]: 64bit thread ID of the owner if any
 * ext[1]: 64bit thread ID of the servicer if any
 * ext[2]: number of workloops owned by the caller thread
 *
 * If this interface is supported by the kernel, the returned error is EBUSY,
 * if not it is EINVAL.
 */
static bool
_dispatch_kevent_workloop_get_info(dispatch_wlh_t wlh, dispatch_kevent_t ke)
{
	uint32_t kev_flags = KEVENT_FLAG_IMMEDIATE | KEVENT_FLAG_ERROR_EVENTS |
			KEVENT_FLAG_DYNAMIC_KQ_MUST_EXIST;
	*ke = (dispatch_kevent_s){
		.filter = EVFILT_WORKLOOP,
		.flags  = EV_ADD | EV_ENABLE,
	};
	if (_dispatch_kq_poll(wlh, ke, 1, ke, 1, NULL, NULL, kev_flags)) {
		dispatch_assert(ke->flags & EV_ERROR);
		return ke->data == EBUSY;
	}
	*ke = (dispatch_kevent_s){
		.flags = EV_ERROR,
		.data  = ENOENT,
	};
	return true;
}
#endif

DISPATCH_ALWAYS_INLINE
static inline pthread_priority_t
_dispatch_kevent_workloop_priority(dispatch_queue_t dq, int which,
		dispatch_qos_t qos)
{
	dispatch_priority_t rq_pri = dq->do_targetq->dq_priority;
	if (qos < _dispatch_priority_qos(rq_pri)) {
		qos = _dispatch_priority_qos(rq_pri);
	}
	if (qos == DISPATCH_QOS_UNSPECIFIED) {
#if 0 // we need to understand why this is happening first...
		if (which != DISPATCH_WORKLOOP_ASYNC_FROM_SYNC) {
			DISPATCH_INTERNAL_CRASH(which, "Should have had a QoS");
		}
#else
		(void)which;
#endif
		//
		// <rdar://32326125> When an enqueue happens right when a barrier ends,
		// the barrier that ends may notice the next item before the enqueuer
		// has had the time to set the max QoS on the queue.
		//
		// It is inconvenient to drop this thread request, and this case is rare
		// enough that we instead ask for MAINTENANCE to avoid the kernel
		// failing with ERANGE.
		//
		qos = DISPATCH_QOS_MAINTENANCE;
	}
	pthread_priority_t pp = _dispatch_qos_to_pp(qos);
	return pp | (rq_pri & DISPATCH_PRIORITY_FLAG_OVERCOMMIT);
}

DISPATCH_ALWAYS_INLINE_NDEBUG
static void
_dispatch_kq_fill_workloop_event(dispatch_kevent_t ke, int which,
		dispatch_wlh_t wlh, uint64_t dq_state)
{
	dispatch_queue_t dq = (dispatch_queue_t)wlh;
	dispatch_qos_t qos = _dq_state_max_qos(dq_state);
	pthread_priority_t pp = 0;
	uint32_t fflags = 0;
	uint64_t mask = 0;
	uint16_t action = 0;

	switch (which) {
	case DISPATCH_WORKLOOP_ASYNC_FROM_SYNC:
		fflags |= NOTE_WL_END_OWNERSHIP;
		/* FALLTHROUGH */
	case DISPATCH_WORKLOOP_ASYNC:
	case DISPATCH_WORKLOOP_ASYNC_DISCOVER_SYNC:
	case DISPATCH_WORKLOOP_ASYNC_QOS_UPDATE:
		dispatch_assert(_dq_state_is_base_wlh(dq_state));
		dispatch_assert(_dq_state_is_enqueued_on_target(dq_state));
		action = EV_ADD | EV_ENABLE;
		mask |= DISPATCH_QUEUE_ROLE_MASK;
		mask |= DISPATCH_QUEUE_ENQUEUED;
		mask |= DISPATCH_QUEUE_MAX_QOS_MASK;
		if (which == DISPATCH_WORKLOOP_ASYNC_DISCOVER_SYNC) {
			dispatch_assert(!_dq_state_in_sync_transfer(dq_state));
			dispatch_assert(_dq_state_drain_locked(dq_state));
			mask |= DISPATCH_QUEUE_SYNC_TRANSFER;
			fflags |= NOTE_WL_DISCOVER_OWNER;
		} else {
			fflags |= NOTE_WL_IGNORE_ESTALE;
		}
		fflags |= NOTE_WL_UPDATE_QOS;
		pp = _dispatch_kevent_workloop_priority(dq, which, qos);
		break;

	case DISPATCH_WORKLOOP_ASYNC_LEAVE_FROM_SYNC:
		fflags |= NOTE_WL_END_OWNERSHIP;
		/* FALLTHROUGH */
	case DISPATCH_WORKLOOP_ASYNC_LEAVE_FROM_TRANSFER:
		fflags |= NOTE_WL_IGNORE_ESTALE;
		/* FALLTHROUGH */
	case DISPATCH_WORKLOOP_ASYNC_LEAVE:
		dispatch_assert(!_dq_state_is_enqueued_on_target(dq_state));
		action = EV_ADD | EV_DELETE | EV_ENABLE;
		mask |= DISPATCH_QUEUE_ENQUEUED;
		break;

	case DISPATCH_WORKLOOP_ASYNC_FORCE_END_OWNERSHIP:
		// 0 is never a valid queue state, so the knote attach will fail due to
		// the debounce. However, NOTE_WL_END_OWNERSHIP is always observed even
		// when ESTALE is returned, which is the side effect we're after here.
		fflags |= NOTE_WL_END_OWNERSHIP;
		fflags |= NOTE_WL_IGNORE_ESTALE;
		action = EV_ADD | EV_ENABLE;
		mask = ~0ull;
		dq_state = 0;
		pp = _dispatch_kevent_workloop_priority(dq, which, qos);
		break;

	case DISPATCH_WORKLOOP_RETARGET:
		action = EV_ADD | EV_DELETE | EV_ENABLE;
		fflags |= NOTE_WL_END_OWNERSHIP;
		break;

	default:
		DISPATCH_INTERNAL_CRASH(which, "Invalid transition");
	}

	*ke = (dispatch_kevent_s){
		.ident  = (uintptr_t)wlh,
		.filter = EVFILT_WORKLOOP,
		.flags  = action,
		.fflags = fflags | NOTE_WL_THREAD_REQUEST,
		.qos    = (__typeof__(ke->qos))pp,
		.udata  = (uintptr_t)wlh,

		.ext[EV_EXTIDX_WL_ADDR]  = (uintptr_t)&dq->dq_state,
		.ext[EV_EXTIDX_WL_MASK]  = mask,
		.ext[EV_EXTIDX_WL_VALUE] = dq_state,
	};
	_dispatch_kevent_wlh_debug(_dispatch_workloop_actions[which], ke);
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_kq_fill_ddi_workloop_event(dispatch_deferred_items_t ddi,
		int which, dispatch_wlh_t wlh, uint64_t dq_state)
{
	int slot = _dispatch_kq_deferred_find_slot(ddi, EVFILT_WORKLOOP,
			(uint64_t)wlh, (uint64_t)wlh);
	if (slot == ddi->ddi_nevents) {
		dispatch_assert(slot < DISPATCH_DEFERRED_ITEMS_EVENT_COUNT);
		ddi->ddi_nevents++;
	}
	_dispatch_kq_fill_workloop_event(&ddi->ddi_eventlist[slot],
			which, wlh, dq_state);
}

DISPATCH_ALWAYS_INLINE_NDEBUG
static void
_dispatch_kq_fill_workloop_sync_event(dispatch_kevent_t ke, int which,
		dispatch_wlh_t wlh, uint64_t dq_state, dispatch_tid tid)
{
	dispatch_queue_t dq = (dispatch_queue_t)wlh;
	pthread_priority_t pp = 0;
	uint32_t fflags = 0;
	uint64_t mask = 0;
	uint16_t action = 0;

	switch (which) {
	case DISPATCH_WORKLOOP_SYNC_WAIT:
		action = EV_ADD | EV_DISABLE;
		fflags = NOTE_WL_SYNC_WAIT;
		pp     = _dispatch_get_priority();
		if (_dispatch_qos_from_pp(pp) == 0) {
			pp = _dispatch_qos_to_pp(DISPATCH_QOS_DEFAULT);
		}
		if (_dq_state_received_sync_wait(dq_state)) {
			fflags |= NOTE_WL_DISCOVER_OWNER;
			mask = DISPATCH_QUEUE_ROLE_MASK | DISPATCH_QUEUE_RECEIVED_SYNC_WAIT;
		}
		break;

	case DISPATCH_WORKLOOP_SYNC_FAKE:
		action = EV_ADD | EV_DISABLE;
		fflags = NOTE_WL_SYNC_WAKE;
		break;

	case DISPATCH_WORKLOOP_SYNC_WAKE:
		dispatch_assert(_dq_state_drain_locked_by(dq_state, tid));
		action = EV_ADD | EV_DISABLE;
		fflags = NOTE_WL_SYNC_WAKE | NOTE_WL_DISCOVER_OWNER;
		break;

	case DISPATCH_WORKLOOP_SYNC_END:
		action = EV_DELETE | EV_ENABLE;
		fflags = NOTE_WL_SYNC_WAKE | NOTE_WL_END_OWNERSHIP;
		break;

	default:
		DISPATCH_INTERNAL_CRASH(which, "Invalid transition");
	}

	*ke = (dispatch_kevent_s){
		.ident  = tid,
		.filter = EVFILT_WORKLOOP,
		.flags  = action,
		.fflags = fflags,
		.udata  = (uintptr_t)wlh,
		.qos    = (__typeof__(ke->qos))pp,

		.ext[EV_EXTIDX_WL_MASK] = mask,
		.ext[EV_EXTIDX_WL_VALUE] = dq_state,
	};
	if (fflags & NOTE_WL_DISCOVER_OWNER) {
		ke->ext[EV_EXTIDX_WL_ADDR] = (uintptr_t)&dq->dq_state;
	}
	_dispatch_kevent_wlh_debug(_dispatch_workloop_actions[which], ke);
}

#define DISPATCH_KEVENT_WORKLOOP_ALLOW_ENOENT 1
#define DISPATCH_KEVENT_WORKLOOP_ALLOW_ESTALE 2
#define DISPATCH_KEVENT_WORKLOOP_ALLOW_EINTR  4

DISPATCH_ALWAYS_INLINE
static inline int
_dispatch_kevent_workloop_drain_error(dispatch_kevent_t ke, long flags)
{
	int err = (int)ke->data;

	_dispatch_kevent_wlh_debug("received error", ke);
	dispatch_assert(ke->flags & EV_ERROR);
	//
	// Clear the error so that we can use the same struct to redrive as is
	// but leave a breadcrumb about the error in xflags for debugging
	//
	ke->flags &= ~EV_ERROR;
	ke->xflags = (uint32_t)err;
	ke->data = 0;

	switch (err) {
	case EINTR:
		if ((flags & DISPATCH_KEVENT_WORKLOOP_ALLOW_EINTR) &&
				(ke->fflags & NOTE_WL_SYNC_WAIT)) {
			return EINTR;
		}
		break;
	case ENOENT:
		if ((flags & DISPATCH_KEVENT_WORKLOOP_ALLOW_ENOENT) &&
				(ke->flags & EV_DELETE) && (ke->fflags & NOTE_WL_SYNC_WAKE) &&
				(ke->fflags & NOTE_WL_END_OWNERSHIP)) {
			//
			// When breaking out a waiter because of a retarget, that waiter may
			// not have made his wait syscall yet, and we can't really prepost
			// an EV_DELETE, so we have to redrive on ENOENT in this case
			//
			return ENOENT;
		}
		break;
	case ESTALE:
		if ((flags & DISPATCH_KEVENT_WORKLOOP_ALLOW_ESTALE) &&
				!(ke->fflags & NOTE_WL_IGNORE_ESTALE) &&
				ke->ext[EV_EXTIDX_WL_ADDR] && ke->ext[EV_EXTIDX_WL_MASK]) {
			return ESTALE;
		}
		break;
	case ERANGE:
		DISPATCH_INTERNAL_CRASH((uintptr_t)ke->qos, "Broken priority");
	case EOWNERDEAD:
		DISPATCH_CLIENT_CRASH((uintptr_t)ke->ext[EV_EXTIDX_WL_VALUE],
				"Invalid workloop owner, possible memory corruption");
	default:
		break;
	}
	DISPATCH_INTERNAL_CRASH(err, "Unexpected error from kevent");
}

DISPATCH_ALWAYS_INLINE
static void
_dispatch_kevent_workloop_stash(dispatch_wlh_t wlh, dispatch_kevent_t ke,
		dispatch_deferred_items_t ddi)
{
	dispatch_queue_t dq = (dispatch_queue_t)wlh;
	dispatch_assert(!ddi->ddi_stashed_dou._dq);
	ddi->ddi_wlh_needs_delete = true;
	_dispatch_retain(dq);
	ddi->ddi_stashed_rq = upcast(dq->do_targetq)._dgq;
	ddi->ddi_stashed_dou._dq = dq;
	ddi->ddi_stashed_qos = _dispatch_qos_from_pp((pthread_priority_t)ke->qos);
}

DISPATCH_ALWAYS_INLINE
static inline int
_dispatch_event_loop_get_action_for_state(uint64_t dq_state)
{
	dispatch_assert(_dq_state_is_base_wlh(dq_state));

	if (!_dq_state_is_enqueued_on_target(dq_state)) {
		return DISPATCH_WORKLOOP_ASYNC_LEAVE;
	}
	if (!_dq_state_drain_locked(dq_state)) {
		return DISPATCH_WORKLOOP_ASYNC;
	}
	if (!_dq_state_in_sync_transfer(dq_state)) {
		return DISPATCH_WORKLOOP_ASYNC_DISCOVER_SYNC;
	}
	return DISPATCH_WORKLOOP_ASYNC_QOS_UPDATE;
}

DISPATCH_NOINLINE
static void
_dispatch_kevent_workloop_poke_drain(dispatch_kevent_t ke)
{
	dispatch_deferred_items_t ddi = _dispatch_deferred_items_get();
	dispatch_wlh_t wlh = (dispatch_wlh_t)ke->udata;

#if DISPATCH_USE_WL_SYNC_IPC_HANDOFF
	if (ke->fflags & NOTE_WL_SYNC_IPC) {
		dispatch_assert((ke->flags & EV_ERROR) && ke->data == ENOENT);
		return _dispatch_kevent_wlh_debug("ignoring", ke);
	}
#endif // DISPATCH_USE_WL_SYNC_IPC_HANDOFF

	dispatch_assert(ke->fflags & NOTE_WL_THREAD_REQUEST);
	if (ke->flags & EV_ERROR) {
		uint64_t dq_state = ke->ext[EV_EXTIDX_WL_VALUE];

		_dispatch_kevent_workloop_drain_error(ke,
				DISPATCH_KEVENT_WORKLOOP_ALLOW_ESTALE);

		if (!_dq_state_is_base_wlh(dq_state)) {
			dispatch_assert((ke->flags & EV_DELETE) == 0);
			//
			// A late async request bounced because the queue is no longer
			// a workloop. There is a DISPATCH_WORKLOOP_RETARGET transition that
			// will take care of deleting the thread request
			//
			return _dispatch_kevent_wlh_debug("ignoring", ke);
		}

		//
		// We're draining a failed _dispatch_event_loop_leave_deferred()
		// so repeat its logic.
		//
		int action = _dispatch_event_loop_get_action_for_state(dq_state);
		if (action == DISPATCH_WORKLOOP_ASYNC) {
			_dispatch_kevent_wlh_debug("retry drain", ke);
			return _dispatch_kevent_workloop_stash(wlh, ke, ddi);
		} else {
			_dispatch_kq_fill_workloop_event(ke, action, wlh, dq_state);
			return _dispatch_kq_deferred_update(wlh, ke);
		}
	} else if (ddi->ddi_wlh_needs_delete) {
		//
		// we knew about this thread request because we learned about it
		// in _dispatch_kevent_workloop_poke_self() while merging another event.
		// It has already been accounted for, so just swallow it.
		//
		return _dispatch_kevent_wlh_debug("ignoring", ke);
	} else {
		//
		// This is a new thread request, it is carrying a +1 reference.
		//
		_dispatch_kevent_wlh_debug("got drain", ke);
		return _dispatch_kevent_workloop_stash(wlh, ke, ddi);
	}
}

static void
_dispatch_kevent_workloop_poke(dispatch_wlh_t wlh, uint64_t dq_state,
		uint32_t flags)
{
	uint32_t kev_flags = KEVENT_FLAG_IMMEDIATE | KEVENT_FLAG_ERROR_EVENTS;
	dispatch_kevent_s ke;
	int action;

	dispatch_assert(_dq_state_is_enqueued_on_target(dq_state));
	dispatch_assert(!_dq_state_is_enqueued_on_manager(dq_state));
	action = _dispatch_event_loop_get_action_for_state(dq_state);
override:
	_dispatch_kq_fill_workloop_event(&ke, action, wlh, dq_state);

	if (_dispatch_kq_poll(wlh, &ke, 1, &ke, 1, NULL, NULL, kev_flags)) {
		_dispatch_kevent_workloop_drain_error(&ke,
				DISPATCH_KEVENT_WORKLOOP_ALLOW_ESTALE);
		dispatch_assert(action == DISPATCH_WORKLOOP_ASYNC_DISCOVER_SYNC);
		dq_state = ke.ext[EV_EXTIDX_WL_VALUE];
		//
		// There are 4 things that can cause an ESTALE for DISCOVER_SYNC:
		// - the queue role changed, we don't want to redrive
		// - the queue is no longer enqueued, we don't want to redrive
		// - the max QoS changed, whoever changed it is doing the same
		//   transition, so we don't need to redrive
		// - the DISPATCH_QUEUE_IN_SYNC_TRANFER bit got set
		//
		// The interesting case is the last one, and will only happen in the
		// following chain of events:
		// 1. uncontended dispatch_sync()
		// 2. contended dispatch_sync()
		// 3. contended dispatch_async()
		//
		// And this code is running because of (3). It is possible that (1)
		// hands off to (2) while this call is being made, causing the
		// DISPATCH_QUEUE_IN_TRANSFER_SYNC to be set, and we don't need to tell
		// the kernel about the owner anymore. However, the async in that case
		// will have set a QoS on the queue (since dispatch_sync()s don't but
		// dispatch_async()s always do), and we need to redrive to tell it
		// to the kernel.
		//
		if (_dq_state_is_base_wlh(dq_state) &&
				_dq_state_is_enqueued_on_target(dq_state) &&
				_dq_state_in_sync_transfer(dq_state)) {
			action = DISPATCH_WORKLOOP_ASYNC;
			goto override;
		}
	}

	if (!(flags & DISPATCH_EVENT_LOOP_OVERRIDE)) {
		// Consume the reference that kept the workloop valid
		// for the duration of the syscall.
		return _dispatch_release_tailcall((dispatch_queue_t)wlh);
	}
	if (flags & DISPATCH_EVENT_LOOP_CONSUME_2) {
		return _dispatch_release_2_tailcall((dispatch_queue_t)wlh);
	}
}

DISPATCH_NOINLINE
static void
_dispatch_kevent_workloop_override_self(dispatch_deferred_items_t ddi,
		uint64_t dq_state, uint32_t flags)
{
	dispatch_wlh_t wlh = ddi->ddi_wlh;
	uint32_t kev_flags = KEVENT_FLAG_IMMEDIATE | KEVENT_FLAG_ERROR_EVENTS;
	dispatch_kevent_s ke;
	//
	// The workloop received work from itself that caused an override
	// after the drain lock has been taken, just comply and move on.
	//
	dispatch_assert(ddi->ddi_wlh_needs_delete);
	ddi->ddi_wlh_needs_update = false;

	_dispatch_kq_fill_workloop_event(&ke, DISPATCH_WORKLOOP_ASYNC,
			wlh, dq_state);
	if (_dispatch_kq_poll(wlh, &ke, 1, &ke, 1, NULL, NULL, kev_flags)) {
		_dispatch_kevent_workloop_drain_error(&ke, 0);
		__builtin_unreachable();
	}
	if (flags & DISPATCH_EVENT_LOOP_CONSUME_2) {
		return _dispatch_release_2_no_dispose((dispatch_queue_t)wlh);
	}
}

static void
_dispatch_kevent_workloop_poke_self(dispatch_deferred_items_t ddi,
		uint64_t dq_state, uint32_t flags)
{
	dispatch_queue_t dq = (dispatch_queue_t)ddi->ddi_wlh;

	if (ddi->ddi_wlh_servicing) {
		dispatch_assert(ddi->ddi_wlh_needs_delete);
		if (flags & DISPATCH_EVENT_LOOP_OVERRIDE) {
			return _dispatch_kevent_workloop_override_self(ddi, dq_state,flags);
		}
		//
		// dx_invoke() wants to re-enqueue itself e.g.  because the thread pool
		// needs narrowing, or the queue is suspended, or any other reason that
		// interrupts the drain.
		//
		// This is called with a +2 on the queue, a +1 goes to the thread
		// request, the other we dispose of.
		//
		dispatch_assert(!_dq_state_drain_locked(dq_state));
		dispatch_assert(_dq_state_is_enqueued_on_target(dq_state));
		dispatch_assert(flags & DISPATCH_EVENT_LOOP_CONSUME_2);
		_dispatch_release_no_dispose(dq);
		return _dispatch_event_loop_leave_deferred(ddi, dq_state);
	}

	//
	// This codepath is only used during the initial phase of merging
	// incoming kernel events in _dispatch_workloop_worker_thread, before
	// trying to take the drain lock in order to drain the workloop.
	//
	// Once we have taken the drain lock, wakeups will not reach this codepath
	// because ddi->ddi_wlh_servicing will be set.
	//

	if (ddi->ddi_wlh_needs_delete) {
		//
		// We know there is a thread request already (stolen or real).
		// However, an event is causing the workloop to be overridden.
		// The kernel already has applied the override, so we can
		// safely swallow this event, which carries no refcount.
		//
		dispatch_assert(flags & DISPATCH_EVENT_LOOP_OVERRIDE);
		dispatch_assert(ddi->ddi_stashed_dou._dq);
		if (flags & DISPATCH_EVENT_LOOP_CONSUME_2) {
			return _dispatch_release_2_no_dispose(dq);
		}
		return;
	}

	if (flags & DISPATCH_EVENT_LOOP_OVERRIDE) {
		//
		// An event delivery is causing an override, but didn't know
		// about a thread request yet. However, since we're receving an override
		// it means this initial thread request either exists in the kernel
		// or is about to be made.
		//
		// If it is about to be made, it is possible that it will bounce with
		// ESTALE, and will not be retried. It means we can't be sure there
		// really is or even will be a knote in the kernel for it.
		//
		// We still want to take over the +1 this thread request carries whether
		// it made it (or will make it) to the kernel, and turn it into a +2
		// below.
		//
		// Overrides we receive in this way are coalesced and acknowleged
		// only when we have to do a kevent() call for other reasons. The kernel
		// will continue to apply the overrides in question until we acknowledge
		// them, so there's no rush.
		//
		if (flags & DISPATCH_EVENT_LOOP_CONSUME_2) {
			_dispatch_release_no_dispose(dq);
		} else {
			_dispatch_retain(dq);
		}
	} else {
		//
		// Merging events causes a thread request to be issued, this means
		// the queue is empty in userland and the kernel event is the first
		// thing enqueued. Consume the caller's +2.
		//
		dispatch_assert(flags & DISPATCH_EVENT_LOOP_CONSUME_2);
	}
	dispatch_assert(!ddi->ddi_stashed_dou._dq);
	ddi->ddi_wlh_needs_delete = true;
	ddi->ddi_wlh_needs_update = true;
	ddi->ddi_stashed_rq = upcast(dq->do_targetq)._dgq;
	ddi->ddi_stashed_dou._dq = dq;
	ddi->ddi_stashed_qos = _dq_state_max_qos(dq_state);
}
#endif // DISPATCH_USE_KEVENT_WORKLOOP

DISPATCH_NOINLINE
void
_dispatch_event_loop_poke(dispatch_wlh_t wlh, uint64_t dq_state, uint32_t flags)
{
	if (wlh == DISPATCH_WLH_MANAGER) {
		dispatch_kevent_s ke = (dispatch_kevent_s){
			.ident  = 1,
			.filter = EVFILT_USER,
			.fflags = NOTE_TRIGGER,
			.udata = (dispatch_kevent_udata_t)DISPATCH_WLH_MANAGER,
		};
		return _dispatch_kq_deferred_update(DISPATCH_WLH_ANON, &ke);
	} else if (wlh && wlh != DISPATCH_WLH_ANON) {
#if DISPATCH_USE_KEVENT_WORKLOOP
		dispatch_queue_t dq = (dispatch_queue_t)wlh;
		dispatch_assert(_dq_state_is_base_wlh(dq_state));
		if (unlikely(_dq_state_is_enqueued_on_manager(dq_state))) {
			dispatch_assert(!(flags & DISPATCH_EVENT_LOOP_OVERRIDE));
			dispatch_assert(flags & DISPATCH_EVENT_LOOP_CONSUME_2);
			_dispatch_trace_item_push(&_dispatch_mgr_q, dq);
			return dx_push(_dispatch_mgr_q._as_dq, dq, 0);
		}
		dispatch_deferred_items_t ddi = _dispatch_deferred_items_get();
		if (ddi && ddi->ddi_wlh == wlh) {
			return _dispatch_kevent_workloop_poke_self(ddi, dq_state, flags);
		}
		return _dispatch_kevent_workloop_poke(wlh, dq_state, flags);
#else
		(void)dq_state; (void)flags;
#endif // DISPATCH_USE_KEVENT_WORKLOOP
	}
	DISPATCH_INTERNAL_CRASH(wlh, "Unsupported wlh configuration");
}

DISPATCH_NOINLINE
void
_dispatch_event_loop_drain(uint32_t flags)
{
	dispatch_deferred_items_t ddi = _dispatch_deferred_items_get();
	dispatch_wlh_t wlh = ddi->ddi_wlh;
	int n;

again:
#if DISPATCH_USE_KEVENT_WORKLOOP
	if (ddi->ddi_wlh_needs_update) {
		// see _dispatch_event_loop_drain() comments about the lazy handling
		// of DISPATCH_EVENT_LOOP_OVERRIDE
		dispatch_queue_t dq = (dispatch_queue_t)wlh;
		uint64_t dq_state = os_atomic_load2o(dq, dq_state, relaxed);

		dispatch_assert(ddi->ddi_wlh_needs_delete);
		ddi->ddi_wlh_needs_update = false;
		_dispatch_kq_fill_ddi_workloop_event(ddi,
				DISPATCH_WORKLOOP_ASYNC_QOS_UPDATE, wlh, dq_state);
	}
#endif // DISPATCH_USE_KEVENT_WORKLOOP
	n = ddi->ddi_nevents;
	ddi->ddi_nevents = 0;
	_dispatch_kq_drain(wlh, ddi->ddi_eventlist, n, flags);

#if DISPATCH_USE_KEVENT_WORKLOOP
	dispatch_workloop_t dwl = _dispatch_wlh_to_workloop(wlh);
	if (dwl) {
		dispatch_timer_heap_t dth = dwl->dwl_timer_heap;
		if (dth && dth[0].dth_dirty_bits) {
			_dispatch_event_loop_drain_timers(dth, DISPATCH_TIMER_WLH_COUNT);
		}
	}
#endif // DISPATCH_USE_KEVENT_WORKLOOP

	if ((flags & KEVENT_FLAG_IMMEDIATE) &&
			!(flags & KEVENT_FLAG_ERROR_EVENTS) &&
			_dispatch_needs_to_return_to_kernel()) {
		goto again;
	}
}

void
_dispatch_event_loop_merge(dispatch_kevent_t events, int nevents)
{
	dispatch_deferred_items_t ddi = _dispatch_deferred_items_get();
	dispatch_wlh_t wlh = ddi->ddi_wlh;
	dispatch_kevent_s kev[nevents];

	// now we can re-use the whole event list, but we need to save one slot
	// for the event loop poke
	memcpy(kev, events, sizeof(kev));
	ddi->ddi_maxevents = DISPATCH_DEFERRED_ITEMS_EVENT_COUNT - 2;

	for (int i = 0; i < nevents; i++) {
		_dispatch_kevent_drain(&kev[i]);
	}

	if (wlh == DISPATCH_WLH_ANON) {
		if (ddi->ddi_stashed_dou._do && ddi->ddi_nevents) {
			// We will drain the stashed item and not return to the kernel
			// right away. As a consequence, do not delay these updates.
			_dispatch_event_loop_drain(KEVENT_FLAG_IMMEDIATE |
					KEVENT_FLAG_ERROR_EVENTS);
		}
#if DISPATCH_USE_KEVENT_WORKLOOP
	} else if (dx_metatype((dispatch_queue_t)wlh) == _DISPATCH_WORKLOOP_TYPE) {
		dispatch_timer_heap_t dth = ((dispatch_workloop_t)wlh)->dwl_timer_heap;
		if (dth && dth[0].dth_dirty_bits) {
			_dispatch_event_loop_drain_timers(dth, DISPATCH_TIMER_WLH_COUNT);
		}
#endif // DISPATCH_USE_KEVENT_WORKLOOP
	}
}

void
_dispatch_event_loop_leave_immediate(uint64_t dq_state)
{
#if DISPATCH_USE_KEVENT_WORKLOOP
	dispatch_deferred_items_t ddi = _dispatch_deferred_items_get();
	dispatch_wlh_t wlh = ddi->ddi_wlh;
	uint32_t kev_flags = KEVENT_FLAG_IMMEDIATE | KEVENT_FLAG_ERROR_EVENTS |
			KEVENT_FLAG_DYNAMIC_KQ_MUST_EXIST;
	dispatch_kevent_s ke;
	dispatch_assert(!_dq_state_is_base_wlh(dq_state));

	//
	// A workloop is being retargeted, we need to synchronously destroy
	// the thread request as delivering it later would confuse the workloop
	// thread into trying to drain this queue as a bottom one.
	//
	// Doing it synchronously prevents races where the queue is retargeted
	// again, and becomes a workloop again
	//
	dispatch_assert(ddi->ddi_wlh_needs_delete);
	ddi->ddi_wlh_needs_delete = false;
	ddi->ddi_wlh_needs_update = false;
	_dispatch_kq_fill_workloop_event(&ke,
			DISPATCH_WORKLOOP_RETARGET, wlh, dq_state);
	if (_dispatch_kq_poll(wlh, &ke, 1, &ke, 1, NULL, NULL, kev_flags)) {
		_dispatch_kevent_workloop_drain_error(&ke, 0);
		__builtin_unreachable();
	}
#else
	(void)dq_state;
#endif // DISPATCH_USE_KEVENT_WORKLOOP
}

void
_dispatch_event_loop_leave_deferred(dispatch_deferred_items_t ddi,
		uint64_t dq_state)
{
#if DISPATCH_USE_KEVENT_WORKLOOP
	int action = _dispatch_event_loop_get_action_for_state(dq_state);
	dispatch_assert(ddi->ddi_wlh_needs_delete);
	ddi->ddi_wlh_needs_delete = false;
	ddi->ddi_wlh_needs_update = false;
	_dispatch_kq_fill_ddi_workloop_event(ddi, action, ddi->ddi_wlh, dq_state);
#else
	(void)ddi; (void)dq_state;
#endif // DISPATCH_USE_KEVENT_WORKLOOP
}

void
_dispatch_event_loop_cancel_waiter(dispatch_sync_context_t dsc)
{
#if DISPATCH_USE_KEVENT_WORKLOOP
	dispatch_wlh_t wlh = dsc->dc_data;
	uint32_t kev_flags = KEVENT_FLAG_IMMEDIATE | KEVENT_FLAG_ERROR_EVENTS;
	dispatch_kevent_s ke;

	_dispatch_kq_fill_workloop_sync_event(&ke, DISPATCH_WORKLOOP_SYNC_END,
			wlh, 0, dsc->dsc_waiter);
	if (_dispatch_kq_poll(wlh, &ke, 1, &ke, 1, NULL, NULL, kev_flags)) {
		_dispatch_kevent_workloop_drain_error(&ke, dsc->dsc_waiter_needs_cancel ?
				0 : DISPATCH_KEVENT_WORKLOOP_ALLOW_ENOENT);
		//
		// Our deletion attempt is opportunistic as in most cases we will find
		// the matching knote and break the waiter out.
		//
		// However, if the waiter hasn't had a chance to make the syscall
		// to wait yet, we get ENOENT. In this case, pre-post the WAKE,
		// and transfer the responsibility to delete the knote to the waiter.
		//
		dsc->dsc_waiter_needs_cancel = true;
		_dispatch_kq_fill_workloop_sync_event(&ke,
				DISPATCH_WORKLOOP_SYNC_FAKE, wlh, 0, dsc->dsc_waiter);
		if (_dispatch_kq_poll(wlh, &ke, 1, &ke, 1, NULL, NULL, kev_flags)) {
			_dispatch_kevent_workloop_drain_error(&ke, 0);
			__builtin_unreachable();
		}
	}
#else
	(void)dsc;
#endif // DISPATCH_USE_KEVENT_WORKLOOP
}

void
_dispatch_event_loop_wake_owner(dispatch_sync_context_t dsc,
		dispatch_wlh_t wlh, uint64_t old_state, uint64_t new_state)
{
#if DISPATCH_USE_KEVENT_WORKLOOP
	dispatch_deferred_items_t ddi = _dispatch_deferred_items_get();
	dispatch_wlh_t waiter_wlh = dsc->dc_data;
	uint32_t kev_flags = KEVENT_FLAG_IMMEDIATE | KEVENT_FLAG_ERROR_EVENTS;
	dispatch_kevent_s ke[3];
	int action, n = 0;

	dispatch_assert(_dq_state_drain_locked_by(new_state, dsc->dsc_waiter));

	if (wlh != DISPATCH_WLH_ANON && ddi && ddi->ddi_wlh == wlh) {
		dispatch_assert(ddi->ddi_wlh_needs_delete);
		ddi->ddi_wlh_needs_delete = false;
		ddi->ddi_wlh_needs_update = false;

		if (wlh == waiter_wlh) { // async -> sync handoff
			dispatch_assert(_dq_state_is_enqueued_on_target(old_state));
			dispatch_assert(!_dq_state_in_sync_transfer(old_state));
			dispatch_assert(_dq_state_in_sync_transfer(new_state));

			if (_dq_state_is_enqueued_on_target(new_state)) {
				action = DISPATCH_WORKLOOP_ASYNC_QOS_UPDATE;
			} else {
				action = DISPATCH_WORKLOOP_ASYNC_LEAVE_FROM_TRANSFER;
			}
			_dispatch_kq_fill_ddi_workloop_event(ddi, action, wlh, new_state);

			int slot = _dispatch_kq_deferred_find_slot(ddi, EVFILT_WORKLOOP,
					(uint64_t)wlh, dsc->dsc_waiter);
			if (slot == ddi->ddi_nevents) {
				dispatch_assert(slot < DISPATCH_DEFERRED_ITEMS_EVENT_COUNT);
				ddi->ddi_nevents++;
			}
			_dispatch_kq_fill_workloop_sync_event(&ddi->ddi_eventlist[slot],
					DISPATCH_WORKLOOP_SYNC_WAKE, wlh, new_state, dsc->dsc_waiter);
			return;
		}
	}

	if ((old_state ^ new_state) & DISPATCH_QUEUE_ENQUEUED) {
		dispatch_assert(_dq_state_is_enqueued_on_target(old_state));
		dispatch_assert(_dq_state_in_sync_transfer(new_state));
		// During the handoff, the waiter noticed there was no work *after*
		// that last work item, so we want to kill the thread request while
		// there's an owner around to avoid races betwen knote_process() and
		// knote_drop() in the kernel.
		_dispatch_kq_fill_workloop_event(&ke[n++],
				DISPATCH_WORKLOOP_ASYNC_LEAVE_FROM_TRANSFER, wlh, new_state);
	}
	if (_dq_state_in_sync_transfer(new_state)) {
		// Even when waiter_wlh != wlh we can pretend we got woken up
		// which is a knote we will be able to delete later with a SYNC_END.
		// This allows rectifying incorrect ownership sooner, and also happens
		// on resume if the first item is a sync waiter.
		_dispatch_kq_fill_workloop_sync_event(&ke[n++],
				DISPATCH_WORKLOOP_SYNC_WAKE, wlh, new_state, dsc->dsc_waiter);
	}
	if (_dq_state_in_sync_transfer(old_state)) {
		dispatch_tid tid = _dispatch_tid_self();
		_dispatch_kq_fill_workloop_sync_event(&ke[n++],
				DISPATCH_WORKLOOP_SYNC_END, wlh, new_state, tid);
	}
	//
	// Past this call it is not safe to look at `wlh` anymore as the callers
	// sometimes borrow the refcount of the waiter which we will wake up.
	//
	if (_dispatch_kq_poll(wlh, ke, n, ke, n, NULL, NULL, kev_flags)) {
		_dispatch_kevent_workloop_drain_error(&ke[0], 0);
		__builtin_unreachable();
	}

	if (unlikely(waiter_wlh != DISPATCH_WLH_ANON && waiter_wlh != wlh)) {
		_dispatch_bug_deprecated("Changing target queue hierarchy "
				"with a dispatch_sync in flight");
		_dispatch_event_loop_cancel_waiter(dsc);
	}
#else
	(void)dsc; (void)wlh; (void)old_state; (void)new_state;
#endif // DISPATCH_USE_KEVENT_WORKLOOP
}

void
_dispatch_event_loop_wait_for_ownership(dispatch_sync_context_t dsc)
{
#if DISPATCH_USE_KEVENT_WORKLOOP
	dispatch_wlh_t wlh = dsc->dc_data;
	dispatch_kevent_s ke[2];
	uint32_t kev_flags = KEVENT_FLAG_IMMEDIATE | KEVENT_FLAG_ERROR_EVENTS;
	uint64_t dq_state;
	int i, n = 0;

	dq_state = os_atomic_load2o((dispatch_queue_t)wlh, dq_state, relaxed);
	if (dsc->dsc_wlh_was_first && !_dq_state_drain_locked(dq_state) &&
			_dq_state_is_enqueued_on_target(dq_state)) {
		//
		// <rdar://problem/32123779>
		//
		// When an enqueuer is racing with the servicer draining the item that
		// is being enqueued and going away, it is possible for the enqueuer to
		// mark an empty queue as enqueued and make a thread request for it.
		//
		// If then a thread is selected to deliver this event, but doesn't make
		// it to userland to take the drain lock, any sync waiter will
		// nevertheless have to wait for that servicer to consume the thread
		// request, trying to delete it will be no good. This is why
		// _dispatch_push_sync_waiter() for workloops will not try to "save
		// itself" if the enqueued bit is set.
		//
		// However, we don't know whether this thread request exists, it may
		// have bounced, or still be in the process of being added by a much
		// lower priority thread, so we need to drive it once to avoid priority
		// inversions.
		//
		_dispatch_kq_fill_workloop_event(&ke[n++], DISPATCH_WORKLOOP_ASYNC,
				wlh, dq_state);
	}

again:
	_dispatch_kq_fill_workloop_sync_event(&ke[n++], DISPATCH_WORKLOOP_SYNC_WAIT,
			wlh, dq_state, dsc->dsc_waiter);
	n = _dispatch_kq_poll(wlh, ke, n, ke, n, NULL, NULL, kev_flags);
	for (i = 0; i < n; i++) {
		long flags = 0;
		if (ke[i].fflags & NOTE_WL_SYNC_WAIT) {
			flags = DISPATCH_KEVENT_WORKLOOP_ALLOW_EINTR |
					DISPATCH_KEVENT_WORKLOOP_ALLOW_ESTALE;
		}
		_dispatch_kevent_workloop_drain_error(&ke[i], flags);
	}
	if (n) {
		dispatch_assert(n == 1 && (ke[0].fflags & NOTE_WL_SYNC_WAIT));
		_dispatch_kevent_wlh_debug("restarting", &ke[0]);
		dq_state = ke[0].ext[EV_EXTIDX_WL_VALUE];
		n = 0;
		goto again;
	}
#endif
	if (dsc->dsc_waiter_needs_cancel) {
		_dispatch_event_loop_cancel_waiter(dsc);
		dsc->dsc_waiter_needs_cancel = false;
	}
	if (dsc->dsc_release_storage) {
		_dispatch_queue_release_storage(dsc->dc_data);
	}
}

void
_dispatch_event_loop_end_ownership(dispatch_wlh_t wlh, uint64_t old_state,
		uint64_t new_state, uint32_t flags)
{
#if DISPATCH_USE_KEVENT_WORKLOOP
	uint32_t kev_flags = KEVENT_FLAG_IMMEDIATE | KEVENT_FLAG_ERROR_EVENTS;
	dispatch_kevent_s ke[2];
	bool needs_forceful_end_ownership = false;
	int n = 0;

	dispatch_assert(_dq_state_is_base_wlh(new_state));
	if (_dq_state_is_enqueued_on_target(new_state)) {
		_dispatch_kq_fill_workloop_event(&ke[n++],
				DISPATCH_WORKLOOP_ASYNC_FROM_SYNC, wlh, new_state);
	} else if (_dq_state_is_enqueued_on_target(old_state)) {
		//
		// <rdar://problem/41389180> Because the thread request knote may not
		// have made it, DISPATCH_WORKLOOP_ASYNC_LEAVE_FROM_SYNC may silently
		// turn into a no-op.
		//
		// However, the kernel may know about our ownership anyway, so we need
		// to make sure it is forcefully ended.
		//
		needs_forceful_end_ownership = true;
		dispatch_assert(_dq_state_is_suspended(new_state));
		_dispatch_kq_fill_workloop_event(&ke[n++],
				DISPATCH_WORKLOOP_ASYNC_LEAVE_FROM_SYNC, wlh, new_state);
	} else if (_dq_state_received_sync_wait(old_state)) {
		//
		// This case happens when the current workloop got waited on by some
		// thread calling _dispatch_event_loop_wait_for_ownership.
		//
		// When the workloop became IDLE, it didn't find the sync waiter
		// continuation, didn't have a thread request to cancel either, and so
		// we need the kernel to forget about the current thread ownership
		// of the workloop.
		//
		// To forget this ownership, we create a fake WAKE knote that can not
		// coalesce with any meaningful one, just so that we can EV_DELETE it
		// with the NOTE_WL_END_OWNERSHIP.
		//
		// This is a gross hack, but this will really only ever happen for
		// cases where a sync waiter started to wait on a workloop, but his part
		// of the graph got mutated and retargeted onto a different workloop.
		// In doing so, that sync waiter has snitched to the kernel about
		// ownership, and the workloop he's bogusly waiting on will go through
		// this codepath.
		//
		needs_forceful_end_ownership = true;
	}

	if (_dq_state_in_sync_transfer(old_state)) {
		dispatch_tid tid = _dispatch_tid_self();
		_dispatch_kq_fill_workloop_sync_event(&ke[n++],
				DISPATCH_WORKLOOP_SYNC_END, wlh, new_state, tid);
	} else if (needs_forceful_end_ownership) {
		kev_flags |= KEVENT_FLAG_DYNAMIC_KQ_MUST_EXIST;
		_dispatch_kq_fill_workloop_event(&ke[n++],
				DISPATCH_WORKLOOP_ASYNC_FORCE_END_OWNERSHIP, wlh, new_state);
	}

	if (_dispatch_kq_poll(wlh, ke, n, ke, n, NULL, NULL, kev_flags)) {
		_dispatch_kevent_workloop_drain_error(&ke[0], 0);
		__builtin_unreachable();
	}

	_dispatch_event_loop_assert_not_owned(wlh);

	int extra_refs = (flags & DISPATCH_EVENT_LOOP_CONSUME_2) ? 2 : 0;
	if (_dq_state_is_enqueued_on_target(old_state)) extra_refs++;
	if (_dq_state_is_enqueued_on_target(new_state)) extra_refs--;
	dispatch_assert(extra_refs >= 0);
	if (extra_refs > 0) _dispatch_release_n((dispatch_queue_t)wlh, extra_refs);
#else
	(void)wlh; (void)old_state; (void)new_state; (void)flags;
#endif // DISPATCH_USE_KEVENT_WORKLOOP
}

#if DISPATCH_WLH_DEBUG
void
_dispatch_event_loop_assert_not_owned(dispatch_wlh_t wlh)
{
#if DISPATCH_USE_KEVENT_WORKLOOP
	if (wlh != DISPATCH_WLH_ANON) {
		dispatch_kevent_s ke;
		if (_dispatch_kevent_workloop_get_info(wlh, &ke)) {
			dispatch_assert(ke.ext[0] != _pthread_threadid_self_np_direct());
		}
	}
#else
	(void)wlh;
#endif // DISPATCH_USE_KEVENT_WORKLOOP
}
#endif // DISPATCH_WLH_DEBUG

#pragma mark -
#pragma mark dispatch_event_loop timers

#define DISPATCH_KEVENT_TIMEOUT_IDENT_MASK (~0ull << 8)

DISPATCH_NOINLINE
static void
_dispatch_event_loop_timer_program(dispatch_timer_heap_t dth, uint32_t tidx,
		uint64_t target, uint64_t leeway, uint16_t action)
{
	dispatch_wlh_t wlh = _dispatch_get_wlh();
#if DISPATCH_USE_KEVENT_QOS
	pthread_priority_t pp = _PTHREAD_PRIORITY_EVENT_MANAGER_FLAG;
	if (wlh != DISPATCH_WLH_ANON) {
		pp = _dispatch_qos_to_pp(dth[tidx].dth_max_qos);
	}
#endif
	dispatch_kevent_s ke = {
		.ident = DISPATCH_KEVENT_TIMEOUT_IDENT_MASK | tidx,
		.filter = EVFILT_TIMER,
		.flags = action | EV_ONESHOT,
		.fflags = _dispatch_timer_index_to_fflags[tidx],
		.data = (int64_t)target,
		.udata = (dispatch_kevent_udata_t)dth,
#if DISPATCH_HAVE_TIMER_COALESCING
		.ext[1] = leeway,
#endif
#if DISPATCH_USE_KEVENT_QOS
		.qos = (__typeof__(ke.qos))pp,
#endif
	};
	(void)leeway; // if !DISPATCH_HAVE_TIMER_COALESCING

	_dispatch_kq_deferred_update(wlh, &ke);
}

void
_dispatch_event_loop_timer_arm(dispatch_timer_heap_t dth, uint32_t tidx,
		dispatch_timer_delay_s range, dispatch_clock_now_cache_t nows)
{
	dispatch_clock_t clock = DISPATCH_TIMER_CLOCK(tidx);
	uint64_t target = range.delay + _dispatch_time_now_cached(clock, nows);
	if (unlikely(_dispatch_timers_force_max_leeway)) {
		target += range.leeway;
		range.leeway = 0;
	}

	_dispatch_event_loop_timer_program(dth, tidx, target, range.leeway,
			EV_ADD | EV_ENABLE);
#if HAVE_MACH
	if (clock == DISPATCH_CLOCK_WALL) {
		_dispatch_mach_host_calendar_change_register();
	}
#endif
}

void
_dispatch_event_loop_timer_delete(dispatch_timer_heap_t dth, uint32_t tidx)
{
	_dispatch_event_loop_timer_program(dth, tidx, 0, 0, EV_DELETE);
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_kevent_timer_drain(dispatch_kevent_t ke)
{
	dispatch_timer_heap_t dth = (dispatch_timer_heap_t)ke->udata;
	uint32_t tidx = ke->ident & ~DISPATCH_KEVENT_TIMEOUT_IDENT_MASK;

	dispatch_assert(ke->data > 0);
	dispatch_assert(ke->ident == (tidx | DISPATCH_KEVENT_TIMEOUT_IDENT_MASK));
	dispatch_assert(tidx < DISPATCH_TIMER_COUNT);

	_dispatch_timers_heap_dirty(dth, tidx);
	dth[tidx].dth_needs_program = true;
	dth[tidx].dth_armed = false;
}

#pragma mark -
#pragma mark kevent specific sources

static dispatch_unote_t
_dispatch_source_proc_create(dispatch_source_type_t dst DISPATCH_UNUSED,
		uintptr_t handle, unsigned long mask DISPATCH_UNUSED)
{
	dispatch_unote_t du = _dispatch_unote_create_with_handle(dst, handle, mask);
	if (du._du && (mask & DISPATCH_PROC_EXIT_STATUS)) {
		du._du->du_has_extended_status = true;
	}
	return du;
}

const dispatch_source_type_s _dispatch_source_type_proc = {
	.dst_kind       = "proc",
	.dst_filter     = EVFILT_PROC,
	.dst_flags      = DISPATCH_EV_DIRECT|EV_CLEAR,
	.dst_fflags     = NOTE_EXIT, // rdar://16655831
	.dst_mask       = NOTE_EXIT|NOTE_FORK|NOTE_EXEC|NOTE_EXITSTATUS
#if HAVE_DECL_NOTE_SIGNAL
			|NOTE_SIGNAL
#endif
#if HAVE_DECL_NOTE_REAP
			|NOTE_REAP
#endif
			,
	.dst_action     = DISPATCH_UNOTE_ACTION_SOURCE_OR_FFLAGS,
	.dst_size       = sizeof(struct dispatch_source_refs_s),
	.dst_strict     = false,

	.dst_create     = _dispatch_source_proc_create,
	.dst_merge_evt  = _dispatch_source_merge_evt,
};

const dispatch_source_type_s _dispatch_source_type_vnode = {
	.dst_kind       = "vnode",
	.dst_filter     = EVFILT_VNODE,
	.dst_flags      = DISPATCH_EV_DIRECT|EV_CLEAR|EV_VANISHED,
	.dst_mask       = NOTE_DELETE|NOTE_WRITE|NOTE_EXTEND|NOTE_ATTRIB|NOTE_LINK
			|NOTE_RENAME|NOTE_FUNLOCK
#if HAVE_DECL_NOTE_REVOKE
			|NOTE_REVOKE
#endif
#if HAVE_DECL_NOTE_NONE
			|NOTE_NONE
#endif
			,
	.dst_action     = DISPATCH_UNOTE_ACTION_SOURCE_OR_FFLAGS,
	.dst_size       = sizeof(struct dispatch_source_refs_s),
	.dst_strict     = false,

	.dst_create     = _dispatch_unote_create_with_fd,
	.dst_merge_evt  = _dispatch_source_merge_evt,
};

const dispatch_source_type_s _dispatch_source_type_vfs = {
	.dst_kind       = "vfs",
	.dst_filter     = EVFILT_FS,
	.dst_flags      = DISPATCH_EV_DIRECT|EV_CLEAR,
	.dst_mask       = VQ_NOTRESP|VQ_NEEDAUTH|VQ_LOWDISK|VQ_MOUNT|VQ_UNMOUNT
			|VQ_DEAD|VQ_ASSIST|VQ_NOTRESPLOCK
#if HAVE_DECL_VQ_UPDATE
			|VQ_UPDATE
#endif
#if HAVE_DECL_VQ_VERYLOWDISK
			|VQ_VERYLOWDISK
#endif
#if HAVE_DECL_VQ_QUOTA
			|VQ_QUOTA
#endif
#if HAVE_DECL_VQ_NEARLOWDISK
			|VQ_NEARLOWDISK
#endif
#if HAVE_DECL_VQ_DESIRED_DISK
			|VQ_DESIRED_DISK
#endif
#if HAVE_DECL_VQ_FREE_SPACE_CHANGE
			|VQ_FREE_SPACE_CHANGE
#endif
			,
	.dst_action     = DISPATCH_UNOTE_ACTION_SOURCE_OR_FFLAGS,
	.dst_size       = sizeof(struct dispatch_source_refs_s),
	.dst_strict     = false,

	.dst_create     = _dispatch_unote_create_without_handle,
	.dst_merge_evt  = _dispatch_source_merge_evt,
};

#ifdef EVFILT_SOCK
const dispatch_source_type_s _dispatch_source_type_sock = {
	.dst_kind       = "sock",
	.dst_filter     = EVFILT_SOCK,
	.dst_flags      = DISPATCH_EV_DIRECT|EV_CLEAR|EV_VANISHED,
	.dst_mask       = NOTE_CONNRESET|NOTE_READCLOSED|NOTE_WRITECLOSED
			|NOTE_TIMEOUT|NOTE_NOSRCADDR|NOTE_IFDENIED|NOTE_SUSPEND|NOTE_RESUME
			|NOTE_KEEPALIVE
#ifdef NOTE_ADAPTIVE_WTIMO
			|NOTE_ADAPTIVE_WTIMO|NOTE_ADAPTIVE_RTIMO
#endif
#ifdef NOTE_CONNECTED
			|NOTE_CONNECTED|NOTE_DISCONNECTED|NOTE_CONNINFO_UPDATED
#endif
#ifdef NOTE_NOTIFY_ACK
			|NOTE_NOTIFY_ACK
#endif
		,
	.dst_action     = DISPATCH_UNOTE_ACTION_SOURCE_OR_FFLAGS,
	.dst_size       = sizeof(struct dispatch_source_refs_s),
	.dst_strict     = false,

	.dst_create     = _dispatch_unote_create_with_fd,
	.dst_merge_evt  = _dispatch_source_merge_evt,
};
#endif // EVFILT_SOCK

#ifdef EVFILT_NW_CHANNEL
const dispatch_source_type_s _dispatch_source_type_nw_channel = {
	.dst_kind       = "nw_channel",
	.dst_filter     = EVFILT_NW_CHANNEL,
	.dst_flags      = DISPATCH_EV_DIRECT|EV_CLEAR|EV_VANISHED,
	.dst_mask       = NOTE_FLOW_ADV_UPDATE|NOTE_CHANNEL_EVENT|NOTE_IF_ADV_UPD,
	.dst_action     = DISPATCH_UNOTE_ACTION_SOURCE_OR_FFLAGS,
	.dst_size       = sizeof(struct dispatch_source_refs_s),
	.dst_strict     = false,

	.dst_create     = _dispatch_unote_create_with_fd,
	.dst_merge_evt  = _dispatch_source_merge_evt,
};
#endif // EVFILT_NW_CHANNEL

#if DISPATCH_USE_MEMORYSTATUS

#if DISPATCH_USE_MEMORYPRESSURE_SOURCE
#define DISPATCH_MEMORYPRESSURE_SOURCE_MASK ( \
		DISPATCH_MEMORYPRESSURE_NORMAL | \
		DISPATCH_MEMORYPRESSURE_WARN | \
		DISPATCH_MEMORYPRESSURE_CRITICAL | \
		DISPATCH_MEMORYPRESSURE_PROC_LIMIT_WARN | \
		DISPATCH_MEMORYPRESSURE_PROC_LIMIT_CRITICAL | \
		DISPATCH_MEMORYPRESSURE_MSL_STATUS)

#define DISPATCH_MEMORYPRESSURE_MALLOC_MASK ( \
		DISPATCH_MEMORYPRESSURE_WARN | \
		DISPATCH_MEMORYPRESSURE_CRITICAL | \
		DISPATCH_MEMORYPRESSURE_PROC_LIMIT_WARN | \
		DISPATCH_MEMORYPRESSURE_PROC_LIMIT_CRITICAL | \
		DISPATCH_MEMORYPRESSURE_MSL_STATUS)


static void
_dispatch_memorypressure_handler(void *context)
{
	dispatch_source_t ds = context;
	unsigned long memorypressure = dispatch_source_get_data(ds);

	if (memorypressure & DISPATCH_MEMORYPRESSURE_NORMAL) {
		_dispatch_memory_warn = false;
		_dispatch_continuation_cache_limit = DISPATCH_CONTINUATION_CACHE_LIMIT;
#if VOUCHER_USE_MACH_VOUCHER
		if (_firehose_task_buffer) {
			firehose_buffer_clear_bank_flags(_firehose_task_buffer,
					FIREHOSE_BUFFER_BANK_FLAG_LOW_MEMORY);
		}
#endif
	}
	if (memorypressure & DISPATCH_MEMORYPRESSURE_WARN) {
		_dispatch_memory_warn = true;
		_dispatch_continuation_cache_limit =
				DISPATCH_CONTINUATION_CACHE_LIMIT_MEMORYPRESSURE_PRESSURE_WARN;
#if VOUCHER_USE_MACH_VOUCHER
		if (_firehose_task_buffer) {
			firehose_buffer_set_bank_flags(_firehose_task_buffer,
					FIREHOSE_BUFFER_BANK_FLAG_LOW_MEMORY);
		}
#endif
	}
	memorypressure &= DISPATCH_MEMORYPRESSURE_MALLOC_MASK;
	if (memorypressure) {
		malloc_memory_event_handler(memorypressure);
	}
}

static void
_dispatch_memorypressure_init(void)
{
	dispatch_source_t ds = dispatch_source_create(
			DISPATCH_SOURCE_TYPE_MEMORYPRESSURE, 0,
			DISPATCH_MEMORYPRESSURE_SOURCE_MASK, _dispatch_mgr_q._as_dq);
	dispatch_set_context(ds, ds);
	dispatch_source_set_event_handler_f(ds, _dispatch_memorypressure_handler);
	dispatch_activate(ds);
}
#endif // DISPATCH_USE_MEMORYPRESSURE_SOURCE

#if TARGET_OS_SIMULATOR // rdar://problem/9219483
static int _dispatch_ios_simulator_memory_warnings_fd = -1;
static void
_dispatch_ios_simulator_memorypressure_init(void *context DISPATCH_UNUSED)
{
	char *e = getenv("SIMULATOR_MEMORY_WARNINGS");
	if (!e) return;
	_dispatch_ios_simulator_memory_warnings_fd = open(e, O_EVTONLY);
	if (_dispatch_ios_simulator_memory_warnings_fd == -1) {
		(void)dispatch_assume_zero(errno);
	}
}

static dispatch_unote_t
_dispatch_source_memorypressure_create(dispatch_source_type_t dst,
	uintptr_t handle, unsigned long mask)
{
	static dispatch_once_t pred;
	dispatch_once_f(&pred, NULL, _dispatch_ios_simulator_memorypressure_init);

	if (handle) {
		return DISPATCH_UNOTE_NULL;
	}

	dst = &_dispatch_source_type_vnode;
	handle = (uintptr_t)_dispatch_ios_simulator_memory_warnings_fd;
	mask = NOTE_ATTRIB;

	dispatch_unote_t du = dux_create(dst, handle, mask);
	if (du._du) {
		du._du->du_memorypressure_override = true;
	}
	return du;
}
#endif // TARGET_OS_SIMULATOR

const dispatch_source_type_s _dispatch_source_type_memorypressure = {
	.dst_kind       = "memorystatus",
	.dst_filter     = EVFILT_MEMORYSTATUS,
	.dst_flags      = EV_UDATA_SPECIFIC|EV_DISPATCH,
	.dst_mask       = NOTE_MEMORYSTATUS_PRESSURE_NORMAL
			|NOTE_MEMORYSTATUS_PRESSURE_WARN|NOTE_MEMORYSTATUS_PRESSURE_CRITICAL
			|NOTE_MEMORYSTATUS_LOW_SWAP|NOTE_MEMORYSTATUS_PROC_LIMIT_WARN
			|NOTE_MEMORYSTATUS_PROC_LIMIT_CRITICAL
			|NOTE_MEMORYSTATUS_MSL_STATUS,
	.dst_action     = DISPATCH_UNOTE_ACTION_SOURCE_OR_FFLAGS,
	.dst_size       = sizeof(struct dispatch_source_refs_s),
	.dst_strict     = false,

#if TARGET_OS_SIMULATOR
	.dst_create     = _dispatch_source_memorypressure_create,
	// redirected to _dispatch_source_type_vnode
#else
	.dst_create     = _dispatch_unote_create_without_handle,
	.dst_merge_evt  = _dispatch_source_merge_evt,
#endif
};

static dispatch_unote_t
_dispatch_source_vm_create(dispatch_source_type_t dst DISPATCH_UNUSED,
		uintptr_t handle, unsigned long mask DISPATCH_UNUSED)
{
	// Map legacy vm pressure to memorypressure warning rdar://problem/15907505
	dispatch_unote_t du = dux_create(&_dispatch_source_type_memorypressure,
			handle, NOTE_MEMORYSTATUS_PRESSURE_WARN);
	if (du._du) {
		du._du->du_vmpressure_override = 1;
	}
	return du;
}

const dispatch_source_type_s _dispatch_source_type_vm = {
	.dst_kind       = "vm (deprecated)",
	.dst_filter     = EVFILT_MEMORYSTATUS,
	.dst_flags      = EV_UDATA_SPECIFIC|EV_DISPATCH,
	.dst_mask       = NOTE_VM_PRESSURE,
	.dst_action     = DISPATCH_UNOTE_ACTION_SOURCE_OR_FFLAGS,
	.dst_size       = sizeof(struct dispatch_source_refs_s),
	.dst_strict     = false,

	.dst_create     = _dispatch_source_vm_create,
	// redirected to _dispatch_source_type_memorypressure
};
#endif // DISPATCH_USE_MEMORYSTATUS

#pragma mark mach send / notifications
#if HAVE_MACH

// Flags for all notifications that are registered/unregistered when a
// send-possible notification is requested/delivered
#define _DISPATCH_MACH_SP_FLAGS (DISPATCH_MACH_SEND_POSSIBLE| \
		DISPATCH_MACH_SEND_DEAD|DISPATCH_MACH_SEND_DELETED)

static void _dispatch_mach_host_notify_update(void *context);

DISPATCH_STATIC_GLOBAL(dispatch_once_t _dispatch_mach_notify_port_pred);
DISPATCH_STATIC_GLOBAL(dispatch_once_t _dispatch_mach_calendar_pred);
DISPATCH_STATIC_GLOBAL(mach_port_t _dispatch_mach_notify_port);

static void
_dispatch_timers_calendar_change(void)
{
	dispatch_timer_heap_t dth = _dispatch_timers_heap;
	uint32_t qos, tidx;

	// calendar change may have gone past the wallclock deadline
	for (qos = 0; qos < DISPATCH_TIMER_QOS_COUNT; qos++) {
		tidx = DISPATCH_TIMER_INDEX(DISPATCH_CLOCK_WALL, qos);
		_dispatch_timers_heap_dirty(dth, tidx);
		dth[tidx].dth_needs_program = true;
	}
}

static mach_msg_audit_trailer_t *
_dispatch_mach_msg_get_audit_trailer(mach_msg_header_t *hdr)
{
	mach_msg_trailer_t *tlr = NULL;
	mach_msg_audit_trailer_t *audit_tlr = NULL;
	tlr = (mach_msg_trailer_t *)((unsigned char *)hdr +
			round_msg(hdr->msgh_size));
	// The trailer should always be of format zero.
	if (tlr->msgh_trailer_type == MACH_MSG_TRAILER_FORMAT_0) {
		if (tlr->msgh_trailer_size >= sizeof(mach_msg_audit_trailer_t)) {
			audit_tlr = (mach_msg_audit_trailer_t *)tlr;
		}
	}
	return audit_tlr;
}

bool
_dispatch_mach_msg_sender_is_kernel(mach_msg_header_t *hdr)
{
	mach_msg_audit_trailer_t *tlr;
	tlr = _dispatch_mach_msg_get_audit_trailer(hdr);
	if (!tlr) {
		DISPATCH_INTERNAL_CRASH(0, "message received without expected trailer");
	}

	return tlr->msgh_audit.val[DISPATCH_MACH_AUDIT_TOKEN_PID] == 0;
}

DISPATCH_NOINLINE
static void
_dispatch_mach_notification_merge_msg(dispatch_unote_t du, uint32_t flags,
		mach_msg_header_t *hdr, mach_msg_size_t msgsz DISPATCH_UNUSED,
		pthread_priority_t msg_pp DISPATCH_UNUSED,
		pthread_priority_t ovr_pp DISPATCH_UNUSED)
{
	mig_reply_error_t reply;
	dispatch_assert(sizeof(mig_reply_error_t) == sizeof(union
		__ReplyUnion___dispatch_libdispatch_internal_protocol_subsystem));
	dispatch_assert(sizeof(mig_reply_error_t) <
			DISPATCH_MACH_RECEIVE_MAX_INLINE_MESSAGE_SIZE);
	if (hdr->msgh_id <= MACH_NOTIFY_LAST &&
			!dispatch_assume(_dispatch_mach_msg_sender_is_kernel(hdr))) {
		mach_msg_destroy(hdr);
		goto out;
	}

	boolean_t success = libdispatch_internal_protocol_server(hdr, &reply.Head);
	if (!success && reply.RetCode == MIG_BAD_ID &&
			(hdr->msgh_id == HOST_CALENDAR_SET_REPLYID ||
			hdr->msgh_id == HOST_CALENDAR_CHANGED_REPLYID)) {
		_dispatch_debug("calendar-change notification");
		_dispatch_timers_calendar_change();
		_dispatch_mach_host_notify_update(NULL);
		success = TRUE;
		reply.RetCode = KERN_SUCCESS;
	}
	if (dispatch_assume(success) && reply.RetCode != MIG_NO_REPLY) {
		(void)dispatch_assume_zero(reply.RetCode);
	}
	if (!success || (reply.RetCode && reply.RetCode != MIG_NO_REPLY)) {
		mach_msg_destroy(hdr);
	}

out:
	if (flags & DISPATCH_EV_MSG_NEEDS_FREE) {
		free(hdr);
	}
	return _dispatch_unote_resume(du);
}

DISPATCH_NOINLINE
static void
_dispatch_mach_notify_port_init(void *context DISPATCH_UNUSED)
{
	mach_port_options_t opts = { .flags = MPO_CONTEXT_AS_GUARD | MPO_STRICT };
	mach_port_context_t guard = (uintptr_t)&_dispatch_mach_notify_port;
	kern_return_t kr;

	kr = mach_port_construct(mach_task_self(), &opts, guard,
			&_dispatch_mach_notify_port);
	if (unlikely(kr)) {
		DISPATCH_CLIENT_CRASH(kr,
				"mach_port_construct() failed: cannot create receive right");
	}

	dispatch_unote_t du = dux_create(&_dispatch_mach_type_notification,
			_dispatch_mach_notify_port, 0);

	// make sure _dispatch_kevent_mach_msg_recv can call
	// _dispatch_retain_unote_owner
	du._du->du_owner_wref = _dispatch_ptr2wref(&_dispatch_mgr_q);

	dispatch_assume(_dispatch_unote_register(du, DISPATCH_WLH_ANON,
			DISPATCH_PRIORITY_FLAG_MANAGER));
}

static void
_dispatch_mach_host_port_init(void *ctxt DISPATCH_UNUSED)
{
	kern_return_t kr;
	mach_port_t mp, mhp = mach_host_self();
	kr = host_get_host_port(mhp, &mp);
	DISPATCH_VERIFY_MIG(kr);
	if (likely(!kr)) {
		// mach_host_self returned the HOST_PRIV port
		kr = mach_port_deallocate(mach_task_self(), mhp);
		DISPATCH_VERIFY_MIG(kr);
		mhp = mp;
	} else if (kr != KERN_INVALID_ARGUMENT) {
		(void)dispatch_assume_zero(kr);
	}
	if (unlikely(!mhp)) {
		DISPATCH_CLIENT_CRASH(kr, "Could not get unprivileged host port");
	}
	_dispatch_mach_host_port = mhp;
}

mach_port_t
_dispatch_get_mach_host_port(void)
{
	dispatch_once_f(&_dispatch_mach_host_port_pred, NULL,
			_dispatch_mach_host_port_init);
	return _dispatch_mach_host_port;
}

DISPATCH_ALWAYS_INLINE
static inline mach_port_t
_dispatch_get_mach_notify_port(void)
{
	dispatch_once_f(&_dispatch_mach_notify_port_pred, NULL,
			_dispatch_mach_notify_port_init);
	return _dispatch_mach_notify_port;
}

static void
_dispatch_mach_host_notify_update(void *context DISPATCH_UNUSED)
{
	kern_return_t kr;
	_dispatch_debug("registering for calendar-change notification");

	kr = host_request_notification(_dispatch_get_mach_host_port(),
			HOST_NOTIFY_CALENDAR_SET, _dispatch_get_mach_notify_port());
	DISPATCH_VERIFY_MIG(kr);
	(void)dispatch_assume_zero(kr);
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_mach_host_calendar_change_register(void)
{
	dispatch_once_f(&_dispatch_mach_calendar_pred, NULL,
			_dispatch_mach_host_notify_update);
}

static kern_return_t
_dispatch_mach_notify_update(dispatch_muxnote_t dmn, uint32_t new_flags,
		uint32_t del_flags, uint32_t mask, mach_msg_id_t notify_msgid,
		mach_port_mscount_t notify_sync)
{
	mach_port_t previous, port = (mach_port_t)dmn->dmn_kev.ident;
	__typeof__(dmn->dmn_kev.data) prev = dmn->dmn_kev.data;
	kern_return_t kr, krr = 0;

	// Update notification registration state.
	dmn->dmn_kev.data |= (new_flags | dmn->dmn_kev.fflags) & mask;
	dmn->dmn_kev.data &= ~(del_flags & mask);

	_dispatch_debug_machport(port);
	if ((dmn->dmn_kev.data & mask) && !(prev & mask)) {
		_dispatch_debug("machport[0x%08x]: registering for send-possible "
				"notification", port);
		previous = MACH_PORT_NULL;
		krr = mach_port_request_notification(mach_task_self(), port,
				notify_msgid, notify_sync, _dispatch_get_mach_notify_port(),
				MACH_MSG_TYPE_MAKE_SEND_ONCE, &previous);
		DISPATCH_VERIFY_MIG(krr);

		switch (krr) {
		case KERN_INVALID_NAME:
		case KERN_INVALID_RIGHT:
			// Suppress errors & clear registration state
			dmn->dmn_kev.data &= ~mask;
			break;
		default:
			// Else, we don't expect any errors from mach. Log any errors
			if (dispatch_assume_zero(krr)) {
				// log the error & clear registration state
				dmn->dmn_kev.data &= ~mask;
			} else if (dispatch_assume_zero(previous)) {
				// Another subsystem has beat libdispatch to requesting the
				// specified Mach notification on this port. We should
				// technically cache the previous port and message it when the
				// kernel messages our port. Or we can just say screw those
				// subsystems and deallocate the previous port.
				// They should adopt libdispatch :-P
				kr = mach_port_deallocate(mach_task_self(), previous);
				DISPATCH_VERIFY_MIG(kr);
				(void)dispatch_assume_zero(kr);
				previous = MACH_PORT_NULL;
			}
		}
	} else if (!(dmn->dmn_kev.data & mask) && (prev & mask)) {
		_dispatch_debug("machport[0x%08x]: unregistering for send-possible "
				"notification", port);
		previous = MACH_PORT_NULL;
		kr = mach_port_request_notification(mach_task_self(), port,
				notify_msgid, notify_sync, MACH_PORT_NULL,
				MACH_MSG_TYPE_MOVE_SEND_ONCE, &previous);
		DISPATCH_VERIFY_MIG(kr);

		switch (kr) {
		case KERN_INVALID_NAME:
		case KERN_INVALID_RIGHT:
		case KERN_INVALID_ARGUMENT:
			break;
		default:
			if (dispatch_assume_zero(kr)) {
				// log the error
			}
		}
	} else {
		return 0;
	}
	if (unlikely(previous)) {
		// the kernel has not consumed the send-once right yet
		(void)dispatch_assume_zero(
				_dispatch_send_consume_send_once_right(previous));
	}
	return krr;
}

static bool
_dispatch_kevent_mach_notify_resume(dispatch_muxnote_t dmn, uint32_t new_flags,
		uint32_t del_flags)
{
	kern_return_t kr = KERN_SUCCESS;
	dispatch_assert_zero(new_flags & del_flags);
	if ((new_flags & _DISPATCH_MACH_SP_FLAGS) ||
			(del_flags & _DISPATCH_MACH_SP_FLAGS)) {
		// Requesting a (delayed) non-sync send-possible notification
		// registers for both immediate dead-name notification and delayed-arm
		// send-possible notification for the port.
		// The send-possible notification is armed when a mach_msg() with the
		// the MACH_SEND_NOTIFY to the port times out.
		// If send-possible is unavailable, fall back to immediate dead-name
		// registration rdar://problem/2527840&9008724
		kr = _dispatch_mach_notify_update(dmn, new_flags, del_flags,
				_DISPATCH_MACH_SP_FLAGS, MACH_NOTIFY_SEND_POSSIBLE,
				MACH_NOTIFY_SEND_POSSIBLE == MACH_NOTIFY_DEAD_NAME);
	}
	return kr == KERN_SUCCESS;
}

DISPATCH_NOINLINE
static void
_dispatch_mach_notify_merge(mach_port_t name, uint32_t data, bool final)
{
	dispatch_unote_linkage_t dul, dul_next;
	dispatch_muxnote_t dmn;
	uint32_t flags = EV_ENABLE;

	_dispatch_debug_machport(name);
	dmn = _dispatch_mach_muxnote_find(name, DISPATCH_EVFILT_MACH_NOTIFICATION);
	if (!dmn) {
		return;
	}

	dmn->dmn_kev.data &= ~_DISPATCH_MACH_SP_FLAGS;
	if (final || !_dispatch_kevent_mach_notify_resume(dmn, data, 0)) {
		flags = EV_ONESHOT;
		dmn->dmn_kev.flags |= EV_DELETE;
	}
	os_atomic_store(&DISPATCH_MACH_NOTIFICATION_ARMED(dmn), 0, relaxed);

	LIST_FOREACH_SAFE(dul, &dmn->dmn_unotes_head, du_link, dul_next) {
		if (os_atomic_load(&DISPATCH_MACH_NOTIFICATION_ARMED(dmn), relaxed)) {
			dispatch_assert(!final);
			break;
		}
		dispatch_unote_t du = _dispatch_unote_linkage_get_unote(dul);
		uint32_t fflags = (data & du._du->du_fflags);
		os_atomic_store2o(du._du, dmsr_notification_armed, 0, relaxed);
		if (final || fflags) {
			// consumed by dux_merge_evt()
			_dispatch_retain_unote_owner(du);
			if (final) _dispatch_unote_unregister_muxed(du);
			if (fflags && dux_type(du._du)->dst_action ==
					DISPATCH_UNOTE_ACTION_SOURCE_OR_FFLAGS) {
				os_atomic_or2o(du._dr, ds_pending_data, fflags, relaxed);
			}
			dux_merge_evt(du._du, flags, fflags, 0);
		}
	}
}

kern_return_t
_dispatch_mach_notify_port_deleted(mach_port_t notify DISPATCH_UNUSED,
		mach_port_name_t name)
{
#if DISPATCH_DEBUG
	_dispatch_log("Corruption: Mach send/send-once/dead-name right 0x%x "
			"deleted prematurely", name);
#endif
	_dispatch_debug_machport(name);
	_dispatch_mach_notify_merge(name, DISPATCH_MACH_SEND_DELETED, true);
	return KERN_SUCCESS;
}

kern_return_t
_dispatch_mach_notify_dead_name(mach_port_t notify DISPATCH_UNUSED,
		mach_port_name_t name)
{
	kern_return_t kr;

	_dispatch_debug("machport[0x%08x]: dead-name notification", name);
	_dispatch_debug_machport(name);
	_dispatch_mach_notify_merge(name, DISPATCH_MACH_SEND_DEAD, true);

	// the act of receiving a dead name notification allocates a dead-name
	// right that must be deallocated
	kr = mach_port_deallocate(mach_task_self(), name);
	DISPATCH_VERIFY_MIG(kr);
	//(void)dispatch_assume_zero(kr);
	return KERN_SUCCESS;
}

kern_return_t
_dispatch_mach_notify_send_possible(mach_port_t notify DISPATCH_UNUSED,
		mach_port_name_t name)
{
	_dispatch_debug("machport[0x%08x]: send-possible notification", name);
	_dispatch_debug_machport(name);
	_dispatch_mach_notify_merge(name, DISPATCH_MACH_SEND_POSSIBLE, false);
	return KERN_SUCCESS;
}

void
_dispatch_mach_notification_set_armed(dispatch_mach_send_refs_t dmsr)
{
	dispatch_muxnote_t dmn = _dispatch_unote_get_linkage(dmsr)->du_muxnote;
	dispatch_unote_linkage_t dul;
	if (dmn) {
#if HAVE_MACH
		os_atomic_store(&DISPATCH_MACH_NOTIFICATION_ARMED(dmn), 1, relaxed);
		LIST_FOREACH(dul, &dmn->dmn_unotes_head, du_link) {
			dispatch_unote_t du = _dispatch_unote_linkage_get_unote(dul);
			os_atomic_store2o(du._du, dmsr_notification_armed, 1, relaxed);
		}
		_dispatch_debug("machport[0x%08x]: send-possible notification armed",
				(mach_port_name_t)dmn->dmn_kev.ident);
#endif
	}
}

static dispatch_unote_t
_dispatch_source_mach_send_create(dispatch_source_type_t dst,
		uintptr_t handle, unsigned long mask)
{
	if (!mask) {
		// Preserve legacy behavior that (mask == 0) => DISPATCH_MACH_SEND_DEAD
		mask = DISPATCH_MACH_SEND_DEAD;
	}
	if (!handle) {
		handle = MACH_PORT_DEAD; // <rdar://problem/27651332>
	}
	return _dispatch_unote_create_with_handle(dst, handle, mask);
}

static bool
_dispatch_mach_send_update(dispatch_muxnote_t dmn)
{
	if (dmn->dmn_kev.flags & EV_DELETE) {
		return _dispatch_kevent_mach_notify_resume(dmn, 0, dmn->dmn_kev.fflags);
	} else {
		return _dispatch_kevent_mach_notify_resume(dmn, dmn->dmn_kev.fflags, 0);
	}
}

const dispatch_source_type_s _dispatch_source_type_mach_send = {
	.dst_kind       = "mach_send",
	.dst_filter     = DISPATCH_EVFILT_MACH_NOTIFICATION,
	.dst_flags      = EV_CLEAR,
	.dst_mask       = DISPATCH_MACH_SEND_DEAD|DISPATCH_MACH_SEND_POSSIBLE,
	.dst_action     = DISPATCH_UNOTE_ACTION_SOURCE_OR_FFLAGS,
	.dst_size       = sizeof(struct dispatch_source_refs_s),
	.dst_strict     = false,

	.dst_create     = _dispatch_source_mach_send_create,
	.dst_update_mux = _dispatch_mach_send_update,
	.dst_merge_evt  = _dispatch_source_merge_evt,
};

static dispatch_unote_t
_dispatch_mach_send_create(dispatch_source_type_t dst,
	uintptr_t handle, unsigned long mask)
{
	// without handle because the mach code will set the ident later
	dispatch_unote_t du =
			_dispatch_unote_create_without_handle(dst, handle, mask);
	if (du._dmsr) {
		du._dmsr->dmsr_disconnect_cnt = DISPATCH_MACH_NEVER_CONNECTED;
		LIST_INIT(&du._dmsr->dmsr_replies);
	}
	return du;
}

const dispatch_source_type_s _dispatch_mach_type_send = {
	.dst_kind       = "mach_send (mach)",
	.dst_filter     = DISPATCH_EVFILT_MACH_NOTIFICATION,
	.dst_flags      = EV_CLEAR,
	.dst_mask       = DISPATCH_MACH_SEND_DEAD|DISPATCH_MACH_SEND_POSSIBLE,
	.dst_action     = DISPATCH_UNOTE_ACTION_PASS_FFLAGS,
	.dst_size       = sizeof(struct dispatch_mach_send_refs_s),
	.dst_strict     = false,

	.dst_create     = _dispatch_mach_send_create,
	.dst_update_mux = _dispatch_mach_send_update,
	.dst_merge_evt  = _dispatch_mach_notification_merge_evt,
};

#endif // HAVE_MACH
#pragma mark mach recv / reply
#if HAVE_MACH

static void
_dispatch_kevent_mach_msg_recv(dispatch_unote_t du, uint32_t flags,
		mach_msg_header_t *hdr, pthread_priority_t msg_pp,
		pthread_priority_t ovr_pp)
{
	mach_port_t name = hdr->msgh_local_port;
	mach_msg_size_t siz;

	if (os_add_overflow(hdr->msgh_size, DISPATCH_MACH_TRAILER_SIZE, &siz)) {
		DISPATCH_CLIENT_CRASH(hdr->msgh_size, "Overlarge message received");
	}
	if (os_unlikely(name == MACH_PORT_NULL)) {
		DISPATCH_CLIENT_CRASH(hdr->msgh_id, "Received message with "
				"MACH_PORT_NULL msgh_local_port");
	}

	_dispatch_debug_machport(name);
	// consumed by dux_merge_evt()
	_dispatch_retain_unote_owner(du);
	_dispatch_kevent_merge_ev_flags(du, flags);
	return dux_merge_msg(du._du, flags, hdr, siz, msg_pp, ovr_pp);
}

DISPATCH_NOINLINE
static void
_dispatch_kevent_mach_msg_drain(dispatch_kevent_t ke)
{
	mach_msg_header_t *hdr = _dispatch_kevent_mach_msg_buf(ke);
	mach_msg_size_t siz = _dispatch_kevent_mach_msg_size(ke);
	dispatch_unote_t du = _dispatch_kevent_get_unote(ke);
	pthread_priority_t msg_pp = (pthread_priority_t)(ke->ext[2] >> 32);
	pthread_priority_t ovr_pp = (pthread_priority_t)ke->qos;
	uint32_t flags = ke->flags;
	mach_msg_return_t kr = (mach_msg_return_t)ke->fflags;

	if (unlikely(kr == MACH_RCV_TOO_LARGE)) {
		if (unlikely(!siz)) {
			DISPATCH_INTERNAL_CRASH(kr, "EVFILT_MACHPORT with no message size");
		}
	} else if (unlikely(kr == MACH_RCV_INVALID_DATA)) {
		dispatch_assert(siz == 0);
		DISPATCH_CLIENT_CRASH(kr, "Unable to copyout msg, possible port leak");
	} else {
		if (unlikely(!hdr)) {
			DISPATCH_INTERNAL_CRASH(kr, "EVFILT_MACHPORT with no message");
		}
		if (likely(!kr)) {
			return _dispatch_kevent_mach_msg_recv(du, flags, hdr, msg_pp, ovr_pp);
		}
		goto out;
	}

	if (!ke->data) {
		DISPATCH_INTERNAL_CRASH(0, "MACH_RCV_LARGE_IDENTITY with no identity");
	}
	if (unlikely(ke->ext[1] > (UINT_MAX - DISPATCH_MACH_TRAILER_SIZE))) {
		DISPATCH_INTERNAL_CRASH(ke->ext[1],
				"EVFILT_MACHPORT with overlarge message");
	}

	mach_msg_options_t extra_options = 0;
	if (du._du->du_fflags & MACH_MSG_STRICT_REPLY) {
		extra_options |= MACH_MSG_STRICT_REPLY;
	}
	const mach_msg_option_t options = ((DISPATCH_MACH_RCV_OPTIONS |
			MACH_RCV_TIMEOUT | extra_options) & ~MACH_RCV_LARGE);
	siz += DISPATCH_MACH_TRAILER_SIZE;
	hdr = malloc(siz); // mach_msg will return TOO_LARGE if hdr/siz is NULL/0
	kr = mach_msg(hdr, options, 0, dispatch_assume(hdr) ? siz : 0,
			(mach_port_name_t)ke->data, MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
	if (likely(!kr)) {
		flags |= DISPATCH_EV_MSG_NEEDS_FREE;
		return _dispatch_kevent_mach_msg_recv(du, flags, hdr, msg_pp, ovr_pp);
	}

	if (kr == MACH_RCV_TOO_LARGE) {
		_dispatch_log("BUG in libdispatch client: "
				"_dispatch_kevent_mach_msg_drain: dropped message too "
				"large to fit in memory: id = 0x%x, size = %u",
				hdr->msgh_id, _dispatch_kevent_mach_msg_size(ke));
		kr = MACH_MSG_SUCCESS;
	}
	free(hdr);

out:
	if (unlikely(kr)) {
		_dispatch_bug_mach_client("_dispatch_kevent_mach_msg_drain: "
				"message reception failed", kr);
	}
}

const dispatch_source_type_s _dispatch_source_type_mach_recv = {
	.dst_kind       = "mach_recv",
	.dst_filter     = EVFILT_MACHPORT,
	.dst_flags      = EV_UDATA_SPECIFIC|EV_DISPATCH|EV_VANISHED,
	.dst_fflags     = 0,
	.dst_mask       = 0
#ifdef MACH_RCV_SYNC_PEEK
			| MACH_RCV_SYNC_PEEK
#endif
			,
	.dst_action     = DISPATCH_UNOTE_ACTION_SOURCE_OR_FFLAGS,
	.dst_size       = sizeof(struct dispatch_source_refs_s),
	.dst_strict     = false,

	.dst_create     = _dispatch_unote_create_with_handle,
	.dst_merge_evt  = _dispatch_source_merge_evt,
	.dst_merge_msg  = NULL, // never receives messages directly

	.dst_per_trigger_qos = true,
	.dst_allow_empty_mask = true,
};

static void
_dispatch_mach_notification_event(dispatch_unote_t du, uint32_t flags DISPATCH_UNUSED,
		uintptr_t data DISPATCH_UNUSED, pthread_priority_t pp DISPATCH_UNUSED)
{
	DISPATCH_CLIENT_CRASH(du._du->du_ident, "Unexpected non message event");
}

const dispatch_source_type_s _dispatch_mach_type_notification = {
	.dst_kind       = "mach_notification",
	.dst_filter     = EVFILT_MACHPORT,
	.dst_flags      = EV_UDATA_SPECIFIC|EV_DISPATCH|EV_VANISHED,
	.dst_fflags     = DISPATCH_MACH_RCV_OPTIONS & ~MACH_RCV_VOUCHER,
	.dst_action     = DISPATCH_UNOTE_ACTION_PASS_FFLAGS,
	.dst_size       = sizeof(struct dispatch_unote_class_s),
	.dst_strict     = false,

	.dst_create     = _dispatch_unote_create_with_handle,
	.dst_merge_evt  = _dispatch_mach_notification_event,
	.dst_merge_msg  = _dispatch_mach_notification_merge_msg,

	.dst_per_trigger_qos = true,
};

static void
_dispatch_mach_recv_direct_merge_evt(dispatch_unote_t du, uint32_t flags,
		uintptr_t data, pthread_priority_t pp)
{
	if (flags & EV_VANISHED) {
		DISPATCH_CLIENT_CRASH(0,
				"Unexpected EV_VANISHED (do not destroy random mach ports)");
	}
	return _dispatch_source_merge_evt(du, flags, data, pp);
}

const dispatch_source_type_s _dispatch_mach_type_recv = {
	.dst_kind       = "mach_recv (channel)",
	.dst_filter     = EVFILT_MACHPORT,
	.dst_flags      = EV_UDATA_SPECIFIC|EV_DISPATCH|EV_VANISHED,
	.dst_fflags     = DISPATCH_MACH_RCV_OPTIONS,
	.dst_action     = DISPATCH_UNOTE_ACTION_PASS_FFLAGS,
	.dst_size       = sizeof(struct dispatch_mach_recv_refs_s),
	.dst_strict     = false,

	// without handle because the mach code will set the ident after connect
	.dst_create     = _dispatch_unote_create_without_handle,
	.dst_merge_evt  = _dispatch_mach_recv_direct_merge_evt,
	.dst_merge_msg  = _dispatch_mach_merge_msg,

	.dst_per_trigger_qos = true,
};

DISPATCH_NORETURN
static void
_dispatch_mach_reply_merge_evt(dispatch_unote_t du DISPATCH_UNUSED,
		uint32_t flags, uintptr_t data DISPATCH_UNUSED,
		pthread_priority_t pp DISPATCH_UNUSED)
{
	if (flags & EV_VANISHED) {
		DISPATCH_CLIENT_CRASH(0,
				"Unexpected EV_VANISHED (do not destroy random mach ports)");
	}
	DISPATCH_INTERNAL_CRASH(flags, "Unexpected event");
}

const dispatch_source_type_s _dispatch_mach_type_reply = {
	.dst_kind       = "mach reply",
	.dst_filter     = EVFILT_MACHPORT,
	.dst_flags      = EV_UDATA_SPECIFIC|EV_DISPATCH|EV_ONESHOT|EV_VANISHED,
	.dst_fflags     = DISPATCH_MACH_RCV_OPTIONS & ~MACH_RCV_VOUCHER,
	.dst_action     = DISPATCH_UNOTE_ACTION_PASS_FFLAGS,
	.dst_size       = sizeof(struct dispatch_mach_reply_refs_s),
	.dst_strict     = false,

	.dst_create     = _dispatch_unote_create_with_handle,
	.dst_merge_evt  = _dispatch_mach_reply_merge_evt,
	.dst_merge_msg  = _dispatch_mach_reply_merge_msg,
};

#pragma mark Mach channel SIGTERM notification (for XPC channels only)

const dispatch_source_type_s _dispatch_xpc_type_sigterm = {
	.dst_kind       = "sigterm (xpc)",
	.dst_filter     = EVFILT_SIGNAL,
	.dst_flags      = DISPATCH_EV_DIRECT|EV_CLEAR|EV_ONESHOT,
	.dst_fflags     = 0,
	.dst_action     = DISPATCH_UNOTE_ACTION_PASS_DATA,
	.dst_size       = sizeof(struct dispatch_xpc_term_refs_s),
	.dst_strict     = false,

	.dst_create     = _dispatch_unote_create_with_handle,
	.dst_merge_evt  = _dispatch_xpc_sigterm_merge_evt,
};

#endif // HAVE_MACH

#endif // DISPATCH_EVENT_BACKEND_KEVENT
