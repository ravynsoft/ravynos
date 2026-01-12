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

#ifndef __FIREHOSE_BUFFER_PRIVATE__
#define __FIREHOSE_BUFFER_PRIVATE__

#if OS_FIREHOSE_SPI
#ifdef KERNEL
#include <stdint.h>
#else
#include <os/base.h>
#include <os/availability.h>
#include <os/base_private.h>
#include <dispatch/dispatch.h>
#endif

#define OS_FIREHOSE_SPI_VERSION 20180226

/*!
 * @group Firehose SPI
 * SPI intended for logd only
 * Layout of structs is subject to change without notice
 */

#define FIREHOSE_BUFFER_LIBTRACE_HEADER_SIZE	2048ul
#define FIREHOSE_BUFFER_KERNEL_MIN_CHUNK_COUNT	16
#define FIREHOSE_BUFFER_KERNEL_MAX_CHUNK_COUNT	64

typedef struct firehose_buffer_range_s {
	uint16_t fbr_offset; // offset from the start of the buffer
	uint16_t fbr_length;
} *firehose_buffer_range_t;

#ifdef KERNEL

typedef struct firehose_chunk_s *firehose_chunk_t;

// implemented by the kernel
extern void __firehose_buffer_push_to_logd(firehose_buffer_t fb, bool for_io);
extern void __firehose_critical_region_enter(void);
extern void __firehose_critical_region_leave(void);
extern void __firehose_allocate(vm_offset_t *addr, vm_size_t size);
extern uint8_t __firehose_buffer_kernel_chunk_count;
extern uint8_t __firehose_num_kernel_io_pages;

#define FIREHOSE_BUFFER_KERNEL_DEFAULT_CHUNK_COUNT FIREHOSE_BUFFER_KERNEL_MIN_CHUNK_COUNT
#define FIREHOSE_BUFFER_KERNEL_DEFAULT_IO_PAGES    8

#define FIREHOSE_BUFFER_KERNEL_CHUNK_COUNT __firehose_buffer_kernel_chunk_count
#define FIREHOSE_BUFFER_CHUNK_PREALLOCATED_COUNT (__firehose_buffer_kernel_chunk_count - 1) // the first chunk is the header

// exported for the kernel
firehose_tracepoint_t
__firehose_buffer_tracepoint_reserve(uint64_t stamp, firehose_stream_t stream,
		uint16_t pubsize, uint16_t privsize, uint8_t **privptr);

void
__firehose_buffer_tracepoint_flush(firehose_tracepoint_t vat,
		firehose_tracepoint_id_u vatid);

firehose_buffer_t
__firehose_buffer_create(size_t *size);

void
__firehose_merge_updates(firehose_push_reply_t update);

int
__firehose_kernel_configuration_valid(uint8_t chunk_count, uint8_t io_pages);

#else

#define __firehose_critical_region_enter()
#define __firehose_critical_region_leave()

OS_EXPORT
const uint32_t _firehose_spi_version;

OS_ALWAYS_INLINE
static inline const uint8_t *
_firehose_tracepoint_reader_init(firehose_chunk_t fc, const uint8_t **endptr)
{
	const uint8_t *start = fc->fc_data;
	const uint8_t *end = fc->fc_start + fc->fc_pos.fcp_next_entry_offs;

	if (end > fc->fc_start + FIREHOSE_CHUNK_SIZE) {
		end = start;
	}
	*endptr = end;
	return start;
}

OS_ALWAYS_INLINE
static inline firehose_tracepoint_t
_firehose_tracepoint_reader_next(const uint8_t **ptr, const uint8_t *end)
{
	const uint16_t ft_size = offsetof(struct firehose_tracepoint_s, ft_data);
	struct ft_unaligned_s {
		struct firehose_tracepoint_s ft;
	} __attribute__((packed, aligned(1))) *uft;

	do {
		uft = (struct ft_unaligned_s *)*ptr;
		if (uft->ft.ft_data >= end) {
			// reached the end
			return NULL;
		}
		if (!uft->ft.ft_length) {
			// tracepoint write didn't even start
			return NULL;
		}
		if (uft->ft.ft_length > end - uft->ft.ft_data) {
			// invalid length
			return NULL;
		}
		*ptr += roundup(ft_size + uft->ft.ft_length, 8);
		// test whether write of the tracepoint was finished
	} while (os_unlikely(uft->ft.ft_id.ftid_value == 0));

	return (firehose_tracepoint_t)uft;
}

#define firehose_tracepoint_foreach(ft, fbc) \
		for (const uint8_t *end, *p = _firehose_tracepoint_reader_init(fbc, &end); \
				((ft) = _firehose_tracepoint_reader_next(&p, end)); )

OS_ALWAYS_INLINE
static inline bool
firehose_buffer_range_validate(firehose_chunk_t fc, firehose_tracepoint_t ft,
		firehose_buffer_range_t range)
{
	if (range->fbr_offset + range->fbr_length > FIREHOSE_CHUNK_SIZE) {
		return false;
	}
	if (fc->fc_start + range->fbr_offset < ft->ft_data + ft->ft_length) {
		return false;
	}
	return true;
}

#endif // !KERNEL

#endif // OS_FIREHOSE_SPI

#endif // __FIREHOSE_BUFFER_PRIVATE__
