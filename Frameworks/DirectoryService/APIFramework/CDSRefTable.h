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
 */

#ifndef __CDSRefTable_h__
#define	__CDSRefTable_h__		1

#include "DirServicesTypes.h"
#include "PrivateTypes.h"
#include "DSMutexSemaphore.h"

#define		kMaxFWTableItems	512
#define		kMaxFWTables		0x0F

typedef struct sListFWInfo *sListFWInfoPtr;

//note fPID defined everywhere as SInt32 to remove warnings of comparing -1 to UInt32 since failed PID is -1

//struct to contain reference to the actual ref entry of the child ref
typedef struct sListFWInfo {
	UInt32			fRefNum;
	UInt32			fType;
	SInt32			fPID;
	sListFWInfoPtr	fNext;
} sListFWInfo;

//struct to contain PID of the child client process granted access to a ref from the parent client process
typedef struct sPIDFWInfo {
	SInt32			fPID;
	sPIDFWInfo	   *fNext;
} sPIDFWInfo;

//struct of the main ref entry
typedef struct sFWRefEntry {
	UInt32			fRefNum;
	UInt32			fType;
    UInt32			fOffset;
	UInt32			fBufTag;
	UInt32			fParentID;
	SInt32			fPID;
	sListFWInfo	   *fChildren;
	sPIDFWInfo	   *fChildPID;
} sFWRefEntry;

// -------------------------------------------

typedef struct sRefFWTable *sRefFWTablePtr;

typedef struct sRefFWTable {
	UInt32			fTableNum;
	UInt32			fCurRefNum;
	UInt32			fItemCnt;
	sFWRefEntry		*fTableData[ kMaxFWTableItems ];
} sRefFWTable;

//------------------------------------------------------------------------------------
//	* CDSRefTable
//------------------------------------------------------------------------------------

class CDSRefTable
{
public:
				CDSRefTable			( void );
				~CDSRefTable		( void );
	
	void		ClearAllTables		( void );

	tDirStatus	VerifyDirRef		( tDirReference inDirRef, SInt32 inPID );
	tDirStatus	VerifyNodeRef		( tDirNodeReference inDirNodeRef, SInt32 inPID );
	tDirStatus	VerifyRecordRef		( tRecordReference inRecordRef, SInt32 inPID );
	tDirStatus	VerifyAttrListRef	( tAttributeListRef inAttributeListRef, SInt32 inPID );
	tDirStatus	VerifyAttrValueRef	( tAttributeValueListRef inAttributeValueListRef, SInt32 inPID );

	tDirStatus	NewDirRef			( UInt32 *outNewRef, SInt32 inPID );
	tDirStatus	NewNodeRef			( UInt32 *outNewRef, UInt32 inParentID, SInt32 inPID );
	tDirStatus	NewRecordRef		( UInt32 *outNewRef, UInt32 inParentID, SInt32 inPID );
	tDirStatus	NewAttrListRef		( UInt32 *outNewRef, UInt32 inParentID, SInt32 inPID );
	tDirStatus	NewAttrValueRef		( UInt32 *outNewRef, UInt32 inParentID, SInt32 inPID );

	tDirStatus	RemoveDirRef		( UInt32 inDirRef, SInt32 inPID );
	tDirStatus	RemoveNodeRef		( UInt32 inNodeRef, SInt32 inPID );
	tDirStatus	RemoveRecordRef		( UInt32 inRecRef, SInt32 inPID );
	tDirStatus	RemoveAttrListRef	( UInt32 inAttrListRef, SInt32 inPID );
	tDirStatus	RemoveAttrValueRef	( UInt32 InAttrValueRef, SInt32 inPID );

    tDirStatus	GetOffset			( UInt32 inRefNum, UInt32 inType, UInt32* outOffset, SInt32 inPID );
    tDirStatus	SetOffset			( UInt32 inRefNum, UInt32 inType, UInt32 inOffset, SInt32 inPID );
    tDirStatus	GetBufTag			( UInt32 inRefNum, UInt32 inType, UInt32* outBufTag, SInt32 inPID );
    tDirStatus	SetBufTag			( UInt32 inRefNum, UInt32 inType, UInt32 inBufTag, SInt32 inPID );

private:
	DSMutexSemaphore	fTableMutex;
	UInt32				fTableCount;
	sRefFWTable			*fRefTables[ kMaxFWTables + 1 ];	//added 1 since table is 1-based and code depends upon having that last
															//index in as kMaxFWTables ie. note array is 0-based
	UInt32				fRefCount;
	
private:
	tDirStatus	VerifyReference		( tDirReference inDirRef, UInt32 inType, SInt32 inPID );
	tDirStatus	GetNewRef			( UInt32 *outRef, UInt32 inParentID, eRefTypes inType, SInt32 inPID );
	tDirStatus	RemoveRef			( UInt32 inRefNum, UInt32 inType, SInt32 inPID );

	tDirStatus	GetReference		( UInt32 inRefNum, sFWRefEntry **outRefData );

	tDirStatus	LinkToParent		( UInt32 inRefNum, UInt32 inType, UInt32 inParentID, SInt32 inPID );
	tDirStatus	UnlinkFromParent	( UInt32 inRefNum );

	void		RemoveChildren		( sListFWInfo *inChildList, SInt32 inPID );

	sRefFWTable*	GetNextTable	( sRefFWTable *inCurTable );
	sRefFWTable*	GetThisTable	( UInt32 inTableNum );

	sFWRefEntry*	GetTableRef		( UInt32 inRefNum );

	UInt32			GetRefCount		( void );
};

#endif

