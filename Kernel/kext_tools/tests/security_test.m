/*
 *  security_test.m
 *  kext_tools
 *
 *  Copyright 2017 Apple Inc. All rights reserved.
 *
 */
#import <Foundation/Foundation.h>
#import <copyfile.h>

#import "unit_test.h"
#import "security.h"
#import "staging.h"

#pragma mark External Function Declarations
extern Boolean pathIsSecure(NSString *path);
extern Boolean bundleValidates(NSURL *bundleURL, BOOL isGPUBundle);
extern Boolean stageBundle(NSURL *sourceURL, NSURL *destinationURL, NSString *validationRoot, BOOL isGPUBundle);
extern NSData *copyIdentifierFromBundle(NSURL *url);
typedef BOOL (^BundleURLHandler)(NSURL *, NSURL *);
extern void forEachInsecureBundleHelper(NSArray *bundles, BundleURLHandler callbackHandler, NSURL *sourceBaseURL, NSURL *targetBaseURL);
extern NSURL *createStagingURL(NSURL *originalURL);
extern BOOL bundleNeedsStaging(NSURL *sourceURL, NSURL *destinationURL);
extern NSURL *createURLWithoutPrefix(NSURL *url, NSString *prefix);
extern Boolean pruneStagingDirectoryHelper(NSString *stagingRoot);
extern Boolean clearStagingDirectoryHelper(NSString *stagingRoot);

/* It's unfortunate that this is required, but --remove-signature is flaky and its error output
 * isn't reflective of the failures, so the simplest thing to do is call it a few times.
 * For more information, see <rdar://problem/36603724>.
 */
static void
remove_signature(const char *path)
{
    const static int CALL_COUNT = 5;
    NSString *command = [NSString stringWithFormat:@"codesign --remove-signature %s", path];
    int calls = 0;

    while (calls < CALL_COUNT) {
        system(command.UTF8String);
        calls += 1;
    }
}

#pragma mark Test Functions
static void
test_path_secure()
{
    TEST_START("path security");

    TEST_CASE("/S/L/E apple driver is secure", pathIsSecure(@"/System/Library/Extensions/AppleHV.kext")== true);
    TEST_CASE("/L/E third party kext is not secure", pathIsSecure(@"/Library/Extensions/PromiseSTEX.kext") == false);
    TEST_CASE("staged extension directory is secure", pathIsSecure(@"/Library/StagedExtensions") == true);
    TEST_CASE("staged gpu bundle directory is secure", pathIsSecure(@"/Library/GPUBundles") == true);
}

static void
test_bundle_validation()
{
    NSURL *bundleURL = nil;

    TEST_START("bundle validation");

    bundleURL = [NSURL fileURLWithPath:@"/System/Library/Extensions/AppleHV.kext"];
    TEST_CASE("apple kext validates as bundle", bundleValidates(bundleURL, YES) == true);
    TEST_CASE("apple kext validates as kext", bundleValidates(bundleURL, NO) == true);

    bundleURL = [NSURL fileURLWithPath:@"/Library/Extensions/PromiseSTEX.kext"];
    TEST_CASE("third party kext does not validate as gpu bundle", bundleValidates(bundleURL, YES) == false);
    TEST_CASE("third party kext validates as kext", bundleValidates(bundleURL, NO) == true);
}

