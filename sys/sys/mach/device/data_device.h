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

#include <types.h>
#include <kern/macro_help.h>
#include <kern/queue.h>
#include <device/dev_hdr.h>
#include <device/device_types.h>
#include <device/io_req.h>

#ifndef _DEVICE_DATA_DEVICE_H_
#define	_DEVICE_DATA_DEVICE_H_

/*
 * Request sent to the bootstrap task
 */
#define	DATADEV_NAME	"data_device"		      /* Device generic name */

typedef struct datadev {
    queue_chain_t	  dd_chain;		      /* data request chain */
    const char		 *dd_name;		      /* name */
    unsigned		  dd_size;		      /* name size */
    dev_t		  dd_dev;		      /* data device */
    void		(*dd_close)(void *);	      /* close function */
    void		(*dd_write)(void *, io_req_t);/* write function */
    void		 *dd_arg;		      /* functions argument */
} *datadev_t;

#define	DATADEV_INIT(_ddev, _name, _close, _write, _arg)	\
MACRO_BEGIN							\
    const char *_p = (_ddev)->dd_name  = (const char *)(_name);	\
    (_ddev)->dd_size = 1;					\
    while (*_p++ != '\0')					\
	(_ddev)->dd_size++;					\
    (_ddev)->dd_close = (void (*)(void *))(_close);		\
    (_ddev)->dd_write = (void (*)(void *, io_req_t))(_write);	\
    (_ddev)->dd_arg   = (_arg);					\
MACRO_END

extern void		datadev_init(void);

extern void		datadev_request(datadev_t);

extern io_return_t	datadev_open(dev_t,
				     dev_mode_t,
				     io_req_t);

extern void		datadev_close(dev_t);

extern io_return_t	datadev_read(dev_t,
				     io_req_t);

extern io_return_t	datadev_write(dev_t,
				      io_req_t);

extern io_return_t	datadev_dinfo(dev_t,
				      dev_flavor_t,
				      char *);

#endif /* _DEVICE_DATA_DEVICE_H_ */
