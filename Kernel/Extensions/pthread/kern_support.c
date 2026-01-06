/*
 * Copyright (c) 2000-2017 Apple Inc. All rights reserved.
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
/* Copyright (c) 1995-2005 Apple Computer, Inc. All Rights Reserved */
/*
 *	pthread_synch.c
 */

#pragma mark - Front Matter

#define _PTHREAD_CONDATTR_T
#define _PTHREAD_COND_T
#define _PTHREAD_MUTEXATTR_T
#define _PTHREAD_MUTEX_T
#define _PTHREAD_RWLOCKATTR_T
#define _PTHREAD_RWLOCK_T

#undef pthread_mutexattr_t
#undef pthread_mutex_t
#undef pthread_condattr_t
#undef pthread_cond_t
#undef pthread_rwlockattr_t
#undef pthread_rwlock_t

#include <sys/cdefs.h>
#include <os/log.h>

// <rdar://problem/26158937> panic() should be marked noreturn
extern void panic(const char *string, ...) __printflike(1,2) __dead2;

#include <sys/param.h>
#include <sys/queue.h>
#include <sys/resourcevar.h>
//#include <sys/proc_internal.h>
#include <sys/kauth.h>
#include <sys/systm.h>
#include <sys/timeb.h>
#include <sys/times.h>
#include <sys/acct.h>
#include <sys/kernel.h>
#include <sys/wait.h>
#include <sys/signalvar.h>
#include <sys/sysctl.h>
#include <sys/syslog.h>
#include <sys/stat.h>
#include <sys/lock.h>
#include <sys/kdebug.h>
//#include <sys/sysproto.h>
#include <sys/vm.h>
#include <sys/user.h>		/* for coredump */
#include <sys/proc_info.h>	/* for fill_procworkqueue */

#include <mach/mach_port.h>
#include <mach/mach_types.h>
#include <mach/semaphore.h>
#include <mach/sync_policy.h>
#include <mach/task.h>
#include <mach/vm_prot.h>
#include <kern/kern_types.h>
#include <kern/task.h>
#include <kern/clock.h>
#include <mach/kern_return.h>
#include <kern/thread.h>
#include <kern/zalloc.h>
#include <kern/sched_prim.h>	/* for thread_exception_return */
#include <kern/processor.h>
#include <kern/assert.h>
#include <mach/mach_vm.h>
#include <mach/mach_param.h>
#include <mach/thread_status.h>
#include <mach/thread_policy.h>
#include <mach/message.h>
#include <mach/port.h>
//#include <vm/vm_protos.h>
#include <vm/vm_fault.h>
#include <vm/vm_map.h>
#include <mach/thread_act.h> /* for thread_resume */
#include <machine/machine_routines.h>
#include <mach/shared_region.h>

#include <libkern/OSAtomic.h>
#include <libkern/libkern.h>

#include "kern_internal.h"

#ifndef WQ_SETUP_EXIT_THREAD
#define WQ_SETUP_EXIT_THREAD    8
#endif

// XXX: Ditto for thread tags from kern/thread.h
#define	THREAD_TAG_MAINTHREAD 0x1
#define	THREAD_TAG_PTHREAD 0x10
#define	THREAD_TAG_WORKQUEUE 0x20

lck_grp_attr_t   *pthread_lck_grp_attr;
lck_grp_t    *pthread_lck_grp;
lck_attr_t   *pthread_lck_attr;

#define C_32_STK_ALIGN          16
#define C_64_STK_ALIGN          16

// WORKQ use the largest alignment any platform needs
#define C_WORKQ_STK_ALIGN       16

#if defined(__arm64__)
/* Pull the pthread_t into the same page as the top of the stack so we dirty one less page.
 * <rdar://problem/19941744> The _pthread struct at the top of the stack shouldn't be page-aligned
 */
#define PTHREAD_T_OFFSET (12*1024)
#else
#define PTHREAD_T_OFFSET 0
#endif

/*
 * Flags filed passed to bsdthread_create and back in pthread_start
31  <---------------------------------> 0
_________________________________________
| flags(8) | policy(8) | importance(16) |
-----------------------------------------
*/

#define PTHREAD_START_CUSTOM		0x01000000 // <rdar://problem/34501401>
#define PTHREAD_START_SETSCHED		0x02000000
// was PTHREAD_START_DETACHED		0x04000000
#define PTHREAD_START_QOSCLASS		0x08000000
#define PTHREAD_START_TSD_BASE_SET	0x10000000
#define PTHREAD_START_SUSPENDED		0x20000000
#define PTHREAD_START_QOSCLASS_MASK	0x00ffffff
#define PTHREAD_START_POLICY_BITSHIFT 16
#define PTHREAD_START_POLICY_MASK 0xff
#define PTHREAD_START_IMPORTANCE_MASK 0xffff

#define SCHED_OTHER      POLICY_TIMESHARE
#define SCHED_FIFO       POLICY_FIFO
#define SCHED_RR         POLICY_RR

#define BASEPRI_DEFAULT 31

uint32_t pthread_debug_tracing = 1;

static uint32_t pthread_mutex_default_policy;

