/*
 * Copyright (c) 2002 Apple Computer, Inc. All rights reserved.
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
 * @header CNodeList
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <mach/mach_time.h>	// for dsTimeStamp

#include "CLog.h"
#include "CNodeList.h"
#include "CRefTable.h"
#include "CServerPlugin.h"
#include "CString.h"
#include "DSCThread.h"
#include "PrivateTypes.h"
#include "DSUtils.h"
#include "CPlugInList.h"
#include "ServerControl.h"
#include "od_passthru.h"
#include <DirectoryService/DirServicesPriv.h>

extern	CPlugInList		*gPlugins;
extern dsBool			gDSLocalOnlyMode;

#ifndef DISABLE_CONFIGURE_PLUGIN
	extern DSEventSemaphore gKickConfigRequests;
#endif
extern DSEventSemaphore gKickNodeRequests;
#ifndef DISABLE_LOCAL_PLUGIN
	extern DSEventSemaphore gKickLocalNodeRequests;
#endif
#ifndef DISABLE_SEARCH_PLUGIN
	extern DSEventSemaphore gKickSearchRequests;
#endif
#ifndef DISABLE_CACHE_PLUGIN
	extern DSEventSemaphore gKickCacheRequests;
#endif
#ifndef DISABLE_BSD_PLUGIN
	extern DSEventSemaphore gKickBSDRequests;
#endif

extern	dsBool			gDSInstallDaemonMode;

// ---------------------------------------------------------------------------
//	* CNodeList ()
// ---------------------------------------------------------------------------

CNodeList::CNodeList ( void ) : fMutex("CNodeList::fMutex")
{
	fTreePtr					= nil;
	fCount						= 0;
	fNodeChangeToken			= 1001;	//some arbitrary start value
	fLocalNode					= nil;
	fCacheNode					= nil;
	fAuthenticationSearchNode	= nil;
	fContactsSearchNode			= nil;
	fNetworkSearchNode			= nil;
	fConfigureNode				= nil;
	fLocalHostedNodes			= nil;
	fDefaultNetworkNodes		= nil;
	fBSDNode					= nil;
} // CNodeList


// ---------------------------------------------------------------------------
//	* ~CNodeList ()
// ---------------------------------------------------------------------------

CNodeList::~CNodeList ( void )
{
	this->DeleteTree( &fTreePtr );
	fTreePtr = nil;

	if ( fLocalNode != nil )
	{
		if ( fLocalNode->fNodeName != nil )
		{
			free( fLocalNode->fNodeName );
		}
		free( fLocalNode );
		fLocalNode = nil;
	}

	if ( fCacheNode != nil )
	{
		if ( fCacheNode->fNodeName != nil )
		{
			free( fCacheNode->fNodeName );
		}
		free( fCacheNode );
		fCacheNode = nil;
	}

	if ( fAuthenticationSearchNode != nil )
	{
		if ( fAuthenticationSearchNode->fNodeName != nil )
		{
			free( fAuthenticationSearchNode->fNodeName );
		}
		free( fAuthenticationSearchNode );
		fAuthenticationSearchNode = nil;
	}

	if ( fContactsSearchNode != nil )
	{
		if ( fContactsSearchNode->fNodeName != nil )
		{
			free( fContactsSearchNode->fNodeName );
		}
		free( fContactsSearchNode );
		fContactsSearchNode = nil;
	}

	if ( fNetworkSearchNode != nil )
	{
		if ( fNetworkSearchNode->fNodeName != nil )
		{
			free( fNetworkSearchNode->fNodeName );
		}
		free( fNetworkSearchNode );
		fNetworkSearchNode = nil;
	}

	if ( fConfigureNode != nil )
	{
		if ( fConfigureNode->fNodeName != nil )
		{
			free( fConfigureNode->fNodeName );
		}
		free( fConfigureNode );
		fConfigureNode = nil;
	}
	
	if ( fLocalHostedNodes != nil )
	{
		this->DeleteTree( &fLocalHostedNodes );
		fLocalHostedNodes = nil;
	}

	if ( fDefaultNetworkNodes != nil )
	{
		this->DeleteTree( &fDefaultNetworkNodes );
		fDefaultNetworkNodes = nil;
	}
	
} // ~CNodeList


// ---------------------------------------------------------------------------
//	* DeleteTree ()
// ---------------------------------------------------------------------------

SInt32 CNodeList::DeleteTree ( sTreeNode **inTree )
{
	fMutex.WaitLock();

	try
	{
		if ( *inTree != nil )
		{
			this->DeleteTree( &((*inTree)->left) );
			this->DeleteTree( &((*inTree)->right) );
			if ( (*inTree)->fDataListPtr != nil )
			{
				::dsDataListDeallocatePriv( (*inTree)->fDataListPtr );
				//need to free the header as well
				free( (*inTree)->fDataListPtr );
				(*inTree)->fDataListPtr = nil;
			}
			delete( *inTree );
		}
		*inTree = nil;
	}

	catch( SInt32 err )
	{
	}

	fMutex.SignalLock();


	return( 1 );

} // DeleteTree


// ---------------------------------------------------------------------------
//	* AddNode () RETURNS ZERO IF NODE ALREADY EXISTS
// ---------------------------------------------------------------------------

SInt32 CNodeList::AddNode ( const char		*inNodeName,
							tDataList		*inListPtr,
							eDirNodeType	 inType,
							CServerPlugin	*inPlugInPtr,
							UInt32			 inToken )
{
	SInt32		siResult	= 0;

	fMutex.WaitLock();

	try
	{
		switch(inType)
		{
			case kLocalHostedType:
				siResult = AddNodeToTree( &fLocalHostedNodes, inNodeName, inListPtr, inType, inPlugInPtr, inToken );
				break;
			case kDefaultNetworkNodeType:
				siResult = AddNodeToTree( &fDefaultNetworkNodes, inNodeName, inListPtr, inType, inPlugInPtr, inToken );
				break;
			case kSearchNodeType:
				if (fAuthenticationSearchNode == nil)
				{
					siResult = AddAuthenticationSearchNode( inNodeName, inListPtr, inType, inPlugInPtr, inToken );
				}
				else
				{
					DbgLog( kLogApplication, "Attempt to register second Authentication Search Node failed." );
				}
				break;
			case kContactsSearchNodeType:
				if (fContactsSearchNode == nil)
				{
					siResult = AddContactsSearchNode( inNodeName, inListPtr, inType, inPlugInPtr, inToken );
				}
				else
				{
					DbgLog( kLogApplication, "Attempt to register second Contacts Search Node failed." );
				}
				break;
			case kNetworkSearchNodeType:
				if (fNetworkSearchNode == nil)
				{
					siResult = AddNetworkSearchNode( inNodeName, inListPtr, inType, inPlugInPtr, inToken );
				}
				else
				{
					DbgLog( kLogApplication, "Attempt to register second Network Search Node failed." );
				}
				break;
			case kConfigNodeType:
				if (fConfigureNode == nil)
				{
					siResult = AddConfigureNode( inNodeName, inListPtr, inType, inPlugInPtr, inToken );
				}
				else
				{
					DbgLog( kLogApplication, "Attempt to register second Configure Node failed." );
				}
				break;
			//no longer add the localnode name to the overall tree ie. fixed name
			case kLocalNodeType:
				if (fLocalNode == nil)
				{
					siResult = AddLocalNode( inNodeName, inListPtr, inType, inPlugInPtr, inToken );
				}
				else
				{
					DbgLog( kLogApplication, "Attempt to register second Local Node failed." );
				}
				break;
			case kCacheNodeType:
				if (fCacheNode == nil)
				{
					siResult = AddCacheNode( inNodeName, inListPtr, inType, inPlugInPtr, inToken );
				}
				else
				{
					DbgLog( kLogApplication, "Attempt to register second Cache Node failed." );
				}
				break;
			case kBSDNodeType:
				if (fBSDNode == nil)
				{
					siResult = AddBSDNode( inNodeName, inListPtr, inType, inPlugInPtr, inToken );
				}
				else
				{
					DbgLog( kLogApplication, "Attempt to register second BSD Node failed." );
				}
				break;
			case kDirNodeType:
				siResult = AddNodeToTree( &fTreePtr, inNodeName, inListPtr, inType, inPlugInPtr, inToken );
				
				// not really tDirStatus, anything other than 0 is success
				if ( siResult != 0 )
				{
					CFStringRef newNodeRef = CFStringCreateWithCString( kCFAllocatorDefault, inNodeName, kCFStringEncodingUTF8 );
					if ( newNodeRef != NULL )
					{
						SCDynamicStoreRef store = SCDynamicStoreCreate( kCFAllocatorDefault, NULL, NULL, NULL );
						if ( store != NULL ) {
							if ( SCDynamicStoreSetValue(store, CFSTR(kDSStdNotifyDirectoryNodeAdded), newNodeRef) == false ) {
								DbgLog( kLogError, "Could not notify %s node added via SystemConfiguration", inNodeName );
							}
							
							CFRelease( store );
							store = NULL;
						}
						else {
							DbgLog( kLogError, "CNodeList::AddNode - SCDynamicStoreCreate failed" );
						}
						
						CFRelease( newNodeRef );
						newNodeRef = NULL;
					}
					else
					{
						DbgLog( kLogError, "Could not notify that dir node: (%s) was added due to an encoding problem", inNodeName );
					}
					fCount++;
					fNodeChangeToken++;
				}
				break;
			default:
				break;
		}//switch end
	}

	catch( SInt32 err )
	{
		siResult = err;
	}

	fMutex.SignalLock();

	return( siResult );

} // AddNode


// ---------------------------------------------------------------------------
//	* AddLocalNode () RETURNS ZERO IF NODE ALREADY EXISTS
// ---------------------------------------------------------------------------

SInt32 CNodeList::AddLocalNode (	const char		*inNodeName,
									tDataList		*inListPtr,
									eDirNodeType	 inType,
									CServerPlugin	*inPlugInPtr,
									UInt32			 inToken )
{
	SInt32		siResult	= 1;
	sTreeNode  *aLocalNode	= nil;
	//local needed here and in all the addxxxxxnode methods below
	//since we test elsewhere on the sTreeNode pointer to use its contents
	//however, need to be assured that there is contents
	//probably should test on the contents as well

	if ( fLocalNode != nil )
	{
		if (inListPtr != nil)
		{
			::dsDataListDeallocatePriv( inListPtr );
			//need to free the header as well
			free( inListPtr );
			inListPtr = nil;
		}
		return( 0 );
	}

	fMutex.WaitLock();

	try
	{
		aLocalNode 					= (sTreeNode *)::calloc( 1, sizeof( sTreeNode ) );
		if ( aLocalNode == nil ) throw((SInt32)eMemoryAllocError);
		
		aLocalNode->fNodeName 		= (char *)::calloc( 1, ::strlen( inNodeName ) + 1 );
		if ( aLocalNode->fNodeName == nil ) throw((SInt32)eMemoryAllocError);
		
		::strcpy( aLocalNode->fNodeName, inNodeName );
		aLocalNode->fDataListPtr	= inListPtr;
		aLocalNode->fPlugInPtr		= inPlugInPtr;
		aLocalNode->fPlugInToken	= inToken;
		aLocalNode->fType			= inType;
		aLocalNode->left			= nil;
		aLocalNode->right			= nil;
		fLocalNode					= aLocalNode;
		fWaitForLN.PostEvent();
		DbgLog( kLogApplication, "Added local node to node list." );
	}

	catch( SInt32 err )
	{
		if ( aLocalNode != nil )
		{
			if (aLocalNode->fNodeName != nil)
			{
				free(aLocalNode->fNodeName);
				aLocalNode->fNodeName = nil;
			}
			if (aLocalNode->fDataListPtr != nil)
			{
				::dsDataListDeallocatePriv( aLocalNode->fDataListPtr );
				//need to free the header as well
				free ( aLocalNode->fDataListPtr );
				aLocalNode->fDataListPtr = nil;
			}
			free( aLocalNode );
			aLocalNode = nil;
			fLocalNode = nil;
		}
		siResult = 0;
	}

	fMutex.SignalLock();

	return( siResult );

} // AddLocalNode


// ---------------------------------------------------------------------------
//	* AddCacheNode () RETURNS ZERO IF NODE ALREADY EXISTS
// ---------------------------------------------------------------------------

SInt32 CNodeList::AddCacheNode (	const char		*inNodeName,
									tDataList		*inListPtr,
									eDirNodeType	 inType,
									CServerPlugin	*inPlugInPtr,
									UInt32			 inToken )
{
	SInt32		siResult	= 1;
	sTreeNode  *aCacheNode	= nil;
	//Cache needed here and in all the addxxxxxnode methods below
	//since we test elsewhere on the sTreeNode pointer to use its contents
	//however, need to be assured that there is contents
	//probably should test on the contents as well

	if ( fCacheNode != nil )
	{
		if (inListPtr != nil)
		{
			::dsDataListDeallocatePriv( inListPtr );
			//need to free the header as well
			free( inListPtr );
			inListPtr = nil;
		}
		return( 0 );
	}

	fMutex.WaitLock();

	try
	{
		aCacheNode 					= (sTreeNode *)::calloc( 1, sizeof( sTreeNode ) );
		if ( aCacheNode == nil ) throw((SInt32)eMemoryAllocError);
		
		aCacheNode->fNodeName 		= (char *)::calloc( 1, ::strlen( inNodeName ) + 1 );
		if ( aCacheNode->fNodeName == nil ) throw((SInt32)eMemoryAllocError);
		
		::strcpy( aCacheNode->fNodeName, inNodeName );
		aCacheNode->fDataListPtr	= inListPtr;
		aCacheNode->fPlugInPtr		= inPlugInPtr;
		aCacheNode->fPlugInToken	= inToken;
		aCacheNode->fType			= inType;
		aCacheNode->left			= nil;
		aCacheNode->right			= nil;
		fCacheNode					= aCacheNode;
		fWaitForCacheN.PostEvent();
	}

	catch( SInt32 err )
	{
		if ( aCacheNode != nil )
		{
			if (aCacheNode->fNodeName != nil)
			{
				free(aCacheNode->fNodeName);
				aCacheNode->fNodeName = nil;
			}
			if (aCacheNode->fDataListPtr != nil)
			{
				::dsDataListDeallocatePriv( aCacheNode->fDataListPtr );
				//need to free the header as well
				free ( aCacheNode->fDataListPtr );
				aCacheNode->fDataListPtr = nil;
			}
			free( aCacheNode );
			aCacheNode = nil;
			fCacheNode = nil;
		}
		siResult = 0;
	}

	fMutex.SignalLock();

	return( siResult );

} // AddCacheNode


// ---------------------------------------------------------------------------
//	* AddAuthenticationSearchNode () RETURNS ZERO IF NODE ALREADY EXISTS
// ---------------------------------------------------------------------------

SInt32 CNodeList:: AddAuthenticationSearchNode (	const char		*inNodeName,
													tDataList		*inListPtr,
													eDirNodeType	 inType,
													CServerPlugin	*inPlugInPtr,
													UInt32			 inToken )
{
	SInt32		siResult					= 1;
	sTreeNode  *anAuthenticationSearchNode	= nil;

	if ( fAuthenticationSearchNode != nil )
	{
		if (inListPtr != nil)
		{
			::dsDataListDeallocatePriv( inListPtr );
			//need to free the header as well
			free( inListPtr );
			inListPtr = nil;
		}
		return( 0 );
	}

	fMutex.WaitLock();

	try
	{
		anAuthenticationSearchNode 					= (sTreeNode *)::calloc( 1, sizeof( sTreeNode ) );
		if ( anAuthenticationSearchNode == nil ) throw((SInt32)eMemoryAllocError);
		
		anAuthenticationSearchNode->fNodeName		= (char *)::calloc( 1, ::strlen( inNodeName ) + 1 );
		if ( anAuthenticationSearchNode->fNodeName == nil ) throw((SInt32)eMemoryAllocError);
		
		::strcpy( anAuthenticationSearchNode->fNodeName, inNodeName );
		anAuthenticationSearchNode->fDataListPtr	= inListPtr;
		anAuthenticationSearchNode->fPlugInPtr		= inPlugInPtr;
		anAuthenticationSearchNode->fPlugInToken	= inToken;
		anAuthenticationSearchNode->fType			= inType;
		anAuthenticationSearchNode->left			= nil;
		anAuthenticationSearchNode->right			= nil;
		fAuthenticationSearchNode					= anAuthenticationSearchNode;
		fWaitForAuthenticationSN.PostEvent();
		DbgLog( kLogApplication, "Added authentication search node to node list." );
	}

	catch( SInt32 err )
	{
		if ( anAuthenticationSearchNode != nil )
		{
			if (anAuthenticationSearchNode->fNodeName != nil)
			{
				free(anAuthenticationSearchNode->fNodeName);
				anAuthenticationSearchNode->fNodeName = nil;
			}
			if (anAuthenticationSearchNode->fDataListPtr != nil)
			{
				::dsDataListDeallocatePriv( anAuthenticationSearchNode->fDataListPtr );
				//need to free the header as well
				free ( anAuthenticationSearchNode->fDataListPtr );
				anAuthenticationSearchNode->fDataListPtr = nil;
			}
			free( anAuthenticationSearchNode );
			anAuthenticationSearchNode = nil;
			fAuthenticationSearchNode = nil;
		}
		siResult = 0;
	}

	fMutex.SignalLock();

	return( siResult );

} // AddAuthenticationSearchNode


// ---------------------------------------------------------------------------
//	* AddContactsSearchNode () RETURNS ZERO IF NODE ALREADY EXISTS
// ---------------------------------------------------------------------------

SInt32 CNodeList:: AddContactsSearchNode (	const char		*inNodeName,
											tDataList		*inListPtr,
											eDirNodeType	 inType,
											CServerPlugin	*inPlugInPtr,
											UInt32			 inToken )
{
	SInt32		siResult					= 1;
	sTreeNode  *aContactsSearchNode			= nil;

	if ( fContactsSearchNode != nil )
	{
		if (inListPtr != nil)
		{
			::dsDataListDeallocatePriv( inListPtr );
			//need to free the header as well
			free( inListPtr );
			inListPtr = nil;
		}
		return( 0 );
	}

	fMutex.WaitLock();

	try
	{
		aContactsSearchNode 				= (sTreeNode *)::calloc( 1, sizeof( sTreeNode ) );
		if ( aContactsSearchNode == nil ) throw((SInt32)eMemoryAllocError);
		
		aContactsSearchNode->fNodeName		= (char *)::calloc( 1, ::strlen( inNodeName ) + 1 );
		if ( aContactsSearchNode->fNodeName == nil ) throw((SInt32)eMemoryAllocError);
		
		::strcpy( aContactsSearchNode->fNodeName, inNodeName );
		aContactsSearchNode->fDataListPtr	= inListPtr;
		aContactsSearchNode->fPlugInPtr		= inPlugInPtr;
		aContactsSearchNode->fPlugInToken	= inToken;
		aContactsSearchNode->fType			= inType;
		aContactsSearchNode->left			= nil;
		aContactsSearchNode->right			= nil;
		fContactsSearchNode					= aContactsSearchNode;
		fWaitForContactsSN.PostEvent();
	}

	catch( SInt32 err )
	{
		if ( aContactsSearchNode != nil )
		{
			if (aContactsSearchNode->fNodeName != nil)
			{
				free(aContactsSearchNode->fNodeName);
				aContactsSearchNode->fNodeName = nil;
			}
			if (aContactsSearchNode->fDataListPtr != nil)
			{
				::dsDataListDeallocatePriv( aContactsSearchNode->fDataListPtr );
				//need to free the header as well
				free ( aContactsSearchNode->fDataListPtr );
				aContactsSearchNode->fDataListPtr = nil;
			}
			free( aContactsSearchNode );
			aContactsSearchNode = nil;
			fContactsSearchNode = nil;
		}
		siResult = 0;
	}

	fMutex.SignalLock();

	return( siResult );

} // AddContactsSearchNode


// ---------------------------------------------------------------------------
//	* AddNetworkSearchNode () RETURNS ZERO IF NODE ALREADY EXISTS
// ---------------------------------------------------------------------------

SInt32 CNodeList:: AddNetworkSearchNode (	const char		*inNodeName,
											tDataList		*inListPtr,
											eDirNodeType	 inType,
											CServerPlugin	*inPlugInPtr,
											UInt32			 inToken )
{
	SInt32		siResult					= 1;
	sTreeNode  *aNetworkSearchNode			= nil;

	if ( fNetworkSearchNode != nil )
	{
		if (inListPtr != nil)
		{
			::dsDataListDeallocatePriv( inListPtr );
			//need to free the header as well
			free( inListPtr );
			inListPtr = nil;
		}
		return( 0 );
	}

	fMutex.WaitLock();

	try
	{
		aNetworkSearchNode 				= (sTreeNode *)::calloc( 1, sizeof( sTreeNode ) );
		if ( aNetworkSearchNode == nil ) throw((SInt32)eMemoryAllocError);
		
		aNetworkSearchNode->fNodeName		= (char *)::calloc( 1, ::strlen( inNodeName ) + 1 );
		if ( aNetworkSearchNode->fNodeName == nil ) throw((SInt32)eMemoryAllocError);
		
		::strcpy( aNetworkSearchNode->fNodeName, inNodeName );
		aNetworkSearchNode->fDataListPtr	= inListPtr;
		aNetworkSearchNode->fPlugInPtr		= inPlugInPtr;
		aNetworkSearchNode->fPlugInToken	= inToken;
		aNetworkSearchNode->fType			= inType;
		aNetworkSearchNode->left			= nil;
		aNetworkSearchNode->right			= nil;
		fNetworkSearchNode					= aNetworkSearchNode;
		fWaitForNetworkSN.PostEvent();
	}

	catch( SInt32 err )
	{
		if ( aNetworkSearchNode != nil )
		{
			if (aNetworkSearchNode->fNodeName != nil)
			{
				free(aNetworkSearchNode->fNodeName);
				aNetworkSearchNode->fNodeName = nil;
			}
			if (aNetworkSearchNode->fDataListPtr != nil)
			{
				::dsDataListDeallocatePriv( aNetworkSearchNode->fDataListPtr );
				//need to free the header as well
				free ( aNetworkSearchNode->fDataListPtr );
				aNetworkSearchNode->fDataListPtr = nil;
			}
			free( aNetworkSearchNode );
			aNetworkSearchNode = nil;
			fNetworkSearchNode = nil;
		}
		siResult = 0;
	}

	fMutex.SignalLock();

	return( siResult );

} // AddNetworkSearchNode


// ---------------------------------------------------------------------------
//	* AddConfigureNode () RETURNS ZERO IF NODE ALREADY EXISTS
// ---------------------------------------------------------------------------

SInt32 CNodeList:: AddConfigureNode (	const char		*inNodeName,
									tDataList		*inListPtr,
									eDirNodeType	 inType,
									CServerPlugin	*inPlugInPtr,
									UInt32			 inToken )
{
	SInt32		siResult				= 1;
	sTreeNode  *aConfigureNode			= nil;

	if ( fConfigureNode != nil )
	{
		if (inListPtr != nil)
		{
			::dsDataListDeallocatePriv( inListPtr );
			//need to free the header as well
			free( inListPtr );
			inListPtr = nil;
		}
		return( 0 );
	}

	fMutex.WaitLock();

	try
	{
		aConfigureNode 					= (sTreeNode *)::calloc( 1, sizeof( sTreeNode ) );
		if ( aConfigureNode == nil ) throw((SInt32)eMemoryAllocError);
		
		aConfigureNode->fNodeName 		= (char *)::calloc( 1, ::strlen( inNodeName ) + 1 );
		if ( aConfigureNode->fNodeName == nil ) throw((SInt32)eMemoryAllocError);
		
		::strcpy( aConfigureNode->fNodeName, inNodeName );
		aConfigureNode->fDataListPtr	= inListPtr;
		aConfigureNode->fPlugInPtr		= inPlugInPtr;
		aConfigureNode->fPlugInToken	= inToken;
		aConfigureNode->fType			= inType;
		aConfigureNode->left			= nil;
		aConfigureNode->right			= nil;
		fConfigureNode					= aConfigureNode;
		fWaitForConfigureN.PostEvent();
	}

	catch( SInt32 err )
	{
		if ( aConfigureNode != nil )
		{
			if (aConfigureNode->fNodeName != nil)
			{
				free(aConfigureNode->fNodeName);
				aConfigureNode->fNodeName = nil;
			}
			if (aConfigureNode->fDataListPtr != nil)
			{
				::dsDataListDeallocatePriv( aConfigureNode->fDataListPtr );
				//need to free the header as well
				free ( aConfigureNode->fDataListPtr );
				aConfigureNode->fDataListPtr = nil;
			}
			free( aConfigureNode );
			aConfigureNode = nil;
			fConfigureNode = nil;
		}
		siResult = 0;
	}

	fMutex.SignalLock();

	return( siResult );

} // AddConfigureNode

// ---------------------------------------------------------------------------
//	* AddBSDNode () RETURNS ZERO IF NODE ALREADY EXISTS
// ---------------------------------------------------------------------------

SInt32 CNodeList:: AddBSDNode (	const char		*inNodeName,
								tDataList		*inListPtr,
								eDirNodeType	 inType,
								CServerPlugin	*inPlugInPtr,
								UInt32			 inToken )
{
	SInt32		siResult				= 1;
	sTreeNode  *aBSDNode				= nil;
	
	if ( fBSDNode != nil )
	{
		if (inListPtr != nil)
		{
			::dsDataListDeallocatePriv( inListPtr );
			//need to free the header as well
			free( inListPtr );
			inListPtr = nil;
		}
		return( 0 );
	}
	
	fMutex.WaitLock();
	
	try
	{
		aBSDNode				= (sTreeNode *)::calloc( 1, sizeof( sTreeNode ) );
		if ( aBSDNode == nil ) throw((SInt32)eMemoryAllocError);
		
		aBSDNode->fNodeName		= (char *)::calloc( 1, ::strlen( inNodeName ) + 1 );
		if ( aBSDNode->fNodeName == nil ) throw((SInt32)eMemoryAllocError);
		
		::strcpy( aBSDNode->fNodeName, inNodeName );
		aBSDNode->fDataListPtr	= inListPtr;
		aBSDNode->fPlugInPtr	= inPlugInPtr;
		aBSDNode->fPlugInToken	= inToken;
		aBSDNode->fType			= inType;
		aBSDNode->left			= nil;
		aBSDNode->right			= nil;
		fBSDNode				= aBSDNode;
		fWaitForBSDN.PostEvent();
	}
	
	catch( SInt32 err )
	{
		if ( aBSDNode != nil )
		{
			if (aBSDNode->fNodeName != nil)
			{
				free(aBSDNode->fNodeName);
				aBSDNode->fNodeName = nil;
			}
			if (aBSDNode->fDataListPtr != nil)
			{
				::dsDataListDeallocatePriv( aBSDNode->fDataListPtr );
				//need to free the header as well
				free ( aBSDNode->fDataListPtr );
				aBSDNode->fDataListPtr = nil;
			}
			free( aBSDNode );
			aBSDNode = nil;
			fBSDNode = nil;
		}
		siResult = 0;
	}
	
	fMutex.SignalLock();
	
	return( siResult );
	
} // AddBSDNode

// ---------------------------------------------------------------------------
//	* AddNodeToTree ()
// ---------------------------------------------------------------------------

SInt32 CNodeList:: AddNodeToTree (	sTreeNode	  **inTree,
									const char	   *inNodeName,
									tDataList	   *inListPtr,
									eDirNodeType	inType,
									CServerPlugin  *inPlugInPtr,
									UInt32			 inToken )
{
	SInt32			siResult	= 1;
	sTreeNode	   *current		= nil;
	sTreeNode	   *parent		= nil;
	sTreeNode	   *pNewNode	= nil;
	bool			bAddNode	= true;

	fMutex.WaitLock();

	try
	{
		// Go down tree to find parent node of new insertion node
		current = *inTree;
		while ( current != nil )
		{
			parent = current;
			siResult = CompareCString( inNodeName, current->fNodeName );
			if ( siResult < 0 )
			{
				current = current->left;
			}
			else if ( siResult > 0 )
			{
				current = current->right;
			}
			else //we found a duplicate
			{
				current = nil;
				if (inListPtr != nil)
				{
					::dsDataListDeallocatePriv( inListPtr );
					//need to free the header as well
					free ( inListPtr );
					inListPtr = nil;
				}
				bAddNode = false;
			}
		}

		if (bAddNode)
		{
			pNewNode = (sTreeNode *)::calloc( 1, sizeof( sTreeNode ) );
			if ( pNewNode == nil ) throw((SInt32)eMemoryAllocError);
			
			pNewNode->fNodeName = (char *)::calloc( 1, ::strlen( inNodeName ) + 1 );
			if ( pNewNode->fNodeName == nil ) throw((SInt32)eMemoryAllocError);
			
			::strcpy( pNewNode->fNodeName, inNodeName );
			pNewNode->fDataListPtr	= inListPtr;
			pNewNode->fPlugInPtr	= inPlugInPtr;
			pNewNode->fPlugInToken	= inToken;
			pNewNode->fType			= inType;
			pNewNode->left			= nil;
			pNewNode->right			= nil;
	
			// If this is the first insertion then you are defining the root
			if ( parent == nil )
			{
				*inTree = pNewNode;
	
				siResult = 1;
			}
			else
			{
				// Otherwise we are assigning a newnode to the left or right of the parent leaf node
				siResult = CompareCString( inNodeName, parent->fNodeName );
				if ( siResult < 0 )
				{
					parent->left = pNewNode;
				}
				else if ( siResult > 0 )
				{
					parent->right = pNewNode;
				}
			}
		} //if (bAddNode)
	}

	catch( SInt32 err )
	{
		if ( pNewNode != nil )
		{
			if (pNewNode->fNodeName != nil)
			{
				free(pNewNode->fNodeName);
				pNewNode->fNodeName = nil;
			}
			if (pNewNode->fDataListPtr != nil)
			{
				::dsDataListDeallocatePriv( pNewNode->fDataListPtr );
				//need to free the header as well
				free ( pNewNode->fDataListPtr );
				pNewNode->fDataListPtr = nil;
			}
			free( pNewNode );
			pNewNode = nil;
		}
		siResult = 0;
	}

	fMutex.SignalLock();

	return( siResult );

} // AddNodeToTree


// ---------------------------------------------------------------------------
//	* GetLocalNode ()
// ---------------------------------------------------------------------------

bool CNodeList::GetLocalNode ( CServerPlugin **outPlugInPtr )
{
	bool	found = false;

	fMutex.WaitLock();

	if ( fLocalNode != nil )
	{
		*outPlugInPtr = GetPluginPtr(fLocalNode);
		found = true;
	}

	fMutex.SignalLock();

	return( found );

} // GetLocalNode 


// ---------------------------------------------------------------------------
//	* GetLocalNode ()
// ---------------------------------------------------------------------------

bool CNodeList::GetLocalNode ( tDataList **outListPtr )
{
	bool	found = false;

	fMutex.WaitLock();

	if ( fLocalNode != nil )
	{
		*outListPtr = fLocalNode->fDataListPtr;
		found = true;
	}

	fMutex.SignalLock();

	return( found );

} // GetLocalNode 


// ---------------------------------------------------------------------------
//	* GetCacheNode ()
// ---------------------------------------------------------------------------

bool CNodeList::GetCacheNode ( CServerPlugin **outPlugInPtr )
{
	bool	found = false;

	fMutex.WaitLock();

	if ( fCacheNode != nil )
	{
		*outPlugInPtr = GetPluginPtr(fCacheNode);
		found = true;
	}

	fMutex.SignalLock();

	return( found );

} // GetCacheNode 


// ---------------------------------------------------------------------------
//	* GetCacheNode ()
// ---------------------------------------------------------------------------

bool CNodeList::GetCacheNode ( tDataList **outListPtr )
{
	bool	found = false;

	fMutex.WaitLock();

	if ( fCacheNode != nil )
	{
		*outListPtr = fCacheNode->fDataListPtr;
		found = true;
	}

	fMutex.SignalLock();

	return( found );

} // GetCacheNode 


// ---------------------------------------------------------------------------
//	* GetAuthenticationSearchNode ()
// ---------------------------------------------------------------------------

bool CNodeList::GetAuthenticationSearchNode ( CServerPlugin **outPlugInPtr )
{
	bool	found = false;

	fMutex.WaitLock();

	if ( fAuthenticationSearchNode != nil )
	{
		*outPlugInPtr = GetPluginPtr(fAuthenticationSearchNode);
		found = true;
	}

	fMutex.SignalLock();

	return( found );

} // GetAuthenticationSearchNode 


// ---------------------------------------------------------------------------
//	* GetAuthenticationSearchNode ()
// ---------------------------------------------------------------------------

bool CNodeList::GetAuthenticationSearchNode ( tDataList **outListPtr )
{
	bool	found = false;

	fMutex.WaitLock();

	if ( fAuthenticationSearchNode != nil )
	{
		*outListPtr = fAuthenticationSearchNode->fDataListPtr;
		found = true;
	}

	fMutex.SignalLock();

	return( found );

} // GetAuthenticationSearchNode 


// ---------------------------------------------------------------------------
//	* GetContactsSearchNode ()
// ---------------------------------------------------------------------------

bool CNodeList::GetContactsSearchNode ( CServerPlugin **outPlugInPtr )
{
	bool	found = false;

	fMutex.WaitLock();

	if ( fContactsSearchNode != nil )
	{
		*outPlugInPtr = GetPluginPtr(fContactsSearchNode);
		found = true;
	}

	fMutex.SignalLock();

	return( found );

} // GetContactsSearchNode 


// ---------------------------------------------------------------------------
//	* GetContactsSearchNode ()
// ---------------------------------------------------------------------------

bool CNodeList::GetContactsSearchNode ( tDataList **outListPtr )
{
	bool	found = false;

	fMutex.WaitLock();

	if ( fContactsSearchNode != nil )
	{
		*outListPtr = fContactsSearchNode->fDataListPtr;
		found = true;
	}

	fMutex.SignalLock();

	return( found );

} // GetContactsSearchNode 


// ---------------------------------------------------------------------------
//	* GetNetworkSearchNode ()
// ---------------------------------------------------------------------------

bool CNodeList::GetNetworkSearchNode ( CServerPlugin **outPlugInPtr )
{
	bool	found = false;

	fMutex.WaitLock();

	if ( fNetworkSearchNode != nil )
	{
		*outPlugInPtr = GetPluginPtr(fNetworkSearchNode);
		found = true;
	}

	fMutex.SignalLock();

	return( found );

} // GetNetworkSearchNode 


// ---------------------------------------------------------------------------
//	* GetNetworkSearchNode ()
// ---------------------------------------------------------------------------

bool CNodeList::GetNetworkSearchNode ( tDataList **outListPtr )
{
	bool	found = false;

	fMutex.WaitLock();

	if ( fNetworkSearchNode != nil )
	{
		*outListPtr = fNetworkSearchNode->fDataListPtr;
		found = true;
	}

	fMutex.SignalLock();

	return( found );

} // GetNetworkSearchNode 


// ---------------------------------------------------------------------------
//	* RegisterAll ()
// ---------------------------------------------------------------------------

void CNodeList::RegisterAll ( void )
{
	fMutex.WaitLock();

	try
	{
		this->Register( fTreePtr );
		if (fAuthenticationSearchNode != NULL) {
			od_passthru_register_node(fAuthenticationSearchNode->fNodeName, false);
		}

		if (fContactsSearchNode != NULL) {
			od_passthru_register_node(fContactsSearchNode->fNodeName, false);
		}

		if (fNetworkSearchNode != NULL) {
			od_passthru_register_node(fNetworkSearchNode->fNodeName, true);
		}
		
		if (fConfigureNode != NULL) {
			od_passthru_register_node(fConfigureNode->fNodeName, true);
		}
		
		if (fCacheNode != NULL) {
			od_passthru_register_node(fCacheNode->fNodeName, true);
		}
		
		gPlugins->RegisterPlugins();
	}

	catch( SInt32 err )
	{
	}

	fMutex.SignalLock();

} // RegisterAll


// ---------------------------------------------------------------------------
//	* Register ()
// ---------------------------------------------------------------------------

void CNodeList::Register ( sTreeNode *inTree )
{
	if ( inTree != nil )
	{
		if (inTree->fType == kDirNodeType) {
			od_passthru_register_node(inTree->fNodeName, false);
		}
		
		Register( inTree->left );
		Register( inTree->right );
	}
} // Register


// ---------------------------------------------------------------------------
//	* GetNodeCount ()
// ---------------------------------------------------------------------------

UInt32 CNodeList::GetNodeCount ( void )
{
	return( fCount );
} // GetNodeCount


// ---------------------------------------------------------------------------
//	* GetNodeChangeToken ()
// ---------------------------------------------------------------------------

UInt32 CNodeList:: GetNodeChangeToken ( void )
{
	return( fNodeChangeToken );
} // GetNodeChangeToken


// ---------------------------------------------------------------------------
//	* CountNodes ()
// ---------------------------------------------------------------------------

void CNodeList::CountNodes ( UInt32 *outCount )
{
	fMutex.WaitLock();

	try
	{
		this->Count( fTreePtr, outCount );
	}

	catch( SInt32 err )
	{
	}

	fMutex.SignalLock();

} // CountNodes


// ---------------------------------------------------------------------------
//	* Count ()
// ---------------------------------------------------------------------------


void CNodeList::Count ( sTreeNode *inTree, UInt32 *outCount )
{
	if ( inTree != nil )
	{
		Count( inTree->left, outCount );
		*outCount += 1;
		Count( inTree->right, outCount );
	}

} // Count


// ---------------------------------------------------------------------------
//	* GetNodes ()
// ---------------------------------------------------------------------------

SInt32 CNodeList::GetNodes ( char			   *inStr,
							 tDirPatternMatch	inMatch,
							 tDataBuffer	   *inBuff )
{
	SInt32			siResult	= eDSNoErr;
	sTreeNode	   *outNodePtr	= nil;

	// If the pre-defined nodes are not currently in the list, let's wait
	//	for a bit for them to show up.  Do not grab the list mutex so
	//	it is free for node registration
	if ( (inMatch == eDSLocalNodeNames) && (fLocalNode == nil) )
	{
#ifndef DISABLE_LOCAL_PLUGIN
		WaitForLocalNode();
#else
		return eDSUnknownNodeName;
#endif
	}
	if ( (inMatch == eDSCacheNodeName) && (fCacheNode == nil) )
	{
#ifndef DISABLE_CACHE_PLUGIN
		WaitForCacheNode();
#else
		return eDSUnknownNodeName;
#endif
	}
	if ( (inMatch == eDSAuthenticationSearchNodeName) && (fAuthenticationSearchNode == nil) )
	{
#ifndef DISABLE_SEARCH_PLUGIN
		if ( gDSLocalOnlyMode == true ) {
			return eDSUnknownNodeName;
		}
		WaitForLocalNode();
		WaitForBSDNode();
		WaitForAuthenticationSearchNode();
#else
		return eDSUnknownNodeName;
#endif
	}
	else if ( (inMatch == eDSContactsSearchNodeName) && (fContactsSearchNode == nil) )
	{
#ifndef DISABLE_SEARCH_PLUGIN
		if ( gDSLocalOnlyMode == true ) {
			return eDSUnknownNodeName;
		}
		WaitForLocalNode();
		WaitForBSDNode();
		WaitForContactsSearchNode();
#else
		return eDSUnknownNodeName;
#endif
	}
	else if ( (inMatch == eDSNetworkSearchNodeName) && (fNetworkSearchNode == nil) )
	{
#ifndef DISABLE_SEARCH_PLUGIN
		if ( gDSLocalOnlyMode == true ) {
			return eDSUnknownNodeName;
		}
		WaitForLocalNode(); //assumes that the local node is part of the networksearchnode path
		WaitForNetworkSearchNode();
#else
		return eDSUnknownNodeName;
#endif
	}
	else if ( (inMatch == eDSDefaultNetworkNodes) && (fNetworkSearchNode == nil) )
	{
#ifndef DISABLE_SEARCH_PLUGIN
		WaitForLocalNode(); //assumes that the local node is part of the networksearchnode path
#else
		return eDSUnknownNodeName;
#endif
	}
	else if ( (inMatch == eDSConfigNodeName) && (fConfigureNode == nil) )
	{
		WaitForConfigureNode();
	}

	fMutex.WaitLock();

	try
	{
		if ( inMatch == eDSAuthenticationSearchNodeName )
		{
			if ( fAuthenticationSearchNode != nil )
			{
				if ( fAuthenticationSearchNode->fDataListPtr != nil )
				{
					siResult = AddNodePathToTDataBuff( fAuthenticationSearchNode->fDataListPtr, inBuff );
				}
			}
		}
		else if ( inMatch == eDSContactsSearchNodeName )
		{
			if ( fContactsSearchNode != nil )
			{
				if ( fContactsSearchNode->fDataListPtr != nil )
				{
					siResult = AddNodePathToTDataBuff( fContactsSearchNode->fDataListPtr, inBuff );
				}
			}
		}
		else if ( inMatch == eDSNetworkSearchNodeName )
		{
			if ( fNetworkSearchNode != nil )
			{
				if ( fNetworkSearchNode->fDataListPtr != nil )
				{
					siResult = AddNodePathToTDataBuff( fNetworkSearchNode->fDataListPtr, inBuff );
				}
			}
		}
		else if ( inMatch == eDSConfigNodeName )
		{
			if ( fConfigureNode != nil )
			{
				if ( fConfigureNode->fDataListPtr != nil )
				{
					siResult = AddNodePathToTDataBuff( fConfigureNode->fDataListPtr, inBuff );
				}
			}
		}
		else if ( inMatch == eDSLocalNodeNames )
		{
			if ( fLocalNode != nil )
			{
				if ( fLocalNode->fDataListPtr != nil )
				{
					siResult = AddNodePathToTDataBuff( fLocalNode->fDataListPtr, inBuff );
				}
			}
		}
		else if ( inMatch == eDSCacheNodeName )
		{
			if ( fCacheNode != nil )
			{
				if ( fCacheNode->fDataListPtr != nil )
				{
					siResult = AddNodePathToTDataBuff( fCacheNode->fDataListPtr, inBuff );
				}
			}
		}
		else if ( inMatch == eDSLocalHostedNodes )
		{
			siResult = this->DoGetNode( fLocalHostedNodes, inStr, inMatch, inBuff, &outNodePtr );
		}
		else if ( inMatch == eDSDefaultNetworkNodes )
		{
			siResult = this->DoGetNode( fDefaultNetworkNodes, inStr, inMatch, inBuff, &outNodePtr );
		}
		else
		{
			siResult = this->DoGetNode( fTreePtr, inStr, inMatch, inBuff, &outNodePtr );
		}
	}

	catch( SInt32 err )
	{
	}

	fMutex.SignalLock();

	return( siResult );

} // GetNodes


// ---------------------------------------------------------------------------
//	* DoGetNode ()
// ---------------------------------------------------------------------------

SInt32 CNodeList::DoGetNode ( sTreeNode		   *inLeaf,
							 char			   *inStr,
							 tDirPatternMatch	inMatch,
							 tDataBuffer	   *inBuff,
							 sTreeNode		  **outNodePtr )
{
	char	   *aString1	= nil;
	char	   *aString2	= nil;
	bool		bAddToBuff	= false;
	SInt32		uiStrLen	= 0;
	SInt32		uiInStrLen	= 0;
	SInt32		siResult	= eDSNoErr;

	if ( inLeaf != nil )
	{
		siResult = DoGetNode( inLeaf->left, inStr, inMatch, inBuff, outNodePtr );

		switch( inMatch )
		{
			case eDSLocalNodeNames:
				if ( inLeaf->fType == kLocalNodeType )
				{
					bAddToBuff = true;
				}
				break;
				
			case eDSCacheNodeName:
				if ( inLeaf->fType == kCacheNodeType )
				{
					bAddToBuff = true;
				}
				break;
				
			case eDSAuthenticationSearchNodeName:
				if ( inLeaf->fType == kSearchNodeType )
				{
					bAddToBuff = true;
				}
				break;
				
			case eDSContactsSearchNodeName:
				if ( inLeaf->fType == kContactsSearchNodeType )
				{
					bAddToBuff = true;
				}
				break;
				
			case eDSNetworkSearchNodeName:
				if ( inLeaf->fType == kNetworkSearchNodeType )
				{
					bAddToBuff = true;
				}
				break;
				
			case eDSConfigNodeName:
				if ( inLeaf->fType == kConfigNodeType )
				{
					bAddToBuff = true;
				}
				break;

			case eDSLocalHostedNodes:
				if ( inLeaf->fType == kLocalHostedType )
				{
					bAddToBuff = true;
				}
				break;

			case eDSDefaultNetworkNodes:
				if ( inLeaf->fType == kDefaultNetworkNodeType )
				{
					bAddToBuff = true;
				}
				break;

			//KW is the following pattern matching UTF-8 capable?
			case eDSExact:
				if ( ::strcmp( inLeaf->fNodeName, inStr ) == 0 )
				{
					bAddToBuff = true;
				}
				break;

			case eDSStartsWith:
				uiInStrLen = ::strlen( inStr );
				if ( ::strncmp( inLeaf->fNodeName, inStr, uiInStrLen ) == 0 )
				{
					bAddToBuff = true;
				}
				break;

			case eDSEndsWith:
				uiInStrLen = ::strlen( inStr );
				if (uiInStrLen > 1) //means that there is something after the first delimiter passed in with the inStr
				{
					uiStrLen = ::strlen( inLeaf->fNodeName );
					if ( uiInStrLen <= uiStrLen )
					{
						aString1 = inLeaf->fNodeName + (uiStrLen - uiInStrLen + 1);
						aString2 = inStr + 1;
						if ( ::strcmp( aString1, aString2 ) == 0 )
						{
							bAddToBuff = true;
						}
					}
				}
				break;

			case eDSContains:
				uiInStrLen = ::strlen( inStr );
				if (uiInStrLen > 1) //means that there is something after the first delimiter passed in with the inStr
				{
					aString2 = inStr + 1;
					if ( ::strstr( inLeaf->fNodeName, aString2 ) != nil )
					{
						bAddToBuff = true;
					}
				}
				break;

			case eDSiExact:
				uiInStrLen = ::strlen( inStr );
				uiStrLen = ::strlen( inLeaf->fNodeName );
				if ( uiInStrLen == uiStrLen )
				{
					aString1 = inStr;
					aString2 = inLeaf->fNodeName;
					bAddToBuff = true;
					while ( *aString1 != '\0' )
					{
						if ( ::toupper( *aString2 ) != ::toupper( *aString1 ) )
						{
							bAddToBuff = false;
							break;
						}
						aString2++;
						aString1++;
					}
				}
				break;

			case eDSiStartsWith:
				uiInStrLen = ::strlen( inStr );
				uiStrLen = ::strlen( inLeaf->fNodeName );
				if ( uiInStrLen <= uiStrLen )
				{
					aString1 = inStr;
					aString2 = inLeaf->fNodeName;
					bAddToBuff = true;
					while ( *aString1 != '\0' )
					{
						if ( ::toupper( *aString2 ) != ::toupper( *aString1 ) )
						{
							bAddToBuff = false;
							break;
						}
						aString2++;
						aString1++;
					}
				}
				break;

			case eDSiEndsWith:
				uiInStrLen = ::strlen( inStr );
				if (uiInStrLen > 1) //means that there is something after the first delimiter passed in with the inStr
				{
					uiStrLen = ::strlen( inLeaf->fNodeName );
					if ( uiInStrLen <= uiStrLen )
					{
						aString1 = inStr + 1;
						aString2 = inLeaf->fNodeName + ( uiStrLen - uiInStrLen + 1 );
						bAddToBuff = true;
						while ( *aString1 != '\0' )
						{
							if ( ::toupper( *aString2 ) != ::toupper( *aString1 ) )
							{
								bAddToBuff = false;
								break;
							}
							aString2++;
							aString1++;
						}
					}
				}
				break;

			case eDSiContains:
				uiInStrLen = ::strlen( inStr );
				if (uiInStrLen > 1) //means that there is something after the first delimiter passed in with the inStr
				{
					uiStrLen = ::strlen( inLeaf->fNodeName );
					if ( uiInStrLen <= uiStrLen )
					{
						CString		tmpStr1( 128 );
						CString		tmpStr2( 128 );

						aString1 = inStr + 1;
						aString2 = inLeaf->fNodeName;
						bAddToBuff = false;

						while ( *aString1 != '\0' )
						{
							tmpStr1.Append( ::toupper( *aString1 ) );
							aString1++;
						}

						while ( *aString2 != '\0' )
						{
							tmpStr2.Append( ::toupper( *aString2 ) );
							aString2++;
						}

						if ( ::strstr( tmpStr2.GetData(), tmpStr1.GetData() ) != nil )
						{
							bAddToBuff = true;
						}
					}
				}
				break;

			default:
				break;
		}

		if ( bAddToBuff == true )
		{
			siResult = AddNodePathToTDataBuff( inLeaf->fDataListPtr, inBuff );
			*outNodePtr = inLeaf;
		}

		if ( siResult == eDSNoErr )
		{
			siResult = DoGetNode( inLeaf->right, inStr, inMatch, inBuff, outNodePtr );
		}
	}

	return( siResult );

} // DoGetNode


// ---------------------------------------------------------------------------
//	* DeleteNode ()
// ---------------------------------------------------------------------------

bool CNodeList::DeleteNode ( char *inStr )
{
	bool		found  		= false;

	fMutex.WaitLock();

	// kSearchNodeType | kContactsSearchNodeType | kNetworkSearchNodeType | kConfigNodeType | kLocalNodeType | kCacheNodeType
	// these nodes are not allowed to be removed at this time
	
	found = DeleteNodeFromTree( inStr, fLocalHostedNodes );
	
	found = DeleteNodeFromTree( inStr, fDefaultNetworkNodes ) || found;
	
	if (DeleteNodeFromTree( inStr, fTreePtr ))
	{
		found = true;
		fCount--;
		fNodeChangeToken++;
		
		if ( inStr != NULL )
		{
			CFStringRef		oldNodeRef = CFStringCreateWithCString( kCFAllocatorDefault, inStr, kCFStringEncodingUTF8 );
			
			if ( oldNodeRef != NULL )
			{
				SCDynamicStoreRef store = SCDynamicStoreCreate( kCFAllocatorDefault, NULL, NULL, NULL );
				if (store != NULL)
				{
					if ( SCDynamicStoreSetValue(store, CFSTR(kDSStdNotifyDirectoryNodeDeleted), oldNodeRef) == false ) {
						ErrLog( kLogApplication, "Could not set the DirectoryService:NotifyDirNodeDeleted in System Configuration" );
					}
					
					CFRelease( store );
					store = NULL;
				}
				else
				{
					ErrLog( kLogApplication, "CNodeList::DeleteNode - SCDynamicStoreCreate not yet available from System Configuration" );
				}
				
				CFRelease( oldNodeRef );
				oldNodeRef = NULL;
			}
			else
			{
				ErrLog( kLogApplication, "Could not notify that dir node: (%s) was deleted due to an encoding problem", inStr );
			}
		}
	}
	
	fMutex.SignalLock();

	return( found );

} // DeleteNode


// ---------------------------------------------------------------------------
//	* DeleteNodeFromTree ()
// ---------------------------------------------------------------------------

bool CNodeList::DeleteNodeFromTree ( char *inStr, sTreeNode  *inTree )
{
	bool			found  			= false;
	SInt32			siResult		= 0;
	sTreeNode	   *aTree			= inTree;
	sTreeNode	   *aTreeParent		= nil;
	sTreeNode	   *remTree			= nil;
	sTreeNode	   *remTreeParent	= nil;
	sTreeNode	   *remTreeChild	= nil;
	eDirNodeType	aDirNodeType	= kUnknownNodeType;

	fMutex.WaitLock();

	// looking only for the three node types that use trees
	if (aTree == fLocalHostedNodes)
	{
		aDirNodeType = kLocalHostedType;
	}
	else if (aTree == fDefaultNetworkNodes)
	{
		aDirNodeType = kDefaultNetworkNodeType;
	}
	else if (aTree == fTreePtr)
	{
		aDirNodeType = kDirNodeType;
	}
	else
	{
		aTree = nil;
	}

	//find the matching node
	while ( (!found) && (aTree != nil) )
	{
		siResult = CompareCString( inStr, aTree->fNodeName );
		if ( siResult == 0 )
		{
			found = true;
		}
		else
		{
			aTreeParent = aTree;
			if ( siResult < 0 )
			{
				aTree = aTree->left;
			}
			else
			{
				aTree = aTree->right;
			}
		}
	}

	//remove the matching node from the tree
	if ( found == true )
	{
		if ( aTree->left == nil )
		{
			remTree = aTree->right;
		}
		else
		{
			if ( aTree->right == nil )
			{
				remTree = aTree->left;
			}
			else
			{
				remTreeParent = aTree;
				remTree = aTree->right;
				remTreeChild = remTree->left;
				while ( remTreeChild != nil )
				{
					remTreeParent = remTree;
					remTree = remTreeChild;
					remTreeChild = remTree->left;
				}
				if ( remTreeParent != aTree )
				{
					remTreeParent->left = remTree->right;
					remTree->right = aTree->right;
				}
				remTree->left = aTree->left;
			}
		}

		if ( aTreeParent == nil )
		{
			if (aDirNodeType == kLocalHostedType)
			{
				fLocalHostedNodes = remTree;
			}
			else if (aDirNodeType == kDefaultNetworkNodeType)
			{
				fDefaultNetworkNodes = remTree;
			}
			else if (aDirNodeType == kDirNodeType)
			{
				fTreePtr = remTree;
			}
		}
		else
		{
			if ( aTree == aTreeParent->left )
			{
				aTreeParent->left = remTree;
			}
			else
			{
				aTreeParent->right = remTree;
			}
		}
		
		if ( aTree->fNodeName != nil )
		{
			od_passthru_unregister_node(aTree->fNodeName);
			
			free( aTree->fNodeName );
			aTree->fNodeName = nil;
		}

		if ( aTree->fDataListPtr != nil )
		{
			::dsDataListDeallocatePriv( aTree->fDataListPtr );
			//need to free the header as well
			free ( aTree->fDataListPtr );
			aTree->fDataListPtr = nil;
		}

		delete( aTree );
	}
	
	fMutex.SignalLock();

	return( found );

} // DeleteNodeFromTree


// ---------------------------------------------------------------------------
//	* IsPresent ()
// ---------------------------------------------------------------------------

bool CNodeList::IsPresent ( const char *inStr, eDirNodeType inType )
{
	bool		found		= false;
	SInt32		siResult	= 0;
	sTreeNode  *current		= nil;

	fMutex.WaitLock();

	if	( inType & ( kSearchNodeType | kContactsSearchNodeType | kNetworkSearchNodeType | kConfigNodeType | kLocalNodeType | kCacheNodeType | kBSDNodeType) )
	{
		//these nodes are not allowed to be compared to at this time
		//so we always return false - actually possible mis-information
		//so the actual AddNode call will need to decide whether to add or not
		fMutex.SignalLock();
		return false;
	}
	else if (inType == kLocalHostedType)
	{
		current = fLocalHostedNodes;
	}
	else if (inType == kDefaultNetworkNodeType)
	{
		current = fDefaultNetworkNodes;
	}
	else //this will be the simple node type
	{
		current = fTreePtr;
	}
	
	while ( (!found) && (current != nil) )
	{
		siResult = CompareCString( inStr, current->fNodeName );
		if ( siResult == 0 )
		{
			found = true;
		}
		else
		{
			if ( siResult < 0 )
			{
				current = current->left;
			}
			else
			{
				current = current->right;
			}
		}
	}

	fMutex.SignalLock();

	return( found );

} // IsPresent 


// ---------------------------------------------------------------------------
//	* GetPluginHandle ()
// ---------------------------------------------------------------------------

bool CNodeList::GetPluginHandle ( const char *inStr, CServerPlugin **outPlugInPtr )
{
	bool		found		= false;
	SInt32		siResult	= 0;
	sTreeNode  *current		= nil;

	fMutex.WaitLock();

	//check the special nodes in anticipated order of frequency of request
	if ( fLocalNode != nil)
	{
		if (fLocalNode->fNodeName != nil)
		{
			if (strcmp(inStr,fLocalNode->fNodeName) == 0)
			{
				if ( outPlugInPtr != nil )
				{
					*outPlugInPtr = GetPluginPtr(fLocalNode);
				}
				fMutex.SignalLock();
				return( true );
			}
		}
	}
	
	//check the special nodes in anticipated order of frequency of request
	if ( fCacheNode != nil)
	{
		if (fCacheNode->fNodeName != nil)
		{
			if (strcmp(inStr,fCacheNode->fNodeName) == 0)
			{
				if ( outPlugInPtr != nil )
				{
					*outPlugInPtr = GetPluginPtr(fCacheNode);
				}
				fMutex.SignalLock();
				return( true );
			}
		}
	}
	
	if (strlen(inStr) > 6 )  //compare to search node prefix if string long enough
	{
		if ( strncmp(inStr+1,"Search",6) == 0 )  //this is one of the search nodes
		{
			if ( fAuthenticationSearchNode != nil)
			{
				if (fAuthenticationSearchNode->fNodeName != nil)
				{
					if (strcmp(inStr,fAuthenticationSearchNode->fNodeName) == 0)
					{
						if ( outPlugInPtr != nil )
						{
							*outPlugInPtr = GetPluginPtr(fAuthenticationSearchNode);
						}
						fMutex.SignalLock();
						return( true );
					}
				}
			}
			if ( fNetworkSearchNode != nil)
			{
				if (fNetworkSearchNode->fNodeName != nil)
				{
					if (strcmp(inStr,fNetworkSearchNode->fNodeName) == 0)
					{
						if ( outPlugInPtr != nil )
						{
							*outPlugInPtr = GetPluginPtr(fNetworkSearchNode);
						}
						fMutex.SignalLock();
						return( true );
					}
				}
			}
		
			if ( fContactsSearchNode != nil)
			{
				if (fContactsSearchNode->fNodeName != nil)
				{
					if (strcmp(inStr,fContactsSearchNode->fNodeName) == 0)
					{
						if ( outPlugInPtr != nil )
						{
							*outPlugInPtr = GetPluginPtr(fContactsSearchNode);
						}
						fMutex.SignalLock();
						return( true );
					}
				}
			}
		}
	}

	//assumption here is that both the DefaultNetworkNodes and the LocalHostedNodes are also in the main node tree
	//KW why do we keep duplicates across different node types?
	current = fTreePtr;
	while ( (!found) && (current != nil) )
	{
		siResult = CompareCString( inStr, current->fNodeName );
		if ( siResult == 0 )
		{
			found = true;
			if ( outPlugInPtr != nil )
			{
				*outPlugInPtr = GetPluginPtr(current);
			}
		}
		else
		{
			if ( siResult < 0 )
			{
				current = current->left;
			}
			else
			{
				current = current->right;
			}
		}
	}

	if ( fConfigureNode != nil)
	{
		if (fConfigureNode->fNodeName != nil)
		{
			if (strcmp(inStr,fConfigureNode->fNodeName) == 0)
			{
				found = true;
				if ( outPlugInPtr != nil )
				{
					*outPlugInPtr = GetPluginPtr(fConfigureNode);
				}
			}
		}
	}

	fMutex.SignalLock();

	return( found );

} // GetPluginHandle 

CServerPlugin* CNodeList::GetPluginPtr( sTreeNode* nodePtr )
{
	if ( nodePtr->fPlugInPtr == nil )
	{
		nodePtr->fPlugInPtr = gPlugins->GetPlugInPtr( nodePtr->fPlugInToken, true );
	}
	
	return nodePtr->fPlugInPtr;
} // GetPluginPtr

// ---------------------------------------------------------------------------
//	* CompareCString ()
// ---------------------------------------------------------------------------

SInt32 CNodeList::CompareCString ( const char *inStr_1, const char *inStr_2 )
{
	volatile SInt32	siResult = -22;

	if ( (inStr_1 != nil) && (inStr_2 != nil) )
	{
		siResult = ::strcmp( inStr_1, inStr_2 );
	}

	return( siResult );

} // CompareCString


// ---------------------------------------------------------------------------
//	* BuildNodeListBuff ()
// ---------------------------------------------------------------------------

SInt32 CNodeList::BuildNodeListBuff ( sGetDirNodeList *inData )
{
	SInt32			siResult	= eDSNoErr;
	UInt32			outCount	= 0;

	fMutex.WaitLock();

	inData->fIOContinueData = nil;

	siResult = this->DoBuildNodeListBuff( fTreePtr, inData->fOutDataBuff, &outCount );
	inData->fOutNodeCount = outCount;

	fMutex.SignalLock();

	return( siResult );

} // BuildNodeListBuff


// ---------------------------------------------------------------------------
//	* DoBuildNodeListBuff ()
// ---------------------------------------------------------------------------

SInt32 CNodeList::DoBuildNodeListBuff ( sTreeNode *inTree, tDataBuffer *inBuff, UInt32 *outCount )
{
	SInt32			siResult	= eDSNoErr;
	tDataList	   *pNodeList	= nil;

	if ( inTree != nil )
	{
		siResult = DoBuildNodeListBuff( inTree->left, inBuff, outCount );
		if ( siResult == eDSNoErr )
		{
			pNodeList = inTree->fDataListPtr;
			if ( pNodeList != nil )
			{
				siResult = AddNodePathToTDataBuff( pNodeList, inBuff );
				if ( siResult == eDSNoErr )
				{
					*outCount	+= 1;
					siResult	= DoBuildNodeListBuff( inTree->right, inBuff, outCount );
				}
				else
				{
					*outCount = 0;
				}
			}
		}
	}

	return( siResult );

} // DoBuildNodeListBuff


// ---------------------------------------------------------------------------
//	* AddNodePathToTDataBuff ()
// ---------------------------------------------------------------------------

SInt32 CNodeList::AddNodePathToTDataBuff ( tDataList *inPtr, tDataBuffer *inBuff )
{
	SInt32				siResult	= eDSBufferTooSmall;
	FourCharCode		uiBuffType	= 'npss'; // node path strings
	UInt32				inBuffType	= 'xxxx';
	char			   *segmentStr	= nil;
	UInt16				uiStrLen	= 0;
	UInt32				uiItemCnt	= 0;
	UInt16				segmentCnt	= 0;
	UInt32				pathLength	= 0;
	UInt32				iSegment	= 0;
	UInt32				offset		= 0;

	if ( (inPtr != nil) && (inBuff != nil) )
	{
	//buffer format ie. only used by FW in dsGetDirNodeName
	//4 byte tag
	//4 byte count of node paths
	//repeated:
	// 2 byte count of string segments - ttt
	// sub repeated:
	// 2 byte segment string length - ttt
	// actual segment string - ttt
	// ..... blank space
	// offsets for each node path in reverse order from end of buffer
	// note that buffer length is length of data aboved tagged with ttt
		if ( inBuff->fBufferSize > 7 )
		{
			//get the current buff type
			::memcpy( &inBuffType, inBuff->fBufferData,  4 );
			
			if (inBuffType != uiBuffType) //new buffer
			{
				// set buffer type tag
				::memcpy( inBuff->fBufferData, &uiBuffType, 4 );
				//ensure length is zero
				inBuff->fBufferLength = 0;
			}
			else
			{
				//get the current count
				::memcpy( &uiItemCnt, inBuff->fBufferData + 4,  4 );
			}
			
			//retrieve number of segments in node path
			segmentCnt	= (UInt16) dsDataListGetNodeCountPriv( inPtr );
			//retrieve the segment's overall length
			pathLength	= dsGetDataLengthPriv ( inPtr );
			//adjust the pathLength for each segment string length
			//and overall segment count stored each in 2 bytes
			pathLength	+= segmentCnt * 2 + 2;
			
			//check that buffer will not overflow with this addition
			//ie. buffer length plus new length plus 8 header bytes needs to be
			//less than the buffer size minus the storage for the entry offsets
			if ( ( inBuff->fBufferLength + pathLength + 8) <= (inBuff->fBufferSize - ((uiItemCnt+1) * 4)) )
			{
				//increment the count of node paths to be in the buffer
				uiItemCnt++;
				::memcpy( inBuff->fBufferData + 4, &uiItemCnt, 4 );
				
				//set the offset for this node path
				offset = inBuff->fBufferLength + 8; //shift past header of data
				::memcpy( inBuff->fBufferData + inBuff->fBufferSize - (uiItemCnt * 4) , &(offset), 4 );
				
				//set the number of segments for this node path
				::memcpy( inBuff->fBufferData + 8 + inBuff->fBufferLength, &segmentCnt, 2 );
				
				//add to the data length the segment count 2 bytes
				inBuff->fBufferLength += 2;
				
				for (iSegment = 1; iSegment <= segmentCnt; iSegment++ )
				{
					segmentStr = dsDataListGetNodeStringPriv( inPtr, iSegment );
					if (segmentStr != nil);
					{
						uiStrLen = strlen(segmentStr);
						::memcpy( inBuff->fBufferData + 8 + inBuff->fBufferLength, &uiStrLen, 2 );
						::memcpy( inBuff->fBufferData + 8 + inBuff->fBufferLength + 2, segmentStr, uiStrLen );
						
						free(segmentStr);
						segmentStr = nil;
						
						//update the used buffer length
						inBuff->fBufferLength += 2 + uiStrLen;
					}
				}

				siResult = eDSNoErr;
			}
			else
			{
				// Buffer is full but we return that it is too small since
				// we want to return all results in a single buffer
				siResult = eDSBufferTooSmall;
			}
		}
	}

	return( siResult );

} // AddNodePathToTDataBuff


// ---------------------------------------------------------------------------
//	* WaitForAuthenticationSearchNode ()
// ---------------------------------------------------------------------------

void CNodeList::WaitForAuthenticationSearchNode( void )
{
#ifndef DISABLE_SEARCH_PLUGIN
	fWaitForAuthenticationSN.WaitForEvent();
	gKickSearchRequests.WaitForEvent();
#endif
} // WaitForAuthenticationSearchNode


// ---------------------------------------------------------------------------
//	* WaitForContactsSearchNode ()
// ---------------------------------------------------------------------------

void CNodeList:: WaitForContactsSearchNode ( void )
{
#ifndef DISABLE_SEARCH_PLUGIN
	fWaitForContactsSN.WaitForEvent();
	gKickSearchRequests.WaitForEvent();
#endif
} // WaitForContactsSearchNode


// ---------------------------------------------------------------------------
//	* WaitForNetworkSearchNode ()
// ---------------------------------------------------------------------------

void CNodeList:: WaitForNetworkSearchNode ( void )
{
#ifndef DISABLE_SEARCH_PLUGIN
	fWaitForNetworkSN.WaitForEvent();
	gKickSearchRequests.WaitForEvent();
#endif
} // WaitForNetworkSearchNode


// ---------------------------------------------------------------------------
//	* WaitForLocalNode ()
// ---------------------------------------------------------------------------

void CNodeList::WaitForLocalNode( void )
{
#ifndef DISABLE_LOCAL_PLUGIN
	fWaitForLN.WaitForEvent();
	gKickLocalNodeRequests.WaitForEvent();
#endif
} // WaitForLocalNode


// ---------------------------------------------------------------------------
//	* WaitForCacheNode ()
// ---------------------------------------------------------------------------

void CNodeList::WaitForCacheNode( void )
{
#ifndef DISABLE_CACHE_PLUGIN
	fWaitForCacheN.WaitForEvent();
	gKickCacheRequests.WaitForEvent();
#endif
} // WaitForCacheNode

// ---------------------------------------------------------------------------
//	* WaitForBSDNode ()
// ---------------------------------------------------------------------------

void CNodeList::WaitForBSDNode( void )
{
#ifndef DISABLE_BSD_PLUGIN
	fWaitForBSDN.WaitForEvent();
	gKickBSDRequests.WaitForEvent();
#endif
} // WaitForCacheNode


// ---------------------------------------------------------------------------
//	* WaitForConfigureNode ()
// ---------------------------------------------------------------------------

void CNodeList::WaitForConfigureNode( void )
{
#ifndef DISABLE_CONFIGURE_PLUGIN
	fWaitForConfigureN.WaitForEvent();
	gKickConfigRequests.WaitForEvent();
#endif
} // WaitForConfigureNode
