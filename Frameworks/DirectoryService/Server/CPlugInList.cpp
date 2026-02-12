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
 * @header CPlugInList
 */

#include "CPlugInList.h"
#include "CServerPlugin.h"
#include "CLauncher.h"
#include "DSUtils.h"
#include "PrivateTypes.h"
#include "SharedConsts.h"
#include "CPluginConfig.h"
#include "CLog.h"
#include "CNodeList.h"
#include "od_passthru.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

extern CFRunLoopRef			gPluginRunLoop;
extern DSMutexSemaphore    *gKerberosMutex;
#ifndef DISABLE_CONFIGURE_PLUGIN
extern CPluginConfig	   *gPluginConfig;
#endif
extern DSMutexSemaphore	   *gLazyPluginLoadingLock;
extern CPlugInList		   *gPlugins;
extern CNodeList		   *gNodeList;

UInt32 gMaxLazyLoaders = 20;
char* gLazyLoadAttemptedList[20] = {nil};

//Use of the table entries directly will need a Mutex if the order can change
//but likely will continue only to add on to the end of the table and
//not delete table entries so it is currently a non issue

// ---------------------------------------------------------------------------
//	* CPlugInList ()
//
// ---------------------------------------------------------------------------

CPlugInList::CPlugInList ( void ) : fMutex("CPlugInList::fMutex")
{
	fPICount	= 0;
	fTable		= nil;
	fTableTail  = nil;
	fCFRecordTypeRestrictions = NULL;

} // CPlugInList


// ---------------------------------------------------------------------------
//	* ~CPlugInList ()
//
// ---------------------------------------------------------------------------

CPlugInList::~CPlugInList ( void )
{
} // ~CPlugInList


// ---------------------------------------------------------------------------
//	* AddPlugIn ()
//
// ---------------------------------------------------------------------------

SInt32 CPlugInList::AddPlugIn ( const char		*inName,
								const char		*inVersion,
								const char		*inConfigAvail,
								const char		*inConfigFile,
								eDSPluginLevel	 inLevel,
								FourCharCode	 inKey,
								CServerPlugin	*inPluginPtr,
								CFPlugInRef		 inPluginRef,
								CFUUIDRef		 inCFuuidFactory,
								UInt32			 inULVers )
{
	SInt32			siResult	= eDSInvalidPlugInConfigData;
	sTableData     *aTableEntry = nil;

	if ( inName == nil )
	{
		return( eDSNullParameter );
	}
	
#ifndef DISABLE_CONFIGURE_PLUGIN
	// ask the plugin about it state outside the lock to avoid deadlock.
	ePluginState pluginState = gPluginConfig->GetPluginState( inName );
#else
	ePluginState pluginState = kActive;
#endif

	fMutex.WaitLock();

	try
	{
		aTableEntry = (sTableData *)calloc(1, sizeof(sTableData));
		if (fTableTail != nil)
		{
			fTableTail->pNext   = aTableEntry;
			fTableTail			= aTableEntry;
		}
		else
		{
			fTable = aTableEntry;
			fTableTail = aTableEntry;
		}
		fTableTail->pNext			= nil;
		fTableTail->fName			= inName;
		fTableTail->fVersion		= inVersion;
		fTableTail->fConfigAvail	= inConfigAvail;
		fTableTail->fConfigFile		= inConfigFile;
		fTableTail->fPluginPtr		= inPluginPtr;
		fTableTail->fValidDataStamp	= 0;
		fTableTail->fLevel			= inLevel;
		
		if ( inPluginRef )
		{
			fTableTail->fPluginRef = inPluginRef;
			CFRetain( fTableTail->fPluginRef );
		}
		
		if ( inCFuuidFactory )
		{
			fTableTail->fCFuuidFactory = inCFuuidFactory;
			CFRetain( fTableTail->fCFuuidFactory );
		}
		
		if ( inULVers )
			fTableTail->fULVers	= inULVers;
			
		fTableTail->fKey = inKey;
		
		fTableTail->fState = pluginState | kUninitialized;

		fPICount++;

		siResult = eDSNoErr;
	}

	catch( SInt32 err )
	{
		siResult = err;
	}

	fMutex.SignalLock();

	return( siResult );

} // AddPlugIn


