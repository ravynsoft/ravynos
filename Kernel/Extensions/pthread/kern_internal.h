/*
 * Copyright (c) 2000-2003 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 *
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */

#ifndef _SYS_PTHREAD_INTERNAL_H_
#define _SYS_PTHREAD_INTERNAL_H_

#include <pthread/bsdthread_private.h>
#include <pthread/priority_private.h>
#include <pthread/workqueue_syscalls.h>

#ifdef KERNEL
struct ksyn_waitq_element;
#include <stdatomic.h>
#include <kern/thread_call.h>
#include <kern/kcdata.h>
#include <sys/pthread_shims.h>
#include <sys/queue.h>
#include <sys/proc_info.h>

#ifdef __arm64__
#define PTHREAD_INLINE_RMW_ATOMICS 0
#else
#define PTHREAD_INLINE_RMW_ATOMICS 1
#endif
#endif // KERNEL

#include "kern/synch_internal.h"
#include "kern/workqueue_internal.h"
#include "kern/kern_trace.h"
#include "pthread/qos.h"
#include "private/qos_private.h"

/* pthread userspace SPI feature checking, these constants are returned from bsdthread_register,
 * as a bitmask, to inform userspace of the supported feature set. Old releases of OS X return
 * from this call either zero or -1, allowing us to return a positive number for feature bits.
 */
#define PTHREAD_FEATURE_DISPATCHFUNC	0x01		/* same as WQOPS_QUEUE_NEWSPISUPP, checks for dispatch function support */
#define PTHREAD_FEATURE_FINEPRIO		0x02		/* are fine grained prioirities available */
#define PTHREAD_FEATURE_BSDTHREADCTL	0x04		/* is the bsdthread_ctl syscall available */
#define PTHREAD_FEATURE_SETSELF			0x08		/* is the BSDTHREAD_CTL_SET_SELF command of bsdthread_ctl available */
#define PTHREAD_FEATURE_QOS_MAINTENANCE	0x10		/* is QOS_CLASS_MAINTENANCE available */
#define PTHREAD_FEATURE_RESERVED		0x20		/* burnt, shipped in OSX 10.11 & iOS 9 with partial kevent delivery support */
#define PTHREAD_FEATURE_KEVENT          0x40		/* supports direct kevent delivery */
#define PTHREAD_FEATURE_WORKLOOP          0x80		/* supports workloops */
#define PTHREAD_FEATURE_QOS_DEFAULT		0x40000000	/* the kernel supports QOS_CLASS_DEFAULT */

/* userspace <-> kernel registration struct, for passing data to/from the kext during main thread init. */
struct _pthread_registration_data {
	/*
	 * version == sizeof(struct _pthread_registration_data)
	 *
	 * The structure can only grow, so we use its size as the version.
	 * Userspace initializes this to the size of its structure and the kext
	 * will copy out the version that was actually consumed.
	 *
	 * n.b. you must make sure the size of this structure isn't LP64-dependent
	 */
	uint64_t version;

	uint64_t dispatch_queue_offset; /* copy-in */
	uint64_t /* pthread_priority_t */ main_qos; /* copy-out */
	uint32_t tsd_offset; /* copy-in */
	uint32_t return_to_kernel_offset; /* copy-in */
	uint32_t mach_thread_self_offset; /* copy-in */
	mach_vm_address_t stack_addr_hint; /* copy-out */
	uint32_t mutex_default_policy; /* copy-out */
} __attribute__ ((packed));

/*
 * "error" flags returned by fail condvar syscalls
 */
#define ECVCLEARED	0x100
#define ECVPREPOST	0x200

#ifdef KERNEL

/* The set of features, from the feature bits above, that we support. */
#define PTHREAD_FEATURE_SUPPORTED	( \
	PTHREAD_FEATURE_DISPATCHFUNC | \
	PTHREAD_FEATURE_FINEPRIO | \
	PTHREAD_FEATURE_BSDTHREADCTL | \
	PTHREAD_FEATURE_SETSELF | \
	PTHREAD_FEATURE_QOS_MAINTENANCE | \
	PTHREAD_FEATURE_QOS_DEFAULT | \
	PTHREAD_FEATURE_KEVENT | \
	PTHREAD_FEATURE_WORKLOOP )

