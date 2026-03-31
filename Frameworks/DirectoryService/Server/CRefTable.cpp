/*
 * Copyright (c) 2008 Apple Computer, Inc. All rights reserved.
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
 * @header CRefTable
 */

#include "CRefTable.h"
#include "CLog.h"
#include "DirServicesConst.h"
#include "DSUtils.h"
#include <DirectoryServiceCore/DSSemaphore.h>
#include "CInternalDispatch.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/sysctl.h>	// for struct kinfo_proc and sysctl()
#include <syslog.h>		// for syslog()
#include <arpa/inet.h>
#include "od_passthru.h"

/*
 * working around some block issues here too
 */

//--------------------------------------------------------------------------------------------------
//	* Globals
//--------------------------------------------------------------------------------------------------

// API logging
extern bool			gLogAPICalls;
extern UInt32		gRefCountWarningLimit;

extern UInt32		gLocalSessionCount;
extern dsBool		gDSLocalOnlyMode;
extern pid_t		gDaemonPID;
extern DSSemaphore	gLocalSessionLock;

sRefEntry::sRefEntry( void )
{
	fNodeName = NULL;
	fPlugin	= NULL;
}

sRefEntry::~sRefEntry( void )
{
	tRefToEntryI	refIter;
	size_t			size = fSubRefs.size();
	
	if ( size > 0 ) {
		DbgLog( kLogInfo, "sRefEntry::~sRefEntry - Will remove %d sub-references for reference %d", (int) size, fRefNum );
		
		for ( refIter = fSubRefs.begin(); refIter != fSubRefs.end(); ) {
			sRefEntry	*tempEntry = refIter->second;
			
			fSubRefs.erase( refIter++ );
			
			fRefTable->RemoveReference( tempEntry->fRefNum );
			tempEntry->Release(); // release from this list
		}
	}
	
	RefDeallocateProc *deallocProc = fRefTable->GetDeallocProc();
	if ( deallocProc != NULL ) {
		CServerPlugin *plugin = fPlugin;
		UInt32 refNum = fRefNum;
		
		// need to use a separate queue for deletions otherwise we deadlock if a reference causes another reference
		// to be used.  
		//
		// We use a non-concurrent queue to ensure ordered cleanup.  Out-of-order cleanup can cause a node release
		// before the records that reference the node, etc.
		dispatch_async( fRefTable->fCleanupQueue,
					    ^(void) {
							CInternalDispatch::AddCapability();
							deallocProc( refNum, GetRefType(refNum), plugin );
						} );
	}
	
	DbgLog( kLogDebug, "sRefEntry::~sRefEntry - Deallocated reference %d", fRefNum );

	DSFree( fNodeName );
	fPlugin = NULL;
}

sClientEntry::sClientEntry( void )
{
	fFlags = 0;
}

sClientEntry::~sClientEntry( void )
{
	ClearChildRefs();
	
	if ( (fFlags & kClientTypeMach) != 0 ) {
		DbgLog( kLogInfo, "sClientEntry::~sClientEntry - Deallocated client entry for PID %d mach port %u", clientID.fPID, portInfo.fMachPort );
	}
	else if ( LoggingEnabled(kLogInfo) ) {
		char	clientIP[ INET6_ADDRSTRLEN ] = { 0, };

		GetClientIPString( (sockaddr *) &clientID.fAddress, clientIP, sizeof(clientIP) );
		
		DbgLog( kLogInfo, "sClientEntry::~sClientEntry - Deallocated client entry for remote client %s port %u", clientIP, portInfo.fSocket );
	}
}

void
sClientEntry::ClearChildRefs( void )
{
	tRefToEntryI	refIter;
	
	for ( refIter = fSubRefs.begin(); refIter != fSubRefs.end(); ) {
		sRefEntry	*tempEntry = refIter->second;
		
		fSubRefs.erase( refIter++ );
		
		fRefTable->RemoveReference( tempEntry->fRefNum );
		tempEntry->Release(); // release from this list
	}
}

CRefTable::CRefTable( RefDeallocateProc *deallocProc ) : fDeallocProc(deallocProc)
{
	fNextIndex = 0;
	fQueue = dispatch_queue_create( "CRefTable", NULL );
	fCleanupQueue = dispatch_queue_create( "CRefTableCleanup", NULL );
	dispatch_queue_set_width( fQueue, LONG_MAX );
}

CRefTable::~CRefTable( void )
{
	dispatch_release( fCleanupQueue );
	dispatch_release( fQueue );
	fQueue = NULL;
}

