/*
 * Copyright (c) 2008-2011 Apple Inc. All rights reserved.
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
#include "fsck_messages.h"
#include "fsck_hfs_msgnums.h"

/*
 * HFS-specific status messages -- just indicating what phase fsck_hfs is in.
 * The structure is explained in fsck_strings.c
 */
fsck_message_t
hfs_messages[] = {
    /* Message Number           Message                                                     Type            Verbosity   Arguments */
    /* 201 - 210 */
    { hfsExtBTCheck,            "Checking extents overflow file.",                          fsckMsgVerify,  fsckLevel0,   0, },
    { hfsCatBTCheck,            "Checking catalog file.",                                   fsckMsgVerify,  fsckLevel0,   0, },
    { hfsCatHierCheck,          "Checking catalog hierarchy.",                              fsckMsgVerify,  fsckLevel0,   0, },
    { hfsExtAttrBTCheck,        "Checking extended attributes file.",                       fsckMsgVerify,  fsckLevel0,   0, },
    { hfsVolBitmapCheck,        "Checking volume bitmap.",                                  fsckMsgVerify,  fsckLevel0,   0, },
    { hfsVolInfoCheck,          "Checking volume information.",                             fsckMsgVerify,  fsckLevel0,   0, },
    { hfsHardLinkCheck,         "Checking multi-linked files.",                             fsckMsgVerify,  fsckLevel0,   0, },
    { hfsRebuildExtentBTree,    "Rebuilding extents overflow B-tree.",                      fsckMsgVerify,  fsckLevel0,   0, },
    { hfsRebuildCatalogBTree,   "Rebuilding catalog B-tree.",                               fsckMsgVerify,  fsckLevel0,   0, },
    { hfsRebuildAttrBTree,      "Rebuilding extended attributes B-tree.",                   fsckMsgVerify,  fsckLevel0,   0, },
    
    /* 211 - 217 */
    { hfsCaseSensitive,         "Detected a case-sensitive volume.",                        fsckMsgVerify,  fsckLevel0,   0, },
    { hfsMultiLinkDirCheck,     "Checking multi-linked directories.",                       fsckMsgVerify,  fsckLevel0,   0, },
    { hfsJournalVolCheck,       "Checking Journaled HFS Plus volume.",                      fsckMsgVerify,  fsckLevel0,   0, },
    { hfsLiveVerifyCheck,       "Performing live verification.",                            fsckMsgVerify,  fsckLevel0,   0, },
    { hfsVerifyVolWithWrite,    "Verifying volume when it is mounted with write access.",   fsckMsgVerify,  fsckLevel0,   0, },
    { hfsCheckHFS,              "Checking HFS volume.",                                     fsckMsgVerify,  fsckLevel0,   0, },
    { hfsCheckNoJnl,            "Checking non-journaled HFS Plus Volume.",                  fsckMsgVerify,  fsckLevel0,   0, },

    /* End of the array */
    { 0, },
};

/*
 * HFS-specific error messages.  Most are repairable; some are not, but there's no indication of
 * which is which here (see fsck_hfs_msgnums.h; negative values are non-repairable).
 * Messages need not be in any particular order, as fsckAddMessages will sort everything.
 */
