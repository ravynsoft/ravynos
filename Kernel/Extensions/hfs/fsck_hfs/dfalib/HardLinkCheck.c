/*
 * Copyright (c) 2000-2002, 2004-2008 Apple Inc. All rights reserved.
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
#include <sys/stat.h>

#define DEBUG_HARDLINKCHECK 0

/* If set, the node in hash is initialized and has valid inodeID */ 
#define LINKINFO_INIT	0x01
/* If set, verification of corresponding inode is completed successfully */
#define LINKINFO_CHECK	0x02

/* info saved for each indirect link encountered */
struct IndirectLinkInfo {
	/* linkID is link reference number for file hard links, and 
	 * inodeID for directory hard links.
	 */
	UInt32	linkID;	
	UInt32	linkCount;
	UInt32 	flags;
	struct HardLinkList *list;
};

#define VISITED_INODE_ID 1

struct HardLinkInfo {
	UInt32	privDirID;
	SGlobPtr globals;
	uint32_t	priv_dir_itime;	/* Creation (initialization) time of metadata folder */
	uint32_t	root_dir_itime;	/* Creation (initialization) time of root folder */
	PrimeBuckets	*fileBucket;
};

struct HardLinkList {
	UInt32	prev;
	UInt32	fileID;
	UInt32	next;
};

HFSPlusCatalogKey gMetaDataDirKey = {
	48,		/* key length */
	2,		/* parent directory ID */
	{
		21,	/* number of unicode characters */
		{
			'\0','\0','\0','\0',
			'H','F','S','+',' ',
			'P','r','i','v','a','t','e',' ',
			'D','a','t','a'
		}
	}
};

/* private local routines */
static int  GetPrivateDir(SGlobPtr gp, CatalogRecord *rec);
static int  GetRootDir(SGlobPtr gp, CatalogRecord *rec);
static int  RecordOrphanOpenUnlink(SGlobPtr gp, UInt32 parID, unsigned char * filename);
static int  RecordBadHardLinkChainFirst(SGlobPtr, UInt32, UInt32, UInt32);
static int  RecordBadHardLinkNext(SGlobPtr gp, UInt32 fileID, UInt32 is, UInt32 shouldbe);
static int  RecordBadHardLinkPrev(SGlobPtr gp, UInt32 fileID, UInt32 is, UInt32 shouldbe);
static int  RecordBadLinkCount(SGlobPtr gp, UInt32 inodeID, UInt32 is, UInt32 shouldbe) ;
static int  RecordOrphanLink(SGlobPtr gp, Boolean isdir, UInt32 linkID);
static int  RecordOrphanInode(SGlobPtr gp, Boolean isdir, UInt32 inodeID);
static void hash_insert(UInt32 linkID, int totalSlots, int slotsUsed, struct IndirectLinkInfo *linkInfo);
static struct IndirectLinkInfo * hash_search(UInt32 linkID, int totalSlots, int slotsUsed, struct IndirectLinkInfo *linkInfo);

/*
 * Some functions used when sorting the hard link chain.
 * chain_compare() is used by qsort; find_id is just a linear
 * search to find a specific fileID; and tsort does a
 * topological sort on the linked list.
 */
static int
chain_compare(const void *a1, const void *a2) {
	struct HardLinkList *left = (struct HardLinkList*)a1;
	struct HardLinkList *right = (struct HardLinkList*)a2;

	return (left->prev - right->prev);
}

static int
find_id(struct HardLinkList *list, int nel, int id)
{
	int i;
	for (i = 0; i < nel; i++) {
		if (list[i].fileID == id)
			return i;
	}
	return 0;
}

static int
tsort(struct HardLinkList *list, int nel)
{
	struct HardLinkList *tmp;
	int cur_indx, tmp_indx = 0;

	int rv = 0;

	tmp = calloc(sizeof(struct HardLinkList), nel);
	if (tmp == NULL) {
		rv = ENOMEM;
		goto done;
	}

	/*
	 * Since we only check list.next when doing the sort, we want to
	 * start with nodes that have prev == 0 (ones at the top of the
	 * graph, in other words).  If there aren't any with a prev of 0,
	 * then the chain is broken somehow, and we'll repair it later.
	 */
	qsort(list, nel, sizeof(list[0]), chain_compare);

	for (cur_indx = 0; cur_indx < nel; cur_indx++) {
		int i;
		/* Skip nodes we've already come across */
		if (list[cur_indx].fileID == 0)
			continue;

		/* Copy this node over to the new list */
		tmp[tmp_indx++] = list[cur_indx];
		list[cur_indx].fileID = 0;

		/* ... and then find all its children. */
		for (i = tmp[tmp_indx-1].next; i != 0; ) {
			// look for the node in list with that fileID
			int j = find_id(list, nel, i);
			if (j == 0) {
				// We couldn't find it
				// So we're done
				i = 0;
			} else {
				// we add this one to tmp
				tmp[tmp_indx++] = list[j];
				list[j].fileID = 0;
				i = tmp[tmp_indx-1].next;
			}
		}
	}

	/* Copy the temporary list over, and we're done. */
	memcpy(list, tmp, nel * sizeof(struct HardLinkList));
done:
	if (tmp) {
		free(tmp);
	}

	return rv;
}

/*
 * CheckHardLinkList
 *
 * Verify that the linked list of hard link nodes (the catalog entries, not the indirect
 * node in the private metadata directory) are correct and sane.  If any discrepancies 
 * are detected, create repair order.
 *
 * To do this, we need to topologically sort the list, and then ensure that the prev/next
 * chains are correct.
 *
 */
