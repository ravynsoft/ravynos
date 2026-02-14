//
//  notify_benchmark.c
//  darwintests
//
//  Created by Brycen Wershing on 7/26/18.
//

#include <darwintest.h>
#include <notify.h>
#include <dispatch/dispatch.h>
#include "notify_private.h"
#include <stdlib.h>


static const uint32_t CNT = 10;

#if TARGET_OS_WATCH
static const uint32_t SPL = 1000;
#else
static const uint32_t SPL = 10000;
#endif

static const uint32_t CHECK_WEIGHT = 100;
static const uint32_t COAL_WEIGHT = 25;

// We use bench assert here as it has a lower observer effect than T_* checks.
#define bench_assert(X) if(!(X)) {T_FAIL("bench_assertion failed on line %d: %s", __LINE__, #X);}

static void
notify_fence(void)
{
	int fence_token;
	notify_register_check("com.apple.notify.test", &fence_token);
	notify_cancel(fence_token);
}

T_DECL(notify_benchmark_post,
	"notify benchmark post",
	T_META_EASYPERF(true),
	T_META_EASYPERF_ARGS("-p notifyd"),
	T_META("owner", "Core Darwin Daemons & Tools"),
	T_META("as_root", "false"))
{
	uint32_t r;
	uint32_t i, j;

	int t[CNT];
	char *n[CNT];
	size_t l[CNT];


	for (i = 0; i < CNT; i++)
	{
		r = asprintf(&n[i], "dummy.test.%d", i);
		assert(r != -1);
		l[i] = strlen(n[i]);
	}

	for (j = 0 ; j < SPL; j++)
	{


		/* Register Plain */
		for (i = 0; i < CNT; i++)
		{
			r = notify_register_plain(n[i], &t[i]);
			bench_assert(r == 0);
		}

		/* Post 1 */
		for (i = 0; i < CNT; i++)
		{
			r = notify_post(n[i]);
			bench_assert(r == 0);
		}

		/* Post 2 */
		for (i = 0; i < CNT; i++)
		{
			r = notify_post(n[i]);
			bench_assert(r == 0);
		}

		/* Post 3 */
		for (i = 0; i < CNT; i++)
		{
			r = notify_post(n[i]);
			bench_assert(r == 0);
		}

		/* Cancel Plain */
		for (i = 0; i < CNT; i++)
		{
			r = notify_cancel(t[i]);
			bench_assert(r == 0);
		}

	}

	for (i = 0; i < CNT; i++)
	{
		free(n[i]);
	}

	T_PASS("Notify Benchmark Succeeded!");
}


T_DECL(notify_benchmark_state,
       "notify benchmark state",
       T_META_EASYPERF(true),
       T_META_EASYPERF_ARGS("-p notifyd"),
       T_META("owner", "Core Darwin Daemons & Tools"),
       T_META("as_root", "false"))
{
	uint32_t r;
	unsigned i, j, k;

	int t[CNT];
	mach_port_t p[CNT];
	char *n[CNT];
	size_t l[CNT];
	uint64_t s;

	for (i = 0; i < CNT; i++)
	{
		r = asprintf(&n[i], "dummy.test.%d", i);
		assert(r != -1);
		l[i] = strlen(n[i]);
	}

	for (j = 0 ; j < SPL; j++)
	{


		/* Register Mach Port */
		for (i = 0; i < CNT; i++)
		{
			r = notify_register_mach_port(n[i], &p[i], 0, &t[i]);
			bench_assert(r == 0);
		}


		/* Set State 1 */
		for (i = 0; i < CNT; i++)
		{
			r = notify_set_state(t[i], 1);
			bench_assert(r == 0);
		}

		/* Get State */
		for (i = 0; i < CNT; i++)
		{
			uint64_t dummy;
			r = notify_get_state(t[i], &dummy);
			bench_assert(r == 0);
		}

		/* Set State 2 */
		for (i = 0; i < CNT; i++)
		{
			r = notify_set_state(t[i], 2);
			bench_assert(r == 0);
		}

		/* Cancel Port */
		for (i = 0; i < CNT; i++)
		{
			r = notify_cancel(t[i]);
			bench_assert(r == 0);
		}


	}

	for (i = 0; i < CNT; i++)
	{
		free(n[i]);
	}

	T_PASS("Notify Benchmark Succeeded!");
}



