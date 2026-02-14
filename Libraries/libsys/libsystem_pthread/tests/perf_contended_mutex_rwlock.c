#include <stdlib.h>
#include <stdio.h>
#include <stdatomic.h>
#include <math.h>
#include <unistd.h>
#include <sys/sysctl.h>
#include <mach/mach.h>
#include <pthread.h>
#include <pthread/pthread_spis.h>
#include <os/lock.h>
#include <darwintest.h>

// number of times the lock is taken per dt_stat batch
#define ITERATIONS_PER_DT_STAT_BATCH 10000ull
// number of times the contended mutex is taken per dt_stat batch
#define ITERATIONS_PER_DT_STAT_BATCH_CONTENDED_MUTEX 1000ull
// shift determining power of 2 factor of time spent by worker threads in the
// busy() function while outside of the lock vs inside the lock
#define OUTER_VS_INNER_SHIFT 4
// fraction of read lock vs write lock acquires
#define RDLOCK_FRACTION 0.99f
// maintain and print progress counters in between measurement batches
#define COUNTERS 0

// move the darwintest assertion code out of the straight line execution path
// since it is has non-trivial overhead and codegen impact even if the assertion
// is never triggered.
#define iferr(_e) if(__builtin_expect(!!(_e), 0))

#pragma mark -

uint64_t
random_busy_counts(unsigned int *seed, uint64_t *inner, uint64_t *outer)
{
	uint64_t random = rand_r(seed);
	const uint64_t of = (1 << OUTER_VS_INNER_SHIFT);
	*inner = 0x4 + (random & (0x10 - 1));
	*outer = 0x4 * of + ((random >> 4) & (0x10 * of - 1));
	return random;
}

// By default busy() does cpu busy work for a passed in number of iterations
enum {
	busy_is_nothing = 0,
	busy_is_cpu_busy,
	busy_is_cpu_yield,
};
static int busy_select = busy_is_cpu_busy;

static double
cpu_busy(uint64_t n)
{
	double d = M_PI;
	uint64_t i;
	for (i = 0; i < n; i++) d *= M_PI;
	return d;
}

static double
cpu_yield(uint64_t n)
{
	uint64_t i;
	for (i = 0; i < n; i++) {
#if defined(__arm__) || defined(__arm64__)
	asm volatile("yield");
#elif defined(__x86_64__) || defined(__i386__)
	asm volatile("pause");
#else
#error Unrecognized architecture
#endif
	}
	return 0;
}

__attribute__((noinline))
static double
busy(uint64_t n)
{
	switch(busy_select) {
	case busy_is_cpu_busy:
		return cpu_busy(n);
	case busy_is_cpu_yield:
		return cpu_yield(n);
	default:
		return 0;
	}
}

#pragma mark -

static semaphore_t ready_sem, start_sem, end_sem;
static uint32_t nthreads;
static _Atomic uint32_t active_thr;
static _Atomic int64_t todo;
uint64_t iterations_per_dt_stat_batch = ITERATIONS_PER_DT_STAT_BATCH;

#if COUNTERS
static _Atomic uint64_t total_locks, total_rdlocks, total_wrlocks;
#define ctr_inc(_t) atomic_fetch_add_explicit(&(_t), 1, memory_order_relaxed)
#else
#define ctr_inc(_t)
#endif

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

__attribute__((noinline))
static void
threaded_bench(dt_stat_time_t s, int batch_size)
{
	kern_return_t kr;
	for (int i = 0; i < nthreads; i++) {
		kr = semaphore_wait(ready_sem);
		iferr (kr) {T_QUIET; T_ASSERT_MACH_SUCCESS(kr, "semaphore_wait");}
	}
	atomic_init(&active_thr, nthreads);
	atomic_init(&todo, batch_size * iterations_per_dt_stat_batch);
	dt_stat_token t = dt_stat_begin(s);
	kr = semaphore_signal_all(start_sem);
	iferr (kr) {T_QUIET; T_ASSERT_MACH_SUCCESS(kr, "semaphore_signal_all");}
	kr = semaphore_wait(end_sem);
	iferr (kr) {T_QUIET; T_ASSERT_MACH_SUCCESS(kr, "semaphore_wait");}
	dt_stat_end_batch(s, batch_size, t);
}

