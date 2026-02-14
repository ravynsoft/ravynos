//
//  nano_tests.c
//  libmalloc
//
//  Tests that are specific to the implementation details of Nanov2.
//
#include <TargetConditionals.h>
#include <darwintest.h>
#include <darwintest_utils.h>
#include <spawn.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <malloc/malloc.h>
#include <../private/malloc_private.h>
#include <../src/internal.h>

#if CONFIG_NANOZONE

#pragma mark -
#pragma mark Enumerator access

static int range_count;					// Total number of allocated ranges
static int ptr_count;					// Total number of allocated pointers
static size_t total_ranges_size;		// Size of all allocated ranges
static size_t total_in_use_ptr_size;	// Size of all allocated pointers

static void
range_recorder(task_t task, void *context, unsigned type, vm_range_t *ranges,
			   unsigned count)
{
	for (int i = 0; i < count; i++) {
		total_ranges_size += ranges[i].size;
	}
	range_count += count;
}

static void
pointer_recorder(task_t task, void *context, unsigned type, vm_range_t *ranges,
			 unsigned count)
{
	for (int i = 0; i < count; i++) {
		total_in_use_ptr_size += ranges[i].size;
	}
	ptr_count += count;
}

static kern_return_t
memory_reader(task_t remote_task, vm_address_t remote_address, vm_size_t size,
			  void **local_memory)
{
	if (local_memory) {
		*local_memory = (void*)remote_address;
		return KERN_SUCCESS;
	}
	return KERN_FAILURE;
}

static void
run_enumerator()
{
	total_ranges_size = 0;
	total_in_use_ptr_size = 0;
	range_count = 0;
	ptr_count = 0;
	malloc_zone_t *zone = malloc_default_zone();
	zone->introspect->enumerator(mach_task_self(), NULL,
			MALLOC_PTR_REGION_RANGE_TYPE, (vm_address_t)zone, memory_reader,
			range_recorder);
	zone->introspect->enumerator(mach_task_self(), NULL,
			MALLOC_PTR_IN_USE_RANGE_TYPE, (vm_address_t)zone, memory_reader,
			pointer_recorder);
}

#endif // CONFIG_NANOZONE

#pragma mark -
#pragma mark Enumerator tests

#if TARGET_OS_WATCH
#define ALLOCATION_COUNT 10000
#else // TARGET_OS_WATCH
#define ALLOCATION_COUNT 100000
#endif // TARGET_OS_WATCH

static void *allocations[ALLOCATION_COUNT];

T_GLOBAL_META(T_META_RUN_CONCURRENTLY(true));

T_DECL(nano_active_test, "Test that Nano is activated",
	   T_META_ENVVAR("MallocNanoZone=1"))
{
#if CONFIG_NANOZONE
	void *ptr = malloc(16);
	T_LOG("Nano ptr is %p\n", ptr);
	T_ASSERT_EQ(NANOZONE_SIGNATURE, (uint64_t)((uintptr_t)ptr) >> SHIFT_NANO_SIGNATURE,
			"Nanozone is active");
	T_ASSERT_NE(malloc_engaged_nano(), 0, "Nanozone engaged");
	free(ptr);
#else // CONFIG_NANOZONE
	T_SKIP("Nano allocator not configured");
#endif // CONFIG_NANOZONE
}

