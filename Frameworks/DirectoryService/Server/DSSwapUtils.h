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

/*!
 * @header DSSwapUtils
 * Provides routines to byte swap endian data buffers.
 */

#ifndef __DSSWAPUTILS_H
#define __DSSWAPUTILS_H

#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include "DirServicesTypes.h"

enum
{
	kDSSwapHostToNetworkOrder	= 0,
	kDSSwapNetworkToHostOrder	= 1,
};

typedef uint32_t eSwapDirection;

#define DSSwapLong(a,b)		DSGetAndSwapLong(a,b)

__BEGIN_DECLS

void DSSwapObjectData( UInt32 type, char* data, UInt32 size, bool swapAuth, bool isCustomCall,
					   UInt32 inCustomRequestNum, const char* inPluginName, bool isAPICallResponse,
					   eSwapDirection inSwapDir );

UInt32 DSGetLong( void* ptr, eSwapDirection inSwapDir );
UInt32 DSGetAndSwapLong( void* ptr, eSwapDirection inSwapDir );
    
__END_DECLS

#endif
