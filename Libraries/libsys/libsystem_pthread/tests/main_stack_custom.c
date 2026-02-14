#include <stdlib.h>
#include <pthread.h>
#include "darwintest_defaults.h"
#include <machine/vmparam.h>

T_DECL(main_stack_custom, "tests the reported values for a custom main thread stack"){
	T_EXPECT_EQ((size_t)STACKSIZE, pthread_get_stacksize_np(pthread_self()), NULL);

	const uintptr_t stackaddr = (uintptr_t)pthread_get_stackaddr_np(pthread_self());
	size_t stacksize = pthread_get_stacksize_np(pthread_self());
	T_LOG("stack: %zx -> %zx (+%zx)", stackaddr - stacksize, stackaddr, stacksize);
	T_EXPECT_LT((uintptr_t)__builtin_frame_address(0), stackaddr, NULL);
	T_EXPECT_GT((uintptr_t)__builtin_frame_address(0), stackaddr - stacksize, NULL);

	struct rlimit lim;
	T_QUIET; T_ASSERT_POSIX_SUCCESS(getrlimit(RLIMIT_STACK, &lim), NULL);
	lim.rlim_cur = lim.rlim_cur + 32 * PAGE_SIZE;
	T_EXPECT_EQ(setrlimit(RLIMIT_STACK, &lim), -1, "setrlimit for stack should fail with custom stack");
	T_EXPECT_EQ((size_t)STACKSIZE, pthread_get_stacksize_np(pthread_self()), "reported stacksize shouldn't change");
}
