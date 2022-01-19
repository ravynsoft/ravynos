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
 * Revision 2.8.2.3  92/09/15  17:23:39  jeffreyh
 * 	Added mach/prof_types.h
 * 	[92/07/24            bernadat]
 * 
 * Revision 2.8.2.2  92/06/24  18:05:31  jeffreyh
 * 	Added host_paging_t for NORMA_IPC
 * 	[92/06/17            jeffreyh]
 * 
 * Revision 2.8.2.1  92/02/21  11:24:01  jsb
 * 	NORMA_VM: define mach_xmm_obj_t and xmm_kobj_lookup().
 * 	[92/02/10  08:47:29  jsb]
 * 
 * Revision 2.8  91/06/25  10:30:20  rpd
 * 	Added KERNEL-compilation includes for *_array_t types.
 * 	[91/05/23            rpd]
 * 
 * Revision 2.7  91/06/06  17:08:07  jsb
 * 	Added emulation_vector_t for new get/set emulation vector calls.
 * 	[91/05/24  17:46:31  jsb]
 * 
 * Revision 2.6  91/05/14  16:55:17  mrt
 * 	Correcting copyright
 * 
 * Revision 2.5  91/02/05  17:33:43  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:18:38  mrt]
 * 
 * Revision 2.4  90/08/07  18:00:30  rpd
 * 	Added processor_set_name_array_t.
 * 	Removed vm_region_t, vm_region_array_t.
 * 	[90/08/07            rpd]
 * 
 * Revision 2.3  90/06/02  14:58:42  rpd
 * 	Converted to new IPC.
 * 	[90/03/26  22:33:59  rpd]
 * 
 * Revision 2.2  90/01/22  23:05:48  af
 * 	Added inclusion of vm_attributes.
 * 	[89/12/09            af]
 * 
 * 	Moved KERNEL type definitions into kern/mach_types_kernel.h, so
 * 	that changing them will not affect user programs.
 * 	[89/04/06            dbg]
 * 
 * 	Removed io_buf_t, io_buf_ptr_t in favor of device interface.
 * 	Removed include of ipc_netport.h.  Removed vm_page_data_t
 * 	(obsolete).
 * 	[89/01/14            dbg]
 * 
 * Revision 2.1  89/08/03  16:02:27  rwd
 * Created.
 * 
 * Revision 2.3  89/02/25  18:38:04  gm0w
 * 	Changes for cleanup.
 * 
 * Revision 2.2  89/01/15  16:30:50  rpd
 * 	Moved from kern/ to mach/.
 * 	[89/01/15  14:35:53  rpd]
 * 
 * Revision 2.10  89/01/12  11:15:18  rpd
 * 	Removed pointer_t declaration; it belongs in std_types.h.
 * 
 * Revision 2.9  89/01/12  07:57:53  rpd
 * 	Moved basic stuff to std_types.h.  Removed debugging definitions.
 * 	Moved io_buf definitions to device_types.h.
 * 	[89/01/12  04:51:54  rpd]
 * 
 * Revision 2.8  89/01/04  13:37:34  rpd
 * 	Include <kern/fpa_counters.h>, for fpa_counters_t.
 * 	[89/01/01  15:03:52  rpd]
 * 
 * Revision 2.7  88/09/25  22:15:28  rpd
 * 	Changed sys/callout.h to kern/callout_statistics.h.
 * 	[88/09/09  14:00:19  rpd]
 * 	
 * 	Changed includes to the new style.
 * 	Added include of sys/callout.h.
 * 	[88/09/09  04:47:42  rpd]
 * 
 * Revision 2.6  88/08/06  18:22:34  rpd
 * Changed sys/mach_ipc_netport.h to kern/ipc_netport.h.
 * 
 * Revision 2.5  88/07/21  00:36:06  rpd
 * Added include of ipc_statistics.h.
 * 
 * Revision 2.4  88/07/17  19:33:20  mwyoung
 * *** empty log message ***
 * 
 * 29-Jun-88  Michael Young (mwyoung) at Carnegie-Mellon University
 *	Use new <mach/memory_object.h>.
 *
 *  9-Apr-88  Daniel Julin (dpj) at Carnegie-Mellon University
 *	Changed mach_ipc_vmtp.h to mach_ipc_netport.h.
 *
 *  1-Mar-88  Mary Thompson (mrt) at Carnegie Mellon
 *	Added a conditional on _MACH_INIT_ before the include
 *	of mach_init.h so that the kernel make of mach_user_internal
 *	would not include mach_init.h
 *
 * 18-Jan-88  David Golub (dbg) at Carnegie-Mellon University
 *	Added includes of task_info, thread_info, task_special_ports,
 *	thread_special_ports for new interfaces.
 *
 * 12-Dec-87  Michael Young (mwyoung) at Carnegie-Mellon University
 *	Reduced old history.
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
 *	File:	mach/mach_types.h
 *	Author:	Avadis Tevanian, Jr., Michael Wayne Young
 *	Date:	1986
 *
 *	Mach external interface definitions.
 *
 */