SYSCTL_INT(_kern, OID_AUTO, pthread_mutex_default_policy, CTLFLAG_RW | CTLFLAG_LOCKED,
	   &pthread_mutex_default_policy, 0, "");

#pragma mark - Process/Thread Setup/Teardown syscalls

static mach_vm_offset_t
stack_addr_hint(proc_t p, vm_map_t vmap)
{
	mach_vm_offset_t stackaddr;
	mach_vm_offset_t aslr_offset;
	bool proc64bit = proc_is64bit(p);
	bool proc64bit_data = proc_is64bit_data(p);

	// We can't safely take random values % something unless its a power-of-two
	_Static_assert(powerof2(PTH_DEFAULT_STACKSIZE), "PTH_DEFAULT_STACKSIZE is a power-of-two");

#if defined(__i386__) || defined(__x86_64__)
	(void)proc64bit_data;
	if (proc64bit) {
		// Matches vm_map_get_max_aslr_slide_pages's image shift in xnu
		aslr_offset = random() % (1 << 28); // about 512 stacks
	} else {
		// Actually bigger than the image shift, we've got ~256MB to work with
		aslr_offset = random() % (16 * PTH_DEFAULT_STACKSIZE);
	}
	aslr_offset = vm_map_trunc_page_mask(aslr_offset, vm_map_page_mask(vmap));
	if (proc64bit) {
		// Above nanomalloc range (see NANOZONE_SIGNATURE)
		stackaddr = 0x700000000000 + aslr_offset;
	} else {
		stackaddr = SHARED_REGION_BASE_I386 + SHARED_REGION_SIZE_I386 + aslr_offset;
	}
#elif defined(__arm__) || defined(__arm64__)
	user_addr_t main_thread_stack_top = 0;
	if (pthread_kern->proc_get_user_stack) {
		main_thread_stack_top = pthread_kern->proc_get_user_stack(p);
	}
	if (proc64bit && main_thread_stack_top) {
		// The main thread stack position is randomly slid by xnu (c.f.
		// load_main() in mach_loader.c), so basing pthread stack allocations
		// where the main thread stack ends is already ASLRd and doing so
		// avoids creating a gap in the process address space that may cause
		// extra PTE memory usage. rdar://problem/33328206
		stackaddr = vm_map_trunc_page_mask((vm_map_offset_t)main_thread_stack_top,
				vm_map_page_mask(vmap));
	} else {
		// vm_map_get_max_aslr_slide_pages ensures 1MB of slide, we do better
		aslr_offset = random() % ((proc64bit ? 4 : 2) * PTH_DEFAULT_STACKSIZE);
		aslr_offset = vm_map_trunc_page_mask((vm_map_offset_t)aslr_offset,
				vm_map_page_mask(vmap));
		if (proc64bit) {
			// 64 stacks below shared region
			stackaddr = SHARED_REGION_BASE_ARM64 - 64 * PTH_DEFAULT_STACKSIZE - aslr_offset;
		} else {
			// If you try to slide down from this point, you risk ending up in memory consumed by malloc
			if (proc64bit_data) {
				stackaddr = SHARED_REGION_BASE_ARM64_32;
			} else {
				stackaddr = SHARED_REGION_BASE_ARM;
			}

			stackaddr -= 32 * PTH_DEFAULT_STACKSIZE + aslr_offset;
		}
	}
#else
#error Need to define a stack address hint for this architecture
#endif
	return stackaddr;
}

static bool
_pthread_priority_to_policy(pthread_priority_t priority,
		thread_qos_policy_data_t *data)
{
	data->qos_tier = _pthread_priority_thread_qos(priority);
	data->tier_importance = _pthread_priority_relpri(priority);
	if (data->qos_tier == THREAD_QOS_UNSPECIFIED || data->tier_importance > 0 ||
			data->tier_importance < THREAD_QOS_MIN_TIER_IMPORTANCE) {
		return false;
	}
	return true;
}

/**
 * bsdthread_create system call.  Used by pthread_create.
 */
