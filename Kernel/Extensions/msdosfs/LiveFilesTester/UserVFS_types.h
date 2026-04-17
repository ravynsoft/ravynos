//
//  UserVFS_types.h
//  UserFS
//
//  Created by Jason Thorpe on 11/30/17.
//  Copyright © 2017 Apple Inc. All rights reserved.
//

#ifndef UserVFS_types_h
#define UserVFS_types_h

#ifndef __clang_tapi__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <time.h>

typedef struct _UVFSFileAttributes {
    uint64_t        __fa_rsvd0;     // reserved for future use
    uint64_t        fa_validmask;   // mask of valid fields; see below
    uint32_t        fa_type;        // see FA_TYPE constants below (read-only)
    uint32_t        fa_mode;        // see FA_MODE bits below
    uint32_t        fa_nlink;       // number of hard-links / directory references (read-only)
    uint32_t        fa_uid;         // owning UNIX user ID
    uint32_t        fa_gid;         // owning UNIX group ID
    uint32_t        fa_bsd_flags;   // BSD file flags
    uint64_t        fa_size;        // logical size of the file, in bytes
    uint64_t        fa_allocsize;   // actual allocated size of the file, in bytes (read-only)
    uint64_t        fa_fileid;      // unique ID of the file (e.g. inode number) (read-only)
    uint64_t        fa_parentid;    // unique ID of the file's parent (e.g. inode number) (read-only)
    struct timespec fa_atime;       // time of last access (relative to UNIX epoch)
    struct timespec fa_mtime;       // time of last modification (relative to UNIX epoch)
    struct timespec fa_ctime;       // time of last attribute change (relative to UNIX epoch) (read-only)
    struct timespec fa_birthtime;   // time of file creation ("birthtime")
    struct timespec fa_backuptime;  // time of last backup
    struct timespec fa_addedtime;   // directory entry creation time (HFS-centric)
} UVFSFileAttributes;

// fa_validmask bits
#define UVFS_FA_VALID_TYPE          (1ULL << 0)
#define UVFS_FA_VALID_MODE          (1ULL << 1)
#define UVFS_FA_VALID_NLINK         (1ULL << 2)
#define UVFS_FA_VALID_UID           (1ULL << 3)
#define UVFS_FA_VALID_GID           (1ULL << 4)
#define UVFS_FA_VALID_BSD_FLAGS     (1ULL << 5)
#define UVFS_FA_VALID_SIZE          (1ULL << 6)
#define UVFS_FA_VALID_ALLOCSIZE     (1ULL << 7)
#define UVFS_FA_VALID_FILEID        (1ULL << 8)
#define UVFS_FA_VALID_PARENTID      (1ULL << 9)
#define UVFS_FA_VALID_ATIME         (1ULL << 10)
#define UVFS_FA_VALID_MTIME         (1ULL << 11)
#define UVFS_FA_VALID_CTIME         (1ULL << 12)
#define UVFS_FA_VALID_BIRTHTIME     (1ULL << 13)
#define UVFS_FA_VALID_BACKUPTIME    (1ULL << 14)
#define UVFS_FA_VALID_ADDEDTIME     (1ULL << 15)

// fa_type values
#define UVFS_FA_TYPE_UNKNOWN        0       // unknown type
#define UVFS_FA_TYPE_FILE           1       // regular file
#define UVFS_FA_TYPE_DIR            2       // directory
#define UVFS_FA_TYPE_SYMLINK        3       // symbolic link
#define UVFS_FA_TYPE_FIFO           4       // named pipe (fifo)
#define UVFS_FA_TYPE_CHAR           5       // character special
#define UVFS_FA_TYPE_BLOCK          6       // block special
#define UVFS_FA_TYPE_SOCKET         7       // socket

// fa_mode values
#define UVFS_FA_MODE_X              0000001       // X bit (execute)
#define UVFS_FA_MODE_W              0000002       // W bit (write)
#define UVFS_FA_MODE_R              0000004       // R bit (read)
#define UVFS_FA_MODE_RWX            0000007       // mask of all RWX bits

#define UVFS_FA_MODE_OTH(bits)      ((bits))      // RWX bits for "other"
#define UVFS_FA_MODE_GRP(bits)      ((bits) << 3) // RWX bits for "group"
#define UVFS_FA_MODE_USR(bits)      ((bits) << 6) // RWX bits for "user"

