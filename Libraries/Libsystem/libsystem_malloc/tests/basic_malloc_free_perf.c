//
//  basic_malloc_free_perf.c
//  libmalloc
//
//  Simple and repeatable performance tests for malloc/free, running tests
// 	on the regular, Nanov1 and Nanov2 allocators.
//
#include <darwintest.h>
#include <dispatch/dispatch.h>
#include <../src/internal.h>
#include <perfcheck_keys.h>

// This value is a guess that will be refined over time.
#define PERFCHECK_THRESHOLD_PCT	10.0

static uint32_t
ncpu(void)
{
	static uint32_t activecpu, physicalcpu;
	if (!activecpu) {
		uint32_t n;
		size_t s = sizeof(n);
		sysctlbyname("hw.activecpu", &n, &s, NULL, 0);
		activecpu = n;
		s = sizeof(n);
		sysctlbyname("hw.physicalcpu", &n, &s, NULL, 0);
		physicalcpu = n;
	}
	return MIN(activecpu, physicalcpu);
}

// Code to run a single test and save the converged sample time as the DPS
// metric. The code also measures the time taken in dispatch_apply(), but that
// should be more or less constant in all cases. We are only interested in
// whether the overall sampled time regresses, not in the absolute time value.

static void
run_test(void (^test)(size_t), bool singlethreaded)
{
	uint32_t nthreads = 0;
	if (singlethreaded) {
		nthreads = 1;
	} else {
		const char *e;
		if ((e = getenv("DT_STAT_NTHREADS"))) {
			nthreads = strtoul(e, NULL, 0);
		}
		if (nthreads < 2) {
			nthreads = ncpu();
		}
	}
	dt_stat_time_t s = dt_stat_time_create(
			nthreads > 1 ? "basic_malloc_free_perf multithreaded" :
					"basic_malloc_free_perf singlethreaded");
	dt_stat_set_variable((dt_stat_t)s, "threads", nthreads);
	do {
		int batch_size = dt_stat_batch_size(s);
		dt_stat_token t = dt_stat_begin(s);
		// rdar://problem/40417821: disable thresholds for now.
		//dt_stat_set_variable((dt_stat_t)s, kPCFailureThresholdPctVar,
		//		PERFCHECK_THRESHOLD_PCT);

		dispatch_apply(nthreads,
				dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0), test);

		dt_stat_end_batch(s, batch_size, t);
	} while (!dt_stat_stable(s));
	dt_stat_finalize(s);
}

// Test content. Each of the functions is called from six different test cases,
// with singlethreaded true and false and MallocNanoZone set to 0, V1 and V2.
// The test content is biased towards the implementation of Nanov2.

static void
basic_perf_malloc_free_8_bytes(bool singlethreaded)
{
#define NUM_ALLOCS 512
	run_test(^(size_t iteration __unused) {
		void *ptrs[NUM_ALLOCS];
		for (int i = 0; i < NUM_ALLOCS; i++) {
			ptrs[i] = malloc(8);
		}

		for (int i = 0; i < NUM_ALLOCS; i++) {
			free(ptrs[i]);
		}
	}, singlethreaded);
#undef NUM_ALLOCS
}

static void
basic_perf_malloc_free_8_bytes_multi_block(bool singlethreaded)
{
	// Use a large number of allocations to amortize the cost of scanning
	// for a new block.
#define NUM_ALLOCS 65535
	run_test(^(size_t iteration __unused) {
		void **ptrs = calloc(NUM_ALLOCS, sizeof(void *));
		for (int i = 0; i < NUM_ALLOCS; i++) {
			ptrs[i] = malloc(8);
		}

		for (int i = 0; i < NUM_ALLOCS; i++) {
			free(ptrs[i]);
		}
		free(ptrs);
	}, singlethreaded);
#undef NUM_ALLOCS
}

