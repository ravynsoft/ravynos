#include <darwintest.h>
#include <sys/types.h>
#include <pthread.h>
#include <mach/mach_types.h>
#include <unistd.h>
#include <stdlib.h>
#include <dispatch/dispatch.h>

typedef enum { PTHREAD, WORKQUEUE } thread_type_t;

typedef enum {
	NO_CORRUPTION,
	SIG_CORRUPTION,
	FULL_CORRUPTION,
} corrupt_type_t;

static void *
body(void *ctx)
{
	corrupt_type_t corrupt_type = (corrupt_type_t)ctx;
	pthread_t self = pthread_self();

	T_LOG("Helper thread running: %d", corrupt_type);

	// The pthread_t is stored at the top of the stack and could be
	// corrupted because of a stack overflow. To make the test more
	// reliable, we will manually smash the pthread struct directly.
	switch (corrupt_type) {
	case NO_CORRUPTION:
		break;
	case SIG_CORRUPTION:
		memset(self, 0x41, 128);
		break;
	case FULL_CORRUPTION: /* includes TSD */
		memset(self, 0x41, 4096);
		break;
	}

	// Expected behavior is that if a thread calls abort, the process should
	// abort promptly.
	abort();
	T_FAIL("Abort didn't?");
}

static void
abort_test(thread_type_t type, corrupt_type_t corrupt_type)
{
	pid_t child = fork();

	if (child == 0) {
		T_LOG("Child running");
		switch (type) {
		case PTHREAD: {
			pthread_t tid;
			T_QUIET;
			T_ASSERT_POSIX_ZERO(
					pthread_create(&tid, NULL, body, (void *)corrupt_type), NULL);
			break;
		}
		case WORKQUEUE: {
			dispatch_async_f(dispatch_get_global_queue(
									 DISPATCH_QUEUE_PRIORITY_DEFAULT, 0),
					(void *)corrupt_type, &body);
			break;
		}
		}
		sleep(5);
		T_FAIL("Child didn't abort");
		exit(-1);
	}

	// Wait and check the exit status of the child
	int status = 0;
	pid_t pid = wait(&status);
	T_QUIET;
	T_ASSERT_EQ(pid, child, NULL);
	T_QUIET;
	T_EXPECT_FALSE(WIFEXITED(status), "WIFEXITED Status: %x", status);
	T_QUIET;
	T_EXPECT_TRUE(WIFSIGNALED(status), "WIFSIGNALED Status: %x", status);
	T_QUIET;
	T_EXPECT_FALSE(WIFSTOPPED(status), "WIFSTOPPED Status: %x", status);

	// This test is successful if we trigger a SIGSEGV|SIGBUS or SIGABRT
	// since both will promptly terminate the program
	int signal = WTERMSIG(status);

#if defined(__i386__) || defined(__x86_64__)
	// on intel pthread_self() reads a TSD so FULL corruption results
	// in SIGSEGV/SIGBUS
	if (corrupt_type == FULL_CORRUPTION) {
		// any of these signals may happen depending on which libpthread
		// you're running on.
		if (signal == SIGBUS) {
			T_LOG("Converting %d to SIGSEGV", signal);
			signal = SIGSEGV;
		}
		T_EXPECT_EQ(signal, SIGSEGV, NULL);
		T_END;
	}
#endif

	/* pthread calls abort_with_reason if only the signature is corrupt */
	T_EXPECT_EQ(signal, SIGABRT, NULL);
}

static void
signal_handler(int signo)
{
	// The user's signal handler should not be called during abort
	T_FAIL("Unexpected signal: %d\n", signo);
}

T_DECL(abort_pthread_corrupt_test_full, "Tests abort")
{
	abort_test(PTHREAD, FULL_CORRUPTION);
}

T_DECL(abort_workqueue_corrupt_test_full, "Tests abort")
{
	abort_test(WORKQUEUE, FULL_CORRUPTION);
}

T_DECL(abort_pthread_handler_test_full, "Tests abort")
{
	// rdar://52892057
	T_SKIP("Abort hangs if the user registers their own SIGSEGV handler");
	signal(SIGSEGV, signal_handler);
	abort_test(PTHREAD, FULL_CORRUPTION);
}

T_DECL(abort_workqueue_handler_test_full, "Tests abort")
{
	// rdar://52892057
	T_SKIP("Abort hangs if the user registers their own SIGSEGV handler");
	signal(SIGSEGV, signal_handler);
	abort_test(WORKQUEUE, FULL_CORRUPTION);
}

T_DECL(abort_pthread_corrupt_test_sig, "Tests abort")
{
	abort_test(PTHREAD, SIG_CORRUPTION);
}

T_DECL(abort_workqueue_corrupt_test_sig, "Tests abort")
{
	abort_test(WORKQUEUE, SIG_CORRUPTION);
}

T_DECL(abort_pthread_test, "Tests abort")
{
	abort_test(PTHREAD, NO_CORRUPTION);
}

T_DECL(abort_workqueue_test, "Tests abort")
{
	abort_test(WORKQUEUE, NO_CORRUPTION);
}
