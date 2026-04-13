/* Copyright © 2017-2018 Apple Inc. All rights reserved. */

#ifndef UserVFS_h
#define UserVFS_h

#if TARGET_OS_IPHONE
#include <UserFS/UserVFS_types.h>
#else
#include "UserVFS_types.h"
#endif

#ifndef __clang_tapi__

typedef void * UVFSFileNode;

/*
 * int (*fsops_init)(void);
 * This routine is used to initialize the file system plugin.
 * This will be called once per plugin and can be used to allocate and/or
 * initialize any global data structures used by the plugin.  Return 0 for
 * success or an appropriate errno value on failure.
 */
typedef int (*fsops_init_func_t )(void);

/*
 * void (*fsops_fini)(void);
 *
 * This routine is used to finalize and clean up the file system plugin.
 * Any global data structures that were allocated by the plugin should
 * be released during this call.  This call does not return a value.
 */
typedef void (*fsops_fini_func_t )(void);

/*
 * int (*fsops_taste)(int diskFd);
 *
 * This routine is used to "taste" the file system on the disk corresponding to
 * diskFd to see if this plugin matches the file system on disk.  The plugin
 * should consult the data structures on disk to determine if it understands
 * the on-disk file system format.  Return 0 for success or ENOTSUP if the
 * on-disk format is not supported by this plugin.
 */
typedef int (*fsops_taste_func_t)(int diskFd);

/*
 * int (*fsops_scanvols)(int diskFd, UVFSScanVolsRequest *request, UVFSScanVolsReply *reply);
 *
 * This routine is used to provide information about any number of volumes
 * that a plugin may expose for a given disk.  The plugin should return information
 * about volumes that are present on the given disk one at a time.  The sr_volid field
 * of the UVFSScanVolsRequest structure can be used to determine where on the disk to
 * start iterating for volumes from.  Return 0 if a volume has successfully been found,
 * an errno on error, and UVFS_SCANVOLS_EOF_REACHED if no more volumes are present on the disk.
 */
typedef int (*fsops_scanvols_func_t)(int diskFd, UVFSScanVolsRequest *request, UVFSScanVolsReply *reply);

/*
 * int (*fsops_mount)(int diskFd, UVFSVolumeId volId, UVFSMountFlags mountFlags,
 *                    UVFSVolumeCredential *volumeCreds,
 *                    UVFSFileNode *outRootFileNode);
 *
 * This routine is used to initialize a file system instance for the disk corresponding
 * to diskFd with the volume id volId.  Credentials required to access the file system,
 * if any, are provided in volumeCreds.  The plugin should allocate any in-memory state
 * required to represent the file system and cache the provided diskFd to use for I/O
 * as needed, taking into account any provided mount flags in mountFlags.  The plugin
 * should allocate a file node for the root directory of the file system, associate the
 * file system state with that file node, and return that file node via the outRootFileNode
 * parameter.  The process hosting this plugin will cache the root file node for the lifetime
 * of the file system instance and use it as the jumping off point for all file lookups.
 * Return 0 on success or an appropriate errno value on failure.
 */
typedef int (*fsops_mount_func_t)(int diskFd, UVFSVolumeId volId, UVFSMountFlags mountFlags,
                                  UVFSVolumeCredential *volumeCreds,
                                  UVFSFileNode *outRootFileNode);

/*
 * int (*fsops_sync)(UVFSFileNode node);
 *
 * This routine is used to synchronize and mark clean a previously-initialized file system instance.
 * The plugin should ensure that the file system on disk is consistent (in the case of FAT file systems,
 * for example, ensuring that the secondary FAT copy is consistent with the primary FAT, and that the
 * dirty bit in the volume header is cleared).  The node parameter may be any file node associated with
 * the file system.  Return 0 on success or an appropriate errno value on failure.
 */
typedef int (*fsops_sync_func_t)(UVFSFileNode node);

//* Context hint for unmount */
typedef enum {
    UVFSUnmountHintNone     = 0,
    UVFSUnmountHintForce    = 1,    /* Forced unmount */
} UVFSUnmountHint;

