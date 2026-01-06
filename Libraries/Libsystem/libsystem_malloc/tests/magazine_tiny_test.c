//
//  magazine_tiny_test.c
//  libmalloc
//
//  Created by Matt Wright on 8/22/16.
//
//

#include <darwintest.h>

#include "../src/magazine_tiny.c"
#include "magazine_testing.h"

T_GLOBAL_META(T_META_RUN_CONCURRENTLY(true));

static inline void
test_rack_setup(rack_t *rack)
{
	memset(rack, 'a', sizeof(rack));
	rack_init(rack, RACK_TYPE_TINY, 1, 0);
	T_QUIET; T_ASSERT_NOTNULL(rack->magazines, "magazine initialisation");
}

T_DECL(basic_tiny_alloc, "tiny rack init and alloc")
{
	struct rack_s rack;
	test_rack_setup(&rack);

	void *ptr = tiny_malloc_should_clear(&rack, TINY_MSIZE_FOR_BYTES(32), false);
	T_ASSERT_NOTNULL(ptr, "allocation");

	region_t *rgn = tiny_region_for_ptr_no_lock(&rack, ptr);
	T_ASSERT_NOTNULL(rgn, "allocation region found in rack");

	size_t sz = tiny_size(&rack, ptr);
	T_ASSERT_EQ((int)sz, 32, "size == 32");
}

T_DECL(basic_tiny_teardown, "tiny rack init, alloc, teardown")
{
	struct rack_s rack;
	test_rack_setup(&rack);

	void *ptr = tiny_malloc_should_clear(&rack, TINY_MSIZE_FOR_BYTES(32), false);
	T_ASSERT_NOTNULL(ptr, "allocation");

	rack_destroy_regions(&rack, TINY_REGION_SIZE);
	for (int i=0; i < rack.region_generation->num_regions_allocated; i++) {
		T_QUIET;
		T_ASSERT_TRUE(rack.region_generation->hashed_regions[i] == HASHRING_OPEN_ENTRY ||
					  rack.region_generation->hashed_regions[i] == HASHRING_REGION_DEALLOCATED,
					  "all regions destroyed");
	}

	rack_destroy(&rack);
	T_ASSERT_NULL(rack.magazines, "magazines destroyed");
}

T_DECL(basic_tiny_free, "tiny free")
{
	struct rack_s rack;
	test_rack_setup(&rack);

	void *ptr = tiny_malloc_should_clear(&rack, TINY_MSIZE_FOR_BYTES(32), false);
	T_ASSERT_NOTNULL(ptr, "allocation");

	// free doesn't return an error (unless we assert here)
	free_tiny(&rack, ptr, TINY_REGION_FOR_PTR(ptr), 0, false);

	size_t sz = tiny_size(&rack, ptr);
	T_ASSERT_EQ((int)sz, 0, "allocation freed (sz == 0)");
}

T_DECL(basic_tiny_shrink, "tiny rack shrink")
{
	struct rack_s rack;
	test_rack_setup(&rack);

	void *ptr = tiny_malloc_should_clear(&rack, TINY_MSIZE_FOR_BYTES(64), false);
	T_ASSERT_NOTNULL(ptr, "allocation");

	size_t sz = tiny_size(&rack, ptr);
	T_ASSERT_EQ((int)sz, 64, "size == 64");

	void *nptr = tiny_try_shrink_in_place(&rack, ptr, sz, 16);
	size_t nsz = tiny_size(&rack, nptr);
	T_ASSERT_EQ_PTR(ptr, nptr, "ptr == nptr");
	T_ASSERT_EQ((int)nsz, 16, "nsz == 16");
}

T_DECL(basic_tiny_realloc_in_place, "tiny rack realloc in place")
{
	struct rack_s rack;
	test_rack_setup(&rack);

	// Allocate two blocks and free the second, then try to realloc() the first.
	// This should extend in-place using the one-level death row cache that's
	// occupied by the second block.
	void *ptr = tiny_malloc_should_clear(&rack, TINY_MSIZE_FOR_BYTES(16), false);
	T_ASSERT_NOTNULL(ptr, "allocation");

	size_t sz = tiny_size(&rack, ptr);
	T_ASSERT_EQ((int)sz, 16, "size == 16");

	void *ptr2 = tiny_malloc_should_clear(&rack, TINY_MSIZE_FOR_BYTES(16), false);
	T_ASSERT_NOTNULL(ptr2, "allocation 2");
	T_ASSERT_EQ_PTR(ptr2, (void *)((uintptr_t)ptr + 16), "sequential allocations");
	free_tiny(&rack, ptr2, TINY_REGION_FOR_PTR(ptr2), 0, false);

	// Attempt to realloc up to 32 bytes, this should happen in place
	// because of the death-row cache.
	boolean_t reallocd = tiny_try_realloc_in_place(&rack, ptr, sz, 32);
	T_ASSERT_TRUE(reallocd, "realloced #1");

	size_t nsz = tiny_size(&rack, ptr);
	T_ASSERT_EQ((int)nsz, 32, "realloc size == 32");

	// Try another realloc(). This should extend in place because the rest of
	// the rack is empty.
	reallocd = tiny_try_realloc_in_place(&rack, ptr, nsz, 64);
	T_ASSERT_TRUE(reallocd, "realloced #2");
	nsz = tiny_size(&rack, ptr);
	T_ASSERT_EQ((int)nsz, 64, "realloc size == 64");

	free_tiny(&rack, ptr, TINY_REGION_FOR_PTR(ptr), 0, false);
}