#ifndef	_MACH_MACH_TYPES_H_
#define _MACH_MACH_TYPES_H_
#include <sys/cdefs.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/lock.h>
#include <sys/mutex.h>
#include <sys/sx.h>
#include <sys/malloc.h>
#include <vm/vm.h>

#ifndef THREAD_STATE_MAX
#define THREAD_STATE_MAX 32
#endif

#include <sys/mach/host_info.h>
#include <sys/mach/machine.h>
#include <sys/mach/memory_object.h>
#include <sys/mach/exception_types.h>
#include <sys/mach/port.h>
#include <sys/mach/processor_info.h>
#include <sys/mach/task_info.h>
#include <sys/mach/task_policy.h>
#include <sys/mach/task_special_ports.h>
#include <sys/mach/thread_info.h>
#include <sys/mach/thread_special_ports.h>
#include <sys/mach/thread_status.h>
#include <sys/mach/time_value.h>
#include <sys/mach/clock_types.h>
#include <sys/mach/kern_return.h>
#include <sys/mach/mach_voucher_types.h>
#include <sys/mach/vm_attributes.h>
#include <sys/mach/vm_inherit.h>
#include <sys/mach/vm_behavior.h>
#include <sys/mach/vm_prot.h>
#include <sys/mach/vm_statistics.h>
#include <sys/mach/vm_sync.h>
#include <sys/mach/vm_region.h>
#include <sys/mach/prof_types.h>
#include <sys/mach/host_notify.h>

#define __MigPackStructs 1

#ifdef	_KERNEL

#define __MigTypeCheck 1
#define __MigKernelSpecificCode 1

typedef struct ipc_space	*ipc_space_t;
typedef struct mach_task                     *task_name_t, *task_suspension_token_t;
typedef struct thread_shuttle *thread_t, *thread_act_t;
typedef struct coalition                *coalition_t;
typedef struct host                     *host_t;
typedef struct host                     *host_priv_t;
typedef struct host                     *host_security_t;
typedef struct processor_set            *processor_set_t, *processor_set_control_t;
typedef struct semaphore                *semaphore_t;
typedef struct alarm                    *alarm_t;
typedef struct clock                    *clock_serv_t;
typedef struct clock                    *clock_ctrl_t;
typedef struct ledger					*ledger_t;
typedef processor_set_t         processor_set_name_t;
MALLOC_DECLARE(M_MACH_TMP);
#ifdef MACH_CORRUPTION_DEBUG
#define M_MACH_IPC_KMSG M_MACH_TMP
#define M_MACH_IPC_ENTRY M_MACH_TMP
#define M_MACH_IPC_TABLE M_MACH_TMP
#define M_MACH_KALLOC M_MACH_TMP
#define M_MACH_VM M_MACH_TMP
#else
MALLOC_DECLARE(M_MACH_IPC_KMSG);
MALLOC_DECLARE(M_MACH_IPC_ENTRY);
MALLOC_DECLARE(M_MACH_IPC_TABLE);
MALLOC_DECLARE(M_MACH_KALLOC);
MALLOC_DECLARE(M_MACH_VM);
#endif
#else	/* MACH_KERNEL */

typedef mach_port_t		task_t;
typedef mach_port_t		task_port_t;
typedef mach_port_t		task_name_t;
typedef mach_port_t		lock_set_t;
typedef mach_port_t		semaphore_t;
typedef mach_port_t     ledger_t;
typedef mach_port_t		processor_t;
typedef mach_port_t		processor_set_t;
typedef mach_port_t		processor_set_control_t;
typedef mach_port_t		alarm_t;
typedef mach_port_t		clock_serv_t;
typedef mach_port_t		clock_ctrl_t;
typedef mach_port_t		io_master_t;
typedef mach_port_t		ipc_space_t;

typedef mach_port_t             task_suspension_token_t;
typedef mach_port_t             host_t;
typedef mach_port_t             host_priv_t;
typedef mach_port_t             host_security_t;
typedef host_t                  host_name_t;
typedef host_t                  host_name_port_t;

typedef	task_port_t		*task_port_array_t;
typedef mach_port_t		thread_port_t;
typedef	thread_port_t		*thread_port_array_t;
typedef mach_port_t		processor_set_control_port_t;
typedef mach_port_t		processor_set_name_port_t;
typedef vm_offset_t		*emulation_vector_t;
typedef	mach_port_t		thread_t, thread_act_t;
typedef mach_port_t		thread_act_port_t;
typedef	thread_act_port_t	*thread_act_port_array_t;
typedef mach_port_t		lock_set_port_t;
typedef mach_port_t		semaphore_port_t;
typedef mach_port_t		security_port_t;
typedef integer_t		ledger_item_t;
typedef mach_port_t		*processor_array_t;
typedef processor_set_t         processor_set_name_t;
typedef processor_set_name_t		*processor_set_name_array_t;

