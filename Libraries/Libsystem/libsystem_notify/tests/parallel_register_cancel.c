//
//  parallel_register_cancel.c
//  Libnotify
//

#include <darwintest.h>
#include <dispatch/dispatch.h>
#include <notify.h>
#include <stdio.h>

T_DECL(parallel_register_cancel,
       "parallel register/cancel test",
       T_META("owner", "Core Darwin Daemons & Tools"),
       T_META("as_root", "false"))
{
	dispatch_queue_t noteQueue = dispatch_queue_create("noteQ", DISPATCH_QUEUE_SERIAL_WITH_AUTORELEASE_POOL);
	static int tokens[100000];
	dispatch_apply(100000, DISPATCH_APPLY_AUTO, ^(size_t i) {
        assert(notify_register_check("com.example.test", &tokens[i]) == NOTIFY_STATUS_OK);
        assert(notify_cancel(tokens[i]) == NOTIFY_STATUS_OK);
		assert(notify_register_dispatch("com.example.test", &tokens[i], noteQueue, ^(int i){}) == NOTIFY_STATUS_OK);
		assert(notify_cancel(tokens[i]) == NOTIFY_STATUS_OK);
		assert(notify_post("com.example.test") == NOTIFY_STATUS_OK);
	});


	dispatch_apply(100000, DISPATCH_APPLY_AUTO, ^(size_t i) {
		assert(notify_register_check("self.example.test", &tokens[i]) == NOTIFY_STATUS_OK);
		assert(notify_cancel(tokens[i]) == NOTIFY_STATUS_OK);
		assert(notify_register_dispatch("self.example.test", &tokens[i], noteQueue, ^(int i){}) == NOTIFY_STATUS_OK);
		assert(notify_cancel(tokens[i]) == NOTIFY_STATUS_OK);
		assert(notify_post("self.example.test") == NOTIFY_STATUS_OK);
	});

	T_PASS("Success");
}