int
_bsdthread_create(struct proc *p,
		__unused user_addr_t user_func, __unused user_addr_t user_funcarg,
		user_addr_t user_stack, user_addr_t user_pthread, uint32_t flags,
		user_addr_t *retval)
{
	kern_return_t kret;
	void * sright;
	int error = 0;
	mach_vm_offset_t th_tsd_base;
	mach_port_name_t th_thport;
	thread_t th;
	task_t ctask = current_task();
	unsigned int policy, importance;
	uint32_t tsd_offset;
	bool start_suspended = (flags & PTHREAD_START_SUSPENDED);

	if (pthread_kern->proc_get_register(p) == 0) {
		return EINVAL;
	}

	PTHREAD_TRACE(pthread_thread_create | DBG_FUNC_START, flags, 0, 0, 0);

	kret = pthread_kern->thread_create(ctask, &th);
	if (kret != KERN_SUCCESS)
		return(ENOMEM);
	thread_reference(th);

	pthread_kern->thread_set_tag(th, THREAD_TAG_PTHREAD);

	sright = (void *)pthread_kern->convert_thread_to_port(th);
	th_thport = pthread_kern->ipc_port_copyout_send(sright, pthread_kern->task_get_ipcspace(ctask));
	if (!MACH_PORT_VALID(th_thport)) {
		error = EMFILE; // userland will convert this into a crash
		goto out;
	}

	if ((flags & PTHREAD_START_CUSTOM) == 0) {
		error = EINVAL;
		goto out;
	}

	PTHREAD_TRACE(pthread_thread_create|DBG_FUNC_NONE, 0, 0, 0, 3);

	tsd_offset = pthread_kern->proc_get_pthread_tsd_offset(p);
	if (tsd_offset) {
		th_tsd_base = user_pthread + tsd_offset;
		kret = pthread_kern->thread_set_tsd_base(th, th_tsd_base);
		if (kret == KERN_SUCCESS) {
			flags |= PTHREAD_START_TSD_BASE_SET;
		}
	}
	/*
	 * Strip PTHREAD_START_SUSPENDED so that libpthread can observe the kernel
	 * supports this flag (after the fact).
	 */
	flags &= ~PTHREAD_START_SUSPENDED;

	/*
	 * Set up registers & function call.
	 */
#if defined(__i386__) || defined(__x86_64__)
	if (proc_is64bit_data(p)) {
		x86_thread_state64_t state = {
			.rip = (uint64_t)pthread_kern->proc_get_threadstart(p),
			.rdi = (uint64_t)user_pthread,
			.rsi = (uint64_t)th_thport,
			.rdx = (uint64_t)user_func,    /* golang wants this */
			.rcx = (uint64_t)user_funcarg, /* golang wants this */
			.r8  = (uint64_t)user_stack,   /* golang wants this */
			.r9  = (uint64_t)flags,

			.rsp = (uint64_t)user_stack,
		};

		(void)pthread_kern->thread_set_wq_state64(th, (thread_state_t)&state);
	} else {
		x86_thread_state32_t state = {
			.eip = (uint32_t)pthread_kern->proc_get_threadstart(p),
			.eax = (uint32_t)user_pthread,
			.ebx = (uint32_t)th_thport,
			.ecx = (uint32_t)user_func,    /* golang wants this */
			.edx = (uint32_t)user_funcarg, /* golang wants this */
			.edi = (uint32_t)user_stack,   /* golang wants this */
			.esi = (uint32_t)flags,

			.esp = (uint32_t)user_stack,
		};

		(void)pthread_kern->thread_set_wq_state32(th, (thread_state_t)&state);
	}
#elif defined(__arm__) || defined(__arm64__)
	if (proc_is64bit_data(p)) {
#ifdef __arm64__
		arm_thread_state64_t state = {
			.pc   = (uint64_t)pthread_kern->proc_get_threadstart(p),
			.x[0] = (uint64_t)user_pthread,
			.x[1] = (uint64_t)th_thport,
			.x[2] = (uint64_t)user_func,    /* golang wants this */
			.x[3] = (uint64_t)user_funcarg, /* golang wants this */
			.x[4] = (uint64_t)user_stack,   /* golang wants this */
			.x[5] = (uint64_t)flags,

			.sp   = (uint64_t)user_stack,
		};

		(void)pthread_kern->thread_set_wq_state64(th, (thread_state_t)&state);
#else
		panic("Shouldn't have a 64-bit thread on a 32-bit kernel...");
#endif // defined(__arm64__)
	} else {
		arm_thread_state_t state = {
			.pc   = (uint32_t)pthread_kern->proc_get_threadstart(p),
			.r[0] = (uint32_t)user_pthread,
			.r[1] = (uint32_t)th_thport,
			.r[2] = (uint32_t)user_func,    /* golang wants this */
			.r[3] = (uint32_t)user_funcarg, /* golang wants this */
			.r[4] = (uint32_t)user_stack,   /* golang wants this */
			.r[5] = (uint32_t)flags,

			.sp   = (uint32_t)user_stack,
		};

		(void)pthread_kern->thread_set_wq_state32(th, (thread_state_t)&state);
	}
#else
#error bsdthread_create  not defined for this architecture
#endif

	if (flags & PTHREAD_START_SETSCHED) {
		/* Set scheduling parameters if needed */
		thread_extended_policy_data_t    extinfo;
		thread_precedence_policy_data_t   precedinfo;

		importance = (flags & PTHREAD_START_IMPORTANCE_MASK);
		policy = (flags >> PTHREAD_START_POLICY_BITSHIFT) & PTHREAD_START_POLICY_MASK;

		if (policy == SCHED_OTHER) {
			extinfo.timeshare = 1;
		} else {
			extinfo.timeshare = 0;
		}

		thread_policy_set(th, THREAD_EXTENDED_POLICY, (thread_policy_t)&extinfo, THREAD_EXTENDED_POLICY_COUNT);

		precedinfo.importance = (importance - BASEPRI_DEFAULT);
		thread_policy_set(th, THREAD_PRECEDENCE_POLICY, (thread_policy_t)&precedinfo, THREAD_PRECEDENCE_POLICY_COUNT);
	} else if (flags & PTHREAD_START_QOSCLASS) {
		/* Set thread QoS class if requested. */
		thread_qos_policy_data_t qos;

		if (!_pthread_priority_to_policy(flags & PTHREAD_START_QOSCLASS_MASK, &qos)) {
			error = EINVAL;
			goto out;
		}
		pthread_kern->thread_policy_set_internal(th, THREAD_QOS_POLICY,
				(thread_policy_t)&qos, THREAD_QOS_POLICY_COUNT);
	}

	if (pthread_kern->proc_get_mach_thread_self_tsd_offset) {
		uint64_t mach_thread_self_offset =
				pthread_kern->proc_get_mach_thread_self_tsd_offset(p);
		if (mach_thread_self_offset && tsd_offset) {
			bool proc64bit = proc_is64bit(p);
			if (proc64bit) {
				uint64_t th_thport_tsd = (uint64_t)th_thport;
				error = copyout(&th_thport_tsd, user_pthread + tsd_offset +
						mach_thread_self_offset, sizeof(th_thport_tsd));
			} else {
				uint32_t th_thport_tsd = (uint32_t)th_thport;
				error = copyout(&th_thport_tsd, user_pthread + tsd_offset +
						mach_thread_self_offset, sizeof(th_thport_tsd));
			}
			if (error) {
				goto out;
			}
		}
	}

	if (!start_suspended) {
		kret = pthread_kern->thread_resume(th);
		if (kret != KERN_SUCCESS) {
			error = EINVAL;
			goto out;
		}
	}
	thread_deallocate(th);	/* drop the creator reference */

	PTHREAD_TRACE(pthread_thread_create|DBG_FUNC_END, error, user_pthread, 0, 0);

	*retval = user_pthread;
	return(0);

out:
	(void)pthread_kern->mach_port_deallocate(pthread_kern->task_get_ipcspace(ctask), th_thport);
	if (pthread_kern->thread_will_park_or_terminate) {
		pthread_kern->thread_will_park_or_terminate(th);
	}
	(void)thread_terminate(th);
	(void)thread_deallocate(th);
	return(error);
}

