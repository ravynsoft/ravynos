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

#include "internal.h"
#include "allocator_internal.h"


#if DISPATCH_ALLOCATOR

#ifndef VM_MEMORY_LIBDISPATCH
#define VM_MEMORY_LIBDISPATCH 74
#endif

// _dispatch_main_heap is is the first heap in the linked list, where searches
// always begin.
//
// _dispatch_main_heap, and dh_next, are read normally but only written (in
// try_create_heap) by cmpxchg. They start life at 0, and are only written
// once to non-zero. They are not marked volatile. There is a small risk that
// some thread may see a stale 0 value and enter try_create_heap. It will
// waste some time in an allocate syscall, but eventually it will try to
// cmpxchg, expecting to overwite 0 with an address. This will fail
// (because another thread already did this), the thread will deallocate the
// unused allocated memory, and continue with the new value.
//
// If something goes wrong here, the symptom would be a NULL dereference
// in alloc_continuation_from_heap or _magazine when derefing the magazine ptr.
static dispatch_heap_t _dispatch_main_heap;

DISPATCH_ALWAYS_INLINE
static void
set_last_found_page(bitmap_t *val)
{
	dispatch_assert(_dispatch_main_heap);
	unsigned int cpu = _dispatch_cpu_number();
	_dispatch_main_heap[cpu].header.last_found_page = val;
}

DISPATCH_ALWAYS_INLINE
static bitmap_t *
last_found_page(void)
{
	dispatch_assert(_dispatch_main_heap);
	unsigned int cpu = _dispatch_cpu_number();
	return _dispatch_main_heap[cpu].header.last_found_page;
}

#pragma mark -
#pragma mark dispatch_alloc_bitmaps

DISPATCH_ALWAYS_INLINE_NDEBUG DISPATCH_CONST
static bitmap_t *
supermap_address(struct dispatch_magazine_s *magazine, unsigned int supermap)
{
	return &magazine->supermaps[supermap];
}

DISPATCH_ALWAYS_INLINE_NDEBUG DISPATCH_CONST
static bitmap_t *
bitmap_address(struct dispatch_magazine_s *magazine, unsigned int supermap,
		unsigned int map)
{
	return &magazine->maps[supermap][map];
}

DISPATCH_ALWAYS_INLINE_NDEBUG DISPATCH_CONST
static dispatch_continuation_t
continuation_address(struct dispatch_magazine_s *magazine,
		unsigned int supermap, unsigned int map, unsigned int index)
{
#if DISPATCH_DEBUG
	dispatch_assert(supermap < SUPERMAPS_PER_MAGAZINE);
	dispatch_assert(map < BITMAPS_PER_SUPERMAP);
	dispatch_assert(index < CONTINUATIONS_PER_BITMAP);
#endif
	return (dispatch_continuation_t)&magazine->conts[supermap][map][index];
}

DISPATCH_ALWAYS_INLINE_NDEBUG DISPATCH_CONST
static struct dispatch_magazine_s *
magazine_for_continuation(dispatch_continuation_t c)
{
	return (struct dispatch_magazine_s *)((uintptr_t)c & MAGAZINE_MASK);
}

DISPATCH_ALWAYS_INLINE_NDEBUG
static void
get_cont_and_indices_for_bitmap_and_index(bitmap_t *bitmap,
		unsigned int index, dispatch_continuation_t *continuation_out,
		bitmap_t **supermap_out, unsigned int *bitmap_index_out)
{
	// m_for_c wants a continuation not a bitmap, but it works because it
	// just masks off the bottom bits of the address.
	struct dispatch_magazine_s *m = magazine_for_continuation((void *)bitmap);
	unsigned int mindex = (unsigned int)(bitmap - m->maps[0]);
	unsigned int bindex = mindex % BITMAPS_PER_SUPERMAP;
	unsigned int sindex = mindex / BITMAPS_PER_SUPERMAP;
	dispatch_assert(&m->maps[sindex][bindex] == bitmap);
	if (fastpath(continuation_out)) {
		*continuation_out = continuation_address(m, sindex, bindex, index);
	}
	if (fastpath(supermap_out)) *supermap_out = supermap_address(m, sindex);
	if (fastpath(bitmap_index_out)) *bitmap_index_out = bindex;
}

