/*  Copyright © 2017-2018 Apple Inc. All rights reserved.
 *
 *  lf_hfs_xattr.c
 *  livefiles_hfs
 *
 *  Created by Or Haimovich on 28/3/18.
 */

#include <sys/xattr.h>
#include <sys/acl.h>
#include <sys/kauth.h>
#include "lf_hfs_xattr.h"
#include "lf_hfs.h"
#include "lf_hfs_vnops.h"
#include "lf_hfs_raw_read_write.h"
#include "lf_hfs_btrees_io.h"
#include "lf_hfs_btrees_internal.h"
#include "lf_hfs_vfsutils.h"
#include "lf_hfs_sbunicode.h"
#include "lf_hfs_endian.h"
#include "lf_hfs_logger.h"
#include "lf_hfs_utils.h"

#define  ATTRIBUTE_FILE_NODE_SIZE   8192

//#define HFS_XATTR_VERBOSE           1

/* State information for the listattr_callback callback function. */
struct listattr_callback_state {
    u_int32_t   fileID;
    int         result;
    void        *buf;
    size_t      bufsize;
    size_t      size;
};

static u_int32_t emptyfinfo[8] = {0};


static int hfs_zero_hidden_fields (struct cnode *cp, u_int8_t *finderinfo);

const char hfs_attrdatafilename[] = "Attribute Data";

static int  listattr_callback(const HFSPlusAttrKey *key, const HFSPlusAttrData *data,
                              struct listattr_callback_state *state);

static int remove_attribute_records(struct hfsmount *hfsmp, BTreeIterator * iterator);

static int  getnodecount(struct hfsmount *hfsmp, size_t nodesize);

static size_t  getmaxinlineattrsize(struct vnode * attrvp);

static int  read_attr_data(struct hfsmount *hfsmp, void * buf, size_t datasize, HFSPlusExtentDescriptor *extents);

static int  write_attr_data(struct hfsmount *hfsmp, void * buf, size_t datasize, HFSPlusExtentDescriptor *extents);

static int  alloc_attr_blks(struct hfsmount *hfsmp, size_t attrsize, size_t extentbufsize, HFSPlusExtentDescriptor *extents, int *blocks);

static void  free_attr_blks(struct hfsmount *hfsmp, int blkcnt, HFSPlusExtentDescriptor *extents);

static int  has_overflow_extents(HFSPlusForkData *forkdata);

static int count_extent_blocks(int maxblks, HFSPlusExtentRecord extents);


/* Zero out the date added field for the specified cnode */
static int hfs_zero_hidden_fields (struct cnode *cp, u_int8_t *finderinfo)
{
    u_int8_t *finfo = finderinfo;

    /* Advance finfo by 16 bytes to the 2nd half of the finderinfo */
    finfo = finfo + 16;

    if (S_ISREG(cp->c_attr.ca_mode) || S_ISLNK(cp->c_attr.ca_mode)) {
        struct FndrExtendedFileInfo *extinfo = (struct FndrExtendedFileInfo *)finfo;
        extinfo->document_id = 0;
        extinfo->date_added = 0;
        extinfo->write_gen_counter = 0;
    } else if (S_ISDIR(cp->c_attr.ca_mode)) {
        struct FndrExtendedDirInfo *extinfo = (struct FndrExtendedDirInfo *)finfo;
        extinfo->document_id = 0;
        extinfo->date_added = 0;
        extinfo->write_gen_counter = 0;
    } else {
        /* Return an error */
        return -1;
    }
    return 0;
}

/*
 * Retrieve the data of an extended attribute.
 */
