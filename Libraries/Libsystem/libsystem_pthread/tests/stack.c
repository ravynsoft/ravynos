#include <signal.h>
#include <pthread/stack_np.h>

#include "darwintest_defaults.h"
#include <darwintest_utils.h>

#if defined(__arm64__)
#define call_chkstk(value) \
		__asm__ volatile("orr x9, xzr, %0\t\n" \
				"bl _thread_chkstk_darwin" : : "i"(value) : "x9")
#elif defined(__x86_64__)
#define call_chkstk(value) \
		__asm__ volatile("movq %0, %%rax\t\n" \
				"callq _thread_chkstk_darwin" : : "i"(value) : "rax")
#elif defined(__i386__)
#define call_chkstk(value) \
		__asm__ volatile("movl %0, %%eax\t\n" \
				"calll _thread_chkstk_darwin" : : "i"(value) : "eax")
#endif

static void
got_signal(int signo __unused)
{
	T_PASS("calling with 1 << 24 crashed");
	T_END;
}

T_DECL(chkstk, "chkstk",
		T_META_ALL_VALID_ARCHS(YES), T_META_CHECK_LEAKS(NO))
{
#if defined(__arm__)
	T_SKIP("not on armv7");
#else

	call_chkstk(1 << 8);
	T_PASS("calling with 1 << 8");

	call_chkstk(1 << 16);
	T_PASS("calling with 1 << 16");

	stack_t ss = {
		.ss_sp    = malloc(MINSIGSTKSZ),
		.ss_size  = MINSIGSTKSZ,
	};
	T_ASSERT_POSIX_SUCCESS(sigaltstack(&ss, NULL), "sigaltstack");

	struct sigaction sa = {
		.sa_handler = got_signal,
		.sa_flags = SA_ONSTACK,
	};
	T_ASSERT_POSIX_SUCCESS(sigaction(SIGSEGV, &sa, NULL), "sigaction");

#if __LP64__
	call_chkstk(1ul << 32);
#else
	call_chkstk(1ul << 24);
#endif
	T_FAIL("should have crashed");
#endif
}

struct frame {
	uintptr_t frame;
	uintptr_t ret;
};

OS_NOINLINE OS_NOT_TAIL_CALLED
static void
do_stack_frame_decode_test(struct frame frames[], size_t n, size_t count)
{
	if (n < count) {
		frames[n].frame = (uintptr_t)__builtin_frame_address(1);
		frames[n].ret = (uintptr_t)__builtin_return_address(0);
		do_stack_frame_decode_test(frames, n + 1, count);
	} else {
		uintptr_t frame = (uintptr_t)__builtin_frame_address(1);
		uintptr_t ret;
		while (count-- > 0) {
			frame = pthread_stack_frame_decode_np(frame, &ret);
			T_EXPECT_EQ(frames[count].frame, frame, "Frame %zd", count);
			T_EXPECT_EQ(frames[count].ret, ret, "Retaddr %zd", count);
		}
	}
}

T_DECL(pthread_stack_frame_decode_np, "pthread_stack_frame_decode_np",
		T_META_ALL_VALID_ARCHS(YES), T_META_CHECK_LEAKS(NO))
{
	struct frame frames[10];
	frames[0].frame = (uintptr_t)__builtin_frame_address(1);
	frames[0].ret = (uintptr_t)__builtin_return_address(0);
	do_stack_frame_decode_test(frames, 1, 10);
}
