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
 * @header CDSRefTable
 * DirectoryService Framework reference table for all
 * "client side buffer parsing" operations on plugin data
 * returned in standard buffer format.
 * References here always use 0x00300000 bits
 *                              XX			= table number (0x01 - 0x0f)
 *                                30		= CSBP tag (always 0x30)
 *                                  XXXX	= reference value (1 - kMaxTableItems)
 */

#include "CDSRefTable.h"

#include <stdlib.h>
#include <string.h>
#include <syslog.h>		// for syslog() to log calls
#include <libkern/OSAtomic.h>

//------------------------------------------------------------------------------------
//	* CDSRefTable
//------------------------------------------------------------------------------------

CDSRefTable::CDSRefTable ( void ) : fTableMutex( "CDSRefTable::fTableMutex", false )
{
	fTableCount		= 0;
	fRefCount		= 0;
	memset( fRefTables, 0, sizeof( fRefTables ) );
} // CDSRefTable


//------------------------------------------------------------------------------------
//	* ~CDSRefTable
//------------------------------------------------------------------------------------

CDSRefTable::~CDSRefTable ( void )
{
	ClearAllTables();
} // ~CDSRefTable


//--------------------------------------------------------------------------------------------------
//	* ClearAllTables
//
//--------------------------------------------------------------------------------------------------

void CDSRefTable::ClearAllTables( void )
{
	UInt32		i	= 1;
	UInt32		j	= 1;

	fTableMutex.WaitLock();
	
	for ( i = 1; i <= kMaxFWTables; i++ )	//array is still zero based even if first entry NOT used
											//-- added the last kMaxTable in the .h file so this should work now
	{
		if ( fRefTables[ i ] != nil )
		{
			for (j=0; j< kMaxFWTableItems; j++)
			{
				if (fRefTables[ i ]->fTableData[j] != nil)
				{
					free(fRefTables[ i ]->fTableData[j]);
					fRefTables[ i ]->fTableData[j] = nil;
				}
			}
			free( fRefTables[ i ] );  //free all the calloc'ed sRefEntries inside as well - code above
			fRefTables[ i ] = nil;
			fTableCount--;
		}
	}
	
	fTableMutex.SignalLock();
}

//--------------------------------------------------------------------------------------------------
//	* VerifyReference
//
//--------------------------------------------------------------------------------------------------

tDirStatus CDSRefTable::VerifyReference (	tDirReference	inDirRef,
                                            UInt32			inType,
                                            SInt32			inPID )
{
	tDirStatus		siResult	= eDSNoErr;
	sFWRefEntry	   *refData		= nil;
	sPIDFWInfo	   *pPIDInfo	= nil;

	siResult = GetReference( inDirRef, &refData );
	//ref actually exists
	if ( siResult == eDSNoErr )
	{
		//check if the type is correct
		if ( refData->fType != inType )
		{
			siResult = eDSInvalidRefType;
		}
		else
		{
			//check the PIDs here - both parent and child client ones
			siResult = eDSInvalidRefType;
			//parent client PID check
			if (refData->fPID == inPID)
			{
				siResult = eDSNoErr;
			}
			else
			{
				//child clients PID check
				pPIDInfo = refData->fChildPID;
				while ( (pPIDInfo != nil) && (siResult != eDSNoErr) )
				{
					if (pPIDInfo->fPID == inPID)
					{
						siResult = eDSNoErr;
					}
					pPIDInfo = pPIDInfo->fNext;
				}
			}
		}
	}

	return( siResult );

} // VerifyReference



//------------------------------------------------------------------------------------
//	* VerifyDirRef
//
//------------------------------------------------------------------------------------

tDirStatus CDSRefTable::VerifyDirRef (	tDirReference		inDirRef,
										SInt32				inPID )
{
	tDirStatus		siResult	= eDSDirSrvcNotOpened;

	siResult = VerifyReference( inDirRef, eDirectoryRefType, inPID );

	return( siResult );

} // VerifyDirRef



//------------------------------------------------------------------------------------
//	* VerifyNodeRef
//
//------------------------------------------------------------------------------------

tDirStatus CDSRefTable::VerifyNodeRef (	tDirNodeReference	inDirNodeRef,
										SInt32				inPID )
{
	tDirStatus		siResult	= eDSDirSrvcNotOpened;

	siResult = VerifyReference( inDirNodeRef, eNodeRefType, inPID );

	return( siResult );

} // VerifyNodeRef


