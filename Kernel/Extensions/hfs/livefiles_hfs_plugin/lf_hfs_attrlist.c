/*  Copyright © 2017-2018 Apple Inc. All rights reserved.
 *
 *  lf_hfs_attrlist.c
 *  livefiles_hfs
 *
 *  Created by Or Haimovich on 25/3/18.
 */

#include "lf_hfs_attrlist.h"
#include "lf_hfs_locks.h"
#include "lf_hfs.h"
#include "lf_hfs_vfsutils.h"
#include "lf_hfs_vnops.h"
#include "lf_hfs_fileops_handler.h"
#include "lf_hfs_logger.h"
#include "lf_hfs_chash.h"

static void SetAttrIntoStruct(UVFSDirEntryAttr* psAttrEntry, struct cat_attr* pAttr, struct cat_desc* psDesc, struct hfsmount* psHfsm, struct cat_fork* pDataFork)
{
    psAttrEntry->dea_attrs.fa_validmask = VALID_OUT_ATTR_MASK;

    psAttrEntry->dea_attrs.fa_atime.tv_sec = pAttr->ca_atime;
    psAttrEntry->dea_attrs.fa_ctime.tv_sec = pAttr->ca_ctime;
    psAttrEntry->dea_attrs.fa_mtime.tv_sec = pAttr->ca_mtime;
    psAttrEntry->dea_attrs.fa_birthtime.tv_sec = pAttr->ca_btime;
    psAttrEntry->dea_attrs.fa_atime.tv_nsec = psAttrEntry->dea_attrs.fa_mtime.tv_nsec = psAttrEntry->dea_attrs.fa_ctime.tv_nsec = psAttrEntry->dea_attrs.fa_birthtime.tv_nsec = 0;
    psAttrEntry->dea_attrs.fa_fileid       = psDesc->cd_cnid;
    psAttrEntry->dea_attrs.fa_parentid     = psDesc->cd_parentcnid;
    psAttrEntry->dea_attrs.fa_mode         = pAttr->ca_mode & ALL_UVFS_MODES;
    psAttrEntry->dea_attrs.fa_bsd_flags    = pAttr->ca_bsdflags;

    if (VTTOUVFS(IFTOVT(pAttr->ca_mode)) == UVFS_FA_TYPE_DIR)
    {
        //If its a directory need to add . and .. to the direntries count
        psAttrEntry->dea_attrs.fa_nlink = 2 + pAttr->ca_entries;

        //If this is the root folder need to hide the journal files */
        if (psHfsm->jnl && psDesc->cd_cnid == kHFSRootFolderID)
        {
            psAttrEntry->dea_attrs.fa_nlink -= 2;
        }
        psAttrEntry->dea_attrs.fa_size      = (pAttr->ca_entries + 2) * AVERAGE_HFSDIRENTRY_SIZE ;
    } else {
        psAttrEntry->dea_attrs.fa_nlink = pAttr->ca_linkcount;
        psAttrEntry->dea_attrs.fa_size  = pDataFork->cf_size;
    }

    psAttrEntry->dea_attrs.fa_allocsize = pDataFork->cf_blocks * HFSTOVCB(psHfsm)->blockSize;
    psAttrEntry->dea_attrs.fa_type      = VTTOUVFS(IFTOVT(pAttr->ca_mode));
    psAttrEntry->dea_attrs.fa_gid       = pAttr->ca_gid;
    psAttrEntry->dea_attrs.fa_uid       = pAttr->ca_uid ;
}