static void
test_staging_function()
{
    Boolean success = false;
    NSURL *sourceURL = nil;
    NSURL *targetURL = nil;
    NSArray<NSURL *> *contents = nil;
    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSURL *tmpDir = [NSURL fileURLWithPath:@"/tmp/kext_security_test"];

    TEST_START("staging functionality");

    // Setup temporary directory.
    [fileManager removeItemAtURL:tmpDir error:nil];
    [fileManager createDirectoryAtURL:tmpDir withIntermediateDirectories:NO attributes:nil error:nil];

    // Stage an apple kext, which will validate properly and end up in the destination.
    // Copy the kext into an insecure location first so we strip the SIP xattrs, which would fail
    // to stage since the staging copy can't put the extended attributes on in the staged location.
    sourceURL = [NSURL fileURLWithPath:@"/System/Library/Extensions/AppleHV.kext"];
    targetURL = [NSURL fileURLWithPath:@"/tmp/AppleHV.kext"];
    [fileManager copyItemAtURL:sourceURL
                         toURL:targetURL
                         error:nil];
    sourceURL = targetURL;
    targetURL = [tmpDir URLByAppendingPathComponent:@"SecureCopy.kext"];
	success = stageBundle(sourceURL, targetURL, tmpDir.path, YES);
    contents = [fileManager contentsOfDirectoryAtURL:tmpDir includingPropertiesForKeys:nil options:0 error:nil];

    TEST_CASE("SETUP: apple kext staging", [fileManager fileExistsAtPath:sourceURL.path]);
    TEST_CASE("apple kext stages", success == true);
    TEST_CASE("apple kext staging ends up in right location", [fileManager fileExistsAtPath:targetURL.path]);
    TEST_CASE("apple kext staging removes temporary artifacts", contents.count == 1);

    [fileManager removeItemAtURL:sourceURL error:nil];
    [fileManager removeItemAtURL:targetURL error:nil];

    // Try staging a third party kext, which should fail and leave no trace in the output directory.
    sourceURL = [NSURL fileURLWithPath:@"/Library/Extensions/PromiseSTEX.kext"];
    targetURL = [tmpDir URLByAppendingPathComponent:@"DoesntExist.kext"];
    success = stageBundle(sourceURL, targetURL, tmpDir.path, YES);
    contents = [fileManager contentsOfDirectoryAtURL:tmpDir includingPropertiesForKeys:nil options:0 error:nil];

    TEST_CASE("third party kext fails staging as a bundle", success == false);
    TEST_CASE("third party kext staging as a bundle removes all artifacts", contents.count == 0);

    // Try staging a third party kext as a kext, which should succeed.
    sourceURL = [NSURL fileURLWithPath:@"/Library/Extensions/PromiseSTEX.kext"];
    targetURL = [tmpDir URLByAppendingPathComponent:@"PromiseCopy.kext"];
    success = stageBundle(sourceURL, targetURL, tmpDir.path, NO);
    contents = [fileManager contentsOfDirectoryAtURL:tmpDir includingPropertiesForKeys:nil options:0 error:nil];

    TEST_CASE("third party kext stages", success == true);
    TEST_CASE("third party kext staging ends up in right location", [fileManager fileExistsAtPath:targetURL.path]);
    TEST_CASE("third party kext staging removes temporary artifacts", contents.count == 1);

    // Cleanup temporary directory.
    [fileManager removeItemAtURL:tmpDir error:nil];
}

static void
test_system_doesnt_need_staging()
{
    NSArray<NSURL *> *contents = nil;
    NSFileManager *fileManager = [NSFileManager defaultManager];

    TEST_START("/S/L/E doesn't contain kexts that need any staging");

    // Validate nothing in /S/L/E actually needs any staging.
    contents = [fileManager contentsOfDirectoryAtURL:[NSURL fileURLWithPath:@"/System/Library/Extensions"]
                          includingPropertiesForKeys:nil
                                             options:0
                                               error:nil];
    for (NSURL *kextURL in contents) {
        OSKextRef stagedKext = NULL;
        NSString *testName = nil;
        OSKextRef kext = OSKextCreate(NULL, (__bridge CFURLRef)kextURL);
        if (!kext) {
            // Skip non-kext objects.
            continue;
        }

        testName = [NSString stringWithFormat:@"%@ doesn't need GPU bundles staged", kextURL.path];
        TEST_CASE(testName.UTF8String, needsGPUBundlesStaged(kext) == false);

        testName = [NSString stringWithFormat:@"%@ doesn't need kext staging", kextURL.path];
        stagedKext = createStagedKext(kext);
        TEST_CASE(testName.UTF8String, stagedKext && (stagedKext == kext));
    }
}