//------------------------------------------------------------------------------------
//	* VerifyRecordRef
//
//------------------------------------------------------------------------------------

tDirStatus CDSRefTable::VerifyRecordRef (	tRecordReference	inRecordRef,
                                            SInt32				inPID )
{
	tDirStatus		siResult	= eDSDirSrvcNotOpened;

	siResult = VerifyReference( inRecordRef, eRecordRefType, inPID );

	return( siResult );

} // VerifyRecordRef



//------------------------------------------------------------------------------------
//	* VerifyAttrListRef
//
//------------------------------------------------------------------------------------

tDirStatus CDSRefTable::VerifyAttrListRef (	tAttributeListRef	inAttributeListRef,
											SInt32				inPID )
{
	tDirStatus		siResult	= eDSDirSrvcNotOpened;

	siResult = VerifyReference( inAttributeListRef, eAttrListRefType, inPID );

	return( siResult );

} // VerifyAttrListRef


//------------------------------------------------------------------------------------
//	* VerifyAttrValueRef
//
//------------------------------------------------------------------------------------

tDirStatus CDSRefTable::VerifyAttrValueRef (	tAttributeValueListRef	inAttributeValueListRef,
                                                SInt32					inPID )
{
	tDirStatus		siResult	= eDSDirSrvcNotOpened;

	siResult = VerifyReference( inAttributeValueListRef, eAttrValueListRefType, inPID );

	return( siResult );

} // VerifyAttrValueRef


//------------------------------------------------------------------------------------
//	* NewDirRef
//------------------------------------------------------------------------------------

tDirStatus CDSRefTable::NewDirRef ( UInt32 *outNewRef, SInt32 inPID )
{
	tDirStatus		siResult	= eDSDirSrvcNotOpened;

	siResult = GetNewRef( outNewRef, 0, eDirectoryRefType, inPID );

	return( siResult );

} // NewDirRef


//------------------------------------------------------------------------------------
//	* NewNodeRef
//------------------------------------------------------------------------------------

tDirStatus CDSRefTable::NewNodeRef (	UInt32			*outNewRef,
										UInt32			inParentID,
										SInt32			inPID )
{
	tDirStatus		siResult	= eDSDirSrvcNotOpened;

	siResult = GetNewRef( outNewRef, inParentID, eNodeRefType, inPID );

	return( siResult );

} // NewNodeRef


//------------------------------------------------------------------------------------
//	* NewRecordRef
//------------------------------------------------------------------------------------

tDirStatus CDSRefTable::NewRecordRef (	UInt32			*outNewRef,
										UInt32			inParentID,
										SInt32			inPID )
{
	tDirStatus		siResult	= eDSDirSrvcNotOpened;

	siResult = GetNewRef( outNewRef, inParentID, eRecordRefType, inPID );

	return( siResult );

} // NewRecordRef


//------------------------------------------------------------------------------------
//	* NewAttrListRef
//------------------------------------------------------------------------------------

tDirStatus CDSRefTable::NewAttrListRef (	UInt32			*outNewRef,
											UInt32			inParentID,
											SInt32			inPID )
{
	tDirStatus		siResult	= eDSDirSrvcNotOpened;

	siResult = GetNewRef( outNewRef, inParentID, eAttrListRefType, inPID );

	return( siResult );

} // NewAttrListRef


//------------------------------------------------------------------------------------
//	* NewAttrValueRef
//------------------------------------------------------------------------------------

tDirStatus CDSRefTable::NewAttrValueRef (	UInt32			*outNewRef,
											UInt32			inParentID,
											SInt32			inPID )
{
	tDirStatus		siResult	= eDSDirSrvcNotOpened;

	siResult = GetNewRef( outNewRef, inParentID, eAttrValueListRefType, inPID );

	return( siResult );

} // NewAttrValueRef


//------------------------------------------------------------------------------------
//	* RemoveDirRef
//------------------------------------------------------------------------------------

tDirStatus CDSRefTable::RemoveDirRef ( UInt32 inDirRef, SInt32 inPID )
{
	tDirStatus		siResult	= eDSDirSrvcNotOpened;

	siResult = RemoveRef( inDirRef, eDirectoryRefType, inPID );

	return( siResult );

} // RemoveDirRef