static void AddNewAttrEntry(ReadDirBuff_s* psReadDirBuffer, struct hfsmount* psHfsm, struct cat_desc* psDesc, struct cat_attr* pAttr, struct cat_fork* pDataFork, uint32_t* puUioSize, int iIndex, UVFSDirEntryAttr** ppsPrevAttrEntry)
{
    UVFSDirEntryAttr* psAttrEntry = (UVFSDirEntryAttr*) (psReadDirBuffer->pvBuffer + READDIR_BUF_OFFSET(psReadDirBuffer));

    SetAttrIntoStruct(psAttrEntry, pAttr, psDesc, psHfsm, pDataFork);

    psAttrEntry->dea_namelen = psDesc->cd_namelen;
    psAttrEntry->dea_nextrec = _UVFS_DIRENTRYATTR_RECLEN(UVFS_DIRENTRYATTR_NAMEOFF, psDesc->cd_namelen);
    psAttrEntry->dea_spare0 = 0;
    psAttrEntry->dea_nameoff = UVFS_DIRENTRYATTR_NAMEOFF;
    const u_int8_t * name = psDesc->cd_nameptr;
    memcpy( UVFS_DIRENTRYATTR_NAMEPTR(psAttrEntry), name, psDesc->cd_namelen );
    UVFS_DIRENTRYATTR_NAMEPTR(psAttrEntry)[psAttrEntry->dea_namelen] = 0;

    *puUioSize = psAttrEntry->dea_nextrec;

    //Update prevEntry with cookie
    if (*ppsPrevAttrEntry != NULL)
    {
        (*ppsPrevAttrEntry)->dea_nextcookie = iIndex | ((u_int64_t)psDesc->cd_cnid << 32);
    }

    *ppsPrevAttrEntry = psAttrEntry;
    psReadDirBuffer->uBufferResid -= *puUioSize;

    return;
}

static bool CompareTimes(const struct timespec* psTimeA, const struct timespec* psTimeB)
{
    //Returns true if a happened at or after b.
    if (psTimeA->tv_sec == psTimeB->tv_sec)
        return psTimeA->tv_nsec == psTimeB->tv_nsec;
    else
        return psTimeA->tv_sec > psTimeB->tv_sec;
}

