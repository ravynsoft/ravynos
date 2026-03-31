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

#include <netdb_async.h>
#include "CFNetworkInternal.h"
#include <sys/errno.h>
#include <mach-o/dyld.h>

#ifndef DYNAMICALLY_LOAD_LIBRESOLV
#define DYNAMICALLY_LOAD_LIBRESOLV 1
#endif

#if DYNAMICALLY_LOAD_LIBRESOLV

static const void* libresolv = NULL;

static int32_t returns_int32(void) { return EFAULT; }

#define GET_DYNAMIC_SYMBOL(sym, rettype, arglist, alt)	\
static rettype (* sym##_proc)arglist = NULL;	\
if (sym##_proc == NULL) {	\
	if (libresolv || (libresolv = __CFNetworkLoadFramework("/usr/lib/libresolv.9.dylib")))	\
		sym##_proc = (rettype(*)arglist)NSAddressOfSymbol(NSLookupSymbolInImage(libresolv, "_"#sym, NSLOOKUPSYMBOLINIMAGE_OPTION_BIND));	\
			if (! sym##_proc) sym##_proc = (rettype(*)arglist)alt;	\
}

int32_t
dns_async_start(mach_port_t *p, const char *name, uint16_t dnsclass, uint16_t dnstype, uint32_t do_search, dns_async_callback callback, void *context) {
	
    GET_DYNAMIC_SYMBOL(dns_async_start, int32_t, (mach_port_t*, const char*, uint16_t, uint16_t, uint32_t, dns_async_callback, void*), returns_int32);
    
    return (*dns_async_start_proc)(p, name, dnsclass, dnstype, do_search, callback, context);
}

int32_t
dns_async_handle_reply(void *msg) {
	
    GET_DYNAMIC_SYMBOL(dns_async_handle_reply, int32_t, (void*), returns_int32);
    
    return (*dns_async_handle_reply_proc)(msg);
}

#undef GET_DYNAMIC_SYMBOL
#endif	/* DYNAMICALLY_LOAD_LIBRESOLV */
