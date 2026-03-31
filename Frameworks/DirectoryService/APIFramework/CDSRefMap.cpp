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
 * @header CDSRefMap
 * DirectoryService Framework reference map table to references of
 * all TCP connected "remote" DirectoryService daemons.
 * References here always use 0x00C00000 bits
 */

#include "CDSRefMap.h"

#include <stdlib.h>
#include <string.h>
#include <libkern/OSAtomic.h>

//------------------------------------------------------------------------------------
//	* CDSRefMap
//------------------------------------------------------------------------------------

CDSRefMap::CDSRefMap ( void ) : fMapMutex( "CDSRefMap::fMapMutex" )
{
	fTableCount		= 0;
	memset( fRefMapTables, 0, sizeof( fRefMapTables ) );
} // CDSRefMap


//------------------------------------------------------------------------------------
//	* ~CDSRefMap
//------------------------------------------------------------------------------------

CDSRefMap::~CDSRefMap ( void )
{
	ClearAllMaps();
} // ~CDSRefMap

void CDSRefMap::ClearAllMaps( void )
{
	UInt32		i	= 1;
	UInt32		j	= 1;

	fMapMutex.WaitLock();

	for ( i = 1; i <= kMaxFWTables; i++ )	//array is still zero based even if first entry NOT used
											//-- added the last kMaxTable in the .h file so this should work now
	{
		if ( fRefMapTables[ i ] != nil )
		{
			for (j=0; j< kMaxFWTableItems; j++)
			{
				if (fRefMapTables[ i ]->fTableData[j] != nil)
				{
					free(fRefMapTables[ i ]->fTableData[j]);
					fRefMapTables[ i ]->fTableData[j] = nil;
				}
			}
			free( fRefMapTables[ i ] );  //free all the calloc'ed sRefEntries inside as well - code above
			fRefMapTables[ i ] = nil;
			fTableCount--;
		}
	}
	
	fMapMutex.SignalLock();
}

//--------------------------------------------------------------------------------------------------
//	* VerifyReference
//
//--------------------------------------------------------------------------------------------------

tDirStatus CDSRefMap::VerifyReference (	tDirReference	inDirRef, //should be generic ref here
										UInt32			inType,
										SInt32			inPID )
{
	tDirStatus		siResult	= eDSInvalidReference;
	sFWRefMapEntry *refData		= nil;
	sPIDFWInfo	   *pPIDInfo	= nil;
	
	if ((inDirRef & 0x00C00000) != 0)
	{
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
	}

	return( siResult );

} // VerifyReference



//------------------------------------------------------------------------------------
//	* VerifyDirRef (static)
//
//------------------------------------------------------------------------------------

tDirStatus CDSRefMap::VerifyDirRef (	tDirReference		inDirRef,
										SInt32				inPID )
{
	tDirStatus		siResult	= eDSDirSrvcNotOpened;

	siResult = VerifyReference( inDirRef, eDirectoryRefType, inPID );

	return( siResult );

} // VerifyDirRef



//------------------------------------------------------------------------------------
//	* VerifyNodeRef (static)
//
//------------------------------------------------------------------------------------

tDirStatus CDSRefMap::VerifyNodeRef (	tDirNodeReference	inDirNodeRef,
										SInt32				inPID )
{
	tDirStatus		siResult	= eDSDirSrvcNotOpened;

	siResult = VerifyReference( inDirNodeRef, eNodeRefType, inPID );

	return( siResult );

} // VerifyNodeRef


//------------------------------------------------------------------------------------
//	* VerifyRecordRef (static)
//
//------------------------------------------------------------------------------------

tDirStatus CDSRefMap::VerifyRecordRef (	tRecordReference	inRecordRef,
                                            SInt32				inPID )
{
	tDirStatus		siResult	= eDSDirSrvcNotOpened;

	siResult = VerifyReference( inRecordRef, eRecordRefType, inPID );

	return( siResult );

} // VerifyRecordRef



//------------------------------------------------------------------------------------
//	* VerifyAttrListRef (static)
//
//------------------------------------------------------------------------------------

tDirStatus CDSRefMap::VerifyAttrListRef (	tAttributeListRef	inAttributeListRef,
											SInt32				inPID )
{
	tDirStatus		siResult	= eDSDirSrvcNotOpened;

	siResult = VerifyReference( inAttributeListRef, eAttrListRefType, inPID );

	return( siResult );

} // VerifyAttrListRef


//------------------------------------------------------------------------------------
//	* VerifyAttrValueRef (static)
//
//------------------------------------------------------------------------------------

