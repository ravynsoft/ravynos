/*
 *  staging.m
 *  kext_tools
 *
 *  Copyright 2017 Apple Inc. All rights reserved.
 *
 */
#import <Foundation/Foundation.h>
#import <sys/csr.h>
#import <rootless.h>
#import <copyfile.h>
#import <os/cleanup.h>

#import <IOKit/kext/OSKextPrivate.h>

#import "kext_tools_util.h"
#import "security.h"
#import "staging.h"
#import "syspolicy.h"

// This requirement says a bundle must be apple signed or signed by NVIDIA's kernel extension
// signing certificate (a generic kernel extension certificate + NVIDIA's team ID).
#define GPU_BUNDLE_STAGING_REQUIREMENT "anchor apple or (anchor apple generic " \
        "and certificate 1[field.1.2.840.113635.100.6.2.6] " \
        "and certificate leaf[field.1.2.840.113635.100.6.1.13] " \
        "and certificate leaf[field.1.2.840.113635.100.6.1.18] " \
        "and certificate leaf[subject.OU] = \"6KR3T733EC\")"

#define INSECURE_GRAPHICS_BUNDLE_PATH "/System/Library/Extensions"
#define SECURE_GRAPHICS_BUNDLE_STAGING_PATH "/Library/GPUBundles"
#define SECURE_KEXT_STAGING_PATH "/Library/StagedExtensions"
#define SECURE_KEXT_VALIDATION_ROOT "/private/var/db/KernelExtensionManagement/Staging"

static CFStringRef kCompanionBundlesKey = CFSTR("GPUCompanionBundles");
static CFStringRef kOptionalCompanionBundlesKey = CFSTR("GPUOptionalCompanionBundles");

typedef BOOL (^BundleURLHandler)(NSURL *, NSURL *);


#pragma mark Helper Functions

static BOOL
isSIPDisabled(void) {
    static BOOL sInitialized = NO;
    static BOOL sSIPDisabled = NO;

    if (!sInitialized) {
        sSIPDisabled = csr_check(CSR_ALLOW_UNTRUSTED_KEXTS) == 0 ? YES : NO;
        sInitialized = YES;
    }

    return sSIPDisabled;
}

Boolean
pathIsSecure(NSString *path) {
    Boolean is_secure = false;
    BOOL is_protected_volume = rootless_protected_volume(path.UTF8String) == 1 ? YES : NO;
    BOOL is_trusted_path = rootless_check_trusted_class(path.UTF8String, "KernelExtensionManagement") == 0 ? YES : NO;

    if (isSIPDisabled()) {
        // SIP is disabled so everything is considered secure.
        is_secure = true;
    } else if (!is_protected_volume) {
        // SIP is enabled and the volume is not protected, so it's insecure.
        is_secure = false;
    } else {
        // SIP is enabled and it is a protected volume, so it's only secure if it's trusted.
        is_secure = is_trusted_path;
    }
    return is_secure;
}

static BOOL
gpuBundleValidates(NSURL *bundleURL)
{
    OSStatus result = 0;
    SecStaticCodeRef staticCodeRef = NULL;
    SecRequirementRef requirementRef = NULL;
    BOOL passesRequirement = NO;

    result = SecStaticCodeCreateWithPath((__bridge CFURLRef)bundleURL, kSecCSDefaultFlags, &staticCodeRef);
    if (result != errSecSuccess) {
        OSKextLogCFString(NULL,
                          kOSKextLogErrorLevel | kOSKextLogLoadFlag,
                          CFSTR("Unable to create SecCode (%d): %@"),
                          (int)result, bundleURL.path);
        goto __out;
    }

    result = SecRequirementCreateWithString(CFSTR(GPU_BUNDLE_STAGING_REQUIREMENT), kSecCSDefaultFlags, &requirementRef);
    if (result) {
        OSKextLogCFString(NULL,
                          kOSKextLogErrorLevel | kOSKextLogLoadFlag,
                          CFSTR("Error creating requirement: %d"),
                          (int)result);
        goto __out;
    }

    result = SecStaticCodeCheckValidity(staticCodeRef, (SecCSFlags)kSecCSStrictValidate, requirementRef);
    if (result) {
        OSKextLogCFString(NULL,
                          kOSKextLogErrorLevel | kOSKextLogLoadFlag,
                          CFSTR("GPU Bundle failed validity check (%d): %@"),
                          (int)result, bundleURL.path);
        goto __out;
    }

    passesRequirement = YES;

__out:
    SAFE_RELEASE(requirementRef);
    SAFE_RELEASE(staticCodeRef);
    return passesRequirement;
}

