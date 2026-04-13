/*  Copyright © 2017-2018 Apple Inc. All rights reserved.
 *
 *  lf_hfs_cnode.c
 *  livefiles_hfs
 *
 *  Created by Or Haimovich on 20/3/18.
 */

#include "lf_hfs_cnode.h"
#include "lf_hfs.h"
#include "lf_hfs_vfsops.h"
#include "lf_hfs_chash.h"
#include "lf_hfs_vfsutils.h"
#include "lf_hfs_vnops.h"
#include "lf_hfs_logger.h"
#include "lf_hfs_utils.h"
#include "lf_hfs_btrees_internal.h"
#include "lf_hfs_readwrite_ops.h"
#include "lf_hfs_utils.h"
#include <sys/stat.h>
#include "lf_hfs_xattr.h"
#include "lf_hfs_link.h"
#include "lf_hfs_generic_buf.h"

static void
hfs_reclaim_cnode(struct cnode *cp)
{
    /*
     * If the descriptor has a name then release it
     */
    if ((cp->c_desc.cd_flags & CD_HASBUF) && (cp->c_desc.cd_nameptr != 0))
    {
        cp->c_desc.cd_flags &= ~CD_HASBUF;
        cp->c_desc.cd_namelen = 0;
        hfs_free((void*)cp->c_desc.cd_nameptr);
        cp->c_desc.cd_nameptr = NULL;
    }

    /*
     * We only call this function if we are in hfs_vnop_reclaim and
     * attempting to reclaim a cnode with only one live fork.  Because the vnode
     * went through reclaim, any future attempts to use this item will have to
     * go through lookup again, which will need to create a new vnode.  Thus,
     * destroying the locks below is safe.
     */

    lf_lck_rw_destroy(&cp->c_rwlock);
    lf_cond_destroy(&cp->c_cacsh_cond);
    lf_lck_rw_destroy(&cp->c_truncatelock);

    hfs_free(cp);
}

/*
 * hfs_getnewvnode - get new default vnode
 *
 * The vnode is returned with an iocount and the cnode locked.
 * The cnode of the parent vnode 'dvp' may or may not be locked, depending on
 * the circumstances.   The cnode in question (if acquiring the resource fork),
 * may also already be locked at the time we enter this function.
 *
 * Note that there are both input and output flag arguments to this function.
 * If one of the input flags (specifically, GNV_USE_VP), is set, then
 * hfs_getnewvnode will use the parameter *vpp, which is traditionally only
 * an output parameter, as both an input and output parameter.  It will use
 * the vnode provided in the output, and pass it to vnode_create with the
 * proper flavor so that a new vnode is _NOT_ created on our behalf when
 * we dispatch to VFS.  This may be important in various HFS vnode creation
 * routines, such a create or get-resource-fork, because we risk deadlock if
 * jetsam is involved.
 *
 * Deadlock potential exists if jetsam is synchronously invoked while we are waiting
 * for a vnode to be recycled in order to give it the identity we want.  If jetsam
 * happens to target a process for termination that is blocked in-kernel, waiting to
 * acquire the cnode lock on our parent 'dvp', while our current thread has it locked,
 * neither side will make forward progress and the watchdog timer will eventually fire.
 * To prevent this, a caller of hfs_getnewvnode may choose to proactively force
 * any necessary vnode reclamation/recycling while it is not holding any locks and
 * thus not prone to deadlock.  If this is the case, GNV_USE_VP will be set and
 * the parameter will be used as described above.
 *
 *  !!! <NOTE> !!!!
 * In circumstances when GNV_USE_VP is set, this function _MUST_ clean up and either consume
 * or dispose of the provided vnode. We funnel all errors to a single return value so that
 * if provided_vp is still non-NULL, then we will dispose of the vnode. This will occur in
 * all error cases of this function --  anywhere we zero/NULL out the *vpp parameter. It may
 * also occur if the current thread raced with another to create the same vnode, and we
 * find the entry already present in the cnode hash.
 * !!! </NOTE> !!!
 */
