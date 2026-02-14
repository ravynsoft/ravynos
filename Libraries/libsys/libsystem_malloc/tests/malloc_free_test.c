//
//  malloc_free_test.c
//  libmalloc
//
//  test allocating and freeing all sizes
//

#include <darwintest.h>
#include <stdlib.h>
#include <stdio.h>
#include <malloc/malloc.h>

T_GLOBAL_META(T_META_RUN_CONCURRENTLY(true));

static inline void*
t_malloc(size_t s)
{
	void *ptr = malloc(s);
	T_QUIET; T_ASSERT_NOTNULL(ptr, "allocation");
	size_t sz = malloc_size(ptr);
	T_QUIET; T_EXPECT_LE(s, sz, "allocation size");
	const uint64_t pat = 0xdeadbeefcafebabeull;
	memset_pattern8(ptr, &pat, s);
	return ptr;
}

static void
test_malloc_free(size_t min, size_t max, size_t incr)
{
	for (size_t s =  min; s <= max; s += incr) {
		void *ptr = t_malloc(s);
		free(ptr); // try to go through mag_last_free SMALL_CACHE
	}
	for (size_t s = min, t = max; s <= max; s += incr, t -= incr) {
		void *ptr1 = t_malloc(s);
		void *ptr2 = t_malloc(t);
		free(ptr1); // try to defeat mag_last_free SMALL_CACHE
		free(ptr2);
	}
}

static void
test_malloc_free_random(size_t min, size_t max, size_t incr, size_t n)
{
	const size_t r = (max - min) / incr, P = 100;
	void *ptrs[P] = {};
	for (size_t i = 0, j = 0, k = 0; i < n + P; i++, j = k, k = (k + 1) % P) {
		void *ptr = NULL;
		if (i < n) ptr = t_malloc(min + arc4random_uniform(r) * incr);
		free(ptrs[j]);
		ptrs[k] = ptr;
	}
}

T_DECL(malloc_free_nano, "nanomalloc and free all sizes <= 256",
	   T_META_ENVVAR("MallocNanoZone=1"))
{
	test_malloc_free(0, 256, 1); // NANO_MAX_SIZE
	test_malloc_free_random(0, 256, 1, 10000);
}

T_DECL(malloc_free_tiny, "tiny malloc and free 16b increments <= 1008",
	   T_META_ENVVAR("MallocNanoZone=0"))
{
	test_malloc_free(0, 1008, 16); // SMALL_THRESHOLD
	test_malloc_free_random(0, 1008, 16, 10000);
}

T_DECL(malloc_free, "malloc and free all 512b increments <= 256kb",
	   T_META_ENVVAR("MallocNanoZone=0"))
{
	test_malloc_free(1024, 256 * 1024, 512); // > LARGE_THRESHOLD_LARGEMEM
	test_malloc_free_random(1024, 256 * 1024, 512, 100000);
}