static void
basic_perf_malloc_free_different_size_classes(bool singlethreaded)
{
#define NUM_ALLOCS 512
	run_test(^(size_t iteration __unused) {
		void *ptrs[NUM_ALLOCS];
		size_t sz = (iteration + 1) * 16;
		if (sz > 256) {
			// Too big for Nano.
			return;
		}
		for (int i = 0; i < NUM_ALLOCS; i++) {
			ptrs[i] = malloc(sz);
		}

		for (int i = 0; i < NUM_ALLOCS; i++) {
			free(ptrs[i]);
		}
	}, singlethreaded);
#undef NUM_ALLOCS
}

static void
basic_perf_malloc_free_by_size_class(bool singlethreaded)
{
#define NUM_LOOPS 16
	run_test(^(size_t iteration __unused) {
		void *ptrs[NUM_LOOPS * 16];
		int k = 0;
		for (int i = 0; i < NUM_LOOPS; i++) {
			for (int j = 0; j < 16; j++) {
				ptrs[k++] = malloc(16 * j + 8);
			}
		}

		for (int i = 0; i < k; i++) {
			free(ptrs[i]);
		}
	}, singlethreaded);
#undef NUM_LOOPS
}

static void
basic_perf_malloc_free_by_size_class_offset(bool singlethreaded)
{
#define NUM_LOOPS 16
	int cpu_number = _os_cpu_number();
	run_test(^(size_t iteration __unused) {
		void *ptrs[NUM_LOOPS * 16];
		int k = 0;
		for (int i = 0; i < NUM_LOOPS; i++) {
			for (int j = 0; j < 16; j++) {
				ptrs[k++] = malloc(16 * ((j + cpu_number) % 16) + 8);
			}
		}

		for (int i = 0; i < k; i++) {
			free(ptrs[i]);
		}
	}, singlethreaded);
#undef NUM_LOOPS
}

// The tests that follow are grouped as follows:
// 	- single-thread non-Nano version
//  - single-threaded NanoV1 version
//	- single-threaded NanoV2 version
// 	- parallel non-Nano version
//  - parallel NanoV1 version
//	- parallel NanoV2 version
// Each group probably could be built with a macro, but that would be harder
// to debug when there is a problem.

#pragma mark -
#pragma mark 8-byte allocation/free

T_DECL(basic_perf_serial_8_bytes, "Malloc/Free 8 bytes single-threaded",
	   T_META_TAG_PERF, T_META_ALL_VALID_ARCHS(NO),
	   T_META_LTEPHASE(LTE_POSTINIT), T_META_CHECK_LEAKS(false),
	   T_META_ENVVAR("MallocNanoZone=0"))
{
	basic_perf_malloc_free_8_bytes(true);
}

T_DECL(basic_perf_serial_8_bytes_V1, "Malloc/Free 8 bytes single-threaded on V1",
	   T_META_TAG_PERF, T_META_ALL_VALID_ARCHS(NO),
	   T_META_LTEPHASE(LTE_POSTINIT), T_META_CHECK_LEAKS(false),
	   T_META_ENVVAR("MallocNanoZone=V1"))
{
#if CONFIG_NANOZONE
	basic_perf_malloc_free_8_bytes(true);
#else // CONFIG_NANOZONE
	T_SKIP("Nano allocator not configured");
#endif // CONFIG_NANOZONE
}

T_DECL(basic_perf_serial_8_bytes_V2, "Malloc/Free 8 bytes single-threaded on V2",
	   T_META_TAG_PERF, T_META_ALL_VALID_ARCHS(NO),
	   T_META_LTEPHASE(LTE_POSTINIT), T_META_CHECK_LEAKS(false),
	   T_META_ENVVAR("MallocNanoZone=V2"))
{
#if CONFIG_NANOZONE
	basic_perf_malloc_free_8_bytes(true);
#else // CONFIG_NANOZONE
	T_SKIP("Nano allocator not configured");
#endif // CONFIG_NANOZONE
}

T_DECL(basic_perf_parallel_8_bytes, "Malloc/Free 8 bytes parallel",
	   T_META_TAG_PERF, T_META_ALL_VALID_ARCHS(NO),
	   T_META_LTEPHASE(LTE_POSTINIT), T_META_CHECK_LEAKS(false),
	   T_META_ENVVAR("MallocNanoZone=0"))
{
	basic_perf_malloc_free_8_bytes(false);
}

