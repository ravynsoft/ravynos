/*
 * Copyright (c) 2005 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
/*
 * Copyright (c) 2003 Apple Computer, Inc. All Rights Reserved.
 * 
 * The contents of this file constitute Original Code as defined in and are
 * subject to the Apple Public Source License Version 1.2 (the 'License').
 * You may not use this file except in compliance with the License. Please 
 * obtain a copy of the License at http://www.apple.com/publicsource and 
 * read it before using this file.
 * 
 * This Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER 
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES, 
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT. 
 * Please see the License for the specific language governing rights and 
 * limitations under the License.
 */

/*
 * spnegoBlob.h - GSS and "SPNEGO blob" formatting routines 
 * for SPNEGO tool
 *
 * Created July 7 2003 by dmitch 
 */
 
#ifndef	_SPNEGO_BLOB_H_
#define _SPNEGO_BLOB_H_

//#include "spnegoTool.h"
#include <Security/cssmtype.h>

#ifdef __cplusplus
extern "C" {
#endif


/*
 * Given a kerberos service tiocket in GSS form (i.e., an AP_REQ),
 * cook up a DER-encoded SPNEGO blob. Result is malloc'd; caller 
 * must free.
 */
int spnegoCreateInit(
	const unsigned char *gssBlob,
	unsigned gssBlobLen,
	unsigned char **spnegoBlob,		// mallocd and RETURNED
	unsigned *spnegoBlobLen);		// RETURNED

/*
 * High-level "give me the SPNEGO blob for this principal" routine. 
 *
 * The result is optionally base64 encoded data and the caller must free. 
 *
 * Returns nonzero on any error. 
 *
 * If principal is NULL, the actual kerberos ticket is not
 * calculated and not included in the blob. This can be used to 
 * query a server for what MechTypes it supports. 
 */
int spnegoTokenInitFromPrincipal(
	const char 		*inHostname,
	const char		*inServiceType,
	char **spnegoBlob,		// mallocd and RETURNED
	unsigned *spnegoBlobLen);	// RETURNED


#ifdef __cplusplus
}
#endif

#endif	/* _SPNEGO_BLOB_H_ */