int
hfs_vnop_getxattr(vnode_t vp, const char *attr_name, void *buf, size_t bufsize, size_t *actual_size)
{
    struct cnode *cp;
    struct hfsmount *hfsmp;
    int result;

    if (attr_name == NULL || attr_name[0] == '\0') {
        return (EINVAL);  /* invalid name */
    }
    if (strlen(attr_name) > XATTR_MAXNAMELEN) {
        return (ENAMETOOLONG);
    }
    if (actual_size == NULL) {
        return (EINVAL);
    }
    if (VNODE_IS_RSRC(vp)) {
        return (EPERM);
    }

    cp = VTOC(vp);

    /* Get the Finder Info. */
    if (strcmp(attr_name, XATTR_FINDERINFO_NAME) == 0) {
        u_int8_t finderinfo[32];
        size_t attrsize = 32;

        if ((result = hfs_lock(cp, HFS_SHARED_LOCK, HFS_LOCK_DEFAULT))) {
            return (result);
        }
        /* Make a copy since we may not export all of it. */
        bcopy(cp->c_finderinfo, finderinfo, sizeof(finderinfo));
        hfs_unlock(cp);

        /* Zero out the date added field in the local copy */
        hfs_zero_hidden_fields (cp, finderinfo);

        /* Don't expose a symlink's private type/creator. */
        if (vnode_islnk(vp)) {
            struct FndrFileInfo *fip;

            fip = (struct FndrFileInfo *)&finderinfo;
            fip->fdType = 0;
            fip->fdCreator = 0;
        }
        /* If Finder Info is empty then it doesn't exist. */
        if (bcmp(finderinfo, emptyfinfo, sizeof(emptyfinfo)) == 0) {
            return (ENOATTR);
        }
        *actual_size = attrsize;

        if (buf == NULL) {
            return (0);
        }
        if (bufsize < attrsize)
            return (ERANGE);

        memcpy(buf, (caddr_t)&finderinfo, attrsize);
        return (0);
    }

    /* Read the Resource Fork. */
    if (strcmp(attr_name, XATTR_RESOURCEFORK_NAME) == 0) {
        return (ENOATTR);
    }

    hfsmp = VTOHFS(vp);
    if ((result = hfs_lock(cp, HFS_SHARED_LOCK, HFS_LOCK_DEFAULT))) {
        return (result);
    }

    /* Check for non-rsrc, non-finderinfo EAs - getxattr_internal */

    struct filefork *btfile;
    BTreeIterator * iterator = NULL;
    size_t attrsize = 0;
    HFSPlusAttrRecord *recp = NULL;
    FSBufferDescriptor btdata;
    int lockflags = 0;
    u_int16_t datasize = 0;
    u_int32_t target_id = 0;

    if (cp) {
        target_id = cp->c_fileid;
    } else {
        target_id = kHFSRootParentID;
    }

    /* Bail if we don't have an EA B-Tree. */
    if ((hfsmp->hfs_attribute_vp == NULL) ||
        ((cp) &&  (cp->c_attr.ca_recflags & kHFSHasAttributesMask) == 0)) {
        result = ENOATTR;
        goto exit;
    }

    /* Initialize the B-Tree iterator for searching for the proper EA */
    btfile = VTOF(hfsmp->hfs_attribute_vp);

    iterator = hfs_mallocz(sizeof(*iterator));

    /* Allocate memory for reading in the attribute record.  This buffer is
     * big enough to read in all types of attribute records.  It is not big
     * enough to read inline attribute data which is read in later.
     */
    recp = hfs_malloc(sizeof(HFSPlusAttrRecord));
    btdata.bufferAddress = recp;
    btdata.itemSize = sizeof(HFSPlusAttrRecord);
    btdata.itemCount = 1;

    result = hfs_buildattrkey(target_id, attr_name, (HFSPlusAttrKey *)&iterator->key);
    if (result) {
        goto exit;
    }

    /* Lookup the attribute in the Attribute B-Tree */
    lockflags = hfs_systemfile_lock(hfsmp, SFL_ATTRIBUTE, HFS_SHARED_LOCK);
    result = BTSearchRecord(btfile, iterator, &btdata, &datasize, NULL);
    hfs_systemfile_unlock(hfsmp, lockflags);

    if (result) {
        if (result == btNotFound) {
            result = ENOATTR;
        }
        goto exit;
    }

    /*
     * Operate differently if we have inline EAs that can fit in the attribute B-Tree or if
     * we have extent based EAs.
     */
    switch (recp->recordType) {

            /* Attribute fits in the Attribute B-Tree */
        case kHFSPlusAttrInlineData: {
            /*
             * Sanity check record size. It's not required to have any
             * user data, so the minimum size is 2 bytes less that the
             * size of HFSPlusAttrData (since HFSPlusAttrData struct
             * has 2 bytes set aside for attribute data).
             */
            if (datasize < (sizeof(HFSPlusAttrData) - 2)) {
                LFHFS_LOG(LEVEL_DEBUG, "hfs_getxattr: vol=%s %d,%s invalid record size %d (expecting %lu)\n",
                          hfsmp->vcbVN, target_id, attr_name, datasize, sizeof(HFSPlusAttrData));
                result = ENOATTR;
                break;
            }
            *actual_size = recp->attrData.attrSize;
            if (buf && recp->attrData.attrSize != 0) {
                if (*actual_size > bufsize) {
                    /* User provided buffer is not large enough for the xattr data */
                    result = ERANGE;
                } else {
                    /* Previous BTreeSearchRecord() read in only the attribute record,
                     * and not the attribute data.  Now allocate enough memory for
                     * both attribute record and data, and read the attribute record again.
                     */
                    attrsize = sizeof(HFSPlusAttrData) - 2 + recp->attrData.attrSize;
                    hfs_free(recp);
                    recp = hfs_malloc(attrsize);

                    btdata.bufferAddress = recp;
                    btdata.itemSize = attrsize;
                    btdata.itemCount = 1;

                    bzero(iterator, sizeof(*iterator));
                    result = hfs_buildattrkey(target_id, attr_name, (HFSPlusAttrKey *)&iterator->key);
                    if (result) {
                        goto exit;
                    }

                    /* Lookup the attribute record and inline data */
                    lockflags = hfs_systemfile_lock(hfsmp, SFL_ATTRIBUTE, HFS_SHARED_LOCK);
                    result = BTSearchRecord(btfile, iterator, &btdata, &datasize, NULL);
                    hfs_systemfile_unlock(hfsmp, lockflags);
                    if (result) {
                        if (result == btNotFound) {
                            result = ENOATTR;
                        }
                        goto exit;
                    }

                    /* Copy-out the attribute data to the user buffer */
                    *actual_size = recp->attrData.attrSize;
                    memcpy(buf, (caddr_t) &recp->attrData.attrData, recp->attrData.attrSize);
                }
            }
            break;
        }

            /* Extent-Based EAs */
        case kHFSPlusAttrForkData: {
            if (datasize < sizeof(HFSPlusAttrForkData)) {
                LFHFS_LOG(LEVEL_DEBUG, "hfs_getxattr: vol=%s %d,%s invalid record size %d (expecting %lu)\n",
                          hfsmp->vcbVN, target_id, attr_name, datasize, sizeof(HFSPlusAttrForkData));
                result = ENOATTR;
                break;
            }
            *actual_size = recp->forkData.theFork.logicalSize;
            if (buf == NULL) {
                break;
            }
            if (*actual_size > bufsize) {
                result = ERANGE;
                break;
            }
            /* Process overflow extents if necessary. */
            if (has_overflow_extents(&recp->forkData.theFork)) {
                HFSPlusExtentDescriptor *extentbuf;
                HFSPlusExtentDescriptor *extentptr;
                size_t extentbufsize;
                u_int32_t totalblocks;
                u_int32_t blkcnt;
                u_int64_t attrlen;

                totalblocks = recp->forkData.theFork.totalBlocks;
                /* Ignore bogus block counts. */
                if (totalblocks > howmany(HFS_XATTR_MAXSIZE, hfsmp->blockSize)) {
                    result = ERANGE;
                    break;
                }
                attrlen = recp->forkData.theFork.logicalSize;

                /* Get a buffer to hold the worst case amount of extents. */
                extentbufsize = totalblocks * sizeof(HFSPlusExtentDescriptor);
                extentbufsize = roundup(extentbufsize, sizeof(HFSPlusExtentRecord));
                extentbuf = hfs_mallocz(extentbufsize);
                extentptr = extentbuf;

                /* Grab the first 8 extents. */
                bcopy(&recp->forkData.theFork.extents[0], extentptr, sizeof(HFSPlusExtentRecord));
                extentptr += kHFSPlusExtentDensity;
                blkcnt = count_extent_blocks(totalblocks, recp->forkData.theFork.extents);

                /* Now lookup the overflow extents. */
                lockflags = hfs_systemfile_lock(hfsmp, SFL_ATTRIBUTE, HFS_SHARED_LOCK);
                while (blkcnt < totalblocks) {
                    ((HFSPlusAttrKey *)&iterator->key)->startBlock = blkcnt;
                    result = BTSearchRecord(btfile, iterator, &btdata, &datasize, NULL);
                    if (result ||
                        (recp->recordType != kHFSPlusAttrExtents) ||
                        (datasize < sizeof(HFSPlusAttrExtents))) {
                        LFHFS_LOG(LEVEL_DEBUG, "hfs_getxattr: %s missing extents, only %d blks of %d found\n",
                                  attr_name, blkcnt, totalblocks);
                        break;   /* break from while */
                    }
                    /* Grab the next 8 extents. */
                    bcopy(&recp->overflowExtents.extents[0], extentptr, sizeof(HFSPlusExtentRecord));
                    extentptr += kHFSPlusExtentDensity;
                    blkcnt += count_extent_blocks(totalblocks, recp->overflowExtents.extents);
                }

                /* Release Attr B-Tree lock */
                hfs_systemfile_unlock(hfsmp, lockflags);

                if (blkcnt < totalblocks) {
                    result = ENOATTR;
                } else {
                    result = read_attr_data(hfsmp, buf, attrlen, extentbuf);
                }
                hfs_free(extentbuf);

            } else { /* No overflow extents. */
                result = read_attr_data(hfsmp, buf, recp->forkData.theFork.logicalSize, recp->forkData.theFork.extents);
            }
            break;
        }

        default:
            /* We only support inline EAs.  Default to ENOATTR for anything else */
            result = ENOATTR;
            break;
    }

exit:
    hfs_free(iterator);
    hfs_free(recp);
    hfs_unlock(cp);

    return MacToVFSError(result);
}

/*
 * Set the data of an extended attribute.
 */
