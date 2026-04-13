/*  Copyright © 2017-2018 Apple Inc. All rights reserved.
 *
 *  lf_hfs_vnode.c
 *  livefiles_hfs
 *
 *  Created by Or Haimovich on 20/3/18.
 */

#include "lf_hfs_vnode.h"
#include "lf_hfs_utils.h"
#include "lf_hfs_vfsutils.h"
#include "lf_hfs_generic_buf.h"
#include "lf_hfs_fileops_handler.h"
#include "lf_hfs_xattr.h"
#include <System/sys/decmpfs.h>

int VTtoUVFS_tab[16] =
{
    VNON,
    /* 1 - 5 */
    UVFS_FA_TYPE_FILE, UVFS_FA_TYPE_DIR, VNON, VNON, UVFS_FA_TYPE_SYMLINK,
    /* 6 - 10 */
    VNON, VNON, VNON, VNON, VNON
};

enum vtype iftovt_tab[16] =
{
    VNON, VFIFO, VCHR, VNON, VDIR, VNON, VBLK, VNON,
    VREG, VNON, VLNK, VNON, VSOCK, VNON, VNON, VBAD,
};

int uvfsToVtype_tab[4] =
{
    VNON,VREG,VDIR,VLNK,
};

mode_t  vttoif_tab[9] =
{
    0, S_IFREG, S_IFDIR, S_IFBLK, S_IFCHR, S_IFLNK,
    S_IFSOCK, S_IFIFO, S_IFMT,
};

errno_t vnode_initialize(uint32_t size, void *data, vnode_t *vpp)
{
    memset(*vpp,0,sizeof(struct vnode));
    (*vpp)->cnode = NULL;
    assert(size == sizeof((*vpp)->sFSParams));
    memcpy((void *) &(*vpp)->sFSParams,data,size);

    if ((*vpp)->sFSParams.vnfs_vtype == VDIR)
    {
        (*vpp)->sExtraData.sDirData.uDirVersion = 1;
    }
    return 0;
}

errno_t vnode_create(uint32_t size, void  *data, vnode_t *vpp)
{
    *vpp = hfs_malloc(sizeof(struct vnode));
    if (*vpp == NULL)
    {
        return ENOMEM;
    }

    return (vnode_initialize(size, data, vpp));
}

void vnode_rele(vnode_t vp)
{
    if (vp) {
        lf_hfs_generic_buf_cache_LockBufCache();
        lf_hfs_generic_buf_cache_remove_vnode(vp);
        lf_hfs_generic_buf_cache_UnLockBufCache();
        hfs_free(vp);
    }
    vp = NULL;
}

mount_t vnode_mount(vnode_t vp)
{
    return (vp->sFSParams.vnfs_mp);
}

int vnode_issystem(vnode_t vp)
{
    return (vp->sFSParams.vnfs_marksystem);
}

int vnode_isreg(vnode_t vp)
{
    return (vp->sFSParams.vnfs_vtype == VREG);
}

int vnode_isdir(vnode_t vp)
{
    return (vp->sFSParams.vnfs_vtype == VDIR);
}

int vnode_islnk(vnode_t vp)
{
    return (vp->sFSParams.vnfs_vtype == VLNK);
}

/*!
 @function vnode_update_identity
 case:
    VNODE_UPDATE_PARENT: set parent.
    VNODE_UPDATE_NAME: set name.
    VNODE_UPDATE_CACHE: flush cache entries for hard links associated with this file.

 
 */
void vnode_update_identity(vnode_t vp, vnode_t dvp, const char *name, int name_len, uint32_t name_hashval, int flags)
{
    if (flags & VNODE_UPDATE_PARENT)
    {
        vp->sFSParams.vnfs_dvp = dvp;
    }

    if (flags & VNODE_UPDATE_NAME)
    {
        if (!vp->sFSParams.vnfs_cnp) {
            vp->sFSParams.vnfs_cnp = hfs_malloc(sizeof(struct componentname));
            if (vp->sFSParams.vnfs_cnp == NULL) {
                LFHFS_LOG(LEVEL_ERROR, "vnode_update_identity: failed to malloc vnfs_cnp\n");
                assert(0);
            }
            bzero(vp->sFSParams.vnfs_cnp, sizeof(struct componentname));
        }
        vp->sFSParams.vnfs_cnp->cn_namelen = name_len;
        if (vp->sFSParams.vnfs_cnp->cn_nameptr) {
            hfs_free(vp->sFSParams.vnfs_cnp->cn_nameptr);
            vp->sFSParams.vnfs_cnp->cn_nameptr = NULL;
        }
        vp->sFSParams.vnfs_cnp->cn_nameptr = lf_hfs_utils_allocate_and_copy_string( (char*) name, name_len );
        vp->sFSParams.vnfs_cnp->cn_hash = name_hashval;
    }
}