fsck_message_t
hfs_errors[] = {
    /* Message Number           Message                                                         Type            Verbosity   Arguments */
    /* 500 - 509 */
    { E_PEOF,                   "Incorrect block count for file %s",                            fsckMsgError,   fsckLevel1,   1, (const int[]){ fsckTypeFile } },
    { E_LEOF,                   "Incorrect size for file %s",                                   fsckMsgError,   fsckLevel1,   1, (const int[]){ fsckTypeFile } },
    { E_DirVal,                 "Invalid directory item count",                                 fsckMsgError,   fsckLevel1,   0, } ,
    { E_CName,                  "Invalid length for file name",                                 fsckMsgError,   fsckLevel1,   0, } ,
    { E_NHeight,                "Invalid node height",                                          fsckMsgError,   fsckLevel1,   0, } ,
    { E_NoFile,                 "Missing file record for file thread",                          fsckMsgError,   fsckLevel1,   0, } ,
    { E_ABlkSz,                 "Invalid allocation block size",                                fsckMsgError,   fsckLevel1,   0, } ,
    { E_NABlks,                 "Invalid number of allocation blocks",                          fsckMsgError,   fsckLevel1,   0, },
    { E_VBMSt,                  "Invalid VBM start block",                                      fsckMsgError,   fsckLevel1,   0, },
    { E_ABlkSt,                 "Invalid allocation block start",                               fsckMsgError,   fsckLevel1,   0, },

    /* 510 - 519 */
    { E_ExtEnt,                 "Invalid extent entry",                                         fsckMsgError,   fsckLevel1,   0, },
    { E_OvlExt,                 "Overlapped extent allocation (id = %u, %s)",                   fsckMsgError,   fsckLevel1,   2, (const int[]){ fsckTypeInt, fsckTypePath } },
    { E_LenBTH,                 "Invalid BTH length",                                           fsckMsgError,   fsckLevel1,   0, } ,
    { E_ShortBTM,               "BT map too short during repair",                               fsckMsgError,   fsckLevel1,   0, } ,
    { E_BTRoot,                 "Invalid root node number",                                     fsckMsgError,   fsckLevel1,   0, },
    { E_NType,                  "Invalid node type",                                            fsckMsgError,   fsckLevel1,   0, },
    { E_NRecs,                  "Invalid record count",                                         fsckMsgError,   fsckLevel1,   0, },
    { E_IKey,                   "Invalid index key",                                            fsckMsgError,   fsckLevel1,   0, },
    { E_IndxLk,                 "Invalid index link",                                           fsckMsgError,   fsckLevel1,   0, },
    { E_SibLk,                  "Invalid sibling link",                                         fsckMsgError,   fsckLevel1,   0, },

    /* 520 - 529 */
    { E_BadNode,                "Invalid node structure",                                       fsckMsgError,   fsckLevel1,   0, },
    { E_OvlNode,                "Overlapped node allocation",                                   fsckMsgError,   fsckLevel1,   0, },
    { E_MapLk,                  "Invalid map node linkage",                                     fsckMsgError,   fsckLevel1,   0, },
    { E_KeyLen,                 "Invalid key length",                                           fsckMsgError,   fsckLevel1,   0, },
    { E_KeyOrd,                 "Keys out of order",                                            fsckMsgError,   fsckLevel1,   0, },
    { E_BadMapN,                "Invalid map node",                                             fsckMsgError,   fsckLevel1,   0, },
    { E_BadHdrN,                "Invalid header node",                                          fsckMsgError,   fsckLevel1,   0, },
    { E_BTDepth,                "Exceeded maximum B-tree depth",                                fsckMsgError,   fsckLevel1,   0, },
    { E_CatRec,                 "Invalid catalog record type",                                  fsckMsgError,   fsckLevel1,   0, },
    { E_LenDir,                 "Invalid directory record length",                              fsckMsgError,   fsckLevel1,   0, },

    /* 530 - 539 */
    { E_LenThd,                 "Invalid thread record length",                                 fsckMsgError,   fsckLevel1,   0, },
    { E_LenFil,                 "Invalid file record length",                                   fsckMsgError,   fsckLevel1,   0, },
    { E_NoRtThd,                "Missing thread record for root dir",                           fsckMsgError,   fsckLevel1,   0, },
    { E_NoThd,                  "Missing thread record (id = %u)",                              fsckMsgError,   fsckLevel1,   1, (const int[]){ fsckTypeInt } },
    { E_NoDir,                  "Missing directory record (id = %u)",                           fsckMsgError,   fsckLevel1,   1, (const int[]){ fsckTypeInt } },
    { E_ThdKey,                 "Invalid key for thread record",                                fsckMsgError,   fsckLevel1,   0, },
    { E_ThdCN,                  "Invalid parent CName in thread record",                        fsckMsgError,   fsckLevel1,   0, },
    { E_LenCDR,                 "Invalid catalog record length",                                fsckMsgError,   fsckLevel1,   0, },
    { E_DirLoop,                "Loop in directory hierarchy",                                  fsckMsgError,   fsckLevel1,   0, },
    { E_RtDirCnt,               "Invalid root directory count",                                 fsckMsgError,   fsckLevel1,   0, },

    /* 540 - 549 */
    { E_RtFilCnt,               "Invalid root file count",                                      fsckMsgError,   fsckLevel1,   0, },
    { E_DirCnt,                 "Invalid volume directory count",                               fsckMsgError,   fsckLevel1,   0, },
    { E_FilCnt,                 "Invalid volume file count",                                    fsckMsgError,   fsckLevel1,   0, },
    { E_CatPEOF,                "Invalid catalog PEOF",                                         fsckMsgError,   fsckLevel1,   0, },
    { E_ExtPEOF,                "Invalid extent file PEOF",                                     fsckMsgError,   fsckLevel1,   0, },
    { E_CatDepth,               "Nesting of folders has exceeded %d folders",                   fsckMsgError,   fsckLevel1,   1, (const int[]){ fsckTypeInt } },
    { E_NoFThdFlg,              "File thread flag not set in file record",                      fsckMsgError,   fsckLevel1,   0, },
    { E_CatalogFlagsNotZero,    "Reserved fields in the catalog record have incorrect data",    fsckMsgError,   fsckLevel1,   0, }, 
    { E_BadFileName,            "Invalid file name",                                            fsckMsgError,   fsckLevel1,   0, },
    { E_InvalidClumpSize,       "Invalid file clump size",                                      fsckMsgError,   fsckLevel1,   0, },

    /* 550 - 559 */
    { E_InvalidBTreeHeader,     "Invalid B-tree header",                                        fsckMsgError,   fsckLevel1,   0, },
    { E_LockedDirName,          "Directory name locked",                                        fsckMsgError,   fsckLevel1,   0, },
    { E_EntryNotFound,          "Catalog file entry not found for extent",                      fsckMsgError,   fsckLevel1,   0, },
    { E_FreeBlocks,             "Invalid volume free block count",                              fsckMsgError,   fsckLevel1,   0, },
    { E_MDBDamaged,             "Master Directory Block needs minor repair",                    fsckMsgError,   fsckLevel1,   0, },
    { E_VolumeHeaderDamaged,    "Volume header needs minor repair",                             fsckMsgError,   fsckLevel1,   0, },
    { E_VBMDamaged,             "Volume bitmap needs repair for under-allocation",              fsckMsgError,   fsckLevel0,   0, },
    { E_InvalidNodeSize,        "Invalid B-tree node size",                                     fsckMsgError,   fsckLevel1,   0, },
    { E_LeafCnt,                "Invalid leaf record count",                                    fsckMsgError,   fsckLevel1,   0, },
    { E_BadValue,               "(It should be %s instead of %s)",                              fsckMsgDamageInfo,fsckLevel1, 2, (const int[]){ fsckTypeString, fsckTypeString } },

    /* 560 - 569 */
    { E_InvalidID,              "Invalid file or directory ID found",                           fsckMsgError,   fsckLevel1,   0, },
    { E_VolumeHeaderTooNew,     "I can't understand this version of HFS Plus",                  fsckMsgError,   fsckLevel1,   0, },
    { E_DiskFull,               "Disk full error",                                              fsckMsgError,   fsckLevel1,   0, },
    { E_InternalFileOverlap,    "Internal files overlap (file %u)",                             fsckMsgError,   fsckLevel1,   1, (const int[]){ fsckTypeInt } },
    { E_InvalidVolumeHeader,    "Invalid volume header",                                        fsckMsgError,   fsckLevel1,   0, },
    { E_InvalidMDBdrAlBlSt,     "HFS wrapper volume needs repair",                              fsckMsgError,   fsckLevel1,   0, },
    { E_InvalidWrapperExtents,  "Wrapper catalog file location needs repair",                   fsckMsgError,   fsckLevel1,   0, },
    { E_InvalidLinkCount,       "Indirect node %d needs link count adjustment",                 fsckMsgError,   fsckLevel1,   1, (const int[]){ fsckTypeInt } },
    { E_UnlinkedFile,           "Orphaned open unlinked file %s",                               fsckMsgError,   fsckLevel1,   1, (const int[]){ fsckTypeFile } },
    { E_InvalidPermissions,     "Invalid BSD file type",                                        fsckMsgError,   fsckLevel1,   0, },

    /* 570 - 579 */
    { E_InvalidUID_Unused,      "Invalid BSD User ID",                                          fsckMsgError,   fsckLevel1,   0, },
    { E_IllegalName,            "Illegal name",                                                 fsckMsgError,   fsckLevel1,   0, },
    { E_IncorrectNumThdRcd,     "Incorrect number of thread records",                           fsckMsgError,   fsckLevel1,   0, },
    { E_SymlinkCreate,          "Cannot create links to all corrupt files",                     fsckMsgError,   fsckLevel1,   0, },
    { E_BadJournal,             "Invalid content in journal",                                   fsckMsgError,   fsckLevel1,   0, },
    { E_IncorrectAttrCount,     "Incorrect number of extended attributes",                      fsckMsgError,   fsckLevel1,   0, },
    { E_IncorrectSecurityCount, "Incorrect number of Access Control Lists",                     fsckMsgError,   fsckLevel1,   0, },
    { E_PEOAttr,                "Incorrect block count for attribute %s of file %s",            fsckMsgError,   fsckLevel1,   2, (const int[]){ fsckTypeString, fsckTypeFile } },
    { E_LEOAttr,                "Incorrect size for attribute %s of file %s",                   fsckMsgError,   fsckLevel1,   2, (const int[]){ fsckTypeString, fsckTypeFile } },
    { E_AttrRec,                "Invalid attribute record",                                     fsckMsgError,   fsckLevel1,   0, },

    /* 580 - 589 */
    { E_FldCount,               "Incorrect folder count in a directory (id = %u)",              fsckMsgError,   fsckLevel1,   1, (const int[]){ fsckTypeInt } },
    { E_HsFldCount,             "HasFolderCount flag needs to be set (id = %u)",                fsckMsgError,   fsckLevel1,   1, (const int[]){ fsckTypeInt } },
    { E_BadPermPrivDir,         "Incorrect permissions for private directory",                  fsckMsgError,   fsckLevel1,   0, },
    { E_DirInodeBadFlags,       "Incorrect flags for directory inode (id = %u)",                fsckMsgError,   fsckLevel1,   1, (const int[]){ fsckTypeInt, } },
    { E_DirInodeBadParent,      "Invalid parent for directory inode (id = %u)",                 fsckMsgError,   fsckLevel1,   1, (const int[]){ fsckTypeInt, } },
    { E_DirInodeBadName,        "Invalid name for directory inode (id = %u)",                   fsckMsgError,   fsckLevel1,   1, (const int[]){ fsckTypeInt, } },
    { E_DirHardLinkChain,       "Incorrect number of directory hard links",                     fsckMsgError,   fsckLevel1,   0, },
    { E_DirHardLinkOwnerFlags,  "Incorrect owner flags for directory hard link (id = %u)",      fsckMsgError,   fsckLevel1,   1, (const int[]){ fsckTypeInt } },
    { E_DirHardLinkFinderInfo,  "Invalid Finder info for directory hard link (id = %u)",        fsckMsgError,   fsckLevel1,   1, (const int[]){ fsckTypeInt } },
    { E_DirLinkAncestorFlags,   "Incorrect flags for directory hard link ancestor (id = %u)",   fsckMsgError,   fsckLevel1,   1, (const int[]){ fsckTypeInt } },

    /* 590 - 599 */
    { E_BadParentHierarchy,     "Bad parent directory hierarchy (id = %u)",                     fsckMsgError,   fsckLevel1,   1, (const int[]){ fsckTypeInt } },
    { E_DirHardLinkNesting,     "Maximum nesting of folders and directory hard links reached",  fsckMsgError,   fsckLevel1,   0, },
    { E_MissingPrivDir,         "Missing private directory for directory hard links",           fsckMsgError,   fsckLevel1,   0, },
    { E_InvalidLinkChainPrev,   "Previous ID in a hard link chain is incorrect (id = %u)",      fsckMsgError,   fsckLevel1,   1, (const int[]){ fsckTypeInt, } },
    { E_InvalidLinkChainNext,   "Next ID in a hard link chain is incorrect (id = %u)",          fsckMsgError,   fsckLevel1,   1, (const int[]){ fsckTypeInt, } },
    { E_FileInodeBadFlags,      "Incorrect flags for file inode (id = %u)",                     fsckMsgError,   fsckLevel1,   1, (const int[]){ fsckTypeInt, } },
    { E_FileInodeBadParent,     "Invalid parent for file inode (id = %u)",                      fsckMsgError,   fsckLevel1,   1, (const int[]){ fsckTypeInt, } },
    { E_FileInodeBadName,       "Invalid name for file inode (id = %u)",                        fsckMsgError,   fsckLevel1,   1, (const int[]){ fsckTypeInt, } },
    { E_FileHardLinkChain,      "Incorrect number of file hard links",                          fsckMsgError,   fsckLevel1,   0, },
    { E_FileHardLinkFinderInfo, "Invalid Finder info for file hard link (id = %u)",             fsckMsgError,   fsckLevel1,   1, (const int[]){ fsckTypeInt, } },

    /* 600 - 609 */
    { E_InvalidLinkChainFirst,  "Invalid first link in hard link chain (id = %u)",              fsckMsgError,   fsckLevel1,   1, (const int[]){ fsckTypeInt, } },
    { E_FileLinkBadFlags,       "Incorrect flags for file hard link (id = %u)",                 fsckMsgError,   fsckLevel1,   1, (const int[]){ fsckTypeInt, } },
    { E_DirLinkBadFlags,        "Incorrect flags for directory hard link (id = %u)",            fsckMsgError,   fsckLevel1,   1, (const int[]){ fsckTypeInt, } },
    { E_OrphanFileLink,         "Orphaned file hard link (id = %u)",                            fsckMsgError,   fsckLevel1,   1, (const int[]){ fsckTypeInt, } },
    { E_OrphanDirLink,          "Orphaned directory hard link (id = %u)",                       fsckMsgError,   fsckLevel1,   1, (const int[]){ fsckTypeInt, } },
    { E_OrphanFileInode,        "Orphaned file inode (id = %u)",                                fsckMsgError,   fsckLevel1,   1, (const int[]){ fsckTypeInt, } },
    { E_OrphanDirInode,         "Orphaned directory inode (id = %u)",                           fsckMsgError,   fsckLevel1,   1, (const int[]){ fsckTypeInt, } },
    { E_OvlExtID,               "Overlapped extent allocation (id = %d)",                       fsckMsgError,   fsckLevel1,   1, (const int[]){ fsckTypeInt, } },
    { E_UnusedNodeNotZeroed,    "Unused node is not erased (node = %u)",                        fsckMsgError,   fsckLevel1,   1, (const int[]){ fsckTypeInt, } },
    { E_VBMDamagedOverAlloc,    "Volume bitmap needs minor repair for orphaned blocks",         fsckMsgError,   fsckLevel1,   0, },

    /* 610 - 619 */
    { E_BadHardLinkDate,        "Bad hard link creation date",                                   fsckMsgError, fsckLevel1,    0, },
    { E_DirtyJournal,           "Journal needs to be replayed but volume is read-only",          fsckMsgError, fsckLevel1,    0, },
    { E_LinkChainNonLink,       "File record has hard link chain flag (id = %u)",                fsckMsgError, fsckLevel1,    1, (const int[]){ fsckTypeInt, } },
    { E_LinkHasData,            "Hard link record has data extents (id = %u)",                   fsckMsgError, fsckLevel1,    1, (const int[]){ fsckTypeInt, } },
    { E_FileLinkCountError,     "File has incorrect number of links (id = %u)",                  fsckMsgError, fsckLevel1,    1, (const int[]){ fsckTypeInt, } },
    { E_BTreeSplitNode,         "B-tree node is split across extents (file id %u)",              fsckMsgError, fsckLevel1,    1, (const int[]){ fsckTypeInt, } },
    { E_BadSymLink,             "Bad information for symbolic link (file id %u)",                fsckMsgError, fsckLevel1,    1, (const int[]){ fsckTypeInt, } },
    { E_BadSymLinkLength,       "Symbolic link (file id %u) has bad length (is %u, should be %u)",
                                                                                                 fsckMsgError, fsckLevel1,    3, (const int[]){ fsckTypeInt, fsckTypeInt, fsckTypeInt} },
    { E_BadSymLinkName,         "Bad symbolic link is `%s'",                                     fsckMsgError, fsckLevel1,    1, (const int[]){ fsckTypePath, } },

    /* And all-zeroes to indicate the end */
    { 0, },
};