int
hfs_getnewvnode(struct hfsmount *hfsmp, struct vnode *dvp, struct componentname *cnp, struct cat_desc *descp, int flags, struct cat_attr *attrp, struct cat_fork *forkp, struct vnode **vpp, int *out_flags)
{
    struct mount *mp = HFSTOVFS(hfsmp);
    struct vnode *vp = NULL;
    struct vnode **cvpp;
    struct cnode *cp = NULL;
    struct filefork *fp = NULL;
    struct vnode *provided_vp = NULL;
    struct vnode_fsparam vfsp = {0};
    enum   vtype vtype = IFTOVT(attrp->ca_mode);
    int retval = 0;
    int hflags = 0;
    int issystemfile = (descp->cd_flags & CD_ISMETA) && (vtype == VREG);
    int wantrsrc = flags & GNV_WANTRSRC;;
    int need_update_identity = 0;
    
    /* Zero out the out_flags */
    *out_flags = 0;

    if (flags & GNV_USE_VP)
    {
        /* Store the provided VP for later use */
        provided_vp = *vpp;
    }

    /* Zero out the vpp regardless of provided input */
    *vpp = NULL;

    if (attrp->ca_fileid == 0)
    {
        retval = ENOENT;
        goto gnv_exit;
    }

    /* Sanity checks: */
    if ( (vtype == VBAD) ||
         ( (vtype != VDIR && forkp &&
           ( (attrp->ca_blocks < forkp->cf_blocks) || (howmany((uint64_t)forkp->cf_size, hfsmp->blockSize) > forkp->cf_blocks) ||
             ( (vtype == VLNK) && ((uint64_t)forkp->cf_size > MAXPATHLEN) ) ) ) ) )
    {
         /* Mark the FS as corrupt and bail out */
         hfs_mark_inconsistent(hfsmp, HFS_INCONSISTENCY_DETECTED);
         retval = EINVAL;
         goto gnv_exit;
    }

    /*
     * Get a cnode (new or existing)
     */
    cp = hfs_chash_getcnode(hfsmp, attrp->ca_fileid, vpp, wantrsrc, (flags & GNV_SKIPLOCK), out_flags, &hflags);

    /*
     * If the id is no longer valid for lookups we'll get back a NULL cp.
     */
    if (cp == NULL)
    {
        retval = ENOENT;
        goto gnv_exit;
    }

    /*
     * We may have been provided a vnode via
     * GNV_USE_VP.  In this case, we have raced with
     * a 2nd thread to create the target vnode. The provided
     * vnode that was passed in will be dealt with at the
     * end of the function, as we don't zero out the field
     * until we're ready to pass responsibility to VFS.
     */


    /*
     * If we get a cnode/vnode pair out of hfs_chash_getcnode, then update the
     * descriptor in the cnode as needed if the cnode represents a hardlink.
     * We want the caller to get the most up-to-date copy of the descriptor
     * as possible. However, we only do anything here if there was a valid vnode.
     * If there isn't a vnode, then the cnode is brand new and needs to be initialized
     * as it doesn't have a descriptor or cat_attr yet.
     *
     * If we are about to replace the descriptor with the user-supplied one, then validate
     * that the descriptor correctly acknowledges this item is a hardlink.  We could be
     * subject to a race where the calling thread invoked cat_lookup, got a valid lookup
     * result but the file was not yet a hardlink. With sufficient delay between there
     * and here, we might accidentally copy in the raw inode ID into the descriptor in the
     * call below.  If the descriptor's CNID is the same as the fileID then it must
     * not yet have been a hardlink when the lookup occurred.
     */

    if (!(cp->c_flag & (C_DELETED | C_NOEXISTS)))
    {
        //
        // If the bytes of the filename in the descp do not match the bytes in the
        // cnp (and we're not looking up the resource fork), then we want to update
        // the vnode identity to contain the bytes that HFS stores so that when an
        // fsevent gets generated, it has the correct filename.  otherwise daemons
        // that match filenames produced by fsevents with filenames they have stored
        // elsewhere (e.g. bladerunner, backupd, mds), the filenames will not match.
        // See: <rdar://problem/8044697> FSEvents doesn't always decompose diacritical unicode chars in the paths of the changed directories
        // for more details.
        //
        if (*vpp && cnp && cnp->cn_nameptr && descp && descp->cd_nameptr && strncmp((const char *)cnp->cn_nameptr, (const char *)descp->cd_nameptr, descp->cd_namelen) != 0)
        {
            vnode_update_identity (*vpp, dvp, (const char *)descp->cd_nameptr, descp->cd_namelen, 0, VNODE_UPDATE_NAME);
        }

        if ((cp->c_flag & C_HARDLINK) && descp->cd_nameptr && descp->cd_namelen > 0)
        {
            /* If cnode is uninitialized, its c_attr will be zeroed out; cnids wont match. */
            if ((descp->cd_cnid == cp->c_attr.ca_fileid)  && (attrp->ca_linkcount != cp->c_attr.ca_linkcount))
            {

                if ((flags & GNV_SKIPLOCK) == 0)
                {
                    /*
                     * Then we took the lock. Drop it before calling
                     * vnode_put, which may invoke hfs_vnop_inactive and need to take
                     * the cnode lock again.
                     */
                    hfs_unlock(cp);
                }

                /*
                 * Emit ERECYCLE and GNV_CAT_ATTRCHANGED to
                 * force a re-drive in the lookup routine.
                 * Drop the iocount on the vnode obtained from
                 * chash_getcnode if needed.
                 */
                if (*vpp != NULL)
                {
                    hfs_free(*vpp);
                    *vpp = NULL;
                }

                /*
                 * If we raced with VNOP_RECLAIM for this vnode, the hash code could
                 * have observed it after the c_vp or c_rsrc_vp fields had been torn down;
                 * the hash code peeks at those fields without holding the cnode lock because
                 * it needs to be fast.  As a result, we may have set H_ATTACH in the chash
                 * call above.  Since we're bailing out, unset whatever flags we just set, and
                 * wake up all waiters for this cnode.
                 */
                if (hflags)
                {
                    hfs_chashwakeup(hfsmp, cp, hflags);
                }

                *out_flags = GNV_CAT_ATTRCHANGED;
                retval = ERECYCLE;
                goto gnv_exit;
            }
            else
            {
                /*
                 * Otherwise, CNID != fileid. Go ahead and copy in the new descriptor.
                 *
                 * Replacing the descriptor here is fine because we looked up the item without
                 * a vnode in hand before.  If a vnode existed, its identity must be attached to this
                 * item.  We are not susceptible to the lookup fastpath issue at this point.
                 */
                replace_desc(cp, descp);

                /*
                 * This item was a hardlink, and its name needed to be updated. By replacing the
                 * descriptor above, we've now updated the cnode's internal representation of
                 * its link ID/CNID, parent ID, and its name.  However, VFS must now be alerted
                 * to the fact that this vnode now has a new parent, since we cannot guarantee
                 * that the new link lived in the same directory as the alternative name for
                 * this item.
                 */
                if ((*vpp != NULL) && (cnp || cp->c_desc.cd_nameptr))
                {
                    /* we could be requesting the rsrc of a hardlink file... */
                    if (cp->c_desc.cd_nameptr)
                    {
                        // Update the identity with what we have stored on disk as the name of this file.
                        vnode_update_identity (*vpp, dvp, (const char *)cp->c_desc.cd_nameptr, cp->c_desc.cd_namelen, 0, (VNODE_UPDATE_PARENT | VNODE_UPDATE_NAME));
                    }
                    else if (cnp)
                    {
                        vnode_update_identity (*vpp, dvp, cnp->cn_nameptr, cnp->cn_namelen, cnp->cn_hash, (VNODE_UPDATE_PARENT | VNODE_UPDATE_NAME));
                    }
                }
            }
        }
    }

    /*
     * At this point, we have performed hardlink and open-unlinked checks
     * above.  We have now validated the state of the vnode that was given back
     * to us from the cnode hash code and find it safe to return.
     */
    if (*vpp != NULL)
    {
        retval = 0;
        goto gnv_exit;
    }

    /*
     * If this is a new cnode then initialize it.
     */
    if (ISSET(cp->c_hflag, H_ALLOC))
    {
        lf_lck_rw_init(&cp->c_truncatelock);

        /* Make sure its still valid (ie exists on disk). */
        if (!(flags & GNV_CREATE))
        {
            int error = 0;
            if (!hfs_valid_cnode (hfsmp, dvp, (wantrsrc ? NULL : cnp), cp->c_fileid, attrp, &error))
            {
                hfs_chash_abort(hfsmp, cp);
                if ((flags & GNV_SKIPLOCK) == 0)
                {
                    hfs_unlock(cp);
                }

                hfs_reclaim_cnode(cp);
                *vpp = NULL;
                /*
                 * If we hit this case, that means that the entry was there in the catalog when
                 * we did a cat_lookup earlier.  Think hfs_lookup.  However, in between the time
                 * that we checked the catalog and the time we went to get a vnode/cnode for it,
                 * it had been removed from the namespace and the vnode totally reclaimed.  As a result,
                 * it's not there in the catalog during the check in hfs_valid_cnode and we bubble out
                 * an ENOENT.  To indicate to the caller that they should really double-check the
                 * entry (it could have been renamed over and gotten a new fileid), we mark a bit
                 * in the output flags.
                 */
                if (error == ENOENT)
                {
                    *out_flags = GNV_CAT_DELETED;
                    retval = ENOENT;
                    goto gnv_exit;
                }

                /*
                 * Also, we need to protect the cat_attr acquired during hfs_lookup and passed into
                 * this function as an argument because the catalog may have changed w.r.t hardlink
                 * link counts and the firstlink field.  If that validation check fails, then let
                 * lookup re-drive itself to get valid/consistent data with the same failure condition below.
                 */
                if (error == ERECYCLE)
                {
                    *out_flags = GNV_CAT_ATTRCHANGED;
                    retval = ERECYCLE;
                    goto gnv_exit;
                }
            }
        }
        bcopy(attrp, &cp->c_attr, sizeof(struct cat_attr));
        bcopy(descp, &cp->c_desc, sizeof(struct cat_desc));
        
        /* The name was inherited so clear descriptor state... */
        descp->cd_nameptr = NULL;
        descp->cd_namelen = 0;
        descp->cd_flags &= ~CD_HASBUF;

        /* Tag hardlinks */
        if ( (vtype == VREG || vtype == VDIR || vtype == VSOCK || vtype == VFIFO) &&
             (descp->cd_cnid != attrp->ca_fileid || ISSET(attrp->ca_recflags, kHFSHasLinkChainMask) ) )
        {
            cp->c_flag |= C_HARDLINK;
        }

        /*
         * Fix-up dir link counts.
         *
         * Earlier versions of Leopard used ca_linkcount for posix
         * nlink support (effectively the sub-directory count + 2).
         * That is now accomplished using the ca_dircount field with
         * the corresponding kHFSHasFolderCountMask flag.
         *
         * For directories the ca_linkcount is the true link count,
         * tracking the number of actual hardlinks to a directory.
         *
         * We only do this if the mount has HFS_FOLDERCOUNT set;
         * at the moment, we only set that for HFSX volumes.
         */
        if ( (hfsmp->hfs_flags & HFS_FOLDERCOUNT) && (vtype == VDIR) &&
             (!(attrp->ca_recflags & kHFSHasFolderCountMask)) && (cp->c_attr.ca_linkcount > 1) )
        {
            if (cp->c_attr.ca_entries == 0)
            {
                cp->c_attr.ca_dircount = 0;
            }
            else
            {
                cp->c_attr.ca_dircount = cp->c_attr.ca_linkcount - 2;
            }

            cp->c_attr.ca_linkcount = 1;
            cp->c_attr.ca_recflags |= kHFSHasFolderCountMask;
            if ( !(hfsmp->hfs_flags & HFS_READ_ONLY) )
            {
                cp->c_flag |= C_MODIFIED;
            }
        }

        /* Mark the output flag that we're vending a new cnode */
        *out_flags |= GNV_NEW_CNODE;
    }

    if (vtype == VDIR)
    {
        if (cp->c_vp != NULL)
        {
            LFHFS_LOG(LEVEL_ERROR, "hfs_getnewvnode: orphaned vnode (data)");
            assert(0);
        }
        cvpp = &cp->c_vp;
    }
    else
    {
        /*
         * Allocate and initialize a file fork...
         */
        fp = hfs_malloc(sizeof(struct filefork));
        if (fp == NULL)
        {
            retval = ENOMEM;
            goto gnv_exit;
        }
        memset(fp,0,sizeof(struct filefork));

        fp->ff_cp = cp;
        if (forkp)
        {
            bcopy(forkp, &fp->ff_data, sizeof(struct cat_fork));
        }
        else
        {
            bzero(&fp->ff_data, sizeof(struct cat_fork));
        }
        rl_init(&fp->ff_invalidranges);
        fp->ff_sysfileinfo = 0;

        if (wantrsrc)
        {
            if (cp->c_rsrcfork != NULL)
            {
                LFHFS_LOG(LEVEL_ERROR, "hfs_getnewvnode: orphaned rsrc fork");
                hfs_assert(0);
            }
            if (cp->c_rsrc_vp != NULL)
            {
                LFHFS_LOG(LEVEL_ERROR, "hfs_getnewvnode: orphaned vnode (rsrc)");
                hfs_assert(0);
            }
            cp->c_rsrcfork = fp;
            cvpp = &cp->c_rsrc_vp;
            if (cp->c_vp != NULL )
            {
                cp->c_flag |= C_NEED_DVNODE_PUT;
            }
        }
        else
        {
            if (cp->c_datafork != NULL)
            {
                LFHFS_LOG(LEVEL_ERROR, "hfs_getnewvnode: orphaned data fork");
                hfs_assert(0);
            }
            if (cp->c_vp != NULL)
            {
                LFHFS_LOG(LEVEL_ERROR, "hfs_getnewvnode: orphaned vnode (data)");
                hfs_assert(0);
            }

            cp->c_datafork = fp;
            cvpp = &cp->c_vp;
            if (cp->c_rsrc_vp != NULL)
            {
                cp->c_flag |= C_NEED_RVNODE_PUT;
            }
        }
    }

    vfsp.vnfs_mp = mp;
    vfsp.vnfs_vtype = vtype;
    vfsp.vnfs_str = "hfs";
    if ((cp->c_flag & C_HARDLINK) && (vtype == VDIR))
    {
        vfsp.vnfs_dvp = NULL;  /* no parent for me! */
        vfsp.vnfs_cnp = NULL;  /* no name for me! */
    }
    else
    {
        vfsp.vnfs_dvp = dvp;
        if (cnp)
        {
            vfsp.vnfs_cnp = hfs_malloc(sizeof(struct componentname));
            if (vfsp.vnfs_cnp == NULL)
            {
                if (fp)
                {
                    hfs_free(fp);
                }
                retval = ENOMEM;
                goto gnv_exit;
            }
            bzero(vfsp.vnfs_cnp, sizeof(struct componentname));
            memcpy((void*) vfsp.vnfs_cnp, (void*)cnp, sizeof(struct componentname));
            vfsp.vnfs_cnp->cn_nameptr = lf_hfs_utils_allocate_and_copy_string( (char*) cnp->cn_nameptr, cnp->cn_namelen );

        } else {
            // Incase of ScanID of hardlinks, take the filename from the cnode
            if (cp && cp->c_desc.cd_nameptr) {
                vfsp.vnfs_cnp = hfs_malloc(sizeof(struct componentname));
                if (vfsp.vnfs_cnp == NULL) {
                    if (fp) hfs_free(fp);
                    retval = ENOMEM;
                    goto gnv_exit;
                }
                bzero(vfsp.vnfs_cnp, sizeof(struct componentname));
                vfsp.vnfs_cnp->cn_nameptr = lf_hfs_utils_allocate_and_copy_string( (char*) cp->c_desc.cd_nameptr, cp->c_desc.cd_namelen );
                vfsp.vnfs_cnp->cn_namelen = cp->c_desc.cd_namelen;
            }
        }
    }

    vfsp.vnfs_fsnode = cp;
    vfsp.vnfs_rdev = 0;

    if (forkp)
    {
        vfsp.vnfs_filesize = forkp->cf_size;
    }
    else
    {
        vfsp.vnfs_filesize = 0;
    }

    if (cnp && cnp->cn_nameptr && cp->c_desc.cd_nameptr && strncmp((const char *)cnp->cn_nameptr, (const char *)cp->c_desc.cd_nameptr, cp->c_desc.cd_namelen) != 0)
    {
        //
        // We don't want VFS to add an entry for this vnode because the name in the
        // cnp does not match the bytes stored on disk for this file.  Instead we'll
        // update the identity later after the vnode is created and we'll do so with
        // the correct bytes for this filename.  For more details, see:
        //   <rdar://problem/8044697> FSEvents doesn't always decompose diacritical unicode chars in the paths of the changed directories
        //
        need_update_identity = 1;
    }


    /* Tag system files */
    vfsp.vnfs_marksystem = issystemfile;

    /* Tag root directory */
    if (descp->cd_cnid == kHFSRootFolderID)
    {
        vfsp.vnfs_markroot = 1;
    }
    else
    {
        vfsp.vnfs_markroot = 0;
    }

    /*
     * If provided_vp was non-NULL, then it is an already-allocated (but not
     * initialized) vnode. We simply need to initialize it to this identity.
     * If it was NULL, then assume that we need to call vnode_create with the
     * normal arguments/types.
     */
    if (provided_vp)
    {
        vp = provided_vp;
        /*
         * After we assign the value of provided_vp into 'vp' (so that it can be
         * mutated safely by vnode_initialize), we can NULL it out.  At this point, the disposal
         * and handling of the provided vnode will be the responsibility of VFS, which will
         * clean it up and vnode_put it properly if vnode_initialize fails.
         */
        provided_vp = NULL;
        retval = vnode_initialize (sizeof(struct vnode_fsparam), &vfsp, &vp);
        /* See error handling below for resolving provided_vp */
    }
    else
    {
        /* Do a standard vnode_create */
        retval = vnode_create (sizeof(struct vnode_fsparam), &vfsp, &vp);
    }

    /*
     * We used a local variable to hold the result of vnode_create/vnode_initialize so that
     * on error cases in vnode_create we won't accidentally harm the cnode's fields
     */

    if (retval)
    {
        /* Clean up if we encountered an error */
        if (fp) {
            if (fp == cp->c_datafork)
                cp->c_datafork = NULL;
            else
                cp->c_rsrcfork = NULL;

            hfs_free(fp);
        }
        /*
         * If this is a newly created cnode or a vnode reclaim
         * occurred during the attachment, then cleanup the cnode.
         */
        if ((cp->c_vp == NULL) && (cp->c_rsrc_vp == NULL))
        {
            hfs_chash_abort(hfsmp, cp);

            if ((flags & GNV_SKIPLOCK) == 0)
            {
                hfs_unlock(cp);
            }
            hfs_reclaim_cnode(cp);
        }
        else
        {
            hfs_chashwakeup(hfsmp, cp, H_ALLOC | H_ATTACH);
            if ((flags & GNV_SKIPLOCK) == 0)
            {
                hfs_unlock(cp);
            }
        }
        *vpp = NULL;
        goto gnv_exit;
    }

    /* If no error, then assign the value into the cnode's fields  */
    *cvpp = vp;

    if (cp->c_flag & C_HARDLINK)
    {
        //TBD - this set is for vfs -> since we have the C_HARDLINK
        //      currently disable this set.
        //vnode_setmultipath(vp);
    }

    if (vp && need_update_identity)
    {
        //
        // As above, update the name of the vnode if the bytes stored in hfs do not match
        // the bytes in the cnp.  See this radar:
        //    <rdar://problem/8044697> FSEvents doesn't always decompose diacritical unicode chars in the paths of the changed directories
        // for more details.
        //
        vnode_update_identity (vp, dvp, (const char *)cp->c_desc.cd_nameptr, cp->c_desc.cd_namelen, 0, VNODE_UPDATE_NAME);
    }
    /*
     * Tag resource fork vnodes as needing an VNOP_INACTIVE
     * so that any deferred removes (open unlinked files)
     * have the chance to process the resource fork.
     */
    if (vp && VNODE_IS_RSRC(vp))
    {
        vp->is_rsrc = true;
    }
    hfs_chashwakeup(hfsmp, cp, H_ALLOC | H_ATTACH);

    SET_NODE_AS_VALID(vp);
    *vpp = vp;
    retval = 0;

gnv_exit:
    if (provided_vp)
    {
        /* Release our empty vnode if it was not used */
        vnode_rele (provided_vp);
    }
    return retval;
}