int
hfs_vnop_setxattr(vnode_t vp, const char *attr_name, const void *buf, size_t bufsize, UVFSXattrHow option)
{
    struct cnode *cp = NULL;
    struct hfsmount *hfsmp;
    size_t attrsize;
    int result;

    if (attr_name == NULL || attr_name[0] == '\0') {
        return (EINVAL);  /* invalid name */
    }
    if (strlen(attr_name) > XATTR_MAXNAMELEN) {
        return (ENAMETOOLONG);
    }
    if (buf == NULL) {
        return (EINVAL);
    }
    if (VNODE_IS_RSRC(vp)) {
        return (EPERM);
    }

    hfsmp = VTOHFS(vp);

    /* Set the Finder Info. */
    if (strcmp(attr_name, XATTR_FINDERINFO_NAME) == 0) {
        union {
            uint8_t data[32];
            char cdata[32];
            struct FndrFileInfo info;
        } fi;
        void * finderinfo_start;
        u_int8_t *finfo = NULL;
        u_int16_t fdFlags;
        u_int32_t dateadded = 0;
        u_int32_t write_gen_counter = 0;
        u_int32_t document_id = 0;

        attrsize = sizeof(VTOC(vp)->c_finderinfo);

        if (bufsize != attrsize) {
            return (ERANGE);
        }
        /* Grab the new Finder Info data. */
        memcpy(fi.cdata, buf, attrsize);

        if ((result = hfs_lock(VTOC(vp), HFS_EXCLUSIVE_LOCK, HFS_LOCK_DEFAULT))) {
            return (result);
        }
        cp = VTOC(vp);

        /* Symlink's don't have an external type/creator. */
        if (vnode_islnk(vp)) {
            /* Skip over type/creator fields. */
            finderinfo_start = &cp->c_finderinfo[8];
            attrsize -= 8;
        } else {
            finderinfo_start = &cp->c_finderinfo[0];
            /*
             * Don't allow the external setting of
             * file type to kHardLinkFileType.
             */
            if (fi.info.fdType == SWAP_BE32(kHardLinkFileType)) {
                hfs_unlock(cp);
                return (EPERM);
            }
        }

        /* Grab the current date added from the cnode */
        dateadded = hfs_get_dateadded (cp);
        if (S_ISREG(cp->c_attr.ca_mode) || S_ISLNK(cp->c_attr.ca_mode)) {
            struct FndrExtendedFileInfo *extinfo = (struct FndrExtendedFileInfo *)((u_int8_t*)cp->c_finderinfo + 16);
            /*
             * Grab generation counter directly from the cnode
             * instead of calling hfs_get_gencount(), because
             * for zero generation count values hfs_get_gencount()
             * lies and bumps it up to one.
             */
            write_gen_counter = extinfo->write_gen_counter;
            document_id = extinfo->document_id;
        } else if (S_ISDIR(cp->c_attr.ca_mode)) {
            struct FndrExtendedDirInfo *extinfo = (struct FndrExtendedDirInfo *)((u_int8_t*)cp->c_finderinfo + 16);
            write_gen_counter = extinfo->write_gen_counter;
            document_id = extinfo->document_id;
        }

        /*
         * Zero out the finder info's reserved fields like date added,
         * generation counter, and document id to ignore user's attempts
         * to set it
         */
        hfs_zero_hidden_fields(cp, fi.data);

        if (bcmp(finderinfo_start, emptyfinfo, attrsize)) {
            /* attr exists and "create" was specified. */
            if (option == UVFSXattrHowCreate) {
                hfs_unlock(cp);
                return (EEXIST);
            }
        } else { /* empty */
            /* attr doesn't exists and "replace" was specified. */
            if (option == UVFSXattrHowReplace) {
                hfs_unlock(cp);
                return (ENOATTR);
            }
        }

        /*
         * Now restore the date added and other reserved fields to the finderinfo to
         * be written out.  Advance to the 2nd half of the finderinfo to write them
         * out into the buffer.
         *
         * Make sure to endian swap the date added back into big endian.  When we used
         * hfs_get_dateadded above to retrieve it, it swapped into local endianness
         * for us.  But now that we're writing it out, put it back into big endian.
         */
        finfo = &fi.data[16];
        if (S_ISREG(cp->c_attr.ca_mode) || S_ISLNK(cp->c_attr.ca_mode)) {
            struct FndrExtendedFileInfo *extinfo = (struct FndrExtendedFileInfo *)finfo;
            extinfo->date_added = OSSwapHostToBigInt32(dateadded);
            extinfo->write_gen_counter = write_gen_counter;
            extinfo->document_id = document_id;
        } else if (S_ISDIR(cp->c_attr.ca_mode)) {
            struct FndrExtendedDirInfo *extinfo = (struct FndrExtendedDirInfo *)finfo;
            extinfo->date_added = OSSwapHostToBigInt32(dateadded);
            extinfo->write_gen_counter = write_gen_counter;
            extinfo->document_id = document_id;
        }

        /* Set the cnode's Finder Info. */
        if (attrsize == sizeof(cp->c_finderinfo)) {
            bcopy(&fi.data[0], finderinfo_start, attrsize);
        } else {
            bcopy(&fi.data[8], finderinfo_start, attrsize);
        }

        /* Updating finderInfo updates change time and modified time */
        cp->c_touch_chgtime = TRUE;
        cp->c_flag |= C_MODIFIED;

        /*
         * Mirror the invisible bit to the UF_HIDDEN flag.
         *
         * The fdFlags for files and frFlags for folders are both 8 bytes
         * into the userInfo (the first 16 bytes of the Finder Info).  They
         * are both 16-bit fields.
         */
        fdFlags = *((u_int16_t *) &cp->c_finderinfo[8]);
        if (fdFlags & OSSwapHostToBigConstInt16(kFinderInvisibleMask)) {
            cp->c_bsdflags |= UF_HIDDEN;
        } else {
            cp->c_bsdflags &= ~UF_HIDDEN;
        }

        result = hfs_update(vp, 0);

        hfs_unlock(cp);
        return (result);
    }

    /* Write the Resource Fork. */
    if (strcmp(attr_name, XATTR_RESOURCEFORK_NAME) == 0) {
        return (ENOTSUP);
    }

    attrsize = bufsize;

    result = hfs_lock(VTOC(vp), HFS_EXCLUSIVE_LOCK, HFS_LOCK_DEFAULT);
    if (result) {
        goto exit;
    }
    cp = VTOC(vp);

    /*
     * If we're trying to set a non-finderinfo, non-resourcefork EA, then
     * call the breakout function - hfs_setxattr_internal.
     */
    int started_transaction = 0;
    BTreeIterator * iterator = NULL;
    struct filefork *btfile = NULL;
    FSBufferDescriptor btdata;
    HFSPlusAttrRecord attrdata;  /* 90 bytes */
    HFSPlusAttrRecord *recp = NULL;
    HFSPlusExtentDescriptor *extentptr = NULL;
    size_t extentbufsize = 0;
    int lockflags = 0;
    int exists = 0;
    int allocatedblks = 0;
    u_int32_t target_id;

    if (cp) {
        target_id = cp->c_fileid;
    } else {
        target_id = kHFSRootParentID;
    }

    /* Start a transaction for our changes. */
    if (hfs_start_transaction(hfsmp) != 0) {
        result = EINVAL;
        goto exit;
    }
    started_transaction = 1;

    /*
     * Once we started the transaction, nobody can compete
     * with us, so make sure this file is still there.
     */
    if ((cp) && (cp->c_flag & C_NOEXISTS)) {
        result = ENOENT;
        goto exit;
    }

    /*
     * If there isn't an attributes b-tree then create one.
     */
    if (hfsmp->hfs_attribute_vp == NULL) {
        result = hfs_create_attr_btree(hfsmp, ATTRIBUTE_FILE_NODE_SIZE,
                                       getnodecount(hfsmp, ATTRIBUTE_FILE_NODE_SIZE));
        if (result) {
            goto exit;
        }
    }
    if (hfsmp->hfs_max_inline_attrsize == 0) {
        hfsmp->hfs_max_inline_attrsize = getmaxinlineattrsize(hfsmp->hfs_attribute_vp);
    }

    lockflags = hfs_systemfile_lock(hfsmp, SFL_ATTRIBUTE, HFS_EXCLUSIVE_LOCK);

    /* Build the b-tree key. */
    iterator = hfs_mallocz(sizeof(*iterator));
    result = hfs_buildattrkey(target_id, attr_name, (HFSPlusAttrKey *)&iterator->key);
    if (result) {
        goto exit_lock;
    }

    /* Preflight for replace/create semantics. */
    btfile = VTOF(hfsmp->hfs_attribute_vp);
    btdata.bufferAddress = &attrdata;
    btdata.itemSize = sizeof(attrdata);
    btdata.itemCount = 1;
    exists = BTSearchRecord(btfile, iterator, &btdata, NULL, NULL) == 0;

    /* Replace requires that the attribute already exists. */
    if ((option == UVFSXattrHowReplace) && !exists) {
        result = ENOATTR;
        goto exit_lock;
    }
    /* Create requires that the attribute doesn't exist. */
    if ((option == UVFSXattrHowCreate) && exists) {
        result = EEXIST;
        goto exit_lock;
    }

    /* Enforce an upper limit. */
    if (attrsize > HFS_XATTR_MAXSIZE) {
        result = E2BIG;
        goto exit_lock;
    }

    /* If it won't fit inline then use extent-based attributes. */
    if (attrsize > hfsmp->hfs_max_inline_attrsize) {
        int blkcnt;
        int extentblks;
        u_int32_t *keystartblk;
        int i;

        /* Get some blocks. */
        blkcnt = (int)howmany(attrsize, hfsmp->blockSize);
        extentbufsize = blkcnt * sizeof(HFSPlusExtentDescriptor);
        extentbufsize = roundup(extentbufsize, sizeof(HFSPlusExtentRecord));
        extentptr = hfs_mallocz(extentbufsize);
        result = alloc_attr_blks(hfsmp, attrsize, extentbufsize, extentptr, &allocatedblks);
        if (result) {
            allocatedblks = 0;
            goto exit_lock;  /* no more space */
        }
        /* Copy data into the blocks. */
        result = write_attr_data(hfsmp, (void*)buf, attrsize, extentptr);
        if (result) {
            if (vp) {
                LFHFS_LOG(LEVEL_DEBUG, "hfs_setxattr: write_attr_data vol=%s err (%d) :%s\n",
                          hfsmp->vcbVN, result, attr_name);
            }
            goto exit_lock;
        }

        /* Now remove any previous attribute. */
        if (exists) {
            result = remove_attribute_records(hfsmp, iterator);
            if (result) {
                if (vp) {
                    LFHFS_LOG(LEVEL_DEBUG, "hfs_setxattr: remove_attribute_records vol=%s err (%d) %s:%s\n",
                              hfsmp->vcbVN, result, "", attr_name);
                }
                goto exit_lock;
            }
        }
        /* Create attribute fork data record. */
        recp = hfs_malloc(sizeof(HFSPlusAttrRecord));

        btdata.bufferAddress = recp;
        btdata.itemCount = 1;
        btdata.itemSize = sizeof(HFSPlusAttrForkData);

        recp->recordType = kHFSPlusAttrForkData;
        recp->forkData.reserved = 0;
        recp->forkData.theFork.logicalSize = attrsize;
        recp->forkData.theFork.clumpSize = 0;
        recp->forkData.theFork.totalBlocks = blkcnt;
        bcopy(extentptr, recp->forkData.theFork.extents, sizeof(HFSPlusExtentRecord));

        (void) hfs_buildattrkey(target_id, attr_name, (HFSPlusAttrKey *)&iterator->key);

        result = BTInsertRecord(btfile, iterator, &btdata, btdata.itemSize);
        if (result) {
            LFHFS_LOG(LEVEL_DEBUG, "hfs_setxattr: BTInsertRecord(): vol=%s %d,%s err=%d\n",
                    hfsmp->vcbVN, target_id, attr_name, result);
            goto exit_lock;
        }
        extentblks = count_extent_blocks(blkcnt, recp->forkData.theFork.extents);
        blkcnt -= extentblks;
        keystartblk = &((HFSPlusAttrKey *)&iterator->key)->startBlock;
        i = 0;

        /* Create overflow extents as needed. */
        while (blkcnt > 0) {
            /* Initialize the key and record. */
            *keystartblk += (u_int32_t)extentblks;
            btdata.itemSize = sizeof(HFSPlusAttrExtents);
            recp->recordType = kHFSPlusAttrExtents;
            recp->overflowExtents.reserved = 0;

            /* Copy the next set of extents. */
            i += kHFSPlusExtentDensity;
            bcopy(&extentptr[i], recp->overflowExtents.extents, sizeof(HFSPlusExtentRecord));

            result = BTInsertRecord(btfile, iterator, &btdata, btdata.itemSize);
            if (result) {
                LFHFS_LOG(LEVEL_DEBUG, "hfs_setxattr: BTInsertRecord() overflow: vol=%s %d,%s err=%d\n",
                          hfsmp->vcbVN, target_id, attr_name, result);
                goto exit_lock;
            }
            extentblks = count_extent_blocks(blkcnt, recp->overflowExtents.extents);
            blkcnt -= extentblks;
        }
    } else { /* Inline data */
        if (exists) {
            result = remove_attribute_records(hfsmp, iterator);
            if (result) {
                goto exit_lock;
            }
        }

        /* Calculate size of record rounded up to multiple of 2 bytes. */
        btdata.itemSize = sizeof(HFSPlusAttrData) - 2 + attrsize + ((attrsize & 1) ? 1 : 0);
        recp = hfs_malloc(btdata.itemSize);

        recp->recordType = kHFSPlusAttrInlineData;
        recp->attrData.reserved[0] = 0;
        recp->attrData.reserved[1] = 0;
        recp->attrData.attrSize = (u_int32_t)attrsize;

        /* Copy in the attribute data (if any). */
        if (attrsize > 0) {
            bcopy(buf, &recp->attrData.attrData, attrsize);
        }

        (void) hfs_buildattrkey(target_id, attr_name, (HFSPlusAttrKey *)&iterator->key);

        btdata.bufferAddress = recp;
        btdata.itemCount = 1;
        result = BTInsertRecord(btfile, iterator, &btdata, btdata.itemSize);
    }

exit_lock:
    if (btfile && started_transaction) {
        (void) BTFlushPath(btfile);
    }
    hfs_systemfile_unlock(hfsmp, lockflags);
    if (result == 0) {
        if (vp) {
            cp = VTOC(vp);
            /* Setting an attribute only updates change time and not
             * modified time of the file.
             */
            cp->c_touch_chgtime = TRUE;
            cp->c_flag |= C_MODIFIED;
            cp->c_attr.ca_recflags |= kHFSHasAttributesMask;
            if ((strcmp(attr_name, KAUTH_FILESEC_XATTR) == 0)) {
                cp->c_attr.ca_recflags |= kHFSHasSecurityMask;
            }
            (void) hfs_update(vp, 0);
        }
    }
    if (started_transaction) {
        if (result && allocatedblks) {
            free_attr_blks(hfsmp, allocatedblks, extentptr);
        }
        hfs_end_transaction(hfsmp);
    }

    hfs_free(recp);
    hfs_free(extentptr);
    hfs_free(iterator);

exit:
    if (cp) {
        hfs_unlock(cp);
    }

    return (result == btNotFound ? ENOATTR : MacToVFSError(result));
}

