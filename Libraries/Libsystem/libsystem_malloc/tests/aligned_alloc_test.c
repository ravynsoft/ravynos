//
//  aligned_alloc_test.c
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
t_aligned_alloc(size_t alignment, size_t size)
{
	void *ptr = aligned_alloc(alignment, size);
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

T_DECL(aligned_alloc_free, "aligned_alloc all power of two alignments <= 256kb")
{
	for (size_t alignment = sizeof(void*); alignment < 4096; alignment *= 2) {
		// test several sizes that are multiples of the alignment
		for (size_t size = alignment; size <= 512*alignment; size += alignment) {
			void* ptr = t_aligned_alloc(alignment, size);
			free(ptr);
		}
	}
}

T_DECL(aligned_alloc_alignment_not_multiple_of_size, "aligned_alloc should set errno to EINVAL if size is not a multiple of alignment")
{
	{
		void *ptr = aligned_alloc(8, 12);
		T_QUIET; T_ASSERT_NULL(ptr, "ptr should be null");
		T_QUIET; T_ASSERT_EQ(errno, EINVAL, "errno should be EINVAL");
	}

	{
		void *ptr = aligned_alloc(32, 16);
		T_QUIET; T_ASSERT_NULL(ptr, "ptr should be null");
		T_QUIET; T_ASSERT_EQ(errno, EINVAL, "errno should be EINVAL");
	}
}

T_DECL(aligned_alloc_alignment_not_power_of_two, "aligned_alloc should set errno to EINVAL if alignment is not a power of two (implementation constraint)")
{
	{
		void *ptr = aligned_alloc(24, 48); // alignment is even, but not a power of two
		T_QUIET; T_ASSERT_NULL(ptr, "ptr should be null");
		T_QUIET; T_ASSERT_EQ(errno, EINVAL, "errno should be EINVAL");
	}

	{
		void *ptr = aligned_alloc(23, 46); // alignment is odd, and not a power of two
		T_QUIET; T_ASSERT_NULL(ptr, "ptr should be null");
		T_QUIET; T_ASSERT_EQ(errno, EINVAL, "errno should be EINVAL");
	}
}

T_DECL(aligned_alloc_alignment_not_a_multiple_of_voidstar, "aligned_alloc should set errno to EINVAL if alignment is not a multiple of sizeof(void*) (implementation constraint)")
{
	const size_t alignment = sizeof(void*)+1;
	void *ptr = aligned_alloc(alignment, alignment * 2);
	T_QUIET; T_ASSERT_NULL(ptr, "ptr should be null");
	T_QUIET; T_ASSERT_EQ(errno, EINVAL, "aligned_alloc should set errno to EINVAL");
}