/*
 * int (*fsops_unmount)(UVFSFileNode rootFileNode, UVFSUnmountHint hint);
 *
 * This routine is used to tear down a previously-initialized file system instance.  The plugin should
 * close the previously cached diskFd and release any resources allocated for the file system instance.
 * The process hosting this plugin will guarantee that all other file nodes associated with this file system
 * instance will have been released by a reclaim call.  This routine should not need to perform any I/O;
 * in cases where that is desired, the process hosting this plugin will have already issued a sync call to
 * perform cleanup-related I/O.  The hint parameter indicates additional information about the unmount.
 * Return 0 on success or an appropriate errno value on failure.
 */
typedef int (*fsops_unmount_func_t)(UVFSFileNode rootFileNode, UVFSUnmountHint hint);

/*
 * int (*fsops_getattr)(UVFSFileNode Node, UVFSFileAttributes *outAttrs);
 *
 * This routine is used to fetch the attributes for the file system object
 * referenced by Node.   The plugin should fill in the file attribute buffer
 * referenced by outAttrs.  For each field that is returned by the plugin, the
 * corresponding UVFS_FA_VALID_* bit must be set in outAttrs->fa_validmask.  For each
 * field not supported by the plugin, the corresponding bit in
 * outAttr->fa_validmask must be cleared.  For file systems that do not support
 * hard links, outAttrs->fa_nlinks should be 1 for regular files and symbolic
 * links.  Return 0 for success an appropriate errno value on failure.
 */
typedef int (*fsops_getattr_func_t )(UVFSFileNode Node, UVFSFileAttributes *outAttrs);

/*
 * int (*fsops_setattr)(UVFSFileNode Node, const UVFSFileAttributes *attrs, UVFSFileAttributes *outAttrs);
 *
 * This routine is used to set the attributes for the file system object
 * referenced by Node.  The plugin should consult attrs->fa_validmask to
 * determine which attributes are to be updated.  Note that several attributes are
 * considered to be "read-only", and attempts to set those attributes should
 * result in an error of EINVAL being returned.  If fa_size is set beyond the end
 * of file and the underlying file system does not support sparse files, space to
 * fulfill the new file size must be allocated and zero-filled.  If fa_size is set
 * below the current end of file, the file is truncated and any space no longer
 * required to fulfill the new file size must be returned to the file system as
 * free space.  The new full set of attributes for the file are returned in
 * outAttrs, using the same semantics as the (*fsops_getattr)() call.  If the
 * caller attempts to set an attribute not supported by the on-disk file system
 * format, no error should be returned; instead, that situation will be detected
 * by the upper layers by comparing the bits in attrs->fa_validmask and
 * outAttrs->fa_validmask.  Return 0 on success or an appropriate errno value on
 * failure.
 */
typedef int (*fsops_setattr_func_t )(UVFSFileNode Node, const UVFSFileAttributes *attrs, UVFSFileAttributes *outAttrs);

/*
 * int (*fsops_lookup)(UVFSFileNode dirNode, const char *name, UVFSFileNode *outNode);
 *
 * This routine is used to look up a file in the directory referenced by
 * dirNode.  The name argument will be a NUL-terminated C-string in UTF8.
 * (See the discussion for (*fsops_readdir)()/(*fsops_readdirattr)() for more
 * information about the composition of this string).
 * Upon success, 0 should be returned and a file Node for the file system object
 * found by the lookup should be returned in outNode.  If the entry does not
 * exist, ENOENT should be returned.  An appropriate errno value should be
 * returned on any other kind of failure.
 */
typedef int (*fsops_lookup_func_t  )(UVFSFileNode dirNode, const char *name, UVFSFileNode *outNode);

/*
 * int (*fsops_reclaim)(UVFSFileNode Node);
 *
 * This routine is used to release any resources allocated by the plugin for the
 * file system object referenced by Node.  usbstoraged guarantees that for every
 * file Node returned by the plugin, a corresponding reclaim operation will
 * occur once the upper layers no longer reference that file system object.  The
 * exception is the Node for the root directory; that Node is released by
 * (*fsops_unmount)().  Return 0 on success or an appropriate errno value on failure.
 */
typedef int (*fsops_reclaim_func_t )(UVFSFileNode Node);

/*
 * int (*fsops_readlink)(UVFSFileNode Node, void *outBuf, size_t bufsize, size_t *actuallyRead, UVFSFileAttributes *outAttrs);
 *
 * This routine is used to read the contents of a symbolic link referenced by
 * Node.  The contents are returned in outBuf, the size of which is specified by
 * bufsize.  The returned symbolic link contents should be a NUL-terminated
 * C-string in UTF8.  If the resulting link (including NUL terminator) does not
 * fit, ENOBUFS should be returned.  The number of bytes copied into outBuf
 * should be returned in actuallyRead.  The attributes of the symbolic link
 * should be returned in outAttrs as if (*fsops_getattr)() were called.
 * Return 0 on success, EINVAL if Node does not refer to a symbolic link, ENOBUFS
 * as noted above, or an appropriate errno value on any other kind of failure.
 */
