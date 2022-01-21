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

#ifndef _DEVICE_TEST_DEVICE_STATUS_H_
#define _DEVICE_TEST_DEVICE_STATUS_H_

#include <device/device_types.h>

struct test_device_status {
	mach_port_t	reply_port;
	int		reply_type;
#define	DEVICE_TEST_READ_REPLY			1
#define DEVICE_TEST_READ_INBAND_REPLY		2
#define DEVICE_TEST_READ_OVERWRITE_REPLY	3
#define DEVICE_TEST_WRITE_REPLY			4
#define DEVICE_TEST_WRITE_INBAND_REPLY		5
#define DEVICE_TEST_FILTERED_PACKET		6
	mach_port_t	reply_thread;
	vm_address_t	reply_uaddr;
	int		reply_count;
	int		reply_size;
	boolean_t	reply_synchronous;
};
typedef struct test_device_status	test_device_status_t;
#define TEST_DEVICE_STATUS_COUNT (sizeof (test_device_status_t) / sizeof (int))

#define TEST_DEVICE_STATUS			1
#define TEST_DEVICE_GENERATE_REPLIES		2

#endif	/* _DEVICE_TEST_DEVICE_STATUS_H_ */