sRefEntry *
CRefTable::GetRefEntry( UInt32 inRef )
{
	__block sRefEntry		*entry = NULL;
	__block tRefToEntryI	iter;
	
	dispatch_sync( fQueue,
				   ^(void) {
					   iter = fRefToEntry.find( inRef );
					   if ( iter != fRefToEntry.end() ) {
						   entry = iter->second->Retain();
					   }
				   } );
	
	return entry;
}

tDirStatus
CRefTable::CreateReference( UInt32 *outRef, eRefType inType, CServerPlugin *inPlugin, UInt32 inParentID, pid_t inPID, mach_port_t inMachPort, 
							sockaddr *inAddress, int inSocket, const char *inNodeName )
{
	UInt32							type	= (((UInt32) inType) << 24);
	__block tDirStatus				status	= eDSRefSpaceFull;
	__block UInt32					newRef	= 0;
	__block sClientEntry			*client = NULL;
	__block sRefEntry				*entry	= NULL;
	__block sRefEntry				*parent = NULL;
	__block tRefToClientEntryI		clientIter;
	__block tRefToEntryI			refIter;
	__block tPortToClientEntryI		portIter;
	__block tMachPortToClientEntryI	machIter;
	__block size_t					size;
	__block UInt32					warnLimit;
	__block char					*clientName;
	
	if ( inPID == 0 && inSocket == 0 ) {
		inPID = gDaemonPID;
	}
	
	if ( inParentID != 0 ) {
		
		if ( VerifyReference(inParentID, GetRefType(inParentID), NULL, inMachPort, inSocket) == eDSNoErr ) {
			dispatch_sync( fQueue, 
						   ^(void) {
							   refIter = fRefToEntry.find( inParentID );
							   if ( refIter != fRefToEntry.end() ) {
								   parent = refIter->second->Retain();
								   
								   // now get the client pointer
								   clientIter = fRefToClientEntry.find( inParentID );
								   if ( clientIter != fRefToClientEntry.end() ) {
									   client = clientIter->second->Retain();
								   }
							   }
						   } );
		}
		
		if ( parent == NULL ) {
			return eDSInvalidReference;
		}
	}
	else {
		dispatch_barrier_sync( fQueue,
					   ^(void) {
						   if ( inSocket != 0 ) {
							   portIter = fPortToClientEntry.find( inSocket );
							   if ( portIter != fPortToClientEntry.end() ) {
								   client = portIter->second->Retain();
							   }
						   }
						   else {
							   machIter = fMachPortToClientEntry.find( inMachPort );
							   if ( machIter != fMachPortToClientEntry.end() ) {
								   client = machIter->second->Retain();
							   }
						   }
						   
						   if ( client == NULL ) {
							   client = new sClientEntry;
							   client->fRefTable = this;
							   if ( inSocket != 0 ) {
								   client->portInfo.fSocket = inSocket;
								   if ( inAddress != NULL ) {
									   bcopy( inAddress, &client->clientID, inAddress->sa_len );
								   }
								   client->fFlags = kClientTypeTCP;
								   fPortToClientEntry[inSocket] = client->Retain();
							   }
							   else {
								   client->portInfo.fMachPort = inMachPort;
								   client->clientID.fPID = inPID;
								   client->fFlags = kClientTypeMach;
								   fMachPortToClientEntry[inMachPort] = client->Retain();
							   }
						   }
					   } );
	}
	
	// we shouldn't reach max limit, so don't expect it
	if ( DSexpect_true(fRefToEntry.size() < 0xfffe) ) {
		dispatch_barrier_sync( fQueue,
					   ^(void) {
						   while ( 1 ) {
							   // we should have mostly empty spots in normal case
							   newRef = type | (fNextIndex++ & kIndexMask);
							   refIter = fRefToEntry.find( newRef );
							   if ( refIter == fRefToEntry.end() ) {
								   break;
							   }
						   }
						   
						   entry = new sRefEntry;
						   entry->fParentID = inParentID;
						   entry->fRefNum = newRef;
						   entry->fNodeName = (inNodeName ? strdup(inNodeName) : NULL);
						   entry->fPlugin = inPlugin;
						   entry->fRefTable = this;
						   
						   fRefToEntry[newRef] = entry->Retain();
						   if ( client != NULL ) {
							   client->fSubRefs[newRef] = entry->Retain();	// add to the subrefs
							   fRefToClientEntry[newRef] = client->Retain();	// link to client
							   
							   size = client->fSubRefs.size();
							   warnLimit = (inPID == gDaemonPID ? 2000 : gRefCountWarningLimit);
							   
							   if ( size > 0 && (size % warnLimit) == 0 ) {
								   if ( DSexpect_true(inPID != gDaemonPID) ) {
									   clientName = dsGetNameForProcessID( inPID );
									   
									   syslog( LOG_ALERT, "Client: %s - PID: %d, has %d open references, the warning limit is %d.",
											  clientName, inPID, size, warnLimit );
									   DbgLog( kLogError, "Client: %s - PID: %d, has %d open references, the warning limit is %d.",
											  clientName, inPID, size, warnLimit );
									   DSFree( clientName );
								   }
								   else {
									   syslog( LOG_ALERT, "DirectoryService has %d internal references open (due to clients), warning limit is %d.",
											   size, warnLimit );
									   DbgLog( kLogError, "DirectoryService has %d internal references open (due to clients), warning limit is %d.",
											   size, warnLimit );
								   }
							   }
							   else if (gLogAPICalls) {
								   syslog( LOG_ALERT,"Client PID: %d, has %d open references.", inPID, size );
							   }
						   }
						   
						   if ( parent != NULL ) {
							   parent->fSubRefs[newRef] = entry->Retain();
						   }
						   
						   // now set the returns
						   *outRef = newRef;
						   status = eDSNoErr;
					   } );
	}
	
	if ( client != NULL ) {
		client->Release();
	}

	if ( entry != NULL ) {
		entry->Release();
	}

	if ( parent != NULL ) {
		parent->Release();
	}
			
	return status;
}

