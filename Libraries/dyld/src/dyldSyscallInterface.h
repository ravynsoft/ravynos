/* -*- mode: C++; c-basic-offset: 4; tab-width: 4 -*-
 *
 * Copyright (c) 2004-2013 Apple Inc. All rights reserved.
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


#ifndef __DYLD_SYSCALL_HELPERS__
#define __DYLD_SYSCALL_HELPERS__

#include <sys/cdefs.h>

#include <dirent.h>
#if __has_include(<libamfi.h>)
#include <libamfi.h>
#else
__BEGIN_DECLS
extern int amfi_check_dyld_policy_self(uint64_t input_flags, uint64_t* output_flags);
__END_DECLS
#endif
#include <libkern/OSAtomic.h>
#include <libproc.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <mach-o/loader.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <sys/kdebug_private.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <System/sys/reason.h>

#define DYLD_SYSCALL_VTABLE_ENTRY(x) __typeof__ (x) *x

#if __cplusplus
namespace dyld {
#endif
	//
	// This file contains the table of function pointers the host dyld supplies
	// to the iOS simulator dyld.
	//
	struct SyscallHelpers
	{
		uintptr_t		version;
		DYLD_SYSCALL_VTABLE_ENTRY(open);
		DYLD_SYSCALL_VTABLE_ENTRY(close);
		DYLD_SYSCALL_VTABLE_ENTRY(pread);
		DYLD_SYSCALL_VTABLE_ENTRY(write);
		DYLD_SYSCALL_VTABLE_ENTRY(mmap);
		DYLD_SYSCALL_VTABLE_ENTRY(munmap);
		DYLD_SYSCALL_VTABLE_ENTRY(madvise);
		DYLD_SYSCALL_VTABLE_ENTRY(stat);
		DYLD_SYSCALL_VTABLE_ENTRY(fcntl);
		DYLD_SYSCALL_VTABLE_ENTRY(ioctl);
		DYLD_SYSCALL_VTABLE_ENTRY(issetugid);
		DYLD_SYSCALL_VTABLE_ENTRY(getcwd);
		DYLD_SYSCALL_VTABLE_ENTRY(realpath);
		DYLD_SYSCALL_VTABLE_ENTRY(vm_allocate);
		DYLD_SYSCALL_VTABLE_ENTRY(vm_deallocate);
		DYLD_SYSCALL_VTABLE_ENTRY(vm_protect);
		void			(*vlog)(const char* format, va_list list);
		void			(*vwarn)(const char* format, va_list list);
		DYLD_SYSCALL_VTABLE_ENTRY(pthread_mutex_lock);
		DYLD_SYSCALL_VTABLE_ENTRY(pthread_mutex_unlock);
		DYLD_SYSCALL_VTABLE_ENTRY(mach_thread_self);
		DYLD_SYSCALL_VTABLE_ENTRY(mach_port_deallocate);
		DYLD_SYSCALL_VTABLE_ENTRY(task_self_trap);
		DYLD_SYSCALL_VTABLE_ENTRY(mach_timebase_info);
#if OSATOMIC_USE_INLINED
		bool			(*OSAtomicCompareAndSwapPtrBarrier)(void* old, void* nw, void * volatile *value);
		void			(*OSMemoryBarrier)(void);
#else
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated"
		DYLD_SYSCALL_VTABLE_ENTRY(OSAtomicCompareAndSwapPtrBarrier);
		DYLD_SYSCALL_VTABLE_ENTRY(OSMemoryBarrier);
#pragma clang diagnostic pop
#endif
		void*			(*getProcessInfo)(void); // returns dyld_all_image_infos*;
		int*			(*errnoAddress)(void);
		DYLD_SYSCALL_VTABLE_ENTRY(mach_absolute_time);
		// Added in version 2
		DYLD_SYSCALL_VTABLE_ENTRY(thread_switch);
		// Added in version 3
		DYLD_SYSCALL_VTABLE_ENTRY(opendir);
		DYLD_SYSCALL_VTABLE_ENTRY(readdir_r);
		DYLD_SYSCALL_VTABLE_ENTRY(closedir);
		// Added in version 4
		void			(*coresymbolication_load_notifier)(void *connection, uint64_t load_timestamp, const char *image_path, const struct mach_header *mach_header);
		void			(*coresymbolication_unload_notifier)(void *connection, uint64_t unload_timestamp, const char *image_path, const struct mach_header *mach_header);
		// Added in version 5
		DYLD_SYSCALL_VTABLE_ENTRY(proc_regionfilename);
		DYLD_SYSCALL_VTABLE_ENTRY(getpid);
		DYLD_SYSCALL_VTABLE_ENTRY(mach_port_insert_right);
		DYLD_SYSCALL_VTABLE_ENTRY(mach_port_allocate);
		DYLD_SYSCALL_VTABLE_ENTRY(mach_msg);
		// Added in version 6
		DYLD_SYSCALL_VTABLE_ENTRY(abort_with_payload);
		// Add in version 7
		DYLD_SYSCALL_VTABLE_ENTRY(task_register_dyld_image_infos);
		DYLD_SYSCALL_VTABLE_ENTRY(task_unregister_dyld_image_infos);
		DYLD_SYSCALL_VTABLE_ENTRY(task_get_dyld_image_infos);
		DYLD_SYSCALL_VTABLE_ENTRY(task_register_dyld_shared_cache_image_info);
		DYLD_SYSCALL_VTABLE_ENTRY(task_register_dyld_set_dyld_state);
		DYLD_SYSCALL_VTABLE_ENTRY(task_register_dyld_get_process_state);
		DYLD_SYSCALL_VTABLE_ENTRY(task_info);
		DYLD_SYSCALL_VTABLE_ENTRY(thread_info);
		// Add in version 8
		DYLD_SYSCALL_VTABLE_ENTRY(kdebug_is_enabled);
		DYLD_SYSCALL_VTABLE_ENTRY(kdebug_trace);
		// Add in version 9
		DYLD_SYSCALL_VTABLE_ENTRY(kdebug_trace_string);
		// Add in version 10
		DYLD_SYSCALL_VTABLE_ENTRY(amfi_check_dyld_policy_self);
		// Add in version 11
		void			(*notifyMonitoringDyldMain)(void);
		void			(*notifyMonitoringDyld)(bool unloading, unsigned imageCount, const struct mach_header* loadAddresses[], const char* imagePaths[]);
		// Add in version 12
		DYLD_SYSCALL_VTABLE_ENTRY(mach_msg_destroy);
		DYLD_SYSCALL_VTABLE_ENTRY(mach_port_construct);
		DYLD_SYSCALL_VTABLE_ENTRY(mach_port_destruct);
	};
	extern const struct SyscallHelpers* gSyscallHelpers;


#if __cplusplus
}
#endif

#endif
