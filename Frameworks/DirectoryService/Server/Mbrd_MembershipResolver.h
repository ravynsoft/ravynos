/*
 * Copyright (c) 2006 Apple Computer, Inc. All rights reserved.
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

#ifndef DISABLE_SEARCH_PLUGIN
 
#ifndef __Mbrd_MembershipResolver_h__
#define	__Mbrd_MembershipResolver_h__

#include "DSmemberdMIG_types.h"
#include "Mbrd_Cache.h"
#include <stdbool.h>
#include <membership.h>

__BEGIN_DECLS

void Mbrd_SwapRequest(struct kauth_identity_extlookup* request);
void Mbrd_ProcessLookup(struct kauth_identity_extlookup* request);
int Mbrd_ProcessGetGroups(uint32_t uid, uint32_t* numGroups, GIDArray gids);
int Mbrd_ProcessGetAllGroups(uint32_t uid, uint32_t *numGroups, GIDList *gids );
int Mbrd_ProcessMapIdentifier(int idType, const void *identifier, ssize_t identifierSize, guid_t *guid);
void Mbrd_ProcessGetStats(StatBlock *stats);
void Mbrd_ProcessResetStats(void);
void Mbrd_ProcessDumpState(void);
void Mbrd_InitializeGlobals(void);
void Mbrd_Initialize(void);
int Mbrd_SetNodeAvailability( const char *nodeName, bool nodeAvailable );
void Mbrd_SweepCache(void *);
void Mbrd_ProcessResetCache( void );

void Mbrd_SetMembershipThread( bool bActive );
bool Mbrd_IsMembershipThread( void );

void dsNodeStateChangeOccurred( void ); // this expires entries but does not remove them
void dsFlushMembershipCache( void ); // this flushes the cache entirely
bool dsIsUserMemberOfGroup( const char *insername, const char *inGroupName );

__END_DECLS

#endif

#else

#include <stdbool.h>
#include <unistd.h>

__BEGIN_DECLS

void dsFlushMembershipCache( void ); // this flushes the cache entirely
bool dsIsUserMemberOfGroup( const char *inUsername, const char *inGroupName );
#define Mbrd_IsMembershipThread() false

__END_DECLS

#endif
