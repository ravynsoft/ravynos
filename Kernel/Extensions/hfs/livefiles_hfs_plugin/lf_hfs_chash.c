/*  Copyright © 2017-2018 Apple Inc. All rights reserved.
 *
 *  lf_hfs_chash.c
 *  livefiles_hfs
 *
 *  Created by Or Haimovich on 18/3/18.
 */

#include "lf_hfs_chash.h"
#include "lf_hfs_cnode.h"
#include "lf_hfs_locks.h"
#include "lf_hfs_utils.h"
#include "lf_hfs_logger.h"
#include "lf_hfs_vfsutils.h"

#define DESIRED_VNODES (128)        /* number of vnodes desired */
#define CNODEHASH(hfsmp, inum) (&hfsmp->hfs_cnodehashtbl[(inum) & hfsmp->hfs_cnodehash])

void
hfs_chash_wait(struct hfsmount *hfsmp, struct cnode  *cp)
{
    SET(cp->c_hflag, H_WAITING);
    pthread_cond_wait(&cp->c_cacsh_cond, &hfsmp->hfs_chash_mutex);
}

static void
hfs_chash_broadcast_and_unlock(struct hfsmount *hfsmp, struct cnode  *cp)
{
    if (cp)
        pthread_cond_signal(&cp->c_cacsh_cond);
    hfs_chash_unlock(hfsmp);
}

static void
hfs_chash_wait_and_unlock(struct hfsmount *hfsmp, struct cnode  *cp)
{
    SET(cp->c_hflag, H_WAITING);
    pthread_cond_wait(&cp->c_cacsh_cond, &hfsmp->hfs_chash_mutex);
    hfs_chash_broadcast_and_unlock(hfsmp, cp);
}

void
hfs_chash_raise_OpenLookupCounter(struct cnode *cp)
{
    if (!cp || cp->uOpenLookupRefCount == UINT32_MAX)
    {
        LFHFS_LOG(LEVEL_ERROR,
                  "hfs_chash_raise_OpenLookupCounter:"
                  "cp[%p] is NULL or reached max Open Lookup Counter", cp);
        hfs_assert(0);
    }
    cp->uOpenLookupRefCount++;
}

void
hfs_chash_lower_OpenLookupCounter(struct cnode *cp)
{
    if (cp->uOpenLookupRefCount == 0)
    {
        LFHFS_LOG(LEVEL_ERROR, "hfs_chash_lower_OpenLookupCounter: reached min Open Lookup Counter \n");
        hfs_assert(0);
    }
    cp->uOpenLookupRefCount--;
}

/*
 * Initialize cnode hash table.
 */
void
hfs_chashinit()
{
}

void hfs_chash_lock(struct hfsmount *hfsmp)
{
    lf_lck_mtx_lock(&hfsmp->hfs_chash_mutex);
}

void hfs_chash_lock_spin(struct hfsmount *hfsmp)
{
    lf_lck_mtx_lock_spin(&hfsmp->hfs_chash_mutex);
}


void hfs_chash_unlock(struct hfsmount *hfsmp)
{
    lf_lck_mtx_unlock(&hfsmp->hfs_chash_mutex);
}

void
hfs_chashinit_finish(struct hfsmount *hfsmp)
{
    lf_lck_mtx_init(&hfsmp->hfs_chash_mutex);
    hfsmp->hfs_cnodehashtbl = hashinit(DESIRED_VNODES / 4, &hfsmp->hfs_cnodehash);
}

void
hfs_delete_chash(struct hfsmount *hfsmp)
{
    struct cnode  *cp;
    hfs_chash_lock_spin(hfsmp);
    
    for (ino_t inum = 0;  inum < (DESIRED_VNODES/4); inum++)
    {
        for (cp = CNODEHASH(hfsmp, inum)->lh_first; cp; cp = cp->c_hash.le_next) {
            LFHFS_LOG(LEVEL_ERROR, "hfs_delete_chash: Cnode for file [%s], cnid: [%d] with open count [%d] left in the cache \n", cp->c_desc.cd_nameptr, cp->c_desc.cd_cnid, cp->uOpenLookupRefCount);
        }
    }
    
        
    hfs_chash_unlock(hfsmp);
    lf_lck_mtx_destroy(&hfsmp->hfs_chash_mutex);
    hfs_free(hfsmp->hfs_cnodehashtbl);
}

