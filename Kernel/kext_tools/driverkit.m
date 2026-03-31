#import <Foundation/Foundation.h>
#import <CoreFoundation/CFXPCBridge.h>

#include <IOKit/kext/OSKext.h>
#include <IOKit/kext/OSKextPrivate.h>

#include <launch_priv.h>
#include <Security/Authorization.h>

#include <SystemExtensions/SystemExtensions.h>
#include <SystemExtensions/SystemExtensions_Private.h>

#include "driverkit.h"
#include "signposts.h"
#include "security.h"
#include "kext_tools_util.h"
#include "paths.h"

#define DEXT_DEBUG_ENTITLEMENT  "get-task-allow"

Boolean isDextSignedForDebugging(OSKextRef aDext);
Boolean isDextStandaloneExecutable(OSKextRef aDext);

extern AuthOptions_t KextdAuthenticationOptions;

static OSSystemExtensionClient *gSysextClient = nil;

/*
 * This function takes a dext's identifier and executable path, and wraps
 * them up in the necessary data structures to make launchd happy. It starts
 * a dext daemon with the bundle id as its service name.
 * This function started out using CF things, but the dictionary verbosity
 * got too unwieldy. Trying to contain the ObjC in this file to just this function...
 */
bool submitJob(CFStringRef     serverName,
               CFNumberRef     serverTag,
               CFURLRef        executableURL,
               CFDictionaryRef extraEnvVars,
               Boolean         enableCoreDump,
               Boolean         asDriver)
{
    int                error            = 0;
    bool               result           = false;

    NSArray           *executableArgs   = nil;
    NSString          *bundlePath       = nil;
    NSString          *serverTagString  = nil;
    NSString          *serverNameAndTag = nil;
    xpc_object_t       xjob             = NULL;
    xpc_object_t       ldrequest        = NULL;

    NSMutableDictionary *jobDict   = nil;
    NSMutableDictionary *envVars   = nil;

    bundlePath = [(__bridge NSURL *)executableURL path];
    if (!bundlePath) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "Could not create string path to bundle.");
        goto finish;
    }

    serverTagString = [NSString stringWithFormat:@"0x%llx", [(__bridge NSNumber *)serverTag unsignedLongLongValue]];
    if (!serverTagString) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "Could not get unique tag for DriverKit daemon.");
        goto finish;
    }

    executableArgs = @[ bundlePath, (__bridge NSString *)serverName, serverTagString ];
    if (!executableArgs) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "Could not create array for DriverKit daemon arguments.");
        OSKextLogMemError();
        goto finish;
    }

    serverNameAndTag = [NSString stringWithFormat:@"%@-(%@)", (__bridge NSString *)serverName, serverTagString];
    if (!serverNameAndTag) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "Could not create dext service label!");
        goto finish;
    }

    envVars = [@{
        @"LIBTRACE_DRIVERKIT" : @"1",
    } mutableCopy];
    if (!envVars) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "Could not create environment variables!");
        goto finish;
    }
    if (!asDriver) {
        envVars[@"MallocNanoZone"] = @"1";
    }
    if (extraEnvVars && CFGetTypeID(extraEnvVars) == CFDictionaryGetTypeID()) {
        [envVars addEntriesFromDictionary:(__bridge NSDictionary *)extraEnvVars];
    }

    jobDict = [@{
        @"Label"                : serverNameAndTag,
        @"ProgramArguments"     : executableArgs,
        @"ProcessType"          : asDriver ? @(LAUNCH_KEY_PROCESSTYPE_DRIVER) : @"Interactive",
        @"RunAtLoad"            : @YES,
        @"LaunchOnlyOnce"       : @YES,
        @"EnvironmentVariables" : envVars,
        @"SandboxProfile"       : @"com.apple.dext",
        @"UserName"             : @"_driverkit",
        @"_NullBootstrapPort"   : @YES,
    } mutableCopy];
    if (!jobDict) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "Could not create dictionary for DriverKit daemon job submission.");
        OSKextLogMemError();
        goto finish;
    }
    if (enableCoreDump) {
        [jobDict addEntriesFromDictionary:@{
            @"HardResourceLimits" : @{ @"Core" : @(RLIM_INFINITY) },
            @"SoftResourceLimits" : @{ @"Core" : @(RLIM_INFINITY) },
        }];
    }

    os_signpost_event_emit(get_signpost_log(), OS_SIGNPOST_ID_EXCLUSIVE, SIGNPOST_KEXTD_DEXT_LAUNCH, "%s - %s",
                            [bundlePath UTF8String],
                            [serverNameAndTag UTF8String]);

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations" // needed for SMJobSubmit()

    xjob = _CFXPCCreateXPCObjectFromCFObject((__bridge CFDictionaryRef)jobDict);
    if (!xjob) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "Could not create launchd job for dext.");
        goto finish;
    }

    ldrequest = xpc_dictionary_create(NULL, NULL, 0);
    if (!ldrequest) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "Could not create launchd request dictionary for dext.");
        goto finish;
    }

    xpc_dictionary_set_value(ldrequest, LAUNCH_KEY_SUBMITJOB, xjob);

    // wham-cast. (See SMShared.c in ServiceManagement)
    launch_data_t ldreply = launch_msg((__bridge void *)ldrequest);
    if (!ldreply) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "Could not create reply message from launchd.");
        goto finish;
    }

    error = launch_data_get_errno(ldreply);
    launch_data_free(ldreply);
    if (error) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogIPCFlag,
            "Error %d (%s) in reply received from launchd!", error, strerror(error));
        goto finish;
    }

