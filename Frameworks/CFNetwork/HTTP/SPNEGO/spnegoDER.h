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
 * spnegoDER.h - DER encode/decode support for SPNEGO
 *
 * Created July 7 2003 by dmitch 
 */
 
#ifndef	_SPNEGO_DER_H_
#define _SPNEGO_DER_H_

#include <Security/secasn1t.h>
#include <Security/cssmtype.h>

#ifdef __cplusplus
extern "C" {
#endif

/***
Ê*** ASN and OID definitions from Microsoft:
 *** http://msdn.microsoft.com/library/
 ***    default.asp?url=/library/en-us/dnsecure/html/http-sso-2.asp
 ***/
 
/* SPNEGO-specific OIDS */

/* 1.3.6.1.1.5.5.2 SPNEGO */
extern const CSSM_OID CSSMOID_SPNEGO;

/* 1.2.840.48018.1.2.2 Kerberos V5 Legacy (same as Kerberos 
 * V5, but off by 1 bit required for legacy compatibility) */
extern const CSSM_OID CSSMOID_KERB_V5_LEGACY;

/* 1.2.840.113554.1.2.2 Kerberos V5 */
extern const CSSM_OID CSSMOID_KERB_V5;

/*
 * RFC 2478 defines this stuff this way:
 *
 * NegotiationToken ::= CHOICE {
 *     negTokenInit  [0]  NegTokenInit,
 *     negTokenTarg  [1]  NegTokenTarg }
 * 
 * MechTypeList ::= SEQUENCE OF MechType
 * 
 * NegTokenInit ::= SEQUENCE {
 *     mechTypes       [0] MechTypeList  OPTIONAL,
 *     reqFlags        [1] ContextFlags  OPTIONAL,
 *     mechToken       [2] OCTET STRING  OPTIONAL,
 *     mechListMIC     [3] OCTET STRING  OPTIONAL
 * }
 * 
 * ContextFlags ::= BIT STRING {
 *         delegFlag       (0),
 *         mutualFlag      (1),
 *         replayFlag      (2),
 *         sequenceFlag    (3),
 *         anonFlag        (4),
 *         confFlag        (5),
 *         integFlag       (6)
 * }
 *
 * Note well: Miscrosoft encodes NegTokenInit as a context-specific
 * explicit constructed sequence wrapped in a GSS header like so:
 *
 * NegTokenInitGss ::= APPLICATION SPECIFIC[0] {
 *		oid			OID,		// spnego
 * 		token		NegTokenInit EXPLICIT[0]
 * }
 *
 * Also NOTE WELL: contrary to both RFC 2478 and Microsoft's own 
 * documentation, all of the fields in both NegTokenInit and 
 * NegTokenTarget are EXPLICITLY tagged. This was determined
 * the hard way, via empirical observation of traffic to and 
 * from an IIS machine. 
 */
 
typedef struct {
	CSSM_OID	**mechTypeList;	// SEQUENCE OF, optional
	CSSM_DATA	*contextFlags;	// BIT STRING, optional
	CSSM_DATA	*mechToken;		// optional
	CSSM_DATA	*mechListMIC;	// optional
} SpnegoNegTokenInit;

typedef struct {
	CSSM_OID			oid;	// CSSMOID_SPNEGO
	SpnegoNegTokenInit	token;
} SpnegoNegTokenInitGss;

extern const SecAsn1Template SpnegoNegTokenInitTemplate[];
extern const SecAsn1Template SpnegoNegTokenInitGssTemplate[];

/*
 * Here's what Microsoft has to say about NegTokenTarg.
 *
 * NegTokenTarg      ::=  SEQUENCE {
 *   negResult      [0]  ENUMERATED {
 *                            accept_completed (0),
 *                            accept_incomplete (1),
 *                            rejected (2) }  OPTIONAL,
 *   supportedMech  [1]  MechType             OPTIONAL,
 *   responseToken  [2]  OCTET STRING         OPTIONAL,
 *   mechListMIC    [3]  OCTET STRING         OPTIONAL
 * }
 *
 * However empirical observation indicates that this sequence
 * is wrapped in an EXPLICIT CONTEXT_SPECIFIC[1] wrapper. 
 * Also each field in the NegTokenTarg is most definitely
 * explicitly tagged. 
 */
typedef struct {
	CSSM_DATA		*negResult;		// SpegoNegResult, optional
	CSSM_OID		*mechType;		// optional
	CSSM_DATA		*responseToken;	// optional
	CSSM_DATA		*mechListMIC;	// optional
} SpnegoNegTokenTarg;

extern const SecAsn1Template SpnegoNegTokenTargTemplate[];

#ifdef __cplusplus
}
#endif

#endif	/* _SPNEGO_DER_H_ */