static bool DirScanIsMatch(ScanDirRequest_s* psScanDirRequest, struct cat_desc* psDesc, struct cat_attr* pAttr, struct hfsmount* psHfsm, struct cat_fork* pDataFork)
{
    bool bIsMatch = true;
    UVFSDirEntryAttr* psDirEntry = psScanDirRequest->psMatchingResult->smr_entry;
    const char* pcName = (const char *) psDesc->cd_nameptr;

    if (pcName && ((psDesc->cd_namelen == (sizeof(HFSPLUSMETADATAFOLDER) - 1) &&
                    memcmp(pcName, HFSPLUSMETADATAFOLDER, sizeof(HFSPLUSMETADATAFOLDER))) ||
                   (psDesc->cd_namelen == (sizeof(HFSPLUS_DIR_METADATA_FOLDER) - 1) &&
                    memcmp(pcName, HFSPLUS_DIR_METADATA_FOLDER, sizeof(HFSPLUS_DIR_METADATA_FOLDER)))))
    {
        // Skip over special dirs
        return false;
    } else if (pcName == NULL)
    {
        //XXXab: Should not happen anymore
        LFHFS_LOG(LEVEL_ERROR, "got NULL name during scandir: %#x!", psDesc->cd_cnid);
        return false;
    }

    bool bAllowHiddenFiles = (psScanDirRequest->psMatchingCriteria->smr_attribute_filter->fa_validmask & LI_FA_VALID_BSD_FLAGS) &&
                             (psScanDirRequest->psMatchingCriteria->smr_attribute_filter->fa_bsd_flags & UF_HIDDEN);


    // filter out hidden files
    if (bAllowHiddenFiles == false) {
        // Filter out files with BSD UF_HIDDEN flag set or filenames that begins with a dot
        if ( (pAttr->ca_flags & UF_HIDDEN) || (pcName[0] == '.') ) {  
            return false;
        }
        // Filter out directories and files with kFinderInvisibleMask flag set
        if ( S_ISDIR(pAttr->ca_mode)) {
            if (pAttr->ca_finderdirinfo.frFlags & OSSwapHostToBigConstInt16(kFinderInvisibleMask)) {
                return false;
            }
        } else { // file
            if (pAttr->ca_finderfileinfo.fdFlags & OSSwapHostToBigConstInt16(kFinderInvisibleMask)) {
                return false;
            }
        }
    }
    
    // If need to verify name contains
    if (psScanDirRequest->psMatchingCriteria->smr_filename_contains != NULL)
    {
        //For each name in smr_filename_contains
        bool bAtLeastOneNameContainsMatched = false;
        char** ppcNameContainsStr = psScanDirRequest->psMatchingCriteria->smr_filename_contains;
        while ( (*ppcNameContainsStr) && (strlen(*ppcNameContainsStr) != 0) && !bAtLeastOneNameContainsMatched)
        {
            uint64_t uNameContainsLength = strlen(*ppcNameContainsStr);
            if (uNameContainsLength <= strlen(pcName))
            {
                if(!hfs_strstr((const u_int8_t*) pcName, strlen(pcName), (const u_int8_t*) *ppcNameContainsStr, uNameContainsLength))
                {
                    bAtLeastOneNameContainsMatched |= true;
                }
            }
            ppcNameContainsStr++;
        }
        bIsMatch = bAtLeastOneNameContainsMatched;
    }

    if (!bIsMatch) goto check_if_directory;

    // If need to verify name appendix
    if (psScanDirRequest->psMatchingCriteria->smr_filename_ends_with != NULL)
    {
        //For each name in smr_filename_contains
        bool bAtLeastOneNameEndWithMatched = false;
        char** ppcNameEndsWithStr = psScanDirRequest->psMatchingCriteria->smr_filename_ends_with;
        while ( (*ppcNameEndsWithStr) && (strlen(*ppcNameEndsWithStr) != 0) && !bAtLeastOneNameEndWithMatched)
        {
            uint64_t uNameEndsWithLength = strlen(*ppcNameEndsWithStr);
            if (uNameEndsWithLength <= strlen(pcName))
            {
                if ( !hfs_apendixcmp((const u_int8_t*) pcName, strlen(pcName),(const u_int8_t*) *ppcNameEndsWithStr, uNameEndsWithLength) )
                {
                    bAtLeastOneNameEndWithMatched |= true;
                }
            }
            ppcNameEndsWithStr++;
        }
        bIsMatch = bAtLeastOneNameEndWithMatched;
    }

    if (!bIsMatch) goto check_if_directory;

    //If need to validate any other param
    if (psScanDirRequest->psMatchingCriteria->smr_attribute_filter->fa_validmask != 0)
    {
        // If need to verify the file type
        if (psScanDirRequest->psMatchingCriteria->smr_attribute_filter->fa_validmask & UVFS_FA_VALID_TYPE)
        {
            uint32_t uEntryType = VTTOUVFS(IFTOVT(pAttr->ca_mode));
            if ((psScanDirRequest->psMatchingCriteria->smr_attribute_filter->fa_type & uEntryType) != uEntryType)
            {
                bIsMatch = false;
            }
        }

        if (!bIsMatch) goto check_if_directory;

        // If need to verify the file mTime
        if (psScanDirRequest->psMatchingCriteria->smr_attribute_filter->fa_validmask & UVFS_FA_VALID_MTIME)
        {
            //Comapre if the Mtime of the found entry is after the search Mtime
            struct timespec mTime = {0};
            mTime.tv_sec = pAttr->ca_mtime;
            if (!CompareTimes(&mTime, &psScanDirRequest->psMatchingCriteria->smr_attribute_filter->fa_mtime))
            {
                bIsMatch = false;
            }
        }
    }

    if (bIsMatch)
    {
        psScanDirRequest->psMatchingResult->smr_result_type = SEARCH_RESULT_MATCH;
    }

check_if_directory:
    //In case that one of the requested creteria wasn't fullfiled we need to check if this is a folder and return
    if (VTTOUVFS(IFTOVT(pAttr->ca_mode)) == UVFS_FA_TYPE_DIR)
    {
        psScanDirRequest->psMatchingResult->smr_result_type |= SEARCH_RESULT_PUSH;
        bIsMatch = true;
    }

    if (bIsMatch)
    {
        UVFSDirEntryAttr* psAttrEntry = psScanDirRequest->psMatchingResult->smr_entry;
        SetAttrIntoStruct(psAttrEntry, pAttr, psDesc, psHfsm, pDataFork);

        psDirEntry->dea_namelen      = strlen( pcName );
        psDirEntry->dea_spare0       = 0;
        psDirEntry->dea_nameoff      = UVFS_DIRENTRYATTR_NAMEOFF;
        memcpy( UVFS_DIRENTRYATTR_NAMEPTR(psDirEntry), pcName, psDesc->cd_namelen);
        UVFS_DIRENTRYATTR_NAMEPTR(psDirEntry)[psDirEntry->dea_namelen] = 0;
    }
    return bIsMatch;
}