static void
setup_threaded_bench(void* (*thread_fn)(void*), bool singlethreaded)
{
	kern_return_t kr;
	int r;
	char *e;

	if (singlethreaded) {
		nthreads = 1;
	} else {
		if ((e = getenv("DT_STAT_NTHREADS"))) nthreads = strtoul(e, NULL, 0);
		if (nthreads < 2) nthreads = ncpu();
	}
	if ((e = getenv("DT_STAT_CPU_BUSY"))) busy_select = strtoul(e, NULL, 0);

	kr = semaphore_create(mach_task_self(), &ready_sem, SYNC_POLICY_FIFO, 0);
	T_QUIET; T_ASSERT_MACH_SUCCESS(kr, "semaphore_create");
	kr = semaphore_create(mach_task_self(), &start_sem, SYNC_POLICY_FIFO, 0);
	T_QUIET; T_ASSERT_MACH_SUCCESS(kr, "semaphore_create");
	kr = semaphore_create(mach_task_self(), &end_sem, SYNC_POLICY_FIFO, 0);
	T_QUIET; T_ASSERT_MACH_SUCCESS(kr, "semaphore_create");

	pthread_attr_t attr;
	r = pthread_attr_init(&attr);
	T_QUIET; T_ASSERT_POSIX_ZERO(r, "pthread_attr_init");
	r = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	T_QUIET; T_ASSERT_POSIX_ZERO(r, "pthread_attr_setdetachstate");

	for (int i = 0; i < nthreads; i++) {
		pthread_t th;
		r = pthread_create(&th, &attr, thread_fn, (void *)(uintptr_t)(i+1));
		T_QUIET; T_ASSERT_POSIX_ZERO(r, "pthread_create");
	}
}

#pragma mark -

static pthread_mutex_t mutex;

static void *
mutex_bench_thread(void * arg)
{
	kern_return_t kr;
	int r;
	unsigned int seed;
	volatile double dummy;

restart:
	seed = (uintptr_t)arg; // each thread repeats its own sequence
	kr = semaphore_wait_signal(start_sem, ready_sem);
	T_QUIET; T_ASSERT_MACH_SUCCESS(kr, "semaphore_wait_signal");

	while (atomic_fetch_sub_explicit(&todo, 1, memory_order_relaxed) > 0) {
		uint64_t inner, outer;
		random_busy_counts(&seed, &inner, &outer);
		dummy = busy(outer);
		r = pthread_mutex_lock(&mutex);
		iferr (r) {T_QUIET; T_ASSERT_POSIX_ZERO(r, "mutex_lock");}
		dummy = busy(inner);
		r = pthread_mutex_unlock(&mutex);
		iferr (r) {T_QUIET; T_ASSERT_POSIX_ZERO(r, "mutex_unlock");}
		ctr_inc(total_locks);
	}

	if (atomic_fetch_sub_explicit(&active_thr, 1, memory_order_relaxed) == 1) {
		kr = semaphore_signal(end_sem);
		T_QUIET; T_ASSERT_MACH_SUCCESS(kr, "semaphore_signal");
	}
	goto restart;
}

