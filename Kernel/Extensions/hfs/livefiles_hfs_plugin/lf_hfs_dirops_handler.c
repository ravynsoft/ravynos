/*  Copyright © 2017-2018 Apple Inc. All rights reserved.
 *
 *  lf_hfs_fileops_handler.c
 *  livefiles_hfs
 *
 *  Created by Yakov Ben Zaken on 31/12/2017.
 */

#include "lf_hfs_dirops_handler.h"
#include "lf_hfs_fileops_handler.h"
#include "lf_hfs_vnode.h"
#include "lf_hfs_lookup.h"
#include "lf_hfs_logger.h"
#include "lf_hfs_vnops.h"
#include "lf_hfs_vfsutils.h"
#include "lf_hfs_attrlist.h"
#include "lf_hfs_vfsops.h"

//---------------------------------- Functions Decleration ---------------------------------------
static int DIROPS_VerifyCookieAndVerifier(uint64_t uCookie, vnode_t psParentVnode, uint64_t uVerifier);
//---------------------------------- Functions Implementation ------------------------------------

static int
DIROPS_VerifyCookieAndVerifier(uint64_t uCookie, vnode_t psParentVnode, uint64_t uVerifier)
{
    int iError = 0;
    struct cnode* dcp = VTOC(psParentVnode);

    if ( uCookie == 0 )
    {
        if ( uVerifier != UVFS_DIRCOOKIE_VERIFIER_INITIAL )
        {
            iError =  UVFS_READDIR_VERIFIER_MISMATCHED;
            goto exit;
        }
    }
    else if (uCookie == UVFS_DIRCOOKIE_EOF)
    {
        iError =  UVFS_READDIR_EOF_REACHED;
        goto exit;
    }
    else if ( uVerifier != psParentVnode->sExtraData.sDirData.uDirVersion )
    {
        iError = UVFS_READDIR_VERIFIER_MISMATCHED;
        goto exit;
    }

    cnid_t uChildIndex = (cnid_t)(uCookie & HFS_INDEX_MASK);
    if (uChildIndex > (dcp->c_entries + 2))
    { /* searching pass the last item */
        iError = UVFS_READDIR_BAD_COOKIE;
    }

exit:
    return iError;
}

int DIROPS_RemoveInternal( UVFSFileNode psDirNode, const char *pcUTF8Name )
{
    int iErr = 0;
    vnode_t psParentVnode               = (vnode_t)psDirNode;
    UVFSFileNode psFileNode             = {0};

    if (!vnode_isdir(psParentVnode))
    {
        return ENOTDIR;
    }

    iErr = DIROPS_LookupInternal( psDirNode, pcUTF8Name, &psFileNode );
    if ( iErr != 0 )
    {
        goto exit;
    }
    vnode_t psVnode = (vnode_t)psFileNode;

    if (vnode_isdir(psVnode))
    {
        return EISDIR;
    }

    struct componentname sCompName  = {0};
    sCompName.cn_nameiop            = DELETE;
    sCompName.cn_flags              = ISLASTCN;
    sCompName.cn_pnbuf              = (char *)pcUTF8Name;
    sCompName.cn_pnlen              = (int)strlen(pcUTF8Name);
    sCompName.cn_nameptr            = (char *)pcUTF8Name;
    sCompName.cn_namelen            = (int)strlen(pcUTF8Name);
    sCompName.cn_hash               = 0;
    sCompName.cn_consume            = (int)strlen(pcUTF8Name);

    iErr = hfs_vnop_remove(psParentVnode,psVnode, &sCompName, VNODE_REMOVE_NODELETEBUSY | VNODE_REMOVE_SKIP_NAMESPACE_EVENT );

    LFHFS_Reclaim(psFileNode);

exit:
    return iErr;
}