#define UVFS_FA_MODE_SUID           0004000       // set user ID on execution
#define UVFS_FA_MODE_SGID           0002000       // set group ID on execution
#define UVFS_FA_MODE_SVTX           0001000       // directory restricted delete ("sticky" bit)

// The UVFSDirEntry structure represents a single entry in a directory.
typedef struct _UVFSDirEntry {
    uint64_t        de_fileid;          // file ID of file described by entry
    uint64_t        de_nextcookie;      // directory entry offset cookie of NEXT entry
    uint16_t        de_nextrec;         // offset to next record (0 if no more in current buffer)
#define de_reclen   de_nextrec
    uint16_t        de_namelen;         // length of name (not including NUL-terminator)
    uint8_t         de_filetype;        // UVFS_FA_TYPE_* of the corresponding file system object
    char            de_name[];          // NUL-terminated name (variable length)
} UVFSDirEntry;

#define UVFS_DIRENTRY_COPYSIZE(namelen)     (offsetof(UVFSDirEntry, de_name) + 1 + (namelen))
#define UVFS_DIRENTRY_RECLEN(namelen)       ((UVFS_DIRENTRY_COPYSIZE(namelen) + 7) & ~7)

// The UVFSDirEntryAttr structure represents a single entry in a directory along with
// the attributes corresponding to the file system object represented by the entry.
// This structure is designed to allow the UVFSFileAttributes structure to grow over time,
// and thus attributes should NOT be accessed unless their corresponding bit in the bitmask
// is set.
typedef struct _UVFSDirEntryAttr {
    uint64_t            dea_nextcookie; // directory entry offset cookie of NEXT entry
    uint16_t            dea_nextrec;    // offset to next record (0 if no more in current buffer)
    uint16_t            dea_nameoff;    // offset (from beginning of record) to name NUL-terminated name
    uint16_t            dea_namelen;    // length of name (not including NUL-terminator)
    uint16_t            dea_spare0;
    UVFSFileAttributes  dea_attrs;      // attributes for this entry's FS object; subsumes
                                        // de_fileid as well as de_filetype; these fields
                                        // **must** be valid in the returned attributes.
                                        // ONLY ACCESS ATTRIBUTES THAT HAVE A VALID BIT IN
                                        // fa_validmask!
    // The name string comes AFTER the UVFSFileAttributes, and must only be accessed by
    // using the UVFS_DIRENTRYATTR_NAMEPTR() macro.  DO NOT USE THIS STRUCTURE MEMBER
    // TO ACCESS THE NAME.
    char                _dea_name_placeholder_[];
} UVFSDirEntryAttr;

// UVFS_DIRENTRYATTR_NAMEOFF is used only when constructing a UVFSDirEntryAttr.  It
// should never be used as an accessor.
#define UVFS_DIRENTRYATTR_NAMEOFF       (offsetof(UVFSDirEntryAttr, _dea_name_placeholder_))

#define UVFS_DIRENTRYATTR_NAMEPTR(dea)  (((char *)(dea)) + (dea)->dea_nameoff)
#define _UVFS_DIRENTRYATTR_COPYSIZE(nameoff, namelen) \
                                        ((nameoff) + (namelen) + 1)
#define UVFS_DIRENTRYATTR_COPYSIZE(dea, namelen) \
                                        _UVFS_DIRENTRYATTR_COPYSIZE((dea)->dea_nameoff, (namelen))
#define _UVFS_DIRENTRYATTR_RECLEN(nameoff, namelen) \
                                        ((_UVFS_DIRENTRYATTR_COPYSIZE((nameoff), (namelen)) + 7) & ~7)
#define UVFS_DIRENTRYATTR_RECLEN(dea, namelen) \
                                        _UVFS_DIRENTRYATTR_RECLEN((dea)->dea_nameoff, (namelen))

// Special dircookie values used by ReadDir and ReadDirWithAttrs.
#define UVFS_DIRCOOKIE_VERIFIER_INITIAL     ( 0ULL)
#define UVFS_DIRCOOKIE_EOF                  (~0ULL)

// Special error return codes used by ReadDir and ReadDirWithAttrs.
#define UVFS_READDIR_VERIFIER_MISMATCHED    (-1000)
#define UVFS_READDIR_EOF_REACHED            (-1001)
#define UVFS_READDIR_BAD_COOKIE             (-1002)

