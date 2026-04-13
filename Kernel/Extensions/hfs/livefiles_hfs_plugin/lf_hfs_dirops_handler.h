/*  Copyright © 2017-2018 Apple Inc. All rights reserved.
 *
 *  lf_hfs_dirops_handler.h
 *  livefiles_hfs
 *
 *  Created by Yakov Ben Zaken on 31/12/2017.
*/

#ifndef lf_hfs_dirpos_handler_h
#define lf_hfs_dirpos_handler_h

#include "lf_hfs_common.h"
#include "lf_hfs_catalog.h"

#define MAX_UTF8_NAME_LENGTH (NAME_MAX*3+1)

int LFHFS_MkDir         ( UVFSFileNode psDirNode, const char *pcName, const UVFSFileAttributes *psFileAttr, UVFSFileNode *ppsOutNode );
int LFHFS_RmDir         ( UVFSFileNode psDirNode, const char *pcUTF8Name );
int LFHFS_Remove        ( UVFSFileNode psDirNode, const char *pcUTF8Name, UVFSFileNode victimNode);
int LFHFS_Lookup        ( UVFSFileNode psDirNode, const char *pcUTF8Name, UVFSFileNode *ppsOutNode );
int LFHFS_ReadDir       ( UVFSFileNode psDirNode, void* pvBuf, size_t iBufLen, uint64_t uCookie, size_t *iReadBytes, uint64_t *puVerifier );
int LFHFS_ReadDirAttr   ( UVFSFileNode psDirNode, void *pvBuf, size_t iBufLen, uint64_t uCookie, size_t *iReadBytes, uint64_t *puVerifier );
int LFHFS_ScanDir       ( UVFSFileNode psDirNode, scandir_matching_request_t* psMatchingCriteria, scandir_matching_reply_t* psMatchingResult );
int LFHFS_ScanIDs       ( UVFSFileNode psNode, __unused uint64_t uRequestedAttributes, const uint64_t* puFileIDArray, unsigned int iFileIDCount, scanids_match_block_t fMatchCallback);

int DIROPS_RemoveInternal( UVFSFileNode psDirNode, const char *pcUTF8Name );
int DIROPS_LookupInternal( UVFSFileNode psDirNode, const char *pcUTF8Name, UVFSFileNode *ppsOutNode );
#endif /* lf_hfs_dirpos_handler_h */
