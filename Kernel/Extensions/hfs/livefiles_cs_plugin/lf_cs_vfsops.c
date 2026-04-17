//
// Copyright (c) 2019-2019 Apple Inc. All rights reserved.
//
// lf_cs_vfsops.c - Implemenents routines for handling VFS operations for
//                  livefiles Apple_CoreStorage plugin.
//

#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/disk.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/storage/IOMedia.h>
#include <MediaKit/GPTTypes.h>
#include <IOKit/IOBSD.h>
#include <IOKit/IOTypes.h>

#include "lf_cs_checksum.h"
#include "lf_cs_disk_format.h"
#include "lf_cs_logging.h"
#include "lf_cs.h"

#define VALUE_UNSPECIFIED ((uint64_t)-1)

//
// Verify the given data checksum.  If no explicit 'chk' area the cksum
// value to match against is assumed to reside at the beginning of the
// provided buffer (typically a 'metadata_header_t') and is excluded from
// the cksum calculation itself.
//
static unsigned
cs_verify_cksum(cksum_alg_t alg, const void *ptr, size_t len,
		uint8_t *chk)
{
	uint8_t c[MAX_CKSUM_NBYTES];

	if (!(alg == CKSUM_NONE) && !(alg == CKSUM_ALG_CRC_32)) {
		return CS_STATUS_INVALID;
	}

	cksum_init(alg, c);
	if (chk == NULL) {
		cksum(alg, (chk = (uint8_t *)ptr) + MAX_CKSUM_NBYTES,
				len - MAX_CKSUM_NBYTES, c);
	} else {
		cksum(alg, ptr, len, c);
	}

	if (memcmp(c, chk, MAX_CKSUM_NBYTES)) {
		return CS_STATUS_CKSUM;
	}

	return CS_STATUS_OK;
}

//
// These block types must belong to the current transaction; all
// other types can be referenced from older transactions.
//
#define CS_CHKPOINT_BLK(_b) \
	((_b)->mh_blk_type >= BLK_TYPE_VOL_HEADER &&\
		(_b)->mh_blk_type <= BLK_TYPE_SUT)


//
//  These block types are the only ones with Secure Deletion content (LVF).
//
#define CS_DSDLIST_BLK(_b) \
	((_b)->mh_blk_type == BLK_TYPE_LV_FAMILY_SUPERBLOCK ||\
		(_b)->mh_blk_type == BLK_TYPE_LV_FAMILY_XATTR)

//
// cs_verify_blkhdr - Verify standard metadata header fields.
//
// Each block begins with a 'metadata_header_t', which often has known/expected
// values.
//
static unsigned
cs_verify_blkhdr(const metadata_header_t *hdr, uint16_t alg,
		uint8_t type, uint8_t subtype, uint64_t vaddr,
		uint64_t laddr, uint64_t txg, uint32_t blksz)
{
	unsigned status;

	if (alg != CKSUM_NONE && (status = cs_verify_cksum((cksum_alg_t)alg,
					hdr, blksz, NULL)) != CS_STATUS_OK) {
		return status;
	}

	if (type != BLK_TYPE_UNKNOWN && hdr->mh_blk_type != type) {
		return CS_STATUS_BLKTYPE;
	}

	if (subtype != BLK_SUBTYPE_UNKNOWN && hdr->mh_blk_subtype != subtype) {
		return CS_STATUS_INVALID;
	}

	if (vaddr != VALUE_UNSPECIFIED && hdr->mh_vaddr != vaddr) {
		return CS_STATUS_ADDRESS;
	}

	if (laddr != VALUE_UNSPECIFIED && hdr->mh_laddr != laddr) {
		return CS_STATUS_ADDRESS;
	}

	if (txg != VALUE_UNSPECIFIED) {
		if (CS_CHKPOINT_BLK(hdr) && hdr->mh_txg_id != txg)
			return CS_STATUS_TXG;
		else if (hdr->mh_txg_id > txg)
			return CS_STATUS_TXG;
	}

	if (blksz != 0 && hdr->mh_blk_size != blksz) {
		return CS_STATUS_INVALID;
	}

	if (!CS_DSDLIST_BLK(hdr) &&
			(hdr->mh_blk_flags & BLK_FLAG_IN_DSD_LIST)) {
		return CS_STATUS_INVALID;
	}

	return CS_STATUS_OK;
}

