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
 *  libresolv.c
 *  CFNetwork
 *
 *  Created by Jeremy Wyld on 9/29/04.
 *  Copyright 2004 Apple Computer, Inc. All rights reserved.
 *
 */

#include <security_cdsa_utils/cuEnc64.h>
#include <security_cdsa_utils/cuCdsaUtils.h>
#include <CFNetwork/CFNetworkInternal.h>
#include <sys/errno.h>
#include <mach-o/dyld.h>

//#ifndef DYNAMICALLY_LOAD_CDSA_UTILS
//#define DYNAMICALLY_LOAD_CDSA_UTILS 1
//#endif

#if DYNAMICALLY_LOAD_CDSA_UTILS

static const void* cdsa_utils = NULL;
static const char* const kCDSAUtilsPath = "/usr/local/SecurityPieces/Frameworks/security_cdsa_utils.framework/Versions/A/security_cdsa_utils";

static CSSM_HANDLE returns_CSSM_HANDLE(void) { return CSSM_INVALID_HANDLE; }
static CSSM_RETURN returns_CSSM_RETURN(void) { return -1; }
static unsigned char* returns_uchar(void) { return NULL; }

#define GET_DYNAMIC_SYMBOL(sym, rettype, arglist, alt)	\
static rettype (* sym##_proc)arglist = NULL;	\
if (sym##_proc == NULL) {	\
	if (cdsa_utils || (cdsa_utils = __CFNetworkLoadFramework(kCDSAUtilsPath)))	\
		sym##_proc = (rettype(*)arglist)NSAddressOfSymbol(NSLookupSymbolInImage(cdsa_utils, "_"#sym, NSLOOKUPSYMBOLINIMAGE_OPTION_BIND));	\
			if (! sym##_proc) sym##_proc = (rettype(*)arglist)alt;	\
}

CSSM_CSP_HANDLE
cuCspStartup(CSSM_BOOL bareCsp) {
	
    GET_DYNAMIC_SYMBOL(cuCspStartup, CSSM_CSP_HANDLE, (CSSM_BOOL), returns_CSSM_HANDLE);
    
    return (*cuCspStartup_proc)(bareCsp);
}

CSSM_RETURN
cuCspDetachUnload(CSSM_CSP_HANDLE cspHand, CSSM_BOOL bareCsp) {
	
    GET_DYNAMIC_SYMBOL(cuCspDetachUnload, CSSM_RETURN, (CSSM_CSP_HANDLE, CSSM_BOOL), returns_CSSM_RETURN);
    
    return (*cuCspDetachUnload_proc)(cspHand, bareCsp);
}

unsigned char*
cuEnc64(const unsigned char *inbuf, unsigned inlen, unsigned *outlen) {
	
    GET_DYNAMIC_SYMBOL(cuEnc64, unsigned char*, (const unsigned char*, unsigned, unsigned*), returns_uchar);
    
    return (*cuEnc64_proc)(inbuf, inlen, outlen);
}

unsigned char*
cuDec64(const unsigned char *inbuf, unsigned inlen, unsigned *outlen) {
	
    GET_DYNAMIC_SYMBOL(cuDec64, unsigned char*, (const unsigned char*, unsigned, unsigned*), returns_uchar);
    
    return (*cuDec64_proc)(inbuf, inlen, outlen);
}

#undef GET_DYNAMIC_SYMBOL
#endif	/* DYNAMICALLY_LOAD_CDSA_UTILS */
