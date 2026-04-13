/*
 * Copyright (c) 2007-2008 Apple Inc. All rights reserved.
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
#include "SRuntime.h"
#include <sys/stat.h>
#include <ctype.h>

/* Looks up a catalog file/folder record for given file/folder ID.  
 * The functionality of this routine is same as GetCatalogRecord() in 
 * dfalib/SRepair.c, but this implementation is better because it does not 
 * change the lastIterator stored in the catalog BTreeControlBlock.
 * Therefore this function does not interfere with other catalog btree 
 * iterations.
 */
OSErr GetCatalogRecordByID(SGlobPtr GPtr, UInt32 file_id, Boolean isHFSPlus, CatalogKey *key, CatalogRecord *rec, uint16_t *recsize)
{
	int retval = 0;
	SFCB *fcb;
	BTreeControlBlock *btcb;
	FSBufferDescriptor buf_desc;
	BTreeIterator search_iterator;
	BTreeIterator result_iterator;
	uint32_t thread_key_parentID = 0;

	fcb = GPtr->calculatedCatalogFCB;
	btcb = (BTreeControlBlock *)fcb->fcbBtree;

	/* Lookup the thread record with given file/folderID */
	bzero(&buf_desc, sizeof(buf_desc));
	bzero(&search_iterator, sizeof(search_iterator));
	buf_desc.bufferAddress = rec;
	buf_desc.itemCount = 1;
	buf_desc.itemSize = sizeof(CatalogRecord);

	BuildCatalogKey(file_id, NULL, isHFSPlus, (CatalogKey *)&(search_iterator.key));
	retval = BTSearchRecord(fcb, &search_iterator, kInvalidMRUCacheKey, 
			&buf_desc, recsize, &result_iterator);
	if (retval) {
		goto out;
	}

	/* Check if really we found a thread record */
	if (isHFSPlus) {
		if ((rec->recordType != kHFSPlusFolderThreadRecord) &&
		    (rec->recordType != kHFSPlusFileThreadRecord)) {
			retval = ENOENT;
			goto out;
		}
	} else {
		if ((rec->recordType != kHFSFolderThreadRecord) &&
		    (rec->recordType != kHFSFileThreadRecord)) {
			retval = ENOENT;
			goto out;
		}
	}

	if (isHFSPlus) {
		thread_key_parentID = ((CatalogKey *)&(result_iterator.key))->hfsPlus.parentID;
	}
	
	/* Lookup the corresponding file/folder record */
	bzero(&buf_desc, sizeof(buf_desc));
	bzero(&search_iterator, sizeof(search_iterator));
	buf_desc.bufferAddress = rec;
	buf_desc.itemCount = 1;
	buf_desc.itemSize = sizeof(CatalogRecord);

	if (isHFSPlus) {
		BuildCatalogKey(rec->hfsPlusThread.parentID, 
			(CatalogName *)&(rec->hfsPlusThread.nodeName),
			isHFSPlus, (CatalogKey *)&(search_iterator.key));
	} else {
		BuildCatalogKey(rec->hfsThread.parentID, 
			(CatalogName *)&(rec->hfsThread.nodeName),
			isHFSPlus, (CatalogKey *)&(search_iterator.key));
	}
	retval = BTSearchRecord(fcb, &search_iterator, kInvalidMRUCacheKey, 
			&buf_desc, recsize, &result_iterator);
	if (retval) {
		goto out;
	}
	
	bcopy(&(result_iterator.key), key, CalcKeySize(btcb, &(result_iterator.key)));
	
	if (isHFSPlus) {
		/* For catalog file or folder record, the parentID in the thread 
		 * record's key should be equal to the fileID in the file/folder 
		 * record --- which is equal to the ID of the file/folder record
		 * that is being looked up.  If not, mark the volume for repair. 
		 */
		if (thread_key_parentID != rec->hfsPlusFile.fileID) {
			RcdError(GPtr, E_IncorrectNumThdRcd);
			if (fsckGetVerbosity(GPtr->context) >= kDebugLog) {
				plog("\t%s: fileID=%u, thread.key.parentID=%u, record.fileID=%u\n", 
					__FUNCTION__, file_id, thread_key_parentID, rec->hfsPlusFile.fileID);
			}
			GPtr->CBTStat |= S_Orphan;
		}
	}

out:
	return retval;
}

/* Record minor repair order for invalid permissions for directory hardlink priv dir */
static int record_privdir_bad_perm(SGlobPtr gptr, uint32_t cnid)
{
	RepairOrderPtr p;

	RcdError (gptr, E_BadPermPrivDir);
	p = AllocMinorRepairOrder(gptr, 0);
	if (p == NULL) {
		return ENOMEM;
	}

	p->type = E_BadPermPrivDir;
	p->parid = cnid;
	gptr->CatStat |= S_LinkErrRepair;

	return 0;
}

/* Record minor repair order for invalid flags for file/directory hard links */
int record_link_badflags(SGlobPtr gptr, uint32_t link_id, Boolean isdir,
		uint32_t incorrect, uint32_t correct)
{
	RepairOrderPtr p;
	char str1[12];
	char str2[12];

	fsckPrint(gptr->context, isdir? E_DirLinkBadFlags : E_FileLinkBadFlags, link_id);
	snprintf(str1, sizeof(str1), "0x%x", correct);
	snprintf(str2, sizeof(str2), "0x%x", incorrect);
	fsckPrint(gptr->context, E_BadValue, str1, str2);
	
	p = AllocMinorRepairOrder(gptr, 0);
	if (p == NULL) {
		return ENOMEM;
	}
	
	p->type = isdir ? E_DirLinkBadFlags : E_FileLinkBadFlags;
	p->correct = correct;
	p->incorrect = incorrect;
	p->parid = link_id;

	gptr->CatStat |= S_LinkErrRepair;

	return 0;
}

/* Record minor repair order for invalid flags for file/directory inode 
 * If a corruption is recorded during verification, do not check for 
 * duplicates as none should exist.  If this corruption is recorded 
 * during repair, check for duplicates because before early termination 
 * of verification we might have seen this corruption.
 */
int record_inode_badflags(SGlobPtr gptr, uint32_t inode_id, Boolean isdir,
		uint32_t incorrect, uint32_t correct, Boolean check_duplicates)
{
	RepairOrderPtr p;
	char str1[12];
	char str2[12];

	p = AllocMinorRepairOrder(gptr, 0);
	if (p == NULL) {
		return ENOMEM;
	}
	
	p->type = isdir ? E_DirInodeBadFlags : E_FileInodeBadFlags;
	p->correct = correct;
	p->incorrect = incorrect;
	p->parid = inode_id;

	gptr->CatStat |= S_LinkErrRepair;

	if ((check_duplicates != 0) && 
	    (IsDuplicateRepairOrder(gptr, p) == 1)) {
		DeleteRepairOrder(gptr, p);
	} else {
		fsckPrint(gptr->context, isdir? E_DirInodeBadFlags : E_FileInodeBadFlags, inode_id);
		snprintf(str1, sizeof(str1), "0x%x", correct);
		snprintf(str2, sizeof(str2), "0x%x", incorrect);
		fsckPrint(gptr->context, E_BadValue, str1, str2);
	}

	return 0;
}