/*
 * Use the device, fileid pair to find the incore cnode.
 * If no cnode if found one is created
 *
 * If it is in core, but locked, wait for it.
 *
 * If the cnode is C_DELETED, then return NULL since that
 * inum is no longer valid for lookups (open-unlinked file).
 *
 * If the cnode is C_DELETED but also marked C_RENAMED, then that means
 * the cnode was renamed over and a new entry exists in its place.  The caller
 * should re-drive the lookup to get the newer entry.  In that case, we'll still
 * return NULL for the cnode, but also return GNV_CHASH_RENAMED in the output flags
 * of this function to indicate the caller that they should re-drive.
 */
struct cnode*
hfs_chash_getcnode(struct hfsmount *hfsmp, ino_t inum, struct vnode **vpp, int wantrsrc, int skiplock, int *out_flags, int *hflags)
{
    struct cnode  *cp;
    struct cnode  *ncp = NULL;
    vnode_t       vp;

    /*
     * Go through the hash list
     * If a cnode is in the process of being cleaned out or being
     * allocated, wait for it to be finished and then try again.
     */
loop:
    hfs_chash_lock_spin(hfsmp);
loop_with_lock:
    for (cp = CNODEHASH(hfsmp, inum)->lh_first; cp; cp = cp->c_hash.le_next)
    {
        if (cp->c_fileid != inum)
        {
            continue;
        }
        /*
         * Wait if cnode is being created, attached to or reclaimed.
         */
        if (ISSET(cp->c_hflag, H_ALLOC | H_ATTACH | H_TRANSIT))
        {
            hfs_chash_wait(hfsmp, cp);
            goto loop_with_lock;
        }
        
        vp = wantrsrc ? cp->c_rsrc_vp : cp->c_vp;
        if (vp == NULL)
        {
            /*
             * The desired vnode isn't there so tag the cnode.
             */
            SET(cp->c_hflag, H_ATTACH);
            *hflags |= H_ATTACH;
        }

        if (ncp)
        {
            /*
             * someone else won the race to create
             * this cnode and add it to the hash
             * just dump our allocation
             */
            hfs_free(ncp);
            ncp = NULL;
        }

        if (!skiplock)
        {
            if (hfs_lock(cp, HFS_TRY_EXCLUSIVE_LOCK, HFS_LOCK_ALLOW_NOEXISTS))
            {
                SET(cp->c_hflag, H_WAITING);
                hfs_chash_broadcast_and_unlock(hfsmp, cp);
                usleep(100);
                goto loop;
            }
        }
        vp = wantrsrc ? cp->c_rsrc_vp : cp->c_vp;
        
        /*
         * Skip cnodes that are not in the name space anymore
         * we need to check with the cnode lock held because
         * we may have blocked acquiring the vnode ref or the
         * lock on the cnode which would allow the node to be
         * unlinked.
         *
         * Don't return a cnode in this case since the inum
         * is no longer valid for lookups.
         */
        if (((cp->c_flag & (C_NOEXISTS | C_DELETED)) && !wantrsrc) ||
            ((vp != NULL) &&
            ((cp->uOpenLookupRefCount == 0) ||
            (vp->uValidNodeMagic1 == VALID_NODE_BADMAGIC) ||
            (vp->uValidNodeMagic2 == VALID_NODE_BADMAGIC))))
        {
            int renamed = 0;
            if (cp->c_flag & C_RENAMED)
                renamed = 1;
            if (!skiplock)
            {
                hfs_unlock(cp);
            }
            
            if (vp != NULL)
            {
                vnode_rele(vp);
            }
            else
            {
                hfs_chashwakeup(hfsmp, cp, H_ATTACH);
                *hflags &= ~H_ATTACH;
            }
            
            pthread_cond_signal(&cp->c_cacsh_cond);
            vp = NULL;
            cp = NULL;
            if (renamed)
            {
                *out_flags = GNV_CHASH_RENAMED;
            }
        }
        
        if (cp) {
            hfs_chash_raise_OpenLookupCounter(cp);
        }

        hfs_chash_broadcast_and_unlock(hfsmp, cp);

        *vpp = vp;
        return (cp);
    }

    /*
     * Allocate a new cnode
     */
    if (skiplock && !wantrsrc)
    {
        LFHFS_LOG(LEVEL_ERROR, "hfs_chash_getcnode: should never get here when skiplock is set \n");
        hfs_assert(0);
    }

    if (ncp == NULL)
    {
        hfs_chash_unlock(hfsmp);
        ncp = hfs_mallocz(sizeof(struct cnode));
        if (ncp == NULL)
        {
            return ncp;
        }
        /*
         * since we dropped the chash lock,
         * we need to go back and re-verify
         * that this node hasn't come into
         * existence...
         */
        goto loop;
    }

    bzero(ncp, sizeof(*ncp));

    SET(ncp->c_hflag, H_ALLOC);
    *hflags |= H_ALLOC;
    ncp->c_fileid = (cnid_t) inum;
    TAILQ_INIT(&ncp->c_hintlist); /* make the list empty */
    TAILQ_INIT(&ncp->c_originlist);

    lf_lck_rw_init(&ncp->c_rwlock);
    lf_cond_init(&ncp->c_cacsh_cond);

    if (!skiplock)
    {
        (void) hfs_lock(ncp, HFS_EXCLUSIVE_LOCK, HFS_LOCK_DEFAULT);
    }

    /* Insert the new cnode with it's H_ALLOC flag set */
    LIST_INSERT_HEAD(CNODEHASH(hfsmp, inum), ncp, c_hash);
    hfs_chash_raise_OpenLookupCounter(ncp);
    hfs_chash_unlock(hfsmp);
    *vpp = NULL;
    return (ncp);
}

