/*
 * Copyright (c) 2000-2013 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */
/*
 * Copyright 1996 1995 by Open Software Foundation, Inc. 1997 1996 1995 1994 1993 1992 1991
 *              All Rights Reserved
 *
 * Permission to use, copy, modify, and distribute this software and
 * its documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appears in all copies and
 * that both the copyright notice and this permission notice appear in
 * supporting documentation.
 *
 * OSF DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE.
 *
 * IN NO EVENT SHALL OSF BE LIABLE FOR ANY SPECIAL, INDIRECT, OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN ACTION OF CONTRACT,
 * NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */
/*
 * MkLinux
 */

/*
 * POSIX Pthread Library
 */

#include "resolver.h"
#include "internal.h"
#include "private.h"
#include "workqueue_private.h"
#include "introspection_private.h"
#include "qos_private.h"
#include "tsd_private.h"
#include "pthread/stack_np.h"
#include "offsets.h" // included to validate the offsets at build time

#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <mach/mach_init.h>
#include <mach/mach_vm.h>
#include <mach/mach_sync_ipc.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/sysctl.h>
#include <sys/queue.h>
#include <sys/ulock.h>
#include <sys/mman.h>
#include <machine/vmparam.h>
#define	__APPLE_API_PRIVATE
#include <machine/cpu_capabilities.h>
#if __has_include(<ptrauth.h>)
#include <ptrauth.h>
#endif // __has_include(<ptrauth.h>)

#include <_simple.h>
#include <platform/string.h>
#include <platform/compat.h>

#include <stack_logging.h>

// Defined in libsyscall; initialized in libmalloc
extern malloc_logger_t *__syscall_logger;

extern int __sysctl(int *name, u_int namelen, void *oldp, size_t *oldlenp,
		void *newp, size_t newlen);
extern void __exit(int) __attribute__((noreturn));
extern int __pthread_kill(mach_port_t, int);

extern void _pthread_joiner_wake(pthread_t thread);

#if !VARIANT_DYLD
PTHREAD_NOEXPORT extern struct _pthread *_main_thread_ptr;
#define main_thread() (_main_thread_ptr)
#endif // VARIANT_DYLD

// Default stack size is 512KB; independent of the main thread's stack size.
#define DEFAULT_STACK_SIZE (size_t)(512 * 1024)


//
// Global constants
//

/*
 * The pthread may be offset into a page.  In that event, by contract
 * with the kernel, the allocation will extend PTHREAD_SIZE from the
 * start of the next page.  There's also one page worth of allocation
 * below stacksize for the guard page. <rdar://problem/19941744>
 */
#define PTHREAD_SIZE ((size_t)mach_vm_round_page(sizeof(struct _pthread)))
#define PTHREAD_ALLOCADDR(stackaddr, stacksize) ((stackaddr - stacksize) - vm_page_size)
#define PTHREAD_ALLOCSIZE(stackaddr, stacksize) ((round_page((uintptr_t)stackaddr) + PTHREAD_SIZE) - (uintptr_t)PTHREAD_ALLOCADDR(stackaddr, stacksize))

static const pthread_attr_t _pthread_attr_default = {
	.sig       = _PTHREAD_ATTR_SIG,
	.stacksize = 0,
	.detached  = PTHREAD_CREATE_JOINABLE,
	.inherit   = _PTHREAD_DEFAULT_INHERITSCHED,
	.policy    = _PTHREAD_DEFAULT_POLICY,
	.defaultguardpage = true,
	// compile time constant for _pthread_default_priority(0)
	.qosclass  = (1U << (THREAD_QOS_LEGACY - 1 + _PTHREAD_PRIORITY_QOS_CLASS_SHIFT)) |
			((uint8_t)-1 & _PTHREAD_PRIORITY_PRIORITY_MASK),
};

#if PTHREAD_LAYOUT_SPI

const struct pthread_layout_offsets_s pthread_layout_offsets = {
	.plo_version = 1,
	.plo_pthread_tsd_base_offset = offsetof(struct _pthread, tsd),
	.plo_pthread_tsd_base_address_offset = 0,
	.plo_pthread_tsd_entry_size = sizeof(((struct _pthread *)NULL)->tsd[0]),
};

#endif // PTHREAD_LAYOUT_SPI

//
// Global exported variables
//

// This global should be used (carefully) by anyone needing to know if a
// pthread (other than the main thread) has been created.
int __is_threaded = 0;
int __unix_conforming = 0;

//
// Global internal variables
//

// _pthread_list_lock protects _pthread_count, access to the __pthread_head
// list. Externally imported by pthread_cancelable.c.
struct __pthread_list __pthread_head = TAILQ_HEAD_INITIALIZER(__pthread_head);
_pthread_lock _pthread_list_lock = _PTHREAD_LOCK_INITIALIZER;

uint32_t _main_qos;

#if VARIANT_DYLD
// The main thread's pthread_t
struct _pthread _main_thread __attribute__((aligned(64))) = { };
#define main_thread() (&_main_thread)
#else // VARIANT_DYLD
struct _pthread *_main_thread_ptr;
#endif // VARIANT_DYLD

#if PTHREAD_DEBUG_LOG
#include <fcntl.h>
int _pthread_debuglog;
uint64_t _pthread_debugstart;
#endif

//
// Global static variables
//
static bool __workq_newapi;
static uint8_t default_priority;
#if !VARIANT_DYLD
static uint8_t max_priority;
static uint8_t min_priority;
#endif // !VARIANT_DYLD
static int _pthread_count = 1;
static int pthread_concurrency;
uintptr_t _pthread_ptr_munge_token;

static void (*exitf)(int) = __exit;
#if !VARIANT_DYLD
static void *(*_pthread_malloc)(size_t) = NULL;
static void (*_pthread_free)(void *) = NULL;
#endif // !VARIANT_DYLD

// work queue support data
PTHREAD_NORETURN
static void
__pthread_invalid_keventfunction(void **events, int *nevents)
{
	PTHREAD_CLIENT_CRASH(0, "Invalid kqworkq setup");
}

PTHREAD_NORETURN
static void
__pthread_invalid_workloopfunction(uint64_t *workloop_id, void **events, int *nevents)
{
	PTHREAD_CLIENT_CRASH(0, "Invalid kqwl setup");
}
static pthread_workqueue_function2_t __libdispatch_workerfunction;
static pthread_workqueue_function_kevent_t __libdispatch_keventfunction = &__pthread_invalid_keventfunction;
static pthread_workqueue_function_workloop_t __libdispatch_workloopfunction = &__pthread_invalid_workloopfunction;
static int __pthread_supported_features; // supported feature set

#if defined(__i386__) || defined(__x86_64__)
static mach_vm_address_t __pthread_stack_hint = 0xB0000000;
#elif defined(__arm__) || defined(__arm64__)
static mach_vm_address_t __pthread_stack_hint = 0x30000000;
#else
#error no __pthread_stack_hint for this architecture
#endif

//
// Function prototypes
//

// pthread primitives
static inline void _pthread_struct_init(pthread_t t, const pthread_attr_t *attrs,
		void *stack, size_t stacksize, void *freeaddr, size_t freesize);

#if VARIANT_DYLD
static void _pthread_set_self_dyld(void);
#endif // VARIANT_DYLD
static inline void _pthread_set_self_internal(pthread_t);

static void _pthread_dealloc_reply_port(pthread_t t);
static void _pthread_dealloc_special_reply_port(pthread_t t);

static inline void __pthread_started_thread(pthread_t t);

static void _pthread_exit(pthread_t self, void *value_ptr) __dead2;

static inline void _pthread_introspection_thread_create(pthread_t t);
static inline void _pthread_introspection_thread_start(pthread_t t);
static inline void _pthread_introspection_thread_terminate(pthread_t t);
static inline void _pthread_introspection_thread_destroy(pthread_t t);

extern void _pthread_set_self(pthread_t);
extern void start_wqthread(pthread_t self, mach_port_t kport, void *stackaddr, void *unused, int reuse); // trampoline into _pthread_wqthread
extern void thread_start(pthread_t self, mach_port_t kport, void *(*fun)(void *), void * funarg, size_t stacksize, unsigned int flags); // trampoline into _pthread_start

/*
 * Flags filed passed to bsdthread_create and back in pthread_start
 * 31  <---------------------------------> 0
 * _________________________________________
 * | flags(8) | policy(8) | importance(16) |
 * -----------------------------------------
 */
#define PTHREAD_START_CUSTOM		0x01000000 // <rdar://problem/34501401>
#define PTHREAD_START_SETSCHED		0x02000000
// was PTHREAD_START_DETACHED		0x04000000
#define PTHREAD_START_QOSCLASS		0x08000000
#define PTHREAD_START_TSD_BASE_SET	0x10000000
#define PTHREAD_START_SUSPENDED		0x20000000
#define PTHREAD_START_QOSCLASS_MASK 0x00ffffff
#define PTHREAD_START_POLICY_BITSHIFT 16
#define PTHREAD_START_POLICY_MASK 0xff
#define PTHREAD_START_IMPORTANCE_MASK 0xffff

extern pthread_t __bsdthread_create(void *(*func)(void *), void * func_arg, void * stack, pthread_t  thread, unsigned int flags);
extern int __bsdthread_register(void (*)(pthread_t, mach_port_t, void *(*)(void *), void *, size_t, unsigned int), void (*)(pthread_t, mach_port_t, void *, void *, int), int,void (*)(pthread_t, mach_port_t, void *(*)(void *), void *, size_t, unsigned int), int32_t *,__uint64_t);
extern int __bsdthread_terminate(void * freeaddr, size_t freesize, mach_port_t kport, mach_port_t joinsem);
extern __uint64_t __thread_selfid( void );

#if __LP64__
_Static_assert(offsetof(struct _pthread, tsd) == 224, "TSD LP64 offset");
#else
_Static_assert(offsetof(struct _pthread, tsd) == 176, "TSD ILP32 offset");
#endif
_Static_assert(offsetof(struct _pthread, tsd) + _PTHREAD_STRUCT_DIRECT_THREADID_OFFSET
		== offsetof(struct _pthread, thread_id),
		"_PTHREAD_STRUCT_DIRECT_THREADID_OFFSET is correct");

#pragma mark pthread attrs

_Static_assert(sizeof(struct _pthread_attr_t) == sizeof(__darwin_pthread_attr_t),
		"internal pthread_attr_t == external pthread_attr_t");

int
pthread_attr_destroy(pthread_attr_t *attr)
{
	int ret = EINVAL;
	if (attr->sig == _PTHREAD_ATTR_SIG) {
		attr->sig = 0;
		ret = 0;
	}
	return ret;
}

int
pthread_attr_getdetachstate(const pthread_attr_t *attr, int *detachstate)
{
	int ret = EINVAL;
	if (attr->sig == _PTHREAD_ATTR_SIG) {
		*detachstate = attr->detached;
		ret = 0;
	}
	return ret;
}

int
pthread_attr_getinheritsched(const pthread_attr_t *attr, int *inheritsched)
{
	int ret = EINVAL;
	if (attr->sig == _PTHREAD_ATTR_SIG) {
		*inheritsched = attr->inherit;
		ret = 0;
	}
	return ret;
}

