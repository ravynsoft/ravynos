/*  Copyright © 2017-2018 Apple Inc. All rights reserved.
 *
 *  lf_hfs_link.c
 *  livefiles_hfs
 *
 *  Created by Or Haimovich on 17/05/2018.
 */

#include "lf_hfs_link.h"
#include "lf_hfs_vfsutils.h"
#include "lf_hfs_vnops.h"
#include "lf_hfs_vfsops.h"
#include "lf_hfs_logger.h"
#include "lf_hfs_utils.h"
#include "lf_hfs_btrees_internal.h"
#include "lf_hfs_xattr.h"
#include "lf_hfs_endian.h"
#include "lf_hfs_format.h"
#include "lf_hfs_defs.h"

/*
 * Private directories where hardlink inodes reside.
 */
const char *hfs_private_names[] = {
    HFSPLUSMETADATAFOLDER,      /* FILE HARDLINKS */
    HFSPLUS_DIR_METADATA_FOLDER /* DIRECTORY HARDLINKS */
};

static int  getfirstlink(struct hfsmount * hfsmp, cnid_t fileid, cnid_t *firstlink);
static int  setfirstlink(struct hfsmount * hfsmp, cnid_t fileid, cnid_t firstlink);
/*
 * Set the first link attribute for a given file id.
 *
 * The attributes b-tree must already be locked.
 * If journaling is enabled, a transaction must already be started.
 */
static int
setfirstlink(struct hfsmount * hfsmp, cnid_t fileid, cnid_t firstlink)
{
    FCB * btfile;
    BTreeIterator * iterator;
    FSBufferDescriptor btdata;
    u_int8_t attrdata[FIRST_LINK_XATTR_REC_SIZE];
    HFSPlusAttrData *dataptr;
    int result;
    u_int16_t datasize;

    if (hfsmp->hfs_attribute_cp == NULL) {
        return (EPERM);
    }
    iterator = hfs_mallocz(sizeof(*iterator));
    if (iterator == NULL)
        return ENOMEM;

    result = hfs_buildattrkey(fileid, FIRST_LINK_XATTR_NAME, (HFSPlusAttrKey *)&iterator->key);
    if (result) {
        goto out;
    }
    dataptr = (HFSPlusAttrData *)&attrdata[0];
    dataptr->recordType = kHFSPlusAttrInlineData;
    dataptr->reserved[0] = 0;
    dataptr->reserved[1] = 0;

    /*
     * Since attrData is variable length, we calculate the size of
     * attrData by subtracting the size of all other members of
     * structure HFSPlusAttData from the size of attrdata.
     */
    (void)snprintf((char *)&dataptr->attrData[0], sizeof(dataptr) - (4 * sizeof(uint32_t)), "%lu", (unsigned long)firstlink);

    dataptr->attrSize = (u_int32_t)( 1 + strlen((char *)&dataptr->attrData[0]));

    /* Calculate size of record rounded up to multiple of 2 bytes. */
    datasize = sizeof(HFSPlusAttrData) - 2 + dataptr->attrSize + ((dataptr->attrSize & 1) ? 1 : 0);

    btdata.bufferAddress = dataptr;
    btdata.itemSize = datasize;
    btdata.itemCount = 1;

    btfile = hfsmp->hfs_attribute_cp->c_datafork;

    /* Insert the attribute. */
    result = BTInsertRecord(btfile, iterator, &btdata, datasize);
    if (result == btExists) {
        result = BTReplaceRecord(btfile, iterator, &btdata, datasize);
    }
    (void) BTFlushPath(btfile);
out:
    hfs_free(iterator);

    return MacToVFSError(result);
}

/*
 * Get the first link attribute for a given file id.
 *
 * The attributes b-tree must already be locked.
 */
static int
getfirstlink(struct hfsmount * hfsmp, cnid_t fileid, cnid_t *firstlink)
{
    FCB * btfile;
    BTreeIterator * iterator;
    FSBufferDescriptor btdata;
    u_int8_t attrdata[FIRST_LINK_XATTR_REC_SIZE];
    HFSPlusAttrData *dataptr;
    int result = 0;

    if (hfsmp->hfs_attribute_cp == NULL) {
        return (EPERM);
    }
    iterator = hfs_mallocz(sizeof(*iterator));
    if (iterator == NULL)
        return ENOMEM;
    
    result = hfs_buildattrkey(fileid, FIRST_LINK_XATTR_NAME, (HFSPlusAttrKey *)&iterator->key);
    if (result)
        goto out;

    dataptr = (HFSPlusAttrData *)&attrdata[0];

    btdata.bufferAddress = dataptr;
    btdata.itemSize = sizeof(attrdata);
    btdata.itemCount = 1;

    btfile = hfsmp->hfs_attribute_cp->c_datafork;

    result = BTSearchRecord(btfile, iterator, &btdata, NULL, NULL);
    if (result)
        goto out;

    if (dataptr->attrSize < 3) {
        result = ENOENT;
        goto out;
    }
    *firstlink = (cnid_t) strtoul((char*)&dataptr->attrData[0], NULL, 10);
out:
    hfs_free(iterator);

    return MacToVFSError(result);
}

