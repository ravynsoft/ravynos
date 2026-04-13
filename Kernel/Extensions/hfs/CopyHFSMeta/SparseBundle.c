#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <err.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <removefile.h>

#include <CoreFoundation/CoreFoundation.h>
#include <System/sys/fsctl.h> 

#include "hfsmeta.h"
#include "Sparse.h"

/*
 * Routines to maniupulate a sparse bundle.
 * N.B.:  The sparse bundle format it uses is a subset of
 * the real sparse bundle format:  no partition map, and
 * no encryption.
 */

#ifndef MIN
# define MIN(a, b) \
	({ __typeof(a) __a = (a); __typeof(b) __b = (b); \
		__a < __b ? __a : __b; })
#endif

/*
 * Context for the sparse bundle routines.  The path name,
 * size of the band files, and cached file descriptor and
 * band numbers, to reduce the amount of pathname lookups
 * required.
 */
struct SparseBundleContext {
	char *pathname;
	size_t bandSize;
	int cfd;	// Cached file descriptor
	int cBandNum;	// cached bandfile number
};

static const int kBandSize = 8388608;

// Prototype bundle Info.plist file
static const char *bundlePrototype =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
"<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
"<plist version=\"1.0\">\n"
"<dict>\n"
                "\t<key>CFBundleInfoDictionaryVersion</key>\n"
                "\t<string>6.0</string>\n"
                "\t<key>band-size</key>\n"
                "\t<integer>%d</integer>\n"
                "\t<key>bundle-backingstore-version</key>\n"
                "\t<integer>1</integer>\n"
                "\t<key>diskimage-bundle-type</key>\n"
                "\t<string>com.apple.diskimage.sparsebundle</string>\n"
                "\t<key>size</key>\n"
                "\t<integer>%llu</integer>\n"
"</dict>\n"
	"</plist>\n";


/*
 * Read from a sparse bundle.  If the band file doesn't exist, or is shorter than
 * what we need to get from it, we pad out with 0's.
 */
static ssize_t
doSparseRead(struct IOWrapper *context, off_t offset, void *buffer, off_t len)
{
	struct SparseBundleContext *ctx = context->context;
	off_t blockSize = ctx->bandSize;
	ssize_t nread = 0;
	ssize_t retval = -1;

	while (nread < len) {
		off_t bandNum = (offset + nread) / blockSize;	// Which band file to use
		off_t bandOffset = (offset + nread) % blockSize;	// how far to go into the file
		ssize_t amount = MIN(len - nread, blockSize - bandOffset);	// How many bytes to write in this band file
		char *bandName;
		ssize_t n;
		int fd;

		asprintf(&bandName, "%s/bands/%llx", ctx->pathname, bandNum);
		if (!bandName) {
			warn("Cannot allocate memory for path '%s/bands/%llx'", ctx->pathname, bandNum);
			retval = -1;
			goto done;
		}
		fd = open(bandName, O_RDONLY);
		if (fd == -1) {
			if (errno == ENOENT) {
				// Doesn't exist, so we just write zeroes
				free(bandName);
				memset(buffer + nread, 0, amount);
				nread += amount;
				continue;
			}
			warn("Cannot open band file %s for offset %llu", bandName, offset + nread);
			retval = -1;
			free(bandName);
			goto done;
		}

		n = pread(fd, (char*)buffer + nread, amount, bandOffset);
		if (n == -1) {
			warn("Cannot read from band file %s for offset %llu for amount %zu", bandName, offset+nread, amount);
			close(fd);
			free(bandName);
			goto done;
		}
		if (n < amount) {	// hit EOF, pad out with zeroes
			memset(buffer + nread + amount, 0, amount - n);
		}
		free(bandName);
		close(fd);
		nread += n;
	}
	retval = nread;
done:
	return retval;
	
}

/*
 * Write a chunk of data to a bundle.
 */
