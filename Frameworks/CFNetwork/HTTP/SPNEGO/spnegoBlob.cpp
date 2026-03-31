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
 * spnegoBlob.cpp - GSS and "SPNEGO blob" formatting routines 
 * for SPNEGO tool
 *
 * Created July 7 2003 by dmitch 
 */
 
#include "spnegoBlob.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "spnegoKrb.h"
#include <security_cdsa_utils/cuEnc64.h>
#include <Security/SecAsn1Coder.h>
#include "spnegoDER.h"


#define PA_CHUNK_SIZE	1024


/* malloc a NULL-ed array of pointers of size num+1 */
static void **nssNullArray(
						   SecAsn1CoderRef coder,
						   uint32 num)
{
	unsigned len = (num + 1) * sizeof(void *);
	void **p = (void **)SecAsn1Malloc(coder, len);
	memset(p, 0, len);
	return p;
}


/*
 * Given a kerberos service tiocket in GSS form (i.e., an AP_REQ),
 * cook up a DER-encoded SPNEGO blob. Result is malloc'd; caller 
 * must free.
 */
int spnegoCreateInit(
					 const unsigned char *gssBlob,
					 unsigned gssBlobLen,
					 unsigned char **spnegoBlob,			// mallocd and RETURNED
					 unsigned *spnegoBlobLen)			// RETURNED
{
	SpnegoNegTokenInitGss negInit;
	SecAsn1CoderRef coder;
	
	if(SecAsn1CoderCreate(&coder)) {
		/* memory failure */
		return -1;
	}
	memset(&negInit, 0, sizeof(negInit));
	negInit.oid = CSSMOID_SPNEGO;
	negInit.token.mechTypeList = (CSSM_OID **)nssNullArray(coder, 2);
	negInit.token.mechTypeList[0] = (CSSM_OID *)&CSSMOID_KERB_V5_LEGACY;
	negInit.token.mechTypeList[1] = (CSSM_OID *)&CSSMOID_KERB_V5;
	/* no contextFlags for now, though we might need 'em */
	CSSM_DATA gssData;
	if(gssBlob) {
		gssData.Data = (uint8 *)gssBlob;
		gssData.Length = gssBlobLen;
		negInit.token.mechToken = &gssData;
	}
	
	CSSM_DATA res = {0, NULL};
	OSStatus ortn = SecAsn1EncodeItem(coder, &negInit, 
									  SpnegoNegTokenInitGssTemplate, &res);
	if(ortn) {
		SecAsn1CoderRelease(coder);
		return -1;
	}
	*spnegoBlob = (unsigned char *)malloc(res.Length);
	memmove(*spnegoBlob, res.Data, res.Length);
	*spnegoBlobLen = res.Length;
	
	/* this frees all memory allocated during SecAsn1EncodeItem() */
	SecAsn1CoderRelease(coder);
	return 0;
}

	
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
	char 			**spnegoBlob,		// mallocd and RETURNED
	unsigned 		*spnegoBlobLen)		// RETURNED
{
	unsigned char 	*rawBlob = NULL;
	unsigned 		rawBlobLen = 0;
	unsigned char 	*tkt = NULL;
	unsigned 		tktLen = 0;
	int				ourRtn = 0;
	
	/* 
	 * Get kerberos ticket for specified principal
	 */
	if(inHostname && inServiceType) {
		krb5_error_code kerr = GetSvcTicketForHost(inHostname, inServiceType, &tktLen, &tkt);
		if(kerr) {
			return -1;
		}
	}
	
	/* now an SPNEGO blob */
	ourRtn = spnegoCreateInit(tkt, tktLen, &rawBlob, &rawBlobLen);
	if(ourRtn) {
		if (tkt) free(tkt);
		return ourRtn;
	}

	/* caller wants binary DER encoded data */
	*spnegoBlob = (char *)rawBlob;
	*spnegoBlobLen = rawBlobLen;

	if(tkt) {
		free(tkt);
	}
	return ourRtn;
}