typedef int (*fsops_readlink_func_t)(UVFSFileNode Node, void *outBuf, size_t bufsize, size_t *actuallyRead, UVFSFileAttributes *outAttrs);

/*
 * int (*fsops_read)(UVFSFileNode Node, uint64_t offset, size_t length, void *outBuf, size_t *actuallyRead);
 *
 * This routine is used to read the contents of a regular file referenced by
 * Node, starting at byte offset offset, for a total of length bytes.  The
 * contents are returned in outBuf.  The number of bytes successfully read are
 * returned in actuallyRead.  If the number of bytes requested exceeds the number
 * of bytes available before the end of the file, then only those bytes are
 * returned.  If offset points beyond the last valid byte of the file, the routine
 * succeeds but 0 should be returned in actuallyRead.  Return 0 on success, EISDIR
 * if Node refers to a directory or EINVAL if Node refers to something other
 * than a regular file, or an appropriate errno value on any other kind of
 * failure.
 */
typedef int (*fsops_read_func_t    )(UVFSFileNode Node, uint64_t offset, size_t length, void *outBuf, size_t *actuallyRead);

/*
 * int (*fsops_write)(UVFSFileNode Node, uint64_t offset, size_t length, const void *buf, size_t *actuallyWritten);
 *
 * This routine is used to write the contents of a regular file referenced by
 * Node, starting at byte offset offset, for a total of length bytes.  The
 * contents to be written are provided in buf.  The number of bytes successfully
 * written are returned in actuallyWritten.  This routine is expected to allocate
 * space in the file system to extend the file as necessary.  If the file system
 * runs out of space, but succeeds in writing any part of the requested range, the
 * routine succeeds and actuallyWritten should reflect the number of bytes
 * successfully written before space was exhausted; if no part of the range was
 * successfully written in an out-of-space condition, ENOSPC should be returned.
 * Otherwise, return 0 on success, EISDIR if Node refers to a directory or
 * EINVAL if Node refers to something other than a regular file, or an
 * appropriate errno value on any other kind of failure.
 */
typedef int (*fsops_write_func_t   )(UVFSFileNode Node, uint64_t offset, size_t length, const void *buf, size_t *actuallyWritten);

/*
 * int (*fsops_create)(UVFSFileNode dirNode, const char *name, const UVFSFileAttributes *attrs, UVFSFileNode *outNode);
 *
 * This routine is used to create a regular file in the directory referenced by
 * dirNode with the file name name.  The name argument will be a NUL-terminated
 * C-string in UTF8.  If the target file does not exist, it is created with an
 * initial set of attributes specified by attrs, with the same semantics as the
 * (*fsops_setattr)() routine.  If the target file already exists, no changes are
 * made to the file system and an error of EEXIST is returned.  If no error case
 * above is true, return 0 on success and return a file Node corresponding to the
 * newly created file in outNode.
 */
typedef int (*fsops_create_func_t  )(UVFSFileNode dirNode, const char *name, const UVFSFileAttributes *attrs, UVFSFileNode *outNode);

/*
 * int (*fsops_mkdir)(UVFSFileNode dirNode, const char *name, UVFSFileAttributes *attrs, UVFSFileNode outNode);
 *
 * This routine is used to create a directory in the directory referenced by
 * dirNode with the name name.  The name argument will be a NUL-terminated
 * C-string in UTF8.  If name already exists in the directory referenced by
 * dirNode, EEXIST is returned.  Otherwise, the directory is created with the
 * attributes set to attrs with the same semantics as (*fsops_setattr)().  Return
 * 0 on success or an appropriate errno value on any other kind of failure.
 */
typedef int (*fsops_mkdir_func_t   )(UVFSFileNode dirNode, const char *name, const UVFSFileAttributes *attrs, UVFSFileNode *outNode);

