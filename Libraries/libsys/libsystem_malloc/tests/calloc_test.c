//
//  malloc_calloc_test.c
//  libmalloc
//
//  test calloc for various sizes
//
#include <TargetConditionals.h>
#include <darwintest.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <malloc/malloc.h>

T_GLOBAL_META(T_META_RUN_CONCURRENTLY(true));

static inline void*
t_calloc(size_t count, size_t s)
{
	void *ptr = calloc(count, s);
	T_QUIET; T_ASSERT_NOTNULL(ptr, "allocation");
	size_t sz = malloc_size(ptr);
	T_QUIET; T_EXPECT_GE(sz, s * count, "allocation size");
	char *p = ptr;
	for (int i = 0; i < s * count; i++, p++) {
		T_QUIET; T_ASSERT_EQ(*p, '\0', "nonzero byte at offset %d\n", i);
	}
	return ptr;
}

static void
test_calloc(size_t count, size_t min, size_t max, size_t incr)
{
	for (size_t s =  min; s <= max; s += incr) {
		void *ptr = t_calloc(count, s);
		free(ptr);
	}
}

static void
test_calloc_random(size_t count, size_t min, size_t max, size_t incr, size_t n)
{
	const size_t r = (max - min) / incr, P = 100;
	void *ptrs[P] = {};
	for (size_t i = 0, j = 0, k = 0; i < n + P; i++, j = k, k = (k + 1) % P) {
		void *ptr = NULL;
		if (i < n) ptr = t_calloc(count, min + arc4random_uniform(r) * incr);
		free(ptrs[j]);
		ptrs[k] = ptr;
	}
}

T_DECL(calloc_overflow_nano, "calloc with overflow (nano)",
	T_META_ENVVAR("MallocNanoZone=1"))
{
	void *ptr = calloc(LONG_MAX, 256);
	T_ASSERT_EQ(ptr, NULL, "calloc overflow check #1");
	free(ptr);

	ptr = calloc(256, LONG_MAX);
	T_ASSERT_EQ(ptr, NULL, "calloc overflow check #2");
	free(ptr);
}

T_DECL(calloc_overflow, "calloc with overflow",
	T_META_ENVVAR("MallocNanoZone=0"))
{
	void *ptr = calloc(LONG_MAX, 1000);
	T_ASSERT_EQ(ptr, NULL, "calloc overflow check #1");
	free(ptr);

	ptr = calloc(1000, LONG_MAX);
	T_ASSERT_EQ(ptr, NULL, "calloc overflow check #2");
	free(ptr);
}

T_DECL(calloc_nano, "nano calloc all sizes <= 256",
	   T_META_ENVVAR("MallocNanoZone=1"))
{
	test_calloc(1, 0, 256, 1);		// NANO_MAX_SIZE
	test_calloc_random(1, 0, 256, 1, 100);

	test_calloc(16, 0, 256/16, 1);	// NANO_MAX_SIZE
	test_calloc_random(16, 0, 256/16, 1, 100);

	test_calloc(32, 0, 256/32, 1);	// NANO_MAX_SIZE
	test_calloc_random(32, 0, 256/32, 1, 100);
}

T_DECL(calloc_tiny, "tiny calloc 16b increments <= 1008",
	   T_META_ENVVAR("MallocNanoZone=0"))
{
	test_calloc(1, 0, 1008, 16); 		// SMALL_THRESHOLD
	test_calloc_random(1, 0, 1008, 16, 100);

	test_calloc(4, 0, 1008/4, 16);		// SMALL_THRESHOLD
	test_calloc_random(4, 0, 1008/4, 16, 100);

	test_calloc(16, 0, 1008/16, 16);	// SMALL_THRESHOLD
	test_calloc_random(16, 0, 1008/16, 16, 100);
}

// The next test is too demanding for some watches (taking over 15 minutes to
// run) and for some AppleTVs, so use a cut-down version.
#if TARGET_OS_WATCH || TARGET_OS_TV
T_DECL(calloc, "calloc all 2048b increments <= 130kb",
	   T_META_ENVVAR("MallocNanoZone=0"))
{
	test_calloc(1, 1024, 130 * 1024, 2048);		// > LARGE_THRESHOLD_LARGEMEM
	test_calloc_random(1, 1024, 130 * 1024, 2048, 50);

	test_calloc(16, 1024/16, 130 * 1024/16, 2048);
	test_calloc_random(16, 1024/16, 130 * 1024/16, 2048, 50);

	test_calloc(64, 1024/64, 130 * 1024/64, 2048);
	test_calloc_random(64, 1024/64, 130 * 1024/64, 2048, 50);
}
#else // !TARGET_OS_WATCH && !TARGET_OS_TV
T_DECL(calloc, "calloc all 512b increments <= 256kb",
	   T_META_ENVVAR("MallocNanoZone=0"))
{
	test_calloc(1, 1024, 256 * 1024, 512);		// > LARGE_THRESHOLD_LARGEMEM
	test_calloc_random(1, 1024, 256 * 1024, 512, 100);

	test_calloc(16, 1024/16, 256 * 1024/16, 512);
	test_calloc_random(16, 1024/16, 256 * 1024/16, 512, 100);

	test_calloc(64, 1024/64, 256 * 1024/64, 512);
	test_calloc_random(64, 1024/64, 256 * 1024/64, 512, 100);
}
#endif // !TARGET_OS_WATCH && !TARGET_OS_TV