void CPlugInList::LoadPlugin( sTableData *inTableEntry )
{
	bool			done		= false;
	SInt32			siResult 	= eDSNoErr;
	UInt32			uiCntr		= 0;
	UInt32			uiAttempts	= 100;
	UInt32			uiWaitTime 	= 1;
	sHeader			aHeader;
	CServerPlugin  *ourPluginPtr= nil;
	UInt32			listCount	= 0;
	
	gLazyPluginLoadingLock->WaitLock();

	// check the list of already attempted lazy loads before continuing ie. eliminate race possibility
	while( (gLazyLoadAttemptedList[listCount] != nil) && (listCount < gMaxLazyLoaders) )
	{
		if (strcmp(gLazyLoadAttemptedList[listCount],inTableEntry->fName) == 0)
		{
			gLazyPluginLoadingLock->SignalLock();
			return;
		}
		listCount++;
	}
	if (listCount != gMaxLazyLoaders)
	{
		// add to the list since we attempt to load this one
		gLazyLoadAttemptedList[listCount] = strdup(inTableEntry->fName);
	}

	try
	{
		if ( inTableEntry->fPluginPtr == nil)
		{
			inTableEntry->fPluginPtr = new CServerPlugin( inTableEntry->fPluginRef, inTableEntry->fCFuuidFactory, inTableEntry->fKey, inTableEntry->fULVers, inTableEntry->fName );
			
			ourPluginPtr = (CServerPlugin *)inTableEntry->fPluginPtr;

			if ( ourPluginPtr == NULL ) throw( (SInt32)eMemoryError );
			
			ourPluginPtr->Validate( inTableEntry->fVersion, inTableEntry->fKey );
			
			while ( !done )
			{
				uiCntr++;
		
				// Attempt to initialize it
				siResult = ourPluginPtr->Initialize();
				if ( ( siResult != eDSNoErr ) && ( uiCntr == 1 ) )
				{
					ErrLog( kLogApplication, "Attempt #%l to initialize plug-in %s failed.\n  Will retry initialization at most 100 times every %l second.", uiCntr, ourPluginPtr->GetPluginName(), uiWaitTime );
				}
					
				if ( siResult == eDSNoErr )
				{
					DbgLog( kLogApplication, "Initialization of plug-in %s succeeded with #%l attempt.", ourPluginPtr->GetPluginName(), uiCntr );
		
					// we start initialized but inactive, we set our active flag later
					inTableEntry->fState = kInitialized | kInactive;
		
					//provide the CFRunLoop to the plugins that need it
					if (gPluginRunLoop != NULL)
					{
						aHeader.fType			= kServerRunLoop;
						aHeader.fResult			= eDSNoErr;
						aHeader.fContextData	= (void *)gPluginRunLoop;
						siResult = ourPluginPtr->ProcessRequest( (void*)&aHeader ); //don't handle return
					}
		
					// provide the Kerberos Mutex to plugins that need it
					if (gKerberosMutex != NULL)
					{
						aHeader.fType			= kKerberosMutex;
						aHeader.fResult			= eDSNoErr;
						aHeader.fContextData	= (void *)gKerberosMutex;
						ourPluginPtr->ProcessRequest( (void*)&aHeader ); // don't handle return
					}
						
					done = true;
				}
		
				if ( !done )
				{
					// We will try this 100 times before we bail
					if ( uiCntr == uiAttempts )
					{
						ErrLog( kLogApplication, "%l attempts to initialize plug-in %s failed.\n  Setting plug-in state to inactive.", uiCntr, ourPluginPtr->GetPluginName() );
	
						inTableEntry->fState = kInactive | kFailedToInit;
						done = true;
					}
					else
					{
						fWaitToInit.WaitForEvent( uiWaitTime * kMilliSecsPerSec );
					}
				}
			}

			SrvrLog( kLogApplication, "Plugin \"%s\", Version \"%s\", loaded on demand successfully.", inTableEntry->fName, inTableEntry->fVersion );
		}
	}

	catch( SInt32 err )
	{
		SrvrLog( kLogApplication, "Plugin \"%s\", Version \"%s\", failed to load on demand (%d).", inTableEntry->fName, inTableEntry->fVersion, err );
	}
	
	gLazyPluginLoadingLock->SignalLock();
	
}


// ---------------------------------------------------------------------------
//	* InitPlugIns ()
//
// ---------------------------------------------------------------------------

void CPlugInList::InitPlugIns ( eDSPluginLevel inLevel )
{
	sTableData     *aTableEntry = nil;

	fMutex.WaitLock();

	aTableEntry = fTable;
	while ( aTableEntry != nil )
	{
		if ( ( aTableEntry->fLevel == inLevel) && ( ( aTableEntry->fState & kUninitialized ) || ( aTableEntry->fState == 0 ) ) )
		{
			if ( (aTableEntry->fName != nil) && (aTableEntry->fPluginPtr != nil) )
			{
				try
				{
					//this constructor could throw
					CLauncher *cpLaunch = new CLauncher( (CServerPlugin *)aTableEntry->fPluginPtr );
					if ( cpLaunch != nil )
					{
						//this call could throw
						cpLaunch->StartThread();
					}
					DbgLog( kLogApplication, "Plugin \"%s\", Version \"%s\", activated successfully.", aTableEntry->fName, aTableEntry->fVersion );
				}
			
				catch( SInt32 err )
				{
					DbgLog( kLogApplication, "Plugin \"%s\", Version \"%s\", failed to launch initialization thread.", aTableEntry->fName, aTableEntry->fVersion );
				}
			}
			else if ( aTableEntry->fName != nil )
			{
				// if this plugin is supposed to be active, we should mark it as such, it still should be uninitialized.
#ifndef DISABLE_CONFIGURE_PLUGIN
				ePluginState		pluginState = gPluginConfig->GetPluginState( aTableEntry->fName );
#else
				ePluginState		pluginState = kActive;
#endif

				aTableEntry->fState = pluginState | kUninitialized;
				DbgLog( kLogApplication, "Plugin \"%s\", Version \"%s\", referenced to be loaded on demand successfully.", aTableEntry->fName, aTableEntry->fVersion );
			}
		}
		aTableEntry = aTableEntry->pNext;
	}

	fMutex.SignalLock();

} // InitPlugIns


// ---------------------------------------------------------------------------
//	* IsPresent ()
//
// ---------------------------------------------------------------------------

