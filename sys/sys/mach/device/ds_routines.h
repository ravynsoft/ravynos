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
 * Revision 2.5  91/08/28  11:11:22  jsb
 * 	Page list support: device_write_dealloc returns a boolean.
 * 	[91/08/05  17:32:28  dlb]
 * 
 * Revision 2.4  91/05/14  15:47:56  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/05  17:09:33  mrt
 * 	Changed to new Mach copyright
 * 	[91/01/31  17:29:15  mrt]
 * 
 * Revision 2.2  89/09/08  11:24:24  dbg
 * 	Created.
 * 	[89/08/04            dbg]
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
 *	Author: David B. Golub, Carnegie Mellon University
 *	Date: 	8/89
 *
 *	Device service utility routines.
 */

#ifndef	DS_ROUTINES_H
#define	DS_ROUTINES_H

#include <norma_device.h>
#include <vm/vm_map.h>
#include <mach/machine/vm_types.h>
#include <mach/vm_sync.h>
#include <kern/host.h>
#include <device/device_types.h>

/*
 * Map for device IO memory.
 */
vm_map_t	device_io_map;

#define iodone(ior)	io_completed(ior, FALSE)

extern void		io_completed(
				io_req_t	ior,
				boolean_t	can_block);
extern void		io_done_thread(void);
extern void		iowait(
				io_req_t	ior);
extern kern_return_t	device_read_alloc_sg(
				io_req_t	ior,
				vm_size_t	size);
extern kern_return_t	device_read_alloc(
				io_req_t	ior,
				vm_size_t	size);
extern kern_return_t	device_write_get(
				io_req_t	ior,
				boolean_t	* wait);
extern boolean_t	device_write_dealloc(
				io_req_t	ior);
extern boolean_t	dev_name_lookup(
				char		* name,
				dev_ops_t	* ops,
				int		* unit);
extern void		dev_set_indirection(
				char		* name,
				dev_ops_t	ops,
				int		unit);
extern boolean_t	dev_change_indirect(
				char		* iname,
				char		* dname,
				int		unit);
extern boolean_t	 dev_find_indirect(
				dev_ops_t 	devops,
				int		unit,
				char		*realname);

extern kern_return_t	device_pager_setup(
				mach_device_t	device,
				vm_prot_t	prot,
				vm_offset_t	offset,
				vm_size_t	size,
				ipc_port_t	* port);
extern void		device_pager_init(void);
extern kern_return_t	device_pager_data_request(
			        ipc_port_t	pager,
			        ipc_port_t      pager_request,
			        vm_offset_t     offset,
			        vm_size_t       length,
			        vm_prot_t       protection_required);
extern kern_return_t	device_pager_synchronize(
				ipc_port_t	memory_object,
				ipc_port_t	memory_control,
				vm_offset_t	offset,
				vm_offset_t	length,
				vm_sync_t	sync_flags);
extern kern_return_t	device_pager_data_return(
				ipc_port_t		pager,
				ipc_port_t		pager_req,
				vm_offset_t		offset,
				pointer_t		addr,
				mach_msg_type_number_t	data_cnt,
				boolean_t		dirty,
				boolean_t		kernel_copy);
extern kern_return_t	device_pager_change_completed(
				ipc_port_t	pager,
				ipc_port_t	pager_req,
				memory_object_flavor_t	flavor);
extern kern_return_t	device_pager_init_pager(
				ipc_port_t	pager,
				ipc_port_t	pager_req,
				vm_size_t	pager_page_size);
extern kern_return_t	device_pager_terminate(
				ipc_port_t	pager,
				ipc_port_t	pager_req);
extern kern_return_t	device_pager_data_unlock(
				ipc_port_t	memory_obj,
				ipc_port_t	control_port,
				vm_offset_t	offset,
				vm_size_t	length,
				vm_prot_t	access);
extern kern_return_t	device_pager_lock_completed(
				ipc_port_t	memory_obj,
				ipc_port_t	pager_req,
				vm_offset_t	offset,
				vm_size_t	length);
extern kern_return_t	device_pager_supply_completed(
				ipc_port_t	pager,
				ipc_port_t	pager_request,
				vm_offset_t	offset,
				vm_size_t	length,
				kern_return_t	result,
				vm_offset_t	error_offset);
extern void		device_service_create(void);
extern kern_return_t	device_write_get_sg(
				io_req_t	ior,
				boolean_t	* wait);
extern kern_return_t	device_write_get(
				io_req_t	ior,
				boolean_t	* wait);
extern boolean_t	device_write_dealloc(
				io_req_t	ior);
extern void		ds_init(void);
extern boolean_t	ds_notify(
				mach_msg_header_t
						* msg);
extern boolean_t	ds_master_notify(
				mach_msg_header_t
						* msg);
extern boolean_t	ds_open_done(
				io_req_t	ior);
extern boolean_t	ds_read_done(
				io_req_t	ior);
extern boolean_t	ds_write_done(
				io_req_t	ior);
extern io_return_t	ds_io_done_queue_wait(
				io_done_queue_t		queue,
				io_done_result_t	*result,
				io_done_result_t	*ures);
extern void		io_done_queue_deallocate(
				io_done_queue_t		queue);
extern io_return_t	ds_device_write_common(
				device_t		device,
				ipc_port_t		reply_port,
				mach_msg_type_name_t	reply_port_type,
				dev_mode_t		mode,
				recnum_t		recnum,
				io_buf_ptr_t		data,
				mach_msg_type_number_t	count,
				int			op,
				io_buf_len_t		*written);
extern io_return_t	ds_device_read_common(
				device_t		device,
				ipc_port_t		reply_port,
				mach_msg_type_name_t	reply_port_type,
				dev_mode_t		mode,
				recnum_t		recnum,
				io_buf_len_t		bytes_wanted,
				int			op,
				io_buf_ptr_t		*data,
				mach_msg_type_number_t	*data_count);

#if	NORMA_DEVICE
extern char		*dev_forward_name(
				char		*name,
				char		*namebuf,
				int		namelen);
#endif	/* NORMA_DEVICE */

#if	MACH_KDB
extern void		db_show_ior(
				io_req_t	ior);
#endif	/* MACH_KDB */

#endif	/* DS_ROUTINES_H */