/**
 * bsdthread_terminate system call.  Used by pthread_terminate
 */
int
_bsdthread_terminate(__unused struct proc *p,
		     user_addr_t stackaddr,
		     size_t size,
		     uint32_t kthport,
		     uint32_t sem,
		     __unused int32_t *retval)
{
	mach_vm_offset_t freeaddr;
	mach_vm_size_t freesize;
	kern_return_t kret;
	thread_t th = current_thread();

	freeaddr = (mach_vm_offset_t)stackaddr;
	freesize = size;

	PTHREAD_TRACE(pthread_thread_terminate|DBG_FUNC_START, freeaddr, freesize, kthport, 0xff);

	if ((freesize != (mach_vm_size_t)0) && (freeaddr != (mach_vm_offset_t)0)) {
		if (pthread_kern->thread_get_tag(th) & THREAD_TAG_MAINTHREAD){
			vm_map_t user_map = pthread_kern->current_map();
			freesize = vm_map_trunc_page_mask((vm_map_offset_t)freesize - 1, vm_map_page_mask(user_map));
			kret = mach_vm_behavior_set(user_map, freeaddr, freesize, VM_BEHAVIOR_REUSABLE);
#if MACH_ASSERT
			if (kret != KERN_SUCCESS && kret != KERN_INVALID_ADDRESS) {
				os_log_error(OS_LOG_DEFAULT, "unable to make thread stack reusable (kr: %d)", kret);
			}
#endif
			kret = kret ? kret : mach_vm_protect(user_map, freeaddr, freesize, FALSE, VM_PROT_NONE);
			assert(kret == KERN_SUCCESS || kret == KERN_INVALID_ADDRESS);
		} else {
			kret = mach_vm_deallocate(pthread_kern->current_map(), freeaddr, freesize);
			if (kret != KERN_SUCCESS) {
				PTHREAD_TRACE(pthread_thread_terminate|DBG_FUNC_END, kret, 0, 0, 0);
			}
		}
	}

	if (pthread_kern->thread_will_park_or_terminate) {
		pthread_kern->thread_will_park_or_terminate(th);
	}
	(void)thread_terminate(th);
	if (sem != MACH_PORT_NULL) {
		kret = pthread_kern->semaphore_signal_internal_trap(sem);
		if (kret != KERN_SUCCESS) {
			PTHREAD_TRACE(pthread_thread_terminate|DBG_FUNC_END, kret, 0, 0, 0);
		}
	}

	if (kthport != MACH_PORT_NULL) {
		pthread_kern->mach_port_deallocate(pthread_kern->task_get_ipcspace(current_task()), kthport);
	}

	PTHREAD_TRACE(pthread_thread_terminate|DBG_FUNC_END, 0, 0, 0, 0);

	pthread_kern->thread_exception_return();
	__builtin_unreachable();
}

/**
 * bsdthread_register system call.  Performs per-process setup.  Responsible for
 * returning capabilitiy bits to userspace and receiving userspace function addresses.
 */
