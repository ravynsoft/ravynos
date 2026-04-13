//
//  disk-image.h
//  hfs
//
//  Created by Chris Suter on 8/12/15.
//
//

#ifndef disk_image_h_
#define disk_image_h_

#include <stdbool.h>
#include <TargetConditionals.h>

/*
 * One 'shared' disk image is created for any test to use, if it wants.
 * To use this disk image, call disk_image_get(). To create a 'not-shared'
 * disk image for use just within your test, call disk_image_create().
 *
 * Callers of disk_image_create() and disk_image_get() should not free the pointer they receive,
 * as it is freed automatically.
 */

__BEGIN_DECLS

#define GB * (1024 * 1024 * 1024ULL)
#define TB * (1024 * 1024 * 1024 * 1024ULL)

#define SHARED_PATH "/tmp/shared.sparseimage"
#define SHARED_MOUNT "/tmp/mnt/shared"

typedef struct disk_image {
	const char *mount_point;
	const char *disk;
	const char *path;
} disk_image_t;

typedef struct disk_image_opts {
	const char *partition_type;
	bool enable_owners;
	const char *mount_point;
	unsigned long long size; // in bytes
} disk_image_opts_t;

disk_image_t *disk_image_create(const char *path, disk_image_opts_t *opts);
disk_image_t *disk_image_get(void);
bool disk_image_cleanup(disk_image_t *di);

__END_DECLS

#endif /* disk_image_h_ */