extern pthread_callbacks_t pthread_kern;

struct ksyn_waitq_element {
	TAILQ_ENTRY(ksyn_waitq_element) kwe_list;	/* link to other list members */
	void *          kwe_kwqqueue;            	/* queue blocked on */
	thread_t        kwe_thread;
	uint16_t        kwe_state;			/* state */
	uint16_t        kwe_flags;
	uint32_t        kwe_lockseq;			/* the sequence of the entry */
	uint32_t	kwe_count;			/* upper bound on number of matches still pending */
	uint32_t 	kwe_psynchretval;		/* thread retval */
	void		*kwe_uth;			/* uthread */
};
typedef struct ksyn_waitq_element * ksyn_waitq_element_t;

#define PTH_DEFAULT_STACKSIZE 512*1024
#define MAX_PTHREAD_SIZE 64*1024

/* exported from the kernel but not present in any headers. */
extern thread_t port_name_to_thread(mach_port_name_t port_name);

/* function declarations for pthread_kext.c */
void pthread_init(void);
void psynch_zoneinit(void);
void _pth_proc_hashinit(proc_t p);
void _pth_proc_hashdelete(proc_t p);
void pth_global_hashinit(void);
void psynch_wq_cleanup(void*, void*);

void _pthread_init(void);
int _fill_procworkqueue(proc_t p, struct proc_workqueueinfo * pwqinfo);
uint32_t _get_pwq_state_kdp(proc_t p);
void _workqueue_exit(struct proc *p);
void _workqueue_mark_exiting(struct proc *p);
void _workqueue_thread_yielded(void);
sched_call_t _workqueue_get_sched_callback(void);

int _bsdthread_create(struct proc *p, user_addr_t user_func, user_addr_t user_funcarg, user_addr_t user_stack, user_addr_t user_pthread, uint32_t flags, user_addr_t *retval);
int _bsdthread_register(struct proc *p, user_addr_t threadstart, user_addr_t wqthread, int pthsize, user_addr_t dummy_value, user_addr_t targetconc_ptr, uint64_t dispatchqueue_offset, int32_t *retval);
int _bsdthread_terminate(struct proc *p, user_addr_t stackaddr, size_t size, uint32_t kthport, uint32_t sem, int32_t *retval);
int _bsdthread_ctl_set_qos(struct proc *p, user_addr_t cmd, mach_port_name_t kport, user_addr_t tsd_priority_addr, user_addr_t arg3, int *retval);
int _bsdthread_ctl_set_self(struct proc *p, user_addr_t cmd, pthread_priority_t priority, mach_port_name_t voucher, _pthread_set_flags_t flags, int *retval);
int _bsdthread_ctl_qos_override_start(struct proc *p, user_addr_t cmd, mach_port_name_t kport, pthread_priority_t priority, user_addr_t resource, int *retval);
int _bsdthread_ctl_qos_override_end(struct proc *p, user_addr_t cmd, mach_port_name_t kport, user_addr_t resource, user_addr_t arg3, int *retval);
int _bsdthread_ctl_qos_override_dispatch(struct proc __unused *p, user_addr_t __unused cmd, mach_port_name_t kport, pthread_priority_t priority, user_addr_t arg3, int __unused *retval);
int _bsdthread_ctl_qos_override_reset(struct proc __unused *p, user_addr_t __unused cmd, user_addr_t arg1, user_addr_t arg2, user_addr_t arg3, int __unused *retval);
int _bsdthread_ctl_qos_dispatch_asynchronous_override_add(struct proc __unused *p, user_addr_t __unused cmd, mach_port_name_t kport, pthread_priority_t priority, user_addr_t resource, int __unused *retval);
int _bsdthread_ctl_qos_dispatch_asynchronous_override_reset(struct proc __unused *p, user_addr_t __unused cmd, int reset_all, user_addr_t resource, user_addr_t arg3, int __unused *retval);
int _bsdthread_ctl(struct proc *p, user_addr_t cmd, user_addr_t arg1, user_addr_t arg2, user_addr_t arg3, int *retval);
int _thread_selfid(__unused struct proc *p, uint64_t *retval);
int _workq_kernreturn(struct proc *p, int options, user_addr_t item, int arg2, int arg3, int32_t *retval);
int _workq_open(struct proc *p, int32_t *retval);

