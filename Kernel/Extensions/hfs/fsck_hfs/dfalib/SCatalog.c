/*
 * Copyright (c) 1999-2000, 2002, 2007 Apple Inc. All rights reserved.
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

#include "Scavenger.h"


OSErr FlushCatalogFile( SVCB *vcb )
{
	OSErr	err;

	err = BTFlushPath(vcb->vcbCatalogFile);
	if ( err == noErr )
	{
		if( ( vcb->vcbCatalogFile->fcbFlags & fcbModifiedMask ) != 0 )
		{
			(void) MarkVCBDirty( vcb );
			err = FlushVolumeControlBlock( vcb );
		}
	}
	
	return( err );
}

OSErr LocateCatalogNode(SFCB *fcb, BTreeIterator *iterator, FSBufferDescriptor *btRecord, UInt16 *reclen)
{
	CatalogRecord * recp;
	CatalogKey * keyp;
	CatalogName * namep = NULL;
	UInt32 threadpid = 0;
	OSErr result;
	Boolean isHFSPlus = false;

	result = BTSearchRecord(fcb, iterator, kInvalidMRUCacheKey, btRecord, reclen, iterator);
	if (result == btNotFound)
		result = cmNotFound;	
	ReturnIfError(result);

	recp = (CatalogRecord *)btRecord->bufferAddress;
	keyp = (CatalogKey*)&iterator->key;

	/* if we got a thread record, then go look up real record */
	switch (recp->recordType) {
	case kHFSFileThreadRecord:
	case kHFSFolderThreadRecord:
		threadpid = recp->hfsThread.parentID;
		namep = (CatalogName *) &recp->hfsThread.nodeName;
		isHFSPlus = false;
		break;

	case kHFSPlusFileThreadRecord:
	case kHFSPlusFolderThreadRecord:
		threadpid = recp->hfsPlusThread.parentID;
		namep = (CatalogName *) &recp->hfsPlusThread.nodeName;
		isHFSPlus = true;
		break;

	default:
		threadpid = 0;
		break;
	}

	if (threadpid) {
		(void) BTInvalidateHint(iterator);
		BuildCatalogKey(threadpid, namep, isHFSPlus, keyp);
		result = BTSearchRecord(fcb, iterator, kInvalidMRUCacheKey, btRecord, reclen, iterator);
	}

	return result;
}


OSErr
UpdateFolderCount(SVCB *vcb, HFSCatalogNodeID pid, const CatalogName *name, SInt16 newType,
					UInt32 hint, SInt16 valenceDelta)
{
	CatalogRecord		tempData;	// 520 bytes
	HFSCatalogNodeID	folderID;
	UInt16				reclen;
	OSErr				result;
	BTreeIterator btIterator;
	FSBufferDescriptor btRecord;

	btRecord.bufferAddress = &tempData;
	btRecord.itemCount = 1;
	btRecord.itemSize = sizeof(tempData);

	ClearMemory(&btIterator, sizeof(btIterator));
	btIterator.hint.nodeNum = hint;
	BuildCatalogKey(pid, name, vcb->vcbSignature == kHFSPlusSigWord, (CatalogKey*)&btIterator.key);
	result = LocateCatalogNode(vcb->vcbCatalogFile, &btIterator, &btRecord, &reclen);
	ReturnIfError(result);

	if (vcb->vcbSignature == kHFSPlusSigWord) {
		UInt32		timeStamp;

		timeStamp = GetTimeUTC();
		tempData.hfsPlusFolder.valence += valenceDelta;		// adjust valence
		tempData.hfsPlusFolder.contentModDate = timeStamp;	// set date/time last modified
		folderID = tempData.hfsPlusFolder.folderID;
	} else /* kHFSSigWord */ {
		tempData.hfsFolder.valence += valenceDelta;				// adjust valence
		tempData.hfsFolder.modifyDate = GetTimeLocal(true);		// set date/time last modified
		folderID = tempData.hfsFolder.folderID;
	}

	result = BTReplaceRecord(vcb->vcbCatalogFile, &btIterator, &btRecord, reclen);
	ReturnIfError(result);

	if (folderID == kHFSRootFolderID) {
		if (newType == kHFSFolderRecord || newType == kHFSPlusFolderRecord)
			vcb->vcbNmRtDirs += valenceDelta;	// adjust root folder count (undefined for HFS Plus)
		else
			vcb->vcbNmFls += valenceDelta;		// adjust root file count (used by GetVolInfo)
	}

	if (newType == kHFSFolderRecord || newType == kHFSPlusFolderRecord)
		vcb->vcbFolderCount += valenceDelta;			// adjust volume folder count, €€ worry about overflow?
	else
		vcb->vcbFileCount += valenceDelta;			// adjust volume file count
	
	vcb->vcbModifyDate = GetTimeUTC();	// update last modified date
	MarkVCBDirty( vcb );

	return result;
}