void vnode_GetAttrInternal (vnode_t vp, UVFSFileAttributes *psOutAttr )
{
    struct cnode *cp = VTOC(vp);
    enum vtype v_type;
    
    memset( psOutAttr, 0, sizeof(UVFSFileAttributes) );
    
    psOutAttr->fa_validmask     = VALID_OUT_ATTR_MASK;
    
    psOutAttr->fa_gid   = cp->c_gid;
    psOutAttr->fa_uid   = cp->c_uid;
    psOutAttr->fa_mode  = cp->c_mode & ALL_UVFS_MODES;
    
    v_type = vp->sFSParams.vnfs_vtype;
    psOutAttr->fa_type = VTOUVFS(v_type);
    
    psOutAttr->fa_atime.tv_sec  = cp->c_atime;
    psOutAttr->fa_ctime.tv_sec  = cp->c_ctime;
    psOutAttr->fa_mtime.tv_sec  = cp->c_mtime;
    psOutAttr->fa_birthtime.tv_sec  = cp->c_btime;
    
    psOutAttr->fa_fileid        = cp->c_fileid;
    psOutAttr->fa_parentid      = cp->c_parentcnid;
    psOutAttr->fa_bsd_flags     = cp->c_bsdflags;
    
    if (v_type == VDIR)
    {
        psOutAttr->fa_allocsize = 0;
        psOutAttr->fa_size      = (cp->c_entries + 2) * AVERAGE_HFSDIRENTRY_SIZE;
        psOutAttr->fa_nlink     = cp->c_entries + 2;
    }
    else
    {
        if (psOutAttr->fa_bsd_flags & UF_COMPRESSED)
        {
            if (VNODE_IS_RSRC(vp))
            {
                psOutAttr->fa_allocsize = VTOF(vp)->ff_blocks * VTOHFS(vp)->blockSize;
                psOutAttr->fa_size      = VTOF(vp)->ff_size;
            }
            else
            {
                hfs_unlock(VTOC(vp));
                void* data = NULL;
                size_t attr_size;
                int iErr = hfs_vnop_getxattr(vp, "com.apple.decmpfs", NULL, 0, &attr_size);
                if (iErr != 0) {
                    goto fail;
                }
                
                if (attr_size < sizeof(decmpfs_disk_header) || attr_size > MAX_DECMPFS_XATTR_SIZE) {
                    iErr = EINVAL;
                    goto fail;
                }
                /* allocation includes space for the extra attr_size field of a compressed_header */
                data = (char *) malloc(attr_size);
                if (!data) {
                    iErr = ENOMEM;
                    goto fail;
                }
                
                /* read the xattr into our buffer, skipping over the attr_size field at the beginning */
                size_t read_size;
                iErr =  hfs_vnop_getxattr(vp, "com.apple.decmpfs", data, attr_size, &read_size);
                if (iErr != 0) {
                    goto fail;
                }
                if (read_size != attr_size) {
                    iErr = EINVAL;
                    goto fail;
                }
                
                decmpfs_header Hdr;
                Hdr.attr_size = (uint32_t) attr_size;
                Hdr.compression_magic = *((uint32_t*)data);
                Hdr.compression_type  = *((uint32_t*)(data + sizeof(uint32_t)));
                Hdr.uncompressed_size = *((uint32_t*)(data + sizeof(uint64_t)));

fail:
                if (iErr)
                {
                    psOutAttr->fa_allocsize = VCTOF(vp, cp)->ff_blocks * VTOHFS(vp)->blockSize;
                    psOutAttr->fa_size      = VCTOF(vp, cp)->ff_size;
                }
                else
                {
                    psOutAttr->fa_allocsize = ROUND_UP(Hdr.uncompressed_size,VTOHFS(vp)->blockSize);
                    psOutAttr->fa_size = Hdr.uncompressed_size;
                }
                
                if (data) free(data);
                hfs_lock(VTOC(vp), 0, 0);
            }
        }
        else
        {
            psOutAttr->fa_allocsize = VCTOF(vp, cp)->ff_blocks * VTOHFS(vp)->blockSize;
            psOutAttr->fa_size      = VCTOF(vp, cp)->ff_size;
        }
        psOutAttr->fa_nlink     = (cp->c_flag & C_HARDLINK)? cp->c_linkcount : 1;
    }
}
