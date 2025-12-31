#include <stdlib.h>
#include <notify.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <darwintest.h>
#include <signal.h>
#include "../libnotify.h"

#define T_ASSERT_GROUP_WAIT(g, timeout, ...)  ({ \
		dispatch_time_t _timeout = dispatch_time(DISPATCH_TIME_NOW, \
				(uint64_t)(timeout * NSEC_PER_SEC)); \
		T_ASSERT_FALSE(dispatch_group_wait(g, _timeout), __VA_ARGS__); \
	})

static bool has_port_been_notified;
static int m_token;

static void port_handler(mach_port_t port)
{
	int tid;
	mach_msg_empty_rcv_t msg;
	kern_return_t status;

	if (port == MACH_PORT_NULL) return;

	memset(&msg, 0, sizeof(msg));
	status = mach_msg(&msg.header, MACH_RCV_MSG, 0, sizeof(msg), port, 0, MACH_PORT_NULL);
	if (status != KERN_SUCCESS) return;

	tid = msg.header.msgh_id;

	T_ASSERT_EQ(m_token, tid, "port_handler should be called with mach_port token");

	has_port_been_notified = true;

}

T_DECL(notify_regenerate, "Make sure regenerate registrations works",
		T_META("owner", "Core Darwin Daemons & Tools"),
		T_META_ASROOT(YES))
{
	const char *KEY = "com.apple.notify.test.regenerate";
	int v_token, n_token, rc;
	pid_t old_pid, new_pid;
	uint64_t state;
	mach_port_t watch_port;
	dispatch_source_t port_src;

	dispatch_queue_t watch_queue = dispatch_queue_create("Watch Q", NULL);
	dispatch_queue_t dq = dispatch_queue_create_with_target("q", NULL, NULL);
	dispatch_group_t dg = dispatch_group_create();

	T_LOG("Grab the current instance pid & version");
	{
		rc = notify_register_check(NOTIFY_IPC_VERSION_NAME, &v_token);
		T_ASSERT_EQ(rc, NOTIFY_STATUS_OK, "register_check(NOTIFY_IPC_VERSION_NAME)");

		state = ~0ull;
		rc = notify_get_state(v_token, &state);
		T_ASSERT_EQ(rc, NOTIFY_STATUS_OK, "notify_get_state(NOTIFY_IPC_VERSION_NAME)");

		old_pid = (pid_t)(state >> 32);
		T_EXPECT_EQ((uint32_t)state, NOTIFY_IPC_VERSION, "IPC version should be set");
	}

	T_LOG("Register for our test topic, and check it works");
	{
		rc = notify_register_mach_port(KEY, &watch_port, 0, &m_token);
		T_ASSERT_EQ(rc, NOTIFY_STATUS_OK, "register mach port should work");

		port_src = dispatch_source_create(DISPATCH_SOURCE_TYPE_MACH_RECV, watch_port, 0, watch_queue);
		dispatch_source_set_event_handler(port_src, ^{
			port_handler(watch_port);
		});
		dispatch_activate(port_src);


		rc = notify_register_dispatch(KEY, &n_token, dq, ^(int token){
			// if we crash here, it means we got a spurious post
			// e.g. if you run the test twice concurrently this could happen
			dispatch_group_leave(dg);
		});
		T_ASSERT_EQ(rc, NOTIFY_STATUS_OK, "register dispatch should work");

		dispatch_group_enter(dg);
		has_port_been_notified = false;
		notify_post(KEY);
		sleep(1);
		T_ASSERT_GROUP_WAIT(dg, 5., "we received our own notification");
		T_ASSERT_EQ(has_port_been_notified, true, "mach port should receive notification");
	}

	T_LOG("Make sure notifyd changes pid due to a kill");
	{
		state = ~0ull;
		rc = notify_get_state(v_token, &state);
		T_ASSERT_EQ(rc, NOTIFY_STATUS_OK, "notify_get_state(NOTIFY_IPC_VERSION_NAME)");

		new_pid = (pid_t)(state >> 32);
		T_ASSERT_EQ(old_pid, new_pid, "Pid should not have changed yet");

		rc = kill(old_pid, SIGKILL);
		T_ASSERT_POSIX_SUCCESS(rc, "Killing notifyd");

		new_pid = old_pid;
		for (int i = 0; i < 10 && new_pid == old_pid; i++) {
			usleep(100000); /* wait for .1s for notifyd to die */
			state = ~0ull;
			rc = notify_get_state(v_token, &state);
			T_ASSERT_EQ(rc, NOTIFY_STATUS_OK, "notify_get_state(NOTIFY_IPC_VERSION_NAME)");
			new_pid = (pid_t)(state >> 32);
		}
		T_ASSERT_NE(old_pid, new_pid, "Pid should have changed within 1s");
	}

	T_LOG("Make sure our old registration works");
	{
		dispatch_group_enter(dg);
		has_port_been_notified = false;
		notify_post(KEY);
		sleep(1);
		T_ASSERT_GROUP_WAIT(dg, 5., "we received our own notification");
		T_ASSERT_EQ(has_port_been_notified, true, "mach port should receive notification");
	}

	dispatch_release(port_src);

}
