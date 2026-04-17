/*
 * Copyright (c) 2000-2015 Apple Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 *
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */
#if CONFIG_PROTECT

#include <sys/mount.h>
#include <sys/random.h>
#include <sys/xattr.h>
#include <sys/vnode_if.h>
#include <sys/fcntl.h>
#include <libkern/OSByteOrder.h>
#include <libkern/crypto/sha1.h>
#include <sys/proc.h>
#include <sys/kauth.h>
#include <sys/sysctl.h>
#include <sys/ubc.h>
#include <uuid/uuid.h>

#include "hfs.h"
#include "hfs_cnode.h"
#include "hfs_fsctl.h"
#include "hfs_cprotect.h"
#include "hfs_iokit.h"

#if HFS_CONFIG_KEY_ROLL
#include "hfs_key_roll.h"
#endif

#define PTR_ADD(type, base, offset)		(type)((uintptr_t)(base) + (offset))

extern int (**hfs_vnodeop_p) (void *);

/*
 * CP private functions
 */
static int cp_root_major_vers(mount_t mp);
static int cp_getxattr(cnode_t *, struct hfsmount *hfsmp, struct cprotect **);
static void cp_entry_dealloc(hfsmount_t *hfsmp, struct cprotect *entry);
static int cp_restore_keys(struct cprotect *, struct hfsmount *hfsmp, struct cnode *);
static int cp_lock_vnode_callback(vnode_t, void *);
static int cp_vnode_is_eligible (vnode_t);
static int cp_check_access (cnode_t *cp, struct hfsmount *hfsmp, int vnop);
static int cp_unwrap(struct hfsmount *, struct cprotect *, struct cnode *);
static void cp_init_access(aks_cred_t access, struct cnode *cp);

// -- cp_key_pair accessors --

void cpkp_init(cp_key_pair_t *cpkp, uint16_t max_pers_key_len,
			   uint16_t max_cached_key_len)
{
	cpkp->cpkp_max_pers_key_len = max_pers_key_len;
	cpkp->cpkp_pers_key_len = 0;

	cpx_t embedded_cpx = cpkp_cpx(cpkp);
	/* XNU requires us to allocate the AES context separately */
	cpx_alloc_ctx (embedded_cpx);

	cpx_init(cpkp_cpx(cpkp), max_cached_key_len);

	// Default to using offsets
	cpx_set_use_offset_for_iv(cpkp_cpx(cpkp), true);
}

uint16_t cpkp_max_pers_key_len(const cp_key_pair_t *cpkp)
{
	return cpkp->cpkp_max_pers_key_len;
}

uint16_t cpkp_pers_key_len(const cp_key_pair_t *cpkp)
{
	return cpkp->cpkp_pers_key_len;
}

static bool cpkp_has_pers_key(const cp_key_pair_t *cpkp)
{
	return cpkp->cpkp_pers_key_len > 0;
}

static void *cpkp_pers_key(const cp_key_pair_t *cpkp)
{
	return PTR_ADD(void *, &cpkp->cpkp_cpx, cpx_sizex(cpkp_cpx(cpkp)));
}