int DIROPS_LookupInternal( UVFSFileNode psDirNode, const char *pcUTF8Name, UVFSFileNode *ppsOutNode )
{
    int iErr = 0;
    // We are not supporting "." and ".." lookup.
    if ( (strcmp( (char*)pcUTF8Name, "." ) == 0) || (strcmp( (char*)pcUTF8Name, ".." ) == 0) )
    {
        *ppsOutNode = NULL;
        iErr = EPERM;
        goto exit;
    }

    vnode_t psVnode = (vnode_t)psDirNode;

    if (!vnode_isdir(psVnode))
    {
        iErr = ENOTDIR;
        goto exit;
    }

    struct componentname    sCompName       = {0};
    sCompName.cn_nameiop    = LOOKUP;
    sCompName.cn_flags      = ISLASTCN;
    sCompName.cn_pnbuf      = (char *)pcUTF8Name;
    sCompName.cn_pnlen      = (int)strlen(pcUTF8Name);
    sCompName.cn_nameptr    = (char *)pcUTF8Name;
    sCompName.cn_namelen    = (int)strlen(pcUTF8Name);
    sCompName.cn_hash       = 0;
    sCompName.cn_consume    = (int)strlen(pcUTF8Name);

    iErr = hfs_vnop_lookup( psVnode, (vnode_t*)ppsOutNode, &sCompName );

exit:
    return iErr;
}

int
LFHFS_MkDir ( UVFSFileNode psDirNode, const char *pcName, const UVFSFileAttributes *psFileAttr, UVFSFileNode *ppsOutNode )
{
    LFHFS_LOG(LEVEL_DEBUG, "LFHFS_MkDir\n");
    VERIFY_NODE_IS_VALID(psDirNode);

    int iError = 0;
    vnode_t psParentVnode = (vnode_t)psDirNode;

    if (!vnode_isdir(psParentVnode))
    {
        iError = ENOTDIR;
        goto exit;
    }

    //@param cnp Name information for new directory.
    struct componentname sNewDirComponentName = {0};
    sNewDirComponentName.cn_nameptr = (char*) pcName;
    sNewDirComponentName.cn_namelen = (int) strlen(pcName);

    iError = hfs_vnop_mkdir(psParentVnode,(vnode_t*)ppsOutNode, &sNewDirComponentName, (UVFSFileAttributes *) psFileAttr);

exit:
    return iError;
}

int
LFHFS_RmDir ( UVFSFileNode psDirNode, const char *pcUTF8Name )
{
    LFHFS_LOG(LEVEL_DEBUG, "LFHFS_RmDir\n");
    VERIFY_NODE_IS_VALID(psDirNode);

    int iErr                            = 0;
    vnode_t psParentVnode               = (vnode_t)psDirNode;

    if (!vnode_isdir(psParentVnode))
    {
        iErr = ENOTDIR;
        goto exit;
    }

    UVFSFileNode psFileNode             = {0};
    struct componentname    sCompName   = {0};

    iErr = DIROPS_LookupInternal( psDirNode, pcUTF8Name, &psFileNode );
    if ( iErr != 0 )
    {
        goto exit;
    }

    vnode_t psVnode = (vnode_t)psFileNode;

    sCompName.cn_nameiop    = DELETE;
    sCompName.cn_flags      = ISLASTCN;
    sCompName.cn_pnbuf      = (char *)pcUTF8Name;
    sCompName.cn_pnlen      = (int)strlen(pcUTF8Name);
    sCompName.cn_nameptr    = (char *)pcUTF8Name;
    sCompName.cn_namelen    = (int)strlen(pcUTF8Name);
    sCompName.cn_hash       = 0;
    sCompName.cn_consume    = (int)strlen(pcUTF8Name);

    iErr =  hfs_vnop_rmdir(psParentVnode, psVnode, &sCompName);

    hfs_vnop_reclaim(psVnode);

exit:
    return iErr;
}

int
LFHFS_Remove ( UVFSFileNode psDirNode, const char *pcUTF8Name, __unused UVFSFileNode victimNode)
{
    LFHFS_LOG(LEVEL_DEBUG, "LFHFS_Remove\n");
    VERIFY_NODE_IS_VALID(psDirNode);

    int iErr = DIROPS_RemoveInternal( psDirNode, pcUTF8Name );
    return iErr;
}

int
LFHFS_Lookup ( UVFSFileNode psDirNode, const char *pcUTF8Name, UVFSFileNode *ppsOutNode )
{
    LFHFS_LOG(LEVEL_DEBUG, "LFHFS_Lookup\n");
    VERIFY_NODE_IS_VALID(psDirNode);

    return DIROPS_LookupInternal( psDirNode, pcUTF8Name, ppsOutNode );
}