tDirStatus CDSRefMap::VerifyAttrValueRef (	tAttributeValueListRef	inAttributeValueListRef,
                                                SInt32					inPID )
{
	tDirStatus		siResult	= eDSDirSrvcNotOpened;

	siResult = VerifyReference( inAttributeValueListRef, eAttrValueListRefType, inPID );

	return( siResult );

} // VerifyAttrValueRef


//------------------------------------------------------------------------------------
//	* NewDirRef
//------------------------------------------------------------------------------------

tDirStatus CDSRefMap::NewDirRefMap ( UInt32 *outNewRef, SInt32 inPID, UInt32 serverRef,
									 UInt32 messageIndex )
{
	tDirStatus		siResult	= eDSDirSrvcNotOpened;

	siResult = GetNewRef( outNewRef, 0, eDirectoryRefType, inPID, serverRef, messageIndex );
	
	return( siResult );

} // NewDirRef


//------------------------------------------------------------------------------------
//	* NewNodeRefMap
//------------------------------------------------------------------------------------

tDirStatus CDSRefMap::NewNodeRefMap (	UInt32			*outNewRef,
										UInt32			inParentID,
										SInt32			inPID,
										UInt32			serverRef,
										UInt32			messageIndex,
										char		   *inPluginName)
{
	tDirStatus		siResult	= eDSDirSrvcNotOpened;

	siResult = GetNewRef( outNewRef, inParentID, eNodeRefType, inPID, serverRef, messageIndex );
	if (siResult == eDSNoErr)
	{
		sFWRefMapEntry *pCurrRef = GetTableRef( *outNewRef );
		if ( pCurrRef != NULL )
		{
			pCurrRef->fPluginName = inPluginName;
		}
	}

	return( siResult );

} // NewNodeRefMap


//------------------------------------------------------------------------------------
//	* NewRecordRef
//------------------------------------------------------------------------------------

tDirStatus CDSRefMap::NewRecordRefMap (	UInt32			*outNewRef,
										UInt32			inParentID,
										SInt32			inPID,
											UInt32		serverRef,
											UInt32		messageIndex )
{
	tDirStatus		siResult	= eDSDirSrvcNotOpened;

	siResult = GetNewRef( outNewRef, inParentID, eRecordRefType, inPID, serverRef, messageIndex );

	return( siResult );

} // NewRecordRef


//------------------------------------------------------------------------------------
//	* NewAttrListRef
//------------------------------------------------------------------------------------

tDirStatus CDSRefMap::NewAttrListRefMap (	UInt32			*outNewRef,
											UInt32			inParentID,
											SInt32			inPID,
											UInt32		serverRef,
											UInt32		messageIndex )
{
	tDirStatus		siResult	= eDSDirSrvcNotOpened;

	siResult = GetNewRef( outNewRef, inParentID, eAttrListRefType, inPID, serverRef, messageIndex );

	return( siResult );

} // NewAttrListRef


//------------------------------------------------------------------------------------
//	* NewAttrValueRef
//------------------------------------------------------------------------------------

tDirStatus CDSRefMap::NewAttrValueRefMap (	UInt32			*outNewRef,
											UInt32			inParentID,
											SInt32			inPID,
											UInt32		serverRef,
											UInt32		messageIndex )
{
	tDirStatus		siResult	= eDSDirSrvcNotOpened;

	siResult = GetNewRef( outNewRef, inParentID, eAttrValueListRefType, inPID, serverRef, messageIndex );

	return( siResult );

} // NewAttrValueRef


//------------------------------------------------------------------------------------
//	* RemoveDirRef
//------------------------------------------------------------------------------------

tDirStatus CDSRefMap::RemoveDirRef ( UInt32 inDirRef, SInt32 inPID )
{
	tDirStatus		siResult	= eDSDirSrvcNotOpened;

	siResult = RemoveRef( inDirRef, eDirectoryRefType, inPID );

	return( siResult );

} // RemoveDirRef


//------------------------------------------------------------------------------------
//	* RemoveNodeRef
//------------------------------------------------------------------------------------

tDirStatus CDSRefMap::RemoveNodeRef ( UInt32 inNodeRef, SInt32 inPID )
{
	tDirStatus		siResult	= eDSDirSrvcNotOpened;

	siResult = RemoveRef( inNodeRef, eNodeRefType, inPID );

	return( siResult );

} // RemoveNodeRef


//------------------------------------------------------------------------------------
//	* RemoveRecordRef
//------------------------------------------------------------------------------------

