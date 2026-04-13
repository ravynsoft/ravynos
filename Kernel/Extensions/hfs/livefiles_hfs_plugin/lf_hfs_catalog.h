/*  Copyright © 2017-2018 Apple Inc. All rights reserved.
 *
 *  lf_hfs_catalog.h
 *  livefiles_hfs
 *
 *  Created by Or Haimovich on 18/3/18.
 */

#ifndef lf_hfs_catalog_h
#define lf_hfs_catalog_h

#include "lf_hfs_format.h"
#include "lf_hfs_locks.h"

#include <sys/queue.h>

#define HFS_IDHASH_DEFAULT (64)

/*
 * Catalog Operations Hint
 *
 * lower 16 bits: count of B-tree insert operations
 * upper 16 bits: count of B-tree delete operations
 *
 */
#define CAT_DELETE    0x00010000
#define CAT_CREATE    0x00000002
#define CAT_RENAME    0x00010002
#define CAT_EXCHANGE    0x00010002

typedef struct
{
    void*    pvBuffer;
    uint64_t uBufferResid;
    uint64_t uBufferSize;
} ReadDirBuff_s, *ReadDirBuff_t;

#define READDIR_BUF_OFFSET(buf) (buf->uBufferSize - buf->uBufferResid)
/*
 * Catalog ADTs
 *
 * The cat_desc, cat_attr, and cat_fork structures are
 * use to import/export data to/from the Catalog file.
 * The fields in these structures are always in BSD
 * runtime format (e.g. dates and names).
 */

typedef u_int32_t    cnid_t;

/*
 * Catalog Node Descriptor (runtime)
 */
struct cat_desc {
    u_int8_t        cd_flags;       /* see below (8 bits) */
    u_int8_t        cd_encoding;    /* name encoding */
    int16_t         cd_namelen;     /* length of cnode name */
    cnid_t          cd_parentcnid;  /* parent directory CNID */
    u_int32_t       cd_hint;        /* catalog file hint */
    cnid_t          cd_cnid;        /* cnode id (for getattrlist) */
    const u_int8_t  *cd_nameptr;    /* pointer to cnode name */
};

/* cd_flags
 *
 * CD_EOF is used by hfs_vnop_readdir / cat_getdirentries to indicate EOF was
 * encountered during a directory enumeration.  When this flag is observed
 * on the next call to hfs_vnop_readdir it tells the caller that there's no
 * need to descend into the catalog as EOF was encountered during the last call.
 * This flag should only be set on the descriptor embedded in the directoryhint.
 */

#define     CD_HASBUF       0x01    /* allocated filename buffer */
#define     CD_DECOMPOSED   0x02    /* name is fully decomposed */
#define     CD_EOF          0x04    /* see above */
#define     CD_ISMETA       0x40    /* describes a metadata file */
#define     CD_ISDIR        0x80    /* describes a directory */

/*
 * Catalog Node Attributes (runtime)
 */
struct cat_attr {
    cnid_t          ca_fileid;      /* inode number (for stat) normally == cnid */
    mode_t          ca_mode;        /* file access mode and type (16 bits) */
    u_int16_t       ca_recflags;    /* catalog record flags (16 bit integer) */
    u_int32_t       ca_linkcount;   /* real hard link count */
    uid_t           ca_uid;         /* file owner */
    gid_t           ca_gid;         /* file group */
    union {
        dev_t       cau_rdev;       /* special file device (VBLK or VCHAR only) */
        u_int32_t   cau_linkref;    /* hardlink reference number */
    } ca_union1;
    time_t        ca_atime;         /* last access time */
    time_t        ca_atimeondisk;   /* access time value on disk */
    time_t        ca_mtime;         /* last data modification time */
    time_t        ca_ctime;         /* last file status change */
    time_t        ca_itime;         /* file initialization time */
    time_t        ca_btime;         /* last backup time */
    u_int32_t    ca_flags;          /* status flags (chflags) */
    union {
        u_int32_t    cau_blocks;    /* total file blocks used (rsrc + data) */
        u_int32_t    cau_entries;   /* total directory entries (valence) */
    } ca_union2;
    union {
        u_int32_t    cau_dircount;  /* count of sub dirs (for posix nlink) */
        u_int32_t    cau_firstlink; /* first hardlink link (files only) */
    } ca_union3;
    union {
        u_int8_t     ca_finderinfo[32]; /* Opaque Finder information */
        struct {
            FndrFileInfo                        ca_finderfileinfo;
            struct FndrExtendedFileInfo         ca_finderextendedfileinfo;
        };
        struct {
            FndrDirInfo                         ca_finderdirinfo;
            struct FndrExtendedDirInfo          ca_finderextendeddirinfo;
        };
    };
};