SInt32 CPlugInList::IsPresent ( const char *inName )
{
	SInt32			siResult	= ePluginNameNotFound;
	sTableData     *aTableEntry = nil;

	if ( inName == nil )
	{
		return( eDSNullParameter );
	}
	
	fMutex.WaitLock();

	aTableEntry = fTable;
	while ( aTableEntry != nil )
	{
		if ( aTableEntry->fName != nil )
		{
			if ( ::strcmp( aTableEntry->fName, inName ) == 0 )
			{
				siResult = eDSNoErr;

				break;
			}
		}
		aTableEntry = aTableEntry->pNext;
	}

	fMutex.SignalLock();

	return( siResult );

} // IsPresent


// ---------------------------------------------------------------------------
//	* SetState ()
//
// ---------------------------------------------------------------------------

SInt32 CPlugInList::SetState ( const char *inName, const UInt32 inState )
{
	SInt32			siResult		= ePluginNameNotFound;
	sTableData     *aTableEntry		= nil;
	sTableData     *tmpTableEntry	= nil;
	UInt32			curState		= kUnknownState;
	sTableData	   *pluginEntry		= NULL;

	if ( inName == nil )
	{
		return( eDSNullParameter );
	}
	
	fMutex.WaitLock();

	aTableEntry = fTable;
	while ( aTableEntry != nil )
	{
		if ( aTableEntry->fName != nil )
		{
			if ( ::strcmp( aTableEntry->fName, inName ) == 0 )
			{
				curState = aTableEntry->fState;
				
				// this means we will try to load the plugin below
				if ( (inState & kActive) && aTableEntry->fPluginPtr == NULL )
					tmpTableEntry = MakeTableEntryCopy( aTableEntry );

				pluginEntry = aTableEntry;
				break;
			}
		}
		aTableEntry = aTableEntry->pNext;
	}

	fMutex.SignalLock();
	
	if (tmpTableEntry != nil)
	{
		// This plugin has just been set to active but we haven't loaded it yet.
		// should NOT hold the table mutex for this
		gNodeList->Lock();
		LoadPlugin( tmpTableEntry );
		gNodeList->Unlock();

		if (tmpTableEntry->fPluginPtr != NULL)
		{
			// we actually loaded the plugin so go ahead and update the table
			fMutex.WaitLock();
			
			// now use the tmpTableEntry pluginPtr
			if ( pluginEntry->fPluginPtr == NULL )
				pluginEntry->fPluginPtr = tmpTableEntry->fPluginPtr;

			fMutex.SignalLock();
		}
		DSFree(tmpTableEntry);
	}
	
	// so we can call SetPluginState without the mutex being locked to avoid deadlock.
	CServerPlugin* pluginPtr = NULL;

	fMutex.WaitLock();

	if ( (curState & inState) != inState && pluginEntry != NULL && pluginEntry->fPluginPtr != NULL )
	{
		pluginEntry->fState = inState;

		// this will trigger a call to SetPluginState below
		pluginPtr = pluginEntry->fPluginPtr;
		
		siResult = eDSNoErr;
	}	
	
	fMutex.SignalLock();
	
	if ( pluginPtr )
	{
		pluginPtr->SetPluginState( inState );
	}

	return( siResult );

} // SetState


// ---------------------------------------------------------------------------
//	* GetState ()
//
// ---------------------------------------------------------------------------

SInt32 CPlugInList::GetState ( const char *inName, UInt32 *outState )
{
	SInt32			siResult		= ePluginNameNotFound;
	sTableData     *aTableEntry		= nil;

	if ( inName == nil )
	{
		return( eDSNullParameter );
	}
	
	fMutex.WaitLock();

	aTableEntry = fTable;
	while ( aTableEntry != nil )
	{
		if ( aTableEntry->fName != nil )
		{
			if ( ::strcmp( aTableEntry->fName, inName ) == 0 )
			{
				*outState = aTableEntry->fState;

				siResult = eDSNoErr;

				break;
			}
		}
		aTableEntry = aTableEntry->pNext;
	}

	fMutex.SignalLock();

	return( siResult );

} // GetState


// ---------------------------------------------------------------------------
//	* UpdateValidDataStamp ()
//
// ---------------------------------------------------------------------------

SInt32 CPlugInList::UpdateValidDataStamp ( const char *inName )
{
	SInt32			siResult		= ePluginNameNotFound;
	sTableData     *aTableEntry		= nil;

	if ( inName == nil )
	{
		return( eDSNullParameter );
	}
	
	fMutex.WaitLock();

	aTableEntry = fTable;
	while ( aTableEntry != nil )
	{
		if ( aTableEntry->fName != nil )
		{
			if ( ::strcmp( aTableEntry->fName, inName ) == 0 )
			{
				aTableEntry->fValidDataStamp++;

				siResult = eDSNoErr;

				break;
			}
		}
		aTableEntry = aTableEntry->pNext;
	}

	fMutex.SignalLock();

	return( siResult );

} // UpdateValidDataStamp


// ---------------------------------------------------------------------------
//	* GetValidDataStamp ()
//
// ---------------------------------------------------------------------------

UInt32 CPlugInList::GetValidDataStamp ( const char *inName )
{
	UInt32			outStamp		= 0;
	sTableData     *aTableEntry		= nil;

	if ( inName == nil )
	{
		return( eDSNullParameter );
	}
	
	fMutex.WaitLock();

	aTableEntry = fTable;
	while ( aTableEntry != nil )
	{
		if ( aTableEntry->fName != nil )
		{
			if ( ::strcmp( aTableEntry->fName, inName ) == 0 )
			{
				outStamp = aTableEntry->fValidDataStamp;

				break;
			}
		}
		aTableEntry = aTableEntry->pNext;
	}

	fMutex.SignalLock();

	return( outStamp );

} // GetValidDataStamp