/* Record minor repair order for invalid parent of directory/file inode */
/* XXX -- not repaired yet (file or directory) */
static int record_inode_badparent(SGlobPtr gptr, uint32_t inode_id, Boolean isdir,
		uint32_t incorrect, uint32_t correct)
{
	char str1[12];
	char str2[12];

	fsckPrint(gptr->context, isdir? E_DirInodeBadParent : E_FileInodeBadParent, inode_id);
	snprintf(str1, sizeof(str1), "%u", correct);
	snprintf(str2, sizeof(str2), "%u", incorrect);
	fsckPrint(gptr->context, E_BadValue, str1, str2);

	gptr->CatStat |= S_LinkErrNoRepair;

	return 0;
}

/* Record minor repair order for invalid name of directory inode */
/* XXX - not repaired yet (file or directory) */
static int record_inode_badname(SGlobPtr gptr, uint32_t inode_id,
		char *incorrect, char *correct)
{
	fsckPrint(gptr->context, E_DirInodeBadName, inode_id);
	fsckPrint(gptr->context, E_BadValue, correct, incorrect);

	gptr->CatStat |= S_LinkErrNoRepair;

	return 0;
}

/* Record corruption for incorrect number of directory hard links and 
 * directory inode, and invalid list of directory hard links
 */
void record_link_badchain(SGlobPtr gptr, Boolean isdir)
{
	int fval = (isdir ? S_DirHardLinkChain : S_FileHardLinkChain);
	int err = (isdir ? E_DirHardLinkChain : E_FileHardLinkChain);
	if ((gptr->CatStat & fval) == 0) {
		fsckPrint(gptr->context, err);
		gptr->CatStat |= fval;
	}
}

/* Record minor repair for invalid ownerflags for directory hard links.
 * If corruption is recorded during verification, do not check for 
 * duplicates as none should exist.  If this corruption is recorded 
 * during repair, check for duplicates because before early termination 
 * of verification, we might have seen this corruption.
 */
int record_dirlink_badownerflags(SGlobPtr gptr, uint32_t file_id,
			uint8_t incorrect, uint8_t correct, int check_duplicates)
{
	RepairOrderPtr p;
	char str1[12];
	char str2[12];

	p = AllocMinorRepairOrder(gptr, 0);
	if (p == NULL) {
		return ENOMEM;
	}
	
	p->type = E_DirHardLinkOwnerFlags;
	p->correct = correct;
	p->incorrect = incorrect;
	p->parid = file_id;

	gptr->CatStat |= S_LinkErrRepair;

	if ((check_duplicates != 0) && 
	    (IsDuplicateRepairOrder(gptr, p) == 1)) {
		DeleteRepairOrder(gptr, p);
	} else {
		fsckPrint(gptr->context, E_DirHardLinkOwnerFlags, file_id);
		snprintf(str1, sizeof(str1), "0x%x", correct);
		snprintf(str2, sizeof(str2), "0x%x", incorrect);
		fsckPrint(gptr->context, E_BadValue, str1, str2);
	}

	return 0;
}

/* Record minor repair for invalid finderInfo for directory hard links */
int record_link_badfinderinfo(SGlobPtr gptr, uint32_t file_id, Boolean isdir)
{
	RepairOrderPtr p;

	p = AllocMinorRepairOrder(gptr, 0);
	if (p == NULL) {
		return ENOMEM;
	}
	
	p->type = isdir ? E_DirHardLinkFinderInfo : E_FileHardLinkFinderInfo;
	p->parid = file_id;

	gptr->CatStat |= (isdir ? S_DirHardLinkChain : S_FileHardLinkChain);

	/* Recording this corruption is being called from both 
	 * inode_check() and dirlink_check().  It is possible that 
	 * the error we are adding is a duplicate error.  Check for 
	 * duplicates, and if any duplicates are found delete the new
	 * repair order.
	 */
	if (IsDuplicateRepairOrder(gptr, p) == 1) {
		DeleteRepairOrder(gptr, p);
	} else {
		fsckPrint(gptr->context, p->type, file_id);
	}

	return 0;
}

/* Record minor repair for invalid flags in one of the parent directories 
 * of a directory hard link.
 */
static int record_parent_badflags(SGlobPtr gptr, uint32_t dir_id,
		uint32_t incorrect, uint32_t correct)
{
	RepairOrderPtr p;
	char str1[12];
	char str2[12];

	p = AllocMinorRepairOrder(gptr, 0);
	if (p == NULL) {
		return ENOMEM;
	}
	
	p->type = E_DirLinkAncestorFlags;
	p->correct = correct;
	p->incorrect = incorrect;
	p->parid = dir_id;

	gptr->CatStat |= S_LinkErrRepair;

	/* This corruption is logged when traversing ancestors of all
	 * directory hard links.  Therefore common corrupt ancestors of 
	 * directory hard link will result in duplicate repair orders. 
	 * Check for duplicates, and if any duplicates are found delete 
	 * the new repair order.
	 */
	if (IsDuplicateRepairOrder(gptr, p) == 1) {
		DeleteRepairOrder(gptr, p);
	} else {
		fsckPrint(gptr->context, E_DirLinkAncestorFlags, dir_id);
		snprintf(str1, sizeof(str1), "0x%x", correct);
		snprintf(str2, sizeof(str2), "0x%x", incorrect);
		fsckPrint(gptr->context, E_BadValue, str1, str2);
	}

	return 0;
}

/* Look up the ".HFS+ Private Directory Data\xd" directory */
static int priv_dir_lookup(SGlobPtr gptr, CatalogKey *key, CatalogRecord *rec) 
{
	int i;
	int retval;
	char *dirname = HFSPLUS_DIR_METADATA_FOLDER;
	CatalogName cat_dirname;
	uint16_t recsize;
	uint32_t hint;

	/* Look up the catalog btree record for the private metadata directory */
	cat_dirname.ustr.length = strlen(dirname);
	for (i = 0; i < cat_dirname.ustr.length; i++) {
		cat_dirname.ustr.unicode[i] = (u_int16_t) dirname[i];
	}
	BuildCatalogKey(kHFSRootFolderID, &cat_dirname, true, key);
	retval = SearchBTreeRecord (gptr->calculatedCatalogFCB, key, kNoHint, 
	                            NULL, rec, &recsize, &hint);
	return retval;
} 

/* This function initializes the directory hard link check by looking up
 * private directory that stores directory inodes.  
 */
int dirhardlink_init(SGlobPtr gptr)
{
	int retval = 0; 
	CatalogRecord rec;
	CatalogKey key;

	/* Check if the volume is HFS+. */ 
	if (VolumeObjectIsHFSPlus() == false) {
		goto out;
	}

	/* Look up the private metadata directory */
	retval = priv_dir_lookup(gptr, &key, &rec);
	if (retval == 0) {
		gptr->dirlink_priv_dir_id = rec.hfsPlusFolder.folderID;
		gptr->dirlink_priv_dir_valence = rec.hfsPlusFolder.valence;
	} else {
		gptr->dirlink_priv_dir_id = 0;
		gptr->dirlink_priv_dir_valence = 0;
	}

	retval = 0;

out:
	return retval;
}