/*
 * int (*fsops_symlink)(UVFSFileNode dirNode, const char *name, const char *contents, const UVFSFileAttributes *attrs, UVFSFileNode *outNode);
 *
 * This routine is used to create a symbolic link in the directory referenced by
 * dirNode with the name name.  The name argument will be a NUL-terminated
 * C-string in UTF8.  If the name already exists in the directory referenced by
 * dirNode, EEXIST is returned.  Otherwise, the symbolic link is created with
 * the attributes set to attrs with the same semantics as (*fsops_setattr)().
 * Return 0 on success or an appropriate errno value on any other kind of failure.
 */
typedef int (*fsops_symlink_func_t )(UVFSFileNode dirNode, const char *name, const char *contents, const UVFSFileAttributes *attrs, UVFSFileNode *outNode);

/*
 * int (*fsops_remove)(UVFSFileNode dirNode, const char *name);
 *
 * This routine is used to remove a file system object other than a directory from
 * the directory referenced by dirNode with the name name.  The name argument
 * will be a NUL-terminated C-string in UTF8.  If this operation removes the
 * last reference to the on-disk structures for that file (i.e. removal of an
 * object with fa_nlink == 1), then the resources associated with the file are
 * released back to the file system for re-use.  Return 0 on success, ENOENT if the
 * entry does not exist in the directory referenced by dirNode, EISDIR if the
 * named item is a directory, or an appropriate errno value on any other kind of
 * failure.
 */
typedef int (*fsops_remove_func_t  )(UVFSFileNode dirNode, const char *name, UVFSFileNode victimNode);

/*
 * int (*fsops_rmdir)(UVFSFileNode dirNode, const char *name);
 *
 * This routine is used to remove a directory from the directory referenced by
 * dirNode with the name name.  The name argument will be a NUL-terminated
 * C-string in UTF8.  The directory to be removed must be empty. All resources
 * associated with the directory shall be released back to the file system for
 * re-use upon completion of this call.  Return 0 on success, ENOENT if the entry
 * does not exist in the directory referenced by dirNode, ENOTDIR if the entry
 * name is not a directory, ENOTEMPTY is the directory to be removed is not empty,
 * or an appropriate errno value on any other kind of failure.
 */
typedef int (*fsops_rmdir_func_t   )(UVFSFileNode dirNode, const char *name);

/*
 * int (*fsops_rename)(UVFSFileNode fromDirNode, UVFSFileNode fromNode, const char *fromName,
 *                     UVFSFileNode toDirNode, UVFSFileNode toNode, const char *toName,
 *                     uint32_t flags);
 *
 * This routine is used to rename a file system object from one path in the file
 * system to another.  The arguments are as follows:
 *
 *     fromDirNode          The directory that currently contains the file system
 *                          object being renamed.
 *
 *     fromNode             The actual file system object being renamed.
 *
 *     fromName             The name within fromDir of the file system object
 *                          being renamed.
 *
 *     toDirNode            The directory that will contain the renamed file
 *                          system object. Note that this *may* be equal to
 *                          fromDirNode.
 *
 *     toNode               The file system object if destination exists and has
 *                          been looked up before.
 *
 *     toName               The new name of the file system object being renamed
 *                          within toDir.
 *
 *     flags                Flags to control the rename operation.  Valid
 *                          flags are LI_RENAME_SWAP and LI_RENAME_EXCL.
 *                          See rename(2) for discussion of these options.
 *
 * NOTE ABOUT fromNode:
 * fromNode *may* or *may not* be NULL when (*fsops_rename)() is called.  If it
 * is *not NULL*, then it means there is currently a valid file handle vended
 * to a client that must remain valid and usable regardless of the success or
 * failure of the rename operation.  Any in-memory state needed to meet this
 * requirement MUST be updated in the fileNode if it is provided.
 *
 * NOTE ABOUT toNode: In case of the toNode been non-null marking
 *                    the state, pointed by toNode, as dead must be done.
 *                    Silly rename strategy of moving allocated space to invalid
 *                    directory name which can be cleaned up during reclaim is encouraged.
 *                    After marking the state as dead, reclaim operation must finish
 *                    without errors.
 *
 * The basic algorithm is as follows:
 *
 *      If there is already an object at the "to" location, ensure the objects are compatible:
 *          -- If the "from" object is not a directory and the "to" object is a directory,
 *             the operation shall fail with EISDIR.
 *          -- If the "from" object is a directory and the "to" object is not a directory,
 *             the operation shall fail with ENOTDIR.
 *
 *      If a file move:
 *          -- If the destination file exists:
 *              -- Remove the destination file.
 *          -- If source and destination are in the same directory:
 *              -- Rewrite name in existing directory entry.
 *          else:
 *              -- Write new entry in destination directory.
 *              -- Clear old directory entry.
 *
 *      If a directory move:
 *          -- If destination directory exists:
 *              -- If destination directory is not empty, the operation shall fail with ENOTEMPTY.
 *              -- Remove the destination directory.
 *          -- If source and destination are in the same directory:
 *              -- Rewrite name in existing directory entry.
 *          else:
 *              -- Be sure the destination is not a child of the source.
 *              -- Write new entry in destination directory.
 *              -- Update "." and ".." in the moved directory.
 *              -- Clear old directory entry.
 */