//
// cs_verify_versions - Verify volume header version.
//
static unsigned
cs_verify_versions(const metadata_header_t *hdr)
{
	if (hdr != NULL && hdr->mh_format_version !=
			CORESTORAGE_FORMAT_VERSION) {
		return CS_INFO_VERSIONITIS | CS_STATUS_NOTCS;
	}

	if (hdr != NULL &&
			hdr->mh_blk_type == BLK_TYPE_VOL_HEADER &&
			((dk_vol_header_t *)hdr)->vh_endianness !=
			BYTE_ORDER_MARK) {
		return CS_INFO_VERSIONITIS | CS_STATUS_NOTCS;
	}

	return CS_STATUS_OK;
}

//
// cs_verify_vh - Read and verify the Volume Headers (first/last 512-bytes of
//                the PV).
//
static unsigned
cs_verify_vh(dk_vol_header_t *hdr, uint32_t blksz)
{
	unsigned status;

	status = cs_verify_blkhdr(&hdr->vh_header, hdr->vh_cksum_alg,
			BLK_TYPE_VOL_HEADER,
			BLK_SUBTYPE_NO_SUBTYPE, 0, 0,
			VALUE_UNSPECIFIED, VOL_HEADER_NBYTES);
	if (status != CS_STATUS_OK) {
		return status;
	}

	status = cs_verify_versions(&hdr->vh_header);
	if (status != CS_STATUS_OK) {
		return status;
	}

	if (!hdr->vh_num_labels || hdr->vh_num_labels > MAX_DISK_LABELS) {
		return CS_STATUS_INVALID;
	}

	if (hdr->vh_label_max_nbytes % blksz != 0) {
		return CS_STATUS_INVALID;
	}

	if (hdr->vh_blksz == 0 || hdr->vh_blksz % blksz != 0) {
		return CS_STATUS_INVALID;
	}

	if (hdr->vh_pv_nbytes % blksz != 0 ||
			hdr->vh_pv_nbytes < CS_ALIGN(VOL_HEADER_NBYTES,
				blksz, true) * NUM_VOL_HEADERS) {
		return CS_STATUS_INVALID;
	}

	if (hdr->vh_pv_resize != 0 && (hdr->vh_pv_resize % blksz != 0 ||
				hdr->vh_pv_resize < CS_ALIGN(VOL_HEADER_NBYTES,
					blksz, true) * NUM_VOL_HEADERS)) {
		return CS_STATUS_INVALID;
	}

	if (hdr->vh_old_pv_nbytes != 0 &&
			(hdr->vh_old_pv_nbytes % blksz != 0 ||
			 hdr->vh_old_pv_nbytes < CS_ALIGN(VOL_HEADER_NBYTES,
				 blksz, true) * NUM_VOL_HEADERS)) {
		return CS_STATUS_INVALID;
	}

	return CS_STATUS_OK;
}

//
// cs_get_content_hint_for_pv - get content hint of the disk from IOReg. We
//                              read the IOReg to check if the passed disk has
//                              a Apple_CoreStorage content hint.
//
static bool
cs_get_content_hint_for_pv(struct stat *st)
{
	bool has_cs_hint;
	int dev_major, dev_minor;
	CFMutableDictionaryRef matching;

	has_cs_hint = false;
	if ((matching = IOServiceMatching(kIOMediaClass))) {
		CFTypeRef str;
		io_service_t service;

		if ((matching = IOServiceMatching(kIOMediaClass))) {
			CFNumberRef num_ref;

			dev_major = major(st->st_rdev);
			dev_minor = minor(st->st_rdev);

			num_ref = CFNumberCreate(kCFAllocatorDefault,
					kCFNumberIntType, &dev_major);
			if (num_ref) {
				CFDictionarySetValue(matching,
						CFSTR(kIOBSDMajorKey),
						num_ref);
				CFRelease(num_ref);
			}

			num_ref = CFNumberCreate(kCFAllocatorDefault,
					kCFNumberIntType, &dev_minor);
			if (num_ref) {
				CFDictionarySetValue(matching,
						CFSTR(kIOBSDMinorKey),
						num_ref);
				CFRelease(num_ref);
			}

			service = IOServiceGetMatchingService(
					kIOMasterPortDefault, matching);
			if (!service) {
				goto out;
			}

			if ((str = IORegistryEntryCreateCFProperty( service,
						CFSTR(kIOMediaContentHintKey),
						kCFAllocatorDefault, 0)) !=
					NULL) {

				has_cs_hint = CFStringCompare((CFStringRef)str,
					CFSTR(APPLE_CORESTORAGE_UUID), 0) ==
					kCFCompareEqualTo;

				CFRelease(str);
				IOObjectRelease(service);
			}
		}
	}
out:
	return has_cs_hint;
}