static void
mutex_bench(bool singlethreaded)
{
	int r;
	int batch_size;
#if COUNTERS
	uint64_t batch = 0;
#endif

	setup_threaded_bench(mutex_bench_thread, singlethreaded);

	pthread_mutexattr_t attr;
	r = pthread_mutexattr_init(&attr);
	T_QUIET; T_ASSERT_POSIX_ZERO(r, "mutexattr_init");
	pthread_mutexattr_setpolicy_np(&attr, _PTHREAD_MUTEX_POLICY_FAIRSHARE);
	T_QUIET; T_ASSERT_POSIX_ZERO(r, "mutexattr_setpolicy_np");
	r = pthread_mutex_init(&mutex, &attr);
	T_QUIET; T_ASSERT_POSIX_ZERO(r, "mutex_init");

	dt_stat_time_t s = dt_stat_time_create("%llu pthread_mutex_lock & "
			"pthread_mutex_unlock (fairshare) on %u thread%s",
			iterations_per_dt_stat_batch, nthreads, nthreads > 1 ? "s" : "");
	do {
		batch_size = dt_stat_batch_size(s);
		threaded_bench(s, batch_size);
#if COUNTERS
		fprintf(stderr, "\rbatch: %4llu\t size: %4d\tmutexes: %8llu",
				++batch, batch_size,
				atomic_load_explicit(&total_locks, memory_order_relaxed));
#endif
	} while (!dt_stat_stable(s));
#if COUNTERS
	fprintf(stderr, "\n");
#endif
	dt_stat_finalize(s);
}

T_DECL(perf_uncontended_mutex_bench, "Uncontended fairshare mutex",
		T_META_TYPE_PERF, T_META_ALL_VALID_ARCHS(NO),
		T_META_LTEPHASE(LTE_POSTINIT), T_META_CHECK_LEAKS(false))
{
	mutex_bench(true);
}

T_DECL(perf_contended_mutex_bench, "Contended fairshare mutex",
		T_META_TYPE_PERF, T_META_ALL_VALID_ARCHS(NO),
		T_META_LTEPHASE(LTE_POSTINIT), T_META_CHECK_LEAKS(false))
{
	iterations_per_dt_stat_batch = ITERATIONS_PER_DT_STAT_BATCH_CONTENDED_MUTEX;
	mutex_bench(false);
}

#pragma mark -

static pthread_rwlock_t rwlock;

static void *
rwlock_bench_thread(void * arg)
{
	kern_return_t kr;
	int r;
	unsigned int seed;
	volatile double dummy;
	const uint64_t rand_rdlock_max = (double)RAND_MAX * RDLOCK_FRACTION;

restart:
	seed = (uintptr_t)arg; // each thread repeats its own sequence
	kr = semaphore_wait_signal(start_sem, ready_sem);
	T_QUIET; T_ASSERT_MACH_SUCCESS(kr, "semaphore_wait_signal");

	while (atomic_fetch_sub_explicit(&todo, 1, memory_order_relaxed) > 0) {
		uint64_t inner, outer;
		uint64_t random = random_busy_counts(&seed, &inner, &outer);
		dummy = busy(outer);
		if (random < rand_rdlock_max) {
			r = pthread_rwlock_rdlock(&rwlock);
			iferr (r) {T_QUIET; T_ASSERT_POSIX_ZERO(r, "rwlock_rdlock");}
			dummy = busy(inner);
			r = pthread_rwlock_unlock(&rwlock);
			iferr (r) {T_QUIET; T_ASSERT_POSIX_ZERO(r, "rwlock_unlock");}
			ctr_inc(total_rdlocks);
		} else {
			r = pthread_rwlock_wrlock(&rwlock);
			iferr (r) {T_QUIET; T_ASSERT_POSIX_ZERO(r, "rwlock_wrlock");}
			dummy = busy(inner);
			r = pthread_rwlock_unlock(&rwlock);
			iferr (r) {T_QUIET; T_ASSERT_POSIX_ZERO(r, "rwlock_unlock");}
			ctr_inc(total_wrlocks);
		}
		ctr_inc(total_locks);
	}

	if (atomic_fetch_sub_explicit(&active_thr, 1, memory_order_relaxed) == 1) {
		kr = semaphore_signal(end_sem);
		T_QUIET; T_ASSERT_MACH_SUCCESS(kr, "semaphore_signal");
	}
	goto restart;
}

