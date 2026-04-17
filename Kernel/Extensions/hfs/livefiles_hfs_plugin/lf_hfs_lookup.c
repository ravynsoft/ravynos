//
//  lf_hfs_lookup.c
//  livefiles_hfs
//
//  Created by Yakov Ben Zaken on 25/03/2018.
//

#include "lf_hfs.h"
#include "lf_hfs_lookup.h"
#include "lf_hfs_cnode.h"
#include "lf_hfs_vfsutils.h"
#include "lf_hfs_link.h"

static int
hfs_lookup(struct vnode *dvp, struct vnode **vpp, struct componentname *cnp, int *cnode_locked)
{
    struct cnode *dcp;    /* cnode for directory being searched */
    struct vnode *tvp;    /* target vnode */
    struct hfsmount *hfsmp;
    int flags;
    int nameiop;
    int retval = 0;
    struct cat_desc desc;
    struct cat_desc cndesc;
    struct cat_attr attr;
    struct cat_fork fork;
    int lockflags;
    int newvnode_flags = 0;
    
retry:
    newvnode_flags = 0;
    dcp = NULL;
    hfsmp = VTOHFS(dvp);
    *vpp = NULL;
    *cnode_locked = 0;
    tvp = NULL;
    nameiop = cnp->cn_nameiop;
    flags = cnp->cn_flags;
    bzero(&desc, sizeof(desc));

    if (hfs_lock(VTOC(dvp), HFS_EXCLUSIVE_LOCK, HFS_LOCK_DEFAULT) != 0) {
        retval = ENOENT;  /* The parent no longer exists ? */
        goto exit;
    }
    dcp = VTOC(dvp);

    /*
     * Need to understand if we need this check.. as we took exclusive lock..
     */
    if (dcp->c_flag & C_DIR_MODIFICATION){
        hfs_unlock(dcp);
        usleep( 1000 );
        goto retry;
    }

    /*
     * We shouldn't need to go to the catalog if there are no children.
     * However, in the face of a minor disk corruption where the valence of
     * the directory is off, we could infinite loop here if we return ENOENT
     * even though there are actually items in the directory.  (create will
     * see the ENOENT, try to create something, which will return with
     * EEXIST over and over again).  As a result, always check the catalog.
     */

    bzero(&cndesc, sizeof(cndesc));
    cndesc.cd_nameptr = (const u_int8_t *)cnp->cn_nameptr;
    cndesc.cd_namelen = cnp->cn_namelen;
    cndesc.cd_parentcnid = dcp->c_fileid;
    cndesc.cd_hint = dcp->c_childhint;

    lockflags = hfs_systemfile_lock(hfsmp, SFL_CATALOG, HFS_SHARED_LOCK);
    retval = cat_lookup(hfsmp, &cndesc, 0, &desc, &attr, &fork, NULL);
    hfs_systemfile_unlock(hfsmp, lockflags);

    if (retval == 0) {
        dcp->c_childhint = desc.cd_hint;
        /*
         * Note: We must drop the parent lock here before calling
         * hfs_getnewvnode (which takes the child lock).
         */
        hfs_unlock(dcp);
        dcp = NULL;

        /* Verify that the item just looked up isn't one of the hidden directories. */
        if (desc.cd_cnid == hfsmp->hfs_private_desc[FILE_HARDLINKS].cd_cnid ||
            desc.cd_cnid == hfsmp->hfs_private_desc[DIR_HARDLINKS].cd_cnid) {
            retval = ENOENT;
            goto exit;
        }
        goto found;
    }

    if (retval == HFS_ERESERVEDNAME) {
        /*
         * We found the name in the catalog, but it is unavailable
         * to us. The exact error to return to our caller depends
         * on the operation, and whether we've already reached the
         * last path component. In all cases, avoid a negative
         * cache entry, since someone else may be able to access
         * the name if their lookup is configured differently.
         */

        cnp->cn_flags &= ~MAKEENTRY;

        if (((flags & ISLASTCN) == 0) || ((nameiop == LOOKUP) || (nameiop == DELETE))) {
            /* A reserved name for a pure lookup is the same as the path not being present */
            retval = ENOENT;
        } else {
            /* A reserved name with intent to create must be rejected as impossible */
            retval = EEXIST;
        }
    }
    if (retval != ENOENT)
        goto exit;
    /*
     * This is a non-existing entry
     *
     * If creating, and at end of pathname and current
     * directory has not been removed, then can consider
     * allowing file to be created.
     */
    if ((nameiop == CREATE || nameiop == RENAME) &&
        (flags & ISLASTCN) &&
        !(ISSET(dcp->c_flag, C_DELETED | C_NOEXISTS))) {
        retval = EJUSTRETURN;
        goto exit;
    }
    
    goto exit;

found:
    if (flags & ISLASTCN) {
        switch(nameiop) {
            case DELETE:
                cnp->cn_flags &= ~MAKEENTRY;
                break;

            case RENAME:
                cnp->cn_flags &= ~MAKEENTRY;
                break;
            default:
                break;
        }
    }

    int type = (attr.ca_mode & S_IFMT);

    if (!(flags & ISLASTCN) && (type != S_IFDIR) && (type != S_IFLNK)) {
        retval = ENOTDIR;
        goto exit;
    }
    /* Don't cache directory hardlink names. */
    if (attr.ca_recflags & kHFSHasLinkChainMask) {
        cnp->cn_flags &= ~MAKEENTRY;
    }
    /* Names with composed chars are not cached. */
    if (cnp->cn_namelen != desc.cd_namelen)
        cnp->cn_flags &= ~MAKEENTRY;
    
    retval = hfs_getnewvnode(hfsmp, dvp, cnp, &desc, 0, &attr, &fork, &tvp, &newvnode_flags);
    
    if (retval) {
        /*
         * If this was a create/rename operation lookup, then by this point
         * we expected to see the item returned from hfs_getnewvnode above.
         * In the create case, it would probably eventually bubble out an EEXIST
         * because the item existed when we were trying to create it.  In the
         * rename case, it would let us know that we need to go ahead and
         * delete it as part of the rename.  However, if we hit the condition below
         * then it means that we found the element during cat_lookup above, but
         * it is now no longer there.  We simply behave as though we never found
         * the element at all and return EJUSTRETURN.
         */
        if ((retval == ENOENT) &&
            ((cnp->cn_nameiop == CREATE) || (cnp->cn_nameiop == RENAME)) &&
            (flags & ISLASTCN)) {
            retval = EJUSTRETURN;
        }

        /*
         * If this was a straight lookup operation, we may need to redrive the entire
         * lookup starting from cat_lookup if the element was deleted as the result of
         * a rename operation.  Since rename is supposed to guarantee atomicity, then
         * lookups cannot fail because the underlying element is deleted as a result of
         * the rename call -- either they returned the looked up element prior to rename
         * or return the newer element.  If we are in this region, then all we can do is add
         * workarounds to guarantee the latter case. The element has already been deleted, so
         * we just re-try the lookup to ensure the caller gets the most recent element.
         */
        if ((retval == ENOENT) && (cnp->cn_nameiop == LOOKUP) &&
            (newvnode_flags & (GNV_CHASH_RENAMED | GNV_CAT_DELETED))) {
            if (dcp) {
                hfs_unlock (dcp);
            }
            /* get rid of any name buffers that may have lingered from the cat_lookup call */
            cat_releasedesc (&desc);
            goto retry;
        }

        /* Also, re-drive the lookup if the item we looked up was a hardlink, and the number
         * or name of hardlinks has changed in the interim between the cat_lookup above, and
         * our call to hfs_getnewvnode.  hfs_getnewvnode will validate the cattr we passed it
         * against what is actually in the catalog after the cnode is created.  If there were
         * any issues, it will bubble out ERECYCLE, which we need to swallow and use as the
         * key to redrive as well.  We need to special case this below because in this case,
         * it needs to occur regardless of the type of lookup we're doing here.
         */
        if ((retval == ERECYCLE) && (newvnode_flags & GNV_CAT_ATTRCHANGED)) {
            if (dcp) {
                hfs_unlock (dcp);
            }
            /* get rid of any name buffers that may have lingered from the cat_lookup call */
            cat_releasedesc (&desc);
            goto retry;
        }

        /* skip to the error-handling code if we can't retry */
        goto exit;
    }

    /*
     * Save the origin info for file and directory hardlinks.  Directory hardlinks
     * need the origin for '..' lookups, and file hardlinks need it to ensure that
     * competing lookups do not cause us to vend different hardlinks than the ones requested.
     */
    if (ISSET(VTOC(tvp)->c_flag, C_HARDLINK))
        hfs_savelinkorigin(VTOC(tvp), VTOC(dvp)->c_fileid);

    *cnode_locked = 1;
    *vpp = tvp;

exit:
    if (dcp) {
        hfs_unlock(dcp);
    }
    cat_releasedesc(&desc);

    return (retval);
}

int
hfs_vnop_lookup(struct vnode *dvp, struct vnode **vpp, struct componentname *cnp)
{
    int error = 0;
    int cnode_locked = 0;
    *vpp = NULL;

    error = hfs_lookup(dvp, vpp, cnp, &cnode_locked);

    if (cnode_locked)
        hfs_unlock(VTOC(*vpp));
    
    return (error);
}