int
_bsdthread_register(struct proc *p,
		    user_addr_t threadstart,
		    user_addr_t wqthread,
		    int pthsize,
		    user_addr_t pthread_init_data,
		    user_addr_t pthread_init_data_size,
		    uint64_t dispatchqueue_offset,
		    int32_t *retval)
{
	struct _pthread_registration_data data = {};
	uint32_t max_tsd_offset;
	kern_return_t kr;
	size_t pthread_init_sz = 0;

	/* syscall randomizer test can pass bogus values */
	if (pthsize < 0 || pthsize > MAX_PTHREAD_SIZE) {
		return(EINVAL);
	}
	/*
	 * if we have pthread_init_data, then we use that and target_concptr
	 * (which is an offset) get data.
	 */
	if (pthread_init_data != 0) {
		if (pthread_init_data_size < sizeof(data.version)) {
			return EINVAL;
		}
		pthread_init_sz = MIN(sizeof(data), (size_t)pthread_init_data_size);
		int ret = copyin(pthread_init_data, &data, pthread_init_sz);
		if (ret) {
			return ret;
		}
		if (data.version != (size_t)pthread_init_data_size) {
			return EINVAL;
		}
	} else {
		data.dispatch_queue_offset = dispatchqueue_offset;
	}

	/* We have to do this before proc_get_register so that it resets after fork */
	mach_vm_offset_t stackaddr = stack_addr_hint(p, pthread_kern->current_map());
	pthread_kern->proc_set_stack_addr_hint(p, (user_addr_t)stackaddr);

	/* prevent multiple registrations */
	if (pthread_kern->proc_get_register(p) != 0) {
		return(EINVAL);
	}

	pthread_kern->proc_set_threadstart(p, threadstart);
	pthread_kern->proc_set_wqthread(p, wqthread);
	pthread_kern->proc_set_pthsize(p, pthsize);
	pthread_kern->proc_set_register(p);

	uint32_t tsd_slot_sz = proc_is64bit(p) ? sizeof(uint64_t) : sizeof(uint32_t);
	if ((uint32_t)pthsize >= tsd_slot_sz &&
			data.tsd_offset <= (uint32_t)(pthsize - tsd_slot_sz)) {
		max_tsd_offset = ((uint32_t)pthsize - data.tsd_offset - tsd_slot_sz);
	} else {
		data.tsd_offset = 0;
		max_tsd_offset = 0;
	}
	pthread_kern->proc_set_pthread_tsd_offset(p, data.tsd_offset);

	if (data.dispatch_queue_offset > max_tsd_offset) {
		data.dispatch_queue_offset = 0;
	}
	pthread_kern->proc_set_dispatchqueue_offset(p, data.dispatch_queue_offset);

	if (pthread_kern->proc_set_return_to_kernel_offset) {
		if (data.return_to_kernel_offset > max_tsd_offset) {
			data.return_to_kernel_offset = 0;
		}
		pthread_kern->proc_set_return_to_kernel_offset(p,
				data.return_to_kernel_offset);
	}

	if (pthread_kern->proc_set_mach_thread_self_tsd_offset) {
		if (data.mach_thread_self_offset > max_tsd_offset) {
			data.mach_thread_self_offset = 0;
		}
		pthread_kern->proc_set_mach_thread_self_tsd_offset(p,
				data.mach_thread_self_offset);
	}

	if (pthread_init_data != 0) {
		/* Outgoing data that userspace expects as a reply */
		data.version = sizeof(struct _pthread_registration_data);
		data.main_qos = _pthread_unspecified_priority();

		if (pthread_kern->qos_main_thread_active()) {
			mach_msg_type_number_t nqos = THREAD_QOS_POLICY_COUNT;
			thread_qos_policy_data_t qos;
			boolean_t gd = FALSE;

			kr = pthread_kern->thread_policy_get(current_thread(),
					THREAD_QOS_POLICY, (thread_policy_t)&qos, &nqos, &gd);
			if (kr != KERN_SUCCESS || qos.qos_tier == THREAD_QOS_UNSPECIFIED) {
				/*
				 * Unspecified threads means the kernel wants us
				 * to impose legacy upon the thread.
				 */
				qos.qos_tier = THREAD_QOS_LEGACY;
				qos.tier_importance = 0;

				kr = pthread_kern->thread_policy_set_internal(current_thread(),
						THREAD_QOS_POLICY, (thread_policy_t)&qos,
						THREAD_QOS_POLICY_COUNT);
			}

			if (kr == KERN_SUCCESS) {
				data.main_qos = _pthread_priority_make_from_thread_qos(
						qos.qos_tier, 0, 0);
			}
		}

		data.stack_addr_hint = stackaddr;
		data.mutex_default_policy = pthread_mutex_default_policy;

		kr = copyout(&data, pthread_init_data, pthread_init_sz);
		if (kr != KERN_SUCCESS) {
			return EINVAL;
		}
	}

	/* return the supported feature set as the return value. */
	*retval = PTHREAD_FEATURE_SUPPORTED;

	return(0);
}


#pragma mark - Workqueue Thread Support