DISPATCH_ALWAYS_INLINE_NDEBUG DISPATCH_CONST
static bool
continuation_is_in_first_page(dispatch_continuation_t c)
{
#if PACK_FIRST_PAGE_WITH_CONTINUATIONS
	// (the base of c's magazine == the base of c's page)
	// => c is in first page of magazine
	return (((uintptr_t)c & MAGAZINE_MASK) ==
			((uintptr_t)c & ~(uintptr_t)DISPATCH_ALLOCATOR_PAGE_MASK));
#else
	(void)c;
	return false;
#endif
}

DISPATCH_ALWAYS_INLINE_NDEBUG
static void
get_maps_and_indices_for_continuation(dispatch_continuation_t c,
		bitmap_t **supermap_out, unsigned int *bitmap_index_out,
		bitmap_t **bitmap_out, unsigned int *index_out)
{
	unsigned int cindex, sindex, index, mindex;
	padded_continuation *p = (padded_continuation *)c;
	struct dispatch_magazine_s *m = magazine_for_continuation(c);
#if PACK_FIRST_PAGE_WITH_CONTINUATIONS
	if (fastpath(continuation_is_in_first_page(c))) {
		cindex = (unsigned int)(p - m->fp_conts);
		index = cindex % CONTINUATIONS_PER_BITMAP;
		mindex = cindex / CONTINUATIONS_PER_BITMAP;
		if (fastpath(supermap_out)) *supermap_out = NULL;
		if (fastpath(bitmap_index_out)) *bitmap_index_out = mindex;
		if (fastpath(bitmap_out)) *bitmap_out = &m->fp_maps[mindex];
		if (fastpath(index_out)) *index_out = index;
		return;
	}
#endif // PACK_FIRST_PAGE_WITH_CONTINUATIONS
	cindex = (unsigned int)(p - (padded_continuation *)m->conts);
	sindex = cindex / (BITMAPS_PER_SUPERMAP * CONTINUATIONS_PER_BITMAP);
	mindex = (cindex / CONTINUATIONS_PER_BITMAP) % BITMAPS_PER_SUPERMAP;
	index = cindex % CONTINUATIONS_PER_BITMAP;
	if (fastpath(supermap_out)) *supermap_out = &m->supermaps[sindex];
	if (fastpath(bitmap_index_out)) *bitmap_index_out = mindex;
	if (fastpath(bitmap_out)) *bitmap_out = &m->maps[sindex][mindex];
	if (fastpath(index_out)) *index_out = index;
}

// Base address of page, or NULL if this page shouldn't be madvise()d
DISPATCH_ALWAYS_INLINE_NDEBUG DISPATCH_CONST
static void *
madvisable_page_base_for_continuation(dispatch_continuation_t c)
{
	if (fastpath(continuation_is_in_first_page(c))) {
		return NULL;
	}
	void *page_base = (void *)((uintptr_t)c &
			~(uintptr_t)DISPATCH_ALLOCATOR_PAGE_MASK);
#if DISPATCH_DEBUG
	struct dispatch_magazine_s *m = magazine_for_continuation(c);
	if (slowpath(page_base < (void *)&m->conts)) {
		DISPATCH_CRASH("madvisable continuation too low");
	}
	if (slowpath(page_base > (void *)&m->conts[SUPERMAPS_PER_MAGAZINE-1]
			[BITMAPS_PER_SUPERMAP-1][CONTINUATIONS_PER_BITMAP-1])) {
		DISPATCH_CRASH("madvisable continuation too high");
	}
#endif
	return page_base;
}