static void
rwlock_bench(bool singlethreaded)
{
	int r;
	int batch_size;
#if COUNTERS
	uint64_t batch = 0;
#endif

	setup_threaded_bench(rwlock_bench_thread, singlethreaded);

	r = pthread_rwlock_init(&rwlock, NULL);
	T_QUIET; T_ASSERT_POSIX_ZERO(r, "rwlock_init");

	dt_stat_time_t s = dt_stat_time_create("%llu pthread_rwlock_rd/wrlock & "
			"pthread_rwlock_unlock (%.0f%% rdlock) on %u thread%s",
			iterations_per_dt_stat_batch, RDLOCK_FRACTION * 100, nthreads,
			nthreads > 1 ? "s" : "");
	do {
		batch_size = dt_stat_batch_size(s);
		threaded_bench(s, batch_size);
#if COUNTERS
		fprintf(stderr, "\rbatch: %4llu\t size: %4d\trwlocks: %8llu\t"
				"rd: %8llu\twr: %8llu", ++batch, batch_size,
				atomic_load_explicit(&total_locks,   memory_order_relaxed),
				atomic_load_explicit(&total_rdlocks, memory_order_relaxed),
				atomic_load_explicit(&total_wrlocks, memory_order_relaxed));
#endif
	} while (!dt_stat_stable(s));
#if COUNTERS
	fprintf(stderr, "\n");
#endif
	dt_stat_finalize(s);
}

T_DECL(perf_uncontended_rwlock_bench, "Uncontended rwlock",
		T_META_TYPE_PERF, T_META_ALL_VALID_ARCHS(NO),
		T_META_LTEPHASE(LTE_POSTINIT), T_META_CHECK_LEAKS(false))
{
	rwlock_bench(true);
}

T_DECL(perf_contended_rwlock_bench, "Contended rwlock",
		T_META_TYPE_PERF, T_META_ALL_VALID_ARCHS(NO),
		T_META_LTEPHASE(LTE_POSTINIT), T_META_CHECK_LEAKS(false))
{
	rwlock_bench(false);
}

#pragma mark -

static os_unfair_lock unfair_lock;

static void *
unfair_lock_bench_thread(void * arg)
{
	kern_return_t kr;
	unsigned int seed;
	volatile double dummy;

restart:
	seed = (uintptr_t)arg; // each thread repeats its own sequence
	kr = semaphore_wait_signal(start_sem, ready_sem);
	T_QUIET; T_ASSERT_MACH_SUCCESS(kr, "semaphore_wait_signal");

	while (atomic_fetch_sub_explicit(&todo, 1, memory_order_relaxed) > 0) {
		uint64_t inner, outer;
		random_busy_counts(&seed, &inner, &outer);
		dummy = busy(outer);
		os_unfair_lock_lock(&unfair_lock);
		dummy = busy(inner);
		os_unfair_lock_unlock(&unfair_lock);
		ctr_inc(total_locks);
	}

	if (atomic_fetch_sub_explicit(&active_thr, 1, memory_order_relaxed) == 1) {
		kr = semaphore_signal(end_sem);
		T_QUIET; T_ASSERT_MACH_SUCCESS(kr, "semaphore_signal");
	}
	goto restart;
}

static void
unfair_lock_bench(bool singlethreaded)
{
	int r;
	int batch_size;
#if COUNTERS
	uint64_t batch = 0;
#endif

	setup_threaded_bench(unfair_lock_bench_thread, singlethreaded);

	dt_stat_time_t s = dt_stat_time_create("%llu os_unfair_lock_lock & "
			"os_unfair_lock_unlock on %u thread%s",
			iterations_per_dt_stat_batch, nthreads, nthreads > 1 ? "s" : "");
	do {
		batch_size = dt_stat_batch_size(s);
		threaded_bench(s, batch_size);
#if COUNTERS
		fprintf(stderr, "\rbatch: %4llu\t size: %4d\tunfair_locks: %8llu",
				++batch, batch_size,
				atomic_load_explicit(&total_locks, memory_order_relaxed));
#endif
	} while (!dt_stat_stable(s));
#if COUNTERS
	fprintf(stderr, "\n");
#endif
	dt_stat_finalize(s);
}

T_DECL(perf_uncontended_unfair_lock_bench, "Unontended unfair lock",
		T_META_TYPE_PERF, T_META_ALL_VALID_ARCHS(NO),
		T_META_LTEPHASE(LTE_POSTINIT), T_META_CHECK_LEAKS(false))
{
	unfair_lock_bench(true);
}