typedef int (*fsops_rename_func_t  )(UVFSFileNode fromDirNode, UVFSFileNode fromNode, const char *fromName,
                                     UVFSFileNode toDirNode, UVFSFileNode toNode, const char *toName,
                                     uint32_t flags);

/*
 * int (*fsops_link)(UVFSFileNode fromNode, UVFSFileNode toDirNode, const char *toName,
 *                                          UVFSFileAttributes *outFileAttrs, UVFSFileAttributes *outDirAttrs);
 *
 * This routine is used to create a hard link to the file represented by fromNode
 * inside the directory represented by toDirNode using the name toName. The
 * name argument will be a NUL-terminated C-string in UTF8.  If toName already
 * exists in toDirNode, EEXIST should be returned.  If creating the hard link
 * would result in exceeding the maximum number of hard links supported on fromNode,
 * EMLINK should be returned. If the file system does not support creating hard
 * links to the type of file system object represented by fromNode, ENOTSUP should
 * be returned.  Otherwise, return 0 on success or an appropriate errno value on
 * any other kind of failure.
 *
 * Upon success, outFileAttrs and outDirAttrs must contain the new attributes
 * of fromNode and dirDirNode, respectively.
 */
typedef int (*fsops_link_func_t)(UVFSFileNode fromNode, UVFSFileNode toDirNode, const char *toName, UVFSFileAttributes *outFileAttrs, UVFSFileAttributes *outDirAttrs);