/* Check the private directory for directory hard links */
static void dirlink_priv_dir_check(SGlobPtr gptr, HFSPlusCatalogFolder *rec, 
		HFSPlusCatalogKey *key)
{
	/* ownerFlags should have UF_IMMUTABLE and UF_HIDDEN set, and 
	   fileMode should have S_ISVTX set */
	if (((rec->bsdInfo.ownerFlags & UF_IMMUTABLE) == 0) ||
	    //(((rec->bsdInfo.adminFlags << 16) & UF_HIDDEN) == 0) || 
	    ((rec->bsdInfo.fileMode & S_ISVTX) == 0)) {
		record_privdir_bad_perm(gptr, rec->folderID);
	}
}

/* Get the first link ID information for a hard link inode.
 * For directory inodes, we get it from the extended attribute 
 * of the directory inode; for files, we get it from hl_firstLinkID
 * Returns - zero if the lookup succeeded with the first link ID 
 * in the pointer provided, and non-zero if the extended attribute
 * does not exist, or any other error encountered during lookup.
 */
int get_first_link_id(SGlobPtr gptr, CatalogRecord *inode_rec, uint32_t inode_id, 
		Boolean isdir, uint32_t *first_link_id)
{
	int retval = 0;
	int i;
	BTreeIterator iterator;
	FSBufferDescriptor bt_data;
	HFSPlusAttrData *rec;
	HFSPlusAttrKey *key;
	u_int8_t attrdata[FIRST_LINK_XATTR_REC_SIZE];
	size_t unicode_bytes = 0;

	bzero(&iterator, sizeof(iterator));

	if (isdir) {
		/* Create key for the required attribute */
		key = (HFSPlusAttrKey *)&iterator.key;
		utf_decodestr((unsigned char *)FIRST_LINK_XATTR_NAME, 
			      strlen(FIRST_LINK_XATTR_NAME), key->attrName, 
			      &unicode_bytes, sizeof(key->attrName));
		key->attrNameLen = unicode_bytes / sizeof(UniChar);
		key->keyLength = kHFSPlusAttrKeyMinimumLength + unicode_bytes;
		key->pad = 0;
		key->fileID = inode_id;
		key->startBlock = 0;

		rec = (HFSPlusAttrData *)&attrdata[0];
		bt_data.bufferAddress = rec;
		bt_data.itemSize = sizeof(attrdata);
		bt_data.itemCount = 1;

		retval = BTSearchRecord(gptr->calculatedAttributesFCB, &iterator, kNoHint, 
					&bt_data, NULL, NULL);
		if (retval == 0) {
            unsigned long link_id;

			/* Attribute should be an inline attribute */
			if (rec->recordType != kHFSPlusAttrInlineData) {
				if (fsckGetVerbosity(gptr->context) >= kDebugLog) {
					plog ("\tfirst link EA is not inline for dirinode=%u (found=0x%x)\n", inode_id, rec->recordType);
				}
				retval = ENOENT;
				goto out;
			}
				
			/* Attribute data should be null terminated, attrSize includes
			 * size of the attribute data including the null termination.
			 */
			if (rec->attrData[rec->attrSize-1] != '\0') {
				if (fsckGetVerbosity(gptr->context) >= kDebugLog) {
					plog ("\tfirst link EA attrData is not NULL terminated for dirinode=%u\n", inode_id);
				}
				retval = ENOENT;
				goto out;
			}

			/* All characters are numbers in the attribute data */
			for (i = 0; i < rec->attrSize-1; i++) {
				if (isdigit(rec->attrData[i]) == 0) {
					if (fsckGetVerbosity(gptr->context) >= kDebugLog) {
						plog ("\tfirst link EA attrData contains non-digit 0x%x for dirinode=%u\n", rec->attrData[i], inode_id);
					}
					retval = ENOENT;
				goto out;
				}
			}

            link_id = strtoul((char *)&rec->attrData[0], NULL, 10);
            if (link_id > UINT32_MAX) {
                if (fsckGetVerbosity(gptr->context) >= kDebugLog) {
                    plog ("\tfirst link ID=%lu is > UINT32_MAX for dirinode=%u\n", link_id, inode_id);
                }
                *first_link_id = 0;
                retval = ENOENT;
                goto out;
            }
			*first_link_id = (uint32_t)link_id;
			if (*first_link_id < kHFSFirstUserCatalogNodeID) {
				if (fsckGetVerbosity(gptr->context) >= kDebugLog) {
					plog ("\tfirst link ID=%u is < 16 for dirinode=%u\n", *first_link_id, inode_id);
				}
				*first_link_id = 0;
				retval = ENOENT;
			goto out;
			}
		} 
	} else {
		*first_link_id = 0;
		if ((inode_rec != NULL) && 
		    (inode_rec->recordType == kHFSPlusFileRecord)) {
			*first_link_id = inode_rec->hfsPlusFile.hl_firstLinkID;
			if (*first_link_id < kHFSFirstUserCatalogNodeID) {
				if (fsckGetVerbosity(gptr->context) >= kDebugLog) {
					plog("\tfirst link ID=%u is < 16 for fileinode=%u\n", *first_link_id, inode_id);
				}
				*first_link_id = 0;
				retval = ENOENT;
				goto out;
			}
		} else {
			CatalogRecord rec;
			CatalogKey key;
			uint16_t recsize;

			/* No record or bad record provided, look it up */
			retval = GetCatalogRecordByID(gptr, inode_id, true, &key, &rec, &recsize);
			if (retval == 0) {
				*first_link_id = rec.hfsPlusFile.hl_firstLinkID;
				if (rec.recordType != kHFSPlusFileRecord ||
					*first_link_id < kHFSFirstUserCatalogNodeID) {
					if (fsckGetVerbosity(gptr->context) >= kDebugLog) {
						plog("\tfirst link ID=%u is < 16 for fileinode=%u\n", *first_link_id, inode_id);
					}
					*first_link_id = 0;
					retval = ENOENT;
				}
			} else {
				*first_link_id = 0;
				retval = ENOENT;
			}
		}
	}

out:
	return retval;
}

/* Adds the directory inode, and directory hard link pair to the 
 * prime remainder bucket provided.  This is based on Chinese Remainder
 * Theorem, and the buckets are later compared to find if the directory 
 * hard link chains for all directory inodes are valid.   
 */
void hardlink_add_bucket(PrimeBuckets *bucket, uint32_t inode_id, 
		uint32_t cur_link_id) 
{
	uint64_t num;

	num = ((uint64_t)inode_id << 32) | cur_link_id;

	add_prime_bucket_uint64(bucket, num);
}

/* Structure to store the directory hard link IDs found during doubly linked 
 * list traversal in inode_check() 
 */ 
struct link_list {
	uint32_t link_id;
	struct link_list *next;
};

/* Verifies the inode record.  Validates if the flags are set 
 * correctly, parent is the private metadata directory, first link ID 
 * is stored correctly, and the doubly linked * list of hard links is valid.  
 *
 * Returns - 
 * 	zero - 	if no corruption is detected, or the corruption detected is 
 *		such that a repair order can be created. 
 *  non-zero - 	if the corruption detected requires complete knowledge of 
 *		all the related directory hard links to suggest repair.
 */