static BOOL
kextBundleValidates(NSURL *bundleURL)
{
    BOOL validates = NO;
    OSKextRef kext = NULL;
    OSStatus status = 0;

    kext = OSKextCreate(NULL, (__bridge CFURLRef)bundleURL);
    if (!kext) {
        OSKextLogCFString(NULL,
                          kOSKextLogErrorLevel | kOSKextLogLoadFlag,
                          CFSTR("Could not create kext to validate: %@"), bundleURL.path);
        goto __out;
    }

    status = checkKextSignature(kext, true, true);
    if (status) {
        if (isInvalidSignatureAllowed()) {
            OSKextLogCFString(NULL,
                              kOSKextLogErrorLevel | kOSKextLogValidationFlag,
                              CFSTR("Kext with invalid signature (%d) allowed: %@"),
                              (int)status, bundleURL.path);
        } else {
            OSKextLogCFString(NULL,
                              kOSKextLogErrorLevel | kOSKextLogValidationFlag,
                              CFSTR("Kext with invalid signature (%d) denied: %@"),
                              (int)status, bundleURL.path);
            goto __out;
        }
    }

    validates = YES;

__out:
    SAFE_RELEASE(kext);
    return validates;
}

static void
ensureValidationAreaInitialized(void)
{
    int result = 0;
    // This mode is 755, which is the same permission the installer places on the root folder.
    mode_t mode = S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;

    // This is necessary for a while to allow tests to pass and to allow roots to move around betwee
    // systems that already have this protected directory and versions that don't yet.
    // <rdar://problem/60972335> Remove rootless_mkdir_restricted once its been in enough builds
    result = rootless_mkdir_restricted("/private/var/db/KernelExtensionManagement", mode, "KernelExtensionManagement");
    if (result == -1 && errno != EEXIST) {
        OSKextLogCFString(NULL,
                          kOSKextLogErrorLevel | kOSKextLogLoadFlag,
                          CFSTR("Error creating new protected root: %d"),
                          errno);
    }

    result = mkdir(SECURE_KEXT_VALIDATION_ROOT, mode);
    if (result == -1 && errno != EEXIST) {
        OSKextLogCFString(NULL,
                          kOSKextLogErrorLevel | kOSKextLogLoadFlag,
                          CFSTR("Error creating staging directory: %d"),
                          errno);
    }
}

static NSURL *
makeTemporaryValidationRoot(NSString *validationRoot)
{
    int __os_close fd = -1;
    NSURL *validationURL = nil;
    const char *newDirectory = NULL;
    char template[] = "tmp.XXXXXX";

    ensureValidationAreaInitialized();

    fd = open(validationRoot.fileSystemRepresentation, O_RDONLY);
    if (fd < 0) {
        OSKextLogCFString(NULL,
                          kOSKextLogErrorLevel | kOSKextLogLoadFlag,
                          CFSTR("Error opening staging root: %d"),
                          errno);
        goto lb_exit;
    }

    newDirectory = mkdtempat_np(fd, template);
    if (newDirectory == NULL) {
        OSKextLogCFString(NULL,
                          kOSKextLogErrorLevel | kOSKextLogLoadFlag,
                          CFSTR("Error making temporary directory: %d"),
                          errno);
        goto lb_exit;
    }

    validationURL = [NSURL fileURLWithPath:validationRoot];
    validationURL = [validationURL URLByAppendingPathComponent:@(newDirectory)];

lb_exit:
    return validationURL;
}

BOOL
bundleValidates(NSURL *bundleURL, BOOL isGPUBundle)
{
    return isGPUBundle ? gpuBundleValidates(bundleURL) : kextBundleValidates(bundleURL);
}

