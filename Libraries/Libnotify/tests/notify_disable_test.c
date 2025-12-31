#include <stdlib.h>
#include <stdio.h>
#include <notify.h>
#include <notify_private.h>
#include <darwintest.h>

#define KEY1 "com.apple.notify.test.disable"
#define KEY2 "com.apple.notify.test.disable.fail"

T_DECL(notify_disable_test,
       "notify disable test",
       T_META("owner", "Core Darwin Daemons & Tools"),
       T_META("as_root", "false"))
{
	int token1, token2, status, fd;
	uint64_t state;

	token1 = NOTIFY_TOKEN_INVALID;
	token2 = NOTIFY_TOKEN_INVALID;
	fd = -1;

	status = notify_register_file_descriptor(KEY1, &fd, 0, &token1);
    T_EXPECT_EQ(status, NOTIFY_STATUS_OK, "notify_register_file_descriptor %d == NOTIFY_STATUS_OK", status);
	state = 123454321;
	status = notify_set_state(token1, state);
    T_EXPECT_EQ(status, NOTIFY_STATUS_OK, "notify_set_state %d == NOTIFY_STATUS_OK", status);
	
	state = 0;
	status = notify_get_state(token1, &state);
    T_EXPECT_EQ(status, NOTIFY_STATUS_OK, "notify_get_state %d == NOTIFY_STATUS_OK", status);
    T_EXPECT_EQ(state, 123454321ULL, "notify_get_state %llu == 123454321", state);

    // Disable
    T_LOG("notify_set_options(NOTIFY_OPT_DISABLE)");
	notify_set_options(NOTIFY_OPT_DISABLE);
	
	status = notify_register_check(KEY2, &token2);
    T_EXPECT_NE(status, NOTIFY_STATUS_OK, "notify_register_check %d != NOTIFY_STATUS_OK", status);

	state = 0;
	status = notify_get_state(token1, &state);
    T_EXPECT_NE(status, NOTIFY_STATUS_OK, "notify_get_state %d != NOTIFY_STATUS_OK", status);

    // Re-enable
    T_LOG("notify_set_options(NOTIFY_OPT_ENABLE)");
	notify_set_options(NOTIFY_OPT_ENABLE);
	
	state = 0;
	status = notify_get_state(token1, &state);
    T_EXPECT_EQ(status, NOTIFY_STATUS_OK, "notify_get_state %d ==  NOTIFY_STATUS_OK", status);
    T_EXPECT_EQ(state, 123454321ULL, "notify_get_state %llu == 123454321", state);

    T_LOG("checking token validity");
    status = notify_is_valid_token(token1);
    T_EXPECT_GT(status, 0, "notify_is_valid_token(token1) > 0 (%d)", status);

    T_LOG("canceling token1");
	notify_cancel(token1);
    status = notify_is_valid_token(token1);
    T_EXPECT_EQ(status, 0, "notify_is_valid_token(token1) == 0 (%d)", status);
}