static PTHREAD_ALWAYS_INLINE void
_pthread_attr_get_schedparam(const pthread_attr_t *attr,
		struct sched_param *param)
{
	if (attr->schedset) {
		*param = attr->param;
	} else {
		param->sched_priority = default_priority;
		param->quantum = 10; /* quantum isn't public yet */
	}
}

int
pthread_attr_getschedparam(const pthread_attr_t *attr, struct sched_param *param)
{
	int ret = EINVAL;
	if (attr->sig == _PTHREAD_ATTR_SIG) {
		_pthread_attr_get_schedparam(attr, param);
		ret = 0;
	}
	return ret;
}

int
pthread_attr_getschedpolicy(const pthread_attr_t *attr, int *policy)
{
	int ret = EINVAL;
	if (attr->sig == _PTHREAD_ATTR_SIG) {
		*policy = attr->policy;
		ret = 0;
	}
	return ret;
}

int
pthread_attr_init(pthread_attr_t *attr)
{
	*attr = _pthread_attr_default;
	return 0;
}

int
pthread_attr_setdetachstate(pthread_attr_t *attr, int detachstate)
{
	int ret = EINVAL;
	if (attr->sig == _PTHREAD_ATTR_SIG &&
			(detachstate == PTHREAD_CREATE_JOINABLE ||
			detachstate == PTHREAD_CREATE_DETACHED)) {
		attr->detached = detachstate;
		ret = 0;
	}
	return ret;
}

int
pthread_attr_setinheritsched(pthread_attr_t *attr, int inheritsched)
{
	int ret = EINVAL;
	if (attr->sig == _PTHREAD_ATTR_SIG &&
			(inheritsched == PTHREAD_INHERIT_SCHED ||
			inheritsched == PTHREAD_EXPLICIT_SCHED)) {
		attr->inherit = inheritsched;
		ret = 0;
	}
	return ret;
}

int
pthread_attr_setschedparam(pthread_attr_t *attr, const struct sched_param *param)
{
	int ret = EINVAL;
	if (attr->sig == _PTHREAD_ATTR_SIG) {
		/* TODO: Validate sched_param fields */
		attr->param = *param;
		attr->schedset = 1;
		ret = 0;
	}
	return ret;
}

int
pthread_attr_setschedpolicy(pthread_attr_t *attr, int policy)
{
	int ret = EINVAL;
	if (attr->sig == _PTHREAD_ATTR_SIG && (policy == SCHED_OTHER ||
			policy == SCHED_RR || policy == SCHED_FIFO)) {
		if (!_PTHREAD_POLICY_IS_FIXEDPRI(policy)) {
			/* non-fixedpri policy should remove cpupercent */
			attr->cpupercentset = 0;
		}
		attr->policy = policy;
		attr->policyset = 1;
		ret = 0;
	}
	return ret;
}

int
pthread_attr_setscope(pthread_attr_t *attr, int scope)
{
	int ret = EINVAL;
	if (attr->sig == _PTHREAD_ATTR_SIG) {
		if (scope == PTHREAD_SCOPE_SYSTEM) {
			// No attribute yet for the scope.
			ret = 0;
		} else if (scope == PTHREAD_SCOPE_PROCESS) {
			ret = ENOTSUP;
		}
	}
	return ret;
}

int
pthread_attr_getscope(const pthread_attr_t *attr, int *scope)
{
	int ret = EINVAL;
	if (attr->sig == _PTHREAD_ATTR_SIG) {
		*scope = PTHREAD_SCOPE_SYSTEM;
		ret = 0;
	}
	return ret;
}

int
pthread_attr_getstackaddr(const pthread_attr_t *attr, void **stackaddr)
{
	int ret = EINVAL;
	if (attr->sig == _PTHREAD_ATTR_SIG) {
		*stackaddr = attr->stackaddr;
		ret = 0;
	}
	return ret;
}

int
pthread_attr_setstackaddr(pthread_attr_t *attr, void *stackaddr)
{
	int ret = EINVAL;
	if (attr->sig == _PTHREAD_ATTR_SIG &&
			((uintptr_t)stackaddr % vm_page_size) == 0) {
		attr->stackaddr = stackaddr;
		attr->defaultguardpage = false;
		attr->guardsize = 0;
		ret = 0;
	}
	return ret;
}

static inline size_t
_pthread_attr_stacksize(const pthread_attr_t *attr)
{
	return attr->stacksize ? attr->stacksize : DEFAULT_STACK_SIZE;
}

int
pthread_attr_getstacksize(const pthread_attr_t *attr, size_t *stacksize)
{
	int ret = EINVAL;
	if (attr->sig == _PTHREAD_ATTR_SIG) {
		*stacksize = _pthread_attr_stacksize(attr);
		ret = 0;
	}
	return ret;
}

int
pthread_attr_setstacksize(pthread_attr_t *attr, size_t stacksize)
{
	int ret = EINVAL;
	if (attr->sig == _PTHREAD_ATTR_SIG &&
			(stacksize % vm_page_size) == 0 &&
			stacksize >= PTHREAD_STACK_MIN) {
		attr->stacksize = stacksize;
		ret = 0;
	}
	return ret;
}

int
pthread_attr_getstack(const pthread_attr_t *attr, void **stackaddr, size_t * stacksize)
{
	int ret = EINVAL;
	if (attr->sig == _PTHREAD_ATTR_SIG) {
		*stackaddr = (void *)((uintptr_t)attr->stackaddr - attr->stacksize);
		*stacksize = _pthread_attr_stacksize(attr);
		ret = 0;
	}
	return ret;
}

// Per SUSv3, the stackaddr is the base address, the lowest addressable byte
// address. This is not the same as in pthread_attr_setstackaddr.
int
pthread_attr_setstack(pthread_attr_t *attr, void *stackaddr, size_t stacksize)
{
	int ret = EINVAL;
	if (attr->sig == _PTHREAD_ATTR_SIG &&
			((uintptr_t)stackaddr % vm_page_size) == 0 &&
			(stacksize % vm_page_size) == 0 &&
			stacksize >= PTHREAD_STACK_MIN) {
		attr->stackaddr = (void *)((uintptr_t)stackaddr + stacksize);
		attr->stacksize = stacksize;
		ret = 0;
	}
	return ret;
}

int
pthread_attr_setguardsize(pthread_attr_t *attr, size_t guardsize)
{
	int ret = EINVAL;
	if (attr->sig == _PTHREAD_ATTR_SIG && (guardsize % vm_page_size) == 0) {
		/* Guardsize of 0 is valid, means no guard */
		attr->defaultguardpage = false;
		attr->guardsize = guardsize;
		ret = 0;
	}
	return ret;
}

static inline size_t
_pthread_attr_guardsize(const pthread_attr_t *attr)
{
	return attr->defaultguardpage ? vm_page_size : attr->guardsize;
}

int
pthread_attr_getguardsize(const pthread_attr_t *attr, size_t *guardsize)
{
	int ret = EINVAL;
	if (attr->sig == _PTHREAD_ATTR_SIG) {
		*guardsize = _pthread_attr_guardsize(attr);
		ret = 0;
	}
	return ret;
}

int
pthread_attr_setcpupercent_np(pthread_attr_t *attr, int percent,
		unsigned long refillms)
{
	int ret = EINVAL;
	if (attr->sig == _PTHREAD_ATTR_SIG && percent < UINT8_MAX &&
			refillms < _PTHREAD_ATTR_REFILLMS_MAX && attr->policyset &&
			_PTHREAD_POLICY_IS_FIXEDPRI(attr->policy)) {
		attr->cpupercent = percent;
		attr->refillms = (uint32_t)(refillms & 0x00ffffff);
		attr->cpupercentset = 1;
		ret = 0;
	}
	return ret;
}

#pragma mark pthread lifetime

// Allocate a thread structure, stack and guard page.
//
// The thread structure may optionally be placed in the same allocation as the
// stack, residing above the top of the stack. This cannot be done if a
// custom stack address is provided.
//
// Similarly the guard page cannot be allocated if a custom stack address is
// provided.
//
// The allocated thread structure is initialized with values that indicate how
// it should be freed.

static pthread_t
_pthread_allocate(const pthread_attr_t *attrs, void **stack,
		bool from_mach_thread)
{
	mach_vm_address_t allocaddr = __pthread_stack_hint;
	size_t allocsize, guardsize, stacksize, pthreadoff;
	kern_return_t kr;
	pthread_t t;

	if (os_unlikely(attrs->stacksize != 0 &&
			attrs->stacksize < PTHREAD_STACK_MIN)) {
		PTHREAD_CLIENT_CRASH(attrs->stacksize, "Stack size in attrs is too small");
	}

	if (os_unlikely(((uintptr_t)attrs->stackaddr % vm_page_size) != 0)) {
		PTHREAD_CLIENT_CRASH(attrs->stacksize, "Unaligned stack addr in attrs");
	}

	// Allocate a pthread structure if necessary

	if (attrs->stackaddr != NULL) {
		allocsize = PTHREAD_SIZE;
		guardsize = 0;
		pthreadoff = 0;
		// <rdar://problem/42588315> if the attrs struct specifies a custom
		// stack address but not a custom size, using ->stacksize here instead
		// of _pthread_attr_stacksize stores stacksize as zero, indicating
		// that the stack size is unknown.
		stacksize = attrs->stacksize;
	} else {
		guardsize = _pthread_attr_guardsize(attrs);
		stacksize = _pthread_attr_stacksize(attrs) + PTHREAD_T_OFFSET;
		pthreadoff = stacksize + guardsize;
		allocsize = pthreadoff + PTHREAD_SIZE;
		allocsize = mach_vm_round_page(allocsize);
	}

	kr = mach_vm_map(mach_task_self(), &allocaddr, allocsize, vm_page_size - 1,
			 VM_MAKE_TAG(VM_MEMORY_STACK)| VM_FLAGS_ANYWHERE, MEMORY_OBJECT_NULL,
			 0, FALSE, VM_PROT_DEFAULT, VM_PROT_ALL, VM_INHERIT_DEFAULT);

	if (kr != KERN_SUCCESS) {
		kr = mach_vm_allocate(mach_task_self(), &allocaddr, allocsize,
				 VM_MAKE_TAG(VM_MEMORY_STACK)| VM_FLAGS_ANYWHERE);
	} else if (__syscall_logger && !from_mach_thread) {
		// libsyscall will not output malloc stack logging events when
		// VM_MEMORY_STACK is passed in to facilitate mach thread promotion.
		// To avoid losing the stack traces for normal p-thread create
		// operations, libpthread must pretend to be the vm syscall and log
		// the allocations. <rdar://36418708>
		int eventTypeFlags = stack_logging_type_vm_allocate |
				stack_logging_type_mapped_file_or_shared_mem;
		__syscall_logger(eventTypeFlags | VM_MAKE_TAG(VM_MEMORY_STACK),
				(uintptr_t)mach_task_self(), (uintptr_t)allocsize, 0,
				(uintptr_t)allocaddr, 0);
	}

	if (kr != KERN_SUCCESS) {
		*stack  = NULL;
		return NULL;
	} else if (__syscall_logger && !from_mach_thread) {
		// libsyscall will not output malloc stack logging events when
		// VM_MEMORY_STACK is passed in to facilitate mach thread promotion.
		// To avoid losing the stack traces for normal p-thread create
		// operations, libpthread must pretend to be the vm syscall and log
		// the allocations. <rdar://36418708>
		int eventTypeFlags = stack_logging_type_vm_allocate;
		__syscall_logger(eventTypeFlags | VM_MAKE_TAG(VM_MEMORY_STACK),
						 (uintptr_t)mach_task_self(), (uintptr_t)allocsize, 0,
						 (uintptr_t)allocaddr, 0);
	}

	// The stack grows down.
	// Set the guard page at the lowest address of the
	// newly allocated stack. Return the highest address
	// of the stack.
	if (guardsize) {
		(void)mach_vm_protect(mach_task_self(), allocaddr, guardsize,
				FALSE, VM_PROT_NONE);
	}

	// Thread structure resides at the top of the stack (when using a
	// custom stack, allocsize == PTHREAD_SIZE, so places the pthread_t
	// at allocaddr).
	t = (pthread_t)(allocaddr + pthreadoff);
	if (attrs->stackaddr) {
		*stack = attrs->stackaddr;
	} else {
		*stack = t;
	}

	_pthread_struct_init(t, attrs, *stack, stacksize, allocaddr, allocsize);
	return t;
}

