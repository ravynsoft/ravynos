#include <TargetConditionals.h>

#if (TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR)

#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/sysctl.h>
#include <System/sys/fsgetpath.h>
#include <MobileKeyBag/MobileKeyBag.h>
#import <Security/SecItemPriv.h>
#include <hfs/hfs_fsctl.h>
#include <sys/mount.h>
#include <sys/param.h>

#include "hfs-tests.h"
#include "test-utils.h"

TEST(class_roll, .run_as_root = true)

#define CLASS_ROLL_TEST_FILE	"/tmp/class-roll.data"

typedef enum generation_status {
    generation_current              = 1 << 0,
    generation_change_in_progress   = 1 << 1,
    generation_change_pending       = 1 << 2,
} generation_status_t;

int run_class_roll(__unused test_ctx_t *ctx)
{
	// The root file system needs to be HFS
	struct statfs sfs;
	
	assert(statfs("/tmp", &sfs) == 0);
	if (strcmp(sfs.f_fstypename, "hfs")) {
		printf("class_roll needs hfs as root file system - skipping.\n");
		return 0;
	}
	
	// Let's create a file to work with
	unlink(CLASS_ROLL_TEST_FILE);
	int fd;

	assert_with_errno((fd = open(CLASS_ROLL_TEST_FILE, 
								 O_CREAT | O_RDWR, 0666)) >= 0);

	size_t size = 1000000 * 4;

	assert_no_err(ftruncate(fd, size));

	int *p;
	assert_with_errno((p = mmap(NULL, size, PROT_READ | PROT_WRITE,
								MAP_SHARED, fd, 0)) != MAP_FAILED);

	for (int i = 0; i < 1000000; ++i)
		p[i] = i;

	assert_no_err(msync(p, size, MS_SYNC));

	// Switch to class C
	assert_no_err(fcntl(fd, F_SETPROTECTIONCLASS, 3));

	// Figure out the ID
	struct stat sb;
	assert_no_err(fstat(fd, &sb));

	// Start class rolling
	int ret = MKBKeyBagChangeSystemGeneration(NULL, 1);

	if (ret && ret != kMobileKeyBagNotReady)
		assert_fail("MKBKeyBagChangeSystemGeneration returned %d\n", ret);

	uint32_t keybag_state;
	assert(!MKBKeyBagGetSystemGeneration(&keybag_state)
		   && (keybag_state & generation_change_in_progress));

	static const uint32_t max_ids = 1000000;
	static const uint32_t max_ids_per_iter = 262144;

	struct cp_listids list_file_ids = {
		.api_version = CPLIDS_API_VERSION_1,
		.flags = (CPLID_USING_OLD_CLASS_KEY | CPLID_PROT_CLASS_A | CPLID_PROT_CLASS_B | CPLID_PROT_CLASS_C),
	};
	
	uint32_t *file_ids = malloc(4 * max_ids);
	uint32_t count = 0;
	
	bzero(file_ids, 4 * max_ids);
	
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

	assert_no_err(statfs("/private/var", &sfs));

	bool did_file = false;

	hfs_key_auto_roll_args_t auto_roll_args = {
		.api_version = HFS_KEY_AUTO_ROLL_API_LATEST_VERSION,
		.flags = HFS_KEY_AUTO_ROLL_OLD_CLASS_GENERATION,
	};

	assert_no_err(fsctl("/private/var", HFSIOC_SET_KEY_AUTO_ROLL,
						&auto_roll_args, 0));

	auto_roll_args.min_key_os_version = auto_roll_args.max_key_os_version = 0xffff;
	auto_roll_args.flags = 0xffffffff;

	assert_no_err(fsctl("/private/var", HFSIOC_GET_KEY_AUTO_ROLL, 
						&auto_roll_args, 0));

	assert(auto_roll_args.flags == HFS_KEY_AUTO_ROLL_OLD_CLASS_GENERATION);

	for (unsigned i = 0; i < count; ++i) {
		char path[PATH_MAX];

		if (fsgetpath(path, sizeof(path), &sfs.f_fsid,
					  (uint64_t)file_ids[i]) < 0) {
			assert_with_errno(errno == ENOENT);
			continue;
		}

		if (file_ids[i] == sb.st_ino) {
			hfs_key_roll_args_t args = {
				.api_version = HFS_KR_API_LATEST_VERSION,
				.operation = HFS_KR_OP_STATUS,
			};

			assert_no_err(ffsctl(fd, HFSIOC_KEY_ROLL, &args, 0));

			assert(args.done == -1 
				   && (args.key_class_generation
					   == ((keybag_state & generation_current) ^ 1)));

			int raw_fd;
			assert_with_errno((raw_fd = open_dprotected_np(CLASS_ROLL_TEST_FILE,
														   O_RDONLY, 0, 1, 0)) >= 0);

			uint16_t old_key_rev = args.key_revision;

			assert_no_err(ffsctl(fd, HFSIOC_KEY_ROLL, &args, 0));

			assert(args.done == 0
				   && (args.key_class_generation
					   == ((keybag_state & generation_current))));

			assert((args.key_revision & 0xff00)
				   == ((old_key_rev + 0x0100) & 0xff00));

			// Force flush of cache
			assert_no_err(msync(p, sb.st_size, MS_INVALIDATE));

			// Check the file
			for (int i = 0; i < 1000000; ++i) {
				assert(p[i] == i);
			}

			// Roll 1 chunk
			args.operation = HFS_KR_OP_STEP;

			assert_no_err(ffsctl(fd, HFSIOC_KEY_ROLL, &args, 0));

			assert(args.done == 2 * 1024 * 1024);

			// Force flush of cache
			assert_no_err(msync(p, sb.st_size, MS_INVALIDATE));

			// Check the file again
			for (int i = 0; i < 1000000; ++i) {
				assert(p[i] == i);
			}

			// We're done with the mmap
			assert_no_err(munmap(p, sb.st_size));

			assert_no_err(close(raw_fd));

			did_file = true;
		}

		assert_no_err(fsctl("/private/var", HFSIOC_OP_CPFORCEREWRAP, 
							&file_ids[i], 0));
	}

	// Mark as done
	uint32_t flags = HFS_SET_CPFLAG;

	fsctl("/private/var", HFSIOC_OP_CRYPTOGEN, &flags, 0);
	
	int attempts = 0;
	while (!_SecKeychainRollKeys(true, NULL) && ++attempts < 1000)
			;
	assert(attempts < 1000);

	// Tell MobileKeyBag that we're done
	assert(!MKBKeyBagChangeSystemGeneration(NULL, 2));
	assert(!MKBKeyBagGetSystemGeneration(&keybag_state));

	assert(did_file);

	auto_roll_args.min_key_os_version = auto_roll_args.max_key_os_version = 0xffff;
	auto_roll_args.flags = 0xffffffff;

	assert_no_err(fsctl("/private/var", HFSIOC_GET_KEY_AUTO_ROLL, 
						&auto_roll_args, 0));

	assert(!auto_roll_args.flags);
	assert(auto_roll_args.min_key_os_version == 0);
	assert(auto_roll_args.max_key_os_version == 0);

	free(file_ids);
	
	return 0;
}

#endif // TARGET_OS_IPHONE & !SIM