T_DECL(nano_enumerator_test, "Test the Nanov2 enumerator",
	   T_META_ENVVAR("MallocNanoZone=V2"))
{
#if CONFIG_NANOZONE
	T_ASSERT_EQ(malloc_engaged_nano(), 2, "Nanozone V2 engaged");

	// This test is problematic because the allocator is used before the test
	// starts, so we can't start everything from zero.
	// Grab a baseline
	malloc_statistics_t stats;
	malloc_zone_statistics(malloc_default_zone(), &stats);
	const unsigned int initial_blocks_in_use = stats.blocks_in_use;
	const size_t initial_size_in_use = stats.size_in_use;
	const size_t initial_size_allocated = stats.size_allocated;

	run_enumerator();
	const int initial_ptrs = ptr_count;
	const size_t initial_ranges_size = total_ranges_size;
	const size_t initial_in_use_ptr_size = total_in_use_ptr_size;

	// Allocate memory of random sizes, all less than the max Nano size.
	size_t total_requested_size = 0;
	for (int i = 0; i < ALLOCATION_COUNT; i++) {
		size_t sz = malloc_good_size(arc4random_uniform(257));
		allocations[i] = malloc(sz);
		total_requested_size += sz;
	}

	// Get the stats and enumerator values again and check whether the result is consistent.
	malloc_zone_statistics(malloc_default_zone(), &stats);
	run_enumerator();

	T_ASSERT_EQ(stats.blocks_in_use, initial_blocks_in_use + ALLOCATION_COUNT,
			"Incorrect blocks_in_use");
	T_ASSERT_EQ(stats.size_in_use, initial_size_in_use + total_requested_size,
			"Incorrect size_in_use");
	T_ASSERT_TRUE(stats.size_allocated - initial_size_allocated >= total_requested_size,
			"Size allocated must be >= size requested");

	T_ASSERT_EQ(ptr_count, initial_ptrs + ALLOCATION_COUNT,
			"Incorrect number of pointers");
	T_ASSERT_EQ(total_in_use_ptr_size, initial_in_use_ptr_size + total_requested_size,
			"Incorrect in-use pointer size");

	// Free half of the memory and recheck the statistics
	size_t size_freed = 0;
	for (int i = 0; i < ALLOCATION_COUNT / 2; i++) {
		size_freed += malloc_size(allocations[i]);
		free(allocations[i]);
	}

	// Check the stats and enumerator values.
	malloc_zone_statistics(malloc_default_zone(), &stats);
	run_enumerator();
	T_ASSERT_EQ(stats.blocks_in_use, initial_blocks_in_use + ALLOCATION_COUNT/2,
			"Incorrect blocks_in_use after half free");
	T_ASSERT_EQ(stats.size_in_use,
			initial_size_in_use + total_requested_size - size_freed,
			"Incorrect size_in_use after half free");
	T_ASSERT_TRUE(stats.size_allocated >= initial_size_allocated ,
			"Size allocated must be >= size requested");

	T_ASSERT_EQ(ptr_count, initial_ptrs + ALLOCATION_COUNT / 2,
			"Incorrect number of pointers after half free");
	T_ASSERT_EQ(total_in_use_ptr_size,
			initial_in_use_ptr_size + total_requested_size - size_freed,
			"Incorrect in-use pointer size after half free");

	// Free the rest the memory and recheck the statistics
	for (int i = ALLOCATION_COUNT / 2; i < ALLOCATION_COUNT; i++) {
		free(allocations[i]);
	}
	
	// Check the stats and enumerator values one more time.
	malloc_zone_statistics(malloc_default_zone(), &stats);
	run_enumerator();

	T_ASSERT_EQ(stats.blocks_in_use, initial_blocks_in_use,
			"Incorrect blocks_in_use after full free");
	T_ASSERT_EQ(stats.size_in_use, initial_size_in_use,
			"Incorrect size_in_use after full free");
	T_ASSERT_TRUE(stats.size_allocated >= initial_size_allocated ,
			"Size allocated must be >= size requested");
	
	T_ASSERT_EQ(ptr_count, initial_ptrs, "Incorrect number of pointers after free");
	T_ASSERT_EQ(total_in_use_ptr_size, initial_in_use_ptr_size,
			"Incorrect in-use pointer size after free");
#else // CONFIG_NANOZONE
	T_SKIP("Nano allocator not configured");
#endif // CONFIG_NANOZONE
}

#pragma mark -
#pragma mark Nano realloc tests

// These tests are specific to the Nano implementation of realloc(). They
// don't necessarily work with other allocators, since the behavior tested is
// not part of the documented behavior of realloc().

const char * const data = "abcdefghijklm";

