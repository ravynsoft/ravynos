/*
 * Copyright 1991-1998 by Open Software Foundation, Inc. 
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
 */
/*
 * MkLinux
 */
/* CMU_HIST */
/*
 * Revision 2.4  91/06/25  10:30:50  rpd
 * 	Added ipc/ipc_port.h inside the kernel.
 * 	[91/05/27            rpd]
 * 
 * Revision 2.3  91/05/14  16:59:01  mrt
 * 	Correcting copyright
 * 
 * Revision 2.2  91/02/05  17:35:39  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:20:57  mrt]
 * 
 * Revision 2.1  89/08/03  16:04:44  rwd
 * Created.
 * 
 * Revision 2.3  89/02/25  18:40:23  gm0w
 * 	Changes for cleanup.
 * 
 * Revision 2.2  89/01/15  16:31:59  rpd
 * 	Moved from kern/ to mach/.
 * 	[89/01/15  14:34:14  rpd]
 * 
 * Revision 2.2  89/01/12  07:59:07  rpd
 * 	Created.
 * 	[89/01/12  04:15:40  rpd]
 * 
 */
/* CMU_ENDHIST */
/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989,1988 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 * 
 * Carnegie Mellon requests users of this software to return to
 * 
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 * 
 * any improvements or extensions that they make and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */
/*
 */
/*
 *	Mach standard external interface type definitions.
 *
 */

#ifndef	STD_TYPES_H_
#define	STD_TYPES_H_
#define MACH_KDB 0
#define MACH_COUNTERS 0
#define ZONE_DEBUG 0
#include <sys/mach/kern_return.h>
#include <sys/mach/port.h>
#include <sys/mach/vm_types.h>

#ifdef _KERNEL
#include <sys/mach/mach_vm.h>
#include <sys/cdefs.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/lock.h>
#include <sys/mutex.h>


#ifndef MACH_VERBOSE_DEBUGGING
#define MACH_VERBOSE_DEBUGGING 0
#endif

#ifndef MDPRINTF
#if MACH_VERBOSE_DEBUGGING
#define MDPRINTF(x) printf x
#else
#define MDPRINTF(...)
#endif
#endif

extern int mach_debug_enable;

#define decl_mutex_data(__annot, __lock) __annot struct mtx __lock;
#define assert(exp) KASSERT(exp, (#exp))
#define MACH_VERIFY(exp, str) do {					\
		if (!(exp)) printf str;				\
	} while (0) 
#define mach_mutex_init(a, b) mtx_init(a, b, NULL, MTX_DEF|MTX_DUPOK)
#define MACH_IPC_DEBUG 0
#define MACH_DEBUG 0
#define MACH_MACHINE_ROUTINES 0
#define BYTE_SIZE NBBY
#define THREAD_NULL NULL
#define XK_PROXY 0
#define NORMA_IPC 0
#define VM_MAP_COPYIN_OPT_SRC_DESTROY 0
#define VM_MAP_COPYIN_OPT_STEAL_PAGES 0
#define VM_MAP_COPYIN_OPT_STEAL_PAGES 0
#define VM_MAP_REMOVE_WAIT_FOR_KWIRE 0
#define VM_MAP_REMOVE_INTERRUPTIBLE 0
#define page_aligned(addr) ((addr & PAGE_MASK) == 0)
/* XXX FIX THIS */
#define zinit(size1, sizemax, size2, name) uma_zcreate(name, size1, NULL, NULL, NULL, NULL, 0, 0)
#define zone_change(zone, flag, val)
#define kalloc(size) malloc(size, M_MACH_KALLOC, M_WAITOK)
#define KALLOC(size, rt) ((vm_offset_t)malloc(size, M_MACH_KALLOC, M_WAITOK))
#define kfree(ptr, size) free((void *)(ptr), M_MACH_KALLOC)
#define KFREE(ptr, size, rt) free((void *)(ptr), M_MACH_KALLOC)
#define copyinmsg copyin
#define copyoutmsg copyout
#if MACH_DEBUG && defined(INVARIANTS)
#define UNSUPPORTED { panic("%s not supported", __FUNCTION__); return (KERN_NOT_SUPPORTED); }
#elif defined(INVARIANTS)
#define UNSUPPORTED { if (mach_debug_enable) {printf("%s not supported", __FUNCTION__);} return (KERN_NOT_SUPPORTED); }
#else
#define UNSUPPORTED { return (KERN_NOT_SUPPORTED); }
#endif
/* drop reference */
#define vm_map_copyin_page_list(map, addr, length, options, etc0, etc1) 0
#define vm_map_deallocate(map)
#define vm_object_pager_wakeup(map)
#define ds_notify(map) 0
#define ds_master_notify(map) 0
#define vm_map_copy_steal_pages(copy)
#define thread_deallocate(thread)
#define task_name_deallocate(task)
#define assert_wait(a, b)
#define cpu_number() curcpu
#define copyoutmap(a, b, c, d) copyout((const void *)b, (void *)c, d)
#define copyinmap(a, b, c, d) copyin((const void *)b, (void *)c, d)

#define decl_simple_lock_data(a, b) a struct mtx b;
	
#include <vm/vm.h>

#define VM_MAP_COPY_PAGE_LIST_MAX	20
#define	VM_MAP_COPY_PAGE_LIST_MAX_SIZE	(VM_MAP_COPY_PAGE_LIST_MAX * PAGE_SIZE)


#include <sys/mach/macro_help.h>
#else
#define decl_simple_lock_data(a, b)
#define decl_mutex_data(__annot, __lock)
#endif
#endif	/* STD_TYPES_H_ */