/*
 * int (*fsops_readdir)(UVFSFileNode dirNode, void *buf, size_t buflen, uint64_t cookie, size_t *bytes_read, uint64_t *verifier);
 * int (*fsops_readdirattr)(UVFSFileNode dirNode, void *buf, size_t buflen, uint64_t cookie, size_t *bytes_read, uint64_t *verifier);
 *
 * These routines are used to enumerate a directory specified by dirNode.  The
 * directory entry information is packed into the buffer buf of size buflen
 * provided by the caller as an array of UVFSDirEntry (or UVFSDirEntryAttr)
 * structures, which vary in length.  cookie is used to indicate the location
 * within the directory to enumerate from, bytes_read is the amount of bytes of
 * the directory read and stored in buf, and verifier is a mechanism to ensure
 * that the provided cookie is valid for the current version of the directory.
 *
 * The general flow of a readdir is as follows: When an enumeration is started,
 * (*fsops_readdir)() will be called with a cookie of 0 and a *verifier equal to
 * UVFS_DIRCOOKIE_VERIFIER_INITIAL.  A cookie of 0 indicates that enumeration starts
 * at the first entry, and the *verifier value of UVFS_DIRCOOKIE_VERIFIER_INITIAL
 * indicates that the verifier should not be compared.  After packing the initial
 * set of directory entries into the caller's buffer, (*fsops_readdir)() sets
 * *verifier to a non-zero value that reflects the directory's current version,
 * sets *bytes_read to the number of bytes read and stored in buf, and returns 0 to
 * the caller. When next called, (*fsops_readdir)() first compares the value of
 * *verifier with the directory's current version.  If they match, then
 * (*fsops_readdir)() packs the next set of directory entries into the caller's
 * buffer starting with the entry associated with cookie.  If the passed in verifier
 * does not match, then UVFS_READDIR_VERIFIER_MISMATCHED must be returned so that the upper
 * layers know to restart directory enumeration.  If cookie is equal to UVFS_DIRCOOKIE_EOF,
 * then UVFS_READDIR_EOF_REACHED must be returned.  If cookie does not resolve to a valid
 * directory entry, then UVFS_READDIR_BAD_COOKIE must be returned.
 *
 * UVFSDirEntry structures are variable length, to pack them into the buffer as
 * efficiently as possible.  The caller can calculate the size of the directory
 * entry using the provided convenience macro UVFS_DIRENTRY_COPYSIZE(namelen), where
 * namelen is the length of the entry's name in UTF8 not including the terminating NUL
 * (as if the length were calculated with the strlen() routine).  If the entry
 * fits into the caller's buffer, the entry should be stored in the caller's buffer.
 * The convenience macro UVFS_DIRENTRY_RECLEN(namelen) is used to calculate the rounded
 * size of the entry (required to ensure proper alignment of subsequent entries in
 * the caller's buffer).  This value should also be stored in the de_nextrec
 * field.  The last entry stored in the caller's buffer should contain a de_nextrec
 * value of 0.  If the end of the directory is reached, the last entry should have
 * a de_nextcookie value set to UVFS_DIRCOOKIE_EOF; otherwise, de_nextcookie should be
 * set to a valid cookie value for that directory entry.
 *
 * Note: There should always be at least two entries in a directory: "." (an entry
 * corresponding to the current directory) and ".." (an entry representing the
 * parent directory).  These entries are standard UVFS_FA_TYPE_DIR entries.  In the case
 * of the root directory of the file system, "." and ".." have identical contents.
 *
 * In addition to the return values above, return 0 on success or an appropriate
 * errno value for any other kind of failure.  In particular, E2BIG should be returned
 * if there is not enough space in the caller's buffer to store at least one maximum-size
 * directory entry record.
 *
 * The (*fsops_readdirattr)() function is identical to (*fsops_readdir)() with two
 * key differences:
 *
 *  --> It returns UVFSDirEntryAttr records.  These records include the attributes
 *      of the file system object referenced by the directory entry.
 *
 *  --> Records for "." and ".." are not returned in the results.
 *
 * Some care is required when constructing a UVFSDirEntryAttr record:
 *
 *  --> fa_type and fa_fileid MUST be returned.
 *
 *  --> The dea_nameoff member MUST be initialized to UVFS_DIRENTRYATTR_NAMEOFF.
 *
 *  --> The start of the name string in the record MUST be obtained using the
 *      UVFS_DIRENTRYATTR_NAMEPTR() macro AFTER initializing dea_nameoff.
 *
 * The plugin must ensure that names returned as part of UVFSDirEntry (or UVFSDirEntryAttr)
 * structures are acceptable and unambiguous input to all file operations that take names
 * (like (*fsops_lookup)()) without additional normalization. The plugin should also be aware that
 * above UserVFS, Apple presentation layers may normalize filenames to UTF8-D for display purposes.
 */
typedef int (*fsops_readdir_func_t )(UVFSFileNode dirNode, void *buf, size_t buflen, uint64_t cookie, size_t *bytes_read, uint64_t *verifier);

typedef int (*fsops_readdirattr_func_t)(UVFSFileNode dirNode, void *buf, size_t buflen, uint64_t cookie, size_t *bytes_read, uint64_t *verifier);

/*
 * int (*fsops_getfsattr)(UVFSFileNode Node, const char *attr, UVFSFSAttributeValue *val, size_t len, size_t *retlen);
 *
 * This routine is used to query file system information.  It can be called on any valid file
 * system node.  Note that some file system attributes may have different values depending on
 * the type of node passed to (*fsops_getfsattr)().  Not all attributes are supported by all
 * file system types.  For unsupported attributes, return ENOTSUP.
 *
 * Attributes are selected by strings.  Depending on the type of the attribute, the value
 * is returned in a different field of the UVFSFSAttributeValue union.  (*fsops_getfsattr)()
 * should validate that the buffer provided by the caller is of sufficient size for the
 * value being returned:
 *
 *      bool            len >= sizeof(bool)
 *      number          len >= sizeof(uint64_t)
 *      opaque          len >= size of the object to be returned
 *      string          len >= length of the string (in UTF8) plus the NUL terminator
 *
 * If the provided buffer is not large enough, return E2BIG.  In all cases, the actual
 * size required to return the attribute shall be returned in *retlen.
 *
 * See UserVFS_types.h for the list of defined attributes.
 */
typedef int (*fsops_getfsattr_func_t)(UVFSFileNode Node, const char *attr, UVFSFSAttributeValue *val, size_t len, size_t *retlen);