tDirStatus CDSRefMap::RemoveRecordRef ( UInt32 inRecRef, SInt32 inPID )
{
	tDirStatus		siResult	= eDSDirSrvcNotOpened;

	siResult = RemoveRef( inRecRef, eRecordRefType, inPID );

	return( siResult );

} // RemoveRecordRef


//------------------------------------------------------------------------------------
//	* RemoveAttrListRef
//------------------------------------------------------------------------------------

tDirStatus CDSRefMap::RemoveAttrListRef ( UInt32 inAttrListRef, SInt32 inPID )
{
	tDirStatus		siResult	= eDSDirSrvcNotOpened;

	siResult = RemoveRef( inAttrListRef, eAttrListRefType, inPID );

	return( siResult );

} // RemoveAttrListRef


//------------------------------------------------------------------------------------
//	* RemoveAttrValueRef
//------------------------------------------------------------------------------------

tDirStatus CDSRefMap::RemoveAttrValueRef ( UInt32 inAttrValueRef, SInt32 inPID )
{
	tDirStatus		siResult	= eDSDirSrvcNotOpened;

	siResult = RemoveRef( inAttrValueRef, eAttrValueListRefType, inPID );

	return( siResult );

} // RemoveAttrValueRef


//------------------------------------------------------------------------------------
//	* GetTableRef
//------------------------------------------------------------------------------------

sFWRefMapEntry* CDSRefMap::GetTableRef ( UInt32 inRefNum )
{
	UInt32			uiSlot			= 0;
	UInt32			uiRefNum		= (inRefNum & 0x00FFFFFF);
	UInt32			uiTableNum		= (inRefNum & 0xFF000000) >> 24;
	sRefMapTable	   *pTable			= nil;
	sFWRefMapEntry	   *pOutEntry		= nil;

	fMapMutex.WaitLock();

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

	fMapMutex.SignalLock();

	return( pOutEntry );

} // GetTableRef


//------------------------------------------------------------------------------------
//	* GetNextTable
//------------------------------------------------------------------------------------

sRefMapTable* CDSRefMap::GetNextTable ( sRefMapTable *inTable )
{
	UInt32		uiTblNum	= 0;
	sRefMapTable	*pOutTable	= nil;

	fMapMutex.WaitLock();

	try
	{
		if ( inTable == nil )
		{
			// Get the first reference table, create it if it wasn't already
			if ( fRefMapTables[ 1 ] == nil )
			{
				// No tables have been allocated yet so lets make one
				fRefMapTables[ 1 ] = (sRefMapTable *)calloc( sizeof( sRefMapTable ), sizeof( char ) );
				if ( fRefMapTables[ 1 ] == nil ) throw((SInt32)eMemoryAllocError);

				fRefMapTables[ 1 ]->fTableNum = 1;

				fTableCount = 1;
			}

			pOutTable = fRefMapTables[ 1 ];
		}
		else
		{
			uiTblNum = inTable->fTableNum + 1;
			if (uiTblNum > kMaxFWTables) throw( (SInt32)eDSInvalidReference );

			if ( fRefMapTables[ uiTblNum ] == nil )
			{
				// Set the table counter
				fTableCount = uiTblNum;

				// No tables have been allocated yet so lets make one
				fRefMapTables[ uiTblNum ] = (sRefMapTable *)calloc( sizeof( sRefMapTable ), sizeof( char ) );
				if( fRefMapTables[ uiTblNum ] == nil ) throw((SInt32)eMemoryAllocError);

				if (uiTblNum == 0) throw( (SInt32)eDSInvalidReference );
				fRefMapTables[ uiTblNum ]->fTableNum = uiTblNum;
			}

			pOutTable = fRefMapTables[ uiTblNum ];
		}
	}

	catch( SInt32 err )
	{
	}

	fMapMutex.SignalLock();

	return( pOutTable );

} // GetNextTable


//------------------------------------------------------------------------------------
//	* GetThisTable
//------------------------------------------------------------------------------------

sRefMapTable* CDSRefMap::GetThisTable ( UInt32 inTableNum )
{
	sRefMapTable	*pOutTable	= nil;

	fMapMutex.WaitLock();

	pOutTable = fRefMapTables[ inTableNum ];

	fMapMutex.SignalLock();

	return( pOutTable );

} // GetThisTable


//------------------------------------------------------------------------------------
//	* GetNewRef
//------------------------------------------------------------------------------------