int
LFHFS_ReadDir ( UVFSFileNode psDirNode, void* pvBuf, size_t iBufLen, uint64_t uCookie, size_t *iReadBytes, uint64_t *puVerifier )
{
    LFHFS_LOG(LEVEL_DEBUG, "LFHFS_ReadDir\n");
    VERIFY_NODE_IS_VALID(psDirNode);
    
    int iError = 0;
    *iReadBytes = 0;
    struct vnode* psParentVnode = (struct vnode*) psDirNode;

    if (iReadBytes == NULL || puVerifier == NULL)
    {
        return EINVAL;
    }
    *iReadBytes = 0;

    // Make sure the UVFSFileNode is a directory.
    if ( !IS_DIR(psParentVnode) )
    {
        LFHFS_LOG(LEVEL_ERROR, "HFS_ReadDir node is not a directory.\n", ENOTDIR);
        return ENOTDIR;
    }

    // Make sure there is a place for at least one entry with maximal allowed name
    uint64_t uMaxRecLen = UVFS_DIRENTRY_RECLEN(MAX_UTF8_NAME_LENGTH);
    if ( iBufLen < uMaxRecLen )
    {
        return EINVAL;
    }

    iError = DIROPS_VerifyCookieAndVerifier(uCookie,psParentVnode, *puVerifier);
    if ( iError != 0 )
    {
        goto exit;
    }


    *puVerifier = psParentVnode->sExtraData.sDirData.uDirVersion;

    //Setting readDir Args
    int iEofflag;
    int iNumdirent;
    int flags = VNODE_READDIR_EXTENDED|VNODE_READDIR_REQSEEKOFF;
    ReadDirBuff_s sReadDirBuffer = {0};
    sReadDirBuffer.pvBuffer = pvBuf;
    sReadDirBuffer.uBufferResid =  sReadDirBuffer.uBufferSize = iBufLen;

    iError = hfs_vnop_readdir( psParentVnode, &iEofflag, &iNumdirent, &sReadDirBuffer, uCookie, flags);

    if (iError)
        goto exit;

    if (iNumdirent == 0)
    {
        if (iEofflag)
        {
            iError = UVFS_READDIR_EOF_REACHED;
        }
        else
        {
            iError = EINVAL;
        }
    }

    *iReadBytes = sReadDirBuffer.uBufferSize - sReadDirBuffer.uBufferResid;
exit:
    return iError;
}

int
LFHFS_ReadDirAttr( UVFSFileNode psDirNode, void *pvBuf, size_t iBufLen, uint64_t uCookie, size_t *iReadBytes, uint64_t *puVerifier )
{
    LFHFS_LOG(LEVEL_DEBUG, "LFHFS_ReadDirAttr\n");
    VERIFY_NODE_IS_VALID(psDirNode);
    
    int iError = 0;
    *iReadBytes = 0;
    struct vnode* psParentVnode = (struct vnode*) psDirNode;

    if (iReadBytes == NULL || puVerifier == NULL)
    {
        return EINVAL;
    }
    *iReadBytes = 0;

    // Make sure the UVFSFileNode is a directory.
    if ( !IS_DIR(psParentVnode) )
    {
        LFHFS_LOG(LEVEL_ERROR, "HFS_ReadDir node is not a directory.\n", ENOTDIR);
        return ENOTDIR;
    }

    // Make sure there is a place for at least one entry with maximal allowed name
    uint64_t uMaxRecLen = UVFS_DIRENTRY_RECLEN(MAX_UTF8_NAME_LENGTH);
    if ( iBufLen < uMaxRecLen )
    {
        return EINVAL;
    }

    iError = DIROPS_VerifyCookieAndVerifier(uCookie, psParentVnode, *puVerifier);
    if ( iError != 0 )
    {
        goto exit;
    }

    *puVerifier = psParentVnode->sExtraData.sDirData.uDirVersion;


    //Setting readDirAttr Args
    int iEofflag;
    int iNumdirent;
    ReadDirBuff_s sReadDirBuffer = {0};
    sReadDirBuffer.pvBuffer = pvBuf;
    sReadDirBuffer.uBufferResid =  sReadDirBuffer.uBufferSize = iBufLen;

    iError = hfs_vnop_readdirattr( psParentVnode, &iEofflag, &iNumdirent, &sReadDirBuffer, uCookie);

    if (iError)
        goto exit;

    if (iNumdirent == 0)
    {
        if (iEofflag)
        {
            iError = UVFS_READDIR_EOF_REACHED;
        }
        else
        {
            iError = EINVAL;
        }
    }

    *iReadBytes = sReadDirBuffer.uBufferSize - sReadDirBuffer.uBufferResid;

exit:
    return iError;
}