tDirStatus
CRefTable::VerifyReference( UInt32 inRef, eRefType inType, CServerPlugin **outPlugin, mach_port_t inMachPort, int inSocket )
{
	__block tDirStatus			status	= eDSInvalidReference;
	__block sClientEntry		*client	= NULL;
	__block tRefToClientEntryI	clientIter;
	__block tRefToEntryI		entryIter;
	
	if ( GetRefType(inRef) != inType ) {
		DbgLog( kLogNotice, "CRefTable::VerifyReference - reference value of <%u> is not reference is wrong type.", inRef, inType );
		return eDSInvalidRefType;
	}

	dispatch_sync( fQueue, 
				   ^(void) {
					   clientIter = fRefToClientEntry.find( inRef );
					   if ( clientIter != fRefToClientEntry.end() ) {
						   client = clientIter->second->Retain();
					   }
				   } );
	
	if ( client != NULL ) {
		if ( inSocket != 0 ) {
			if ( (client->fFlags & kClientTypeTCP) != 0 ) {
				if ( client->portInfo.fSocket == inSocket ) {
					if ( outPlugin != NULL ) {
						dispatch_sync( fQueue, 
									   ^(void) {
										   entryIter = client->fSubRefs.find( inRef );
										   if ( entryIter != client->fSubRefs.end() ) {
											   (*outPlugin) = entryIter->second->fPlugin;
											   status = eDSNoErr;
										   }
									   } );
					}
					else {
						status = eDSNoErr;
					}
				}
			}
			
			if ( status != eDSNoErr ) {
				DbgLog( kLogNotice, "CRefTable::VerifyReference - reference value of <%u> was found but client TCP port mismatch %d != %d",
					    inRef, inSocket, client->portInfo.fSocket );
			}
		}
		else {
			if ( (client->fFlags & kClientTypeMach) != 0 ) {
				if ( client->portInfo.fMachPort == inMachPort ) {
					if ( outPlugin != NULL ) {
						dispatch_sync( fQueue, 
									   ^(void) {
										   entryIter = client->fSubRefs.find( inRef );
										   if ( entryIter != client->fSubRefs.end() ) {
											   (*outPlugin) = entryIter->second->fPlugin;
											   status = eDSNoErr;
										   }
									   } );
					}
					else {
						status = eDSNoErr;
					}
				}
			}
			
			if ( status != eDSNoErr ) {
				DbgLog( kLogNotice, "CRefTable::VerifyReference - reference value of <%u> was found but client MACH port mismatch %d != %d",
					    inRef, inMachPort, client->portInfo.fMachPort );
			}
		}
		
		client->Release();
	}
	else {
		DbgLog( kLogNotice, "CRefTable::VerifyReference - reference value of <%u> was not found", inRef );
	}
	
	return status;
}