// Bitmap that controls the first few continuations in the same page as
// the continuations controlled by the passed bitmap. Undefined results if the
// passed bitmap controls continuations in the first page.
DISPATCH_ALWAYS_INLINE_NDEBUG DISPATCH_CONST
static bitmap_t *
first_bitmap_in_same_page(bitmap_t *b)
{
#if DISPATCH_DEBUG
	struct dispatch_magazine_s *m;
	m = magazine_for_continuation((void*)b);
	dispatch_assert(b >= &m->maps[0][0]);
	dispatch_assert(b <  &m->maps[SUPERMAPS_PER_MAGAZINE]
			[BITMAPS_PER_SUPERMAP]);
#endif
	const uintptr_t PAGE_BITMAP_MASK = (BITMAPS_PER_PAGE *
			BYTES_PER_BITMAP) - 1;
	return (bitmap_t *)((uintptr_t)b & ~PAGE_BITMAP_MASK);
}

DISPATCH_ALWAYS_INLINE_NDEBUG DISPATCH_CONST
static bool
bitmap_is_full(bitmap_t bits)
{
	return (bits == BITMAP_ALL_ONES);
}

#define NO_BITS_WERE_UNSET (UINT_MAX)

// max_index is the 0-based position of the most significant bit that is
// allowed to be set.
DISPATCH_ALWAYS_INLINE_NDEBUG
static unsigned int
bitmap_set_first_unset_bit_upto_index(volatile bitmap_t *bitmap,
		unsigned int max_index)
{
	// No barriers needed in acquire path: the just-allocated
	// continuation is "uninitialized", so the caller shouldn't
	// load from it before storing, so we don't need to guard
	// against reordering those loads.
	dispatch_assert(sizeof(*bitmap) == sizeof(unsigned long));
	return dispatch_atomic_set_first_bit(bitmap,max_index);
}

DISPATCH_ALWAYS_INLINE
static unsigned int
bitmap_set_first_unset_bit(volatile bitmap_t *bitmap)
{
	return bitmap_set_first_unset_bit_upto_index(bitmap, UINT_MAX);
}

#define CLEAR_EXCLUSIVELY true
#define CLEAR_NONEXCLUSIVELY false

// Return true if this bit was the last in the bitmap, and it is now all zeroes
DISPATCH_ALWAYS_INLINE_NDEBUG
static bool
bitmap_clear_bit(volatile bitmap_t *bitmap, unsigned int index,
		bool exclusively)
{
#if DISPATCH_DEBUG
	dispatch_assert(index < CONTINUATIONS_PER_BITMAP);
#endif
	const bitmap_t mask = BITMAP_C(1) << index;
	bitmap_t b;

	if (exclusively == CLEAR_EXCLUSIVELY) {
		if (slowpath((*bitmap & mask) == 0)) {
			DISPATCH_CRASH("Corruption: failed to clear bit exclusively");
		}
	}

	// and-and-fetch
	b = dispatch_atomic_and(bitmap, ~mask, release);
	return b == 0;
}

DISPATCH_ALWAYS_INLINE_NDEBUG
static void
mark_bitmap_as_full_if_still_full(volatile bitmap_t *supermap,
		unsigned int bitmap_index, volatile bitmap_t *bitmap)
{
#if DISPATCH_DEBUG
	dispatch_assert(bitmap_index < BITMAPS_PER_SUPERMAP);
#endif
	const bitmap_t mask = BITMAP_C(1) << bitmap_index;
	bitmap_t s, s_new, s_masked;

	if (!bitmap_is_full(*bitmap)) {
		return;
	}
	s_new = *supermap;
	for (;;) {
		// No barriers because supermaps are only advisory, they
		// don't protect access to other memory.
		s = s_new;
		s_masked = s | mask;
		if (dispatch_atomic_cmpxchgvw(supermap, s, s_masked, &s_new, relaxed) ||
				!bitmap_is_full(*bitmap)) {
			return;
		}
	}
}

#pragma mark -
#pragma mark dispatch_alloc_continuation_alloc