static int
CheckHardLinkList(SGlobPtr gp, UInt32 inodeID, struct HardLinkList *list, int calc_link_count, UInt32 firstID)
{
	int retval;
	int indx;

	if (list == NULL) {
		if (fsckGetVerbosity(gp->context) >= kDebugLog) {
			plog("\tCheckHardLinkList: list=NULL for inodeID = %u\n", inodeID);
		}
		return ENOMEM;
	}
	/*
	 * If we have a list, then we sort and verify it.  It's pretty easy, once
	 * we're sorted, and tsort() above does the hard work for that.
	 */
	if (calc_link_count > 1) {
		retval = tsort(list, calc_link_count);
		if (retval) {
			goto done;
		}
	}

	/* Previous link of first link should always be zero */
	if (list[0].prev != 0) {
		RecordBadHardLinkPrev(gp, list[0].fileID, list[0].prev, 0);
	}

	/* First ID in the inode should match the ID of the first hard link */
	if (list[0].fileID != firstID) {
		RecordBadHardLinkChainFirst(gp, inodeID, firstID, list[0].fileID);
	}

	/* Check if the previous/next IDs for all nodes except the first node are valid */
	for (indx = 1; indx < calc_link_count; indx++) {
		if (list[indx-1].next != list[indx].fileID) {
			RecordBadHardLinkNext(gp, list[indx-1].fileID, list[indx-1].next, list[indx].fileID);
		}

		if (list[indx].prev != list[indx-1].fileID) {
			RecordBadHardLinkPrev(gp, list[indx].fileID, list[indx].prev, list[indx-1].fileID);
		}
	}

	/* Next ID for the last link should always be zero */
	if (list[calc_link_count-1].next != 0) {
		RecordBadHardLinkNext(gp, list[calc_link_count-1].fileID, list[calc_link_count-1].next, 0);
	}

done:
#if DEBUG_HARDLINKCHECK
	/* This is just for debugging -- it's useful to know what the list looks like */
	if (fsckGetVerbosity(gp->context) >= kDebugLog) {
		for (indx = 0; indx < calc_link_count; indx++) {
			fplog(stderr, "CNID %u: #%u:  <%u, %u, %u>\n", inodeID, indx, list[indx].prev, list[indx].fileID, list[indx].next);
		}
	}
#endif

	return 0;
}

/*
 * HardLinkCheckBegin
 *
 * Get ready to capture indirect link info.
 * Called before iterating over all the catalog leaf nodes.
 */
int
HardLinkCheckBegin(SGlobPtr gp, void** cookie)
{
	struct HardLinkInfo *info;
	CatalogRecord rec;
	CatalogRecord rootFolder;
	UInt32 folderID;

	if (GetPrivateDir(gp, &rec) == 0) {
		folderID = rec.hfsPlusFolder.folderID;
	} else {
		folderID = 0;
	}
	
	info = (struct HardLinkInfo *) malloc(sizeof(struct HardLinkInfo));

	if (info == NULL) {
		if (fsckGetVerbosity(gp->context) >= kDebugLog) {
			plog("hardLinkCheckBegin:  malloc(%zu) failed\n", sizeof(struct HardLinkInfo));
		}
		return 1;
	}

	info->privDirID = folderID;
	info->priv_dir_itime = folderID ? rec.hfsPlusFolder.createDate : 0;
	if (GetRootDir(gp, &rootFolder) == 0) {
			info->root_dir_itime = rootFolder.hfsPlusFolder.createDate;
	} else {
			info->root_dir_itime = 0;
	}

	info->globals = gp;
	
	/* We will use the ID of private metadata directory for file hard 
	 * links to skip over hard link inode for an alias from directory 
	 * hard link checks.
	 */
	gp->filelink_priv_dir_id = folderID;


	info->fileBucket = calloc(1, sizeof(PrimeBuckets));
	if (info->fileBucket == NULL) {
		if (fsckGetVerbosity(gp->context) >= kDebugLog) {
			plog("HardLinkCheckBegin: prime bucket allocation failed\n");
		}
	}

	* cookie = info;
	return (0);
}

/*
 * HardLinkCheckEnd
 *
 * Dispose of captured data.
 * Called after calling CheckHardLinks.
 */
void
HardLinkCheckEnd(void * cookie)
{
	if (cookie) {
		struct HardLinkInfo *		infoPtr;
		
		infoPtr = (struct HardLinkInfo *) cookie;
		if (infoPtr->fileBucket) {
			free(infoPtr->fileBucket);
			infoPtr->fileBucket = NULL;
		}
		DisposeMemory(cookie);
	}

}

/* Structures for file hard link hash used when a 
 * file hard link created in pre-Leopard OS is detected 
 * i.e. the file inode and hard links do not have the 
 * kHFSHasLinkChainBit set, and the first link, the 
 * previous link and the next link IDs are zero.  The 
 * link count for such hard links cannot be verified 
 * using CRT, therefore it is accounted in this hash.
 */
#define FILELINK_HASH_SIZE 257

struct filelink_hash {
	UInt32 link_ref_num;
	UInt32 found_link_count;
	UInt32 calc_link_count;
	struct filelink_hash *next;
};

struct filelink_hash **filelink_head = NULL;
UInt32 filelink_entry_count = 0;

/* Search and return pointer to the entry for given inode ID.
 * If no entry is found, return NULL.
 */
