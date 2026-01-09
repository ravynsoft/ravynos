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

DISPATCH_NOINLINE
static void *
__DISPATCH_WAIT_FOR_ENQUEUER__(void **ptr)
{
	int spins = 0;
	void *value;
	while ((value = os_atomic_load(ptr, relaxed)) == NULL) {
		_dispatch_preemption_yield(++spins);
	}
	return value;
}

void *
_dispatch_wait_for_enqueuer(void **ptr)
{
#if !DISPATCH_HW_CONFIG_UP
#if defined(__arm__) || defined(__arm64__)
	int spins = DISPATCH_WAIT_SPINS_WFE;
	void *value;
	while (unlikely(spins-- > 0)) {
		if (likely(value = __builtin_arm_ldrex(ptr))) {
			__builtin_arm_clrex();
			return value;
		}
		__builtin_arm_wfe();
	}
#else
	int spins = DISPATCH_WAIT_SPINS;
	void *value;
	while (unlikely(spins-- > 0)) {
		if (likely(value = os_atomic_load(ptr, relaxed))) {
			return value;
		}
		dispatch_hardware_pause();
	}
#endif
#endif // DISPATCH_HW_CONFIG_UP
	return __DISPATCH_WAIT_FOR_ENQUEUER__(ptr);
}
