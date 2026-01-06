#include <assert.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <dispatch/dispatch.h>
#include <sys/mman.h>
#include <stdatomic.h>

#include "darwintest_defaults.h"
#include <darwintest_multiprocess.h>
#include <darwintest_utils.h>

#define T_LOG_VERBOSE(...)

#ifndef T_MAYFAIL_WITH_REASON
#define T_MAYFAIL_WITH_REASON(x) T_MAYFAIL
#endif

#ifdef __LP64__
#define STACK_LOCATIONS 16
#else
#define STACK_LOCATIONS 8
#endif

static const int attempts = 128, attempt_rounds = 3;

static void*
thread_routine(void *loc)
{
	int foo;
	*(uintptr_t*)loc = (uintptr_t)&foo;
	return NULL;
}

static int
pointer_compare(const void *ap, const void *bp)
{
	uintptr_t a = *(const uintptr_t*)ap;
	uintptr_t b = *(const uintptr_t*)bp;
	return a > b ? 1 : a < b ? -1 : 0;
}

typedef struct shmem_s {
	_Atomic int ctr, done;
	uintptr_t addr_array[attempts];
} *shmem_t;

static shmem_t
test_shmem_open(const char* shmem_name, int creatflags)
{
	int fd = open(shmem_name, O_RDWR | creatflags, 0600);
	T_QUIET; T_ASSERT_POSIX_SUCCESS(fd, "open temp file");
	if (creatflags) {
		T_QUIET; T_ASSERT_POSIX_SUCCESS(ftruncate(fd,
				sizeof(struct shmem_s)), "resize temp file");
	}
	shmem_t shmem = mmap(NULL, sizeof(struct shmem_s),
			PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	T_QUIET; T_ASSERT_NOTNULL(shmem, "mmap shmem");
	T_QUIET; T_ASSERT_POSIX_SUCCESS(close(fd), "close temp file");
	return shmem;
}

static uintptr_t*
test_shmem_start(shmem_t shmem)
{
	int idx = atomic_fetch_add(&shmem->ctr, 1);
	return &shmem->addr_array[idx];
}

static void
test_shmem_end(shmem_t shmem)
{
	atomic_fetch_add(&shmem->done, 1);
}

T_HELPER_DECL(wq_stack_aslr_helper,
		"Confirm that workqueue stacks are ASLRed (Helper)")
{
	shmem_t shmem = test_shmem_open(argv[0], 0);
	uintptr_t *addr = test_shmem_start(shmem);
	dispatch_group_t g = dispatch_group_create();
	dispatch_group_async_f(g, dispatch_get_global_queue(0,0), addr,
			(dispatch_function_t)thread_routine);
	dispatch_group_wait(g, DISPATCH_TIME_FOREVER);
	dispatch_release(g);
	test_shmem_end(shmem);
}

T_HELPER_DECL(pthread_stack_aslr_helper,
		"Confirm that stacks are ASLRed (Helper)")
{
	shmem_t shmem = test_shmem_open(argv[0], 0);
	uintptr_t *addr = test_shmem_start(shmem);
	pthread_t th;
	int ret = pthread_create(&th, NULL, thread_routine, addr);
	assert(ret == 0);
	ret = pthread_join(th, NULL);
	assert(ret == 0);
	test_shmem_end(shmem);
}

static void
test_stack_aslr(bool workqueue_thread)
{
	const char *tmpdir = dt_tmpdir();
	char *tmp;
	asprintf(&tmp, "%s/pthread_stack_aslr_XXXXX", tmpdir);
	T_QUIET; T_ASSERT_NOTNULL(mkdtemp(tmp), "mkdtemp");

	char *shmem_name;
	asprintf(&shmem_name, "%s/shmem", tmp);
	shmem_t shmem = test_shmem_open(shmem_name, O_CREAT|O_EXCL);
	uintptr_t *addr_array = shmem->addr_array;

	dt_helper_t helpers[attempts * attempt_rounds];
	const char* helper = workqueue_thread ? "wq_stack_aslr_helper" :
			"pthread_stack_aslr_helper";
	char *helper_args[] = {shmem_name, NULL};
	size_t helper_idx = 0;

	struct rlimit l;
	if (!getrlimit(RLIMIT_NOFILE, &l)) {
		l.rlim_cur += 3 * attempts * attempt_rounds; // 3 fifos per helper
		T_QUIET; T_ASSERT_POSIX_SUCCESS(setrlimit(RLIMIT_NOFILE, &l),
				"setrlimit");
	}
	signal(SIGCHLD, SIG_IGN);

	int attempt_round = attempt_rounds;
again:
	bzero(shmem, sizeof(struct shmem_s));

	for (int i = 0; i < attempts; i++) {
		char *t;
		asprintf(&t, "%s/%d", tmp, i);
		T_QUIET; T_ASSERT_POSIX_SUCCESS(mkdir(t, 0700), "mkdir");
		setenv("BATS_TMP_DIR", t, 1); // hack to workaround rdar://33443485
		free(t);
		helpers[helper_idx++] = dt_child_helper_args(helper, helper_args);
		int w = 100;
		do {
			if (!w--) {
				T_QUIET; T_FAIL("Helper should complete in <.1s");
				goto timeout;
			}
			usleep(1000 * 100);
		} while (shmem->done <= i);
	}
	setenv("BATS_TMP_DIR", tmpdir, 1);

	qsort(addr_array, attempts, sizeof(uintptr_t), pointer_compare);
	T_LOG("Stack address range: %p - %p (+%lx)", (void*)addr_array[0],
			(void*)addr_array[attempts-1],
			addr_array[attempts-1] - addr_array[0]);

	int unique_values = 0;
	T_LOG_VERBOSE("[%p]", (void*)addr_array[0]);
	for (int i = 1; i < attempts; i++) {
		T_LOG_VERBOSE("[%p]", (void*)addr_array[i]);
		if (addr_array[i-1] != addr_array[i]) {
			unique_values++;
		}
	}

	if (--attempt_round) T_MAYFAIL_WITH_REASON("ASLR");
	T_EXPECT_GE(unique_values, STACK_LOCATIONS,
			"Should have more than %d unique stack locations", STACK_LOCATIONS);
	if (attempt_round && unique_values < STACK_LOCATIONS) goto again;

timeout:
	T_QUIET; T_EXPECT_POSIX_SUCCESS(unlink(shmem_name), "unlink temp file");
	free(shmem_name);
	free(tmp);
	dt_run_helpers(helpers, helper_idx, 5);
}

T_DECL(pthread_stack_aslr, "Confirm that stacks are ASLRed",
		T_META_CHECK_LEAKS(NO), T_META_ALL_VALID_ARCHS(YES))
{
	test_stack_aslr(false);
}

T_DECL(wq_stack_aslr, "Confirm that workqueue stacks are ASLRed",
		T_META_CHECK_LEAKS(NO), T_META_ALL_VALID_ARCHS(YES))
{
	test_stack_aslr(true);
}