Boolean
stageBundle(NSURL *sourceURL, NSURL *destinationURL, NSString *validationRoot, BOOL isGPUBundle)
{
    Boolean success = false;
    int copyStatus = 0;
    NSError *error = nil;
    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSUUID *uuid = nil;
    NSURL *temporaryURL = nil;
    NSURL *temporaryRoot = nil;
    NSString *dirName = nil;

    // If the target directory already exists, we need to delete it first.
    if ([fileManager fileExistsAtPath:[destinationURL path]]) {
        [fileManager removeItemAtURL:destinationURL error:&error];
        if (error) {
            OSKextLogCFString(NULL,
                              kOSKextLogErrorLevel | kOSKextLogLoadFlag,
                              CFSTR("Error deleting old bundle while staging: %@"),
                              error);
            success = false;
            goto __out;
        }
    }

    // Create a temporary staging directory name to use pre-validation.
    uuid = [NSUUID UUID];
    if (!uuid) {
        OSKextLogMemError();
        success = false;
        goto __out;
    }

    if (isGPUBundle) {
        dirName = uuid.UUIDString;
    } else {
        // Since we use real OSKext objects to assist validation, we need these to have
        // the appropriate kext ending or they get rejected by IOKit.
        dirName = [NSString stringWithFormat:@"%@.%@", uuid.UUIDString, @"kext"];
    }
    if (!dirName) {
        OSKextLogMemError();
        success = false;
        goto __out;
    }

    temporaryRoot = makeTemporaryValidationRoot(validationRoot);
    if (!temporaryRoot) {
        OSKextLogMemError();
        success = false;
        goto __out;
    }

    temporaryURL = [temporaryRoot URLByAppendingPathComponent:dirName];
    if (!temporaryURL) {
        OSKextLogMemError();
        success = false;
        goto __out;
    }

    copyStatus = copyfile(sourceURL.path.UTF8String, temporaryURL.path.UTF8String, NULL, COPYFILE_ALL | COPYFILE_NOFOLLOW | COPYFILE_RECURSIVE);
    if (copyStatus < 0) {
        OSKextLogCFString(NULL,
                          kOSKextLogErrorLevel | kOSKextLogLoadFlag,
                          CFSTR("Error copying bundle during staging: %d"),
                          copyStatus);
        success = false;
        goto __out;
    }

    // Validate bundle and rename or delete, as appropriate.
    if (bundleValidates(temporaryURL, isGPUBundle)) {
        NSURL *resultURL = nil;

        // Ensure entire set of parent directories exists before we rename the validated bundle into place.
        success = [fileManager createDirectoryAtPath:destinationURL.URLByDeletingLastPathComponent.path
                            withIntermediateDirectories:YES
                                            attributes:nil
                                                error:&error];
        if (!success) {
            OSKextLogCFString(NULL,
                              kOSKextLogErrorLevel | kOSKextLogLoadFlag,
                              CFSTR("Error creating directory during staging: %@"),
                              error);
            success = false;
            [fileManager removeItemAtURL:temporaryURL error:nil];
            goto __out;
        }

        success = [fileManager replaceItemAtURL:destinationURL
                                  withItemAtURL:temporaryURL
                                 backupItemName:nil
                                        options:0
                               resultingItemURL:&resultURL
                                          error:&error];
        if (!success || error) {
            success = false;
            OSKextLogCFString(NULL,
                              kOSKextLogErrorLevel | kOSKextLogLoadFlag,
                              CFSTR("Error renaming bundle during staging: %@"),
                              error);
            [fileManager removeItemAtURL:temporaryURL error:nil];
            goto __out;
        }

        if (![destinationURL.path isEqualToString:resultURL.path]) {
            success = false;
            OSKextLogCFString(NULL,
                              kOSKextLogErrorLevel | kOSKextLogLoadFlag,
                              CFSTR("Error renaming bundle, result ended up in %@. Deleting."),
                              resultURL.path);
            [fileManager removeItemAtURL:resultURL error:nil];
            goto __out;
        }
    } else {
        OSKextLogCFString(NULL,
                          kOSKextLogErrorLevel | kOSKextLogLoadFlag,
                          CFSTR("Bundle (%@) failed to validate, deleting: %@"),
                          sourceURL.path, temporaryURL.path);
        [fileManager removeItemAtURL:temporaryURL error:nil];
        success = false;
        goto __out;
    }

    success = true;

__out:
    if (temporaryRoot) {
        [fileManager removeItemAtPath:temporaryRoot.path error:nil];
    }
    return success;
}

