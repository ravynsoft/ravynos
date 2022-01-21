/*-
 * Copyright (c) 2009-2014, Stacey Son <sson@freebsd.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice unmodified, this list of conditions, and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * $FreeBSD: $
 */

#include <sys/param.h>

struct _pthread_workqueue;

typedef struct _pthread_workqueue * 	pthread_workqueue_t;
typedef void *				pthread_workitem_handle_t;

/* Pad size to 64 bytes. */
typedef struct {
	uint32_t sig;
	int queueprio;
	int overcommit;
	unsigned int pad[13];
} pthread_workqueue_attr_t;

/* Work queue priority attributes. */
#define	WORKQ_HIGH_PRIOQUEUE	0	/* Assign to high priority queue. */
#define	WORKQ_DEFAULT_PRIOQUEUE	1	/* Assign to default priority queue. */
#define	WORKQ_LOW_PRIOQUEUE	2	/* Assign to low priority queue. */
#define WORKQ_BG_PRIOQUEUE	3	/* background priority queue */

#define WORKQ_NUM_PRIOQUEUE	4

extern __int32_t workq_targetconc[WORKQ_NUM_PRIOQUEUE];

int pthread_workqueue_init_np(void);
int pthread_workqueue_create_np(pthread_workqueue_t * workqp,
    const pthread_workqueue_attr_t * attr);
int pthread_workqueue_additem_np(pthread_workqueue_t workq,
    void *( *workitem_func)(void *), void * workitem_arg,
    pthread_workitem_handle_t * itemhandlep, unsigned int *gencountp);
int pthread_workqueue_attr_init_np(pthread_workqueue_attr_t * attrp);
int pthread_workqueue_attr_destroy_np(pthread_workqueue_attr_t * attr);
int pthread_workqueue_attr_setqueuepriority_np(pthread_workqueue_attr_t * attr,
    int qprio);
int pthread_workqueue_attr_getovercommit_np(
    const pthread_workqueue_attr_t * attr, int * ocommp);
int pthread_workqueue_attr_setovercommit_np(pthread_workqueue_attr_t * attr,
    int ocomm);
int pthread_workqueue_requestconcurrency_np(pthread_workqueue_t workq,
    int queue, int request_concurrency);
int pthread_workqueue_getovercommit_np(pthread_workqueue_t workq,
    unsigned int *ocommp);
void pthread_workqueue_atfork_child(void);
void pthread_workqueue_atfork_parent(void);
void pthread_workqueue_atfork_prepare(void);
__END_DECLS
