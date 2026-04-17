/*
 * This test verifies the system calls that extract the data
 * protection class (A -> D) work properly in the face of a key bag
 * class roll.  Normally, the data protection class only occupies the
 * lower 5 bits of 32 bit space.  However, once we undergo a keyroll
 * one of the bits becomes hijacked to indicate whether or not the
 * file belongs to new (vs. old) key bag for the class keys.
 *
 * If our APIs are not working properly, this bit (or others) could be
 * exposed to userland — userland should only see the values 0 -> 32.
 */

#include <TargetConditionals.h>

#if (TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR)

#include <unistd.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/mount.h>
#include <sys/mman.h>
#include <sys/sysctl.h>
#include <System/sys/fsgetpath.h>
#include <MobileKeyBag/MobileKeyBag.h>
#import <Foundation/Foundation.h>
#import <Security/SecItemPriv.h>

#define CONFIG_PROTECT 1

#include "hfs-tests.h"
#include "test-utils.h"

#include "../../core/hfs_fsctl.h"

TEST(getattrlist_dprotect, .run_as_root = true)

typedef enum generation_status {
    generation_current              = 1 << 0,
    generation_change_in_progress   = 1 << 1,
    generation_change_pending       = 1 << 2,
} generation_status_t;

#define TEST_FILE	"/tmp/getattrlist_dprotect.data"

int run_getattrlist_dprotect(__unused test_ctx_t *ctx)
{
	// The root file system needs to be HFS
	struct statfs sfs;
	
	assert(statfs("/tmp", &sfs) == 0);
	if (strcmp(sfs.f_fstypename, "hfs")) {
		printf("getatttrlist_dprotect needs hfs as root file system - skipping.\n");
		return 0;
	}
	
	// Create a file
	unlink(TEST_FILE);
	int fd = open_dprotected_np(TEST_FILE,
								O_RDWR | O_CREAT, 3, 0, 0666);

	assert_with_errno(fd >= 0);

	struct attrlist attrlist = {
		.bitmapcount = ATTR_BIT_MAP_COUNT,
		.commonattr = ATTR_CMN_DATA_PROTECT_FLAGS,
	};

	struct attrs {
		uint32_t len;
		uint32_t dp_flags;
	} attrs;

	assert_no_err(fgetattrlist(fd, &attrlist, &attrs, sizeof(attrs), 0));

	// The generation should not be returned here
	assert(attrs.dp_flags == 3);

	// Check Foundation's API
	NSFileManager *fm = [NSFileManager defaultManager];

	assert([[fm attributesOfItemAtPath:@TEST_FILE error:NULL][NSFileProtectionKey] 
			isEqualToString:NSFileProtectionCompleteUntilFirstUserAuthentication]);

	// Change to class A
	assert([fm setAttributes:@{ NSFileProtectionKey: NSFileProtectionComplete }
				ofItemAtPath:@TEST_FILE
					   error:NULL]);

	assert([[fm attributesOfItemAtPath:@TEST_FILE error:NULL][NSFileProtectionKey] 
			isEqualToString:NSFileProtectionComplete]);

	uint32_t keybag_state;
	assert(!MKBKeyBagGetSystemGeneration(&keybag_state));

	// Class roll
	int ret = MKBKeyBagChangeSystemGeneration(NULL, 1);

	if (ret && ret != kMobileKeyBagNotReady)
		assert_fail("MKBKeyBagChangeSystemGeneration returned %d\n", ret);

	assert(!MKBKeyBagGetSystemGeneration(&keybag_state)
		   && (keybag_state & generation_change_in_progress));

	static const uint32_t max_ids = 1000000;
	static const uint32_t max_ids_per_iter = 262144;

	struct listxattrid_cp list_file_ids = {
		.flags = (LSXCP_PROT_CLASS_A | LSXCP_PROT_CLASS_B | LSXCP_PROT_CLASS_C),
	};

	uint32_t *file_ids = malloc(4 * max_ids);
	uint32_t count = 0;
	
	bzero(file_ids, 4 * max_ids);
	
	do {
		list_file_ids.count = max_ids_per_iter;
		list_file_ids.fileid = file_ids + count;
		
		if (fsctl("/private/var", HFSIOC_LISTXATTRID_CP, &list_file_ids, 0) < 0) {
			assert_with_errno(errno == EINTR);
			count = 0;
			bzero(list_file_ids.state, sizeof(list_file_ids.state));
			continue;
		}
		count += list_file_ids.count;

		assert(count < max_ids);
	} while (list_file_ids.count == max_ids_per_iter);

	assert_no_err(statfs("/private/var", &sfs));

	for (unsigned i = 0; i < count; ++i) {
		char path[PATH_MAX];

		if (fsgetpath(path, sizeof(path), &sfs.f_fsid,
					  (uint64_t)file_ids[i]) < 0) {
			assert_with_errno(errno == ENOENT);
			continue;
		}

		if (fsctl("/private/var", HFSIOC_OP_CPFORCEREWRAP, 
				  &file_ids[i], 0) != 0) {
			assert_with_errno(errno == ENOENT);
		}
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

	// Check class again
	assert_no_err(fgetattrlist(fd, &attrlist, &attrs, sizeof(attrs), 0));

	// The class should still be A
	assert(attrs.dp_flags == 1);

	// Check Foundation's API
	assert([[fm attributesOfItemAtPath:@TEST_FILE error:NULL][NSFileProtectionKey] 
			isEqualToString:NSFileProtectionComplete]);

	// Change to class C
	assert([fm setAttributes:@{ NSFileProtectionKey: NSFileProtectionCompleteUntilFirstUserAuthentication }
				ofItemAtPath:@TEST_FILE
					   error:NULL]);

	assert([[fm attributesOfItemAtPath:@TEST_FILE error:NULL][NSFileProtectionKey] 
			isEqualToString:NSFileProtectionCompleteUntilFirstUserAuthentication]);

	assert_no_err(close(fd));
	assert_no_err(unlink(TEST_FILE));

	// Create a new file
	fd = open_dprotected_np(TEST_FILE,
							O_RDWR | O_CREAT, 3, 0, 0666);

	assert_with_errno(fd >= 0);

	assert_no_err(fgetattrlist(fd, &attrlist, &attrs, sizeof(attrs), 0));

	// Check the class
	assert(attrs.dp_flags == 3);

	// Check Foundation's API
	assert([[fm attributesOfItemAtPath:@TEST_FILE error:NULL][NSFileProtectionKey] 
			isEqualToString:NSFileProtectionCompleteUntilFirstUserAuthentication]);

	// Change to class A
	assert([fm setAttributes:@{ NSFileProtectionKey: NSFileProtectionComplete }
				ofItemAtPath:@TEST_FILE
					   error:NULL]);

	assert([[fm attributesOfItemAtPath:@TEST_FILE error:NULL][NSFileProtectionKey] 
			isEqualToString:NSFileProtectionComplete]);

	assert_no_err(unlink(TEST_FILE));

	printf("[PASSED] getattrlist-dprotect\n");

	return 0;
}

#endif // (TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR)
