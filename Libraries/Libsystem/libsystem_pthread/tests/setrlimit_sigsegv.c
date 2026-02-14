#include "darwintest_defaults.h"
#include <spawn.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/resource.h>


extern char **environ;

T_DECL(setrlimit_overflow_segfault,
		"sigsegv is sent when stack(limit set with setrlimit) is fully used.",
		T_META_IGNORECRASHES(".*stackoverflow_crash.*"),
		T_META_CHECK_LEAKS(NO),
		T_META_ALL_VALID_ARCHS(YES),
		T_META_ASROOT(YES))
{
	pid_t child_pid = 0;
	int rv = 0;

	struct rlimit lim, save;
	T_ASSERT_POSIX_SUCCESS(getrlimit(RLIMIT_STACK, &save), NULL);
	T_ASSERT_POSIX_SUCCESS(getrlimit(RLIMIT_STACK, &lim), NULL);
	T_LOG("parent: stack limits cur=%llx max=%llx", lim.rlim_cur, lim.rlim_max);
	lim.rlim_cur = lim.rlim_cur/8;
	T_ASSERT_POSIX_SUCCESS(setrlimit(RLIMIT_STACK, &lim), NULL);
	int status = 0;
	int exit_signal = 0;

	char *crash_cmd[] = { "./assets/stackoverflow_crash", NULL };
	posix_spawn_file_actions_t fact;
	posix_spawn_file_actions_init(&fact);
	T_ASSERT_POSIX_SUCCESS(posix_spawn_file_actions_addinherit_np(&fact, STDIN_FILENO), NULL);
	T_ASSERT_POSIX_SUCCESS(posix_spawn_file_actions_addinherit_np(&fact, STDOUT_FILENO), NULL);
	T_ASSERT_POSIX_SUCCESS(posix_spawn_file_actions_addinherit_np(&fact, STDERR_FILENO), NULL);
	T_LOG("spawning %s", crash_cmd[0]);
	rv = posix_spawn(&child_pid, crash_cmd[0], &fact, NULL, crash_cmd, environ);
	T_ASSERT_POSIX_SUCCESS(rv, "spawning the stackoverflow program");

	T_LOG("parent: waiting for child process with pid %d", child_pid);
	wait(&status);
	T_LOG("parent: child process exited. status=%d", WEXITSTATUS(status));


	T_ASSERT_POSIX_SUCCESS(setrlimit(RLIMIT_STACK, &save), "Restore original limtis");
	posix_spawn_file_actions_destroy(&fact);

	T_ASSERT_TRUE(WIFSIGNALED(status), "child exit with a signal");
	exit_signal = WTERMSIG(status);
	T_ASSERT_EQ(exit_signal, SIGSEGV, "child should receive SIGSEGV");

	return;
}