/*
 * Check ordering of two cnodes. Return true if they are are in-order.
 */
static int
hfs_isordered(struct cnode *cp1, struct cnode *cp2)
{
    if (cp1 == cp2)
        return (0);
    if (cp1 == NULL || cp2 == (struct cnode *)0xffffffff)
        return (1);
    if (cp2 == NULL || cp1 == (struct cnode *)0xffffffff)
        return (0);
    /*
     * Locking order is cnode address order.
     */
    return (cp1 < cp2);
}

/*
 * Acquire 4 cnode locks.
 *   - locked in cnode address order (lesser address first).
 *   - all or none of the locks are taken
 *   - only one lock taken per cnode (dup cnodes are skipped)
 *   - some of the cnode pointers may be null
 */
int
hfs_lockfour(struct cnode *cp1, struct cnode *cp2, struct cnode *cp3,
             struct cnode *cp4, enum hfs_locktype locktype, struct cnode **error_cnode)
{
    struct cnode * a[3];
    struct cnode * b[3];
    struct cnode * list[4];
    struct cnode * tmp;
    int i, j, k;
    int error;
    if (error_cnode) {
        *error_cnode = NULL;
    }

    if (hfs_isordered(cp1, cp2))
    {
        a[0] = cp1; a[1] = cp2;
    }
    else {
        a[0] = cp2; a[1] = cp1;
    }
    if (hfs_isordered(cp3, cp4)) {
        b[0] = cp3; b[1] = cp4;
    } else {
        b[0] = cp4; b[1] = cp3;
    }
    a[2] = (struct cnode *)0xffffffff;  /* sentinel value */
    b[2] = (struct cnode *)0xffffffff;  /* sentinel value */

    /*
     * Build the lock list, skipping over duplicates
     */
    for (i = 0, j = 0, k = 0; (i < 2 || j < 2); ) {
        tmp = hfs_isordered(a[i], b[j]) ? a[i++] : b[j++];
        if (k == 0 || tmp != list[k-1])
            list[k++] = tmp;
    }

