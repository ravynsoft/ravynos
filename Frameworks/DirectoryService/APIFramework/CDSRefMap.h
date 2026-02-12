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

#ifndef __CDSRefMap_h__
#define	__CDSRefMap_h__		1

#include "DirServicesTypes.h"
#include "DSMutexSemaphore.h"
#include "PrivateTypes.h"
#include "CDSRefTable.h"

//many constants, enums, and typedefs used from CDSRefTable.h
//KW: at some point need a base class that can handle this and the other two derived ones
//ie. complete with tracking of statistics using STL base containers?

//struct of the main ref entry
typedef struct sFWRefMapEntry {
	UInt32			fRefNum;
	UInt32			fType;
    UInt32			fRemoteRefNum;
	UInt32			fParentID;
	SInt32			fPID;
	sListFWInfo	   *fChildren;
	sPIDFWInfo	   *fChildPID;
	UInt32			fMessageTableIndex;
	char		   *fPluginName;
	bool			fBigEndian;
} sFWRefMapEntry;

// -------------------------------------------

typedef struct sRefMapTable *sRefMapTablePtr;

typedef struct sRefMapTable {
	UInt32			fTableNum;
	UInt32			fCurRefNum;
	UInt32			fItemCnt;
	sFWRefMapEntry *fTableData[ kMaxFWTableItems ];
} sRefMapTable;

//------------------------------------------------------------------------------------
//	* CDSRefMap
//------------------------------------------------------------------------------------

class CDSRefMap
{
public:
				CDSRefMap			( void );
				~CDSRefMap			( void );
	
	void		ClearAllMaps		( void );
	
	tDirStatus	VerifyDirRef		( tDirReference inDirRef, SInt32 inPID );
	tDirStatus	VerifyNodeRef		( tDirNodeReference inDirNodeRef, SInt32 inPID );
	tDirStatus	VerifyRecordRef		( tRecordReference inRecordRef, SInt32 inPID );
	tDirStatus	VerifyAttrListRef	( tAttributeListRef inAttributeListRef, SInt32 inPID );
	tDirStatus	VerifyAttrValueRef	( tAttributeValueListRef inAttributeValueListRef, SInt32 inPID );

	tDirStatus	NewDirRefMap		( UInt32 *outNewRef, SInt32 inPID, UInt32 serverRef, UInt32 messageIndex );
	tDirStatus	NewNodeRefMap		( UInt32 *outNewRef, UInt32 inParentID, SInt32 inPID, UInt32 serverRef, UInt32 messageIndex, char* inPluginName );
	tDirStatus	NewRecordRefMap		( UInt32 *outNewRef, UInt32 inParentID, SInt32 inPID, UInt32 serverRef, UInt32 messageIndex );
	tDirStatus	NewAttrListRefMap	( UInt32 *outNewRef, UInt32 inParentID, SInt32 inPID, UInt32 serverRef, UInt32 messageIndex );
	tDirStatus	NewAttrValueRefMap	( UInt32 *outNewRef, UInt32 inParentID, SInt32 inPID, UInt32 serverRef, UInt32 messageIndex );

	tDirStatus	RemoveDirRef		( UInt32 inDirRef, SInt32 inPID );
	tDirStatus	RemoveNodeRef		( UInt32 inNodeRef, SInt32 inPID );
	tDirStatus	RemoveRecordRef		( UInt32 inRecRef, SInt32 inPID );
	tDirStatus	RemoveAttrListRef	( UInt32 inAttrListRef, SInt32 inPID );
	tDirStatus	RemoveAttrValueRef	( UInt32 InAttrValueRef, SInt32 inPID );

	UInt32		GetMessageTableIndex( UInt32 inRefNum, UInt32 inType, SInt32 inPID );
	tDirStatus	SetMessageTableIndex( UInt32 inRefNum, UInt32 inType, UInt32 inMsgTableIndex, SInt32 inPID );

	char*		GetPluginName		( UInt32 inRefNum, SInt32 inPID );

	UInt32		GetRefNum			( UInt32 inRefNum, UInt32 inType, SInt32 inPID );
	
private:
	DSMutexSemaphore	fMapMutex;
	UInt32				fTableCount;
	sRefMapTable		*fRefMapTables[ kMaxFWTables + 1 ];	//added 1 since table is 1-based and code depends upon having that last
															//index in as kMaxFWTables ie. note array is 0-based
private:
	tDirStatus		VerifyReference		( tDirReference inDirRef, UInt32 inType, SInt32 inPID );
	tDirStatus		GetNewRef			( UInt32 *outRef, UInt32 inParentID, eRefTypes inType, SInt32 inPID, UInt32 serverRef, UInt32 messageIndex );
	tDirStatus		RemoveRef			( UInt32 inRefNum, UInt32 inType, SInt32 inPID );

	tDirStatus		GetReference		( UInt32 inRefNum, sFWRefMapEntry **outRefData );

	tDirStatus		LinkToParent		( UInt32 inRefNum, UInt32 inType, UInt32 inParentID, SInt32 inPID );
	tDirStatus		UnlinkFromParent	( UInt32 inRefNum );

	void			RemoveChildren		( sListFWInfo *inChildList, SInt32 inPID );

	sRefMapTable*	GetNextTable		( sRefMapTable *inCurTable );
	sRefMapTable*	GetThisTable		( UInt32 inTableNum );

	sFWRefMapEntry*	GetTableRef			( UInt32 inRefNum );
};


#endif