//------------------------------------------------------------------------------------
//	* RemoveNodeRef
//------------------------------------------------------------------------------------

tDirStatus CDSRefTable::RemoveNodeRef ( UInt32 inNodeRef, SInt32 inPID )
{
	tDirStatus		siResult	= eDSDirSrvcNotOpened;

	siResult = RemoveRef( inNodeRef, eNodeRefType, inPID );

	return( siResult );

} // RemoveNodeRef


//------------------------------------------------------------------------------------
//	* RemoveRecordRef
//------------------------------------------------------------------------------------

tDirStatus CDSRefTable::RemoveRecordRef ( UInt32 inRecRef, SInt32 inPID )
{
	tDirStatus		siResult	= eDSDirSrvcNotOpened;

	siResult = RemoveRef( inRecRef, eRecordRefType, inPID );

	return( siResult );

} // RemoveRecordRef


//------------------------------------------------------------------------------------
//	* RemoveAttrListRef
//------------------------------------------------------------------------------------

tDirStatus CDSRefTable::RemoveAttrListRef ( UInt32 inAttrListRef, SInt32 inPID )
{
	tDirStatus		siResult	= eDSDirSrvcNotOpened;

	siResult = RemoveRef( inAttrListRef, eAttrListRefType, inPID );

	return( siResult );

} // RemoveAttrListRef


//------------------------------------------------------------------------------------
//	* RemoveAttrValueRef
//------------------------------------------------------------------------------------

tDirStatus CDSRefTable::RemoveAttrValueRef ( UInt32 inAttrValueRef, SInt32 inPID )
{
	tDirStatus		siResult	= eDSDirSrvcNotOpened;

	siResult = RemoveRef( inAttrValueRef, eAttrValueListRefType, inPID );

	return( siResult );

} // RemoveAttrValueRef


//------------------------------------------------------------------------------------
//	* GetTableRef
//------------------------------------------------------------------------------------

sFWRefEntry* CDSRefTable::GetTableRef ( UInt32 inRefNum )
{
	UInt32			uiSlot			= 0;
	UInt32			uiRefNum		= (inRefNum & 0x0000FFFF);
	UInt32			uiTableNum		= (inRefNum & 0xFF000000) >> 24;
	sRefFWTable	   *pTable			= nil;
	sFWRefEntry	   *pOutEntry		= nil;

	fTableMutex.WaitLock();

	try
	{
		pTable = GetThisTable( uiTableNum );
		if ( pTable == nil ) throw( (SInt32)eDSInvalidReference );

		uiSlot = uiRefNum % kMaxFWTableItems;
		if ( pTable->fTableData != nil)
		{
			if ( pTable->fTableData[ uiSlot ] != nil )
			{
				if ( uiRefNum == pTable->fTableData[ uiSlot ]->fRefNum )
				{
					pOutEntry = pTable->fTableData[ uiSlot ];
				}
			}
		}
	}

	catch( SInt32 err )
	{
	}

	fTableMutex.SignalLock();

	return( pOutEntry );

} // GetTableRef


//------------------------------------------------------------------------------------
//	* GetNextTable
//------------------------------------------------------------------------------------

sRefFWTable* CDSRefTable::GetNextTable ( sRefFWTable *inTable )
{
	UInt32		uiTblNum	= 0;
	sRefFWTable	*pOutTable	= nil;

	fTableMutex.WaitLock();

	try
	{
		if ( inTable == nil )
		{
			// Get the first reference table, create it if it wasn't already
			if ( fRefTables[ 1 ] == nil )
			{
				// No tables have been allocated yet so lets make one
				fRefTables[ 1 ] = (sRefFWTable *) calloc( 1, sizeof(sRefFWTable) );
				if ( fRefTables[ 1 ] == nil )  throw((SInt32)eMemoryAllocError);

				fRefTables[ 1 ]->fTableNum = 1;

				fTableCount = 1;
			}

			pOutTable = fRefTables[ 1 ];
		}
		else
		{
			uiTblNum = inTable->fTableNum + 1;
			if (uiTblNum > kMaxFWTables) throw( (SInt32)eDSInvalidReference );

			if ( fRefTables[ uiTblNum ] == nil )
			{
				// Set the table counter
				fTableCount = uiTblNum;

				// No tables have been allocated yet so lets make one
				fRefTables[ uiTblNum ] = (sRefFWTable *) calloc( 1, sizeof(sRefFWTable) );
				if ( fRefTables[ uiTblNum ] == nil ) throw((SInt32)eMemoryAllocError);

				if (uiTblNum == 0) throw( (SInt32)eDSInvalidReference );
				fRefTables[ uiTblNum ]->fTableNum = uiTblNum;
			}

			pOutTable = fRefTables[ uiTblNum ];
		}
	}

	catch( SInt32 err )
	{
	}

	fTableMutex.SignalLock();

	return( pOutTable );

} // GetNextTable


