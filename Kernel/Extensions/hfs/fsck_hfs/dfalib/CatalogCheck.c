/*
 * Copyright (c) 2000-2009 Apple Inc. All rights reserved.
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
#include "DecompDataEnums.h"
#include "DecompData.h"

#include <sys/stat.h>

extern int RcdFCntErr( SGlobPtr GPtr, OSErr type, UInt32 correct, UInt32 incorrect, HFSCatalogNodeID);
extern int RcdHsFldCntErr( SGlobPtr GPtr, OSErr type, UInt32 correct, UInt32 incorrect, HFSCatalogNodeID);

/*
 * information collected when visiting catalog records
 */
struct CatalogIterationSummary {
	UInt32 parentID;
	UInt32 rootDirCount;   /* hfs only */
	UInt32 rootFileCount;  /* hfs only */
	UInt32 dirCount;
	UInt32 dirThreads;
	UInt32 fileCount;
	UInt32 filesWithThreads; /* hfs only */
	UInt32 fileThreads;
	UInt32 nextCNID;
	UInt64 encodings;
	void * hardLinkRef;
};

/* Globals used during Catalog record checks */
struct CatalogIterationSummary gCIS;

SGlobPtr gScavGlobals;

/* Local routines for checking catalog structures */
static int  CheckCatalogRecord(SGlobPtr GPtr, const HFSPlusCatalogKey *key,
                               const CatalogRecord *rec, UInt16 reclen);
static int  CheckCatalogRecord_HFS(const HFSCatalogKey *key,
                                   const CatalogRecord *rec, UInt16 reclen);

static int  CheckDirectory(const HFSPlusCatalogKey * key, const HFSPlusCatalogFolder * dir);
static int  CheckFile(const HFSPlusCatalogKey * key, const HFSPlusCatalogFile * file);
static int  CheckThread(const HFSPlusCatalogKey * key, const HFSPlusCatalogThread * thread);

static int  CheckDirectory_HFS(const HFSCatalogKey * key, const HFSCatalogFolder * dir);
static int  CheckFile_HFS(const HFSCatalogKey * key, const HFSCatalogFile * file);
static int  CheckThread_HFS(const HFSCatalogKey * key, const HFSCatalogThread * thread);

static void CheckBSDInfo(const HFSPlusCatalogKey * key, const HFSPlusBSDInfo * bsdInfo, int isdir);
static int  CheckCatalogName(u_int16_t charCount, const u_int16_t *uniChars,
                             u_int32_t parentID, Boolean thread);
static int  CheckCatalogName_HFS(u_int16_t charCount, const u_char *filename,
                                 u_int32_t parentID, Boolean thread);

static int  CaptureMissingThread(UInt32 threadID, const HFSPlusCatalogKey *nextKey);
static 	OSErr	UniqueDotName( 	SGlobPtr GPtr, 
                                CatalogName * theNewNamePtr, 
                                UInt32 theParID, 
                                Boolean isSingleDotName,
                                Boolean isHFSPlus );
static Boolean 	FixDecomps(	u_int16_t charCount, const u_int16_t *inFilename, HFSUniStr255 *outFilename );

/*
 * This structure is used to keep track of the folderCount field in
 * HFSPlusCatalogFolder records.  For now, this is only done on HFSX volumes.
 */
struct folderCountInfo {
	UInt32 folderID;
	UInt32 recordedCount;
	UInt32 computedCount;
	struct folderCountInfo *next;
};

/*
 * Print a symbolic link name given the fileid
 */
static void
printSymLinkName(SGlobPtr GPtr, UInt32 fid)
{
	char pathname[PATH_MAX+1], filename[PATH_MAX+1];
	unsigned int path_len = sizeof(pathname), fname_len = sizeof(filename);
	u_int16_t status;

	if (GetFileNamePathByID(GPtr, fid, pathname, &path_len, filename, &fname_len, &status) == 0) {
		fsckPrint(GPtr->context, E_BadSymLinkName, pathname);
	}
	return;
}

/*
 * CountFolderRecords - Counts the number of folder records contained within a
 * given folder.  That is, how many direct subdirectories it has.  This is used
 * to update the folderCount field, if necessary.
 *
 * CountFolderRecords is a straight-forward iteration:  given a HFSPlusCatalogFolder
 * record, it iterates through the catalog BTree until it runs out of records that
 * belong to it.  For each folder record it finds, it increments a count.  When it's
 * done, it compares the two, and if there is a mismatch, requests a repair to be
 * done.
 */
static OSErr
CountFolderRecords(HFSPlusCatalogKey *myKey, HFSPlusCatalogFolder *folder, SGlobPtr GPtr)
{
	SFCB *fcb = GPtr->calculatedCatalogFCB;
	OSErr err = 0;
	BTreeIterator iterator;
	FSBufferDescriptor btRecord;
	union {
		HFSPlusCatalogFolder catRecord;
		HFSPlusCatalogFile catFile;
	} catRecord;
	HFSPlusCatalogKey *key;
	UInt16 recordSize = 0;
	UInt32 folderCount = 0;

	ClearMemory(&iterator, sizeof(iterator));

	key = (HFSPlusCatalogKey*)&iterator.key;
	BuildCatalogKey(folder->folderID, NULL, true, (CatalogKey*)key);
	btRecord.bufferAddress = &catRecord;
	btRecord.itemCount = 1;
	btRecord.itemSize = sizeof(catRecord);

	for (err = BTSearchRecord(fcb, &iterator, kNoHint, &btRecord, &recordSize, &iterator);
		err == 0;
		err = BTIterateRecord(fcb, kBTreeNextRecord, &iterator, &btRecord, &recordSize)) {
		switch (catRecord.catRecord.recordType) {
		case kHFSPlusFolderThreadRecord:
		case kHFSPlusFileThreadRecord:
			continue;
		}
		if (key->parentID != folder->folderID)
			break;
		if (catRecord.catRecord.recordType == kHFSPlusFolderRecord) {
			folderCount++;
		} else if ((catRecord.catRecord.recordType == kHFSPlusFileRecord) && 
			   (catRecord.catFile.flags & kHFSHasLinkChainMask) &&
			   (catRecord.catFile.userInfo.fdType == kHFSAliasType) &&
			   (catRecord.catFile.userInfo.fdCreator == kHFSAliasCreator) &&
			   (key->parentID != GPtr->filelink_priv_dir_id)) {
			/* A directory hard link is treated as normal directory	
			 * for calculation of folder count.
			 */
			folderCount++;
		}
	}
	if (err == btNotFound)
		err = 0;
	if (err == 0) {
		if (folderCount != folder->folderCount) {
			err = RcdFCntErr( GPtr,
				E_FldCount,
				folderCount,
				folder->folderCount,
				folder->folderID);
		}
	}
	return err;
}

static void
releaseFolderCountInfo(struct folderCountInfo *fcip, int numFolders)
{
	int i;

	for (i = 0; i < numFolders; i++) {
		struct folderCountInfo *f = &fcip[i];

		f = f->next;
		while (f) {
			struct folderCountInfo *t = f->next;
			free(f);
			f = t;
		}
	}
	free(fcip);
}

static struct folderCountInfo *
findFolderEntry(struct folderCountInfo *fcip, int numFolders, UInt32 fid)
{
	struct folderCountInfo *retval = NULL;
	int indx;

	indx = fid % numFolders;	// Slot index

	retval = &fcip[indx];
	if (retval->folderID == fid) {
		goto done;
	}
	while (retval->next != NULL) {
		retval = retval->next;
		if (retval->folderID == fid)
			goto done;
	}
	retval = NULL;
done:
	return retval;
}

static struct folderCountInfo *
addFolderEntry(struct folderCountInfo *fcip, int numFolders, UInt32 fid)
{
	struct folderCountInfo *retval = NULL;
	int indx;

	indx = fid % numFolders;
	retval = &fcip[indx];

	if (retval->folderID == fid)
		goto done;
	while (retval->folderID != 0) {
		if (retval->next == NULL) {
			retval->next = calloc(1, sizeof(struct folderCountInfo));
			if (retval->next == NULL) {
				retval = NULL;
				goto done;
			} else
				retval = retval->next;
		} else if (retval->folderID == fid) {
			goto done;
		} else
			retval = retval->next;
	}

	retval->folderID = fid;

done:
	return retval;
}

/*
 * folderCountAdd - Accounts for given folder record or directory hard link  
 * for folder count of the given parent directory.  For directory hard links, 
 * the folder ID and count should be zero.  For a folder record, the values 
 * read from the catalog record are provided which are used to add the 
 * given folderID to the cache (folderCountInfo *ficp).
 */
static int
folderCountAdd(struct folderCountInfo *fcip, int numFolders, UInt32 parentID, UInt32 folderID, UInt32 count)
{
	int retval = 0;
	struct folderCountInfo *curp = NULL;


	/* Only add directories represented by folder record to the cache */
	if (folderID != 0) {
		/*
		 * We track two things here.
		 * First, we need to find the entry matching this folderID.  If we don't find it,
		 * we add it.  If we do find it, or if we add it, we set the recordedCount.
		 */

		curp = findFolderEntry(fcip, numFolders, folderID);
		if (curp == NULL) {
			curp = addFolderEntry(fcip, numFolders, folderID);
			if (curp == NULL) {
				retval = ENOMEM;
				goto done;
			}
		}
		curp->recordedCount = count;

	}

	/*
	 * After that, we try to find the parent to this entry.  When we find it
	 * (or if we add it to the list), we increment the computedCount.
	 */
	curp = findFolderEntry(fcip, numFolders, parentID);
	if (curp == NULL) {
		curp = addFolderEntry(fcip, numFolders, parentID);
		if (curp == NULL) {
			retval = ENOMEM;
			goto done;
		}
	}
	curp->computedCount++;

done:
	return retval;
}

