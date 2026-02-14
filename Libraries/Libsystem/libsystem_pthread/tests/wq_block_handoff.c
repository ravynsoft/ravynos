#include <dispatch/dispatch.h>
#include <sys/sysctl.h>
#include <stdio.h>

static int x = 0;
static int y = 0;

int main(void)
{
	/* found in <rdar://problem/16326400> 12A216: Spotlight takes a long time to show results */
	
	/* we need to start up NCPU-1 threads in a given bucket, then fire up one more at a separate
	 * priority.
	 *
	 * each of these waiters needs to be non-blocked until the point where dispatch wants to
	 * request a new thread.
	 *
	 * if dispatch ever fixes sync_barrier -> sync handoff to not require an extra thread,
	 * then this test will never fail and will be invalid.
	 */
	 
	printf("[TEST] barrier_sync -> async @ ncpu threads\n");
	 
	dispatch_semaphore_t sema = dispatch_semaphore_create(0);
	
	int ncpu = 1;
	size_t sz = sizeof(ncpu);
	sysctlbyname("hw.ncpu", &ncpu, &sz, NULL, 0);
	printf("starting up %d waiters.\n", ncpu);
	
	dispatch_queue_t q = dispatch_queue_create("moo", DISPATCH_QUEUE_CONCURRENT);
	dispatch_barrier_sync(q, ^{
		dispatch_async(q, ^{ 
			printf("async.\n"); 
			dispatch_semaphore_signal(sema);
		});
		for (int i=0; i<ncpu-1; i++) {
			dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0), ^{
				printf("waiter %d* up.\n", i);
				while (y == 0) { };
			});
		}
		dispatch_async(dispatch_get_global_queue(0, 0), ^{
			printf("waiter %d up.\n", ncpu-1);
			while (x == 0) { };
			printf("waiter %d idle.\n", ncpu-1);
			usleep(1000);
			dispatch_sync(q, ^{ printf("quack %d\n", ncpu-1); });
		});
		printf("waiting...\n");
		sleep(1);
		printf("done.\n");
	});
	
	x = 1;
	int rv = dispatch_semaphore_wait(sema, dispatch_time(DISPATCH_TIME_NOW, 2ull * NSEC_PER_SEC));
	printf("[%s] barrier_sync -> async completed\n", rv == 0 ? "PASS" : "FAIL");

	return rv;
}