/* Find the oldest / last hardlink in the link chain */
int
hfs_lookup_lastlink (struct hfsmount *hfsmp, cnid_t linkfileid, cnid_t *lastid, struct cat_desc *cdesc) {
    int lockflags;
    int error;

    *lastid = 0;

    lockflags = hfs_systemfile_lock(hfsmp, SFL_CATALOG, HFS_SHARED_LOCK);

    error = cat_lookup_lastlink(hfsmp, linkfileid, lastid, cdesc);

    hfs_systemfile_unlock(hfsmp, lockflags);

    /*
     * cat_lookup_lastlink will zero out the lastid/cdesc arguments as needed
     * upon error cases.
     */
    return error;
}

/*
 * Release a specific origin for a directory or file hard link
 *
 * cnode must be lock on entry
 */
void
hfs_relorigin(struct cnode *cp, cnid_t parentcnid)
{
    linkorigin_t *origin, *prev;
    pthread_t thread = pthread_self();

    TAILQ_FOREACH_SAFE(origin, &cp->c_originlist, lo_link, prev)
    {
        if (origin->lo_thread == thread) {
            TAILQ_REMOVE(&cp->c_originlist, origin, lo_link);
            hfs_free(origin);
            break;
        } else if (origin->lo_parentcnid == parentcnid) {
            /*
             * If the threads don't match, then we don't want to
             * delete the entry because that might cause other threads
             * to fall back and use whatever happens to be in
             * c_parentcnid or the wrong link ID.  By setting the
             * values to zero here, it should serve as an indication
             * that the path is no longer valid and that's better than
             * using a random parent ID or link ID.
             */
            origin->lo_parentcnid = 0;
            origin->lo_cnid = 0;
        }
    }
}

/*
 * Remove a link to a hardlink file/dir.
 *
 * Note: dvp and vp cnodes are already locked.
 */
