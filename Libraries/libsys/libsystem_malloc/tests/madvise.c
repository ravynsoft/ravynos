#include <TargetConditionals.h>
#include <darwintest.h>
#include <sys/mman.h>
#include <stdio.h>
#include <malloc/malloc.h>
#include <mach/vm_page_size.h>
#include <stdlib.h>

#include "base.h"
#include "platform.h"
#include "nano_zone_common.h"
#include "nano_zone.h"

extern int
malloc_engaged_nano(void);

#define T_EXPECT_BYTES(p, len, byte, msg, ...) do { \
	char *_p = (char *)(p); \
	bool bytes = true; \
	for (int i=0; i<len; i++) { \
		T_QUIET; T_EXPECT_EQ_CHAR(*(_p+i), byte, "*(%p+0x%x)", _p, i); \
		if (*(_p+i) != byte) { bytes = false; break; } \
	} \
	T_EXPECT_TRUE(bytes, msg, ## __VA_ARGS__); \
} while(0)

// vm.madvise_free_debug should cause the kernel to forcibly discard
// pages that are madvised when the call is made. Making testing
// madvise behaviour predictable under test.
T_DECL(madvise_free_debug, "test vm.madvise_free_debug",
	   T_META_SYSCTL_INT("vm.madvise_free_debug=1"),
	   T_META_ASROOT(YES))
{
	// Map 32k of memory.
	size_t memsz = 32 * vm_page_size;
	void *mem = mmap(NULL, memsz, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, 0, 0);
	T_EXPECT_NE_PTR(mem, MAP_FAILED, "mapped pages should not be MAP_FAILED");

	// Fill it with scribble.
	memset(mem, 0xa, 32 * vm_page_size);

	// Madvise a specfic page.
	T_EXPECT_POSIX_ZERO(
			madvise(mem + (4 * vm_page_size), vm_page_size, MADV_FREE_REUSABLE),
			"madvise (mem + 4 pages)");

	// Check the entire page is empty.
	T_EXPECT_BYTES(mem + (4 * vm_page_size), vm_page_size, 0x0, "madvise'd memory is all zeros");
	T_EXPECT_POSIX_SUCCESS(munmap(mem, memsz), "munmap");
}

T_DECL(subpage_madvise_free_debug, "test vm.madvise_free_debug",
	   T_META_SYSCTL_INT("vm.madvise_free_debug=1"),
	   T_META_ASROOT(YES))
{
	// Skip if we dont' have vm_kernel_page_size < vm_page_size
	if (vm_kernel_page_size >= vm_page_size) {
		T_SKIP("vm_kernel_page_size >= vm_page_size, skipping subpage tests");
		return;
	}

	// Map 32k of memory.
	size_t memsz = 32 * vm_page_size;
	void *mem = mmap(NULL, memsz, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, 0, 0);
	T_EXPECT_NE_PTR(mem, MAP_FAILED, "mapped pages should not be MAP_FAILED");

	// Fill it with scribble.
	memset(mem, 0xa, 32 * vm_page_size);

	// Madvise a specfic page.
	T_EXPECT_POSIX_ZERO(
			madvise(mem + (4 * vm_kernel_page_size), vm_kernel_page_size, MADV_FREE_REUSABLE),
			"madvise (mem + 4 pages)");

	// Check the entire page is empty.
	T_EXPECT_BYTES(mem + (4 * vm_kernel_page_size), vm_kernel_page_size, 0x0, "madvise'd memory is all zeros");

	// Check that the subsequent page is 0xaa.
	T_EXPECT_BYTES(mem + (5 * vm_kernel_page_size), vm_kernel_page_size, 0xa, "un-madvise'd memory is all 0xaa");

	T_EXPECT_POSIX_SUCCESS(munmap(mem, memsz), "munmap");
}

// <rdar://problem/31844360> disable nano_subpage_madvise due to consistent
// failures.
#if 0

T_DECL(nano_subpage_madvise, "nano allocator madvise",
	   T_META_SYSCTL_INT("vm.madvise_free_debug=1"),
	   T_META_ENVVAR("MallocNanoZone=1"),
	   T_META_CHECK_LEAKS(NO),
	   T_META_ASROOT(YES))
{
	T_EXPECT_TRUE(malloc_engaged_nano(), "nano zone enabled");

	void *ptr = malloc(128);
	T_EXPECT_EQ_PTR(
			(void *)(((uintptr_t)ptr) >> (64-NANO_SIGNATURE_BITS)),
			(void *)NANOZONE_SIGNATURE,
			"malloc == nano allocation");
	free(ptr);

	const size_t granularity = 128;
	const size_t allocations = 128 * 1024;

	void *bank[allocations / granularity];
	for (int i=0; i<(sizeof(bank)/sizeof(*bank)); i++) {
		// allocate 128k of memory, scribble them
		bank[i] = malloc(granularity);
		memset(bank[i], 'A', granularity);
	}

	ptr = NULL;
	size_t limit = vm_kernel_page_size;
	for (int i=0; i<256; i++) {
		// find the first entry that lies on the user page
		// boundary, rather than kernel, to try and find
		// bugs where we accidentally round up to other page
		// sizes.
		if (!ptr && trunc_page((uintptr_t)bank[i]) != (uintptr_t)bank[i]) {
			continue;
		}

		// mark active, free the entry, then decrement the
		// limit until we get to a full page.
		if (!ptr) {
			ptr = bank[i];
		}

		free(bank[i]);
		bank[i] = NULL;
		limit -= 128;

		if (limit == 0) {
			// finished, break.
			break;
		}
	}

	// force the nano alloc to madvise things
	malloc_zone_pressure_relief(malloc_default_zone(), 0);

	// we should be able to test for the entire range that's
	// madvised being zeros now.
	T_EXPECT_BYTES(ptr, vm_kernel_page_size, 0x0,
			"madvised region was cleared");

	// and that the page immediately after the kernel page is
	// stil intacted.
	T_EXPECT_BYTES(ptr + vm_kernel_page_size, vm_kernel_page_size, 'A',
			"non-madvised page check");

	for (int i=0; i<(sizeof(bank)/sizeof(*bank)); i++) {
		free(bank[i]);
	}
}