static void
test_insecure_identifier()
{
    NSURL *testRootURL = [NSURL fileURLWithPath:@"/private/tmp/kexttest"];
    NSURL *sourceURL = nil;
    NSData *cdhash = nil;
    NSFileManager *fileManager = [NSFileManager defaultManager];

    TEST_START("insecure identifier generation");

    // Cleanup in case a previous test has failed to cleanup.
    [fileManager removeItemAtURL:testRootURL error:nil];
    [fileManager createDirectoryAtPath:testRootURL.path
           withIntermediateDirectories:YES
                            attributes:nil
                                 error:nil];

    sourceURL = [NSURL fileURLWithPath:@"/Library/Extensions/SoftRAID.kext"];
    cdhash = copyIdentifierFromBundle(sourceURL);
    TEST_CASE("valid signature looks like cdhash", cdhash && cdhash.length == 20);

    // Make an unsigned bundle by copying and removing the signature from a signed bundle.
    NSURL *testKextURL = [testRootURL URLByAppendingPathComponent:@"test.kext"];
    [fileManager copyItemAtURL:sourceURL
                         toURL:testKextURL
                         error:nil];
    remove_signature(testKextURL.path.UTF8String);
    cdhash = copyIdentifierFromBundle(testKextURL);
    TEST_CASE("unsigned bundle returns adhoc cdhash as string", cdhash.length == 40);

    sourceURL = [NSURL fileURLWithPath:@"/System/Library/LaunchDaemons/com.apple.kextd.plist"];
    cdhash = copyIdentifierFromBundle(sourceURL);
    TEST_CASE("flat file returns adhoc cdhash as string", cdhash.length == 40);

    [fileManager removeItemAtURL:testRootURL error:nil];
}

static void
test_for_each_insecure_bundle()
{
    __block int callCount = 0;
    BundleURLHandler handler = NULL;
    NSArray<NSString *> *bundles = nil;
    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSURL *sleURL = [NSURL fileURLWithPath:@"/System/Library/Extensions"];
    NSURL *tempSourceURL = [NSURL fileURLWithPath:@"/tmp/kext_source"];
    NSURL *tempTargetURL = [NSURL fileURLWithPath:@"/tmp/kext_target"];
    NSURL *url = nil;

    handler = ^ BOOL (NSURL *sourceURL, NSURL *targetURL) {
        callCount += 1;
        return YES;
    };

    TEST_START("foreach insecure bundle");

    [fileManager removeItemAtURL:tempSourceURL error:nil];
    [fileManager removeItemAtURL:tempTargetURL error:nil];
    [fileManager createDirectoryAtURL:tempSourceURL withIntermediateDirectories:NO attributes:nil error:nil];
    [fileManager createDirectoryAtURL:tempTargetURL withIntermediateDirectories:NO attributes:nil error:nil];

    callCount = 0;
    bundles = @[@"../"];
    forEachInsecureBundleHelper(bundles, handler, sleURL, tempTargetURL);
    TEST_CASE("../ in bundle name results in no callbacks", callCount == 0);

    callCount = 0;
    bundles = @[@"symlink"];
    url = [tempSourceURL URLByAppendingPathComponent:@"symlink"];
    [fileManager createSymbolicLinkAtURL:url withDestinationURL:sleURL error:nil];
    forEachInsecureBundleHelper(bundles, handler, tempSourceURL, tempTargetURL);
    TEST_CASE("symlink source results in no callbacks", callCount == 0);

    callCount = 0;
    bundles = @[@"AppleHV.kext"];
    forEachInsecureBundleHelper(bundles, handler, sleURL, tempTargetURL);
    TEST_CASE("secure source results in no callbacks", callCount == 0);

    callCount = 0;
    bundles = @[@"AppleHV.kext"];
    url = [tempSourceURL URLByAppendingPathComponent:@"AppleHV.kext"];
    copyfile("/System/Library/Extensions/AppleHV.kext", url.path.UTF8String, NULL, COPYFILE_STAT | COPYFILE_DATA | COPYFILE_RECURSIVE);
    forEachInsecureBundleHelper(bundles, handler, tempSourceURL, tempTargetURL);
    TEST_CASE("insecure bundle results in callback", callCount == 1);

    callCount = 0;
    url = [tempTargetURL URLByAppendingPathComponent:@"AppleHV.kext"];
    copyfile("/System/Library/Extensions/AppleHV.kext", url.path.UTF8String, NULL, COPYFILE_STAT | COPYFILE_DATA | COPYFILE_RECURSIVE);
    forEachInsecureBundleHelper(bundles, handler, tempSourceURL, tempTargetURL);
    TEST_CASE("target exists and has no differences results in no callbacks", callCount == 0);

    callCount = 0;
    [fileManager removeItemAtURL:url error:nil];
    copyfile("/System/Library/Extensions/apfs.kext", url.path.UTF8String, NULL, COPYFILE_STAT | COPYFILE_DATA | COPYFILE_RECURSIVE);
    forEachInsecureBundleHelper(bundles, handler, tempSourceURL, tempTargetURL);
    TEST_CASE("target exists and is different results in a callback", callCount == 1);

    [fileManager removeItemAtURL:tempSourceURL error:nil];
    [fileManager removeItemAtURL:tempTargetURL error:nil];
}