T_DECL(basic_perf_parallel_8_bytes_V1, "Malloc/Free 8 bytes parallel on V1",
	   T_META_TAG_PERF, T_META_ALL_VALID_ARCHS(NO),
	   T_META_LTEPHASE(LTE_POSTINIT), T_META_CHECK_LEAKS(false),
	   T_META_ENVVAR("MallocNanoZone=V1"))
{
#if CONFIG_NANOZONE
	basic_perf_malloc_free_8_bytes(false);
#else // CONFIG_NANOZONE
	T_SKIP("Nano allocator not configured");
#endif // CONFIG_NANOZONE
}

T_DECL(basic_perf_parallel_8_bytes_V2, "Malloc/Free 8 bytes single-threaded on V2",
	   T_META_TAG_PERF, T_META_ALL_VALID_ARCHS(NO),
	   T_META_LTEPHASE(LTE_POSTINIT), T_META_CHECK_LEAKS(false),
	   T_META_ENVVAR("MallocNanoZone=V2"))
{
#if CONFIG_NANOZONE
	basic_perf_malloc_free_8_bytes(false);
#else // CONFIG_NANOZONE
	T_SKIP("Nano allocator not configured");
#endif // CONFIG_NANOZONE
}

#pragma mark -
#pragma mark 8-byte allocation/free forcing block overflow with default scan policy

T_DECL(basic_perf_serial_8_bytes_multi_block_default_scan_policy,
	   "Malloc/Free 8 bytes single-threaded with block overflow, default scan policy",
	   T_META_TAG_PERF, T_META_ALL_VALID_ARCHS(NO),
	   T_META_LTEPHASE(LTE_POSTINIT), T_META_CHECK_LEAKS(false),
	   T_META_ENVVAR("MallocNanoZone=0"))
{
	basic_perf_malloc_free_8_bytes_multi_block(true);
}

T_DECL(basic_perf_serial_8_bytes_multi_block_default_scan_policy_V1,
	   	"Malloc/Free 8 bytes single-threaded with block overflow, default scan policy on V1",
	   T_META_TAG_PERF, T_META_ALL_VALID_ARCHS(NO),
	   T_META_LTEPHASE(LTE_POSTINIT), T_META_CHECK_LEAKS(false),
	   T_META_ENVVAR("MallocNanoZone=V1"))
{
#if CONFIG_NANOZONE
	basic_perf_malloc_free_8_bytes_multi_block(true);
#else // CONFIG_NANOZONE
	T_SKIP("Nano allocator not configured");
#endif // CONFIG_NANOZONE
}

T_DECL(basic_perf_serial_8_bytes_multi_block_default_scan_policy_V2,
	   	"Malloc/Free 8 bytes single-threaded with block overflow, default scan policy on V2",
	   T_META_TAG_PERF, T_META_ALL_VALID_ARCHS(NO),
	   T_META_LTEPHASE(LTE_POSTINIT), T_META_CHECK_LEAKS(false),
	   T_META_ENVVAR("MallocNanoZone=V2"))
{
#if CONFIG_NANOZONE
	basic_perf_malloc_free_8_bytes_multi_block(true);
#else // CONFIG_NANOZONE
	T_SKIP("Nano allocator not configured");
#endif // CONFIG_NANOZONE
}

T_DECL(basic_perf_parallel_8_bytes_multi_block_default_scan_policy,
	   "Malloc/Free 8 bytes parallel with block overflow, default scan policy",
	   T_META_TAG_PERF, T_META_ALL_VALID_ARCHS(NO),
	   T_META_LTEPHASE(LTE_POSTINIT), T_META_CHECK_LEAKS(false),
	   T_META_ENVVAR("MallocNanoZone=0"))
{
	basic_perf_malloc_free_8_bytes_multi_block(false);
}

T_DECL(basic_perf_parallel_8_bytes_multi_block_default_scan_policy_V1,
	   	"Malloc/Free 8 bytes parallel with block overflow, default scan policy on V1",
	   T_META_TAG_PERF, T_META_ALL_VALID_ARCHS(NO),
	   T_META_LTEPHASE(LTE_POSTINIT), T_META_CHECK_LEAKS(false),
	   T_META_ENVVAR("MallocNanoZone=V1"))
{
#if CONFIG_NANOZONE
	basic_perf_malloc_free_8_bytes_multi_block(false);
#else // CONFIG_NANOZONE
	T_SKIP("Nano allocator not configured");
#endif // CONFIG_NANOZONE
}

