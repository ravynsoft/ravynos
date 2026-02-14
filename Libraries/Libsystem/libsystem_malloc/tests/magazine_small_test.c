//
//  magazine_small_test.c
//  libmalloc
//
//  Created by Matt Wright on 8/22/16.
//
//

#include <darwintest.h>

#include "../src/magazine_small.c"
#include "magazine_testing.h"

T_GLOBAL_META(T_META_RUN_CONCURRENTLY(true));

static inline void
test_rack_setup(rack_t *rack)
{
	memset(rack, 'a', sizeof(rack));
	rack_init(rack, RACK_TYPE_SMALL, 1, 0);
	T_QUIET; T_ASSERT_NOTNULL(rack->magazines, "magazine initialisation");
}

T_DECL(basic_small_alloc, "small rack init and alloc")
{
	struct rack_s rack;
	test_rack_setup(&rack);

	void *ptr = small_malloc_should_clear(&rack, SMALL_MSIZE_FOR_BYTES(512), false);
	T_ASSERT_NOTNULL(ptr, "allocation");

	region_t *rgn = small_region_for_ptr_no_lock(&rack, ptr);
	T_ASSERT_NOTNULL(rgn, "allocation region found in rack");

	size_t sz = small_size(&rack, ptr);
	T_ASSERT_EQ((int)sz, 512, "size == 32");
}

T_DECL(basic_small_teardown, "small rack init, alloc, teardown")
{
	struct rack_s rack;
	test_rack_setup(&rack);

	void *ptr = small_malloc_should_clear(&rack, TINY_MSIZE_FOR_BYTES(512), false);
	T_ASSERT_NOTNULL(ptr, "allocation");

	rack_destroy_regions(&rack, SMALL_REGION_SIZE);
	for (int i=0; i < rack.region_generation->num_regions_allocated; i++) {
		T_QUIET;
		T_ASSERT_TRUE(rack.region_generation->hashed_regions[i] == HASHRING_OPEN_ENTRY ||
					  rack.region_generation->hashed_regions[i] == HASHRING_REGION_DEALLOCATED,
					  "all regions destroyed");
	}

	rack_destroy(&rack);
	T_ASSERT_NULL(rack.magazines, "magazines destroyed");
}

T_DECL(basic_small_free, "small free")
{
	struct rack_s rack;
	test_rack_setup(&rack);

	void *ptr = small_malloc_should_clear(&rack, SMALL_MSIZE_FOR_BYTES(512), false);
	T_ASSERT_NOTNULL(ptr, "allocation");

	// free doesn't return an error (unless we assert here)
	free_small(&rack, ptr, SMALL_REGION_FOR_PTR(ptr), 0);

	size_t sz = small_size(&rack, ptr);
	T_ASSERT_EQ((int)sz, 0, "allocation freed (sz == 0)");
}

T_DECL(basic_small_shrink, "small rack shrink")
{
	struct rack_s rack;
	test_rack_setup(&rack);

	void *ptr = small_malloc_should_clear(&rack, SMALL_MSIZE_FOR_BYTES(1024), false);
	T_ASSERT_NOTNULL(ptr, "allocation");

	size_t sz = small_size(&rack, ptr);
	T_ASSERT_EQ((int)sz, 1024, "size == 1024");

	void *nptr = small_try_shrink_in_place(&rack, ptr, sz, 512);
	size_t nsz = small_size(&rack, nptr);
	T_ASSERT_EQ_PTR(ptr, nptr, "ptr == nptr");
	T_ASSERT_EQ((int)nsz, 512, "nsz == 512");
}

T_DECL(basic_small_realloc_in_place, "small rack realloc in place")
{
	struct rack_s rack;
	test_rack_setup(&rack);

	// Allocate two blocks and free the second, then try to realloc() the first.
	// This should extend in-place using the one-level death row cache that's
	// occupied by the second block.
	void *ptr = small_malloc_should_clear(&rack, SMALL_MSIZE_FOR_BYTES(512), false);
	T_ASSERT_NOTNULL(ptr, "allocation");

	size_t sz = small_size(&rack, ptr);
	T_ASSERT_EQ((int)sz, 512, "size == 512");

	void *ptr2 = small_malloc_should_clear(&rack, SMALL_MSIZE_FOR_BYTES(512), false);
	T_ASSERT_NOTNULL(ptr2, "allocation 2");
	T_ASSERT_EQ_PTR(ptr2, (void *)((uintptr_t)ptr + 512), "sequential allocations");

	free_small(&rack, ptr2, SMALL_REGION_FOR_PTR(ptr2), 0);

	// Attempt to realloc up to 1024 bytes, this should happen in place
	// because of the death-row cache.
	boolean_t reallocd = small_try_realloc_in_place(&rack, ptr, sz, 1024);
	T_ASSERT_TRUE(reallocd, "realloced");

	size_t nsz = small_size(&rack, ptr);
	T_ASSERT_EQ((int)nsz, 1024, "realloc size == 1024");

	// Try another realloc(). This should extend in place because the rest of
	// the rack is empty.
	reallocd = small_try_realloc_in_place(&rack, ptr, nsz, 2048);
	T_ASSERT_TRUE(reallocd, "realloced #2");
	nsz = small_size(&rack, ptr);
	T_ASSERT_EQ((int)nsz, 2048, "realloc size == 2048");

	free_small(&rack, ptr, SMALL_REGION_FOR_PTR(ptr), 0);
}