#if PACK_FIRST_PAGE_WITH_CONTINUATIONS
DISPATCH_ALWAYS_INLINE_NDEBUG
static dispatch_continuation_t
alloc_continuation_from_first_page(struct dispatch_magazine_s *magazine)
{
	unsigned int i, index, continuation_index;

	// TODO: unroll if this is hot?
	for (i = 0; i < FULL_BITMAPS_IN_FIRST_PAGE; i++) {
		index = bitmap_set_first_unset_bit(&magazine->fp_maps[i]);
		if (fastpath(index != NO_BITS_WERE_UNSET)) goto found;
	}
	if (REMAINDERED_CONTINUATIONS_IN_FIRST_PAGE) {
		index = bitmap_set_first_unset_bit_upto_index(&magazine->fp_maps[i],
				REMAINDERED_CONTINUATIONS_IN_FIRST_PAGE - 1);
		if (fastpath(index != NO_BITS_WERE_UNSET)) goto found;
	}
	return NULL;

found:
	continuation_index = (i * CONTINUATIONS_PER_BITMAP) + index;
	return (dispatch_continuation_t)&magazine->fp_conts[continuation_index];
}
#endif // PACK_FIRST_PAGE_WITH_CONTINUATIONS

DISPATCH_ALWAYS_INLINE_NDEBUG
static dispatch_continuation_t
alloc_continuation_from_magazine(struct dispatch_magazine_s *magazine)
{
	unsigned int s, b, index;

	for (s = 0; s < SUPERMAPS_PER_MAGAZINE; s++) {
		volatile bitmap_t *supermap = supermap_address(magazine, s);
		if (bitmap_is_full(*supermap)) {
			continue;
		}
		for (b = 0; b < BITMAPS_PER_SUPERMAP; b++) {
			volatile bitmap_t *bitmap = bitmap_address(magazine, s, b);
			index = bitmap_set_first_unset_bit(bitmap);
			if (index != NO_BITS_WERE_UNSET) {
				set_last_found_page(
						first_bitmap_in_same_page((bitmap_t *)bitmap));
				mark_bitmap_as_full_if_still_full(supermap, b, bitmap);
				return continuation_address(magazine, s, b, index);
			}
		}
	}
	return NULL;
}