static ssize_t
doSparseWrite(IOWrapper_t *context, off_t offset, void *buffer, off_t len)
{
	struct SparseBundleContext *ctx = context->context;
	off_t blockSize = ctx->bandSize;
	ssize_t written = 0;
	ssize_t retval = -1;

	while (written < len) {
		off_t bandNum = (offset + written) / blockSize;	// Which band file to use
		off_t bandOffset = (offset + written) % blockSize;	// how far to go into the file
		size_t amount = MIN(len - written, blockSize - bandOffset);	// How many bytes to write in this band file
		ssize_t nwritten;
		int fd;

		if (ctx->cfd == -1 || ctx->cBandNum != bandNum) {
			char *bandName = NULL;

			if (ctx->cfd != -1) {
				close(ctx->cfd);
				ctx->cfd = -1;
			}
			asprintf(&bandName, "%s/bands/%llx", ctx->pathname, bandNum);
			if (!bandName) {
				warnx("Cannot allocate memory for band %s/bands/%llx", ctx->pathname, bandNum);
				retval = -1;
				goto done;
			}

			fd = open(bandName, O_WRONLY | O_CREAT, 0666);
			if (fd == -1) {
				warn("Cannot open band file %s for offset %llu", bandName, offset + written);
				free(bandName);
				retval = -1;
				goto done;
			}
			/*
			 * When we create a new band file, we sync the volume
			 * it's on, so that we can ensure that the band file is present
			 * on disk.  (Otherwise, with a crash, we can end up with the
			 * data not where we expected.)  In this case, however, we probably
			 * don't need to wait for it -- just start the sync.
			 */
			fsync_volume_np(fd, 0);
			fcntl(fd, F_NOCACHE, 1);
			free(bandName);
			bandName = NULL;
			ctx->cfd = fd;
			assert(bandNum < INT_MAX);
			ctx->cBandNum = (int)bandNum;
		} else {
			fd = ctx->cfd;
		}
		nwritten = pwrite(fd, (char*)buffer + written, amount, bandOffset);
		if (nwritten == -1) {
			warn("Cannot write to band file %s/band/%llx for offset %llu for amount %zu", ctx->pathname, bandNum, offset+written, amount);
			close(fd);
			ctx->cfd = -1;
			retval = -1;
			goto done;
		}
		// Sync the data out.
		fsync(fd);
		written += nwritten;
	}
	retval = written;
done:
	return retval;
	
}

/*
 * Write a given extent (<start, length> pair) from an input device to the
 * sparse bundle.  We also use a block to update progress.
 */
static ssize_t
WriteExtentToSparse(struct IOWrapper * context, DeviceInfo_t *devp, off_t start, off_t len, void (^bp)(off_t))
{
	const ssize_t bufSize = 1024 * 1024;
	uint8_t *buffer = NULL;
	ssize_t retval = 0;
	off_t total = 0;

	if (debug) printf("Writing extent <%lld, %lld>\n", start, len);
	buffer = malloc(bufSize);
	if (buffer == NULL) {
		warn("%s(%s):  Could not allocate %zu bytes for buffer", __FILE__, __FUNCTION__, bufSize);
		retval = -1;
		goto done;
	}

	while (total < len) {
		ssize_t nread;
		ssize_t nwritten;
		ssize_t amt = MIN(bufSize, len - total);
		nread = UnalignedRead(devp, buffer, amt, start + total);
		if (nread == -1) {
			warn("Cannot read from device at offset %lld", start + total);
			retval = -1;
			break;
		}
		if (nread < amt) {
			warnx("Short read from source device -- got %zd, expected %zd", nread, amt);
		}
		nwritten = doSparseWrite(context, start + total, buffer, nread);
		if (nwritten == -1) {
			retval = -1;
			break;
		}
		bp(nread);
		total += nread;
	}
	if (debug) printf("\twrote %lld\n", total);
done:
	if (buffer)
		free(buffer);
	return retval;
}

static const CFStringRef kBandSizeKey = CFSTR("band-size");
static const CFStringRef kDevSizeKey = CFSTR("size");

/*
 * We need to be able to get the size of the "device" from a sparse bundle;
 * we do this by using CF routines to parse the Info.plist file, and then
 * get the two keys we care about:  band-size (size of the band files), and
 * size (size -- in bytes -- of the "disk").
 */
