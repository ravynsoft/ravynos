/*
 * Copyright (c) 2004-2006 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * The contents of this file constitute Original Code as defined in and
 * are subject to the Apple Public Source License Version 1.1 (the
 * "License").  You may not use this file except in compliance with the
 * License.  Please obtain a copy of the License at
 * http://www.apple.com/publicsource and read it before using this file.
 *
 * This Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */

#ifndef __Mbrd_Cache_h__
#define __Mbrd_Cache_h__		1

#include <stdint.h>
#include <stdbool.h>
#include "Mbrd_UserGroup.h"

typedef struct _MbrdCache MbrdCache;

#define kDefaultExpirationServer 1*60*60
#define kDefaultNegativeExpirationServer 30*60

#define kDefaultExpirationClient 4*60*60
#define kDefaultNegativeExpirationClient 2*60*60
#define kDefaultKernelExpiration 2*60
#define kDefaultMaximumRefresh 15*60

#define KAUTH_EXTLOOKUP_REFRESH_MEMBERSHIP	(1 << 15)
#define kKernelRequest			(1 << 31)
#define kNoNegativeEntry		(1 << 30)

// these are internal types, to simplify logic
#define ID_TYPE_GUID			-1
#define ID_TYPE_GROUPMEMBERS	-2
#define ID_TYPE_GROUPMEMBERSHIP	-3
#define ID_TYPE_NESTEDGROUPS	-4
#define ID_TYPE_RID				-5
#define ID_TYPE_GROUPRID		-6
#define ID_TYPE_GROUPSID		-7

__BEGIN_DECLS

MbrdCache* MbrdCache_Create( int32_t defaultExpiration, int32_t defaultNegativeExpiration, int32_t kernelExp, int32_t maxRefresh,
							 int32_t kerberosFallback );
#define MbrdCache_Retain(a)			((MbrdCache *) dsRetainObject(a, &a->fRefCount))
void MbrdCache_Release( MbrdCache *cache );

int32_t MbrdCache_GetDefaultExpiration( MbrdCache *cache );

UserGroup* MbrdCache_GetAndRetain( MbrdCache *cache, int recordType, int idType, const void *idValue, int32_t flags );

// may return the original entry or could return an existing after merging data
UserGroup *MbrdCache_AddOrUpdate( MbrdCache *cache, UserGroup *entry, uint32_t flags );

// refreshes hashes because something about the record changed (possibly for kernel transients)
// must not be used for new entries
void MbrdCache_RefreshHashes( MbrdCache *cache, UserGroup *existing );

int MbrdCache_SetNodeAvailability( MbrdCache *cache, const char *nodeName, bool nodeAvailable );
void MbrdCache_Sweep( MbrdCache *cache );
void MbrdCache_NodeChangeOccurred( MbrdCache *cache );
void MbrdCache_ResetCache( MbrdCache *cache );
void MbrdCache_DumpState( MbrdCache *cache );
int32_t MbrdCache_TTL( MbrdCache *cache, UserGroup *entry, int32_t flags );
int32_t MbrdCache_KerberosFallback( MbrdCache *cache );

void ConvertSIDToString( char* string, ntsid_t* sid );

__END_DECLS

#endif