//------------------------------------------------------------------------------------
//	* GetThisTable
//------------------------------------------------------------------------------------

sRefFWTable* CDSRefTable::GetThisTable ( UInt32 inTableNum )
{
	sRefFWTable	*pOutTable	= nil;

	fTableMutex.WaitLock();

	pOutTable = fRefTables[ inTableNum ];

	fTableMutex.SignalLock();

	return( pOutTable );

} // GetThisTable


//------------------------------------------------------------------------------------
//	* GetNewRef
//------------------------------------------------------------------------------------

tDirStatus CDSRefTable::GetNewRef (	UInt32		   *outRef,
									UInt32			inParentID,
									eRefTypes		inType,
									SInt32			inPID )
{
	bool			done		= false;
	tDirStatus		outResult	= eDSNoErr;
	sRefFWTable	   *pCurTable	= nil;
	UInt32			uiRefNum	= 0;
	UInt32			uiSlot		= 0;

	fTableMutex.WaitLock();

	try
	{
		*outRef = 0;

		while ( !done )
		{
			pCurTable = GetNextTable( pCurTable );
			if ( pCurTable == nil ) throw( (SInt32)eDSRefTableCSBPAllocError );

			// we don't use slot #1 so we need to compare against kMaxFWTableItems - 1
			if ( pCurTable->fItemCnt < (kMaxFWTableItems - 1) )
			{
				if ( pCurTable->fCurRefNum == 0 || pCurTable->fCurRefNum > kMaxFWTableItems )
					pCurTable->fCurRefNum = 1;

				for ( uiRefNum = pCurTable->fCurRefNum; uiRefNum < kMaxFWTableItems; uiRefNum++ )
				{
					// Find a slot in the table for this ref number
					uiSlot = uiRefNum % kMaxFWTableItems;
					if ( pCurTable->fTableData[ uiSlot ] == nil )
					{
						pCurTable->fTableData[ uiSlot ] = (sFWRefEntry *) calloc( 1, sizeof(sFWRefEntry) );
						if ( pCurTable->fTableData[ uiSlot ] == nil ) throw( (SInt32)eDSRefTableCSBPAllocError );
						
						// Add the table number to the reference number
						*outRef = (pCurTable->fTableNum << 24) | 0x00300000 | uiRefNum;
						
						// We found an empty slot, now set this table entry
						pCurTable->fTableData[ uiSlot ]->fRefNum		= uiRefNum;
						pCurTable->fTableData[ uiSlot ]->fType			= inType;
						pCurTable->fTableData[ uiSlot ]->fParentID		= inParentID;
						pCurTable->fTableData[ uiSlot ]->fPID			= inPID;
						pCurTable->fTableData[ uiSlot ]->fOffset		= 0;
						pCurTable->fTableData[ uiSlot ]->fBufTag		= 0;
						pCurTable->fTableData[ uiSlot ]->fChildren		= nil;
						pCurTable->fTableData[ uiSlot ]->fChildPID		= nil;

						// Up the item count
						pCurTable->fItemCnt++;
						
						//keep track of the total ref count here
						fRefCount++;

						outResult = eDSNoErr;
						done = true;
						break;
					}
				}
				
				// skip forward 1 so we can avoid immediate reference number re-use
				pCurTable->fCurRefNum = uiRefNum + 1;
			}
		}

		if ( outResult == eDSNoErr && inParentID != 0 )
		{
			outResult = LinkToParent( *outRef, inType, inParentID, inPID );
		}
	}

	catch( SInt32 err )
	{
		outResult = (tDirStatus)err;
	}

	fTableMutex.SignalLock();
	
	if ( outResult == eDSNoErr )
	{
		static uint32_t warnRefCount = 0x000001ff; // start at 512 as our first warning point
		
		// warn if we have exceeded the next level
		if ( fRefCount > warnRefCount )
		{
			syslog( LOG_WARNING, "DirectoryService CSBP significant amount of refs - %d", fRefCount );
			warnRefCount = ((warnRefCount << 1) | 0x00000001) & 0x00007fff; // up to the next level
		}
		// see if we happen to be less than the last warning level
		else if ( warnRefCount > 0x000001ff && fRefCount < (warnRefCount >> 1) ) {
			warnRefCount >>= 1;
		}		
	}
	
	return( outResult );

} // GetNewRef


