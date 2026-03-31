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
 * @header CPlugInRef
 */

#include "CPlugInRef.h"

#include <stdlib.h>
#include <string.h>

//------------------------------------------------------------------------------------
//	* CPlugInRef
//------------------------------------------------------------------------------------

CPlugInRef::CPlugInRef ( DeallocateProc inProcPtr ) : fMutex("CPluginRef::fMutex")
{
	fHashArrayLength = 128;
	fRefNumCount = 0;
	fLookupTable = (sTableEntry**)calloc(fHashArrayLength, sizeof(sTableEntry*));

	fDeallocProcPtr = inProcPtr;

} // CPlugInRef


CPlugInRef::CPlugInRef ( DeallocateProc inProcPtr, UInt32 inHashArrayLength ) : fMutex("CPluginRef::fMutex")
{
	fHashArrayLength = inHashArrayLength;
	fRefNumCount = 0;
	fLookupTable = (sTableEntry**)calloc(fHashArrayLength, sizeof(sTableEntry*));

	fDeallocProcPtr = inProcPtr;

} // CPlugInRef


//------------------------------------------------------------------------------------
//	* ~CPlugInRef
//------------------------------------------------------------------------------------

CPlugInRef::~CPlugInRef ( void )
{
} // ~CPlugInRef


//------------------------------------------------------------------------------------
//	* AddItem
//------------------------------------------------------------------------------------

SInt32 CPlugInRef::AddItem ( UInt32 inRefNum, void *inData )
{
	SInt32			siResult	= eDSNoErr;
	UInt32			uiSlot		= 0;
	sTableEntry	   *pNewEntry	= nil;
	sTableEntry	   *pCurrEntry	= nil;

	fMutex.WaitLock();

	// Create the new entry object
	pNewEntry = (sTableEntry *)::calloc( 1, sizeof( sTableEntry ) );
	if ( pNewEntry != nil )
	{
		pNewEntry->fRefNum		= inRefNum;
		pNewEntry->fData		= inData;
	}
	else
	{
		siResult = eMemoryError;
	}

	if ( siResult == eDSNoErr )
	{
		// Calculate where we are going to put this entry
		uiSlot = inRefNum % fHashArrayLength;

		if ( fLookupTable[ uiSlot ] == nil )
		{
			// This slot is currently empty so this is the first one
			fLookupTable[ uiSlot ] = pNewEntry;
			fRefNumCount++;
		}
		else
		{
			// Check to see if this item has already been added
			pCurrEntry = fLookupTable[ uiSlot ];
			while ( pCurrEntry != nil )
			{
				if ( pCurrEntry->fRefNum == inRefNum )
				{
					// We found a duplicate.
					siResult = eDSInvalidIndex;
					free( pNewEntry );
					pNewEntry = nil;
					break;
				}
				pCurrEntry = pCurrEntry->fNext;
			}

			if ( siResult == eDSNoErr )
			{
				// This slot is occupied so add it to the head of the list
				pCurrEntry = fLookupTable[ uiSlot ];

				pNewEntry->fNext = pCurrEntry;

				fLookupTable[ uiSlot ] = pNewEntry;
				fRefNumCount++;
			}
		}
	}

	fMutex.SignalLock();

	return( siResult );

} // AddItem


//------------------------------------------------------------------------------------
//	* GetItem
//------------------------------------------------------------------------------------

void *CPlugInRef::GetItemData ( UInt32 inRefNum )
{
	void		   *pvResult	= nil;
	UInt32			uiSlot		= 0;
	sTableEntry	   *pEntry		= nil;

	fMutex.WaitLock();

	// Calculate where we thought we put it last
	uiSlot = inRefNum % fHashArrayLength;

	// Look across all entries at this position
	pEntry = fLookupTable[ uiSlot ];
	while ( pEntry != nil )
	{
		// Is it the one we want
		if ( pEntry->fRefNum == inRefNum )
		{
			// Get the data
			pvResult = pEntry->fData;

			break;
		}
		pEntry = pEntry->fNext;
	}

	fMutex.SignalLock();

	// A return of NULL means that we did not find the item
	return( pvResult );

} // GetItem


//------------------------------------------------------------------------------------
//	* RemoveItem
//
//		- Remove the item.  There could be duplicates, so we must deal with this.
//
//------------------------------------------------------------------------------------

SInt32 CPlugInRef::RemoveItem ( UInt32 inRefNum )
{
	SInt32			siResult	= eDSIndexNotFound;
	UInt32			uiSlot		= 0;
	sTableEntry	   *pCurrEntry	= nil;
	sTableEntry	   *pPrevEntry	= nil;

	fMutex.WaitLock();

	// Calculate where we thought we put it last
	uiSlot = inRefNum % fHashArrayLength;

	// Look across all entries at this position
	pCurrEntry = fLookupTable[ uiSlot ];
	pPrevEntry = fLookupTable[ uiSlot ];
	while ( pCurrEntry != nil )
	{
		// Is it the one we want
		if ( pCurrEntry->fRefNum == inRefNum )
		{
			// Is it the first one in the list
			if ( pCurrEntry == pPrevEntry )
			{
				// Remove the first item from the list
				fLookupTable[ uiSlot ] = pCurrEntry->fNext;
			}
			else
			{
				// Keep the list linked
				pPrevEntry->fNext = pCurrEntry->fNext;
			}

			if ( (fDeallocProcPtr != nil) && (pCurrEntry->fData != nil) )
			{
				fMutex.SignalLock();
				(fDeallocProcPtr)( pCurrEntry->fData );
				fMutex.WaitLock();
			}
			free( pCurrEntry );
			pCurrEntry = nil;
			fRefNumCount--;
			
			siResult = eDSNoErr;

			break;
		}

		if ( pCurrEntry != nil )
		{
			pPrevEntry = pCurrEntry;
			pCurrEntry = pPrevEntry->fNext;
		}
	}

	fMutex.SignalLock();

	return( siResult );

} // RemoveItem

//------------------------------------------------------------------------------------
//	* DoOnAllItems
//------------------------------------------------------------------------------------

void CPlugInRef:: DoOnAllItems ( OperationProc *inProcPtr )
{
	UInt32			uiSlot		= 0;
	sTableEntry	   *pEntry		= nil;

	if (inProcPtr == nil)
	{
		return;
	}
	fMutex.WaitLock();

	for (uiSlot = 0; uiSlot < fHashArrayLength; uiSlot++)
	{
		// Look across all entries at this position
		pEntry = fLookupTable[ uiSlot ];
		while ( pEntry != nil )
		{
			if ( pEntry->fData != nil )
			{
				(inProcPtr)(pEntry->fData);
			}
			pEntry = pEntry->fNext;
		}
	}

	fMutex.SignalLock();

} // DoOnAllItems