NSData *
copyIdentifierFromBundle(NSURL *url)
{
    OSStatus result = 0;
    SecStaticCodeRef staticCodeRef = NULL;
    SecRequirementRef requirementRef = NULL;
    CFDictionaryRef signingInfo = NULL;
    NSData *cdhashData = nil;

    // This function must compute some identifier from a URL to identify the bundle at the URL
    // such that we can insecurely determine if two copies are identical are not.
    //  - For signed code, we use the cdhash data blob directly out of the signing information.
    //  - For unsigned code, we use a string of the cdhash generated using the legacy adhoc cdhash
    // generation function that was used to generate the legacy allow list.

    result = SecStaticCodeCreateWithPath((__bridge CFURLRef)url, kSecCSDefaultFlags, &staticCodeRef);
    if (result) {
        OSKextLogMemError();
        goto __out;
    }

    result = SecStaticCodeCheckValidity(staticCodeRef, (SecCSFlags)kSecCSBasicValidateOnly, NULL);
    if (result && (result != errSecCSUnsigned)) {
        OSKextLogCFString(NULL,
                          kOSKextLogErrorLevel | kOSKextLogLoadFlag,
                          CFSTR("Basic validation of bundle failed (%d): %@"),
                          (int)result, url.path);
        goto __out;
    }

    if (result == errSecCSUnsigned) {
        // The unsigned case - generate the adhoc signature and package up the string version
        char *cdhashCString = NULL;

        getAdhocSignatureHash((__bridge CFURLRef)url, &cdhashCString, NULL);
        if (!cdhashCString) {
            OSKextLogCFString(NULL,
                              kOSKextLogErrorLevel | kOSKextLogLoadFlag,
                              CFSTR("Unable to create adhoc cdhashfor bundle: %@"),
                              url.path);
            goto __out;
        }

        cdhashData = [NSData dataWithBytes:cdhashCString length:strlen(cdhashCString)];
        free(cdhashCString);
    } else {
        // The signed case - copy the cdhash data blob out of the codesigning dictionary
        result = SecCodeCopySigningInformation(staticCodeRef, kSecCSDefaultFlags, &signingInfo);
        if (result) {
            OSKextLogCFString(NULL,
                              kOSKextLogErrorLevel | kOSKextLogLoadFlag,
                              CFSTR("Failed to copy signing information (%d): %@"),
                              (int)result, url.path);
            goto __out;
        }

        cdhashData = (__bridge NSData *)CFDictionaryGetValue(signingInfo, kSecCodeInfoUnique);
    }

__out:
    SAFE_RELEASE(requirementRef);
    SAFE_RELEASE(signingInfo);
    SAFE_RELEASE(staticCodeRef);
    return cdhashData;
}

BOOL
bundleNeedsStaging(NSURL *sourceURL, NSURL *destinationURL)
{
    BOOL needsStaging = NO;
    NSFileManager *fileManager = [NSFileManager defaultManager];

    if (pathIsSecure(sourceURL.path)) {
        // If the source location is "secure", no need for any staging or further checks.
        needsStaging = NO;
    } else if ([fileManager fileExistsAtPath:destinationURL.path]) {
        // Given the source is insecure, look for an already staged copy and then we only need
        // to re-stage if they no longer match content.
        // NOTE: it is ok to use an insecure comparison here because we will re-validate the bundle once
        // its been moved into a SIP protected location, so the worst an attacker can do here is
        // prevent us from copying a new version, which is still safe.
        NSData *sourceIdentifier = copyIdentifierFromBundle(sourceURL);
        NSData *destinationIdentifier = copyIdentifierFromBundle(destinationURL);

        if (!sourceIdentifier || ![sourceIdentifier isEqualToData:destinationIdentifier]) {
            needsStaging = YES;
        }
    } else {
        // This bundle has never been staged and is in an insecure location, so stage it.
        needsStaging = YES;
    }

    return needsStaging;
}

