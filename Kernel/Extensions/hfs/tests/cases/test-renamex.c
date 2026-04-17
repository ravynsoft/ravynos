#include <TargetConditionals.h>

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>

#include "hfs-tests.h"
#include "test-utils.h"
#include "disk-image.h"

TEST(renamex_test)

static disk_image_t *di;
extern int errno;

int run_renamex_test (__unused test_ctx_t *ctx) {

	di = disk_image_get();
	char* dst_file;
	int dst_file_fd;
	char* src_file;
	int src_file_fd;
	asprintf (&dst_file, "%s/renamex_dst", di->mount_point);
	asprintf (&src_file, "%s/renamex_src", di->mount_point);

	/* create the files */

	src_file_fd = open (src_file, O_RDWR | O_CREAT | O_TRUNC, 0666);
	assert (src_file_fd >= 0);
	
	dst_file_fd = open (dst_file, O_RDWR | O_CREAT | O_TRUNC, 0666);
	assert (src_file_fd >= 0);

	/* attempt the renamex calls...*/

	//first verify non-supported flags error out 
	int error = renamex_np (src_file, dst_file, (RENAME_SWAP));
	assert (error != 0);

	//now try both flags
	error = renamex_np (src_file, dst_file, (RENAME_SWAP | RENAME_EXCL));
	assert (error != 0);

	//now verify it errors out because destination exists. 
	error = renamex_np (src_file, dst_file, (RENAME_EXCL));
	assert ((error != 0) && (errno == EEXIST));

	/* now delete dst and try again */
	error = unlink (dst_file);
	assert (error == 0);

	error = renamex_np (src_file, dst_file, (RENAME_EXCL));
	assert (error == 0);

	error = unlink(dst_file);
	assert (error == 0);

	assert_no_err(close(src_file_fd));
	assert_no_err(close(dst_file_fd));

	return 0;
}	