/*
 * int (*fsops_setfsattr)(UVFSFileNode Node, const char *attr, const UVFSFSAttributeValue *val, size_t len);
 *
 * This routine is used to query file system information.  It can be called on any valid file
 * system node.  The attributes are as defined for (*fsops_getfsattr)().
 *
 * This function slot is reserved for future use; all current implementations should return
 * ENOTSUP.
 */
typedef int (*fsops_setfsattr_func_t)(UVFSFileNode Node, const char *attr, const UVFSFSAttributeValue *val, size_t len, UVFSFSAttributeValue *out_value, size_t out_len);

//* Enumeration which controls how the filesystem check is actually performed */
typedef enum {
    INVALID          = 0,
    QUICK_CHECK      = 1,   /* Perform quick check, returning 0 if filesystem was cleanly unmounted, or 1 otherwise */
    CHECK            = 2,   /* Perform full check but no repairs. Return 0 if the filesystem is consistent, or appropriate errno otherwise */
    CHECK_AND_REPAIR = 3,   /* Perform full check and carry out any necessary repairs to make filesystem consistent */
} check_flags_t;

/*
 * int (*fsops_check_func_t)(int diskFd, UVFSVolumeId volId,
 *                           UVFSVolumeCredential *volumeCreds,
 *                           check_flags_t how);
 *
 * This routine is used to check and potentially repair a file system
 * hosted on the disk represented by the diskFd parameter at the volume index
 * volId.  Credentials necessary to access this file system, if any,
 * are provided in volumeCreds.
 *
 * The how parameter dictates which checks to perform and what to do if
 * any inconsistencies are found during these checks.
 *
 * 0 is returned upon success, and an appropriate errno value is returned upon failure.
 *
 * Note: It's expected that an error during QUICK_CHECK will trigger CHECK_AND_REPAIR
 * for now.
 *
 */
typedef int (*fsops_check_func_t)(int diskFd, UVFSVolumeId volId,
								  UVFSVolumeCredential *volumeCreds,
								  check_flags_t how);

/*
 * int (*fsops_getxattr_func_t)(UVFSFileNode node, const char *attr, void *buf,
 *                              size_t bufsize, size_t *actual_size);
 *
 * This routine fetches the extended attribute named by "attr" associated with
 * the file system object "node" and stores it in the buffer "buf". The size of
 * "buf" is specified by "bufsize".  The actual size of the attribute is returned
 * in "*actual_size".
 *
 * If the file system does not support extended attributes, return ENOTSUP.
 *
 * This interface does not support extended data that may be in the same
 * namespace but that has stream semantics, for example "com.apple.ResourceFork".
 * Attempts to access this kind of extended data should return ENOATTR.
 *
 * If the attribute specified does not exist, return ENOATTR.
 *
 * If the attribute name is longer than XATTR_MAXNAMELEN (defined in <sys/xattr.h>),
 * return ENAMETOOLONG.
 *
 * If the attribute is found, the actual size of the extended attribute is always
 * returned in "*actual_size".
 *
 * If the caller passes NULL for "buf", return 0 after storing the attribute
 * size.  Otherwise, if the buffer is too small, return ERANGE.  Otherwise,
 * copy the attribute value into the buffer and return 0.  If some other error
 * occurs, return an appropriate errno to indicate the mode of failure.
 */
typedef int (*fsops_getxattr_func_t)(UVFSFileNode node, const char *attr,
                                     void *buf, size_t bufsize,
                                     size_t *actual_size);

/*
 * int (*fsops_setxattr_func_t)(UVFSFileNode node, const char *attr, const void *buf,
 *                              size_t bufsize, UVFSXattrHow how);
 *
 * This routine sets the extended attribute named by "attr" associated with
 * the file system object "node" with the value specified by the buffer "buf"
 * that is "buflen" in length.
 *
 * If the file system does not support extended attributes, return ENOTSUP.
 *
 * This interface does not support extended data that may be in the same
 * namespace but that has stream semantics, for example "com.apple.ResourceFork".
 * Attempts to manipulate this kind of extended data should return ENOTSUP.
 *
 * If the attribute name is longer than XATTR_MAXNAMELEN (defined in <sys/xattr.h>),
 * return ENAMETOOLONG.
 *
 * The behavior of the setxattr function is controlled by "how":
 *
 *  - UVFSXattrHowSet -- set the attribute, creating or replacing as needed.
 *  - UVFSXattrHowCreate -- create the attribute; fail if it already exists
 *    (returning EEXIST).
 *  - UVFSXattrHowReplace -- replace the attribute; fail if it does not already
 *    exist (return ENOATTR).
 *  - UVFSXattrHowRemove -- remove the attribute; return ENOATTR if it does not
 *    exist.
 *
 * If UVFSXattrHowRemove is specified, then "buf" should be NULL.
 *
 * Upon success, return 0.  Otherwise, return an appropriate errno value to
 * indicate the mode of failure.
 */
