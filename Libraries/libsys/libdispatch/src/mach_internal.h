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

/*
 * IMPORTANT: This header file describes INTERNAL interfaces to libdispatch
 * which are subject to change in future releases of Mac OS X. Any applications
 * relying on these interfaces WILL break.
 */

#ifndef __DISPATCH_MACH_INTERNAL__
#define __DISPATCH_MACH_INTERNAL__
#if HAVE_MACH

#ifndef __DISPATCH_INDIRECT__
#error "Please #include <dispatch/dispatch.h> instead of this file directly."
#include <dispatch/base.h> // for HeaderDoc
#endif

// NOTE: dispatch_source_mach_send_flags_t and dispatch_source_mach_recv_flags_t
//       bit values must not overlap as they share the same kevent fflags !

/*!
 * @enum dispatch_source_mach_send_flags_t
 *
 * @constant DISPATCH_MACH_SEND_DELETED
 * Port-deleted notification. Disabled for source registration.
 */
enum {
	DISPATCH_MACH_SEND_DELETED = 0x4,
};
/*!
 * @enum dispatch_source_mach_recv_flags_t
 *
 * @constant DISPATCH_MACH_RECV_MESSAGE
 * Receive right has pending messages
 */
enum {
	DISPATCH_MACH_RECV_MESSAGE = 0x2,
};

DISPATCH_CLASS_DECL(mach, QUEUE);
DISPATCH_CLASS_DECL(mach_msg, OBJECT);

struct dispatch_mach_s {
	DISPATCH_SOURCE_CLASS_HEADER(mach);
	dispatch_mach_send_refs_t dm_send_refs;
	dispatch_xpc_term_refs_t dm_xpc_term_refs;
} DISPATCH_ATOMIC64_ALIGN;
dispatch_assert_valid_lane_type(dispatch_mach_s);

struct dispatch_mach_msg_s {
	DISPATCH_OBJECT_HEADER(mach_msg);
	union {
		mach_msg_option_t dmsg_options;
		mach_error_t dmsg_error;
	};
	mach_port_t dmsg_reply;
	pthread_priority_t dmsg_priority;
	voucher_t dmsg_voucher;
	dispatch_mach_msg_destructor_t dmsg_destructor;
	size_t dmsg_size;
	union {
		mach_msg_header_t *dmsg_msg;
		char dmsg_buf[0];
	};
};

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_mach_xref_dispose(struct dispatch_mach_s *dm)
{
	if (dm->dm_is_xpc) {
		dm->dm_recv_refs->dmrr_handler_ctxt = (void *)0xbadfeed;
	}
}

extern dispatch_mach_xpc_hooks_t _dispatch_mach_xpc_hooks;
extern const struct dispatch_mach_xpc_hooks_s _dispatch_mach_xpc_hooks_default;

void _dispatch_mach_ipc_handoff_invoke(dispatch_continuation_t dc,
		dispatch_invoke_context_t dic, dispatch_invoke_flags_t flags);
void _dispatch_mach_msg_async_reply_invoke(dispatch_continuation_t dc,
		dispatch_invoke_context_t dic, dispatch_invoke_flags_t flags);
void _dispatch_mach_dispose(dispatch_mach_t dm, bool *allow_free);
void _dispatch_mach_activate(dispatch_mach_t dm);
void _dispatch_mach_invoke(dispatch_mach_t dm, dispatch_invoke_context_t dic,
		dispatch_invoke_flags_t flags);
void _dispatch_mach_wakeup(dispatch_mach_t dm, dispatch_qos_t qos,
		dispatch_wakeup_flags_t flags);
DISPATCH_COLD
size_t _dispatch_mach_debug(dispatch_mach_t dm, char* buf, size_t bufsiz);
void _dispatch_mach_notification_merge_evt(dispatch_unote_t du,
		uint32_t flags, uintptr_t data, pthread_priority_t pp);
void _dispatch_mach_merge_msg(dispatch_unote_t du, uint32_t flags,
		mach_msg_header_t *msg, mach_msg_size_t msgsz,
		pthread_priority_t msg_pp, pthread_priority_t ovr_pp);
void _dispatch_mach_reply_merge_msg(dispatch_unote_t du, uint32_t flags,
		mach_msg_header_t *msg, mach_msg_size_t msgsz,
		pthread_priority_t msg_pp, pthread_priority_t ovr_pp);
void _dispatch_xpc_sigterm_merge_evt(dispatch_unote_t du, uint32_t flags,
		uintptr_t data, pthread_priority_t pp);

void _dispatch_mach_msg_dispose(dispatch_mach_msg_t dmsg, bool *allow_free);
void _dispatch_mach_msg_invoke(dispatch_mach_msg_t dmsg,
		dispatch_invoke_context_t dic, dispatch_invoke_flags_t flags);
DISPATCH_COLD
size_t _dispatch_mach_msg_debug(dispatch_mach_msg_t dmsg, char* buf,
		size_t bufsiz);

void _dispatch_mach_send_barrier_drain_invoke(dispatch_continuation_t dc,
		dispatch_invoke_context_t dic, dispatch_invoke_flags_t flags);
void _dispatch_mach_barrier_invoke(dispatch_continuation_t dc,
		dispatch_invoke_context_t dic, dispatch_invoke_flags_t flags);

#endif // HAVE_MACH
#endif /* __DISPATCH_MACH_INTERNAL__ */