T_DECL(perf_contended_unfair_lock_bench, "Contended unfair lock",
		T_META_TYPE_PERF, T_META_ALL_VALID_ARCHS(NO),
		T_META_LTEPHASE(LTE_POSTINIT), T_META_CHECK_LEAKS(false))
{
	unfair_lock_bench(false);
}

#pragma mark -

static pthread_mutex_t ffmutex;

static void *
ffmutex_bench_thread(void * arg)
{
	kern_return_t kr;
	int r;
	unsigned int seed;
	volatile double dummy;

restart:
	seed = (uintptr_t)arg; // each thread repeats its own sequence
	kr = semaphore_wait_signal(start_sem, ready_sem);
	T_QUIET; T_ASSERT_MACH_SUCCESS(kr, "semaphore_wait_signal");

	while (atomic_fetch_sub_explicit(&todo, 1, memory_order_relaxed) > 0) {
		uint64_t inner, outer;
		random_busy_counts(&seed, &inner, &outer);
		dummy = busy(outer);
		r = pthread_mutex_lock(&ffmutex);
		iferr (r) {T_QUIET; T_ASSERT_POSIX_ZERO(r, "mutex_lock");}
		dummy = busy(inner);
		r = pthread_mutex_unlock(&ffmutex);
		iferr (r) {T_QUIET; T_ASSERT_POSIX_ZERO(r, "mutex_unlock");}
		ctr_inc(total_locks);
	}

	if (atomic_fetch_sub_explicit(&active_thr, 1, memory_order_relaxed) == 1) {
		kr = semaphore_signal(end_sem);
		T_QUIET; T_ASSERT_MACH_SUCCESS(kr, "semaphore_signal");
	}
	goto restart;
}

static void
ffmutex_bench(bool singlethreaded)
{
	int r;
	int batch_size;
#if COUNTERS
	uint64_t batch = 0;
#endif

	setup_threaded_bench(ffmutex_bench_thread, singlethreaded);

	pthread_mutexattr_t attr;
	r = pthread_mutexattr_init(&attr);
	T_QUIET; T_ASSERT_POSIX_ZERO(r, "mutexattr_init");
	pthread_mutexattr_setpolicy_np(&attr, _PTHREAD_MUTEX_POLICY_FIRSTFIT);
	T_QUIET; T_ASSERT_POSIX_ZERO(r, "mutexattr_setpolicy_np");
	r = pthread_mutex_init(&ffmutex, &attr);
	T_QUIET; T_ASSERT_POSIX_ZERO(r, "mutex_init");

	dt_stat_time_t s = dt_stat_time_create("%llu pthread_mutex_lock & "
			"pthread_mutex_unlock (first-fit) on %u thread%s",
			iterations_per_dt_stat_batch, nthreads, nthreads > 1 ? "s" : "");
	do {
		batch_size = dt_stat_batch_size(s);
		threaded_bench(s, batch_size);
#if COUNTERS
		fprintf(stderr, "\rbatch: %4llu\t size: %4d\tffmutexes: %8llu",
				++batch, batch_size,
				atomic_load_explicit(&total_locks, memory_order_relaxed));
#endif
	} while (!dt_stat_stable(s));
#if COUNTERS
	fprintf(stderr, "\n");
#endif
	dt_stat_finalize(s);
}

T_DECL(perf_uncontended_ffmutex_bench, "Uncontended first-fit mutex",
		T_META_TYPE_PERF, T_META_ALL_VALID_ARCHS(NO),
		T_META_LTEPHASE(LTE_POSTINIT), T_META_CHECK_LEAKS(false))
{
	ffmutex_bench(true);
}

T_DECL(perf_contended_ffmutex_bench, "Contended first-fit mutex",
		T_META_TYPE_PERF, T_META_ALL_VALID_ARCHS(NO),
		T_META_LTEPHASE(LTE_POSTINIT), T_META_CHECK_LEAKS(false))
{
	ffmutex_bench(false);
}