tDirStatus CDSRefMap::GetNewRef (	UInt32		   *outRef,
									UInt32			inParentID,
									eRefTypes		inType,
									SInt32			inPID,
									UInt32			serverRef,
									UInt32			messageIndex )
{
	bool			done		= false;
	tDirStatus		outResult	= eDSNoErr;
	sRefMapTable	   *pCurTable	= nil;
	UInt32			uiRefNum	= 0;
	UInt32			uiCntr		= 0;
	UInt32			uiSlot		= 0;
	UInt32			uiTableNum	= 0;

	fMapMutex.WaitLock();

	try
	{
		*outRef = 0;

		while ( !done )
		{
			pCurTable = GetNextTable( pCurTable );
			if ( pCurTable == nil ) throw( (SInt32)eDSRefTableFWAllocError );

			if ( pCurTable->fItemCnt < kMaxFWTableItems )
			{
				uiCntr = 0;
				uiTableNum = pCurTable->fTableNum;
				while ( (uiCntr < kMaxFWTableItems) && !done )	//KW80 - uiCntr was a condition never used
																//fixed below with uiCntr++; code addition
				{
					if ( (pCurTable->fCurRefNum == 0) || 
						 (pCurTable->fCurRefNum > 0x000FFFFF) )
					{
						// Either it's the first reference for this table or we have
						//	used 1024*1024 references and need to start over
						pCurTable->fCurRefNum = 1;
					}

					// Get the ref num and increment its place holder
					uiRefNum = pCurTable->fCurRefNum++;
					uiRefNum |= 0x00C00000;

					// Find a slot in the table for this ref number
					uiSlot = uiRefNum % kMaxFWTableItems;
					if ( pCurTable->fTableData[ uiSlot ] == nil )
					{
						pCurTable->fTableData[ uiSlot ] = (sFWRefMapEntry *)calloc( sizeof( sFWRefMapEntry ), sizeof( char ) );
						if ( pCurTable->fTableData[ uiSlot ] == nil ) throw( (SInt32)eDSRefTableFWAllocError );
						
						// We found an empty slot, now set this table entry
						pCurTable->fTableData[ uiSlot ]->fRefNum		= uiRefNum;
						pCurTable->fTableData[ uiSlot ]->fType			= inType;
						pCurTable->fTableData[ uiSlot ]->fParentID		= inParentID;
						pCurTable->fTableData[ uiSlot ]->fPID			= inPID;
						pCurTable->fTableData[ uiSlot ]->fRemoteRefNum	= serverRef;
						pCurTable->fTableData[ uiSlot ]->fChildren		= nil;
						pCurTable->fTableData[ uiSlot ]->fChildPID		= nil;
						pCurTable->fTableData[ uiSlot ]->fMessageTableIndex = messageIndex;

						// Add the table number to the reference number
						uiTableNum = (uiTableNum << 24);
						uiRefNum = uiRefNum | uiTableNum;

						*outRef = uiRefNum;

						// Up the item count
						pCurTable->fItemCnt++;

						outResult = eDSNoErr;
						done = true;
					}
					uiCntr++;	//KW80 needed for us to only go through the table once
								//ie the uiCntr does not get used directly BUT the uiRefNum gets
								//incremented only kMaxFWTableItems times since uiCntr is in the while condition
				}
			}
		}

		if ( inParentID != 0 )
		{
			outResult = LinkToParent( *outRef, inType, inParentID, inPID );
		}
	}

	catch( SInt32 err )
	{
		outResult = (tDirStatus)err;
	}

	fMapMutex.SignalLock();

	return( outResult );

} // GetNewRef


//------------------------------------------------------------------------------------
//	* LinkToParent
//------------------------------------------------------------------------------------