//------------------------------------------------------------------------------------
//	* LinkToParent
//------------------------------------------------------------------------------------

tDirStatus CDSRefTable::LinkToParent ( UInt32 inRefNum, UInt32 inType, UInt32 inParentID, SInt32 inPID )
{
	tDirStatus		dsResult		= eDSNoErr;
	sFWRefEntry	   *pCurrRef		= nil;
	sListFWInfo	   *pChildInfo		= nil;

	fTableMutex.WaitLock();

	try
	{
		pCurrRef = GetTableRef( inParentID );
		if ( pCurrRef == nil ) throw( (SInt32)eDSInvalidReference );

		// This is the one we want
		pChildInfo = (sListFWInfo *)::calloc( sizeof( sListFWInfo ), sizeof( char ) );
		if ( pChildInfo == nil ) throw( (SInt32)eDSRefTableCSBPAllocError );

		// Save the info required later for removal if the parent gets removed
		pChildInfo->fRefNum		= inRefNum;
		pChildInfo->fType		= inType;
		pChildInfo->fPID		= inPID;

		// Set the link info
		pChildInfo->fNext = pCurrRef->fChildren;
		pCurrRef->fChildren = pChildInfo;
	}

	catch( SInt32 err )
	{
		dsResult = (tDirStatus)err;
	}

	fTableMutex.SignalLock();

	return( dsResult );

} // LinkToParent


//------------------------------------------------------------------------------------
//	* UnlinkFromParent
//------------------------------------------------------------------------------------

tDirStatus CDSRefTable::UnlinkFromParent ( UInt32 inRefNum )
{
	tDirStatus		dsResult		= eDSNoErr;
	UInt32			i				= 1;
	UInt32			parentID		= 0;
	sFWRefEntry	   *pCurrRef		= nil;
	sFWRefEntry	   *pParentRef		= nil;
	sListFWInfo	   *pCurrChild		= nil;
	sListFWInfo	   *pPrevChild		= nil;

	fTableMutex.WaitLock();

	try
	{
		//Node references are currently not linked to their parent dir reference
		//So, there is no issue when any child client PID has a reference linked to a parent since
		//it is unique to that child client PID and can be unlinked as before
		
		// Get the current reference
		pCurrRef = GetTableRef( inRefNum );
		if ( pCurrRef == nil ) throw( (SInt32)eDSInvalidReference );

		parentID = pCurrRef->fParentID;

		if ( parentID != 0 )
		{
			// Get the parent reference
			pParentRef = GetTableRef( parentID );
			if ( pParentRef == nil ) throw( (SInt32)eDSInvalidReference );

			pCurrChild = pParentRef->fChildren;
			pPrevChild = pParentRef->fChildren;

			while ( pCurrChild != nil )
			{
				//this will only work if no two or more children have the same ref number
				if ( pCurrChild->fRefNum == inRefNum )
				{
					// Remove this node
					if ( i == 1 )
					{
						// Link the next node to the base pointer
						pParentRef->fChildren = pCurrChild->fNext;
					}
					else
					{
						// Link the next node to the previous node
						pPrevChild->fNext = pCurrChild->fNext;
					}

					free( pCurrChild );
					pCurrChild = nil;
					break;
				}
				pPrevChild = pCurrChild;
				pCurrChild = pCurrChild->fNext;

				i++;
			}
		}
	}

	catch( SInt32 err )
	{
		dsResult = (tDirStatus)err;
	}

	fTableMutex.SignalLock();

	return( dsResult );

} // UnlinkFromParent


//------------------------------------------------------------------------------------
//	* GetReference
//------------------------------------------------------------------------------------