int
hfs_unlink(struct hfsmount *hfsmp, struct vnode *dvp, struct vnode *vp, struct componentname *cnp, int skip_reserve)
{
    struct cnode *cp;
    struct cnode *dcp;
    struct cat_desc cndesc;
    char inodename[32];
    cnid_t  prevlinkid;
    cnid_t  nextlinkid;
    int lockflags = 0;
    int started_tr;
    int error;

    cp = VTOC(vp);
    dcp = VTOC(dvp);

    dcp->c_flag |= C_DIR_MODIFICATION;

    if ((error = hfs_start_transaction(hfsmp)) != 0) {
        started_tr = 0;
        goto out;
    }
    started_tr = 1;

    /*
     * Protect against a race with rename by using the component
     * name passed in and parent id from dvp (instead of using
     * the cp->c_desc which may have changed).
     *
     * Re-lookup the component name so we get the correct cnid
     * for the name (as opposed to the c_cnid in the cnode which
     * could have changed before the cnode was locked).
     */
    cndesc.cd_flags = vnode_isdir(vp) ? CD_ISDIR : 0;
    cndesc.cd_encoding = cp->c_desc.cd_encoding;
    cndesc.cd_nameptr = (const u_int8_t *)cnp->cn_nameptr;
    cndesc.cd_namelen = cnp->cn_namelen;
    cndesc.cd_parentcnid = dcp->c_fileid;
    cndesc.cd_hint = dcp->c_childhint;

    lockflags = SFL_CATALOG | SFL_ATTRIBUTE;
    if (cndesc.cd_flags & CD_ISDIR) {
        /* We'll be removing the alias resource allocation blocks. */
        lockflags |= SFL_BITMAP;
    }
    lockflags = hfs_systemfile_lock(hfsmp, lockflags, HFS_EXCLUSIVE_LOCK);

    if ((error = cat_lookuplink(hfsmp, &cndesc, &cndesc.cd_cnid, &prevlinkid, &nextlinkid))) {
        goto out;
    }

    /* Reserve some space in the catalog file. */
    if (!skip_reserve && (error = cat_preflight(hfsmp, 2 * CAT_DELETE, NULL))) {
        goto out;
    }

    /* Purge any cached origin entries for a directory or file hard link. */
    hfs_relorigin(cp, dcp->c_fileid);
    if (dcp->c_fileid != dcp->c_cnid) {
        hfs_relorigin(cp, dcp->c_cnid);
    }

    /* Delete the link record. */
    if ((error = cat_deletelink(hfsmp, &cndesc))) {
        goto out;
    }

    /* Update the parent directory. */
    if (dcp->c_entries > 0) {
        dcp->c_entries--;
    }
    if (cndesc.cd_flags & CD_ISDIR) {
        DEC_FOLDERCOUNT(hfsmp, dcp->c_attr);
    }
    dcp->c_dirchangecnt++;
    hfs_incr_gencount(dcp);

    struct timeval tv;
    microtime(&tv);
    dcp->c_touch_chgtime = dcp->c_touch_modtime = true;
    dcp->c_flag |= C_MODIFIED;
    hfs_update(dcp->c_vp, 0);

    /*
     * If this is the last link then we need to process the inode.
     * Otherwise we need to fix up the link chain.
     */
    --cp->c_linkcount;
    if (cp->c_linkcount < 1) {
        char delname[32];
        struct cat_desc to_desc;
        struct cat_desc from_desc;

        /*
         * If a file inode or directory inode is being deleted, rename
         * it to an open deleted file.  This ensures that deletion
         * of inode and its corresponding extended attributes does
         * not overflow the journal.  This inode will be deleted
         * either in hfs_vnop_inactive() or in hfs_remove_orphans().
         * Note: a rename failure here is not fatal.
         */
        bzero(&from_desc, sizeof(from_desc));
        bzero(&to_desc, sizeof(to_desc));
        if (vnode_isdir(vp)) {
            if (cp->c_entries != 0) {
                LFHFS_LOG(LEVEL_ERROR, "hfs_unlink: dir not empty (id %d, %d entries)", cp->c_fileid, cp->c_entries);
                hfs_assert(0);
            }
            MAKE_DIRINODE_NAME(inodename, sizeof(inodename),
                               cp->c_attr.ca_linkref);
            from_desc.cd_parentcnid = hfsmp->hfs_private_desc[DIR_HARDLINKS].cd_cnid;
            from_desc.cd_flags = CD_ISDIR;
            to_desc.cd_flags = CD_ISDIR;
        } else {
            MAKE_INODE_NAME(inodename, sizeof(inodename),
                            cp->c_attr.ca_linkref);
            from_desc.cd_parentcnid = hfsmp->hfs_private_desc[FILE_HARDLINKS].cd_cnid;
            from_desc.cd_flags = 0;
            to_desc.cd_flags = 0;
        }
        from_desc.cd_nameptr = (const u_int8_t *)inodename;
        from_desc.cd_namelen = strlen(inodename);
        from_desc.cd_cnid = cp->c_fileid;

        MAKE_DELETED_NAME(delname, sizeof(delname), cp->c_fileid);
        to_desc.cd_nameptr = (const u_int8_t *)delname;
        to_desc.cd_namelen = strlen(delname);
        to_desc.cd_parentcnid = hfsmp->hfs_private_desc[FILE_HARDLINKS].cd_cnid;
        to_desc.cd_cnid = cp->c_fileid;

        error = cat_rename(hfsmp, &from_desc, &hfsmp->hfs_private_desc[FILE_HARDLINKS],
                           &to_desc, (struct cat_desc *)NULL);
        if (error == 0) {
            cp->c_flag |= C_DELETED;
            cp->c_attr.ca_recflags &= ~kHFSHasLinkChainMask;
            cp->c_attr.ca_firstlink = 0;
            if (vnode_isdir(vp)) {
                hfsmp->hfs_private_attr[DIR_HARDLINKS].ca_entries--;
                DEC_FOLDERCOUNT(hfsmp, hfsmp->hfs_private_attr[DIR_HARDLINKS]);

                hfsmp->hfs_private_attr[FILE_HARDLINKS].ca_entries++;
                INC_FOLDERCOUNT(hfsmp, hfsmp->hfs_private_attr[FILE_HARDLINKS]);

                (void)cat_update(hfsmp, &hfsmp->hfs_private_desc[DIR_HARDLINKS],
                                 &hfsmp->hfs_private_attr[DIR_HARDLINKS], NULL, NULL);
                (void)cat_update(hfsmp, &hfsmp->hfs_private_desc[FILE_HARDLINKS],
                                 &hfsmp->hfs_private_attr[FILE_HARDLINKS], NULL, NULL);
            }
        } else {
            error = 0;  /* rename failure here is not fatal */
        }
    } else /* Still some links left */ {
        cnid_t firstlink = 0;

        /*
         * Update the start of the link chain.
         * Note: Directory hard links store the first link in an attribute.
         */
        if (IS_DIR(vp) &&
            getfirstlink(hfsmp, cp->c_fileid, &firstlink) == 0 &&
            firstlink == cndesc.cd_cnid) {
            if (setfirstlink(hfsmp, cp->c_fileid, nextlinkid) == 0)
                cp->c_attr.ca_recflags |= kHFSHasAttributesMask;
        } else if (cp->c_attr.ca_firstlink == cndesc.cd_cnid) {
            cp->c_attr.ca_firstlink = nextlinkid;
        }
        /* Update previous link. */
        if (prevlinkid) {
            (void) cat_update_siblinglinks(hfsmp, prevlinkid, HFS_IGNORABLE_LINK, nextlinkid);
        }
        /* Update next link. */
        if (nextlinkid) {
            (void) cat_update_siblinglinks(hfsmp, nextlinkid, prevlinkid, HFS_IGNORABLE_LINK);
        }
    }

    /*
     * The call to cat_releasedesc below will only release the name
     * buffer; it does not zero out the rest of the fields in the
     * 'cat_desc' data structure.
     *
     * As a result, since there are still other links at this point,
     * we need to make the current cnode descriptor point to the raw
     * inode.  If a path-based system call comes along first, it will
     * replace the descriptor with a valid link ID.  If a userland
     * process already has a file descriptor open, then they will
     * bypass that lookup, though.  Replacing the descriptor CNID with
     * the raw inode will force it to generate a new full path.
     */
    cp->c_cnid = cp->c_fileid;

    /* Push new link count to disk. */
    cp->c_ctime = tv.tv_sec;
    (void) cat_update(hfsmp, &cp->c_desc, &cp->c_attr, NULL, NULL);

    /* All done with the system files. */
    hfs_systemfile_unlock(hfsmp, lockflags);
    lockflags = 0;

    /* Update file system stats. */
    hfs_volupdate(hfsmp, VOL_RMFILE, (dcp->c_cnid == kHFSRootFolderID));

    /*
     * All done with this cnode's descriptor...
     *
     * Note: all future catalog calls for this cnode may be
     * by fileid only.  This is OK for HFS (which doesn't have
     * file thread records) since HFS doesn't support hard links.
     */
    cat_releasedesc(&cp->c_desc);

out:
    if (lockflags) {
        hfs_systemfile_unlock(hfsmp, lockflags);
    }
    if (started_tr) {
        hfs_end_transaction(hfsmp);
    }

    dcp->c_flag &= ~C_DIR_MODIFICATION;
    //TBD - We have wakeup here but can't see anyone who's msleeping on c_flag...
    //wakeup((caddr_t)&dcp->c_flag);

    return (error);
}

