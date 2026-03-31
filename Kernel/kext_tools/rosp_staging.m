//
//  rosp_staging.m
//  kext_tools
//
//  Created by Jack Kim-Biggs on 7/16/19.
//

#import <Foundation/Foundation.h>
#import <IOKit/kext/OSKext.h>
#import <IOKit/kext/OSKextPrivate.h>
#import <Bom/Bom.h>
#import <APFS/APFS.h>

#import <stdio.h>
#import <stdbool.h>
#import <sysexits.h>

#import "safecalls.h"
#import "rosp_staging.h"
#import "bootcaches.h"
#import "kext_tools_util.h"

static int copyKernels(NSString *srcDir, NSString *dstDir);
static int copyKernelWithCopier(BOMCopier copier, NSString *srcDir, NSString *filename, NSString *dstPath);

void fatalFileErrorHandler(BOMCopier copier, const char *path, int errno_)
{
	(void)copier;
	LOG_ERROR("BOMCopier fatal file error: %d at %s", errno_, path);
}
void fatalErrorHandler(BOMCopier copier, const char *message)
{
	(void)copier;
	LOG_ERROR("BOMCopier fatal error: %s", message);
}
BOMCopierCopyOperation fileErrorHandler(BOMCopier copier, const char *path, int errno_)
{
	(void)copier;
	LOG_ERROR("BOMCopier file error: %d at %s", errno_, path);
	return BOMCopierContinue;
}

bool requiresDeferredUpdate(char *prelinkedDirPath)
{
	NSString *plkPath = [NSString stringWithUTF8String:prelinkedDirPath];
	plkPath = plkPath.stringByResolvingSymlinksInPath;
	return [plkPath hasSuffix:@(_kOSKextTemporaryPrelinkedKernelsPath)];
}

#define kOSKextDeferredBootcachesInstallScriptPath "/private/var/install/shove_kernels"
#define kOSKextDeferredBootcachesInstallScriptMode 0700
#define kOSKextDeferredUpdateScriptContents \
"#!/bin/sh\n" \
"/usr/sbin/kcditto\n" \
"exit 0"

int createDeferredUpdateScript(void)
{
	LOG("Creating deferred update script...");
	int result = EX_SOFTWARE;
	int rootfd = 0;

	rootfd = open("/", O_RDONLY | O_NOFOLLOW);
	if (rootfd < 0) {
		result = EX_OSERR;
		goto finish;
	}

	int fd = sopen_ifExists(
				rootfd,
				kOSKextDeferredBootcachesInstallScriptPath,
				O_WRONLY | O_CREAT | O_TRUNC | O_NOFOLLOW,
				kOSKextDeferredBootcachesInstallScriptMode,
				/* failIfExists */ false);
	if (fd < 0) {
		result = EX_OSERR;
		goto finish;
	}

	if (fchmod(fd, kOSKextDeferredBootcachesInstallScriptMode) < 0) {
		result = EX_OSERR;
		goto finish;
	}

	ssize_t written;
	do {
		written = write(fd, kOSKextDeferredUpdateScriptContents, sizeof(kOSKextDeferredUpdateScriptContents));
	} while (written < 0 && errno == EINTR);
	if (written < 0) {
		result = EX_OSERR;
		goto finish;
	}
	result = EX_OK;
finish:
	if (result != EX_OK) {
		LOG_ERROR("Error creating script %s: %d (%s)", kOSKextDeferredBootcachesInstallScriptPath, errno, strerror(errno));
	}
	return result;
}

int copyKernelsInVolume(char *volRoot)
{
	int result = EX_SOFTWARE;
	struct bootCaches *caches = NULL;
	char *tempPath = NULL, *destPath = NULL;
	NSString *tempPathString = nil; NSString *destPathString = nil;

	if (geteuid() != 0) {
		LOG_ERROR("You must be running as root to manage the prelinked kernel staging area.");
		result = EX_NOPERM;
		goto finish;
	}

	caches = readBootCaches(volRoot, kBROptsNone);
	if (!caches) {
		LOG_ERROR("Error reading bootcaches for volume %s...", volRoot);
		goto finish;
	}

	if (!caches->readonly_kext_boot_cache_file) {
		LOG_ERROR("Error reading bootcaches for volume %s - no plk path?", volRoot);
		goto finish;
	}
	destPath = caches->readonly_kext_boot_cache_file->rpath;

	if (!caches->kext_boot_cache_file) {
		LOG_ERROR("Error reading bootcaches for volume %s - no plk staging path?", volRoot);
		goto finish;
	}
	tempPath = caches->kext_boot_cache_file->rpath;

	/* Remove the last path component, since it names the prelinkedkernel filename. */
	tempPathString = [NSString pathWithComponents:@[
		[NSString stringWithUTF8String:volRoot],
		[[NSString stringWithUTF8String:tempPath] stringByDeletingLastPathComponent],
	]];
	if (!tempPathString) {
		LOG_ERROR("Error considering prelinked staging area...");
		goto finish;
	}

	destPathString = [NSString pathWithComponents:@[
		[NSString stringWithUTF8String:volRoot],
		[[NSString stringWithUTF8String:destPath] stringByDeletingLastPathComponent],
	]];
	if (!destPathString) {
		LOG_ERROR("Error considering prelinked destination...");
		goto finish;
	}

	if ((result = copyKernels(tempPathString, destPathString) != EX_OK)) {
		LOG_ERROR("Error copying kernels...");
		goto finish;
	}

	result = EX_OK;
finish:
	free(caches);
	return result;
}