void
hfs_chashwakeup(struct hfsmount *hfsmp, struct cnode *cp, int hflags)
{
    hfs_chash_lock_spin(hfsmp);

    CLR(cp->c_hflag, hflags);

    if (ISSET(cp->c_hflag, H_WAITING)) {
        CLR(cp->c_hflag, H_WAITING);
    }
    
    hfs_chash_broadcast_and_unlock(hfsmp, cp);
}

/*
 * Remove a cnode from the hash table and wakeup any waiters.
 */
void
hfs_chash_abort(struct hfsmount *hfsmp, struct cnode *cp)
{
    hfs_chash_lock_spin(hfsmp);

    LIST_REMOVE(cp, c_hash);
    cp->c_hash.le_next = NULL;
    cp->c_hash.le_prev = NULL;

    CLR(cp->c_hflag, H_ATTACH | H_ALLOC);
    if (ISSET(cp->c_hflag, H_WAITING))
    {
        CLR(cp->c_hflag, H_WAITING);
    }
    hfs_chash_broadcast_and_unlock(hfsmp, cp);
}

/*
 * Use the device, inum pair to find the incore cnode.
 *
 * If it is in core, but locked, wait for it.
 */
struct vnode *
hfs_chash_getvnode(struct hfsmount *hfsmp, ino_t inum, int wantrsrc, int skiplock, int allow_deleted)
{
    struct cnode *cp;
    struct vnode *vp;

    /*
     * Go through the hash list
     * If a cnode is in the process of being cleaned out or being
     * allocated, wait for it to be finished and then try again.
     */
loop:
    hfs_chash_lock_spin(hfsmp);

    for (cp = CNODEHASH(hfsmp, inum)->lh_first; cp; cp = cp->c_hash.le_next) {
        if (cp->c_fileid != inum)
            continue;
        /* Wait if cnode is being created or reclaimed. */
        if (ISSET(cp->c_hflag, H_ALLOC | H_TRANSIT | H_ATTACH)) {
            SET(cp->c_hflag, H_WAITING);
            hfs_chash_wait_and_unlock(hfsmp,cp);
            goto loop;
        }
        /* Obtain the desired vnode. */
        vp = wantrsrc ? cp->c_rsrc_vp : cp->c_vp;
        if (vp == NULL)
        {
            goto exit;
        }
        
        if (!skiplock)
        {
            if (hfs_lock(cp, HFS_TRY_EXCLUSIVE_LOCK, HFS_LOCK_ALLOW_NOEXISTS))
            {
                SET(cp->c_hflag, H_WAITING);
                hfs_chash_broadcast_and_unlock(hfsmp, cp);
                usleep(100);
                goto loop;
            }
        }
        vp = wantrsrc ? cp->c_rsrc_vp : cp->c_vp;

        /*
         * Skip cnodes that are not in the name space anymore
         * we need to check with the cnode lock held because
         * we may have blocked acquiring the vnode ref or the
         * lock on the cnode which would allow the node to be
         * unlinked
         */
        if (!allow_deleted) {
            if (cp->c_flag & (C_NOEXISTS | C_DELETED)) {
                if (!skiplock) hfs_unlock(cp);
                goto exit;
            }
        }
        
        hfs_chash_raise_OpenLookupCounter(cp);
        hfs_chash_broadcast_and_unlock(hfsmp, cp);
        return (vp);
    }

exit:
    hfs_chash_unlock(hfsmp);
    return (NULL);
}

