#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "iterate_hfs_metadata.h"
#include "../CopyHFSMeta/Data.h"
#include "../CopyHFSMeta/hfsmeta.h"

/*
 * These variables are used by the CopyHFSMeta routines, so we have to define them
 */

__private_extern__ int debug = 0;
__private_extern__ int verbose = 0;
__private_extern__ int printProgress = 0;

/*
 * This is essentially the guts of CopyHFSMeta, only without
 * the ability to write out to a sparse bundle.  It also always
 * wants to get the "other" metadata (symlinks and large EA extents).
 *
 * For each extent it finds, it calls the passed-in function pointer,
 * with the start and length.
 *
 * It will go through and get the symlink and EA extents first, and
 * then go through the rest of the extents.  It does not attempt any
 * sorting past that.
 */
int
iterate_hfs_metadata(char *device, int (*handle_extent)(int fd, off_t start, off_t length, void *ctx), void *context_ptr)
{
	struct DeviceInfo *devp = NULL;
	struct VolumeDescriptor *vdp = NULL;
	VolumeObjects_t *vop = NULL;
	int retval = 0;

	devp = OpenDevice(device, 0);
	if (devp == NULL) {
		retval = errno;
		goto done;
	}

	vdp = VolumeInfo(devp);
	if (vdp == NULL) {
		retval = errno;
		goto done;
	}

	if (CompareVolumeHeaders(vdp) == -1) {
		retval = EINVAL;
		goto done;
	}

	vop = InitVolumeObject(devp, vdp);
	if (vop == NULL) {
		retval = errno;
		goto done;
	}

	/*
	 * These guys don't have an error.  Probably
	 * a mistake.
	 */
	AddHeaders(vop, 1);
	AddJournal(vop);
	AddFileExtents(vop);

	/*
	 * By this point, vop has all of the system metadata.
	 * The only metadata we don't have would be symlinks (from
	 * the catalog file), and extended attribute fork extents
	 * (from the attributes file).  We get those with
	 * FindOtherMetadata().
	 */
	retval = FindOtherMetadata(vop, ^(int fid __unused, off_t start, off_t len) {
			return (*handle_extent)(devp->fd, start, len, context_ptr);
		});

	if (retval != 0)
		goto done;

	/*
	 * Now, we've handled all of the other metadata, so we need
	 * to go through the rest of our metadata, which is in vop.
	 */
	ExtentList_t *extList;
	for (extList = vop->list;
	     extList;
	     extList = extList->next) {
		size_t index;

		for (index = 0; index < extList->count; index++) {
			retval = (*handle_extent)(devp->fd, extList->extents[index].base, extList->extents[index].length, context_ptr);
			if (retval != 0)
				goto done;
		}
	}
	
done:
	if (vop) {
		ReleaseVolumeObjects(vop);
	} else {
		if (vdp) {
			ReleaseVolumeDescriptor(vdp);
		}
		if (devp) {
			ReleaseDeviceInfo(devp);
		}
	}

	return retval;

}