T_DECL(basic_perf_parallel_8_bytes_multi_block_default_scan_policy_V2,
	   	"Malloc/Free 8 bytes parallel with block overflow, default scan policy on V2",
	   T_META_TAG_PERF, T_META_ALL_VALID_ARCHS(NO),
	   T_META_LTEPHASE(LTE_POSTINIT), T_META_CHECK_LEAKS(false),
	   T_META_ENVVAR("MallocNanoZone=V2"))
{
#if CONFIG_NANOZONE
	basic_perf_malloc_free_8_bytes_multi_block(false);
#else // CONFIG_NANOZONE
	T_SKIP("Nano allocator not configured");
#endif // CONFIG_NANOZONE
}

#pragma mark -
#pragma mark 8-byte allocation/free forcing block overflow with first-fit

// This test only makes sense on Nanov2
T_DECL(basic_perf_serial_8_bytes_multi_block_first_fit_V2,
	   "Malloc/Free 8 bytes single-threaded with block overflow, first-fit on V2",
	   T_META_TAG_PERF, T_META_ALL_VALID_ARCHS(NO),
	   T_META_LTEPHASE(LTE_POSTINIT), T_META_CHECK_LEAKS(false),
	   T_META_ENVVAR("MallocNanoZone=V2"),
	   T_META_ENVVAR("MallocNanoScanPolicy=firstfit"))
{
#if CONFIG_NANOZONE
	basic_perf_malloc_free_8_bytes_multi_block(true);
#else // CONFIG_NANOZONE
	T_SKIP("Nano allocator not configured");
#endif // CONFIG_NANOZONE
}

T_DECL(basic_perf_parallel_8_bytes_multi_block_first_fit_V2,
	   "Malloc/Free 8 bytes parallel with block overflow, first-fit on V2",
	   T_META_TAG_PERF, T_META_ALL_VALID_ARCHS(NO),
	   T_META_LTEPHASE(LTE_POSTINIT),
	   T_META_ENVVAR("MallocNanoZone=V2"),
	   T_META_ENVVAR("MallocNanoScanPolicy=firstfit"))
{
#if CONFIG_NANOZONE
	basic_perf_malloc_free_8_bytes_multi_block(false);
#else // CONFIG_NANOZONE
	T_SKIP("Nano allocator not configured");
#endif // CONFIG_NANOZONE
}

#pragma mark -
#pragma mark Repeated allocation/free where each thread uses a different size class

T_DECL(basic_perf_serial_different_size_classes,
	   "Malloc/Free in different size classes single-threaded",
	   T_META_TAG_PERF, T_META_ALL_VALID_ARCHS(NO),
	   T_META_LTEPHASE(LTE_POSTINIT),
	   T_META_ENVVAR("MallocNanoZone=0"))
{
	basic_perf_malloc_free_different_size_classes(false);
}

T_DECL(basic_perf_serial_different_size_classes_V1,
	   "Malloc/Free in different size classes single-threaded on V1",
	   T_META_TAG_PERF, T_META_ALL_VALID_ARCHS(NO),
	   T_META_LTEPHASE(LTE_POSTINIT),
	   T_META_ENVVAR("MallocNanoZone=V1"))
{
#if CONFIG_NANOZONE
	basic_perf_malloc_free_different_size_classes(false);
#else // CONFIG_NANOZONE
	T_SKIP("Nano allocator not configured");
#endif // CONFIG_NANOZONE
}

T_DECL(basic_perf_serial_different_size_classes_V2,
	   "Malloc/Free in different size classes single-threaded on V2",
	   T_META_TAG_PERF, T_META_ALL_VALID_ARCHS(NO),
	   T_META_LTEPHASE(LTE_POSTINIT),
	   T_META_ENVVAR("MallocNanoZone=V2"))
{
#if CONFIG_NANOZONE
	basic_perf_malloc_free_different_size_classes(true);
#else // CONFIG_NANOZONE
	T_SKIP("Nano allocator not configured");
#endif // CONFIG_NANOZONE
}

