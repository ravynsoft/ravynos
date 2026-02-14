/*
 * Copyright (c) 2014 Apple Inc. All rights reserved.
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

#ifndef __DISPATCH_LAYOUT_PRIVATE__
#define __DISPATCH_LAYOUT_PRIVATE__

#ifndef __DISPATCH_INDIRECT__
#error "Please #include <dispatch/private.h> instead of this file directly."
#include <dispatch/base.h> // for HeaderDoc
#endif

__BEGIN_DECLS

API_AVAILABLE(macos(10.6), ios(4.0))
DISPATCH_EXPORT const struct dispatch_queue_offsets_s {
	// always add new fields at the end
	const uint16_t dqo_version;
	const uint16_t dqo_label;
	const uint16_t dqo_label_size;
	const uint16_t dqo_flags;
	const uint16_t dqo_flags_size;
	const uint16_t dqo_serialnum;
	const uint16_t dqo_serialnum_size;
	const uint16_t dqo_width;
	const uint16_t dqo_width_size;
	const uint16_t dqo_running;
	const uint16_t dqo_running_size;
	// fields added in dqo_version 5:
	const uint16_t dqo_suspend_cnt;
	const uint16_t dqo_suspend_cnt_size;
	const uint16_t dqo_target_queue;
	const uint16_t dqo_target_queue_size;
	const uint16_t dqo_priority;
	const uint16_t dqo_priority_size;
} dispatch_queue_offsets;

#if DISPATCH_LAYOUT_SPI
/*!
 * @group Data Structure Layout SPI
 * SPI intended for CoreSymbolication only
 */

API_AVAILABLE(macos(10.10), ios(8.0))
DISPATCH_EXPORT const struct dispatch_tsd_indexes_s {
	// always add new fields at the end
	const uint16_t dti_version;
	const uint16_t dti_queue_index;
	const uint16_t dti_voucher_index;
	const uint16_t dti_qos_class_index;
	/* version 3 */
	const uint16_t dti_continuation_cache_index;
} dispatch_tsd_indexes;

#if TARGET_OS_MAC

#include <malloc/malloc.h>

API_AVAILABLE(macos(10.14), ios(12.0), tvos(12.0), watchos(5.0))
DISPATCH_EXPORT const struct dispatch_allocator_layout_s {
	const uint16_t dal_version;
	/* version 1 */
	/* Pointer to the allocator metadata address, points to NULL if unused */
	void **const dal_allocator_zone;
	/* Magical "isa" for allocations that are on freelists */
	void *const *const dal_deferred_free_isa;
	/* Size of allocations made in the magazine */
	const uint16_t dal_allocation_size;
	/* fields used by the enumerator */
	const uint16_t dal_magazine_size;
	const uint16_t dal_first_allocation_offset;
	const uint16_t dal_allocation_isa_offset;
	/* Enumerates allocated continuations */
	kern_return_t (*dal_enumerator)(task_t remote_task,
			const struct dispatch_allocator_layout_s *remote_allocator_layout,
			vm_address_t zone_address, memory_reader_t reader,
			void (^recorder)(vm_address_t dc_address, void *dc_mem,
					size_t size, bool *stop));
} dispatch_allocator_layout;
#endif // TARGET_OS_MAC
#endif // DISPATCH_LAYOUT_SPI

__END_DECLS

#endif // __DISPATCH_LAYOUT_PRIVATE__
