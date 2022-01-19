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
 * MkLinux
 */
/* 
 *  Includes all the types that a normal user
 *  of Mach programs should need
 */

#ifndef	_MACH_H_
#define	_MACH_H_

#include <mach/vm_types.h>
#include <mach/mach_types.h>
#include <mach/mach_interface.h>
#include <mach/mach_port.h>
#include <mach/mach_init.h>
#include <mach/mach_error.h>
#include <mach/task.h>

/*
 * Standard prototypes
 */
extern void			panic_init(mach_port_t);
extern void			panic(const char *, ...);

extern void			safe_gets(char *,
					  char *,
					  int);

extern void			slot_name(cpu_type_t,
					  cpu_subtype_t,
					  char **,
					  char **);

extern void			mig_reply_setup(mach_msg_header_t *,
						mach_msg_header_t *);

extern void			mach_msg_destroy(mach_msg_header_t *);

extern mach_msg_return_t	mach_msg_receive(mach_msg_header_t *);

extern mach_msg_return_t	mach_msg_send(mach_msg_header_t *);

extern mach_msg_return_t	mach_msg_server_once(boolean_t (*)
						     (mach_msg_header_t *,
						      mach_msg_header_t *),
						     mach_msg_size_t,
						     mach_port_t,
						     mach_msg_options_t);
extern mach_msg_return_t	mach_msg_server(boolean_t (*)
						(mach_msg_header_t *,
						 mach_msg_header_t *),
						mach_msg_size_t,
						mach_port_t,
						mach_msg_options_t);

extern kern_return_t		device_read_overwrite_request(mach_port_t,
							      mach_port_t,
							      dev_mode_t,
							      recnum_t,
							      io_buf_len_t,
							      vm_address_t);

extern kern_return_t		device_read_overwrite(mach_port_t,
						      dev_mode_t,
						      recnum_t,
						      io_buf_len_t,
						      vm_address_t,
						      mach_msg_type_number_t *);

/*
 * Prototypes for compatibility
 */
extern kern_return_t	clock_get_res(mach_port_t,
				      clock_res_t *);
extern kern_return_t	clock_set_res(mach_port_t,
				      clock_res_t);

extern kern_return_t	clock_sleep(mach_port_name_t,
									mach_sleep_type_t,
									int,
									int,
				    tvalspec_t *);
#endif	/* _MACH_H_ */
