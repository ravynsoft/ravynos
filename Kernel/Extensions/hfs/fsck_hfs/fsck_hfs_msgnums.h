/*
 * Copyright (c) 2008-2009 Apple Inc. All rights reserved.
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

#ifndef __FSCK_HFS_MSGNUMS_H
#define __FSCK_HFS_MSGNUMS_H

/*
 * HFS-specific status messages.  These indicate the current 
 * state of fsck_hfs run.
 */
enum {
        hfsUnknown              = 200,

        hfsExtBTCheck           = 201,  /* Checking Extents Overflow file */
        hfsCatBTCheck           = 202,  /* Checking Catalog file */
        hfsCatHierCheck         = 203,  /* Checking Catalog hierarchy */
        hfsExtAttrBTCheck       = 204,  /* Checking Extended Attributes file */
        hfsVolBitmapCheck       = 205,  /* Checking volume bitmap */
        hfsVolInfoCheck         = 206,  /* Checking volume information */
        hfsHardLinkCheck        = 207,  /* Checking multi-linked files */
        hfsRebuildExtentBTree   = 208,  /* Rebuilding Extents Overflow B-tree */
        hfsRebuildCatalogBTree  = 209,  /* Rebuilding Catalog B-tree */
        hfsRebuildAttrBTree     = 210,  /* Rebuilding Extended Attributes B-tree */

        hfsCaseSensitive        = 211,  /* Detected a case-sensitive catalog */
        hfsMultiLinkDirCheck    = 212,  /* Checking multi-linked directories */
        hfsJournalVolCheck      = 213,  /* Checking Journaled HFS Plus volume */
        hfsLiveVerifyCheck      = 214,  /* Performing live verification */
        hfsVerifyVolWithWrite   = 215,  /* Verifying volume when it is mounted with write access */  
        hfsCheckHFS             = 216,  /* Checking HFS volume */
        hfsCheckNoJnl           = 217,  /* Checking Non-journaled HFS Plus volume */
};

/*
 * Scavenger errors.  They are mostly corruptions detected 
 * during scavenging process.
 * If negative, they are unrecoverable (scavenging terminates).
 * If positive, they are recoverable (scavenging continues).
 */     
enum {
        E_FirstError            =  500,

        E_PEOF                  =  500, /* Invalid PEOF */
        E_LEOF                  =  501, /* Invalid LEOF */
        E_DirVal                =  502, /* Invalid directory valence */
        E_CName                 =  503, /* Invalid CName */
        E_NHeight               =  504, /* Invalid node height */
        E_NoFile                =  505, /* Missing file record for file thread */
        E_ABlkSz                = -506, /* Invalid allocation block size */
        E_NABlks                = -507, /* Invalid number of allocation blocks */
        E_VBMSt                 = -508, /* Invalid VBM start block */
        E_ABlkSt                = -509, /* Invalid allocation block start */

        E_ExtEnt                = -510, /* Invalid extent entry */
        E_OvlExt                =  511, /* Overlapped extent allocation (id, path) */
        E_LenBTH                = -512, /* Invalid BTH length */
        E_ShortBTM              = -513, /* BT map too short to repair */
        E_BTRoot                = -514, /* Invalid root node number */
        E_NType                 = -515, /* Invalid node type */
        E_NRecs                 = -516, /* Invalid record count */
        E_IKey                  = -517, /* Invalid index key */
        E_IndxLk                = -518, /* Invalid index link */
        E_SibLk                 = -519, /* Invalid sibling link */

        E_BadNode               = -520, /* Invalid node structure */
        E_OvlNode               = -521, /* overlapped node allocation */
        E_MapLk                 = -522, /* Invalid map node linkage */
        E_KeyLen                = -523, /* Invalid key length */
        E_KeyOrd                = -524, /* Keys out of order */
        E_BadMapN               = -525, /* Invalid map node */
        E_BadHdrN               = -526, /* Invalid header node */
        E_BTDepth               = -527, /* exceeded maximum BTree depth */
        E_CatRec                = -528, /* Invalid catalog record type */
        E_LenDir                = -529, /* Invalid directory record length */

        E_LenThd                = -530, /* Invalid thread record length */
        E_LenFil                = -531, /* Invalid file record length */
        E_NoRtThd               = -532, /* Missing thread record for root directory */
        E_NoThd                 = -533, /* Missing thread record */
        E_NoDir                 =  534, /* Missing directory record */
        E_ThdKey                = -535, /* Invalid key for thread record */
        E_ThdCN                 = -536, /* Invalid  parent CName in thread record */
        E_LenCDR                = -537, /* Invalid catalog record length */
        E_DirLoop               = -538, /* Loop in directory hierarchy */
        E_RtDirCnt              =  539, /* Invalid root directory count */

        E_RtFilCnt              =  540, /* Invalid root file count */
        E_DirCnt                =  541, /* Invalid volume directory count */
        E_FilCnt                =  542, /* Invalid volume file count */
        E_CatPEOF               = -543, /* Invalid catalog PEOF */
        E_ExtPEOF               = -544, /* Invalid extent file PEOF */
        E_CatDepth              =  545, /* Nesting of folders has exceeded the recommended limit of 100 */
        E_NoFThdFlg             = -546, /* File thread flag not set in file record */
        E_CatalogFlagsNotZero   =  547, /* Reserved fields in the catalog record have incorrect data */
        E_BadFileName           = -548, /* Invalid file/folder name problem */
        E_InvalidClumpSize      =  549, /* Invalid file clump size */