static unsigned
cs_verify_hdrfields(void *block, uint16_t alg, uint8_t type,
		uint8_t subtype, uint64_t vaddr, uint64_t laddr,
		uint32_t blksz)
{
	unsigned status;

	if (alg != CKSUM_NONE && (status = cs_verify_cksum((cksum_alg_t)alg,
					block, blksz, NULL)) !=
			CS_STATUS_OK) {
		return status;
	}

	if (type != BLK_TYPE_VOL_HEADER) {
		return CS_STATUS_BLKTYPE;
	}

	if (subtype != BLK_SUBTYPE_NO_SUBTYPE) {
		return CS_STATUS_INVALID;
	}

	if (vaddr != 0) {
		return CS_STATUS_ADDRESS;
	}

	if (laddr != 0) {
		return CS_STATUS_ADDRESS;
	}

	if (blksz != VOL_HEADER_NBYTES) {
		return CS_STATUS_INVALID;
	}

	return CS_STATUS_OK;
}

//
// Verify if a Volume Header error might actually be due to a stale format.
// Some historic block layouts were such that they spuriously fail recent
// validity checks (there was a relocation of some key identifying fields in
// the v9->v10->v11 switch).  Thus we follow-up a failure by some rudimentary
// probing (incl cksum) and override error.
//
static unsigned
cs_older_cs_version(dk_vol_header_t *hdr, unsigned status)
{
	uint16_t cksum_alg;

	struct {
		uint8_t         zeroes[VOL_HEADER_NBYTES];
	} inprogress = {0};

	struct v11_volhdr {
		uint8_t         mh_cksum[MAX_CKSUM_NBYTES];
		uint16_t        mh_format_version;
		uint8_t         mh_blk_type;
		uint8_t         mh_blk_subtype;
		uint32_t        mh_bundle_version;
		uint64_t        mh_txg_id;
		uint64_t        mh_vaddr;
		uint64_t        mh_laddr;
		uint64_t        mh_blk_owner;
		uint32_t        mh_blk_size;
		uint8_t         mh_blk_flags;
		uint8_t         mh_reserved1;
		uint16_t        mh_reserved2;
		uint64_t        mh_reserved8;
		uint64_t        vh_pv_nbytes;
		uint64_t        vh_pv_resize;
		uint64_t        vh_old_pv_nbytes;
		uint16_t        vh_endianness;
		uint16_t        vh_cksum_alg;
		uint16_t        vh_reserved2;
		uint16_t        vh_num_labels;
		uint32_t        vh_blksz;
		uint32_t        vh_label_max_nbytes;
		uint64_t        vh_label_addr[MAX_DISK_LABELS];
		uint64_t        vh_move_label_addr[MAX_DISK_LABELS];
		uint16_t        vh_wipe_key_nbytes[2];
		uint16_t        vh_wipe_key_alg[2];
		uint8_t         vh_wipe_key[2][MAX_WIPEKEY_NBYTES];
		uint8_t         vh_pv_uuid[16];
		uint8_t         vh_lvg_uuid[16];
	} *v11;


	if ((CS_STATUS(status) != CS_STATUS_OK) &&
			(hdr->vh_header.mh_format_version !=
			CORESTORAGE_FORMAT_VERSION)) {

		//
		// Ensure that the volume header is not totally empty before
		// trying to check if this is indeed an older version.
		//
		if (!memcmp(hdr, &inprogress, VOL_HEADER_NBYTES)) {
			return CS_INFO_ZERO_VH | CS_STATUS_NOTCS;
		}

		v11 = (struct v11_volhdr *)hdr;
		if (v11->mh_format_version == 11) {

			cksum_alg = v11->vh_cksum_alg;
			if (cs_verify_hdrfields(v11, cksum_alg,
						v11->mh_blk_type,
						v11->mh_blk_subtype,
						v11->mh_vaddr,
						v11->mh_laddr,
						v11->mh_blk_size) ==
					CS_STATUS_OK) {

				return CS_INFO_VERSIONITIS | CS_STATUS_NOTCS;
			}
		}
	}

	return status;
}