T_DECL(realloc_nano_size_class_change, "realloc with size class change",
	   T_META_ENVVAR("MallocNanoZone=1"))
{
#if CONFIG_NANOZONE
	void *ptr = malloc(16);
	strcpy(ptr, data);
	void *new_ptr;

	// Each pass of the loop realloc's to the next size class up. We must
	// get a new pointer each time and the content must have been copied.
	for (int i = 32; i <= 256; i += 16) {
		new_ptr = realloc(ptr, i);
		T_QUIET; T_ASSERT_TRUE(ptr != new_ptr, "realloc pointer should change");
		T_QUIET; T_ASSERT_EQ(i, (int)malloc_size(new_ptr), "Check size for new allocation");
		T_QUIET; T_ASSERT_TRUE(!strncmp(new_ptr, data, strlen(data)), "Content must be copied");
		T_QUIET; T_ASSERT_EQ(0, (int)malloc_size(ptr), "Old allocation not freed");
		ptr = new_ptr;
	}
	free(new_ptr);
#else // CONFIG_NANOZONE
	T_SKIP("Nano allocator not configured");
#endif // CONFIG_NANOZONE
}

T_DECL(realloc_nano_ptr_change, "realloc with pointer change",
	   T_META_ENVVAR("MallocNanoZone=1"))
{
#if CONFIG_NANOZONE
	void *ptr = malloc(32);
	strcpy(ptr, data);

	void *new_ptr = realloc(ptr, 128);
	T_ASSERT_TRUE(ptr != new_ptr, "realloc pointer should change");
	T_ASSERT_EQ(128, (int)malloc_size(new_ptr), "Wrong size for new allocation");
	T_ASSERT_TRUE(!strncmp(new_ptr, data, strlen(data)), "Content must be copied");
	T_ASSERT_EQ(0, (int)malloc_size(ptr), "Old allocation not freed");
	
	free(new_ptr);
#else // CONFIG_NANOZONE
	T_SKIP("Nano allocator not configured");
#endif // CONFIG_NANOZONE
}

T_DECL(realloc_nano_to_other, "realloc with allocator change (nano)",
	   T_META_ENVVAR("MallocNanoZone=1"))
{
#if CONFIG_NANOZONE
	void *ptr = malloc(32);					// From Nano
	strcpy(ptr, data);

	void *new_ptr = realloc(ptr, 1024);		// Cannot be Nano.

	T_ASSERT_TRUE(ptr != new_ptr, "realloc pointer should change");
	T_ASSERT_EQ(1024, (int)malloc_size(new_ptr), "Wrong size for new allocation");
	T_ASSERT_TRUE(!strncmp(new_ptr, data, strlen(data)), "Content must be copied");
	T_ASSERT_EQ(0, (int)malloc_size(ptr), "Old allocation not freed");

	free(new_ptr);
#else // CONFIG_NANOZONE
	T_SKIP("Nano allocator not configured");
#endif // CONFIG_NANOZONE
}

T_DECL(realloc_nano_to_zero_size, "realloc with target size zero",
	   T_META_ENVVAR("MallocNanoZone=1"))
{
#if CONFIG_NANOZONE
	void *ptr = malloc(16);

	// Realloc to 0 frees the old memory and returns a valid pointer.
	void *new_ptr = realloc(ptr, 0);
	T_ASSERT_EQ(0, (int)malloc_size(ptr), "Old allocation not freed");
	T_ASSERT_NOTNULL(new_ptr, "New allocation must be non-NULL");
	T_ASSERT_TRUE(malloc_size(new_ptr) > 0, "New allocation not known");

	free(new_ptr);
#else // CONFIG_NANOZONE
	T_SKIP("Nano allocator not configured");
#endif // CONFIG_NANOZONE
}

T_DECL(realloc_nano_shrink, "realloc to smaller size",
	   T_META_ENVVAR("MallocNanoZone=1"))
{
#if CONFIG_NANOZONE
	void *ptr = malloc(64);
	strcpy(ptr, data);

	// Reallocate to greater than half the current size - should remain
	// in-place.
	void *new_ptr = realloc(ptr, 40);
	T_ASSERT_TRUE(ptr == new_ptr, "realloc pointer should not change");
	T_ASSERT_TRUE(!strncmp(new_ptr, data, strlen(data)), "Content changed");

	// Reallocate to less than half the current size - should get a new pointer
	// Realloc to 0 frees the old memory and returns a valid pointer.
	ptr = new_ptr;
	new_ptr = realloc(ptr, 16);
	T_ASSERT_TRUE(ptr != new_ptr, "realloc pointer should change");
	T_ASSERT_TRUE(!strncmp(new_ptr, data, strlen(data)), "Content must be copied");

	free(new_ptr);
#else // CONFIG_NANOZONE
	T_SKIP("Nano allocator not configured");
#endif // CONFIG_NANOZONE
}

