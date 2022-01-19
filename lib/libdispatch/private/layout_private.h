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

#if !TARGET_OS_WIN32
__OSX_AVAILABLE_STARTING(__MAC_10_6,__IPHONE_4_0)
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
#endif

#if DISPATCH_LAYOUT_SPI

/*!
 * @group Data Structure Layout SPI
 * SPI intended for CoreSymbolication only
 */

__OSX_AVAILABLE_STARTING(__MAC_10_10,__IPHONE_8_0)
DISPATCH_EXPORT const struct dispatch_tsd_indexes_s {
	// always add new fields at the end
	const uint16_t dti_version;
	const uint16_t dti_queue_index;
	const uint16_t dti_voucher_index;
	const uint16_t dti_qos_class_index;
} dispatch_tsd_indexes;

__OSX_AVAILABLE_STARTING(__MAC_10_10,__IPHONE_8_0)
DISPATCH_EXPORT const struct voucher_offsets_s {
	// always add new fields at the end
	const uint16_t vo_version;
	const uint16_t vo_activity_ids_count;
	const uint16_t vo_activity_ids_count_size;
	const uint16_t vo_activity_ids_array;
	const uint16_t vo_activity_ids_array_entry_size;
} voucher_offsets;

#endif // DISPATCH_LAYOUT_SPI

__END_DECLS

#endif // __DISPATCH_LAYOUT_PRIVATE__