static int
GetSizesFromPlist(const char *path, size_t *bandSize, off_t *devSize)
{
	int retval = -1;
	CFReadStreamRef inFile = NULL;
	CFURLRef inFileURL = NULL;
	CFStringRef cfPath = NULL;
	CFPropertyListRef cfDict = NULL;
	CFNumberRef cfVal = NULL;
	int tmpInt;
	long long tmpLL;


	inFileURL = CFURLCreateFromFileSystemRepresentation(NULL, (const UInt8 *)path, strlen(path), FALSE);
	if (inFileURL == NULL) {
		if (debug) warn("Cannot create url from pathname %s", path);
		goto done;
	}

	inFile = CFReadStreamCreateWithFile(NULL, inFileURL);
	if (inFile == NULL) {
		if (debug) warn("cannot create read stream from path %s", path);
		goto done;
	}

	if (CFReadStreamOpen(inFile) == FALSE) {
		if (debug) warn("cannot open read stream");
		goto done;
	}

	cfDict = CFPropertyListCreateWithStream(NULL, inFile, 0, 0, NULL, NULL);
	if (cfDict == NULL) {
		if (debug) warnx("cannot create propertly list from stream for path %s", path);
		goto done;
	}

	cfVal = CFDictionaryGetValue(cfDict, kBandSizeKey);
	if (cfVal == NULL) {
		if (debug) warnx("cannot get bandsize key from plist");
		goto done;
	}

	if (CFNumberGetValue(cfVal, kCFNumberIntType, &tmpInt) == false) {
		if (debug) warnx("cannot get value from band size number");
		goto done;
	} else {
		*bandSize = tmpInt;
	}

	cfVal = CFDictionaryGetValue(cfDict, kDevSizeKey);
	if (cfVal == NULL) {
		if (debug) warnx("cannot get dev size key from plist");
		goto done;
	}
	if (CFNumberGetValue(cfVal, kCFNumberLongLongType, &tmpLL) == false) {
		goto done;
	} else {
		*devSize = tmpLL;
	}
	retval = 0;

done:

	if (cfPath)
		CFRelease(cfPath);
	if (inFileURL)
		CFRelease(inFileURL);
	if (inFile)
		CFRelease(inFile);
	if (cfDict)
		CFRelease(cfDict);
	return retval;
}

#define kProgressName "HC.progress.txt"

/*
 * Get the progress state from a sparse bundle.  If it's not there, then
 * no progress.
 */
static off_t
GetProgress(struct IOWrapper *context)
{
	struct SparseBundleContext *ctx = context->context;
	FILE *fp = NULL;
	off_t retval = 0;
	char progFile[strlen(ctx->pathname) + sizeof(kProgressName) + 2];	// '/' and NUL

	sprintf(progFile, "%s/%s", ctx->pathname, kProgressName);
	fp = fopen(progFile, "r");
	if (fp == NULL) {
		goto done;
	}
	if (fscanf(fp, "%llu", &retval) != 1) {
		retval = 0;
	}
	fclose(fp);
done:
	return retval;
}

/*
 * Write the progress information out.  This involves writing a file in
 * the sparse bundle with the amount -- in bytes -- we've written so far.
 */
static void
SetProgress(struct IOWrapper *context, off_t prog)
{
	struct SparseBundleContext *ctx = context->context;
	FILE *fp = NULL;
	char progFile[strlen(ctx->pathname) + sizeof(kProgressName) + 2];	// '/' and NUL

	sprintf(progFile, "%s/%s", ctx->pathname, kProgressName);
	if (prog == 0) {
		remove(progFile);
	} else {
		fp = fopen(progFile, "w");
		if (fp) {
			(void)fprintf(fp, "%llu\n", prog);
			fclose(fp);
		}
	}
	return;
}

/*
 * Clean up.  This is used when we have to initialize the bundle, but don't
 * have any progress information -- in that case, we don't want to have any
 * of the old band files laying around.  We use removefile() to recursively
 * remove them, but keep the bands directory.
 */
int
doCleanup(struct IOWrapper *ctx)
{
	struct SparseBundleContext *context = ctx->context;
	int rv = 0;
	char bandsDir[strlen(context->pathname) + sizeof("/bands") + 1];	// 1 for NUL

	sprintf(bandsDir, "%s/bands", context->pathname);

	if (debug)
		fprintf(stderr, "Cleaning up, about to call removefile\n");
	rv = removefile(bandsDir, NULL, REMOVEFILE_RECURSIVE | REMOVEFILE_KEEP_PARENT);
	if (debug)
		fprintf(stderr, "removefile returned %d\n", rv);

	return (rv == 0) ? 0 : -1;
}

/*
 * Initialize the IOWrapper structure for a sparse bundle.  This will
 * create the bundle directory (but not its parents!) if needed, and
 * will populate it out.  It checks to see if there is an existing bundle
 * of the same name, and, if so, ensures that the izes are correct.  Then
 * it sets up all the function pointers.
 */
