#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <err.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/disk.h>

#include "hfsmeta.h"

#define MIN(a, b) \
	({ __typeof(a) __a = (a); __typeof(b) __b = (b); \
		__a < __b ? __a : __b; })

/*
 * Get a block from a given input device.
 */
__private_extern__
ssize_t
GetBlock(DeviceInfo_t *devp, off_t offset, uint8_t *buffer)
{
	ssize_t retval = -1;
	off_t baseOffset = (offset / devp->blockSize) * devp->blockSize;

	retval = pread(devp->fd, buffer, devp->blockSize, baseOffset);
	if (retval != devp->blockSize) {
		warn("GetBlock: pread returned %zd", retval);
	}
	if (offset != baseOffset) {
		size_t off = offset % devp->blockSize;
		memmove(buffer, buffer + off, devp->blockSize - off);
	}
	retval = 0;
done:
	return retval;
}

/*
 * Initialize a VolumeObject.  Simple function.
 */
__private_extern__
VolumeObjects_t *
InitVolumeObject(struct DeviceInfo *devp, struct VolumeDescriptor *vdp)
{
	VolumeObjects_t *retval = NULL;

	retval = malloc(sizeof(*retval));
	if (retval) {
		retval->devp = devp;
		retval->vdp = vdp;
		retval->count = 0;
		retval->byteCount = 0;
		retval->list = NULL;
	}

done:
	return retval;
}

/*
 * Add an extent (<start, length> pair) to a volume list.
 * Note that this doesn't try to see if an extent is already
 * in the list; the presumption is that an fsck_hfs run will
 * note overlapping extents in that case.  It adds the extents
 * in groups of kExtentCount; the goal here is to minimize the
 * number of objects we allocate, while still trying to keep
 * the waste memory allocation low.
 */
__private_extern__
int
AddExtent(VolumeObjects_t *vdp, off_t start, off_t length)
{
	return AddExtentForFile(vdp, start, length, 0);
}

__private_extern__
int
AddExtentForFile(VolumeObjects_t *vdp, off_t start, off_t length, unsigned int fid)
{
	int retval = 0;
	size_t indx;
	ExtentList_t **ep = &vdp->list;

	if (debug) printf("AddExtent(%p, %lld, %lld) (file id %u)\n", vdp, start, length, fid);
	while (*ep) {
		if ((*ep)->count < kExtentCount) {
			indx = (*ep)->count;
			(*ep)->extents[indx].base = start;
			(*ep)->extents[indx].length = length;
			(*ep)->extents[indx].fid = fid;
			(*ep)->count++;
			break;
		} else {
			ep = &(*ep)->next;
		}
	}
	if (*ep == NULL) {
		*ep = malloc(sizeof(ExtentList_t));
		if (*ep == NULL) {
			err(1, "cannot allocate a new ExtentList object");
		}
		(*ep)->count = 1;
		(*ep)->extents[0].base = start;
		(*ep)->extents[0].length = length;
		(*ep)->extents[0].fid = fid;
		(*ep)->next = NULL;
	}
	vdp->count++;
	vdp->byteCount += length;

done:
	return retval;
}

// Debugging function
__private_extern__
void
PrintVolumeObject(VolumeObjects_t *vop)
{
	ExtentList_t *exts;

	printf("Volume Information\n");
	if (vop->devp) {
		printf("\tDevice %s\n", vop->devp->devname);
		printf("\t\tSize %lld\n", vop->devp->size);
		printf("\t\tBlock size %d\n", vop->devp->blockSize);
		printf("\t\tBlock Count %lld\n", vop->devp->blockCount);
	}
	printf("\tObject count %zu\n", vop->count);
	printf("\tByte count %lld\n", vop->byteCount);
	printf("\tExtent list:\n");
	for (exts = vop->list;
	     exts;
	     exts = exts->next) {
		int indx;
		for (indx = 0; indx < exts->count; indx++) {
			printf("\t\t<%lld, %lld> (file %u)\n", exts->extents[indx].base, exts->extents[indx].length, exts->extents[indx].fid);
		}
	}
	return;
}

/*
 * The main routine:  given a Volume descriptor, copy the metadata from it
 * to the given destination object (a device or sparse bundle).  It keeps
 * track of progress, and also takes an amount to skip (which happens if it's
 * resuming an earlier, interrupted copy).
 */
__private_extern__
int
CopyObjectsToDest(VolumeObjects_t *vop, struct IOWrapper *wrapper, off_t skip)
{
	ExtentList_t *exts;
	off_t total = 0;

	if (skip == 0) {
		wrapper->cleanup(wrapper);
	}
	for (exts = vop->list;
	     exts;
	     exts = exts->next) {
		int indx;
		for (indx = 0; indx < exts->count; indx++) {
			off_t start = exts->extents[indx].base;
			off_t len = exts->extents[indx].length;
			if (skip < len) {
				__block off_t totalWritten;
				void (^bp)(off_t);

				if (skip) {
					off_t amt = MIN(skip, len);
					len -= amt;
					start += amt;
					total += amt;
					skip -= amt;
					wrapper->setprog(wrapper, total);
					if (debug)
						printf("* * * Wrote %lld of %lld\n", total, vop->byteCount);
					else
						printf("%d%%\n", (int)((total * 100) / vop->byteCount));
					fflush(stdout);
				}
				totalWritten = total;
				if (printProgress) {
					bp = ^(off_t amt) {
						totalWritten += amt;
						wrapper->setprog(wrapper, totalWritten);
						if (debug)
							printf("* * Wrote %lld of %lld (%d%%)\n", totalWritten, vop->byteCount, (int)((totalWritten * 100) / vop->byteCount));
						else
							printf("%d%%\n", (int)((totalWritten * 100) / vop->byteCount));
						fflush(stdout);
						return;
					};
				} else {
					bp = ^(off_t amt) {
						totalWritten += amt;
						return;
					};
				}
				if (wrapper->writer(wrapper, vop->devp, start, len, bp) == -1) {
					int t = errno;
					if (verbose)
						warnx("Writing extent <%lld, %lld> failed", start, len);
					errno = t;
					return -1;
				}
				total = totalWritten;
			} else {
				skip -= len;
				total += len;
				if (printProgress) {
					wrapper->setprog(wrapper, total);
					if (debug)
						printf("Wrote %lld of %lld\n", total, vop->byteCount);
					else
						printf("%d%%\n", (int)((total * 100) / vop->byteCount));
					fflush(stdout);
				}
			}
		}
	}

	if (total == vop->byteCount) {
		wrapper->setprog(wrapper, 0);	// remove progress
	}

	return 0;
}
