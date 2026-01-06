#include <pthread.h>
#include <pthread/private.h>
#include <dispatch/dispatch.h>
#include <sys/wait.h>
#include <stdlib.h>

#include "darwintest_defaults.h"

extern __uint64_t __thread_selfid( void );

static void *do_test(void * arg)
{
	uint64_t threadid = __thread_selfid();
	T_ASSERT_NE(threadid, (uint64_t)0, "__thread_selfid()");

	uint64_t pth_threadid = 0;
	T_ASSERT_POSIX_ZERO(pthread_threadid_np(NULL, &pth_threadid), NULL);
	T_ASSERT_POSIX_ZERO(pthread_threadid_np(pthread_self(), &pth_threadid), NULL);
	T_EXPECT_EQ(threadid, pth_threadid, "pthread_threadid_np()");

	pth_threadid = _pthread_threadid_self_np_direct();
	T_EXPECT_EQ(threadid, pth_threadid, "pthread_threadid_np_direct()");

	if (arg) {
		*(uint64_t *)arg = pth_threadid;
	}
	return NULL;
}

T_DECL(pthread_threadid_np, "pthread_threadid_np",
	   T_META_ALL_VALID_ARCHS(YES))
{
	T_LOG("Main Thread");
	do_test(NULL);

	T_LOG("Pthread");
	for (int i = 0; i < 100; i++) {
		uint64_t tid1 = 0, tid2 = 0;
		pthread_t pth;
		T_ASSERT_POSIX_ZERO(pthread_create(&pth, NULL, do_test, &tid1), NULL);
		T_EXPECT_POSIX_ZERO(pthread_threadid_np(pth, &tid2), NULL);
		T_ASSERT_POSIX_ZERO(pthread_join(pth, NULL), NULL);
		T_EXPECT_EQ(tid1, tid2, "parent and child agree");
	}

	T_LOG("Workqueue Thread");
	dispatch_queue_t dq = dispatch_queue_create("myqueue", NULL);
	dispatch_async(dq, ^{ do_test(NULL); });
	dispatch_sync(dq, ^{});

	T_LOG("Workqueue Thread Reuse");
	dispatch_async(dq, ^{ do_test(NULL); });
	dispatch_sync(dq, ^{});
}

T_DECL(pthread_threadid_fork, "pthread_threadid_np post-fork test")
{
	uint64_t tid = __thread_selfid();
	T_ASSERT_NE(tid, (uint64_t)0, "__thread_selfid()");

	uint64_t ptid = 0;
	T_ASSERT_POSIX_ZERO(pthread_threadid_np(NULL, &ptid), NULL);
	T_ASSERT_EQ(ptid, tid, NULL);

	pid_t pid = fork();
	if (pid == 0) {
		// child
		uint64_t ntid = __thread_selfid();
		if (ntid == tid) {
			T_LOG("FAIL: forked child tid is equal to parent tid");
			exit(1);
		}

		uint64_t nptid = 0;
		if (pthread_threadid_np(NULL, &nptid) != 0) {
			T_LOG("FAIL: pthread_threadid_np: %d", errno);
			exit(1);
		}

		if (nptid != ntid) {
			T_LOG("FAIL: pthread_threadid_np == tid (expected: %lld == %lld)",
					nptid, ntid);
			exit(1);
		}
		exit(0);
	} else {
		int status;
		T_ASSERT_EQ(waitpid(pid, &status, 0), pid, NULL);
		int exitstatus = WEXITSTATUS(status);
		T_ASSERT_EQ(exitstatus, 0, NULL);
	}
}
