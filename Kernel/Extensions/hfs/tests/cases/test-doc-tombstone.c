//
//  doc-tombstone.c
//  hfs
//
//  Created by Chris Suter on 8/12/15.
//
//

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/attr.h>
#include <unistd.h>

#include "hfs-tests.h"
#include "test-utils.h"
#include "disk-image.h"

TEST(doc_tombstone)

int run_doc_tombstone(__unused test_ctx_t *ctx)
{
	
	disk_image_t *di = disk_image_get();
	
	char *file;
	char *new;
	char *old;
	
	asprintf(&file, "%s/doc-tombstone", di->mount_point);
	asprintf(&new, "%s.new", file);
	asprintf(&old, "%s.old", file);
	
	int fd;
	
	struct attrlist al = {
		.bitmapcount = ATTR_BIT_MAP_COUNT,
		.commonattr = ATTR_CMN_DOCUMENT_ID
	};
	
	struct attrs {
		uint32_t len;
		uint32_t doc_id;
	} attrs;
	
	uint32_t orig_doc_id;
	
	unlink(file);
	rmdir(file);
	
	
	/*
	 * 1.a. Move file over existing
	 */
	
	fd = open(file, O_RDWR | O_CREAT, 0666);
	
	assert_with_errno(fd >= 0);
	
	assert_no_err(fchflags(fd, UF_TRACKED));
	
	assert_no_err(fgetattrlist(fd, &al, &attrs, sizeof(attrs),
							   FSOPT_ATTR_CMN_EXTENDED));
	
	orig_doc_id = attrs.doc_id;
	
	assert_no_err(close(fd));
	
	assert_with_errno((fd = open(new,
								 O_RDWR | O_CREAT, 0666)) >= 0);
	
	assert_no_err(rename(new, file));
	
	assert_no_err(fgetattrlist(fd, &al, &attrs, sizeof(attrs),
							   FSOPT_ATTR_CMN_EXTENDED));
	
	assert_equal(orig_doc_id, attrs.doc_id, "%u");
	
	assert_no_err(close(fd));
	assert_no_err(unlink(file));
	
	
	/*
	 * 1.b. Move directory over existing
	 */
	
	assert_no_err(mkdir(file, 0777));
	
	assert_no_err(chflags(file, UF_TRACKED));
	
	assert_no_err(getattrlist(file, &al, &attrs, sizeof(attrs),
							  FSOPT_ATTR_CMN_EXTENDED));
	
	orig_doc_id = attrs.doc_id;
	
	assert_no_err(mkdir(new, 0777));
	
	assert_no_err(rename(new, file));
	
	assert_no_err(getattrlist(file, &al, &attrs, sizeof(attrs),
							  FSOPT_ATTR_CMN_EXTENDED));
	
	assert_equal(orig_doc_id, attrs.doc_id, "%u");
	
	assert_no_err(rmdir(file));
	
	
	/*
	 * 2.a. Move original file out of the way, move new file into place, delete original
	 */
	
	fd = open(file, O_RDWR | O_CREAT, 0666);
	
	assert_with_errno(fd >= 0);
	
	assert_no_err(fchflags(fd, UF_TRACKED));
	
	assert_no_err(fgetattrlist(fd, &al, &attrs, sizeof(attrs),
							   FSOPT_ATTR_CMN_EXTENDED));
	
	orig_doc_id = attrs.doc_id;
	
	assert_no_err(close(fd));
	
	assert_with_errno((fd = open(new,
								 O_RDWR | O_CREAT, 0666)) >= 0);
	
	assert_with_errno(fd >= 0);
	
	assert_no_err(rename(file, old));
	assert_no_err(rename(new, file));
	
	assert_no_err(fgetattrlist(fd, &al, &attrs, sizeof(attrs),
							   FSOPT_ATTR_CMN_EXTENDED));
	
	assert_equal(orig_doc_id, attrs.doc_id, "%u");
	
	assert_no_err(close(fd));
	assert_no_err(unlink(old));
	assert_no_err(unlink(file));
	
	
	/*
	 * 2.b. Move original directory out of the way, move new directory into place, delete original
	 */
	
	assert_no_err(mkdir(file, 0777));
	
	assert_no_err(chflags(file, UF_TRACKED));
	
	assert_no_err(getattrlist(file, &al, &attrs, sizeof(attrs),
							  FSOPT_ATTR_CMN_EXTENDED));
	
	orig_doc_id = attrs.doc_id;
	
	assert_no_err(mkdir(new, 0777));
	
	assert_no_err(rename(file, old));
	assert_no_err(rename(new, file));
	
	assert_no_err(getattrlist(file, &al, &attrs, sizeof(attrs),
							  FSOPT_ATTR_CMN_EXTENDED));
	
	assert_equal(orig_doc_id, attrs.doc_id, "%u");
	
	assert_no_err(rmdir(old));
	assert_no_err(rmdir(file));
	
	
	/*
	 * 3.a. Delete original file, move new file into place
	 */
	
	assert_with_errno((fd = open(file,
								 O_RDWR | O_CREAT, 0666)) >= 0);
	
	assert_with_errno(fd >= 0);
	
	assert_no_err(fchflags(fd, UF_TRACKED));
	
	assert_no_err(fgetattrlist(fd, &al, &attrs, sizeof(attrs),
							   FSOPT_ATTR_CMN_EXTENDED));
	
	orig_doc_id = attrs.doc_id;
	
	assert_no_err(close(fd));
	
	assert_with_errno((fd = open(new,
								 O_RDWR | O_CREAT, 0666)) >= 0);
	
	assert_with_errno(fd >= 0);
	
	assert_no_err(unlink(file));
	
	assert_no_err(rename(new, file));
	
	assert_no_err(fgetattrlist(fd, &al, &attrs, sizeof(attrs),
							   FSOPT_ATTR_CMN_EXTENDED));
	
	assert_equal(orig_doc_id, attrs.doc_id, "%u");
	
	assert_no_err(close(fd));
	assert_no_err(unlink(file));
	
	
	/*
	 * 3.b. Delete original directory, move new directory into place
	 */
	
	assert_no_err(mkdir(file, 0777));
	
	assert_no_err(chflags(file, UF_TRACKED));
	
	assert_no_err(getattrlist(file, &al, &attrs, sizeof(attrs),
							  FSOPT_ATTR_CMN_EXTENDED));
	
	orig_doc_id = attrs.doc_id;
	
	assert_no_err(mkdir(new, 0777));
	
	assert_no_err(rmdir(file));
	assert_no_err(rename(new, file));
	
	assert_no_err(getattrlist(file, &al, &attrs, sizeof(attrs),
							  FSOPT_ATTR_CMN_EXTENDED));
	
	assert_equal(orig_doc_id, attrs.doc_id, "%u");
	
	assert_no_err(rmdir(file));
	
	
	/*
	 * 4.a. Delete original file, create new file in place
	 */
	
	assert_with_errno((fd = open(file,
								 O_RDWR | O_CREAT, 0666)) >= 0);
	
	assert_no_err(fchflags(fd, UF_TRACKED));
	
	assert_no_err(fgetattrlist(fd, &al, &attrs, sizeof(attrs),
							   FSOPT_ATTR_CMN_EXTENDED));
	
	orig_doc_id = attrs.doc_id;
	
	assert_no_err(close(fd));
	assert_no_err(unlink(file));
	
	assert_with_errno((fd = open(file,
								 O_RDWR | O_CREAT, 0666)) >= 0);
	
	assert_no_err(fgetattrlist(fd, &al, &attrs, sizeof(attrs),
							   FSOPT_ATTR_CMN_EXTENDED));
	
	assert_equal(orig_doc_id, attrs.doc_id, "%u");
	
	assert_no_err(close(fd));
	assert_no_err(unlink(file));
	
	
	/*
	 * 4.b. Delete original directory, create new directory in place
	 */
	
	assert_no_err(mkdir(file, 0777));
	
	assert_no_err(chflags(file, UF_TRACKED));
	
	assert_no_err(getattrlist(file, &al, &attrs, sizeof(attrs),
							  FSOPT_ATTR_CMN_EXTENDED));
	
	orig_doc_id = attrs.doc_id;
	
	assert_no_err(rmdir(file));
	
	assert_no_err(mkdir(file, 0777));
	
	assert_no_err(getattrlist(file, &al, &attrs, sizeof(attrs),
							  FSOPT_ATTR_CMN_EXTENDED));
	
	assert_equal(orig_doc_id, attrs.doc_id, "%u");
	
	assert_no_err(rmdir(file));

	return 0;
}
