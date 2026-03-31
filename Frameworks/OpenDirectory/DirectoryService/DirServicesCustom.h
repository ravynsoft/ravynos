/*
 * Copyright (c) 2009 Apple Inc. All rights reserved.
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
 * @header DirServicesCustom
 * To be deprecated.
 */

#ifndef __DirServicesCustom_h__
#define __DirServicesCustom_h__	1

#include <AvailabilityMacros.h>
#include <DirectoryService/DirServicesTypes.h>

#ifdef __cplusplus
extern "C" {
#endif

//-------------------------------------------------------------------------
// register custom allocate/deallocate routines MacOS 8.x Applications Only
tDirStatus	dsRegisterCustomMemory		(	tDirReference		inDirReference,
											fpCustomAllocate	inCustomAllocate,
											fpCustomDeAllocate	inCustomDeAllocate,
											tClientData			inClientData		)
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;
	
tDirStatus	dsGetCustomAllocate			(	tDirReference		inDirReference,
											fpCustomAllocate	*outCustomAllocate,
											fpCustomDeAllocate	*outCustomDeAllocate,
											tClientData			*outClientData		)
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;

tDirStatus	dsUnRegisterCustomMemory	(	tDirReference		inDirReference,
											fpCustomAllocate	inCustomAllocate,
											fpCustomDeAllocate	inCustomDeAllocate,
											tClientData			inClientData		)
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;

//------------------------------------------------------------
// register custom thread routines MacOS 8.x Applications Only
tDirStatus	dsRegisterCustomThread		(	tDirReference			inDirReference,
											fpCustomThreadBlock		inCustomBlock,
											fpCustomThreadUnBlock	inCustomUnBlock,
											fpCustomThreadYield		inCustomYield,
											tClientData				inClientData	)
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;

tDirStatus	dsGetCustomThread			(	tDirReference			inDirReference,
											fpCustomThreadBlock		*outCustomBlock,
											fpCustomThreadUnBlock	*outCustomUnBlock,
											fpCustomThreadYield		*outCustomYield,
											tClientData				*outClientData	)
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;

tDirStatus	dsUnRegisterCustomThread	(	tDirReference			inDirReference,
											fpCustomThreadBlock		inCustomBlock,
											fpCustomThreadUnBlock	inCustomUnBlock,
											fpCustomThreadYield		inCustomYield,
											tClientData				inClientData	)
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;

#ifdef __cplusplus
}
#endif

#endif