/*
 * CheckFolderCount - Verify the folderCount fields of the HFSPlusCatalogFolder records
 * in the catalog BTree.  This is currently only done for HFSX.
 *
 * Conceptually, this is a fairly simple routine:  simply iterate through the catalog
 * BTree, and count the number of subfolders contained in each folder.  This value
 * is used for the stat.st_nlink field, on HFSX.
 *
 * However, since scanning the entire catalog can be a very costly operation, we dot
 * it one of two ways.  The first way is to simply iterate through the catalog once,
 * and keep track of each folder ID we come across.  This uses a fair bit of memory,
 * so we limit the cache to 5MBytes, which works out to some 400k folderCountInfo
 * entries (at the current size of three 4-byte entries per folderCountInfo entry).
 * If the filesystem has more than that, we instead use the slower (but significantly
 * less memory-intensive) method in CountFolderRecords:  for each folder ID we
 * come across, we call CountFolderRecords, which does its own iteration through the
 * catalog, looking for children of the given folder.
 */

OSErr
CheckFolderCount( SGlobPtr GPtr )
{
	OSErr err = 0;
	int numFolders;
	BTreeIterator iterator;
	FSBufferDescriptor btRecord;
	HFSPlusCatalogKey *key;
	union {
		HFSPlusCatalogFolder catRecord;
		HFSPlusCatalogFile catFile;
	} catRecord;
	UInt16 recordSize = 0;
	struct folderCountInfo *fcip = NULL;

	ClearMemory(&iterator, sizeof(iterator));
	if (!VolumeObjectIsHFSX(GPtr)) {
		goto done;
	}

	if (GPtr->calculatedVCB == NULL) {
		err = EINVAL;
		goto done;
	}

#if 0
	/*
	 * We add two so we can account for the root folder, and
	 * the root folder's parent.  Neither of which is real,
	 * but they show up as parent IDs in the catalog.
	 */
	numFolders = GPtr->calculatedVCB->vcbFolderCount + 2;
#else
	/*
	 * Since we're using a slightly smarter hash method,
	 * we don't care so much about the number of folders
	 * allegedly on the volume; instead, we'll pick a nice
	 * prime number to use as the number of buckets.
	 * This bears some performance checking later.
	 */
	numFolders = 257;
#endif

	/*
	 * Limit the size of the folder count cache to 5Mbytes;
	 * if the requested number of folders don't fit, then
	 * we don't use the cache at all.
	 */
#define MAXCACHEMEM	(5 * 1024 * 1024)	/* 5Mbytes */
#define LCALLOC(c, s, l)	\
	({ __typeof(c) _count = (c); __typeof(s) _size = (s); __typeof(l) _lim = (l); \
		((_count * _size) > _lim) ? NULL : calloc(_count, _size); })

	fcip = LCALLOC(numFolders, sizeof(*fcip), MAXCACHEMEM);
#undef MAXCACHEMEM
#undef LCALLOC

restart:
	/* these objects are used by the BT* functions to iterate through the catalog */
	key = (HFSPlusCatalogKey*)&iterator.key;
	BuildCatalogKey(kHFSRootFolderID, NULL, true, (CatalogKey*)key);
	btRecord.bufferAddress = &catRecord;
	btRecord.itemCount = 1;
	btRecord.itemSize = sizeof(catRecord);

	/*
	 * Iterate through the catalog BTree until the end.
	 * For each folder we either cache the value, or we call CheckFolderCount.
	 * We also check the kHFSHasFolderCountMask flag in the folder flags field;
	 * if it's not set, we set it.  (When migrating a volume from an older version.
	 * this will affect every folder entry; after that, it will only affect any
	 * corrupted areas.)
	 */
	for (err = BTIterateRecord(GPtr->calculatedCatalogFCB, kBTreeFirstRecord,
			&iterator, &btRecord, &recordSize);
		err == 0;
		err = BTIterateRecord(GPtr->calculatedCatalogFCB, kBTreeNextRecord,
			&iterator, &btRecord, &recordSize)) {

		switch (catRecord.catRecord.recordType) {
		case kHFSPlusFolderRecord:
			if (!(catRecord.catRecord.flags & kHFSHasFolderCountMask)) {
				/* RcdHsFldCntErr requests a repair order to fix up the flags field */
				err = RcdHsFldCntErr( GPtr,
							E_HsFldCount,
							catRecord.catRecord.flags | kHFSHasFolderCountMask,
							catRecord.catRecord.flags,
							catRecord.catRecord.folderID );
				if (err != 0)
					goto done;
			}
			if (fcip) {
				if (folderCountAdd(fcip, numFolders,
					key->parentID,
					catRecord.catRecord.folderID,
					catRecord.catRecord.folderCount)) {
					/*
					 * We got an error -- this only happens if folderCountAdd()
					 * cannot allocate memory for a new node.  In that case, we
					 * need to bail on the whole cache, and use the slow method.
					 * This also lets us release the memory, which will hopefully
					 * let some later allocations succeed.  We restart just after
					 * the cache was allocated, and start over as if we had never
					 * allocated a cache in the first place.
					 */
					releaseFolderCountInfo(fcip, numFolders);
					fcip = NULL;
					goto restart;
				}
			} else {
				err = CountFolderRecords(key, &catRecord.catRecord, GPtr);
				if (err != 0)
					goto done;
			}
			break;
		case kHFSPlusFileRecord:
			/* If this file record is a directory hard link, count
			 * it towards our folder count calculations.
			 */
			if ((catRecord.catFile.flags & kHFSHasLinkChainMask) &&
			    (catRecord.catFile.userInfo.fdType == kHFSAliasType) &&
			    (catRecord.catFile.userInfo.fdCreator == kHFSAliasCreator) &&
			    (key->parentID != GPtr->filelink_priv_dir_id)) {
			    	/* If we are using folder count cache, account
				 * for directory hard links by incrementing
				 * associated parentID in the cache.  If an 
				 * extensive search for catalog is being 
				 * performed, account for directory hard links 
				 * in CountFolderRecords()
				 */
			    	if (fcip) {
					if (folderCountAdd(fcip, numFolders, 
						key->parentID, 0, 0)) {
						/* See above for why we release & restart */
						releaseFolderCountInfo(fcip, numFolders);
						fcip = NULL;
						goto restart;
					}
				}
			}
			break;
		}
	}

	if (err == btNotFound)
		err = 0;	// We hit the end of the file, which is okay
	if (err == 0 && fcip != NULL) {
		int i;

		/*
		 * At this point, we are itereating through the cache, looking for
		 * mis-counts. (If we're not using the cache, then CountFolderRecords has
		 * already dealt with any miscounts.)
		 */
		for (i = 0; i < numFolders; i++) {
			struct folderCountInfo *curp;

			for (curp = &fcip[i]; curp; curp = curp->next) {
				if (curp->folderID == 0) {
					// fplog(stderr, "fcip[%d] has a folderID of 0?\n", i);
				} else if (curp->folderID == kHFSRootParentID) {
					// Root's parent doesn't really exist
					continue;
				} else {
					if (curp->recordedCount != curp->computedCount) {
						/* RcdFCntErr requests a repair order to correct the folder count */
						err = RcdFCntErr( GPtr,
									E_FldCount,
									curp->computedCount,
									curp->recordedCount,
									curp->folderID );
						if (err != 0)
							goto done;
					}
				}
			}
		}
	}
done:
	if (fcip) {
		releaseFolderCountInfo(fcip, numFolders);
		fcip = NULL;
	}
	return err;
}

/*
 * CheckCatalogBTree - Verifies the catalog B-tree structure
 *
 * Causes CheckCatalogRecord to be called for every leaf record
 */