/* Aliases for common fields */
#define    ca_rdev          ca_union1.cau_rdev
#define    ca_linkref       ca_union1.cau_linkref
#define    ca_blocks        ca_union2.cau_blocks
#define    ca_entries       ca_union2.cau_entries
#define    ca_dircount      ca_union3.cau_dircount
#define    ca_firstlink     ca_union3.cau_firstlink
#define    ca_bsdflags      ca_flags

/*
 * Catalog Node Fork (runtime)
 *
 * NOTE: this is not the same as a struct HFSPlusForkData
 *
 * NOTE: if cf_new_size > cf_size, then a write is in progress and is extending
 * the EOF; the new EOF will be cf_new_size.  Writes and pageouts may validly
 * write up to cf_new_size, but reads should only read up to cf_size.  When
 * an extending write is not in progress, cf_new_size is zero.
 */

struct cat_fork {
    off_t           cf_size;        /* fork's logical size in bytes */
    off_t           cf_new_size;    /* fork's logical size after write completes */
    union {
        u_int32_t   cfu_clump;      /* fork's clump size in bytes (sys files only) */
        u_int64_t   cfu_bytesread;  /* bytes read from this fork */
    } cf_union;
    u_int32_t       cf_vblocks;     /* virtual (unalloated) blocks */
    u_int32_t       cf_blocks;      /* total blocks used by this fork */
    struct HFSPlusExtentDescriptor  cf_extents[8];  /* initial set of extents */

    /*
     * NOTE: If you change this structure, make sure you change you change
     * hfs_fork_copy.
     */
};

#define cf_clump        cf_union.cfu_clump
#define cf_bytesread    cf_union.cfu_bytesread

#define HFS_MAXDIRHINTS 32
#define HFS_DIRHINT_TTL 45

#define HFS_INDEX_MASK  0x03ffffff
#define HFS_INDEX_BITS  26

/* Finder Info's file type and creator for directory hard link alias */
enum {
    kHFSAliasType    = 0x66647270,  /* 'fdrp' */
    kHFSAliasCreator = 0x4D414353   /* 'MACS' */
};

/*
 * Directory Hint
 * Used to hold state across directory enumerations.
 *
 */
struct directoryhint {
    TAILQ_ENTRY(directoryhint) dh_link; /* chain */
    int     dh_index;                   /* index into directory (zero relative) */
    u_int32_t  dh_threadhint;           /* node hint of a directory's thread record */
    u_int32_t  dh_time;
    struct  cat_desc  dh_desc;          /* entry's descriptor */
};
typedef struct directoryhint directoryhint_t;

/*
 * The size of cat_cookie_t must match the size of
 * the nreserve struct (in BTreeNodeReserve.c).
 */
typedef    struct cat_cookie_t {
#if defined(__LP64__)
    char    opaque[40];
#else
    char    opaque[24];
#endif
} cat_cookie_t;

/* Universal catalog key */
union CatalogKey {
    HFSPlusCatalogKey  hfsPlus;
};
typedef union CatalogKey  CatalogKey;

/* Universal catalog data record */
union CatalogRecord {
    int16_t               recordType;
    HFSPlusCatalogFolder  hfsPlusFolder;
    HFSPlusCatalogFile    hfsPlusFile;
    HFSPlusCatalogThread  hfsPlusThread;
};
typedef union CatalogRecord  CatalogRecord;

/*
 * Catalog Node Entry
 *
 * A cat_entry is used for bulk enumerations (hfs_readdirattr).
 */
struct cat_entry {
    struct cat_desc    ce_desc;
    struct cat_attr    ce_attr;
    off_t        ce_datasize;
    off_t        ce_rsrcsize;
    u_int32_t        ce_datablks;
    u_int32_t        ce_rsrcblks;
};

/*
 * Catalog Node Entry List
 *
 * A cat_entrylist is a list of Catalog Node Entries.
 */
struct cat_entrylist {
    u_int32_t  maxentries;    /* number of entries requested */
    u_int32_t  realentries;   /* number of valid entries returned */
    u_int32_t  skipentries;   /* number of entries skipped (reserved HFS+ files) */
    struct cat_entry  entry[1];   /* array of entries */
};

#define CE_LIST_SIZE(entries)    \
            sizeof (*ce_list) + (((entries) - 1) * sizeof (struct cat_entry))


