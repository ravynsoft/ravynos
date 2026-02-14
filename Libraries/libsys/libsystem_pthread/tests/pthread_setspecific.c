/*
 * @OSF_COPYRIGHT@
 *
 */
/*
 * HISTORY
 * $Log: pthread_test3.c,v $
 * Revision 1.1.4.2  1996/10/03  17:53:38  emcmanus
 * 	Changed fprintf(stderr...) to printf(...) to allow building with
 * 	PURE_MACH includes.
 * 	[1996/10/03  16:17:34  emcmanus]
 *
 * Revision 1.1.4.1  1996/10/01  07:36:02  emcmanus
 * 	Copied from rt3_merge.
 * 	Include <stdlib.h> for malloc() prototype.
 * 	[1996/10/01  07:35:53  emcmanus]
 *
 * Revision 1.1.2.1  1996/09/27  13:12:15  gdt
 * 	Add support for thread specific data
 * 	[1996/09/27  13:11:17  gdt]
 *
 * $EndLog$
 */

/*
 * Test POSIX Thread Specific Data
 */

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

#include "darwintest_defaults.h"

static pthread_key_t key;

static void *
thread(void * arg)
{
	char * msg;
	T_LOG("thread %lx here: %s\n", (uintptr_t)pthread_self(), (char *)arg);
	msg = malloc(256);
	sprintf(msg, "This is thread specific data for %lx\n", (uintptr_t)pthread_self());
	T_ASSERT_POSIX_ZERO(pthread_setspecific(key, msg), NULL);
	return (arg);
}

static void
grim_reaper(void * param)
{
	T_LOG("grim_reaper - self: %lx, param: %lx  value: %s", (uintptr_t)pthread_self(), (uintptr_t)param, (char *)param);
	free(param);
}

T_DECL(pthread_setspecific, "pthread_setspecific",
       T_META_ALL_VALID_ARCHS(YES))
{
	void * thread_res;
	pthread_t t1, t2;
	T_ASSERT_POSIX_ZERO(pthread_key_create(&key, grim_reaper), NULL);
	T_ASSERT_POSIX_ZERO(pthread_create(&t1, (pthread_attr_t *)NULL, thread, "thread #1 arg"), NULL);
	T_ASSERT_POSIX_ZERO(pthread_create(&t2, (pthread_attr_t *)NULL, thread, "thread #2 arg"), NULL);
	T_ASSERT_POSIX_ZERO(pthread_join(t1, &thread_res), NULL);
	T_ASSERT_POSIX_ZERO(pthread_join(t2, &thread_res), NULL);
}