/*
 * Cache the origin of a directory or file hard link
 *
 * cnode must be lock on entry
 */
void
hfs_savelinkorigin(cnode_t *cp, cnid_t parentcnid)
{
    linkorigin_t *origin = NULL, *next = NULL;
    pthread_t thread = pthread_self();
    int count = 0;
    int maxorigins = (S_ISDIR(cp->c_mode)) ? MAX_CACHED_ORIGINS : MAX_CACHED_FILE_ORIGINS;
    /*
     *  Look for an existing origin first.  If not found, create/steal one.
     */
    TAILQ_FOREACH_SAFE(origin, &cp->c_originlist, lo_link, next) {
        ++count;
        if (origin->lo_thread == thread) {
            TAILQ_REMOVE(&cp->c_originlist, origin, lo_link);
            break;
        }
    }
    if (origin == NULL) {
        /* Recycle the last (i.e., the oldest) if we have too many. */
        if (count > maxorigins) {
            origin = TAILQ_LAST(&cp->c_originlist, hfs_originhead);
            TAILQ_REMOVE(&cp->c_originlist, origin, lo_link);
        } else {
            origin = hfs_malloc(sizeof(linkorigin_t));
        }
        origin->lo_thread = thread;
    }
    origin->lo_cnid = cp->c_cnid;
    origin->lo_parentcnid = parentcnid;
    TAILQ_INSERT_HEAD(&cp->c_originlist, origin, lo_link);
}

/*
 * Initialize the HFS+ private system directories.
 *
 * These directories are used to hold the inodes
 * for file and directory hardlinks as well as
 * open-unlinked files.
 *
 * If they don't yet exist they will get created.
 *
 * This call is assumed to be made during mount.
 */
