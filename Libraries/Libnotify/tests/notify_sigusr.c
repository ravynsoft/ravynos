#include <stdlib.h>
#include <notify.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <darwintest.h>
#include <signal.h>
#include "../libnotify.h"

T_DECL(notify_sigusr, "Make sure SIGUSR{1,2} dumps status",
		T_META("owner", "Core Darwin Daemons & Tools"),
		T_META_ASROOT(YES))
{
	int v_token, rc;
	pid_t pid;
	uint64_t state;

	T_LOG("Grab the current instance pid & version");
	{
		rc = notify_register_check(NOTIFY_IPC_VERSION_NAME, &v_token);
		T_ASSERT_EQ(rc, NOTIFY_STATUS_OK, "register_check(NOTIFY_IPC_VERSION_NAME)");

		state = ~0ull;
		rc = notify_get_state(v_token, &state);
		T_ASSERT_EQ(rc, NOTIFY_STATUS_OK, "notify_get_state(NOTIFY_IPC_VERSION_NAME)");

		pid = (pid_t)(state >> 32);
		T_EXPECT_EQ((uint32_t)state, NOTIFY_IPC_VERSION, "IPC version should be set");
	}

	char *status_file;
	asprintf(&status_file, "/var/run/notifyd_%u.status", pid);

	T_LOG("Make sure SIGUSR1 works");
	{
		rc = kill(pid, SIGUSR1);
		T_ASSERT_POSIX_SUCCESS(rc, "Killing notifyd");

		rc = -1;
		for (int i = 0; i < 10 && rc == -1; i++) {
			rc = unlink(status_file);
			if (rc == -1 && errno != ENOENT) {
				T_ASSERT_POSIX_SUCCESS(rc, "unlink(%s)", status_file);
			}
			usleep(100000); /* wait for .1s for notifyd to dump status */
		}
		T_ASSERT_POSIX_SUCCESS(rc, "unlink(%s)", status_file);
	}

	T_LOG("Make sure SIGUSR2 works");
	{
		rc = kill(pid, SIGUSR2);
		T_ASSERT_POSIX_SUCCESS(rc, "Killing notifyd");

		rc = -1;
		for (int i = 0; i < 10 && rc == -1; i++) {
			rc = unlink(status_file);
			if (rc == -1 && errno != ENOENT) {
				T_ASSERT_POSIX_SUCCESS(rc, "unlink(%s)", status_file);
			}
			usleep(100000); /* wait for .1s for notifyd to dump status */
		}
		T_ASSERT_POSIX_SUCCESS(rc, "unlink(%s)", status_file);
	}

	free(status_file);
}