int inode_check(SGlobPtr gptr, PrimeBuckets *bucket, 
		CatalogRecord *rec, CatalogKey *key, Boolean isdir)
{
	int retval = 0;
	uint32_t inode_id;
	uint32_t cur_link_id;
	uint32_t prev_link_id;
	uint32_t count;
	uint32_t linkCount;
	char calc_name[32];
	char found_name[NAME_MAX];
	size_t calc_len;
	size_t found_len;
	CatalogKey linkkey;
	CatalogRecord linkrec;
	uint16_t recsize;
	int flags;
	uint32_t parentid;
	uint32_t link_ref_num = 0;

	struct link_list *head = NULL;
	struct link_list *cur;

	(void) utf_encodestr(key->hfsPlus.nodeName.unicode, key->hfsPlus.nodeName.length * 2, 
			(unsigned char *)found_name, &found_len, NAME_MAX);
	found_name[found_len] = '\0';

	if (isdir) {
		inode_id = rec->hfsPlusFolder.folderID;
		flags = rec->hfsPlusFolder.flags;
		linkCount = rec->hfsPlusFolder.bsdInfo.special.linkCount;
		parentid = gptr->dirlink_priv_dir_id;
	} else {
        unsigned long ref_num;

		inode_id = rec->hfsPlusFile.fileID;
		flags = rec->hfsPlusFile.flags;
		linkCount = rec->hfsPlusFile.bsdInfo.special.linkCount;
		parentid = gptr->filelink_priv_dir_id;
        ref_num = strtoul(&found_name[strlen(HFS_INODE_PREFIX)], NULL, 10);
        if (ref_num > UINT32_MAX) {
            if (fsckGetVerbosity(gptr->context) >= kDebugLog) {
                plog ("\tlink reference num=%lu is > UINT32_MAX for inode=%u\n", ref_num, inode_id);
            }
            retval = 1;
            goto out;
        }
		link_ref_num = (uint32_t)ref_num;
	}

	/* inode should only reside in its corresponding private directory */
	if ((parentid != 0) && (key->hfsPlus.parentID != parentid)) {
		(void) record_inode_badparent(gptr, inode_id, isdir, key->hfsPlus.parentID, parentid);
	}

	/* Compare the names for directory inode only because the names 
	 * of file inodes can have random number suffixed.
	 */
	if (isdir) {
		(void) snprintf(calc_name, sizeof(calc_name), "%s%u", HFS_DIRINODE_PREFIX, inode_id);
		calc_len = strlen(calc_name);
	
		if ((found_len != calc_len) ||
		    (strncmp(calc_name, found_name, calc_len) != 0)) {
			(void) record_inode_badname(gptr, inode_id, found_name,
					calc_name);
		}
	}

	/* At least one hard link should always point at an inode. */
	if (linkCount == 0) {
		record_link_badchain(gptr, isdir);
		if (fsckGetVerbosity(gptr->context) >= kDebugLog) {
			plog ("\tlinkCount=0 for dirinode=%u\n", inode_id);
		}
		retval = 1;
		goto out;
	}

	/* A directory inode should always have kHFSHasLinkChainBit 
	 * set.  A file inode created on pre-Leopard OS does not have 
	 * kHFSHasLinkChainBit set and firstLinkID is zero.  Therefore 
	 * ignore such file inodes from CRT check and instead add the 
	 * the inode to hash used for checking link count. 
	 */
	if ((flags & kHFSHasLinkChainMask) == 0) {
		if ((isdir) || (!isdir && (rec->hfsPlusFile.hl_firstLinkID != 0))) {
			(void) record_inode_badflags(gptr, inode_id, isdir, 
					flags, flags | kHFSHasLinkChainMask, false);
		} else {
			filelink_hash_inode(link_ref_num, linkCount);
			retval = 0;
			goto out;
		}
	}

	/* Lookup the ID of first link from the extended attribute */
	retval = get_first_link_id(gptr, rec, inode_id, isdir, &cur_link_id);
	if (retval) {
		record_link_badchain(gptr, isdir);
		if (fsckGetVerbosity(gptr->context) >= kDebugLog) {
			plog ("\tError getting first link ID for inode=%u\n", inode_id);
		}
		goto out;
	}

	/* Check doubly linked list of hard links that point to this inode */
	prev_link_id = 0;
	count = 0;

	while (cur_link_id != 0) {
		/* Lookup the current directory link record */
		retval = GetCatalogRecordByID(gptr, cur_link_id, true, 
				&linkkey, &linkrec, &recsize);
		if (retval) {
			record_link_badchain(gptr, isdir);
			if (fsckGetVerbosity(gptr->context) >= kDebugLog) {
				plog ("\tError getting link=%u for inode=%u\n", cur_link_id, inode_id);
			}
			goto out;
		}

		/* Hard link is a file record */
		if (linkrec.recordType != kHFSPlusFileRecord) {
			record_link_badchain(gptr, isdir);
			if (fsckGetVerbosity(gptr->context) >= kDebugLog) {
				plog ("\tIncorrect record type for link=%u for inode=%u (expected=2, found=%u)\n", cur_link_id, inode_id, linkrec.recordType);
			}
			retval = 1;
			goto out;
		}
		   
		/* Hard link should have hard link bit set */
		if ((linkrec.hfsPlusFile.flags & kHFSHasLinkChainMask) == 0) {
			(void) record_link_badchain(gptr, isdir);
			if (fsckGetVerbosity(gptr->context) >= kDebugLog) {
				plog ("\tIncorrect flag for link=%u for inode=%u (found=0x%x)\n", cur_link_id, inode_id, linkrec.hfsPlusFile.flags);
			}
			retval = 1;
			goto out;
		}
		
		if (isdir) {
			/* Check if the hard link has correct finder info */
			if ((linkrec.hfsPlusFile.userInfo.fdType != kHFSAliasType) ||
			    (linkrec.hfsPlusFile.userInfo.fdCreator != kHFSAliasCreator) || 
			    ((linkrec.hfsPlusFile.userInfo.fdFlags & kIsAlias) == 0)) {
				record_link_badfinderinfo(gptr, linkrec.hfsPlusFile.fileID, isdir);
				if (fsckGetVerbosity(gptr->context) >= kDebugLog) {
					plog("\tdirlink: fdType = 0x%08lx, fdCreator = 0x%08lx\n", 
						(unsigned long)linkrec.hfsPlusFile.userInfo.fdType, 
						(unsigned long)linkrec.hfsPlusFile.userInfo.fdCreator);
				}
			}

			/* Check if hard link points to the current inode */
			if (linkrec.hfsPlusFile.hl_linkReference != inode_id) {
				record_link_badchain(gptr, isdir);
				if (fsckGetVerbosity(gptr->context) >= kDebugLog) {
					plog ("\tIncorrect dirinode ID for dirlink=%u (expected=%u, found=%u)\n", cur_link_id, inode_id, linkrec.hfsPlusFile.hl_linkReference);
				}
				retval = 1;
				goto out;
			}

		} else {
			/* Check if the hard link has correct finder info */
			if ((linkrec.hfsPlusFile.userInfo.fdType != kHardLinkFileType) ||
			    (linkrec.hfsPlusFile.userInfo.fdCreator != kHFSPlusCreator)) {
				record_link_badfinderinfo(gptr, linkrec.hfsPlusFile.fileID, isdir);
				if (fsckGetVerbosity(gptr->context) >= kDebugLog) {
					plog("\tfilelink: fdType = 0x%08lx, fdCreator = 0x%08lx\n", 
						(unsigned long)linkrec.hfsPlusFile.userInfo.fdType, 
						(unsigned long)linkrec.hfsPlusFile.userInfo.fdCreator);
				}
			}

			/* Check if hard link has correct link reference number */
			if (linkrec.hfsPlusFile.hl_linkReference != link_ref_num) {
				record_link_badchain(gptr, isdir);
				if (fsckGetVerbosity(gptr->context) >= kDebugLog) {
					plog ("\tIncorrect link reference number for filelink=%u (expected=%u, found=%u)\n", cur_link_id, inode_id, linkrec.hfsPlusFile.hl_linkReference);
				}
				retval = 1;
				goto out;
			}
		}

		/* For directory hard links, add the directory inode ID and 
		 * the current link ID pair to the prime bucket.  For file 
		 * hard links, add the link reference number and current 
		 * link ID pair to the prime bucket.
		 */
		if (isdir) {
			hardlink_add_bucket(bucket, inode_id, cur_link_id);
		} else {
			hardlink_add_bucket(bucket, link_ref_num, cur_link_id);
		}

		/* Check the previous directory hard link */
		if (prev_link_id != linkrec.hfsPlusFile.hl_prevLinkID) {
			record_link_badchain(gptr, isdir);
			if (fsckGetVerbosity(gptr->context) >= kDebugLog) {
				plog ("\tIncorrect prevLinkID for link=%u for inode=%u (expected=%u, found=%u)\n", cur_link_id, inode_id, prev_link_id, linkrec.hfsPlusFile.hl_prevLinkID);
			}
			retval = 1;
			goto out;
		}
		
		/* Check if we saw this directory hard link previously */
		cur = head;
		while (cur) {
			if (cur->link_id == cur_link_id) {
				if (fsckGetVerbosity(gptr->context) >= kDebugLog) {
					plog ("\tDuplicate link=%u found in list for inode=%u\n", cur_link_id, inode_id);
				}
				record_link_badchain(gptr, isdir);
				retval = 1;
				goto out;
			}
			cur = cur->next;
		}
		
		/* Add the new unique directory hard link to our list */
		cur = malloc(sizeof(struct link_list));
		if (!cur) {
			retval = ENOMEM;
			goto out;
		}
		cur->link_id = cur_link_id;
		cur->next = head;
		head = cur;

		count++;
		prev_link_id = cur_link_id;
		cur_link_id = linkrec.hfsPlusFile.hl_nextLinkID;
	}

	/* If the entire chain looks good, match the link count */
	if (linkCount != count) {
		record_link_badchain(gptr, isdir);
		if (fsckGetVerbosity(gptr->context) >= kDebugLog) {
			plog ("\tIncorrect linkCount for inode=%u (expected=%u, found=%u)\n", inode_id, count, linkCount);
		}
		retval = 1;
		goto out;
	}

out:
	/* Free memory used for checking duplicates in the doubly linked list */
	while(head) {
		cur = head;
		head = head->next;
		free(cur);
	}

	return retval;
}


