#include <stdio.h>
#include <stdlib.h>
#include <dispatch/dispatch.h>
#include <dispatch/private.h>
#include <sys/time.h>
#include <sys/sysctl.h>

#include "darwintest_defaults.h"
#include <darwintest_utils.h>

static uint64_t end_spin;

static uint32_t
get_ncpu(void)
{
	static uint32_t activecpu;
	if (!activecpu) {
		uint32_t n;
		size_t s = sizeof(activecpu);
		sysctlbyname("hw.activecpu", &n, &s, NULL, 0);
		activecpu = n;
	}
	return activecpu;
}

static void
spin_and_pause(void *ctx)
{
	long i = (long)ctx;

	printf("Thread %ld starts\n", i);

	while (clock_gettime_nsec_np(CLOCK_MONOTONIC) < end_spin) {
#if defined(__x86_64__) || defined(__i386__)
		__asm__("pause");
#elif defined(__arm__) || defined(__arm64__)
		__asm__("wfe");
#endif
	}
	printf("Thread %ld blocks\n", i);
	pause();
}

static void
spin(void *ctx)
{
	long i = (long)ctx;

	printf("Thread %ld starts\n", i);

	while (clock_gettime_nsec_np(CLOCK_MONOTONIC)) {
#if defined(__x86_64__) || defined(__i386__)
		__asm__("pause");
#elif defined(__arm__) || defined(__arm64__)
		__asm__("wfe");
#endif
	}
}

T_DECL(thread_request_32848402, "repro for rdar://32848402")
{
	dispatch_queue_attr_t bg_attr, in_attr;

	bg_attr = dispatch_queue_attr_make_with_qos_class(DISPATCH_QUEUE_CONCURRENT,
			QOS_CLASS_BACKGROUND, 0);
	in_attr = dispatch_queue_attr_make_with_qos_class(DISPATCH_QUEUE_CONCURRENT,
			QOS_CLASS_USER_INITIATED, 0);

	dispatch_queue_t a = dispatch_queue_create_with_target("in", in_attr, NULL);
	dispatch_queue_t b = dispatch_queue_create_with_target("bg", bg_attr, NULL);

	end_spin = clock_gettime_nsec_np(CLOCK_MONOTONIC) + 2 * NSEC_PER_SEC;

	dispatch_async_f(a, (void *)0, spin_and_pause);
	long n_threads = MIN((long)get_ncpu(),
			pthread_qos_max_parallelism(QOS_CLASS_BACKGROUND, 0));
	for (long i = 1; i < n_threads; i++) {
		dispatch_async_f(b, (void *)i, spin);
	}

	dispatch_async(b, ^{
		T_PASS("The NCPU+1-nth block got scheduled");
		T_END;
	});

	sleep(10);
	T_FAIL("The NCPU+1-nth block didn't get scheduled");
}