static void
test_custom_authentication(void)
{
    OSKextRef kextRef = NULL;
    int result = 0;
    NSFileManager *fm = [NSFileManager defaultManager];
    AuthOptions_t testOptions = {0};
    NSURL *kextURL = nil;

    testOptions.allowNetwork = true;
    testOptions.isCacheLoad = true;
    testOptions.performFilesystemValidation = true;
    testOptions.performSignatureValidation = true;
    testOptions.requireSecureLocation = true;
    testOptions.respectSystemPolicy = true;

    TEST_START("custom authentication method testing");

    kextURL = [NSURL fileURLWithPath:@"/System/Library/Extensions/AppleHV.kext"];
    kextRef = OSKextCreate(NULL, (__bridge CFURLRef)kextURL);
    TEST_CASE("system kext authenticates as cache load", authenticateKext(kextRef, &testOptions));

    testOptions.isCacheLoad = false;
    testOptions.allowNetwork = false;
    TEST_CASE("system kext authenticates without network as runtime load", authenticateKext(kextRef, &testOptions));

    result = copyfile("/System/Library/Extensions/AppleHV.kext", "/tmp/AppleHV.kext", NULL, COPYFILE_STAT | COPYFILE_DATA | COPYFILE_RECURSIVE);
    kextURL = [NSURL fileURLWithPath:@"/tmp/AppleHV.kext"];
    kextRef = OSKextCreate(NULL, (__bridge CFURLRef)kextURL);
    TEST_CASE("SETUP: copied system kext properly", result == 0 && kextRef != NULL);
    TEST_CASE("copied system kext doesn't authenticate", authenticateKext(kextRef, &testOptions) == false);

    testOptions.performFilesystemValidation = false;
    testOptions.requireSecureLocation = false;
    TEST_CASE("copied system kext authenticates without filesystem / location checks", authenticateKext(kextRef, &testOptions));

    system("touch /tmp/AppleHV.kext/Contents/BadFile");
    TEST_CASE("SETUP: succesfully modified kext", [fm fileExistsAtPath:@"/tmp/AppleHV.kext/Contents/BadFile"]);
    TEST_CASE("modified kext authenticates due to Apple Internal policy", authenticateKext(kextRef, &testOptions));

    testOptions.performSignatureValidation = false;
    TEST_CASE("modified kext authenticates without signature validation", authenticateKext(kextRef, &testOptions));

    system("rm -rf /tmp/AppleHV.kext");
}