/* Check if the parent ancestors starting at the given directory has
 * the kHFSHasChildLinkBit set.  This bit indicates that a descendant of 
 * this directory is a directory hard link.  Note that the root folder
 * and the "private directory data" directory does not have this bit 
 * set, and the check stops as soon as we encounter one of these 
 * directories.
 */
static void check_dirlink_ancestors(SGlobPtr gptr, uint32_t dir_id)
{
	int retval = 0;
	CatalogRecord rec;
	CatalogKey key;
	uint16_t recsize;

	while ((dir_id != kHFSRootFolderID) && (dir_id != gptr->dirlink_priv_dir_id)) {
		retval = GetCatalogRecordByID(gptr, dir_id, true, &key, &rec, &recsize);
		if (retval != 0) {
			break;
		}

		if (rec.recordType != kHFSPlusFolderRecord) {
			break;
		}

		if ((rec.hfsPlusFolder.flags & kHFSHasChildLinkMask) == 0) {
			(void) record_parent_badflags(gptr, dir_id,
					rec.hfsPlusFolder.flags,
					rec.hfsPlusFolder.flags | kHFSHasChildLinkMask);
		}

		dir_id = key.hfsPlus.parentID;
	}

	/* If there was any problem in looking up parent directory, 
	 * the catalog check should have also detected the problem.  
	 * But there are cases which are not detected like names in 
	 * thread record and file/folder record key do not match. 
	 * Therefore force repair for incorrect number of thread 
	 * records if lookup fails.
	 */
	if ((dir_id != kHFSRootFolderID) && (dir_id != gptr->dirlink_priv_dir_id)) {
		fsckPrint(gptr->context, E_BadParentHierarchy, dir_id);
		gptr->CBTStat |= S_Orphan;
	}

	return;
}

/* Verifies the directory hard link record.  Validates if the flags are set 
 * correctly, the finderInfo fields are correct, and if the parent hierarchy 
 * till the root folder (except the root folder) has the kHFSHasChildLinkBit 
 * set correctly.  This function also add the directory inode, and the  
 * directory hard link pair to the prime buckets for comparison later.
 *
 * This function does not verify the first and the next directory hard link
 * pointers in the doubly linked list because the check is already done 
 * in directory inode check (inode_check()) .  Any orphan directory 
 * hard link will also be detected later by the prime bucket comparison.
 */
static void dirlink_check(SGlobPtr gptr, PrimeBuckets *bucket, 
		HFSPlusCatalogFile *rec, HFSPlusCatalogKey *key, Boolean isdir)
{
	/* Add this directory hard link and corresponding inode number pair
	 * to prime buckets
	 */
#if DEBUG_HARDLINKCHECK
	if (fsckGetVerbosity(gptr->context) >= kDebugLog)
		plog("link_check:  adding <%u, %u>\n", rec->hl_linkReference, rec->fileID);
#endif

	hardlink_add_bucket(bucket, rec->hl_linkReference, rec->fileID);

	/* Check if the directory hard link has UF_IMMUTABLE bit set */
	if ((rec->bsdInfo.ownerFlags & UF_IMMUTABLE) == 0) {
		record_dirlink_badownerflags(gptr, rec->fileID, 
			rec->bsdInfo.ownerFlags, 
			rec->bsdInfo.ownerFlags | UF_IMMUTABLE, false);
	}

	/* Check Finder Info */
	if ((rec->userInfo.fdType != kHFSAliasType) ||
	    (rec->userInfo.fdCreator != kHFSAliasCreator) ||
	    ((rec->userInfo.fdFlags & kIsAlias) == 0)) {
	    	record_link_badfinderinfo(gptr, rec->fileID, isdir);
	}

	/* XXX - Check resource fork/alias data */

	/* Check if all the parent directories have the kHFSHasChildLinkBit set */
	check_dirlink_ancestors(gptr, key->parentID);
}

