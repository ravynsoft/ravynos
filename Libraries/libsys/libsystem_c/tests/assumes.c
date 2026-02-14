#define OS_CRASH_ENABLE_EXPERIMENTAL_LIBTRACE 1
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

T_DECL(os_crash_sanity, "sanity check for os_crash")
{
	expected_message = "My AWESOME assertion message.";
	os_crash(expected_message);
}

extern int two;
int two = 2;

T_DECL(os_assert_no_msg, "sanity check for os_assert w/o a message")
{
	expected_message = "assertion failure: \"two + two == 5\" -> %lld";
	os_assert(two + two == 5);
}

#define DOGMA "Today, we celebrate the first glorious anniversary of the Information Purification Directives."
T_DECL(os_assert_msg, "sanity check for os_assert with a message")
{
	expected_message = "assertion failure: " DOGMA;
	os_assert(two + two == 5, DOGMA);
}