//
// cs_fd_is_corestorage_pv - taste if the disk is Apple_CoreStorage PV.
//
static int
cs_fd_is_corestorage_pv(int disk_fd, bool *is_cs_pv)
{
	struct stat st;
	bool pv_has_csuuid_hint;
	int vh_idx, error;
	uint32_t pv_blksz;
	uint64_t pv_nblks;
	uint64_t offset[NUM_VOL_HEADERS + 1];
	unsigned status[NUM_VOL_HEADERS + 1];

	dk_vol_header_t hdr[NUM_VOL_HEADERS + 1];

	error = 0;
	pv_blksz = 0;
	pv_nblks = 0;
	*is_cs_pv = false;
	pv_has_csuuid_hint = false;

	infomsg("Tasting Apple_CoreStorage plugin, fd: %d\n", disk_fd);

	//
	// Userfs corestorage plugin only supports block device. Thus, we
	// ensure that the passed device is block device before proceeding
	// further.
	//
	fstat(disk_fd, &st);
	if (!S_ISBLK(st.st_mode)) {
		errmsg("Apple_CoreStorage plugin only supports block "
				"device. Aborting taste.\n");
		return ENOTSUP;
	}

	//
	// Each PV has two volume headers, each has size 512 bytes, and resides
	// on the first and last 512 bytes of the PV. Each volume header has a
	// common block header with txg id and checksum, and only the volume
	// header with the right checksum and largest txg id is used when
	// mounting the LVG. Thus, to read the last volume header we need to
	// know the block-size and the number of blocks in the block device.
	//
	if (ioctl(disk_fd, DKIOCGETBLOCKSIZE, &pv_blksz) == -1) {
		error = errno;
	}

	if (!error && ioctl(disk_fd, DKIOCGETBLOCKCOUNT, &pv_nblks) == -1) {
		error = errno;
	}

	//
	// If we fail to determine the block size and block count, we bail out.
	//
	if (!error && (!pv_blksz || !pv_nblks)) {
		error = ENOTBLK;
	}

	if (error) {
		errmsg("Failed to get blocksize and block count "
				"for the device. Aborting taste.\n");
		return error;
	}

	//
	// Check if the device is tagged as being Apple_CoreStorage (Content
	// Hint). If not we bail-out now.
	//
	pv_has_csuuid_hint =  cs_get_content_hint_for_pv(&st);
	if (!pv_has_csuuid_hint) {
		*is_cs_pv = false;
		return 0;
	}

	//
	// We go through the two volume headers at offset 0 and at offset
	// (pv_nblks * pv_blksz - VOL_HEADER_NBYTES) and try to verify if
	// the volume headers are valid core storage physical volume volume
	// headers.
	//
	offset[0] = 0;
	offset[1] = pv_nblks * pv_blksz - VOL_HEADER_NBYTES;

	//
	// Initialize status as invalid.
	//
	for (vh_idx = 0; vh_idx < NUM_VOL_HEADERS; ++vh_idx ) {
		status[vh_idx] = CS_STATUS_INVALID;
	}

	for (vh_idx = 0; vh_idx < NUM_VOL_HEADERS; ++vh_idx) {
		ssize_t bytes_read;

		//
		// Read the PV volume header and cache it inside `hdr[vh_idx]`.
		//
		bytes_read = pread(disk_fd, &hdr[vh_idx], VOL_HEADER_NBYTES,
				offset[vh_idx]);

		if (bytes_read == -1) {
			error = errno;
		}

		if (!error && bytes_read != VOL_HEADER_NBYTES) {
			error = EIO;
		}

		if (error) {
			errmsg("Failed to read volume-hearder at offset %llu "
				"for disk with fd %d, Aborting taste.\n",
				offset[vh_idx], disk_fd);
			break;
		}

		//
		// Verify read volume-header.
		//
		status[vh_idx] = cs_verify_vh(&hdr[vh_idx], pv_blksz);
		if (CS_STATUS(status[vh_idx]) != CS_STATUS_OK) {

			//
			// Check if this physical volume has an older version
			// of header.
			//
			status[vh_idx] = cs_older_cs_version(&hdr[vh_idx],
					status[vh_idx]);
			if (CS_INFO(status[vh_idx]) & CS_INFO_VERSIONITIS) {

				infomsg("Disk with fd %u has older physical "
						"volume header format.\n",
						disk_fd);
				status[vh_idx] = CS_STATUS_OK;
			}
		}

		if (status[vh_idx] != CS_STATUS_OK) {
			break;
		}
	}

	if (error) {
		return error;
	}

	//
	// If there was no error and both the volume headers passed
	// verification that means this is a core storage physical volume.
	//
	for (vh_idx = 0; vh_idx < NUM_VOL_HEADERS; ++vh_idx ) {
		if (status[vh_idx] != CS_STATUS_OK) {
			break;
		}
		*is_cs_pv = true;
	}

	return 0;
}

