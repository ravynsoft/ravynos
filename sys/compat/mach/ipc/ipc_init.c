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
 * Revision 2.12.2.2  92/03/03  16:18:29  jeffreyh
 * 	19-Feb-92 David L. Black (dlb) at Open Software Foundation
 * 	Don't make port zone exhaustible.  Kernel panics when
 * 	it fails to allocate an internal port after exhaustion.
 * 	[92/02/26  11:41:12  jeffreyh]
 * 
 * Revision 2.12.2.1  92/01/03  16:34:52  jsb
 * 	Corrected log.
 * 	[91/12/24  14:18:23  jsb]
 * 
 * Revision 2.12  91/12/10  13:25:41  jsb
 * 	Removed reference counting bug workaround.
 * 	[91/12/10  11:17:21  jsb]
 * 
 * Revision 2.11  91/11/19  09:54:12  rvb
 * 	Added reference counting bug workaround.
 * 	[91/11/00            jsb]
 * 
 * Revision 2.10  91/08/03  18:18:12  jsb
 * 	Removed call to ipc_clport_init.
 * 	[91/07/24  22:11:04  jsb]
 * 
 * Revision 2.9  91/06/17  15:46:00  jsb
 * 	Renamed NORMA conditionals.
 * 	[91/06/17  10:45:48  jsb]
 * 
 * Revision 2.8  91/06/06  17:05:46  jsb
 * 	Added call to ipc_clport_init.
 * 	[91/05/13  17:17:10  jsb]
 * 
 * Revision 2.7  91/05/14  16:32:33  mrt
 * 	Correcting copyright
 * 
 * Revision 2.6  91/02/05  17:21:37  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  15:45:02  mrt]
 * 
 * Revision 2.5  91/01/08  15:13:40  rpd
 * 	Changed zchange calls to make the IPC zones non-collectable.
 * 	[90/12/29            rpd]
 * 
 * Revision 2.4  90/12/20  16:38:41  jeffreyh
 * 	Changes to zchange to account for new collectable field. Made all
 * 	ipc zones collectable.
 * 	[90/12/11            jeffreyh]
 * 
 * Revision 2.3  90/09/28  16:54:44  jsb
 * 	Added NORMA_IPC support.
 * 	[90/09/28  14:02:05  jsb]
 * 
 * Revision 2.2  90/06/02  14:49:55  rpd
 * 	Created for new IPC.
 * 	[90/03/26  20:55:13  rpd]
 * 
 */
/* CMU_ENDHIST */
/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989 Carnegie Mellon University
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
 *	File:	ipc/ipc_init.c
 *	Author:	Rich Draves
 *	Date:	1989
 *
 *	Functions to initialize the IPC system.
 */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/lock.h>
#include <sys/mutex.h>

#include <sys/mach/kern_return.h>
#include <sys/mach/mach_param.h>
#include <sys/mach/mach_types.h>

#include <sys/mach/ipc/ipc_entry.h>
#include <sys/mach/ipc/ipc_space.h>
#include <sys/mach/ipc/ipc_object.h>
#include <sys/mach/ipc/ipc_port.h>
#include <sys/mach/ipc/ipc_pset.h>
#include <sys/mach/ipc/ipc_notify.h>
#include <sys/mach/ipc/ipc_kmsg.h>
#include <sys/mach/ipc/ipc_hash.h>
#include <sys/mach/ipc/ipc_init.h>
#include <sys/mach/ndr_def.h>   /* NDR_record */

vm_map_t ipc_kernel_map;
vm_size_t ipc_kernel_map_size = 1024 * 1024;

vm_map_t ipc_kernel_copy_map;
#define IPC_KERNEL_COPY_MAP_SIZE (8 * 1024 * 1024)
vm_size_t ipc_kernel_copy_map_size = IPC_KERNEL_COPY_MAP_SIZE;
vm_size_t ipc_kmsg_max_vm_space = (IPC_KERNEL_COPY_MAP_SIZE * 7)/8;

uma_zone_t ipc_kmsg_zone;


/*
 * values to limit inline message body handling
 * avoid copyin/out limits - even after accounting for maximum descriptor expansion.
 */