DISPATCH_NOINLINE
static void
_dispatch_alloc_try_create_heap(dispatch_heap_t *heap_ptr)
{
#if HAVE_MACH
	kern_return_t kr;
	mach_vm_size_t vm_size = MAGAZINES_PER_HEAP * BYTES_PER_MAGAZINE;
	mach_vm_offset_t vm_mask = ~MAGAZINE_MASK;
	mach_vm_address_t vm_addr = vm_page_size;
	while (slowpath(kr = mach_vm_map(mach_task_self(), &vm_addr, vm_size,
			vm_mask, VM_FLAGS_ANYWHERE | VM_MAKE_TAG(VM_MEMORY_LIBDISPATCH),
			MEMORY_OBJECT_NULL, 0, FALSE, VM_PROT_DEFAULT, VM_PROT_ALL,
			VM_INHERIT_DEFAULT))) {
		if (kr != KERN_NO_SPACE) {
			(void)dispatch_assume_zero(kr);
			DISPATCH_CLIENT_CRASH("Could not allocate heap");
		}
		_dispatch_temporary_resource_shortage();
		vm_addr = vm_page_size;
	}
	uintptr_t aligned_region = (uintptr_t)vm_addr;
#else // HAVE_MACH
	const size_t region_sz = (1 + MAGAZINES_PER_HEAP) * BYTES_PER_MAGAZINE;
	void *region_p;
	while (!dispatch_assume((region_p = mmap(NULL, region_sz,
			PROT_READ|PROT_WRITE, MAP_ANON | MAP_PRIVATE,
			VM_MAKE_TAG(VM_MEMORY_LIBDISPATCH), 0)) != MAP_FAILED)) {
		_dispatch_temporary_resource_shortage();
	}
	uintptr_t region = (uintptr_t)region_p;
	uintptr_t region_end = region + region_sz;
	uintptr_t aligned_region, aligned_region_end;
	uintptr_t bottom_slop_len, top_slop_len;
	// Realign if needed; find the slop at top/bottom to unmap
	if ((region & ~(MAGAZINE_MASK)) == 0) {
		bottom_slop_len = 0;
		aligned_region = region;
		aligned_region_end = region_end - BYTES_PER_MAGAZINE;
		top_slop_len = BYTES_PER_MAGAZINE;
	} else {
		aligned_region = (region & MAGAZINE_MASK) + BYTES_PER_MAGAZINE;
		aligned_region_end = aligned_region +
				(MAGAZINES_PER_HEAP * BYTES_PER_MAGAZINE);
		bottom_slop_len = aligned_region - region;
		top_slop_len = BYTES_PER_MAGAZINE - bottom_slop_len;
	}
#if DISPATCH_DEBUG
	// Double-check our math.
	dispatch_assert(aligned_region % DISPATCH_ALLOCATOR_PAGE_SIZE == 0);
	dispatch_assert(aligned_region % vm_kernel_page_size == 0);
	dispatch_assert(aligned_region_end % DISPATCH_ALLOCATOR_PAGE_SIZE == 0);
	dispatch_assert(aligned_region_end % vm_kernel_page_size == 0);
	dispatch_assert(aligned_region_end > aligned_region);
	dispatch_assert(top_slop_len % DISPATCH_ALLOCATOR_PAGE_SIZE == 0);
	dispatch_assert(bottom_slop_len % DISPATCH_ALLOCATOR_PAGE_SIZE == 0);
	dispatch_assert(aligned_region_end + top_slop_len == region_end);
	dispatch_assert(region + bottom_slop_len == aligned_region);
	dispatch_assert(region_sz == bottom_slop_len + top_slop_len +
			MAGAZINES_PER_HEAP * BYTES_PER_MAGAZINE);
	if (bottom_slop_len) {
		(void)dispatch_assume_zero(mprotect((void *)region, bottom_slop_len,
				PROT_NONE));
	}
	if (top_slop_len) {
		(void)dispatch_assume_zero(mprotect((void *)aligned_region_end,
				top_slop_len, PROT_NONE));
	}
#else
	if (bottom_slop_len) {
		(void)dispatch_assume_zero(munmap((void *)region, bottom_slop_len));
	}
	if (top_slop_len) {
		(void)dispatch_assume_zero(munmap((void *)aligned_region_end,
				top_slop_len));
	}
#endif // DISPATCH_DEBUG
#endif // HAVE_MACH

	if (!dispatch_atomic_cmpxchg(heap_ptr, NULL, (void *)aligned_region,
			relaxed)) {
		// If we lost the race to link in the new region, unmap the whole thing.
#if DISPATCH_DEBUG
		(void)dispatch_assume_zero(mprotect((void *)aligned_region,
				MAGAZINES_PER_HEAP * BYTES_PER_MAGAZINE, PROT_NONE));
#else
		(void)dispatch_assume_zero(munmap((void *)aligned_region,
				MAGAZINES_PER_HEAP * BYTES_PER_MAGAZINE));
#endif
	}
}

DISPATCH_NOINLINE
static dispatch_continuation_t
_dispatch_alloc_continuation_from_heap(dispatch_heap_t heap)
{
	dispatch_continuation_t cont;

	unsigned int cpu_number = _dispatch_cpu_number();
#ifdef DISPATCH_DEBUG
	dispatch_assert(cpu_number < NUM_CPU);
#endif

#if PACK_FIRST_PAGE_WITH_CONTINUATIONS
	// First try the continuations in the first page for this CPU
	cont = alloc_continuation_from_first_page(&(heap[cpu_number]));
	if (fastpath(cont)) {
		return cont;
	}
#endif
	// Next, try the rest of the magazine for this CPU
	cont = alloc_continuation_from_magazine(&(heap[cpu_number]));
	return cont;
}