#if 0 /* Not Yet */
//
// Search results are similar to directory listings, but are more complicated as the results
// almost invariably will be located within different parent directories. To handle this, search
// results are returned in a stream, and this stream maintains a path cursor. All returned
// results are relative to this path cursor, and some return results modify the path cursor.
//
// The search results are returned in one or more buffers. Exactly how these results are propagated
// depends on the API level performing the search. At the UVFS layer, each buffer contains one or more
// complete result objects; result objects may NOT be split across multiple buffers. As such, a receiver
// must always be able to receive one response with a maximal-length name.
//
// The UVFSSearchEntry structure represents a single entry in the result stream. Almost all entries
// are of type UVFSSearchEntryAttr, a subtype of UVFSSearchEntry which also includes a name and file
// attributes.
//
// Before proceeding, consider a search which returns two objects having the following paths:
//          /usr/include/stdio.h
//          /usr/lib/libSystem.B.dylib
//
// Further consider the buffering is sized such that only two entries are returned
// in each buffer. The resulting search buffers would be:
//
// Buffer #     Entry Type                  Name or sea_option
//     1      UVFS_SEARCHENTRY_PUSHNAME     usr
//     1      UVFS_SEARCHENTRY_PUSHNAME     include
//
//     2      UVFS_SEARCHENTRY_RESULT       stdio.h
//     2      UVFS_SEARCHENTRY_POPNAMES     1
//
//     3      UVFS_SEARCHENTRY_PUSHNAME     lib
//     3      UVFS_SEARCHENTRY_RESULT       libSystem.B.dylib
//
// The search maintains the path cursor as a stack of directories which are the
// parent of any returned results. Any result returned with an sea_type of UVFS_SEARCHENTRY_RESULT
// is an object in the cursor's directory which matches the search criteria; it is a
// positive result of the search. A result returned with an sea_type of UVFS_SEARCHENTRY_PUSHNAME
// is NOT a result of the search but instead is an object which will parent future search
// results. It updates the cursor, and is only valid for objects of type UVFS_FA_TYPE_DIR.
// A result returned with an sea_type of UVFS_SEARCHENTRY_RESULTANDPUSH combines both operations;
// it is a directory object which matches the search and which also updates the cursor. The fourth
// type of object in the stream is of sea_type of UVFS_SEARCHENTRY_POPNAMES, which removes
// sea_option entries from the cursor directory stack. Such an operation is comparable to
// a repeated invocation of "cd .." in the working directory of a UNIX shell.
//
// All entries in the search result stream have a valid sea_type and sea_nextrec pointing to
// the next record. These values are eight-byte aligned, and the special value of 0 indicates no
// more entries are in this buffer. Entries of sea_type of UVFS_SEARCHENTRY_POPNAMES are
// only eight bytes long. All other entries have a valid UVFSFileAttributes, and have a
// NUL-terminated name pointed to by sea_nameoff.
//
// The cursor is maintained for the lifetime of the search; it remains valid and unchanged
// as one buffer ends and another begins to propagate results. The cursor is not reset anew
// for each buffer.
//
// This structure is designed to allow the UVFSFileAttributes structure to grow over time,
// and thus attributes should NOT be accessed unless their corresponding bit in the bitmask
// is set.
#define UVFS_SEARCHENTRY_RESULT         0   // entry is a UVFSSearchEntryAttr result
#define UVFS_SEARCHENTRY_RESULTANDPUSH  1   // entry is a UVFSSearchEntryAttr and name is pushed
                                            // onto path cursor. Entry must be a directory
#define UVFS_SEARCHENTRY_PUSHNAME       2   // Entry is not a match but a UVFSSearchEntryAttr
                                            // to push to the path cursor
#define UVFS_SEARCHENTRY_POPNAMES       3   // pop _se_option entries off of path cursor
typedef struct _UVFSSearchEntry {
    uint16_t            _se_type:3,     // entry type
                        _se_option:13;  // option value for certain entries
    uint16_t            _se_nextrec;    // offset to next record (0 if no more in current buffer)
    uint16_t            _se_nameoff;    // offset (from beginning of record) to name NUL-terminated name
    uint16_t            _se_namelen;    // length of name (not including NUL-terminator)
} UVFSSearchEntry;
typedef struct _UVFSSearchEntryAttr {
    UVFSSearchEntry     _se;
#define sea_type        _se.se_type     // entry type
#define sea_option      _se.se_option   // option value for certain entries
#define sea_nextrec     _se.se_nextrec  // offset to next record (0 if no more in current buffer)
#define sea_nameoff     _se.se_nameoff  // offset (from beginning of record) to name NUL-terminated name
#define sea_namelen     _se.se_namelen  // length of name (not including NUL-terminator)
    UVFSFileAttributes  sea_attrs;      // attributes for this entry's FS object; subsumes
    // de_fileid as well as de_filetype; these fields
    // **must** be valid in the returned attributes.
    // ONLY ACCESS ATTRIBUTES THAT HAVE A VALID BIT IN
    // fa_validmask!
    // The name string comes AFTER the UVFSFileAttributes, and must only be accessed by
    // using the UVFS_DIRENTRYATTR_NAMEPTR() macro.  DO NOT USE THIS STRUCTURE MEMBER
    // TO ACCESS THE NAME.
    char                _sea_name_placeholder_[];
} UVFSSearchEntryAttr;