static struct filelink_hash *filelink_hash_search(UInt32 link_ref_num) 
{
	struct filelink_hash *cur;

	if (filelink_head == NULL) {
		return NULL;
	}

	cur = filelink_head[link_ref_num % FILELINK_HASH_SIZE];
	while (cur) {
		if (link_ref_num == cur->link_ref_num) {
			break;
		}
		cur = cur->next;
	}

	return cur;
}

/* Allocate and insert entry for given inode ID in the 
 * hash.  The caller function is responsible for searching 
 * for duplicates before calling this function.  
 * Returns the pointer to the new hash entry.
 */
static struct filelink_hash *filelink_hash_insert(UInt32 link_ref_num) 
{
	struct filelink_hash *cur;

	cur = malloc(sizeof(struct filelink_hash));
	if (!cur) {
		return cur;
	}
	cur->link_ref_num = link_ref_num;
	cur->found_link_count = 0;
	cur->calc_link_count = 0;
	cur->next = filelink_head[link_ref_num % FILELINK_HASH_SIZE];
	filelink_head[link_ref_num % FILELINK_HASH_SIZE] = cur; 
	filelink_entry_count++;
	return cur;
}

/* Update the hash with information about a file hard link 
 * that points to given inode ID.  The calculated link count 
 * for given inode is incremented.
 * Returns zero if the value was successfully updated to hash,
 * and ENOMEM on error.
 */
static int filelink_hash_link(UInt32 link_ref_num)
{
	struct filelink_hash *cur;
	
	/* If no hash exists, allocate the hash */
	if (filelink_head == NULL) {
		filelink_head = calloc(FILELINK_HASH_SIZE, sizeof(struct filelink_hash *));
		if (filelink_head == NULL) {
			return ENOMEM;
		}
	}

	cur = filelink_hash_search(link_ref_num);
	if (!cur) {
		cur = filelink_hash_insert(link_ref_num);
	}
	if (cur) {
		cur->calc_link_count++;
		return 0;
	}

	return ENOMEM;
}

/* Update the hash with information about given file inode. 
 * The found link count in the hash is updated with the 
 * link count value provided.
 * Returns zero if the value was successfully updated to hash,
 * and ENOMEM on error.
 */ 
int filelink_hash_inode(UInt32 link_ref_num, UInt32 linkCount)
{
	struct filelink_hash *cur;

	/* If no hash exists, allocate the hash */
	if (filelink_head == NULL) {
		filelink_head = calloc(FILELINK_HASH_SIZE, sizeof(struct filelink_hash *));
		if (filelink_head == NULL) {
			return ENOMEM;
		}
	}

	cur = filelink_hash_search(link_ref_num);
	if (!cur) {
		cur = filelink_hash_insert(link_ref_num);
	}
	if (cur) {
		cur->found_link_count = linkCount;
		return 0;
	} 
	return ENOMEM;
}

/* If the file link hash was used to account for 
 * link count of file hard links created on pre-Leopard
 * OS, destroy the hash by freeing all allocated 
 * memory.
 */
static void filelink_hash_destroy(void) 
{
	int i;
	struct filelink_hash *cur;

	for (i = 0; i < FILELINK_HASH_SIZE; i++) {
		while (filelink_head[i]) {
			cur = filelink_head[i];
			filelink_head[i] = cur->next;
			free (cur);
		}
	}
	free(filelink_head);
	filelink_head = NULL;
	filelink_entry_count = 0;
}

/*
 * CaptureHardLink
 *
 * Capture indirect link info.
 * Called for every indirect link in the catalog.
 */
void
CaptureHardLink(void *cookie, const HFSPlusCatalogFile *file)
{
	struct HardLinkInfo * info = (struct HardLinkInfo *) cookie;

	/* A file hard link created on pre-Leopard OS does not 
	 * have kHFSHasLinkChainBit set or prev/next link IDs. 
	 * Ignore such file hard links from all check and CRT account 
	 * and instead account the information in hash to verify the 
	 * link counts later.
	 */
	if ((info->fileBucket == NULL) || 
	    (((file->flags & kHFSHasLinkChainMask) == 0) && 
	     (file->hl_prevLinkID == 0) && 
	     (file->hl_nextLinkID == 0))) {
		filelink_hash_link(file->hl_linkReference);
	} else {
		/* For file hard links, add link reference number 
		 * and catalog link ID pair to the prime buckets.
		 */
		hardlink_add_bucket(info->fileBucket, file->hl_linkReference, 
			file->fileID);

		if ((file->flags & kHFSHasLinkChainMask) == 0) {
			record_link_badchain(info->globals, false);
		}
	}
	if ((info->priv_dir_itime && file->createDate != info->priv_dir_itime) &&
		(info->root_dir_itime && file->createDate != info->root_dir_itime)) {
		RepairOrderPtr p;
		char str1[12];
		char str2[12];
		uint32_t correct;

		if (debug)
			plog("Hard Link catalog entry %u has bad time %u (should be %u, or at least %u)\n",
				file->fileID, file->createDate, info->priv_dir_itime, info->root_dir_itime);
		correct = info->priv_dir_itime;

		p = AllocMinorRepairOrder(info->globals, 0);
		if (p == NULL) {
			if (debug)
				plog("Unable to allocate hard link time repair order!");
			return;
		}

		fsckPrint(info->globals->context, E_BadHardLinkDate);
		snprintf(str1, sizeof(str1), "%u", correct);
		snprintf(str2, sizeof(str2), "%u", file->createDate);
		fsckPrint(info->globals->context, E_BadValue, str1, str2);

		p->type = E_BadHardLinkDate;
		p->parid = file->fileID;
		p->correct = info->priv_dir_itime;
		p->incorrect = file->createDate;
	}

	return;
}