struct sRemoveContext
{
	UInt32				refNum;
	tRefToClientEntry	*refToClientEntry;
	tRefToEntry			*refToEntry;
};

void
CRefTable::RemoveReference( void *inContext )
{
	// need to delete from all tables
	//   fRefToEntry
	//   fRefToClientEntry
	//		-- fSubRefs
	//   fParent->fSubRefs
	
	sRemoveContext	*context = (sRemoveContext *) inContext;
	
	tRefToClientEntryI clientIter = context->refToClientEntry->find( context->refNum );
	if ( clientIter != context->refToClientEntry->end() ) {
		sClientEntry	*client = clientIter->second;
		
		context->refToClientEntry->erase( clientIter );
		
		tRefToEntryI refIter = client->fSubRefs.find( context->refNum );
		if ( refIter != client->fSubRefs.end() ) {
			sRefEntry *entry = refIter->second;
			
			client->fSubRefs.erase( refIter );
			
			DbgLog( kLogDebug, "CRefTable::RemoveReference - Removed reference %d from client subrefs", context->refNum );
			entry->Release();
		}
		
		client->Release();
	}
	
	tRefToEntryI refIter = context->refToEntry->find( context->refNum );
	if ( refIter != context->refToEntry->end() ) {
		sRefEntry *	entry = refIter->second;
		UInt32		parentID = entry->fParentID;
		
		context->refToEntry->erase( refIter );
		
		if ( parentID != 0 ) {
			refIter = context->refToEntry->find( parentID );
			if ( refIter != context->refToEntry->end() ) {
				sRefEntry *	parent = refIter->second;
				
				refIter = parent->fSubRefs.find( context->refNum );
				if ( refIter != parent->fSubRefs.end() ) {
					parent->fSubRefs.erase( refIter );
					DbgLog( kLogDebug, "CRefTable::RemoveReference - Removed reference %d from parent %d subrefs", context->refNum, parentID );
					
					entry->Release();
				}
			}
		}
		
		// if we are in localonly mode, we need to decrement our session count here
		if ( gDSLocalOnlyMode == true && GetRefType(context->refNum) == eRefTypeDir ) {
			if (__sync_sub_and_fetch( &gLocalSessionCount, 1) == 0) {
				od_passthru_localonly_exit();
			}
		}
		
		DbgLog( kLogInfo, "CRefTable::RemoveReference - Removed reference %d", context->refNum );
		entry->Release();
	}

	delete context;
}

void
CRefTable::RemoveReference( UInt32 inRef )
{
	sRemoveContext *context = new sRemoveContext;
	context->refNum = inRef;
	context->refToClientEntry = &fRefToClientEntry;
	context->refToEntry = &fRefToEntry;

	// workaround dispatch + block related limitations
	dispatch_barrier_async_f( fQueue, context, RemoveReference );
}

tDirStatus
CRefTable::RemoveReference( UInt32 inRef, eRefType inRefType, mach_port_t inMachPort, int inSocket )
{
	tDirStatus	status	= VerifyReference( inRef, inRefType, NULL, inMachPort, inSocket );
	
	if ( status == eDSNoErr ) {
		RemoveReference( inRef );
	}
	
	return status;
}

CServerPlugin *
CRefTable::GetPluginForRef( UInt32 inRef )
{
	sRefEntry		*entry		= GetRefEntry( inRef );
	CServerPlugin	*pluginPtr	= NULL;
	
	if ( entry != NULL ) {
		pluginPtr = entry->fPlugin;
		entry->Release();
	}
	
	return pluginPtr;
}

char *
CRefTable::CopyNodeRefName( tDirNodeReference inDirNodeRef )
{
	char *returnName = NULL;
	
	sRefEntry *entry = GetRefEntry( inDirNodeRef );
	if ( entry != NULL ) {
		if ( entry->fNodeName != NULL ) {
			returnName = strdup( entry->fNodeName );
		}
		
		entry->Release();
	}
	
	return returnName;
}

tDirStatus
CRefTable::SetNodePluginPtr( tDirNodeReference inNodeRef, CServerPlugin *inPlugin )
{
	tDirStatus status = eDSInvalidReference;
	
	sRefEntry *entry = GetRefEntry( inNodeRef );
	if ( entry != NULL ) {
		entry->fPlugin = inPlugin;
		entry->Release();
		status = eDSNoErr;
	}
	
	return status;
}

