//
//  test-transcode.m
//  hfs
//
//  Created by Chris Suter on 8/21/15.
//
//

#import <TargetConditionals.h>

#if (TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR)

#import <fcntl.h>
#import <MobileKeyBag/MobileKeyBag.h>
#import <Foundation/Foundation.h>
#import <sys/param.h>
#import <sys/mount.h>

#import "hfs-tests.h"
#import "test-utils.h"

TEST(transcode)

#define TEST_FILE "/tmp/transcode-test.file"

int run_transcode(__unused test_ctx_t *ctx)
{
	// The root file system needs to be HFS
	struct statfs sfs;
	
	assert(statfs("/tmp", &sfs) == 0);
	if (strcmp(sfs.f_fstypename, "hfs")) {
		printf("transcode needs hfs as root file system.\n");
		return 0;
	}
	
	MKBKeyBagHandleRef handle;
	CFDataRef data;

	assert_no_err(MKBKeyBagCreateOTABackup(NULL, &handle));
	assert_no_err(MKBKeyBagCopyData(handle, &data));
	assert_no_err(MKBKeyBagRegisterOTABackup(data, NULL));

	unlink(TEST_FILE);
	int fd = open_dprotected_np(TEST_FILE, O_RDWR | O_CREAT,
								1, 0, 0666);

	assert_with_errno(fd >= 0);

	char *key = malloc(1024);
	int res = fcntl(fd, F_TRANSCODEKEY, key);

	assert_with_errno(res != -1);

	// Keys should be at least 16 bytes
	assert(res >= 16);

	assert_no_err(unlink(TEST_FILE));

	assert_no_err(MKBKeyBagRegisterOTABackup(NULL, NULL));
	assert_no_err(MKBKeyBagRelease(handle));

	close(fd);
	free(key);

	return 0;
}

#endif
