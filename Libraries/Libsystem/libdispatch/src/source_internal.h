/*
 * Copyright (c) 2008-2013 Apple Inc. All rights reserved.
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

#ifndef __DISPATCH_SOURCE_INTERNAL__
#define __DISPATCH_SOURCE_INTERNAL__

#ifndef __DISPATCH_INDIRECT__
#error "Please #include <dispatch/dispatch.h> instead of this file directly."
#include <dispatch/base.h> // for HeaderDoc
#endif

_OS_OBJECT_CLASS_IMPLEMENTS_PROTOCOL(dispatch_source, dispatch_object)
DISPATCH_CLASS_DECL_BARE(source, QUEUE);

DISPATCH_CLASS_DECL(channel, QUEUE);

#define DISPATCH_SOURCE_CLASS_HEADER(x) \
	DISPATCH_LANE_CLASS_HEADER(x); \
	uint16_t \
		/* set under the drain lock */ \
		ds_is_installed:1, \
		ds_latched:1, \
		dm_connect_handler_called:1, \
		dm_cancel_handler_called:1, \
		dm_is_xpc:1, \
		dm_arm_no_senders:1, \
		dm_strict_reply:1, \
		__ds_flags_pad : 9; \
	uint16_t __dq_flags_separation[0]; \
	uint16_t \
		/* set under the send queue lock */ \
		dm_needs_mgr:1, \
		dm_disconnected:1, \
		__dm_flags_pad : 14

struct dispatch_source_s {
	DISPATCH_SOURCE_CLASS_HEADER(source);
} DISPATCH_ATOMIC64_ALIGN;
dispatch_assert_valid_lane_type(dispatch_source_s);
dispatch_static_assert(sizeof(struct dispatch_source_s) <= 128);

struct dispatch_channel_s {
	DISPATCH_SOURCE_CLASS_HEADER(channel);
} DISPATCH_ATOMIC64_ALIGN;
dispatch_assert_valid_lane_type(dispatch_channel_s);
dispatch_static_assert(sizeof(struct dispatch_channel_s) <= 128);

void _dispatch_source_xref_dispose(dispatch_source_t ds);
void _dispatch_source_dispose(dispatch_source_t ds, bool *allow_free);
void _dispatch_source_activate(dispatch_source_t ds);
void _dispatch_source_invoke(dispatch_source_t ds,
		dispatch_invoke_context_t dic, dispatch_invoke_flags_t flags);
void _dispatch_source_wakeup(dispatch_source_t ds, dispatch_qos_t qos,
		dispatch_wakeup_flags_t flags);
void _dispatch_source_merge_evt(dispatch_unote_t du, uint32_t flags,
		uintptr_t data, pthread_priority_t pp);
DISPATCH_COLD
size_t _dispatch_source_debug(dispatch_source_t ds, char *buf, size_t bufsiz);

void _dispatch_channel_xref_dispose(dispatch_channel_t dch);
void _dispatch_channel_dispose(dispatch_channel_t dch, bool *allow_free);
void _dispatch_channel_invoke(dispatch_channel_t dch,
		dispatch_invoke_context_t dic, dispatch_invoke_flags_t flags);
void _dispatch_channel_wakeup(dispatch_channel_t dch, dispatch_qos_t qos,
		dispatch_wakeup_flags_t flags);
DISPATCH_COLD
size_t _dispatch_channel_debug(dispatch_channel_t dch, char *buf, size_t bufsiz);

#endif /* __DISPATCH_SOURCE_INTERNAL__ */
