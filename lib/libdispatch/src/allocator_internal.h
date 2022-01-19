/*
 * Copyright (c) 2012-2013 Apple Inc. All rights reserved.
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

#ifndef __DISPATCH_ALLOCATOR_INTERNAL__
#define __DISPATCH_ALLOCATOR_INTERNAL__

#ifndef DISPATCH_ALLOCATOR
#if TARGET_OS_MAC && (defined(__LP64__) || TARGET_OS_EMBEDDED)
#define DISPATCH_ALLOCATOR 1
#endif
#endif

#if TARGET_IPHONE_SIMULATOR && IPHONE_SIMULATOR_HOST_MIN_VERSION_REQUIRED < 1090
#undef DISPATCH_USE_NANOZONE
#define DISPATCH_USE_NANOZONE 0
#endif
#ifndef DISPATCH_USE_NANOZONE
#if TARGET_OS_MAC && defined(__LP64__) && \
		(__MAC_OS_X_VERSION_MIN_REQUIRED >= 1090 || \
		__IPHONE_OS_VERSION_MIN_REQUIRED >= 70000)
#define DISPATCH_USE_NANOZONE 1
#endif
#endif

#ifndef DISPATCH_USE_MALLOCZONE
#if (TARGET_OS_MAC && !DISPATCH_USE_NANOZONE) || \
		(!TARGET_OS_MAC && HAVE_MALLOC_CREATE_ZONE)
#define DISPATCH_USE_MALLOCZONE 1
#endif
#endif

#ifndef DISPATCH_CONTINUATION_MALLOC
#if DISPATCH_USE_NANOZONE || !DISPATCH_ALLOCATOR
#define DISPATCH_CONTINUATION_MALLOC 1
#endif
#endif

#if !DISPATCH_ALLOCATOR && !DISPATCH_CONTINUATION_MALLOC
#error Invalid allocator configuration
#endif

#if DISPATCH_ALLOCATOR && DISPATCH_CONTINUATION_MALLOC
#define DISPATCH_ALLOC_NOINLINE DISPATCH_NOINLINE
#else
#define DISPATCH_ALLOC_NOINLINE
#endif

#pragma mark -
#pragma mark DISPATCH_ALLOCATOR

#if DISPATCH_ALLOCATOR

// Configuration here!
#define NUM_CPU dispatch_hw_config(logical_cpus)
#define MAGAZINES_PER_HEAP (NUM_CPU)

// Do you care about compaction or performance?
#if TARGET_OS_EMBEDDED
#define PACK_FIRST_PAGE_WITH_CONTINUATIONS 1
#else
#define PACK_FIRST_PAGE_WITH_CONTINUATIONS 0
#endif

#ifndef PAGE_MAX_SIZE
#define PAGE_MAX_SIZE PAGE_SIZE
#endif
#ifndef PAGE_MAX_MASK
#define PAGE_MAX_MASK PAGE_MASK
#endif
#define DISPATCH_ALLOCATOR_PAGE_SIZE PAGE_MAX_SIZE
#define DISPATCH_ALLOCATOR_PAGE_MASK PAGE_MAX_MASK


#if TARGET_OS_EMBEDDED
#define PAGES_PER_MAGAZINE 64
#else
#define PAGES_PER_MAGAZINE 512
#endif

// Use the largest type your platform is comfortable doing atomic ops with.
// TODO: rdar://11477843
typedef unsigned long bitmap_t;
#if defined(__LP64__)
#define BYTES_PER_BITMAP 8
#else
#define BYTES_PER_BITMAP 4
#endif

#define BITMAP_C(v) ((bitmap_t)(v))
#define BITMAP_ALL_ONES (~BITMAP_C(0))

// Stop configuring.

#define CONTINUATIONS_PER_BITMAP (BYTES_PER_BITMAP * 8)
#define BITMAPS_PER_SUPERMAP (BYTES_PER_SUPERMAP * 8)

#define BYTES_PER_MAGAZINE (PAGES_PER_MAGAZINE * DISPATCH_ALLOCATOR_PAGE_SIZE)
#define CONSUMED_BYTES_PER_BITMAP (BYTES_PER_BITMAP + \
		(DISPATCH_CONTINUATION_SIZE * CONTINUATIONS_PER_BITMAP))

#define BYTES_PER_SUPERMAP BYTES_PER_BITMAP
#define CONSUMED_BYTES_PER_SUPERMAP (BYTES_PER_SUPERMAP + \
		(BITMAPS_PER_SUPERMAP * CONSUMED_BYTES_PER_BITMAP))

#define BYTES_PER_HEAP (BYTES_PER_MAGAZINE * MAGAZINES_PER_HEAP)

#define BYTES_PER_PAGE DISPATCH_ALLOCATOR_PAGE_SIZE
#define CONTINUATIONS_PER_PAGE (BYTES_PER_PAGE / DISPATCH_CONTINUATION_SIZE)
#define BITMAPS_PER_PAGE (CONTINUATIONS_PER_PAGE / CONTINUATIONS_PER_BITMAP)

// Assumption: metadata will be only in the first page.
#define SUPERMAPS_PER_MAGAZINE ((BYTES_PER_MAGAZINE - BYTES_PER_PAGE) / \
		CONSUMED_BYTES_PER_SUPERMAP)
#define BITMAPS_PER_MAGAZINE (SUPERMAPS_PER_MAGAZINE * BITMAPS_PER_SUPERMAP)
#define CONTINUATIONS_PER_MAGAZINE \
		(BITMAPS_PER_MAGAZINE * CONTINUATIONS_PER_BITMAP)

#define HEAP_MASK (~(uintptr_t)(BYTES_PER_HEAP - 1))
#define MAGAZINE_MASK (~(uintptr_t)(BYTES_PER_MAGAZINE - 1))

#define PADDING_TO_CONTINUATION_SIZE(x) (ROUND_UP_TO_CONTINUATION_SIZE(x) - (x))

#if defined(__LP64__)
#define SIZEOF_HEADER 16
#else
#define SIZEOF_HEADER 8
#endif

#define SIZEOF_SUPERMAPS (BYTES_PER_SUPERMAP * SUPERMAPS_PER_MAGAZINE)
#define SIZEOF_MAPS (BYTES_PER_BITMAP * BITMAPS_PER_SUPERMAP * \
		SUPERMAPS_PER_MAGAZINE)

// header is expected to end on supermap's required alignment
#define HEADER_TO_SUPERMAPS_PADDING 0
#define SUPERMAPS_TO_MAPS_PADDING (PADDING_TO_CONTINUATION_SIZE( \
		SIZEOF_SUPERMAPS + HEADER_TO_SUPERMAPS_PADDING + SIZEOF_HEADER))
#define MAPS_TO_FPMAPS_PADDING (PADDING_TO_CONTINUATION_SIZE(SIZEOF_MAPS))

#define BYTES_LEFT_IN_FIRST_PAGE (BYTES_PER_PAGE - \
		(SIZEOF_HEADER + HEADER_TO_SUPERMAPS_PADDING + SIZEOF_SUPERMAPS + \
		SUPERMAPS_TO_MAPS_PADDING + SIZEOF_MAPS + MAPS_TO_FPMAPS_PADDING))

#if PACK_FIRST_PAGE_WITH_CONTINUATIONS

#define FULL_BITMAPS_IN_FIRST_PAGE \
		(BYTES_LEFT_IN_FIRST_PAGE / CONSUMED_BYTES_PER_BITMAP)
#define REMAINDER_IN_FIRST_PAGE (BYTES_LEFT_IN_FIRST_PAGE - \
		(FULL_BITMAPS_IN_FIRST_PAGE * CONSUMED_BYTES_PER_BITMAP) - \
		(FULL_BITMAPS_IN_FIRST_PAGE ? 0 : \
		ROUND_UP_TO_CONTINUATION_SIZE(BYTES_PER_BITMAP)))

#define REMAINDERED_CONTINUATIONS_IN_FIRST_PAGE \
		(REMAINDER_IN_FIRST_PAGE / DISPATCH_CONTINUATION_SIZE)
#define CONTINUATIONS_IN_FIRST_PAGE (FULL_BITMAPS_IN_FIRST_PAGE * \
		CONTINUATIONS_PER_BITMAP) + REMAINDERED_CONTINUATIONS_IN_FIRST_PAGE
#define BITMAPS_IN_FIRST_PAGE (FULL_BITMAPS_IN_FIRST_PAGE + \
		(REMAINDERED_CONTINUATIONS_IN_FIRST_PAGE == 0 ? 0 : 1))

#define FPMAPS_TO_FPCONTS_PADDING (PADDING_TO_CONTINUATION_SIZE(\
		BYTES_PER_BITMAP * BITMAPS_IN_FIRST_PAGE))

#else // PACK_FIRST_PAGE_WITH_CONTINUATIONS

#define MAPS_TO_CONTS_PADDING BYTES_LEFT_IN_FIRST_PAGE

#endif // PACK_FIRST_PAGE_WITH_CONTINUATIONS

#define AFTER_CONTS_PADDING (BYTES_PER_MAGAZINE - (BYTES_PER_PAGE + \
		(DISPATCH_CONTINUATION_SIZE * CONTINUATIONS_PER_MAGAZINE)))

// This is the object our allocator allocates: a chunk of memory rounded up
// from sizeof(struct dispatch_continuation_s) to the cacheline size, so
// unrelated continuations don't share cachelines. It'd be nice if
// dispatch_continuation_s included this rounding/padding, but it doesn't.
typedef char padded_continuation[DISPATCH_CONTINUATION_SIZE] __aligned(8);

// A dispatch_heap_t is the base address of an array of dispatch_magazine_s,
// one magazine per CPU.
typedef struct dispatch_magazine_s * dispatch_heap_t;

struct dispatch_magazine_header_s {
	// Link to the next heap in the chain. Only used in magazine 0's header
	dispatch_heap_t dh_next;

	// Points to the first bitmap in the page where this CPU succesfully
	// allocated a continuation last time. Only used in the first heap.
	bitmap_t *last_found_page;
};

// A magazine is a complex data structure. It must be exactly
// PAGES_PER_MAGAZINE * PAGE_SIZE bytes long, and that value must be a
// power of 2. (See magazine_for_continuation()).
struct dispatch_magazine_s {
	// See above.
	struct dispatch_magazine_header_s header;

	// Align supermaps as needed.
#if HEADER_TO_SUPERMAPS_PADDING > 0
	char _pad0[HEADER_TO_SUPERMAPS_PADDING];
#endif

	// Second-level bitmap; each set bit means a bitmap_t in maps[][]
	// is completely full (and can be skipped while searching).
	bitmap_t supermaps[SUPERMAPS_PER_MAGAZINE];

	// Align maps to a cacheline.
#if SUPERMAPS_TO_MAPS_PADDING > 0
	char _pad1[SUPERMAPS_TO_MAPS_PADDING];
#endif

	// Each bit in maps[][] is the free/used state of a member of conts[][][].
	bitmap_t maps[SUPERMAPS_PER_MAGAZINE][BITMAPS_PER_SUPERMAP];

	// Align fp_maps to a cacheline.
#if MAPS_TO_FPMAPS_PADDING > 0
	char _pad2[MAPS_TO_FPMAPS_PADDING];
#endif

#if PACK_FIRST_PAGE_WITH_CONTINUATIONS
	// Bitmaps for the continuations that live in the first page, which
	// are treated specially (they have faster search code).
	bitmap_t fp_maps[BITMAPS_IN_FIRST_PAGE];

	// Align fp_conts to cacheline.
#if FPMAPS_TO_FPCONTS_PADDING > 0
	char _pad3[FPMAPS_TO_FPCONTS_PADDING];
#endif

	// Continuations that live in the first page.
	padded_continuation fp_conts[CONTINUATIONS_IN_FIRST_PAGE];

#else // PACK_FIRST_PAGE_WITH_CONTINUATIONS

#if MAPS_TO_CONTS_PADDING > 0
	char _pad4[MAPS_TO_CONTS_PADDING];
#endif
#endif // PACK_FIRST_PAGE_WITH_CONTINUATIONS

	// This is the big array of continuations.
	// This must start on a page boundary.
	padded_continuation conts[SUPERMAPS_PER_MAGAZINE][BITMAPS_PER_SUPERMAP]
	[CONTINUATIONS_PER_BITMAP];

	// Fill the unused space to exactly BYTES_PER_MAGAZINE
#if AFTER_CONTS_PADDING > 0
	char _pad5[AFTER_CONTS_PADDING];
#endif
};

#if DISPATCH_DEBUG
#define DISPATCH_ALLOCATOR_SCRIBBLE ((uintptr_t)0xAFAFAFAFAFAFAFAF)
#endif

#endif // DISPATCH_ALLOCATOR

#endif // __DISPATCH_ALLOCATOR_INTERNAL__
