/*
 * Copyright (c) 2018 Apple Inc. All rights reserved.
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
#if DISPATCH_EVENT_BACKEND_WINDOWS

#pragma mark dispatch_unote_t

bool
_dispatch_unote_register(dispatch_unote_t du DISPATCH_UNUSED,
		dispatch_wlh_t wlh DISPATCH_UNUSED,
		dispatch_priority_t pri DISPATCH_UNUSED)
{
	WIN_PORT_ERROR();
	return false;
}

void
_dispatch_unote_resume(dispatch_unote_t du DISPATCH_UNUSED)
{
	WIN_PORT_ERROR();
}

bool
_dispatch_unote_unregister(dispatch_unote_t du DISPATCH_UNUSED,
		uint32_t flags DISPATCH_UNUSED)
{
	WIN_PORT_ERROR();
	return false;
}

#pragma mark timers

void
_dispatch_event_loop_timer_arm(uint32_t tidx DISPATCH_UNUSED,
		dispatch_timer_delay_s range DISPATCH_UNUSED,
		dispatch_clock_now_cache_t nows DISPATCH_UNUSED)
{
	WIN_PORT_ERROR();
}

void
_dispatch_event_loop_timer_delete(uint32_t tidx DISPATCH_UNUSED)
{
	WIN_PORT_ERROR();
}

#pragma mark dispatch_loop

void
_dispatch_event_loop_poke(dispatch_wlh_t wlh DISPATCH_UNUSED,
		uint64_t dq_state DISPATCH_UNUSED, uint32_t flags DISPATCH_UNUSED)
{
	WIN_PORT_ERROR();
}

DISPATCH_NOINLINE
void
_dispatch_event_loop_drain(uint32_t flags DISPATCH_UNUSED)
{
	WIN_PORT_ERROR();
}

void
_dispatch_event_loop_wake_owner(dispatch_sync_context_t dsc,
		dispatch_wlh_t wlh, uint64_t old_state, uint64_t new_state)
{
	(void)dsc; (void)wlh; (void)old_state; (void)new_state;
}

void
_dispatch_event_loop_wait_for_ownership(dispatch_sync_context_t dsc)
{
	if (dsc->dsc_release_storage) {
		_dispatch_queue_release_storage(dsc->dc_data);
	}
}

void
_dispatch_event_loop_end_ownership(dispatch_wlh_t wlh, uint64_t old_state,
		uint64_t new_state, uint32_t flags)
{
	(void)wlh; (void)old_state; (void)new_state; (void)flags;
}

#if DISPATCH_WLH_DEBUG
void
_dispatch_event_loop_assert_not_owned(dispatch_wlh_t wlh)
{
	(void)wlh;
}
#endif

void
_dispatch_event_loop_leave_immediate(dispatch_wlh_t wlh, uint64_t dq_state)
{
	(void)wlh; (void)dq_state;
}

#endif // DISPATCH_EVENT_BACKEND_WINDOWS
