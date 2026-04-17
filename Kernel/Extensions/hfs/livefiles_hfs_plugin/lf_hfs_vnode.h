/*  Copyright © 2017-2018 Apple Inc. All rights reserved.
 *
 *  lf_hfs_vnode.h
 *  livefiles_hfs
 *
 *  Created by Or Haimovich on 18/3/18.
 */

#ifndef lf_hfs_vnode_h
#define lf_hfs_vnode_h

#include <sys/_types/_guid_t.h>

#include "lf_hfs_common.h"
#include <System/sys/vnode.h>

extern int VTtoUVFS_tab[];

#define VTTOUVFS(vt)    (VTtoUVFS_tab[vt])
#define IS_ROOT(vp)     (vp->sFSParams.vnfs_markroot)
#define IS_DIR(vp)      (vp->sFSParams.vnfs_vtype == VDIR)
#define IS_LNK(vp)      (vp->sFSParams.vnfs_vtype == VLNK)


/*
 * Convert between vnode types and inode formats (since POSIX.1
 * defines mode word of stat structure in terms of inode formats).
 */
struct componentname {
    /*
     * Arguments to lookup.
     */
    uint32_t    cn_nameiop;         /* lookup operation */
    uint32_t    cn_flags;           /* flags (see below) */
    
    /*
     * Shared between lookup and commit routines.
     */
    char        *cn_pnbuf;          /* pathname buffer */
    int         cn_pnlen;           /* length of allocated buffer */
    char        *cn_nameptr;        /* pointer to looked up name */
    int         cn_namelen;         /* length of looked up component */
    uint32_t    cn_hash;            /* hash value of looked up name */
    uint32_t    cn_consume;         /* chars to consume in lookup() */
};

/* The following structure specifies a vnode for creation */
struct vnode_fsparam {
    struct mount            *vnfs_mp;               /* mount point to which this vnode_t is part of */
    enum vtype              vnfs_vtype;             /* vnode type */
    const char              *vnfs_str;              /* File system Debug aid */
    struct vnode            *vnfs_dvp;              /* The parent vnode */
    void                    *vnfs_fsnode;           /* inode */
//    int                     (**vnfs_vops)(void *);  /* vnode dispatch table */
    int                     vnfs_markroot;          /* is this a root vnode in FS (not a system wide one) */
    int                     vnfs_marksystem;        /* is  a system vnode */
    dev_t 	                vnfs_rdev;              /* dev_t  for block or char vnodes */
    off_t                   vnfs_filesize;          /* that way no need for getattr in UBC */
    struct componentname    *vnfs_cnp;              /* component name to add to namecache */
    uint32_t                vnfs_flags;             /* flags */
};

typedef struct
{
    uint64_t  uDirVersion;
} DirData_s;

typedef struct vnode
{
    uint32_t uValidNodeMagic1;

    struct hfsmount *mount;
    bool is_rsrc;
    struct cnode *cnode;
    struct vnode_fsparam sFSParams;
    FileSystemRecord_s* psFSRecord;
    bool bIsMountVnode;

    union
    {
        DirData_s sDirData;
    } sExtraData;

    uint32_t uValidNodeMagic2;

} *vnode_t;

typedef struct mount
{
    struct hfsmount* psHfsmount;
    int mnt_flag;
} *mount_t;

struct vnode_attr {
    /* bitfields */
    uint64_t    va_supported;
    uint64_t    va_active;
    /*
     * Control flags.  The low 16 bits are reserved for the
     * ioflags being passed for truncation operations.
     */
    int        va_vaflags;

    /* traditional stat(2) parameter fields */
    dev_t        va_rdev;    /* device id (device nodes only) */
    uint64_t    va_nlink;    /* number of references to this file */
    uint64_t    va_total_size;    /* size in bytes of all forks */
    uint64_t    va_total_alloc;    /* disk space used by all forks */
    uint64_t    va_data_size;    /* size in bytes of the fork managed by current vnode */
    uint64_t    va_data_alloc;    /* disk space used by the fork managed by current vnode */
    uint32_t    va_iosize;    /* optimal I/O blocksize */
    /* file security information */
    uid_t        va_uid;        /* owner UID */
    gid_t        va_gid;        /* owner GID */
    mode_t        va_mode;    /* posix permissions */
    uint32_t    va_flags;    /* file flags */
    struct kauth_acl *va_acl;    /* access control list */
    /* timestamps */
    struct timespec    va_create_time;    /* time of creation */
    struct timespec    va_access_time;    /* time of last access */
    struct timespec    va_modify_time;    /* time of last data modification */
    struct timespec    va_change_time;    /* time of last metadata change */
    struct timespec    va_backup_time;    /* time of last backup */

    /* file parameters */
    uint64_t    va_fileid;    /* file unique ID in filesystem */
    uint64_t    va_linkid;    /* file link unique ID */
    uint64_t    va_parentid;    /* parent ID */
    uint32_t    va_fsid;    /* filesystem ID */
    uint64_t    va_filerev;    /* file revision counter */    /* XXX */
    uint32_t    va_gen;        /* file generation count */    /* XXX - relationship of
                                                               * these two? */
    /* misc parameters */
    uint32_t    va_encoding;    /* filename encoding script */
    enum vtype    va_type;    /* file type */
    char *        va_name;    /* Name for ATTR_CMN_NAME; MAXPATHLEN bytes */
    guid_t        va_uuuid;    /* file owner UUID */
    guid_t        va_guuid;    /* file group UUID */

    /* Meaningful for directories only */
    uint64_t    va_nchildren;     /* Number of items in a directory */
    uint64_t    va_dirlinkcount;  /* Real references to dir (i.e. excluding "." and ".." refs) */