/*
 * Remove an extended attribute.
 */
int
hfs_vnop_removexattr(vnode_t vp, const char *attr_name)
{
    struct cnode *cp = VTOC(vp);
    struct hfsmount *hfsmp;
    BTreeIterator * iterator = NULL;
    int lockflags;
    int result;

    if (attr_name == NULL || attr_name[0] == '\0') {
        return (EINVAL);  /* invalid name */
    }
    hfsmp = VTOHFS(vp);
    if (VNODE_IS_RSRC(vp)) {
        return (EPERM);
    }

    /* Write the Resource Fork. */
    if (strcmp(attr_name, XATTR_RESOURCEFORK_NAME) == 0) {
        return (ENOTSUP);
    }

    /* Clear out the Finder Info. */
    if (strcmp(attr_name, XATTR_FINDERINFO_NAME) == 0) {
        void * finderinfo_start;
        int finderinfo_size;
        u_int8_t finderinfo[32];
        u_int32_t date_added = 0, write_gen_counter = 0, document_id = 0;
        u_int8_t *finfo = NULL;

        if ((result = hfs_lock(cp, HFS_EXCLUSIVE_LOCK, HFS_LOCK_DEFAULT))) {
            return (result);
        }

        /* Use the local copy to store our temporary changes. */
        bcopy(cp->c_finderinfo, finderinfo, sizeof(finderinfo));

        /* Zero out the date added field in the local copy */
        hfs_zero_hidden_fields (cp, finderinfo);

        /* Don't expose a symlink's private type/creator. */
        if (vnode_islnk(vp)) {
            struct FndrFileInfo *fip;

            fip = (struct FndrFileInfo *)&finderinfo;
            fip->fdType = 0;
            fip->fdCreator = 0;
        }

        /* Do the byte compare against the local copy */
        if (bcmp(finderinfo, emptyfinfo, sizeof(emptyfinfo)) == 0) {
            hfs_unlock(cp);
            return (ENOATTR);
        }

        /*
         * If there was other content, zero out everything except
         * type/creator and date added.  First, save the date added.
         */
        finfo = cp->c_finderinfo;
        finfo = finfo + 16;
        if (S_ISREG(cp->c_attr.ca_mode) || S_ISLNK(cp->c_attr.ca_mode)) {
            struct FndrExtendedFileInfo *extinfo = (struct FndrExtendedFileInfo *)finfo;
            date_added = extinfo->date_added;
            write_gen_counter = extinfo->write_gen_counter;
            document_id = extinfo->document_id;
        } else if (S_ISDIR(cp->c_attr.ca_mode)) {
            struct FndrExtendedDirInfo *extinfo = (struct FndrExtendedDirInfo *)finfo;
            date_added = extinfo->date_added;
            write_gen_counter = extinfo->write_gen_counter;
            document_id = extinfo->document_id;
        }

        if (vnode_islnk(vp)) {
            /* Ignore type/creator */
            finderinfo_start = &cp->c_finderinfo[8];
            finderinfo_size = sizeof(cp->c_finderinfo) - 8;
        } else {
            finderinfo_start = &cp->c_finderinfo[0];
            finderinfo_size = sizeof(cp->c_finderinfo);
        }
        bzero(finderinfo_start, finderinfo_size);

        /* Now restore the date added */
        if (S_ISREG(cp->c_attr.ca_mode) || S_ISLNK(cp->c_attr.ca_mode)) {
            struct FndrExtendedFileInfo *extinfo = (struct FndrExtendedFileInfo *)finfo;
            extinfo->date_added = date_added;
            extinfo->write_gen_counter = write_gen_counter;
            extinfo->document_id = document_id;
        } else if (S_ISDIR(cp->c_attr.ca_mode)) {
            struct FndrExtendedDirInfo *extinfo = (struct FndrExtendedDirInfo *)finfo;
            extinfo->date_added = date_added;
            extinfo->write_gen_counter = write_gen_counter;
            extinfo->document_id = document_id;
        }

        /* Updating finderInfo updates change time and modified time */
        cp->c_touch_chgtime = TRUE;
        cp->c_flag |= C_MODIFIED;
        hfs_update(vp, 0);

        hfs_unlock(cp);

        return (0);
    }

    if (hfsmp->hfs_attribute_vp == NULL) {
        return (ENOATTR);
    }

    iterator = hfs_mallocz(sizeof(*iterator));

    if ((result = hfs_lock(cp, HFS_EXCLUSIVE_LOCK, HFS_LOCK_DEFAULT))) {
        goto exit_nolock;
    }

    result = hfs_buildattrkey(cp->c_fileid, attr_name, (HFSPlusAttrKey *)&iterator->key);
    if (result) {
        goto exit;
    }

    if (hfs_start_transaction(hfsmp) != 0) {
        result = EINVAL;
        goto exit;
    }
    lockflags = hfs_systemfile_lock(hfsmp, SFL_ATTRIBUTE | SFL_BITMAP, HFS_EXCLUSIVE_LOCK);

    result = remove_attribute_records(hfsmp, iterator);

    hfs_systemfile_unlock(hfsmp, lockflags);

    if (result == 0) {
        cp->c_touch_chgtime = TRUE;

        lockflags = hfs_systemfile_lock(hfsmp, SFL_ATTRIBUTE, HFS_SHARED_LOCK);

        /* If no more attributes exist, clear attribute bit */
        result = file_attribute_exist(hfsmp, cp->c_fileid);
        if (result == 0) {
            cp->c_attr.ca_recflags &= ~kHFSHasAttributesMask;
            cp->c_flag |= C_MODIFIED;
        }
        if (result == EEXIST) {
            result = 0;
        }

        hfs_systemfile_unlock(hfsmp, lockflags);

        /* If ACL was removed, clear security bit */
        if (strcmp(attr_name, KAUTH_FILESEC_XATTR) == 0) {
            cp->c_attr.ca_recflags &= ~kHFSHasSecurityMask;
            cp->c_flag |= C_MODIFIED;
        }
        (void) hfs_update(vp, 0);
    }

    hfs_end_transaction(hfsmp);
exit:
    hfs_unlock(cp);
exit_nolock:
    hfs_free(iterator);
    return MacToVFSError(result);
}