// UVFS_SEARCHENTRYATTR_NAMEOFF is used only when constructing a UVFSSearchEntryAttr.  It
// should never be used as an accessor.
#define UVFS_SEARCHENTRYATTR_NAMEOFF       (offsetof(UVFSSearchEntryAttr, _sea_name_placeholder_))

#define UVFS_SEARCHENTRYATTR_NAMEPTR(sea)  (((char *)(sea)) + (sea)->sea_nameoff)
#define _UVFS_SEARCHENTRYATTR_COPYSIZE(nameoff, namelen) \
((nameoff) + (namelen) + 1)
#define UVFS_SEARCHENTRYATTR_COPYSIZE(sea, namelen) \
_UVFS_SEARCHENTRYATTR_COPYSIZE((sea)->sea_nameoff, (namelen))
#define _UVFS_SEARCHENTRYATTR_RECLEN(nameoff, namelen) \
((_UVFS_SEARCHENTRYATTR_COPYSIZE((nameoff), (namelen)) + 7) & ~7)
#define UVFS_SEARCHENTRYATTR_RECLEN(sea, namelen) \
_UVFS_SEARCHENTRYATTR_RECLEN((sea)->sea_nameoff, (namelen))
#endif /* Not Yet */

//
// UVFS_FSATTR_PC_LINK_MAX (number) [required]
// The maximum number of hard links for the file system object referenced by Node.
// Note this value may be different for files vs. directories.  If the file system
// does not support hard links, return the value 1.
//
// UVFS_FSATTR_PC_NAME_MAX (number) [required]
// The maximum file name length in BYTES (not characters).
//
// UVFS_FSATTR_PC_NO_TRUNC (bool) [required]
// True if file name components longer than the value of UVFS_FSATTR_PC_NAME_MAX
// will result in an ENAMETOOLONG error; false if the name is silently truncated.
//
// UVFS_FSATTR_PC_FILESIZEBITS (number) [required]
// The number of bits used to represent the size (in bytes) of a file.
//
// UVFS_FSATTR_PC_XATTR_SIZE_BITS (number)
// The number of bits used to represent the size (in bytes) of an extended
// attribute.
//
// UVFS_FSATTR_BLOCKSIZE (number) [required]
// Size (in bytes) of a fundamental file system block.
//
// UVFS_FSATTR_IOSIZE (number) [required]
// Size (in bytes) of the optimal transfer block size.
//
// UVFS_FSATTR_TOTALBLOCKS (number) [required]
// Total number of file system blocks.
//
// UVFS_FSATTR_BLOCKSFREE (number) [required]
// The number of free file system blocks.
//
// UVFS_FSATTR_BLOCKSAVAIL (number) [required]
// The number of file system blocks available for allocation to files.
// (This is usually the same as BLOCKSFREE.)
//
// UVFS_FSATTR_BLOCKSUSED (number) [required]
// The number of file system blocks currently allocated for some use.
// (This is usually TOTALBLOCKS - BLOCKSAVAIL.)
//
// UVFS_FSATTR_FSTYPENAME (string) [required]
// A string representing the type of the file system, e.g. "FAT" or "ExFAT".
//
// UVFS_FSATTR_FSSUBTYPE (string)
// A string representing the variant of the file system, e.g. "FAT12", "FAT16", or "FAT32".
//
// UVFS_FSATTR_VOLNAME (string)
// The volume's name.
//
// UVFS_FSATTR_VOLUUID (opaque)
// The volume's UUID (as a uuid_t).
//
// UVFS_FSATTR_CAPS_FORMAT (number) [required]
// A bitmask indicating the capabilities of the volume format.
// See VOL_CAP_FMT_// bits in <sys/attr.h>.
//
// UVFS_FSATTR_CAPS_INTERFACES (number) [required]
// A bitmask indicating the interface capabilities of the file system.
// See VOL_CAP_INT_* bits in <sys/attr.h>.
//

