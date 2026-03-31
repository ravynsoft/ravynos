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
 * spnegoDER.cpp - DER encode/decode support for SPNEGO
 *
 * Created July 7 2003 by dmitch 
 *
 * This file contains templates used in conjunction with 
 * libnssasn1.a to DER encode and decode data structs associated
 * with the Microsoft implementation of SPNEGO. The DER produced
 * and consumed by these templates is known to work with Microsoft
 * IIS v. 5.0. This encoding and decoding does NOT match the ASN.1
 * definitions provided by Microsoft and it does not match the 
 * ASN.1 definitions in RFC 2478, upon which SPNEGO is claimed to
 * be based. The DER encoding and decoding performed here was 
 * developed the hard way, by observing traffic with IIS 5.0. 
 */
 
#include "spnegoDER.h"
#include <Security/cssmtype.h>
#include <Security/asn1Templates.h>
#include <stddef.h>
#include "spnegoBlob.h"

/***
 *** SPNEGO-specific OIDS 
 ***/
 
/* 1.3.6.1.1.5.5.2 SPNEGO */
static const uint8 OID_SPNEGO[] = 
	{ 0x2b, 0x06, 0x01, 0x05, 0x05, 0x02 };
const CSSM_OID CSSMOID_SPNEGO =
	{ sizeof(OID_SPNEGO), (uint8 *)OID_SPNEGO };
	
/* 1.2.840.48018.1.2.2 Kerberos V5 Legacy (same as Kerberos 
 * V5, but off by 1 bit required for legacy compatibility) */
static const uint8 OID_KERB_V5_LEGACY[] = 
	{ 0x2a, 0x86, 0x48, 0x82, 0xf7, 0x12, 0x01, 0x02, 0x02 };
const CSSM_OID CSSMOID_KERB_V5_LEGACY = 
	{ sizeof(OID_KERB_V5_LEGACY), (uint8 *)OID_KERB_V5_LEGACY };

/* 1.2.840.113554.1.2.2 Kerberos V5 */
static const uint8 OID_KERB_V5[] =  
	{ 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x12, 0x01, 0x02, 0x02 };
const CSSM_OID CSSMOID_KERB_V5 = 
	{ sizeof(OID_KERB_V5), (uint8 *)OID_KERB_V5 };

/* SpnegoNegTokenInit */
const SecAsn1Template SpnegoNegTokenInitTemplate[] = {
    { SEC_ASN1_SEQUENCE,
	  0, NULL, sizeof(SpnegoNegTokenInit) },
	  
	{ SEC_ASN1_OPTIONAL | SEC_ASN1_CONTEXT_SPECIFIC | 0 |
	  SEC_ASN1_EXPLICIT | SEC_ASN1_CONSTRUCTED,
	  offsetof(SpnegoNegTokenInit,mechTypeList), 
	  kSecAsn1SequenceOfObjectIDTemplate },
	  
	{ SEC_ASN1_OPTIONAL | SEC_ASN1_CONTEXT_SPECIFIC | 1 |
	  SEC_ASN1_EXPLICIT,
	  offsetof(SpnegoNegTokenInit,contextFlags), 
	  kSecAsn1PointerToBitStringTemplate },
	  
	{ SEC_ASN1_OPTIONAL | SEC_ASN1_CONTEXT_SPECIFIC | 2 |
	  SEC_ASN1_EXPLICIT | SEC_ASN1_CONSTRUCTED,
	  offsetof(SpnegoNegTokenInit,mechToken),
	  kSecAsn1PointerToOctetStringTemplate },
	  
	{ SEC_ASN1_OPTIONAL | SEC_ASN1_CONTEXT_SPECIFIC | 3 |
	  SEC_ASN1_EXPLICIT | SEC_ASN1_CONSTRUCTED,
	  offsetof(SpnegoNegTokenInit,mechListMIC),
	  kSecAsn1PointerToOctetStringTemplate},
	{ 0 }
};

/* SpnegoNegTokenInitGss */
const SecAsn1Template _SpnegoNegTokenInitGssTemplate[] = {
	{ SEC_ASN1_SEQUENCE,
	  0, NULL, sizeof(SpnegoNegTokenInitGss) },
	{ SEC_ASN1_OBJECT_ID,  
	  offsetof(SpnegoNegTokenInitGss,oid) },
	{ SEC_ASN1_CONTEXT_SPECIFIC | 0 | SEC_ASN1_CONSTRUCTED |
	  SEC_ASN1_EXPLICIT,
	  offsetof(SpnegoNegTokenInitGss,token),
	  SpnegoNegTokenInitTemplate },
	{ 0 }
};

/*
 * This does the App-specific wrapper around the actual defined
 * SpnegoNegTokenInitGss. 
 */
const SecAsn1Template SpnegoNegTokenInitGssTemplate[] = {
	{ SEC_ASN1_APPLICATION | 0 | SEC_ASN1_CONSTRUCTED,
	  0, _SpnegoNegTokenInitGssTemplate, sizeof(SpnegoNegTokenInitGss) },
	{ 0 }
};

const SecAsn1Template _SpnegoNegTokenTargTemplate[] = {
    { SEC_ASN1_SEQUENCE,
	  0, NULL, sizeof(SpnegoNegTokenTarg) },
	  
	/* Microsoft IIS passes all these back as constructed, I swear */
	{ SEC_ASN1_OPTIONAL | SEC_ASN1_CONTEXT_SPECIFIC | 0 |
	  SEC_ASN1_EXPLICIT | SEC_ASN1_CONSTRUCTED,
	  offsetof(SpnegoNegTokenTarg,negResult), 
	  kSecAsn1PointerToEnumeratedTemplate },

	{ SEC_ASN1_OPTIONAL | SEC_ASN1_CONTEXT_SPECIFIC | 1 |
	  SEC_ASN1_EXPLICIT | SEC_ASN1_CONSTRUCTED,
	  offsetof(SpnegoNegTokenTarg,mechType), 
	  kSecAsn1PointerToObjectIDTemplate },

	{ SEC_ASN1_OPTIONAL | SEC_ASN1_CONTEXT_SPECIFIC | 2 |
	  SEC_ASN1_EXPLICIT | SEC_ASN1_CONSTRUCTED,
	  offsetof(SpnegoNegTokenTarg,responseToken), 
	  kSecAsn1PointerToOctetStringTemplate },

	{ SEC_ASN1_OPTIONAL | SEC_ASN1_CONTEXT_SPECIFIC | 3 |
	  SEC_ASN1_EXPLICIT | SEC_ASN1_CONSTRUCTED,
	  offsetof(SpnegoNegTokenTarg,mechListMIC), 
	  kSecAsn1PointerToOctetStringTemplate },
	{ 0 }
};

/*
 * This does the context-specific wrapper around the actual defined
 * SpnegoNegTokenTarg. 
 */
const SecAsn1Template SpnegoNegTokenTargTemplate[] = {
	{ SEC_ASN1_CONTEXT_SPECIFIC | 1 | SEC_ASN1_CONSTRUCTED | 
	  SEC_ASN1_EXPLICIT,
	  0, _SpnegoNegTokenTargTemplate, sizeof(SpnegoNegTokenTarg) },
	{ 0 }
};
