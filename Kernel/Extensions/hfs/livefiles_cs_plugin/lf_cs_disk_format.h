//
// Copyright (c) 2009-2019 Apple Inc. All rights reserved.
//
// lf_cs_disk_format.h - Defines on disk format for Apple_CoreStorage physical
//                       volumes for livefiles Apple_CoreStorage plugin.
//
// This header is copied from CoreStorage project.
//

#ifndef _LF_CS_DISK_FORMAT_H
#define _LF_CS_DISK_FORMAT_H

#define MAX_CKSUM_NBYTES 8
#define CORESTORAGE_FORMAT_VERSION 1

//
// The 'dl_lvg.lvg_signature' in a DiskLabel (mimics 'CORESTOR')
//
#define CORE_STORAGE_SIGNATURE 0xC07E5707

//
// The 'mh_endianness' used to check endianness (non-palindromic 'CS')
//
#define BYTE_ORDER_MARK 0x5343

//
// Metadata block header.
//
struct metadata_header {

	//
	// Block cksum.
	//
	uint8_t mh_cksum[MAX_CKSUM_NBYTES];

	//
	// The on-disk format version of CoreStorage.
	//
	uint16_t mh_format_version;

	//
	// Enum metadata_blk_type (BLK_TYPE_*).
	//
	uint8_t mh_blk_type;

	//
	// Enum metadata_blk_subtype (BLK_SUBTYPE_*), for b-tree/fsck.
	//
	uint8_t mh_blk_subtype;

	//
	// The version of CoreStorage software that wrote this block.
	//
	uint32_t mh_bundle_version;

	//
	// Transaction identification.
	//
	uint64_t mh_txg_id;
	uint64_t mh_vaddr;
	uint64_t mh_laddr;

	//
	// Records the owner of the block.  The owner of any blocks in a B-Tree
	// is the root of the tree.  The owner of any blocks in a list like
	// structure (DSD list, attribute blocks) is the first block in the
	// list.  This field can be used for crash recovery by fsck_cs.  For
	// example, if this field has the B-Tree root vaddr that this block
	// belongs, fsck_cs could reconstruct the B-Tree based on all leaf
	// nodes.
	//
	uint64_t mh_blk_owner;

	//
	// Block size (LVG MLV blksz).
	//
	uint32_t mh_blk_size;

	//
	// Flags (BLK_FLAG_*).
	//
	uint8_t mh_blk_flags;

	//
	// Reserved for future extensions
	// make blk header 64 bytes (crypto alignment)
	//
	uint8_t mh_reserved1;
	uint16_t mh_reserved2;
	uint64_t mh_reserved8;
};
typedef struct metadata_header metadata_header_t;

int check_dk_metadata_header_size[sizeof(metadata_header_t) == 64 ? 1 : -1];


#define NUM_VOL_HEADERS 2
#define VOL_HEADER_NBYTES 512
#define MAX_DISK_LABELS 4
#define MAX_WIPEKEY_NBYTES 64

//
// This structure is padded such that it is of correct size.
//
struct dk_vol_header {
	union {
		struct {
			metadata_header_t vh_header;

			//
			// PV size in bytes, rounded down to VOL_HEADER_NBYTES
			// boundary.
			//
			uint64_t vh_pv_nbytes;

			//
			// If this is not 0, it records a PV resize operation
			//
			uint64_t vh_pv_resize;

			//
			// If this is not 0, it records the old Volume Header
			// location before the PV resize started; it is again 0
			// after completion.
			//
			uint64_t vh_old_pv_nbytes;

			//
			// The cpu-platform endian that formatted this LVG.
			//
			uint16_t vh_endianness;

			//
			// Checksum algorithm of VolHdr metadata header.
			//
			uint16_t vh_cksum_alg;

			//
			// Reserved.
			//
			uint16_t vh_reserved2;

			//
			// Number of disk labels.
			//
			uint16_t vh_num_labels;

			//
			// Label address uses this block size.
			//
			uint32_t vh_blksz;

			//
			// Maximum disk label size.
			//
			uint32_t vh_label_max_nbytes;

			//
			// (Physical) Location of the new/old disk labels.
			//
			uint64_t vh_label_addr[MAX_DISK_LABELS];
			uint64_t vh_move_label_addr[MAX_DISK_LABELS];

