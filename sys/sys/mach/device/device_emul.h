/*
 * Mach device emulation definitions (i386at version).
 *
 * Copyright (c) 1996 The University of Utah and
 * the Computer Systems Laboratory at the University of Utah (CSL).
 * All rights reserved.
 *
 * Permission to use, copy, modify and distribute this software is hereby
 * granted provided that (1) source code retains these copyright, permission,
 * and disclaimer notices, and (2) redistributions including binaries
 * reproduce the notices in supporting documentation, and (3) all advertising
 * materials mentioning features or use of this software display the following
 * acknowledgement: ``This product includes software developed by the
 * Computer Systems Laboratory at the University of Utah.''
 *
 * THE UNIVERSITY OF UTAH AND CSL ALLOW FREE USE OF THIS SOFTWARE IN ITS "AS
 * IS" CONDITION.  THE UNIVERSITY OF UTAH AND CSL DISCLAIM ANY LIABILITY OF
 * ANY KIND FOR ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 *
 * CSL requests users of this software to return to csl-dist@cs.utah.edu any
 * improvements that they make and grant CSL redistribution rights.
 *
 *      Author: Shantanu Goel, University of Utah CSL
 */

#ifndef _DEVICE_DEVICE_EMUL_H_
#define _DEVICE_DEVICE_EMUL_H_

#include <kern/ledger.h>
#include <mach/notify.h>
#include <device/net_status.h>

/* Each emulation layer provides these operations.  */
struct device_emulation_ops
{
  void (*reference) (void *);
  void (*dealloc) (void *);
  ipc_port_t (*dev_to_port) (void *);
  io_return_t (*open) (ipc_port_t, mach_msg_type_name_t, ledger_t,
		       dev_mode_t, security_token_t, char *, device_t *);
  io_return_t (*close) (void *);
  io_return_t (*write) (void *, ipc_port_t, mach_msg_type_name_t,
			dev_mode_t, recnum_t, io_buf_ptr_t, mach_msg_type_number_t, io_buf_len_t *);
  io_return_t (*write_inband) (void *, ipc_port_t, mach_msg_type_name_t,
			       dev_mode_t, recnum_t, io_buf_ptr_inband_t,
			       mach_msg_type_number_t, io_buf_len_t *);
  io_return_t (*write_common) (void *, ipc_port_t, mach_msg_type_name_t,
			       dev_mode_t, recnum_t, io_buf_ptr_t,
			       mach_msg_type_number_t, int, io_buf_len_t *);
  io_return_t (*read) (void *, ipc_port_t, mach_msg_type_name_t,
		       dev_mode_t, recnum_t, io_buf_len_t, io_buf_ptr_t *,
		       mach_msg_type_number_t *);
  io_return_t (*read_inband) (void *, ipc_port_t, mach_msg_type_name_t,
			      dev_mode_t, recnum_t, io_buf_len_t,
			      io_buf_ptr_inband_t, mach_msg_type_number_t *);
  io_return_t (*read_common) (void *, ipc_port_t, mach_msg_type_name_t,
			      dev_mode_t, recnum_t, io_buf_len_t, int,
			      io_buf_ptr_t *, mach_msg_type_number_t *);
  io_return_t (*set_status) (void *, dev_flavor_t, dev_status_t,
			     mach_msg_type_number_t);
  io_return_t (*get_status) (void *, dev_flavor_t, dev_status_t,
			     mach_msg_type_number_t *);
  io_return_t (*set_filter) (void *, ipc_port_t, int, filter_t [],
			     mach_msg_type_number_t);
  io_return_t (*map) (void *, vm_prot_t, vm_offset_t,
		      vm_size_t, ipc_port_t *, boolean_t);
  void (*no_senders) (mach_no_senders_notification_t *);
};

#endif /* _DEVICE_DEVICE_EMUL_H_ */