// A helper function that iterates through an array of bundle names and calls a callback function
// for each bundle that is found to be in an insecure location or where the currently staged
// version no longer matches the unstaged version.
void
forEachInsecureBundleHelper(NSArray *bundles, BundleURLHandler callbackHandler, NSURL *sourceBaseURL, NSURL *targetBaseURL)
{
    NSFileManager *fileManager = [NSFileManager defaultManager];

    for (NSString *bundleName in bundles) {
        NSURL *sourceURL = [[sourceBaseURL URLByAppendingPathComponent:bundleName] URLByStandardizingPath];
        NSURL *targetURL = [[targetBaseURL URLByAppendingPathComponent:bundleName] URLByStandardizingPath];
        if ([fileManager fileExistsAtPath:sourceURL.path]) {
            NSDictionary *attributes = nil;
            NSError *error = nil;

            // There's definitely a usermode bundle present.
            BOOL needsStaging = NO;

            // Ensure URL's are still prefixed by their expected base to prevent any ../-style filesystem escapes.
            if (![sourceURL.path hasPrefix:sourceBaseURL.path]) {
                OSKextLogCFString(NULL,
                                  kOSKextLogErrorLevel | kOSKextLogLoadFlag,
                                  CFSTR("Source URL is no longer contained in expected directory: %@"),
                                  sourceURL.path);
                continue;
            } else if (![targetURL.path hasPrefix:targetBaseURL.path]) {
                OSKextLogCFString(NULL,
                                  kOSKextLogErrorLevel | kOSKextLogLoadFlag,
                                  CFSTR("Target URL is no longer contained in expected directory: %@"),
                                  targetURL.path);
                continue;
            }

            // Validate the sourceURL as a directory and ensure it's not a symlink.
            attributes = [fileManager attributesOfItemAtPath:sourceURL.path error:&error];
            if (!attributes || error) {
                // Error, do not stage this if we can't check its attributes.
                OSKextLogCFString(NULL,
                                  kOSKextLogErrorLevel | kOSKextLogLoadFlag,
                                  CFSTR("Failed to get attributes of bundle source (%@): %@"),
                                  sourceURL.path, error);
                continue;
            }

            if (![NSFileTypeDirectory isEqualToString:attributes[NSFileType]]) {
                // If this isn't a directory, log the error and skip it.
                OSKextLogCFString(NULL,
                                  kOSKextLogErrorLevel | kOSKextLogLoadFlag,
                                  CFSTR("Bundle is not a directory (type = %@), skipping %@"),
                                  attributes[NSFileType], bundleName);
                continue;
            }

            needsStaging = bundleNeedsStaging(sourceURL, targetURL);
            if (needsStaging) {
                BOOL shouldContinue = callbackHandler(sourceURL, targetURL);
                if (!shouldContinue) {
                    break;
                }
            }
        }
    }
}

static void
forEachInsecureBundle(NSArray *bundles, BundleURLHandler callbackHandler)
{
    NSURL *sourceBaseURL = [NSURL fileURLWithPath:@INSECURE_GRAPHICS_BUNDLE_PATH];
    NSURL *targetBaseURL = [NSURL fileURLWithPath:@SECURE_GRAPHICS_BUNDLE_STAGING_PATH];

    forEachInsecureBundleHelper(bundles, callbackHandler, sourceBaseURL, targetBaseURL);
}

static Boolean
arrayHasInsecureBundles(NSArray *bundles) {
    __block BOOL insecure = NO;
    forEachInsecureBundle(bundles, ^ BOOL (NSURL * __unused sourceURL, NSURL * __unused targetURL) {
        insecure = YES;
        return NO;
    });
    return insecure;
}

NSURL *
createStagingURL(NSURL *sourceURL)
{
    static NSArray *stagingURLComponents = nil;

    if (!stagingURLComponents) {
        stagingURLComponents = [NSURL fileURLWithPath:@SECURE_KEXT_STAGING_PATH isDirectory:YES].pathComponents;
    }

    // To create the full URL, we start with the staging URL, add in all the path components of the source URL
    // without the initial /, then add add a trailing / to ensure it's a directory URL.
    NSMutableArray *pathComponents = [NSMutableArray arrayWithArray:stagingURLComponents];
    [pathComponents addObjectsFromArray:[sourceURL.pathComponents subarrayWithRange:NSMakeRange(1, sourceURL.pathComponents.count - 1)]];
    [pathComponents addObject:@"/"];

    // NOTE: The output URL MUST be a directory and contain a trailing /
    // IOKit function calls rely on this or the OSKext object doesn't fill out properly.
    return [NSURL fileURLWithPathComponents:pathComponents];
}

