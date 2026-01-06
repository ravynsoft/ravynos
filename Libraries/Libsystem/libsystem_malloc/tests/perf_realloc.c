#include <stdlib.h>
#include <mach/vm_page_size.h>
#include <../src/internal.h>
#include <darwintest.h>
#include <perfcheck_keys.h>

#pragma mark -
#pragma mark Helper code

#define PAGE_SPREAD 8
#define MAX_NAME_SIZE 256
#define OUTLIER_TIME 20000

// This value is a guess that will be refined over time.
#define PERFCHECK_THRESHOLD_PCT	10.0

static char name[MAX_NAME_SIZE];
#define NUMBER_OF_SAMPLES_FOR_BATCH(s) _dt_stat_batch_size((dt_stat_t)s)

// Measures the time required to realloc a block of memory of given size
// by a specified amount.
static void
realloc_by_amount(const char *base_metric_name, size_t size, ssize_t amount,
		bool amount_is_random)
{
	if (amount_is_random) {
		snprintf(name, MAX_NAME_SIZE, amount >= 0 ?
			 "%s_from_%llu_up_random" : "%s_from_%llu_down_random",
			 base_metric_name, (uint64_t)size);
	} else {
		snprintf(name, MAX_NAME_SIZE, amount >= 0 ?
				 "%s_from_%llu_up_%ld" : "%s_from_%llu_down_%ld",
				 base_metric_name, (uint64_t)size, labs(amount));
	}

	uint64_t total_time = 0;
	int count = 0;
	dt_stat_time_t s = dt_stat_time_create(name);
	dt_stat_set_variable((dt_stat_t)s, "size (bytes)", (unsigned int)size);
	dt_stat_set_variable((dt_stat_t)s, "amount (bytes)", (int)amount);
	// rdar://problem/40417821: disable thresholds for now.
	//dt_stat_set_variable((dt_stat_t)s, kPCFailureThresholdPctVar,
	//		PERFCHECK_THRESHOLD_PCT);
	dt_stat_token now = dt_stat_time_begin(s);

	for (;;) {
		void *ptr = malloc(size);
		T_QUIET; T_ASSERT_NOTNULL(ptr, "malloc for size %llu failed", (uint64_t)size);

		dt_stat_token start = dt_stat_time_begin(s);
		ptr = realloc(ptr, size + amount);
		dt_stat_token end = dt_stat_time_begin(s);
		T_QUIET; T_ASSERT_NOTNULL(ptr,
				"realloc from size %llu to size %llu failed", (uint64_t)size,
				(uint64_t)(size + amount));
		total_time += end - start;
		free(ptr);

		if (++count >= NUMBER_OF_SAMPLES_FOR_BATCH(s)) {
			// Discard outliers or the test won't converge -- this should
			// be done in libdarwintest
			if (total_time/count < OUTLIER_TIME) {
				dt_stat_mach_time_add_batch(s, count, total_time);
			}
			total_time = 0;
			count = 0;
			if (dt_stat_stable(s)) {
				break;
			}
		}
	}

	dt_stat_finalize(s);
}

// Times a set of size adjustments from a given base.
static void
realloc_test_set(const char *base_name, size_t start_size)
{
	ssize_t adj = 1;
	realloc_by_amount(base_name, start_size, adj, false);
	realloc_by_amount(base_name, start_size + adj, -adj, false);

	adj = 8;
	realloc_by_amount(base_name, start_size, adj, false);
	realloc_by_amount(base_name, start_size + adj, -adj, false);

	adj = 16;
	realloc_by_amount(base_name, start_size, adj, false);
	realloc_by_amount(base_name, start_size + adj, -adj, false);

#if INCLUDE_RANDOM_SIZE_TESTS
	adj = 1 + arc4random_uniform(vm_page_size - 1);
	realloc_by_amount(base_name, start_size, adj, true);
	realloc_by_amount(base_name, start_size + adj, -adj, true);

	uint32_t pages = 1 + arc4random_uniform(PAGE_SPREAD);
	adj = pages * vm_page_size;
	realloc_by_amount(base_name, start_size, adj, true);
	realloc_by_amount(base_name, start_size + adj, -adj, true);
#else // INCLUDE_RANDOM_SIZE_TESTS
	T_LOG("Skipping random size tests");
#endif // INCLUDE_RANDOM_SIZE_TESTS

	adj = vm_page_size;
	realloc_by_amount(base_name, start_size, adj, false);
	realloc_by_amount(base_name, start_size + adj, -adj, false);

	adj = 4 * vm_page_size;
	realloc_by_amount(base_name, start_size, adj, false);
	realloc_by_amount(base_name, start_size + adj, -adj, false);

	// Adjust up and then down by over half the size of the allocation --
	// this skips the fast path in some allocators.
	adj = 4 * vm_page_size;
	realloc_by_amount(base_name, start_size, adj, false);
	realloc_by_amount(base_name, start_size + adj, -(((ssize_t)start_size) + adj)/2 - 1, false);

	adj = 32;
	realloc_by_amount(base_name, start_size, adj, false);
	realloc_by_amount(base_name, start_size + adj, -(((ssize_t)start_size) + adj)/2 - 1, false);
}

static void
realloc_tests(const char *base_name, boolean_t using_nano)
{
	// tiny or nano
	realloc_test_set(using_nano ? base_name : "Tiny", 8);

	if (!using_nano) {
		// No point in running these tests three times.

		// tiny
		realloc_test_set("Tiny", 512);

		// small
		realloc_test_set("Small", 2048);

		// large
		realloc_test_set("Large", 128 * 1024);
	}

	T_END;
}

#pragma mark -
#pragma mark Tests for realloc()

T_DECL(realloc_perf_base, "realloc without nano",
	   T_META_TAG_PERF, T_META_ENVVAR("MallocNanoZone=0"))
{
	realloc_tests("NoNano", false);
}

T_DECL(realloc_perf_nanov1, "realloc with nanoV1",
	   T_META_TAG_PERF, T_META_ENVVAR("MallocNanoZone=V1"))
{
#if CONFIG_NANOZONE
	realloc_tests("Nanov1", true);
#else // CONFIG_NANOZONE
	T_SKIP("Nano allocator not configured");
#endif // CONFIG_NANOZONE
}

T_DECL(realloc_perf_nanov2, "realloc with nanoV2",
	   T_META_TAG_PERF, T_META_ENVVAR("MallocNanoZone=V2"))
{
#if CONFIG_NANOZONE
	realloc_tests("Nanov2", true);
#else // CONFIG_NANOZONE
	T_SKIP("Nano allocator not configured");
#endif // CONFIG_NANOZONE
}