/*
 * RepairHardLinkChains
 *
 * Cycle through the catalog tree, and generate repair orders for hard 
 * links that may be broken.
 */
int
RepairHardLinkChains(SGlobPtr gp, Boolean isdir)
{
	int result = 0;
	struct IndirectLinkInfo *linkInfo = NULL;
	CatalogRecord	rec;
	HFSPlusCatalogKey	*keyp;
	BTreeIterator	iterator;
	FSBufferDescriptor	btrec;
	UInt16	reclen;
	UInt32	linkID, inodeID;
	UInt32	metadirid;
	SFCB	*fcb;
	size_t	prefixlen;
	int	slotsUsed = 0, slots = 0;
	char *prefixName;
	UInt32 folderID;
	UInt32 link_ref_num;
	int entries;
	UInt32 flags;

	if (isdir) {
		metadirid = gp->dirlink_priv_dir_id;
		prefixlen = strlen(HFS_DIRINODE_PREFIX);
		prefixName = HFS_DIRINODE_PREFIX;
	} else {
		metadirid = gp->filelink_priv_dir_id;
		prefixlen = strlen(HFS_INODE_PREFIX);
		prefixName = HFS_INODE_PREFIX;
	}

	if (metadirid == 0) {
		if (fsckGetVerbosity(gp->context) >= kDebugLog) {
			if (isdir) {
				plog ("\tPrivate directory for dirlinks not found.  Stopping repairs.\n");
			} else {
				plog ("\tPrivate directory for filelinks not found.  Stopping repairs.\n");
			}
		} 
		result = ENOENT;
		goto done;
	}

	// Initialize the hash
	if (GetPrivateDir(gp, &rec) == 0 && rec.hfsPlusFolder.valence != 0) {
		entries = rec.hfsPlusFolder.valence + 10;
		folderID = rec.hfsPlusFolder.folderID;
	} else {
		entries = 1000;
		folderID = 0;
	}

	for (slots = 1; slots <= entries; slots <<= 1)
		continue;
	if (slots < (entries + (entries/3)))
		slots <<= 1;
	linkInfo = calloc(slots, sizeof(struct IndirectLinkInfo));
	if (linkInfo == NULL) {
		if (fsckGetVerbosity(gp->context) >= kDebugLog) {
			plog("RepairHardLinkChains:  calloc(%d, %zu) failed\n", slots, sizeof(struct IndirectLinkInfo));
		}
		result = ENOMEM;
		goto done;
	}
	// Done initializing the hash

	// Set up the catalog BTree iterator
	// (start from the root folder, and work our way down)
	fcb = gp->calculatedCatalogFCB;
	ClearMemory(&iterator, sizeof(iterator));
	keyp = (HFSPlusCatalogKey*)&iterator.key;
	BuildCatalogKey(kHFSRootFolderID, NULL, true, (CatalogKey*)keyp);
	btrec.bufferAddress = &rec;
	btrec.itemCount = 1;
	btrec.itemSize = sizeof(rec);

	/* Counter for number of inodes found and verified in the
	 * hash.  When an inode is found when checking the hard links,
	 * the value is incremented.  When an inode's linked list and 
	 * link count are verified, the value is decremented.  If 
	 * this value remains non-zero at the end, there are 
	 * orphan hard links.
	 */
	entries = 0;

	/*
	 * This chunk of code iterates through the entire catalog BTree.
	 * For each hard link node (that is, the "directory entry" that
	 * points to the actual node in the metadata directory), it may
	 * add it to the hash (if it doesn't exist yet; it also then increments
	 * the link count for that "inode"); it also creates an array
	 * of <previous, fileid, next> for the linked list.
	 */
	for (result = BTIterateRecord(fcb, kBTreeFirstRecord, &iterator, &btrec, &reclen);
		result == 0;
		result = BTIterateRecord(fcb, kBTreeNextRecord, &iterator, &btrec, &reclen)) {
		HFSPlusCatalogFile *file = &rec.hfsPlusFile;
		Boolean islink = false;

		if (rec.recordType != kHFSPlusFileRecord)
			continue;

		if (isdir) {
			/* Assume that this is a directory hard link if 
			 * the atleast one value in finder info corresponds to 
			 * alias, and the alias is not a file inode, and 
			 * either the inode number is greater than 
			 * kHFSFirstUserCatalogNodeID or the flag has 
			 * kHFSHasLinkChainBit set.
			 */
			if (((file->userInfo.fdType == kHFSAliasType) ||
			     (file->userInfo.fdCreator == kHFSAliasCreator) ||
			     (file->userInfo.fdFlags & kIsAlias)) &&
			    (keyp->parentID != gp->filelink_priv_dir_id) &&
			    ((file->hl_linkReference >= kHFSFirstUserCatalogNodeID) || 
			     (file->flags & kHFSHasLinkChainMask))) {
				flags = rec.hfsPlusFile.flags;
				islink = true;
			}
		} else if (file->userInfo.fdType == kHardLinkFileType &&
			   file->userInfo.fdCreator == kHFSPlusCreator) {
			flags = rec.hfsPlusFile.flags;
			islink = true;
		}
		if (islink) {
			struct IndirectLinkInfo *li = NULL;
			struct HardLinkList *tlist = NULL;
			int i;
			int count;

			linkID = file->fileID;
			inodeID = file->bsdInfo.special.iNodeNum;

			/* Now that we are in repair, all hard links should 
			 * have this bit set because we upgrade all pre-Leopard
			 * file hard links to Leopard hard links on any 
			 * file hard link repairs.
			 */
			if ((flags & kHFSHasLinkChainMask) == 0) {
				record_link_badflags(gp, linkID, isdir, flags,
					flags | kHFSHasLinkChainMask);
			}

			/* For directory hard links, check ownerFlags and 
			 * finderInfo because we could have missed creating 
			 * repair orders in verification.  Verification could 
			 * have stopped before we saw this record because it 
			 * stops as soon as it determines that it needs full 
			 * knowledge of hard links on the disk during repair.
			 */
			if (isdir) {
				/* Check if the directory hard link has UF_IMMUTABLE bit set */
				if ((file->bsdInfo.ownerFlags & UF_IMMUTABLE) == 0) {
					record_dirlink_badownerflags(gp, file->fileID, 
						file->bsdInfo.ownerFlags, 
						file->bsdInfo.ownerFlags | UF_IMMUTABLE, true);
				}

				/* Check Finder Info */
				if ((file->userInfo.fdType != kHFSAliasType) ||
				    (file->userInfo.fdCreator != kHFSAliasCreator) ||
				    ((file->userInfo.fdFlags & kIsAlias) == 0)) {
					record_link_badfinderinfo(gp, file->fileID, true);
				}
			}

			/* For directory hard links, hash using inodeID.  For 
			 * file hard links, hash using link reference number 
			 * (which is same as inode ID for file hard links 
			 * created post-Tiger).  For each inodeID, add the 
			 * <prev, id, next> triad.
			 */
			li = hash_search(inodeID, slots, slotsUsed, linkInfo);
			if (li) {
				li->linkCount++;
			} else {
				entries++;
				/* hash_insert() initializes linkCount to 1 */
				hash_insert(inodeID, slots, slotsUsed++, linkInfo);
				li = hash_search(inodeID, slots, slotsUsed, linkInfo);
			}
			if (li == NULL) {
				/*
				 * Either the hash passed in should have the entry, or
				 * the one we just created should (because we just added it);
				 * either way, if it's not here, we've got something weird
				 * going on, so let's just abort now.
				 */
				result = ENOENT;
				goto done;
			}

			count = li->linkCount - 1;
			/* Reallocate memory to store information about file/directory hard links */
			if ((count % 10) == 0) {
				tlist = realloc(li->list, (count + 10) * sizeof(struct HardLinkList));
				if (tlist == NULL) {
					free(li->list);
					li->list = NULL;
					result = ENOMEM;
					goto done;
				} else {
					li->list = tlist;	// May be the same
					for (i = count; i < (count + 10); i++) {
						memset(&li->list[i], 0, sizeof(li->list[i]));
					}
				}
			}

			/* Store information about this hard link */
			if (li->list) {
				li->list[count].fileID = linkID;
				li->list[count].prev = file->hl_prevLinkID;
				li->list[count].next = file->hl_nextLinkID;
			}
		}
	}

	if (result == btNotFound)
		result = 0;	// If we hit the end of the catalog tree, that's okay

	if (result) {
		goto done;
	}

	/*
	 * Next, we iterate through the metadata directory, and check the linked list.
	 */

	ClearMemory(&iterator, sizeof(iterator));
	keyp = (HFSPlusCatalogKey*)&iterator.key;
	BuildCatalogKey(metadirid, NULL, true, (CatalogKey*)keyp);
	btrec.bufferAddress = &rec;
	btrec.itemCount = 1;
	btrec.itemSize = sizeof(rec);

	for (result = BTSearchRecord(fcb, &iterator, kInvalidMRUCacheKey, &btrec, &reclen, &iterator);
		result == 0;
		result = BTIterateRecord(fcb, kBTreeNextRecord, &iterator, &btrec, &reclen)) {
		unsigned char filename[64];
		size_t len;
		struct IndirectLinkInfo *li = NULL;

		if (rec.recordType == kHFSPlusFolderThreadRecord ||
		    rec.recordType == kHFSPlusFileThreadRecord)
			continue;
		if (keyp->parentID != metadirid)
			break;
		if ((isdir && rec.recordType != kHFSPlusFolderRecord) ||
		    (!isdir && rec.recordType != kHFSPlusFileRecord))
			continue;
		(void)utf_encodestr(keyp->nodeName.unicode,
					keyp->nodeName.length * 2,
					filename, &len, sizeof(filename));
		filename[len] = 0;
		if (strstr((char*)filename, prefixName) != (char*)filename)
			continue;

		if (isdir) {
			inodeID = rec.hfsPlusFolder.folderID;
			link_ref_num = 0;
			flags = rec.hfsPlusFolder.flags;
			li = hash_search(inodeID, slots, slotsUsed, linkInfo);
		} else {
            long ref_num;

			inodeID = rec.hfsPlusFile.fileID;
            ref_num = atol((char*)&filename[prefixlen]);
            if (ref_num < 0 || ref_num > UINT32_MAX) {
                result = EINVAL;
                if (fsckGetVerbosity(gp->context) >= kDebugLog) {
                    plog ("\tLink reference num=%ld is invalid for inode=%u result=%d\n", ref_num, inodeID, result);
                }
                break;
            }
			link_ref_num = (UInt32)ref_num;
			flags = rec.hfsPlusFile.flags;
			li = hash_search(link_ref_num, slots, slotsUsed, linkInfo);
		}

		/* file/directory inode should always have kHFSHasLinkChainBit set */
		if ((flags & kHFSHasLinkChainMask) == 0) {
			record_inode_badflags(gp, inodeID, isdir, flags,
				flags | kHFSHasLinkChainMask, true);
		}

		if (li) {
			UInt32 first_link_id = 0;
			uint32_t linkCount = 0;

			result = get_first_link_id(gp, &rec, inodeID, isdir, &first_link_id);
			if (result != 0) {
				if (fsckGetVerbosity(gp->context) >= kDebugLog)
					plog("\tError getting first link ID for inode = %u (result=%d)\n", inodeID, result);
			}

			/* Check and create repairs for doubly linked list */
			result = CheckHardLinkList(gp, inodeID, li->list, li->linkCount, first_link_id);

			linkCount = isdir ? rec.hfsPlusFolder.bsdInfo.special.linkCount : rec.hfsPlusFile.bsdInfo.special.linkCount;
			if (linkCount != li->linkCount) {
				RecordBadLinkCount(gp, inodeID, linkCount, li->linkCount);
			}
		
			li->flags |= LINKINFO_CHECK;
			entries--;
		} else {
			/* Not found in hash, this is orphaned file/directory inode */
			RecordOrphanInode(gp, isdir, inodeID);
		}
	}

	if (result == btNotFound) {
		result = 0;
	}
	if (result) {
		goto done;
	}

	/* Check for orphan hard links */
	if (entries) {
	 	int i, j;
		for (i = 0; i < slots; i++) {
			/* If node is initialized but never checked, record orphan link */
			if ((linkInfo[i].flags & LINKINFO_INIT) && 
			    ((linkInfo[i].flags & LINKINFO_CHECK) == 0)) {
				for (j = 0; j < linkInfo[i].linkCount; j++) {
					RecordOrphanLink(gp, isdir, linkInfo[i].list[j].fileID);
				}
			}
		}
	}

done:
	if (linkInfo) {
		int i;
		for (i = 0; i < slots; i++) {
			if (linkInfo[i].list)
				free(linkInfo[i].list);
		}
		free(linkInfo);
	}

	return result;
}

