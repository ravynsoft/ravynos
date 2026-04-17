//
//  test-getattrlist.c
//  hfs
//
//  Created by Chris Suter on 8/21/15.
//
//

#import <TargetConditionals.h>
#include <fcntl.h>
#include <sys/attr.h>
#include <unistd.h>
#include <sys/mount.h>
#include <sys/kauth.h>
#include <sys/param.h>

#include "hfs-tests.h"
#include "test-utils.h"
#include "disk-image.h"



TEST(getattrlist)

#define TEST_FILE_NAME "getattrlist-test.file"

int run_getattrlist(__unused test_ctx_t *ctx)
{
	disk_image_t *di = NULL;
	const char *tstdir;
	bool hfs_root = true; //true until proven otherwise.

	struct statfs sfs;
	assert(statfs("/tmp", &sfs) == 0);
	if (strcmp(sfs.f_fstypename, "hfs")) {
		hfs_root = false;
		di = disk_image_get();
		tstdir = di->mount_point;
	} else {
		tstdir = "/tmp";
	}
	
	char *file;
	asprintf(&file, "%s/getattrlist-test.file", tstdir);
	
	unlink(file);
	int fd = open_dprotected_np(file, O_RDWR | O_CREAT, 3, 0, 0666);

	assert_with_errno(fd >= 0);

	struct attrlist al = {
		.bitmapcount = ATTR_BIT_MAP_COUNT,
		.commonattr = ATTR_CMN_NAME | ATTR_CMN_DEVID | ATTR_CMN_FSID
		| ATTR_CMN_OBJTYPE | ATTR_CMN_OBJTAG | ATTR_CMN_OBJID
		| ATTR_CMN_OBJPERMANENTID | ATTR_CMN_PAROBJID | ATTR_CMN_SCRIPT
		| ATTR_CMN_CRTIME | ATTR_CMN_MODTIME | ATTR_CMN_CHGTIME
		| ATTR_CMN_ACCTIME | ATTR_CMN_BKUPTIME | ATTR_CMN_FNDRINFO
		| ATTR_CMN_OWNERID | ATTR_CMN_GRPID | ATTR_CMN_ACCESSMASK
		| ATTR_CMN_FLAGS | ATTR_CMN_GEN_COUNT | ATTR_CMN_DOCUMENT_ID
		| ATTR_CMN_USERACCESS | ATTR_CMN_EXTENDED_SECURITY
		| ATTR_CMN_UUID | ATTR_CMN_GRPUUID | ATTR_CMN_FILEID
		| ATTR_CMN_PARENTID | ATTR_CMN_FULLPATH | ATTR_CMN_ADDEDTIME
		| ATTR_CMN_DATA_PROTECT_FLAGS | ATTR_CMN_RETURNED_ATTRS,
	};

#pragma pack(push, 4)
	struct {
		uint32_t				len;
		attribute_set_t			returned_attrs;
		struct attrreference	name;
		dev_t					devid;
		fsid_t					fsid;
		fsobj_type_t			obj_type;
		fsobj_tag_t				obj_tag;
		fsobj_id_t				obj_id;
		fsobj_id_t				obj_perm_id;
		fsobj_id_t				par_obj_id;
		text_encoding_t			encoding;
		struct timespec			cr_time, mod_time, chg_time, acc_time, backup_time;
		uint8_t					finder_info[32];
		uid_t					owner;
		gid_t					group;
		uint32_t				access_mask;
		uint32_t				flags;
		uint32_t				gen_count;
		uint32_t				doc_id;
		uint32_t				access;
		struct attrreference	ext_security;
		guid_t					guid;
		guid_t					grp_uuid;
		uint64_t				file_id;
		uint64_t				parent_id;
		struct attrreference	full_path;
		struct timespec			added_time;
		uint32_t				protection_class;
		uint8_t					extra[1024];
	} attrs;
#pragma pack(pop)

	assert_no_err(fgetattrlist(fd, &al, &attrs, sizeof(attrs),
							   FSOPT_ATTR_CMN_EXTENDED | FSOPT_PACK_INVAL_ATTRS));

	uint32_t expected = al.commonattr ^ ATTR_CMN_EXTENDED_SECURITY;

#if (TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR)
	if (hfs_root == true) {
		assert(attrs.protection_class == 3);
	}
#else
	expected ^= ATTR_CMN_DATA_PROTECT_FLAGS;
	assert(attrs.protection_class == 0);
#endif

	assert_equal_int(attrs.returned_attrs.commonattr, expected);

	// Just check a few things, not everything...

	assert(!strcmp((char *)&attrs.name + attrs.name.attr_dataoffset,
					TEST_FILE_NAME));
	assert(strlen(TEST_FILE_NAME) + 1 == attrs.name.attr_length);

	char path[PATH_MAX];
	assert(!strcmp((char *)&attrs.full_path + attrs.full_path.attr_dataoffset,
				   realpath(file, path)));
	assert(strlen(path) + 1 == attrs.full_path.attr_length);

	time_t t = time(NULL);

	assert(attrs.cr_time.tv_sec > t - 5);
	assert(attrs.mod_time.tv_sec > t - 5);
	assert(attrs.chg_time.tv_sec > t - 5);
	assert(attrs.acc_time.tv_sec > t - 5);
	assert(attrs.backup_time.tv_sec == 0);
	assert(attrs.added_time.tv_sec > t - 5);

	if (hfs_root) {
		struct statfs sfs2;
		assert(statfs("/bin", &sfs2) == 0);
		assert(strcmp(sfs2.f_fstypename, "hfs") == 0);
		
		// Check a file on the system partition (this only valid if root == hfs) 
		assert_no_err(getattrlist("/bin/sh", &al, &attrs, sizeof(attrs),
					FSOPT_ATTR_CMN_EXTENDED | FSOPT_PACK_INVAL_ATTRS));

		assert(attrs.returned_attrs.commonattr == expected);
		assert(attrs.protection_class == 0);

		assert(!strcmp((char *)&attrs.name + attrs.name.attr_dataoffset, "sh"));
		assert(attrs.name.attr_length == 3);

		assert(!strcmp((char *)&attrs.full_path + attrs.full_path.attr_dataoffset,
					"/bin/sh"));
		assert(attrs.full_path.attr_length == 8);
	}

	assert_no_err(close(fd));
	assert_no_err(unlink(file));
	free(file);

	return 0;
}