OSErr
CheckCatalogBTree( SGlobPtr GPtr )
{
	OSErr err;
	int hfsplus;
	
	gScavGlobals = GPtr;
	hfsplus = VolumeObjectIsHFSPlus( );

	ClearMemory(&gCIS, sizeof(gCIS));
	gCIS.parentID = kHFSRootParentID;
	gCIS.nextCNID = kHFSFirstUserCatalogNodeID;

	if (hfsplus) {
		/* Initialize check for file hard links */
        	HardLinkCheckBegin(gScavGlobals, &gCIS.hardLinkRef);

		/* Initialize check for directory hard links */
		dirhardlink_init(gScavGlobals);
	}

	GPtr->journal_file_id = GPtr->jib_file_id = 0;

	if (CheckIfJournaled(GPtr, true)) {
		CatalogName fname;
		CatalogKey key;
		CatalogRecord rec;
		UInt16 recSize;
		int i;
		
#define	HFS_JOURNAL_FILE	".journal"
#define	HFS_JOURNAL_INFO	".journal_info_block"

		fname.ustr.length = strlen(HFS_JOURNAL_FILE);
		for (i = 0; i < fname.ustr.length; i++)
			fname.ustr.unicode[i] = HFS_JOURNAL_FILE[i];
		BuildCatalogKey(kHFSRootFolderID, &fname, true, &key);
		if (SearchBTreeRecord(GPtr->calculatedCatalogFCB, &key, kNoHint, NULL, &rec, &recSize, NULL) == noErr &&
			rec.recordType == kHFSPlusFileRecord) {
			GPtr->journal_file_id = rec.hfsPlusFile.fileID;
		}
		fname.ustr.length = strlen(HFS_JOURNAL_INFO);
		for (i = 0; i < fname.ustr.length; i++)
			fname.ustr.unicode[i] = HFS_JOURNAL_INFO[i];
		BuildCatalogKey(kHFSRootFolderID, &fname, true, &key);
		if (SearchBTreeRecord(GPtr->calculatedCatalogFCB, &key, kNoHint, NULL, &rec, &recSize, NULL) == noErr &&
			rec.recordType == kHFSPlusFileRecord) {
			GPtr->jib_file_id = rec.hfsPlusFile.fileID;
		}
	}
		
	/* for compatibility, init these globals */
	gScavGlobals->TarID = kHFSCatalogFileID;
	GetVolumeObjectBlockNum( &gScavGlobals->TarBlock );

	/*
	 * Check out the BTree structure
	 */
	err = BTCheck(gScavGlobals, kCalculatedCatalogRefNum, (CheckLeafRecordProcPtr)CheckCatalogRecord);
	if (err) goto exit;

	if (gCIS.dirCount != gCIS.dirThreads) {
		RcdError(gScavGlobals, E_IncorrectNumThdRcd);
		gScavGlobals->CBTStat |= S_Orphan;  /* a directory record is missing */
		if (fsckGetVerbosity(gScavGlobals->context) >= kDebugLog) {
			plog ("\t%s: dirCount = %u, dirThread = %u\n", __FUNCTION__, gCIS.dirCount, gCIS.dirThreads);
		}
	}

	if (hfsplus && (gCIS.fileCount != gCIS.fileThreads)) {
		RcdError(gScavGlobals, E_IncorrectNumThdRcd);
		gScavGlobals->CBTStat |= S_Orphan;
		if (fsckGetVerbosity(gScavGlobals->context) >= kDebugLog) {
			plog ("\t%s: fileCount = %u, fileThread = %u\n", __FUNCTION__, gCIS.fileCount, gCIS.fileThreads);
		}
	}

	if (!hfsplus && (gCIS.fileThreads != gCIS.filesWithThreads)) {
		RcdError(gScavGlobals, E_IncorrectNumThdRcd);
		gScavGlobals->CBTStat |= S_Orphan;
		if (fsckGetVerbosity(gScavGlobals->context) >= kDebugLog) {
			plog ("\t%s: fileThreads = %u, filesWithThread = %u\n", __FUNCTION__, gCIS.fileThreads, gCIS.filesWithThreads);
		}
	}

	gScavGlobals->calculatedVCB->vcbEncodingsBitmap = gCIS.encodings;
	gScavGlobals->calculatedVCB->vcbNextCatalogID = gCIS.nextCNID;
	gScavGlobals->calculatedVCB->vcbFolderCount = gCIS.dirCount - 1;
	gScavGlobals->calculatedVCB->vcbFileCount = gCIS.fileCount;
	if (!hfsplus) {
		gScavGlobals->calculatedVCB->vcbNmRtDirs = gCIS.rootDirCount;
		gScavGlobals->calculatedVCB->vcbNmFls = gCIS.rootFileCount;
	}

	/*
	 * Check out the allocation map structure
	 */
	err = BTMapChk(gScavGlobals, kCalculatedCatalogRefNum);
	if (err) goto exit;

	/*
	 * Make sure unused nodes in the B-tree are zero filled.
	 */
	err = BTCheckUnusedNodes(gScavGlobals, kCalculatedCatalogRefNum, &gScavGlobals->CBTStat);
	if (err) goto exit;
	
	/*
	 * Compare BTree header record on disk with scavenger's BTree header record 
	 */
	err = CmpBTH(gScavGlobals, kCalculatedCatalogRefNum);
	if (err) goto exit;

	/*
	 * Compare BTree map on disk with scavenger's BTree map
	 */
	err = CmpBTM(gScavGlobals, kCalculatedCatalogRefNum);

	if (hfsplus) {
		if (scanflag == 0) {
			(void) CheckHardLinks(gCIS.hardLinkRef);

			/* If any unrepairable corruption was detected for file 
			 * hard links, stop the verification process by returning 
			 * negative value.
			 */
			if (gScavGlobals->CatStat & S_LinkErrNoRepair) {
				err = -1;
				goto exit;
			}
		}
	}

 exit:
	if (hfsplus)
		HardLinkCheckEnd(gCIS.hardLinkRef);

	return (err);
}

/*
 * CheckCatalogRecord - verify a catalog record
 *
 * Called in leaf-order for every leaf record in the Catalog B-tree
 */
static int
CheckCatalogRecord(SGlobPtr GPtr, const HFSPlusCatalogKey *key, const CatalogRecord *rec, UInt16 reclen)
{
	int 						result = 0;
	Boolean						isHFSPlus;

	isHFSPlus = VolumeObjectIsHFSPlus( );
	++gScavGlobals->itemsProcessed;

	if (!isHFSPlus)
		return CheckCatalogRecord_HFS((HFSCatalogKey *)key, rec, reclen);

	gScavGlobals->CNType = rec->recordType;

	switch (rec->recordType) {
	case kHFSPlusFolderRecord:
		++gCIS.dirCount;
		if (reclen != sizeof(HFSPlusCatalogFolder)){
			RcdError(gScavGlobals, E_LenDir);
			result = E_LenDir;
			break;
		}
		if (key->parentID != gCIS.parentID) {
			result = CaptureMissingThread(key->parentID, key);
			if (result) break;
			/* Pretend thread record was there */
			++gCIS.dirThreads;
			gCIS.parentID = key->parentID;
		}
		result = CheckDirectory(key, (HFSPlusCatalogFolder *)rec);
		break;

	case kHFSPlusFileRecord:
		++gCIS.fileCount;
		if (reclen != sizeof(HFSPlusCatalogFile)){
			RcdError(gScavGlobals, E_LenFil);
			result = E_LenFil;
			break;
		}
		if (key->parentID != gCIS.parentID) {
			result = CaptureMissingThread(key->parentID, key);
			if (result) break;
			/* Pretend thread record was there */
			++gCIS.dirThreads;
			gCIS.parentID = key->parentID;
		}
		result = CheckFile(key, (HFSPlusCatalogFile *)rec);
		break;

	case kHFSPlusFolderThreadRecord:
		++gCIS.dirThreads;
		gCIS.parentID = key->parentID;
		/* Fall through */

	case kHFSPlusFileThreadRecord:
		if (rec->recordType == kHFSPlusFileThreadRecord)
			++gCIS.fileThreads;

		if (reclen > sizeof(HFSPlusCatalogThread) ||
			reclen < sizeof(HFSPlusCatalogThread) - sizeof(HFSUniStr255)) {
			RcdError(gScavGlobals, E_LenThd);
			result = E_LenThd;
			break;
		} else if (reclen == sizeof(HFSPlusCatalogThread)) {
			gScavGlobals->VeryMinorErrorsStat |= S_BloatedThreadRecordFound;
		}
		result = CheckThread(key, (HFSPlusCatalogThread *)rec);
		break;

	default:
		RcdError(gScavGlobals, E_CatRec);
		result = E_CatRec;
	}
	
	return (result);
}

/*
 * CheckCatalogRecord_HFS - verify an HFS catalog record
 *
 * Called in leaf-order for every leaf record in the Catalog B-tree
 */
static int
CheckCatalogRecord_HFS(const HFSCatalogKey *key, const CatalogRecord *rec, UInt16 reclen)
{
	int result = 0;

	gScavGlobals->CNType = rec->recordType;

	switch (rec->recordType) {
	case kHFSFolderRecord:
		++gCIS.dirCount;
		if (key->parentID == kHFSRootFolderID )
			++gCIS.rootDirCount;
		if (reclen != sizeof(HFSCatalogFolder)){
			RcdError(gScavGlobals, E_LenDir);
			result = E_LenDir;
			break;
		}
		if (key->parentID != gCIS.parentID) {
			result = CaptureMissingThread(key->parentID, (HFSPlusCatalogKey *)key);
			if (result) break;
			/* Pretend thread record was there */
			++gCIS.dirThreads;
			gCIS.parentID = key->parentID;
		}
		result = CheckDirectory_HFS(key, (HFSCatalogFolder *)rec);
		break;

	case kHFSFileRecord:
		++gCIS.fileCount;
		if (key->parentID == kHFSRootFolderID )
			++gCIS.rootFileCount;
		if (reclen != sizeof(HFSCatalogFile)){
			RcdError(gScavGlobals, E_LenFil);
			result = E_LenFil;
			break;
		}
		if (key->parentID != gCIS.parentID) {
			result = CaptureMissingThread(key->parentID, (HFSPlusCatalogKey *)key);
			if (result) break;
			/* Pretend thread record was there */
			++gCIS.dirThreads;
			gCIS.parentID = key->parentID;
		}
		result = CheckFile_HFS(key, (HFSCatalogFile *)rec);
		break;

	case kHFSFolderThreadRecord:
		++gCIS.dirThreads;
		gCIS.parentID = key->parentID;
		/* Fall through */
	case kHFSFileThreadRecord:
		if (rec->recordType == kHFSFileThreadRecord)
			++gCIS.fileThreads;

		if (reclen != sizeof(HFSCatalogThread)) {
			RcdError(gScavGlobals, E_LenThd);
			result = E_LenThd;
			break;
		}
		result = CheckThread_HFS(key, (HFSCatalogThread *)rec);
		break;


	default:
		RcdError(gScavGlobals, E_CatRec);
		result = E_CatRec;
	}
	
	return (result);
}

/*
 * CheckDirectory - verify a catalog directory record
 *
 * Also collects info for later processing.
 * Called in leaf-order for every directory record in the Catalog B-tree
 */