        E_InvalidBTreeHeader    =  550, /* Invalid B-tree header */
        E_LockedDirName         =  551, /* Inappropriate locked folder name */
        E_EntryNotFound         = -552, /* volume catalog entry not found */
        E_FreeBlocks            =  553,	/* Invalid volume free block count */
        E_MDBDamaged            =  554, /* Master Directory Block needs minor repair */
        E_VolumeHeaderDamaged   =  555, /* Volume Header needs minor repair */
        E_VBMDamaged            =  556, /* Volume Bit Map needs repair */
        E_InvalidNodeSize       = -557, /* Invalid B-tree node size */
        E_LeafCnt               =  558,	/* Invalid leaf record count */
        E_BadValue              =  559,	/* (It should be %s instead of %s) */

        E_InvalidID             =  560,	/* Invalid file or directory ID found */
        E_VolumeHeaderTooNew    =  561,	/* I can't understand this version of HFS Plus */
        E_DiskFull              = -562,	/* Disk full error */
        E_InternalFileOverlap   = -563, /* Internal files overlap (file %d) */
        E_InvalidVolumeHeader   = -564,	/* Invalid volume header */
        E_InvalidMDBdrAlBlSt    =  565,	/* HFS wrapper volume needs repair */
        E_InvalidWrapperExtents =  566,	/* Wrapper catalog file location needs repair */
        E_InvalidLinkCount      =  567, /* Indirect node %s needs link count adjustment */
        E_UnlinkedFile          =  568, /* Unlinked file needs to be deleted */
        E_InvalidPermissions    =  569,	/* Invalid BSD file type */

        E_InvalidUID_Unused     =  570, /* Invalid UID/GID in BSD info - Unused (4538396) */
        E_IllegalName           =  571,	/* Illegal name */
        E_IncorrectNumThdRcd    =  572,	/* Incorrect number of thread records */
        E_SymlinkCreate         =  573,	/* Cannot create links to all corrupt files */
        E_BadJournal            =  574, /* Invalid content in Journal */
        E_IncorrectAttrCount    =  575, /* Incorrect number of attributes in attr btree when compared with attr bits set in catalog btree */
        E_IncorrectSecurityCount=  576, /* Incorrect number of security attributes in attr btree when compared with security bits set in catalog btree */
        E_PEOAttr               =  577, /* Incorrect physical end of extended attribute data */
        E_LEOAttr               =  578, /* Incorrect logical end of extended attribute data */
        E_AttrRec               =  579, /* Invalid attribute record (overflow extent without original extent, unknown type) */

        E_FldCount              =  580, /* Incorrect folder count in a directory */
        E_HsFldCount            =  581, /* HasFolderCount flag needs to be set */
        E_BadPermPrivDir        =  582, /* Incorrect permissions for private directory for directory hard links */
        E_DirInodeBadFlags      =  583, /* Incorrect flags for directory inode */
        E_DirInodeBadParent     = -584, /* Invalid parent for directory inode */
        E_DirInodeBadName       = -585, /* Invalid name for directory inode */
        E_DirHardLinkChain      =  586, /* Incorrect number of directory hard link count */
        E_DirHardLinkOwnerFlags =  587, /* Incorrect owner flags for directory hard link */
        E_DirHardLinkFinderInfo =  588, /* Invalid finder info for directory hard link */
        E_DirLinkAncestorFlags  =  589, /* Invalid flags for directory hard link parent ancestor */
        
        E_BadParentHierarchy    = -590, /* Bad parent hierarchy, could not lookup parent directory record */
        E_DirHardLinkNesting    = -591, /* Maximum nesting of folders and directory hard links reached */
        E_MissingPrivDir        = -592, /* Missing private directory for directory hard links */
        E_InvalidLinkChainPrev  =  593, /* Previous ID in a hard lnk chain is incorrect */
        E_InvalidLinkChainNext  =  594, /* Next ID in a hard link chain is incorrect */
        E_FileInodeBadFlags     =  595, /* Incorrecgt flags for file inode */
        E_FileInodeBadParent    = -596, /* Invalid parent for file inode */
        E_FileInodeBadName      = -597, /* Invalid name for file inode */
        E_FileHardLinkChain     =  598, /* Incorrect number of file hard link count */
        E_FileHardLinkFinderInfo=  599, /* Invalid finder info for file hard link */

        E_InvalidLinkChainFirst =  600, /* Invalid first link in hard link chain */
        E_FileLinkBadFlags      =  601, /* Incorrect flags for file hard link */
        E_DirLinkBadFlags       =  602, /* Incorrect flags for directory hard link */
        E_OrphanFileLink        =  603, /* Orphan file hard link */
        E_OrphanDirLink         =  604, /* Orphan directory hard link */
        E_OrphanFileInode       =  605, /* Orphan file inode, no file hard links pointing to this inode */
        E_OrphanDirInode        =  606, /* Orphan directory inode, no directory hard links pointing to this inode */
        E_OvlExtID              =  607, /* Overlapped extent allocation (id) */
        E_UnusedNodeNotZeroed	=  608, /* An unused B-tree node is not full of zeroes */
	E_VBMDamagedOverAlloc	=  609,	/* Volume bitmap has has orphaned block allocation */

	E_BadHardLinkDate	=  610, /* Bad hard link creation date */
	E_DirtyJournal		=  611,	/* Journal need to be replayed but volume is read-only */
	E_LinkChainNonLink	=  612, /* File record has hard link chain flag */
	E_LinkHasData		= -613, /* Hard link record has data extents */
	E_FileLinkCountError	=  614, /* File has incorrect link count */
	E_BTreeSplitNode	=  615, /* B-tree node is split across extents */
	E_BadSymLink		=  616, /* Bad information for symlink */
	E_BadSymLinkLength	=  617,	/* Symlink has bad length */
	E_BadSymLinkName	=  618, /* Bad symbolic link name */
        E_LastError             =  618
};

#endif