#pragma mark -
#pragma mark Nanov2 tests

// These tests are specific to the implementation of the Nanov2 allocator.

// Guaranteed number of 256-byte allocations to be sure we fill an arena.
#define ALLOCS_PER_ARENA ((NANOV2_ARENA_SIZE)/256)

T_DECL(overspill_arena, "force overspill of an arena",
	   T_META_ENVVAR("MallocNanoZone=V2"),
	   T_META_ENVVAR("MallocGuardEdges=all"))
{
#if CONFIG_NANOZONE
	void **ptrs = calloc(ALLOCS_PER_ARENA, sizeof(void *));
	T_QUIET; T_ASSERT_NOTNULL(ptrs, "Unable to allocate pointers");
	int index;

	nanov2_addr_t first_ptr;
	ptrs[0] = malloc(256);
	T_QUIET; T_ASSERT_NOTNULL(ptrs[index], "Failed to allocate");
	first_ptr.addr = ptrs[0];

	for (index = 1; index < ALLOCS_PER_ARENA; index++) {
		ptrs[index] = malloc(256);
		T_QUIET; T_ASSERT_NOTNULL(ptrs[index], "Failed to allocate");

		// Stop allocating once we have crossed into a new arena.
		nanov2_addr_t current_ptr;
		current_ptr.addr = ptrs[index];
		if (current_ptr.fields.nano_arena != first_ptr.fields.nano_arena) {
			break;
		}

		// Write to the pointer to ensure the containing block is not
		// a guard block.
		*(int *)ptrs[index] = 0;
	}

	// Free everything, which is a check that the book-keeping works across
	// arenas.
	for (int i = 0; i <= index; i++) {
		free(ptrs[i]);
	}
	free(ptrs);
#else // CONFIG_NANOZONE
	T_SKIP("Nano allocator not configured");
#endif // CONFIG_NANOZONE
}

#if TARGET_OS_OSX

// Guaranteed number of 256-byte allocations to be sure we fill a region.
#define ALLOCS_PER_REGION ((NANOV2_REGION_SIZE)/256)

// This test is required only on macOS because iOS only uses one region.
T_DECL(overspill_region, "force overspill of a region",
	   T_META_ENVVAR("MallocNanoZone=V2"))
{
#if CONFIG_NANOZONE
	void **ptrs = calloc(ALLOCS_PER_REGION, sizeof(void *));
	T_QUIET; T_ASSERT_NOTNULL(ptrs, "Unable to allocate pointers");
	int index;

	nanov2_addr_t first_ptr;
	ptrs[0] = malloc(256);
	T_QUIET; T_ASSERT_NOTNULL(ptrs[index], "Failed to allocate");
	first_ptr.addr = ptrs[0];

	for (index = 1; index < ALLOCS_PER_REGION; index++) {
		ptrs[index] = malloc(256);
		T_QUIET; T_ASSERT_NOTNULL(ptrs[index], "Failed to allocate");

		// Stop allocating once we have crossed into a new region.
		nanov2_addr_t current_ptr;
		current_ptr.addr = ptrs[index];
		if (current_ptr.fields.nano_region != first_ptr.fields.nano_region) {
			break;
		}
	}

	// Free everything, which is a check that the book-keeping works across
	// regions.
	for (int i = 0; i <= index; i++) {
		free(ptrs[i]);
	}
	free(ptrs);
#else // CONFIG_NANOZONE
	T_SKIP("Nano allocator not configured");
#endif // CONFIG_NANOZONE
}
#endif // TARGET_OS_OSX