			//
			// PV MLV/plist wipe keys ([0] is active, [1] future
			// re-key).
			//
			uint16_t vh_wipe_key_nbytes[2];
			uint16_t vh_wipe_key_alg[2];
			uint8_t vh_wipe_key[2][MAX_WIPEKEY_NBYTES];

			//
			// UUID of this PV and its LVG (for bootstrap/multi-PV)
			//
			uint8_t vh_pv_uuid[16];
			uint8_t vh_lvg_uuid[16];
		};
		uint8_t vh_padding[VOL_HEADER_NBYTES];
	};
};
typedef struct dk_vol_header dk_vol_header_t;
int check_dk_vol_header_size[sizeof(dk_vol_header_t) == VOL_HEADER_NBYTES ? 1 : -1];

//
// Existing block types cannot be changed!
//
enum metadata_blk_type {

	//
	// The values of the B-Tree types are important.  They are the results
	// of the bit operations of BTREE_ROOT, BTREE_NODE, and BTREE_LEAF in
	// B-Tree code.
	//
	BLK_TYPE_BTREE_NODE = 2,      // A B-Tree node that is not a leaf.
	BLK_TYPE_BTREE_ROOT_NODE = 3, // A non-leaf B-Tree node that is also root.
	BLK_TYPE_BTREE_LEAF = 4,      // A B-Tree leaf node that is not a root.
	BLK_TYPE_BTREE_ROOT_LEAF = 5, // A B-Tree leaf node that is also root.

	BLK_TYPE_VOL_HEADER = 16,

	//
	// Fixed part of disk label (first MLV block size).
	//
	BLK_TYPE_DISK_LABEL = 17,

	BLK_TYPE_DISK_LABEL_CONT = 18, // Variable part of disk label.

	//
	// LFS related blocks
	//

	//
	// MLV Partial Segment (PS) header block, first block of the first
	// PS in a Group of Partial Segments (GPS).
	//
	BLK_TYPE_PS_HEADER = 19,

	//
	// MLV Continuation Partial Segment block, first block of a Partial
	// Segment (PS) in a Group of Partial Segments (GPS), where the PS
	// is not the first PS in the GPS.
	//
	BLK_TYPE_PS_CONT_HDR = 20,

	//
	// MLV Partial Segment (PS) Overflow header block, which holds
	// information about added or deleted virtual blocks.
	//
	BLK_TYPE_PS_OVERFLOW_HDR = 21,

	//
	// MLV Virtual Address Table (VAT) blocks, which holds VAT information.
	//
	BLK_TYPE_VAT = 22,

	//
	// MLV Segment Usage Table (SUT) blocks, which holds SUT information.
	//
	BLK_TYPE_SUT = 23,

	//
	// Metadata in MLV.
	//
	// MLV super block, the starting point of all MLV metadata.
	//
	BLK_TYPE_MLV_SUPERBLOCK = 24,

	//
	// Logical Volume Family (LVF) super block.
	//
	BLK_TYPE_LV_FAMILY_SUPERBLOCK = 25,

	//
	// Logical Volume (LV) super block.
	//
	BLK_TYPE_LV_SUPERBLOCK = 26,

	//
	// Hold more LV attributes when they don't fit in the LV superblock.
	//
	BLK_TYPE_LV_XATTR = 27,

	//
	// Holds the Physical Volume (PV) information.
	//
	BLK_TYPE_PV_TABLE = 28,

	//
	// Holds the Physical Volume (PV) Freespace Log (FLOG) table
	// information.
	//
	BLK_TYPE_PV_FLOG_TABLE = 29,

	BLK_TYPE_UNUSED_1 = 30,
	BLK_TYPE_UNUSED_2 = 31,
	BLK_TYPE_UNUSED_3 = 32,

	//
	// Used for space sharing, records the amount of free space not used by
	// the file system inside the Logical Volume.
	//
	BLK_TYPE_LV_FREE_SPACE_SUMMARY_DEPRECATED = 33,

	//
	// Used for Physical Volume (PV) free space management, records the
	// amount of free space in each chunk of the PV and has pointers to
	// the Freespace Log (FLOG).
	//
	BLK_TYPE_PV_SUMMARY_TABLE = 34,

	BLK_TYPE_UNUSED_4 = 35,

	//
	// Hold more LV Family attributes when they don't fit in the
	// LVF superblock.
	//
	BLK_TYPE_LV_FAMILY_XATTR = 36,

	//
	// Hold more Delayed Secure Deletion (DSD) list entries when
	// they cannot fit in the LVF superblock.
	//
	BLK_TYPE_DSD_LIST = 37,

