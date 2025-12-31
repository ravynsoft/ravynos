//
//  notify_pathwatch.c
//  tests
//
//  Created by Brycen Wershing on 8/2/18.
//

#include <darwintest.h>
#include <dispatch/dispatch.h>
#include <notify.h>
#include <stdio.h>
#include <stdlib.h>
#include <mach/mach.h>
#include <darwintest_multiprocess.h>
#include "notify_private.h"

int dispatch_changer;

#define PATHWATCH_TEST_NAME "com.example.test.pathwatch"
#define PATHWATCH_TEST_FILE "/tmp/notify_pathwatch.test"

T_DECL(notify_pathwatch,
       "notify pathwatch test",
       T_META("owner", "Core Darwin Daemons & Tools"),
       T_META("as_root", "true"))
{
	dispatch_queue_t testQueue = dispatch_queue_create("testQ", DISPATCH_QUEUE_SERIAL);
	uint32_t rc;
	static int check_token = NOTIFY_TOKEN_INVALID;
	static int disp_token = NOTIFY_TOKEN_INVALID;
	int check;
	uint64_t state;
	int fd, bytes_written;

	dispatch_changer = 0;


	// register for check and dispatch

	rc = notify_register_check(PATHWATCH_TEST_NAME, &check_token);
	assert(rc == NOTIFY_STATUS_OK);

	rc = notify_check(check_token, &check);
	assert(rc == NOTIFY_STATUS_OK);
	assert(check == 1);

	rc = notify_register_dispatch(PATHWATCH_TEST_NAME, &disp_token, testQueue, ^(int i){dispatch_changer++;});
	assert(rc == NOTIFY_STATUS_OK);


	// add file monitoring for nonexistant file
	unlink(PATHWATCH_TEST_FILE); // return code ignored because the file probably does not exist


	notify_monitor_file(check_token, PATHWATCH_TEST_FILE, 0x3ff);
	notify_monitor_file(disp_token, PATHWATCH_TEST_FILE, 0x3ff);


	// there should not be a post yet
	dispatch_sync(testQueue, ^{assert(
		dispatch_changer == 0);
	});

	rc = notify_check(check_token, &check);
	assert(rc == NOTIFY_STATUS_OK);
	assert(check == 0);


	// post and fence
	rc = notify_post(PATHWATCH_TEST_NAME);
	assert(rc == NOTIFY_STATUS_OK);

	notify_get_state(check_token, &state); //fence
	sleep(1);

	// check to make sure post was good
	rc = notify_check(check_token, &check);
	assert(rc == NOTIFY_STATUS_OK);
	assert(check == 1);

	dispatch_sync(testQueue, ^{
		assert(dispatch_changer == 1);
		dispatch_changer = 0;
	});




	// create the file
	fd = creat(PATHWATCH_TEST_FILE, 0);
	assert(fd != -1);

	sleep(1);
	notify_get_state(check_token, &state); //fence

	// check to make sure post was good
	rc = notify_check(check_token, &check);
	assert(rc == NOTIFY_STATUS_OK);
	assert(check == 1);

	dispatch_sync(testQueue, ^{
		assert(dispatch_changer == 1);
		dispatch_changer = 0;
	});


	// write something to the file
	bytes_written = write(fd, "this", 4);
	assert(bytes_written == 4);

	sleep(1);
	notify_get_state(check_token, &state); //fence

	// check to make sure post was good
	rc = notify_check(check_token, &check);
	assert(rc == NOTIFY_STATUS_OK);
	assert(check == 1);

	dispatch_sync(testQueue, ^{
		assert(dispatch_changer == 1);
		dispatch_changer = 0;
	});


	// delete the file
	rc = close(fd);
	assert(rc == 0);

	unlink(PATHWATCH_TEST_FILE);

	sleep(1);
	notify_get_state(check_token, &state); //fence

	// check to make sure post was good
	rc = notify_check(check_token, &check);
	assert(rc == NOTIFY_STATUS_OK);
	assert(check == 1);

	dispatch_sync(testQueue, ^{
		assert(dispatch_changer == 1);
		dispatch_changer = 0;
	});

	// cleanup
	rc = notify_cancel(check_token);
	assert(rc == NOTIFY_STATUS_OK);

	rc = notify_cancel(disp_token);
	assert(rc == NOTIFY_STATUS_OK);

	dispatch_release(testQueue);

	fd = creat(PATHWATCH_TEST_FILE, 0);
	assert(fd != -1);

	
	dt_helper_t helpers[50];
	for(int i = 0; i < 50; i++){
		helpers[i] = dt_child_helper("notify_pathwatch_helper");
	}

	dt_run_helpers(helpers, 50, 180);

	rc = close(fd);
	assert(rc == 0);

	unlink(PATHWATCH_TEST_FILE);


	T_PASS("Success");
}


T_HELPER_DECL(notify_pathwatch_helper,
	      "notify pathwatch test helper",
	      T_META("owner", "Core Darwin Daemons & Tools"))
{
	int token;
	char *filename;
	uint32_t block_rc;
	block_rc = notify_register_check(PATHWATCH_TEST_NAME, &token);
	assert(block_rc == NOTIFY_STATUS_OK);
	notify_monitor_file(token, PATHWATCH_TEST_FILE, 0x3ff);
	T_PASS("Helper passed");
	T_END;
	exit(0);
}