static void
test_kext_staging_helpers()
{
    int result = 0;
    OSKextRef kext = NULL;
    NSURL *sourceURL = nil;
    NSURL *destinationURL = nil;

    TEST_START("staging helpers");

    // createStagingURL tests
    sourceURL = [NSURL fileURLWithPath:@"/Library/Extensions/ArcMSR.kext"];
    destinationURL = createStagingURL(sourceURL);
    TEST_CASE("creates proper staging url location", [destinationURL.path isEqualToString:@"/Library/StagedExtensions/Library/Extensions/ArcMSR.kext"]);
    TEST_CASE("staging url has a trailing /", [destinationURL.absoluteString hasSuffix:@"/"]);

    // bundleNeedsStaging tests
    sourceURL = [NSURL fileURLWithPath:@"/Library/Extensions/ArcMSR.kext"];
    destinationURL = [NSURL fileURLWithPath:@"/tmp/ArcMSR.kext"];
    TEST_CASE("non-SIP protected URL needs staging", bundleNeedsStaging(sourceURL, destinationURL) == YES);

    sourceURL = [NSURL fileURLWithPath:@"/Library/Extensions/ArcMSR.kext"];
    destinationURL = [NSURL fileURLWithPath:@"/tmp/ArcMSR.kext"];
    result = copyfile(sourceURL.path.UTF8String, destinationURL.path.UTF8String, NULL, COPYFILE_STAT | COPYFILE_DATA | COPYFILE_RECURSIVE);
    TEST_CASE("SETUP: copied ArcMSR kext properly", result == 0);
    TEST_CASE("bundle doesn't need staging if it was already staged", bundleNeedsStaging(sourceURL, destinationURL) == NO);
    system("rm -rf /tmp/ArcMSR.kext");

    sourceURL = [NSURL fileURLWithPath:@"/System/Library/Extensions/AppleHV.kext"];
    destinationURL = [NSURL fileURLWithPath:@"/Library/StagedExtensions/AppleHV.kext"];
    TEST_CASE("SIP protected URL doesn't need staging", bundleNeedsStaging(sourceURL, destinationURL) == NO);

    // kextRequiresStaging tests
    sourceURL = [NSURL fileURLWithPath:@"/System/Library/Extensions/AppleHV.kext"];
    kext = OSKextCreate(NULL, (__bridge CFURLRef)sourceURL);
    TEST_CASE("SIP protected kext doesn't need staging", kextRequiresStaging(kext) == false);

    sourceURL = [NSURL fileURLWithPath:@"/Library/Extensions/ArcMSR.kext"];
    kext = OSKextCreate(NULL, (__bridge CFURLRef)sourceURL);
    TEST_CASE("non-SIP protected kext needs staging", kextRequiresStaging(kext) == true);

    // createURLWithoutPrefix helper function tests.
    sourceURL = [NSURL fileURLWithPath:@"/Library/Extensions/ArcMSR.kext"];
    destinationURL = createURLWithoutPrefix(sourceURL, @"/Library/StagedExtensions");
    TEST_CASE("createURLWithoutPrefix returns original url if not prefixed", destinationURL == sourceURL);

    destinationURL = createURLWithoutPrefix(sourceURL, @"/Library");
    TEST_CASE("createURLWithoutPrefix can remove 1 component prefix", [destinationURL.path isEqualToString:@"/Extensions/ArcMSR.kext"]);

    destinationURL = createURLWithoutPrefix(sourceURL, @"/Library/Extensions");
    TEST_CASE("createURLWithoutPrefix can remove 2 component prefix", [destinationURL.path isEqualToString:@"/ArcMSR.kext"]);
}