/*
 * CheckHardLinks
 *
 * Check indirect node link counts against the indirect
 * links that were found. There are 4 possible problems
 * that can occur.
 *  1. orphaned indirect node (i.e. no links found)
 *  2. orphaned indirect link (i.e. indirect node missing)
 *  3. incorrect link count
 *  4. indirect link id was 0 (i.e. link id wasn't preserved)
 */
int
CheckHardLinks(void *cookie)
{
	struct HardLinkInfo *info = (struct HardLinkInfo *)cookie;
	SGlobPtr gp;
	UInt32              folderID;
	SFCB              * fcb;
	CatalogRecord       rec;
	HFSPlusCatalogKey * keyp;
	BTreeIterator       iterator;
	FSBufferDescriptor  btrec;
	UInt16              reclen;
	size_t len;
	size_t prefixlen;
	int result;
	unsigned char filename[64];
	PrimeBuckets *catBucket;

	/* All done if no hard links exist. */
	if (info == NULL)
		return (0);

	gp = info->globals;
	fsckPrint(gp->context, hfsHardLinkCheck);

	folderID = info->privDirID;

	catBucket = calloc(1, sizeof(PrimeBuckets));
	if (catBucket == NULL) {
		if (fsckGetVerbosity(gp->context) >= kDebugLog) {
			plog("CheckHardLinks:  calloc(1, %zu) failed\n", sizeof(PrimeBuckets));
		}
		result = ENOMEM;
		goto exit;
	}
		

	fcb = gp->calculatedCatalogFCB;
	prefixlen = strlen(HFS_INODE_PREFIX);
	ClearMemory(&iterator, sizeof(iterator));
	keyp = (HFSPlusCatalogKey*)&iterator.key;
	btrec.bufferAddress = &rec;
	btrec.itemCount = 1;
	btrec.itemSize = sizeof(rec);
	/*
	 * position iterator at folder thread record
	 * (i.e. one record before first child)
	 */
	ClearMemory(&iterator, sizeof(iterator));
	BuildCatalogKey(folderID, NULL, true, (CatalogKey *)keyp);
	result = BTSearchRecord(fcb, &iterator, kInvalidMRUCacheKey, &btrec,
				&reclen, &iterator);
	if ((result != 0) && (result != btNotFound)) { 
		goto exit;
	}

	/* Visit all the children in private directory. */
	for (;;) {
		result = BTIterateRecord(fcb, kBTreeNextRecord, &iterator,
					&btrec, &reclen);
		if (result || keyp->parentID != folderID)
			break;

		if (rec.recordType != kHFSPlusFileRecord)
			continue;

		(void) utf_encodestr(keyp->nodeName.unicode,
					keyp->nodeName.length * 2,
					filename, &len, sizeof(filename));
		filename[len] = '\0';
		
		/* Report Orphaned nodes only in debug mode */
		if ((strstr((char *)filename, HFS_DELETE_PREFIX) == (char *)filename) &&
			(fsckGetVerbosity(gp->context) == kDebugLog)) {
			RecordOrphanOpenUnlink(gp, folderID, filename);
			continue;		
		}
				
		if (strstr((char *)filename, HFS_INODE_PREFIX) != (char *)filename)
			continue;
		
		result = inode_check(gp, catBucket, (CatalogRecord*)&rec, (CatalogKey*)keyp, false);
		if (result) {
			break;
		}
		filename[0] = '\0';
	}

	if (result == btNotFound) {
		result = 0;
	}

	/*
	 * If we've reached this point, and result is clean,
	 * then we need to compare the two hard link
	 * buckets:  if they don't match, then we have a hard link chain error, and
	 * need to either repair it, or just mark the error.
	 */
	if ((result == 0) && (info->fileBucket != NULL)) {
		result = compare_prime_buckets(catBucket, info->fileBucket);
		if (result) {
			record_link_badchain(gp, false);
			if (fsckGetVerbosity(gp->context) >= kDebugLog) {
				plog("\tfilelink prime buckets do not match\n");
			}
			goto exit;
		}
	}

	/* If hard links created in pre-Leopard OS were detected, they were 
	 * added to the hash for checking link counts later.  Check the 
	 * link counts from the hash.  Note that the hard links created in 
	 * pre-Leopard OS do not have kHFSHasLinkChainBit set in the inode 
	 * and the hard links, and the first/prev/next ID is zero --- and 
	 * hence they were ignored from CRT check and added to hash.
	 */
	if (filelink_entry_count) {
		int i;
		struct filelink_hash *cur;

		/* Since pre-Leopard OS hard links were detected, they 
		 * should be updated to new version.  This is however 
		 * an opportunistic repair and no corruption will be 
		 * reported.  This will be performed only if any other 
		 * file hard link repairs are performed.
		 */
		if (fsckGetVerbosity(gp->context) >= kDebugLog) {
			plog("\tCheckHardLinks: found %u pre-Leopard file inodes.\n", filelink_entry_count);
		}

		for (i = 0; i < FILELINK_HASH_SIZE; i++) {
			cur = filelink_head[i];
			while (cur) {
				if ((cur->found_link_count == 0) || 
				    (cur->calc_link_count == 0) ||
				    (cur->found_link_count != cur->calc_link_count)) {
					record_link_badchain(gp, false);
					goto exit;
				}
				cur = cur->next;
			}
		}
	}

exit:
	if (filelink_entry_count) {
		filelink_hash_destroy();
	}

	if (catBucket)
		free(catBucket);

	return (result);
}

