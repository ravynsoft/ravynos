/*  Copyright © 2017-2018 Apple Inc. All rights reserved.
 *
 *  lf_hfs_fsops_handler.h
 *  livefiles_hfs
 *
 *  Created by Yakov Ben Zaken on 31/12/2017.
*/

#ifndef lf_hfs_fsops_handler_h
#define lf_hfs_fsops_handler_h

#include "lf_hfs_common.h"
#include "lf_hfs_vnode.h"

uint64_t FSOPS_GetOffsetFromClusterNum(vnode_t vp, uint64_t uClusterNum);
int      LFHFS_Mount   (int iFd, UVFSVolumeId puVolId, __unused UVFSMountFlags puMountFlags,
	__unused UVFSVolumeCredential *psVolumeCreds, UVFSFileNode *ppsRootNode);
int      LFHFS_Unmount (UVFSFileNode psRootNode, UVFSUnmountHint hint);
int      LFHFS_ScanVols (int iFd, UVFSScanVolsRequest *psRequest, UVFSScanVolsReply *psReply );
int      LFHFS_Taste ( int iFd );

extern UVFSFSOps HFS_fsOps;

#endif /* lf_hfs_fsops_handler_h */