    /*
     * Now we can lock using list[0 - k].
     * Skip over NULL entries.
     */
    for (i = 0; i < k; ++i) {
        if (list[i])
            if ((error = hfs_lock(list[i], locktype, HFS_LOCK_DEFAULT))) {
                /* Only stuff error_cnode if requested */
                if (error_cnode) {
                    *error_cnode = list[i];
                }
                /* Drop any locks we acquired. */
                while (--i >= 0) {
                    if (list[i])
                        hfs_unlock(list[i]);
                }
                return (error);
            }
    }
    return (0);
}

/*
 * Unlock a group of cnodes.
 */
void
hfs_unlockfour(struct cnode *cp1, struct cnode *cp2, struct cnode *cp3, struct cnode *cp4)
{
    struct cnode * list[4];
    int i, k = 0;

    if (cp1) {
        hfs_unlock(cp1);
        list[k++] = cp1;
    }
    if (cp2) {
        for (i = 0; i < k; ++i) {
            if (list[i] == cp2)
                goto skip1;
        }
        hfs_unlock(cp2);
        list[k++] = cp2;
    }
skip1:
    if (cp3) {
        for (i = 0; i < k; ++i) {
            if (list[i] == cp3)
                goto skip2;
        }
        hfs_unlock(cp3);
        list[k++] = cp3;
    }
skip2:
    if (cp4) {
        for (i = 0; i < k; ++i) {
            if (list[i] == cp4)
                return;
        }
        hfs_unlock(cp4);
    }
}

/*
 * Lock a cnode.
 * N.B. If you add any failure cases, *make* sure hfs_lock_always works
 */
int
hfs_lock(struct cnode *cp, enum hfs_locktype locktype, enum hfs_lockflags flags)
{
    pthread_t thread = pthread_self();

    if (cp->c_lockowner == thread)
    {
        /*
         * Only the extents and bitmap files support lock recursion
         * here.  The other system files support lock recursion in
         * hfs_systemfile_lock.  Eventually, we should change to
         * handle recursion solely in hfs_systemfile_lock.
         */
        if ((cp->c_fileid == kHFSExtentsFileID) || (cp->c_fileid == kHFSAllocationFileID))
        {
            cp->c_syslockcount++;
        }
        else
        {
            LFHFS_LOG(LEVEL_ERROR, "hfs_lock: locking against myself!");
            hfs_assert(0);
        }
    }
    else if (locktype == HFS_SHARED_LOCK)
    {
        lf_lck_rw_lock_shared(&cp->c_rwlock);
        cp->c_lockowner = HFS_SHARED_OWNER;
    }
    else if (locktype == HFS_TRY_EXCLUSIVE_LOCK)
    {
        if (!lf_lck_rw_try_lock(&cp->c_rwlock, LCK_RW_TYPE_EXCLUSIVE))
        {
            cp->c_lockowner = thread;

            /* Only the extents and bitmap files support lock recursion. */
            if ((cp->c_fileid == kHFSExtentsFileID) || (cp->c_fileid == kHFSAllocationFileID))
            {
                cp->c_syslockcount = 1;
            }
        }
        else
        {
            return (1);
        }
    }
    else
    { /* HFS_EXCLUSIVE_LOCK */
        lf_lck_rw_lock_exclusive(&cp->c_rwlock);
        cp->c_lockowner = thread;
        /* Only the extents and bitmap files support lock recursion. */
        if ((cp->c_fileid == kHFSExtentsFileID) || (cp->c_fileid == kHFSAllocationFileID))
        {
            cp->c_syslockcount = 1;
        }
    }

    /*
     * Skip cnodes for regular files that no longer exist
     * (marked deleted, catalog entry gone).
     */
    if (((flags & HFS_LOCK_ALLOW_NOEXISTS) == 0) && ((cp->c_desc.cd_flags & CD_ISMETA) == 0) && (cp->c_flag & C_NOEXISTS))
    {
        hfs_unlock(cp);
        return (ENOENT);
    }
    return (0);
}

/*
 * Unlock a cnode.
 */
void
hfs_unlock(struct cnode *cp)
{
    u_int32_t c_flag = 0;

    /*
     * Only the extents and bitmap file's support lock recursion.
     */
    if ((cp->c_fileid == kHFSExtentsFileID) || (cp->c_fileid == kHFSAllocationFileID))
    {
        if (--cp->c_syslockcount > 0)
        {
            return;
        }
    }

    pthread_t thread = pthread_self();

    if (cp->c_lockowner == thread)
    {
        c_flag = cp->c_flag;

        // If we have the truncate lock, we must defer the puts
        if (cp->c_truncatelockowner == thread)
        {
            if (ISSET(c_flag, C_NEED_DVNODE_PUT)
                && !cp->c_need_dvnode_put_after_truncate_unlock)
            {
                CLR(c_flag, C_NEED_DVNODE_PUT);
                cp->c_need_dvnode_put_after_truncate_unlock = true;
            }
            if (ISSET(c_flag, C_NEED_RVNODE_PUT)
                && !cp->c_need_rvnode_put_after_truncate_unlock)
            {
                CLR(c_flag, C_NEED_RVNODE_PUT);
                cp->c_need_rvnode_put_after_truncate_unlock = true;
            }
        }

        CLR(cp->c_flag, (C_NEED_DATA_SETSIZE | C_NEED_RSRC_SETSIZE  | C_NEED_DVNODE_PUT | C_NEED_RVNODE_PUT));

        cp->c_lockowner = NULL;
        lf_lck_rw_unlock_exclusive(&cp->c_rwlock);
    }
    else
    {
        cp->c_lockowner = NULL;
        lf_lck_rw_unlock_shared(&cp->c_rwlock);
    }
}

/*
 * hfs_valid_cnode
 *
 * This function is used to validate data that is stored in-core against what is contained
 * in the catalog.  Common uses include validating that the parent-child relationship still exist
 * for a specific directory entry (guaranteeing it has not been renamed into a different spot) at
 * the point of the check.
 */
int
hfs_valid_cnode(struct hfsmount *hfsmp, struct vnode *dvp, struct componentname *cnp, cnid_t cnid, struct cat_attr *cattr, int *error)
{
    struct cat_attr attr;
    struct cat_desc cndesc;
    int stillvalid = 0;

    /* System files are always valid */
    if (cnid < kHFSFirstUserCatalogNodeID)
    {
        *error = 0;
        return (1);
    }

    /* XXX optimization:  check write count in dvp */
    int lockflags = hfs_systemfile_lock(hfsmp, SFL_CATALOG, HFS_SHARED_LOCK);

    if (dvp && cnp)
    {
        int lookup = 0;
        struct cat_fork fork;
        bzero(&cndesc, sizeof(cndesc));
        cndesc.cd_nameptr = (const u_int8_t *)cnp->cn_nameptr;
        cndesc.cd_namelen = cnp->cn_namelen;
        cndesc.cd_parentcnid = VTOC(dvp)->c_fileid;
        cndesc.cd_hint = VTOC(dvp)->c_childhint;

        /*
         * We have to be careful when calling cat_lookup.  The result argument
         * 'attr' may get different results based on whether or not you ask
         * for the filefork to be supplied as output.  This is because cat_lookupbykey
         * will attempt to do basic validation/smoke tests against the resident
         * extents if there are no overflow extent records, but it needs someplace
         * in memory to store the on-disk fork structures.
         *
         * Since hfs_lookup calls cat_lookup with a filefork argument, we should
         * do the same here, to verify that block count differences are not
         * due to calling the function with different styles.  cat_lookupbykey
         * will request the volume be fsck'd if there is true on-disk corruption
         * where the number of blocks does not match the number generated by
         * summing the number of blocks in the resident extents.
         */
        lookup = cat_lookup (hfsmp, &cndesc, 0, NULL, &attr, &fork, NULL);

        if ((lookup == 0) && (cnid == attr.ca_fileid))
        {
            stillvalid = 1;
            *error = 0;
        }
        else
        {
            *error = ENOENT;
        }
        /*
         * In hfs_getnewvnode, we may encounter a time-of-check vs. time-of-vnode creation
         * race.  Specifically, if there is no vnode/cnode pair for the directory entry
         * being looked up, we have to go to the catalog.  But since we don't hold any locks (aside
         * from the dvp in 'shared' mode) there is nothing to protect us against the catalog record
         * changing in between the time we do the cat_lookup there and the time we re-grab the
         * catalog lock above to do another cat_lookup.
         *
         * However, we need to check more than just the CNID and parent-child name relationships above.
         * Hardlinks can suffer the same race in the following scenario:  Suppose we do a
         * cat_lookup, and find a leaf record and a raw inode for a hardlink.  Now, we have
         * the cat_attr in hand (passed in above).  But in between then and now, the vnode was
         * created by a competing hfs_getnewvnode call, and is manipulated and reclaimed before we get
         * a chance to do anything.  This is possible if there are a lot of threads thrashing around
         * with the cnode hash.  In this case, if we don't check/validate the cat_attr in-hand, we will
         * blindly stuff it into the cnode, which will make the in-core data inconsistent with what is
         * on disk.  So validate the cat_attr below, if required.  This race cannot happen if the cnode/vnode
         * already exists, as it does in the case of rename and delete.
         */
        if (stillvalid && cattr != NULL)
        {
            if (cattr->ca_linkcount != attr.ca_linkcount)
            {
                stillvalid = 0;
                *error = ERECYCLE;
                goto notvalid;
            }

            if (cattr->ca_union1.cau_linkref != attr.ca_union1.cau_linkref)
            {
                stillvalid = 0;
                *error = ERECYCLE;
                goto notvalid;
            }

            if (cattr->ca_union3.cau_firstlink != attr.ca_union3.cau_firstlink)
            {
                stillvalid = 0;
                *error = ERECYCLE;
                goto notvalid;
            }
            if (cattr->ca_union2.cau_blocks != attr.ca_union2.cau_blocks)
            {
                stillvalid = 0;
                *error = ERECYCLE;
                goto notvalid;
            }
        }
    }
    else
    {
        if (cat_idlookup(hfsmp, cnid, 0, 0, NULL, NULL, NULL) == 0)
        {
            stillvalid = 1;
            *error = 0;
        }
        else
        {
            *error = ENOENT;
        }
    }

notvalid:
    hfs_systemfile_unlock(hfsmp, lockflags);
    
    return (stillvalid);
}