#pragma clang diagnostic pop

    result = true; /* success */
finish:
    return result;
}

/* This function starts the userland driver portion of a dext by reading a key
 * in the dext's Info.plist which specifies an executable path. This path is
 * handed over to launchd, which starts the driver as a daemon.
 *
 * This function is called by kextdProcessDaemonLaunchRequest
 * in kextd_request.c. It should only be called
 * AFTER all security and staging has taken place.
 */
bool startUserExtension(OSKextRef aDext, CFStringRef serverName, CFNumberRef serverTag)
{
    bool            result        = false;
    bool            coreDumps     = false;
    bool            asDriver      = true; // XXX: Can we delete this soon
    CFURLRef        executableURL = NULL; // do not release
    CFURLRef        absoluteURL   = NULL; // release
    CFStringRef     absolutePath  = NULL; // release
    CFDictionaryRef envVars       = NULL; // do not release
    CFBooleanRef    coreDumpsRef  = NULL; // do not release

    if (!serverName) {
        OSKextLog(aDext,
            kOSKextLogErrorLevel | kOSKextLogIPCFlag | kOSKextLogGeneralFlag,
            "Could not get dext service name.");
        goto finish;
    }

    executableURL = OSKextGetUserExecutableURL(aDext);
    if (!executableURL) {
        OSKextLog(aDext,
            kOSKextLogErrorLevel | kOSKextLogIPCFlag | kOSKextLogGeneralFlag,
            "Could not get URL to executable.");
        goto finish;
    }

    absoluteURL = CFURLCopyAbsoluteURL(OSKextGetURL(aDext));
    if (!absoluteURL) {
        OSKextLog(aDext,
            kOSKextLogErrorLevel | kOSKextLogIPCFlag | kOSKextLogGeneralFlag,
            "Could not get URL to dext.");
        goto finish;
    }

    /*
     * Only allow dexts to run as interactive daemons if they're Apple dexts
     * that are running out of /S/L/DE. Otherwise, they need the DriverKit
     * entitlement to run under the driver process type.
     */
    if (!checkEntitlementAtURL(
                absoluteURL,
                CFSTR(DEXT_LAUNCH_ENTITLEMENT),
                KextdAuthenticationOptions.allowNetwork)) {

        absolutePath = CFURLCopyFileSystemPath(absoluteURL, kCFURLPOSIXPathStyle);
        if (!absolutePath) {
            OSKextLog(aDext,
                kOSKextLogErrorLevel | kOSKextLogIPCFlag | kOSKextLogGeneralFlag,
                "Could not get path to dext.");
            goto finish;
        }

        if (_OSKextIdentifierHasApplePrefix(aDext) &&
            CFStringHasPrefix(absolutePath, CFSTR(_kOSKextSystemLibraryDriverExtensionsFolder))) {
            asDriver = false;
        } else {
            OSKextLog(aDext,
                kOSKextLogErrorLevel | kOSKextLogIPCFlag | kOSKextLogGeneralFlag,
                "Dext executable is not signed with the %s entitlement.", DEXT_LAUNCH_ENTITLEMENT);
            goto finish;
        }
    }

    if (checkEntitlementAtURL(
                absoluteURL,
                CFSTR(DEXT_DEBUG_ENTITLEMENT),
                KextdAuthenticationOptions.allowNetwork)) {

        OSKextLog(aDext,
            kOSKextLogDebugLevel | kOSKextLogGeneralFlag,
            "Dext with server name %s is configured for debugging.",
            CFStringGetCStringPtr(serverName, kCFStringEncodingUTF8));
        envVars      = OSKextGetValueForInfoDictionaryKey(aDext, CFSTR("EnvironmentVariables"));
        coreDumpsRef = OSKextGetValueForInfoDictionaryKey(aDext, CFSTR("EnableCoreDumps"));
        coreDumps    = (coreDumpsRef && CFEqual(coreDumpsRef, kCFBooleanTrue));
    }

    if (!submitJob(serverName, serverTag, executableURL, envVars, coreDumps, asDriver)) {
        OSKextLog(aDext,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "Unable to start user extension daemon.");
        goto finish;
    }

    result = true; /* success */
finish:
    SAFE_RELEASE(absoluteURL);
    SAFE_RELEASE(absolutePath);
    return result;
}