typedef struct cat_preflightid {
    cnid_t fileid;
    LIST_ENTRY(cat_preflightid) id_hash;
} cat_preflightid_t;

void hfs_idhash_init    (struct hfsmount *hfsmp);
void hfs_idhash_destroy (struct hfsmount *hfsmp);

int     cat_binarykeycompare( HFSPlusCatalogKey *searchKey, HFSPlusCatalogKey *trialKey );
int     CompareExtendedCatalogKeys( HFSPlusCatalogKey *searchKey, HFSPlusCatalogKey *trialKey );
void    cat_releasedesc( struct cat_desc *descp );
int     cat_lookup(struct hfsmount *hfsmp, struct cat_desc *descp, int wantrsrc,
                   struct cat_desc *outdescp, struct cat_attr *attrp,
                   struct cat_fork *forkp, cnid_t *desc_cnid);
int     cat_idlookup(struct hfsmount *hfsmp, cnid_t cnid, int allow_system_files, int wantrsrc,
                     struct cat_desc *outdescp, struct cat_attr *attrp, struct cat_fork *forkp);
int     cat_lookupmangled(struct hfsmount *hfsmp, struct cat_desc *descp, int wantrsrc,
                          struct cat_desc *outdescp, struct cat_attr *attrp, struct cat_fork *forkp);
int     cat_findname(struct hfsmount *hfsmp, cnid_t cnid, struct cat_desc *outdescp);
int     cat_getdirentries(struct hfsmount *hfsmp, u_int32_t entrycnt, directoryhint_t *dirhint,
                          ReadDirBuff_s* psReadDirBuffer, int flags, int *items, bool *eofflag, UVFSDirEntry* psDotDotEntry);
int     cat_getentriesattr(struct hfsmount *hfsmp, directoryhint_t *dirhint, struct cat_entrylist *ce_list,
                           int *reachedeof);
bool    IsEntryAJnlFile(struct hfsmount *hfsmp, cnid_t  cnid);
int     cat_preflight(struct hfsmount *hfsmp, uint32_t ops, cat_cookie_t *cookie);
void    cat_postflight(struct hfsmount *hfsmp, cat_cookie_t *cookie);
int     cat_rename ( struct hfsmount * hfsmp, struct cat_desc * from_cdp, struct cat_desc * todir_cdp,
                        struct cat_desc * to_cdp, struct cat_desc * out_cdp );
int     cat_delete(struct hfsmount *hfsmp, struct cat_desc *descp, struct cat_attr *attrp);
int     cat_update(struct hfsmount *hfsmp, struct cat_desc *descp, struct cat_attr *attrp,
                    const struct cat_fork *dataforkp, const struct cat_fork *rsrcforkp);
int     cat_acquire_cnid (struct hfsmount *hfsmp, cnid_t *new_cnid);
int     cat_create(struct hfsmount *hfsmp, cnid_t new_fileid, struct cat_desc *descp, struct cat_attr *attrp, struct cat_desc *out_descp);
int     cat_set_childlinkbit(struct hfsmount *hfsmp, cnid_t cnid);
int     cat_check_link_ancestry(struct hfsmount *hfsmp, cnid_t cnid, cnid_t pointed_at_cnid);

// ------------------------------ Hard-Link Related ------------------------------ 

#define HFS_IGNORABLE_LINK  0x00000001

int     cat_deletelink(struct hfsmount *hfsmp, struct cat_desc *descp);
int     cat_update_siblinglinks(struct hfsmount *hfsmp, cnid_t linkfileid, cnid_t prevlinkid, cnid_t nextlinkid);
int     cat_lookup_lastlink(struct hfsmount *hfsmp, cnid_t linkfileid, cnid_t *lastlink, struct cat_desc *cdesc);
int     cat_resolvelink(struct hfsmount *hfsmp, u_int32_t linkref, int isdirlink, struct HFSPlusCatalogFile *recp);
int     cat_lookuplink(struct hfsmount *hfsmp, struct cat_desc *descp, cnid_t *linkfileid, cnid_t *prevlinkid,  cnid_t *nextlinkid);
void    cat_convertattr(struct hfsmount *hfsmp, CatalogRecord * recp, struct cat_attr *attrp, struct cat_fork *datafp, struct cat_fork *rsrcfp);
int     cat_createlink(struct hfsmount *hfsmp, struct cat_desc *descp, struct cat_attr *attrp, cnid_t nextlinkid, cnid_t *linkfileid);
#endif /* lf_hfs_catalog_h */