/*
 * Protect a cnode against a truncation.
 *
 * Used mainly by read/write since they don't hold the
 * cnode lock across calls to the cluster layer.
 *
 * The process doing a truncation must take the lock
 * exclusive. The read/write processes can take it
 * shared.  The locktype argument is the same as supplied to
 * hfs_lock.
 */
void
hfs_lock_truncate(struct cnode *cp, enum hfs_locktype locktype, enum hfs_lockflags flags)
{
    pthread_t thread = pthread_self();

    if (cp->c_truncatelockowner == thread) {
        /*
         * Ignore grabbing the lock if it the current thread already
         * holds exclusive lock.
         *
         * This is needed on the hfs_vnop_pagein path where we need to ensure
         * the file does not change sizes while we are paging in.  However,
         * we may already hold the lock exclusive due to another
         * VNOP from earlier in the call stack.  So if we already hold
         * the truncate lock exclusive, allow it to proceed, but ONLY if
         * it's in the recursive case.
         */
        if ((flags & HFS_LOCK_SKIP_IF_EXCLUSIVE) == 0)
        {
            LFHFS_LOG(LEVEL_ERROR, "hfs_lock_truncate: cnode %p locked!", cp);
            hfs_assert(0);
        }
    } else if (locktype == HFS_SHARED_LOCK) {
        lf_lck_rw_lock_shared(&cp->c_truncatelock);
        cp->c_truncatelockowner = HFS_SHARED_OWNER;
    } else { /* HFS_EXCLUSIVE_LOCK */
        lf_lck_rw_lock_exclusive(&cp->c_truncatelock);
        cp->c_truncatelockowner = thread;
    }
}

/*
 * Unlock the truncate lock, which protects against size changes.
 *
 * If HFS_LOCK_SKIP_IF_EXCLUSIVE flag was set, it means that a previous
 * hfs_lock_truncate() might have skipped grabbing a lock because
 * the current thread was already holding the lock exclusive and
 * we may need to return from this function without actually unlocking
 * the truncate lock.
 */
void
hfs_unlock_truncate(struct cnode *cp, enum hfs_lockflags flags)
{
    pthread_t thread = pthread_self();

    /*
     * If HFS_LOCK_SKIP_IF_EXCLUSIVE is set in the flags AND the current
     * lock owner of the truncate lock is our current thread, then
     * we must have skipped taking the lock earlier by in
     * hfs_lock_truncate() by setting HFS_LOCK_SKIP_IF_EXCLUSIVE in the
     * flags (as the current thread was current lock owner).
     *
     * If HFS_LOCK_SKIP_IF_EXCLUSIVE is not set (most of the time) then
     * we check the lockowner field to infer whether the lock was taken
     * exclusively or shared in order to know what underlying lock
     * routine to call.
     */
    if (flags & HFS_LOCK_SKIP_IF_EXCLUSIVE) {
        if (cp->c_truncatelockowner == thread) {
            return;
        }
    }

    /* HFS_LOCK_EXCLUSIVE */
    if (thread == cp->c_truncatelockowner) {
//        vnode_t vp = NULL, rvp = NULL;

        /*
         * If there are pending set sizes, the cnode lock should be dropped
         * first.
         */
        hfs_assert(!(cp->c_lockowner == thread
                     && ISSET(cp->c_flag, C_NEED_DATA_SETSIZE | C_NEED_RSRC_SETSIZE)));

//        if (cp->c_need_dvnode_put_after_truncate_unlock) {
//            vp = cp->c_vp;
//            cp->c_need_dvnode_put_after_truncate_unlock = false;
//        }
//        if (cp->c_need_rvnode_put_after_truncate_unlock) {
//            rvp = cp->c_rsrc_vp;
//            cp->c_need_rvnode_put_after_truncate_unlock = false;
//        }

        cp->c_truncatelockowner = NULL;
        lf_lck_rw_unlock_exclusive(&cp->c_truncatelock);
//
//        // Do the puts now
//        if (vp)
//            vnode_put(vp);
//        if (rvp)
//            vnode_put(rvp);
    } else
    { /* HFS_LOCK_SHARED */
        lf_lck_rw_unlock_shared(&cp->c_truncatelock);
    }
}

/*
 * Lock a pair of cnodes.
 */
int
hfs_lockpair(struct cnode *cp1, struct cnode *cp2, enum hfs_locktype locktype)
{
    struct cnode *first, *last;
    int error;

    /*
     * If cnodes match then just lock one.
     */
    if (cp1 == cp2)
    {
        return hfs_lock(cp1, locktype, HFS_LOCK_DEFAULT);
    }

    /*
     * Lock in cnode address order.
     */
    if (cp1 < cp2)
    {
        first = cp1;
        last = cp2;
    }
    else
    {
        first = cp2;
        last = cp1;
    }

    if ( (error = hfs_lock(first, locktype, HFS_LOCK_DEFAULT)))
    {
        return (error);
    }
    if ( (error = hfs_lock(last, locktype, HFS_LOCK_DEFAULT)))
    {
        hfs_unlock(first);
        return (error);
    }
    return (0);
}

/*
 * Unlock a pair of cnodes.
 */
void
hfs_unlockpair(struct cnode *cp1, struct cnode *cp2)
{
    hfs_unlock(cp1);
    if (cp2 != cp1)
        hfs_unlock(cp2);
}

/*
 * Increase the gen count by 1; if it wraps around to 0, increment by
 * two.  The cnode *must* be locked exclusively by the caller.
 *
 * You may think holding the lock is unnecessary because we only need
 * to change the counter, but consider this sequence of events: thread
 * A calls hfs_incr_gencount and the generation counter is 2 upon
 * entry.  A context switch occurs and thread B increments the counter
 * to 3, thread C now gets the generation counter (for whatever
 * purpose), and then another thread makes another change and the
 * generation counter is incremented again---it's now 4.  Now thread A
 * continues and it sets the generation counter back to 3.  So you can
 * see, thread C would miss the change that caused the generation
 * counter to increment to 4 and for this reason the cnode *must*
 * always be locked exclusively.
 */
uint32_t hfs_incr_gencount (struct cnode *cp)
{
    u_int8_t *finfo = NULL;
    u_int32_t gcount = 0;

    /* overlay the FinderInfo to the correct pointer, and advance */
    finfo = (u_int8_t*)cp->c_finderinfo;
    finfo = finfo + 16;

    /*
     * FinderInfo is written out in big endian... make sure to convert it to host
     * native before we use it.
     *
     * NOTE: the write_gen_counter is stored in the same location in both the
     *       FndrExtendedFileInfo and FndrExtendedDirInfo structs (it's the
     *       last 32-bit word) so it is safe to have one code path here.
     */
    if (S_ISDIR(cp->c_attr.ca_mode) || S_ISREG(cp->c_attr.ca_mode))
    {
        struct FndrExtendedFileInfo *extinfo = (struct FndrExtendedFileInfo *)finfo;
        gcount = extinfo->write_gen_counter;

        /* Was it zero to begin with (file originated in 10.8 or earlier?) */
        if (gcount == 0)
        {
            gcount++;
        }

        /* now bump it */
        gcount++;

        /* Did it wrap around ? */
        if (gcount == 0)
        {
            gcount++;
        }
        extinfo->write_gen_counter = OSSwapHostToBigInt32 (gcount);

        SET(cp->c_flag, C_MINOR_MOD);
    }
    else
    {
        gcount = 0;
    }

    return gcount;
}

void hfs_write_gencount (struct cat_attr *attrp, uint32_t gencount)
{
    u_int8_t *finfo = NULL;

    /* overlay the FinderInfo to the correct pointer, and advance */
    finfo = (u_int8_t*)attrp->ca_finderinfo;
    finfo = finfo + 16;

    /*
     * Make sure to write it out as big endian, since that's how
     * finder info is defined.
     *
     * Generation count is only supported for files.
     */
    if (S_ISREG(attrp->ca_mode)) {
        struct FndrExtendedFileInfo *extinfo = (struct FndrExtendedFileInfo *)finfo;
        extinfo->write_gen_counter = OSSwapHostToBigInt32(gencount);
    }

    /* If it were neither directory/file, then we'd bail out */
    return;
}