void
hfs_privatedir_init(struct hfsmount * hfsmp, enum privdirtype type)
{
    struct vnode * dvp = NULL;
    struct cnode * dcp = NULL;
    struct cat_desc *priv_descp;
    struct cat_attr *priv_attrp;
    struct timeval tv;
    int lockflags;
    int trans = 0;
    int error;

    priv_descp = &hfsmp->hfs_private_desc[type];
    priv_attrp = &hfsmp->hfs_private_attr[type];

    /* Check if directory already exists. */
    if (priv_descp->cd_cnid != 0) {
        return;
    }

    priv_descp->cd_parentcnid = kRootDirID;
    priv_descp->cd_nameptr = (const u_int8_t *)hfs_private_names[type];
    priv_descp->cd_namelen = strlen((const char *)priv_descp->cd_nameptr);
    priv_descp->cd_flags = CD_ISDIR | CD_DECOMPOSED;

    lockflags = hfs_systemfile_lock(hfsmp, SFL_CATALOG, HFS_SHARED_LOCK);
    error = cat_lookup(hfsmp, priv_descp, 0, NULL, priv_attrp, NULL, NULL);

    hfs_systemfile_unlock(hfsmp, lockflags);

    if (error == 0) {
        if (type == FILE_HARDLINKS) {
            hfsmp->hfs_metadata_createdate = (uint32_t) priv_attrp->ca_itime;
        }
        priv_descp->cd_cnid = priv_attrp->ca_fileid;
        goto exit;
    }

    /* Directory is missing, if this is read-only then we're done. */
    if (hfsmp->hfs_flags & HFS_READ_ONLY) {
        goto exit;
    }

    /* Grab the root directory so we can update it later. */
    if (hfs_vget(hfsmp, kRootDirID, &dvp, 0, 0) != 0) {
        goto exit;
    }
    
    dcp = VTOC(dvp);

    /* Setup the default attributes */
    bzero(priv_attrp, sizeof(struct cat_attr));
    priv_attrp->ca_flags = UF_IMMUTABLE | UF_HIDDEN;
    priv_attrp->ca_mode = S_IFDIR;
    if (type == DIR_HARDLINKS) {
        priv_attrp->ca_mode |= S_ISVTX | S_IRUSR | S_IXUSR | S_IRGRP |
        S_IXGRP | S_IROTH | S_IXOTH;
    }
    priv_attrp->ca_linkcount = 1;
    priv_attrp->ca_itime = hfsmp->hfs_itime;
    priv_attrp->ca_recflags = kHFSHasFolderCountMask;

    //TBD - Probebly need to adjust for files app and not for finder....
    struct FndrDirInfo * fndrinfo;
    fndrinfo = (struct FndrDirInfo *)&priv_attrp->ca_finderinfo;
    fndrinfo->frLocation.v = SWAP_BE16(16384);
    fndrinfo->frLocation.h = SWAP_BE16(16384);
    fndrinfo->frFlags = SWAP_BE16(kIsInvisible + kNameLocked);

    if (hfs_start_transaction(hfsmp) != 0) {
        goto exit;
    }
    trans = 1;

    /* Need the catalog and EA b-trees for CNID acquisition */
    lockflags = hfs_systemfile_lock(hfsmp, SFL_CATALOG | SFL_ATTRIBUTE, HFS_EXCLUSIVE_LOCK);

    /* Make sure there's space in the Catalog file. */
    if (cat_preflight(hfsmp, CAT_CREATE, NULL) != 0) {
        hfs_systemfile_unlock(hfsmp, lockflags);
        goto exit;
    }

    /* Get the CNID for use */
    cnid_t new_id;
    if (cat_acquire_cnid(hfsmp, &new_id)) {
        hfs_systemfile_unlock (hfsmp, lockflags);
        goto exit;
    }

    /* Create the private directory on disk. */
    error = cat_create(hfsmp, new_id, priv_descp, priv_attrp, NULL);
    if (error == 0) {
        priv_descp->cd_cnid = priv_attrp->ca_fileid;

        /* Update the parent directory */
        dcp->c_entries++;
        INC_FOLDERCOUNT(hfsmp, dcp->c_attr);
        dcp->c_dirchangecnt++;
        hfs_incr_gencount(dcp);
        microtime(&tv);
        dcp->c_ctime = tv.tv_sec;
        dcp->c_mtime = tv.tv_sec;
        (void) cat_update(hfsmp, &dcp->c_desc, &dcp->c_attr, NULL, NULL);
    }

    hfs_systemfile_unlock(hfsmp, lockflags);

    if (error) {
        goto exit;
    }
    if (type == FILE_HARDLINKS) {
        hfsmp->hfs_metadata_createdate = (uint32_t) priv_attrp->ca_itime;
    }
    hfs_volupdate(hfsmp, VOL_MKDIR, 1);
exit:
    if (trans) {
        hfs_end_transaction(hfsmp);
    }
    if (dvp) {
        hfs_unlock(dcp);
        hfs_vnop_reclaim(dvp);
    }

    //Curently disable -need to understand how much we need this...
//    if ((error == 0) && (type == DIR_HARDLINKS)) {
//        hfs_xattr_init(hfsmp);
//    }
}