static OSKextRef
createRefreshedKext(NSURL *url)
{
    CFURLRef kextURL = (__bridge CFURLRef)url;
    OSKextRef theKext = NULL;
    CFArrayRef kexts = NULL;

    // Create all kexts from the URL so IOKit can create objects for the
    // kext itself and any bundled plug-ins.  This guarantees there are
    // new OSKext objects for this kext URL and all plug-ins for use in future
    // API calls.
    kexts = OSKextCreateKextsFromURL(kCFAllocatorDefault, kextURL);
    if (!kexts) {
        OSKextLogCFString(NULL,
                          kOSKextLogErrorLevel | kOSKextLogLoadFlag,
                          CFSTR("Could not create all kexts from url: %@"),
                          url.path);
        goto finish;
    }

    // Create an OSKext object for this kext URL directly so we can return it
    // for future use by the caller.  Internally, this will just be an already
    // created object from the list above, but this saves us the trouble of
    // walking that list to find it.
    theKext = OSKextCreate(kCFAllocatorDefault, kextURL);
    if (!theKext) {
        OSKextLogCFString(NULL,
                          kOSKextLogErrorLevel | kOSKextLogLoadFlag,
                          CFSTR("Could not create refreshed kext at url: %@"),
                          url.path);
        goto finish;
    }

finish:
    SAFE_RELEASE(kexts);
    return theKext;
}

#pragma mark External Functions
OSKextRef
createStagedKext(OSKextRef theKext)
{
    OSKextRef finalKext = NULL;

    if (!stagingEnabled()) {
        finalKext = (OSKextRef)CFRetain(theKext);
        goto finish;
    }

    if (kextRequiresStaging(theKext)) {
        NSURL *sourceURL = (__bridge NSURL *)OSKextGetURL(theKext);
        NSURL *destinationURL = createStagingURL(sourceURL);

        // Even if the source kext "requires staging", the bundle itself may have already
        // been staged properly (from a previous load, etc) and not need any more work.
        if (bundleNeedsStaging(sourceURL, destinationURL)) {
            if (!stageBundle(sourceURL, destinationURL, @SECURE_KEXT_VALIDATION_ROOT, NO)) {
                OSKextLog(NULL,
                          kOSKextLogErrorLevel | kOSKextLogArchiveFlag |
                          kOSKextLogValidationFlag | kOSKextLogGeneralFlag,
                          "Unable to stage kext (%s) to secure location.",
                          sourceURL.path.UTF8String);
                finalKext = NULL;
                goto finish;
            }
        }

        // Regardless of whether the copy happened now or previously, we still need to update
        // the OSKext object to be based on the staged URL.
        finalKext = createRefreshedKext(destinationURL);
    } else {
        // The incoming kext is already secure, so just retain it for the output.
        finalKext = (OSKextRef)CFRetain(theKext);
    }

finish:
    return finalKext;
}

Boolean
stagingEnabled(void)
{
    return !isSIPDisabled();
}

Boolean
kextIsInSecureLocation(OSKextRef theKext)
{
    NSURL *url = (__bridge NSURL *)OSKextGetURL(theKext);
    if (!url) {
        return false;
    }
    return pathIsSecure(url.path);
}

Boolean
kextRequiresStaging(OSKextRef theKext)
{
    return !kextIsInSecureLocation(theKext);
}

Boolean
needsGPUBundlesStaged(OSKextRef theKext)
{
    CFTypeRef companionBundlesValue = NULL;
    CFTypeRef optionalBundlesValue = NULL;
    Boolean hasInsecureBundles = false;

    if (!stagingEnabled()) {
        return false;
    }

    companionBundlesValue = OSKextGetValueForInfoDictionaryKey(theKext, kCompanionBundlesKey);
    optionalBundlesValue = OSKextGetValueForInfoDictionaryKey(theKext, kOptionalCompanionBundlesKey);

    // If neither key exists, it definitely doesn't need any staging.
    if (!companionBundlesValue && !optionalBundlesValue) {
        return false;
    }

    // If either key is malformed, we'll print a warning and not copy anything.
    if ((CFGetTypeID(companionBundlesValue) != CFArrayGetTypeID()) ||
        (CFGetTypeID(companionBundlesValue) != CFArrayGetTypeID())) {
        OSKextLogCFString(NULL,
                          kOSKextLogErrorLevel | kOSKextLogLoadFlag,
                          CFSTR("GPU bundle keys malformed - wrong type."));
        return false;
    }

    // If either array has any insecure bundles, then the kext needs something staged.
    hasInsecureBundles = arrayHasInsecureBundles((__bridge NSArray*)companionBundlesValue);
    hasInsecureBundles = hasInsecureBundles || arrayHasInsecureBundles((__bridge NSArray*)optionalBundlesValue);
    return hasInsecureBundles;
}