static void cpkp_set_pers_key_len(cp_key_pair_t *cpkp, uint16_t key_len)
{
	if (key_len > cpkp->cpkp_max_pers_key_len)
		panic("hfs_cprotect: key too big!");
	cpkp->cpkp_pers_key_len = key_len;
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcast-qual"
cpx_t cpkp_cpx(const cp_key_pair_t *cpkp)
{
	// Cast to remove const qualifier
	return (cpx_t)&cpkp->cpkp_cpx;
}
#pragma clang diagnostic pop

size_t cpkp_size(uint16_t pers_key_len, uint16_t cached_key_len)
{
	return sizeof(cp_key_pair_t) + pers_key_len + cpx_size(cached_key_len);
}

size_t cpkp_sizex(const cp_key_pair_t *cpkp)
{
	return cpkp_size(cpkp->cpkp_max_pers_key_len, cpx_max_key_len(cpkp_cpx(cpkp)));
}

void cpkp_flush(cp_key_pair_t *cpkp)
{
	cpx_flush(cpkp_cpx(cpkp));
	cpkp->cpkp_pers_key_len = 0;
	bzero(cpkp_pers_key(cpkp), cpkp->cpkp_max_pers_key_len);
}

bool cpkp_can_copy(const cp_key_pair_t *src, const cp_key_pair_t *dst)
{
	return (cpkp_pers_key_len(src) <= dst->cpkp_max_pers_key_len
			&& cpx_can_copy(cpkp_cpx(src), cpkp_cpx(dst)));
}

void cpkp_copy(const cp_key_pair_t *src, cp_key_pair_t *dst)
{
	const uint16_t key_len = cpkp_pers_key_len(src);
	cpkp_set_pers_key_len(dst, key_len);
	memcpy(cpkp_pers_key(dst), cpkp_pers_key(src), key_len);
	cpx_copy(cpkp_cpx(src), cpkp_cpx(dst));
}

// --

bool cp_is_supported_version(uint16_t vers)
{
	return vers == CP_VERS_4 || vers == CP_VERS_5;
}

/*
 * Return the appropriate key and, if requested, the physical offset and
 * maximum length for a particular I/O operation.
 */
void cp_io_params(__unused hfsmount_t *hfsmp, cprotect_t cpr,
				  __unused off_rsrc_t off_rsrc,
				  __unused int direction, cp_io_params_t *io_params)
{
#if HFS_CONFIG_KEY_ROLL
	hfs_cp_key_roll_ctx_t *ckr = cpr->cp_key_roll_ctx;

	if (ckr && off_rsrc < ckr->ckr_off_rsrc) {
		/*
		 * When we're in the process of rolling an extent, ckr_off_rsrc will
		 * indicate the end of the extent.
		 */
		const off_rsrc_t roll_loc = ckr->ckr_off_rsrc
			- hfs_blk_to_bytes(ckr->ckr_roll_extent.blockCount,
							   hfsmp->blockSize);

		if (off_rsrc < roll_loc) {
			io_params->max_len		= roll_loc - off_rsrc;
			io_params->phys_offset	= -1;
		} else {
			/*
			 * We should never get reads to the extent we're rolling
			 * because the pages should be locked in the UBC.  If we
			 * did get reads it's not obvious what the right thing to
			 * do is either: we could read from the old location, but
			 * we might have written later data to the new location,
			 * or we could read from the new location, but data might
			 * not have been written there yet.
			 *
			 * Note that whilst raw encrypted reads don't lock any
			 * pages, or take a cluster_read_direct lock, the call to
			 * hfs_key_roll_up_to in hfs_vnop_read will have ensured
			 * that the file has been rolled beyond the offset being
			 * read so this path should never be taken in that case.
			 */
			hfs_assert(direction == VNODE_WRITE);

			// For release builds, just in case...
			if (direction == VNODE_READ) {
				// Use the old key and offset
				goto old_key;
			}

			io_params->max_len = ckr->ckr_off_rsrc - off_rsrc;
			io_params->phys_offset = hfs_blk_to_bytes(ckr->ckr_roll_extent.startBlock,
													  hfsmp->blockSize) + off_rsrc - roll_loc;
		}

		// Use new key
		io_params->cpx = cpkp_cpx(&ckr->ckr_keys);
		return;
	}
old_key:
	// Use old key...
#endif

	io_params->max_len = INT64_MAX;
	io_params->phys_offset = -1;
	io_params->cpx = cpkp_cpx(&cpr->cp_keys);
}

static void cp_flush_cached_keys(cprotect_t cpr)
{
	cpx_flush(cpkp_cpx(&cpr->cp_keys));
#if HFS_CONFIG_KEY_ROLL
	if (cpr->cp_key_roll_ctx)
		cpx_flush(cpkp_cpx(&cpr->cp_key_roll_ctx->ckr_keys));
#endif
}

static bool cp_needs_pers_key(cprotect_t cpr)
{
	if (CP_CLASS(cpr->cp_pclass) == PROTECTION_CLASS_F)
		return !cpx_has_key(cpkp_cpx(&cpr->cp_keys));
	else
		return !cpkp_has_pers_key(&cpr->cp_keys);
}

static cp_key_revision_t cp_initial_key_revision(__unused hfsmount_t *hfsmp)
{
	return 1;
}

cp_key_revision_t cp_next_key_revision(cp_key_revision_t rev)
{
	rev = (rev + 0x0100) ^ (mach_absolute_time() & 0xff);
	if (!rev)
		rev = 1;
	return rev;
}

/*
 * Allocate and initialize a cprotect blob for a new cnode.
 * Called from hfs_getnewvnode: cnode is locked exclusive.
 * 
 * Read xattr data off the cnode. Then, if conditions permit,
 * unwrap the file key and cache it in the cprotect blob.
 */
int
cp_entry_init(struct cnode *cp, struct mount *mp)
{
	struct cprotect *entry = NULL;
	int error = 0;
	struct hfsmount *hfsmp = VFSTOHFS(mp);

	/*
	 * The cnode should be locked at this point, regardless of whether or not
	 * we are creating a new item in the namespace or vending a vnode on behalf
	 * of lookup.  The only time we tell getnewvnode to skip the lock is when 
	 * constructing a resource fork vnode. But a resource fork vnode must come
	 * after the regular data fork cnode has already been constructed.
	 */
	if (!cp_fs_protected (mp)) {
		cp->c_cpentry = NULL;
		return 0;
	}

	if (!S_ISREG(cp->c_mode) && !S_ISDIR(cp->c_mode)) {
		cp->c_cpentry = NULL;
		return 0;
	}

	if (hfsmp->hfs_running_cp_major_vers == 0) {
		panic ("hfs cp: no running mount point version! ");		
	}

	hfs_assert(cp->c_cpentry == NULL);

	error = cp_getxattr(cp, hfsmp, &entry);
	if (error == ENOATTR) {
		/*
		 * Normally, we should always have a CP EA for a file or directory that
		 * we are initializing here. However, there are some extenuating circumstances,
		 * such as the root directory immediately following a newfs_hfs.
		 *
		 * As a result, we leave code here to deal with an ENOATTR which will always
		 * default to a 'D/NONE' key, though we don't expect to use it much.
		 */
		cp_key_class_t target_class = PROTECTION_CLASS_D;

		if (S_ISDIR(cp->c_mode)) {
			target_class = PROTECTION_CLASS_DIR_NONE;
		}

		cp_key_revision_t key_revision = cp_initial_key_revision(hfsmp);

		/* allow keybag to override our class preferences */
		error = cp_new (&target_class, hfsmp, cp, cp->c_mode, CP_KEYWRAP_DIFFCLASS,
						key_revision, (cp_new_alloc_fn)cp_entry_alloc, (void **)&entry);
		if (error == 0) {
			entry->cp_pclass = target_class;
			entry->cp_key_os_version = cp_os_version();
			entry->cp_key_revision = key_revision;
			error = cp_setxattr (cp, entry, hfsmp, cp->c_fileid, XATTR_CREATE);
		}
	}

	/* 
	 * Bail out if:
	 * a) error was not ENOATTR (we got something bad from the getxattr call)
	 * b) we encountered an error setting the xattr above.
	 * c) we failed to generate a new cprotect data structure.
	 */
	if (error) {
		goto out;
	}	

	cp->c_cpentry = entry;

out:
	if (error == 0) {
		entry->cp_backing_cnode = cp;
	}
	else {
		if (entry) {
			cp_entry_destroy(hfsmp, entry);
		}
		cp->c_cpentry = NULL;
	}

	return error;
}

/*
 * cp_setup_newentry
 * 
 * Generate a keyless cprotect structure for use with the new AppleKeyStore kext.
 * Since the kext is now responsible for vending us both wrapped/unwrapped keys
 * we need to create a keyless xattr upon file / directory creation. When we have the inode value
 * and the file/directory is established, then we can ask it to generate keys.  Note that
 * this introduces a potential race;  If the device is locked and the wrapping
 * keys are purged between the time we call this function and the time we ask it to generate
 * keys for us, we could have to fail the open(2) call and back out the entry.
 */

int cp_setup_newentry (struct hfsmount *hfsmp, struct cnode *dcp,
					   cp_key_class_t suppliedclass, mode_t cmode,
					   struct cprotect **tmpentry)
{
	int isdir = 0;
	struct cprotect *entry = NULL;
	uint32_t target_class = hfsmp->default_cp_class;
	suppliedclass = CP_CLASS(suppliedclass);

	if (hfsmp->hfs_running_cp_major_vers == 0) {
		panic ("CP: major vers not set in mount!");
	}
	
	if (S_ISDIR (cmode))  {
		isdir = 1;
	}

	/* Decide the target class.  Input argument takes priority. */
	if (cp_is_valid_class (isdir, suppliedclass)) {
		/* caller supplies -1 if it was not specified so we will default to the mount point value */
		target_class = suppliedclass;
		/*
		 * One exception, F is never valid for a directory
		 * because its children may inherit and userland will be
		 * unable to read/write to the files.
		 */
		if (isdir) {
			if (target_class == PROTECTION_CLASS_F) {
				*tmpentry = NULL;
				return EINVAL;
			}
		}
	}
	else {
		/* 
		 * If no valid class was supplied, behave differently depending on whether or not
		 * the item being created is a file or directory.
		 * 
		 * for FILE:
		 * 		If parent directory has a non-zero class, use that.
		 * 		If parent directory has a zero class (not set), then attempt to
		 *		apply the mount point default.
		 * 
		 * for DIRECTORY:
		 *		Directories always inherit from the parent; if the parent
		 * 		has a NONE class set, then we can continue to use that.
		 */
		if ((dcp) && (dcp->c_cpentry)) {
			uint32_t parentclass = CP_CLASS(dcp->c_cpentry->cp_pclass);
			/* If the parent class is not valid, default to the mount point value */
			if (cp_is_valid_class(1, parentclass)) {
				if (isdir) {
					target_class = parentclass;	
				}
				else if (parentclass != PROTECTION_CLASS_DIR_NONE) {
					/* files can inherit so long as it's not NONE */
					target_class = parentclass;
				}
			}
			/* Otherwise, we already defaulted to the mount point's default */
		}
	}

	/* Generate the cprotect to vend out */
	entry = cp_entry_alloc(NULL, 0, 0, NULL);
	if (entry == NULL) {
		*tmpentry = NULL;
		return ENOMEM;
	}	

	/* 
	 * We don't have keys yet, so fill in what we can.  At this point
	 * this blob has no keys and it has no backing xattr.  We just know the
	 * target class.
	 */
	entry->cp_flags = CP_NO_XATTR;
	/* Note this is only the effective class */
	entry->cp_pclass = target_class;
	*tmpentry = entry;

	return 0;
}

/*
 * Set up an initial key/class pair for a disassociated cprotect entry.
 * This function is used to generate transient keys that will never be
 * written to disk.  We use class F for this since it provides the exact
 * semantics that are needed here.  Because we never attach this blob to
 * a cnode directly, we take a pointer to the cprotect struct.
 *
 * This function is primarily used in the HFS FS truncation codepath
 * where we may rely on AES symmetry to relocate encrypted data from
 * one spot in the disk to another.
 */
int cpx_gentempkeys(cpx_t *pcpx, __unused struct hfsmount *hfsmp)
{
	cpx_t cpx = cpx_alloc(CP_MAX_KEYSIZE, true);

	cpx_set_key_len(cpx, CP_MAX_KEYSIZE);
	read_random(cpx_key(cpx), CP_MAX_KEYSIZE);
	cpx_set_use_offset_for_iv(cpx, true);

	*pcpx = cpx;

	return 0;
}

/*
 * Tear down and clear a cprotect blob for a closing file.
 * Called at hfs_reclaim_cnode: cnode is locked exclusive.
 */
void
cp_entry_destroy(hfsmount_t *hfsmp, struct cprotect *entry_ptr)
{
	if (entry_ptr == NULL) {
		/* nothing to clean up */
		return;
	}
	cp_entry_dealloc(hfsmp, entry_ptr);
}


int
cp_fs_protected (mount_t mnt) 
{
	return (vfs_flags(mnt) & MNT_CPROTECT);
}


/*
 * Return a pointer to underlying cnode if there is one for this vnode.
 * Done without taking cnode lock, inspecting only vnode state.
 */
struct cnode *
cp_get_protected_cnode(struct vnode *vp)
{
	if (!cp_vnode_is_eligible(vp)) {
		return NULL;
	}

	if (!cp_fs_protected(VTOVFS(vp))) {
		/* mount point doesn't support it */
		return NULL;
	}

	return vnode_fsnode(vp);
}


/*
 * Sets *class to persistent class associated with vnode,
 * or returns error.
 */
int
cp_vnode_getclass(struct vnode *vp, cp_key_class_t *class)
{
	struct cprotect *entry;
	int error = 0;
	struct cnode *cp;
	int took_truncate_lock = 0;
	struct hfsmount *hfsmp = NULL;

	/* Is this an interesting vp? */
	if (!cp_vnode_is_eligible (vp)) {
		return EBADF;
	}

	/* Is the mount point formatted for content protection? */
	if (!cp_fs_protected(VTOVFS(vp))) {
		return ENOTSUP;
	}

	cp = VTOC(vp);
	hfsmp = VTOHFS(vp);

	/*
	 * Take the truncate lock up-front in shared mode because we may need
	 * to manipulate the CP blob. Pend lock events until we're done here.
	 */
	hfs_lock_truncate (cp, HFS_SHARED_LOCK, HFS_LOCK_DEFAULT);
	took_truncate_lock = 1;

	/*
	 * We take only the shared cnode lock up-front.  If it turns out that
	 * we need to manipulate the CP blob to write a key out, drop the
	 * shared cnode lock and acquire an exclusive lock.
	 */
	error = hfs_lock(cp, HFS_SHARED_LOCK, HFS_LOCK_DEFAULT);
	if (error) {
		hfs_unlock_truncate(cp, HFS_LOCK_DEFAULT);
		return error;
	}

	/* pull the class from the live entry */
	entry = cp->c_cpentry;

	if (entry == NULL) {
		panic("Content Protection: uninitialized cnode %p", cp);
	}
	
	/* Note that we may not have keys yet, but we know the target class. */

	if (error == 0) {
		*class = CP_CLASS(entry->cp_pclass);
	}

	if (took_truncate_lock) {
		hfs_unlock_truncate(cp, HFS_LOCK_DEFAULT);
	}

	hfs_unlock(cp);
	return error;
}

/*
 * Sets persistent class for this file or directory.
 * If vnode cannot be protected (system file, non-regular file, non-hfs), EBADF.
 * If the new class can't be accessed now, EPERM.
 * Otherwise, record class and re-wrap key if the mount point is content-protected.
 */
int
cp_vnode_setclass(struct vnode *vp, cp_key_class_t newclass)
{
	struct cnode *cp;
	struct cprotect *entry = 0;
	int error = 0;
	int took_truncate_lock = 0;
	struct hfsmount *hfsmp = NULL;
	int isdir = 0;

	if (vnode_isdir (vp)) {
		isdir = 1;
	}

	/* Ensure we only use the effective class here */
	newclass = CP_CLASS(newclass);

	if (!cp_is_valid_class(isdir, newclass)) {
		printf("hfs: CP: cp_setclass called with invalid class %d\n", newclass);
		return EINVAL;
	}

	/* Is this an interesting vp? */
	if (!cp_vnode_is_eligible(vp)) {
		return EBADF;
	}

	/* Is the mount point formatted for content protection? */
	if (!cp_fs_protected(VTOVFS(vp))) {
		return ENOTSUP;
	}

	hfsmp = VTOHFS(vp);
	if (hfsmp->hfs_flags & HFS_READ_ONLY) {
		return EROFS;
	}

	/*
	 * Take the cnode truncate lock exclusive because we want to manipulate the
	 * CP blob. The lock-event handling code is doing the same.  This also forces
	 * all pending IOs to drain before we can re-write the persistent and cache keys.
	 */
	cp = VTOC(vp);
	hfs_lock_truncate (cp, HFS_EXCLUSIVE_LOCK, HFS_LOCK_DEFAULT);
	took_truncate_lock = 1;

	/*
	 * The truncate lock is not sufficient to guarantee the CP blob
	 * isn't being used.  We must wait for existing writes to finish.
	 */
	vnode_waitforwrites(vp, 0, 0, 0, "cp_vnode_setclass");

	if (hfs_lock(cp, HFS_EXCLUSIVE_LOCK, HFS_LOCK_DEFAULT)) {
		return EINVAL;
	}

	entry = cp->c_cpentry;
	if (entry == NULL) {
		error = EINVAL;
		goto out;
	}

	/* 
	 * re-wrap per-file key with new class.  
	 * Generate an entirely new key if switching to F. 
	 */
	if (vnode_isreg(vp)) {
		/*
		 * The vnode is a file.  Before proceeding with the re-wrap, we need
		 * to unwrap the keys before proceeding.  This is to ensure that 
		 * the destination class's properties still work appropriately for the
		 * target class (since B allows I/O but an unwrap prior to the next unlock
		 * will not be allowed).
		 */
		if (!cpx_has_key(cpkp_cpx(&entry->cp_keys))) {
			error = cp_restore_keys (entry, hfsmp, cp);
			if (error) {
				goto out;
			}
		}

		if (newclass == PROTECTION_CLASS_F) {
			/* Verify that file is blockless if switching to class F */
			if (cp->c_datafork->ff_size > 0) {
				error = EINVAL;
				goto out;
			}

			cp_key_pair_t *cpkp = NULL;
			cprotect_t new_entry = cp_entry_alloc(NULL, 0, CP_MAX_KEYSIZE, &cpkp);

			if (!new_entry) {
				error = ENOMEM;
				goto out;
			}

			/* newclass is only the effective class */
			new_entry->cp_pclass = newclass;
			new_entry->cp_key_os_version = cp_os_version();
			new_entry->cp_key_revision = cp_next_key_revision(entry->cp_key_revision);

			cpx_t cpx = cpkp_cpx(cpkp);

			/* Class F files are not wrapped, so they continue to use MAX_KEYSIZE */
			cpx_set_key_len(cpx, CP_MAX_KEYSIZE);
			read_random (cpx_key(cpx), CP_MAX_KEYSIZE);

			cp_replace_entry(hfsmp, cp, new_entry);

			error = 0;
			goto out;
		}

		/* Deny the setclass if file is to be moved from F to something else */
		if (entry->cp_pclass == PROTECTION_CLASS_F) {
			error = EPERM;
			goto out;
		}

		if (!cpkp_has_pers_key(&entry->cp_keys)) {
			struct cprotect *new_entry = NULL;
			/*
			 * We want to fail if we can't wrap to the target class. By not setting
			 * CP_KEYWRAP_DIFFCLASS, we tell keygeneration that if it can't wrap
			 * to 'newclass' then error out.
			 */
			uint32_t flags = 0;
			error = cp_generate_keys (hfsmp, cp, newclass, flags,  &new_entry);
			if (error == 0) {
				cp_replace_entry (hfsmp, cp, new_entry);
			}
			/* Bypass the setxattr code below since generate_keys does it for us */
			goto out;
		}

		cprotect_t new_entry;
		error = cp_rewrap(cp, hfsmp, &newclass, &entry->cp_keys, entry,
						  (cp_new_alloc_fn)cp_entry_alloc, (void **)&new_entry);
		if (error) {
			/* we didn't have perms to set this class. leave file as-is and error out */
			goto out;
		}

#if HFS_CONFIG_KEY_ROLL
		hfs_cp_key_roll_ctx_t *new_key_roll_ctx = NULL;
		if (entry->cp_key_roll_ctx) {
			error = cp_rewrap(cp, hfsmp, &newclass, &entry->cp_key_roll_ctx->ckr_keys,
							  entry->cp_key_roll_ctx,
							  (cp_new_alloc_fn)hfs_key_roll_ctx_alloc,
							  (void **)&new_key_roll_ctx);

			if (error) {
				cp_entry_dealloc(hfsmp, new_entry);
				goto out;
			}

			new_entry->cp_key_roll_ctx = new_key_roll_ctx;
		}
#endif

		new_entry->cp_pclass = newclass;

		cp_replace_entry(hfsmp, cp, new_entry);
		entry = new_entry;
	}
	else if (vnode_isdir(vp)) {
		/* For directories, just update the pclass.  newclass is only effective class */
		entry->cp_pclass = newclass;
		error = 0;	
	}
	else {
		/* anything else, just error out */
		error = EINVAL;
		goto out;	
	}
	
	/* 
	 * We get here if the new class was F, or if we were re-wrapping a cprotect that already
	 * existed. If the keys were never generated, then they'll skip the setxattr calls.
	 */

	error = cp_setxattr(cp, cp->c_cpentry, VTOHFS(vp), 0, XATTR_REPLACE);
	if (error == ENOATTR) {
		error = cp_setxattr(cp, cp->c_cpentry, VTOHFS(vp), 0, XATTR_CREATE);
	}

out:

	if (took_truncate_lock) {
		hfs_unlock_truncate (cp, HFS_LOCK_DEFAULT);
	}
	hfs_unlock(cp);
	return error;
}


int cp_vnode_transcode(vnode_t vp, cp_key_t *k)
{
	struct cnode *cp;
	struct cprotect *entry = 0;
	int error = 0;
	int took_truncate_lock = 0;
	struct hfsmount *hfsmp = NULL;

	/* Structures passed between HFS and AKS */
	struct aks_cred_s access_in;
	struct aks_wrapped_key_s wrapped_key_in, wrapped_key_out;

	/* Is this an interesting vp? */
	if (!cp_vnode_is_eligible(vp)) {
		return EBADF;
	}

	/* Is the mount point formatted for content protection? */
	if (!cp_fs_protected(VTOVFS(vp))) {
		return ENOTSUP;
	}

	cp = VTOC(vp);
	hfsmp = VTOHFS(vp);

	/*
	 * Take the cnode truncate lock exclusive because we want to manipulate the
	 * CP blob. The lock-event handling code is doing the same.  This also forces
	 * all pending IOs to drain before we can re-write the persistent and cache keys.
	 */
	hfs_lock_truncate (cp, HFS_EXCLUSIVE_LOCK, HFS_LOCK_DEFAULT);
	took_truncate_lock = 1;

	if (hfs_lock(cp, HFS_EXCLUSIVE_LOCK, HFS_LOCK_DEFAULT)) {
		return EINVAL;
	}

	entry = cp->c_cpentry;
	if (entry == NULL) {
		error = EINVAL;
		goto out;
	}

	/* Send the per-file key in wrapped form for re-wrap with the current class information
	 * Send NULLs in the output parameters of the wrapper() and AKS will do the rest.
	 * Don't need to process any outputs, so just clear the locks and pass along the error. */
	if (vnode_isreg(vp)) {

		/* Picked up the following from cp_wrap().
		 * If needed, more comments available there. */

		if (CP_CLASS(entry->cp_pclass) == PROTECTION_CLASS_F) {
			error = EINVAL;
			goto out;
		}

		cp_init_access(&access_in, cp);

		bzero(&wrapped_key_in, sizeof(wrapped_key_in));
		bzero(&wrapped_key_out, sizeof(wrapped_key_out));

		cp_key_pair_t *cpkp = &entry->cp_keys;

#if HFS_CONFIG_KEY_ROLL
		if (entry->cp_key_roll_ctx)
			cpkp = &entry->cp_key_roll_ctx->ckr_keys;
#endif

		wrapped_key_in.key = cpkp_pers_key(cpkp);
		wrapped_key_in.key_len = cpkp_pers_key_len(cpkp);

		if (!wrapped_key_in.key_len) {
			error = EINVAL;
			goto out;
		}

		/* Use the actual persistent class when talking to AKS */
		wrapped_key_in.dp_class = entry->cp_pclass;
		wrapped_key_out.key = k->key;
		wrapped_key_out.key_len = k->len;

		error = hfs_backup_key(&access_in,
							   &wrapped_key_in,
							   &wrapped_key_out);

		if(error)
			error = EPERM;
		else
			k->len = wrapped_key_out.key_len;
	}

out:
	if (took_truncate_lock) {
		hfs_unlock_truncate (cp, HFS_LOCK_DEFAULT);
	}
	hfs_unlock(cp);
	return error;
}


/*
 * Check permission for the given operation (read, write) on this node.
 * Additionally, if the node needs work, do it:
 * - create a new key for the file if one hasn't been set before
 * - write out the xattr if it hasn't already been saved
 * - unwrap the key if needed
 *
 * Takes cnode lock, and upgrades to exclusive if modifying cprotect.
 *
 * Note that this function does *NOT* take the cnode truncate lock.  This is because
 * the thread calling us may already have the truncate lock.  It is not necessary
 * because either we successfully finish this function before the keys are tossed
 * and the IO will fail, or the keys are tossed and then this function will fail.
 * Either way, the cnode lock still ultimately guards the keys.  We only rely on the
 * truncate lock to protect us against tossing the keys as a cluster call is in-flight.
 */
int
cp_handle_vnop(struct vnode *vp, int vnop, int ioflag)
{
	struct cprotect *entry;
	int error = 0;
	struct hfsmount *hfsmp = NULL;
	struct cnode *cp = NULL;

	/*
	 * First, do validation against the vnode before proceeding any further:
	 * Is this vnode originating from a valid content-protected filesystem ?
	 */
	if (cp_vnode_is_eligible(vp) == 0) {
		/*
		 * It is either not HFS or not a file/dir.  Just return success. This is a valid
		 * case if servicing i/o against another filesystem type from VFS
		 */
		return 0;
	}

	if (cp_fs_protected (VTOVFS(vp)) == 0) {
		/*
		 * The underlying filesystem does not support content protection.  This is also
		 * a valid case.  Simply return success.
		 */
		return 0;
	}

	/*
	 * At this point, we know we have a HFS vnode that backs a file or directory on a
	 * filesystem that supports content protection
	 */
	cp = VTOC(vp);

	if ((error = hfs_lock(cp, HFS_SHARED_LOCK, HFS_LOCK_DEFAULT))) {
		return error;
	}

	entry = cp->c_cpentry;

	if (entry == NULL) {
		/*
		 * If this cnode is not content protected, simply return success.
		 * Note that this function is called by all I/O-based call sites
		 * when CONFIG_PROTECT is enabled during XNU building.
		 */

		/* 
		 * All files should have cprotect structs.  It's possible to encounter
		 * a directory from a V2.0 CP system but all files should have protection
		 * EAs
		 */
		if (vnode_isreg(vp)) {
			error = EPERM;
		}

		goto out;
	}

	vp = CTOV(cp, 0);
	if (vp == NULL) {
		/* is it a rsrc */
		vp = CTOV(cp,1);
		if (vp == NULL) {
			error = EINVAL;
			goto out;
		}
	}
	hfsmp = VTOHFS(vp);

	if ((error = cp_check_access(cp, hfsmp, vnop))) {
		/* check for raw encrypted access before bailing out */
		if ((ioflag & IO_ENCRYPTED)
#if HFS_CONFIG_KEY_ROLL
			// If we're rolling, we need the keys
			&& !hfs_is_key_rolling(cp)
#endif
			&& (vnop == CP_READ_ACCESS)) {
			/*
			 * read access only + asking for the raw encrypted bytes
			 * is legitimate, so reset the error value to 0
			 */
			error = 0;
		}
		else {
			goto out;
		}
	}

	if (!ISSET(entry->cp_flags, CP_NO_XATTR)) {
		if (!S_ISREG(cp->c_mode))
			goto out;

		// If we have a persistent key and the cached key, we're done
		if (!cp_needs_pers_key(entry)
			&& cpx_has_key(cpkp_cpx(&entry->cp_keys))) {
			goto out;
		}
	}

	/* upgrade to exclusive lock */
	if (lck_rw_lock_shared_to_exclusive(&cp->c_rwlock) == FALSE) {
		if ((error = hfs_lock(cp, HFS_EXCLUSIVE_LOCK, HFS_LOCK_DEFAULT))) {
			return error;
		}
	} else {
		cp->c_lockowner = current_thread();
	}

	/* generate new keys if none have ever been saved */
	if (cp_needs_pers_key(entry)) {
		struct cprotect *newentry = NULL;
		/* 
		 * It's ok if this ends up being wrapped in a different class than 'pclass'.
		 * class modification is OK here. 
		 */		
		uint32_t flags = CP_KEYWRAP_DIFFCLASS;

		error = cp_generate_keys (hfsmp, cp, CP_CLASS(cp->c_cpentry->cp_pclass), flags, &newentry);	
		if (error == 0) {
			cp_replace_entry (hfsmp, cp, newentry);
			entry = newentry;
		}
		else {
			goto out;
		}
	}

	/* unwrap keys if needed */
	if (!cpx_has_key(cpkp_cpx(&entry->cp_keys))) {
		if ((vnop == CP_READ_ACCESS) && (ioflag & IO_ENCRYPTED)) {
			/* no need to try to restore keys; they are not going to be used */
			error = 0;
		}
		else {
			error = cp_restore_keys(entry, hfsmp, cp);
			if (error) {
				goto out;
			}
		}
	}

	/* write out the xattr if it's new */
	if (entry->cp_flags & CP_NO_XATTR)
		error = cp_setxattr(cp, entry, VTOHFS(cp->c_vp), 0, XATTR_CREATE);

out:

	hfs_unlock(cp);
	return error;
}

#if HFS_TMPDBG
#if !SECURE_KERNEL
static void cp_log_eperm (struct vnode* vp, int pclass, boolean_t create) {
	char procname[256] = {};
	const char *fname = "unknown";
	const char *dbgop = "open";

	int ppid = proc_selfpid();
	/* selfname does a strlcpy so we're OK */
	proc_selfname(procname, sizeof(procname));
	if (vp && vp->v_name) {
		/* steal from the namecache */
		fname = vp->v_name;
	}

	if (create) {
		dbgop = "create";	
	}
	
	printf("proc %s (pid %d) class %d, op: %s failure @ file %s\n", procname, ppid, pclass, dbgop, fname);
}
#endif
#endif


int
cp_handle_open(struct vnode *vp, int mode)
{
	struct cnode *cp = NULL ;
	struct cprotect *entry = NULL;
	struct hfsmount *hfsmp;
	int error = 0;

	/* If vnode not eligible, just return success */
	if (!cp_vnode_is_eligible(vp)) {
		return 0;
	}

	/* If mount point not properly set up, then also return success */
	if (!cp_fs_protected(VTOVFS(vp))) {
		return 0;
	}

	cp = VTOC(vp);

	// Allow if raw encrypted mode requested
	if (ISSET(mode, FENCRYPTED)) {
#if HFS_CONFIG_KEY_ROLL
		// If we're rolling, we need the keys
		hfs_lock_always(cp, HFS_SHARED_LOCK);
		bool rolling = hfs_is_key_rolling(cp);
		hfs_unlock(cp);
		if (!rolling)
			return 0;
#else
		return 0;
#endif
	}
	if (ISSET(mode, FUNENCRYPTED)) {
		return 0;
	}

	/* We know the vnode is in a valid state. Acquire cnode and validate */
	hfsmp = VTOHFS(vp);

	if ((error = hfs_lock(cp, HFS_EXCLUSIVE_LOCK, HFS_LOCK_DEFAULT))) {
		return error;
	}

	entry = cp->c_cpentry;
	if (entry == NULL) {
		/* 
		 * If the mount is protected and we couldn't get a cprotect for this vnode,
		 * then it's not valid for opening.
		 */
		if (vnode_isreg(vp)) {
			error = EPERM;
		}
		goto out;
	}

	if (!S_ISREG(cp->c_mode))
		goto out;

	/*
	 * Does the cnode have keys yet?  If not, then generate them.
	 */
	if (cp_needs_pers_key(entry)) {
		struct cprotect *newentry = NULL;
		/* Allow the keybag to override our class preferences */
		uint32_t flags = CP_KEYWRAP_DIFFCLASS;
		error = cp_generate_keys (hfsmp, cp, CP_CLASS(cp->c_cpentry->cp_pclass), flags, &newentry);
		if (error == 0) {
			cp_replace_entry (hfsmp, cp, newentry);
			entry = newentry;
		}	
		else {
			goto out;
		}
	}	

	/*
	 * We want to minimize the number of unwraps that we'll have to do since 
	 * the cost can vary, depending on the platform we're running. 
	 */
	switch (CP_CLASS(entry->cp_pclass)) {
		case PROTECTION_CLASS_B:
			if (mode & O_CREAT) {
				/* 
				 * Class B always allows creation.  Since O_CREAT was passed through
				 * we infer that this was a newly created vnode/cnode.  Even though a potential
				 * race exists when multiple threads attempt to create/open a particular
				 * file, only one can "win" and actually create it.  VFS will unset the
				 * O_CREAT bit on the loser.	 
				 * 
				 * Note that skipping the unwrap check here is not a security issue -- 
				 * we have to unwrap the key permanently upon the first I/O.
				 */
				break;
			}
			
			if (cpx_has_key(cpkp_cpx(&entry->cp_keys)) && !ISSET(mode, FENCRYPTED)) {
				/*
				 * For a class B file, attempt the unwrap if we have the key in
				 * core already. 
				 * The device could have just transitioned into the lock state, and 
				 * this vnode may not yet have been purged from the vnode cache (which would
				 * remove the keys). 
				 */
				struct aks_cred_s access_in;
				struct aks_wrapped_key_s wrapped_key_in;

				cp_init_access(&access_in, cp);
				bzero(&wrapped_key_in, sizeof(wrapped_key_in));
				wrapped_key_in.key = cpkp_pers_key(&entry->cp_keys);
				wrapped_key_in.key_len = cpkp_pers_key_len(&entry->cp_keys);
				/* Use the persistent class when talking to AKS */
				wrapped_key_in.dp_class = entry->cp_pclass;
				error = hfs_unwrap_key(&access_in, &wrapped_key_in, NULL);
				if (error) {
					error = EPERM;
				}
				break;
			}
			/* otherwise, fall through to attempt the unwrap/restore */
		case PROTECTION_CLASS_A:
		case PROTECTION_CLASS_C:
			/*
			 * At this point, we know that we need to attempt an unwrap if needed; we want
			 * to makes sure that open(2) fails properly if the device is either just-locked
			 * or never made it past first unlock.  Since the keybag serializes access to the
			 * unwrapping keys for us and only calls our VFS callback once they've been purged, 
			 * we will get here in two cases:
			 * 
			 * A) we're in a window before the wrapping keys are purged; this is OK since when they get 
			 * purged, the vnode will get flushed if needed.
			 * 
			 * B) The keys are already gone.  In this case, the restore_keys call below will fail. 
			 *
			 * Since this function is bypassed entirely if we're opening a raw encrypted file, 
			 * we can always attempt the restore.
			 */
			if (!cpx_has_key(cpkp_cpx(&entry->cp_keys))) {
				error = cp_restore_keys(entry, hfsmp, cp);
			}
	
			if (error) {
				error = EPERM;
			}
	
			break;

		case PROTECTION_CLASS_D:
		default:
			break;
	}

out:

#if HFS_TMPDBG
#if !SECURE_KERNEL
	if ((hfsmp->hfs_cp_verbose) && (error == EPERM)) {
		cp_log_eperm (vp, CP_CLASS(entry->cp_pclass), false);
	}
#endif
#endif

	hfs_unlock(cp);
	return error;
}


/*
 * cp_getrootxattr:
 * Gets the EA we set on the root folder (fileid 1) to get information about the
 * version of Content Protection that was used to write to this filesystem.
 * Note that all multi-byte fields are written to disk little endian so they must be
 * converted to native endian-ness as needed.
 */
int
cp_getrootxattr(struct hfsmount* hfsmp, struct cp_root_xattr *outxattr) 
{
	void	*buf;

	/*
	 * We allow for an extra 64 bytes to cater for upgrades.  This wouldn't
	 * be necessary if the xattr routines just returned what we asked for.
	 */
	size_t bufsize = roundup(sizeof(struct cp_root_xattr) + 64, 64);

	int error = 0;

	hfs_assert(outxattr);

	buf = hfs_malloc(bufsize);

	uio_t uio = uio_create(1, 0, UIO_SYSSPACE, UIO_READ);

	uio_addiov(uio, CAST_USER_ADDR_T(buf), bufsize);

	size_t attrsize = bufsize;

	struct vnop_getxattr_args args = {
		.a_uio = uio,
		.a_name = CONTENT_PROTECTION_XATTR_NAME,
		.a_size = &attrsize
	};

	error = hfs_getxattr_internal(NULL, &args, hfsmp, 1);

	uio_free(uio);

	if (error != 0) {
		goto out;
	}

	if (attrsize < CP_ROOT_XATTR_MIN_LEN) {
		error = HFS_EINCONSISTENT;
		goto out;
	}

	const struct cp_root_xattr *xattr = buf;

	bzero(outxattr, sizeof(*outxattr));

	/* Now convert the multi-byte fields to native endianness */
	outxattr->major_version = OSSwapLittleToHostInt16(xattr->major_version);
	outxattr->minor_version = OSSwapLittleToHostInt16(xattr->minor_version);
	outxattr->flags = OSSwapLittleToHostInt64(xattr->flags);

	if (outxattr->major_version >= CP_VERS_5) {
		if (attrsize < sizeof(struct cp_root_xattr)) {
			error = HFS_EINCONSISTENT;
			goto out;
		}
#if HFS_CONFIG_KEY_ROLL
		outxattr->auto_roll_min_version = OSSwapLittleToHostInt32(xattr->auto_roll_min_version);
		outxattr->auto_roll_max_version = OSSwapLittleToHostInt32(xattr->auto_roll_max_version);
#endif
	}

out:
	hfs_free(buf, bufsize);
	return error;
}

/*
 * cp_setrootxattr:
 * Sets the EA we set on the root folder (fileid 1) to get information about the
 * version of Content Protection that was used to write to this filesystem.
 * Note that all multi-byte fields are written to disk little endian so they must be
 * converted to little endian as needed.
 *
 * This will be written to the disk when it detects the EA is not there, or when we need
 * to make a modification to the on-disk version that can be done in-place.
 */
int
cp_setrootxattr(struct hfsmount *hfsmp, struct cp_root_xattr *newxattr)
{
	int error = 0;
	struct vnop_setxattr_args args;

	args.a_desc = NULL;
	args.a_vp = NULL;
	args.a_name = CONTENT_PROTECTION_XATTR_NAME;
	args.a_uio = NULL; //pass data ptr instead
	args.a_options = 0;
	args.a_context = NULL; //no context needed, only done from mount.

	const uint64_t flags = newxattr->flags;

	/* Now convert the multi-byte fields to little endian before writing to disk. */
	newxattr->flags = OSSwapHostToLittleInt64(newxattr->flags);

	int xattr_size = sizeof(struct cp_root_xattr);

#if HFS_CONFIG_KEY_ROLL
	bool upgraded = false;

	if (newxattr->auto_roll_min_version || newxattr->auto_roll_max_version) {
		if (newxattr->major_version < CP_VERS_5) {
			printf("hfs: upgrading to cp version %u\n", CP_CURRENT_VERS);

			newxattr->major_version = CP_CURRENT_VERS;
			newxattr->minor_version = CP_MINOR_VERS;

			upgraded = true;
		}

		newxattr->auto_roll_min_version = OSSwapHostToLittleInt32(newxattr->auto_roll_min_version);
		newxattr->auto_roll_max_version = OSSwapHostToLittleInt32(newxattr->auto_roll_max_version);
	} else if (newxattr->major_version == CP_VERS_4)
		xattr_size = offsetof(struct cp_root_xattr, auto_roll_min_version);
#endif

	newxattr->major_version = OSSwapHostToLittleInt16(newxattr->major_version);
	newxattr->minor_version = OSSwapHostToLittleInt16(newxattr->minor_version);

	error = hfs_setxattr_internal(NULL, (caddr_t)newxattr,
			xattr_size, &args, hfsmp, 1);

	if (!error) {
		hfsmp->cproot_flags = flags;
#if HFS_CONFIG_KEY_ROLL
		if (upgraded)
			hfsmp->hfs_running_cp_major_vers = CP_CURRENT_VERS;
#endif
	}

	return error;
}


/*
 * Stores new xattr data on the cnode.
 * cnode lock held exclusive (if available).
 *
 * This function is also invoked during file creation.
 */
int cp_setxattr(struct cnode *cp, struct cprotect *entry, struct hfsmount *hfsmp,
				uint32_t fileid, int options)
{
	int error = 0;
	cp_key_pair_t *cpkp = &entry->cp_keys;
#if HFS_CONFIG_KEY_ROLL
	bool rolling = entry->cp_key_roll_ctx != NULL;

	if (rolling && entry->cp_key_roll_ctx->ckr_off_rsrc == INT64_MAX) {
		// We've finished rolling, but we still have the context
		rolling = false;
		cpkp = &entry->cp_key_roll_ctx->ckr_keys;
	}
#endif

	if (hfsmp->hfs_flags & HFS_READ_ONLY) {
		return EROFS;
	}

	if (hfsmp->hfs_running_cp_major_vers < CP_CURRENT_VERS) {
		// Upgrade
		printf("hfs: upgrading to cp version %u\n", CP_CURRENT_VERS);

		struct cp_root_xattr root_xattr;

		error = cp_getrootxattr(hfsmp, &root_xattr);
		if (error)
			return error;

		root_xattr.major_version = CP_CURRENT_VERS;
		root_xattr.minor_version = CP_MINOR_VERS;

		error = cp_setrootxattr(hfsmp, &root_xattr);
		if (error)
			return error;

		hfsmp->hfs_running_cp_major_vers = CP_CURRENT_VERS;
	}

	struct cp_xattr_v5 *xattr;
	xattr = hfs_malloc(sizeof(*xattr));

	xattr->xattr_major_version	= OSSwapHostToLittleConstInt16(CP_VERS_5);
	xattr->xattr_minor_version	= OSSwapHostToLittleConstInt16(CP_MINOR_VERS);
	xattr->flags				= 0;
#if HFS_CONFIG_KEY_ROLL
	if (rolling)
		xattr->flags |= CP_XAF_KEY_ROLLING;
#endif
	xattr->persistent_class		= OSSwapHostToLittleInt32(entry->cp_pclass);
	xattr->key_os_version		= OSSwapHostToLittleInt32(entry->cp_key_os_version);
	xattr->key_revision			= OSSwapHostToLittleInt16(entry->cp_key_revision);

	uint16_t key_len = cpkp_pers_key_len(cpkp);
	xattr->key_len	= OSSwapHostToLittleInt16(key_len);
	memcpy(xattr->persistent_key, cpkp_pers_key(cpkp), key_len);

	size_t xattr_len = offsetof(struct cp_xattr_v5, persistent_key) + key_len;

#if HFS_CONFIG_KEY_ROLL
	if (rolling) {
		struct cp_roll_info *roll_info = PTR_ADD(struct cp_roll_info *, xattr, xattr_len);

		roll_info->off_rsrc = OSSwapHostToLittleInt64(entry->cp_key_roll_ctx->ckr_off_rsrc);

		key_len = cpkp_pers_key_len(&entry->cp_key_roll_ctx->ckr_keys);
		roll_info->key_len = OSSwapHostToLittleInt16(key_len);

		memcpy(roll_info->key, cpkp_pers_key(&entry->cp_key_roll_ctx->ckr_keys), key_len);

		xattr_len += offsetof(struct cp_roll_info, key) + key_len;
	}
#endif

	struct vnop_setxattr_args args = {
		.a_vp		= cp ? cp->c_vp : NULL,
		.a_name		= CONTENT_PROTECTION_XATTR_NAME,
		.a_options	= options,
		.a_context	= vfs_context_current(),
	};

	error = hfs_setxattr_internal(cp, xattr, xattr_len, &args, hfsmp, fileid);

	hfs_free(xattr, sizeof(*xattr));

	if (error == 0 ) {
		entry->cp_flags &= ~CP_NO_XATTR;
	}

	return error;
}

/*
 * Used by an fcntl to query the underlying FS for its content protection version #
 */

int
cp_get_root_major_vers(vnode_t vp, uint32_t *level) 
{
	int err = 0;
	struct hfsmount *hfsmp = NULL;
	struct mount *mp = NULL;

	mp = VTOVFS(vp);

	/* check if it supports content protection */
	if (cp_fs_protected(mp) == 0) {
		return ENOTSUP;
	}

	hfsmp = VFSTOHFS(mp);
	/* figure out the level */

	err = cp_root_major_vers(mp);

	if (err == 0) {
		*level = hfsmp->hfs_running_cp_major_vers;
	}
	/* in error case, cp_root_major_vers will just return EINVAL. Use that */

	return err;
}

/* Used by fcntl to query default protection level of FS */
int cp_get_default_level (struct vnode *vp, uint32_t *level) {
	int err = 0;
	struct hfsmount *hfsmp = NULL;
	struct mount *mp = NULL;

	mp = VTOVFS(vp);

	/* check if it supports content protection */
	if (cp_fs_protected(mp) == 0) {
		return ENOTSUP;
	}

	hfsmp = VFSTOHFS(mp);
	/* figure out the default */

	*level = hfsmp->default_cp_class;
	return err;
}

/********************
 * Private Functions
 *******************/

static int
cp_root_major_vers(mount_t mp)
{
	int err = 0;
	struct cp_root_xattr xattr;
	struct hfsmount *hfsmp = NULL;

	hfsmp = vfs_fsprivate(mp);
	err = cp_getrootxattr (hfsmp, &xattr);

	if (err == 0) {
		hfsmp->hfs_running_cp_major_vers = xattr.major_version;
	}
	else {
		return EINVAL;
	}

	return 0;
}

static int
cp_vnode_is_eligible(struct vnode *vp)
{
	return !vnode_issystem(vp) && (vnode_isreg(vp) || vnode_isdir(vp));
}

#if DEBUG
static const uint32_t cp_magic1 = 0x7b727063;	// cpr{
static const uint32_t cp_magic2 = 0x7270637d;	// }cpr
#endif

struct cprotect *
cp_entry_alloc(cprotect_t old, uint16_t pers_key_len,
			   uint16_t cached_key_len, cp_key_pair_t **pcpkp)
{
	struct cprotect *cp_entry;

	if (pers_key_len > CP_MAX_WRAPPEDKEYSIZE)
		return (NULL);

	size_t size = (sizeof(struct cprotect) - sizeof(cp_key_pair_t)
				   + cpkp_size(pers_key_len, cached_key_len));

#if DEBUG
	size += 4;	// Extra for magic2
#endif

	cp_entry = hfs_mallocz(size);

	if (old) {
		memcpy(cp_entry, old, offsetof(struct cprotect, cp_keys));

#if HFS_CONFIG_KEY_ROLL
		// We don't copy the key roll context
		cp_entry->cp_key_roll_ctx = NULL;
#endif
	}

#if DEBUG
	cp_entry->cp_magic1 = cp_magic1;
	*PTR_ADD(uint32_t *, cp_entry, size - 4) = cp_magic2;
#endif

	cpkp_init(&cp_entry->cp_keys, pers_key_len, cached_key_len);

	/*
	 * If we've been passed the old entry, then we are in the process of
	 * rewrapping in which case we need to copy the cached key.  This is
	 * important for class B files when the device is locked because we
	 * won't be able to unwrap whilst in this state, yet we still need the
	 * unwrapped key.
	 */
	if (old)
		cpx_copy(cpkp_cpx(&old->cp_keys), cpkp_cpx(&cp_entry->cp_keys));

	if (pcpkp)
		*pcpkp = &cp_entry->cp_keys;

	return cp_entry;
}

static void
cp_entry_dealloc(__unused hfsmount_t *hfsmp, struct cprotect *entry)
{
#if HFS_CONFIG_KEY_ROLL
	hfs_release_key_roll_ctx(hfsmp, entry);
#endif

	cpkp_flush(&entry->cp_keys);

	size_t entry_size = (sizeof(struct cprotect) - sizeof(cp_key_pair_t)
						 + cpkp_sizex(&entry->cp_keys));
	
	/* 
	 * We are freeing the HFS cprotect, which contains the memory for 'cpx'
	 * Don't forget to release the CPX AES context 
	 */
	cpx_t embedded_cpx = cpkp_cpx(&entry->cp_keys);
	cpx_free_ctx (embedded_cpx);

#if DEBUG
	hfs_assert(entry->cp_magic1 == cp_magic1);
	hfs_assert(*PTR_ADD(uint32_t *, entry, (sizeof(struct cprotect) - sizeof(cp_key_pair_t)
										+ cpkp_sizex(&entry->cp_keys) == cp_magic2)));

	entry_size += 4;	// Extra for magic2
#endif

	hfs_free(entry, entry_size);
}

static int cp_read_xattr_v4(__unused hfsmount_t *hfsmp, struct cp_xattr_v4 *xattr,
							size_t xattr_len, cprotect_t *pcpr, cp_getxattr_options_t options)
{
	/* Endian swap the multi-byte fields into host endianness from L.E. */
	xattr->xattr_major_version = OSSwapLittleToHostInt16(xattr->xattr_major_version);
	xattr->xattr_minor_version = OSSwapLittleToHostInt16(xattr->xattr_minor_version);
	xattr->key_size = OSSwapLittleToHostInt32(xattr->key_size);
	xattr->flags = OSSwapLittleToHostInt32(xattr->flags);
	xattr->persistent_class = OSSwapLittleToHostInt32(xattr->persistent_class);
	xattr->key_os_version = OSSwapLittleToHostInt32(xattr->key_os_version);

	/*
	 * Prevent a buffer overflow, and validate the key length obtained from the
	 * EA. If it's too big, then bail out, because the EA can't be trusted at this
	 * point.
	 */
	if (xattr->key_size > CP_MAX_WRAPPEDKEYSIZE)
		return HFS_EINCONSISTENT;

	size_t min_len = offsetof(struct cp_xattr_v4, persistent_key) + xattr->key_size;
	if (xattr_len < min_len)
		return HFS_EINCONSISTENT;

	/*
	 * Class F files have no backing key; their keylength should be 0,
	 * though they should have the proper flags set.
	 *
	 * A request to instantiate a CP for a class F file should result
	 * in a bzero'd cp that just says class F, with key_flushed set.
	 */
	if (CP_CLASS(xattr->persistent_class) == PROTECTION_CLASS_F
		|| ISSET(xattr->flags, CP_XAF_NEEDS_KEYS)) {
		xattr->key_size = 0;
	}

	/* set up entry with information from xattr */
	cp_key_pair_t *cpkp = NULL;
	cprotect_t entry;
	
	if (ISSET(options, CP_GET_XATTR_BASIC_INFO)) {
		/* caller passed in a pre-allocated structure to get the basic info */
		entry = *pcpr;
		bzero(entry, offsetof(struct cprotect, cp_keys));
	}
	else {
		entry = cp_entry_alloc(NULL, xattr->key_size, CP_MAX_CACHEBUFLEN, &cpkp);
	}

	entry->cp_pclass = xattr->persistent_class;
	entry->cp_key_os_version = xattr->key_os_version;


	if (!ISSET(options, CP_GET_XATTR_BASIC_INFO)) {
		if (xattr->key_size) {
			cpkp_set_pers_key_len(cpkp, xattr->key_size);
			memcpy(cpkp_pers_key(cpkp), xattr->persistent_key, xattr->key_size);
		}

		*pcpr = entry;
	}
	else if (xattr->key_size) {
		SET(entry->cp_flags, CP_HAS_A_KEY);
	}

	return 0;
}

int cp_read_xattr_v5(hfsmount_t *hfsmp, struct cp_xattr_v5 *xattr,
					 size_t xattr_len, cprotect_t *pcpr, cp_getxattr_options_t options)
{
	if (xattr->xattr_major_version == OSSwapHostToLittleConstInt16(CP_VERS_4)) {
		return cp_read_xattr_v4(hfsmp, (struct cp_xattr_v4 *)xattr, xattr_len, pcpr, options);
	}

	xattr->xattr_major_version	= OSSwapLittleToHostInt16(xattr->xattr_major_version);

	if (xattr->xattr_major_version != CP_VERS_5) {
		printf("hfs: cp_getxattr: unsupported xattr version %d\n",
			   xattr->xattr_major_version);
		return ENOTSUP;
	}

	size_t min_len = offsetof(struct cp_xattr_v5, persistent_key);

	if (xattr_len < min_len)
		return HFS_EINCONSISTENT;

	xattr->xattr_minor_version	= OSSwapLittleToHostInt16(xattr->xattr_minor_version);
	xattr->flags				= OSSwapLittleToHostInt32(xattr->flags);
	xattr->persistent_class		= OSSwapLittleToHostInt32(xattr->persistent_class);
	xattr->key_os_version		= OSSwapLittleToHostInt32(xattr->key_os_version);
	xattr->key_revision			= OSSwapLittleToHostInt16(xattr->key_revision);
	xattr->key_len				= OSSwapLittleToHostInt16(xattr->key_len);

	uint16_t pers_key_len = xattr->key_len;

	min_len += pers_key_len;
	if (xattr_len < min_len)
		return HFS_EINCONSISTENT;

#if HFS_CONFIG_KEY_ROLL
	struct cp_roll_info *roll_info = NULL;

	if (ISSET(xattr->flags, CP_XAF_KEY_ROLLING)) {
		roll_info = PTR_ADD(struct cp_roll_info *, xattr, min_len);

		min_len += offsetof(struct cp_roll_info, key);

		if (xattr_len < min_len)
			return HFS_EINCONSISTENT;

		roll_info->off_rsrc = OSSwapLittleToHostInt64(roll_info->off_rsrc);

        if (roll_info->off_rsrc % hfsmp->blockSize)
            return HFS_EINCONSISTENT;

		roll_info->key_len = OSSwapLittleToHostInt16(roll_info->key_len);

		min_len += roll_info->key_len;
		if (xattr_len < min_len)
			return HFS_EINCONSISTENT;
	}
#endif

	cp_key_pair_t *cpkp = NULL;
	cprotect_t entry;
	
	/* 
	 * If option CP_GET_XATTR_BASIC_INFO is set, we only return basic
	 * information about the file's protection (and not the key) and
	 * we store the result in the structure the caller passed to us.
	 */
	if (ISSET(options, CP_GET_XATTR_BASIC_INFO)) {
		entry = *pcpr;
		bzero(entry, offsetof(struct cprotect, cp_keys));
#if HFS_CONFIG_KEY_ROLL
		if (ISSET(xattr->flags, CP_XAF_KEY_ROLLING)) {
			SET(entry->cp_flags, CP_KEY_IS_ROLLING);
		}
#endif
	} else {
		entry = cp_entry_alloc(NULL, xattr->key_len, CP_MAX_CACHEBUFLEN, &cpkp);
	}

	entry->cp_pclass			= xattr->persistent_class;
	entry->cp_key_os_version	= xattr->key_os_version;
	entry->cp_key_revision		= xattr->key_revision;

	if (!ISSET(options, CP_GET_XATTR_BASIC_INFO)) {
		if (xattr->key_len) {
			cpkp_set_pers_key_len(cpkp, xattr->key_len);
			memcpy(cpkp_pers_key(cpkp), xattr->persistent_key, xattr->key_len);
		}

#if HFS_CONFIG_KEY_ROLL
		if (roll_info) {
			entry->cp_key_roll_ctx = hfs_key_roll_ctx_alloc(NULL, roll_info->key_len,
															CP_MAX_CACHEBUFLEN, &cpkp);

			entry->cp_key_roll_ctx->ckr_off_rsrc = roll_info->off_rsrc;

			if (roll_info->key_len) {
				cpkp_set_pers_key_len(cpkp, roll_info->key_len);
				memcpy(cpkp_pers_key(cpkp), roll_info->key, roll_info->key_len);
			}
		}
#endif

		*pcpr = entry;
	}
	else if (xattr->key_len) {
		SET(entry->cp_flags, CP_HAS_A_KEY);
	}

	return 0;
}

/*
 * Initializes a new cprotect entry with xattr data from the cnode.
 * cnode lock held shared
 */
static int
cp_getxattr(struct cnode *cp, struct hfsmount *hfsmp, cprotect_t *outentry)
{
	size_t xattr_len;
	struct cp_xattr_v5 *xattr;

	xattr = hfs_malloc(xattr_len = sizeof(*xattr));

	int error = hfs_xattr_read(cp->c_vp, CONTENT_PROTECTION_XATTR_NAME,
							   xattr, &xattr_len);

	if (!error) {
		if (xattr_len < CP_XATTR_MIN_LEN)
			error = HFS_EINCONSISTENT;
		else
			error = cp_read_xattr_v5(hfsmp, xattr, xattr_len, outentry, 0);
	}

#if DEBUG
	if (error && error != ENOATTR) {
		printf("cp_getxattr: bad cp xattr (%d):\n", error);
		for (size_t i = 0; i < xattr_len; ++i)
			printf("%02x ", ((uint8_t *)xattr)[i]);
		printf("\n");
	}
#endif

	hfs_free(xattr, sizeof(*xattr));

	return error;
}

/*
 * If permitted, restore entry's unwrapped key from the persistent key.
 * If not, clear key and set CP_KEY_FLUSHED.
 * cnode lock held exclusive
 */
static int
cp_restore_keys(struct cprotect *entry, struct hfsmount *hfsmp, struct cnode *cp)
{
	int error = 0;

 	error = cp_unwrap(hfsmp, entry, cp);
	if (error) {
		cp_flush_cached_keys(entry);
		error = EPERM;
	}
	return error;
}

void cp_device_locked_callback(mount_t mp, cp_lock_state_t state)
{
	struct hfsmount *hfsmp;

	/*
	 * When iterating the various mount points that may
	 * be present on a content-protected device, we need to skip
	 * those that do not have it enabled.
	 */
	if (!cp_fs_protected(mp)) {
		return;
	}

	hfsmp = VFSTOHFS(mp);

	hfsmp->hfs_cp_lock_state = state;

	if (state == CP_LOCKED_STATE) {
		/* 
		 * We respond only to lock events.  Since cprotect structs
		 * decrypt/restore keys lazily, the unlock events don't
		 * actually cause anything to happen.
		 */
		vnode_iterate(mp, 0, cp_lock_vnode_callback, (void *)(uintptr_t)state);
	}
}

/*
 * Deny access to protected files if keys have been locked.
 */
static int
cp_check_access(struct cnode *cp, struct hfsmount *hfsmp, int vnop __unused)
{
	int error = 0;

	/* 
	 * For now it's OK to examine the state variable here without
	 * holding the HFS lock.  This is only a short-circuit; if the state
	 * transitions (or is in transition) after we examine this field, we'd
	 * have to handle that anyway. 
	 */
	if (hfsmp->hfs_cp_lock_state == CP_UNLOCKED_STATE) {
		return 0;
	}

	if (!cp->c_cpentry) {
		/* unprotected node */
		return 0;
	}

	if (!S_ISREG(cp->c_mode)) {
		return 0;
	}

	/* Deny all access for class A files */
	switch (CP_CLASS(cp->c_cpentry->cp_pclass)) {
		case PROTECTION_CLASS_A: {
			error = EPERM;
			break;
		}
		default:
			error = 0;
			break;
	}

	return error;
}

/*
 * Respond to a lock or unlock event.
 * On lock: clear out keys from memory, then flush file contents.
 * On unlock: nothing (function not called).
 */
static int
cp_lock_vnode_callback(struct vnode *vp, void *arg)
{
	cnode_t *cp = NULL;
	struct cprotect *entry = NULL;
	int error = 0;
	int locked = 1;
	unsigned long action = 0;
	int took_truncate_lock = 0;

	error = vnode_getwithref (vp);
	if (error) {
		return error;
	}

	cp = VTOC(vp);

	/*
	 * When cleaning cnodes due to a lock event, we must
	 * take the truncate lock AND the cnode lock.  By taking
	 * the truncate lock here, we force (nearly) all pending IOs
	 * to drain before we can acquire the truncate lock.  All HFS cluster
	 * io calls except for swapfile IO need to acquire the truncate lock
	 * prior to calling into the cluster layer.
	 */
	hfs_lock_truncate (cp, HFS_EXCLUSIVE_LOCK, HFS_LOCK_DEFAULT);
	took_truncate_lock = 1;

	hfs_lock(cp, HFS_EXCLUSIVE_LOCK, HFS_LOCK_ALLOW_NOEXISTS);

	entry = cp->c_cpentry;
	if (!entry) {
		/* unprotected vnode: not a regular file */
		goto out;
	}

	action = (unsigned long) arg;
	switch (action) {
		case CP_LOCKED_STATE: {
			vfs_context_t ctx;
			if (CP_CLASS(entry->cp_pclass) != PROTECTION_CLASS_A ||
				vnode_isdir(vp)) {
				/*
				 * There is no change at lock for other classes than A.
				 * B is kept in memory for writing, and class F (for VM) does
				 * not have a wrapped key, so there is no work needed for
				 * wrapping/unwrapping.
				 *
				 * Note that 'class F' is relevant here because if
				 * hfs_vnop_strategy does not take the cnode lock
				 * to protect the cp blob across IO operations, we rely
				 * implicitly on the truncate lock to be held when doing IO.
				 * The only case where the truncate lock is not held is during
				 * swapfile IO because HFS just funnels the VNOP_PAGEOUT
				 * directly to cluster_pageout.
				 */
				goto out;
			}

			/* Before doing anything else, zero-fill sparse ranges as needed */
			ctx = vfs_context_current();
			(void) hfs_filedone (vp, ctx, 0);

			/* first, sync back dirty pages */
			hfs_unlock (cp);
			ubc_msync (vp, 0, ubc_getsize(vp), NULL, UBC_PUSHALL | UBC_INVALIDATE | UBC_SYNC);
			hfs_lock (cp, HFS_EXCLUSIVE_LOCK, HFS_LOCK_ALLOW_NOEXISTS);

			/* flush keys:
			 * There was a concern here(9206856) about flushing keys before nand layer is done using them.
			 * But since we are using ubc_msync with UBC_SYNC, it blocks until all IO is completed.
			 * Once IOFS caches or is done with these keys, it calls the completion routine in IOSF.
			 * Which in turn calls buf_biodone() and eventually unblocks ubc_msync()
			 * Also verified that the cached data in IOFS is overwritten by other data, and there
			 * is no key leakage in that layer.
			 */

			cp_flush_cached_keys(entry);

			/* some write may have arrived in the mean time. dump those pages */
			hfs_unlock(cp);
			locked = 0;

			ubc_msync (vp, 0, ubc_getsize(vp), NULL, UBC_INVALIDATE | UBC_SYNC);
			break;
		}
		case CP_UNLOCKED_STATE: {
			/* no-op */
			break;
		}
		default:
			panic("Content Protection: unknown lock action %lu\n", action);
	}

out:
	if (locked) {
		hfs_unlock(cp);
	}

	if (took_truncate_lock) {
		hfs_unlock_truncate (cp, HFS_LOCK_DEFAULT);
	}

	vnode_put (vp);
	return error;
}


/* 
 * cp_rewrap:
 *
 * Generate a new wrapped key based on the existing cache key.
 */

int
cp_rewrap(struct cnode *cp, __unused hfsmount_t *hfsmp,
		  cp_key_class_t *newclass, cp_key_pair_t *cpkp, const void *old_holder,
		  cp_new_alloc_fn alloc_fn, void **pholder)
{
	struct cprotect *entry = cp->c_cpentry;

	uint8_t new_persistent_key[CP_MAX_WRAPPEDKEYSIZE];
	unsigned keylen = CP_MAX_WRAPPEDKEYSIZE;
	int error = 0;
	const cp_key_class_t key_class = CP_CLASS(*newclass);

	/* Structures passed between HFS and AKS */
	struct aks_cred_s access_in;
	struct aks_wrapped_key_s wrapped_key_in;
	struct aks_wrapped_key_s wrapped_key_out;

	/*
	 * PROTECTION_CLASS_F is in-use by VM swapfile; it represents a transient
	 * key that is only good as long as the file is open.  There is no
	 * wrapped key, so there isn't anything to wrap.
	 */
	if (key_class == PROTECTION_CLASS_F) {
		return EINVAL;
	}

	cp_init_access(&access_in, cp);

	bzero(&wrapped_key_in, sizeof(wrapped_key_in));
	wrapped_key_in.key = cpkp_pers_key(cpkp);
	wrapped_key_in.key_len = cpkp_pers_key_len(cpkp);
	/* Use the persistent class when talking to AKS */
	wrapped_key_in.dp_class = entry->cp_pclass;

	bzero(&wrapped_key_out, sizeof(wrapped_key_out));
	wrapped_key_out.key = new_persistent_key;
	wrapped_key_out.key_len = keylen;

	/*
	 * inode is passed here to find the backup bag wrapped blob
	 * from userspace.  This lookup will occur shortly after creation
	 * and only if the file still exists.  Beyond this lookup the
	 * inode is not used.  Technically there is a race, we practically
	 * don't lose.
	 */
	error = hfs_rewrap_key(&access_in,
						   key_class, /* new class */
						   &wrapped_key_in,
						   &wrapped_key_out);

	keylen = wrapped_key_out.key_len;

	if (error == 0) {
		/*
		 * Verify that AKS returned to us a wrapped key of the 
		 * target class requested.   
		 */
		/* Get the effective class here */
		cp_key_class_t effective = CP_CLASS(wrapped_key_out.dp_class);
		if (effective != key_class) {
			/* 
			 * Fail the operation if defaults or some other enforcement
			 * dictated that the class be wrapped differently. 
			 */

			/* TODO: Invalidate the key when 12170074 unblocked */
			return EPERM;
		}

		/* Allocate a new cpentry */
		cp_key_pair_t *new_cpkp;
		*pholder = alloc_fn(old_holder, keylen, CP_MAX_CACHEBUFLEN, &new_cpkp);

		/* copy the new key into the entry */
		cpkp_set_pers_key_len(new_cpkp, keylen);
		memcpy(cpkp_pers_key(new_cpkp), new_persistent_key, keylen);

		/* Actually record/store what AKS reported back, not the effective class stored in newclass */
		*newclass = wrapped_key_out.dp_class;
	}
	else {
		error = EPERM;
	}

	return error;
}

static int cpkp_unwrap(cnode_t *cp, cp_key_class_t key_class, cp_key_pair_t *cpkp)
{
	int error = 0;
	uint8_t iv_key[CP_IV_KEYSIZE];
	cpx_t cpx = cpkp_cpx(cpkp);

	/* Structures passed between HFS and AKS */
	struct aks_cred_s access_in;
	struct aks_wrapped_key_s wrapped_key_in;
	struct aks_raw_key_s key_out;

	cp_init_access(&access_in, cp);

	bzero(&wrapped_key_in, sizeof(wrapped_key_in));
	wrapped_key_in.key = cpkp_pers_key(cpkp);
	wrapped_key_in.key_len = cpkp_max_pers_key_len(cpkp);
	/* Use the persistent class when talking to AKS */
	wrapped_key_in.dp_class = key_class;

	bzero(&key_out, sizeof(key_out));
	key_out.iv_key = iv_key;
	key_out.key = cpx_key(cpx);
	/*
	 * The unwrapper should validate/set the key length for
	 * the IV key length and the cache key length, however we need
	 * to supply the correct buffer length so that AKS knows how
	 * many bytes it has to work with.
	 */
	key_out.iv_key_len = CP_IV_KEYSIZE;
	key_out.key_len = cpx_max_key_len(cpx);

	error = hfs_unwrap_key(&access_in, &wrapped_key_in, &key_out);
	if (!error) {
		if (key_out.key_len == 0 || key_out.key_len > CP_MAX_CACHEBUFLEN) {
			panic ("cp_unwrap: invalid key length! (%ul)\n", key_out.key_len);
		}

		if (key_out.iv_key_len != CP_IV_KEYSIZE)
			panic ("cp_unwrap: invalid iv key length! (%ul)\n", key_out.iv_key_len);

		cpx_set_key_len(cpx, key_out.key_len);

		cpx_set_aes_iv_key(cpx, iv_key);
		cpx_set_is_sep_wrapped_key(cpx, ISSET(key_out.flags, AKS_RAW_KEY_WRAPPEDKEY));
	} else {
		error = EPERM;
	}

	return error;
}

static int
cp_unwrap(__unused struct hfsmount *hfsmp, struct cprotect *entry, struct cnode *cp)
{
	/*
	 * PROTECTION_CLASS_F is in-use by VM swapfile; it represents a transient
	 * key that is only good as long as the file is open.  There is no
	 * wrapped key, so there isn't anything to unwrap.
	 */
	if (CP_CLASS(entry->cp_pclass) == PROTECTION_CLASS_F) {
		return EPERM;
	}

	int error = cpkp_unwrap(cp, entry->cp_pclass, &entry->cp_keys);

#if HFS_CONFIG_KEY_ROLL
	if (!error && entry->cp_key_roll_ctx) {
		error = cpkp_unwrap(cp, entry->cp_pclass, &entry->cp_key_roll_ctx->ckr_keys);
		if (error)
			cpx_flush(cpkp_cpx(&entry->cp_keys));
	}
#endif

	return error;
}

/*
 * cp_generate_keys
 *
 * Take a cnode that has already been initialized and establish persistent and
 * cache keys for it at this time. Note that at the time this is called, the
 * directory entry has already been created and we are holding the cnode lock
 * on 'cp'.
 * 
 */
int cp_generate_keys (struct hfsmount *hfsmp, struct cnode *cp, cp_key_class_t targetclass,
		uint32_t keyflags, struct cprotect **newentry) 
{

	int error = 0;
	struct cprotect *newcp = NULL;
	*newentry = NULL;

	/* Target class must be an effective class only */
	targetclass = CP_CLASS(targetclass);

	/* Validate that it has a cprotect already */
	if (cp->c_cpentry == NULL) {
		/* We can't do anything if it shouldn't be protected. */
		return 0;
	}	

	/* Asserts for the underlying cprotect */
	if (cp->c_cpentry->cp_flags & CP_NO_XATTR) {
		/* should already have an xattr by this point. */
		error = EINVAL;
		goto out;
	}

	if (S_ISREG(cp->c_mode)) {
		if (!cp_needs_pers_key(cp->c_cpentry)) {
			error = EINVAL;
			goto out;
		}
	}

	cp_key_revision_t key_revision = cp_initial_key_revision(hfsmp);

	error = cp_new (&targetclass, hfsmp, cp, cp->c_mode, keyflags, key_revision,
					(cp_new_alloc_fn)cp_entry_alloc, (void **)&newcp);
	if (error) {
		/* 
		 * Key generation failed. This is not necessarily fatal
		 * since the device could have transitioned into the lock 
		 * state before we called this.  
		 */	
		error = EPERM;
		goto out;
	}

	newcp->cp_pclass			= targetclass;
	newcp->cp_key_os_version	= cp_os_version();
	newcp->cp_key_revision		= key_revision;

	/*
	 * If we got here, then we have a new cprotect.
	 * Attempt to write the new one out.
	 */
	error = cp_setxattr (cp, newcp, hfsmp, cp->c_fileid, XATTR_REPLACE);

	if (error) {
		/* Tear down the new cprotect; Tell MKB that it's invalid. Bail out */
		/* TODO: rdar://12170074 needs to be fixed before we can tell MKB */
		if (newcp) {
			cp_entry_destroy(hfsmp, newcp);
		}	
		goto out;
	}

	/* 
	 * If we get here then we can assert that:
	 * 1) generated wrapped/unwrapped keys.
	 * 2) wrote the new keys to disk.
	 * 3) cprotect is ready to go.
	 */

	*newentry = newcp;

out:
	return error;

}

void cp_replace_entry (hfsmount_t *hfsmp, struct cnode *cp, struct cprotect *newentry)
{
	if (cp->c_cpentry) {
#if HFS_CONFIG_KEY_ROLL
		// Transfer the tentative reservation
		if (cp->c_cpentry->cp_key_roll_ctx && newentry->cp_key_roll_ctx) {
			newentry->cp_key_roll_ctx->ckr_tentative_reservation
				= cp->c_cpentry->cp_key_roll_ctx->ckr_tentative_reservation;

			cp->c_cpentry->cp_key_roll_ctx->ckr_tentative_reservation = NULL;
		}
#endif

		cp_entry_destroy (hfsmp, cp->c_cpentry);
	}
	cp->c_cpentry = newentry;
	newentry->cp_backing_cnode = cp;

	return;
}


/*
 * cp_new
 *
 * Given a double-pointer to a cprotect, generate keys (either in-kernel or from keystore),
 * allocate a cprotect, and vend it back to the caller.
 * 
 * Additionally, decide if keys are even needed -- directories get cprotect data structures
 * but they do not have keys.
 *
 */

int
cp_new(cp_key_class_t *newclass_eff, __unused struct hfsmount *hfsmp, struct cnode *cp,
	   mode_t cmode, int32_t keyflags, cp_key_revision_t key_revision,
	   cp_new_alloc_fn alloc_fn, void **pholder)
{
	int error = 0;
	uint8_t new_key[CP_MAX_CACHEBUFLEN];
	unsigned new_key_len = CP_MAX_CACHEBUFLEN;  /* AKS tell us the proper key length, how much of this is used */
	uint8_t new_persistent_key[CP_MAX_WRAPPEDKEYSIZE];
	unsigned new_persistent_len = CP_MAX_WRAPPEDKEYSIZE;
	uint8_t iv_key[CP_IV_KEYSIZE];
	unsigned iv_key_len = CP_IV_KEYSIZE;
	int iswrapped = 0;
	cp_key_class_t key_class = CP_CLASS(*newclass_eff);

	/* Structures passed between HFS and AKS */
	struct aks_cred_s access_in;
	struct aks_wrapped_key_s wrapped_key_out;
	struct aks_raw_key_s key_out;

	/* Sanity check that it's a file or directory here */
	if (!(S_ISREG(cmode)) && !(S_ISDIR(cmode))) {
		return EPERM;
	}

	/*
	 * Step 1: Generate Keys if needed.
	 * 
	 * For class F files, the kernel provides the key.
	 * PROTECTION_CLASS_F is in-use by VM swapfile; it represents a transient
	 * key that is only good as long as the file is open.  There is no
	 * wrapped key, so there isn't anything to wrap.
	 *
	 * For class A->D files, the key store provides the key 
	 * 
	 * For Directories, we only give them a class ; no keys.
	 */
	if (S_ISDIR (cmode)) {
		/* Directories */
		new_persistent_len = 0;
		new_key_len = 0;

		error = 0;
	}
	else {
		/* Must be a file */         
		if (key_class == PROTECTION_CLASS_F) {
			/* class F files are not wrapped; they can still use the max key size */
			new_key_len = CP_MAX_KEYSIZE;
			read_random (&new_key[0], new_key_len);
			new_persistent_len = 0;

			error = 0;
		}
		else {
			/* 
			 * The keystore is provided the file ID so that it can associate
			 * the wrapped backup blob with this key from userspace. This 
			 * lookup occurs after successful file creation.  Beyond this, the
			 * file ID is not used.  Note that there is a potential race here if
			 * the file ID is re-used.  
			 */
			cp_init_access(&access_in, cp);
		
			bzero(&key_out, sizeof(key_out));
			key_out.key = new_key;
			key_out.iv_key = iv_key;
			/* 
			 * AKS will override our key length fields, but we need to supply
			 * the length of the buffer in those length fields so that 
			 * AKS knows hoa many bytes it has to work with.
			 */
			key_out.key_len = new_key_len;
			key_out.iv_key_len = iv_key_len;

			bzero(&wrapped_key_out, sizeof(wrapped_key_out));
			wrapped_key_out.key = new_persistent_key;
			wrapped_key_out.key_len = new_persistent_len;

			access_in.key_revision = key_revision;

			error = hfs_new_key(&access_in,
								key_class,
								&key_out,
								&wrapped_key_out);

			if (error) {
				/* keybag returned failure */
				error = EPERM;
				goto cpnew_fail;
			}

			/* Now sanity-check the output from new_key */
			if (key_out.key_len == 0 || key_out.key_len > CP_MAX_CACHEBUFLEN) {
				panic ("cp_new: invalid key length! (%ul) \n", key_out.key_len);
			}

			if (key_out.iv_key_len != CP_IV_KEYSIZE) {
				panic ("cp_new: invalid iv key length! (%ul) \n", key_out.iv_key_len);
			}	
		
			/* 
			 * AKS is allowed to override our preferences and wrap with a 
			 * different class key for policy reasons. If we were told that 
			 * any class other than the one specified is unacceptable then error out 
			 * if that occurred.  Check that the effective class returned by 
			 * AKS is the same as our effective new class 
			 */
			if (CP_CLASS(wrapped_key_out.dp_class) != key_class) {
				if (!ISSET(keyflags, CP_KEYWRAP_DIFFCLASS)) {
					error = EPERM;
					/* TODO: When 12170074 fixed, release/invalidate the key! */
					goto cpnew_fail;
				}
			}

			*newclass_eff = wrapped_key_out.dp_class;
			new_key_len = key_out.key_len;
			iv_key_len = key_out.iv_key_len;
			new_persistent_len = wrapped_key_out.key_len;

			/* Is the key a SEP wrapped key? */
			if (key_out.flags & AKS_RAW_KEY_WRAPPEDKEY) {
				iswrapped = 1;
			}
		}
	}

	/*
	 * Step 2: allocate cprotect and initialize it.
	 */

	cp_key_pair_t *cpkp;
	*pholder = alloc_fn(NULL, new_persistent_len, new_key_len, &cpkp);
	if (*pholder == NULL) {
		return ENOMEM;
	}

	/* Copy the cache key & IV keys into place if needed. */
	if (new_key_len > 0) {
		cpx_t cpx = cpkp_cpx(cpkp);

		cpx_set_key_len(cpx, new_key_len);
		memcpy(cpx_key(cpx), new_key, new_key_len);

		/* Initialize the IV key */
		if (key_class != PROTECTION_CLASS_F)
			cpx_set_aes_iv_key(cpx, iv_key);

		cpx_set_is_sep_wrapped_key(cpx, iswrapped);
	}
	if (new_persistent_len > 0) {
		cpkp_set_pers_key_len(cpkp, new_persistent_len);
		memcpy(cpkp_pers_key(cpkp), new_persistent_key, new_persistent_len);
	}

cpnew_fail:

#if HFS_TMPDBG
#if !SECURE_KERNEL
	if ((hfsmp->hfs_cp_verbose) && (error == EPERM)) {
		/* Only introspect the data fork */
		cp_log_eperm (cp->c_vp, *newclass_eff, true);
	}
#endif
#endif

	return error;
}


/* Initialize the aks_cred_t structure passed to AKS */
static void cp_init_access(aks_cred_t access, struct cnode *cp)
{
	vfs_context_t context = vfs_context_current();
	kauth_cred_t cred = vfs_context_ucred(context);
	proc_t proc = vfs_context_proc(context);
	struct hfsmount *hfsmp;
	struct vnode *vp;
	uuid_t hfs_uuid;

	bzero(access, sizeof(*access));

	vp = CTOV(cp, 0);
	if (vp == NULL) {
		/* is it a rsrc */
		vp = CTOV(cp,1);
		if (vp == NULL) {
			//leave the struct bzeroed. 
			return;
		}
	}

	hfsmp = VTOHFS(vp);
	hfs_getvoluuid(hfsmp, hfs_uuid);

	/* Note: HFS uses 32-bit fileID, even though inode is a 64-bit value */
	access->inode = cp->c_fileid;
	access->pid = proc_pid(proc);
	access->uid = kauth_cred_getuid(cred);
	uuid_copy (access->volume_uuid, hfs_uuid);	

	if (cp->c_cpentry)
		access->key_revision = cp->c_cpentry->cp_key_revision;

	return;
}

#if HFS_CONFIG_KEY_ROLL

errno_t cp_set_auto_roll(hfsmount_t *hfsmp,
						 const hfs_key_auto_roll_args_t *args)
{
	// 64 bytes should be OK on the stack
	_Static_assert(sizeof(struct cp_root_xattr) < 64, "cp_root_xattr too big!");

	struct cp_root_xattr xattr;
	errno_t ret;

	ret = cp_getrootxattr(hfsmp, &xattr);
	if (ret)
		return ret;

	ret = hfs_start_transaction(hfsmp);
	if (ret)
		return ret;

	xattr.auto_roll_min_version = args->min_key_os_version;
	xattr.auto_roll_max_version = args->max_key_os_version;

	bool roll_old_class_gen = ISSET(args->flags, HFS_KEY_AUTO_ROLL_OLD_CLASS_GENERATION);

	if (roll_old_class_gen)
		SET(xattr.flags, CP_ROOT_AUTO_ROLL_OLD_CLASS_GENERATION);
	else
		CLR(xattr.flags, CP_ROOT_AUTO_ROLL_OLD_CLASS_GENERATION);

	ret = cp_setrootxattr(hfsmp, &xattr);

	errno_t ret2 = hfs_end_transaction(hfsmp);

	if (!ret)
		ret = ret2;

	if (ret)
		return ret;

	hfs_lock_mount(hfsmp);
	hfsmp->hfs_auto_roll_min_key_os_version = args->min_key_os_version;
	hfsmp->hfs_auto_roll_max_key_os_version = args->max_key_os_version;
	hfs_unlock_mount(hfsmp);

	return ret;
}

bool cp_should_auto_roll(hfsmount_t *hfsmp, cprotect_t cpr)
{
	if (cpr->cp_key_roll_ctx) {
		// Already rolling
		return false;
	}

	// Only automatically roll class A, B & C
	if (CP_CLASS(cpr->cp_pclass) < PROTECTION_CLASS_A
		|| CP_CLASS(cpr->cp_pclass) > PROTECTION_CLASS_C) {
		return false;
	}

	if (!cpkp_has_pers_key(&cpr->cp_keys))
		return false;

	/*
	 * Remember, the class generation stored in HFS+ is updated at the *end*,
	 * so it's old if it matches the generation we have stored.
	 */
	if (ISSET(hfsmp->cproot_flags, CP_ROOT_AUTO_ROLL_OLD_CLASS_GENERATION)
		&& cp_get_crypto_generation(cpr->cp_pclass) == hfsmp->cp_crypto_generation) {
		return true;
	}

	if (!hfsmp->hfs_auto_roll_min_key_os_version
		&& !hfsmp->hfs_auto_roll_max_key_os_version) {
		// No minimum or maximum set
		return false;
	}

	if (hfsmp->hfs_auto_roll_min_key_os_version
		&& cpr->cp_key_os_version < hfsmp->hfs_auto_roll_min_key_os_version) {
		// Before minimum
		return false;
	}

	if (hfsmp->hfs_auto_roll_max_key_os_version
		&& cpr->cp_key_os_version >= hfsmp->hfs_auto_roll_max_key_os_version) {
		// Greater than maximum
		return false;
	}

	return true;
}

#endif // HFS_CONFIG_KEY_ROLL

errno_t cp_handle_strategy(buf_t bp)
{
	vnode_t vp = buf_vnode(bp);
	cnode_t *cp = NULL;

	if (bufattr_rawencrypted(buf_attr(bp))
		|| !(cp = cp_get_protected_cnode(vp))
		|| !cp->c_cpentry) {
		// Nothing to do
		return 0;
	}

	/*
	 * For filesystem resize, we may not have access to the underlying
	 * file's cache key for whatever reason (device may be locked).
	 * However, we do not need it since we are going to use the
	 * temporary HFS-wide resize key which is generated once we start
	 * relocating file content.  If this file's I/O should be done
	 * using the resize key, it will have been supplied already, so do
	 * not attach the file's cp blob to the buffer.
	 */
	if (ISSET(cp->c_cpentry->cp_flags, CP_RELOCATION_INFLIGHT))
		return 0;

#if HFS_CONFIG_KEY_ROLL
	/*
	 * We don't require any locks here.  Pages will be locked so no
	 * key rolling can take place until this I/O has completed.
	 */
	if (!cp->c_cpentry->cp_key_roll_ctx)
#endif
	{
		// Fast path
		cpx_t cpx = cpkp_cpx(&cp->c_cpentry->cp_keys);

		if (cpx_has_key(cpx)) {
			bufattr_setcpx(buf_attr(bp), cpx);
			return 0;
		}
	}

	/*
	 * We rely mostly (see note below) upon the truncate lock to
	 * protect the CP cache key from getting tossed prior to our IO
	 * finishing here.  Nearly all cluster io calls to manipulate file
	 * payload from HFS take the truncate lock before calling into the
	 * cluster layer to ensure the file size does not change, or that
	 * they have exclusive right to change the EOF of the file.  That
	 * same guarantee protects us here since the code that deals with
	 * CP lock events must now take the truncate lock before doing
	 * anything.
	 *
	 * If you want to change content protection structures, then the
	 * truncate lock is not sufficient; you must take the truncate
	 * lock and then wait for outstanding writes to complete.  This is
	 * necessary because asynchronous I/O only holds the truncate lock
	 * whilst I/O is being queued.
	 *
	 * One exception should be the VM swapfile IO, because HFS will
	 * funnel the VNOP_PAGEOUT directly into a cluster_pageout call
	 * for the swapfile code only without holding the truncate lock.
	 * This is because individual swapfiles are maintained at
	 * fixed-length sizes by the VM code.  In non-swapfile IO we use
	 * PAGEOUT_V2 semantics which allow us to create our own UPL and
	 * thus take the truncate lock before calling into the cluster
	 * layer.  In that case, however, we are not concerned with the CP
	 * blob being wiped out in the middle of the IO because there
	 * isn't anything to toss; the VM swapfile key stays in-core as
	 * long as the file is open.
	 */

	off_rsrc_t off_rsrc = off_rsrc_make(buf_lblkno(bp) * GetLogicalBlockSize(vp),
										VNODE_IS_RSRC(vp));
	cp_io_params_t io_params;


	/*
	 * We want to take the cnode lock here and because the vnode write
	 * count is a pseudo-lock, we need to do something to preserve
	 * lock ordering; the cnode lock comes before the write count.
	 * Ideally, the write count would be incremented after the
	 * strategy routine returns, but that becomes complicated if the
	 * strategy routine where to call buf_iodone before returning.
	 * For now, we drop the write count here and then pick it up again
	 * later.
	 */
	if (!ISSET(buf_flags(bp), B_READ) && !ISSET(buf_flags(bp), B_RAW))
		vnode_writedone(vp);

	hfs_lock_always(cp, HFS_SHARED_LOCK);
	cp_io_params(VTOHFS(vp), cp->c_cpentry, off_rsrc,
				 ISSET(buf_flags(bp), B_READ) ? VNODE_READ : VNODE_WRITE,
				 &io_params);
	hfs_unlock(cp);

	/*
	 * Last chance: If this data protected I/O does not have unwrapped
	 * keys present, then try to get them.  We already know that it
	 * should, by this point.
	 */
	if (!cpx_has_key(io_params.cpx)) {
		int io_op = ( (buf_flags(bp) & B_READ) ? CP_READ_ACCESS : CP_WRITE_ACCESS);
		errno_t error = cp_handle_vnop(vp, io_op, 0);
		if (error) {
			/*
			 * We have to be careful here.  By this point in the I/O
			 * path, VM or the cluster engine has prepared a buf_t
			 * with the proper file offsets and all the rest, so
			 * simply erroring out will result in us leaking this
			 * particular buf_t.  We need to properly decorate the
			 * buf_t just as buf_strategy would so as to make it
			 * appear that the I/O errored out with the particular
			 * error code.
			 */
			if (!ISSET(buf_flags(bp), B_READ) && !ISSET(buf_flags(bp), B_RAW))
				vnode_startwrite(vp);
			buf_seterror (bp, error);
			buf_biodone(bp);
			return error;
		}

		hfs_lock_always(cp, HFS_SHARED_LOCK);
		cp_io_params(VTOHFS(vp), cp->c_cpentry, off_rsrc,
					 ISSET(buf_flags(bp), B_READ) ? VNODE_READ : VNODE_WRITE,
					 &io_params);
		hfs_unlock(cp);
	}

	hfs_assert(buf_count(bp) <= io_params.max_len);
	bufattr_setcpx(buf_attr(bp), io_params.cpx);

	if (!ISSET(buf_flags(bp), B_READ) && !ISSET(buf_flags(bp), B_RAW))
		vnode_startwrite(vp);

	return 0;
}

#endif /* CONFIG_PROTECT */