static mach_vm_size_t
workq_thread_allocsize(proc_t p, vm_map_t wq_map,
		mach_vm_size_t *guardsize_out)
{
	mach_vm_size_t guardsize = vm_map_page_size(wq_map);
	mach_vm_size_t pthread_size = vm_map_round_page_mask(
			pthread_kern->proc_get_pthsize(p) + PTHREAD_T_OFFSET,
			vm_map_page_mask(wq_map));
	if (guardsize_out) *guardsize_out = guardsize;
	return guardsize + PTH_DEFAULT_STACKSIZE + pthread_size;
}

int
workq_create_threadstack(proc_t p, vm_map_t vmap, mach_vm_offset_t *out_addr)
{
	mach_vm_offset_t stackaddr = pthread_kern->proc_get_stack_addr_hint(p);
	mach_vm_size_t guardsize, th_allocsize;
	kern_return_t kret;

	th_allocsize = workq_thread_allocsize(p, vmap, &guardsize);
	kret = mach_vm_map(vmap, &stackaddr, th_allocsize, page_size - 1,
			VM_MAKE_TAG(VM_MEMORY_STACK) | VM_FLAGS_ANYWHERE, NULL, 0, FALSE,
			VM_PROT_DEFAULT, VM_PROT_ALL, VM_INHERIT_DEFAULT);

	if (kret != KERN_SUCCESS) {
		kret = mach_vm_allocate(vmap, &stackaddr, th_allocsize,
				VM_MAKE_TAG(VM_MEMORY_STACK) | VM_FLAGS_ANYWHERE);
	}

	if (kret != KERN_SUCCESS) {
		goto fail;
	}

	/*
	 * The guard page is at the lowest address
	 * The stack base is the highest address
	 */
	kret = mach_vm_protect(vmap, stackaddr, guardsize, FALSE, VM_PROT_NONE);
	if (kret != KERN_SUCCESS) {
		goto fail_vm_deallocate;
	}

	if (out_addr) {
		*out_addr = stackaddr;
	}
	return 0;

fail_vm_deallocate:
	(void)mach_vm_deallocate(vmap, stackaddr, th_allocsize);
fail:
	return kret;
}

int
workq_destroy_threadstack(proc_t p, vm_map_t vmap, mach_vm_offset_t stackaddr)
{
	return mach_vm_deallocate(vmap, stackaddr,
			workq_thread_allocsize(p, vmap, NULL));
}

void
workq_markfree_threadstack(proc_t OS_UNUSED p, thread_t OS_UNUSED th,
		vm_map_t vmap, user_addr_t stackaddr)
{
	// Keep this in sync with workq_setup_thread()
	const vm_size_t       guardsize = vm_map_page_size(vmap);
	const user_addr_t     freeaddr = (user_addr_t)stackaddr + guardsize;
	const vm_map_offset_t freesize = vm_map_trunc_page_mask(
			(PTH_DEFAULT_STACKSIZE + guardsize + PTHREAD_T_OFFSET) - 1,
			vm_map_page_mask(vmap)) - guardsize;

	__assert_only kern_return_t kr = mach_vm_behavior_set(vmap, freeaddr,
			freesize, VM_BEHAVIOR_REUSABLE);
#if MACH_ASSERT
	if (kr != KERN_SUCCESS && kr != KERN_INVALID_ADDRESS) {
		os_log_error(OS_LOG_DEFAULT, "unable to make thread stack reusable (kr: %d)", kr);
	}
#endif
}

struct workq_thread_addrs {
	user_addr_t self;
	user_addr_t stack_bottom;
	user_addr_t stack_top;
};

static inline void
workq_thread_set_top_addr(struct workq_thread_addrs *th_addrs, user_addr_t addr)
{
	th_addrs->stack_top = (addr & -C_WORKQ_STK_ALIGN);
}

static void
workq_thread_get_addrs(vm_map_t map, user_addr_t stackaddr,
					   struct workq_thread_addrs *th_addrs)
{
	const vm_size_t guardsize = vm_map_page_size(map);

	th_addrs->self = (user_addr_t)(stackaddr + PTH_DEFAULT_STACKSIZE +
			guardsize + PTHREAD_T_OFFSET);
	workq_thread_set_top_addr(th_addrs, th_addrs->self);
	th_addrs->stack_bottom = (user_addr_t)(stackaddr + guardsize);
}

