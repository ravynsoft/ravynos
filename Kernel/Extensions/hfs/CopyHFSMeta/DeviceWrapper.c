#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <err.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/disk.h>

#include "hfsmeta.h"

/*
 * Functions to wrap around a device.
 */
#define MIN(a, b) \
	({ __typeof(a) __a = (a); __typeof(b) __b = (b); \
		__a < __b ? __a : __b; })

struct DeviceWrapperContext {
	char *pathname;
	size_t blockSize;
	off_t devSize;
	int fd;
};

static int
noClean(struct IOWrapper *ctx)
{
	// Conceivably, we could erase the entire device
	return 0;
}

static ssize_t
doRead(struct IOWrapper *ctx, off_t start, void *buffer, off_t len)
{
	// For now, just do a pread
	struct DeviceWrapperContext *dctx = (struct DeviceWrapperContext*)ctx->context;

	return pread(dctx->fd, buffer, (size_t)len, start);
}

static ssize_t
writeExtent(struct IOWrapper *context, DeviceInfo_t *devp, off_t start, off_t len, void (^bp)(off_t))
{
	const size_t bufSize = 1024 * 1024;
	struct DeviceWrapperContext *ctx = (struct DeviceWrapperContext*)context->context;
	uint8_t *buffer = NULL;
	ssize_t retval = 0;
	off_t total = 0;

	if (debug) printf("Writing extent <%lld, %lld> to device %s", start, len, ctx->pathname);

	buffer = malloc(bufSize);
	if (buffer == NULL) {
		warn("%s(%s): Could not allocate %zu bytes for buffer", __FILE__, __FUNCTION__, bufSize);
		retval = -1;
		goto done;
	}

	while (total < len) {
		ssize_t nread;
		size_t amt = MIN(bufSize, len - total);
		// XXX - currently, DeviceWrapepr isn't used, but it needs to deal wit unaligned I/O when it is.
		nread = pread(devp->fd, buffer, amt, start + total);
		if (nread == -1) {
			warn("Cannot read from device at offset %lld", start + total);
			retval = -1;
			goto done;
		}
		(void)pwrite(ctx->fd, (char*)buffer, nread, start + total);
		bp(nread);
		total += nread;
	}
done:
	if (buffer)
		free(buffer);
	return retval;
}

/*
 * Device files can't have progress information stored, so we don't do anything.
 */
static off_t
GetProgress(struct IOWrapper *context)
{
	return 0;
}
static void
SetProgress(struct IOWrapper *context, off_t progr)
{
	return;
}

struct IOWrapper *
InitDeviceWrapper(const char *path, DeviceInfo_t *devp)
{
	struct DeviceWrapperContext ctx = { .fd = -1 };
	struct DeviceWrapperContext *retctx = NULL;
	IOWrapper_t *retval = NULL;
	struct stat sb;
	uint64_t blockCount;
	char rawname[strlen(path) + 2];	// /dev/disk5 -> /dev/rdisk5

	if (strncmp(path, "/dev/disk", 9) == 0) {
		// Need to make it into a raw device name
		sprintf(rawname, "/dev/rdisk%s", path + 9);
	} else {
		strcpy(rawname, path);
	}

	if (lstat(rawname, &sb) == -1) {
		warn("cannot examine raw device %s", rawname);
		goto done;
	}
	if ((sb.st_mode & S_IFMT) != S_IFCHR) {
		warnx("device %s is not a raw device", rawname);
		goto done;
	}

	ctx.fd = open(rawname, O_RDWR);
	if (ctx.fd == -1) {
		warn("Cannot open device %s for reading and writing", rawname);
		goto done;
	}

	if (ioctl(ctx.fd, DKIOCGETBLOCKSIZE, &ctx.blockSize) == -1) {
		ctx.blockSize = 512;	// A reasonable default
	}
	if (ioctl(ctx.fd, DKIOCGETBLOCKCOUNT, &blockCount) == -1) {
		warn("Cannot block count for device %s", rawname);
		goto done;
	}
	ctx.devSize = ctx.blockSize * blockCount;

	if (ctx.devSize != devp->size) {
		warnx("Device %s is not the same size (%lld) as source device (%lld)", rawname, ctx.devSize, devp->size);
		goto done;
	}

	ctx.pathname = strdup(rawname);
	if (ctx.pathname == NULL) {
		warn("Cannot strdup the pathname");
		goto done;
	}
	retctx = malloc(sizeof(ctx));
	if (retctx == NULL) {
		warn("Cannot allocate space for device context");
		goto done;
	}
	*retctx = ctx;
	retval = malloc(sizeof(*retval));
	if (retval == NULL) {
		warn("Cannot allocate space for device wrapper");
		goto done;
	}
	retval->context = retctx;
	retval->reader = &doRead;
	retval->writer = &writeExtent;
	retval->getprog = &GetProgress;
	retval->setprog = &SetProgress;
	retval->cleanup = &noClean;

done:
	if (!retval) {
		free(ctx.pathname);
		free(retctx);
		if (ctx.fd >= 0)
			close(ctx.fd);
	}

	return retval;
}