/*
 * Initialize vnode for attribute data I/O.
 *
 * On success,
 *     - returns zero
 *     - the attrdata vnode is initialized as hfsmp->hfs_attrdata_vp
 *     - an iocount is taken on the attrdata vnode which exists
 *       for the entire duration of the mount.  It is only dropped
 *       during unmount
 *     - the attrdata cnode is not locked
 *
 * On failure,
 *     - returns non-zero value
 *     - the caller does not have to worry about any locks or references
 */
int init_attrdata_vnode(struct hfsmount *hfsmp)
{
    vnode_t vp;
    int result = 0;
    struct cat_desc cat_desc;
    struct cat_attr cat_attr;
    struct cat_fork cat_fork;
    int newvnode_flags = 0;

    bzero(&cat_desc, sizeof(cat_desc));
    cat_desc.cd_parentcnid = kHFSRootParentID;
    cat_desc.cd_nameptr = (const u_int8_t *)hfs_attrdatafilename;
    cat_desc.cd_namelen = strlen(hfs_attrdatafilename);
    cat_desc.cd_cnid = kHFSAttributeDataFileID;
    /* Tag vnode as system file, note that we can still use cluster I/O */
    cat_desc.cd_flags |= CD_ISMETA;

    bzero(&cat_attr, sizeof(cat_attr));
    cat_attr.ca_linkcount = 1;
    cat_attr.ca_mode = S_IFREG;
    cat_attr.ca_fileid = cat_desc.cd_cnid;
    cat_attr.ca_blocks = hfsmp->totalBlocks;

    /*
     * The attribute data file is a virtual file that spans the
     * entire file system space.
     *
     * Each extent-based attribute occupies a unique portion of
     * in this virtual file.  The cluster I/O is done using actual
     * allocation block offsets so no additional mapping is needed
     * for the VNOP_BLOCKMAP call.
     *
     * This approach allows the attribute data to be cached without
     * incurring the high cost of using a separate vnode per attribute.
     *
     * Since we need to acquire the attribute b-tree file lock anyways,
     * the virtual file doesn't introduce any additional serialization.
     */
    bzero(&cat_fork, sizeof(cat_fork));
    cat_fork.cf_size = (u_int64_t)hfsmp->totalBlocks * (u_int64_t)hfsmp->blockSize;
    cat_fork.cf_blocks = hfsmp->totalBlocks;
    cat_fork.cf_extents[0].startBlock = 0;
    cat_fork.cf_extents[0].blockCount = cat_fork.cf_blocks;

    result = hfs_getnewvnode(hfsmp, NULL, NULL, &cat_desc, 0, &cat_attr,
                             &cat_fork, &vp, &newvnode_flags);
    if (result == 0) {
        hfsmp->hfs_attrdata_vp = vp;
        hfs_unlock(VTOC(vp));
    }
    return (result);
}

/* Check if any attribute record exist for given fileID.  This function
 * is called by hfs_vnop_removexattr to determine if it should clear the
 * attribute bit in the catalog record or not.
 *
 * Note - you must acquire a shared lock on the attribute btree before
 *        calling this function.
 *
 * Output:
 *     EEXIST    - If attribute record was found
 *    0    - Attribute was not found
 *    (other)    - Other error (such as EIO)
 */
int
file_attribute_exist(struct hfsmount *hfsmp, uint32_t fileID)
{
    HFSPlusAttrKey *key;
    BTreeIterator * iterator = NULL;
    struct filefork *btfile;
    int result = 0;

    // if there's no attribute b-tree we sure as heck
    // can't have any attributes!
    if (hfsmp->hfs_attribute_vp == NULL) {
        return 0;
    }

    iterator = hfs_mallocz(sizeof(BTreeIterator));
    if (iterator == NULL) return ENOMEM;
    
    key = (HFSPlusAttrKey *)&iterator->key;

    result = hfs_buildattrkey(fileID, NULL, key);
    if (result) {
        goto out;
    }

    btfile = VTOF(hfsmp->hfs_attribute_vp);
    result = BTSearchRecord(btfile, iterator, NULL, NULL, NULL);
    if (result && (result != btNotFound)) {
        goto out;
    }

    result = BTIterateRecord(btfile, kBTreeNextRecord, iterator, NULL, NULL);
    /* If no next record was found or fileID for next record did not match,
     * no more attributes exist for this fileID
     */
    if ((result && (result == btNotFound)) || (key->fileID != fileID)) {
        result = 0;
    } else {
        result = EEXIST;
    }

out:
    hfs_free(iterator);
    return result;
}

/*
 * Read an extent based attribute.
 */
static int
read_attr_data(struct hfsmount *hfsmp, void *buf, size_t datasize, HFSPlusExtentDescriptor *extents)
{
    vnode_t evp = hfsmp->hfs_attrdata_vp;
    uint64_t iosize;
    uint64_t attrsize;
    uint64_t blksize;
    uint64_t alreadyread;
    int i;
    int result = 0;

    hfs_lock_truncate(VTOC(evp), HFS_SHARED_LOCK, HFS_LOCK_DEFAULT);

    attrsize = (uint64_t)datasize;
    blksize = (uint64_t)hfsmp->blockSize;
    alreadyread = 0;

    /*
     * Read the attribute data one extent at a time.
     * For the typical case there is only one extent.
     */
    for (i = 0; (attrsize > 0) && (extents[i].startBlock != 0); ++i) {
        iosize = extents[i].blockCount * blksize;
        iosize = MIN(iosize, attrsize);

        uint64_t actualread = 0;

        result = raw_readwrite_read_internal( evp, extents[i].startBlock, extents[i].blockCount * blksize,
                                              alreadyread, iosize, buf, &actualread );
#if HFS_XATTR_VERBOSE
        LFHFS_LOG(LEVEL_DEBUG, "hfs: read_attr_data: cr iosize %lld [%d, %d] (%d)\n",
                  actualread, extents[i].startBlock, extents[i].blockCount, result);
#endif
        if (result)
            break;

        // read the remaining part after sector boundary if we have such
        if (iosize != actualread)
        {
            result = raw_readwrite_read_internal( evp, extents[i].startBlock, extents[i].blockCount * blksize,
                                                  alreadyread + actualread, iosize - actualread,
                                                  (uint8_t*)buf + actualread, &actualread );
#if HFS_XATTR_VERBOSE
            LFHFS_LOG(LEVEL_DEBUG, "hfs: read_attr_data: cr iosize %lld [%d, %d] (%d)\n",
                      actualread, extents[i].startBlock, extents[i].blockCount, result);
#endif
            if (result)
                break;
        }

        attrsize -= iosize;

        alreadyread += iosize;
        buf = (uint8_t*)buf + iosize;
    }

    hfs_unlock_truncate(VTOC(evp), HFS_LOCK_DEFAULT);
    return (result);
}