static int 
CheckDirectory(const HFSPlusCatalogKey * key, const HFSPlusCatalogFolder * dir)
{
	UInt32 dirID;
	int result = 0;

	dirID = dir->folderID;

	/* Directory cannot have these two flags set */
	if ((dir->flags & (kHFSFileLockedMask | kHFSThreadExistsMask)) != 0) {
		RcdError(gScavGlobals, E_CatalogFlagsNotZero);
		gScavGlobals->CBTStat |= S_ReservedNotZero;
	}

	RecordXAttrBits(gScavGlobals, dir->flags, dir->folderID, kCalculatedCatalogRefNum);
#if DEBUG_XATTR
	plog ("%s: Record folderID=%d for prime modulus calculations\n", __FUNCTION__, dir->folderID); 
#endif

	if (dirID < kHFSFirstUserCatalogNodeID  &&
            dirID != kHFSRootFolderID) {
		RcdError(gScavGlobals, E_InvalidID);
		return (E_InvalidID);
	}
	if (dirID >= gCIS.nextCNID )
		gCIS.nextCNID = dirID + 1;

	gCIS.encodings |= (u_int64_t)(1ULL << MapEncodingToIndex(dir->textEncoding & 0x7F));

	CheckBSDInfo(key, &dir->bsdInfo, true);
	
	CheckCatalogName(key->nodeName.length, &key->nodeName.unicode[0], key->parentID, false);
	
	/* Keep track of the directory inodes found */
	if (dir->flags & kHFSHasLinkChainMask) {
	    	gScavGlobals->calculated_dirinodes++;
	}

	return (result);
}

/*
 * CheckFile - verify a HFS+ catalog file record
 * - sanity check values
 * - collect info for later processing
 *
 * Called in leaf-order for every file record in the Catalog B-tree
 */
static int
CheckFile(const HFSPlusCatalogKey * key, const HFSPlusCatalogFile * file)
{
	UInt32 fileID;
	UInt32 blocks;
	UInt64 bytes;
	int result = 0;
	int islink = 0;
	int isjrnl = 0;
	size_t	len;
	unsigned char filename[256 * 3];

	(void) utf_encodestr(key->nodeName.unicode,
				key->nodeName.length * 2,
				filename, &len, sizeof(filename));
	filename[len] = '\0';

	RecordXAttrBits(gScavGlobals, file->flags, file->fileID, kCalculatedCatalogRefNum);
#if DEBUG_XATTR
	plog ("%s: Record fileID=%d for prime modulus calculations\n", __FUNCTION__, file->fileID); 
#endif

	fileID = file->fileID;
	if (fileID < kHFSFirstUserCatalogNodeID) {
		RcdError(gScavGlobals, E_InvalidID);
		result = E_InvalidID;
		return (result);
	}
	if (fileID >= gCIS.nextCNID )
		gCIS.nextCNID = fileID + 1;

	gCIS.encodings |= (u_int64_t)(1ULL << MapEncodingToIndex(file->textEncoding & 0x7F));

	CheckBSDInfo(key, &file->bsdInfo, false);

	/* check out data fork extent info */
	result = CheckFileExtents(gScavGlobals, file->fileID, kDataFork, NULL, 
                                file->dataFork.extents, &blocks);
	if (result != noErr)
		return (result);

	if (file->dataFork.totalBlocks != blocks) {
		result = RecordBadAllocation(key->parentID, filename, kDataFork,
					file->dataFork.totalBlocks, blocks);
		if (result)
			return (result);
	} else {
		bytes = (UInt64)blocks * (UInt64)gScavGlobals->calculatedVCB->vcbBlockSize;
		if (file->dataFork.logicalSize > bytes) {
			result = RecordTruncation(key->parentID, filename, kDataFork,
					file->dataFork.logicalSize, bytes);
			if (result)
				return (result);
		}
	}
	/* check out resource fork extent info */
	result = CheckFileExtents(gScavGlobals, file->fileID, kRsrcFork, NULL, 
	                          file->resourceFork.extents, &blocks);
	if (result != noErr)
		return (result);

	if (file->resourceFork.totalBlocks != blocks) {
		result = RecordBadAllocation(key->parentID, filename, kRsrcFork,
					file->resourceFork.totalBlocks, blocks);
		if (result)
			return (result);
	} else {
		bytes = (UInt64)blocks * (UInt64)gScavGlobals->calculatedVCB->vcbBlockSize;
		if (file->resourceFork.logicalSize > bytes) {
			result = RecordTruncation(key->parentID, filename, kRsrcFork,
					file->resourceFork.logicalSize, bytes);
			if (result)
				return (result);
		}
	}

	/* Collect indirect link info for later */
	if (file->userInfo.fdType == kHardLinkFileType  &&
            file->userInfo.fdCreator == kHFSPlusCreator) {
		islink = 1;
		CaptureHardLink(gCIS.hardLinkRef, file);
	}

	CheckCatalogName(key->nodeName.length, &key->nodeName.unicode[0], key->parentID, false);

	/* Keep track of the directory hard links found */
	if ((file->flags & kHFSHasLinkChainMask) && 
	    ((file->userInfo.fdType == kHFSAliasType) ||
	    (file->userInfo.fdCreator == kHFSAliasCreator)) &&
		(key->parentID != gScavGlobals->filelink_priv_dir_id &&
		key->parentID != gScavGlobals->dirlink_priv_dir_id)) {
	    	gScavGlobals->calculated_dirlinks++;
		islink = 1;
	}

	/* For non-journaled filesystems, the cached journal file IDs will be 0 */
	if (file->fileID &&
		(file->fileID == gScavGlobals->journal_file_id ||
		file->fileID == gScavGlobals->jib_file_id)) {
		isjrnl = 1;
	}

	if (islink == 0) {
		if (file->flags & kHFSHasLinkChainMask &&
			(gScavGlobals->filelink_priv_dir_id != key->parentID &&
			 gScavGlobals->dirlink_priv_dir_id != key->parentID)) {
			RepairOrderPtr p;
			fsckPrint(gScavGlobals->context, E_LinkChainNonLink, file->fileID);
			p = AllocMinorRepairOrder(gScavGlobals, 0);
			if (p) {
				p->type = E_LinkChainNonLink;
				p->correct = 0;
				p->incorrect = 0;
				p->parid = file->fileID;
				p->hint = 0;
			} else {
				result = memFullErr;
			}
			gScavGlobals->CatStat |= S_LinkErrRepair;
		}

		if (((file->bsdInfo.fileMode & S_IFMT) == S_IFREG) &&
			gScavGlobals->filelink_priv_dir_id != key->parentID &&
			file->bsdInfo.special.linkCount > 1 &&
			isjrnl == 0) {
			RepairOrderPtr p;
			char badstr[16];
			fsckPrint(gScavGlobals->context, E_FileLinkCountError, file->fileID);
			snprintf(badstr, sizeof(badstr), "%u", file->bsdInfo.special.linkCount);
			fsckPrint(gScavGlobals->context, E_BadValue, "1", badstr);

			p = AllocMinorRepairOrder(gScavGlobals, 0);
			if (p) {
				p->type = E_FileLinkCountError;
				p->correct = 1;
				p->incorrect = file->bsdInfo.special.linkCount;
				p->parid = file->fileID;
				p->hint = 0;
			} else {
				result = memFullErr;
			}
			gScavGlobals->CatStat |= S_LinkErrRepair;
		}
		/*
		 * Check for symlinks.
		 * Currently, d_check_slink is 0x1000, so -D 0x1000 on the command line.
		 */
		if ((cur_debug_level & d_check_slink) != 0) {
			if (((file->bsdInfo.fileMode & S_IFMT) == S_IFLNK) ||
			    file->userInfo.fdType == kSymLinkFileType ||
			    file->userInfo.fdCreator == kSymLinkCreator) {
				// Okay, it claims to be a symlink, at least somehow.
				// Check all the info
				if (((file->bsdInfo.fileMode & S_IFMT) != S_IFLNK) ||
				    file->userInfo.fdType != kSymLinkFileType ||
				    file->userInfo.fdCreator != kSymLinkCreator) {
					fsckPrint(gScavGlobals->context, E_BadSymLink, file->fileID);
					// Should find a way to print out the path, no?
				}
				if (file->dataFork.logicalSize > PATH_MAX) {
					fsckPrint(gScavGlobals->context, E_BadSymLinkLength, file->fileID, (unsigned int)file->dataFork.logicalSize, (unsigned int)PATH_MAX);
					printSymLinkName(gScavGlobals, file->fileID);
				} else {
					/*
					 * Reading is hard.
					 * It's made easier by PATH_MAX being so small, so we can assume
					 * (for now) that the file is entirely in the 8 extents in the catalog
					 * record.  (In most cases, it'll be only one extent; in the worst
					 * case, it will only be 2, at least until PATH_MAX is increased.)
					 */
					uint8_t *dataBuffer = malloc(file->dataFork.totalBlocks * gScavGlobals->calculatedVCB->vcbBlockSize + 1);
					
					if (dataBuffer == NULL) {
                        if (debug)
                            plog("Unable to allocate %llu bytes for reading symlink", file->dataFork.logicalSize);
					} else {
						char *curPtr = (char*)dataBuffer;
						size_t nread = 0;
						size_t dataLen;
						HFSPlusExtentDescriptor *ep = (HFSPlusExtentDescriptor*)&file->dataFork.extents[0];
						
						while (nread < file->dataFork.logicalSize) {
							Buf_t *bufp;
							int rv = CacheRead(&fscache, ep->startBlock * (off_t)gScavGlobals->calculatedVCB->vcbBlockSize, ep->blockCount * gScavGlobals->calculatedVCB->vcbBlockSize, &bufp);
							if (rv) {
								abort(); // do something better
							}
							memcpy(curPtr, bufp->Buffer, bufp->Length);
							curPtr += bufp->Length;
							nread += bufp->Length;
							CacheRelease(&fscache, bufp, 0);
						}
						dataLen = strnlen((char*)dataBuffer, file->dataFork.totalBlocks * gScavGlobals->calculatedVCB->vcbBlockSize);
						if (dataLen != file->dataFork.logicalSize) {
							fsckPrint(gScavGlobals->context, E_BadSymLinkLength, file->fileID, (unsigned int)dataLen, (unsigned int)file->dataFork.logicalSize);
							printSymLinkName(gScavGlobals, file->fileID);
							if (debug)
								plog("Symlink for file id %u has bad data length\n", file->fileID);
						}
						free(dataBuffer);
					}
				}
			}
		}
	}
	if (islink == 1 && file->dataFork.totalBlocks != 0) {
		fsckPrint(gScavGlobals->context, E_LinkHasData, file->fileID);
		gScavGlobals->CatStat |= S_LinkErrNoRepair;
	}

	return (result);
}

