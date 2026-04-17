#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <err.h>
#include <errno.h>
#include <zlib.h>
#include <limits.h>
#include <assert.h>

#include "hfsmeta.h"
#include "Data.h"

struct HFSDataObject {
	int64_t	offset;
	int64_t	size;
};
typedef struct HFSDataObject HFSDataObject;

enum {
	kHFSInfoHeaderVersion = 1,
};


#define MIN(a, b) \
	({ __typeof(a) __a = (a); __typeof(b) __b = (b); \
		__a < __b ? __a : __b; })

struct HFSInfoHeader {
	uint32_t	version;
	uint32_t	deviceBlockSize;
	int64_t	rawDeviceSize;
	int32_t	size;	// Size of header
	int32_t	objectCount;
};

static ssize_t
WriteExtent(gzFile outf, DeviceInfo_t *devp, off_t start, off_t len)
{
	const size_t bufSize = 1024 * 1024;
	uint8_t buffer[bufSize];
	off_t total = 0;

	while (total < len) {
		ssize_t nread;
		size_t amt = MIN(bufSize, len - total);
		ssize_t nwritten;
		nread = pread(devp->fd, buffer, amt, start + total);
		if (nread == -1) {
			warn("Cannot read from device at offset %lld", start + total);
			return -1;
		}
		if (nread != amt) {
			warnx("Tried to read %zu bytes, only read %zd", amt, nread);
		}
		nwritten = gzwrite(outf, (char*)buffer, (unsigned)amt);
		if (nwritten == -1) {
			warn("tried to gzwrite %zu bytes", amt);
			return -1;
		} else if (nwritten != amt) {
			warnx("tried to gzwrite %zu bytes, only wrote %u", amt, nwritten);
			total += nwritten;
		} else
			total += amt;
	}
	return 0;
}

void
WriteGatheredData(const char *pathname, VolumeObjects_t *vop)
{
	int fd;
	gzFile outf;
	struct HFSInfoHeader hdr = { 0 };
	HFSDataObject *objs = NULL, *op;
	ExtentList_t *ep;
	int i;
	size_t len;

	hdr.version = S32(kHFSInfoHeaderVersion);
	hdr.deviceBlockSize = S32((uint32_t)vop->devp->blockSize);
	hdr.rawDeviceSize = S64(vop->devp->size);
	hdr.objectCount = S32(vop->count);
	hdr.size = S32(sizeof(hdr) + sizeof(HFSDataObject) * vop->count);

	objs = malloc(sizeof(HFSDataObject) * vop->count);
	if (objs == NULL) {
		warn("Unable to allocate space for data objects (%zu bytes)", sizeof(HFSDataObject)* vop->count);
		goto done;
	}

	op = objs;
	for (ep = vop->list;
	     ep;
	     ep = ep->next) {
		int i;
		for (i = 0; i < ep->count; i++) {
			op->offset = S64(ep->extents[i].base);
			op->size = S64(ep->extents[i].length);
			op++;
		}
	}

	fd = open(pathname, O_WRONLY | O_CREAT | O_TRUNC, 0666);
	if (fd == -1) {
		warn("cannot create gather file %s", pathname);
		goto done;
	}
	outf = gzdopen(fd, "wb");
	if (outf == NULL) {
		warn("Cannot create gz descriptor from file %s", pathname);
		close(fd);
		goto done;
	}

	gzwrite(outf, &hdr, sizeof(hdr));
	len = sizeof(HFSDataObject) * vop->count;
	assert(len < UINT_MAX);
	gzwrite(outf, objs, (unsigned)len);

	int count = 0;
	for (ep = vop->list;
	     ep;
	     ep = ep->next) {
		int i;
		for (i = 0; i < ep->count; i++) {
			if (verbose)
				fprintf(stderr, "Writing extent <%lld, %lld>\n", ep->extents[i].base, ep->extents[i].length);
			if (WriteExtent(outf, vop->devp, ep->extents[i].base, ep->extents[i].length) == -1) {
				if (verbose)
					fprintf(stderr, "\tWrite failed\n");
				break;
			}
			count++;
		}
	}
	gzclose(outf);
	if (count != vop->count)
		fprintf(stderr, "WHOAH!  we're short by %zd objects!\n", vop->count - count);


done:
	if (objs)
		free(objs);
	return;

}