void hfs_clear_might_be_dirty_flag(cnode_t *cp)
{
    /*
     * If we're about to touch both mtime and ctime, we can clear the
     * C_MIGHT_BE_DIRTY_FROM_MAPPING since we can guarantee that
     * subsequent page-outs can only be for data made dirty before
     * now.
     */
    CLR(cp->c_flag, C_MIGHT_BE_DIRTY_FROM_MAPPING);
}

/*
 * Touch cnode times based on c_touch_xxx flags
 *
 * cnode must be locked exclusive
 *
 * This will also update the volume modify time
 */
void
hfs_touchtimes(struct hfsmount *hfsmp, struct cnode* cp)
{

    if (ISSET(hfsmp->hfs_flags, HFS_READ_ONLY) || ISSET(cp->c_flag, C_NOEXISTS)) {
        cp->c_touch_acctime = FALSE;
        cp->c_touch_chgtime = FALSE;
        cp->c_touch_modtime = FALSE;
        CLR(cp->c_flag, C_NEEDS_DATEADDED);
        return;
    }

    if (cp->c_touch_acctime || cp->c_touch_chgtime ||
        cp->c_touch_modtime || (cp->c_flag & C_NEEDS_DATEADDED)) {
        struct timeval tv;
        int touchvol = 0;

        if (cp->c_touch_modtime && cp->c_touch_chgtime)
            hfs_clear_might_be_dirty_flag(cp);

        microtime(&tv);

        if (cp->c_touch_acctime) {
            /*
             * When the access time is the only thing changing, we
             * won't necessarily write it to disk immediately.  We
             * only do the atime update at vnode recycle time, when
             * fsync is called or when there's another reason to write
             * to the metadata.
             */
            cp->c_atime = tv.tv_sec;
            cp->c_touch_acctime = FALSE;
        }
        if (cp->c_touch_modtime) {
            cp->c_touch_modtime = FALSE;
            time_t new_time = tv.tv_sec;
            if (cp->c_mtime != new_time) {
                cp->c_mtime = new_time;
                cp->c_flag |= C_MINOR_MOD;
                touchvol = 1;
            }
        }
        if (cp->c_touch_chgtime) {
            cp->c_touch_chgtime = FALSE;
            if (cp->c_ctime != tv.tv_sec) {
                cp->c_ctime = tv.tv_sec;
                cp->c_flag |= C_MINOR_MOD;
                touchvol = 1;
            }
        }

        if (cp->c_flag & C_NEEDS_DATEADDED) {
            hfs_write_dateadded (&(cp->c_attr), tv.tv_sec);
            cp->c_flag |= C_MINOR_MOD;
            /* untwiddle the bit */
            cp->c_flag &= ~C_NEEDS_DATEADDED;
            touchvol = 1;
        }

        /* Touch the volume modtime if needed */
        if (touchvol) {
            hfs_note_header_minor_change(hfsmp);
            HFSTOVCB(hfsmp)->vcbLsMod = tv.tv_sec;
        }
    }
}

/*
 * Per HI and Finder requirements, HFS should add in the
 * date/time that a particular directory entry was added
 * to the containing directory.
 * This is stored in the extended Finder Info for the
 * item in question.
 *
 * Note that this field is also set explicitly in the hfs_vnop_setxattr code.
 * We must ignore user attempts to set this part of the finderinfo, and
 * so we need to save a local copy of the date added, write in the user
 * finderinfo, then stuff the value back in.
 */
void hfs_write_dateadded (struct cat_attr *attrp, uint64_t dateadded)
{
    u_int8_t *finfo = NULL;

    /* overlay the FinderInfo to the correct pointer, and advance */
    finfo = (u_int8_t*)attrp->ca_finderinfo;
    finfo = finfo + 16;

    /*
     * Make sure to write it out as big endian, since that's how
     * finder info is defined.
     *
     * NOTE: This is a Unix-epoch timestamp, not a HFS/Traditional Mac timestamp.
     */
    if (S_ISREG(attrp->ca_mode)) {
        struct FndrExtendedFileInfo *extinfo = (struct FndrExtendedFileInfo *)finfo;
        extinfo->date_added = OSSwapHostToBigInt32(dateadded);
        attrp->ca_recflags |= kHFSHasDateAddedMask;
    }
    else if (S_ISDIR(attrp->ca_mode)) {
        struct FndrExtendedDirInfo *extinfo = (struct FndrExtendedDirInfo *)finfo;
        extinfo->date_added = OSSwapHostToBigInt32(dateadded);
        attrp->ca_recflags |= kHFSHasDateAddedMask;
    }
    /* If it were neither directory/file, then we'd bail out */
    return;
}

static u_int32_t
hfs_get_dateadded_internal(const uint8_t *finderinfo, mode_t mode)
{
    const uint8_t *finfo = NULL;
    u_int32_t dateadded = 0;

    /* overlay the FinderInfo to the correct pointer, and advance */
    finfo = finderinfo + 16;

    /*
     * FinderInfo is written out in big endian... make sure to convert it to host
     * native before we use it.
     */
    if (S_ISREG(mode)) {
        const struct FndrExtendedFileInfo *extinfo = (const struct FndrExtendedFileInfo *)finfo;
        dateadded = OSSwapBigToHostInt32 (extinfo->date_added);
    }
    else if (S_ISDIR(mode)) {
        const struct FndrExtendedDirInfo *extinfo = (const struct FndrExtendedDirInfo *)finfo;
        dateadded = OSSwapBigToHostInt32 (extinfo->date_added);
    }

    return dateadded;
}

u_int32_t
hfs_get_dateadded(struct cnode *cp)
{
    if ((cp->c_attr.ca_recflags & kHFSHasDateAddedMask) == 0) {
        /* Date added was never set.  Return 0. */
        return (0);
    }

    return (hfs_get_dateadded_internal((u_int8_t*)cp->c_finderinfo,
                                       cp->c_attr.ca_mode));
}

static bool
hfs_cnode_isinuse(struct cnode *cp, uint32_t uRefCount)
{
    return (cp->uOpenLookupRefCount > uRefCount);
}

/*
 * hfs_cnode_teardown
 *
 * This is an internal function that is invoked from both hfs_vnop_inactive
 * and hfs_vnop_reclaim.  As VNOP_INACTIVE is not necessarily called from vnodes
 * being recycled and reclaimed, it is important that we do any post-processing
 * necessary for the cnode in both places.  Important tasks include things such as
 * releasing the blocks from an open-unlinked file when all references to it have dropped,
 * and handling resource forks separately from data forks.
 *
 * Note that we take only the vnode as an argument here (rather than the cnode).
 * Recall that each cnode supports two forks (rsrc/data), and we can always get the right
 * cnode from either of the vnodes, but the reverse is not true -- we can't determine which
 * vnode we need to reclaim if only the cnode is supplied.
 *
 * This function is idempotent and safe to call from both hfs_vnop_inactive and hfs_vnop_reclaim
 * if both are invoked right after the other.  In the second call, most of this function's if()
 * conditions will fail, since they apply generally to cnodes still marked with C_DELETED.
 * As a quick check to see if this function is necessary, determine if the cnode is already
 * marked C_NOEXISTS.  If it is, then it is safe to skip this function.  The only tasks that
 * remain for cnodes marked in such a fashion is to teardown their fork references and
 * release all directory hints and hardlink origins.  However, both of those are done
 * in hfs_vnop_reclaim.  hfs_update, by definition, is not necessary if the cnode's catalog
 * entry is no longer there.
 *
 * 'reclaim' argument specifies whether or not we were called from hfs_vnop_reclaim.  If we are
 * invoked from hfs_vnop_reclaim, we can not call functions that cluster_push since the UBC info
 * is totally gone by that point.
 *
 * Assumes that both truncate and cnode locks for 'cp' are held.
 */