Boolean
stageGPUBundles(OSKextRef theKext)
{
    NSArray<NSString *> *companionBundles = nil;
    NSArray<NSString *> *optionalBundles = nil;
    BundleURLHandler handler = NULL;
    __block Boolean success = true;

    if (!stagingEnabled()) {
        return false;
    }

    handler = ^ BOOL (NSURL *sourceURL, NSURL *targetURL) {
        OSKextLogCFString(NULL,
                          kOSKextLogBasicLevel | kOSKextLogLoadFlag,
                          CFSTR("Staging insecure bundle: %@"),
                          sourceURL.path);
        if (!stageBundle(sourceURL, targetURL, @SECURE_KEXT_VALIDATION_ROOT, YES)) {
            OSKextLogCFString(NULL,
                              kOSKextLogBasicLevel | kOSKextLogLoadFlag,
                              CFSTR("Staging failed for bundle: %@"),
                              sourceURL.path);
            success = false;
        }
        return YES;
    };

    companionBundles = (__bridge NSArray*)OSKextGetValueForInfoDictionaryKey(theKext, kCompanionBundlesKey);
    optionalBundles = (__bridge NSArray*)OSKextGetValueForInfoDictionaryKey(theKext, kOptionalCompanionBundlesKey);

    forEachInsecureBundle(companionBundles, handler);
    forEachInsecureBundle(optionalBundles, handler);

    return success;
}

CFArrayRef
createStagedKextsFromURLs(CFArrayRef kextURLs, Boolean includeUnstaged)
{
    CFArrayRef unstagedKexts = NULL;
    CFMutableArrayRef finalKexts = NULL;
    CFIndex count = 0;

    if (!kextURLs) {
        goto finish;
    }

    if (!createCFMutableArray(&finalKexts, &kCFTypeArrayCallBacks)) {
        OSKextLogMemError();
        goto finish;
    }

    unstagedKexts = OSKextCreateKextsFromURLs(NULL, kextURLs);
    if (!unstagedKexts) {
        OSKextLogCFString(NULL,
                          kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                          CFSTR("Unable to read extensions from URLs: %@"),
                          kextURLs);
        goto finish;
    }

    count = CFArrayGetCount(unstagedKexts);
    for(CFIndex i = 0; i < count; i++) {
        OSKextRef unstagedKext = (OSKextRef)CFArrayGetValueAtIndex(unstagedKexts, i);
        OSKextRef stagedKext = createStagedKext(unstagedKext);
        if (!stagedKext) {
            continue;
        }

        CFArrayAppendValue(finalKexts, stagedKext);
        if (includeUnstaged && (stagedKext != unstagedKext)) {
            CFArrayAppendValue(finalKexts, unstagedKext);
        }
        SAFE_RELEASE(stagedKext);
    }

finish:
    SAFE_RELEASE(unstagedKexts);
    return finalKexts;
}

NSURL *
createURLWithoutPrefix(NSURL *url, NSString *prefix)
{
    if (![url.path hasPrefix:prefix]) {
        return url;
    }

    NSMutableArray *components = [NSMutableArray arrayWithObject:@"/"];
    NSRange prefixRange = NSMakeRange(prefix.pathComponents.count,
                                      url.pathComponents.count - prefix.pathComponents.count);
    NSArray *sourceComponents = [url.pathComponents subarrayWithRange:prefixRange];
    [components addObjectsFromArray:sourceComponents];
    return [NSURL fileURLWithPathComponents:components];
}