/* Delete the catalog node with given name from given parent directory.  
 * The boolean value for_rename indicates that the caller is interested 
 * in deleting this record as part of rename operation and hence when set 
 * to true, the function does not return error if the directory record 
 * being deleted has non-zero valence and does not deallocate blocks for given
 * file.
 */
OSErr
DeleteCatalogNode(SVCB *vcb, UInt32 pid, const CatalogName * name, UInt32 hint, Boolean for_rename) 
{
	CatalogKey * keyp;
	CatalogRecord rec;
	BTreeIterator btIterator;
	FSBufferDescriptor btRecord;

	HFSCatalogNodeID	nodeID;
	HFSCatalogNodeID	nodeParentID;
	UInt16				nodeType;
	UInt16 reclen;
	OSErr result;
	Boolean isHFSPlus = (vcb->vcbSignature == kHFSPlusSigWord);

	btRecord.bufferAddress = &rec;
	btRecord.itemCount = 1;
	btRecord.itemSize = sizeof(rec);

	ClearMemory(&btIterator, sizeof(btIterator));
	btIterator.hint.nodeNum = hint;
	keyp = (CatalogKey*)&btIterator.key;
	BuildCatalogKey(pid, name, isHFSPlus, keyp);

	result = LocateCatalogNode(vcb->vcbCatalogFile, &btIterator, &btRecord, &reclen);
	ReturnIfError(result);

	/* establish real parent cnid  and cnode type */
	nodeParentID = isHFSPlus ? keyp->hfsPlus.parentID : keyp->hfs.parentID;
	nodeType = rec.recordType;
	nodeID = 0;

	switch (nodeType) {
	case kHFSFolderRecord:
		if ((for_rename == false) && (rec.hfsFolder.valence != 0))
			return cmNotEmpty;
		
		nodeID = rec.hfsFolder.folderID;
		break;

	case kHFSPlusFolderRecord:
		if ((for_rename == false) && (rec.hfsPlusFolder.valence != 0))
			return cmNotEmpty;
		
		nodeID = rec.hfsPlusFolder.folderID;
		break;

	case kHFSFileRecord:
		if (rec.hfsFile.flags & kHFSThreadExistsMask)
			nodeID = rec.hfsFile.fileID;
		break;

	case kHFSPlusFileRecord:
		nodeID = rec.hfsPlusFile.fileID;
		break;

	default:
		return cmNotFound;
	}

	if (nodeID == kHFSRootFolderID)
		return cmRootCN;  /* sorry, you can't delete the root! */

	/* delete catalog records for CNode and thread */
	result = BTDeleteRecord(vcb->vcbCatalogFile, &btIterator);
	ReturnIfError(result);

	(void) BTInvalidateHint(&btIterator);

	if ( nodeID ) {		
		BuildCatalogKey(nodeID, NULL, isHFSPlus, keyp);	
		(void) BTDeleteRecord(vcb->vcbCatalogFile, &btIterator);
	}

	/* update directory and volume stats */

	result = UpdateFolderCount(vcb, nodeParentID, NULL, nodeType, kNoHint, -1);
	ReturnIfError(result);

	(void) FlushCatalogFile(vcb);

	if (((nodeType == kHFSPlusFileRecord) || (nodeType == kHFSFileRecord)) && 
	    (for_rename == false)) 
		result = DeallocateFile(vcb, &rec);
	
	return result;
}


OSErr
GetCatalogNode(SVCB *vcb, UInt32 pid, const CatalogName * name, UInt32 hint, CatalogRecord *data)
{
	CatalogKey * keyp;
	BTreeIterator btIterator;
	FSBufferDescriptor btRecord;

	UInt16 reclen;
	OSErr result;
	Boolean isHFSPlus = (vcb->vcbSignature == kHFSPlusSigWord);

	btRecord.bufferAddress = data;
	btRecord.itemCount = 1;
	btRecord.itemSize = sizeof(CatalogRecord);

	ClearMemory(&btIterator, sizeof(btIterator));
	btIterator.hint.nodeNum = hint;
	keyp = (CatalogKey*)&btIterator.key;
	BuildCatalogKey(pid, name, isHFSPlus, keyp);

	result = LocateCatalogNode(vcb->vcbCatalogFile, &btIterator, &btRecord, &reclen);
	
	return result;
}