T_DECL(basic_perf_parallel_different_size_classes,
	   "Malloc/Free in different size classes parallel",
	   T_META_TAG_PERF, T_META_ALL_VALID_ARCHS(NO),
	   T_META_LTEPHASE(LTE_POSTINIT),
	   T_META_ENVVAR("MallocNanoZone=0"))
{
	basic_perf_malloc_free_different_size_classes(false);
}

T_DECL(basic_perf_parallel_different_size_classes_V1,
	   "Malloc/Free in different size classes parallel on V1",
	   T_META_TAG_PERF, T_META_ALL_VALID_ARCHS(NO),
	   T_META_LTEPHASE(LTE_POSTINIT),
	   T_META_ENVVAR("MallocNanoZone=V1"))
{
#if CONFIG_NANOZONE
	basic_perf_malloc_free_different_size_classes(false);
#else // CONFIG_NANOZONE
	T_SKIP("Nano allocator not configured");
#endif // CONFIG_NANOZONE
}

T_DECL(basic_perf_parallel_different_size_classes_V2,
	   "Malloc/Free in different size classes single-threaded on V2",
	   T_META_TAG_PERF, T_META_ALL_VALID_ARCHS(NO),
	   T_META_LTEPHASE(LTE_POSTINIT),
	   T_META_ENVVAR("MallocNanoZone=V2"))
{
#if CONFIG_NANOZONE
	basic_perf_malloc_free_different_size_classes(false);
#else // CONFIG_NANOZONE
	T_SKIP("Nano allocator not configured");
#endif // CONFIG_NANOZONE
}

#pragma mark -
#pragma mark Repeated allocation/free for each size class

T_DECL(basic_perf_serial_by_size_class,
	   "Malloc/Free by size class single-threaded",
	   T_META_TAG_PERF, T_META_ALL_VALID_ARCHS(NO),
	   T_META_LTEPHASE(LTE_POSTINIT),
	   T_META_ENVVAR("MallocNanoZone=0"))
{
	basic_perf_malloc_free_by_size_class(true);
}

T_DECL(basic_perf_serial_by_size_class_V1,
	   "Malloc/Free by size class single-threaded on V1",
	   T_META_TAG_PERF, T_META_ALL_VALID_ARCHS(NO),
	   T_META_LTEPHASE(LTE_POSTINIT),
	   T_META_ENVVAR("MallocNanoZone=V1"))
{
#if CONFIG_NANOZONE
	basic_perf_malloc_free_by_size_class(true);
#else // CONFIG_NANOZONE
	T_SKIP("Nano allocator not configured");
#endif // CONFIG_NANOZONE
}

T_DECL(basic_perf_serial_by_size_class_V2,
	   "Malloc/Free by size class single-threaded on V2",
	   T_META_TAG_PERF, T_META_ALL_VALID_ARCHS(NO),
	   T_META_LTEPHASE(LTE_POSTINIT),
	   T_META_ENVVAR("MallocNanoZone=V2"))
{
#if CONFIG_NANOZONE
	basic_perf_malloc_free_by_size_class(true);
#else // CONFIG_NANOZONE
	T_SKIP("Nano allocator not configured");
#endif // CONFIG_NANOZONE
}

T_DECL(basic_perf_parallel_by_size_class,
	   "Malloc/Free by size class parallel",
	   T_META_TAG_PERF, T_META_ALL_VALID_ARCHS(NO),
	   T_META_LTEPHASE(LTE_POSTINIT),
	   T_META_ENVVAR("MallocNanoZone=0"))
{
	basic_perf_malloc_free_by_size_class(false);
}

T_DECL(basic_perf_parallel_by_size_class_V1,
	   "Malloc/Free by size class parallel on V1",
	   T_META_TAG_PERF, T_META_ALL_VALID_ARCHS(NO),
	   T_META_LTEPHASE(LTE_POSTINIT),
	   T_META_ENVVAR("MallocNanoZone=V1"))
{
#if CONFIG_NANOZONE
	basic_perf_malloc_free_by_size_class(false);
#else // CONFIG_NANOZONE
	T_SKIP("Nano allocator not configured");
#endif // CONFIG_NANOZONE
}