static int
hfs_cnode_teardown (struct vnode *vp, int reclaim)
{
    int forkcount = 0;
    enum vtype v_type = vp->sFSParams.vnfs_vtype;
    struct cnode* cp = VTOC(vp);
    int error = 0;
    bool started_tr = false;
    struct hfsmount *hfsmp = VTOHFS(vp);
    int truncated = 0;
    cat_cookie_t cookie;
    int cat_reserve = 0;
    int lockflags = 0;
    int ea_error = 0;

    if (cp->c_datafork) {
        ++forkcount;
    }
    if (cp->c_rsrcfork) {
        ++forkcount;
    }

    /*
     * Remove any directory hints or cached origins
     */
    if (v_type == VDIR) {
        hfs_reldirhints(cp, 0);
    }
    if (cp->c_flag & C_HARDLINK) {
        hfs_relorigins(cp);
    }
    /*
     * -- Handle open unlinked files --
     *
     * If the vnode is in use, it means a force unmount is in progress
     * in which case we defer cleaning up until either we come back
     * through here via hfs_vnop_reclaim, at which point the UBC
     * information will have been torn down and the vnode might no
     * longer be in use, or if it's still in use, it will get cleaned
     * up when next remounted.
     */
    if (ISSET(cp->c_flag, C_DELETED) && !hfs_cnode_isinuse(cp, 0)) {
        /*
         * This check is slightly complicated.  We should only truncate data
         * in very specific cases for open-unlinked files.  This is because
         * we want to ensure that the resource fork continues to be available
         * if the caller has the data fork open.  However, this is not symmetric;
         * someone who has the resource fork open need not be able to access the data
         * fork once the data fork has gone inactive.
         *
         * If we're the last fork, then we have cleaning up to do.
         *
         * A) last fork, and vp == c_vp
         *    Truncate away own fork data. If rsrc fork is not in core, truncate it too.
         *
         * B) last fork, and vp == c_rsrc_vp
         *    Truncate ourselves, assume data fork has been cleaned due to C).
         *
         * If we're not the last fork, then things are a little different:
         *
         * C) not the last fork, vp == c_vp
         *    Truncate ourselves.  Once the file has gone out of the namespace,
         *    it cannot be further opened.  Further access to the rsrc fork may
         *    continue, however.
         *
         * D) not the last fork, vp == c_rsrc_vp
         *    Don't enter the block below, just clean up vnode and push it out of core.
         */

        if ((v_type == VREG || v_type == VLNK) &&
            ((forkcount == 1) || (!VNODE_IS_RSRC(vp)))) {

            /* Truncate away our own fork data. (Case A, B, C above) */
            if (VTOF(vp) && VTOF(vp)->ff_blocks != 0) {
                /*
                 * SYMLINKS only:
                 *
                 * Encapsulate the entire change (including truncating the link) in
                 * nested transactions if we are modifying a symlink, because we know that its
                 * file length will be at most 4k, and we can fit both the truncation and
                 * any relevant bitmap changes into a single journal transaction.  We also want
                 * the kill_block code to execute in the same transaction so that any dirty symlink
                 * blocks will not be written. Otherwise, rely on
                 * hfs_truncate doing its own transactions to ensure that we don't blow up
                 * the journal.
                 */
                if (!started_tr && (v_type == VLNK)) {
                    if (hfs_start_transaction(hfsmp) != 0) {
                        error = EINVAL;
                        goto out;
                    }
                    else {
                        started_tr = true;
                    }
                }

                /*
                 * At this point, we have decided that this cnode is
                 * suitable for full removal.  We are about to deallocate
                 * its blocks and remove its entry from the catalog.
                 * If it was a symlink, then it's possible that the operation
                 * which created it is still in the current transaction group
                 * due to coalescing.  Take action here to kill the data blocks
                 * of the symlink out of the journal before moving to
                 * deallocate the blocks.  We need to be in the middle of
                 * a transaction before calling buf_iterate like this.
                 *
                 * Note: we have to kill any potential symlink buffers out of
                 * the journal prior to deallocating their blocks.  This is so
                 * that we don't race with another thread that may be doing an
                 * an allocation concurrently and pick up these blocks. It could
                 * generate I/O against them which could go out ahead of our journal
                 * transaction.
                 */

                if (hfsmp->jnl && vnode_islnk(vp)) {
                    lf_hfs_generic_buf_write_iterate(vp, hfs_removefile_callback, BUF_SKIP_NONLOCKED, (void *)hfsmp);
                }

                /*
                 * This truncate call (and the one below) is fine from VNOP_RECLAIM's
                 * context because we're only removing blocks, not zero-filling new
                 * ones.  The C_DELETED check above makes things much simpler.
                 */
                error = hfs_truncate(vp, (off_t)0, IO_NDELAY, 0);
                if (error) {
                    goto out;
                }
                truncated = 1;

                /* (SYMLINKS ONLY): Close/End our transaction after truncating the file record */
                if (started_tr) {
                    hfs_end_transaction(hfsmp);
                    started_tr = false;
                }

            }

            /*
             * Truncate away the resource fork, if we represent the data fork and
             * it is the last fork.  That means, by definition, the rsrc fork is not in
             * core.  To avoid bringing a vnode into core for the sole purpose of deleting the
             * data in the resource fork, we call cat_lookup directly, then hfs_release_storage
             * to get rid of the resource fork's data. Note that because we are holding the
             * cnode lock, it is impossible for a competing thread to create the resource fork
             * vnode from underneath us while we do this.
             *
             * This is invoked via case A above only.
             */
            if ((cp->c_blocks > 0) && (forkcount == 1) && (vp != cp->c_rsrc_vp)) {
                struct cat_lookup_buffer *lookup_rsrc = NULL;
                struct cat_desc *desc_ptr = NULL;

                lookup_rsrc = hfs_mallocz(sizeof(struct cat_lookup_buffer));

                if (cp->c_desc.cd_namelen == 0) {
                    /* Initialize the rsrc descriptor for lookup if necessary*/
                    MAKE_DELETED_NAME (lookup_rsrc->lookup_name, HFS_TEMPLOOKUP_NAMELEN, cp->c_fileid);

                    lookup_rsrc->lookup_desc.cd_nameptr = (const uint8_t*) lookup_rsrc->lookup_name;
                    lookup_rsrc->lookup_desc.cd_namelen = strlen (lookup_rsrc->lookup_name);
                    lookup_rsrc->lookup_desc.cd_parentcnid = hfsmp->hfs_private_desc[FILE_HARDLINKS].cd_cnid;
                    lookup_rsrc->lookup_desc.cd_cnid = cp->c_cnid;

                    desc_ptr = &lookup_rsrc->lookup_desc;
                }
                else {
                    desc_ptr = &cp->c_desc;
                }

                lockflags = hfs_systemfile_lock (hfsmp, SFL_CATALOG, HFS_SHARED_LOCK);

                error = cat_lookup (hfsmp, desc_ptr, 1, (struct cat_desc *) NULL, (struct cat_attr*) NULL, &lookup_rsrc->lookup_fork.ff_data, NULL);

                hfs_systemfile_unlock (hfsmp, lockflags);

                if (error) {
                    hfs_free(lookup_rsrc);
                    goto out;
                }

                /*
                 * Make the filefork in our temporary struct look like a real
                 * filefork.  Fill in the cp, sysfileinfo and rangelist fields..
                 */
                rl_init (&lookup_rsrc->lookup_fork.ff_invalidranges);
                lookup_rsrc->lookup_fork.ff_cp = cp;

                /*
                 * If there were no errors, then we have the catalog's fork information
                 * for the resource fork in question.  Go ahead and delete the data in it now.
                 */

                error = hfs_release_storage (hfsmp, NULL, &lookup_rsrc->lookup_fork, cp->c_fileid);
                hfs_free(lookup_rsrc);

                if (error) {
                    goto out;
                }

                /*
                 * This fileid's resource fork extents have now been fully deleted on-disk
                 * and this CNID is no longer valid. At this point, we should be able to
                 * zero out cp->c_blocks to indicate there is no data left in this file.
                 */
                cp->c_blocks = 0;
            }
        }

        /*
         * If we represent the last fork (or none in the case of a dir),
         * and the cnode has become open-unlinked...
         *
         * We check c_blocks here because it is possible in the force
         * unmount case for the data fork to be in use but the resource
         * fork to not be in use in which case we will truncate the
         * resource fork, but not the data fork.  It will get cleaned
         * up upon next mount.
         */
        if (forkcount <= 1 && !cp->c_blocks) {
            /*
             * If it has EA's, then we need to get rid of them.
             *
             * Note that this must happen outside of any other transactions
             * because it starts/ends its own transactions and grabs its
             * own locks.  This is to prevent a file with a lot of attributes
             * from creating a transaction that is too large (which panics).
             */
            if (ISSET(cp->c_attr.ca_recflags, kHFSHasAttributesMask))
            {
                ea_error = hfs_removeallattr(hfsmp, cp->c_fileid, &started_tr);
                if (ea_error)
                    goto out;
            }

            /*
             * Remove the cnode's catalog entry and release all blocks it
             * may have been using.
             */

            /*
             * Mark cnode in transit so that no one can get this
             * cnode from cnode hash.
             */
            // hfs_chash_mark_in_transit(hfsmp, cp);
            // XXXdbg - remove the cnode from the hash table since it's deleted
            //          otherwise someone could go to sleep on the cnode and not
            //          be woken up until this vnode gets recycled which could be
            //          a very long time...
            hfs_chashremove(hfsmp, cp);

            cp->c_flag |= C_NOEXISTS;   // XXXdbg
            cp->c_rdev = 0;

            if (!started_tr) {
                if (hfs_start_transaction(hfsmp) != 0) {
                    error = EINVAL;
                    goto out;
                }
                started_tr = true;
            }

            /*
             * Reserve some space in the Catalog file.
             */
            if ((error = cat_preflight(hfsmp, CAT_DELETE, &cookie))) {
                goto out;
            }
            cat_reserve = 1;

            lockflags = hfs_systemfile_lock(hfsmp, SFL_CATALOG | SFL_ATTRIBUTE, HFS_EXCLUSIVE_LOCK);

            if (cp->c_blocks > 0) {
                LFHFS_LOG(LEVEL_ERROR, "hfs_inactive: deleting non-empty%sfile %d, "
                          "blks %d\n", VNODE_IS_RSRC(vp) ? " rsrc " : " ",
                          (int)cp->c_fileid, (int)cp->c_blocks);
            }

            //
            // release the name pointer in the descriptor so that
            // cat_delete() will use the file-id to do the deletion.
            // in the case of hard links this is imperative (in the
            // case of regular files the fileid and cnid are the
            // same so it doesn't matter).
            //
            cat_releasedesc(&cp->c_desc);

            /*
             * The descriptor name may be zero,
             * in which case the fileid is used.
             */
            error = cat_delete(hfsmp, &cp->c_desc, &cp->c_attr);

            if (error && truncated && (error != ENXIO)) {
                LFHFS_LOG(LEVEL_ERROR, "hfs_inactive: couldn't delete a truncated file!");
            }

            /* Update HFS Private Data dir */
            if (error == 0) {
                hfsmp->hfs_private_attr[FILE_HARDLINKS].ca_entries--;
                if (vnode_isdir(vp)) {
                    DEC_FOLDERCOUNT(hfsmp, hfsmp->hfs_private_attr[FILE_HARDLINKS]);
                }
                (void)cat_update(hfsmp, &hfsmp->hfs_private_desc[FILE_HARDLINKS],
                                 &hfsmp->hfs_private_attr[FILE_HARDLINKS], NULL, NULL);
            }

            hfs_systemfile_unlock(hfsmp, lockflags);

            if (error) {
                goto out;
            }

            /* Already set C_NOEXISTS at the beginning of this block */
            cp->c_flag &= ~C_DELETED;
            cp->c_touch_chgtime = TRUE;
            cp->c_touch_modtime = TRUE;

            if (error == 0)
                hfs_volupdate(hfsmp, (v_type == VDIR) ? VOL_RMDIR : VOL_RMFILE, 0);
        }
    } // if <open unlinked>

    hfs_update(vp, reclaim ? HFS_UPDATE_FORCE : 0);

    /*
     * Since we are about to finish what might be an inactive call, propagate
     * any remaining modified or touch bits from the cnode to the vnode.  This
     * serves as a hint to vnode recycling that we shouldn't recycle this vnode
     * synchronously.
     *
     * For now, if the node *only* has a dirty atime, we don't mark
     * the vnode as dirty.  VFS's asynchronous recycling can actually
     * lead to worse performance than having it synchronous.  When VFS
     * is fixed to be more performant, we can be more honest about
     * marking vnodes as dirty when it's only the atime that's dirty.
     */
#if LF_HFS_FULL_VNODE_SUPPORT
    //TBD - need to decide how we mark a file as dirty
    if (hfs_is_dirty(cp) == HFS_DIRTY || ISSET(cp->c_flag, C_DELETED)) {
        vnode_setdirty(vp);
    } else {
        vnode_cleardirty(vp);
    }
#endif

out:
    if (cat_reserve)
        cat_postflight(hfsmp, &cookie);

    if (started_tr) {
        hfs_end_transaction(hfsmp);
        started_tr = false;
    }

    return error;
}