#endif

#if 0
// OS X has the recirc depot enabled, so more has to be done to reliably test
// madvise on that platform.

T_DECL(tiny_subpage_madvise, "tiny allocator madvise",
	   T_META_SYSCTL_INT("vm.madvise_free_debug=1"),
	   T_META_ENVVAR("MallocNanoZone=0"),
	   T_META_ASROOT(YES))
{
	T_EXPECT_TRUE(!malloc_engaged_nano(), "nano zone disabled");

	malloc_zone_t *zone = malloc_create_zone(0, 0);

	const size_t granularity = 16;
	const size_t allocations = 128 * 1024;

	void *bank[allocations / granularity];
	for (int i=0; i<(sizeof(bank)/sizeof(*bank)); i++) {
		// allocate 128k of memory, scribble them
		bank[i] = malloc_zone_malloc(zone, granularity);
		memset(bank[i], 'A', granularity);
		printf("%p\n", bank[i]);

		if (i>0) {
			T_QUIET;
			T_ASSERT_EQ_PTR(((uintptr_t)bank[i-1]) + granularity,
							(uintptr_t)bank[i],
							"contiguous allocations required");
		}
	}

	void *ptr = NULL;
	size_t num_needed = vm_kernel_page_size / granularity + 1;

	for (int i=1; i<(sizeof(bank)/sizeof(*bank)); i++) {
		// find the first page aligned entry
		if (!ptr &&
			((uintptr_t)bank[i] > round_page_kernel((uintptr_t)bank[i]) ||
			 (uintptr_t)bank[i] + granularity - 1 < round_page_kernel((uintptr_t)bank[i])))
		{
			continue;
		}

		// when we find the entry, take this pointer and
		// also free the entry before.
		if (!ptr) {
			ptr = (void *)round_page_kernel((uintptr_t)bank[i]);
		}

		malloc_zone_free(zone, bank[i]);
		bank[i] = NULL;
		num_needed--;

		if (num_needed == 0) {
			break;
		}
	}

	T_ASSERT_NOTNULL(ptr, "expected pointer");

	// we should be able to test for the entire range that's
	// madvised being zeros now.
	T_EXPECT_BYTES(ptr, vm_kernel_page_size, 0x0,
				   "madvised region was cleared");

	// and that the page immediately after the kernel page is
	// stil intacted.
	T_EXPECT_BYTES(ptr + vm_kernel_page_size + granularity, vm_kernel_page_size, 'A',
				   "non-madvised page check");

	for (int i=0; i<(sizeof(bank)/sizeof(*bank)); i++) {
		malloc_zone_free(zone, bank[i]);
	}
}

T_DECL(small_subpage_madvise, "small allocator madvise",
	   T_META_SYSCTL_INT("vm.madvise_free_debug=1"),
	   T_META_ENVVAR("MallocNanoZone=0"))
{
	T_EXPECT_TRUE(!malloc_engaged_nano(), "nano zone disabled");

	const size_t granularity = 512;
	const size_t allocations = 128 * 1024;

	void *bank[allocations / granularity];
	for (int i=0; i<(sizeof(bank)/sizeof(*bank)); i++) {
		// allocate 128k of memory, scribble them
		bank[i] = malloc(granularity);
		memset(bank[i], 'A', granularity);
	}

	void *ptr = NULL;
	size_t num_needed = vm_kernel_page_size / granularity + 1;

	for (int i=1; i<(sizeof(bank)/sizeof(*bank)); i++) {
		// find the first page aligned entry
		if (!ptr &&
			((uintptr_t)bank[i] > round_page_kernel((uintptr_t)bank[i]) ||
			 (uintptr_t)bank[i] + granularity - 1 < round_page_kernel((uintptr_t)bank[i])))
		{
			continue;
		}

		// when we find the entry, take this pointer and
		// also free the entry before.
		if (!ptr) {
			ptr = (void *)round_page_kernel((uintptr_t)bank[i]);
		}

		free(bank[i]);
		bank[i] = NULL;
		num_needed--;

		if (num_needed == 0) {
			break;
		}
	}

	T_ASSERT_NOTNULL(ptr, "expected pointer");

	// we should be able to test for the entire range that's
	// madvised being zeros now.
	T_EXPECT_BYTES(ptr, vm_kernel_page_size, 0x0,
				   "madvised region was cleared");

	// and that the page immediately after the kernel page is
	// stil intacted.
	T_EXPECT_BYTES(ptr + vm_kernel_page_size + granularity, vm_kernel_page_size, 'A',
				   "non-madvised page check");

	for (int i=0; i<(sizeof(bank)/sizeof(*bank)); i++) {
		free(bank[i]);
	}
}
#endif // #if 0