#define IPC_KMSG_MAX_SPACE (64 * 1024 * 1024) /* keep in sync with COPYSIZELIMIT_PANIC */
vm_size_t ipc_kmsg_max_body_space = ((IPC_KMSG_MAX_SPACE * 3)/4 - MAX_TRAILER_SIZE);

int ipc_space_max = SPACE_MAX;
int ipc_port_max = PORT_MAX;
int ipc_pset_max = SET_MAX;


#define MSG_OOL_SIZE_SMALL 16385
vm_size_t msg_ool_size_small;


extern void mig_init(void);

/*
 *	Routine:	ipc_bootstrap
 *	Purpose:
 *		Initialization needed before the kernel task
 *		can be created.
 */

static void
ipc_bootstrap_sysinit(void *arg __unused)
{
	kern_return_t kr;

	ipc_port_multiple_lock_init();

	ipc_port_timestamp_lock_init();
	ipc_port_timestamp_data = 0;

	/* all IPC zones should be exhaustible */

	ipc_space_zone = uma_zcreate("ipc_space_zone", sizeof(struct ipc_space),
								 NULL, NULL, NULL, NULL, 1, 0);
	ipc_kmsg_zone = uma_zcreate("ipc_kmsg_zone", IKM_SAVED_MSG_SIZE,
								NULL, NULL, NULL, NULL, 1, 0);
    /*
	 * populate all port(set) zones
	 */
	ipc_object_zones[IOT_PORT] =
		zinit(sizeof(struct ipc_port),
		      ipc_port_max * sizeof(struct ipc_port),
		      sizeof(struct ipc_port),
		      "ipc ports");
	/*
	 * XXX  Can't make the port zone exhaustible because the kernel
	 * XXX	panics when port allocation for an internal object fails.
	 *zone_change(ipc_object_zones[IOT_PORT], Z_EXHAUST, TRUE);
	 */

	ipc_object_zones[IOT_PORT_SET] =
		zinit(sizeof(struct ipc_pset),
		      ipc_pset_max * sizeof(struct ipc_pset),
		      sizeof(struct ipc_pset),
		      "ipc port sets");
	/* make it exhaustible */
	zone_change(ipc_object_zones[IOT_PORT_SET], Z_EXHAUST, TRUE);

	/* create special spaces */

	kr = ipc_space_create_special(&ipc_space_kernel);
	assert(kr == KERN_SUCCESS);


	kr = ipc_space_create_special(&ipc_space_reply);
	assert(kr == KERN_SUCCESS);

	/* initialize modules with hidden data structures */

	mig_init();
	ipc_table_init();
	ipc_notify_init();
	ipc_hash_init();
	msg_ool_size_small = MSG_OOL_SIZE_SMALL;
}


/*
 *	Routine:	ipc_init
 *	Purpose:
 *		Final initialization of the IPC system.
 */

void
ipc_init(void)
{
#if 0	
	kern_return_t retval;
	vm_offset_t min, max;
	extern vm_size_t kalloc_max_prerounded;

	retval = kmem_suballoc(kernel_map, &min, ipc_kernel_map_size,
			       TRUE, TRUE, &ipc_kernel_map);
	if (retval != KERN_SUCCESS)
		panic("ipc_init: kmem_suballoc of ipc_kernel_map failed");

	retval = kmem_suballoc(kernel_map, &min, ipc_kernel_copy_map_size,
			       TRUE, TRUE, &ipc_kernel_copy_map);
	if (retval != KERN_SUCCESS)
		panic("ipc_init: kmem_suballoc of ipc_kernel_copy_map failed");

	ipc_kernel_copy_map->no_zero_fill = TRUE;
	ipc_kernel_copy_map->wait_for_space = TRUE;

	/*
	 * As an optimization, 'small' out of line data regions using a 
	 * physical copy strategy are copied into kalloc'ed buffers.
	 * The value of 'small' is determined here.  Requests kalloc()
	 * with sizes greater or equal to kalloc_max_prerounded may fail.
	 */
	if (kalloc_max_prerounded <=  MSG_OOL_SIZE_SMALL) {
		msg_ool_size_small = kalloc_max_prerounded;
	}
	else {
	}
#endif

}

/* before SI_SUB_INTRINSIC and after SI_SUB_EVENTHANDLER */
SYSINIT(ipc_space, SI_SUB_KLD, SI_ORDER_ANY, ipc_bootstrap_sysinit, NULL);
