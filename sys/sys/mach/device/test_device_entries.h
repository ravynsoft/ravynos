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

extern void		testdev_generate_replies(void);
extern void		testdev_generate_packets(void);

extern io_return_t	testdev_open(
				dev_t		dev,
				dev_mode_t	flag,
				io_req_t	ior);
extern void		testdev_close(
				dev_t		dev);
extern io_return_t	testdev_read(
				dev_t		dev,
				io_req_t	ior);
extern io_return_t	testdev_write(
				dev_t		dev,
				io_req_t	ior);
extern io_return_t	testdev_getstat(
				dev_t		dev,
				dev_flavor_t	flavor,
				dev_status_t	data,
				mach_msg_type_number_t * count);
extern io_return_t	testdev_setstat(
				dev_t		dev,
				dev_flavor_t	flavor,
				dev_status_t	data,
				mach_msg_type_number_t	count);
extern vm_offset_t	testdev_mmap(
				dev_t		dev,
				vm_offset_t	off,
				vm_prot_t	prot);
extern io_return_t	testdev_async_in(
				dev_t		dev,
				ipc_port_t	rcv_port,
				int		pri,
				filter_t	*filter,
				mach_msg_type_number_t fcount,
				mach_device_t	device);
extern void		testdev_reset(
				dev_t		dev);
extern boolean_t	testdev_port_death(
				dev_t		dev,
				ipc_port_t	port);
extern io_return_t	testdev_dev_info(
				dev_t		dev,
				dev_flavor_t	flavor,
				char		* info);