/* Searches the next child directory record to return given the parent ID
 * and the current child ID.  If the current child ID is zero, this is the 
 * first time we are looking up this directory, therefore return the 
 * first child directory or directory hard link found.  If child ID is 
 * non-zero, return the first child directory or directory hard 
 * link found after the current child record.
 * 
 * For normal directories, the folder ID is returned as the new child inode_id
 * and catalog_id.  For directory hard links, the inode_id of the directory 
 * inode is returned in the inode_id, and the fileID of the directory hard link
 * is returned in the catalog_id.  If the inode_id returned corresponds to a 
 * directory inode, is_dirinode is set to true.  If no child record is found, 
 * or an error occurred on btree traversal, these values are zero.
 *
 * Returns -
 *	zero - on successfully determining if the next child record exists 
 *             or not.
 *  non-zero - error, like during btree lookup, etc.
 */
static int find_next_child_dir(SGlobPtr gptr, uint32_t parent_id, 
		uint32_t cur_child_catalog_id, uint32_t *child_inode_id, 
		uint32_t *child_catalog_id, uint32_t *is_dirinode)
{	
	int retval;
	SFCB *fcb;
	int return_next_rec = true;
	BTreeIterator iterator;
	FSBufferDescriptor buf_desc;
	uint16_t recsize;
	CatalogRecord rec;
	CatalogKey *key;

	*child_inode_id = 0;
	*child_catalog_id = 0;
	*is_dirinode = false;

	fcb = gptr->calculatedCatalogFCB;
	key = (CatalogKey *)&iterator.key;

	/* If no child record for this parent has been looked up previously, 
	 * return the first child record found.  Otherwise lookup the 
	 * catalog record for the last child ID provided and return the 
	 * next valid child ID.  If the lookup of the last child failed, 
	 * fall back to iterating all child records for given parent
	 * directory and returning next child found after given child ID. 
	 */
	if (cur_child_catalog_id == 0) {
iterate_parent:
		/* Lookup catalog record with key containing given parent ID and NULL 
		 * name.  This will place iterator just before the first child record 
		 * for this directory.
		 */
		bzero(&iterator, sizeof(iterator));
		bzero(&buf_desc, sizeof(buf_desc));
		buf_desc.bufferAddress = &rec;
		buf_desc.itemCount = 1;
		buf_desc.itemSize = sizeof(rec);
		BuildCatalogKey(parent_id, NULL, true, key);
		retval = BTSearchRecord(fcb, &iterator, kNoHint, &buf_desc, &recsize, 
				&iterator);
		if ((retval != 0) && (retval != btNotFound)) {
			goto out;
		}
	} else {
		/* Lookup the thread record for the last child seen */
		bzero(&iterator, sizeof(iterator));
		bzero(&buf_desc, sizeof(buf_desc));
		buf_desc.bufferAddress = &rec;
		buf_desc.itemCount = 1;
		buf_desc.itemSize = sizeof(rec);
		BuildCatalogKey(cur_child_catalog_id, NULL, true, key);
		retval = BTSearchRecord(fcb, &iterator, kNoHint, &buf_desc,
				&recsize, &iterator);
		if (retval) {
			return_next_rec = false;
			goto iterate_parent;
		}

		/* Check if really we found a thread record */
		if ((rec.recordType != kHFSPlusFolderThreadRecord) &&
		    (rec.recordType != kHFSPlusFileThreadRecord)) {
			return_next_rec = false;
			goto iterate_parent;
		}
	
		/* Lookup the corresponding file/folder record */
		bzero(&iterator, sizeof(iterator));
		bzero(&buf_desc, sizeof(buf_desc));
		buf_desc.bufferAddress = &rec;
		buf_desc.itemCount = 1;
		buf_desc.itemSize = sizeof(rec);
		BuildCatalogKey(rec.hfsPlusThread.parentID, 
			(CatalogName *)&(rec.hfsPlusThread.nodeName),
			true, (CatalogKey *)&(iterator.key));
		retval = BTSearchRecord(fcb, &iterator, kInvalidMRUCacheKey, 
				&buf_desc, &recsize, &iterator);
		if (retval) {
			return_next_rec = false;
			goto iterate_parent;
		}
	}

	/* Lookup the next record */
	retval = BTIterateRecord(fcb, kBTreeNextRecord, &iterator, &buf_desc, 
			&recsize);
	while (retval == 0) {
		/* Not the same parent anymore, stop the search */
		if (key->hfsPlus.parentID != parent_id) {
			break;
		}

		if (rec.recordType == kHFSPlusFolderRecord) {
			/* Found a catalog folder record, and if we are 
			 * supposed to return the next record found, return 
			 * this catalog folder.
			 */ 
			if (return_next_rec) {
				if (rec.hfsPlusFolder.flags & kHFSHasLinkChainMask) {
					*is_dirinode = true;
				}
				*child_inode_id = rec.hfsPlusFolder.folderID;
				*child_catalog_id = rec.hfsPlusFolder.folderID;
				break;
			}
			/* If the current record is the current child, we 
			 * have to return the next child record.
			 */
			if (rec.hfsPlusFolder.folderID == cur_child_catalog_id) {
				return_next_rec = true;
			}
		} else if (rec.recordType == kHFSPlusFileRecord) {
			/* Check if the hard link bit is set with correct 
			 * alias type/creator.  If the parent is private 
			 * metadata directory for file hard links, this 
			 * is a hard link inode for an alias, and not 
			 * directory hard link.  Skip this file from our 
			 * check.
			 */
			if ((rec.hfsPlusFile.flags & kHFSHasLinkChainMask) &&
			    (rec.hfsPlusFile.userInfo.fdType == kHFSAliasType) &&
			    (rec.hfsPlusFile.userInfo.fdCreator == kHFSAliasCreator) &&
			    (key->hfsPlus.parentID != gptr->filelink_priv_dir_id)) {
			    	/* Found a directory hard link, and if we are 
				 * supposed to return the next record found,
				 * then return this directory hard link.
				 */
				if (return_next_rec) {
					*child_inode_id = rec.hfsPlusFile.hl_linkReference; 
					*child_catalog_id = rec.hfsPlusFile.fileID;
					*is_dirinode = true;
					break;
				}
				/* If the current record is the current child,
				 * we have to return the next child record.
				 */
				if (rec.hfsPlusFile.fileID == cur_child_catalog_id) {
					return_next_rec = true;
				}
			}
		}
		
		/* Lookup the next record */
		retval = BTIterateRecord(fcb, kBTreeNextRecord, &iterator, 
				&buf_desc, &recsize);
	}

	if (retval == btNotFound) {
		retval = 0;
	}

out:
	return retval;
} 

/* In-memory state for depth first traversal for finding loops in 
 * directory hierarchy.  inode_id is the user visible ID of the given 
 * directory or directory hard link, and catalog_id is the inode ID for 
 * normal directories, and the directory hard link ID (file ID of the 
 * directory hard link record).
 * 
 * The inode_id is used for checking loops in the hierarchy, whereas 
 * the catalog_id is used to maintain state for depth first traversal.
 */
struct dfs_id {
	uint32_t inode_id;
	uint32_t catalog_id;
};

struct dfs_stack {
	uint32_t depth;
	struct dfs_id *idptr;
};

