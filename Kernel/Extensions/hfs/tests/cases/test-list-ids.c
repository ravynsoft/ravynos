#include <TargetConditionals.h>

#if (TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR)

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <string.h>
#include <sys/attr.h>
#include <sys/stat.h>
#include <sys/sysctl.h>
#include <System/sys/fsgetpath.h>
#include <hfs/hfs_fsctl.h>
#include <sys/mount.h>
#include <sys/param.h>

#include "hfs-tests.h"
#include "test-utils.h"

TEST(list_ids, .run_as_root = true)

#define MAX_FILES	100
#define KB			*1024
#define F_RECYCLE	84

static struct attrs {
	uint32_t len;
	uint64_t file_id;
	uint32_t dp_flags;
} __attribute__((aligned(4), packed)) attrs[MAX_FILES];

static char		*path[MAX_FILES];
static int		fd[MAX_FILES];

static uint32_t max_ids = 1000000;
static uint32_t max_ids_per_iter = 262144;

int run_list_ids(__unused test_ctx_t *ctx)
{
	// The data partition needs to be HFS
	struct statfs sfs;
	
	assert(statfs("/private/var", &sfs) == 0);
	if (strcmp(sfs.f_fstypename, "hfs")) {
		printf("list_ids needs hfs on the data partition.\n");
		return 0;
	}
	
	unsigned i;
	uint32_t *file_ids = malloc(4 * max_ids);
	uint32_t count = 0;
	
	bzero(file_ids, 4 * max_ids);
	
	struct listxattrid_cp listxattrid_file_ids = {
		.flags = (LSXCP_PROT_CLASS_A | LSXCP_PROT_CLASS_B
				  | LSXCP_PROT_CLASS_C | LSXCP_UNROLLED_KEYS),
	};
	
	do {
		listxattrid_file_ids.count = max_ids_per_iter;
		listxattrid_file_ids.fileid = file_ids + count;
		
		assert_no_err(fsctl("/private/var", HFSIOC_LISTXATTRID_CP, &listxattrid_file_ids, 0));
		
		count += listxattrid_file_ids.count;
		assert(count < max_ids);
	} while (count == max_ids_per_iter);

	void *buf = malloc(1 KB);
	memset(buf, 0x25, 1 KB);
	
	for (unsigned i = 0; i < MAX_FILES; i ++) {
		
		asprintf(&path[i], "/private/var/get-matched_listid.data.%u", getpid()+i);
		unlink(path[i]);
		
		assert_with_errno((fd[i] = open(path[i], O_RDWR | O_TRUNC | O_CREAT, 0666)) >= 0);
		
		// Write 1 kB
		check_io(write(fd[i], buf, 1 KB), 1 KB);
		
		// Change file to class B
		assert_no_err(fcntl(fd[i], F_SETPROTECTIONCLASS, 2));
		
		struct attrlist attrlist = {
			.bitmapcount = ATTR_BIT_MAP_COUNT,
			.commonattr = ATTR_CMN_FILEID | ATTR_CMN_DATA_PROTECT_FLAGS,
		};
		
		assert_no_err(fgetattrlist(fd[i], &attrlist, &attrs[i], sizeof(attrs), 0));
		assert((attrs[i].dp_flags & 0x1f) == 2);
		
	}
	
	bool release_build = false;

	if (fcntl(fd[0], F_RECYCLE)) {
		assert_equal_int(errno, ENOTTY);
		release_build = true;
	}

	// HFS_KR_OP_SET_INFO doesn't run in releae build.
	if (release_build)
		goto exit;

	for (unsigned i = 0; i < MAX_FILES; i ++) {
		
		hfs_key_roll_args_t args;
		
		// Change the revision and os version
		args.api_version = HFS_KR_API_LATEST_VERSION;
		args.operation = HFS_KR_OP_SET_INFO;
		args.key_revision = 0x0200;
		args.key_os_version = CP_OS_VERS_PRE_71;
		assert_no_err(ffsctl(fd[i], HFSIOC_KEY_ROLL, &args, 0));
		
		args.operation = HFS_KR_OP_STATUS;
		assert_no_err(ffsctl(fd[i], HFSIOC_KEY_ROLL, &args, 0));
		
		assert(args.done == -1
			   && args.key_revision == 0x0200
			   && args.key_os_version == CP_OS_VERS_PRE_71);
		
	}
	
	max_ids_per_iter = 48;
	count = 0;
	bzero(file_ids, 4 * max_ids);
	
	struct cp_listids list_file_ids = {
		.api_version = CPLIDS_API_VERSION_1,
		.flags = CPLID_MAX_OS_VERSION | CPLID_PROT_CLASS_B,
		.max_key_os_version = CP_OS_VERS_71,
	};
	
	do {
		list_file_ids.count = max_ids_per_iter;
		list_file_ids.file_ids = file_ids + count;
		
		if (fsctl("/private/var", HFSIOC_CP_LIST_IDS, &list_file_ids, 0) < 0) {
			assert_with_errno(errno == EINTR);
			count = 0;
			bzero(list_file_ids.state, sizeof(list_file_ids.state));
			continue;
		}
		count += list_file_ids.count;
		
		assert(count < max_ids);
		
	} while (list_file_ids.count == max_ids_per_iter);
	
	assert(count == MAX_FILES);
	
	statfs("/private/var", &sfs);
	
	for (i = 0; i < count; i++) {
		char path_name[PATH_MAX];
		
		if (fsgetpath(path_name, sizeof(path), &sfs.f_fsid,
					  (uint64_t)file_ids[i]) < 0) {
			assert_with_errno(errno == ENOENT);
			continue;
		}
		assert(file_ids[i] == attrs[i].file_id);
	}

exit:

	free(buf);
	free(file_ids);
	
	for (i = 0; i < MAX_FILES; i++) {
		assert_no_err(close(fd[i]));
		if (path[i]) {
			unlink(path[i]);
			free(path[i]);
		}
	}
	
	return 0;
}

#endif // (TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR)