DISPATCH_NOINLINE
static dispatch_continuation_t
_dispatch_alloc_continuation_from_heap_slow(void)
{
	dispatch_heap_t *heap = &_dispatch_main_heap;
	dispatch_continuation_t cont;

	for (;;) {
		if (!fastpath(*heap)) {
			_dispatch_alloc_try_create_heap(heap);
		}
		cont = _dispatch_alloc_continuation_from_heap(*heap);
		if (fastpath(cont)) {
			return cont;
		}
		// If we have tuned our parameters right, 99.999% of apps should
		// never reach this point! The ones that do have gone off the rails...
		//
		// Magazine is full? Onto the next heap!
		// We tried 'stealing' from other CPUs' magazines. The net effect
		// was worse performance from more wasted search time and more
		// cache contention.

		// rdar://11378331
		// Future optimization: start at the page we last used, start
		// in the *zone* we last used. But this would only improve deeply
		// pathological cases like dispatch_starfish
		heap = &(*heap)->header.dh_next;
	}
}

DISPATCH_ALLOC_NOINLINE
static dispatch_continuation_t
_dispatch_alloc_continuation_alloc(void)
{
	dispatch_continuation_t cont;

	if (fastpath(_dispatch_main_heap)) {
		// Start looking in the same page where we found a continuation
		// last time.
		bitmap_t *last = last_found_page();
		if (fastpath(last)) {
			unsigned int i;
			for (i = 0; i < BITMAPS_PER_PAGE; i++) {
				bitmap_t *cur = last + i;
				unsigned int index = bitmap_set_first_unset_bit(cur);
				if (fastpath(index != NO_BITS_WERE_UNSET)) {
					bitmap_t *supermap;
					unsigned int bindex;
					get_cont_and_indices_for_bitmap_and_index(cur,
							index, &cont, &supermap, &bindex);
					mark_bitmap_as_full_if_still_full(supermap, bindex,
							cur);
					return cont;
				}
			}
		}

		cont = _dispatch_alloc_continuation_from_heap(_dispatch_main_heap);
		if (fastpath(cont)) {
			return cont;
		}
	}
	return _dispatch_alloc_continuation_from_heap_slow();
}

#pragma mark -
#pragma mark dispatch_alloc_continuation_free

DISPATCH_NOINLINE
static void
_dispatch_alloc_maybe_madvise_page(dispatch_continuation_t c)
{
	void *page = madvisable_page_base_for_continuation(c);
	if (!page) {
		// page can't be madvised; maybe it contains non-continuations
		return;
	}
	// Are all the continuations in this page unallocated?
	volatile bitmap_t *page_bitmaps;
	get_maps_and_indices_for_continuation((dispatch_continuation_t)page, NULL,
			NULL, (bitmap_t **)&page_bitmaps, NULL);
	unsigned int i;
	for (i = 0; i < BITMAPS_PER_PAGE; i++) {
		if (page_bitmaps[i] != 0) {
			return;
		}
	}
	// They are all unallocated, so we could madvise the page. Try to
	// take ownership of them all.
	int last_locked = 0;
	do {
		if (!dispatch_atomic_cmpxchg(&page_bitmaps[last_locked], BITMAP_C(0),
				BITMAP_ALL_ONES, relaxed)) {
			// We didn't get one; since there is a cont allocated in
			// the page, we can't madvise. Give up and unlock all.
			goto unlock;
		}
	} while (++last_locked < (signed)BITMAPS_PER_PAGE);
#if DISPATCH_DEBUG
	//fprintf(stderr, "%s: madvised page %p for cont %p (next = %p), "
	//		"[%u+1]=%u bitmaps at %p\n", __func__, page, c, c->do_next,
	//		last_locked-1, BITMAPS_PER_PAGE, &page_bitmaps[0]);
	// Scribble to expose use-after-free bugs
	// madvise (syscall) flushes these stores
	memset(page, DISPATCH_ALLOCATOR_SCRIBBLE, DISPATCH_ALLOCATOR_PAGE_SIZE);
#endif
	(void)dispatch_assume_zero(madvise(page, DISPATCH_ALLOCATOR_PAGE_SIZE,
			MADV_FREE));

unlock:
	while (last_locked > 1) {
		page_bitmaps[--last_locked] = BITMAP_C(0);
	}
	if (last_locked) {
		dispatch_atomic_store(&page_bitmaps[0], BITMAP_C(0), relaxed);
	}
	return;
}