tDirStatus CDSRefMap::LinkToParent ( UInt32 inRefNum, UInt32 inType, UInt32 inParentID, SInt32 inPID )
{
	tDirStatus		dsResult		= eDSNoErr;
	sFWRefMapEntry	   *pCurrRef		= nil;
	sListFWInfo	   *pChildInfo		= nil;

	fMapMutex.WaitLock();

	try
	{
		pCurrRef = GetTableRef( inParentID );
		if ( pCurrRef == nil ) throw( (SInt32)eDSInvalidReference );

		// This is the one we want
		pChildInfo = (sListFWInfo *)calloc( sizeof( sListFWInfo ), sizeof( char ) );
		if ( pChildInfo == nil ) throw( (SInt32)eDSRefTableFWAllocError );

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

	fMapMutex.SignalLock();

	return( dsResult );

} // LinkToParent


//------------------------------------------------------------------------------------
//	* UnlinkFromParent
//------------------------------------------------------------------------------------

tDirStatus CDSRefMap::UnlinkFromParent ( UInt32 inRefNum )
{
	tDirStatus		dsResult		= eDSNoErr;
	UInt32			i				= 1;
	UInt32			parentID		= 0;
	sFWRefMapEntry	   *pCurrRef		= nil;
	sFWRefMapEntry	   *pParentRef		= nil;
	sListFWInfo	   *pCurrChild		= nil;
	sListFWInfo	   *pPrevChild		= nil;

	fMapMutex.WaitLock();

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

	fMapMutex.SignalLock();

	return( dsResult );

} // UnlinkFromParent


//------------------------------------------------------------------------------------
//	* GetReference
//------------------------------------------------------------------------------------

tDirStatus CDSRefMap::GetReference ( UInt32 inRefNum, sFWRefMapEntry **outRefData )
{
	tDirStatus		dsResult		= eDSNoErr;
	sFWRefMapEntry	   *pCurrRef		= nil;

	fMapMutex.WaitLock();

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

	fMapMutex.SignalLock();

	return( dsResult );

} // GetReference


//------------------------------------------------------------------------------------
//	* RemoveRef
//------------------------------------------------------------------------------------

tDirStatus CDSRefMap::RemoveRef ( UInt32 inRefNum, UInt32 inType, SInt32 inPID )
{
	tDirStatus		dsResult		= eDSNoErr;
	sFWRefMapEntry	   *pCurrRef		= nil;
	sRefMapTable	   *pTable			= nil;
	UInt32			uiSlot			= 0;
	UInt32			uiTableNum		= (inRefNum & 0xFF000000) >> 24;
	UInt32			uiRefNum		= (inRefNum & 0x00FFFFFF);
	bool			doFree			= false;
	sPIDFWInfo	   *pPIDInfo		= nil;
	sPIDFWInfo	   *pPrevPIDInfo	= nil;


	fMapMutex.WaitLock();

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

			pCurrRef = GetTableRef( inRefNum );	//KW80 - here we need to know where the sFWRefMapEntry is in the table to delete it
												//that is all the code added above
												// getting this sFWRefMapEntry is the same path used by uiSlot and pTable above
			if ( pCurrRef == nil ) throw( (SInt32)eDSInvalidReference );
			if (inType != pCurrRef->fType) throw( (SInt32)eDSInvalidReference );

			if ( pCurrRef->fChildren != nil )
			{
				fMapMutex.SignalLock();
				RemoveChildren( pCurrRef->fChildren, inPID );
				fMapMutex.WaitLock();
			}

			//Now we check to see if this was a child or parent client PID that we removed - only applies to case of Node refs really
			if (pCurrRef->fPID == inPID)
			{
				pCurrRef->fPID = -1;
				if (pCurrRef->fChildPID == nil)
				{
					doFree = true;
				}
			}
			else
			{
				pPIDInfo		= pCurrRef->fChildPID;
				pPrevPIDInfo	= pCurrRef->fChildPID;
				//remove all child client PIDs that match since all children refs for that PID removed already above
				//ie. within scope of a PID the refs are NOT ref counted
				while (pPIDInfo != nil)
				{
					if (pPIDInfo->fPID == inPID)
					{
						//need to remove this particular PID entry
						if (pPIDInfo == pCurrRef->fChildPID)
						{
							pCurrRef->fChildPID = pCurrRef->fChildPID->fNext;
							free(pPIDInfo);
							pPIDInfo			= pCurrRef->fChildPID;
							pPrevPIDInfo		= pCurrRef->fChildPID;
						}
						else
						{
							pPrevPIDInfo->fNext = pPIDInfo->fNext;
							free(pPIDInfo);
							pPIDInfo			= pPrevPIDInfo->fNext;
						}
					}
					else
					{
						pPrevPIDInfo = pPIDInfo;
						pPIDInfo = pPIDInfo->fNext;
					}
				}
				//child client PIDs now removed so re-eval free
				if ( (pCurrRef->fPID == -1) && (pCurrRef->fChildPID == nil) )
				{
					doFree = true;
				}
			}
			
			if (doFree)
			{
				if ( pTable->fTableData[ uiSlot ] != nil )
				{
					if ( uiRefNum == pTable->fTableData[ uiSlot ]->fRefNum )
					{
						// need to callback to make sure that the plug-ins clean up
						pCurrRef = pTable->fTableData[ uiSlot ];
						pTable->fTableData[ uiSlot ] = nil;
						pTable->fItemCnt--;
						free(pCurrRef);
						pCurrRef = nil;
					}
				}
			}
		}
			
	}

	catch( SInt32 err )
	{
		dsResult = (tDirStatus)err;
	}

	fMapMutex.SignalLock();

	return( dsResult );

} // RemoveRef