/*
 * Release any cached origins for a directory or file hard link
 *
 * cnode must be lock on entry
 */
void
hfs_relorigins(struct cnode *cp)
{
    linkorigin_t *origin, *prev;

    TAILQ_FOREACH_SAFE(origin, &cp->c_originlist, lo_link, prev) {
        hfs_free(origin);
    }
    TAILQ_INIT(&cp->c_originlist);
}

/*
 * Obtain the current parent cnid of a directory or file hard link
 *
 * cnode must be lock on entry
 */
cnid_t
hfs_currentparent(cnode_t *cp, bool have_lock)
{
    if (cp->c_flag & C_HARDLINK) {
        if (!have_lock)
            hfs_lock(cp, HFS_SHARED_LOCK, HFS_LOCK_ALWAYS);

        linkorigin_t *origin;
        pthread_t thread = pthread_self();

        TAILQ_FOREACH(origin, &cp->c_originlist, lo_link) {
            if (origin->lo_thread == thread) {
                if (!have_lock)
                    hfs_unlock(cp);
                return (origin->lo_parentcnid);
            }
        }

        if (!have_lock)
            hfs_unlock(cp);
    }
    return (cp->c_parentcnid);
}

/*
 * Create a new catalog link record
 *
 * An indirect link is a reference to an inode (the real
 * file or directory record).
 *
 * All the indirect links for a given inode are chained
 * together in a doubly linked list.
 *
 * Pre-Leopard file hard links do not have kHFSHasLinkChainBit
 * set and do not have first/prev/next link IDs i.e. the values
 * are zero.  If a new link is being added to an existing
 * pre-Leopard file hard link chain, do not set kHFSHasLinkChainBit.
 */
static int
createindirectlink(struct hfsmount *hfsmp, u_int32_t linknum, struct cat_desc *descp,
                   cnid_t nextcnid, cnid_t *linkcnid, int is_inode_linkchain_set)
{
    struct FndrFileInfo *fip;
    struct cat_attr attr;

    if (linknum == 0) {
        LFHFS_LOG(LEVEL_ERROR, "createindirectlink: linknum is zero!\n");
        return (EINVAL);
    }

    /* Setup the default attributes */
    bzero(&attr, sizeof(attr));

    /* Links are matched to inodes by link ID and to volumes by create date */
    attr.ca_linkref = linknum;
    attr.ca_itime = hfsmp->hfs_metadata_createdate;
    attr.ca_mode = S_IFREG | S_IRUSR | S_IRGRP | S_IROTH;
    attr.ca_recflags = kHFSHasLinkChainMask | kHFSThreadExistsMask;
    attr.ca_flags = UF_IMMUTABLE;
    fip = (struct FndrFileInfo *)&attr.ca_finderinfo;

    if (descp->cd_flags & CD_ISDIR) {
        fip->fdType    = SWAP_BE32 (kHFSAliasType);
        fip->fdCreator = SWAP_BE32 (kHFSAliasCreator);
        fip->fdFlags   = SWAP_BE16 (kIsAlias);
    } else /* file */ {
        fip->fdType    = SWAP_BE32 (kHardLinkFileType);
        fip->fdCreator = SWAP_BE32 (kHFSPlusCreator);
        fip->fdFlags   = SWAP_BE16 (kHasBeenInited);
        /* If the file inode does not have kHFSHasLinkChainBit set
         * and the next link chain ID is zero, assume that this
         * is pre-Leopard file inode.  Therefore clear the bit.
         */
        if ((is_inode_linkchain_set == 0) && (nextcnid == 0)) {
            attr.ca_recflags &= ~kHFSHasLinkChainMask;
        }
    }
    /* Create the indirect link directly in the catalog */
    return cat_createlink(hfsmp, descp, &attr, nextcnid, linkcnid);
}


/*
 * Make a link to the cnode cp in the directory dp
 * using the name in cnp.  src_vp is the vnode that
 * corresponds to 'cp' which was part of the arguments to
 * hfs_vnop_link.
 *
 * The cnodes cp and dcp must be locked.
 */