static int copyKernels(NSString *srcDir, NSString *dstDir)
{
	int result = EX_SOFTWARE;

	NSArray<NSString *> *kernelcache_names = @[
		@"immutablekernel",
		@"prelinkedkernel",
	];

	BOMCopier copier = BOMCopierNew();
	if (!copier) {
		LOG_ERROR("Error creating BOMCopier");
		goto finish;
	}
	BOMCopierSetFatalErrorHandler(copier, fatalErrorHandler);
	BOMCopierSetFatalFileErrorHandler(copier, fatalFileErrorHandler);
	BOMCopierSetFileErrorHandler(copier, fileErrorHandler);


	for (NSString *kernelcache in kernelcache_names) {
		if ((result = copyKernelWithCopier(copier, srcDir, kernelcache, dstDir)) != EX_OK) {
			goto finish;
		}

	}

	BOMCopierFree(copier);
	result = EX_OK;
finish:
	return result;
}

/*
 * filenames look like:
 * immutablekernel
 * immutablekernel.development
 * immutablekernel.development.j132ap.im4m
 * immutablekernel.development.j137ap.im4m
 * immutablekernel.development.x589iclydev.im4m
 * immutablekernel.j132ap.im4m
 * immutablekernel.j137ap.im4m
 * prelinkedkernel
 * prelinkedkernel.development
 * prelinkedkernel.kasan
 *
 * So we treat each kc name / suffix pairing as a "prefix" to copy other stuff over, to make sure
 * that we get the .im4m images and per-device immutable caches in case we're building internally.
 * We'll only try to copy all the files if we manage to stat a file with that prefix.
 */

static int copyKernelWithCopier(BOMCopier copier, NSString *srcDir, NSString *filename, NSString *dstPath)
{
	int result = EX_SOFTWARE;

	NSError             *pathError   = nil;
	NSString            *srcPath     = [srcDir stringByAppendingPathComponent:filename];
	NSPredicate         *dirFilter   = [NSPredicate predicateWithFormat:@"lastPathComponent BEGINSWITH %@", filename];
	NSFileManager       *fm          = [NSFileManager defaultManager];
	NSArray<NSString *> *dirContents = [fm contentsOfDirectoryAtPath:srcDir error:&pathError];
	NSArray<NSString *> *matches     = [dirContents filteredArrayUsingPredicate:dirFilter];

	if (pathError) {
		LOG_ERROR("Encountered error while inspecting path: %s", [pathError.description UTF8String]);
		result = EX_OK; // fail silently and log the error
		goto finish;
	}

	if (![fm fileExistsAtPath:srcPath isDirectory:false]) {
		result = EX_OK; // silently fail, since we didn't find anything
		goto finish;
	}
	for (NSString *match in matches) {
		NSString   *fullPath       = [srcDir stringByAppendingPathComponent:match];
		const char *srcPathCString = [fullPath UTF8String];
		const char *dstPathCString = [dstPath UTF8String];
		if (!srcPathCString || !dstPathCString) {
			LOG_ERROR("Error considering prelinked kernel move: srcPath = %s, dstPath = %s",
					  srcPathCString ? srcPathCString : "(null)",
					  dstPathCString ? dstPathCString : "(null)");
			goto finish;
		}
		LOG("Copying kernel: %s -> %s", srcPathCString, dstPathCString);
		if ((result = BOMCopierCopy(copier, srcPathCString, dstPathCString)) != EX_OK) {
			LOG_ERROR("Error moving prelinked kernel: srcPath = %s, dstPath = %s, error %d",
					  srcPathCString ? srcPathCString : "(null)",
					  dstPathCString ? dstPathCString : "(null)",
					  result);
			goto finish;
		}
	}
	result = EX_OK;
finish:
	return result;
}

