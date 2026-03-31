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
 * Copyright (c) 2000-2004 Apple Computer, Inc. All Rights Reserved.
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

#ifndef _NTLM_GENERATOR_H_
#define _NTLM_GENERATOR_H_

#include <CoreFoundation/CFData.h>
#include <CoreFoundation/CFString.h>
#include <Security/cssmtype.h>

#ifdef  __cplusplus
extern "C" {
#endif

/*
 * This interface provides the capability to generate and parse the authentication
 * blobs which pass back and forth between a client and a server during NTLM
 * authentication. Only the client side is implemented. 
 *
 * All three variants of NTLM authentication are performed: NTLM1, NTLM2, and 
 * NTLMv2. 
 *
 * In general, to use this stuff for HTTP authentication:
 *
 * 1. Determine that NTLM authentication is possible. Drop the connection
 *    to the server if you have a persistent connection open; MS servers
 *    require a clean unused connection for this negotiation to occur. 
 *
 * 2. Create a NtlmGeneratorRef object, specifying possible restrictions
 *    on negotiation version. 
 *
 * 3. Create the client authentication blob using NtlmCreateClientRequest()
 *    and send it to the server, base64 encoded, in a "Authorization: NTLM" 
 *    header line. 
 *
 * 4. The server should send back another 401 status, with its own blob in
 *    a "WWW-Authenticate: NTLM" header. 
 *
 * 5. Base64 decode that blob and feed it into NtlmCreateClientResponse(), the 
 *    output of which is another blob which you send to the server again in 
 *    a "WWW-Authenticate: NTLM" header. 
 *
 * 6. If you're lucky the server will give a 200 status (or something else useful
 *    other than 401) and you're done. 
 *
 * 7. Free the NtlmGeneratorRef object with NtlmGeneratorRelease().
 */
 
/*
 * Opaque reference to an NTLM blob generator object.
 */
typedef struct NtlmGenerator *NtlmGeneratorRef;

/*
 * Which versions of the protocol are acceptable?
 */
enum {
	NW_NTLM1   = 0x00000001,
	NW_NTLM2   = 0x00000002,
	NW_NTLMv2  = 0x00000004,

	// all variants enabled, preferring NTLMv2, then NTLM2, then NTLM1
	NW_Any     = NW_NTLM1 | NW_NTLM2 | NW_NTLMv2
};
typedef uint32_t NLTM_Which;


/* Create/release NtlmGenerator objects.*/
OSStatus NtlmGeneratorCreate(
	NLTM_Which			which,
	NtlmGeneratorRef	*ntlmGen);			/* RETURNED */
	
void NtlmGeneratorRelease(
	NtlmGeneratorRef	ntlmGen);
	
/* create the initial client request */
OSStatus NtlmCreateClientRequest(
	NtlmGeneratorRef	ntlmGen,
	CFDataRef			*clientRequest);	/* RETURNED */
		
/* parse server challenge and respond to it */
OSStatus NtlmCreateClientResponse(
	NtlmGeneratorRef	ntlmGen,
	CFDataRef			serverBlob,			/* obtained from the server */
	CFStringRef			domain,				/* server domain, appears to be optional */
	CFStringRef			userName,
	CFStringRef			password,
	CFDataRef			*clientResponse);   /* RETURNED */
		
/* which version did we negotiate? Returns true for NTLM2, false for traditional NTLM */
bool NtlmNegotiatedNtlm2(
	NtlmGeneratorRef	ntlmGen);

/* which version did we negotiate? */
NLTM_Which NtlmGetNegotiatedVersion(
	NtlmGeneratorRef	ntlmGen);

OSStatus NtlmGeneratePasswordHashes(
	CFAllocatorRef alloc,
	CFStringRef password,
	CFDataRef* ntlmHash,
	CFDataRef* lmHash);

OSStatus _NtlmCreateClientResponse(
	NtlmGeneratorRef	ntlmGen,
	CFDataRef			serverBlob,
	CFStringRef			domain,				/* optional */
	CFStringRef			userName,
	CFDataRef			ntlmHash,
	CFDataRef			lmHash,
	CFDataRef			*clientResponse);	/* RETURNED */
																			
#ifdef  __cplusplus
}
#endif

#endif  /* _NTLM_GENERATOR_H_ */
