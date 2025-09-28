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
#include <mach/error.h>

#ifndef _CDLI_ERROR_H_
#define _CDLI_ERROR_H_

#define CDLI_ERR_BITS (err_kern|err_sub(2))

#define CDLI_ERR_DEMUX_EXISTS		(CDLI_ERR_BITS | 1)
#define CDLI_ERR_DEMUX_DOESNT_EXIST	(CDLI_ERR_BITS | 2)
#define CDLI_ERR_DEMUX_IN_USE		(CDLI_ERR_BITS | 3)
#define CDLI_ERR_NS_EXISTS		(CDLI_ERR_BITS | 4)
#define CDLI_ERR_NS_DOESNT_EXIST	(CDLI_ERR_BITS | 5)
#define CDLI_ERR_NDD_DOESNT_EXIST	(CDLI_ERR_BITS | 6)

#define CDLI_ERR_BAD_ETH_CONFIG_CMD	(CDLI_ERR_BITS | 7)
#define CDLI_ERR_ISR_MISSING		(CDLI_ERR_BITS | 8)
#define CDLI_ERR_PROTOQ_MISSING		(CDLI_ERR_BITS | 9)
#define CDLI_ERR_BAD_LEN		(CDLI_ERR_BITS | 10)
#define CDLI_ERR_BAD_FILTERTYPE		(CDLI_ERR_BITS | 11)
#define CDLI_ERR_NOMEM			(CDLI_ERR_BITS | 12)
#define CDLI_ERR_FILTER_EXISTS		(CDLI_ERR_BITS | 13)
#define CDLI_ERR_FILTER_DOESNT_EXIST	(CDLI_ERR_BITS | 14)
#define CDLI_ERR_ISR_EXISTS		(CDLI_ERR_BITS | 15)
#define CDLI_ERR_ISR_DOESNT_EXIST	(CDLI_ERR_BITS | 16)
#define CDLI_ERR_PROTO_EXISTS		(CDLI_ERR_BITS | 17)
#define CDLI_ERR_AF_OUT_OF_RANGE	(CDLI_ERR_BITS | 17)

#endif