void
CRefTable::CleanRefsForSocket( int inSocket )
{
	if ( inSocket > 0 ) {
		__block tPortToClientEntryI	portIter;
		__block sClientEntry		*client;
		__block char				*clientIP;
		
		dispatch_barrier_sync( fQueue,
					   ^(void) {
							portIter = fPortToClientEntry.find( inSocket );
							if ( portIter != fPortToClientEntry.end() ) {
								client = portIter->second;
								fPortToClientEntry.erase( portIter );
								
								if ( LoggingEnabled(kLogNotice) || gLogAPICalls ) {
									clientIP = (char *)calloc(1, INET6_ADDRSTRLEN);
									
									GetClientIPString( (sockaddr *) &client->clientID.fAddress, clientIP, INET6_ADDRSTRLEN );

									DbgLog( kLogNotice, "Remote Address: %s, Socket: %u, had %d open references before cleanup.", 
										    clientIP, client->portInfo.fSocket, client->fSubRefs.size() );
									free(clientIP);
									
									if (gLogAPICalls) {
										syslog( LOG_ALERT, "Remote Address: %d, Socket: %u, had %d open references before cleanup.", 
											    client->clientID.fPID, client->portInfo.fSocket, client->fSubRefs.size() );
									}
								}
								
								client->ClearChildRefs();
								client->Release();
							}
						} );
	}
}

void
CRefTable::CleanRefsForMachRefs( mach_port_t inMachPort )
{
	__block tMachPortToClientEntryI	machIter;
	__block sClientEntry			*client;
	
	dispatch_barrier_sync( fQueue,
				   ^(void) {
					   machIter = fMachPortToClientEntry.find( inMachPort );
					   if ( machIter != fMachPortToClientEntry.end() ) {
						   client = machIter->second;
						   fMachPortToClientEntry.erase( machIter );
						   
						   DbgLog( kLogNotice, "Client PID: %d, had %d open references before cleanup.", client->clientID.fPID,
								   client->fSubRefs.size() );
						   if (gLogAPICalls) {
							   syslog( LOG_ALERT, "Client PID: %d, had %d open references before cleanup.", client->clientID.fPID, 
									   client->fSubRefs.size() );
						   }
						   
						   // we clear child refs first so other tables get cleaned up accordingly
						   client->ClearChildRefs();
						   client->Release();
					   }
				   } );
}

void
CRefTable::CleanRefsForPlugin(CServerPlugin *inPlugin, dispatch_block_t completeBlock)
{
	__block tMachPortToClientEntryI	machIter;
	__block tPortToClientEntryI		portIter;
	__block sClientEntry *			client;
	__block tRefToEntryI			refIter;
	__block sRefEntry *				refEntry;
	
	if (inPlugin != NULL) {
		dispatch_barrier_sync(fQueue,
							  ^(void) {
								  for (machIter = fMachPortToClientEntry.begin(); machIter != fMachPortToClientEntry.end(); machIter++) {
									  client = machIter->second;
									  
									  for (refIter = client->fSubRefs.begin(); refIter != client->fSubRefs.end(); refIter++) {
										  refEntry = refIter->second;
										  if (refEntry->fPlugin == inPlugin) {
											  DbgLog(kLogInfo, "Force removed reference %d due to plugin disable", refEntry->fRefNum);
											  RemoveReference(refEntry->fRefNum);
										  }
									  }
								  }
								  
								  for (portIter = fPortToClientEntry.begin(); portIter != fPortToClientEntry.end(); portIter++) {
									  client = portIter->second;
									  
									  for (refIter = client->fSubRefs.begin(); refIter != client->fSubRefs.end(); refIter++) {
										  refEntry = refIter->second;
										  if (refEntry->fPlugin == inPlugin) {
											  DbgLog(kLogInfo, "Force removed reference %d due to plugin disable", refEntry->fRefNum);
											  RemoveReference(refEntry->fRefNum);
										  }
									  }
								  }
							  });
	}

	// add the completion block to the tail end of the queue list as a write operation to ensure it happens
	// there is still a potential race here, but it's the best we can do without significant change
	dispatch_barrier_async(fQueue, 
						   ^(void) {
							   // dispatch sync to the cleanup queue to ensure the completion block finishes
							   dispatch_async(fCleanupQueue, completeBlock);
						   });
}

int
GetClientIPString( sockaddr *address, char *clientIP, size_t client_size )
{
	switch ( address->sa_family ) {
		case AF_INET:
			return (inet_ntop( AF_INET, &((sockaddr_in *) address)->sin_addr, clientIP, client_size ) == NULL);
			
		case AF_INET6:
			return (inet_ntop( AF_INET6, &((sockaddr_in6 *) address)->sin6_addr, clientIP, client_size ) == NULL);
	}
	
	return -1;
}