/*
 * GetPrivateDir
 *
 * Get catalog entry for the "HFS+ Private Data" directory.
 * The indirect nodes are stored in this directory.
 */
static int
GetPrivateDir(SGlobPtr gp, CatalogRecord * rec)
{
	HFSPlusCatalogKey * keyp;
	BTreeIterator       iterator;
	FSBufferDescriptor  btrec;
	UInt16              reclen;
	int                 result;
	Boolean 			isHFSPlus;

	isHFSPlus = VolumeObjectIsHFSPlus( );
	if (!isHFSPlus)
		return (-1);

	ClearMemory(&iterator, sizeof(iterator));
	keyp = (HFSPlusCatalogKey*)&iterator.key;

	btrec.bufferAddress = rec;
	btrec.itemCount = 1;
	btrec.itemSize = sizeof(CatalogRecord);

	/* look up record for HFS+ private folder */
	ClearMemory(&iterator, sizeof(iterator));
	CopyMemory(&gMetaDataDirKey, keyp, sizeof(gMetaDataDirKey));
	result = BTSearchRecord(gp->calculatedCatalogFCB, &iterator,
	                        kInvalidMRUCacheKey, &btrec, &reclen, &iterator);

	return (result);
}

/*
 * GetRootDir
 *
 * Get catalog entry for the Root Folder.
 */
