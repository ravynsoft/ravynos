#include <os/assumes.h>

#include <darwintest.h>

void os_crash_function(const char *message);

static const char *expected_message = NULL;

void os_crash_function(const char *message) {
	if (expected_message) {
		T_ASSERT_EQ_STR(message, expected_message, NULL);
		T_END;
	} else {
		T_PASS("Got crash message: %s", message);
		T_END;
	}
}

T_DECL(os_crash_sanity_legacy, "sanity check for os_crash")
{
	expected_message = "My AWESOME assertion message.";
	os_crash(expected_message);
}