typedef enum {
    UVFSXattrHowSet     = 0,
    UVFSXattrHowCreate  = 1,
    UVFSXattrHowReplace = 2,
    UVFSXattrHowRemove  = 3
} UVFSXattrHow;
typedef int (*fsops_setxattr_func_t)(UVFSFileNode node, const char *attr,
                                     const void *buf, size_t bufsize,
                                     UVFSXattrHow how);

/*
 * int (*fsops_listxattr_func_t)(UVFSFileNode node, void *buf, size_t bufsize,
 *                               size_t *actual_size);
 *
 * This routine returns a list of the extended attribute associated with the
 * file system object "node".  The list of names is returned in the buffer
 * specified by "buf" which has size "bufsize".
 *
 * The returned extended attribute names are standard NUL-terminated UTF-8D
 * strings packed one after another, e.g.:
 *
 *      some.attrname\0another.attrname\0
 *
 * If the file system supports stream-oriented extended data in the same
 * namespace as extended attributes, those names should be excluded from
 * the list of returned names.
 *
 * The number of bytes required to return the entire list (including the
 * final terminating NUL byte) is stored in "*actual_size".  If "buf" is NULL,
 * return 0 after storing the required buffer size.  Otherwise, if the buffer
 * provided by the caller is too small to return all of the attribute names,
 * return ERANGE.  For any other error, return an appropriate errno value to
 * indicate the mode of failure.
 */
typedef int (*fsops_listxattr_func_t)(UVFSFileNode node, void *buf,
                                      size_t bufsize, size_t *actual_size);

/*
 * Each plugin must declare their fsOps table as a global variable as
 *
 *      UVFSFSOps userfs_plugin_fsOps = {
 *          .fsops_version      =       FSOPS_VERSION_CURRENT;
 *          .
 *          .
 *          .
 *      };
 *
 * and implement livefiles_plugin_init(), which will be looked up by userfsd using dlsym(3).
 */
typedef struct _UVFSFSOps {
    uint64_t                fsops_version;

    fsops_init_func_t       fsops_init;
    fsops_fini_func_t       fsops_fini;

    fsops_taste_func_t      fsops_taste;
    fsops_mount_func_t      fsops_mount;
    fsops_sync_func_t       fsops_sync;
    fsops_unmount_func_t    fsops_unmount;

    fsops_getfsattr_func_t  fsops_getfsattr;
    fsops_setfsattr_func_t  fsops_setfsattr;
    
    fsops_getattr_func_t    fsops_getattr;
    fsops_setattr_func_t    fsops_setattr;
    fsops_lookup_func_t     fsops_lookup;
    fsops_reclaim_func_t    fsops_reclaim;
    fsops_readlink_func_t   fsops_readlink;
    fsops_read_func_t       fsops_read;
    fsops_write_func_t      fsops_write;
    fsops_create_func_t     fsops_create;
    fsops_mkdir_func_t      fsops_mkdir;
    fsops_symlink_func_t    fsops_symlink;
    fsops_remove_func_t     fsops_remove;
    fsops_rmdir_func_t      fsops_rmdir;
    fsops_rename_func_t     fsops_rename;
    fsops_readdir_func_t    fsops_readdir;
    fsops_readdirattr_func_t fsops_readdirattr;
    fsops_link_func_t       fsops_link;
    fsops_check_func_t      fsops_check;

    fsops_getxattr_func_t   fsops_getxattr;
    fsops_setxattr_func_t   fsops_setxattr;
    fsops_listxattr_func_t  fsops_listxattr;
    fsops_scanvols_func_t   fsops_scanvols;
} UVFSFSOps;

#define UVFS_FSOPS_VERSION_CURRENT       0x0000000100000000ULL       // "1.0"

typedef void (*livefiles_plugin_init_t)(UVFSFSOps **ops);

#endif /* __clang_tapi__ */

#endif /* UserVFS_h */