//------------------------------------------------------------------------------------
//	* RemoveChildren
//------------------------------------------------------------------------------------

void CDSRefMap::RemoveChildren ( sListFWInfo *inChildList, SInt32 inPID )
{
	sListFWInfo	   *pCurrChild		= nil;
	sListFWInfo	   *pNextChild		= nil;
//	sFWRefMapEntry	   *pCurrRef		= nil;

	fMapMutex.WaitLock();

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
				fMapMutex.SignalLock();
				RemoveRef( pCurrChild->fRefNum, pCurrChild->fType, inPID );
				fMapMutex.WaitLock();
			}

			pCurrChild = pNextChild;
		}
	}

	catch( SInt32 err )
	{
	}

	fMapMutex.SignalLock();

} // RemoveChildren


//------------------------------------------------------------------------------------
//	* SetMessageTableIndex
//------------------------------------------------------------------------------------

tDirStatus CDSRefMap::SetMessageTableIndex ( UInt32 inRefNum, UInt32 inType, UInt32 inMsgTableIndex, SInt32 inPID )
{
	tDirStatus			siResult	= eDSDirSrvcNotOpened;
	sFWRefMapEntry	   *pCurrRef	= nil;

	siResult = VerifyReference( inRefNum, inType, inPID );
	
	if (siResult == eDSNoErr)
	{
		pCurrRef = GetTableRef( inRefNum );
		
		siResult = eDSInvalidReference;
		if ( pCurrRef != nil )
		{
			pCurrRef->fMessageTableIndex = inMsgTableIndex;
			siResult = eDSNoErr;
		}
	}
    
	return( siResult );

} // SetMessageTableIndex


//------------------------------------------------------------------------------------
//	* GetRefNum
//------------------------------------------------------------------------------------

UInt32 CDSRefMap::GetRefNum ( UInt32 inRefNum, UInt32 inType, SInt32 inPID )
{
	SInt32				siResult	= eDSNoErr;
	UInt32				theRefNum	= inRefNum; //return the input if not found here
	sFWRefMapEntry	   *pCurrRef	= nil;

	if ((inRefNum & 0x00C00000) != 0)
	{
		siResult = VerifyReference( inRefNum, inType, inPID );
		if (siResult == eDSNoErr)
		{
			pCurrRef = GetTableRef( inRefNum );
			if ( pCurrRef != nil )
			{
				theRefNum = pCurrRef->fRemoteRefNum;
			}
		}
	}
    
	return( theRefNum );

} // GetRefNum


//------------------------------------------------------------------------------------
//	* GetMessageTableIndex
//------------------------------------------------------------------------------------

UInt32 CDSRefMap::GetMessageTableIndex ( UInt32 inRefNum, UInt32 inType, SInt32 inPID )
{
	SInt32				siResult			= eDSNoErr;
	UInt32				theMsgTableIndex	= 0; //return zero if not remote connection
	sFWRefMapEntry	   *pCurrRef			= nil;

	if ((inRefNum & 0x00C00000) != 0)
	{
		siResult = VerifyReference( inRefNum, inType, inPID );
		if (siResult == eDSNoErr)
		{
			pCurrRef = GetTableRef( inRefNum );
			if ( pCurrRef != nil )
			{
				theMsgTableIndex = pCurrRef->fMessageTableIndex;
			}
		}
	}
    
	return( theMsgTableIndex );

} // GetMessageTableIndex


//------------------------------------------------------------------------------------
//	* GetPluginName
//------------------------------------------------------------------------------------

char* CDSRefMap::GetPluginName( UInt32 inRefNum, SInt32 inPID )
{
	SInt32				siResult			= eDSNoErr;
	sFWRefMapEntry	   *pCurrRef			= nil;
	char			   *outPluginName		= nil;

	if ((inRefNum & 0x00C00000) != 0)
	{
		siResult = VerifyReference( inRefNum, eNodeRefType, inPID );
		if (siResult == eDSNoErr)
		{
			pCurrRef = GetTableRef( inRefNum );
			if ( pCurrRef != nil )
			{
				outPluginName = pCurrRef->fPluginName;
			}
		}
	}
    
	return( outPluginName );

} // GetPluginName

