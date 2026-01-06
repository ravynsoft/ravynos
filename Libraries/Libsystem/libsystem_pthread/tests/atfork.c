#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

#include "darwintest_defaults.h"

static const char ATFORK_PREPARE[] = "prepare";
static const char ATFORK_PARENT[] = "parent";
static const char ATFORK_CHILD[] = "child";

struct callback_event {
	size_t registration_idx;
	const char *type;
};

#define NUM_REGISTRATIONS ((size_t) 20)
static struct callback_event events[NUM_REGISTRATIONS * 5];
static size_t recorded_events = 0;

static void
record_callback(size_t registration_idx, const char *type)
{
	if (recorded_events == (sizeof(events) / sizeof(events[0]))) {
		return; // events array is full
	}
	struct callback_event *evt = &events[recorded_events++];
	evt->registration_idx = registration_idx;
	evt->type = type;
	T_LOG("[%d] callback: #%lu %s", getpid(), registration_idx, type);
}

#define TWENTY(X) X(0) X(1) X(2) X(3) X(4) X(5) X(6) X(7) X(8) X(9) X(10) \
		X(11) X(12) X(13) X(14) X(15) X(16) X(17) X(18) X(19)

#define DECLARE_CB(idx) \
static void cb_prepare_##idx() { record_callback(idx, ATFORK_PREPARE); } \
static void cb_parent_##idx() { record_callback(idx, ATFORK_PARENT); } \
static void cb_child_##idx() { record_callback(idx, ATFORK_CHILD); }

TWENTY(DECLARE_CB)

typedef void (*atfork_cb_t)(void);
static const atfork_cb_t callbacks[NUM_REGISTRATIONS][3] = {
	#define CB_ELEM(idx) { cb_prepare_##idx, cb_parent_##idx, cb_child_##idx },
	TWENTY(CB_ELEM)
};

static void assert_event_sequence(struct callback_event *sequence,
		const char *expected_type, size_t start_idx, size_t end_idx)
{
	while (true) {
		struct callback_event *evt = &sequence[0];
		T_QUIET; T_ASSERT_EQ(evt->type, expected_type, NULL);
		T_QUIET; T_ASSERT_EQ(evt->registration_idx, start_idx, NULL);

		if (start_idx == end_idx) {
			break;
		}
		if (start_idx < end_idx) {
			start_idx++;
		} else {
			start_idx--;
		}
		sequence++;
	}
}

static size_t inspect_event_sequence(struct callback_event *sequence,
		const char *expected_type, size_t start_idx, size_t end_idx)
{
	size_t failures = 0;
	while (true) {
		struct callback_event *evt = &sequence[0];
		if (evt->type != expected_type || evt->registration_idx != start_idx) {
			T_LOG("FAIL: expected {idx, type}: {%lu, %s}. got {%lu, %s}",
				  start_idx, expected_type, evt->registration_idx, evt->type);
			failures++;
		}
		if (start_idx == end_idx) {
			break;
		}
		if (start_idx < end_idx) {
			start_idx++;
		} else {
			start_idx--;
		}
		sequence++;
	}
	return failures;
}

T_DECL(atfork, "pthread_atfork")
{
	pid_t pid;
	int status;
	size_t failures = 0;

	for (size_t i = 0; i < NUM_REGISTRATIONS; i++) {
		T_QUIET; T_ASSERT_POSIX_ZERO(pthread_atfork(
				callbacks[i][0], callbacks[i][1], callbacks[i][2]),
				"registering callbacks with pthread_atfork()");
	}

	pid = fork(); // first level fork

	if (pid == 0) {
		// don't use ASSERTs/EXPECTs in child processes so not to confuse
		// darwintest

		pid = fork(); // second level fork
		
		if (pid < 0) {
			T_LOG("FAIL: second fork() failed");
			exit(1);
		}
		if (recorded_events != NUM_REGISTRATIONS * 4) {
			T_LOG("FAIL: unexpected # of events: %lu instead of %lu",
				  recorded_events, NUM_REGISTRATIONS * 4);
			exit(1);
		}
		failures += inspect_event_sequence(&events[2 * NUM_REGISTRATIONS],
				ATFORK_PREPARE, NUM_REGISTRATIONS - 1, 0);
		failures += inspect_event_sequence(&events[3 * NUM_REGISTRATIONS],
				(pid ? ATFORK_PARENT : ATFORK_CHILD), 0, NUM_REGISTRATIONS - 1);
		if (failures) {
			exit((int) failures);
		}

		if (pid > 0) {
			if (waitpid(pid, &status, 0) != pid) {
				T_LOG("FAIL: grandchild waitpid failed");
				exit(1);
			}
			if (WEXITSTATUS(status) != 0) {
				T_LOG("FAIL: grandchild exited with status %d",
					  WEXITSTATUS(status));
				exit(1);
			}
		}
		exit(0); // don't run leaks in the child and the grandchild
	} else {
		T_ASSERT_GE(pid, 0, "first fork()");

		T_ASSERT_EQ(recorded_events, NUM_REGISTRATIONS * 2, "# of events");
		assert_event_sequence(events, ATFORK_PREPARE, NUM_REGISTRATIONS - 1, 0);
		assert_event_sequence(&events[NUM_REGISTRATIONS],
							  ATFORK_PARENT, 0, NUM_REGISTRATIONS - 1);

		T_ASSERT_EQ(pid, waitpid(pid, &status, 0), "child waitpid");
		T_ASSERT_POSIX_ZERO(WEXITSTATUS(status), "child exit status");
	}
}