/*
 * CheckThread - verify a catalog thread
 *
 * Called in leaf-order for every thread record in the Catalog B-tree
 */
static int 
CheckThread(const HFSPlusCatalogKey * key, const HFSPlusCatalogThread * thread)
{
	int result = 0;

	if (key->nodeName.length != 0) {
		RcdError(gScavGlobals, E_ThdKey);
		return (E_ThdKey);
	}

	result = CheckCatalogName(thread->nodeName.length, &thread->nodeName.unicode[0],
                         thread->parentID, true);
	if (result != noErr) {
		RcdError(gScavGlobals, E_ThdCN);
		return (E_ThdCN);
	}	

	if (key->parentID < kHFSFirstUserCatalogNodeID  &&
            key->parentID != kHFSRootParentID  &&
            key->parentID != kHFSRootFolderID) {
		RcdError(gScavGlobals, E_InvalidID);
		return (E_InvalidID);
	}

	if (thread->parentID == kHFSRootParentID) {
		if (key->parentID != kHFSRootFolderID) {
			RcdError(gScavGlobals, E_InvalidID);
			return (E_InvalidID);
		}
	} else if (thread->parentID < kHFSFirstUserCatalogNodeID &&
	           thread->parentID != kHFSRootFolderID) {
		RcdError(gScavGlobals, E_InvalidID);
		return (E_InvalidID);
	}

	return (0);
}

/*
 * CheckDirectory - verify an HFS catalog directory record
 *
 * Also collects info for later processing.
 * Called in leaf-order for every directory record in the Catalog B-tree
 */
static int 
CheckDirectory_HFS(const HFSCatalogKey * key, const HFSCatalogFolder * dir)
{
	UInt32 dirID;
	int result = 0;

	dirID = dir->folderID;

	/* Directory cannot have these two flags set */
	if ((dir->flags & (kHFSFileLockedMask | kHFSThreadExistsMask)) != 0) {
		RcdError(gScavGlobals, E_CatalogFlagsNotZero);
		gScavGlobals->CBTStat |= S_ReservedNotZero;
	}

	if (dirID < kHFSFirstUserCatalogNodeID  &&
            dirID != kHFSRootFolderID) {
		RcdError(gScavGlobals, E_InvalidID);
		return (E_InvalidID);
	}
	if (dirID >= gCIS.nextCNID )
		gCIS.nextCNID = dirID + 1;
	
	CheckCatalogName_HFS(key->nodeName[0], &key->nodeName[1], key->parentID, false);
	
	return (result);
}

/*
 * CheckFile_HFS - verify a HFS catalog file record
 * - sanity check values
 * - collect info for later processing
 *
 * Called in b-tree leaf order for every HFS file
 * record in the Catalog B-tree.
 */
static int
CheckFile_HFS(const HFSCatalogKey * key, const HFSCatalogFile * file)
{
	UInt32 fileID;
	UInt32 blocks;
	char idstr[20];
	int result = 0;

	if (file->flags & kHFSThreadExistsMask)
		++gCIS.filesWithThreads;

	/* 3843017 : Check for reserved field removed to support new bits in future */
	if ((file->dataStartBlock)          ||
	    (file->rsrcStartBlock)          ||
	    (file->reserved))
	{
		RcdError(gScavGlobals, E_CatalogFlagsNotZero);
		gScavGlobals->CBTStat |= S_ReservedNotZero;
	}

	fileID = file->fileID;
	if (fileID < kHFSFirstUserCatalogNodeID) {
		RcdError(gScavGlobals, E_InvalidID);
		result = E_InvalidID;
		return (result);
	}
	if (fileID >= gCIS.nextCNID )
		gCIS.nextCNID = fileID + 1;

	/* check out data fork extent info */
	result = CheckFileExtents(gScavGlobals, file->fileID, kDataFork, NULL, 
                                file->dataExtents, &blocks);
	if (result != noErr)
		return (result);
	if (file->dataPhysicalSize > ((UInt64)blocks * (UInt64)gScavGlobals->calculatedVCB->vcbBlockSize)) {
		snprintf (idstr, sizeof(idstr), "id=%u", fileID);
		fsckPrint(gScavGlobals->context, E_PEOF, idstr);
		return (noErr);		/* we don't fix this, ignore the error */
	}
	if (file->dataLogicalSize > file->dataPhysicalSize) {
		snprintf (idstr, sizeof(idstr), "id=%u", fileID);
		fsckPrint(gScavGlobals->context, E_LEOF, idstr);
                return (noErr);		/* we don't fix this, ignore the error */
	}

	/* check out resource fork extent info */
	result = CheckFileExtents(gScavGlobals, file->fileID, kRsrcFork, NULL,
				file->rsrcExtents, &blocks);
	if (result != noErr)
		return (result);
	if (file->rsrcPhysicalSize > ((UInt64)blocks * (UInt64)gScavGlobals->calculatedVCB->vcbBlockSize)) {
		snprintf (idstr, sizeof(idstr), "id=%u", fileID);
		fsckPrint(gScavGlobals->context, E_PEOF, idstr);
                return (noErr);		/* we don't fix this, ignore the error */
	}
	if (file->rsrcLogicalSize > file->rsrcPhysicalSize) {
		snprintf (idstr, sizeof(idstr), "id=%u", fileID);
		fsckPrint(gScavGlobals->context, E_LEOF, idstr);
                return (noErr);		/* we don't fix this, ignore the error */
	}
#if 1
	/* Keeping handle in globals of file ID's for HFS volume only */
	if (PtrAndHand(&file->fileID, (Handle)gScavGlobals->validFilesList, sizeof(UInt32) ) )
		return (R_NoMem);
#endif
	CheckCatalogName_HFS(key->nodeName[0], &key->nodeName[1], key->parentID, false);

	return (result);
}

/*
 * CheckThread - verify a catalog thread
 *
 * Called in leaf-order for every thread record in the Catalog B-tree
 */
static int
CheckThread_HFS(const HFSCatalogKey * key, const HFSCatalogThread * thread)
{
	int result = 0;

	if (key->nodeName[0] != 0) {
		RcdError(gScavGlobals, E_ThdKey);
		return (E_ThdKey);
	}

	result = CheckCatalogName_HFS(thread->nodeName[0], &thread->nodeName[1],
                         	  thread->parentID, true);
	if (result != noErr) {
		RcdError(gScavGlobals, E_ThdCN);
		return (E_ThdCN);
	}	

	if (key->parentID < kHFSFirstUserCatalogNodeID  &&
            key->parentID != kHFSRootParentID  &&
            key->parentID != kHFSRootFolderID) {
		RcdError(gScavGlobals, E_InvalidID);
		return (E_InvalidID);
	}

	if (thread->parentID == kHFSRootParentID) {
		if (key->parentID != kHFSRootFolderID) {
			RcdError(gScavGlobals, E_InvalidID);
			return (E_InvalidID);
		}
	} else if (thread->parentID < kHFSFirstUserCatalogNodeID &&
	           thread->parentID != kHFSRootFolderID) {
		RcdError(gScavGlobals, E_InvalidID);
		return (E_InvalidID);
	}

	return (0);
}


/* File types from BSD Mode */
#define FT_MASK    0170000	/* Mask of file type. */
#define FT_FIFO    0010000	/* Named pipe (fifo). */
#define FT_CHR     0020000	/* Character device. */
#define FT_DIR     0040000	/* Directory file. */
#define FT_BLK     0060000	/* Block device. */
#define FT_REG     0100000	/* Regular file. */
#define FT_LNK     0120000	/* Symbolic link. */
#define FT_SOCK    0140000	/* BSD domain socket. */

/*
 * CheckBSDInfo - Check BSD Permissions data
 * (HFS Plus volumes only)
 *
 * if repairable then log the error and create a repair order
 */
static void
CheckBSDInfo(const HFSPlusCatalogKey * key, const HFSPlusBSDInfo * bsdInfo, int isdir)
{

	Boolean reset = false;

	/* skip uninitialized BSD info */
	if (bsdInfo->fileMode == 0)
		return;
	
	switch (bsdInfo->fileMode & FT_MASK) {
	  case FT_DIR:
		if (!isdir)
			reset = true;
		break;
	  case FT_REG:
	  case FT_BLK:
	  case FT_CHR:
	  case FT_LNK:
	  case FT_SOCK:
	  case FT_FIFO:
		if (isdir)
			reset = true;
		break;
	  default:
		reset = true;
	}
	
	if (reset) {
		RepairOrderPtr p;
		int n;
		
		gScavGlobals->TarBlock = bsdInfo->fileMode & FT_MASK;
		RcdError(gScavGlobals, E_InvalidPermissions);

		n = CatalogNameSize( (CatalogName *) &key->nodeName, true );
		
		p = AllocMinorRepairOrder(gScavGlobals, n);
		if (p == NULL) return;

 		CopyCatalogName((const CatalogName *)&key->nodeName,
		(CatalogName*)&p->name, true);
		
		p->type      = E_InvalidPermissions;
		p->correct   = 0;
		p->incorrect = bsdInfo->fileMode;
		p->parid = key->parentID;
		p->hint = 0;
		
		gScavGlobals->CatStat |= S_Permissions;
	}
}

