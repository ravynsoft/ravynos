#include <TargetConditionals.h>

#if (TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR)

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#include "hfs-tests.h"
#include "test-utils.h"
#include "disk-image.h"

TEST(secluded_rename, .run_as_root = true)

#define SECLUDE_RENAME 0x00000001
extern int rename_ext(const char *from, const char *to, unsigned int flags);

static char *file1, *file2, *file3, *dir1;

int run_secluded_rename(__unused test_ctx_t *ctx)
{
	disk_image_t *di = disk_image_get();
	asprintf(&file1, "%s/secluded-rename.1", di->mount_point);
	asprintf(&file2, "%s/secluded-rename.2", di->mount_point);
	asprintf(&file3, "%s/secluded-rename.3", di->mount_point);
	asprintf(&dir1, "%s/secluded-rename.dir", di->mount_point);
	
	unlink(file1);
	unlink(file2);
	unlink(file3);
	unlink(dir1);

	int fd = open(file1, O_RDWR | O_CREAT, 0666);

	assert_with_errno((fd >= 0));

	errno = 0;
	assert_with_errno(rename_ext(file1, file2, SECLUDE_RENAME) == -1 && errno == EBUSY);

	assert_no_err(close(fd));

	fd = open(file1, O_EVTONLY);
	assert_with_errno(fd >= 0);

	assert(rename_ext(file1, file2, SECLUDE_RENAME) == -1 && errno == EBUSY);

	assert_no_err(close(fd));

	assert_no_err(rename_ext(file1, file2, SECLUDE_RENAME));

	assert_no_err(link(file2, file3));

	assert(rename_ext(file2, file1, SECLUDE_RENAME) == -1 && errno == EMLINK);

	assert_no_err(unlink(file3));

	assert_no_err(rename_ext(file2, file1, SECLUDE_RENAME));

	assert_no_err(mkdir(dir1, 0777));

	assert(rename_ext(dir1, file3, SECLUDE_RENAME) == -1 && errno == EISDIR);

	return 0;
}

#endif // (TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR)