int
hfs_scandir(struct vnode *dvp, ScanDirRequest_s* psScanDirRequest)
{
    int error = 0;
    struct cat_entrylist *ce_list = NULL;
    struct cnode* dcp = VTOC(dvp);
    struct hfsmount*  hfsmp = VTOHFS(dvp);
    uint64_t uCookie = psScanDirRequest->psMatchingCriteria->smr_start_cookie;
    int reachedeof = 0;

    /*
     * Take an exclusive directory lock since we manipulate the directory hints
     */
    if ((error = hfs_lock(dcp, HFS_EXCLUSIVE_LOCK, HFS_LOCK_DEFAULT)))
    {
        return (error);
    }

    /* Initialize a catalog entry list - allocating 2 for eof reference. */
    ce_list = hfs_mallocz(CE_LIST_SIZE(2));
    ce_list->maxentries = 2;

    bool bContinueIterating = true;
    while (bContinueIterating)
    {
        /*
         * Populate the ce_list from the catalog file.
         */
        int lockflags = hfs_systemfile_lock(hfsmp, SFL_CATALOG, HFS_SHARED_LOCK);

        /* Extract directory index and tag (sequence number) from uio_offset */
        int index = uCookie & HFS_INDEX_MASK;
        unsigned int tag = (unsigned int) uCookie & ~HFS_INDEX_MASK;

        /* Get a detached directory hint (cnode must be locked exclusive) */
        directoryhint_t* dirhint = hfs_getdirhint(dcp, ((index - 1) & HFS_INDEX_MASK) | tag, TRUE);

        /* Hide tag from catalog layer. */
        dirhint->dh_index &= HFS_INDEX_MASK;
        if (dirhint->dh_index == HFS_INDEX_MASK)
        {
            dirhint->dh_index = -1;
        }

        error = cat_getentriesattr(hfsmp, dirhint, ce_list, &reachedeof);
        /* Don't forget to release the descriptors later! */

        hfs_systemfile_unlock(hfsmp, lockflags);

        if (error == ENOENT)
        {
            //In case of an empty directory need to set EOF and go to exit
            //We can get ENOENT with partial results so check if really no file was found
            error = 0;
            if (ce_list->realentries == 0)
            {
                psScanDirRequest->psMatchingResult->smr_entry->dea_nextcookie = UVFS_DIRCOOKIE_EOF;
                hfs_reldirhint(dcp, dirhint);
                goto exit;
            }
        }
        else if (error)
        {
            hfs_reldirhint(dcp, dirhint);
            goto exit;
        }

        dcp->c_touch_acctime = true;

        /*
         * Check for a FS corruption in the valence. We're holding the cnode lock
         * exclusive since we need to serialize the directory hints, so if we found
         * that the valence reported 0, but we actually found some items here, then
         * silently minimally self-heal and bump the valence to 1.
         */
        if ((dcp->c_entries == 0) && (ce_list->realentries > 0))
        {
            dcp->c_entries++;
            dcp->c_flag |= C_MODIFIED;
            LFHFS_LOG(LEVEL_DEBUG, "hfs_scandir: repairing valence to non-zero! \n");

            /* force an update on dcp while we're still holding the lock. */
            hfs_update(dvp, 0);
        }

        struct cnode *cp = NULL;
        struct cat_desc* psDesc = &ce_list->entry[0].ce_desc;
        struct cat_attr* psAttr = &ce_list->entry[0].ce_attr;
        struct hfsmount* psHfsmp = VTOHFS(dvp);
        struct cat_fork sDataFork;

        bzero(&sDataFork, sizeof(sDataFork));
        sDataFork.cf_size   = ce_list->entry[0].ce_datasize;
        sDataFork.cf_blocks = ce_list->entry[0].ce_datablks;

        struct vnode *vp = hfs_chash_getvnode(hfsmp, psAttr->ca_fileid, false, false, false);

        if (vp != NULL)
        {
            cp = VTOC(vp);
            /* Only use cnode's decriptor for non-hardlinks */
            if (!(cp->c_flag & C_HARDLINK) && cp->c_desc.cd_nameptr != NULL)
                psDesc = &cp->c_desc;
            psAttr = &cp->c_attr;
            if (cp->c_datafork)
            {
                sDataFork.cf_size   = cp->c_datafork->ff_size;
                sDataFork.cf_blocks = cp->c_datafork->ff_blocks;
            }
        }

        bool bIsAMatch = DirScanIsMatch(psScanDirRequest, psDesc, psAttr, psHfsmp, &sDataFork);

        if (vp != NULL)
        {
            /* All done with cnode. */
            hfs_unlock(cp);
            cp = NULL;
            
            hfs_vnop_reclaim(vp);
        }
        
        /* If we skipped catalog entries for reserved files that should
         * not be listed in namespace, update the index accordingly.
         */
        index++;
        if (ce_list->skipentries)
        {
            index += ce_list->skipentries;
            ce_list->skipentries = 0;
        }

        uCookie = reachedeof ? UVFS_DIRCOOKIE_EOF : (index | ((u_int64_t)ce_list->entry[1].ce_desc.cd_cnid << 32));

        if (bIsAMatch)
        {
            bContinueIterating = false;
            psScanDirRequest->psMatchingResult->smr_entry->dea_nextcookie = uCookie;
            psScanDirRequest->psMatchingResult->smr_entry->dea_nextrec = 0;
        }

        if (reachedeof)
            hfs_reldirhint(dcp, dirhint);
        else
            hfs_insertdirhint(dcp, dirhint);

        // **** check if can move to exit *******
        /* All done with the catalog descriptors. */
        for (uint32_t i =0; i < ce_list->realentries; i++)
        {
            cat_releasedesc(&ce_list->entry[i].ce_desc);
        }
        ce_list->realentries = 0;
    }

exit:
    //Drop the directory lock
    hfs_unlock(dcp);
    dcp = NULL;

    if (ce_list)
    {
        for (int i = 0; i < (int)ce_list->realentries; ++i)
        {
            cat_releasedesc(&ce_list->entry[i].ce_desc);
        }
        hfs_free(ce_list);
    }

    return (error);
}