DISPATCH_ALLOC_NOINLINE
static void
_dispatch_alloc_continuation_free(dispatch_continuation_t c)
{
	bitmap_t *b, *s;
	unsigned int b_idx, idx;

	get_maps_and_indices_for_continuation(c, &s, &b_idx, &b, &idx);
	bool bitmap_now_empty = bitmap_clear_bit(b, idx, CLEAR_EXCLUSIVELY);
	if (slowpath(s)) {
		(void)bitmap_clear_bit(s, b_idx, CLEAR_NONEXCLUSIVELY);
	}
	// We only try to madvise(2) pages outside of the first page.
	// (Allocations in the first page do not have a supermap entry.)
	if (slowpath(bitmap_now_empty) && slowpath(s)) {
		return _dispatch_alloc_maybe_madvise_page(c);
	}
}

#pragma mark -
#pragma mark dispatch_alloc_init

#if DISPATCH_DEBUG
static void
_dispatch_alloc_init(void)
{
	// Double-check our math. These are all compile time checks and don't
	// generate code.

	dispatch_assert(sizeof(bitmap_t) == BYTES_PER_BITMAP);
	dispatch_assert(sizeof(bitmap_t) == BYTES_PER_SUPERMAP);
	dispatch_assert(sizeof(struct dispatch_magazine_header_s) ==
			SIZEOF_HEADER);

	dispatch_assert(sizeof(struct dispatch_continuation_s) <=
			DISPATCH_CONTINUATION_SIZE);

	// Magazines should be the right size, so they pack neatly into an array of
	// heaps.
	dispatch_assert(sizeof(struct dispatch_magazine_s) == BYTES_PER_MAGAZINE);

	// The header and maps sizes should match what we computed.
	dispatch_assert(SIZEOF_HEADER ==
			sizeof(((struct dispatch_magazine_s *)0x0)->header));
	dispatch_assert(SIZEOF_MAPS ==
			sizeof(((struct dispatch_magazine_s *)0x0)->maps));

	// The main array of continuations should start at the second page,
	// self-aligned.
	dispatch_assert(offsetof(struct dispatch_magazine_s, conts) %
			(CONTINUATIONS_PER_BITMAP * DISPATCH_CONTINUATION_SIZE) == 0);
	dispatch_assert(offsetof(struct dispatch_magazine_s, conts) ==
			DISPATCH_ALLOCATOR_PAGE_SIZE);

#if PACK_FIRST_PAGE_WITH_CONTINUATIONS
	// The continuations in the first page should actually fit within the first
	// page.
	dispatch_assert(offsetof(struct dispatch_magazine_s, fp_conts) <
			DISPATCH_ALLOCATOR_PAGE_SIZE);
	dispatch_assert(offsetof(struct dispatch_magazine_s, fp_conts) %
			DISPATCH_CONTINUATION_SIZE == 0);
	dispatch_assert(offsetof(struct dispatch_magazine_s, fp_conts) +
			sizeof(((struct dispatch_magazine_s *)0x0)->fp_conts) ==
					DISPATCH_ALLOCATOR_PAGE_SIZE);
#endif // PACK_FIRST_PAGE_WITH_CONTINUATIONS
}
#elif (DISPATCH_ALLOCATOR && DISPATCH_CONTINUATION_MALLOC) \
		|| (DISPATCH_CONTINUATION_MALLOC && DISPATCH_USE_MALLOCZONE)
static inline void _dispatch_alloc_init(void) {}
#endif

#endif // DISPATCH_ALLOCATOR

#pragma mark -
#pragma mark dispatch_malloc

#if DISPATCH_CONTINUATION_MALLOC

#if DISPATCH_USE_MALLOCZONE
static malloc_zone_t *_dispatch_ccache_zone;

