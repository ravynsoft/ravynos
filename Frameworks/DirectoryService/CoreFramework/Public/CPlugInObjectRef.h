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
 * @header CPlugInObjectRef
 */

#ifndef __CPlugInObjectRef_h__
#define	__CPlugInObjectRef_h__

#include <DirectoryServiceCore/PrivateTypes.h>
#include <DirectoryServiceCore/SharedConsts.h>
#include <DirectoryServiceCore/DSSemaphore.h>

template <class CObjectClass> class CPlugInObjectRef
{
	struct sObjectTableEntry {
		UInt32						fRefNum;
		CObjectClass				fObject;
		struct sObjectTableEntry	*fNext;
		
		sObjectTableEntry( UInt32 inRefNum, CObjectClass inObject, struct sObjectTableEntry *inNext = NULL )
		{
			fRefNum = inRefNum;
			fObject = inObject->Retain();
			fNext = inNext;
		}
		
		~sObjectTableEntry( void )
		{
			fObject->Release();
		}
	};

	public:
						CPlugInObjectRef	( uint32_t inHashArrayLength = 128 )
						{
							fHashArrayLength = inHashArrayLength;
							fRefNumCount = 0;
							fLookupTable = (sObjectTableEntry **) calloc( fHashArrayLength, sizeof(sObjectTableEntry *) );
						}
	
						~CPlugInObjectRef	( void )
						{
							for ( uint32_t ii = 0; ii < fHashArrayLength; ii++ )
							{
								sObjectTableEntry *pCurrEntry = fLookupTable[ii];
								while ( pCurrEntry != NULL )
								{
									sObjectTableEntry *pDelEntry = pCurrEntry;
									pCurrEntry = pCurrEntry->fNext;
									DSDelete( pDelEntry );
								}
								
								fLookupTable[ii] = NULL;
							}
							
							DSFree( fLookupTable );
						}
	
		tDirStatus		AddObjectForRefNum	( UInt32 inRefNum, CObjectClass inObject )
		{
			tDirStatus	siResult	= eDSNoErr;
			
			if ( inObject == NULL )
				siResult = eDSNullParameter;
			
			if ( siResult == eDSNoErr )
			{
				// Calculate where we are going to put this entry
				uint32_t uiSlot = inRefNum % fHashArrayLength;
				
				fMutex.WaitLock();
				
				// Check to see if this item has already been added
				sObjectTableEntry *pCurrEntry = fLookupTable[uiSlot];
				while ( pCurrEntry != NULL )
				{
					if ( pCurrEntry->fRefNum == inRefNum )
					{
						siResult = eDSInvalidIndex;
						break;
					}
					
					pCurrEntry = pCurrEntry->fNext;
				}
				
				// add it to the list now if we did not get an error
				if ( siResult == eDSNoErr )
				{
					fLookupTable[ uiSlot ] = new sObjectTableEntry( inRefNum, inObject, fLookupTable[uiSlot] );
					fRefNumCount++;
				}
				
				fMutex.SignalLock();
			}
			
			return siResult;
		}

		tDirStatus		RemoveRefNum		( UInt32 inRefNum )
		{
			tDirStatus	siResult	= eDSIndexNotFound;
			uint32_t	uiSlot		= inRefNum % fHashArrayLength;
			
			fMutex.WaitLock();
			
			// Look across all entries at this position
			sObjectTableEntry *pCurrEntry = fLookupTable[ uiSlot ];
			sObjectTableEntry *pPrevEntry = pCurrEntry;
			while ( pCurrEntry != NULL )
			{
				// Is it the one we want
				if ( pCurrEntry->fRefNum == inRefNum )
					break;
				
				pPrevEntry = pCurrEntry;
				pCurrEntry = pCurrEntry->fNext;
			}
			
			if ( pCurrEntry != NULL )
			{
				// if pCurrEntry == pPrevEntry, then it is the first entry
				if ( pCurrEntry == pPrevEntry )
					fLookupTable[ uiSlot ] = pCurrEntry->fNext;
				else
					pPrevEntry->fNext = pCurrEntry->fNext;
				
				fRefNumCount--;
				siResult = eDSNoErr;
			}
			
			fMutex.SignalLock();
			
			DSDelete( pCurrEntry );
			
			return siResult;
		}
	
		CObjectClass	GetObjectForRefNum	( UInt32 inRefNum )
		{
			CObjectClass	pResult	= NULL;
			
			fMutex.WaitLock();
			
			// Look across all entries at this position
			sObjectTableEntry *pEntry = fLookupTable[ inRefNum % fHashArrayLength ];
			while ( pEntry != NULL )
			{
				// Is it the one we want
				if ( pEntry->fRefNum == inRefNum )
				{
					pResult = pEntry->fObject->Retain();
					break;
				}
				
				pEntry = pEntry->fNext;
			}
			
			fMutex.SignalLock();
			
			return pResult;
		}
	
	private:
		sObjectTableEntry	**fLookupTable;
		uint32_t			fHashArrayLength;
		uint32_t			fRefNumCount;
		DSSemaphore			fMutex;
};

#endif