tDirStatus CDSRefTable::GetReference ( UInt32 inRefNum, sFWRefEntry **outRefData )
{
	tDirStatus		dsResult		= eDSNoErr;
	sFWRefEntry	   *pCurrRef		= nil;

	fTableMutex.WaitLock();

	try
	{
		pCurrRef = GetTableRef( inRefNum );
		if ( pCurrRef == nil ) throw( (SInt32)eDSInvalidReference );

		*outRefData = pCurrRef;
	}

	catch( SInt32 err )
	{
		dsResult = (tDirStatus)err;
	}

	fTableMutex.SignalLock();

	return( dsResult );

} // GetReference


//------------------------------------------------------------------------------------
//	* RemoveRef
//------------------------------------------------------------------------------------

tDirStatus CDSRefTable::RemoveRef ( UInt32 inRefNum, UInt32 inType, SInt32 inPID )
{
	tDirStatus		dsResult		= eDSNoErr;
	sFWRefEntry	   *pCurrRef		= nil;
	sRefFWTable	   *pTable			= nil;
	UInt32			uiSlot			= 0;
	UInt32			uiTableNum		= (inRefNum & 0xFF000000) >> 24;
	UInt32			uiRefNum		= (inRefNum & 0x0000FFFF);

	fTableMutex.WaitLock();

	try
	{
		dsResult = VerifyReference( inRefNum, inType, inPID );

		if ( dsResult == eDSNoErr )
		{
			pTable = GetThisTable( uiTableNum );
			if ( pTable == nil ) throw( (SInt32)eDSInvalidReference );

			uiSlot = uiRefNum % kMaxFWTableItems;

			if ( inType != eDirectoryRefType ) // API refs have no parents
			{
				dsResult = UnlinkFromParent( inRefNum );
				if ( dsResult != eDSNoErr ) throw( (SInt32)dsResult );
			}

			pCurrRef = GetTableRef( inRefNum ); // getting this sFWRefEntry is the same path used by uiSlot and pTable above
			if ( pCurrRef == nil ) throw( (SInt32)eDSInvalidReference );
			if ( inType != pCurrRef->fType ) throw( (SInt32)eDSInvalidReference );

			if ( pCurrRef->fChildren != nil )
			{
				fTableMutex.SignalLock();
				RemoveChildren( pCurrRef->fChildren, inPID );
				fTableMutex.WaitLock();
			}

			// always remove the slot since we don't have any child PIDs for CSBP
			if ( pTable->fTableData[ uiSlot ] != nil )
			{
				if ( uiRefNum == pTable->fTableData[ uiSlot ]->fRefNum )
				{
					// need to callback to make sure that the plug-ins clean up
					pCurrRef = pTable->fTableData[ uiSlot ];
					pTable->fTableData[ uiSlot ] = nil;
					pTable->fItemCnt--;

					//keep track of the total ref count here
					fRefCount--;
					
					if ( (pCurrRef->fBufTag == 'DbgA') || (pCurrRef->fBufTag == 'DbgB') )
					{
						if (inType == eAttrListRefType)
						{
							syslog(LOG_CRIT, "DS:dsCloseAttributeList:CDSRefTable::RemoveAttrListRef ref = %d", inRefNum);
						}
						else if (inType == eAttrValueListRefType)
						{
							syslog(LOG_CRIT, "DS:dsCloseAttributeValueList:CDSRefTable::RemoveAttrValueRef ref = %d", inRefNum);
						}
					}
					
					free(pCurrRef);
					pCurrRef = nil;
				}
			}
		}
	}

	catch( SInt32 err )
	{
		dsResult = (tDirStatus)err;
	}

	fTableMutex.SignalLock();
	
	return( dsResult );

} // RemoveRef


//------------------------------------------------------------------------------------
//	* RemoveChildren
//------------------------------------------------------------------------------------

void CDSRefTable::RemoveChildren ( sListFWInfo *inChildList, SInt32 inPID )
{
	sListFWInfo	   *pCurrChild		= nil;
	sListFWInfo	   *pNextChild		= nil;
//	sFWRefEntry	   *pCurrRef		= nil;

	fTableMutex.WaitLock();

	try
	{
		pCurrChild = inChildList;

		//walk the child list of refs
		while ( pCurrChild != nil )
		{
			//we need to get the next entry first before RemoveRef since it will call UnlinkParent
			//which in turn will delete the current entry out from under us
			pNextChild = pCurrChild->fNext;

			//remove ref if it matches the inPID
			if ( pCurrChild->fPID == inPID )
			{
				fTableMutex.SignalLock();
				RemoveRef( pCurrChild->fRefNum, pCurrChild->fType, inPID );
				fTableMutex.WaitLock();
			}

			pCurrChild = pNextChild;
		}
	}

	catch( SInt32 err )
	{
	}

	fTableMutex.SignalLock();

} // RemoveChildren