// ---------------------------------------------------------------------------
//	* GetPlugInCount ()
//
// ---------------------------------------------------------------------------

UInt32 CPlugInList::GetPlugInCount ( void )
{
	return( fPICount );
} // GetPlugInCount

void CPlugInList::RegisterPlugins(void)
{
	sTableData     *aTableEntry		= nil;
	
	fMutex.WaitLock();
	
	aTableEntry = fTable;
	while ( aTableEntry != nil )
	{
		if (aTableEntry->fName != NULL) {
			char name[512];
			
			strlcpy(name, "/", sizeof(name));
			strlcat(name, aTableEntry->fName, sizeof(name));
			
			// hidden by default because top-level nodes are not normal nodes when a list of nodes is requested
			od_passthru_register_node(name, true);
		}
		
		aTableEntry = aTableEntry->pNext;
	}
	
	fMutex.SignalLock();
}

// ---------------------------------------------------------------------------
//	* GetActiveCount ()
//
// ---------------------------------------------------------------------------

UInt32 CPlugInList::GetActiveCount ( void )
{
	UInt32			activeCount		= 0;
	sTableData     *aTableEntry		= nil;

	fMutex.WaitLock();

	aTableEntry = fTable;
	while ( aTableEntry != nil )
	{
		if ( aTableEntry->fName == nil )
		{
			if ( aTableEntry->fState & kActive )
			{
				activeCount++;
			}
		}
		aTableEntry = aTableEntry->pNext;
	}

	fMutex.SignalLock();

	return( activeCount );

} // GetActiveCount

// ---------------------------------------------------------------------------
//	* SetPluginState ()
//
// ---------------------------------------------------------------------------

void CPlugInList::SetPluginState( CServerPlugin	*inPluginPtr, ePluginState inPluginState )
{
	if ( inPluginPtr == NULL )
		return;

	SInt32	siResult	= inPluginPtr->SetPluginState( inPluginState );
	const char *pluginName = inPluginPtr->GetPluginName();
	od_passthru_set_plugin_enabled(pluginName, (inPluginState & kActive) != 0);

	if ( siResult == eDSNoErr ) {
		SrvrLog( kLogApplication, "Plug-in %s state is now %s.", pluginName, (inPluginState == kActive ? "active" : "inactive") );
	}
	else {
		SrvrLog( kLogApplication, "Unable to set %s plug-in state to %s.  Received error %l.", pluginName, (inPluginState == kActive ? "active" : "inactive"),
				siResult );
	}
}


// ---------------------------------------------------------------------------
//	* GetPlugInPtr ()
//
// ---------------------------------------------------------------------------

CServerPlugin* CPlugInList::GetPlugInPtr ( const char *inName, bool loadIfNeeded )
{
	CServerPlugin  *pResult			= nil;
	sTableData     *aTableEntry		= nil;
	sTableData     *tmpTableEntry	= nil;
	ePluginState	newState		= kUnknownState;

	if ( inName == nil )
	{
		return( nil );
	}
	
	fMutex.WaitLock();

	aTableEntry = fTable;
	while ( aTableEntry != nil )
	{
		if ( (aTableEntry->fName != nil) )
		{
			if ( ::strcmp( aTableEntry->fName, inName ) == 0 )
			{
				// if someone specifically asks for a node, even if not active, we should load the plugin
				// this can be for configure nodes, otherwise inactive nodes cannot be configured
				if ( (aTableEntry->fPluginPtr == NULL) && loadIfNeeded )
				{
					// this means we will try to load the plugin below
					tmpTableEntry = MakeTableEntryCopy( aTableEntry );
					break;
				}	

				pResult = aTableEntry->fPluginPtr;

				break;
			}
		}
		aTableEntry = aTableEntry->pNext;
	}

	fMutex.SignalLock();

	if (tmpTableEntry != nil)
	{
		// This plugin has just been set to active but we haven't loaded it yet.
		// should NOT hold the table mutex for this
		gNodeList->Lock();
		LoadPlugin( tmpTableEntry );
		gNodeList->Unlock();

		// we actually loaded the plugin so go ahead and update the table
		fMutex.WaitLock();
		aTableEntry = fTable;
		while ( aTableEntry != nil )
		{
			if ( aTableEntry->fName != nil )
			{
				if ( ::strcmp( aTableEntry->fName, inName ) == 0 )
				{
					if ( loadIfNeeded && aTableEntry->fPluginPtr == NULL )
					{
						// now use the tmpTableEntry
						aTableEntry->fPluginPtr = tmpTableEntry->fPluginPtr;
						aTableEntry->fState = tmpTableEntry->fState;

						if ( aTableEntry->fPluginPtr != NULL )
						{
							// save in newState so that we can call out to the plugin ouside the lock.
#ifndef DISABLE_CONFIGURE_PLUGIN
							newState = gPluginConfig->GetPluginState( aTableEntry->fPluginPtr->GetPluginName() );
#else
							newState = kActive;
#endif
							aTableEntry->fState = newState;
						}
					}
						
					pResult = aTableEntry->fPluginPtr;
					break;
				}
			}
			aTableEntry = aTableEntry->pNext;
		}
		
		fMutex.SignalLock();
		DSFree(tmpTableEntry);
		
		// if the plugin was loaded, set the state.  needs to be done
		// with the mutex unlocked to prevent deadlock.
		if ( loadIfNeeded && pResult != NULL )
		{
			SetPluginState( pResult, newState );
		}
	}

	return( pResult );

} // GetPlugInPtr