int
hfs_makelink(struct hfsmount *hfsmp, struct vnode *src_vp, struct cnode *cp,struct cnode *dcp, struct componentname *cnp)
{
    u_int32_t indnodeno = 0;
    char inodename[32];
    struct cat_desc to_desc;
    struct cat_desc link_desc;
    int newlink = 0;
    int retval = 0;
    cnid_t linkcnid = 0;
    cnid_t orig_firstlink = 0;
    enum privdirtype type = S_ISDIR(cp->c_mode) ? DIR_HARDLINKS : FILE_HARDLINKS;

    if (hfsmp->cur_link_id == 0) {
        hfsmp->cur_link_id = ((random() & 0x3fffffff) + 100);
    }

    /* We don't allow link nodes in our private system directories. */
    if (dcp->c_fileid == hfsmp->hfs_private_desc[FILE_HARDLINKS].cd_cnid ||
        dcp->c_fileid == hfsmp->hfs_private_desc[DIR_HARDLINKS].cd_cnid) {
        return (EPERM);
    }

    cat_cookie_t cookie;
    bzero(&cookie, sizeof(cat_cookie_t));
    /* Reserve some space in the Catalog file. */
    if ((retval = cat_preflight(hfsmp, (2 * CAT_CREATE)+ CAT_RENAME, &cookie))) {
        return (retval);
    }

    int lockflags = SFL_CATALOG | SFL_ATTRIBUTE;
    /* Directory hard links allocate space for a symlink. */
    if (type == DIR_HARDLINKS) {
        lockflags |= SFL_BITMAP;
    }
    lockflags = hfs_systemfile_lock(hfsmp, lockflags, HFS_EXCLUSIVE_LOCK);

    /* Save the current cnid value so we restore it if an error occurs. */
    cnid_t orig_cnid = cp->c_desc.cd_cnid;

    /*
     * If this is a new hardlink then we need to create the inode
     * and replace the original file/dir object with a link node.
     */
    if ((cp->c_linkcount == 2) && !(cp->c_flag & C_HARDLINK)) {
        newlink = 1;
        bzero(&to_desc, sizeof(to_desc));
        to_desc.cd_parentcnid = hfsmp->hfs_private_desc[type].cd_cnid;
        to_desc.cd_cnid = cp->c_fileid;
        to_desc.cd_flags = (type == DIR_HARDLINKS) ? CD_ISDIR : 0;

        do {
            if (type == DIR_HARDLINKS) {
                /* Directory hardlinks always use the cnid. */
                indnodeno = cp->c_fileid;
                MAKE_DIRINODE_NAME(inodename, sizeof(inodename),
                                   indnodeno);
            } else {
                /* Get a unique indirect node number */
                if (retval == 0) {
                    indnodeno = cp->c_fileid;
                } else {
                    indnodeno = hfsmp->cur_link_id++;
                }
                MAKE_INODE_NAME(inodename, sizeof(inodename),
                                indnodeno);
            }
            /* Move original file/dir to data node directory */
            to_desc.cd_nameptr = (const u_int8_t *)inodename;
            to_desc.cd_namelen = strlen(inodename);

            retval = cat_rename(hfsmp, &cp->c_desc, &hfsmp->hfs_private_desc[type],
                                &to_desc, NULL);

            if (retval != 0 && retval != EEXIST) {
                LFHFS_LOG(LEVEL_ERROR, "hfs_makelink: cat_rename to %s failed (%d) fileid=%d, vol=%s\n",
                          inodename, retval, cp->c_fileid, hfsmp->vcbVN);
            }
        } while ((retval == EEXIST) && (type == FILE_HARDLINKS));
        if (retval)
            goto out;

        /*
         * Replace original file/dir with a link record.
         */

        bzero(&link_desc, sizeof(link_desc));
        link_desc.cd_nameptr = cp->c_desc.cd_nameptr;
        link_desc.cd_namelen = cp->c_desc.cd_namelen;
        link_desc.cd_parentcnid = cp->c_parentcnid;
        link_desc.cd_flags = S_ISDIR(cp->c_mode) ? CD_ISDIR : 0;

        retval = createindirectlink(hfsmp, indnodeno, &link_desc, 0, &linkcnid, true);
        if (retval)
        {
            int err;

            /* Restore the cnode's cnid. */
            cp->c_desc.cd_cnid = orig_cnid;

            /* Put the original file back. */
            err = cat_rename(hfsmp, &to_desc, &dcp->c_desc, &cp->c_desc, NULL);
            if (err) {
                if (err != EIO && err != ENXIO)
                    LFHFS_LOG(LEVEL_ERROR, "hfs_makelink: error %d from cat_rename backout 1", err);
                hfs_mark_inconsistent(hfsmp, HFS_ROLLBACK_FAILED);
            }
            if (retval != EIO && retval != ENXIO) {
                LFHFS_LOG(LEVEL_ERROR, "hfs_makelink: createindirectlink (1) failed: %d\n", retval);
                retval = EIO;
            }
            goto out;
        }
        cp->c_attr.ca_linkref = indnodeno;
        cp->c_desc.cd_cnid = linkcnid;
        /* Directory hard links store the first link in an attribute. */
        if (type == DIR_HARDLINKS) {
            if (setfirstlink(hfsmp, cp->c_fileid, linkcnid) == 0)
                cp->c_attr.ca_recflags |= kHFSHasAttributesMask;
        } else /* FILE_HARDLINKS */ {
            cp->c_attr.ca_firstlink = linkcnid;
        }
        cp->c_attr.ca_recflags |= kHFSHasLinkChainMask;
    } else {
        indnodeno = cp->c_attr.ca_linkref;
    }

    /*
     * Create a catalog entry for the new link (parentID + name).
     */

    bzero(&link_desc, sizeof(link_desc));
    link_desc.cd_nameptr = (const u_int8_t *)cnp->cn_nameptr;
    link_desc.cd_namelen = strlen(cnp->cn_nameptr);
    link_desc.cd_parentcnid = dcp->c_fileid;
    link_desc.cd_flags = S_ISDIR(cp->c_mode) ? CD_ISDIR : 0;

    /* Directory hard links store the first link in an attribute. */
    if (type == DIR_HARDLINKS) {
        retval = getfirstlink(hfsmp, cp->c_fileid, &orig_firstlink);
    } else /* FILE_HARDLINKS */ {
        orig_firstlink = cp->c_attr.ca_firstlink;
    }
    if (retval == 0)
        retval = createindirectlink(hfsmp, indnodeno, &link_desc, orig_firstlink, &linkcnid, (cp->c_attr.ca_recflags & kHFSHasLinkChainMask));

    if (retval && newlink) {
        int err;

        /* Get rid of new link */
        (void) cat_delete(hfsmp, &cp->c_desc, &cp->c_attr);

        /* Restore the cnode's cnid. */
        cp->c_desc.cd_cnid = orig_cnid;

        /* Put the original file back. */
        err = cat_rename(hfsmp, &to_desc, &dcp->c_desc, &cp->c_desc, NULL);
        if (err) {
            if (err != EIO && err != ENXIO)
                LFHFS_LOG(LEVEL_ERROR, "hfs_makelink: error %d from cat_rename backout 2", err);
            hfs_mark_inconsistent(hfsmp, HFS_ROLLBACK_FAILED);
        }

        cp->c_attr.ca_linkref = 0;

        if (retval != EIO && retval != ENXIO) {
            LFHFS_LOG(LEVEL_ERROR, "hfs_makelink: createindirectlink (2) failed: %d\n", retval);
            retval = EIO;
        }
        goto out;
    } else if (retval == 0) {

        /* Update the original first link to point back to the new first link. */
        if (cp->c_attr.ca_recflags & kHFSHasLinkChainMask) {
            (void) cat_update_siblinglinks(hfsmp, orig_firstlink, linkcnid, HFS_IGNORABLE_LINK);

            /* Update the inode's first link value. */
            if (type == DIR_HARDLINKS) {
                if (setfirstlink(hfsmp, cp->c_fileid, linkcnid) == 0)
                    cp->c_attr.ca_recflags |= kHFSHasAttributesMask;
            } else {
                cp->c_attr.ca_firstlink = linkcnid;
            }
        }
        /*
         * Finally, if this is a new hardlink then:
         *  - update the private system directory
         *  - mark the cnode as a hard link
         */
        if (newlink) {

            hfsmp->hfs_private_attr[type].ca_entries++;
            /* From application perspective, directory hard link is a
             * normal directory.  Therefore count the new directory
             * hard link for folder count calculation.
             */
            if (type == DIR_HARDLINKS) {
                INC_FOLDERCOUNT(hfsmp, hfsmp->hfs_private_attr[type]);
            }
            retval = cat_update(hfsmp, &hfsmp->hfs_private_desc[type], &hfsmp->hfs_private_attr[type], NULL, NULL);
            if (retval) {
                if (retval != EIO && retval != ENXIO) {
                    LFHFS_LOG(LEVEL_ERROR, "hfs_makelink: cat_update of privdir failed! (%d)\n", retval);
                    retval = EIO;
                }
                hfs_mark_inconsistent(hfsmp, HFS_OP_INCOMPLETE);
            }
            cp->c_flag |= C_HARDLINK;

            vnode_t vp;
            if ((vp = cp->c_vp) != NULL) {
                if (vp != src_vp) {
                    cp->c_flag |= C_NEED_DVNODE_PUT;
                }
            }
            if ((vp = cp->c_rsrc_vp) != NULL) {
                if (vp != src_vp) {
                    cp->c_flag |= C_NEED_RVNODE_PUT;
                }
            }
            cp->c_flag |= C_MODIFIED;
            cp->c_touch_chgtime = TRUE;
        }
    }
out:
    hfs_systemfile_unlock(hfsmp, lockflags);

    cat_postflight(hfsmp, &cookie);

    if (retval == 0 && newlink) {
        hfs_volupdate(hfsmp, VOL_MKFILE, 0);
    }
    return (retval);
}
