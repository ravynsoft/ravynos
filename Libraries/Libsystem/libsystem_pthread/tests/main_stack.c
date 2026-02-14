#include <stdlib.h>
#include <pthread.h>
#include "darwintest_defaults.h"
#include <machine/vmparam.h>

T_DECL(main_stack, "tests the reported values for the main thread stack",
		T_META_CHECK_LEAKS(NO), T_META_ALL_VALID_ARCHS(YES)){
	const uintptr_t stackaddr = (uintptr_t)pthread_get_stackaddr_np(pthread_self());
	const size_t stacksize = pthread_get_stacksize_np(pthread_self());
	T_LOG("stack: %zx -> %zx (+%zx)", stackaddr - stacksize, stackaddr, stacksize);
	T_EXPECT_LT((uintptr_t)__builtin_frame_address(0), stackaddr, NULL);
	T_EXPECT_GT((uintptr_t)__builtin_frame_address(0), stackaddr - stacksize, NULL);

	struct rlimit lim;
	T_ASSERT_POSIX_SUCCESS(getrlimit(RLIMIT_STACK, &lim), NULL);
	T_EXPECT_EQ((size_t)lim.rlim_cur, pthread_get_stacksize_np(pthread_self()), "reported rlimit should match stacksize");

	lim.rlim_cur = lim.rlim_cur / 8;
	T_ASSERT_POSIX_SUCCESS(setrlimit(RLIMIT_STACK, &lim), NULL);

	T_EXPECTFAIL;
	T_EXPECT_EQ((size_t)lim.rlim_cur, pthread_get_stacksize_np(pthread_self()), "new rlimit should should match stacksize");
}
