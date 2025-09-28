/*-
 * Copyright (c) 2014-2015, Matthew Macy <mmacy@nextbsd.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *
 *  2. Neither the name of Matthew Macy nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/param.h>
#include <sys/event.h>
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/syscall.h>
#include <sys/sysctl.h>
#include <sys/sysent.h>
#include <sys/sysproto.h>
#include <sys/types.h>
#include <sys/systm.h>


int mach_debug_enable;

SYSCTL_ROOT_NODE(OID_AUTO,  mach, CTLFLAG_RW, 0,
	"mach subsystem parameters");

SYSCTL_INT(_mach, OID_AUTO, debug_enable, CTLFLAG_RWTUN,
		   &mach_debug_enable, 0, "enable mach debug logging");


extern struct filterops machport_filtops;

static struct syscall_helper_data osx_syscalls[] = {
	SYSCALL_INIT_HELPER(__proc_info),
	SYSCALL_INIT_HELPER(__iopolicysys),
	SYSCALL_INIT_HELPER(_kernelrpc_mach_vm_allocate_trap),
	SYSCALL_INIT_HELPER(_kernelrpc_mach_vm_deallocate_trap),
	SYSCALL_INIT_HELPER(_kernelrpc_mach_vm_protect_trap),
	SYSCALL_INIT_HELPER(_kernelrpc_mach_vm_map_trap),
	SYSCALL_INIT_HELPER(_kernelrpc_mach_port_allocate_trap),
	SYSCALL_INIT_HELPER(_kernelrpc_mach_port_destroy_trap),
	SYSCALL_INIT_HELPER(_kernelrpc_mach_port_deallocate_trap),
	SYSCALL_INIT_HELPER(_kernelrpc_mach_port_mod_refs_trap),
	SYSCALL_INIT_HELPER(_kernelrpc_mach_port_move_member_trap),
	SYSCALL_INIT_HELPER(_kernelrpc_mach_port_insert_right_trap),
	SYSCALL_INIT_HELPER(_kernelrpc_mach_port_insert_member_trap),
	SYSCALL_INIT_HELPER(_kernelrpc_mach_port_extract_member_trap),
	SYSCALL_INIT_HELPER(_kernelrpc_mach_port_construct_trap),
	SYSCALL_INIT_HELPER(_kernelrpc_mach_port_destruct_trap),
	SYSCALL_INIT_HELPER(mach_reply_port),
	SYSCALL_INIT_HELPER(thread_self_trap),
	SYSCALL_INIT_HELPER(task_self_trap),
	SYSCALL_INIT_HELPER(host_self_trap),
	SYSCALL_INIT_HELPER(mach_msg_trap),
	SYSCALL_INIT_HELPER(mach_msg_overwrite_trap),
	SYSCALL_INIT_HELPER(semaphore_signal_trap),
	SYSCALL_INIT_HELPER(semaphore_signal_all_trap),
	SYSCALL_INIT_HELPER(semaphore_signal_thread_trap),
	SYSCALL_INIT_HELPER(semaphore_wait_trap),
	SYSCALL_INIT_HELPER(semaphore_wait_signal_trap),
	SYSCALL_INIT_HELPER(semaphore_timedwait_signal_trap),
	SYSCALL_INIT_HELPER(_kernelrpc_mach_port_guard_trap),
	SYSCALL_INIT_HELPER(_kernelrpc_mach_port_unguard_trap),
	SYSCALL_INIT_HELPER(task_for_pid),
	SYSCALL_INIT_HELPER(pid_for_task),
	SYSCALL_INIT_HELPER(macx_swapon),
	SYSCALL_INIT_HELPER(macx_swapoff),
	SYSCALL_INIT_HELPER(macx_triggers),
	SYSCALL_INIT_HELPER(macx_backing_store_suspend),
	SYSCALL_INIT_HELPER(macx_backing_store_recovery),
	SYSCALL_INIT_HELPER(swtch_pri),
	SYSCALL_INIT_HELPER(swtch),
	SYSCALL_INIT_HELPER(thread_switch),
	SYSCALL_INIT_HELPER(clock_sleep_trap),
	SYSCALL_INIT_HELPER(mach_timebase_info),
	SYSCALL_INIT_HELPER(mach_wait_until),
	SYSCALL_INIT_HELPER(mk_timer_create),
	SYSCALL_INIT_HELPER(mk_timer_destroy),
	SYSCALL_INIT_HELPER(mk_timer_arm),
	SYSCALL_INIT_HELPER(mk_timer_cancel),
	SYSCALL_INIT_LAST
};

static int
mach_mod_init(void)
{
	int err;

	if (!cold) {
		printf("mach services can only be loaded at boot time\n");
		return (EINVAL);
	}

	if ((err = syscall_helper_register(osx_syscalls, SY_THR_STATIC_KLD))) {
		printf("failed to register osx calls: %d\n", err);
		return (EINVAL);
	}
	if (kqueue_add_filteropts(EVFILT_MACHPORT, &machport_filtops)) {
		printf("failed to register machport_filtops\n");
		return (EINVAL);
	}
	return (0);
}

static int
mach_module_event_handler(module_t mod, int what, void *arg)
{
	int err;

	switch (what) {
	case MOD_LOAD:
		if ((err = mach_mod_init()) != 0) {
			printf("mach services failed to load - mach system calls will not be available\n");
			return (err);
		}
		break;
	case MOD_UNLOAD:
		return (EBUSY);
	default:
		return (EOPNOTSUPP);
	}
	printf("mach services loaded - mach system calls available\n");
	return (0);
}

static moduledata_t mach_moduledata = {
	"mach",
	mach_module_event_handler,
	NULL
};

DECLARE_MODULE(mach, mach_moduledata, SI_SUB_KLD, SI_ORDER_ANY);