vector<string> *
CRefTable::GetClientPIDListStrings( void )
{
	vector<tClientDetails>	*entries	= GetClientDetails();
	vector<string>			*returnList	= new vector<string>;
	
	for ( vector<tClientDetails>::iterator iter = entries->begin(); iter != entries->end(); iter++ )
	{
		char			*outString	= NULL;
		tClientDetails	*details	= &(*iter);
		size_t			size		= details->refs.size();
		
		// we have more mach clients than anything so expect as such
		if ( DSexpect_true((details->flags & kClientTypeMach) != 0) ) {
			asprintf( &outString, "MachPort: %6u, PID: %6u, TotalRefs: %6u, Refs: ", details->portInfo.fMachPort, details->clientID.fPID,
					  size );
		}
		else {
			char clientIP[ INET6_ADDRSTRLEN ] = { 0, };
			
			GetClientIPString( (sockaddr *) &details->clientID.fAddress, clientIP, sizeof(clientIP) );
			asprintf( &outString, "IP: %s, Socket: %6u, TotalRefs: %6u, Refs: ", clientIP, details->portInfo.fSocket,
					  size );
		}
		
		// most clients will have a ref
		if ( DSexpect_true(size > 0) ) {
			size_t		outLen	= strlen( outString );
			size_t		refLen	= size * (8 + 1);
			size_t		tempLen = outLen + refLen + 1;
			
			outString = (char *) reallocf( outString, tempLen );
			char *tempString = outString + outLen;
			tempLen -= outLen;

			vector<UInt32>::iterator refIter = details->refs.begin();

			snprintf( tempString, tempLen, "%8u", (unsigned int) (*refIter) );
			tempString += 8;
			tempLen -= 8;
			
			for ( ++refIter; refIter != details->refs.end(); refIter++ ) {
				snprintf( tempString, tempLen, ",%8u", (unsigned int) (*refIter) );
				tempString += 9;
				tempLen -= 9;
			}
		}
		
		returnList->push_back( outString );
		free( outString );
	}
	
	DSDelete( entries );
	
	return returnList;
}

vector<tClientDetails> *
CRefTable::GetClientDetails( mach_port_t inMachPort, int inSocket )
{
	__block vector<tClientDetails>	*returnList = new vector<tClientDetails>;
	__block tMachPortToClientEntryI	machIter;
	__block tPortToClientEntryI		portIter;
	__block tRefToEntryI			entryIter;
	__block sClientEntry *			clientEntry = NULL;
	__block tClientDetails			details;

	void (^copyEntry)(sClientEntry *, tClientDetails *) = ^(sClientEntry *inEntry, tClientDetails *inDetails) {
		inDetails->clientID = inEntry->clientID;
		inDetails->flags = inEntry->fFlags;
		inDetails->portInfo = inEntry->portInfo;
		inDetails->refs.clear();
		
		for ( entryIter = inEntry->fSubRefs.begin(); entryIter != inEntry->fSubRefs.end(); entryIter++ ) {
			inDetails->refs.push_back( entryIter->second->fRefNum );
		}
	};

	if ( inMachPort != -1 && inSocket != -1 ) {
		if ( inMachPort != -1 ) {
			dispatch_sync( fQueue, 
						   ^(void) {
							   machIter = fMachPortToClientEntry.find( inMachPort );
							   if ( machIter != fMachPortToClientEntry.end() ) {
								   copyEntry( machIter->second, &details );
								   returnList->push_back( details );
							   }
						   } );
		}
		else {
			dispatch_sync( fQueue, 
						   ^(void) {
							   portIter = fPortToClientEntry.find( inSocket );
							   if ( portIter != fPortToClientEntry.end() ) {
								   copyEntry( machIter->second, &details );
								   returnList->push_back( details );
							   }
						   } );
		}
	}
	else {
		dispatch_sync( fQueue, 
					   ^(void) {
						   for ( machIter = fMachPortToClientEntry.begin(); machIter != fMachPortToClientEntry.end(); machIter++ ) {
							   copyEntry( machIter->second, &details );
							   returnList->push_back( details );
						   }
						   
						   for ( portIter = fPortToClientEntry.begin(); portIter != fPortToClientEntry.end(); portIter++ ) {
							   copyEntry( portIter->second, &details );
							   returnList->push_back( details );
						   }
					   } );
	}
	
	return returnList;
}