    struct kauth_acl *va_base_acl;

    struct timespec va_addedtime;    /* timestamp when item was added to parent directory */

    /* Data Protection fields */
    uint32_t va_dataprotect_class;    /* class specified for this file if it didn't exist */
    uint32_t va_dataprotect_flags;    /* flags from NP open(2) to the filesystem */
    /* Document revision tracking */
    uint32_t va_document_id;
    /* Fields for Bulk args */
    uint32_t     va_devid;    /* devid of filesystem */
    uint32_t     va_objtype;    /* type of object */
    uint32_t     va_objtag;    /* vnode tag of filesystem */
    uint32_t     va_user_access;    /* access for user */
    uint8_t      va_finderinfo[32];    /* Finder Info */
    uint64_t     va_rsrc_length;    /* Resource Fork length */
    uint64_t     va_rsrc_alloc;    /* Resource Fork allocation size */
    fsid_t         va_fsid64;    /* fsid, of the correct type  */
    uint32_t va_write_gencount;     /* counter that increments each time the file changes */
    uint64_t va_private_size; /* If the file were deleted, how many bytes would be freed immediately */
    /* add new fields here only */
};

/*
 * Convert between vnode types and inode formats (since POSIX.1
 * defines mode word of stat structure in terms of inode formats).
 */
extern enum vtype  iftovt_tab[];
#define IFTOVT(mode)    (iftovt_tab[((mode) & S_IFMT) >> 12])

extern int VTtoUVFS_tab[];
extern int uvfsToVtype_tab[];
extern mode_t   vttoif_tab[];

#define VTOUVFS(type)            (VTtoUVFS_tab[type])
#define UVFSTOV(type)            (uvfsToVtype_tab[type])
#define MAKEIMODE(indx)          (vttoif_tab[indx])

#define VNODE_UPDATE_PARENT   0x01
#define VNODE_UPDATE_NAME     0x02
#define VNODE_UPDATE_CACHE    0x04

#define VNODE_REMOVE_NODELETEBUSY              0x0001 /* Don't delete busy files (Carbon) */
#define VNODE_REMOVE_SKIP_NAMESPACE_EVENT    0x0002 /* Do not upcall to userland handlers */
#define VNODE_REMOVE_NO_AUDIT_PATH        0x0004 /* Do not audit the path */

#define    VNOVAL    (-1)

/*
 * Flags for ioflag.
 */
#define    IO_UNIT        0x0001        /* do I/O as atomic unit */
#define    IO_APPEND    0x0002        /* append write to end */
#define    IO_SYNC        0x0004        /* do I/O synchronously */
#define    IO_NODELOCKED    0x0008        /* underlying node already locked */
#define    IO_NDELAY    0x0010        /* FNDELAY flag set in file table */
#define    IO_NOZEROFILL    0x0020        /* F_SETSIZE fcntl uses to prevent zero filling */
//#ifdef XNU_KERNEL_PRIVATE
#define IO_REVOKE    IO_NOZEROFILL    /* revoked close for tty, will Not be used in conjunction */
//#endif /* XNU_KERNEL_PRIVATE */
#define    IO_TAILZEROFILL    0x0040        /* zero fills at the tail of write */
#define    IO_HEADZEROFILL    0x0080        /* zero fills at the head of write */
#define    IO_NOZEROVALID    0x0100        /* do not zero fill if valid page */
#define    IO_NOZERODIRTY    0x0200        /* do not zero fill if page is dirty */
#define IO_CLOSE    0x0400        /* I/O issued from close path */
#define IO_NOCACHE    0x0800        /* same effect as VNOCACHE_DATA, but only for this 1 I/O */
#define IO_RAOFF    0x1000        /* same effect as VRAOFF, but only for this 1 I/O */
#define IO_DEFWRITE    0x2000        /* defer write if vfs.defwrite is set */
#define IO_PASSIVE    0x4000        /* this I/O is marked as background I/O so it won't throttle Throttleable I/O */
#define IO_BACKGROUND IO_PASSIVE /* used for backward compatibility.  to be removed after IO_BACKGROUND is no longer
* used by DiskImages in-kernel mode */
#define    IO_NOAUTH    0x8000        /* No authorization checks. */
#define IO_NODIRECT     0x10000        /* don't use direct synchronous writes if IO_NOCACHE is specified */
#define IO_ENCRYPTED    0x20000        /* Retrieve encrypted blocks from the filesystem */
#define IO_RETURN_ON_THROTTLE    0x40000
#define IO_SINGLE_WRITER    0x80000
#define IO_SYSCALL_DISPATCH        0x100000    /* I/O was originated from a file table syscall */
#define IO_SWAP_DISPATCH        0x200000    /* I/O was originated from the swap layer */
#define IO_SKIP_ENCRYPTION        0x400000    /* Skips en(de)cryption on the IO. Must be initiated from kernel */
#define IO_EVTONLY                      0x800000        /* the i/o is being done on an fd that's marked O_EVTONLY */

errno_t vnode_create(uint32_t size, void  *data, vnode_t *vpp);
errno_t vnode_initialize(uint32_t size, void *data, vnode_t *vpp);
void    vnode_rele(vnode_t vp);
mount_t vnode_mount(vnode_t vp);
int     vnode_issystem(vnode_t vp);
int     vnode_isreg(vnode_t vp);
int     vnode_isdir(vnode_t vp);
int     vnode_islnk(vnode_t vp);
void    vnode_update_identity(vnode_t vp, vnode_t dvp, const char *name, int name_len, uint32_t name_hashval, int flags);
void    vnode_GetAttrInternal (vnode_t vp, UVFSFileAttributes *psOutAttr );
#endif /* lf_hfs_vnode_h */
