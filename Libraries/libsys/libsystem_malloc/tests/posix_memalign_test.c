//
//  posix_memalign_test.c
//  libmalloc
//
//  test allocating and freeing all sizes and alignments
//

#include <darwintest.h>
#include <errno.h>
#include <malloc/malloc.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

T_GLOBAL_META(T_META_RUN_CONCURRENTLY(true));

static inline void*
t_posix_memalign(size_t alignment, size_t size)
{
	void *ptr = NULL;
	int result = posix_memalign(&ptr, alignment, size);
	size_t allocated_size = malloc_size(ptr);

	T_QUIET; T_ASSERT_NOTNULL(ptr, "allocation");
	T_QUIET; T_ASSERT_EQ((intptr_t)ptr % alignment, 0ul, "pointer should be properly aligned");
	T_QUIET; T_EXPECT_LE(size, allocated_size, "allocation size");

	// Scribble memory pointed to by `ptr` to make sure we're not using that
	// memory for control structures. This also makes sure the memory can be
	// written to.
	const uint64_t pat = 0xdeadbeefcafebabeull;
	memset_pattern8(ptr, &pat, size);
	return ptr;
}

T_DECL(posix_memalign_free, "posix_memalign all power of two alignments <= 4096")
{
	for (size_t alignment = sizeof(void*); alignment < 4096; alignment *= 2) {
		// test several sizes
		for (size_t size = alignment; size <= 256*alignment; size += 8) {
			void* ptr = t_posix_memalign(alignment, size);
			free(ptr);
		}
	}
}

T_DECL(posix_memalign_alignment_not_a_power_of_2, "posix_memalign should return EINVAL if alignment is not a power of 2")
{
	{
		void *ptr = NULL;
		int result = posix_memalign(&ptr, 24, 48); // alignment is even, but not a power of two
		T_QUIET; T_ASSERT_NULL(ptr, "ptr should be null");
		T_QUIET; T_ASSERT_EQ(result, EINVAL, "posix_memalign should return EINVAL");
	}

	{
		void *ptr = NULL;
		int result = posix_memalign(&ptr, 23, 46); // alignment is odd, and not a power of two
		T_QUIET; T_ASSERT_NULL(ptr, "ptr should be null");
		T_QUIET; T_ASSERT_EQ(result, EINVAL, "posix_memalign should return EINVAL");
	}
}

T_DECL(posix_memalign_alignment_not_a_multiple_of_voidstar, "posix_memalign should return EINVAL if alignment is not a multiple of sizeof(void*)")
{
	void *ptr = NULL;
	const size_t alignment = sizeof(void*)+1;
	int result = posix_memalign(&ptr, alignment, alignment * 2);
	T_QUIET; T_ASSERT_NULL(ptr, "ptr should be null");
	T_QUIET; T_ASSERT_EQ(result, EINVAL, "posix_memalign should return EINVAL");
}

T_DECL(posix_memalign_allocate_size_0, "posix_memalign should return something that can be passed to free() when size is 0")
{
	void *ptr = NULL;
	int result = posix_memalign(&ptr, 8, 0);
	T_QUIET; T_ASSERT_EQ(result, 0, "posix_memalign should not return an error when asked for size 0");
	free(ptr);
}