Boolean
pruneStagingDirectoryHelper(NSString *stagingRoot)
{
    Boolean success = true;
    NSError *error = nil;
    NSFileManager *fm = [NSFileManager defaultManager];
    NSDirectoryEnumerator *enumerator = nil;
    NSArray<NSURLResourceKey> *keys = @[NSURLIsDirectoryKey];
    NSMutableArray *kextsToCheck = [NSMutableArray array];

    NSURL *stagingBaseURL = [NSURL fileURLWithPath:stagingRoot isDirectory:YES];

    enumerator = [fm enumeratorAtURL:stagingBaseURL
          includingPropertiesForKeys:keys
                             options:(NSDirectoryEnumerationOptions)0
                        errorHandler:^(NSURL *url, NSError *error) {
                            OSKextLogCFString(NULL,
                                              kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                                              CFSTR("Error pruning staging area: %@, %@"),
                                              url.path, error);
                            return YES;
                        }];

    for (NSURL *url in enumerator) {
        NSNumber *isDirectory = nil;

        if ([url getResourceValue:&isDirectory forKey:NSURLIsDirectoryKey error:nil]) {
            if (isDirectory && isDirectory.boolValue) {
                if ([url.lastPathComponent.pathExtension isEqualToString:@"kext"]) {
                    [kextsToCheck addObject:url];
                    [enumerator skipDescendants];
                }
            }
        }
    }

    for (NSURL *stagedURL in kextsToCheck) {
        NSURL *parentURL = nil;
        NSURL *unstagedURL = nil;

        // Check if it still exists on the filesystem.
        unstagedURL = createURLWithoutPrefix(stagedURL, stagingRoot);
        if ([fm fileExistsAtPath:unstagedURL.path]) {
            continue;
        }

        // If not, remove it.
        OSKextLogCFString(NULL,
                          kOSKextLogDetailLevel | kOSKextLogGeneralFlag,
                          CFSTR("Pruning deleted kernel extension: %@"),
                          unstagedURL.path);
        if (![fm removeItemAtURL:stagedURL error:&error]) {
            OSKextLogCFString(NULL,
                              kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                              CFSTR("Unable to delete kernel extension: %@, %@"),
                              stagedURL.path, error);
            success = false;
            continue;
        }

        // Then walk up directories to see if we need to remove empty parents.
        parentURL = [stagedURL URLByDeletingLastPathComponent];
        if (parentURL == nil) {
            OSKextLogCFString(NULL,
                              kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                              CFSTR("Unable to prune because URL parent cannot be found: %@"),
                              stagedURL.path);
            success = false;
            continue;
        }

        // While walking up, make sure we're within the staging area and break when we
        // hit the top level staging directory itself.
        while ([parentURL.path hasPrefix:stagingRoot] &&
               ![parentURL.path isEqualToString:stagingRoot]) {
            NSArray *contents = [fm contentsOfDirectoryAtPath:parentURL.path error:&error];

            // If parent is now empty, remove it...otherwise, move on to the next kext.
            if (contents.count == 0) {
                if (![fm removeItemAtURL:parentURL error:&error]) {
                    OSKextLogCFString(NULL,
                                      kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                                      CFSTR("Unable to delete empty staging directory: %@, %@"),
                                      parentURL.path, error);
                    success = false;
                    break;
                }

                // Continue walking up to the parent directory.
                NSURL *nextURL = [parentURL URLByDeletingLastPathComponent];
                if (nextURL == nil || [nextURL isEqualTo:parentURL]) {
                    OSKextLogCFString(NULL,
                                      kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                                      CFSTR("Unable to prune because URL parent cannot be found: %@"),
                                      stagedURL.path);
                    success = false;
                    break;
                }
                parentURL = nextURL;
            } else {
                // First non-empty directory, so we're done.
                break;
            }
        }
    }

    return success;
}

Boolean
pruneStagingDirectory(void)
{
    return pruneStagingDirectoryHelper(@SECURE_KEXT_STAGING_PATH);
}

Boolean
clearStagingDirectoryHelper(NSString *stagingRoot)
{
    NSFileManager *fm = [NSFileManager defaultManager];
    NSURL *baseURL = [NSURL fileURLWithPath:stagingRoot isDirectory:YES];

    // Enumerate all directories in the top-level staging directory and remove them.
    NSArray *contents = [fm contentsOfDirectoryAtPath:stagingRoot error:nil];
    for (NSString *itemName in contents) {
        NSError *error = nil;
        NSURL *itemURL = [baseURL URLByAppendingPathComponent:itemName];

        if (![fm removeItemAtURL:itemURL error:&error]) {
            OSKextLogCFString(NULL,
                              kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                              CFSTR("Unable to delete extension: %@, %@"),
                              itemURL.path, error);
        }
    }
    return true;
}

Boolean
clearStagingDirectory(void)
{
    // Clear temporary staging AND clear the entire secure staging area.
    return clearStagingDirectoryHelper(@SECURE_KEXT_VALIDATION_ROOT) &&
           clearStagingDirectoryHelper(@SECURE_KEXT_STAGING_PATH);
}

CFURLRef
copyUnstagedKextURL(CFURLRef kextURL)
{
    NSURL *url = (__bridge NSURL *)kextURL;
    if (!pathIsSecure(url.path)) {
        // if the kext is unstaged, just return the URL
        return CFBridgingRetain(url);
    }

    NSURL *unstagedURL;
    // Check if it still exists on the filesystem.
    unstagedURL = createURLWithoutPrefix(url, @SECURE_KEXT_STAGING_PATH);
    return CFBridgingRetain(unstagedURL);
}