//
// cs_uvfsop_taste - taste if a given disk is Apple_CoreStorage PV.
//
// disk_fd: file descriptor of the disk to taste.
//
// Returns:
//
// i) 0 if the passed disk is indeed an Apple_CoreStorage PV.
//
//  Or
//
// ii) ENOTSUP if the passed disk is not an Apple_CoreStorage PV.
//
//  Or
//
// iii) errno if there was some error attempting to taste the disk.
//
static int
cs_uvfsop_taste(int disk_fd)
{
	int error;
	bool is_cs_pv;

	//
	// Each PV has two volume headers, each has size 512 bytes, and resides
	// on the first and last 512 bytes of the PV. Each volume header has a
	// common block header with transaction-id and checksum, and only the
	// volume header with the right checksum and largest transaction-id is
	// used when mounting the LVG(logical volume group). To verify that
	// the disk with file descriptor is indeed a Apple_CoreStorage PV
	// (physical volume), we:
	//
	// i) Read IOReg to verify that it has `APPLE_CORESTORAGE_UUID` hint.
	//
	//    and
	//
	// ii) Verify the PV volume headers at the start and end of the disk.
	//
	// Please NOTE: This taste function is defensive (strict). We do a
	// strict match so that we ensure that we don't falsely match some
	// other volume format.
	//
	error = cs_fd_is_corestorage_pv(disk_fd, &is_cs_pv);
	if (error) {
		errmsg("Encountered error while tasting disk with file "
				"descriptor %d for Apple_CoreStorage "
				"plugin (error %d).\n", disk_fd, error);
		return error;

	}

	//
	// This is not an Apple_CoreStorage PV.
	//
	if (!is_cs_pv) {
		errmsg("Disk with file descriptor %d is not an corestorage "
				"physical volume.\n", disk_fd);
		return ENOTSUP;
	}

	//
	// We have found an Apple_CoreStorage PV, return success.
	//
	infomsg("Disk with file descriptor %d is corestorage physical "
			"volume.\n", disk_fd);

	return 0;
}

//
// Plugin lifecycle functions.
//
static int
cs_uvfsop_init(void)
{
	infomsg("Initializing CS UserFS plugin...\n");
	return 0;
}

static void
cs_uvfsop_fini(void)
{
	infomsg("Cleaning up CS UserFS plugin...\n");
}

//
// Plugin function registration.
//
UVFSFSOps cs_fsops = {
        .fsops_version = UVFS_FSOPS_VERSION_CURRENT,
        .fsops_init = cs_uvfsop_init,
        .fsops_fini = cs_uvfsop_fini,
        .fsops_taste = cs_uvfsop_taste,
};

__attribute__((visibility("default")))
void
livefiles_plugin_init(UVFSFSOps **ops)
{
        if (ops) {
                *ops = &cs_fsops;
        }
}