int
LFHFS_ScanDir(UVFSFileNode psDirNode, scandir_matching_request_t* psMatchingCriteria, scandir_matching_reply_t* psMatchingResult)
{
    LFHFS_LOG(LEVEL_DEBUG, "LFHFS_ScanDir\n");
    VERIFY_NODE_IS_VALID(psDirNode);

    int iErr = 0;
    struct vnode* psParentVnode = (struct vnode*) psDirNode;
    // Make sure the UVFSFileNode is a directory.
    if ( !IS_DIR(psParentVnode) )
    {
        LFHFS_LOG(LEVEL_ERROR, "LFHFS_ScanDir node is not a directory.\n", ENOTDIR);
        return ENOTDIR;
    }

    iErr = DIROPS_VerifyCookieAndVerifier(psMatchingCriteria->smr_start_cookie, psParentVnode, psMatchingCriteria->smr_verifier);
    if ( iErr != 0 )
    {
        goto exit;
    }

    psMatchingResult->smr_result_type = 0;
    psMatchingResult->smr_verifier = psParentVnode->sExtraData.sDirData.uDirVersion;
    ScanDirRequest_s psScanDirRequest = {.psMatchingCriteria = psMatchingCriteria, .psMatchingResult = psMatchingResult};
    
    iErr = hfs_scandir( psParentVnode, &psScanDirRequest);

exit:
    return iErr;
}

int LFHFS_ScanIDs(UVFSFileNode psNode,
                 __unused uint64_t uRequestedAttributes,
                 const uint64_t* puFileIDArray,
                 unsigned int iFileIDCount,
                 scanids_match_block_t fMatchCallback)
{
    int error = 0;
    LFHFS_LOG(LEVEL_DEBUG, "LFHFS_ScanIDs\n");
    VERIFY_NODE_IS_VALID(psNode);
    struct vnode* psVnode = (struct vnode*) psNode;

    char* pcName = malloc(sizeof(char)*MAX_UTF8_NAME_LENGTH);
    if (pcName == NULL)
        return ENOMEM;

    for (uint32_t uIDCounter = 0; uIDCounter < iFileIDCount; uIDCounter++)
    {
        memset(pcName,0,sizeof(char)*MAX_UTF8_NAME_LENGTH);
        //if we got to the rootParentID just continue
        if ((cnid_t)puFileIDArray[uIDCounter] == kHFSRootParentID)
            continue;

        UVFSFileAttributes sFileAttrs;
        error = hfs_GetInfoByID(VTOHFS(psVnode), (cnid_t)puFileIDArray[uIDCounter], &sFileAttrs, pcName);
        if (error == ENOENT) {
            error = 0;
            continue;
        }

        if (!error) {
            if ((cnid_t)puFileIDArray[uIDCounter] == kHFSRootFolderID) {
                sFileAttrs.fa_parentid = sFileAttrs.fa_fileid;
            }
            LFHFS_LOG(LEVEL_DEBUG, "scan found item %llu parent %llu",
                      sFileAttrs.fa_parentid, sFileAttrs.fa_fileid);
            fMatchCallback((int) uIDCounter, &sFileAttrs, pcName);
        } else {
            LFHFS_LOG(LEVEL_DEBUG, "LFHFS_ScanIDs: hfs_GetInfoByID failed with error %u\n",error);
            break;
        }
    }

    free(pcName);
    return error;
}
