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

#include <device/buf.h>

extern void		harderr(
				struct buf	* bp,
				char		* cp);

extern char             * itoa(
                                int		num,
                                char		* str);

/* Structure for driver shutdown routine callback */
struct drvr_shut {
	struct drvr_shut	*next;
	void			(*callback)(caddr_t);	/* callback routine */
	caddr_t			param;
};
	
#define	DRVR_REGISTER	0x00000001	/* register a callback routine */
#define	DRVR_UNREGISTER	0x00000002	/* de-register a callback routine */

void
drvr_register_shutdown(void (*callback)(caddr_t), caddr_t param, int flags);
void drvr_shutdown(void);