// ---------------------------------------------------------------------------
//	* GetPlugInPtr ()
//
// ---------------------------------------------------------------------------

CServerPlugin* CPlugInList::GetPlugInPtr ( const UInt32 inKey, bool loadIfNeeded )
{
	CServerPlugin  *pResult			= nil;
	sTableData     *aTableEntry		= nil;
	sTableData     *tmpTableEntry	= nil;
	ePluginState	newState		= kUnknownState;

	fMutex.WaitLock();

	aTableEntry = fTable;
	while ( aTableEntry != nil )
	{
		if ( (aTableEntry->fName != nil) )
		{
			if ( aTableEntry->fKey == inKey )
			{
				if (	aTableEntry->fPluginPtr == NULL
#ifndef DISABLE_CONFIGURE_PLUGIN
					&&	(gPluginConfig->GetPluginState(aTableEntry->fName) & kActive)
#endif
					&&	loadIfNeeded )
				{
					// this means we will try to load the plugin below
					tmpTableEntry = MakeTableEntryCopy( aTableEntry );
					break;
				}	

				pResult = aTableEntry->fPluginPtr;

				break;
			}
		}
		aTableEntry = aTableEntry->pNext;
	}

	fMutex.SignalLock();

	if (tmpTableEntry != nil)
	{
		// This plugin has just been set to active but we haven't loaded it yet.
		// should NOT hold the table mutex for this
		gNodeList->Lock();
		LoadPlugin( tmpTableEntry );
		gNodeList->Unlock();

		// we actually loaded the plugin so go ahead and update the table
		fMutex.WaitLock();
		aTableEntry = fTable;
		while ( aTableEntry != nil )
		{
			if ( aTableEntry->fName != nil )
			{
				if ( aTableEntry->fKey == inKey )
				{
					if (	aTableEntry->fPluginPtr == NULL
#ifndef DISABLE_CONFIGURE_PLUGIN
						&&	(gPluginConfig->GetPluginState(aTableEntry->fName) & kActive)
#endif
						&&	loadIfNeeded )
					{
						// now use the tmpTableEntry
						aTableEntry->fPluginPtr = tmpTableEntry->fPluginPtr;
						aTableEntry->fState = tmpTableEntry->fState;
						
						
						if ( aTableEntry->fPluginPtr != NULL )
						{
							// save in newState so that we can call out to the plugin ouside the lock.
#ifndef DISABLE_CONFIGURE_PLUGIN
							newState = gPluginConfig->GetPluginState( aTableEntry->fPluginPtr->GetPluginName() );
#else
							newState = kActive;
#endif
							aTableEntry->fState = newState;
						}
					}
	
					pResult = aTableEntry->fPluginPtr;
	
					break;
				}
			}
			aTableEntry = aTableEntry->pNext;
		}
		fMutex.SignalLock();
		DSFree(tmpTableEntry);

		// if the plugin was loaded, set the state.  needs to be done
		// with the mutex unlocked to prevent deadlock.
		if ( loadIfNeeded && pResult != NULL )
		{
			SetPluginState( pResult, newState );
		}		
	}

	return( pResult );

} // GetPlugInPtr



// ---------------------------------------------------------------------------
//	* Next ()
//
// ---------------------------------------------------------------------------

CServerPlugin* CPlugInList::Next ( UInt32 *inIndex )
{
	CServerPlugin	   *pResult			= nil;
	UInt32				tableIndex		= 0;
	sTableData		   *aTableEntry		= nil;

	fMutex.WaitLock();

	aTableEntry = fTable;
	while ( aTableEntry != nil )
	{
		if (tableIndex == *inIndex)
		{
			if ( (aTableEntry->fName != nil) && (aTableEntry->fPluginPtr != nil) )
			{
				pResult = aTableEntry->fPluginPtr;
				tableIndex++;
				break;
			}
			else
			{
				*inIndex = tableIndex + 1;	// keep looking for next loaded plugin
			}
		}
		tableIndex++;
		aTableEntry = aTableEntry->pNext;
	}

	*inIndex = tableIndex;

	fMutex.SignalLock();

	return( pResult );

} // Next


// ---------------------------------------------------------------------------
//	* GetPlugInInfo ()
//
// ---------------------------------------------------------------------------

CPlugInList::sTableData* CPlugInList::GetPlugInInfo ( UInt32 inIndex )
{
	UInt32				tableIndex		= 0;
	sTableData		   *aTableEntry		= nil;

	fMutex.WaitLock();

	aTableEntry = fTable;
	while ( aTableEntry != nil )
	{
		if (tableIndex == inIndex)
		{
			break;
		}
		tableIndex++;
		aTableEntry = aTableEntry->pNext;
	}

	fMutex.SignalLock();

	return( aTableEntry );

} // GetPlugInInfo