/*
 * Write an extent based attribute.
 */
static int
write_attr_data(struct hfsmount *hfsmp, void *buf, size_t datasize, HFSPlusExtentDescriptor *extents)
{
    vnode_t evp = hfsmp->hfs_attrdata_vp;
    uint64_t iosize;
    uint64_t attrsize;
    uint64_t blksize;
    uint64_t alreadywritten;
    int i;
    int result = 0;

    hfs_lock_truncate(VTOC(evp), HFS_SHARED_LOCK, HFS_LOCK_DEFAULT);

    attrsize = (uint64_t)datasize;
    blksize = (uint64_t)hfsmp->blockSize;
    alreadywritten = 0;

    /*
     * Write the attribute data one extent at a time.
     */
    for (i = 0; (attrsize > 0) && (extents[i].startBlock != 0); ++i) {
        iosize = extents[i].blockCount * blksize;
        iosize = MIN(iosize, attrsize);

        uint64_t actualwritten = 0;

        result = raw_readwrite_write_internal( evp, extents[i].startBlock, extents[i].blockCount * blksize,
                                               alreadywritten, iosize, buf, &actualwritten );
#if HFS_XATTR_VERBOSE
        LFHFS_LOG(LEVEL_DEBUG, "hfs: write_attr_data: cw iosize %lld [%d, %d] (%d)\n",
                  actualwritten, extents[i].startBlock, extents[i].blockCount, result);
#endif
        if (result)
            break;

        // write the remaining part after sector boundary if we have such
        if (iosize != actualwritten)
        {
            result = raw_readwrite_write_internal( evp, extents[i].startBlock, extents[i].blockCount * blksize,
                                                   alreadywritten + actualwritten, iosize - actualwritten,
                                                   (uint8_t*)buf + actualwritten, &actualwritten );
#if HFS_XATTR_VERBOSE
            LFHFS_LOG(LEVEL_DEBUG, "hfs: write_attr_data: cw iosize %lld [%d, %d] (%d)\n",
                      actualwritten, extents[i].startBlock, extents[i].blockCount, result);
#endif
            if (result)
                break;
        }

        attrsize -= iosize;

        alreadywritten += iosize;
        buf = (uint8_t*)buf + iosize;
    }

    hfs_unlock_truncate(VTOC(evp), HFS_LOCK_DEFAULT);
    return (result);
}

/*
 * Allocate blocks for an extent based attribute.
 */
static int
alloc_attr_blks(struct hfsmount *hfsmp, size_t attrsize, size_t extentbufsize, HFSPlusExtentDescriptor *extents, int *blocks)
{
    int blkcnt;
    int startblk;
    int lockflags;
    int i;
    int maxextents;
    int result = 0;

    startblk = hfsmp->hfs_metazone_end;
    blkcnt = (int)howmany(attrsize, hfsmp->blockSize);
    if (blkcnt > (int)hfs_freeblks(hfsmp, 0)) {
        return (ENOSPC);
    }
    *blocks = blkcnt;
    maxextents = (int)extentbufsize / sizeof(HFSPlusExtentDescriptor);

    lockflags = hfs_systemfile_lock(hfsmp, SFL_BITMAP, HFS_EXCLUSIVE_LOCK);

    for (i = 0; (blkcnt > 0) && (i < maxextents); i++) {
        /* Try allocating and see if we find something decent */
        result = BlockAllocate(hfsmp, startblk, blkcnt, blkcnt, 0,
                               &extents[i].startBlock, &extents[i].blockCount);
        /*
         * If we couldn't find anything, then re-try the allocation but allow
         * journal flushes.
         */
        if (result == dskFulErr) {
            result = BlockAllocate(hfsmp, startblk, blkcnt, blkcnt, HFS_ALLOC_FLUSHTXN,
                                   &extents[i].startBlock, &extents[i].blockCount);
        }
#if HFS_XATTR_VERBOSE
        LFHFS_LOG(LEVEL_DEBUG,"hfs: alloc_attr_blks: BA blkcnt %d [%d, %d] (%d)\n",
                  blkcnt, extents[i].startBlock, extents[i].blockCount, result);
#endif
        if (result) {
            extents[i].startBlock = 0;
            extents[i].blockCount = 0;
            break;
        }
        blkcnt -= extents[i].blockCount;
        startblk = extents[i].startBlock + extents[i].blockCount;
    }
    /*
     * If it didn't fit in the extents buffer then bail.
     */
    if (blkcnt) {
        result = ENOSPC;
#if HFS_XATTR_VERBOSE
        LFHFS_LOG(LEVEL_DEBUG, "hfs: alloc_attr_blks: unexpected failure, %d blocks unallocated\n", blkcnt);
#endif
        for (; i >= 0; i--) {
            if ((blkcnt = extents[i].blockCount) != 0) {
                (void) BlockDeallocate(hfsmp, extents[i].startBlock, blkcnt, 0);
                extents[i].startBlock = 0;
                extents[i].blockCount = 0;
            }
        }
    }

    hfs_systemfile_unlock(hfsmp, lockflags);
    return MacToVFSError(result);
}

/*
 * Release blocks from an extent based attribute.
 */
static void
free_attr_blks(struct hfsmount *hfsmp, int blkcnt, HFSPlusExtentDescriptor *extents)
{
    vnode_t evp = hfsmp->hfs_attrdata_vp;
    int remblks = blkcnt;
    int lockflags;
    int i;

    lockflags = hfs_systemfile_lock(hfsmp, SFL_BITMAP, HFS_EXCLUSIVE_LOCK);

    for (i = 0; (remblks > 0) && (extents[i].blockCount != 0); i++) {
        if (extents[i].blockCount > (u_int32_t)blkcnt) {
#if HFS_XATTR_VERBOSE
            LFHFS_LOG(LEVEL_DEBUG, "hfs: free_attr_blks: skipping bad extent [%d, %d]\n",
                      extents[i].startBlock, extents[i].blockCount);
#endif
            extents[i].blockCount = 0;
            continue;
        }
        if (extents[i].startBlock == 0) {
            break;
        }
        (void)BlockDeallocate(hfsmp, extents[i].startBlock, extents[i].blockCount, 0);
        remblks -= extents[i].blockCount;
#if HFS_XATTR_VERBOSE
        LFHFS_LOG(LEVEL_DEBUG, "hfs: free_attr_blks: BlockDeallocate [%d, %d]\n",
                  extents[i].startBlock, extents[i].blockCount);
#endif
        extents[i].startBlock = 0;
        extents[i].blockCount = 0;

        /* Discard any resident pages for this block range. */
        if (evp) {
#if LF_HFS_FULL_VNODE_SUPPORT
            off_t  start, end;
            start = (u_int64_t)extents[i].startBlock * (u_int64_t)hfsmp->blockSize;
            end = start + (u_int64_t)extents[i].blockCount * (u_int64_t)hfsmp->blockSize;
            //TBD - Need to update this vnode
            (void) ubc_msync(hfsmp->hfs_attrdata_vp, start, end, &start, UBC_INVALIDATE);
#endif
        }
    }

    hfs_systemfile_unlock(hfsmp, lockflags);
}

static int
has_overflow_extents(HFSPlusForkData *forkdata)
{
    u_int32_t blocks;

    if (forkdata->extents[7].blockCount == 0)
        return (0);

    blocks = forkdata->extents[0].blockCount +
    forkdata->extents[1].blockCount +
    forkdata->extents[2].blockCount +
    forkdata->extents[3].blockCount +
    forkdata->extents[4].blockCount +
    forkdata->extents[5].blockCount +
    forkdata->extents[6].blockCount +
    forkdata->extents[7].blockCount;

    return (forkdata->totalBlocks > blocks);
}