int _psynch_mutexwait(proc_t p, user_addr_t mutex,  uint32_t mgen, uint32_t  ugen, uint64_t tid, uint32_t flags, uint32_t * retval);
int _psynch_mutexdrop(proc_t p, user_addr_t mutex,  uint32_t mgen, uint32_t  ugen, uint64_t tid, uint32_t flags, uint32_t * retval);
int _psynch_cvbroad(proc_t p, user_addr_t cv, uint64_t cvlsgen, uint64_t cvudgen, uint32_t flags, user_addr_t mutex,  uint64_t mugen, uint64_t tid, uint32_t *retval);
int _psynch_cvsignal(proc_t p, user_addr_t cv, uint64_t cvlsgen, uint32_t cvugen, int thread_port, user_addr_t mutex,  uint64_t mugen, uint64_t tid, uint32_t flags, uint32_t * retval);
int _psynch_cvwait(proc_t p, user_addr_t cv, uint64_t cvlsgen, uint32_t cvugen, user_addr_t mutex,  uint64_t mugen, uint32_t flags, int64_t sec, uint32_t nsec, uint32_t * retval);
int _psynch_cvclrprepost(proc_t p, user_addr_t cv, uint32_t cvgen, uint32_t cvugen, uint32_t cvsgen, uint32_t prepocnt, uint32_t preposeq, uint32_t flags, int *retval);
int _psynch_rw_longrdlock(proc_t p, user_addr_t rwlock, uint32_t lgenval, uint32_t ugenval, uint32_t rw_wc, int flags, uint32_t * retval);
int _psynch_rw_rdlock(proc_t p, user_addr_t rwlock, uint32_t lgenval, uint32_t ugenval, uint32_t rw_wc, int flags, uint32_t *retval);
int _psynch_rw_unlock(proc_t p, user_addr_t rwlock, uint32_t lgenval, uint32_t ugenval, uint32_t rw_wc, int flags, uint32_t *retval);
int _psynch_rw_wrlock(proc_t p, user_addr_t rwlock, uint32_t lgenval, uint32_t ugenval, uint32_t rw_wc, int flags, uint32_t *retval);
int _psynch_rw_yieldwrlock(proc_t p, user_addr_t rwlock, uint32_t lgenval, uint32_t ugenval, uint32_t rw_wc, int flags, uint32_t *retval);

void _pthread_find_owner(thread_t thread, struct stackshot_thread_waitinfo *waitinfo);
void * _pthread_get_thread_kwq(thread_t thread);

extern lck_grp_attr_t *pthread_lck_grp_attr;
extern lck_grp_t *pthread_lck_grp;
extern lck_attr_t *pthread_lck_attr;
extern lck_mtx_t *pthread_list_mlock;
extern thread_call_t psynch_thcall;

struct uthread* current_uthread(void);

int
workq_create_threadstack(proc_t p, vm_map_t vmap, mach_vm_offset_t *out_addr);

int
workq_destroy_threadstack(proc_t p, vm_map_t vmap, mach_vm_offset_t stackaddr);

void
workq_setup_thread(proc_t p, thread_t th, vm_map_t map, user_addr_t stackaddr,
		mach_port_name_t kport, int th_qos, int setup_flags, int upcall_flags);

int
workq_handle_stack_events(proc_t p, thread_t th, vm_map_t map,
		user_addr_t stackaddr, mach_port_name_t kport,
		user_addr_t events, int nevents, int upcall_flags);

void
workq_markfree_threadstack(proc_t p, thread_t th, vm_map_t vmap,
		user_addr_t stackaddr);

#endif // KERNEL

#endif /* _SYS_PTHREAD_INTERNAL_H_ */

