#include <stdlib.h>
#include <notify.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <darwintest.h>

#define KEY "com.apple.notify.test.dispatch_cancel_in_block"
#define COUNT 10

T_DECL(dispatch_cancel_in_block,
       "Tests notify_cancel in dispatch block.",
       T_META("owner", "Core Darwin Daemons & Tools"),
       T_META("as_root", "false"))
{
	int i;
	int status;
    int tokens[COUNT];

	T_LOG("Creating Notify queue");
	dispatch_queue_t queue = dispatch_queue_create("Notify", NULL);

	T_LOG("Creating dispatch group");
    dispatch_group_t group = dispatch_group_create();

	for (i = 0; i < COUNT; ++i)
	{
        dispatch_group_enter(group);

        T_LOG("Registering dispatch notification listeners");
		status = notify_register_dispatch(KEY, &tokens[i], queue, ^(int x){
                int is_valid;
                int status; 

                T_LOG("Received dispatch notification: %d", x);

				status = notify_cancel(x);
                T_EXPECT_EQ(status, NOTIFY_STATUS_OK,
                        "notify_cancel %d == NOTIFY_STATUS_OK", status);    

                is_valid = notify_is_valid_token(x);
                T_EXPECT_EQ(is_valid, 0,
                        "notify_is_valid_token(%d) == %d", x, is_valid);    

                dispatch_group_leave(group);
		});

        T_EXPECT_EQ(status, NOTIFY_STATUS_OK,
                "notify_register_dispatch %d == NOTIFY_STATUS_OK", status);    
        T_EXPECT_TRUE(tokens[i] >= 0, "token %d >= 0", status);    
	}

    T_LOG("Posting notifications");
	notify_post(KEY);

    dispatch_group_notify(group, queue, ^(void) {
            printf("Notified...\n");
            T_END;
    });
	
	dispatch_release(queue);
	dispatch_main();
}