/*
 * Validate a Unicode filename for HFS+ volumes
 *
 * check character count
 * check for illegal names
 *
 * if repairable then log the error and create a repair order
 */
static int
CheckCatalogName(u_int16_t charCount, const u_int16_t *uniChars, u_int32_t parentID, Boolean thread)
{
	OSErr 				result;
	u_int16_t *			myPtr;
	RepairOrderPtr 		roPtr;
	int					myLength;
    CatalogName			newName;

	if ((charCount == 0) || (charCount > kHFSPlusMaxFileNameChars))
		return( E_CName );
        
    // only do the remaining checks for files or directories
    if ( thread )
        return( noErr );
 
    // look for objects with illegal names of "." or "..".  We only do this for
    // file or folder catalog records (the thread records will be taken care of
    // in the repair routines).
    if ( charCount < 3 && *uniChars == 0x2E )
    {
        if ( charCount == 1 || (charCount == 2 && *(uniChars + 1) == 0x2E) )
        {
		fsckPrint(gScavGlobals->context, E_IllegalName);
            if ( fsckGetVerbosity(gScavGlobals->context) >= kDebugLog ) {
               	plog( "\tillegal name is 0x" );
                PrintName( charCount, (UInt8 *) uniChars, true );
           }
          
            // get a new name to use when we rename the file system object
            result = UniqueDotName( gScavGlobals, &newName, parentID, 
                                    ((charCount == 1) ? true : false), true );
            if ( result != noErr ) 
                return( noErr );

            // we will copy the old and new names to our RepairOrder.  The names will
            // look like this:
            // 	 2 byte length of old name
            //   unicode characters for old name
            //   2 byte length of new name
            //   unicode characters for new name 
            myLength = (charCount + 1) * 2; // bytes needed for old name 
            myLength += ((newName.ustr.length + 1) * 2); // bytes needed for new name

            roPtr = AllocMinorRepairOrder( gScavGlobals, myLength );
            if ( roPtr == NULL ) 
                return( noErr );

            myPtr = (u_int16_t *) &roPtr->name;
            *myPtr++ = charCount; // copy in length of old name and bump past it
            CopyMemory( uniChars, myPtr, (charCount * 2) ); // copy in old name
            myPtr += charCount; // bump past old name
            *myPtr++ = newName.ustr.length; // copy in length of new name and bump past it
            CopyMemory( newName.ustr.unicode, myPtr, (newName.ustr.length * 2) ); // copy in new name
            if ( fsckGetVerbosity(gScavGlobals->context) >= kDebugLog ) {
               	plog( "\treplacement name is 0x" );
                PrintName( newName.ustr.length, (UInt8 *) &newName.ustr.unicode, true );
           }

 			roPtr->type = E_IllegalName;
            roPtr->parid = parentID;
            gScavGlobals->CatStat |= S_IllName;
            return( E_IllegalName );
        }
    }

    // look for Unicode decomposition errors in file system object names created before Jaguar (10.2)
    if ( FixDecomps( charCount, uniChars, &newName.ustr ) )
    {
	fsckPrint(gScavGlobals->context, E_IllegalName);
        if ( fsckGetVerbosity(gScavGlobals->context) >= kDebugLog ) {
            plog( "\tillegal name is 0x" );
            PrintName( charCount, (UInt8 *) uniChars, true );
        }

        // we will copy the old and new names to our RepairOrder.  The names will
        // look like this:
        // 	 2 byte length of old name
        //   unicode characters for old name
        //   2 byte length of new name
        //   unicode characters for new name 
        myLength = (charCount + 1) * 2; // bytes needed for old name 
        myLength += ((newName.ustr.length + 1) * 2); // bytes needed for new name

        roPtr = AllocMinorRepairOrder( gScavGlobals, myLength );
        if ( roPtr == NULL ) 
            return( noErr );

        myPtr = (u_int16_t *) &roPtr->name;
        *myPtr++ = charCount; // copy in length of old name and bump past it
        CopyMemory( uniChars, myPtr, (charCount * 2) ); // copy in old name
        myPtr += charCount; // bump past old name
        *myPtr++ = newName.ustr.length; // copy in length of new name and bump past it
        CopyMemory( newName.ustr.unicode, myPtr, (newName.ustr.length * 2) ); // copy in new name
        if ( fsckGetVerbosity(gScavGlobals->context) >= kDebugLog ) {
            plog( "\treplacement name is 0x" );
            PrintName( newName.ustr.length, (UInt8 *) &newName.ustr.unicode, true );
        }

        roPtr->type = E_IllegalName;
        roPtr->parid = parentID;
        gScavGlobals->CatStat |= S_IllName;
        return( E_IllegalName );
    }

	return( noErr );
}


/*
 * Validate an HFS filename 
 *
 * check character count
 * check for illegal names
 *
 * if repairable then log the error and create a repair order
 */
static int
CheckCatalogName_HFS(u_int16_t charCount, const u_char *filename, u_int32_t parentID, Boolean thread)
{
	u_char *			myPtr;
	RepairOrderPtr 		roPtr;
	int					myLength;
    CatalogName			newName;

	if ((charCount == 0) || (charCount > kHFSMaxFileNameChars))
		return( E_CName );

     // only do the remaining checks for files or directories
    if ( thread )
        return( noErr );
       
    // look for objects with illegal names of "." or "..".  We only do this for
    // file or folder catalog records (the thread records will be taken care of
    // in the repair routines).
    if ( charCount < 3 && *filename == 0x2E )
    {
        if ( charCount == 1 || (charCount == 2 && *(filename + 1) == 0x2E) )
        {
            OSErr 				result;
	    fsckPrint(gScavGlobals->context, E_IllegalName);
            if ( fsckGetVerbosity(gScavGlobals->context) >= kDebugLog ) {
               	plog( "\tillegal name is 0x" );
                PrintName( charCount, filename, false );
            }

            // get a new name to use when we rename the file system object
            result = UniqueDotName( gScavGlobals, &newName, parentID, 
                                    ((charCount == 1) ? true : false), false );
            if ( result != noErr ) 
                return( noErr );
            
            // we will copy the old and new names to our RepairOrder.  The names will
            // look like this:
            // 	 1 byte length of old name
            //   characters for old name
            //   1 byte length of new name
            //   characters for new name 
            myLength = charCount + 1; // bytes needed for old name 
            myLength += (newName.pstr[0] + 1); // bytes needed for new name
            roPtr = AllocMinorRepairOrder( gScavGlobals, myLength );
            if ( roPtr == NULL ) 
                return( noErr );
 
            myPtr = (u_char *)&roPtr->name[0];
            *myPtr++ = charCount; // copy in length of old name and bump past it
            CopyMemory( filename, myPtr, charCount );
            myPtr += charCount; // bump past old name
            *myPtr++ = newName.pstr[0]; // copy in length of new name and bump past it
            CopyMemory( &newName.pstr[1], myPtr, newName.pstr[0] ); // copy in new name
            if ( fsckGetVerbosity(gScavGlobals->context) >= kDebugLog ) {
               	plog( "\treplacement name is 0x" );
                PrintName( newName.pstr[0], &newName.pstr[1], false );
            }

 			roPtr->type = E_IllegalName;
            roPtr->parid = parentID;
            gScavGlobals->CatStat |= S_IllName;
            return( E_IllegalName );
        }
    }

	return( noErr );
}


/*------------------------------------------------------------------------------
UniqueDotName:  figure out a unique name we can use to rename a file system
object that has the illegal name of "." or ".."
------------------------------------------------------------------------------*/
static OSErr
UniqueDotName( 	SGlobPtr GPtr, 
            	CatalogName * theNewNamePtr, 
            	UInt32 theParID, 
                Boolean isSingleDotName,
             	Boolean isHFSPlus )
{
    u_char				newChar;
	OSErr 				result;
	size_t				nameLen;
	UInt16				recSize;
	SFCB *				fcbPtr;
    u_char *			myPtr;
	CatalogRecord		record;
    CatalogKey			catKey;
    u_char				dotName[] = {'d', 'o', 't', 'd', 'o', 't', 0x0d, 0x00};

  	fcbPtr = GPtr->calculatedCatalogFCB;
 
    // create key with new name
    if ( isSingleDotName )
        myPtr = &dotName[3];
    else
        myPtr = &dotName[0];
    
	nameLen = strlen((char *) myPtr );
    if ( isHFSPlus )
    {
        int		i;
        theNewNamePtr->ustr.length = nameLen;
        for ( i = 0; i < theNewNamePtr->ustr.length; i++ )
            theNewNamePtr->ustr.unicode[ i ] = (u_int16_t) *(myPtr + i);
    }
    else
    {
        theNewNamePtr->pstr[0] = nameLen;
        memcpy( &theNewNamePtr->pstr[1], myPtr, nameLen );
    }
    
    // if the name is already in use we will try appending ascii characters
    // from '0' (0x30) up to '~' (0x7E) 
    for ( newChar = 0x30; newChar < 0x7F; newChar++ )
    {
        // make sure new name isn't already there
        BuildCatalogKey( theParID, theNewNamePtr, isHFSPlus, &catKey );
        result = SearchBTreeRecord( fcbPtr, &catKey, kNoHint, NULL, &record, &recSize, NULL );
        if ( result != noErr )
            return( noErr );
            
        // new name is already there, try another
        if ( isHFSPlus )
        {
            theNewNamePtr->ustr.unicode[ nameLen ] = (u_int16_t) newChar;
            theNewNamePtr->ustr.length = nameLen + 1;
        }
        else
        {
            theNewNamePtr->pstr[ 0 ] = nameLen + 1;
            theNewNamePtr->pstr[ nameLen + 1 ] = newChar;
        }
    }

    return( -1 );
    
} /* UniqueDotName */