static int
count_extent_blocks(int maxblks, HFSPlusExtentRecord extents)
{
    int blocks;
    int i;

    for (i = 0, blocks = 0; i < kHFSPlusExtentDensity; ++i) {
        /* Ignore obvious bogus extents. */
        if (extents[i].blockCount > (u_int32_t)maxblks)
            continue;
        if (extents[i].startBlock == 0 || extents[i].blockCount == 0)
            break;
        blocks += extents[i].blockCount;
    }
    return (blocks);
}

/*
 * Remove all the records for a given attribute.
 *
 * - Used by hfs_vnop_removexattr, hfs_vnop_setxattr and hfs_removeallattr.
 * - A transaction must have been started.
 * - The Attribute b-tree file must be locked exclusive.
 * - The Allocation Bitmap file must be locked exclusive.
 * - The iterator key must be initialized.
 */
static int
remove_attribute_records(struct hfsmount *hfsmp, BTreeIterator * iterator)
{
    struct filefork *btfile;
    FSBufferDescriptor btdata;
    HFSPlusAttrRecord attrdata;  /* 90 bytes */
    u_int16_t datasize;
    int result;

    btfile = VTOF(hfsmp->hfs_attribute_vp);

    btdata.bufferAddress = &attrdata;
    btdata.itemSize = sizeof(attrdata);
    btdata.itemCount = 1;
    result = BTSearchRecord(btfile, iterator, &btdata, &datasize, NULL);
    if (result) {
        goto exit; /* no records. */
    }
    /*
     * Free the blocks from extent based attributes.
     *
     * Note that the block references (btree records) are removed
     * before releasing the blocks in the allocation bitmap.
     */
    if (attrdata.recordType == kHFSPlusAttrForkData) {
        int totalblks;
        int extentblks;
        u_int32_t *keystartblk;

        if (datasize < sizeof(HFSPlusAttrForkData)) {
            LFHFS_LOG(LEVEL_DEBUG, "remove_attribute_records: bad record size %d (expecting %lu)\n", datasize, sizeof(HFSPlusAttrForkData));
        }
        totalblks = attrdata.forkData.theFork.totalBlocks;

        /* Process the first 8 extents. */
        extentblks = count_extent_blocks(totalblks, attrdata.forkData.theFork.extents);
        if (extentblks > totalblks)
        {
            LFHFS_LOG(LEVEL_ERROR, "remove_attribute_records: corruption (1)...");
            hfs_assert(0);
        }
        if (BTDeleteRecord(btfile, iterator) == 0) {
            free_attr_blks(hfsmp, extentblks, attrdata.forkData.theFork.extents);
        }
        totalblks -= extentblks;
        keystartblk = &((HFSPlusAttrKey *)&iterator->key)->startBlock;

        /* Process any overflow extents. */
        while (totalblks) {
            *keystartblk += (u_int32_t)extentblks;

            result = BTSearchRecord(btfile, iterator, &btdata, &datasize, NULL);
            if (result ||
                (attrdata.recordType != kHFSPlusAttrExtents) ||
                (datasize < sizeof(HFSPlusAttrExtents))) {
                LFHFS_LOG(LEVEL_ERROR, "remove_attribute_records: BTSearchRecord: vol=%s, err=%d (%d), totalblks %d\n",
                          hfsmp->vcbVN, MacToVFSError(result), attrdata.recordType != kHFSPlusAttrExtents, totalblks);
                result = ENOATTR;
                break;   /* break from while */
            }
            /* Process the next 8 extents. */
            extentblks = count_extent_blocks(totalblks, attrdata.overflowExtents.extents);
            if (extentblks > totalblks)
            {
                LFHFS_LOG(LEVEL_ERROR, "remove_attribute_records: corruption (2)...");
                hfs_assert(0);
            }
            if (BTDeleteRecord(btfile, iterator) == 0) {
                free_attr_blks(hfsmp, extentblks, attrdata.overflowExtents.extents);
            }
            totalblks -= extentblks;
        }
    } else {
        result = BTDeleteRecord(btfile, iterator);
    }
    (void) BTFlushPath(btfile);
exit:
    return (result == btNotFound ? ENOATTR :  MacToVFSError(result));
}

/*
 * Retrieve the list of extended attribute names.
 */
int
hfs_vnop_listxattr(vnode_t vp, void *buf, size_t bufsize, size_t *actual_size)
{
    struct cnode *cp = VTOC(vp);
    struct hfsmount *hfsmp;
    BTreeIterator * iterator = NULL;
    struct filefork *btfile;
    struct listattr_callback_state state;
    int lockflags;
    int result;
    u_int8_t finderinfo[32];

    if (actual_size == NULL) {
        return (EINVAL);
    }
    if (VNODE_IS_RSRC(vp)) {
        return (EPERM);
    }

    hfsmp = VTOHFS(vp);
    *actual_size = 0;

    /*
     * Take the truncate lock; this serializes us against the ioctl
     * to truncate data & reset the decmpfs state
     * in the compressed file handler.
     */
    hfs_lock_truncate(cp, HFS_SHARED_LOCK, HFS_LOCK_DEFAULT);

    /* Now the regular cnode lock (shared) */
    if ((result = hfs_lock(cp, HFS_SHARED_LOCK, HFS_LOCK_DEFAULT))) {
        hfs_unlock_truncate(cp, HFS_LOCK_DEFAULT);
        return (result);
    }

    /*
     * Make a copy of the cnode's finderinfo to a local so we can
     * zero out the date added field.  Also zero out the private type/creator
     * for symlinks.
     */
    bcopy(cp->c_finderinfo, finderinfo, sizeof(finderinfo));
    hfs_zero_hidden_fields (cp, finderinfo);

    /* Don't expose a symlink's private type/creator. */
    if (vnode_islnk(vp)) {
        struct FndrFileInfo *fip;

        fip = (struct FndrFileInfo *)&finderinfo;
        fip->fdType = 0;
        fip->fdCreator = 0;
    }


    /* If Finder Info is non-empty then export it's name. */
    if (bcmp(finderinfo, emptyfinfo, sizeof(emptyfinfo)) != 0) {
        if (buf == NULL) {
            *actual_size += sizeof(XATTR_FINDERINFO_NAME);
        } else if (bufsize < sizeof(XATTR_FINDERINFO_NAME)) {
            result = ERANGE;
            goto exit;
        } else {
            *actual_size += sizeof(XATTR_FINDERINFO_NAME);
            strcpy((char*)buf, XATTR_FINDERINFO_NAME);
        }
    }

    /* Bail if we don't have any extended attributes. */
    if ((hfsmp->hfs_attribute_vp == NULL) ||
        (cp->c_attr.ca_recflags & kHFSHasAttributesMask) == 0) {
        result = 0;
        goto exit;
    }
    btfile = VTOF(hfsmp->hfs_attribute_vp);

    iterator = hfs_mallocz(sizeof(*iterator));

    result = hfs_buildattrkey(cp->c_fileid, NULL, (HFSPlusAttrKey *)&iterator->key);
    if (result) {
        goto exit;
    }

    lockflags = hfs_systemfile_lock(hfsmp, SFL_ATTRIBUTE, HFS_SHARED_LOCK);

    result = BTSearchRecord(btfile, iterator, NULL, NULL, NULL);
    if (result && result != btNotFound) {
        hfs_systemfile_unlock(hfsmp, lockflags);
        goto exit;
    }

    state.fileID = cp->c_fileid;
    state.result = 0;
    state.buf = (buf == NULL ? NULL : ((u_int8_t*)buf + *actual_size));
    state.bufsize = bufsize - *actual_size;
    state.size = 0;

    /*
     * Process entries starting just after iterator->key.
     */
    result = BTIterateRecords(btfile, kBTreeNextRecord, iterator,
                              (IterateCallBackProcPtr)listattr_callback, &state);
    hfs_systemfile_unlock(hfsmp, lockflags);

    *actual_size += state.size;

    if (state.result || result == btNotFound) {
        result = state.result;
    }

exit:
    hfs_free(iterator);
    hfs_unlock(cp);
    hfs_unlock_truncate(cp, HFS_LOCK_DEFAULT);

    return MacToVFSError(result);
}

/*
 * Callback - called for each attribute record
 */