	//
	// This block type is used to record information about bad sectors
	// encountered during background encryption/decryption.
	//
	BLK_TYPE_BAD_SECTOR_LIST = 38,

	//
	// For debugging: block is in dummy tree.
	//
	BLK_TYPE_DUMMY = 39,

	//
	// New types are added here to preserve compatible on-disk format.
	//

	NUM_BLK_TYPES,

	BLK_TYPE_UNKNOWN = 255, // Not used for on-disk blocks.
};

//
// Existing block subtypes cannot be changed!
//
enum metadata_blk_subtype {

	//
	// Metadata in MLV.
	//
	BLK_SUBTYPE_NO_SUBTYPE = 0,

	BLK_SUBTYPE_LV_FAMILY_TREE,
	BLK_SUBTYPE_LV_SNAPSHOT_TREE,
	BLK_SUBTYPE_LV_ADDR_MAP_TREE,
	BLK_SUBTYPE_PV_REFCOUNT_TREE,
	BLK_SUBTYPE_LV_UNUSED_SPACE_TREE,
	BLK_SUBTYPE_LVF_REFCOUNT_TREE,

	//
	// The b-tree that holds blocks to populate MLV for testing.
	//
	BLK_SUBTYPE_DUMMY_TREE,

	//
	// New types are added here to preserve compatible on-disk format.
	//

	NUM_BLK_SUBTYPES,

	BLK_SUBTYPE_UNKNOWN = 255, // Not used for on-disk blocks.
};
typedef enum metadata_blk_subtype metadata_blk_subtype_t;


//
// This block should be in the DSD List.  It is not in the DSD list when it
// is first created.  But when it is modified or killed, it will be included
// in the DSD list.
//
#define BLK_FLAG_IN_DSD_LIST 0x02

//
// On-disk information of LVG.
//
struct lvg_info {
	uint32_t lvg_signature;     // A signature that CoreStorage recognizes.

	//
	// Version numbers can help diagnose problems.
	//

	//
	// The version of CoreStorage that created the LVG.
	//
	uint32_t lvg_creator_version;

	//
	// The version of CoreStorage that modified the LVG the last time.
	//
	uint32_t lvg_writer_version;

	//
	// The interval to sync MLV metadata in milliseconds.
	//
	uint16_t lvg_sync_interval_ms;

	//
	// Checksum algorithm used for all metadata blocks and data blocks.
	//
	uint8_t lvg_metadata_cksum_alg;

	//
	// Do not punch hole in the LVs on trim, so that the sparse LVs can become
	// less and less sparse over time.
	//
	uint8_t lvg_no_punch_hole_on_trim;

	//
	// These fields control our forwards/backwards compatibility.
	// Features fall into three categories: compatible, read-only
	// compatible, and incompatible.  For any given version of the
	// file system code, you can mount a file-system with any set
	// of compatible features.      If the file system encounters bits
	// that are set in the read-only compatible features field and
	// it does not know about those features then it must only mount
	// the file system read-only.  If the file system encounters
	// any bits set in the incompatible features field that it does
	// not know about, then it must not mount or modify the file
	// system in any way.
	//
	// The idea for these compatibility fields is rather shamelessly
	// stolen from rocky and ext2.
	//
	//
	uint64_t lvg_compatible_features;
	uint64_t lvg_read_only_compatible_features;
	uint64_t lvg_incompatible_features;

	//
	// When Soft Snapshot is enabled, this is the minimum granularity the
	// extents will be updated with new timestamp.  This is OK to be 0, so a
	// predefined constant will be used.
	//
	uint64_t lvg_soft_snapshot_granularity_nbytes;

	//
	// When LV checksum is enabled, this is the minimum granularity of data
	// being checksummed.  This is OK to be 0, so a predefined constant will
	// be used.
	//
	uint64_t lvg_checksum_granularity_nbytes;

	//
	// The min/max versions of CoreStorage that ever wrote to the LVG.
	//
	uint32_t lvg_min_writer_version;
	uint32_t lvg_max_writer_version;

	//
	// The following unused fields are reserved so we don't need to break
	// format compatibility too often if we need more per-LVG fields.
	//
	uint64_t lvg_unused1;
	uint64_t lvg_unused2;
	uint64_t lvg_unused3;
};
typedef struct lvg_info lvg_info_t;

#endif /* _LF_CS_DISK_FORMAT_H */