/* Function: RecordBadAllocation
 *
 * Description:
 * Record a repair to adjust a file or extended attribute's allocation size.
 * This could also trigger a truncation if the new block count isn't large 
 * enough to cover the current LEOF.
 *
 * Note that it stores different values and prints different error message
 * for file and extended attribute.
 * For files - 
 *	E_PEOF, parentID, filename, forkType (kDataFork/kRsrcFork).
 *	Prints filename.
 * For extended attributes -
 *	E_PEOAttr, fileID, attribute name, forkType (kEAData).
 *	Prints attribute name and filename.  Since the attribute name is 
 *  passed as parameter, it needs to lookup the filename.
 *
 * Input:
 *	For files - 
 *		parID		- parent ID of file
 *		filename	- name of the file	
 *		forkType	- type of fork (kDataFork/kRsrcFork)
 *	For extended attributes - 
 *		parID		- fileID for attribute
 *		filename	- name of the attribute	
 *		forkType	- kEAData
 *	Common inputs -
 *		oldBlkCnt 	- Incorrect block count
 *		newBlkCnt	- Correct block count
 * Output:
 *	On success, zero.
 *	On failure, non-zero.
 *		R_NoMem - out of memory
 *		E_PEOF	- Bad allocation error on plain HFS volume
 */
int
RecordBadAllocation(UInt32 parID, unsigned char * filename, UInt32 forkType, UInt32 oldBlkCnt, UInt32 newBlkCnt)
{
	RepairOrderPtr 			p;
	char 					goodstr[16];
	char 					badstr[16];
	size_t 					n;
	Boolean 				isHFSPlus;
	int 					result;
	char 					*real_filename;
	unsigned int			filenamelen;

	isHFSPlus = VolumeObjectIsHFSPlus( );
	if (forkType == kEAData) {
		/* Print attribute name and filename for extended attribute */
		filenamelen = NAME_MAX * 3;
		real_filename = malloc(filenamelen);
		if (!real_filename) {
			return (R_NoMem);
		}

		/* Get the name of the file */
		result = GetFileNamePathByID(gScavGlobals, parID, NULL, NULL, 
					real_filename, &filenamelen, NULL);
		if (result) {
			/* If error while looking up filename, default to print file ID */
			sprintf(real_filename, "id = %u", parID);
		}

		fsckPrint(gScavGlobals->context, E_PEOAttr, filename, real_filename);
		free(real_filename);
	} else {
		fsckPrint(gScavGlobals->context, E_PEOF, filename);
	}
	sprintf(goodstr, "%d", newBlkCnt);
	sprintf(badstr, "%d", oldBlkCnt);
	fsckPrint(gScavGlobals->context, E_BadValue, goodstr, badstr);

	/* Only HFS+ is repaired here */
	if ( !isHFSPlus )
		return (E_PEOF);

	n = strlen((char *)filename);
	p = AllocMinorRepairOrder(gScavGlobals, n + 1);
	if (p == NULL)
		return (R_NoMem);

	if (forkType == kEAData) {
		p->type = E_PEOAttr;
	} else {
		p->type = E_PEOF;
	}
	p->forkType = forkType;
	p->incorrect = oldBlkCnt;
	p->correct = newBlkCnt;
	p->hint = 0;
	p->parid = parID;
	p->name[0] = n;  /* pascal string */
	CopyMemory(filename, &p->name[1], n);

	gScavGlobals->CatStat |= S_FileAllocation;
	return (0);
}

/* Function: RecordTruncation
 *
 * Description:
 * Record a repair to trucate a file's logical size.
 *
 * Note that it stores different error values and prints 
 * different error message for file and extended attribute.
 * For files - 
 *	E_LEOF, parentID, filename, forkType (kDataFork/kRsrcFork).
 *	Prints filename.
 * For extended attributes -
 *	E_LEOAttr, fileID, attribute name, forkType (kEAData).
 *	Prints attribute name and filename.  Since the attribute name is 
 *  passed as parameter, it needs to lookup the filename.
 *
 * Input:
 *	For files - 
 *		parID		- parent ID of file
 *		filename	- name of the file	
 *		forkType	- type of fork (kDataFork/kRsrcFork)
 *	For extended attributes - 
 *		parID		- fileID for attribute
 *		filename	- name of the attribute	
 *		forkType	- kEAData
 *	Common inputs -
 *		oldSize 	- Incorrect logical size
 *		newSize		- Correct logical size
 * Output:
 *	On success, zero.
 *	On failure, non-zero.
 *		R_NoMem - out of memory
 *		E_LEOF	- Truncation error on plain HFS volume
 */
int
RecordTruncation(UInt32 parID, unsigned char * filename, UInt32 forkType, UInt64 oldSize, UInt64 newSize)
{
	RepairOrderPtr	 		p;
	char 					oldSizeStr[48];
	char 					newSizeStr[48];
	size_t 					n;
	Boolean 				isHFSPlus;
	int 					result;
	char 					*real_filename;
	unsigned int			filenamelen;

	isHFSPlus = VolumeObjectIsHFSPlus( );
	if (forkType == kEAData) {
		/* Print attribute name and filename for extended attribute */
		filenamelen = NAME_MAX * 3;
		real_filename = malloc(filenamelen);
		if (!real_filename) {
			return (R_NoMem);
		}

		/* Get the name of the file */
		result = GetFileNamePathByID(gScavGlobals, parID, NULL, NULL, 
					real_filename, &filenamelen, NULL);
		if (result) {
			/* If error while looking up filename, default to print file ID */
			sprintf(real_filename, "id = %u", parID);
		}

		fsckPrint(gScavGlobals->context, E_LEOAttr, filename, real_filename);
		free(real_filename);
	} else {
		fsckPrint(gScavGlobals->context, E_LEOF, filename);
	}
	sprintf(oldSizeStr, "%qd", oldSize);
	sprintf(newSizeStr, "%qd", newSize);
	fsckPrint(gScavGlobals->context, E_BadValue, newSizeStr, oldSizeStr);

	/* Only HFS+ is repaired here */
	if ( !isHFSPlus )
		return (E_LEOF);

	n = strlen((char *)filename);
	p = AllocMinorRepairOrder(gScavGlobals, n + 1);
	if (p == NULL)
		return (R_NoMem);

	if (forkType == kEAData) {
		p->type = E_LEOAttr;
	} else {
		p->type = E_LEOF;
	}
	p->forkType = forkType;
	p->incorrect = oldSize;
	p->correct = newSize;
	p->hint = 0;
	p->parid = parID;
	p->name[0] = n;  /* pascal string */
	CopyMemory(filename, &p->name[1], n);

	gScavGlobals->CatStat |= S_FileAllocation;
	return (0);
}


/*
 * CaptureMissingThread
 *
 * Capture info for a missing thread record so it
 * can be repaired later.  The next key is saved
 * so that the Catalog Hierarchy check can proceed.
 * The thread PID/NAME are initialized during the
 * Catalog Hierarchy check phase.
 */
static int
CaptureMissingThread(UInt32 threadID, const HFSPlusCatalogKey *nextKey)
{
	MissingThread 			*mtp;
	Boolean 				isHFSPlus;

	isHFSPlus = VolumeObjectIsHFSPlus( );

	fsckPrint(gScavGlobals->context, E_NoThd, threadID);

	/* Only HFS+ missing threads are repaired here */
	if ( !isHFSPlus)
		return (E_NoThd);
	
	mtp = (MissingThread *) AllocateClearMemory(sizeof(MissingThread));
	if (mtp == NULL)
		return (R_NoMem);
	
	/* add it to the list of missing threads */
	mtp->link = gScavGlobals->missingThreadList;
	gScavGlobals->missingThreadList = mtp;
	
	mtp->threadID = threadID;	
	CopyMemory(nextKey, &mtp->nextKey, nextKey->keyLength + 2);

	if (gScavGlobals->RepLevel == repairLevelNoProblemsFound)
		gScavGlobals->RepLevel = repairLevelVolumeRecoverable;

	gScavGlobals->CatStat |= S_MissingThread;
	return (noErr);
}


/*
 FixDecomps.  Originally written by Peter Edberg for use in fsck_hfs.

 If inFilename needs updating and the function was able to do this without
 overflowing the 255-character limit, it returns 1 (true) and outFIlename
 contains the update file. If inFilename did not need updating, or an update
 would overflow the limit, the function returns 0 (false) and the contents of
 outFilename are undefined.
 
Function implementation:

Characters that don't require any special handling have combining class 0 and do
not begin a decomposition sequence (of 1-3 characters) that needs updating. For
these characters, the function just copies them from inFilename to outFilename
and sets the pointer outNameCombSeqPtr to NULL (when this pointer is not NULL,
it points to the beginning of a sequence of combining marks that continues up to
the current character; if the current character is combining, it may need to be
reordered into that sequence). The copying operation in cheap, and postponing it
until we know the filename needs modification would make the code much more
complicated.

This copying operation may be invoked from many places in the code, some deeply
nested - any time the code determines that the current character needs no
special handling. For this reason it has a label (CopyBaseChar) and is located
at the end of the character processing loop; various places in the code use goto
statements to jump to it (this is a situation where they are justified).

The main function loop has 4 sections.

First, it quickly determines if the high 12 bits of the character indicate that
it is in a range that has neither nonzero combining class nor any decomposition
sequences that need updating. If so, the code jumps straight to CopyBaseChar.

Second, the code determines if the character is part of a sequence that needs
updating. It checks if the current character has a corresponding action in the
replaceData array. If so, depending on the action, it may need to check for
additional matching characters in inFilename. If the sequence of 1-3 characters
is successfully matched, then a replacement sequence of 1-3 characters is
inserted at the corresponding position in outFilename. While this replacement
sequence is being inserted, each character must be checked to see if it has
nonzero combining class and needs reordering (some replacement sequences consist
entirely of combining characters and may interact with combining characters in
the filename before the updated sequence).

Third, the code handles characters whose high-order 12 bits indicated that some
action was required, but were not part of sequences that needed updating (these
may include characters that were examined in the second part but were part of
sequences that did not completely match, so there are also goto fallthroughs to
this code - labeled CheckCombClass - from the second part). These characters
have to be checked for nonzero combining class; if so, they are reordered as
necessary. Each time a new nonzero class character is encountered, it is added
to outFIlename at the correct point in any active combining character sequence
(with other characters in the sequence moved as necessary), so the sequence
pointed to by outNameCombSeqPtr is always in correct order up to the current
character.

Finally, the fourth part has the default handlers to just copy characters to
outFilename.

 */