static void
test_staging_management_helpers(void)
{
    NSFileManager *fm = [NSFileManager defaultManager];
    NSURL *testRootURL = [NSURL fileURLWithPath:@"/private/tmp/stagingtest"];
    NSArray *dirContents = nil;
    NSError *error = nil;

    TEST_START("staging management functions");

    // First, ensure clear can delete multiple items in the top level directory.
    [fm removeItemAtURL:testRootURL error:nil];
    [fm createDirectoryAtURL:testRootURL withIntermediateDirectories:YES attributes:nil error:nil];
    [fm createDirectoryAtURL:[testRootURL URLByAppendingPathComponent:@"One"] withIntermediateDirectories:YES attributes:nil error:nil];
    [fm createDirectoryAtURL:[testRootURL URLByAppendingPathComponent:@"Two"] withIntermediateDirectories:YES attributes:nil error:nil];

    clearStagingDirectoryHelper(testRootURL.path);
    dirContents = [fm contentsOfDirectoryAtURL:testRootURL includingPropertiesForKeys:nil options:0 error:&error];
    TEST_CASE("staging directory exists after clear", error == nil);
    TEST_CASE("staging directory is empty after clear", dirContents.count == 0);

	// Create a pruning scenario where all kexts will be removed.
	[fm createDirectoryAtURL:[testRootURL URLByAppendingPathComponent:@"Library/Extensions/Pineapple.kext"] withIntermediateDirectories:YES attributes:nil error:nil];
	[fm createDirectoryAtURL:[testRootURL URLByAppendingPathComponent:@"Library/Extensions/Guava.kext"] withIntermediateDirectories:YES attributes:nil error:nil];
	[fm createDirectoryAtURL:[testRootURL URLByAppendingPathComponent:@"Applications/Company/Mango.kext"] withIntermediateDirectories:YES attributes:nil error:nil];

	pruneStagingDirectoryHelper(testRootURL.path);
	dirContents = [fm contentsOfDirectoryAtPath:testRootURL.path error:&error];
	TEST_CASE("staging directory exists after pruning all kexts", [fm fileExistsAtPath:testRootURL.path]);
	TEST_CASE("staging directory is empty after pruning all kexts", error == nil && dirContents.count == 0);

    // Create a basic pruning scenario:
    //   1. a kext that exists outside staging, which should persist
    //   2. a kext in the same directory as 1 that doesn't exist, to test cleanup and ensure the parent isn't removed
    //   3. a kext in a completely different directory that requires parent traversal for full path cleanup
    [fm createDirectoryAtURL:[testRootURL URLByAppendingPathComponent:@"Library/Extensions/ArcMSR.kext"] withIntermediateDirectories:YES attributes:nil error:nil];
    [fm createDirectoryAtURL:[testRootURL URLByAppendingPathComponent:@"Library/Extensions/Pineapple.kext"] withIntermediateDirectories:YES attributes:nil error:nil];
    [fm createDirectoryAtURL:[testRootURL URLByAppendingPathComponent:@"Applications/Company/Mango.kext"] withIntermediateDirectories:YES attributes:nil error:nil];

    pruneStagingDirectoryHelper(testRootURL.path);
    dirContents = [fm contentsOfDirectoryAtPath:testRootURL.path error:&error];
    TEST_CASE("staging directory exists after pruning", [fm fileExistsAtPath:testRootURL.path]);
    TEST_CASE("staging directory has only one directory after pruning", error == nil && dirContents.count == 1);
    TEST_CASE("existing kext directory still exists in staging area", [fm fileExistsAtPath:[testRootURL URLByAppendingPathComponent:@"Library/Extensions/ArcMSR.kext"].path]);
    TEST_CASE("non-existent kexts were deleted",
              ![fm fileExistsAtPath:[testRootURL URLByAppendingPathComponent:@"Library/Extensions/Pineapple.kext"].path] &&
              ![fm fileExistsAtPath:[testRootURL URLByAppendingPathComponent:@"Applications/Company/Mango.kext"].path]);

    [fm removeItemAtURL:testRootURL error:nil];
}

int main(int argc, char *argv[])
{
    test_path_secure();
    test_bundle_validation();
    test_staging_function();
    test_insecure_identifier();
    test_system_doesnt_need_staging();
    test_for_each_insecure_bundle();
    test_custom_authentication();
    test_kext_staging_helpers();
    test_staging_management_helpers();
    exit(0);
}