static int
GetRootDir(SGlobPtr gp, CatalogRecord * rec)
{
	CatalogKey	key;
	uint16_t	recSize;
	int                 result;
	Boolean 			isHFSPlus;

	isHFSPlus = VolumeObjectIsHFSPlus( );
	if (!isHFSPlus)
		return (-1);

	result = GetCatalogRecordByID(gp, kHFSRootFolderID, isHFSPlus, &key, rec, &recSize);

	return (result);
}

/*
 * RecordOrphanLink
 *
 * Record a repair to delete an orphaned hard links, i.e. hard links 
 * that do not have any corresponding inode.
 */
static int
RecordOrphanLink(SGlobPtr gp, Boolean isdir, UInt32 linkID)
{
	RepairOrderPtr p;
	
	fsckPrint(gp->context, isdir ? E_OrphanDirLink : E_OrphanFileLink, linkID);

	p = AllocMinorRepairOrder(gp, 0);
	if (p == NULL)
		return ENOMEM;

	p->type = isdir ? E_OrphanDirLink : E_OrphanFileLink;
	p->parid = linkID;

	gp->CatStat |= S_LinkErrRepair;

	return 0;
}

/*
 * RecordOrphanInode
 *
 * Record a repair for orphan inode i.e. inodes that do not have 
 * any corresponding hard links.
 */
static int
RecordOrphanInode(SGlobPtr gp, Boolean isdir, UInt32 inodeID)
{
	RepairOrderPtr p;
	
	fsckPrint(gp->context, isdir ? E_OrphanDirInode : E_OrphanFileInode, inodeID);

	p = AllocMinorRepairOrder(gp, 0);
	if (p == NULL)
		return ENOMEM;

	p->type = isdir ? E_OrphanDirInode : E_OrphanFileInode;
	p->parid = inodeID;

	gp->CatStat |= S_LinkErrRepair;

	return 0;
}

/*
 * RecordOrphanOpenUnlink
 *
 * This is only called when debugging is turned on.  Don't
 * record an actual error, just print out a message.
 */