static Boolean 
FixDecomps(	u_int16_t charCount, const u_int16_t *inFilename, HFSUniStr255 *outFilename ) 
{
    // input filename: address of curr input char,
	const u_int16_t *	inNamePtr		= inFilename;
    // and of last input char.
	const u_int16_t *	inNameLastPtr	= &inFilename[charCount - 1];	
    // output filename buffer: addr of next output char,
	u_int16_t *	outNamePtr			= outFilename->unicode;	
    // and of last possible output char.
	u_int16_t *	outNameLastPtr		= &outFilename->unicode[kHFSPlusMaxFileNameChars - 1];	
	u_int16_t *	outNameCombSeqPtr	= NULL;	// if non-NULL, start of output combining seq we are processing.
	u_int32_t	maxClassValueInSeq	= 0;
	Boolean		didModifyName		= 0;

	while (inNamePtr <= inNameLastPtr) {
		u_int16_t	shiftUniChar;	// this must be 16 bits for the kShiftUniCharOffset wraparound to work
		int32_t		rangeIndex;
		u_int32_t	shiftUniCharLo;
		u_int32_t	replDataIndex;
		u_int32_t	currCharClass;

		shiftUniChar = *inNamePtr + kShiftUniCharOffset;
		if ( shiftUniChar >= kShiftUniCharLimit )
			goto CopyBaseChar;
		rangeIndex = classAndReplIndex[shiftUniChar >> kLoFieldBitSize];
		if ( rangeIndex < 0 )
			goto CopyBaseChar;
		shiftUniCharLo = shiftUniChar & kLoFieldMask;
		replDataIndex = replaceRanges[rangeIndex][shiftUniCharLo];

		if ( replDataIndex > 0 ) {
			// we have a possible substitution (replDataIndex != 0)
			const u_int16_t *	replDataPtr;
			u_int32_t			action;
			u_int32_t			copyCount	= 0;

			replDataPtr = &replaceData[replDataIndex];
			action = *replDataPtr++;
			switch (action) {
				case kReplaceCurWithTwo:
				case kReplaceCurWithThree:
					inNamePtr++;
					copyCount = (action == kReplaceCurWithTwo)? 2: 3;
					break;
				// the next 3 cases can have a first char or replacement char with nonzero combining class
				case kIfNextOneMatchesReplaceAllWithOne:
				case kIfNextOneMatchesReplaceAllWithTwo:
					if (inNamePtr + 1 <= inNameLastPtr && *(inNamePtr + 1) == *replDataPtr++) {
						inNamePtr += 2;
						copyCount = (action == kIfNextOneMatchesReplaceAllWithOne)? 1: 2;
					} else {
						// No substitution; check for comb class & copy char
						goto CheckCombClass;
					}
					break;
				case kIfNextTwoMatchReplaceAllWithOne:
					if ( inNamePtr + 2 <= inNameLastPtr && 
                         *(inNamePtr + 1) == *replDataPtr++ && 
                         *(inNamePtr + 2) == *replDataPtr++) 
                    {
						inNamePtr += 3;
						copyCount = 1;
					} else {
						// No substitution; check for comb class & copy char
						goto CheckCombClass;
					}
					break;
			}

			// now copy copyCount chars (1-3) from replDataPtr to output, checking for comb class etc.
			if (outNamePtr + copyCount - 1 > outNameLastPtr) {
				didModifyName = 0;
				break;
			}
			while (copyCount-- > 0) {
				currCharClass = 0;
				shiftUniChar = *replDataPtr + kShiftUniCharOffset;
				if ( shiftUniChar < kShiftUniCharLimit ) {
					rangeIndex = classAndReplIndex[shiftUniChar >> kLoFieldBitSize];
					if (rangeIndex >= 0) {
						shiftUniCharLo = shiftUniChar & kLoFieldMask;
						currCharClass = combClassRanges[rangeIndex][shiftUniCharLo];
					}
				}
				// This part is similar to CheckCombClass below, which has more detailed
				// comments; see them for info.
				if ( currCharClass == 0 ) {
					outNameCombSeqPtr = NULL;
					*outNamePtr++ = *replDataPtr++;
				} else if ( outNameCombSeqPtr == NULL ) {
					outNameCombSeqPtr = outNamePtr;
					maxClassValueInSeq = currCharClass;
					*outNamePtr++ = *replDataPtr++;
				} else if ( currCharClass >= maxClassValueInSeq ) {
					// Sequence is already in correct order with current char,
					// just update maxClassValueInSeq
					maxClassValueInSeq = currCharClass;
					*outNamePtr++ = *replDataPtr++;
				} else if ( outNamePtr - outNameCombSeqPtr == 1) {
					// Here we know we need to reorder.
					// If the sequence is two chars, just interchange them
					*outNamePtr++ = *outNameCombSeqPtr;
					*outNameCombSeqPtr = *replDataPtr++;
				} else {
					// General reordering case for three or more chars.
					u_int16_t *	outNameCombCharPtr;
					u_int32_t	combCharClass;
					
					outNameCombCharPtr = outNamePtr++;
					while (outNameCombCharPtr > outNameCombSeqPtr) {
						shiftUniChar = *(outNameCombCharPtr - 1) + kShiftUniCharOffset;
						rangeIndex = classAndReplIndex[shiftUniChar >> kLoFieldBitSize];
						shiftUniCharLo = shiftUniChar & kLoFieldMask;
						combCharClass = combClassRanges[rangeIndex][shiftUniCharLo];
						if (combCharClass <= currCharClass)
							break;
						*outNameCombCharPtr = *(outNameCombCharPtr - 1);
						outNameCombCharPtr--;
					}
					*outNameCombCharPtr = *replDataPtr++;
				}
			}
			didModifyName = 1;
			continue;
		} /* end of replDataIndex > 0 */

	CheckCombClass:
		// check for combining class
		currCharClass = combClassRanges[rangeIndex][shiftUniCharLo];
		if ( currCharClass == 0 ) {
			goto CopyBaseChar;
		}
		if ( outNameCombSeqPtr == NULL ) {
			// The current char is the first of a possible sequence of chars
			// with nonzero combining class. Initialize sequence stuff, then
			// just copy char to output.
			outNameCombSeqPtr = outNamePtr;
			maxClassValueInSeq = currCharClass;
			goto CopyChar;
		}
		if ( currCharClass >= maxClassValueInSeq ) {
			// The sequence of chars with nonzero combining class is already
			// in correct order through the current char; just update the max
			// class value found in the sequence.
			maxClassValueInSeq = currCharClass;
			goto CopyChar;
		}

		// This char is at least the second in a sequence of chars with
		// nonzero combining class in the output buffer; outNameCombSeqPtr
		// points to the first in the sequence. Need to put the current
		// char into the correct place in the sequence (previous chars in
		// the sequence are already in correct order, but the current char
		// is out of place).
		
		// First make sure there is room for the new char
		if (outNamePtr > outNameLastPtr) {
			didModifyName = 0;
			break;
		}

		if (outNamePtr - outNameCombSeqPtr == 1) {
			// If the sequence is two chars, just interchange them
			*outNamePtr++ = *outNameCombSeqPtr;
			*outNameCombSeqPtr = *inNamePtr++;
		} else {
			// General case: Starting at previous end of sequence, move chars to
			// next position in string as long as their class is higher than current
			// char; insert current char where we stop. We could cache the
			// combining classes instead of re-determining them, but having multiple
			// combining marks is rare enough that it wouldn't be worth the overhead.
			// At least we don't have to recheck shiftUniChar < kShiftUniCharLimit,
			// rangeIndex != 0, etc.)
			u_int16_t *	outNameCombCharPtr;
			u_int32_t	combCharClass;

			outNameCombCharPtr = outNamePtr++;
			while (outNameCombCharPtr > outNameCombSeqPtr) {
				shiftUniChar = *(outNameCombCharPtr - 1) + kShiftUniCharOffset;
				rangeIndex = classAndReplIndex[shiftUniChar >> kLoFieldBitSize];
				shiftUniCharLo = shiftUniChar & kLoFieldMask;
				combCharClass = combClassRanges[rangeIndex][shiftUniCharLo];
				if (combCharClass <= currCharClass)
					break;
				*outNameCombCharPtr = *(outNameCombCharPtr - 1);
				outNameCombCharPtr--;
			}
			*outNameCombCharPtr = *inNamePtr++;
		}
		didModifyName = 1;
		continue;

	CopyBaseChar:
		outNameCombSeqPtr = NULL;
	CopyChar:
		// nothing special happens with this char, just copy to output
		if (outNamePtr <= outNameLastPtr) {
			*outNamePtr++ = *inNamePtr++;
		} else {
			didModifyName = 0;
			break;
		}
	} /* end of while( inNamePtr <= inNameLastPtr ) */

	if (didModifyName) {
		outFilename->length = outNamePtr - outFilename->unicode;
	}
	return didModifyName;
    
} /* FixDecomps */