static inline void
workq_set_register_state(proc_t p, thread_t th,
		struct workq_thread_addrs *addrs, mach_port_name_t kport,
		user_addr_t kevent_list, uint32_t upcall_flags, int kevent_count)
{
	user_addr_t wqstart_fnptr = pthread_kern->proc_get_wqthread(p);
	if (!wqstart_fnptr) {
		panic("workqueue thread start function pointer is NULL");
	}

#if defined(__i386__) || defined(__x86_64__)
	if (proc_is64bit_data(p) == 0) {
		x86_thread_state32_t state = {
			.eip = (unsigned int)wqstart_fnptr,
			.eax = /* arg0 */ (unsigned int)addrs->self,
			.ebx = /* arg1 */ (unsigned int)kport,
			.ecx = /* arg2 */ (unsigned int)addrs->stack_bottom,
			.edx = /* arg3 */ (unsigned int)kevent_list,
			.edi = /* arg4 */ (unsigned int)upcall_flags,
			.esi = /* arg5 */ (unsigned int)kevent_count,

			.esp = (int)((vm_offset_t)addrs->stack_top),
		};

		int error = pthread_kern->thread_set_wq_state32(th, (thread_state_t)&state);
		if (error != KERN_SUCCESS) {
			panic(__func__ ": thread_set_wq_state failed: %d", error);
		}
	} else {
		x86_thread_state64_t state64 = {
			// x86-64 already passes all the arguments in registers, so we just put them in their final place here
			.rip = (uint64_t)wqstart_fnptr,
			.rdi = (uint64_t)addrs->self,
			.rsi = (uint64_t)kport,
			.rdx = (uint64_t)addrs->stack_bottom,
			.rcx = (uint64_t)kevent_list,
			.r8  = (uint64_t)upcall_flags,
			.r9  = (uint64_t)kevent_count,

			.rsp = (uint64_t)(addrs->stack_top)
		};

		int error = pthread_kern->thread_set_wq_state64(th, (thread_state_t)&state64);
		if (error != KERN_SUCCESS) {
			panic(__func__ ": thread_set_wq_state failed: %d", error);
		}
	}
#elif defined(__arm__) || defined(__arm64__)
	if (!proc_is64bit_data(p)) {
		arm_thread_state_t state = {
			.pc = (int)wqstart_fnptr,
			.r[0] = (unsigned int)addrs->self,
			.r[1] = (unsigned int)kport,
			.r[2] = (unsigned int)addrs->stack_bottom,
			.r[3] = (unsigned int)kevent_list,
			// will be pushed onto the stack as arg4/5
			.r[4] = (unsigned int)upcall_flags,
			.r[5] = (unsigned int)kevent_count,

			.sp = (int)(addrs->stack_top)
		};

		int error = pthread_kern->thread_set_wq_state32(th, (thread_state_t)&state);
		if (error != KERN_SUCCESS) {
			panic(__func__ ": thread_set_wq_state failed: %d", error);
		}
	} else {
#if defined(__arm64__)
		arm_thread_state64_t state = {
			.pc = (uint64_t)wqstart_fnptr,
			.x[0] = (uint64_t)addrs->self,
			.x[1] = (uint64_t)kport,
			.x[2] = (uint64_t)addrs->stack_bottom,
			.x[3] = (uint64_t)kevent_list,
			.x[4] = (uint64_t)upcall_flags,
			.x[5] = (uint64_t)kevent_count,

			.sp = (uint64_t)((vm_offset_t)addrs->stack_top),
		};

		int error = pthread_kern->thread_set_wq_state64(th, (thread_state_t)&state);
		if (error != KERN_SUCCESS) {
			panic(__func__ ": thread_set_wq_state failed: %d", error);
		}
#else /* defined(__arm64__) */
		panic("Shouldn't have a 64-bit thread on a 32-bit kernel...");
#endif /* defined(__arm64__) */
	}
#else
#error setup_wqthread  not defined for this architecture
#endif
}

static inline int
workq_kevent(proc_t p, struct workq_thread_addrs *th_addrs,
		user_addr_t eventlist, int nevents, int kevent_flags,
		user_addr_t *kevent_list_out, int *kevent_count_out)
{
	int ret;

	user_addr_t kevent_list = th_addrs->self -
			WQ_KEVENT_LIST_LEN * sizeof(struct kevent_qos_s);
	user_addr_t data_buf = kevent_list - WQ_KEVENT_DATA_SIZE;
	user_size_t data_available = WQ_KEVENT_DATA_SIZE;

	ret = pthread_kern->kevent_workq_internal(p, eventlist, nevents,
			kevent_list, WQ_KEVENT_LIST_LEN,
			data_buf, &data_available,
			kevent_flags, kevent_count_out);

	// squash any errors into just empty output
	if (ret != 0 || *kevent_count_out == -1) {
		*kevent_list_out = NULL;
		*kevent_count_out = 0;
		return ret;
	}

	workq_thread_set_top_addr(th_addrs, data_buf + data_available);
	*kevent_list_out = kevent_list;
	return ret;
}

/**
 * configures initial thread stack/registers to jump into:
 * _pthread_wqthread(pthread_t self, mach_port_t kport, void *stackaddr, void *keventlist, int upcall_flags, int nkevents);
 * to get there we jump through assembily stubs in pthread_asm.s.  Those
 * routines setup a stack frame, using the current stack pointer, and marshall
 * arguments from registers to the stack as required by the ABI.
 *
 * One odd thing we do here is to start the pthread_t 4k below what would be the
 * top of the stack otherwise.  This is because usually only the first 4k of the
 * pthread_t will be used and so we want to put it on the same 16k page as the
 * top of the stack to save memory.
 *
 * When we are done the stack will look like:
 * |-----------| th_stackaddr + th_allocsize
 * |pthread_t  | th_stackaddr + DEFAULT_STACKSIZE + guardsize + PTHREAD_STACK_OFFSET
 * |kevent list| optionally - at most WQ_KEVENT_LIST_LEN events
 * |kevent data| optionally - at most WQ_KEVENT_DATA_SIZE bytes
 * |stack gap  | bottom aligned to 16 bytes
 * |   STACK   |
 * |     ⇓     |
 * |           |
 * |guard page | guardsize
 * |-----------| th_stackaddr
 */