static int
listattr_callback(const HFSPlusAttrKey *key, __unused const HFSPlusAttrData *data, struct listattr_callback_state *state)
{
    char attrname[XATTR_MAXNAMELEN + 1];
    ssize_t bytecount;
    int result;

    if (state->fileID != key->fileID) {
        state->result = 0;
        return (0);    /* stop */
    }
    /*
     * Skip over non-primary keys
     */
    if (key->startBlock != 0) {
        return (1);    /* continue */
    }

    /* Convert the attribute name into UTF-8. */
    result = utf8_encodestr(key->attrName, key->attrNameLen * sizeof(UniChar),
                            (u_int8_t *)attrname, (size_t *)&bytecount, sizeof(attrname), '/', UTF_ADD_NULL_TERM);
    if (result) {
        state->result = result;
        return (0);    /* stop */
    }
    bytecount++; /* account for null termination char */

    state->size += bytecount;

    if (state->buf != NULL) {
        if ((size_t)bytecount > state->bufsize) {
            state->result = ERANGE;
            return (0);    /* stop */
        }

        memcpy(state->buf, attrname, bytecount);

        state->buf = (state->buf == NULL ? NULL : ((u_int8_t*)state->buf + bytecount));
        state->bufsize -= bytecount;
    }
    return (1); /* continue */
}

/*
 * Remove all the attributes from a cnode.
 *
 * This function creates/ends its own transaction so that each
 * attribute is deleted in its own transaction (to avoid having
 * a transaction grow too large).
 *
 * This function takes the necessary locks on the attribute
 * b-tree file and the allocation (bitmap) file.
 *
 * NOTE: Upon sucecss, this function will return with an open
 * transaction.  The reason we do it this way is because when we
 * delete the last attribute, we must make sure the flag in the
 * catalog record that indicates there are no more records is cleared.
 * The caller is responsible for doing this and *must* do it before
 * ending the transaction.
 */
int
hfs_removeallattr(struct hfsmount *hfsmp, u_int32_t fileid, bool *open_transaction)
{
    BTreeIterator *iterator = NULL;
    HFSPlusAttrKey *key;
    struct filefork *btfile;
    int result, lockflags = 0;

    *open_transaction = false;

    if (hfsmp->hfs_attribute_vp == NULL)
        return 0;

    btfile = VTOF(hfsmp->hfs_attribute_vp);

    iterator = hfs_mallocz(sizeof(BTreeIterator));
    if (iterator == NULL)
        return ENOMEM;

    key = (HFSPlusAttrKey *)&iterator->key;

    /* Loop until there are no more attributes for this file id */
    do {
        if (!*open_transaction)
            lockflags = hfs_systemfile_lock(hfsmp, SFL_ATTRIBUTE, HFS_SHARED_LOCK);

        (void) hfs_buildattrkey(fileid, NULL, key);
        result = BTIterateRecord(btfile, kBTreeNextRecord, iterator, NULL, NULL);
        if (result || key->fileID != fileid)
            goto exit;

        hfs_systemfile_unlock(hfsmp, lockflags);
        lockflags = 0;

        if (*open_transaction) {
            hfs_end_transaction(hfsmp);
            *open_transaction = false;
        }

        if (hfs_start_transaction(hfsmp) != 0) {
            result = EINVAL;
            goto exit;
        }

        *open_transaction = true;

        lockflags = hfs_systemfile_lock(hfsmp, SFL_ATTRIBUTE | SFL_BITMAP, HFS_EXCLUSIVE_LOCK);

        result = remove_attribute_records(hfsmp, iterator);

    } while (!result);

exit:
    hfs_free(iterator);

    if (lockflags)
        hfs_systemfile_unlock(hfsmp, lockflags);

    result = result == btNotFound ? 0 : MacToVFSError(result);

    if (result && *open_transaction) {
        hfs_end_transaction(hfsmp);
        *open_transaction = false;
    }

    return result;
}

/*
 * hfs_attrkeycompare - compare two attribute b-tree keys.
 *
 * The name portion of the key is compared using a 16-bit binary comparison.
 * This is called from the b-tree code.
 */
int
hfs_attrkeycompare(HFSPlusAttrKey *searchKey, HFSPlusAttrKey *trialKey)
{
    u_int32_t searchFileID, trialFileID;
    int result;

    searchFileID = searchKey->fileID;
    trialFileID = trialKey->fileID;
    result = 0;

    if (searchFileID > trialFileID) {
        ++result;
    } else if (searchFileID < trialFileID) {
        --result;
    } else {
        u_int16_t * str1 = &searchKey->attrName[0];
        u_int16_t * str2 = &trialKey->attrName[0];
        int length1 = searchKey->attrNameLen;
        int length2 = trialKey->attrNameLen;
        u_int16_t c1, c2;
        int length;

        if (length1 < length2) {
            length = length1;
            --result;
        } else if (length1 > length2) {
            length = length2;
            ++result;
        } else {
            length = length1;
        }

        while (length--) {
            c1 = *(str1++);
            c2 = *(str2++);

            if (c1 > c2) {
                result = 1;
                break;
            }
            if (c1 < c2) {
                result = -1;
                break;
            }
        }
        if (result)
            return (result);
        /*
         * Names are equal; compare startBlock
         */
        if (searchKey->startBlock == trialKey->startBlock) {
            return (0);
        } else {
            return (searchKey->startBlock < trialKey->startBlock ? -1 : 1);
        }
    }

    return result;
}

/*
 * hfs_buildattrkey - build an Attribute b-tree key
 */
int
hfs_buildattrkey(u_int32_t fileID, const char *attrname, HFSPlusAttrKey *key)
{
    int result = 0;
    size_t unicodeBytes = 0;

    if (attrname != NULL) {
        /*
         * Convert filename from UTF-8 into Unicode
         */
        result = utf8_decodestr((const u_int8_t *)attrname, strlen(attrname), key->attrName,
                                &unicodeBytes, sizeof(key->attrName), 0, 0);
        if (result) {
            if (result != ENAMETOOLONG)
                result = EINVAL;  /* name has invalid characters */
            return (result);
        }
        key->attrNameLen = unicodeBytes / sizeof(UniChar);
        key->keyLength = kHFSPlusAttrKeyMinimumLength + unicodeBytes;
    } else {
        key->attrNameLen = 0;
        key->keyLength = kHFSPlusAttrKeyMinimumLength;
    }
    key->pad = 0;
    key->fileID = fileID;
    key->startBlock = 0;

    return (0);
}

/*
 * getnodecount - calculate starting node count for attributes b-tree.
 */
static int
getnodecount(struct hfsmount *hfsmp, size_t nodesize)
{
    u_int64_t freebytes;
    u_int64_t calcbytes;

    /*
     * 10.4: Scale base on current catalog file size (20 %) up to 20 MB.
     * 10.5: Attempt to be as big as the catalog clump size.
     *
     * Use no more than 10 % of the remaining free space.
     */
    freebytes = (u_int64_t)hfs_freeblks(hfsmp, 0) * (u_int64_t)hfsmp->blockSize;

    calcbytes = MIN(hfsmp->hfs_catalog_cp->c_datafork->ff_size / 5, 20 * 1024 * 1024);

    calcbytes = MAX(calcbytes, hfsmp->hfs_catalog_cp->c_datafork->ff_clumpsize);

    calcbytes = MIN(calcbytes, freebytes / 10);

    return (MAX(2, (int)(calcbytes / nodesize)));
}

/*
 * getmaxinlineattrsize - calculate maximum inline attribute size.
 *
 * This yields 3,802 bytes for an 8K node size.
 */
static size_t
getmaxinlineattrsize(struct vnode * attrvp)
{
    BTreeInfoRec btinfo;
    size_t nodesize = ATTRIBUTE_FILE_NODE_SIZE;
    size_t maxsize;

    if (attrvp != NULL) {
        (void) hfs_lock(VTOC(attrvp), HFS_SHARED_LOCK, HFS_LOCK_DEFAULT);
        if (BTGetInformation(VTOF(attrvp), 0, &btinfo) == 0)
            nodesize = btinfo.nodeSize;
        hfs_unlock(VTOC(attrvp));
    }
    maxsize = nodesize;
    maxsize -= sizeof(BTNodeDescriptor);     /* minus node descriptor */
    maxsize -= 3 * sizeof(u_int16_t);        /* minus 3 index slots */
    maxsize /= 2;                            /* 2 key/rec pairs minumum */
    maxsize -= sizeof(HFSPlusAttrKey);       /* minus maximum key size */
    maxsize -= sizeof(HFSPlusAttrData) - 2;  /* minus data header */
    maxsize &= 0xFFFFFFFE;                   /* multiple of 2 bytes */

    return (maxsize);
}
