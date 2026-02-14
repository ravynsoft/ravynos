#include <stdio.h>
#include <stdlib.h>

#include <darwintest.h>
#include <darwintest_perf.h>

static void
perf_fixed_size(size_t size_per_write)
{
	dt_stat_time_t latency = dt_stat_time_create("write_latency",
			"adding %zu bytes to a memstream", size_per_write);
	dt_stat_set_variable_d(latency, "bytes", (double)size_per_write);
	char *src = calloc(1, size_per_write);
	T_QUIET; T_ASSERT_NOTNULL(src, "allocated source buffer");

	while (!dt_stat_stable(latency)) {
		char *buf = NULL;
		size_t size = 0;

		FILE *victim = open_memstream(&buf, &size);
		T_QUIET; T_WITH_ERRNO; T_ASSERT_NOTNULL(victim, "opened memstream");

		T_STAT_MEASURE_BATCH(latency) {
			(void)fwrite(src, size_per_write, 1, victim);
		}

		fclose(victim);
		T_QUIET; T_ASSERT_NOTNULL(buf, "buffer was set by open_memstream");
		T_QUIET; T_ASSERT_GE(size, size_per_write,
				"memstream added non-zero bytes");

		free(buf);
	}

	free(src);
	dt_stat_finalize(latency);
}

T_DECL(perf_open_memstream, "measure the performance of open_memstream")
{
	size_t sizes[] = { 1, 8, 16, 64, 1024, 2048, 4096, 16 * 1024 };
	for (size_t i = 0; i < sizeof(sizes) / sizeof(sizes[0]); i++) {
		perf_fixed_size(sizes[i]);
	}
}
