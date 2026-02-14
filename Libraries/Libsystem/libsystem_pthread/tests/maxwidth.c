#include <dispatch/dispatch.h>
#include <dispatch/private.h>
#include <stdio.h>

#define NUM 100000
static volatile size_t concur;
static volatile size_t final;
dispatch_queue_t resultsq;
dispatch_group_t rgroup;

void finish(void* ctxt)
{
	int c = (uintptr_t)ctxt;
	if (c > final) final = c;
}

void work(void* ctxt)
{
	int c = __sync_add_and_fetch(&concur, 1);
	if (ctxt) {
		usleep(1000);
	} else {
		for (int i=0; i<100000; i++) {
			__asm__ __volatile__ ("");
		}
	}
	dispatch_group_async_f(rgroup, resultsq, (void*)(uintptr_t)c, finish);
	__sync_sub_and_fetch(&concur, 1);
}

int main(int argc, const char *argv[])
{
	size_t i;

	rgroup = dispatch_group_create();
	resultsq = dispatch_queue_create("results", 0);
	dispatch_suspend(resultsq);

	dispatch_group_t group = dispatch_group_create();

	final = concur = 0;
	for (i=0; i<NUM; i++) {
		dispatch_group_async_f(group, dispatch_get_global_queue(0, 0), NULL, work);
	}

	dispatch_group_wait(group, DISPATCH_TIME_FOREVER);
	dispatch_resume(resultsq);

	dispatch_group_wait(rgroup, DISPATCH_TIME_FOREVER);
	printf("max concurrency: %zd threads.\n", final);

	dispatch_suspend(resultsq);
	
	/* ******* */

	final = concur = 0;
	for (i=0; i<NUM; i++) {
		dispatch_group_async_f(group, dispatch_get_global_queue(0, 0), (void*)1, work);
	}

	dispatch_group_wait(group, DISPATCH_TIME_FOREVER);
	dispatch_resume(resultsq);

	dispatch_group_wait(rgroup, DISPATCH_TIME_FOREVER);
	printf("max blocking concurrency: %zd threads.\n", final);

	dispatch_suspend(resultsq);

	/* ******* */

	final = concur = 0;
	for (i=0; i<NUM; i++) {
		dispatch_group_async_f(group, dispatch_get_global_queue(0, DISPATCH_QUEUE_OVERCOMMIT), NULL, work);
	}

	dispatch_group_wait(group, DISPATCH_TIME_FOREVER);
	dispatch_resume(resultsq);

	dispatch_group_wait(rgroup, DISPATCH_TIME_FOREVER);
	printf("max overcommit concurrency: %zd threads.\n", final);
	dispatch_suspend(resultsq);

	/* ******* */

	final = concur = 0;
	for (i=0; i<NUM; i++) {
		dispatch_group_async_f(group, dispatch_get_global_queue(0, DISPATCH_QUEUE_OVERCOMMIT), (void*)1, work);
	}

	dispatch_group_wait(group, DISPATCH_TIME_FOREVER);
	dispatch_resume(resultsq);

	dispatch_group_wait(rgroup, DISPATCH_TIME_FOREVER);
	printf("max blocking overcommit concurrency: %zd threads.\n", final);

	return 0;
}