/*
 * Common function for both hfs_vnop_readdirattr and hfs_vnop_getattrlistbulk.
 * This either fills in a vnode_attr structure or fills in an attrbute buffer
 * Currently the difference in behaviour required for the two vnops is keyed
 * on whether the passed in vnode_attr pointer is null or not. If the pointer
 * is null we fill in buffer passed and if it is not null we fill in the fields
 * of the vnode_attr structure.
 */
int
hfs_readdirattr_internal(struct vnode *dvp, ReadDirBuff_s* psReadDirBuffer, int maxcount, uint32_t *newstate, int *eofflag, int *actualcount, uint64_t uCookie)
{
    int error = 0;
    struct cat_desc *lastdescp = NULL;
    struct cat_entrylist *ce_list = NULL;
    UVFSDirEntryAttr* psPrevAttrEntry = NULL;
    
    int reachedeof = 0;
    *(actualcount) = *(eofflag) = 0;

    /*
     * Take an exclusive directory lock since we manipulate the directory hints
     */
    if ((error = hfs_lock(VTOC(dvp), HFS_EXCLUSIVE_LOCK, HFS_LOCK_DEFAULT)))
    {
        return (error);
    }
    struct cnode* dcp = VTOC(dvp);
    struct hfsmount*  hfsmp = VTOHFS(dvp);

    u_int32_t dirchg = dcp->c_dirchangecnt;

    /* Extract directory index and tag (sequence number) from uio_offset */
    int index = uCookie & HFS_INDEX_MASK;
    unsigned int tag = (unsigned int) uCookie & ~HFS_INDEX_MASK;

    /* Get a detached directory hint (cnode must be locked exclusive) */
    directoryhint_t* dirhint = hfs_getdirhint(dcp, ((index - 1) & HFS_INDEX_MASK) | tag, TRUE);

    /* Hide tag from catalog layer. */
    dirhint->dh_index &= HFS_INDEX_MASK;
    if (dirhint->dh_index == HFS_INDEX_MASK)
    {
        dirhint->dh_index = -1;
    }

    /*
     * Obtain a list of catalog entries and pack their attributes until
     * the output buffer is full or maxcount entries have been packed.
     */
    if (maxcount < 1)
    {
        error = EINVAL;
        goto exit2;
    }

    /* Initialize a catalog entry list. */
    ce_list = hfs_mallocz(CE_LIST_SIZE(maxcount));
    ce_list->maxentries = maxcount;

    /*
     * Populate the ce_list from the catalog file.
     */
    int lockflags = hfs_systemfile_lock(hfsmp, SFL_CATALOG, HFS_SHARED_LOCK);

    error = cat_getentriesattr(hfsmp, dirhint, ce_list, &reachedeof);
    /* Don't forget to release the descriptors later! */

    hfs_systemfile_unlock(hfsmp, lockflags);

    if ((error == ENOENT) || (reachedeof != 0))
    {
        *(eofflag) = true;
        error = 0;
    }
    if (error)
    {
        goto exit1;
    }

    dcp->c_touch_acctime = true;

    /*
     * Check for a FS corruption in the valence. We're holding the cnode lock
     * exclusive since we need to serialize the directory hints, so if we found
     * that the valence reported 0, but we actually found some items here, then
     * silently minimally self-heal and bump the valence to 1.
     */
    if ((dcp->c_entries == 0) && (ce_list->realentries > 0))
    {
        dcp->c_entries++;
        dcp->c_flag |= C_MODIFIED;
        LFHFS_LOG(LEVEL_DEBUG, "hfs_readdirattr_internal: repairing valence to non-zero! \n");

        /* force an update on dcp while we're still holding the lock. */
        hfs_update(dvp, 0);
    }

    /*
     * Drop the directory lock
     */
    hfs_unlock(dcp);
    dcp = NULL;

    /* Process the catalog entries. */
    for (int i = 0; i < (int)ce_list->realentries; ++i)
    {
        struct cnode *cp = NULL;
        struct vnode *vp = NULL;
        struct cat_desc * cdescp;
        struct cat_attr * cattrp;
        struct cat_fork c_datafork;
        struct cat_fork c_rsrcfork;

        bzero(&c_datafork, sizeof(c_datafork));
        bzero(&c_rsrcfork, sizeof(c_rsrcfork));
        cdescp = &ce_list->entry[i].ce_desc;
        cattrp = &ce_list->entry[i].ce_attr;
        c_datafork.cf_size   = ce_list->entry[i].ce_datasize;
        c_datafork.cf_blocks = ce_list->entry[i].ce_datablks;
        c_rsrcfork.cf_size   = ce_list->entry[i].ce_rsrcsize;
        c_rsrcfork.cf_blocks = ce_list->entry[i].ce_rsrcblks;

        vp = hfs_chash_getvnode(hfsmp, cattrp->ca_fileid, false, false, false);

        if (vp != NULL)
        {
            cp = VTOC(vp);
            /* Only use cnode's decriptor for non-hardlinks */
            if (!(cp->c_flag & C_HARDLINK))
                cdescp = &cp->c_desc;
            cattrp = &cp->c_attr;
            if (cp->c_datafork)
            {
                c_datafork.cf_size   = cp->c_datafork->ff_size;
                c_datafork.cf_blocks = cp->c_datafork->ff_blocks;
            }
            if (cp->c_rsrcfork)
            {
                c_rsrcfork.cf_size   = cp->c_rsrcfork->ff_size;
                c_rsrcfork.cf_blocks = cp->c_rsrcfork->ff_blocks;
            }
            /* All done with cnode. */
            hfs_unlock(cp);
            cp = NULL;
            
            hfs_vnop_reclaim(vp);
        }

        u_int32_t currattrbufsize;
        AddNewAttrEntry(psReadDirBuffer, hfsmp, cdescp, cattrp, &c_datafork, &currattrbufsize, index, &psPrevAttrEntry);
        //Check if there was enough space to add the new entry
        if (currattrbufsize == 0)
        {
            break;
        }

        /* Save the last valid catalog entry */
        lastdescp = &ce_list->entry[i].ce_desc;
        index++;
        *actualcount += 1;

        /* Termination checks */
        if ((--maxcount <= 0) || (psReadDirBuffer->uBufferResid < _UVFS_DIRENTRYATTR_RECLEN(UVFS_DIRENTRYATTR_NAMEOFF, 128)))
        {
            break;
        }

    } /* for each catalog entry */

    /*
     * If we couldn't fit all the entries requested in the user's buffer,
     * it's not EOF.
     */
    if (*eofflag && (*actualcount < (int)ce_list->realentries))
        *eofflag = 0;

    /* If we skipped catalog entries for reserved files that should
     * not be listed in namespace, update the index accordingly.
     */
    if (ce_list->skipentries)
    {
        index += ce_list->skipentries;
        ce_list->skipentries = 0;
    }

    /*
     * If there are more entries then save the last name.
     * Key this behavior based on whether or not we observed EOFFLAG.
     *
     * Do not use the valence as a way to determine if we hit EOF, since
     * it can be wrong.  Use the catalog's output only.
     */
    if ((*(eofflag) == 0) && (lastdescp != NULL))
    {
        /* Remember last entry */
        if ((dirhint->dh_desc.cd_flags & CD_HASBUF) && (dirhint->dh_desc.cd_nameptr != NULL))
        {
            dirhint->dh_desc.cd_flags &= ~CD_HASBUF;
            if (dirhint->dh_desc.cd_nameptr != NULL)
                hfs_free((void *) dirhint->dh_desc.cd_nameptr);
            dirhint->dh_desc.cd_nameptr = NULL;
        }
        if (lastdescp->cd_nameptr != NULL)
        {
            dirhint->dh_desc.cd_namelen = lastdescp->cd_namelen;
            dirhint->dh_desc.cd_nameptr = hfs_malloc(sizeof(char)*lastdescp->cd_namelen);
            if (dirhint->dh_desc.cd_nameptr == NULL)
            {
                error = ENOMEM;
                goto exit2;
            }
            memcpy((void *) dirhint->dh_desc.cd_nameptr,(void *) lastdescp->cd_nameptr,lastdescp->cd_namelen);
            dirhint->dh_desc.cd_flags |= CD_HASBUF;
        }
        else
        {
            dirhint->dh_desc.cd_namelen = 0;
            dirhint->dh_desc.cd_nameptr = NULL;
        }
        dirhint->dh_index = index - 1;
        dirhint->dh_desc.cd_cnid = lastdescp->cd_cnid;
        dirhint->dh_desc.cd_hint = lastdescp->cd_hint;
        dirhint->dh_desc.cd_encoding = lastdescp->cd_encoding;
    }

    /* All done with the catalog descriptors. */
    for (int i = 0; i < (int)ce_list->realentries; ++i)
    {
        cat_releasedesc(&ce_list->entry[i].ce_desc);
    }
    ce_list->realentries = 0;

    (void) hfs_lock(VTOC(dvp), HFS_EXCLUSIVE_LOCK, HFS_LOCK_ALLOW_NOEXISTS);
    dcp = VTOC(dvp);

exit1:
    /* Pack directory index and tag into uio_offset. */
    while (tag == 0){
        tag = (++dcp->c_dirhinttag) << HFS_INDEX_BITS;
    }
    
    if (psPrevAttrEntry != NULL)
    {
        //Need to update next cookie in the last entry
        psPrevAttrEntry->dea_nextcookie = *eofflag ? UVFS_DIRCOOKIE_EOF : index | tag;
        // Last entry in the buffer should always have dea_nextrec = 0
        psPrevAttrEntry->dea_nextrec = 0;
    }
    dirhint->dh_index |= tag;

exit2:
    if (newstate)
        *newstate = dirchg;

    /*
     * Drop directory hint on error or if there are no more entries,
     * only if EOF was seen.
     */
    if (dirhint)
    {
        if ((error != 0) || *(eofflag))
        {
            hfs_reldirhint(dcp, dirhint);
        }
        else
        {
            hfs_insertdirhint(dcp, dirhint);
        }
    }

    if (ce_list)
        hfs_free(ce_list);

    hfs_unlock(dcp);
    return (error);
}