/* Assuming that the name of a directory is single byte, the maximum depth 
 * of a directory hierarchy that can accommodate in PATH_MAX will be 
 * PATH_MAX/2.  Note that catalog hierarchy check puts limitation of 100 
 * on the maximum depth of a directory hierarchy.
 */
#define DIRLINK_DEFAULT_DFS_MAX_DEPTH 	PATH_MAX/2

/* Check if the current directory exists in the current traversal path.  
 * If yes, loops in directory exists and return non-zero value.  If not,
 * return zero.
 */
static int check_loops(struct dfs_stack *dfs, struct dfs_id id)
{
	int retval = 0;
	int i;

	for (i = 0; i < dfs->depth; i++) {
		if (dfs->idptr[i].inode_id == id.inode_id) {
			retval = 1;
			break;
		}
	}

	return retval;
}

static void print_dfs(struct dfs_stack *dfs)
{
	int i;

	plog ("\t");
	for (i = 0; i < dfs->depth; i++) {
		plog ("(%u,%u) ", dfs->idptr[i].inode_id, dfs->idptr[i].catalog_id);
	}
	plog ("\n");
}

/* Store information about visited directory inodes such that we do not 
 * reenter the directory multiple times while following directory hard links.
 */
struct visited_dirinode {
	uint32_t *list;		/* Pointer to array of IDs */
	uint32_t size;		/* Maximum number of entries in the array */
	uint32_t offset;	/* Offset where next ID will be added */
	uint32_t wrapped;	/* Boolean, true if list wraps around */
};

/* Add the given dirinode_id to the list of visited nodes.  If all the slots 
 * in visited list are used, wrap around and add the new ID.
 */
static void mark_dirinode_visited(uint32_t dirinode_id, struct visited_dirinode *visited)
{
	if (visited->list == NULL) {
		return;
	} 

	if (visited->offset >= visited->size) {
		visited->offset = 0;
		visited->wrapped = true;
	}
	visited->list[visited->offset] = dirinode_id;
	visited->offset++;
}

/* Check if given directory inode exists in the visited list or not */
static int is_dirinode_visited(uint32_t dirinode_id, struct visited_dirinode *visited)
{
	int is_visited = false;
	uint32_t end_offset;
	uint32_t off;

	if (visited->list == NULL) {
		return is_visited;
	} 

	/* If the list had wrapped, search the entire list */
	if (visited->wrapped == true) {
		end_offset = visited->size;	
	} else {
		end_offset = visited->offset;
	}

	for (off = 0; off < end_offset; off++) {
		if (visited->list[off] == dirinode_id) {
			is_visited = true;
			break;
		}
	}

	return is_visited;
}

/* Check if there are any loops in the directory hierarchy.  
 *
 * This function performs a depth first traversal of directories as they 
 * will be visible to the user.  If the lookup of private metadata directory 
 * succeeded in dirlink_init(), the traversal starts from the private 
 * metadata directory.  Otherwise it starts at the root folder.  It stores 
 * the current depth first traversal state, and looks up catalog records as 
 * required.  The current traversal state consists of two IDs, the user 
 * visible ID or inode_id, and the on-disk ID or catalog_id.  For normal 
 * directories, the user visible ID is same as the on-disk ID, but for 
 * directory hard links, the user visible ID is the inode ID, and the 
 * on-disk ID is the file ID of the directory hard link.  This function 
 * stores the list of visited directory inode ID and checks the list before 
 * traversing down the directory inode hierarchy.  After traversing down a 
 * directory inode and checking that is valid, it adds the directory inode 
 * ID to the visited list.  
 * 
 * The inode_id is used for checking loops in the hierarchy, whereas 
 * the catalog_id is used to maintain state for depth first traversal.
 * 
 * Returns - 
 * 	zero - if the check was performed successfully, and no loops exist
 *             in the directory hierarchy.
 *  non-zero - on error, or if loops were detected in directory hierarchy.
 */
static int check_hierarchy_loops(SGlobPtr gptr) 
{
	int retval = 0;
	struct dfs_stack dfs;
	struct dfs_id unknown_child;
	struct dfs_id child;
	struct dfs_id parent;
	struct visited_dirinode visited;
	size_t max_alloc_depth = DIRLINK_DEFAULT_DFS_MAX_DEPTH;
	uint32_t is_dirinode;

#define DFS_PUSH(dfsid) \
		{ \
			dfs.idptr[dfs.depth].inode_id = dfsid.inode_id; \
			dfs.idptr[dfs.depth].catalog_id = dfsid.catalog_id; \
			dfs.depth++; \
			if (dfs.depth == max_alloc_depth) { \
				void *tptr = realloc(dfs.idptr, (max_alloc_depth + DIRLINK_DEFAULT_DFS_MAX_DEPTH) * sizeof(struct dfs_id)); \
				if (tptr == NULL) { \
					break; \
				} else { \
					dfs.idptr = tptr; \
					max_alloc_depth += DIRLINK_DEFAULT_DFS_MAX_DEPTH; \
				} \
			} \
		}

#define DFS_POP(dfsid) \
		{ \
			dfs.depth--; \
			dfsid.inode_id = dfs.idptr[dfs.depth].inode_id; \
			dfsid.catalog_id = dfs.idptr[dfs.depth].catalog_id; \
		}
	
#define DFS_PEEK(dfsid) \
		{ \
			dfsid.inode_id = dfs.idptr[dfs.depth-1].inode_id; \
			dfsid.catalog_id = dfs.idptr[dfs.depth-1].catalog_id; \
		}

	/* Initialize the traversal stack */
	dfs.idptr = malloc(max_alloc_depth * sizeof(struct dfs_id));
	if (!dfs.idptr) {
		return ENOMEM;
	}
	dfs.depth = 0;

	/* Initialize unknown child IDs which are used when a directory is 
	 * seen for the first time.
	 */
	unknown_child.inode_id = unknown_child.catalog_id = 0;

	/* Allocate visited list for total number of directory inodes seen */
	if (gptr->calculated_dirinodes) {
		visited.size = gptr->calculated_dirinodes;
	} else {
		visited.size = 1024;
	}

	/* If visited list allocation failed, perform search without cache */
	visited.list = malloc(visited.size * sizeof(uint32_t)); 
	if (visited.list == NULL) {
		visited.size = 0;
		if (fsckGetVerbosity(gptr->context) >= kDebugLog) {
			plog ("\tcheck_loops: Allocation failed for visited list\n");
		}
	}
	visited.offset = 0;
	visited.wrapped = false;

	/* Set the starting directory for traversal */
	if (gptr->dirlink_priv_dir_id) {
		parent.inode_id = parent.catalog_id = gptr->dirlink_priv_dir_id;
	} else {
		parent.inode_id = parent.catalog_id = kHFSRootFolderID;
	}

	/* Initialize the first parent and its first unknown child */
	do {
		DFS_PUSH(parent);
		DFS_PUSH(unknown_child);
	} while (0);

	while (dfs.depth > 1) {
		DFS_POP(child);
		DFS_PEEK(parent);
		retval = find_next_child_dir(gptr, parent.inode_id,
				child.catalog_id, &(child.inode_id),
				&(child.catalog_id), &is_dirinode);
		if (retval) {
			break;
		}

		if (child.inode_id) {
			retval = check_loops(&dfs, child);
			if (retval) {
				fsckPrint(gptr->context, E_DirLoop);
				if (fsckGetVerbosity(gptr->context) >= kDebugLog) {
					plog ("\tDetected when adding (%u,%u) to following traversal stack -\n", child.inode_id, child.catalog_id);
					print_dfs(&dfs);
				}
				gptr->CatStat |= S_LinkErrNoRepair;
				retval = E_DirLoop;
				break;
			}

			/* Push the current child on traversal stack */
			DFS_PUSH(child);

			/* Traverse down directory inode only if it was not 
			 * visited previously and mark it visited.  
			 */
			if (is_dirinode == true) {
				if (is_dirinode_visited(child.inode_id, &visited)) {
					continue;
				} else {
					mark_dirinode_visited(child.inode_id, &visited);
				}
			}

			/* Push unknown child to traverse down the child directory */ 
			DFS_PUSH(unknown_child);
		}
	}

	if (dfs.depth >= max_alloc_depth) {
		fsckPrint(gptr->context, E_DirHardLinkNesting);
		if (fsckGetVerbosity(gptr->context) >= kDebugLog) {
			print_dfs(&dfs);
		}
		gptr->CatStat |= S_LinkErrNoRepair;
		retval = E_DirHardLinkNesting;
	}

	if (dfs.idptr) {
		free(dfs.idptr);
	}
	if (visited.list) {
		free(visited.list);
	}
	return retval;
}

