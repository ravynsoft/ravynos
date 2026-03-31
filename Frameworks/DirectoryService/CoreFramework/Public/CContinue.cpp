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
 * @header CContinue
 */

#include "CContinue.h"

#include <stdlib.h>
#include <string.h>

#include <vector>
using std::vector;

struct sContinueEntry
{
	UInt32	fRefNum;
	void	*fPointer;
};

//------------------------------------------------------------------------------------
//	* CContinue
//------------------------------------------------------------------------------------

CContinue::CContinue ( DeallocateProc inProcPtr ) : fMutex("CContinue::fMutex")
{
	fDeallocProcPtr = inProcPtr;
	fNextContextID = 1;

} // CContinue


//------------------------------------------------------------------------------------
//	* ~CContinue
//------------------------------------------------------------------------------------

CContinue::~CContinue ( void )
{
} // ~CContinue


tContextData CContinue::AddPointer( void *inPointer, UInt32 inRefNum )
{
	tContextData contextValue = 0;
	
	if ( inPointer != NULL && inRefNum != 0 )
	{
		fMutex.WaitLock();

		sContinueEntry *entry = new sContinueEntry;
		
		entry->fPointer = inPointer;
		entry->fRefNum = inRefNum;

		// technically if we filled the entire 4 billion entries, we would have a loop
		// but we couldn't allocate that many anyway
		while ( fContextMap.find(fNextContextID) != fContextMap.end() )
		{
			// we never use 0
			if ( 0 == ++fNextContextID )
				fNextContextID++;
		}

		contextValue = fNextContextID;
		fContextMap[contextValue] = entry;
		fNextContextID++; // increment again since we just used this value
		
		fMutex.SignalLock();
	}
	
	return contextValue;
}

void CContinue::RemovePointer( void *inPointer )
{
	void	*thePointer = NULL;
	
	fMutex.WaitLock();
	
	map<tContextData, sContinueEntry *>::iterator	iter;
	for ( iter = fContextMap.begin(); iter != fContextMap.end(); iter++ )
	{
		sContinueEntry *entry = iter->second;
		if ( entry->fPointer == inPointer )
		{
			thePointer = inPointer;
			fContextMap.erase( iter++ );
			DSDelete( entry );
			break;
		}
	}
	
	fMutex.SignalLock();
	
	if ( fDeallocProcPtr != NULL && thePointer != NULL )
		(fDeallocProcPtr)( thePointer );
}

	
void CContinue::RemovePointersForRefNum( UInt32 inRefNum )
{
	vector<void *>	entryDataPendingDelete;
	
	fMutex.WaitLock();
	
	map<tContextData, sContinueEntry *>::iterator	iter = fContextMap.begin();
	while ( iter != fContextMap.end() )
	{
		sContinueEntry *entry = iter->second;
		if ( entry->fRefNum == inRefNum )
		{
			entryDataPendingDelete.push_back( entry->fPointer );
			fContextMap.erase( iter++ );
			DSDelete( entry );
		}
		else
		{
			iter++;
		}
	}
	
	fMutex.SignalLock();
	
	// Now the entry data can be deleted without deadlocking.
	if ( fDeallocProcPtr != NULL )
	{
		while ( entryDataPendingDelete.size() != 0 )
		{
			(fDeallocProcPtr)( (void *) entryDataPendingDelete.back() );
			entryDataPendingDelete.pop_back();
		}
	}
}

tDirStatus CContinue::RemoveContext( tContextData inContextData )
{
	tDirStatus	siResult	= eDSInvalidContinueData;
	
	if ( inContextData != 0 )
	{
		void *thePointer	= NULL;
		
		fMutex.WaitLock();
		
		map<tContextData, sContinueEntry *>::iterator	iter = fContextMap.find( inContextData );
		if ( iter != fContextMap.end() )
		{
			thePointer = iter->second->fPointer;
			DSDelete( iter->second );
			fContextMap.erase( iter );
			siResult = eDSNoErr;
		}
		
		fMutex.SignalLock();
		
		// Now the entry data can be deleted without deadlocking.
		if ( fDeallocProcPtr != NULL && thePointer != NULL )
		{
			(fDeallocProcPtr)( thePointer );
		}		
	}
	else
	{
		siResult = eDSNoErr;
	}
	
	return siResult;
}

void *CContinue::GetPointer( tContextData inContextData )
{
	void	*thePointer	= NULL;
	
	fMutex.WaitLock();
	
	map<tContextData, sContinueEntry *>::iterator	iter = fContextMap.find( inContextData );
	if ( iter != fContextMap.end() )
		thePointer = iter->second->fPointer;
	
	fMutex.SignalLock();
	
	return thePointer;
}

UInt32 CContinue::GetRefNum( tContextData inContextData )
{
	UInt32	refNum = 0;
	
	fMutex.WaitLock();
	
	map<tContextData, sContinueEntry *>::iterator	iter = fContextMap.find( inContextData );
	if ( iter != fContextMap.end() )
		refNum = iter->second->fRefNum;
	
	fMutex.SignalLock();
	
	return refNum;
}
