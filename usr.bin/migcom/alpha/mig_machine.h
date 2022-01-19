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
 * cmk1.1
 */

#define	machine_alignment(SZ,ESZ) 					\
 	(((((ESZ) > 4) && ((SZ) & 7)) ?  				\
	(SZ) = ((SZ) + 7) & ~7 : (((ESZ == 4) && ((SZ) & 3)) ?		\
	(SZ) = ((SZ) + 3) & ~3 : 0)), (SZ) += (ESZ))

#define	machine_padding(BYTES)						\
	((bytes & 3) ? (4 - (bytes &  3)) : 0)

#define PACK_MESSAGES FALSE