struct IOWrapper *
InitSparseBundle(const char *path, DeviceInfo_t *devp)
{
	struct SparseBundleContext ctx = { 0 };
	IOWrapper_t *wrapper = NULL;
	struct stat sb;
	char *tmpname = NULL;
	int tmplen;

	if (strstr(path, ".sparsebundle") == NULL) {
		asprintf(&ctx.pathname, "%s.sparsebundle", path);
	} else {
		ctx.pathname = strdup(path);
	}

	if (ctx.pathname == NULL) {
		return NULL;
	}

	if (lstat(ctx.pathname, &sb) == -1) {
		if (errno != ENOENT) {
			warn("cannot check sparse bundle %s", ctx.pathname);
			goto done;
		}
		if (mkdir(ctx.pathname, 0777) == -1) {
			warn("cannot create sparse bundle %s", ctx.pathname);
			goto done;
		}
	} else if ((sb.st_mode & S_IFMT) != S_IFDIR) {
		warnx("sparse bundle object %s is not a directory", ctx.pathname);
		goto done;
	}
	/* NB! All names we create inside the bundle directory are shorter
	 * then "Info.plist", we re-use the buffer allocated here for
	 * multiple file names. */
	tmplen = asprintf(&tmpname, "%s/Info.plist", ctx.pathname);
	if (tmpname == NULL) {
		goto done;
	}

	if (stat(tmpname, &sb) != -1) {
		size_t bandSize = 0;
		off_t devSize = 0;
		if (GetSizesFromPlist(tmpname, &bandSize, &devSize) == -1) {
			warnx("Existing sparse bundle can't be parsed");
			goto done;
		}
		if (debug)
			printf("Existing sparse bundle size = %lld, bandsize = %zu\n", devSize, bandSize);

		if (devSize != devp->size) {
			warnx("Existing sparse bundle size (%lld) != dev size (%lld)", devSize, devp->size);
			goto done;
		}
		ctx.bandSize = bandSize;
	} else {
		FILE *fp = fopen(tmpname, "w");
		if (fp == NULL) {
			warn("cannot create sparse bundle info plist %s", tmpname);
			goto done;
		}
		ctx.bandSize = kBandSize;
		if (fprintf(fp, bundlePrototype, kBandSize, devp->size) < 0) {
			warn("failed to set bundle information in %s", tmpname);
			fclose(fp); /* eat error return */
			goto done;
		}
		if (fclose(fp) != 0) {
			warn("failed to update bundle information in %s", tmpname);
			goto done;
		}
		snprintf(tmpname, tmplen + 1, "%s/Info.bckup", ctx.pathname);
		if ((int)strlen(tmpname) == tmplen) { /* Only update backup if we get the full path */
			/* Failure to create a backup is not fatal */
			fp = fopen(tmpname, "w");
			if (fp) {
				if ((fprintf(fp, bundlePrototype, kBandSize, devp->size) < 0) ||
					(fclose(fp) < 0)) {
					unlink(tmpname);
				}
			}
		} else {
			warn("Cannot create backup bundle information file in %s", tmpname);
		}
		if ((snprintf(tmpname, tmplen + 1, "%s/bands", ctx.pathname) > tmplen) ||
			(mkdir(tmpname, 0777) == -1)) {
			warn("cannot create bands directory in sparse bundle %s", ctx.pathname);
			goto done;
		}
		if (snprintf(tmpname, tmplen + 1, "%s/token", ctx.pathname) <= tmplen) {
			close(open(tmpname, O_CREAT | O_TRUNC, 0666));
		}
	}

	wrapper = malloc(sizeof(*wrapper));
	if (wrapper) {
		struct SparseBundleContext *wrapped_ctx;

		wrapped_ctx = malloc(sizeof(*wrapped_ctx));
		if (wrapped_ctx) {
			*wrapped_ctx = ctx;
			wrapped_ctx->cfd = -1;

			wrapper->writer = &WriteExtentToSparse;
			wrapper->reader = &doSparseRead;
			wrapper->getprog = &GetProgress;
			wrapper->setprog = &SetProgress;
			wrapper->cleanup = &doCleanup;
			wrapper->context = wrapped_ctx;
		} else {
			free(wrapper);
			wrapper = NULL;
		}
	}
done:
	if (wrapper == NULL) {
		if (ctx.pathname)
			free(ctx.pathname);
	}

	free(tmpname);
	return wrapper;
}