T_DECL(notify_benchmark_check,
       "notify benchmark check",
       T_META_EASYPERF(true),
       T_META_EASYPERF_ARGS("-p notifyd"),
       T_META("owner", "Core Darwin Daemons & Tools"),
       T_META("as_root", "false"))
{
	uint32_t r;
	unsigned i, j, k;

	int t[CNT];
	char *n[CNT];
	size_t l[CNT];
	int check;


	for (i = 0; i < CNT; i++)
	{
		r = asprintf(&n[i], "dummy.test.%d", i);
		assert(r != -1);
		l[i] = strlen(n[i]);
	}

	for (j = 0 ; j < SPL; j++)
	{

		/* Register Check */
		for (i = 0; i < CNT; i++)
		{
			r = notify_register_check("com.apple.notify.test.check", &t[i]);
			bench_assert(r == 0);
		}

		/* Check 1 */
		for (i = 0; i < CNT; i++)
		{
			r = notify_check(t[i], &check);
			bench_assert(r == 0);
			bench_assert(check == 1);
		}

		/* Check 2 */
		for (i = 0; i < CNT; i++)
		{
			for(k = 0; k < CHECK_WEIGHT; k++){
				r = notify_check(t[i], &check);
				bench_assert(r == 0);
				bench_assert(check == 0);
			}
		}

		/* Check 3 */
		for (i = 0; i < CNT; i++)
		{
			for(k = 0; k < CHECK_WEIGHT; k++){
				r = notify_check(t[i], &check);
				bench_assert(r == 0);
				bench_assert(check == 0);
			}
		}

		notify_post("com.apple.notify.test.check");

		notify_fence();

		/* Check 4 */
		for (i = 0; i < CNT; i++)
		{
			r = notify_check(t[i], &check);
			bench_assert(r == 0);
			bench_assert(check == 1);
		}

		/* Check 5 */
		for (i = 0; i < CNT; i++)
		{
			for(k = 0; k < CHECK_WEIGHT; k++){
				r = notify_check(t[i], &check);
				bench_assert(r == 0);
				bench_assert(check == 0);
			}
		}

		/* Cancel Check */
		for (i = 0; i < CNT; i++)
		{
			r = notify_cancel(t[i]);
			bench_assert(r == 0);
		}

	}

	for (i = 0; i < CNT; i++)
	{
		free(n[i]);
	}

	T_PASS("Notify Benchmark Succeeded!");
}

T_DECL(notify_benchmark_dispatch,
       "notify benchmark dispatch",
       T_META_EASYPERF(true),
       T_META_EASYPERF_ARGS("-p notifyd"),
       T_META("owner", "Core Darwin Daemons & Tools"),
       T_META("as_root", "false"))
{
	uint32_t r;
	unsigned i, j, k;

	int t[CNT];
	int t_2[CNT];
	char *n[CNT];
	size_t l[CNT];
	uint64_t s;
	__block volatile int dispatch_changer;


	dispatch_queue_t disp_q = dispatch_queue_create("Notify.Test", NULL);



	for (i = 0; i < CNT; i++)
	{
		r = asprintf(&n[i], "dummy.test.%d", i);
		assert(r != -1);
		l[i] = strlen(n[i]);
	}

	for (j = 0 ; j < SPL; j++)
	{


		/* Register Dispatch 1 */
		for (i = 0; i < CNT; i++)
		{
			r = notify_register_dispatch(n[i], &t[i], disp_q, ^(int x){
				dispatch_changer = x;
			});
			bench_assert(r == 0);
		}

		for (k = 0; k < COAL_WEIGHT; k++){
			/* Register Dispatch 2 (Coalesced) */
			for (i = 0; i < CNT; i++)
			{
				r = notify_register_dispatch(n[i], &t_2[i], disp_q, ^(int x){
					dispatch_changer = x;
				});
				bench_assert(r == 0);
			}

			/* Cancel Coalesced Dispatch */
			for (i = 0; i < CNT; i++){
				r = notify_cancel(t_2[i]);
				bench_assert(r == 0);
			}
		}

		/* Cancel Dispatch */
		for (i = 0; i < CNT; i++)
		{
			r = notify_cancel(t[i]);
			bench_assert(r == 0);

		}
	}

	for (i = 0; i < CNT; i++)
	{
		free(n[i]);
	}

	T_PASS("Notify Benchmark Succeeded!");
}