int
hfs_chash_snoop(struct hfsmount *hfsmp, ino_t inum, int existence_only,
                int (*callout)(const cnode_t *cp, void *), void * arg)
{
    struct cnode *cp;
    int result = ENOENT;

    /*
     * Go through the hash list
     * If a cnode is in the process of being cleaned out or being
     * allocated, wait for it to be finished and then try again.
     */
    hfs_chash_lock(hfsmp);

    for (cp = CNODEHASH(hfsmp, inum)->lh_first; cp; cp = cp->c_hash.le_next) {
        if (cp->c_fileid != inum)
            continue;

        /*
         * Under normal circumstances, we would want to return ENOENT if a cnode is in
         * the hash and it is marked C_NOEXISTS or C_DELETED.  However, if the CNID
         * namespace has wrapped around, then we have the possibility of collisions.
         * In that case, we may use this function to validate whether or not we
         * should trust the nextCNID value in the hfs mount point.
         *
         * If we didn't do this, then it would be possible for a cnode that is no longer backed
         * by anything on-disk (C_NOEXISTS) to still exist in the hash along with its
         * vnode.  The cat_create routine could then create a new entry in the catalog
         * re-using that CNID.  Then subsequent hfs_getnewvnode calls will repeatedly fail
         * trying to look it up/validate it because it is marked C_NOEXISTS.  So we want
         * to prevent that from happening as much as possible.
         */
        if (existence_only) {
            result = 0;
            break;
        }

        /* Skip cnodes that have been removed from the catalog */
        if (cp->c_flag & (C_NOEXISTS | C_DELETED)) {
            result = EACCES;
            break;
        }

        /* Skip cnodes being created or reclaimed. */
        if (!ISSET(cp->c_hflag, H_ALLOC | H_TRANSIT | H_ATTACH)) {
            result = callout(cp, arg);
        }
        break;
    }
    hfs_chash_unlock(hfsmp);

    return (result);
}

/* Search a cnode in the hash.  This function does not return cnode which
 * are getting created, destroyed or in transition.  Note that this function
 * does not acquire the cnode hash mutex, and expects the caller to acquire it.
 * On success, returns pointer to the cnode found.  On failure, returns NULL.
 */
static
struct cnode *
hfs_chash_search_cnid(struct hfsmount *hfsmp, cnid_t cnid)
{
    struct cnode *cp;

    for (cp = CNODEHASH(hfsmp, cnid)->lh_first; cp; cp = cp->c_hash.le_next) {
        if (cp->c_fileid == cnid) {
            break;
        }
    }

    /* If cnode is being created or reclaimed, return error. */
    if (cp && ISSET(cp->c_hflag, H_ALLOC | H_TRANSIT | H_ATTACH)) {
        cp = NULL;
    }

    return cp;
}

/* Search a cnode corresponding to given device and ID in the hash.  If the
 * found cnode has kHFSHasChildLinkBit cleared, set it.  If the cnode is not
 * found, no new cnode is created and error is returned.
 *
 * Return values -
 *    -1 : The cnode was not found.
 *      0 : The cnode was found, and the kHFSHasChildLinkBit was already set.
 *     1 : The cnode was found, the kHFSHasChildLinkBit was not set, and the
 *         function had to set that bit.
 */
int
hfs_chash_set_childlinkbit(struct hfsmount *hfsmp, cnid_t cnid)
{
    int retval = -1;
    struct cnode *cp;

    hfs_chash_lock_spin(hfsmp);

    cp = hfs_chash_search_cnid(hfsmp, cnid);
    if (cp) {
        if (cp->c_attr.ca_recflags & kHFSHasChildLinkMask) {
            retval = 0;
        } else {
            cp->c_attr.ca_recflags |= kHFSHasChildLinkMask;
            retval = 1;
        }
    }
    hfs_chash_broadcast_and_unlock(hfsmp, cp);

    return retval;
}

/*
 * Remove a cnode from the hash table.
 * Need to lock cache from caller
 */
int
hfs_chashremove(struct hfsmount *hfsmp, struct cnode *cp)
{
    hfs_chash_lock_spin(hfsmp);

    /* Check if a vnode is getting attached */
    if (ISSET(cp->c_hflag, H_ATTACH)) {
        hfs_chash_broadcast_and_unlock(hfsmp, cp);
        return (EBUSY);
    }
    if (cp->c_hash.le_next || cp->c_hash.le_prev) {
        LIST_REMOVE(cp, c_hash);
        cp->c_hash.le_next = NULL;
        cp->c_hash.le_prev = NULL;
    }

    hfs_chash_broadcast_and_unlock(hfsmp, cp);

    return (0);
}

/*
 * mark a cnode as in transition
 */
void
hfs_chash_mark_in_transit(struct hfsmount *hfsmp, struct cnode *cp)
{
    hfs_chash_lock_spin(hfsmp);
    
    SET(cp->c_hflag, H_TRANSIT);
    
    hfs_chash_broadcast_and_unlock(hfsmp, cp);
}
