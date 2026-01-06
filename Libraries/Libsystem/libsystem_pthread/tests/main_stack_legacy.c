#include <stdlib.h>
#include <pthread.h>
#include "darwintest_defaults.h"
#include <machine/vmparam.h>

T_DECL(main_stack_legacy, "tests the reported values for a custom main thread stack",
	   T_META_CHECK_LEAKS(NO))
{
	T_EXPECT_LT((uintptr_t)0, pthread_get_stacksize_np(pthread_self()), NULL);

	const uintptr_t stackaddr = (uintptr_t)pthread_get_stackaddr_np(pthread_self());
	size_t stacksize = pthread_get_stacksize_np(pthread_self());
	T_LOG("stack: %zx -> %zx (+%zx)", stackaddr - stacksize, stackaddr, stacksize);
	T_EXPECT_LT((uintptr_t)__builtin_frame_address(0), stackaddr, NULL);
	T_EXPECT_GT((uintptr_t)__builtin_frame_address(0), stackaddr - stacksize, NULL);
}