// ---------------------------------------------------------------------------
//	* CopyRecordTypeRestrictionsDictionary( void )
//
// ---------------------------------------------------------------------------
CFMutableDictionaryRef CPlugInList::CopyRecordTypeRestrictionsDictionary( void )
{
	CFMutableDictionaryRef	restrictions = NULL;
	
	fMutex.WaitLock();
	if( fCFRecordTypeRestrictions )
		restrictions = CFDictionaryCreateMutableCopy( kCFAllocatorDefault, NULL, fCFRecordTypeRestrictions );
	else
		restrictions = CFDictionaryCreateMutable( kCFAllocatorDefault, NULL, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
	fMutex.SignalLock();
	
	return(restrictions);
} // CopyRecordTypeRestrictionsDictionary


// ---------------------------------------------------------------------------
//	* SetRecordTypeRestrictionsDictionary( CFMutableDictionaryRef inDictionary )
//
// ---------------------------------------------------------------------------
void CPlugInList::SetRecordTypeRestrictionsDictionary( CFMutableDictionaryRef inDictionary )
{
	CFURLRef				configFileURL			= NULL;
	CFDataRef				xmlData				= NULL;
	CFStringRef			sPath				= NULL;
	SInt32				errorCode				= 0;
	SInt32				siResult				= 0;

	if (inDictionary != NULL)
	{
		fMutex.WaitLock();
		
		DSCFRelease(fCFRecordTypeRestrictions);
		
		CFRetain( inDictionary );
		fCFRecordTypeRestrictions = inDictionary;
		
		sPath = CFStringCreateWithCString( kCFAllocatorDefault, kRecTypeRestrictionsFilePath, kCFStringEncodingUTF8 );
		if (sPath != NULL)
		{
			configFileURL = ::CFURLCreateWithFileSystemPath( kCFAllocatorDefault, sPath, kCFURLPOSIXPathStyle, false );
			DSCFRelease( sPath );

			xmlData = CFPropertyListCreateXMLData(NULL, inDictionary);
			if ( (configFileURL != NULL) && (xmlData != NULL) )
			{
				//write the XML to the config file
				siResult = CFURLWriteDataAndPropertiesToResource( configFileURL, xmlData, NULL, &errorCode);
				if (siResult == eDSNoErr)
				{
					chmod( kRecTypeRestrictionsFilePath, S_IRUSR | S_IWUSR );
				}
			}
			DSCFRelease(configFileURL);
			DSCFRelease(xmlData);
		}

		fMutex.SignalLock();
	}
} // SetRecordTypeRestrictionsDictionary


// ---------------------------------------------------------------------------
//	* ReadRecordTypeRestrictions ()
//
// ---------------------------------------------------------------------------

SInt32 CPlugInList::ReadRecordTypeRestrictions( void )
{
	SInt32					siResult				= eDSNoErr;
	CFURLRef				configFileURL			= NULL;
	CFURLRef				configFileCorruptedURL	= NULL;
	CFDataRef				xmlData					= NULL;
	struct stat				statResult;
	bool					bReadFile				= false;
	bool					bCorruptedFile			= false;
	bool					bWroteFile				= false;
	SInt32					errorCode				= 0;
	CFStringRef				sCorruptedPath			= NULL;
	CFStringRef				sPath					= NULL;

	fMutex.WaitLock();

//Config data is read from a plist file
//Steps in the process:
//1- see if the file exists
//2- if it exists then try to read it
//3- if existing file is corrupted then rename it and save it while creating a new default file
//4- if file doesn't exist then create a new default file - make sure directories exist/if not create them
	
	//step 1- see if the file exists
	//if not then make sure the directories exist or create them
	//then write the file
	siResult = ::stat( kRecTypeRestrictionsFilePath, &statResult );
	
	sPath = CFStringCreateWithCString( kCFAllocatorDefault, kRecTypeRestrictionsFilePath, kCFStringEncodingUTF8 );
	if (sPath != NULL)
	{
		//create URL always
		configFileURL = ::CFURLCreateWithFileSystemPath( kCFAllocatorDefault, sPath, kCFURLPOSIXPathStyle, false );
		DSCFRelease( sPath );
		
		//if file does not exist, let's make sure the directories are there
		if (siResult != eDSNoErr)
		{
			// file does not exist so checking directory path to enable write of a new file
			CreatePrefDirectory();
			
			//create XML data from the default config
			UInt32 uiDataSize = ::strlen( kDefaultRecTypeRestrictionsConfig );
			xmlData = ::CFDataCreate( nil, (const UInt8 *)kDefaultRecTypeRestrictionsConfig, uiDataSize );

			DbgLog( kLogPlugin, "CPlugInList: Created a new Record Type Restrictions config file since it did not exist" );
			
			if ( (configFileURL != NULL) && (xmlData != NULL) )
			{
				//write the XML to the config file
				siResult = CFURLWriteDataAndPropertiesToResource(	configFileURL,
																	xmlData,
																	NULL,
																	&errorCode);
			}
			
			DSCFRelease(xmlData);
			
		} // file does not exist so creating one
		
		if ( (siResult == eDSNoErr) && (configFileURL != NULL) ) //either stat or new write was successful
		{
			chmod( kRecTypeRestrictionsFilePath, S_IRUSR | S_IWUSR );
			// Read the XML property list file
			bReadFile = CFURLCreateDataAndPropertiesFromResource(	kCFAllocatorDefault,
																	configFileURL,
																	&xmlData,          // place to put file data
																	NULL,           
																	NULL,
																	&siResult);
		}
	} // if (sPath != NULL)
	

	if (bReadFile)
	{
		CFPropertyListRef configPropertyList = NULL;
		if (xmlData != nil)
		{
			// extract the config dictionary from the XML data.
			configPropertyList =	CFPropertyListCreateFromXMLData( kCFAllocatorDefault,
									xmlData,
									kCFPropertyListImmutable, 
									NULL);
			if (configPropertyList != nil )
			{
				//make the propertylist a dict
				if ( CFDictionaryGetTypeID() == CFGetTypeID( configPropertyList ) )
				{
					DSCFRelease(fCFRecordTypeRestrictions);
					fCFRecordTypeRestrictions = (CFDictionaryRef) configPropertyList;
				}
			}
		}
		//check if this XML blob is a property list and can be made into a dictionary
		if (fCFRecordTypeRestrictions == NULL)
		{			
			//if it is not then say the file is corrupted and save off the corrupted file
			DbgLog( kLogPlugin, "CPlugInList: Record Type Restrictions config file is corrupted" );
			bCorruptedFile = true;
			//here we need to make a backup of the file - why? - because

			// Append the subpath.
			sCorruptedPath = ::CFStringCreateWithCString( kCFAllocatorDefault, kRecTypeRestrictionsCorruptedFilePath, kCFStringEncodingUTF8 );

			if (sCorruptedPath != NULL)
			{
				// Convert it into a CFURL.
				configFileCorruptedURL = ::CFURLCreateWithFileSystemPath( kCFAllocatorDefault, sCorruptedPath, kCFURLPOSIXPathStyle, false );
				DSCFRelease( sCorruptedPath ); // build with Create so okay to dealloac here
				if (configFileCorruptedURL != NULL)
				{
					//write the XML to the corrupted copy of the config file
					bWroteFile = CFURLWriteDataAndPropertiesToResource( configFileCorruptedURL,
																		xmlData,
																		NULL,
																		&errorCode);
					if (bWroteFile)
					{
						chmod( kRecTypeRestrictionsCorruptedFilePath, S_IRUSR | S_IWUSR );
					}
				}
			}
		}
		DSCFRelease(xmlData);
	}
	else //existing file is unreadable
	{
		DbgLog( kLogPlugin, "CPlugInList: Record Type Restrictions config file is unreadable" );
		bCorruptedFile = true;
	}
        
	if (bCorruptedFile)
	{
		//create XML data from the default config
		UInt32 uiDataSize = ::strlen( kDefaultRecTypeRestrictionsConfig );
		xmlData = ::CFDataCreate( nil, (const UInt8 *)kDefaultRecTypeRestrictionsConfig, uiDataSize );

		DbgLog( kLogPlugin, "CPlugInList: Created a new Record Type Restrictions config file since existing one was corrupted" );
		
		DSCFRelease(fCFRecordTypeRestrictions);
		//assume that the XML blob is good since we created it here
		fCFRecordTypeRestrictions =	(CFDictionaryRef)CFPropertyListCreateFromXMLData( kCFAllocatorDefault,
									xmlData,
									kCFPropertyListImmutable, 
									NULL);

		if ( (configFileURL != NULL) && (xmlData != NULL) )
		{
			//write the XML to the config file
			siResult = CFURLWriteDataAndPropertiesToResource( configFileURL,
																xmlData,
																NULL,
																&errorCode);
			if (siResult == eDSNoErr)
			{
				chmod( kRecTypeRestrictionsFilePath, S_IRUSR | S_IWUSR );
			}
		}
		DSCFRelease(xmlData);
	}
	
	DSCFRelease(configFileURL); // seems okay to dealloc since Create used and done with it now
    
	DSCFRelease(configFileCorruptedURL); // seems okay to dealloc since Create used and done with it now
	
	fMutex.SignalLock();

    return( siResult );

} // ReadRecordTypeRestrictions

// ---------------------------------------------------------------------------
//	* CreatePrefDirectory
// ---------------------------------------------------------------------------

bool CPlugInList::CreatePrefDirectory( void )
{
	int			siResult			= eDSNoErr;
    struct stat statResult;
	
	DbgLog( kLogPlugin, "CPlugInList: Checking for Record Type Restrictions config file:" );
	DbgLog( kLogPlugin, "CPlugInList: %s", kRecTypeRestrictionsFilePath );
	
	//step 1- see if the file exists
	//if not then make sure the directories exist or create them
	//then create a new file if necessary
	siResult = ::stat( kRecTypeRestrictionsFilePath, &statResult );
	
	//if file does not exist
	if (siResult != eDSNoErr)
		siResult = dsCreatePrefsDirectory();
		
	return (siResult == 0);
	
} //CreatePrefDirectory

    
// ---------------------------------------------------------------------------
//	* IsOKToServiceQuery
// ---------------------------------------------------------------------------

bool CPlugInList::IsOKToServiceQuery( const char *inPluginName, const char *inNodeName, const char *inRecordTypeList, UInt32 inNumberRecordTypes )
{
	bool isOK = true;
	
	if (inRecordTypeList == NULL) //can't see this ever happening as we check before calling this routine
		return(isOK);
	
	fMutex.WaitLock();
	
	if (fCFRecordTypeRestrictions != NULL && inPluginName != NULL)
	{
		CFStringRef cfPluginName = NULL;
		cfPluginName = CFStringCreateWithCString( kCFAllocatorDefault, inPluginName, kCFStringEncodingUTF8 );
		if ( CFDictionaryContainsKey( fCFRecordTypeRestrictions, cfPluginName ) ) //plugin entry is in the dictionary
		{
			CFDictionaryRef cfPluginRestrictions = NULL;
			cfPluginRestrictions = (CFDictionaryRef)CFDictionaryGetValue( fCFRecordTypeRestrictions, cfPluginName );
			if (inNodeName != NULL) //we have a node name that we can check for
			{
				CFStringRef cfNodeName = NULL;
				cfNodeName = CFStringCreateWithCString( kCFAllocatorDefault, inNodeName, kCFStringEncodingUTF8 );
				bool useRestrictions = false;
				if ( CFDictionaryContainsKey( cfPluginRestrictions, cfNodeName ) ) //nodename entry is in the dictionary
				{
					useRestrictions = true;
				}
				else if ( CFDictionaryContainsKey( cfPluginRestrictions, CFSTR("General") ) ) //General entry is in the dictionary
				{
					DSCFRelease(cfNodeName);
					cfNodeName = CFStringCreateWithCString( kCFAllocatorDefault, "General", kCFStringEncodingUTF8 );
					useRestrictions = true;
				}
				
				if (useRestrictions)
				{
					//the record type list requested
					CFStringRef cfRecordTypeList = NULL;
					cfRecordTypeList = CFStringCreateWithCString( kCFAllocatorDefault, inRecordTypeList, kCFStringEncodingUTF8 );

					//get the restrictions dictionary
					CFDictionaryRef cfRestrictions = NULL;
					cfRestrictions = (CFDictionaryRef)CFDictionaryGetValue( cfPluginRestrictions, cfNodeName );

					if ( CFDictionaryContainsKey( cfRestrictions, CFSTR(kRTRAllowKey) ) ) //Allow record type entry is in the dictionary
					{
						isOK = false; //init to false since we look over allowed record types
						
						//get the allow record type array
						CFArrayRef cfAllowRecordTypes = NULL;
						cfAllowRecordTypes = (CFArrayRef)CFDictionaryGetValue( cfRestrictions, CFSTR(kRTRAllowKey) );
						
						CFIndex cfNumberRecordTypesInArray = CFArrayGetCount( cfAllowRecordTypes );
						UInt32 countMatchesFound = 0;
						for( CFIndex i = 0; i<cfNumberRecordTypesInArray; i++ )
						{
							CFStringRef cfRecordType = (CFStringRef)CFArrayGetValueAtIndex( cfAllowRecordTypes, i );
							//if cfRecordType is contained within cfRecordTypeList
							if (CFStringFindWithOptions(cfRecordTypeList, cfRecordType, CFRangeMake( 0, CFStringGetLength( cfRecordTypeList )), kCFCompareCaseInsensitive, NULL))
							{
								countMatchesFound++;
								//confirm that inNumberRecordTypes is equal to countMatchesFound
								if (inNumberRecordTypes == countMatchesFound)
								{
									isOK = true;
									break;
								}
							}
						}
					}
					//check for Deny list ONLY if Allow list is NOT present
					else if ( CFDictionaryContainsKey( cfRestrictions, CFSTR(kRTRDenyKey) ) ) //Deny record type entry is in the dictionary
					{
						//get the deny record type array
						CFArrayRef cfDenyRecordTypes = NULL;
						cfDenyRecordTypes = (CFArrayRef)CFDictionaryGetValue( cfRestrictions, CFSTR(kRTRDenyKey) );
						
						CFIndex cfNumberRecordTypesInArray = CFArrayGetCount( cfDenyRecordTypes );
						for( CFIndex i = 0; i<cfNumberRecordTypesInArray; i++ )
						{
							CFStringRef cfRecordType = (CFStringRef)CFArrayGetValueAtIndex( cfDenyRecordTypes, i );
							//if cfRecordType is contained within cfRecordTypeList
							if (CFStringFindWithOptions(cfRecordTypeList, cfRecordType, CFRangeMake( 0, CFStringGetLength( cfRecordTypeList )), kCFCompareCaseInsensitive, NULL))
							{
								isOK = false; //first match we break out
								break;
							}
						}
					}
					
					DSCFRelease(cfRecordTypeList);
				}
				DSCFRelease(cfNodeName);
			}
		}
		DSCFRelease(cfPluginName);
	}
	
	fMutex.SignalLock();

	return(isOK);
	
} // IsOKToServiceQuery


CPlugInList::sTableData* CPlugInList::MakeTableEntryCopy( sTableData *inEntry )
{
	//to be used only for lazy loading
	sTableData* outEntry = nil;

	if (inEntry == nil )
		return(nil);

	outEntry = (sTableData*)calloc(1, sizeof(sTableData));

	//next four const char do not change so no need to strdup
	outEntry->fName = inEntry->fName;
	outEntry->fVersion = inEntry->fVersion;
	outEntry->fConfigAvail = inEntry->fConfigAvail;
	outEntry->fConfigFile = inEntry->fConfigFile;
	outEntry->fPluginPtr = inEntry->fPluginPtr;
	outEntry->fPluginRef = inEntry->fPluginRef;
	outEntry->fCFuuidFactory = inEntry->fCFuuidFactory;
	outEntry->fULVers = inEntry->fULVers;
	outEntry->fKey = inEntry->fKey;
	outEntry->fState = inEntry->fState;
	outEntry->fValidDataStamp = inEntry->fValidDataStamp;
	outEntry->pNext = nil;

	return(outEntry);
}