typedef union {
    bool            fsa_bool;           // boolean values
    uint64_t        fsa_number;         // number values
    uint8_t         fsa_opaque[0];      // opaque values (variable length)
    char            fsa_string[0];      // string values (variable length, UTF8, NUL-terminated)
} UVFSFSAttributeValue;

#define UVFS_FSATTR_IS_BOOL(attr)           ((attr)[0] == '_' && (attr)[1] == 'B')
#define UVFS_FSATTR_IS_NUMBER(attr)         ((attr)[0] == '_' && (attr)[1] == 'N')
#define UVFS_FSATTR_IS_OPAQUE(attr)         ((attr)[0] == '_' && (attr)[1] == 'O')
#define UVFS_FSATTR_IS_STRING(attr)         ((attr)[0] == '_' && (attr)[1] == 'S')

#define UVFS_FSATTR_PC_LINK_MAX             "_N_PC_LINK_MAX"            // number
#define UVFS_FSATTR_PC_NAME_MAX             "_N_PC_NAME_MAX"            // number
#define UVFS_FSATTR_PC_NO_TRUNC             "_B_PC_NO_TRUNC"            // bool
// _PC_CASE_SENSITIVE is covered by VOL_CAP_FMT_CASE_SENSITIVE
// _PC_CASE_PRESERVING is covered by VOL_CAP_FMT_CASE_PRESERVING
#define UVFS_FSATTR_PC_FILESIZEBITS         "_N_PC_FILESIZEBITS"        // number
#define UVFS_FSATTR_PC_XATTR_SIZE_BITS      "_N_PC_XATTR_SIZE_BITS"     // number

#define UVFS_FSATTR_BLOCKSIZE               "_N_f_bsize"                // number
#define UVFS_FSATTR_IOSIZE                  "_N_f_iosize"               // number
#define UVFS_FSATTR_TOTALBLOCKS             "_N_f_blocks"               // number
#define UVFS_FSATTR_BLOCKSFREE              "_N_f_bfree"                // number
#define UVFS_FSATTR_BLOCKSAVAIL             "_N_f_bavail"               // number
#define UVFS_FSATTR_BLOCKSUSED              "_N_f_bused"                // number
#define UVFS_FSATTR_FSTYPENAME              "_S_f_type"                 // string
#define UVFS_FSATTR_FSSUBTYPE               "_S_f_subtype"              // string
#define UVFS_FSATTR_FSLOCATION              "_S_f_location"             // string
#define UVFS_FSATTR_VOLNAME                 "_S_f_vol_name"             // string
#define UVFS_FSATTR_VOLUUID                 "_O_f_uuid"                 // opaque

#define UVFS_FSATTR_CAPS_FORMAT             "_N_caps_format"            // number
#define UVFS_FSATTR_CAPS_INTERFACES         "_N_caps_interfaces"        // number

typedef uint32_t UVFSMountFlags;
#define UVFS_MOUNT_RDONLY         (1U << 0)

typedef uint64_t UVFSVolumeId; // volume identifier within a disk

typedef struct _UVFSScanVolsRequest
{
    UVFSVolumeId sr_volid; // start iterating volumes at this volume ID
} UVFSScanVolsRequest;

typedef enum {
    UAC_UNLOCKED = 0,
    UAC_ENCRYPTED_REQUIRES_PASSWORD,
    UAC_ENCRYPTED_ROLLING_REQUIRES_PASSWORD
} UVFSAccessControl;

#define UVFS_SCANVOLS_VOLNAME_MAX 256 // with NUL-terminator

typedef struct _UVFSScanVolsReply
{
    UVFSVolumeId sr_volid; // ID of this volume, also next ID to look at
    UVFSAccessControl sr_volac; // access control type required
    char sr_volname[UVFS_SCANVOLS_VOLNAME_MAX]; // name of this volume, optional
} UVFSScanVolsReply;

#define UVFS_SCANVOLS_EOF_REACHED -1001

typedef struct _UVFSVolumeCredential
{
     uint64_t vc_validmask;
     // will expand when we implement crypto
} UVFSVolumeCredential;

#endif /* __clang_tapi__ */

#endif /* UserVFS_types_h */