__attribute__((noreturn,noinline))
void
workq_setup_thread(proc_t p, thread_t th, vm_map_t map, user_addr_t stackaddr,
		mach_port_name_t kport, int th_qos __unused, int setup_flags, int upcall_flags)
{
	struct workq_thread_addrs th_addrs;
	bool first_use = (setup_flags & WQ_SETUP_FIRST_USE);
	user_addr_t kevent_list = NULL;
	int kevent_count = 0;

	workq_thread_get_addrs(map, stackaddr, &th_addrs);

	if (first_use) {
		uint32_t tsd_offset = pthread_kern->proc_get_pthread_tsd_offset(p);
		if (tsd_offset) {
			mach_vm_offset_t th_tsd_base = th_addrs.self + tsd_offset;
			kern_return_t kret = pthread_kern->thread_set_tsd_base(th,
					th_tsd_base);
			if (kret == KERN_SUCCESS) {
				upcall_flags |= WQ_FLAG_THREAD_TSD_BASE_SET;
			}
		}

		/*
		 * Pre-fault the first page of the new thread's stack and the page that will
		 * contain the pthread_t structure.
		 */
		vm_map_offset_t mask = vm_map_page_mask(map);
		vm_map_offset_t th_page = vm_map_trunc_page_mask(th_addrs.self, mask);
		vm_map_offset_t stk_page = vm_map_trunc_page_mask(th_addrs.stack_top - 1, mask);
		if (th_page != stk_page) {
			vm_fault(map, stk_page, VM_PROT_READ | VM_PROT_WRITE, FALSE, THREAD_UNINT, NULL, 0);
		}
		vm_fault(map, th_page, VM_PROT_READ | VM_PROT_WRITE, FALSE, THREAD_UNINT, NULL, 0);
	}

	if (setup_flags & WQ_SETUP_EXIT_THREAD) {
		kevent_count = WORKQ_EXIT_THREAD_NKEVENT;
	} else if (upcall_flags & WQ_FLAG_THREAD_KEVENT) {
		unsigned int flags = KEVENT_FLAG_STACK_DATA | KEVENT_FLAG_IMMEDIATE;
		workq_kevent(p, &th_addrs, NULL, 0, flags, &kevent_list, &kevent_count);
	}

	workq_set_register_state(p, th, &th_addrs, kport,
			kevent_list, upcall_flags, kevent_count);

	if (first_use) {
		pthread_kern->thread_bootstrap_return();
	} else {
		pthread_kern->unix_syscall_return(EJUSTRETURN);
	}
	__builtin_unreachable();
}

int
workq_handle_stack_events(proc_t p, thread_t th, vm_map_t map,
		user_addr_t stackaddr, mach_port_name_t kport,
		user_addr_t events, int nevents, int upcall_flags)
{
	struct workq_thread_addrs th_addrs;
	user_addr_t kevent_list = NULL;
	int kevent_count = 0, error;
	__assert_only kern_return_t kr;

	workq_thread_get_addrs(map, stackaddr, &th_addrs);

	unsigned int flags = KEVENT_FLAG_STACK_DATA | KEVENT_FLAG_IMMEDIATE |
			KEVENT_FLAG_PARKING;
	error = workq_kevent(p, &th_addrs, events, nevents, flags,
			&kevent_list, &kevent_count);

	if (error || kevent_count == 0) {
		return error;
	}

	kr = pthread_kern->thread_set_voucher_name(MACH_PORT_NULL);
	assert(kr == KERN_SUCCESS);

	workq_set_register_state(p, th, &th_addrs, kport,
			kevent_list, upcall_flags, kevent_count);

	pthread_kern->unix_syscall_return(EJUSTRETURN);
	__builtin_unreachable();
}

int
_thread_selfid(__unused struct proc *p, uint64_t *retval)
{
	thread_t thread = current_thread();
	*retval = thread_tid(thread);
	return KERN_SUCCESS;
}

void
_pthread_init(void)
{
	pthread_lck_grp_attr = lck_grp_attr_alloc_init();
	pthread_lck_grp = lck_grp_alloc_init("pthread", pthread_lck_grp_attr);

	/*
	 * allocate the lock attribute for pthread synchronizers
	 */
	pthread_lck_attr = lck_attr_alloc_init();
	pthread_list_mlock = lck_mtx_alloc_init(pthread_lck_grp, pthread_lck_attr);

	pth_global_hashinit();
	psynch_thcall = thread_call_allocate(psynch_wq_cleanup, NULL);
	psynch_zoneinit();

	int policy_bootarg;
	if (PE_parse_boot_argn("pthread_mutex_default_policy", &policy_bootarg, sizeof(policy_bootarg))) {
		pthread_mutex_default_policy = policy_bootarg;
	}

	sysctl_register_oid(&sysctl__kern_pthread_mutex_default_policy);
}