T_DECL(basic_perf_parallel_by_size_class_V2,
	   "Malloc/Free by size class single-threaded on V2",
	   T_META_TAG_PERF, T_META_ALL_VALID_ARCHS(NO),
	   T_META_LTEPHASE(LTE_POSTINIT),
	   T_META_ENVVAR("MallocNanoZone=V2"))
{
#if CONFIG_NANOZONE
	basic_perf_malloc_free_by_size_class(false);
#else // CONFIG_NANOZONE
	T_SKIP("Nano allocator not configured");
#endif // CONFIG_NANOZONE
}

#pragma mark -
#pragma mark Repeated allocation/free for each size class, offset by CPU

T_DECL(basic_perf_serial_by_size_class_offset,
	   "Malloc/Free by size class with offset single-threaded",
	   T_META_TAG_PERF, T_META_ALL_VALID_ARCHS(NO),
	   T_META_LTEPHASE(LTE_POSTINIT),
	   T_META_ENVVAR("MallocNanoZone=0"))
{
	basic_perf_malloc_free_by_size_class_offset(true);
}

T_DECL(basic_perf_serial_by_size_class_offset_V1,
	   	"Malloc/Free by size class with offset single-threaded on V1",
	   T_META_TAG_PERF, T_META_ALL_VALID_ARCHS(NO),
	   T_META_LTEPHASE(LTE_POSTINIT),
	   T_META_ENVVAR("MallocNanoZone=V1"))
{
#if CONFIG_NANOZONE
	basic_perf_malloc_free_by_size_class_offset(true);
#else // CONFIG_NANOZONE
	T_SKIP("Nano allocator not configured");
#endif // CONFIG_NANOZONE
}

T_DECL(basic_perf_serial_by_size_class_offset_V2,
	   	"Malloc/Free by size class with offset single-threaded on V2",
	   T_META_TAG_PERF, T_META_ALL_VALID_ARCHS(NO),
	   T_META_LTEPHASE(LTE_POSTINIT),
	   T_META_ENVVAR("MallocNanoZone=V2"))
{
#if CONFIG_NANOZONE
	basic_perf_malloc_free_by_size_class_offset(true);
#else // CONFIG_NANOZONE
	T_SKIP("Nano allocator not configured");
#endif // CONFIG_NANOZONE
}

T_DECL(basic_perf_parallel_by_size_class_offset,
	   "Malloc/Free by size class with offset parallel",
	   T_META_TAG_PERF, T_META_ALL_VALID_ARCHS(NO),
	   T_META_LTEPHASE(LTE_POSTINIT),
	   T_META_ENVVAR("MallocNanoZone"))
{
	basic_perf_malloc_free_by_size_class_offset(false);
}

T_DECL(basic_perf_parallel_by_size_class_offset_V1,
	   	"Malloc/Free by size class with offset parallel on V1",
	   T_META_TAG_PERF, T_META_ALL_VALID_ARCHS(NO),
	   T_META_LTEPHASE(LTE_POSTINIT),
	   T_META_ENVVAR("MallocNanoZone=V1"))
{
#if CONFIG_NANOZONE
	basic_perf_malloc_free_by_size_class_offset(false);
#else // CONFIG_NANOZONE
	T_SKIP("Nano allocator not configured");
#endif // CONFIG_NANOZONE
}

T_DECL(basic_perf_parallel_by_size_class_offset_V2,
	   	"Malloc/Free by size class with offset single-threaded on V2",
	   T_META_TAG_PERF, T_META_ALL_VALID_ARCHS(NO),
	   T_META_LTEPHASE(LTE_POSTINIT),
	   T_META_ENVVAR("MallocNanoZone=V2"))
{
#if CONFIG_NANOZONE
	basic_perf_malloc_free_by_size_class_offset(false);
#else // CONFIG_NANOZONE
	T_SKIP("Nano allocator not configured");
#endif // CONFIG_NANOZONE
}