static int
RecordOrphanOpenUnlink(SGlobPtr gp, UInt32 parID, unsigned char* filename)
{
	fsckPrint(gp->context, E_UnlinkedFile, filename);
	
	return (noErr);
}


static int
RecordBadHardLinkChainFirst(SGlobPtr gp, UInt32 fileID, UInt32 is, UInt32 shouldbe)
{
	RepairOrderPtr p;
	char goodstr[16], badstr[16];

	fsckPrint(gp->context, E_InvalidLinkChainFirst, fileID);
	sprintf(goodstr, "%u", shouldbe);
	sprintf(badstr, "%u", is);
	fsckPrint(gp->context, E_BadValue, goodstr, badstr);

	p = AllocMinorRepairOrder(gp, 0);

	if (p == NULL) {
		return (ENOMEM);
	}

	p->type = E_InvalidLinkChainFirst;
	p->incorrect = is;
	p->correct = shouldbe;
	p->hint = 0;
	p->parid = fileID;	// *Not* the parent ID!
	gp->CatStat |= S_LinkErrRepair;

	return (0);
}


static int
RecordBadHardLinkPrev(SGlobPtr gp, UInt32 fileID, UInt32 is, UInt32 shouldbe)
{
	RepairOrderPtr p;
	char goodstr[16], badstr[16];

	fsckPrint(gp->context, E_InvalidLinkChainPrev, fileID);
	sprintf(goodstr, "%u", shouldbe);
	sprintf(badstr, "%u", is);
	fsckPrint(gp->context, E_BadValue, goodstr, badstr);

	p = AllocMinorRepairOrder(gp, 0);
	if (p == NULL)
		return (R_NoMem);

	p->type = E_InvalidLinkChainPrev;
	p->incorrect = is;
	p->correct = shouldbe;
	p->hint = 0;
	p->parid = fileID;	// *Not* the parent ID
	gp->CatStat |= S_LinkCount;
	return (0);
}

static int
RecordBadHardLinkNext(SGlobPtr gp, UInt32 fileID, UInt32 is, UInt32 shouldbe)
{
	RepairOrderPtr p;
	char goodstr[16], badstr[16];

	fsckPrint(gp->context, E_InvalidLinkChainNext, fileID);

	sprintf(goodstr, "%u", shouldbe);
	sprintf(badstr, "%u", is);
	fsckPrint(gp->context, E_BadValue, goodstr, badstr);

	p = AllocMinorRepairOrder(gp, 0);
	if (p == NULL)
		return (R_NoMem);

	p->type = E_InvalidLinkChainNext;
	p->incorrect = is;
	p->correct = shouldbe;
	p->hint = 0;
	p->parid = fileID;	// *Not* the parent ID
	gp->CatStat |= S_LinkCount;
	return (0);
}

static int 
RecordBadLinkCount(SGlobPtr gp, UInt32 inodeID, UInt32 is, UInt32 shouldbe) 
{
	RepairOrderPtr p;
	char goodstr[16], badstr[16];
	fsckPrint(gp->context, E_InvalidLinkCount, inodeID);

	sprintf(goodstr, "%u", shouldbe);
	sprintf(badstr, "%u", is);
	fsckPrint(gp->context, E_BadValue, goodstr, badstr);

	p = AllocMinorRepairOrder(gp, 0);
	if (p == NULL)
		return (R_NoMem);

	p->type = E_InvalidLinkCount;
	p->incorrect = is;
	p->correct = shouldbe;
	p->hint = 0;
	p->parid = inodeID;	// *Not* the parent ID
	return (0);
}


static void
hash_insert(UInt32 linkID, int totalSlots, int slotsUsed, struct IndirectLinkInfo *linkInfo)
{
	int i, last;
	
	i = linkID & (totalSlots - 1);
	
	last = (i + (totalSlots-1)) % totalSlots;
	while ((i != last) && 
	       (linkInfo[i].flags & LINKINFO_INIT) &&
	       (linkInfo[i].linkID != linkID)) {
		i = (i + 1) % totalSlots;
	}

	if ((linkInfo[i].flags & LINKINFO_INIT) == 0) {
		if (linkInfo[i].list) {
			plog ("hash: overwriting data! (old:%u, new:%u)\n", linkInfo[i].linkID, linkID);
			exit(13);
		}
		linkInfo[i].flags |= LINKINFO_INIT;
		linkInfo[i].linkID = linkID;
		linkInfo[i].linkCount = 1;
	} else if (linkInfo[i].linkID == linkID) {
		plog("hash: duplicate insert! (%d)\n", linkID);
		exit(13);
	} else {
		plog("hash table full (%d entries) \n", slotsUsed);
		exit(14);
	}
}


static struct IndirectLinkInfo *
hash_search(UInt32 linkID, int totalSlots, int slotsUsed, struct IndirectLinkInfo *linkInfo)
{
	int i, last;
	int p = 1;

	
	i = linkID & (totalSlots - 1);

	last = (i + (slotsUsed-1)) % totalSlots;
	while ((i != last) && 
	       (linkInfo[i].flags & LINKINFO_INIT) &&
	       (linkInfo[i].linkID != linkID)) {
		i = (i + 1) % totalSlots;
		++p;
	}
	
	if ((linkInfo[i].flags & LINKINFO_INIT) && 
	    (linkInfo[i].linkID == linkID)) {
		return (&linkInfo[i]);
	} else {
		return (NULL);
	}
}