PTHREAD_NOINLINE
void
_pthread_deallocate(pthread_t t, bool from_mach_thread)
{
	kern_return_t ret;

	// Don't free the main thread.
	if (t != main_thread()) {
		if (!from_mach_thread) { // see __pthread_add_thread
			_pthread_introspection_thread_destroy(t);
		}
		ret = mach_vm_deallocate(mach_task_self(), t->freeaddr, t->freesize);
		if (ret != KERN_SUCCESS) {
			PTHREAD_INTERNAL_CRASH(ret, "Unable to deallocate stack");
		}
	}
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreturn-stack-address"

PTHREAD_NOINLINE
static void*
_pthread_current_stack_address(void)
{
	int a;
	return &a;
}

#pragma clang diagnostic pop

void
_pthread_joiner_wake(pthread_t thread)
{
	uint32_t *exit_gate = &thread->tl_exit_gate;

	for (;;) {
		int ret = __ulock_wake(UL_UNFAIR_LOCK | ULF_NO_ERRNO, exit_gate, 0);
		if (ret == 0 || ret == -ENOENT) {
			return;
		}
		if (ret != -EINTR) {
			PTHREAD_INTERNAL_CRASH(-ret, "pthread_join() wake failure");
		}
	}
}

// Terminates the thread if called from the currently running thread.
PTHREAD_NORETURN PTHREAD_NOINLINE PTHREAD_NOT_TAIL_CALLED
static void
_pthread_terminate(pthread_t t, void *exit_value)
{
	_pthread_introspection_thread_terminate(t);

	uintptr_t freeaddr = (uintptr_t)t->freeaddr;
	size_t freesize = t->freesize;
	bool should_exit;

	// the size of just the stack
	size_t freesize_stack = t->freesize;

	// We usually pass our structure+stack to bsdthread_terminate to free, but
	// if we get told to keep the pthread_t structure around then we need to
	// adjust the free size and addr in the pthread_t to just refer to the
	// structure and not the stack.  If we do end up deallocating the
	// structure, this is useless work since no one can read the result, but we
	// can't do it after the call to pthread_remove_thread because it isn't
	// safe to dereference t after that.
	if ((void*)t > t->freeaddr && (void*)t < t->freeaddr + t->freesize){
		// Check to ensure the pthread structure itself is part of the
		// allocation described by freeaddr/freesize, in which case we split and
		// only deallocate the area below the pthread structure.  In the event of a
		// custom stack, the freeaddr/size will be the pthread structure itself, in
		// which case we shouldn't free anything (the final else case).
		freesize_stack = trunc_page((uintptr_t)t - (uintptr_t)freeaddr);

		// describe just the remainder for deallocation when the pthread_t goes away
		t->freeaddr += freesize_stack;
		t->freesize -= freesize_stack;
	} else if (t == main_thread()) {
		freeaddr = t->stackaddr - pthread_get_stacksize_np(t);
		uintptr_t stackborder = trunc_page((uintptr_t)_pthread_current_stack_address());
		freesize_stack = stackborder - freeaddr;
	} else {
		freesize_stack = 0;
	}

	mach_port_t kport = _pthread_kernel_thread(t);
	bool keep_thread_struct = false, needs_wake = false;
	semaphore_t custom_stack_sema = MACH_PORT_NULL;

	_pthread_dealloc_special_reply_port(t);
	_pthread_dealloc_reply_port(t);

	_PTHREAD_LOCK(_pthread_list_lock);

	// This piece of code interacts with pthread_join. It will always:
	// - set tl_exit_gate to MACH_PORT_DEAD (thread exited)
	// - set tl_exit_value to the value passed to pthread_exit()
	// - decrement _pthread_count, so that we can exit the process when all
	//   threads exited even if not all of them were joined.
	t->tl_exit_gate = MACH_PORT_DEAD;
	t->tl_exit_value = exit_value;
	should_exit = (--_pthread_count <= 0);

	// If we see a joiner, we prepost that the join has to succeed,
	// and the joiner is committed to finish (even if it was canceled)
	if (t->tl_join_ctx) {
		custom_stack_sema = _pthread_joiner_prepost_wake(t); // unsets tl_joinable
		needs_wake = true;
	}

	// Joinable threads that have no joiner yet are kept on the thread list
	// so that pthread_join() can later discover the thread when it is joined,
	// and will have to do the pthread_t cleanup.
	if (t->tl_joinable) {
		t->tl_joiner_cleans_up = keep_thread_struct = true;
	} else {
		TAILQ_REMOVE(&__pthread_head, t, tl_plist);
	}

	_PTHREAD_UNLOCK(_pthread_list_lock);

	if (needs_wake) {
		// When we found a waiter, we want to drop the very contended list lock
		// before we do the syscall in _pthread_joiner_wake(). Then, we decide
		// who gets to cleanup the pthread_t between the joiner and the exiting
		// thread:
		// - the joiner tries to set tl_join_ctx to NULL
		// - the exiting thread tries to set tl_joiner_cleans_up to true
		// Whoever does it first commits the other guy to cleanup the pthread_t
		_pthread_joiner_wake(t);
		_PTHREAD_LOCK(_pthread_list_lock);
		if (t->tl_join_ctx) {
			t->tl_joiner_cleans_up = true;
			keep_thread_struct = true;
		}
		_PTHREAD_UNLOCK(_pthread_list_lock);
	}

	//
	// /!\ dereferencing `t` past this point is not safe /!\
	//

	if (keep_thread_struct || t == main_thread()) {
		// Use the adjusted freesize of just the stack that we computed above.
		freesize = freesize_stack;
	} else {
		_pthread_introspection_thread_destroy(t);
	}

	// Check if there is nothing to free because the thread has a custom
	// stack allocation and is joinable.
	if (freesize == 0) {
		freeaddr = 0;
	}
	if (should_exit) {
		exitf(0);
	}
	__bsdthread_terminate((void *)freeaddr, freesize, kport, custom_stack_sema);
	PTHREAD_INTERNAL_CRASH(t, "thread didn't terminate");
}

PTHREAD_NORETURN
static void
_pthread_terminate_invoke(pthread_t t, void *exit_value)
{
#if PTHREAD_T_OFFSET
	void *p = NULL;
	// <rdar://problem/25688492> During pthread termination there is a race
	// between pthread_join and pthread_terminate; if the joiner is responsible
	// for cleaning up the pthread_t struct, then it may destroy some part of the
	// stack with it on 16k OSes. So that this doesn't cause _pthread_terminate()
	// to crash because its stack has been removed from under its feet, just make
	// sure termination happens in a part of the stack that is not on the same
	// page as the pthread_t.
	if (trunc_page((uintptr_t)__builtin_frame_address(0)) ==
			trunc_page((uintptr_t)t)) {
		p = alloca(PTHREAD_T_OFFSET);
	}
	// And this __asm__ volatile is needed to stop the compiler from optimising
	// away the alloca() completely.
	__asm__ volatile ("" : : "r"(p) );
#endif
	_pthread_terminate(t, exit_value);
}

#pragma mark pthread start / body

PTHREAD_NORETURN
void
_pthread_start(pthread_t self, mach_port_t kport,
		__unused void *(*fun)(void *), __unused void *arg,
		__unused size_t stacksize, unsigned int pflags)
{
	if (os_unlikely(pflags & PTHREAD_START_SUSPENDED)) {
		PTHREAD_INTERNAL_CRASH(pflags,
				"kernel without PTHREAD_START_SUSPENDED support");
	}
	if (os_unlikely((pflags & PTHREAD_START_TSD_BASE_SET) == 0)) {
		PTHREAD_INTERNAL_CRASH(pflags,
				"thread_set_tsd_base() wasn't called by the kernel");
	}
	PTHREAD_DEBUG_ASSERT(MACH_PORT_VALID(kport));
	PTHREAD_DEBUG_ASSERT(_pthread_kernel_thread(self) == kport);
	_pthread_validate_signature(self);
	_pthread_markcancel_if_canceled(self, kport);

	_pthread_set_self_internal(self);
	__pthread_started_thread(self);
	_pthread_exit(self, (self->fun)(self->arg));
}

PTHREAD_ALWAYS_INLINE
static inline void
_pthread_struct_init(pthread_t t, const pthread_attr_t *attrs,
		void *stackaddr, size_t stacksize, void *freeaddr, size_t freesize)
{
	_pthread_init_signature(t);
	t->tsd[_PTHREAD_TSD_SLOT_PTHREAD_SELF] = t;
	t->tsd[_PTHREAD_TSD_SLOT_ERRNO] = &t->err_no;
	if (attrs->schedset == 0) {
		t->tsd[_PTHREAD_TSD_SLOT_PTHREAD_QOS_CLASS] = attrs->qosclass;
	} else {
		t->tsd[_PTHREAD_TSD_SLOT_PTHREAD_QOS_CLASS] =
				_pthread_unspecified_priority();
	}
	t->tsd[_PTHREAD_TSD_SLOT_PTR_MUNGE] = _pthread_ptr_munge_token;
	t->tl_has_custom_stack = (attrs->stackaddr != NULL);

	_PTHREAD_LOCK_INIT(t->lock);

	t->stackaddr = stackaddr;
	t->stackbottom = stackaddr - stacksize;
	t->freeaddr = freeaddr;
	t->freesize = freesize;

	t->guardsize = _pthread_attr_guardsize(attrs);
	t->tl_joinable = (attrs->detached == PTHREAD_CREATE_JOINABLE);
	t->inherit = attrs->inherit;
	t->tl_policy = attrs->policy;
	t->schedset = attrs->schedset;
	_pthread_attr_get_schedparam(attrs, &t->tl_param);
	t->cancel_state = PTHREAD_CANCEL_ENABLE | PTHREAD_CANCEL_DEFERRED;
}

#pragma mark pthread public interface

/* Need to deprecate this in future */
int
_pthread_is_threaded(void)
{
	return __is_threaded;
}

/* Non portable public api to know whether this process has(had) atleast one thread
 * apart from main thread. There could be race if there is a thread in the process of
 * creation at the time of call . It does not tell whether there are more than one thread
 * at this point of time.
 */
int
pthread_is_threaded_np(void)
{
	return __is_threaded;
}


PTHREAD_NOEXPORT_VARIANT
mach_port_t
pthread_mach_thread_np(pthread_t t)
{
	mach_port_t kport = MACH_PORT_NULL;
	(void)_pthread_is_valid(t, &kport);
	return kport;
}

PTHREAD_NOEXPORT_VARIANT
pthread_t
pthread_from_mach_thread_np(mach_port_t kernel_thread)
{
	struct _pthread *p = NULL;

	/* No need to wait as mach port is already known */
	_PTHREAD_LOCK(_pthread_list_lock);

	TAILQ_FOREACH(p, &__pthread_head, tl_plist) {
		if (_pthread_kernel_thread(p) == kernel_thread) {
			break;
		}
	}

	_PTHREAD_UNLOCK(_pthread_list_lock);

	return p;
}

PTHREAD_NOEXPORT_VARIANT
size_t
pthread_get_stacksize_np(pthread_t t)
{
	size_t size = 0;

	if (t == NULL) {
		return ESRCH; // XXX bug?
	}

#if TARGET_OS_OSX
	// The default rlimit based allocations will be provided with a stacksize
	// of the current limit and a freesize of the max.  However, custom
	// allocations will just have the guard page to free.  If we aren't in the
	// latter case, call into rlimit to determine the current stack size.  In
	// the event that the current limit == max limit then we'll fall down the
	// fast path, but since it's unlikely that the limit is going to be lowered
	// after it's been change to the max, we should be fine.
	//
	// Of course, on arm rlim_cur == rlim_max and there's only the one guard
	// page.  So, we can skip all this there.
	if (t == main_thread()) {
		size_t stacksize = t->stackaddr - t->stackbottom;

		if (stacksize + vm_page_size != t->freesize) {
			// We want to call getrlimit() just once, as it's relatively
			// expensive
			static size_t rlimit_stack;

			if (rlimit_stack == 0) {
				struct rlimit limit;
				int ret = getrlimit(RLIMIT_STACK, &limit);

				if (ret == 0) {
					rlimit_stack = (size_t) limit.rlim_cur;
				}
			}

			if (rlimit_stack == 0 || rlimit_stack > t->freesize) {
				return stacksize;
			} else {
				return round_page(rlimit_stack);
			}
		}
	}
#endif /* TARGET_OS_OSX */

	if (t == pthread_self() || t == main_thread()) {
		size = t->stackaddr - t->stackbottom;;
		goto out;
	}

	if (_pthread_validate_thread_and_list_lock(t)) {
		size = t->stackaddr - t->stackbottom;;
		_PTHREAD_UNLOCK(_pthread_list_lock);
	}

out:
	// <rdar://problem/42588315> binary compatibility issues force us to return
	// DEFAULT_STACK_SIZE here when we do not know the size of the stack
	return size ? size : DEFAULT_STACK_SIZE;
}

PTHREAD_NOEXPORT_VARIANT
void *
pthread_get_stackaddr_np(pthread_t t)
{
	// since the main thread will not get de-allocated from underneath us
	if (t == pthread_self() || t == main_thread()) {
		return t->stackaddr;
	}

	if (!_pthread_validate_thread_and_list_lock(t)) {
		return (void *)(uintptr_t)ESRCH; // XXX bug?
	}

	void *addr = t->stackaddr;
	_PTHREAD_UNLOCK(_pthread_list_lock);
	return addr;
}


static mach_port_t
_pthread_reply_port(pthread_t t)
{
	void *p;
	if (t == NULL) {
		p = _pthread_getspecific_direct(_PTHREAD_TSD_SLOT_MIG_REPLY);
	} else {
		p = t->tsd[_PTHREAD_TSD_SLOT_MIG_REPLY];
	}
	return (mach_port_t)(uintptr_t)p;
}

static void
_pthread_set_reply_port(pthread_t t, mach_port_t reply_port)
{
	void *p = (void *)(uintptr_t)reply_port;
	if (t == NULL) {
		_pthread_setspecific_direct(_PTHREAD_TSD_SLOT_MIG_REPLY, p);
	} else {
		t->tsd[_PTHREAD_TSD_SLOT_MIG_REPLY] = p;
	}
}

static void
_pthread_dealloc_reply_port(pthread_t t)
{
	mach_port_t reply_port = _pthread_reply_port(t);
	if (reply_port != MACH_PORT_NULL) {
		mig_dealloc_reply_port(reply_port);
	}
}

static mach_port_t
_pthread_special_reply_port(pthread_t t)
{
	void *p;
	if (t == NULL) {
		p = _pthread_getspecific_direct(_PTHREAD_TSD_SLOT_MACH_SPECIAL_REPLY);
	} else {
		p = t->tsd[_PTHREAD_TSD_SLOT_MACH_SPECIAL_REPLY];
	}
	return (mach_port_t)(uintptr_t)p;
}

static void
_pthread_dealloc_special_reply_port(pthread_t t)
{
	mach_port_t special_reply_port = _pthread_special_reply_port(t);
	if (special_reply_port != MACH_PORT_NULL) {
		thread_destruct_special_reply_port(special_reply_port,
				THREAD_SPECIAL_REPLY_PORT_ALL);
	}
}

pthread_t
pthread_main_thread_np(void)
{
	return main_thread();
}

/* returns non-zero if the current thread is the main thread */
int
pthread_main_np(void)
{
	return pthread_self() == main_thread();
}


static int
_pthread_threadid_slow(pthread_t thread, uint64_t *thread_id)
{
	unsigned int info_count = THREAD_IDENTIFIER_INFO_COUNT;
	mach_port_t thport = _pthread_kernel_thread(thread);
	struct thread_identifier_info info;
	kern_return_t kr;

	kr = thread_info(thport, THREAD_IDENTIFIER_INFO,
			(thread_info_t)&info, &info_count);
	if (kr == KERN_SUCCESS && info.thread_id) {
		*thread_id = info.thread_id;
		os_atomic_store(&thread->thread_id, info.thread_id, relaxed);
		return 0;
	}
	return EINVAL;
}

/*
 * if we are passed in a pthread_t that is NULL, then we return the current
 * thread's thread_id. So folks don't have to call pthread_self, in addition to
 * us doing it, if they just want their thread_id.
 */
PTHREAD_NOEXPORT_VARIANT
int
pthread_threadid_np(pthread_t thread, uint64_t *thread_id)
{
	int res = 0;
	pthread_t self = pthread_self();

	if (thread_id == NULL) {
		return EINVAL;
	}

	if (thread == NULL || thread == self) {
		*thread_id = self->thread_id;
	} else if (!_pthread_validate_thread_and_list_lock(thread)) {
		res = ESRCH;
	} else {
		*thread_id = os_atomic_load(&thread->thread_id, relaxed);
		if (os_unlikely(*thread_id == 0)) {
			// there is a race at init because the thread sets its own TID.
			// correct this by asking mach
			res = _pthread_threadid_slow(thread, thread_id);
		}
		_PTHREAD_UNLOCK(_pthread_list_lock);
	}
	return res;
}

PTHREAD_NOEXPORT_VARIANT
int
pthread_getname_np(pthread_t thread, char *threadname, size_t len)
{
	if (thread == pthread_self()) {
		strlcpy(threadname, thread->pthread_name, len);
		return 0;
	}

	if (!_pthread_validate_thread_and_list_lock(thread)) {
		return ESRCH;
	}

	strlcpy(threadname, thread->pthread_name, len);
	_PTHREAD_UNLOCK(_pthread_list_lock);
	return 0;
}


int
pthread_setname_np(const char *name)
{
	int res;
	pthread_t self = pthread_self();

	size_t len = 0;
	if (name != NULL) {
		len = strlen(name);
	}

	_pthread_validate_signature(self);

	/* protytype is in pthread_internals.h */
	res = __proc_info(5, getpid(), 2, (uint64_t)0, (void*)name, (int)len);
	if (res == 0) {
		if (len > 0) {
			strlcpy(self->pthread_name, name, MAXTHREADNAMESIZE);
		} else {
			bzero(self->pthread_name, MAXTHREADNAMESIZE);
		}
	}
	return res;

}

PTHREAD_ALWAYS_INLINE
static inline void
__pthread_add_thread(pthread_t t, bool from_mach_thread)
{
	if (from_mach_thread) {
		_PTHREAD_LOCK_FROM_MACH_THREAD(_pthread_list_lock);
	} else {
		_PTHREAD_LOCK(_pthread_list_lock);
	}

	TAILQ_INSERT_TAIL(&__pthread_head, t, tl_plist);
	_pthread_count++;

	if (from_mach_thread) {
		_PTHREAD_UNLOCK_FROM_MACH_THREAD(_pthread_list_lock);
	} else {
		_PTHREAD_UNLOCK(_pthread_list_lock);
	}

	if (!from_mach_thread) {
		// PR-26275485: Mach threads will likely crash trying to run
		// introspection code.  Since the fall out from the introspection
		// code not seeing the injected thread is likely less than crashing
		// in the introspection code, just don't make the call.
		_pthread_introspection_thread_create(t);
	}
}

PTHREAD_ALWAYS_INLINE
static inline void
__pthread_undo_add_thread(pthread_t t, bool from_mach_thread)
{
	if (from_mach_thread) {
		_PTHREAD_LOCK_FROM_MACH_THREAD(_pthread_list_lock);
	} else {
		_PTHREAD_LOCK(_pthread_list_lock);
	}

	TAILQ_REMOVE(&__pthread_head, t, tl_plist);
	_pthread_count--;

	if (from_mach_thread) {
		_PTHREAD_UNLOCK_FROM_MACH_THREAD(_pthread_list_lock);
	} else {
		_PTHREAD_UNLOCK(_pthread_list_lock);
	}
}

PTHREAD_ALWAYS_INLINE
static inline void
__pthread_started_thread(pthread_t t)
{
	mach_port_t kport = _pthread_kernel_thread(t);
	if (os_unlikely(!MACH_PORT_VALID(kport))) {
		PTHREAD_CLIENT_CRASH(kport,
				"Unable to allocate thread port, possible port leak");
	}
	_pthread_introspection_thread_start(t);
}

#define _PTHREAD_CREATE_NONE              0x0
#define _PTHREAD_CREATE_FROM_MACH_THREAD  0x1
#define _PTHREAD_CREATE_SUSPENDED         0x2

static int
_pthread_create(pthread_t *thread, const pthread_attr_t *attrs,
		void *(*start_routine)(void *), void *arg, unsigned int create_flags)
{
	pthread_t t = NULL;
	void *stack = NULL;
	bool from_mach_thread = (create_flags & _PTHREAD_CREATE_FROM_MACH_THREAD);

	if (attrs == NULL) {
		attrs = &_pthread_attr_default;
	} else if (attrs->sig != _PTHREAD_ATTR_SIG) {
		return EINVAL;
	}

	unsigned int flags = PTHREAD_START_CUSTOM;
	if (attrs->schedset != 0) {
		struct sched_param p;
		_pthread_attr_get_schedparam(attrs, &p);
		flags |= PTHREAD_START_SETSCHED;
		flags |= ((attrs->policy & PTHREAD_START_POLICY_MASK) << PTHREAD_START_POLICY_BITSHIFT);
		flags |= (p.sched_priority & PTHREAD_START_IMPORTANCE_MASK);
	} else if (attrs->qosclass != 0) {
		flags |= PTHREAD_START_QOSCLASS;
		flags |= (attrs->qosclass & PTHREAD_START_QOSCLASS_MASK);
	}
	if (create_flags & _PTHREAD_CREATE_SUSPENDED) {
		flags |= PTHREAD_START_SUSPENDED;
	}

	__is_threaded = 1;

	t =_pthread_allocate(attrs, &stack, from_mach_thread);
	if (t == NULL) {
		return EAGAIN;
	}

	t->arg = arg;
	t->fun = start_routine;
	__pthread_add_thread(t, from_mach_thread);

	if (__bsdthread_create(start_routine, arg, stack, t, flags) ==
			(pthread_t)-1) {
		if (errno == EMFILE) {
			PTHREAD_CLIENT_CRASH(0,
					"Unable to allocate thread port, possible port leak");
		}
		__pthread_undo_add_thread(t, from_mach_thread);
		_pthread_deallocate(t, from_mach_thread);
		return EAGAIN;
	}

	// n.b. if a thread is created detached and exits, t will be invalid
	*thread = t;
	return 0;
}

int
pthread_create(pthread_t *thread, const pthread_attr_t *attr,
		void *(*start_routine)(void *), void *arg)
{
	unsigned int flags = _PTHREAD_CREATE_NONE;
	return _pthread_create(thread, attr, start_routine, arg, flags);
}

int
pthread_create_from_mach_thread(pthread_t *thread, const pthread_attr_t *attr,
		void *(*start_routine)(void *), void *arg)
{
	unsigned int flags = _PTHREAD_CREATE_FROM_MACH_THREAD;
	return _pthread_create(thread, attr, start_routine, arg, flags);
}

int
pthread_create_suspended_np(pthread_t *thread, const pthread_attr_t *attr,
		void *(*start_routine)(void *), void *arg)
{
	unsigned int flags = _PTHREAD_CREATE_SUSPENDED;
	return _pthread_create(thread, attr, start_routine, arg, flags);
}


PTHREAD_NOEXPORT_VARIANT
int
pthread_detach(pthread_t thread)
{
	int res = 0;
	bool join = false, wake = false;

	if (!_pthread_validate_thread_and_list_lock(thread)) {
		return ESRCH;
	}

	if (!thread->tl_joinable) {
		res = EINVAL;
	} else if (thread->tl_exit_gate == MACH_PORT_DEAD) {
		// Join the thread if it's already exited.
		join = true;
	} else {
		thread->tl_joinable = false; // _pthread_joiner_prepost_wake uses this
		if (thread->tl_join_ctx) {
			(void)_pthread_joiner_prepost_wake(thread);
			wake = true;
		}
	}
	_PTHREAD_UNLOCK(_pthread_list_lock);

	if (join) {
		pthread_join(thread, NULL);
	} else if (wake) {
		_pthread_joiner_wake(thread);
	}
	return res;
}

PTHREAD_NOEXPORT_VARIANT
int
pthread_kill(pthread_t th, int sig)
{
	if (sig < 0 || sig > NSIG) {
		return EINVAL;
	}

	mach_port_t kport = MACH_PORT_NULL;
	{
		if (!_pthread_is_valid(th, &kport)) {
			return ESRCH;
		}
	}

	int ret = __pthread_kill(kport, sig);

	if (ret == -1) {
		ret = errno;
	}
	return ret;
}

PTHREAD_NOEXPORT_VARIANT
int
__pthread_workqueue_setkill(int enable)
{
	{
		return __bsdthread_ctl(BSDTHREAD_CTL_WORKQ_ALLOW_KILL, enable, 0, 0);
	}
}


/* For compatibility... */

pthread_t
_pthread_self(void)
{
	return pthread_self();
}

/*
 * Terminate a thread.
 */
extern int __disable_threadsignal(int);

PTHREAD_NORETURN
static void
_pthread_exit(pthread_t self, void *exit_value)
{
	struct __darwin_pthread_handler_rec *handler;

	// Disable signal delivery while we clean up
	__disable_threadsignal(1);

	// Set cancel state to disable and type to deferred
	_pthread_setcancelstate_exit(self, exit_value);

	while ((handler = self->__cleanup_stack) != 0) {
		(handler->__routine)(handler->__arg);
		self->__cleanup_stack = handler->__next;
	}
	_pthread_tsd_cleanup(self);

	// Clear per-thread semaphore cache
	os_put_cached_semaphore(SEMAPHORE_NULL);

	_pthread_terminate_invoke(self, exit_value);
}

void
pthread_exit(void *exit_value)
{
	pthread_t self = pthread_self();
	if (os_unlikely(self->wqthread)) {
		PTHREAD_CLIENT_CRASH(0, "pthread_exit() called from a thread "
				"not created by pthread_create()");
	}
	_pthread_validate_signature(self);
	_pthread_exit(self, exit_value);
}


PTHREAD_NOEXPORT_VARIANT
int
pthread_getschedparam(pthread_t thread, int *policy, struct sched_param *param)
{
	if (!_pthread_validate_thread_and_list_lock(thread)) {
		return ESRCH;
	}

	if (policy) *policy = thread->tl_policy;
	if (param) *param = thread->tl_param;
	_PTHREAD_UNLOCK(_pthread_list_lock);
	return 0;
}



PTHREAD_ALWAYS_INLINE
static inline int
pthread_setschedparam_internal(pthread_t thread, mach_port_t kport, int policy,
		const struct sched_param *param)
{
	policy_base_data_t bases;
	policy_base_t base;
	mach_msg_type_number_t count;
	kern_return_t ret;

	switch (policy) {
		case SCHED_OTHER:
			bases.ts.base_priority = param->sched_priority;
			base = (policy_base_t)&bases.ts;
			count = POLICY_TIMESHARE_BASE_COUNT;
			break;
		case SCHED_FIFO:
			bases.fifo.base_priority = param->sched_priority;
			base = (policy_base_t)&bases.fifo;
			count = POLICY_FIFO_BASE_COUNT;
			break;
		case SCHED_RR:
			bases.rr.base_priority = param->sched_priority;
			/* quantum isn't public yet */
			bases.rr.quantum = param->quantum;
			base = (policy_base_t)&bases.rr;
			count = POLICY_RR_BASE_COUNT;
			break;
		default:
			return EINVAL;
	}
	ret = thread_policy(kport, policy, base, count, TRUE);
	return (ret != KERN_SUCCESS) ? EINVAL : 0;
}

PTHREAD_NOEXPORT_VARIANT
int
pthread_setschedparam(pthread_t t, int policy, const struct sched_param *param)
{
	mach_port_t kport = MACH_PORT_NULL;
	int bypass = 1;

	// since the main thread will not get de-allocated from underneath us
	if (t == pthread_self() || t == main_thread()) {
		_pthread_validate_signature(t);
		kport = _pthread_kernel_thread(t);
	} else {
		bypass = 0;
		if (!_pthread_is_valid(t, &kport)) {
			return ESRCH;
		}
	}

	int res = pthread_setschedparam_internal(t, kport, policy, param);
	if (res) return res;

	if (bypass) {
		_PTHREAD_LOCK(_pthread_list_lock);
	} else if (!_pthread_validate_thread_and_list_lock(t)) {
		// Ensure the thread is still valid.
		return ESRCH;
	}

	t->tl_policy = policy;
	t->tl_param = *param;
	_PTHREAD_UNLOCK(_pthread_list_lock);
	return 0;
}


int
sched_get_priority_min(int policy)
{
	return default_priority - 16;
}

int
sched_get_priority_max(int policy)
{
	return default_priority + 16;
}

int
pthread_equal(pthread_t t1, pthread_t t2)
{
	return (t1 == t2);
}

/*
 * Force LLVM not to optimise this to a call to __pthread_set_self, if it does
 * then _pthread_set_self won't be bound when secondary threads try and start up.
 */
PTHREAD_NOINLINE
void
_pthread_set_self(pthread_t p)
{
#if VARIANT_DYLD
	if (os_likely(!p)) {
		return _pthread_set_self_dyld();
	}
#endif // VARIANT_DYLD
	_pthread_set_self_internal(p);
	_thread_set_tsd_base(&p->tsd[0]);
}

#if VARIANT_DYLD
// _pthread_set_self_dyld is noinline+noexport to allow the option for
// static libsyscall to adopt this as the entry point from mach_init if
// desired
PTHREAD_NOINLINE PTHREAD_NOEXPORT
void
_pthread_set_self_dyld(void)
{
	pthread_t p = main_thread();
	p->thread_id = __thread_selfid();

	if (os_unlikely(p->thread_id == -1ull)) {
		PTHREAD_INTERNAL_CRASH(0, "failed to set thread_id");
	}

	// <rdar://problem/40930651> pthread self and the errno address are the
	// bare minimium TSD setup that dyld needs to actually function.  Without
	// this, TSD access will fail and crash if it uses bits of Libc prior to
	// library initialization. __pthread_init will finish the initialization
	// during library init.
	p->tsd[_PTHREAD_TSD_SLOT_PTHREAD_SELF] = p;
	p->tsd[_PTHREAD_TSD_SLOT_ERRNO] = &p->err_no;
	_thread_set_tsd_base(&p->tsd[0]);
}
#endif // VARIANT_DYLD

PTHREAD_ALWAYS_INLINE
static inline void
_pthread_set_self_internal(pthread_t p)
{
	os_atomic_store(&p->thread_id, __thread_selfid(), relaxed);

	if (os_unlikely(p->thread_id == -1ull)) {
		PTHREAD_INTERNAL_CRASH(0, "failed to set thread_id");
	}
}


// <rdar://problem/28984807> pthread_once should have an acquire barrier
PTHREAD_ALWAYS_INLINE
static inline void
_os_once_acquire(os_once_t *predicate, void *context, os_function_t function)
{
	if (OS_EXPECT(os_atomic_load(predicate, acquire), ~0l) != ~0l) {
		_os_once(predicate, context, function);
		OS_COMPILER_CAN_ASSUME(*predicate == ~0l);
	}
}

struct _pthread_once_context {
	pthread_once_t *pthread_once;
	void (*routine)(void);
};

static void
__pthread_once_handler(void *context)
{
	struct _pthread_once_context *ctx = context;
	pthread_cleanup_push((void*)__os_once_reset, &ctx->pthread_once->once);
	ctx->routine();
	pthread_cleanup_pop(0);
	ctx->pthread_once->sig = _PTHREAD_ONCE_SIG;
}

PTHREAD_NOEXPORT_VARIANT
int
pthread_once(pthread_once_t *once_control, void (*init_routine)(void))
{
	struct _pthread_once_context ctx = { once_control, init_routine };
	do {
		_os_once_acquire(&once_control->once, &ctx, __pthread_once_handler);
	} while (once_control->sig == _PTHREAD_ONCE_SIG_init);
	return 0;
}


int
pthread_getconcurrency(void)
{
	return pthread_concurrency;
}

int
pthread_setconcurrency(int new_level)
{
	if (new_level < 0) {
		return EINVAL;
	}
	pthread_concurrency = new_level;
	return 0;
}

#if !defined(VARIANT_STATIC)
void *
malloc(size_t sz)
{
	if (_pthread_malloc) {
		return _pthread_malloc(sz);
	} else {
		return NULL;
	}
}

void
free(void *p)
{
	if (_pthread_free) {
		_pthread_free(p);
	}
}
#endif // VARIANT_STATIC

/*
 * Perform package initialization - called automatically when application starts
 */
struct ProgramVars; /* forward reference */

#if !VARIANT_DYLD
static unsigned long
_pthread_strtoul(const char *p, const char **endptr, int base)
{
	uintptr_t val = 0;

	// Expect hex string starting with "0x"
	if ((base == 16 || base == 0) && p && p[0] == '0' && p[1] == 'x') {
		p += 2;
		while (1) {
			char c = *p;
			if ('0' <= c && c <= '9') {
				val = (val << 4) + (c - '0');
			} else if ('a' <= c && c <= 'f') {
				val = (val << 4) + (c - 'a' + 10);
			} else if ('A' <= c && c <= 'F') {
				val = (val << 4) + (c - 'A' + 10);
			} else {
				break;
			}
			++p;
		}
	}

	*endptr = (char *)p;
	return val;
}

static int
parse_main_stack_params(const char *apple[],
			void **stackaddr,
			size_t *stacksize,
			void **allocaddr,
			size_t *allocsize)
{
	const char *p = _simple_getenv(apple, "main_stack");
	if (!p) return 0;

	int ret = 0;
	const char *s = p;

	*stackaddr = _pthread_strtoul(s, &s, 16);
	if (*s != ',') goto out;

	*stacksize = _pthread_strtoul(s + 1, &s, 16);
	if (*s != ',') goto out;

	*allocaddr = _pthread_strtoul(s + 1, &s, 16);
	if (*s != ',') goto out;

	*allocsize = _pthread_strtoul(s + 1, &s, 16);
	if (*s != ',' && *s != 0) goto out;

	ret = 1;
out:
	bzero((char *)p, strlen(p));
	return ret;
}

static void
parse_ptr_munge_params(const char *envp[], const char *apple[])
{
	const char *p, *s;
	uintptr_t token = 0;
	p = _simple_getenv(apple, "ptr_munge");
	if (p) {
		token = _pthread_strtoul(p, &s, 16);
		bzero((char *)p, strlen(p));
	}
#if !DEBUG
	if (!token) {
#endif
		p = _simple_getenv(envp, "PTHREAD_PTR_MUNGE_TOKEN");
		if (p) {
			uintptr_t t = _pthread_strtoul(p, &s, 16);
			if (t) token = t;
		}
#if !DEBUG
	}

	if (!token) {
		PTHREAD_INTERNAL_CRASH(token, "Token from the kernel is 0");
	}
#endif // DEBUG

	_pthread_ptr_munge_token = token;
	// we need to refresh the main thread signature now that we changed
	// the munge token. We need to do it while TSAN will not look at it
	_pthread_init_signature(_main_thread_ptr);
}

int
__pthread_init(const struct _libpthread_functions *pthread_funcs,
		const char *envp[], const char *apple[],
		const struct ProgramVars *vars __unused)
{
	// Save our provided pushed-down functions
	if (pthread_funcs) {
		exitf = pthread_funcs->exit;

		if (pthread_funcs->version >= 2) {
			_pthread_malloc = pthread_funcs->malloc;
			_pthread_free = pthread_funcs->free;
		}
	}

	// libpthread.a in dyld "owns" the main thread structure itself and sets
	// up the tsd to point to it. So take the pthread_self() from there
	// and make it our main thread point.
	pthread_t thread = (pthread_t)_pthread_getspecific_direct(
			_PTHREAD_TSD_SLOT_PTHREAD_SELF);
	if (os_unlikely(thread == NULL)) {
		PTHREAD_INTERNAL_CRASH(0, "PTHREAD_SELF TSD not initialized");
	}
	_main_thread_ptr = thread;
	// this needs to be done early so that pthread_self() works in TSAN
	_pthread_init_signature(thread);

	//
	// Get host information
	//

	kern_return_t kr;
	host_flavor_t flavor = HOST_PRIORITY_INFO;
	mach_msg_type_number_t count = HOST_PRIORITY_INFO_COUNT;
	host_priority_info_data_t priority_info;
	host_t host = mach_host_self();
	kr = host_info(host, flavor, (host_info_t)&priority_info, &count);
	if (kr != KERN_SUCCESS) {
		PTHREAD_INTERNAL_CRASH(kr, "host_info() failed");
	} else {
		default_priority = (uint8_t)priority_info.user_priority;
		min_priority = (uint8_t)priority_info.minimum_priority;
		max_priority = (uint8_t)priority_info.maximum_priority;
	}
	mach_port_deallocate(mach_task_self(), host);

	//
	// Set up the main thread structure
	//

	// Get the address and size of the main thread's stack from the kernel.
	void *stackaddr = 0;
	size_t stacksize = 0;
	void *allocaddr = 0;
	size_t allocsize = 0;
	if (!parse_main_stack_params(apple, &stackaddr, &stacksize, &allocaddr, &allocsize) ||
		stackaddr == NULL || stacksize == 0) {
		// Fall back to previous bevhaior.
		size_t len = sizeof(stackaddr);
		int mib[] = { CTL_KERN, KERN_USRSTACK };
		if (__sysctl(mib, 2, &stackaddr, &len, NULL, 0) != 0) {
#if defined(__LP64__)
			stackaddr = (void *)USRSTACK64;
#else
			stackaddr = (void *)USRSTACK;
#endif
		}
		stacksize = DFLSSIZ;
		allocaddr = 0;
		allocsize = 0;
	}

	// Initialize random ptr_munge token from the kernel.
	parse_ptr_munge_params(envp, apple);

	PTHREAD_DEBUG_ASSERT(_pthread_attr_default.qosclass ==
			_pthread_default_priority(0));
	_pthread_struct_init(thread, &_pthread_attr_default,
			stackaddr, stacksize, allocaddr, allocsize);
	thread->tl_joinable = true;

	// Finish initialization with common code that is reinvoked on the
	// child side of a fork.

	// Finishes initialization of main thread attributes.
	// Initializes the thread list and add the main thread.
	// Calls _pthread_set_self() to prepare the main thread for execution.
	_pthread_main_thread_init(thread);

	struct _pthread_registration_data registration_data;
	// Set up kernel entry points with __bsdthread_register.
	_pthread_bsdthread_init(&registration_data);

	// Have pthread_key and pthread_mutex do their init envvar checks.
	_pthread_key_global_init(envp);
	_pthread_mutex_global_init(envp, &registration_data);

#if PTHREAD_DEBUG_LOG
	_SIMPLE_STRING path = _simple_salloc();
	_simple_sprintf(path, "/var/tmp/libpthread.%d.log", getpid());
	_pthread_debuglog = open(_simple_string(path),
			O_WRONLY | O_APPEND | O_CREAT | O_NOFOLLOW | O_CLOEXEC, 0666);
	_simple_sfree(path);
	_pthread_debugstart = mach_absolute_time();
#endif

	return 0;
}
#endif // !VARIANT_DYLD

PTHREAD_NOEXPORT void
_pthread_main_thread_init(pthread_t p)
{
	TAILQ_INIT(&__pthread_head);
	_PTHREAD_LOCK_INIT(_pthread_list_lock);
	_PTHREAD_LOCK_INIT(p->lock);
	_pthread_set_kernel_thread(p, mach_thread_self());
	_pthread_set_reply_port(p, mach_reply_port());
	p->__cleanup_stack = NULL;
	p->tl_join_ctx = NULL;
	p->tl_exit_gate = MACH_PORT_NULL;
	p->tsd[__TSD_SEMAPHORE_CACHE] = (void*)(uintptr_t)SEMAPHORE_NULL;
	p->tsd[__TSD_MACH_SPECIAL_REPLY] = 0;

	// Initialize the list of threads with the new main thread.
	TAILQ_INSERT_HEAD(&__pthread_head, p, tl_plist);
	_pthread_count = 1;

	_pthread_introspection_thread_start(p);
}

PTHREAD_NOEXPORT
void
_pthread_main_thread_postfork_init(pthread_t p)
{
	_pthread_main_thread_init(p);
	_pthread_set_self_internal(p);
}

int
sched_yield(void)
{
	swtch_pri(0);
	return 0;
}

// XXX remove
void
cthread_yield(void)
{
	sched_yield();
}

void
pthread_yield_np(void)
{
	sched_yield();
}

// Libsystem knows about this symbol and exports it to libsyscall
int
pthread_current_stack_contains_np(const void *addr, size_t length)
{
	uintptr_t begin = (uintptr_t) addr, end;
	uintptr_t stack_base = (uintptr_t) _pthread_self_direct()->stackbottom;
	uintptr_t stack_top = (uintptr_t) _pthread_self_direct()->stackaddr;

	if (stack_base == stack_top) {
		return -ENOTSUP;
	}

	if (__builtin_add_overflow(begin, length, &end)) {
		return -EINVAL;
	}

	return stack_base <= begin && end <= stack_top;
}



// Libsystem knows about this symbol and exports it to libsyscall
PTHREAD_NOEXPORT_VARIANT
void
_pthread_clear_qos_tsd(mach_port_t thread_port)
{
	if (thread_port == MACH_PORT_NULL || (uintptr_t)_pthread_getspecific_direct(_PTHREAD_TSD_SLOT_MACH_THREAD_SELF) == thread_port) {
		/* Clear the current thread's TSD, that can be done inline. */
		_pthread_setspecific_direct(_PTHREAD_TSD_SLOT_PTHREAD_QOS_CLASS,
				_pthread_unspecified_priority());
	} else {
		pthread_t p;

		_PTHREAD_LOCK(_pthread_list_lock);

		TAILQ_FOREACH(p, &__pthread_head, tl_plist) {
			mach_port_t kp = _pthread_kernel_thread(p);
			if (thread_port == kp) {
				p->tsd[_PTHREAD_TSD_SLOT_PTHREAD_QOS_CLASS] =
						_pthread_unspecified_priority();
				break;
			}
		}

		_PTHREAD_UNLOCK(_pthread_list_lock);
	}
}


#pragma mark pthread/stack_np.h public interface


#if defined(__i386__) || defined(__x86_64__) || defined(__arm__) || defined(__arm64__)
#if __ARM64_ARCH_8_32__
/*
 * arm64_32 uses 64-bit sizes for the frame pointer and
 * return address of a stack frame.
 */
typedef uint64_t frame_data_addr_t;
#else
typedef uintptr_t frame_data_addr_t;
#endif

struct frame_data {
	frame_data_addr_t frame_addr_next;
	frame_data_addr_t ret_addr;
};
#else
#error ********** Unimplemented architecture
#endif

uintptr_t
pthread_stack_frame_decode_np(uintptr_t frame_addr, uintptr_t *return_addr)
{
	struct frame_data *frame = (struct frame_data *)frame_addr;

	if (return_addr) {
#if __has_feature(ptrauth_calls)
		*return_addr = (uintptr_t)ptrauth_strip((void *)frame->ret_addr,
				ptrauth_key_return_address);
#else
		*return_addr = (uintptr_t)frame->ret_addr;
#endif /* __has_feature(ptrauth_calls) */
	}

#if __has_feature(ptrauth_calls)
	return (uintptr_t)ptrauth_strip((void *)frame->frame_addr_next,
			ptrauth_key_frame_pointer);
#endif /* __has_feature(ptrauth_calls) */
	return (uintptr_t)frame->frame_addr_next;
}


#pragma mark pthread workqueue support routines


PTHREAD_NOEXPORT void
_pthread_bsdthread_init(struct _pthread_registration_data *data)
{
	bzero(data, sizeof(*data));
	data->version = sizeof(struct _pthread_registration_data);
	data->dispatch_queue_offset = __PTK_LIBDISPATCH_KEY0 * sizeof(void *);
	data->return_to_kernel_offset = __TSD_RETURN_TO_KERNEL * sizeof(void *);
	data->tsd_offset = offsetof(struct _pthread, tsd);
	data->mach_thread_self_offset = __TSD_MACH_THREAD_SELF * sizeof(void *);

	int rv = __bsdthread_register(thread_start, start_wqthread, (int)PTHREAD_SIZE,
			(void*)data, (uintptr_t)sizeof(*data), data->dispatch_queue_offset);

	if (rv > 0) {
		int required_features =
				PTHREAD_FEATURE_FINEPRIO |
				PTHREAD_FEATURE_BSDTHREADCTL |
				PTHREAD_FEATURE_SETSELF |
				PTHREAD_FEATURE_QOS_MAINTENANCE |
				PTHREAD_FEATURE_QOS_DEFAULT;
		if ((rv & required_features) != required_features) {
			PTHREAD_INTERNAL_CRASH(rv, "Missing required kernel support");
		}
		__pthread_supported_features = rv;
	}

	/*
	 * TODO: differentiate between (-1, EINVAL) after fork (which has the side
	 * effect of resetting the child's stack_addr_hint before bailing out) and
	 * (-1, EINVAL) because of invalid arguments.  We'd probably like to treat
	 * the latter as fatal.
	 *
	 * <rdar://problem/36451838>
	 */

	pthread_priority_t main_qos = (pthread_priority_t)data->main_qos;

	if (_pthread_priority_thread_qos(main_qos) != THREAD_QOS_UNSPECIFIED) {
		_pthread_set_main_qos(main_qos);
		main_thread()->tsd[_PTHREAD_TSD_SLOT_PTHREAD_QOS_CLASS] = main_qos;
	}

	if (data->stack_addr_hint) {
		__pthread_stack_hint = data->stack_addr_hint;
	}

	if (__libdispatch_workerfunction != NULL) {
		// prepare the kernel for workq action
		(void)__workq_open();
	}
}

PTHREAD_NOINLINE
static void
_pthread_wqthread_legacy_worker_wrap(pthread_priority_t pp)
{
	/* Old thread priorities are inverted from where we have them in
	 * the new flexible priority scheme. The highest priority is zero,
	 * up to 2, with background at 3.
	 */
	pthread_workqueue_function_t func = (pthread_workqueue_function_t)__libdispatch_workerfunction;
	bool overcommit = (pp & _PTHREAD_PRIORITY_OVERCOMMIT_FLAG);
	int opts = overcommit ? WORKQ_ADDTHREADS_OPTION_OVERCOMMIT : 0;

	switch (_pthread_priority_thread_qos(pp)) {
	case THREAD_QOS_USER_INITIATED:
		return (*func)(WORKQ_HIGH_PRIOQUEUE, opts, NULL);
	case THREAD_QOS_LEGACY:
		/* B&I builders can't pass a QOS_CLASS_DEFAULT thread to dispatch, for fear of the QoS being
		 * picked up by NSThread (et al) and transported around the system. So change the TSD to
		 * make this thread look like QOS_CLASS_USER_INITIATED even though it will still run as legacy.
		 */
		_pthread_setspecific_direct(_PTHREAD_TSD_SLOT_PTHREAD_QOS_CLASS,
				_pthread_priority_make_from_thread_qos(THREAD_QOS_USER_INITIATED, 0, 0));
		return (*func)(WORKQ_DEFAULT_PRIOQUEUE, opts, NULL);
	case THREAD_QOS_UTILITY:
		return (*func)(WORKQ_LOW_PRIOQUEUE, opts, NULL);
	case THREAD_QOS_BACKGROUND:
		return (*func)(WORKQ_BG_PRIOQUEUE, opts, NULL);
	}
	PTHREAD_INTERNAL_CRASH(pp, "Invalid pthread priority for the legacy interface");
}

PTHREAD_ALWAYS_INLINE
static inline pthread_priority_t
_pthread_wqthread_priority(int flags)
{
	pthread_priority_t pp = 0;
	thread_qos_t qos;

	if (flags & WQ_FLAG_THREAD_KEVENT) {
		pp |= _PTHREAD_PRIORITY_NEEDS_UNBIND_FLAG;
	}
	if (flags & WQ_FLAG_THREAD_EVENT_MANAGER) {
		return pp | _PTHREAD_PRIORITY_EVENT_MANAGER_FLAG;
	}

	if (flags & WQ_FLAG_THREAD_OVERCOMMIT) {
		pp |= _PTHREAD_PRIORITY_OVERCOMMIT_FLAG;
	}
	if (flags & WQ_FLAG_THREAD_PRIO_QOS) {
		qos = (thread_qos_t)(flags & WQ_FLAG_THREAD_PRIO_MASK);
		pp = _pthread_priority_make_from_thread_qos(qos, 0, pp);
	} else if (flags & WQ_FLAG_THREAD_PRIO_SCHED) {
		pp |= _PTHREAD_PRIORITY_SCHED_PRI_MASK;
		pp |= (flags & WQ_FLAG_THREAD_PRIO_MASK);
	} else {
		PTHREAD_INTERNAL_CRASH(flags, "Missing priority");
	}
	return pp;
}

PTHREAD_NOINLINE
static void
_pthread_wqthread_setup(pthread_t self, mach_port_t kport, void *stacklowaddr,
		int flags)
{
	void *stackaddr = self;
	size_t stacksize = (uintptr_t)self - (uintptr_t)stacklowaddr;

	_pthread_struct_init(self, &_pthread_attr_default, stackaddr, stacksize,
			PTHREAD_ALLOCADDR(stackaddr, stacksize),
			PTHREAD_ALLOCSIZE(stackaddr, stacksize));

	_pthread_set_kernel_thread(self, kport);
	self->wqthread = 1;
	self->wqkillset = 0;
	self->tl_joinable = false;

	// Update the running thread count and set childrun bit.
	if (os_unlikely((flags & WQ_FLAG_THREAD_TSD_BASE_SET) == 0)) {
		PTHREAD_INTERNAL_CRASH(flags,
				"thread_set_tsd_base() wasn't called by the kernel");
	}
	_pthread_set_self_internal(self);
	__pthread_add_thread(self, false);
	__pthread_started_thread(self);
}

PTHREAD_NORETURN PTHREAD_NOINLINE
static void
_pthread_wqthread_exit(pthread_t self)
{
	pthread_priority_t pp;
	thread_qos_t qos;

	pp = (pthread_priority_t)self->tsd[_PTHREAD_TSD_SLOT_PTHREAD_QOS_CLASS];
	qos = _pthread_priority_thread_qos(pp);
	if (qos == THREAD_QOS_UNSPECIFIED || qos > WORKQ_THREAD_QOS_CLEANUP) {
		// Reset QoS to something low for the cleanup process
		pp = _pthread_priority_make_from_thread_qos(WORKQ_THREAD_QOS_CLEANUP, 0, 0);
		self->tsd[_PTHREAD_TSD_SLOT_PTHREAD_QOS_CLASS] = (void *)pp;
	}

	_pthread_exit(self, NULL);
}

// workqueue entry point from kernel
void
_pthread_wqthread(pthread_t self, mach_port_t kport, void *stacklowaddr,
		void *keventlist, int flags, int nkevents)
{
	if ((flags & WQ_FLAG_THREAD_REUSE) == 0) {
		_pthread_wqthread_setup(self, kport, stacklowaddr, flags);
	}

	pthread_priority_t pp;

	if (flags & WQ_FLAG_THREAD_OUTSIDEQOS) {
		self->wq_outsideqos = 1;
		pp = _pthread_priority_make_from_thread_qos(THREAD_QOS_LEGACY, 0,
				_PTHREAD_PRIORITY_FALLBACK_FLAG);
	} else {
		self->wq_outsideqos = 0;
		pp = _pthread_wqthread_priority(flags);
	}

	self->tsd[_PTHREAD_TSD_SLOT_PTHREAD_QOS_CLASS] = (void *)pp;

	// avoid spills on the stack hard to keep used stack space minimal
	if (os_unlikely(nkevents == WORKQ_EXIT_THREAD_NKEVENT)) {
		_pthread_wqthread_exit(self);
	} else if (flags & WQ_FLAG_THREAD_WORKLOOP) {
		kqueue_id_t *kqidptr = (kqueue_id_t *)keventlist - 1;
		self->fun = (void *(*)(void*))__libdispatch_workloopfunction;
		self->arg = keventlist;
		self->wq_nevents = nkevents;
		(*__libdispatch_workloopfunction)(kqidptr, &self->arg, &self->wq_nevents);
		__workq_kernreturn(WQOPS_THREAD_WORKLOOP_RETURN, self->arg, self->wq_nevents, 0);
	} else if (flags & WQ_FLAG_THREAD_KEVENT) {
		self->fun = (void *(*)(void*))__libdispatch_keventfunction;
		self->arg = keventlist;
		self->wq_nevents = nkevents;
		(*__libdispatch_keventfunction)(&self->arg, &self->wq_nevents);
		__workq_kernreturn(WQOPS_THREAD_KEVENT_RETURN, self->arg, self->wq_nevents, 0);
	} else {
		self->fun = (void *(*)(void*))__libdispatch_workerfunction;
		self->arg = (void *)(uintptr_t)pp;
		self->wq_nevents = 0;
		if (os_likely(__workq_newapi)) {
			(*__libdispatch_workerfunction)(pp);
		} else {
			_pthread_wqthread_legacy_worker_wrap(pp);
		}
		__workq_kernreturn(WQOPS_THREAD_RETURN, NULL, 0, 0);
	}

	_os_set_crash_log_cause_and_message(self->err_no,
			"BUG IN LIBPTHREAD: __workq_kernreturn returned");
	/*
	 * 52858993: we should never return but the compiler insists on outlining,
	 * so the __builtin_trap() is in _start_wqthread in pthread_asm.s
	 */
}


#pragma mark pthread workqueue API for libdispatch


_Static_assert(WORKQ_KEVENT_EVENT_BUFFER_LEN == WQ_KEVENT_LIST_LEN,
		"Kernel and userland should agree on the event list size");

void
pthread_workqueue_setdispatchoffset_np(int offset)
{
	__workq_kernreturn(WQOPS_QUEUE_NEWSPISUPP, NULL, offset, 0x00);
}

int
pthread_workqueue_setup(struct pthread_workqueue_config *cfg, size_t cfg_size)
{
	int rv = EBUSY;
	struct workq_dispatch_config wdc_cfg;
	size_t min_size = 0;

	if (cfg_size < sizeof(uint32_t)) {
		return EINVAL;
	}

	switch (cfg->version) {
		case 1:
			min_size = offsetof(struct pthread_workqueue_config, queue_label_offs);
			break;
		case 2:
			min_size = sizeof(struct pthread_workqueue_config);
			break;
		default:
			return EINVAL;
		}

	if (!cfg || cfg_size < min_size) {
		return EINVAL;
	}

	if (cfg->flags & ~PTHREAD_WORKQUEUE_CONFIG_SUPPORTED_FLAGS ||
		cfg->version < PTHREAD_WORKQUEUE_CONFIG_MIN_SUPPORTED_VERSION) {
		return ENOTSUP;
	}

	if (__libdispatch_workerfunction == NULL) {
		__workq_newapi = true;

		wdc_cfg.wdc_version = WORKQ_DISPATCH_CONFIG_VERSION;
		wdc_cfg.wdc_flags = 0;
		wdc_cfg.wdc_queue_serialno_offs = cfg->queue_serialno_offs;
#if WORKQ_DISPATCH_CONFIG_VERSION >= 2
		wdc_cfg.wdc_queue_label_offs = cfg->queue_label_offs;
#endif

		// Tell the kernel about dispatch internals
		rv = (int) __workq_kernreturn(WQOPS_SETUP_DISPATCH, &wdc_cfg, sizeof(wdc_cfg), 0);
		if (rv == -1) {
			return errno;
		} else {
			__libdispatch_keventfunction = cfg->kevent_cb;
			__libdispatch_workloopfunction = cfg->workloop_cb;
			__libdispatch_workerfunction = cfg->workq_cb;

			// Prepare the kernel for workq action
			(void)__workq_open();
			if (__is_threaded == 0) {
				__is_threaded = 1;
			}

			return 0;
		}
	}

	return rv;
}

int
_pthread_workqueue_init_with_workloop(pthread_workqueue_function2_t queue_func,
		pthread_workqueue_function_kevent_t kevent_func,
		pthread_workqueue_function_workloop_t workloop_func,
		int offset, int flags)
{
	struct pthread_workqueue_config cfg = {
		.version = PTHREAD_WORKQUEUE_CONFIG_VERSION,
		.flags = 0,
		.workq_cb = queue_func,
		.kevent_cb = kevent_func,
		.workloop_cb = workloop_func,
		.queue_serialno_offs = offset,
		.queue_label_offs = 0,
	};

	return pthread_workqueue_setup(&cfg, sizeof(cfg));
}

int
_pthread_workqueue_init_with_kevent(pthread_workqueue_function2_t queue_func,
		pthread_workqueue_function_kevent_t kevent_func,
		int offset, int flags)
{
	return _pthread_workqueue_init_with_workloop(queue_func, kevent_func, NULL, offset, flags);
}

int
_pthread_workqueue_init(pthread_workqueue_function2_t func, int offset, int flags)
{
	return _pthread_workqueue_init_with_kevent(func, NULL, offset, flags);
}

int
pthread_workqueue_setdispatch_np(pthread_workqueue_function_t worker_func)
{
	struct pthread_workqueue_config cfg = {
		.version = PTHREAD_WORKQUEUE_CONFIG_VERSION,
		.flags = 0,
		.workq_cb = (uint64_t)(pthread_workqueue_function2_t)worker_func,
		.kevent_cb = 0,
		.workloop_cb = 0,
		.queue_serialno_offs = 0,
		.queue_label_offs = 0,
	};

	return pthread_workqueue_setup(&cfg, sizeof(cfg));
}

int
_pthread_workqueue_supported(void)
{
	if (os_unlikely(!__pthread_supported_features)) {
		PTHREAD_INTERNAL_CRASH(0, "libpthread has not been initialized");
	}

	return __pthread_supported_features;
}

int
pthread_workqueue_addthreads_np(int queue_priority, int options, int numthreads)
{
	int res = 0;

	// Cannot add threads without a worker function registered.
	if (__libdispatch_workerfunction == NULL) {
		return EPERM;
	}

	pthread_priority_t kp = 0;
	int compat_priority = queue_priority & WQ_FLAG_THREAD_PRIO_MASK;
	int flags = 0;

	if (options & WORKQ_ADDTHREADS_OPTION_OVERCOMMIT) {
		flags = _PTHREAD_PRIORITY_OVERCOMMIT_FLAG;
	}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
	kp = _pthread_qos_class_encode_workqueue(compat_priority, flags);
#pragma clang diagnostic pop

	res = __workq_kernreturn(WQOPS_QUEUE_REQTHREADS, NULL, numthreads, (int)kp);
	if (res == -1) {
		res = errno;
	}
	return res;
}

bool
_pthread_workqueue_should_narrow(pthread_priority_t pri)
{
	int res = __workq_kernreturn(WQOPS_SHOULD_NARROW, NULL, (int)pri, 0);
	if (res == -1) {
		return false;
	}
	return res;
}

int
_pthread_workqueue_addthreads(int numthreads, pthread_priority_t priority)
{
	int res = 0;

	if (__libdispatch_workerfunction == NULL) {
		return EPERM;
	}

#if TARGET_OS_OSX
	// <rdar://problem/37687655> Legacy simulators fail to boot
	//
	// Older sims set the deprecated _PTHREAD_PRIORITY_ROOTQUEUE_FLAG wrongly,
	// which is aliased to _PTHREAD_PRIORITY_SCHED_PRI_FLAG and that XNU
	// validates and rejects.
	//
	// As a workaround, forcefully unset this bit that cannot be set here
	// anyway.
	priority &= ~_PTHREAD_PRIORITY_SCHED_PRI_FLAG;
#endif

	res = __workq_kernreturn(WQOPS_QUEUE_REQTHREADS, NULL, numthreads, (int)priority);
	if (res == -1) {
		res = errno;
	}
	return res;
}

int
_pthread_workqueue_set_event_manager_priority(pthread_priority_t priority)
{
	int res = __workq_kernreturn(WQOPS_SET_EVENT_MANAGER_PRIORITY, NULL, (int)priority, 0);
	if (res == -1) {
		res = errno;
	}
	return res;
}

int
_pthread_workloop_create(uint64_t workloop_id, uint64_t options, pthread_attr_t *attr)
{
	struct kqueue_workloop_params params = {
		.kqwlp_version = sizeof(struct kqueue_workloop_params),
		.kqwlp_id = workloop_id,
		.kqwlp_flags = 0,
	};

	if (!attr) {
		return EINVAL;
	}

	if (attr->schedset) {
		params.kqwlp_flags |= KQ_WORKLOOP_CREATE_SCHED_PRI;
		params.kqwlp_sched_pri = attr->param.sched_priority;
	}

	if (attr->policyset) {
		params.kqwlp_flags |= KQ_WORKLOOP_CREATE_SCHED_POL;
		params.kqwlp_sched_pol = attr->policy;
	}

	if (attr->cpupercentset) {
		params.kqwlp_flags |= KQ_WORKLOOP_CREATE_CPU_PERCENT;
		params.kqwlp_cpu_percent = attr->cpupercent;
		params.kqwlp_cpu_refillms = attr->refillms;
	}

	int res = __kqueue_workloop_ctl(KQ_WORKLOOP_CREATE, 0, &params,
			sizeof(params));
	if (res == -1) {
		res = errno;
	}
	return res;
}

int
_pthread_workloop_destroy(uint64_t workloop_id)
{
	struct kqueue_workloop_params params = {
		.kqwlp_version = sizeof(struct kqueue_workloop_params),
		.kqwlp_id = workloop_id,
	};

	int res = __kqueue_workloop_ctl(KQ_WORKLOOP_DESTROY, 0, &params,
			sizeof(params));
	if (res == -1) {
		res = errno;
	}
	return res;
}


#pragma mark Introspection SPI for libpthread.


static pthread_introspection_hook_t _pthread_introspection_hook;

pthread_introspection_hook_t
pthread_introspection_hook_install(pthread_introspection_hook_t hook)
{
	pthread_introspection_hook_t prev;
	prev = _pthread_atomic_xchg_ptr((void**)&_pthread_introspection_hook, hook);
	return prev;
}

PTHREAD_NOINLINE
static void
_pthread_introspection_hook_callout_thread_create(pthread_t t)
{
	_pthread_introspection_hook(PTHREAD_INTROSPECTION_THREAD_CREATE, t, t,
			PTHREAD_SIZE);
}

static inline void
_pthread_introspection_thread_create(pthread_t t)
{
	if (os_fastpath(!_pthread_introspection_hook)) return;
	_pthread_introspection_hook_callout_thread_create(t);
}

PTHREAD_NOINLINE
static void
_pthread_introspection_hook_callout_thread_start(pthread_t t)
{
	size_t freesize;
	void *freeaddr;
	if (t == main_thread()) {
		size_t stacksize = t->stackaddr - t->stackbottom;
		freesize = stacksize + t->guardsize;
		freeaddr = t->stackaddr - freesize;
	} else {
		freesize = t->freesize - PTHREAD_SIZE;
		freeaddr = t->freeaddr;
	}
	_pthread_introspection_hook(PTHREAD_INTROSPECTION_THREAD_START, t,
			freeaddr, freesize);
}

static inline void
_pthread_introspection_thread_start(pthread_t t)
{
	if (os_fastpath(!_pthread_introspection_hook)) return;
	_pthread_introspection_hook_callout_thread_start(t);
}

PTHREAD_NOINLINE
static void
_pthread_introspection_hook_callout_thread_terminate(pthread_t t)
{
	size_t freesize;
	void *freeaddr;
	if (t == main_thread()) {
		size_t stacksize = t->stackaddr - t->stackbottom;
		freesize = stacksize + t->guardsize;
		freeaddr = t->stackaddr - freesize;
	} else {
		freesize = t->freesize - PTHREAD_SIZE;
		freeaddr = t->freeaddr;
	}
	_pthread_introspection_hook(PTHREAD_INTROSPECTION_THREAD_TERMINATE, t,
			freeaddr, freesize);
}

static inline void
_pthread_introspection_thread_terminate(pthread_t t)
{
	if (os_fastpath(!_pthread_introspection_hook)) return;
	_pthread_introspection_hook_callout_thread_terminate(t);
}

PTHREAD_NOINLINE
static void
_pthread_introspection_hook_callout_thread_destroy(pthread_t t)
{
	_pthread_introspection_hook(PTHREAD_INTROSPECTION_THREAD_DESTROY, t, t,
			PTHREAD_SIZE);
}

static inline void
_pthread_introspection_thread_destroy(pthread_t t)
{
	if (os_fastpath(!_pthread_introspection_hook)) return;
	_pthread_introspection_hook_callout_thread_destroy(t);
}


#if !VARIANT_DYLD
#pragma mark libplatform shims

#include <platform/string.h>

// pthread_setup initializes large structures to 0,
// which the compiler turns into a library call to memset.
//
// To avoid linking against Libc, provide a simple wrapper
// that calls through to the libplatform primitives

#undef memset
PTHREAD_NOEXPORT
void *
memset(void *b, int c, size_t len)
{
	return _platform_memset(b, c, len);
}

#undef bzero
PTHREAD_NOEXPORT
void
bzero(void *s, size_t n)
{
	_platform_bzero(s, n);
}

#undef memcpy
PTHREAD_NOEXPORT
void *
memcpy(void* a, const void* b, unsigned long s)
{
	return _platform_memmove(a, b, s);
}

#endif // !VARIANT_DYLD