#define calloc(n, s) malloc_zone_calloc(_dispatch_ccache_zone, (n), (s))
#define free(c) malloc_zone_free(_dispatch_ccache_zone, (c))

static void
_dispatch_malloc_init(void)
{
	_dispatch_ccache_zone = malloc_create_zone(0, 0);
	dispatch_assert(_dispatch_ccache_zone);
	malloc_set_zone_name(_dispatch_ccache_zone, "DispatchContinuations");
}
#else
static inline void _dispatch_malloc_init(void) {}
#endif // DISPATCH_USE_MALLOCZONE

static dispatch_continuation_t
_dispatch_malloc_continuation_alloc(void)
{
	dispatch_continuation_t dc;
	while (!(dc = fastpath(calloc(1,
			ROUND_UP_TO_CACHELINE_SIZE(sizeof(*dc)))))) {
		_dispatch_temporary_resource_shortage();
	}
	return dc;
}

static inline void
_dispatch_malloc_continuation_free(dispatch_continuation_t c)
{
	free(c);
}
#endif // DISPATCH_CONTINUATION_MALLOC

#pragma mark -
#pragma mark dispatch_continuation_alloc

#if DISPATCH_ALLOCATOR
#if DISPATCH_CONTINUATION_MALLOC
#if DISPATCH_USE_NANOZONE
extern boolean_t malloc_engaged_nano(void);
#else
#define malloc_engaged_nano() false
#endif // DISPATCH_USE_NANOZONE
static int _dispatch_use_dispatch_alloc;
#else
#define _dispatch_use_dispatch_alloc 1
#endif // DISPATCH_CONTINUATION_MALLOC
#endif // DISPATCH_ALLOCATOR

#if (DISPATCH_ALLOCATOR && (DISPATCH_CONTINUATION_MALLOC || DISPATCH_DEBUG)) \
		|| (DISPATCH_CONTINUATION_MALLOC && DISPATCH_USE_MALLOCZONE)
static void
_dispatch_continuation_alloc_init(void *ctxt DISPATCH_UNUSED)
{
#if DISPATCH_ALLOCATOR
#if DISPATCH_CONTINUATION_MALLOC
	bool use_dispatch_alloc = !malloc_engaged_nano();
	char *e = getenv("LIBDISPATCH_CONTINUATION_ALLOCATOR");
	if (e) {
		use_dispatch_alloc = atoi(e);
	}
	_dispatch_use_dispatch_alloc = use_dispatch_alloc;
#endif // DISPATCH_CONTINUATION_MALLOC
	if (_dispatch_use_dispatch_alloc)
		return _dispatch_alloc_init();
#endif // DISPATCH_ALLOCATOR
#if DISPATCH_CONTINUATION_MALLOC
	return _dispatch_malloc_init();
#endif // DISPATCH_ALLOCATOR
}

static void
_dispatch_continuation_alloc_once()
{
	static dispatch_once_t pred;
	dispatch_once_f(&pred, NULL, _dispatch_continuation_alloc_init);
}
#else
static inline void _dispatch_continuation_alloc_once(void) {}
#endif // DISPATCH_ALLOCATOR ... || DISPATCH_CONTINUATION_MALLOC ...

dispatch_continuation_t
_dispatch_continuation_alloc_from_heap(void)
{
	_dispatch_continuation_alloc_once();
#if DISPATCH_ALLOCATOR
	if (_dispatch_use_dispatch_alloc)
		return _dispatch_alloc_continuation_alloc();
#endif
#if DISPATCH_CONTINUATION_MALLOC
	return _dispatch_malloc_continuation_alloc();
#endif
}

void
_dispatch_continuation_free_to_heap(dispatch_continuation_t c)
{
#if DISPATCH_ALLOCATOR
	if (_dispatch_use_dispatch_alloc)
		return _dispatch_alloc_continuation_free(c);
#endif
#if DISPATCH_CONTINUATION_MALLOC
	return _dispatch_malloc_continuation_free(c);
#endif
}