Boolean addCDHashToDextPersonality(OSKextRef aDext, CFMutableDictionaryRef personality)
{
    Boolean     result        = FALSE;
    CFURLRef    executableURL = NULL; // do not release
    CFStringRef bundleID      = NULL; // do not release
    CFStringRef cdHashString  = NULL; // must release

    /* We don't care about old-school kexts here, so early-out with success */
    if (!OSKextDeclaresUserExecutable(aDext)) {
        result = TRUE;
        goto finish;
    }

    bundleID = OSKextGetIdentifier(aDext);
    if (!bundleID) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogIPCFlag,
            "Unable to get bundleID for extension?");
        goto finish;
    }

    executableURL = OSKextGetUserExecutableURL(aDext);
    if (!executableURL) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogIPCFlag,
            "Unable to get executableURL for extension.");
        goto finish;
    }

    cdHashString = copyCDHashFromURL(executableURL);
    if (!cdHashString) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogIPCFlag,
            "Unable to get cdhash from binary.");
        goto finish;
    }

    OSKextLogCFString(aDext,
        kOSKextLogDetailLevel | kOSKextLogGeneralFlag,
        CFSTR("Adding IOUserServerCDHash %@ for dext %@."),
        cdHashString, bundleID);

    CFDictionaryAddValue(personality, CFSTR("IOUserServerCDHash"), cdHashString);

    result = TRUE;
finish:
    SAFE_RELEASE(cdHashString);
    return result;
}

bool copyDextTeamID(CFURLRef kextURL, CFStringRef *teamIDp)
{
    bool                result      = false;
    SecStaticCodeRef    code        = NULL; // must release
    CFDictionaryRef     information = NULL; // must release
    CFStringRef         teamID      = NULL; // do not release
    OSStatus            status      = noErr;

    if (!kextURL) {
        goto finish;
    }

    if (SecStaticCodeCreateWithPath(kextURL,
                                    kSecCSDefaultFlags,
                                    &code) != 0
        || (code == NULL)) {
        OSKextLogMemError();
        goto finish;
    }

    status = SecCodeCopySigningInformation(code,
                                           kSecCSSigningInformation,
                                           &information);
    if (status != noErr) {
        goto finish;
    }

    /*
     * It's possible to have a non-existant team ID. That's okay, but make
     * sure we don't accidentally retain something that's a NULL pointer.
     */
    teamID = CFDictionaryGetValue(information, kSecCodeInfoTeamIdentifier);
    if (teamID) {
        CFRetain(teamID);
    }

    if (teamIDp) {
        *teamIDp = teamID;
    }

    result = true;
finish:
    SAFE_RELEASE(code);
    SAFE_RELEASE(information);
    return result;
}

bool isDextAllowed(OSKextRef aDext)
{
    CFURLRef    kextURL              = NULL; // must release
    CFStringRef teamID               = NULL; // must release
    CFStringRef bundleID             = NULL; // do not release
    NSError    *error                = NULL; // must release
    bool        teamIDPlatformBinary = false;
    bool        teamIDNone           = false;
    bool        approved             = false;

    // SystemExtensions.framework is weak-linked, so be careful
    if (![OSSystemExtensionClient class]) {
        return true;
    }

    if (!gSysextClient) {
        gSysextClient = [[OSSystemExtensionClient alloc] init];
    }

    bundleID = OSKextGetIdentifier(aDext);

    kextURL = CFURLCopyAbsoluteURL(OSKextGetURL(aDext));
    if (!kextURL) {
        OSKextLogMemError();
        goto finish;
    }

    // XXX - sysextd has no opinions on first-party dexts for now.
    if (_OSKextIdentifierHasApplePrefix(aDext)) {
        // teamIDPlatformBinary = true;
        approved = true;
        goto finish;
    } else {
        if (!copyDextTeamID(kextURL, &teamID)) {
            OSKextLogCFString(aDext, kOSKextLogErrorLevel | kOSKextLogFileAccessFlag,
                    CFSTR("Encountered error while trying to copy team ID at %@"),
                    kextURL);
            goto finish;
        }
        if (!teamID) {
            OSKextLog(aDext, kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                    "No team ID found for dext binary.");
            teamIDNone = true;
        }
    }

    approved = [gSysextClient checkExtension:(__bridge NSString *)bundleID
                                      teamID:(__bridge NSString *)teamID
                        teamIDPlatformBinary:teamIDPlatformBinary
                                  teamIDNone:teamIDNone
                                       error:&error];
    if (error) {
        OSKextLogCFString(aDext, kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                CFSTR("Error encountered during approval check: %@"), error);
        approved = false;
    }

finish:
    SAFE_RELEASE(teamID);
    SAFE_RELEASE(kextURL);

    return approved;
}
