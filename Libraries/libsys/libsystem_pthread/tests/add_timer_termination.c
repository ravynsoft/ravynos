#include <assert.h>
#include <errno.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <sys/qos.h>
#include <mach/mach_time.h>

#include <pthread.h>
#include <pthread/tsd_private.h>
#include <pthread/qos_private.h>
#include <pthread/workqueue_private.h>

#include <dispatch/dispatch.h>

#include "darwintest_defaults.h"
#include <darwintest_utils.h>

extern void __exit(int) __attribute__((noreturn));

static void __attribute__((noreturn))
run_add_timer_termination(void)
{
	const int SOURCES = 32;
	static unsigned int time_to_sleep; time_to_sleep = (unsigned int)(arc4random() % 5000 + 500);

	static int pipes[SOURCES][2];
	static dispatch_source_t s[SOURCES];
	for (int i = 0; i < SOURCES; i++) {
		pipe(pipes[i]);
		s[i] = dispatch_source_create(DISPATCH_SOURCE_TYPE_READ, (uintptr_t)pipes[i][0], 0, NULL);
		dispatch_source_set_event_handler(s[i], ^{
			while(1) pause();
		});
		dispatch_resume(s[i]);
	}

	dispatch_async(dispatch_get_global_queue(0,0), ^{
		for (int i = 1; i < SOURCES; i++){
			write(pipes[i][1], &SOURCES, 1);
			usleep(1);
		}
		while(1) pause();
	});

	usleep(time_to_sleep);
	__exit(0);
}

T_DECL(add_timer_termination, "termination during add timer",
		T_META_CHECK_LEAKS(NO))
{
	const int ROUNDS = 128;
	const int TIMEOUT = 5;
	for (int i = 0; i < ROUNDS; i++){
		pid_t pid = fork();
		T_QUIET; T_ASSERT_POSIX_SUCCESS(pid, "fork");
		if (pid == 0) { // child
			run_add_timer_termination();
		} else { // parent
			bool success = dt_waitpid(pid, NULL, NULL, TIMEOUT);
			T_ASSERT_TRUE(success, "Child %d exits successfully", i);
		}
	}
}
