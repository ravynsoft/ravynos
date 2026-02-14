#include <darwintest.h>
#include <dlfcn.h>
#include <pthread.h>
#include <stdlib.h>

T_GLOBAL_META(T_META_RUN_CONCURRENTLY(true));

T_DECL(tsan_sanity, "TSan Sanity Check", T_META_CHECK_LEAKS(NO))
{
	void *tsan_dylib = dlopen("@rpath/libclang_rt.tsan_osx_dynamic.dylib", RTLD_NOLOAD);
	T_ASSERT_NOTNULL(tsan_dylib, "TSan dylib loaded");

	void *ptr = malloc(16);
	free(ptr);
		
	T_PASS("I didn't crash!");
}

typedef unsigned long long invisible_barrier_t;
void __tsan_testonly_barrier_init(invisible_barrier_t *barrier, unsigned count);
void __tsan_testonly_barrier_wait(invisible_barrier_t *barrier);
int __tsan_get_report_data(void *report, const char **description, int *count,
						   int *stack_count, int *mop_count, int *loc_count,
						   int *mutex_count, int *thread_count,
						   int *unique_tid_count, void **sleep_trace,
						   unsigned long trace_size);

bool tsan_report_hit = false;
char *tsan_description = NULL;
invisible_barrier_t barrier;

const char *__tsan_default_options() {
	return "abort_on_error=0:exitcode=0";
}

void __tsan_on_report(void *report) {
	tsan_report_hit = true;

	const char *description;
	int count;
	int stack_count, mop_count, loc_count, mutex_count, thread_count,
	unique_tid_count;
	void *sleep_trace[16] = {0};
	__tsan_get_report_data(report, &description, &count, &stack_count, &mop_count,
						   &loc_count, &mutex_count, &thread_count,
						   &unique_tid_count, sleep_trace, 16);
	tsan_description = strdup(description);
}

void *thread1(void *arg) {
	__tsan_testonly_barrier_wait(&barrier);
	*((long *)arg) = 42;
	return NULL;
}

void *thread2(void *arg) {
	*((long *)arg) = 43;
	__tsan_testonly_barrier_wait(&barrier);
	return NULL;
}

T_DECL(tsan_data_race_stack, "TSan Detects data-race on stack", T_META_CHECK_LEAKS(NO))
{
	tsan_description = NULL;
	tsan_report_hit = false;
	__tsan_testonly_barrier_init(&barrier, 2);

	long racy_stack_value = 0;
	pthread_t t1;
	pthread_create(&t1, NULL, thread1, &racy_stack_value);
	pthread_t t2;
	pthread_create(&t2, NULL, thread2, &racy_stack_value);
	pthread_join(t2, NULL);
	pthread_join(t1, NULL);

	T_EXPECT_EQ(tsan_report_hit, true, "tsan finds data-race");
	T_EXPECT_NOTNULL(strstr(tsan_description, "data-race"), "tsan header");
}

T_DECL(tsan_data_race_heap, "TSan Detects data-race on heap", T_META_CHECK_LEAKS(NO))
{
	tsan_description = NULL;
	tsan_report_hit = false;
	__tsan_testonly_barrier_init(&barrier, 2);

	long *racy_heap_value = malloc(sizeof(long));
	pthread_t t1;
	pthread_create(&t1, NULL, thread1, racy_heap_value);
	pthread_t t2;
	pthread_create(&t2, NULL, thread2, racy_heap_value);
	pthread_join(t2, NULL);
	pthread_join(t1, NULL);

	T_EXPECT_EQ(tsan_report_hit, true, "tsan finds data-race");
	T_EXPECT_NOTNULL(strstr(tsan_description, "data-race"), "tsan header");
}