/* This function traverses the entire catalog btree, and checks all
 * directory inodes and directory hard links found.
 *
 * Returns zero if the check is successful, and non-zero if an error was 
 * encountered during verification.
 */
int dirhardlink_check(SGlobPtr gptr) 
{
	int retval = 0;
	uint16_t selcode;
	uint32_t hint;
	
	CatalogRecord catrec;
	CatalogKey catkey;
	uint16_t recsize;

	PrimeBuckets *inode_view = NULL;
	PrimeBuckets *dirlink_view = NULL;

	/* Check if the volume is HFS+ */
	if (VolumeObjectIsHFSPlus() == false) {
		goto out;
	}

	/* Shortcut out if no directory hard links exists on the disk */
	if ((gptr->dirlink_priv_dir_valence == 0) &&
	    (gptr->calculated_dirlinks == 0) &&
	    (gptr->calculated_dirinodes == 0)) {
		goto out;
	}

	fsckPrint(gptr->context, hfsMultiLinkDirCheck);

	if (fsckGetVerbosity(gptr->context) >= kDebugLog) {
		plog ("\tprivdir_valence=%u, calc_dirlinks=%u, calc_dirinode=%u\n", gptr->dirlink_priv_dir_valence, gptr->calculated_dirlinks, gptr->calculated_dirinodes); 
	}

	/* If lookup of private directory failed and the volume has 
	 * some directory hard links and directory inodes, we will need
	 * to create the private directory for directory hard links.
	 */
	if (gptr->dirlink_priv_dir_id == 0) {
		fsckPrint(gptr->context, E_MissingPrivDir);
		gptr->CatStat |= S_LinkErrNoRepair;
	}

	/* Initialize the two prime number buckets, both buckets keep track
	 * of inode ID and corresponding directory hard link ID.  The first 
	 * bucket is filled when traversing the directory hard link doubly 
	 * linked list from the directory inode, and the second bucket is 
	 * filled when btree traversal encounters directory hard links.
	 * This method quickly allows us to check if the mapping of all 
	 * inodes and directory hard links is same, and no orphans exists.
	 */
	inode_view = (PrimeBuckets *)calloc(1, sizeof(PrimeBuckets));
	if (!inode_view) {
		retval = ENOMEM;
		goto out;
	}

	dirlink_view = (PrimeBuckets *)calloc(1, sizeof(PrimeBuckets));
	if (!dirlink_view) {
		retval = ENOMEM;
		goto out;
	}
	
	/* Traverse the catalog btree from the first record */
	selcode = 0x8001;
	retval = GetBTreeRecord(gptr->calculatedCatalogFCB, selcode, &catkey,
			&catrec, &recsize, &hint);
	if (retval != 0) {
		goto out;
	}

	/* Set code to get the next record */
	selcode = 1;
	do {
		if (catrec.hfsPlusFolder.recordType == kHFSPlusFolderRecord) {
			/* Check directory hard link private metadata directory */
			if (catrec.hfsPlusFolder.folderID == gptr->dirlink_priv_dir_id) {
				dirlink_priv_dir_check(gptr, 
					&(catrec.hfsPlusFolder), &(catkey.hfsPlus));
			}

			/* Check directory inode */
			if ((catrec.hfsPlusFolder.flags & kHFSHasLinkChainMask) ||
			    (catkey.hfsPlus.parentID == gptr->dirlink_priv_dir_id)) {
				retval = inode_check(gptr, inode_view,
						&catrec,
						&catkey,
						true);
				if (retval) {
					/* If the corruption detected requires
					 * knowledge of all associated directory
					 * hard links for repair, stop the 
					 * catalog btree traversal
					 */
					retval = 0;
					break;
				}
			}
		} else 
		if (catrec.recordType == kHFSPlusFileRecord) {
			/* Check if the hard link bit is set with correct 
			 * alias type/creator.  If the parent is private 
			 * metadata directory for file hard links, this 
			 * is a hard link inode for an alias, and not 
			 * directory hard link.  Skip this file from our 
			 * check.
			 */
			if ((catrec.hfsPlusFile.flags & kHFSHasLinkChainMask) &&
			    (catrec.hfsPlusFile.userInfo.fdType == kHFSAliasType) &&
			    (catrec.hfsPlusFile.userInfo.fdCreator == kHFSAliasCreator) &&
			    (catkey.hfsPlus.parentID != gptr->filelink_priv_dir_id)) {
				dirlink_check(gptr, dirlink_view, 
					&(catrec.hfsPlusFile), &(catkey.hfsPlus), true);
			}
		}

		retval = GetBTreeRecord(gptr->calculatedCatalogFCB, 1, 
				&catkey, &catrec, &recsize, &hint);
	} while (retval == noErr);

	if (retval == btNotFound) {
		retval = 0;
	} else if (retval != 0) {
		goto out;
	}

	/* Compare the two prime number buckets only the if catalog traversal did 
	 * not detect incorrect number of directory hard links corruption.
	 */
	if ((gptr->CatStat & S_DirHardLinkChain) == 0) {
		retval = compare_prime_buckets(inode_view, dirlink_view);
		if (retval) {
			record_link_badchain(gptr, true);
			if (fsckGetVerbosity(gptr->context) >= kDebugLog) {
				plog ("\tdirlink prime buckets do not match\n");
			}
			retval = 0;
		}
	}

	/* Check if there are any loops in the directory hierarchy */
	retval = check_hierarchy_loops(gptr);
	if (retval) {
		retval = 0;
		goto out;
	}

out:
	if (inode_view) {
		free (inode_view);
	}
	if (dirlink_view) {
		free (dirlink_view);
	}

	return retval;
}