int
hfs_fork_release(struct cnode* cp, struct vnode *vp, bool bIsRsc, int* piErr)
{
    struct hfsmount *hfsmp = VTOHFS(vp);
    struct filefork *fp = NULL;
    struct filefork *altfp = NULL;
    int reclaim_cnode = 0;
    
    /*
     * Sync to disk any remaining data in the cnode/vnode.  This includes
     * a call to hfs_update if the cnode has outbound data.
     *
     * If C_NOEXISTS is set on the cnode, then there's nothing teardown needs to do
     * because the catalog entry for this cnode is already gone.
     */
    INVALIDATE_NODE(vp);
    
    if (!ISSET(cp->c_flag, C_NOEXISTS)) {
        *piErr = hfs_cnode_teardown(vp, 1);
        if (*piErr)
        {
            return 0;
        }
    }
    
    if (vp->sFSParams.vnfs_cnp)
    {
        if (vp->sFSParams.vnfs_cnp->cn_nameptr) {
            hfs_free(vp->sFSParams.vnfs_cnp->cn_nameptr);
            vp->sFSParams.vnfs_cnp->cn_nameptr = NULL;
        }
        hfs_free(vp->sFSParams.vnfs_cnp);
        vp->sFSParams.vnfs_cnp = NULL;
    }
    
    
    if (!bIsRsc) {
        fp = cp->c_datafork;
        altfp = cp->c_rsrcfork;
        
        cp->c_datafork = NULL;
        cp->c_vp = NULL;
    } else {
        fp = cp->c_rsrcfork;
        altfp = cp->c_datafork;
        
        cp->c_rsrcfork = NULL;
        cp->c_rsrc_vp = NULL;
    }
    
    /*
     * On the last fork, remove the cnode from its hash chain.
     */
    if (altfp == NULL) {
        /* If we can't remove it then the cnode must persist! */
        if (hfs_chashremove(hfsmp, cp) == 0)
            reclaim_cnode = 1;
        /*
         * Remove any directory hints
         */
        if (vnode_isdir(vp)) {
            hfs_reldirhints(cp, 0);
        }
        
        if(cp->c_flag & C_HARDLINK) {
            hfs_relorigins(cp);
        }
    }
    
    /* Release the file fork and related data */
    if (fp)
    {
        /* Dump cached symlink data */
        if (vnode_islnk(vp) && (fp->ff_symlinkptr != NULL)) {
            hfs_free(fp->ff_symlinkptr);
        }
        rl_remove_all(&fp->ff_invalidranges);
        hfs_free(fp);
    }
    
    return reclaim_cnode;
}


/*
 * Reclaim a cnode so that it can be used for other purposes.
 */
int
hfs_vnop_reclaim(struct vnode *vp)
{
    if (!vp) return EINVAL;

    struct cnode* cp = VTOC(vp);
    struct hfsmount *hfsmp = VTOHFS(vp);
    struct vnode *altvp = NULL;
    int reclaim_cnode = 0;
    int err = 0;

    /*
     * We don't take the truncate lock since by the time reclaim comes along,
     * all dirty pages have been synced and nobody should be competing
     * with us for this thread.
     */
    hfs_chash_mark_in_transit(hfsmp, cp);

    hfs_lock(cp, HFS_EXCLUSIVE_LOCK, HFS_LOCK_DEFAULT);
    lf_hfs_generic_buf_cache_LockBufCache();

    //In case we have other open lookups
    //We need to decrease the counter and exit
    if (cp->uOpenLookupRefCount > 1)
    {
        hfs_chash_lower_OpenLookupCounter(cp);
        hfs_chashwakeup(hfsmp, cp, H_ALLOC | H_TRANSIT);
        lf_hfs_generic_buf_cache_UnLockBufCache();
        hfs_unlock(cp);
        return err;
    }
    
    if (cp->uOpenLookupRefCount == 0) assert(0);
    
    hfs_chash_lower_OpenLookupCounter(cp);
    lf_hfs_generic_buf_cache_remove_vnode(vp);

    lf_hfs_generic_buf_cache_UnLockBufCache();
    
    /*
     * Find file fork for this vnode (if any)
     * Also check if another fork is active
     */
    if (cp->c_vp == vp) {
        
        reclaim_cnode = hfs_fork_release(cp, vp, false, &err);
        if (err) return err;
    
        if (!reclaim_cnode && cp->c_rsrc_vp != NULL)
        {
            altvp = cp->c_rsrc_vp;
            reclaim_cnode = hfs_fork_release(cp, altvp, true, &err);
            if (err) return err;
        }
    } else if (cp->c_rsrc_vp == vp) {
        reclaim_cnode = hfs_fork_release(cp, vp, true, &err);
        if (err) return err;
        
        if (!reclaim_cnode && cp->c_vp != NULL)
        {
            altvp = cp->c_vp;
            reclaim_cnode = hfs_fork_release(cp, altvp, false, &err);
            if (err) return err;
        }
    } else {
        LFHFS_LOG(LEVEL_ERROR, "hfs_vnop_reclaim: vp points to wrong cnode (vp=%p cp->c_vp=%p cp->c_rsrc_vp=%p)\n", vp, cp->c_vp, cp->c_rsrc_vp);
        hfs_assert(0);
    }

    /*
     * If there was only one active fork then we can release the cnode.
     */
    if (reclaim_cnode) {
        hfs_unlock(cp);
        hfs_chashwakeup(hfsmp, cp, H_ALLOC);
        hfs_reclaim_cnode(cp);
    }
    else
    {
        /*
         * cnode in use.  If it is a directory, it could have
         * no live forks. Just release the lock.
         */
        hfs_unlock(cp);
    }
    
    hfs_free(vp);
    if (altvp)
        hfs_free(altvp);
    
    vp = NULL;
    return (0);
}