#endif	/* MACH_KERNEL */
typedef mach_port_t           clock_reply_t;
typedef        mach_port_t             mem_entry_name_port_t;
typedef ledger_t               *ledger_array_t;
typedef thread_act_t            *thread_act_array_t;


typedef void * kmod_args_t;
typedef mach_port_t             exception_handler_t;
typedef exception_handler_t     *exception_handler_array_t;
typedef int kmod_t;
typedef int kmod_control_flavor_t;


typedef mach_port_t UNDServerRef;
typedef mach_port_t		*ledger_port_array_t;
typedef mach_port_t    		ledger_port_t;
typedef char			*user_subsystem_t;
typedef int mach_clock_res_t;
typedef int mach_sleep_type_t;
typedef int mach_absolute_time_t;




/*
 *	Backwards compatibility, for those programs written
 *	before mach/{std,mach}_types.{defs,h} were set up.
 */
#include <sys/mach/std_types.h>
#include <sys/mach/mach_time.h>
#ifdef _KERNEL
#include <sys/mach/processor.h>	/* for processor_array_t,
				       processor_set_array_t,
				       processor_set_name_array_t */

#ifdef __MIG_check__Request__vm_map_subsystem__
#include <sys/mach/ipc/ipc_kmsg.h>
#include <sys/mach/mach_vm_server.h>

int mach_vm_map_page_query(vm_map_t target_map, vm_offset_t offset, integer_t *disposition, integer_t *ref_count);
int mach_vm_mapped_pages_info(vm_map_t task, page_address_array_t *pages, mach_msg_type_number_t *pagesCnt);


#define vm_copy(a0, a1, a2, a3) mach_vm_copy(a0, a1, a2, a3)
#define vm_map(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10) mach_vm_map(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10)
#define vm_inherit(a0, a1, a2, a3) mach_vm_inherit(a0, a1, a2, a3)
#define vm_map_64(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10) mach_vm_map(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10)
#define vm_msync(a0, a1, a2, a3) mach_vm_msync(a0, a1, a2, a3)
#define vm_protect(a0, a1, a2, a3, a4) mach_vm_protect(a0, a1, a2, a3, a4)
#define vm_read(target_task, address, size, data, dataCnt) mach_vm_read(target_task, address, size, data, dataCnt)
#ifdef notyet
#define vm_read_list(target_task, data_list, count) mach_vm_read_list(target_task, (mach_vm_read_entry_t)&data_list, count)
#endif
#define vm_read_overwrite(target_task, address, size, data, outsize) mach_vm_read_overwrite(target_task, address, size, data, outsize)
#define vm_region(a0, a1, a2, a3, a4, a5, a6) mach_vm_region(a0, a1, a2, a3, a4, a5, a6)
#define vm_map(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10) mach_vm_map(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10)
#define vm_write(target_task, address, data, dataCnt) mach_vm_write(target_task, address, data, dataCnt)
#define vm_machine_attribute(target_task, address, size, attribute, value) mach_vm_machine_attribute(target_task, address, size, attribute, value)
#define vm_purgable_control(target_task, address, control, state) mach_vm_purgable_control(target_task, address, control, state)
#define vm_behavior_set(target_task, address, size, new_behavior) mach_vm_behavior_set(target_task, address, size, new_behavior) 
#define vm_remap(target_task, target_address, size, mask, flags, src_task, src_address, copy, cur_protection, max_protection, inheritance) mach_vm_remap(target_task, target_address, size, mask, flags, src_task, src_address, copy, cur_protection, max_protection, inheritance)
#define vm_region_64(target_task, address, size, flavor, info, infoCnt, object_name)  mach_vm_region(target_task, address, size, flavor, info, infoCnt, object_name)
#define vm_region_recurse(target_task, address, size, nesting_depth, info, infoCnt) mach_vm_region_recurse(target_task, address, size, nesting_depth, info, infoCnt)
#define vm_region_recurse_64(target_task, address, size, nesting_depth, info, infoCnt) mach_vm_region_recurse(target_task, address, size, nesting_depth, info, infoCnt)
#define vm_map_page_query(target_map, offset, disposition, ref_count) mach_vm_map_page_query(target_map, offset, disposition, ref_count)
#define vm_mapped_pages_info(task, pages, pagesCnt) mach_vm_mapped_pages_info(task, pages, pagesCnt)

#endif
#ifdef __MIG_check__Request__host_priv_subsystem__
#include <sys/mach/ipc/ipc_kmsg.h>
#include <sys/mach/mach_vm_server.h>
int mach_vm_wire_32(host_priv_t host_priv, vm_map_t task, vm_address_t address, vm_size_t size, vm_prot_t desired_access);

#define vm_wire(host_priv, task, address, size, desired_access) mach_vm_wire_32(host_priv, task, address, size, desired_access)	

#endif
#endif
#endif	/* _MACH_MACH_TYPES_H_ */