//------------------------------------------------------------------------------------
//	* GetOffset
//------------------------------------------------------------------------------------

tDirStatus CDSRefTable::GetOffset ( UInt32 inRefNum, UInt32 inType, UInt32* outOffset, SInt32 inPID )
{
	tDirStatus		siResult	= eDSDirSrvcNotOpened;
	sFWRefEntry	   *pCurrRef	= nil;

	siResult = VerifyReference( inRefNum, inType, inPID );
	
	if (siResult == eDSNoErr)
	{
		pCurrRef = GetTableRef( inRefNum );
		
		siResult = eDSInvalidReference;
		if ( pCurrRef != nil )
		{
			*outOffset = pCurrRef->fOffset;
			siResult = eDSNoErr;
		}
	}
    
	return( siResult );

} // GetOffset

//------------------------------------------------------------------------------------
//	* SetOffset
//------------------------------------------------------------------------------------

tDirStatus CDSRefTable::SetOffset ( UInt32 inRefNum, UInt32 inType, UInt32 inOffset, SInt32 inPID )
{
	tDirStatus		siResult	= eDSDirSrvcNotOpened;
	sFWRefEntry	   *pCurrRef	= nil;

	siResult = VerifyReference( inRefNum, inType, inPID );
	
	if (siResult == eDSNoErr)
	{
		pCurrRef = GetTableRef( inRefNum );
		
		siResult = eDSInvalidReference;
		if ( pCurrRef != nil )
		{
			pCurrRef->fOffset = inOffset;
			siResult = eDSNoErr;
		}
	}
    
	return( siResult );

} // SetOffset

//------------------------------------------------------------------------------------
//	* GetBufTag
//------------------------------------------------------------------------------------

tDirStatus CDSRefTable::GetBufTag ( UInt32 inRefNum, UInt32 inType, UInt32* outBufTag, SInt32 inPID )
{
	tDirStatus		siResult	= eDSDirSrvcNotOpened;
	sFWRefEntry	   *pCurrRef	= nil;

	siResult = VerifyReference( inRefNum, inType, inPID );
	
	if (siResult == eDSNoErr)
	{
		pCurrRef = GetTableRef( inRefNum );
		
		siResult = eDSInvalidReference;
		if ( pCurrRef != nil )
		{
			*outBufTag = pCurrRef->fBufTag;
			siResult = eDSNoErr;
		}
	}
    
	return( siResult );

} // GetBufTag

//------------------------------------------------------------------------------------
//	* SetBufTag
//------------------------------------------------------------------------------------

tDirStatus CDSRefTable::SetBufTag ( UInt32 inRefNum, UInt32 inType, UInt32 inBufTag, SInt32 inPID )
{
	tDirStatus		siResult	= eDSDirSrvcNotOpened;
	sFWRefEntry	   *pCurrRef	= nil;

	siResult = VerifyReference( inRefNum, inType, inPID );
	
	if (siResult == eDSNoErr)
	{
		pCurrRef = GetTableRef( inRefNum );
		
		siResult = eDSInvalidReference;
		if ( pCurrRef != nil )
		{
			pCurrRef->fBufTag = inBufTag;
			if ( (inBufTag == 'DbgA') || (inBufTag == 'DbgB') )
			{
				if ( (inType == eAttrListRefType) || (inType == eAttrValueListRefType) )
				{
					UInt32 aRefCount = GetRefCount();
					if ( ((aRefCount % 25) == 0) && (aRefCount > 100) )
					{
						syslog(LOG_CRIT, "DS:CDSRefTable::Client Side Ref Count Exceeding Reasonable Value - Ref Count = %d", aRefCount);
					}
				}
			}
			siResult = eDSNoErr;
		}
	}
    
	return( siResult );

} // SetBufTag

//------------------------------------------------------------------------------------
//	* GetRefCount
//------------------------------------------------------------------------------------

UInt32 CDSRefTable::GetRefCount ( void )
{
	return( fRefCount );

} // GetRefCount

