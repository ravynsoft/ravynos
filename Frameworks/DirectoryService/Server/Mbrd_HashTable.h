/*
 * Copyright (c) 2004-2009 Apple Inc. All rights reserved.
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

#ifndef __Mbrd_HashTable_h__
#define	__Mbrd_HashTable_h__		1

#include <DirectoryServiceCore/DSUtils.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <dispatch/dispatch.h>
#include "rb.h"

typedef enum eHashType
{
	eIDHash			= 1,
	eGUIDHash		= 2,
	eSIDHash		= 3,
	eNameHash		= 4,
	eKerberosHash	= 5,
	eX509DNHash		= 6
} eHashType;

typedef struct HashTable
{
	volatile int32_t	fRefCount;
	
	dispatch_queue_t	fQueue;
	struct rb_tree		fRBtree;
	uint32_t			fHashType;
	long				fKeyOffset;
	long				fNumEntries;
	void *				fOwner;
	const char *		fName;
} HashTable;

struct UserGroup;

__BEGIN_DECLS

HashTable* HashTable_Create( const char *name, void *owner, eHashType hashType );
void HashTable_Initialize( HashTable *hash, const char *name, void *owner, eHashType hashType );
void HashTable_FreeContents( HashTable *hash );
void HashTable_Reset( HashTable* hash );
int HashTable_ResetMemberships( HashTable *hash ); // only clears online entries

#define HashTable_Retain(a)		((HashTable *) dsRetainObject(a, &a->fRefCount))
void HashTable_Release( HashTable* hash );

bool HashTable_Add( HashTable* hash, struct UserGroup* item, bool replaceExisting );
struct UserGroup* HashTable_GetAndRetain( HashTable* hash, const void* data );
void HashTable_Remove( HashTable* hash, struct UserGroup* item );

void HashTable_Merge( HashTable* destination, HashTable* source );
int HashTable_CreateItemArray( HashTable *hash, struct UserGroup*** itemArray );

__END_DECLS

#endif
