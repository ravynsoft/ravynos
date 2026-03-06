/*
 * Copyright (c) 2012 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */
#include <asl.h>
#include <IOKit/kext/OSKext.h>
#include <IOKit/kext/OSKextPrivate.h>
#include <IOKit/IOKitLib.h>
#include <Security/Security.h>
#include <sys/sysctl.h>
#include <sys/csr.h>
#include <sys/xattr.h>
#include <syslog.h>
#include <servers/bootstrap.h>
#include <IOKit/kext/kextmanager_types.h>

#include <os/feature_private.h>

#include "kext_tools_util.h"
#include "security.h"
#include "signposts.h"
#include "staging.h"
#include "syspolicy.h"
#include "driverkit.h"

#if HAVE_DANGERZONE
#include <dz/dz.h>

// Mark functions as weak linked so we can check if they exist without the compiler
// optimizing out the code.
extern void __attribute__((weak_import))
dz_notify_kext_load_v2(dz_kext_load_t type, const char *kextpath,
        dz_kext_load_flags_t flags, bool allowed);

extern void __attribute__((weak_import))
dz_notify_kextcache_update_v2(const char *kextpath, bool allowed);
#endif // HAVE_DANGERZONE

/*******************************************************************************
 * Helper functions
 *******************************************************************************/
static OSStatus     checkRootCertificateIsApple(OSKextRef aKext);
static CFStringRef  copyCDHash(SecStaticCodeRef code);
static CFStringRef  copyIssuerCN(SecCertificateRef certificate);
static CFArrayRef   copySubjectCNArray(CFURLRef kextURL);
static CFStringRef  copyTeamID(SecCertificateRef certificate);
static CFStringRef  createArchitectureList(OSKextRef aKext, CFBooleanRef *isFat);
static void         filterKextLoadForMT(OSKextRef aKext, CFMutableArrayRef kextList, Boolean userLoad);
static Boolean      hashIsInExceptionList(CFURLRef theKextURL, CFDictionaryRef theDict, CFDictionaryRef codesignAttributes);
static uint32_t     getKextDevModeFlags(void);

Boolean
authenticateKext(OSKextRef theKext, void *context)
{
    Boolean result = false;
    OSStatus sigResult = 0;
    const AuthOptions_t *authOptions = (const AuthOptions_t*)context;
    if (authOptions == NULL) {
        OSKextLogCFString(NULL,
                          kOSKextLogErrorLevel | kOSKextLogValidationFlag,
                          CFSTR("Kext rejected due to invalid authentication params: %@"),
                          theKext);
        result = false;
        goto finish;
    }

    if (OSKextIsInExcludeList(theKext, true)) {
        OSKextLogCFString(NULL,
                          kOSKextLogErrorLevel | kOSKextLogValidationFlag,
                          CFSTR("Kext rejected due to presence on exclude list: %@"),
                          theKext);
        messageTraceExcludedKext(theKext);
        result = false;
        goto finish;
    }

    if (authOptions->performFilesystemValidation &&
        !_OSKextBasicFilesystemAuthentication(theKext, NULL)) {
        OSKextLogCFString(NULL,
                          kOSKextLogErrorLevel | kOSKextLogValidationFlag,
                          CFSTR("Kext rejected due to improper filesystem permissions: %@"),
                          theKext);
        result = false;
        goto finish;
    }

    if (authOptions->performSignatureValidation) {
        sigResult = checkKextSignature(theKext, true, authOptions->allowNetwork);
        if (sigResult != 0) {
            if (isInvalidSignatureAllowed()) {
                OSKextLogCFString(NULL,
                                  kOSKextLogErrorLevel | kOSKextLogValidationFlag,
                                  CFSTR("Kext with invalid signature (%ld) allowed: %@"),
                                  (long)sigResult, theKext);
            }
            else {
                OSKextLogCFString(NULL,
                                  kOSKextLogErrorLevel | kOSKextLogValidationFlag,
                                  CFSTR("Kext rejected due to invalid signature: %@"),
                                  theKext);
                result = false;
                goto finish;
            }
        }
    }

    if (authOptions->requireSecureLocation) {
        if (!kextIsInSecureLocation(theKext)) {
            OSKextLogCFString(NULL,
                              kOSKextLogErrorLevel | kOSKextLogValidationFlag,
                              CFSTR("Kext rejected due to insecure location: %@"),
                              theKext);
            result = false;
            goto finish;
        }
    }

    /*
     * Driver Extensions should have their approval checked by sysextd, except
     * for platform binaries.
     */
    if (authOptions->checkDextApproval && OSKextDeclaresUserExecutable(theKext)) {
        if (!isDextAllowed(theKext)) {
            result = false;
            OSKextLogCFString(NULL,
                              kOSKextLogErrorLevel | kOSKextLogValidationFlag,
                              CFSTR("Driver Extension is not approved to run: %@"),
                              theKext);
            result = false;
            goto finish;
        }
    /*
     * Executable and codeless kexts should have their approval checked by syspolicyd
     */
    } else if (authOptions->respectSystemPolicy && !OSKextDeclaresUserExecutable(theKext)) {
        Boolean allowed = false;

        if (authOptions->isCacheLoad) {
            allowed = SPAllowKextLoadCache(theKext);
        } else {
            allowed = SPAllowKextLoad(theKext);
        }

#if 0
        /*
         * Check the 3rd party allow list for this boot.
         * Don't perform this check if we're building a prelinked kernel with kextcache
         * because we necessarily need to reboot before that code can be loaded into
         * the kernel.
         */
        if (!authOptions->is_kextcache) {
            allowed = allowed && isAllowedToLoadThirdPartyKext(theKext);
        }
#endif /* if 0 */

        if (!allowed) {
            OSKextLogCFString(NULL,
                              kOSKextLogErrorLevel | kOSKextLogValidationFlag,
                              CFSTR("Kext rejected due to system policy: %@"),
                              theKext);
            result = false;
            goto finish;
        }
    }

    result = true;

finish:
    return result;
}

/*******************************************************************************
 * checkEntitlementAtURL() - check if the given url is signed with
 * a particular entitlement
 *******************************************************************************/
Boolean checkEntitlementAtURL(CFURLRef anURL, CFStringRef entitlementString, Boolean allowNetwork)
{
    SecRequirementRef entitlementRef    = NULL;
    SecStaticCodeRef  staticCodeRef     = NULL;
    CFStringRef       entitlementReq    = NULL;
    SecCSFlags        flags             = kSecCSCheckAllArchitectures |
                                          kSecCSStrictValidate;
    bool              result            = false;

    if (!anURL || !entitlementString) {
        goto finish;
    }

    flags |= allowNetwork ? kSecCSEnforceRevocationChecks : kSecCSNoNetworkAccess;

    if (SecStaticCodeCreateWithPath(anURL,
                                    kSecCSDefaultFlags,
                                    &staticCodeRef) != errSecSuccess ||
        (staticCodeRef == NULL)) {
        OSKextLogMemError();
        goto finish;
    }

    entitlementReq = CFStringCreateWithFormat(
                        kCFAllocatorDefault,
                        NULL,
                        CFSTR("entitlement[\"%@\"] exists"),
                        entitlementString);
    if (!entitlementReq) {
        OSKextLogMemError();
        goto finish;
    }

    if (SecRequirementCreateWithString(entitlementReq,
                                       kSecCSDefaultFlags,
                                       &entitlementRef) != errSecSuccess ||
        (entitlementRef == NULL)) {
        OSKextLogMemError();
        goto finish;
    }

    result = (SecStaticCodeCheckValidity(staticCodeRef, flags, entitlementRef) == 0);

finish:
    SAFE_RELEASE(entitlementRef);
    SAFE_RELEASE(staticCodeRef);
    SAFE_RELEASE(entitlementReq);
    return result;
}

/*******************************************************************************
 * messageTraceExcludedKext() - log MessageTracer message for kexts in
 * exclude list.
 *  <rdar://problem/12994418> MessageTrace when we block the load of something
 *                            on the kext exclude list
 *******************************************************************************/

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations" // needed for asl_*
void messageTraceExcludedKext(OSKextRef theKext)
{
    CFStringRef     versionString;
    CFStringRef     bundleIDString;
    CFURLRef        kextURL             = NULL;   // must release
    CFStringRef     filename            = NULL;   // must release
    aslmsg          amsg                = NULL;   // must free
    char            *versionCString     = NULL;   // must free
    char            *bundleIDCString    = NULL;   // must free
    char            *filenameCString    = NULL;   // must free

    kextURL = CFURLCopyAbsoluteURL(OSKextGetURL(theKext));
    if (!kextURL) {
        OSKextLogMemError();
        goto finish;
    }
    versionString = OSKextGetValueForInfoDictionaryKey(theKext,
                                                       kCFBundleVersionKey);
    if (versionString) {
        versionCString = createUTF8CStringForCFString(versionString);
    }
    bundleIDString = OSKextGetValueForInfoDictionaryKey(theKext,
                                                        kCFBundleIdentifierKey);
    if (bundleIDString) {
        bundleIDCString = createUTF8CStringForCFString(bundleIDString);
    }

    filename = CFURLCopyLastPathComponent(kextURL);
    if (filename) {
        filenameCString = createUTF8CStringForCFString(filename);
    }
    SAFE_RELEASE(filename);

    /* log the message tracer data
     */
    amsg = asl_new(ASL_TYPE_MSG);
    if (!amsg) {
        OSKextLogMemError();
        goto finish;
    }

    asl_set(amsg, kMessageTracerDomainKey, kMTKextBlockedDomain);
    asl_set(amsg, kMessageTracerBundleIDKey,
            bundleIDCString ? bundleIDCString : "");
    asl_set(amsg, kMessageTracerVersionKey,
            versionCString ? versionCString : "");
    asl_set(amsg, kMessageTracerKextNameKey,
            filenameCString ? filenameCString : "");

    asl_log(NULL, amsg, ASL_LEVEL_NOTICE, "");

finish:
    SAFE_FREE(versionCString);
    SAFE_FREE(bundleIDCString);
    SAFE_FREE(filenameCString);

    SAFE_RELEASE(kextURL);

    if (amsg) {
        asl_free(amsg);
    }
    return;
}
#pragma clang diagnostic pop

/*******************************************************************************
 * checkRootCertificateIsApple() - check if the root certificate of the kext
 *  is issued by Apple
 *  <rdar://problem/12435992>
 *******************************************************************************/
static OSStatus checkRootCertificateIsApple(OSKextRef aKext)
{
    OSStatus                result          = -1;
    CFURLRef                kextURL         = NULL;   // must release
    SecStaticCodeRef        staticCodeRef   = NULL;   // must release
    SecRequirementRef       requirementRef  = NULL;   // must release
    CFStringRef             myCFString;
    CFStringRef             requirementsString;

    if (aKext == NULL) {
        return result;
    }

    kextURL = CFURLCopyAbsoluteURL(OSKextGetURL(aKext));
    if (!kextURL) {
        OSKextLogMemError();
        goto finish;
    }

    if (SecStaticCodeCreateWithPath(kextURL,
                                    kSecCSDefaultFlags,
                                    &staticCodeRef) != errSecSuccess ||
        (staticCodeRef == NULL)) {
        OSKextLogMemError();
        goto finish;
    }

    /* set up correct requirement string */
    myCFString = OSKextGetIdentifier(aKext);
    if (_OSKextIdentifierHasApplePrefix(aKext)) {
        requirementsString = CFSTR("anchor apple");
    }
    else {
        requirementsString = CFSTR("anchor apple generic");
    }

    if (SecRequirementCreateWithString(requirementsString,
                                       kSecCSDefaultFlags,
                                       &requirementRef) != errSecSuccess ||
        (requirementRef == NULL)) {
        OSKextLogMemError();
        goto finish;
    }

    // errSecCSUnsigned == -67062
    result = SecStaticCodeCheckValidity(staticCodeRef,
                                        kSecCSDefaultFlags,
                                        requirementRef);
    if (result != 0) {
        OSKextLogCFString(NULL,
                          kOSKextLogErrorLevel | kOSKextLogLoadFlag,
                          CFSTR("Invalid signature %ld for kext %@"),
                          (long)result, aKext);
    }

finish:
    SAFE_RELEASE(kextURL);
    SAFE_RELEASE(staticCodeRef);
    SAFE_RELEASE(requirementRef);

    return result;
}


/*******************************************************************************
 * getAdhocSignatureHash() - create a hash signature for an unsigned kext
 *  Syrah requires new adhoc signing rules (16411212)
 *******************************************************************************/

void getAdhocSignatureHash(CFURLRef kextURL, char ** signatureBuffer, CFDictionaryRef codesignAttributes)
{
    OSStatus status                     = errSecSuccess;
    CFMutableDictionaryRef signdict     = NULL;   // must release
    SecCodeSignerRef    signerRef       = NULL;   // must release
    SecStaticCodeRef    staticCodeRef   = NULL;   // must release
    CFDataRef           signature       = NULL;   // must release
    CFDictionaryRef     signingDict     = NULL;   // must release
    CFDataRef           cdhash          = NULL;   // must release
    CFMutableDictionaryRef resourceRules = NULL;  // must release
    CFMutableDictionaryRef rules        = NULL;   // must release
    CFMutableDictionaryRef rules2       = NULL;   // must release
    CFMutableDictionaryRef omitPlugins  = NULL;   // must release
    CFMutableDictionaryRef frameworksDict  = NULL;   // must release
    CFMutableDictionaryRef topDict      = NULL;   // must release
    char *              tempBufPtr      = NULL;   // do not free
    CFNumberRef         myNumValue      = NULL;   // must release
    CFNumberRef         myRealValue     = NULL;   // must release
    CFNumberRef         myHashNumValue  = NULL;   // must release

    /* Ad-hoc sign the code temporarily so we can get its hash */
    if (codesignAttributes) {
        status = SecStaticCodeCreateWithPathAndAttributes(kextURL,
                                                          kSecCSDefaultFlags,
                                                          codesignAttributes,
                                                          &staticCodeRef);
    }
    else {
        status = SecStaticCodeCreateWithPath(kextURL,
                                             kSecCSDefaultFlags,
                                             &staticCodeRef);
    }

    if (status != errSecSuccess || staticCodeRef == NULL) {
        OSKextLogMemError();
        goto finish;
    }

    signature = CFDataCreateMutable(NULL, 0);
    if (signature == NULL) {
        OSKextLogMemError();
        goto finish;
    }

    signdict = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
                                         &kCFTypeDictionaryKeyCallBacks,
                                         &kCFTypeDictionaryValueCallBacks);
    if (signdict == NULL) {
        OSKextLogMemError();
        goto finish;
    }

    CFDictionarySetValue(signdict, kSecCodeSignerIdentity, kCFNull);
    CFDictionarySetValue(signdict, kSecCodeSignerDetached, signature);

    int myHashNum = kSecCodeSignatureHashSHA1;
    myHashNumValue = CFNumberCreate(kCFAllocatorDefault,
                                    kCFNumberIntType, &myHashNum);
    if (myHashNumValue == NULL) {
        OSKextLogMemError();
        goto finish;
    }
    CFDictionarySetValue(signdict, kSecCodeSignerDigestAlgorithm, myHashNumValue); // 24059794

    resourceRules = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
                                              &kCFTypeDictionaryKeyCallBacks,
                                              &kCFTypeDictionaryValueCallBacks);
    if (resourceRules == NULL) {
        OSKextLogMemError();
        goto finish;
    }

    rules = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
                                      &kCFTypeDictionaryKeyCallBacks,
                                      &kCFTypeDictionaryValueCallBacks);
    if (rules == NULL) {
        OSKextLogMemError();
        goto finish;
    }

    rules2 = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
                                      &kCFTypeDictionaryKeyCallBacks,
                                      &kCFTypeDictionaryValueCallBacks);
    if (rules2 == NULL) {
        OSKextLogMemError();
        goto finish;
    }

    omitPlugins = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
                                            &kCFTypeDictionaryKeyCallBacks,
                                            &kCFTypeDictionaryValueCallBacks);
    if (omitPlugins == NULL) {
        OSKextLogMemError();
        goto finish;
    }

    frameworksDict = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
                                               &kCFTypeDictionaryKeyCallBacks,
                                               &kCFTypeDictionaryValueCallBacks);
    if (frameworksDict == NULL) {
        OSKextLogMemError();
        goto finish;
    }

    topDict = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
                                               &kCFTypeDictionaryKeyCallBacks,
                                               &kCFTypeDictionaryValueCallBacks);
    if (topDict == NULL) {
        OSKextLogMemError();
        goto finish;
    }

    /* exclude PlugIns directory (old style, see 16411212)
     */
    int myNum = 100;
    myNumValue = CFNumberCreate(kCFAllocatorDefault,
                                kCFNumberIntType, &myNum);
    if (myNumValue == NULL) {
        OSKextLogMemError();
        goto finish;
    }

    CFDictionarySetValue(omitPlugins, CFSTR("omit"), kCFBooleanTrue);
    CFDictionarySetValue(omitPlugins, CFSTR("weight"), myNumValue);

    CFDictionarySetValue(rules, CFSTR("^.*"), kCFBooleanTrue);
    CFDictionarySetValue(rules, CFSTR("^PlugIns/"), omitPlugins);
    CFDictionarySetValue(resourceRules, CFSTR("rules"), rules);

    /* exclude PlugIns directory (new style, see 16411212)
     */
    float myRealNum = 0.0;
    myRealValue = CFNumberCreate(kCFAllocatorDefault,
                                kCFNumberFloatType, &myRealNum);
    if (myRealValue == NULL) {
        OSKextLogMemError();
        goto finish;
    }

    CFDictionarySetValue(frameworksDict, CFSTR("nested"), kCFBooleanTrue);
    CFDictionarySetValue(frameworksDict, CFSTR("weight"), myRealValue);
    CFDictionarySetValue(rules2,
        CFSTR("^(Frameworks|SharedFrameworks|Plugins|Plug-ins|XPCServices|Helpers|MacOS)/"),
                         frameworksDict);

    CFDictionarySetValue(rules2, CFSTR("^.*"), kCFBooleanTrue);
    CFDictionarySetValue(rules2, CFSTR("^PlugIns/"), omitPlugins);

    CFDictionarySetValue(topDict, CFSTR("top"), kCFBooleanTrue);
    CFDictionarySetValue(topDict, CFSTR("weight"), myRealValue);
    CFDictionarySetValue(rules2,
                         CFSTR("^[^/]+$"),
                         topDict);
    CFDictionarySetValue(resourceRules, CFSTR("rules2"), rules2);

    // add both rules to signdict
    CFDictionarySetValue(signdict, kSecCodeSignerResourceRules, resourceRules);

    if (SecCodeSignerCreate(signdict, kSecCSDefaultFlags | kSecCSSignOpaque, &signerRef) != 0) {
        OSKextLog(NULL, kOSKextLogDebugLevel | kOSKextLogGeneralFlag,
                  "%s - SecCodeSignerCreate failed", __func__);
        goto finish;
    }
    if (SecCodeSignerAddSignature(signerRef, staticCodeRef, kSecCSDefaultFlags) != 0) {
        OSKextLog(NULL, kOSKextLogDebugLevel | kOSKextLogGeneralFlag,
                  "%s - SecCodeSignerAddSignature failed", __func__);
        goto finish;
    }
    if (SecCodeSetDetachedSignature(staticCodeRef, signature, kSecCSDefaultFlags) != 0) {
        OSKextLog(NULL, kOSKextLogDebugLevel | kOSKextLogGeneralFlag,
                  "%s - SecCodeSetDetachedSignature failed", __func__);
        goto finish;
    }

    /* get the hash info
     */
    if (SecCodeCopySigningInformation(staticCodeRef, kSecCSDefaultFlags, &signingDict) != 0
        || signingDict == NULL) {
        OSKextLog(NULL, kOSKextLogDebugLevel | kOSKextLogGeneralFlag,
                  "%s - SecCodeCopySigningInformation failed", __func__);
        goto finish;
    }

    cdhash = (CFDataRef) CFDictionaryGetValue(signingDict, kSecCodeInfoUnique);
    // <rdar://problem/24539186> protect against badly signed kexts
    if (cdhash && CFGetTypeID(cdhash) == CFDataGetTypeID()) {
        CFRetain(cdhash);

        const UInt8 *   hashDataPtr     = NULL;  // don't free
        CFIndex         hashDataLen     = 0;

        hashDataPtr = CFDataGetBytePtr(cdhash);
        hashDataLen = CFDataGetLength(cdhash);
        tempBufPtr = (char *) malloc((hashDataLen + 1) * 2);
        if (tempBufPtr == NULL) {
            OSKextLogMemError();
            goto finish;
        }
#if 0
        OSKextLogCFString(/* kext */ NULL,
                          kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                          CFSTR("%s - kextURL %@"),
                          __func__,
                          kextURL);
        OSKextLogCFString(/* kext */ NULL,
                          kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                          CFSTR("%s - cdhash %@"),
                          __func__,
                          cdhash);
#endif

        bzero(tempBufPtr, ((hashDataLen + 1) * 2));
        for (int i = 0; i < hashDataLen; i++) {
            sprintf(tempBufPtr + (i * 2), "%02.2x", *(hashDataPtr + i));
        }
    }

finish:
    SAFE_RELEASE(signdict);
    SAFE_RELEASE(signerRef);
    SAFE_RELEASE(staticCodeRef);
    SAFE_RELEASE(signature);
    SAFE_RELEASE(signingDict);
    SAFE_RELEASE(cdhash);
    SAFE_RELEASE(resourceRules);
    SAFE_RELEASE(rules);
    SAFE_RELEASE(rules2);
    SAFE_RELEASE(omitPlugins);
    SAFE_RELEASE(frameworksDict);
    SAFE_RELEASE(topDict);
    SAFE_RELEASE(myNumValue);
    SAFE_RELEASE(myRealValue);
    SAFE_RELEASE(myHashNumValue);

    *signatureBuffer = tempBufPtr;
}

/*******************************************************************************
 * createArchitectureList() - create the list of architectures for the kext
 *  <rdar://13529984>
 *  Note: the caller must release the created CFStringRef
 *******************************************************************************/
static CFStringRef createArchitectureList(OSKextRef aKext, CFBooleanRef *isFat)
{
    if (aKext == NULL || isFat == NULL) {
        return NULL;
    }

    *isFat = kCFBooleanFalse;

    const NXArchInfo **         archList            = NULL; // must free
    CFMutableArrayRef           archNamesList       = NULL; // must release
    CFStringRef                 archNames           = NULL; // do not release
    const char *                archNameCString     = NULL; // do not free
    int                         index               = 0;

    archList = OSKextCopyArchitectures(aKext);
    if (!archList) {
        goto finish;
    }

    archNamesList = CFArrayCreateMutable(kCFAllocatorDefault,
                                         0,
                                         &kCFTypeArrayCallBacks);
    if (!archNamesList) {
        goto finish;
    }

    for (index=0; archList[index]; index++) {
        archNameCString = archList[index]->name;
        if (archNameCString) {
            CFStringRef archName = NULL;
            archName = CFStringCreateWithCString(kCFAllocatorDefault,
                                                 archNameCString,
                                                 kCFStringEncodingUTF8);
            if (archName) {
                CFArrayAppendValue(archNamesList, archName);
                SAFE_RELEASE_NULL(archName);
            }
        }
    }

    if (index>1) {
        *isFat = kCFBooleanTrue;
    }
    archNames = CFStringCreateByCombiningStrings(kCFAllocatorDefault,
                                                 archNamesList,
                                                 CFSTR(" "));
    if (!archNames) {
        goto finish;
    }

finish:
    SAFE_RELEASE(archNamesList);
    SAFE_FREE(archList);

    return archNames;
}

/*******************************************************************************
 * copyTeamID() - copy the team id field from the given certificate
 *  <rdar://13646260>
 *  Note: the caller must release the created CFStringRef
 *******************************************************************************/
static CFStringRef copyTeamID(SecCertificateRef certificate)
{
    if (!certificate ||
        CFGetTypeID(certificate) !=SecCertificateGetTypeID()) {
        return NULL;
    }

    CFDictionaryRef     subjectDict     = NULL; // do not release
    CFArrayRef          subjectArray    = NULL; // do not release
    CFDictionaryRef     subjectInfo     = NULL; // do not release
    CFStringRef         teamID          = NULL; // do not release
    CFErrorRef          error           = NULL; // do not release

    CFMutableArrayRef   certificateKeys = NULL; // must release
    CFDictionaryRef     certificateDict = NULL; // must release

    certificateKeys = CFArrayCreateMutable(kCFAllocatorDefault,
                                           1,
                                           &kCFTypeArrayCallBacks);
    if (!certificateKeys) {
        goto finish;
    }

    CFArrayAppendValue(certificateKeys, kSecOIDX509V1SubjectName);

    certificateDict = SecCertificateCopyValues(certificate,
                                               certificateKeys,
                                               &error);
    if (error != errSecSuccess ||
        !certificateDict ||
        CFGetTypeID(certificateDict) != CFDictionaryGetTypeID()) {
        goto finish;
    }

    subjectDict = (CFDictionaryRef) CFDictionaryGetValue(certificateDict,
                                                         kSecOIDX509V1SubjectName);
    if (!subjectDict ||
        CFGetTypeID(subjectDict) != CFDictionaryGetTypeID()) {
        goto finish;
    }

    subjectArray = (CFArrayRef) CFDictionaryGetValue(subjectDict,
                                                     kSecPropertyKeyValue);
    if (!subjectArray ||
        CFGetTypeID(subjectArray) != CFArrayGetTypeID()) {
        goto finish;
    }

    // Try to look for UserID field ("0.9.2342.19200300.100.1.1")
    for (int index=0; index<CFArrayGetCount(subjectArray); index++) {
        subjectInfo = (CFDictionaryRef) CFArrayGetValueAtIndex(subjectArray,
                                                               index);
        if (!subjectInfo ||
            CFGetTypeID(subjectInfo) != CFDictionaryGetTypeID()) {
            continue;
        }

        CFStringRef label = NULL; // do not release
        label = CFDictionaryGetValue(subjectInfo,
                                     kSecPropertyKeyLabel);
        if (kCFCompareEqualTo == CFStringCompare(label,
                                                 CFSTR("0.9.2342.19200300.100.1.1"),
                                                 0)) {
            teamID = CFDictionaryGetValue(subjectInfo,
                                          kSecPropertyKeyValue);
            if (teamID &&
                CFGetTypeID(teamID) == CFStringGetTypeID()) {
                CFRetain(teamID);
                goto finish;
            }
            else {
                teamID = NULL;
            }
        }
    }

    // In case of failure, look for OU field ("2.5.4.11")
    if (!teamID) {
        for (int index=0; index<CFArrayGetCount(subjectArray); index++) {
            subjectInfo = (CFDictionaryRef) CFArrayGetValueAtIndex(subjectArray,
                                                                   index);
            if (!subjectInfo ||
                CFGetTypeID(subjectInfo) != CFDictionaryGetTypeID()) {
                continue;
            }

            CFStringRef label = NULL; // do not release
            label = CFDictionaryGetValue(subjectInfo,
                                         kSecPropertyKeyLabel);
            if (kCFCompareEqualTo == CFStringCompare(label,
                                                     CFSTR("2.5.4.11"),
                                                     0)) {
                teamID = CFDictionaryGetValue(subjectInfo,
                                              kSecPropertyKeyValue);
                if (teamID &&
                    CFGetTypeID(teamID) == CFStringGetTypeID()) {
                    CFRetain(teamID);
                    goto finish;
                }
                else {
                    teamID = NULL;
                }
            }
        }
    }

finish:
    if (!teamID && subjectArray) {
        CFShow(subjectArray);
    }
    SAFE_RELEASE(certificateKeys);
    SAFE_RELEASE(certificateDict);
    return teamID;
}

/*******************************************************************************
 * copyIssuerCN() - copy the issuer CN field from the given certificate
 *  <rdar://13646260>
 *  Note: the caller must release the created CFStringRef
 *******************************************************************************/
static CFStringRef copyIssuerCN(SecCertificateRef certificate)
{
    if (!certificate ||
        CFGetTypeID(certificate) !=SecCertificateGetTypeID()) {
        return NULL;
    }

    CFStringRef         issuerCN        = NULL; // do not release
    CFDictionaryRef     issuerDict      = NULL; // do not release
    CFArrayRef          issuerArray     = NULL; // do not release
    CFDictionaryRef     issuerInfo      = NULL; // do not release
    CFErrorRef          error           = NULL; // do not release

    CFMutableArrayRef   certificateKeys = NULL; // must release
    CFDictionaryRef     certificateDict = NULL; // must release

    certificateKeys = CFArrayCreateMutable(kCFAllocatorDefault,
                                           1,
                                           &kCFTypeArrayCallBacks);
    if (!certificateKeys) {
        goto finish;
    }

    CFArrayAppendValue(certificateKeys, kSecOIDX509V1IssuerName);

    certificateDict = SecCertificateCopyValues(certificate,
                                               certificateKeys,
                                               &error);

    if (error != errSecSuccess ||
        !certificateDict ||
        CFGetTypeID(certificateDict) != CFDictionaryGetTypeID()) {
        goto finish;
    }

    issuerDict = (CFDictionaryRef) CFDictionaryGetValue(certificateDict,
                                                        kSecOIDX509V1IssuerName);
    if (!issuerDict ||
        CFGetTypeID(issuerDict) != CFDictionaryGetTypeID()) {
        goto finish;
    }

    issuerArray = (CFArrayRef) CFDictionaryGetValue(issuerDict,
                                                    kSecPropertyKeyValue);
    if (!issuerArray ||
        CFGetTypeID(issuerArray) != CFArrayGetTypeID()) {
        goto finish;
    }

    for (int index=0; index<CFArrayGetCount(issuerArray); index++) {
        issuerInfo = (CFDictionaryRef) CFArrayGetValueAtIndex(issuerArray,
                                                              index);
        if (!issuerInfo ||
            CFGetTypeID(issuerInfo) != CFDictionaryGetTypeID()) {
            continue;
        }

        CFStringRef label = NULL; // do not release
        label = CFDictionaryGetValue(issuerInfo,
                                     kSecPropertyKeyLabel);
        if (kCFCompareEqualTo == CFStringCompare(label,
                                                 CFSTR("2.5.4.3"),
                                                 0)) {
            issuerCN = CFDictionaryGetValue(issuerInfo,
                                            kSecPropertyKeyValue);
            if (issuerCN &&
                CFGetTypeID(issuerCN) == CFStringGetTypeID()) {
                CFRetain(issuerCN);
                goto finish;
            }
            else {
                issuerCN = NULL;
            }
        }
    }

finish:
    SAFE_RELEASE(certificateDict);
    SAFE_RELEASE(certificateKeys);
    return issuerCN;
}

/*******************************************************************************
 * copyCDHash() - copy the SHA-1 hash of the code
 *  <rdar://13646260>
 *  Note: the caller must release the created CFStringRef
 *******************************************************************************/
static CFStringRef copyCDHash(SecStaticCodeRef code)
{
    CFDictionaryRef signingInfo     = NULL; // must release
    char *          tempBufPtr      = NULL; // free
    const UInt8 *   hashDataPtr     = NULL; // do not free
    CFDataRef       cdhash          = NULL; // do not release
    CFStringRef     hash            = NULL; // do not release
    CFIndex         hashDataLen     = 0;

    SecCodeCopySigningInformation(code,
                                  kSecCSDefaultFlags,
                                  &signingInfo);
    if (!signingInfo) {
        goto finish;
    }

    cdhash = CFDictionaryGetValue(signingInfo, kSecCodeInfoUnique);
    if (!cdhash ||
        CFGetTypeID(cdhash) != CFDataGetTypeID()) {
        goto finish;
    }

    hashDataPtr = CFDataGetBytePtr(cdhash);
    hashDataLen = CFDataGetLength(cdhash);
    tempBufPtr = (char *) malloc((hashDataLen + 1) * 2);
    if (tempBufPtr == NULL) {
        OSKextLogMemError();
        goto finish;
    }

    bzero(tempBufPtr, ((hashDataLen + 1) * 2));
    for (int i = 0; i < hashDataLen; i++) {
        sprintf(tempBufPtr + (i * 2), "%02.2x", *(hashDataPtr + i));
    }

    if (tempBufPtr) {
        hash = CFStringCreateWithCString(kCFAllocatorDefault,
                                         tempBufPtr,
                                         kCFStringEncodingUTF8);
    }

finish:
    SAFE_FREE(tempBufPtr);
    SAFE_RELEASE(signingInfo);
    return hash;
}

/*******************************************************************************
 * copyCDHashFromURL() - copy the cdHash for the resource at the given URL
 * Note: the caller must release the created CFStringRef
 *******************************************************************************/
CFStringRef copyCDHashFromURL(CFURLRef anURL)
{
    SecStaticCodeRef code   = NULL; // must release
    CFStringRef      cdHash = NULL; // do not release

    if (!anURL) {
        goto finish;
    }

    if (SecStaticCodeCreateWithPath(anURL,
                                    kSecCSDefaultFlags,
                                    &code) != 0
        || (code == NULL)) {
        OSKextLogMemError();
        goto finish;
    }

    cdHash = copyCDHash(code);
finish:
    SAFE_RELEASE(code);
    return cdHash;
}

/*******************************************************************************
 * copySigningInfo() - copy a set of signing information for the given kext
 *  cdhash:     the SHA-1 hash
 *  teamid:     the team id of the leaf certificate
 *  subjectCN:  the subject common name of the leaf certificate
 *  issuerCN:   the issuer common name of the leaf certificate
 *  <rdar://13646260>
 *  Note: the caller must release the created CFStringRefs
 *******************************************************************************/
void copySigningInfo(CFURLRef kextURL,
                            CFStringRef* cdhash,
                            CFStringRef* teamId,
                            CFStringRef* subjectCN,
                            CFStringRef* issuerCN)
{
    if (!kextURL) {
        return;
    }

    SecStaticCodeRef    code                = NULL; // must release
    CFDictionaryRef     information         = NULL; // must release

    SecCertificateRef   issuerCertificate   = NULL; // do not release
    CFArrayRef          certificateChain    = NULL; // do not release
    OSStatus            status;
    CFIndex             count;

    if (SecStaticCodeCreateWithPath(kextURL,
                                    kSecCSDefaultFlags,
                                    &code) != 0
        || (code == NULL)) {
        OSKextLogMemError();
        goto finish;
    }

    if (cdhash) {
        *cdhash = copyCDHash(code);
    }

    status = SecCodeCopySigningInformation(code,
                                           kSecCSSigningInformation,
                                           &information);
    if (status != noErr) {
        goto finish;
    }

    // a CFArrayRef array of SecCertificateRef objects
    certificateChain = (CFArrayRef) CFDictionaryGetValue(information, kSecCodeInfoCertificates);
    if (!certificateChain ||
        CFGetTypeID(certificateChain) != CFArrayGetTypeID()) {
        goto finish;
    }

    count = CFArrayGetCount(certificateChain);
    if (count < 1) {
        goto finish;
    }

    issuerCertificate = (SecCertificateRef) CFArrayGetValueAtIndex(certificateChain, 0);
    if (!issuerCertificate) {
        goto finish;
    }

    if (subjectCN) {
        SecCertificateCopyCommonName(issuerCertificate, subjectCN);
    }

    if (teamId) {
        *teamId = copyTeamID(issuerCertificate);
    }

    if (issuerCN) {
        *issuerCN = copyIssuerCN(issuerCertificate);
    }

finish:
    SAFE_RELEASE(code);
    SAFE_RELEASE(information);

    return;
}

/*******************************************************************************
 * copySubjectCNArray() - copy the subject CN from every certificate in the kext's
 *  certificate chain.
 *  <rdar://13646260>
 *  Note: the caller must release the created CFArrayRef
 *******************************************************************************/
static CFArrayRef copySubjectCNArray(CFURLRef kextURL)
{
    if (!kextURL) {
        return NULL;
    }

    CFMutableArrayRef   subjectCNArray      = NULL; // do not release
    CFArrayRef          certificateChain    = NULL; // do not release
    SecCertificateRef   certificate         = NULL; // do not release

    SecStaticCodeRef    code                = NULL; // must release
    CFDictionaryRef     information         = NULL; // must release

    OSStatus            status;
    CFIndex             count;

    subjectCNArray = CFArrayCreateMutable(kCFAllocatorDefault,
                                          0,
                                          &kCFTypeArrayCallBacks);
    if (!subjectCNArray) {
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

    certificateChain = (CFArrayRef) CFDictionaryGetValue(information, kSecCodeInfoCertificates);
    if (!certificateChain ||
        CFGetTypeID(certificateChain) != CFArrayGetTypeID()) {
        goto finish;
    }

    count = CFArrayGetCount(certificateChain);
    if (count < 1) {
        goto finish;
    }

    for (CFIndex i=0; i<count; i++) {
        certificate = (SecCertificateRef) CFArrayGetValueAtIndex(certificateChain, i);
        CFStringRef subjectCN = NULL; // must release
        SecCertificateCopyCommonName(certificate, &subjectCN);
        if (subjectCN) {
            CFArrayAppendValue(subjectCNArray, subjectCN);
            SAFE_RELEASE(subjectCN);
        }
    }

finish:
    SAFE_RELEASE(code);
    SAFE_RELEASE(information);

    return subjectCNArray;
}

/*******************************************************************************
 * Helper to check a personality matches
 *******************************************************************************/

 typedef struct {
     CFMutableArrayRef personalities;
 } __OSKextPersonalityBundleIdentifierContext;

 static void __OSKextPersonalityBundleIdentifierApplierFunction(
     const void * vKey,
     const void * vValue,
           void * vContext)
{
    CFStringRef            personalityName     = (CFStringRef)vKey;
    CFMutableDictionaryRef personality         = (CFMutableDictionaryRef)vValue;
    __OSKextPersonalityBundleIdentifierContext  * context =
     (__OSKextPersonalityBundleIdentifierContext *)vContext;
    CFMutableArrayRef      personalities       = context->personalities;
    io_service_t           match;

    if (!personalities) return;

    CFRetain(personality);
    match = IOServiceGetMatchingService(kIOMasterPortDefault, personality);
    if (MACH_PORT_NULL != match) {
        IOObjectRelease(match);
        CFArrayAppendValue(personalities, personalityName);
    }
}

/*******************************************************************************
 * filterKextLoadForMT() - check that the kext is of interest, and place kext
 * information in the kext list
 *  <rdar://problem/12435992>
 *******************************************************************************/

static void filterKextLoadForMT(OSKextRef aKext, CFMutableArrayRef kextList, Boolean userLoad)
{
    if (aKext == NULL || kextList == NULL)
        return;

    CFStringRef     versionString;                // do not release
    CFStringRef     bundleIDString;               // do not release
    CFStringRef     kextSigningCategory = NULL;   // do not release
    CFDateRef       timestamp           = NULL;   // do not release
    CFBooleanRef    isFat               = kCFBooleanFalse; // do not release
    CFBooleanRef    isSigned            = kCFBooleanFalse; // do not release
    CFURLRef        kextURL             = NULL;   // must release
    CFURLRef        kextExecURL         = NULL;   // must release
    CFStringRef     kextPath            = NULL;   // must release
    CFStringRef     kextExecPath        = NULL;   // must release
    CFStringRef     filename            = NULL;   // must release
    CFStringRef     hashString          = NULL;   // must release
    CFStringRef     archString          = NULL;   // must release
    CFStringRef     teamId              = NULL;   // must release
    CFStringRef     subjectCN           = NULL;   // must release
    CFStringRef     issuerCN            = NULL;   // must release
    CFBooleanRef    codeless            = kCFBooleanFalse;
    SecStaticCodeRef        code          = NULL;   // must release
    CFDictionaryRef         information   = NULL;   // must release
    CFDictionaryRef         personalities = NULL;   // must release
    CFMutableDictionaryRef  kextDict      = NULL;   // must release
    char *          hashCString         = NULL;   // must free
    OSStatus status = noErr;

    __OSKextPersonalityBundleIdentifierContext context = { 0 };

    kextURL = CFURLCopyAbsoluteURL(OSKextGetURL(aKext));
    if (!kextURL) {
        OSKextLogMemError();
        goto finish;
    }
    kextPath = CFURLCopyFileSystemPath(kextURL, kCFURLPOSIXPathStyle);
    if (!kextPath) {
        OSKextLogMemError();
        goto finish;
    }

    versionString   = OSKextGetValueForInfoDictionaryKey(aKext,
                                                         kCFBundleVersionKey);
    bundleIDString  = OSKextGetValueForInfoDictionaryKey(aKext,
                                                         kCFBundleIdentifierKey);
    filename = CFURLCopyLastPathComponent(kextURL);

    archString = createArchitectureList(aKext, &isFat);

    if (OSKextDeclaresExecutable(aKext)) {
        kextExecURL = CFURLCopyAbsoluteURL(OSKextGetKernelExecutableURL(aKext));
        if (kextExecURL) kextExecPath = CFURLCopyFileSystemPath(kextExecURL, kCFURLPOSIXPathStyle);
    } else {
        codeless = kCFBooleanTrue;
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

    isSigned = CFDictionaryContainsKey(information, kSecCodeInfoIdentifier);
    if (!isSigned) {
        /* The kext is unsigned, so there is little information we can retrieve.
         * A hash of the kext is generated for data collection. */
        kextSigningCategory = CFSTR(kUnsignedKext);

        getAdhocSignatureHash(kextURL, &hashCString, NULL);
        if (hashCString) {
            hashString = CFStringCreateWithCString(kCFAllocatorDefault,
                                                   hashCString,
                                                   kCFStringEncodingUTF8);
        }
    }
    else {
        CFStringRef myCFString = NULL; // do not release
        myCFString = OSKextGetIdentifier(aKext);

        timestamp = CFDictionaryGetValue(information, kSecCodeInfoTimestamp);
        if (timestamp && (CFDateGetTypeID() != CFGetTypeID(timestamp))) {
            timestamp = NULL;
        }

        /* MT functions are avoided in early boot, so the network is always available */
        status = checkKextSignature(aKext, true, true);
        if (_OSKextIdentifierHasApplePrefix(aKext)) {
            if (status == noErr) {
                /* This is a signed Apple kext, with an Apple root certificate.
                 * There is no need to log these, so simply jump to the cleanup */
                goto finish;
            }
            else {
                /* This is a signed Apple kext, but the root certificate is not Apple.
                 * This should not happen, so it is better to flag it as unsigned for
                 * collection purpose. */
                kextSigningCategory = CFSTR(kUnsignedKext);
                copySigningInfo(kextURL,
                                &hashString,
                                &teamId,
                                NULL,
                                &issuerCN);
                CFArrayRef subjectCNArray = NULL; // must release
                subjectCNArray = copySubjectCNArray(kextURL);
                if (subjectCNArray) {
                    subjectCN = CFStringCreateByCombiningStrings(kCFAllocatorDefault,
                                                                 subjectCNArray,
                                                                 CFSTR(";"));
                    SAFE_RELEASE(subjectCNArray);
                }
            }
        }
        else {
            if (status == noErr) {
                /* This 3rd-party kext is signed with a devid+ kext certificate */
                kextSigningCategory = CFSTR(k3rdPartyKextWithDevIdPlus);
                copySigningInfo(kextURL,
                                &hashString,
                                &teamId,
                                &subjectCN,
                                &issuerCN);
            }
            else if (status == CSSMERR_TP_CERT_REVOKED) {
                /* This 3rd-party kext is signed with a revoked devid+ kext certificate */
                kextSigningCategory = CFSTR(k3rdPartyKextWithRevokedDevIdPlus);
                copySigningInfo(kextURL,
                                &hashString,
                                &teamId,
                                &subjectCN,
                                &issuerCN);
            }
            else {
                status = checkRootCertificateIsApple(aKext);
                if (status == noErr) {
                    /* This 3rd-party kext is not signed with a devid+ certificate,
                     * but uses an Apple root certificate. */
                    kextSigningCategory = CFSTR(k3rdPartyKextWithAppleRoot);
                    /* The certificates may not have the expected format.
                     * Attempt to get the information if present, and also
                     * retrieve the subject cn of every certificate in the chain. */
                    copySigningInfo(kextURL,
                                    &hashString,
                                    &teamId,
                                    NULL,
                                    &issuerCN);
                    CFArrayRef subjectCNArray = NULL; // must release
                    subjectCNArray = copySubjectCNArray(kextURL);
                    if (subjectCNArray) {
                        subjectCN = CFStringCreateByCombiningStrings(kCFAllocatorDefault,
                                                                     subjectCNArray,
                                                                     CFSTR(";"));
                        SAFE_RELEASE(subjectCNArray);
                    }
                }
                else {
                    /* This 3rd-party kext is not signed with a devid+ certificate,
                     * and does not use an Apple root certificate. */
                    kextSigningCategory = CFSTR(k3rdPartyKextWithoutAppleRoot);
                    /* The certificates may not have the expected format.
                     * The subjectcn, issuercn and teamid must not be logged. */
                    copySigningInfo(kextURL,
                                    &hashString,
                                    NULL,
                                    NULL,
                                    NULL);
                }
            }
        }
    }

    personalities = OSKextGetValueForInfoDictionaryKey(aKext,
        CFSTR(kIOKitPersonalitiesKey));
    if (personalities
        && (CFGetTypeID(personalities) == CFDictionaryGetTypeID())
        && (CFDictionaryGetCount(personalities) > 1))
    {
        context.personalities = CFArrayCreateMutable(kCFAllocatorDefault,
                                                     0,
                                                     &kCFTypeArrayCallBacks);
        CFDictionaryApplyFunction(personalities,
            __OSKextPersonalityBundleIdentifierApplierFunction,
            &context);
    }

    kextDict = CFDictionaryCreateMutable(kCFAllocatorDefault,
                                         0,
                                         &kCFTypeDictionaryKeyCallBacks,
                                         &kCFTypeDictionaryValueCallBacks);
    if (!kextDict) {
        OSKextLogMemError();
        goto finish;
    }

    if (bundleIDString) {
        CFDictionaryAddValue(kextDict,
                             CFSTR(kMessageTracerBundleIDKey),
                             bundleIDString);
    }
    if (versionString) {
        CFDictionaryAddValue(kextDict,
                             CFSTR(kMessageTracerVersionKey),
                             versionString);
    }
    if (filename) {
        CFDictionaryAddValue(kextDict,
                             CFSTR(kMessageTracerKextNameKey),
                             filename);
    }
    if (isFat != NULL) {
        CFDictionaryAddValue(kextDict,
                             CFSTR(kMessageTracerFatKey),
                             isFat);
    }
    if (archString) {
        CFDictionaryAddValue(kextDict,
                             CFSTR(kMessageTracerArchKey),
                             archString);
    }

    if (kextSigningCategory) {
        CFDictionaryAddValue(kextDict,
                             CFSTR(kMessageTracerSignatureTypeKey),
                             kextSigningCategory);
    }

    if (hashString) {
        CFDictionaryAddValue(kextDict,
                             CFSTR(kMessageTracerHashKey),
                             hashString);
    }

    if (teamId) {
        CFDictionaryAddValue(kextDict,
                             CFSTR(kMessageTracerTeamIdKey),
                             teamId);
    }
    if (subjectCN) {
        CFDictionaryAddValue(kextDict,
                             CFSTR(kMessageTracerSubjectCNKey),
                             subjectCN);
    }
    if (issuerCN) {
        CFDictionaryAddValue(kextDict,
                             CFSTR(kMessageTracerIssuerCNKey),
                             issuerCN);
    }

    if (kextPath) {
        CFDictionaryAddValue(kextDict,
                             CFSTR(kMessageTracerPathKey),
                             kextPath);
    }

    if (kextExecPath) {
        CFDictionaryAddValue(kextDict,
                             CFSTR(kMessageTracerExecPathKey),
                             kextExecPath);
    }

    if (timestamp) {
        CFDictionaryAddValue(kextDict,
                             CFSTR(kMessageTracerSigningTimeKey),
                             timestamp);
    }

    CFDictionaryAddValue(kextDict, CFSTR(kMessageTracerCodelessKey), codeless);

    if (userLoad) {
        CFDictionaryAddValue(kextDict, CFSTR(kMessageTracerUserLoadKey), kCFBooleanTrue);
    }

    if (context.personalities && CFArrayGetCount(context.personalities)) {
        CFDictionaryAddValue(kextDict,
                             CFSTR(kMessageTracerPersonalityNamesKey),
                             context.personalities);
    }

    CFArrayAppendValue(kextList, kextDict);

finish:
    SAFE_FREE(hashCString);
    SAFE_RELEASE(kextURL);
    SAFE_RELEASE(kextExecURL);
    SAFE_RELEASE(kextPath);
    SAFE_RELEASE(kextExecPath);
    SAFE_RELEASE(filename);
    SAFE_RELEASE(hashString);
    SAFE_RELEASE(kextDict);
    SAFE_RELEASE(archString);
    SAFE_RELEASE(teamId);
    SAFE_RELEASE(subjectCN);
    SAFE_RELEASE(issuerCN);
    SAFE_RELEASE(code);
    SAFE_RELEASE(information);
    SAFE_RELEASE(context.personalities);
    return;
}

/*******************************************************************************
 * recordKextLoadListForMT() - record the list of loaded kexts
 *  <rdar://problem/12435992>
 *******************************************************************************/
void
recordKextLoadListForMT(CFArrayRef kextList, Boolean userLoad)
{
    CFIndex             count, i;
    CFMutableArrayRef   kextsToMessageTrace = NULL; //must release
    OSKextRef           aKext;                      //do not release

    if (kextList && (count = CFArrayGetCount(kextList))) {
        kextsToMessageTrace = CFArrayCreateMutable(kCFAllocatorDefault,
                                                  CFArrayGetCount(kextList),
                                                  &kCFTypeArrayCallBacks);
        if (kextsToMessageTrace) {
            for (i = 0; i < count; i ++) {
                aKext = (OSKextRef)CFArrayGetValueAtIndex(kextList, i);
                filterKextLoadForMT(aKext, kextsToMessageTrace, userLoad);
            }
            if (CFArrayGetCount(kextsToMessageTrace)) {
                postNoteAboutKextLoadsMT(CFSTR("Loaded Kext Notification"),
                                         kextsToMessageTrace);
            }
            SAFE_RELEASE(kextsToMessageTrace);
        }
    }
}

/*******************************************************************************
 * recordKextLoadForMT() - record the loaded kext
 *  <rdar://problem/12435992>
 *******************************************************************************/
void recordKextLoadForMT(OSKextRef aKext, Boolean userLoad)
{
    CFMutableArrayRef myArray = NULL; // must release

    if (!aKext)
        return;

    myArray = CFArrayCreateMutable(kCFAllocatorDefault,
                                   1,
                                   &kCFTypeArrayCallBacks);
    if (myArray) {
        CFArrayAppendValue(myArray, aKext);
        recordKextLoadListForMT(myArray, userLoad);
        SAFE_RELEASE(myArray);
    }
}

/*******************************************************************************
 * checkKextSignature() - check the signature for given kext.
 *******************************************************************************/
OSStatus checkKextSignature(OSKextRef aKext,
                            Boolean checkExceptionList,
                            Boolean allowNetwork)
{
    OSStatus                result          = errSecCSSignatureFailed;
    CFURLRef                kextURL         = NULL;   // must release
    SecStaticCodeRef        staticCodeRef   = NULL;   // must release
    SecRequirementRef       requirementRef  = NULL;   // must release
    CFStringRef             myCFString;
    CFStringRef             requirementsString;
    SecCSFlags              flags = 0;

    if (aKext == NULL) {
        return result;
    }

    kextURL = CFURLCopyAbsoluteURL(OSKextGetURL(aKext));
    if (!kextURL) {
        OSKextLogMemError();
        goto finish;
    }

    if (SecStaticCodeCreateWithPath(kextURL,
                                    kSecCSDefaultFlags,
                                    &staticCodeRef) != errSecSuccess ||
        (staticCodeRef == NULL)) {
        OSKextLogMemError();
        goto finish;
    }

    /* set up correct requirement string.  Apple kexts are signed by B&I while
     * 3rd party kexts are signed through a special developer kext devid
     * program. 3rd party driver extensions have a special entitlement
     * but are otherwise signed like normal app bundles.
     */
    myCFString = OSKextGetIdentifier(aKext);
    if (_OSKextIdentifierHasApplePrefix(aKext)) {
        requirementsString = CFSTR("anchor apple");
    } else if (OSKextDeclaresUserExecutable(aKext)) {
        /* Message-id: <D72A8C77-8212-462E-A4DB-8738FD46455B@apple.com>, Subject: Re: Code requirement for System Extensions seed 1, Date: Mon, 20 May 2019 19:06:09 -0700
         */
        requirementsString = CFSTR("(anchor apple "
                            "or (anchor apple generic and certificate leaf[field.1.2.840.113635.100.6.1.9] exists) "
                            "or (anchor apple generic and certificate 1[field.1.2.840.113635.100.6.2.6] exists "
                                                     "and certificate leaf[field.1.2.840.113635.100.6.1.13] exists and notarized) "
                            "or (anchor apple generic and certificate leaf[field.1.2.840.113635.100.6.1.9.1] exists) "
                            "or (anchor apple generic and certificate leaf[field.1.2.840.113635.100.6.1.12] exists)) "
                            "and entitlement[" DEXT_LAUNCH_ENTITLEMENT "] exists");

    } else {
        /* DevID for kexts cert
         */
        requirementsString =
            CFSTR("anchor apple generic "
                  "and certificate 1[field.1.2.840.113635.100.6.2.6] "
                  "and certificate leaf[field.1.2.840.113635.100.6.1.13] "
                  "and certificate leaf[field.1.2.840.113635.100.6.1.18]" );
    }

    if (SecRequirementCreateWithString(requirementsString,
                                       kSecCSDefaultFlags,
                                       &requirementRef) != errSecSuccess ||
        (requirementRef == NULL)) {
        OSKextLogMemError();
        goto finish;
    }

    // Set up flags based on whether the network is allowed.
    flags = kSecCSCheckAllArchitectures | kSecCSStrictValidate;
    flags |= allowNetwork ? kSecCSEnforceRevocationChecks : kSecCSNoNetworkAccess;

    result = SecStaticCodeCheckValidity(staticCodeRef, flags, requirementRef);

    if ( result != 0 &&
        checkExceptionList ) {
        if (errSecCSWeakResourceEnvelope == result ||
            errSecCSBadObjectFormat == result ||
            errSecCSBadMainExecutable == result) {
            // errSecCSWeakResourceEnvelope == -67007
            // errSecCSBadObjectFormat == -67049
            // errSecCSBadMainExecutable == -67010
            // <rdar://problem/24773482>
            if (isInStrictExceptionList(aKext, kextURL, true)) {
                result = 0;
            }
        }
        else {
            if (isInExceptionList(aKext, kextURL, true)) {
                result = 0;
            }
        }
    }

finish:
    SAFE_RELEASE(kextURL);
    SAFE_RELEASE(staticCodeRef);
    SAFE_RELEASE(requirementRef);

    return result;
}

/*********************************************************************
 * isAllowedToLoadThirdPartyKext check to see if the given kext
 * has been explicitly allowed to load.
 *
 * A third party kext is explicitly allowed to load after the user
 * approves the kext via the SecureKernelExtentionLoading (SKEL)
 * framework, and the machine has rebooted at least once. An early
 * boot task writes a list of valid third party kexts. This list is
 * read via the readKextHashAllowList() function.
 *********************************************************************/
Boolean isAllowedToLoadThirdPartyKext(OSKextRef theKext)
{
    static CFArrayRef       sValidThirdPartyHashes  = NULL;
    static CFArrayRef       sValidThirdPartyBundles = NULL;
    static CFArrayRef       sExceptionListBundles   = NULL;
    static bool             sLoadedThirdPartyKextList = false;

    Boolean                 result = FALSE;
    CFStringRef             myCFString;
    CFDataRef               cdhash = NULL; // do not release
    CFStringRef             theKextBundleID = NULL; // do not release
    CFURLRef                theKextURL = NULL; // must release
    SecStaticCodeRef        staticCodeRef = NULL; // must release
    CFDictionaryRef         signingInfo = NULL; // must release
    bool                    kextIsDarwinupInstalled = false;

    myCFString = OSKextGetIdentifier(theKext);
    if (_OSKextIdentifierHasApplePrefix(theKext)) {
        /* Apple kexts are always allowed to load: early-out */
        return TRUE;
    }

    if (!OSKextDeclaresExecutable(theKext) || OSKextDeclaresUserExecutable(theKext)) {
        /* codeless kexts (and dexts) don't require a reboot */
        return TRUE;
    }

    /*
     * Load the list of allowed third party kexts (once)
     */
    if (!sLoadedThirdPartyKextList) {
        bool rc = readKextHashAllowList(true, NULL, &sValidThirdPartyHashes, &sValidThirdPartyBundles, &sExceptionListBundles);
        if (!rc) {
            sValidThirdPartyHashes  = CFArrayCreate(kCFAllocatorDefault, NULL, 0, &kCFTypeArrayCallBacks);
            sValidThirdPartyBundles = CFArrayCreate(kCFAllocatorDefault, NULL, 0, &kCFTypeArrayCallBacks);
            sExceptionListBundles   = CFArrayCreate(kCFAllocatorDefault, NULL, 0, &kCFTypeArrayCallBacks);
        }
        sLoadedThirdPartyKextList = true;
    }
    if (!sValidThirdPartyHashes || !sValidThirdPartyBundles || !sExceptionListBundles) {
        OSKextLogCFString(theKext,
                          kOSKextLogErrorLevel | kOSKextLogAuthenticationFlag | kOSKextLogGeneralFlag,
                          CFSTR("Cannot evaluate third party kext loads to to error reading allowed list."));
        goto out;
    }


    theKextURL = CFURLCopyAbsoluteURL(OSKextGetURL(theKext));
    if (!theKextURL) {
        OSKextLogMemError();
        goto out;
    }

    if (csr_check(CSR_ALLOW_APPLE_INTERNAL) == 0) {
        /*
         * On AppleInternal systems, check for an xattr on the kext to see
         * if it was installed via darwinup. If so, allow the load despite
         * any errors that might occur.
         */
        ssize_t ret;
        char buf[32] = {};
        char kextPath[PATH_MAX] = {};
        CFURLRef unstagedURL = copyUnstagedKextURL(theKextURL);
        if (!CFURLGetFileSystemRepresentation(unstagedURL, /* resolveToBase */ true,
                    (UInt8 *)kextPath, sizeof(kextPath))) {
            OSKextLogMemError();
            goto out;
        }
        SAFE_RELEASE(unstagedURL);

        ret = getxattr(kextPath, "com.apple.root.installed", buf, sizeof(buf), 0, 0);
        if (ret > 0) {
            /* the presence of this xattr is enough to indicate that the path has been blessed. */
            kextIsDarwinupInstalled = true;
        }
    }

    if (SecStaticCodeCreateWithPath(theKextURL,
                                    kSecCSDefaultFlags,
                                    &staticCodeRef) != errSecSuccess ||
        (staticCodeRef == NULL)) {
        OSKextLogMemError();
        goto out;
    }

    SecCodeCopySigningInformation(staticCodeRef,
                                  kSecCSDefaultFlags,
                                  &signingInfo);
    if (!signingInfo) {
        OSKextLogMemError();
        goto out;
    }

    theKextBundleID = OSKextGetValueForInfoDictionaryKey(theKext,
                                                         kCFBundleIdentifierKey);

    cdhash = CFDictionaryGetValue(signingInfo, kSecCodeInfoUnique);
    if (!cdhash || (CFGetTypeID(cdhash) != CFDataGetTypeID())) {
        /* check the kext exception list as syspolicyd saw it at boot */
        for (int idx = 0; idx < CFArrayGetCount(sExceptionListBundles); idx++) {
            CFStringRef validBundle = (CFStringRef)CFArrayGetValueAtIndex(sExceptionListBundles, idx);
            if (CFStringCompare(validBundle, theKextBundleID, kCFCompareCaseInsensitive) == kCFCompareEqualTo) {
                result = TRUE;
                OSKextLogCFString(theKext,
                                  kOSKextLogBasicLevel | kOSKextLogAuthenticationFlag | kOSKextLogGeneralFlag,
                                  CFSTR("kext: %@ is in explicit exception list: allowing load"), theKext);
                goto out;
            }
        }
        /* if we didn't see it at boot, but it's in the exception list, then we require a reboot */
        if (isInExceptionList(theKext, theKextURL, TRUE)) {
            OSKextLogCFString(theKext,
                              kOSKextLogErrorLevel | kOSKextLogAuthenticationFlag | kOSKextLogGeneralFlag,
                              CFSTR("%@ is in the exception list, but requires a reboot before loading."), theKext);
        } else {
            OSKextLogCFString(theKext,
                              kOSKextLogErrorLevel | kOSKextLogAuthenticationFlag | kOSKextLogGeneralFlag,
                              CFSTR("%@ is missing or has an invalid CDHash. Disallowing load."), theKext);
        }
        goto out;
    }

    const UInt8 *cdhash_ptr = CFDataGetBytePtr(cdhash);
    const CFIndex cdhash_len = CFDataGetLength(cdhash);
    /* check that that this 3rd party kext is allowed to load (by cdhash) */
    for (int idx = 0; idx < CFArrayGetCount(sValidThirdPartyHashes); idx++) {
        CFDataRef validhash = (CFDataRef)CFArrayGetValueAtIndex(sValidThirdPartyHashes, idx);
        if (cdhash_len != CFDataGetLength(validhash)) {
            continue;
        }
        const UInt8 *vptr = CFDataGetBytePtr(validhash);
        if (memcmp(cdhash_ptr, vptr, (size_t)cdhash_len) == 0) {
            result = TRUE;
            OSKextLogCFString(theKext,
                              kOSKextLogBasicLevel | kOSKextLogAuthenticationFlag | kOSKextLogGeneralFlag,
                              CFSTR("kext: %@ is in allowed list of cdhashes"), theKext);
            goto out;
        }
    }

    /* check that that this 3rd party kext is allowed to load (by bundleID) */
    for (int idx = 0; idx < CFArrayGetCount(sValidThirdPartyBundles); idx++) {
        CFStringRef validBundle = (CFStringRef)CFArrayGetValueAtIndex(sValidThirdPartyBundles, idx);
        if (CFStringCompare(validBundle, theKextBundleID, kCFCompareCaseInsensitive) == kCFCompareEqualTo) {
            result = TRUE;
            OSKextLogCFString(theKext,
                              kOSKextLogBasicLevel | kOSKextLogAuthenticationFlag | kOSKextLogGeneralFlag,
                              CFSTR("kext: %@ is in allowed list of bundleIDs"), theKext);
            goto out;
        }
    }

    OSKextLogCFString(theKext,
                      kOSKextLogErrorLevel | kOSKextLogAuthenticationFlag | kOSKextLogGeneralFlag,
                      CFSTR("Could not find %@ (bundleID:%@ cdhash:%@) in list of %lu/%lu allowed 3rd party kexts."),
                      theKext, theKextBundleID, cdhash,
                      CFArrayGetCount(sValidThirdPartyHashes),
                      CFArrayGetCount(sValidThirdPartyBundles));
    OSKextLogCFString(theKext,
                      kOSKextLogErrorLevel | kOSKextLogAuthenticationFlag | kOSKextLogGeneralFlag,
                      CFSTR("If this kext was just installed, a reboot is required to allow it to load."));

 out:
    if (result != TRUE && (kextIsDarwinupInstalled || (csr_check(CSR_ALLOW_UNTRUSTED_KEXTS) == 0))) {
        /*
         * Allow all 3rd party kexts to load if we're on a SIP disabled machine, or
         * if this kext was installed via darwinup
         */
        if (kextIsDarwinupInstalled) {
            OSKextLogCFString(theKext,
                              kOSKextLogBasicLevel | kOSKextLogAuthenticationFlag | kOSKextLogGeneralFlag,
                              CFSTR("Allowing 3rd party / unsigned kext installed via darwinup: %@"), theKext);
        } else {
            OSKextLogCFString(theKext,
                              kOSKextLogBasicLevel | kOSKextLogAuthenticationFlag | kOSKextLogGeneralFlag,
                              CFSTR("Allowing 3rd party kext w/o reboot: %@"), theKext);
        }
        result = TRUE;
    }

    SAFE_RELEASE(theKextURL);
    SAFE_RELEASE(signingInfo);
    SAFE_RELEASE(staticCodeRef);

    return result;
}


/*********************************************************************
 * isInExceptionList checks to see if the given kext is in the
 * kext signing exception list (in com.apple.driver.KextExcludeList).
 * If useCache is TRUE, we will use the cached copy of the exception list.
 * If useCache is FALSE, we will refresh the cache from disk.
 *
 * The kext signing exception list rarely changes but to insure you have the
 * most recent copy in the cache pass FALSE for the first call and TRUE for
 * subsequent calls (when dealing with a large list of kexts).
 * theKext and theKextURL can be NULL if you just want to invalidate the cache.
 *********************************************************************/
Boolean isInExceptionList(OSKextRef theKext,
                          CFURLRef  theKextURL,
                          Boolean   useCache)
{
    Boolean             result                      = false;
    CFURLRef            kextURL                     = NULL; // must release
    CFStringRef         kextID                      = NULL; // must release
    OSKextRef           excludelistKext             = NULL; // must release
    CFDictionaryRef     tempDict                    = NULL; // do NOT release
    static CFDictionaryRef sExceptionHashListDict   = NULL; // do NOT release

    /* invalidate the exception list "by hash" cache or create if not
     * present
     */
    if (useCache == false || sExceptionHashListDict == NULL) {
        if (sExceptionHashListDict) {
            SAFE_RELEASE_NULL(sExceptionHashListDict);
        }
        kextID = CFStringCreateWithCString(kCFAllocatorDefault,
                                           "com.apple.driver.KextExcludeList",
                                           kCFStringEncodingUTF8);
        if (kextID == NULL) {
            OSKextLogStringError(/* kext */ NULL);
            goto finish;
        }

        excludelistKext = OSKextCreateWithIdentifier(kCFAllocatorDefault,
                                                     kextID);
        if (excludelistKext == NULL) {
            goto finish;
        }

        /* can we trust AppleKextExcludeList.kext?
         * If we are NOT allowing untrusted kexts then make sure
         * AppleKextExcludeList.kext is valid!
         */
        if (csr_check(CSR_ALLOW_UNTRUSTED_KEXTS) != 0) {
            if (checkKextSignature(excludelistKext, false, false) != 0) {
                char kextPath[PATH_MAX];

                if (!CFURLGetFileSystemRepresentation(OSKextGetURL(excludelistKext),
                                                      false,
                                                      (UInt8 *)kextPath,
                                                      sizeof(kextPath))) {
                    strlcpy(kextPath, "(unknown)", sizeof(kextPath));
                }
                OSKextLog(/* kext */ NULL,
                          kOSKextLogErrorLevel | kOSKextLogArchiveFlag |
                          kOSKextLogAuthenticationFlag | kOSKextLogGeneralFlag,
                          "%s has invalid signature; Trust cache is disabled.",
                          kextPath);
                goto finish;
            }
        }

        tempDict = OSKextGetValueForInfoDictionaryKey(
                                        excludelistKext,
                                        CFSTR("OSKextSigExceptionHashList") );
        if (tempDict) {
            if ((unsigned int)CFDictionaryGetCount(tempDict) > 0) {
                sExceptionHashListDict = CFDictionaryCreateCopy(NULL, tempDict);
            }
        }
    }

    /* Passing both arguments as NULL is just a way to prime the cache (above),
     * so there's no more work to do.
     */
    if (theKext == NULL && theKextURL == NULL) {
        goto finish;
    }

    if (sExceptionHashListDict) {
        if (theKextURL == NULL) {
            kextURL = CFURLCopyAbsoluteURL(OSKextGetURL(theKext));
            if (kextURL == NULL) {
                OSKextLogMemError();
                goto finish;
            }
            theKextURL = kextURL;
        }
        if (hashIsInExceptionList(theKextURL, sExceptionHashListDict, NULL)) {
            result = true;
            goto finish;
        }
    }

finish:
    SAFE_RELEASE(kextURL);
    SAFE_RELEASE(kextID);
    SAFE_RELEASE(excludelistKext);
    return result;
}


/* Need to check an additional exception list when the signature check
 * failure is due to strict validation errors.
 * Error: 0xFFFEFA41 -67007 resource envelope is obsolete (version 1 signature)
 * <rdar://problem/24773482>
 */
Boolean isInStrictExceptionList(OSKextRef theKext,
                                CFURLRef  theKextURL,
                                Boolean   useCache)
{
    Boolean             result                      = false;
    CFURLRef            kextURL                     = NULL; // must release
    CFStringRef         kextID                      = NULL; // must release
    OSKextRef           excludelistKext             = NULL; // must release
    CFDictionaryRef     tempDict                    = NULL; // do NOT release
    CFMutableDictionaryRef attributes               = NULL; // must release
    static CFDictionaryRef sStrictExceptionHashListDict = NULL; // do NOT release
    const NXArchInfo   *targetArch                  = NULL;  // do not free
    CFStringRef         archName                    = NULL; // must release

    // For strict validation, the exception list has an entry for each kext and each architecture
    // it is intended to cover so that we only have to check the cdhash of the currently
    // running kernel architecture that will be loaded.
    // <rdar://problem/27010141>
    attributes = CFDictionaryCreateMutable(kCFAllocatorDefault,
                                           1,
                                           &kCFTypeDictionaryKeyCallBacks,
                                           &kCFTypeDictionaryValueCallBacks);
    if (!attributes) {
        OSKextLogMemError();
        goto finish;
    }

    targetArch = OSKextGetRunningKernelArchitecture();
    if (!targetArch) {
        OSKextLogCFString(theKext,
            kOSKextLogErrorLevel | kOSKextLogArchiveFlag |
            kOSKextLogAuthenticationFlag | kOSKextLogGeneralFlag,
            CFSTR("Loading kext with identifier %@ failed retrieving running kernel architecture."),
            OSKextGetIdentifier(theKext));
        goto finish;
    }

    archName = CFStringCreateWithCString(NULL, targetArch->name, kCFStringEncodingASCII);
    if (!archName) {
        OSKextLogMemError();
        goto finish;
    }

    CFDictionaryAddValue(attributes, kSecCodeAttributeArchitecture, archName);

    /* invalidate the exception list "by hash" cache or create if not
     * present
     */
    if (useCache == false || sStrictExceptionHashListDict == NULL) {
        if (sStrictExceptionHashListDict) {
            SAFE_RELEASE_NULL(sStrictExceptionHashListDict);
        }
        kextID = CFStringCreateWithCString(kCFAllocatorDefault,
                                           "com.apple.driver.KextExcludeList",
                                           kCFStringEncodingUTF8);
        if (kextID == NULL) {
            OSKextLogStringError(/* kext */ NULL);
            goto finish;
        }

        excludelistKext = OSKextCreateWithIdentifier(kCFAllocatorDefault,
                                                     kextID);
        if (excludelistKext == NULL) {
            goto finish;
        }

        /* can we trust AppleKextExcludeList.kext?
         * If we are NOT allowing untrusted kexts then make sure
         * AppleKextExcludeList.kext is valid!
         */
        if (csr_check(CSR_ALLOW_UNTRUSTED_KEXTS) != 0) {
            if (checkKextSignature(excludelistKext, false, false) != 0) {
                char kextPath[PATH_MAX];

                if (!CFURLGetFileSystemRepresentation(OSKextGetURL(excludelistKext),
                                                      false,
                                                      (UInt8 *)kextPath,
                                                      sizeof(kextPath))) {
                    strlcpy(kextPath, "(unknown)", sizeof(kextPath));
                }
                OSKextLog(/* kext */ NULL,
                          kOSKextLogErrorLevel | kOSKextLogArchiveFlag |
                          kOSKextLogAuthenticationFlag | kOSKextLogGeneralFlag,
                          "%s has invalid signature; Trust cache is disabled.",
                          kextPath);
                goto finish;
            }
        }

        tempDict = OSKextGetValueForInfoDictionaryKey(
                                                      excludelistKext,
                                                      CFSTR("OSKextStrictExceptionHashList") );
        if (tempDict) {
            if ((unsigned int)CFDictionaryGetCount(tempDict) > 0) {
                sStrictExceptionHashListDict = CFDictionaryCreateCopy(NULL, tempDict);
            }
        }
    }

    /* Passing both arguments as NULL is just a way to prime the cache (above),
     * so there's no more work to do.
     */
    if (theKext == NULL && theKextURL == NULL) {
        goto finish;
    }

    if (sStrictExceptionHashListDict) {
        if (theKextURL == NULL) {
            kextURL = CFURLCopyAbsoluteURL(OSKextGetURL(theKext));
            if (kextURL == NULL) {
                OSKextLogMemError();
                goto finish;
            }
            theKextURL = kextURL;
        }
        if (hashIsInExceptionList(theKextURL, sStrictExceptionHashListDict, attributes)) {
            result = true;
            goto finish;
        } else {
            char kextPath[PATH_MAX];
            if (!CFURLGetFileSystemRepresentation(OSKextGetURL(theKext),
                                                  false, (UInt8 *)kextPath, sizeof(kextPath))) {
                strlcpy(kextPath, "(unknown)", sizeof(kextPath));
            }
            OSKextLog(theKext,
                      kOSKextLogErrorLevel | kOSKextLogArchiveFlag |
                      kOSKextLogAuthenticationFlag | kOSKextLogGeneralFlag,
                      "%s does not appear in strict exception list for architecture: %s",
                      kextPath, targetArch->name);
        }
    }

finish:
    SAFE_RELEASE(archName);
    SAFE_RELEASE(attributes);
    SAFE_RELEASE(kextURL);
    SAFE_RELEASE(kextID);
    SAFE_RELEASE(excludelistKext);
    return result;
}


/*********************************************************************
 * theDict is a dictionary with keys / values of:
 *  key = message tracing hash of kext
 *  value = dictionary containing the bundle ID and version of the kext
 *      the hash key represents
 *
 * Example OSKextSigExceptionHashList from AppleKextExcludeList.kext Info.plist:
 *
 * <key>OSKextSigExceptionHashList</key>
 * <dict>
 *      <key>3205773049fb43b4a54cafc8493aa19217fbae7a</key>
 *      <dict>
 *          <key>CFBundleIdentifier</key>
 *          <string>com.apple.driver.AppleMobileDevice</string>
 *          <key>CFBundleVersion</key>
 *          <string>3.3.0</string>
 *      </dict>
 * </dict>
 *
 *
 * codesignAttributes is a dictionary of codesigning attributes to pass in to
 * SecStaticCodeCreateWithPathAndAttributes that controls exactly how the
 * hash is generated.
 *********************************************************************/

static Boolean hashIsInExceptionList(CFURLRef           theKextURL,
                                     CFDictionaryRef    theDict,
                                     CFDictionaryRef    codesignAttributes)
{
    Boolean         result              = false;
    char *          hashCString         = NULL;     // must free
    CFStringRef     hashString          = NULL;     // must release
    CFStringRef     kextInfoString      = NULL;     // do NOT release

    if (theKextURL == NULL) {
        goto finish;
    }

    /* generate the hash for the kext to look up in exception list */
    getAdhocSignatureHash(theKextURL, &hashCString, codesignAttributes);
    if (hashCString == NULL) {
        goto finish;
    }
    hashString = CFStringCreateWithCString(kCFAllocatorDefault,
                                           hashCString,
                                           kCFStringEncodingUTF8);
    if (hashString == NULL) {
        OSKextLogMemError();
        goto finish;
    }

    kextInfoString = CFDictionaryGetValue(theDict, hashString);
    if (kextInfoString == NULL ||
        CFGetTypeID(kextInfoString) != CFStringGetTypeID()) {
        goto finish;
    }

    OSKextLogCFString(NULL,
                      kOSKextLogGeneralFlag | kOSKextLogErrorLevel,
                      CFSTR("kext %@ is in hash exception list, allowing to load"),
                      theKextURL);
    result = true;

finish:
    SAFE_RELEASE(hashString);
    SAFE_FREE(hashCString);

    return result;
}

#define KEXT_DEV_MODE_STRING "kext-dev-mode"

enum {
    // ignore kext signature check failures
    kKextDevModeFlagsIgnoreKextSigFailures  =   0x00000001ULL,
    // disable auto rebuilding of prelinked kernel, NOTE if this is set
    // the prelinked kernel must be explicitly rebuilt via the kextcache
    // command line tool.  touch /S/L/E or any other "automatic triggers" will
    // not rebuild prelinked kernel
    kKextDevModeFlagsDisableAutoRebuild     =   0x00000002ULL,
};

#if 0 // obsolete with 19635687
/*******************************************************************************
 * isDevMode() - check to see if this machine is in "kext developer mode"
 *******************************************************************************/
Boolean isDevMode(void)
{
    uint64_t    kextDevModeFlags;

    kextDevModeFlags = getKextDevModeFlags();

    return(kextDevModeFlags & kKextDevModeFlagsIgnoreKextSigFailures);
}
#endif

Boolean isPrelinkedKernelAutoRebuildDisabled(void)
{
    uint32_t    kextDevModeFlags = 0x00;

    /* only skip auto rebuild if kernel debugging allowed */
    if (csr_check(CSR_ALLOW_UNTRUSTED_KEXTS) == 0) {
        kextDevModeFlags = getKextDevModeFlags();
    }
    return(kextDevModeFlags & kKextDevModeFlagsDisableAutoRebuild);
}

static uint32_t getKextDevModeFlags(void)
{
    uint32_t    kext_dev_mode = 0;

    (void)get_bootarg_int(KEXT_DEV_MODE_STRING, &kext_dev_mode);
    return kext_dev_mode;
}

/*********************************************************************
 *********************************************************************/
Boolean isInLibraryExtensionsFolder(OSKextRef theKext)
{
    CFStringRef     myKextPath = NULL; // must release
    Boolean         myResult = false;

    myKextPath = copyKextPath(theKext);
    if ( myKextPath ) {
        if ( CFStringHasPrefix(myKextPath,
                               CFSTR(_kOSKextLibraryExtensionsFolder)) ) {
            myResult = true;
        }
    }
    SAFE_RELEASE(myKextPath);
    return(myResult);
}

/*********************************************************************
 *********************************************************************/
Boolean isInSystemLibraryExtensionsFolder(OSKextRef theKext)
{
    CFStringRef     myKextPath = NULL; // must release
    Boolean         myResult = false;

    myKextPath = copyKextPath(theKext);
    if ( myKextPath ) {
        if ( CFStringHasPrefix(myKextPath,
                               CFSTR(_kOSKextSystemLibraryExtensionsFolder)) ) {
            myResult = true;
        }
    }
    SAFE_RELEASE(myKextPath);
    return(myResult);
}

/*******************************************************************************
 * isInvalidSignatureAllowed() - check if kext with invalid signature is
 * allowed to load.  Currently we check to see if we are running with boot-args
 * including "kext-dev-mode".  In the future this is likely be removed or
 * changed to use other methods to set up machines in "developer mode".
 *******************************************************************************/
Boolean isInvalidSignatureAllowed(void)
{
    Boolean      result = false;      // default to not allowed

    if (csr_check(CSR_ALLOW_UNTRUSTED_KEXTS) == 0 || csr_check(CSR_ALLOW_APPLE_INTERNAL) == 0) {
        // Allow kext signature check errors
        result = true;
    }
    else {
        // Do not allow kext signature check errors
        OSKextLog(/* kext */ NULL,
                  kOSKextLogErrorLevel  | kOSKextLogGeneralFlag,
                  "Untrusted kexts are not allowed");
    }

    return(result);
}
#include <Security/SecKeychainPriv.h>

/* If kextd isn't running, assume it's early boot and that securityd
 * isn't running either.  Configure Security to avoid securityd.
 * SecKeychainMDSInstall needs to be called once before any kext signatures
 * are checked.
 */
int callSecKeychainMDSInstall( void )
{
    static int  calledOnce = 0;
    static int  result = 0;

    if (calledOnce)  return(result);

    calledOnce++;
    if (isKextdRunning() == FALSE) {
        OSStatus    err;

        err = SecKeychainMDSInstall();
        if (err != errSecSuccess) {
            OSKextLog(/* kext */ NULL,
                      kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                      "Error in security framework, error %d.", (int) err);
            result = err;
            //result = EX_SOFTWARE;
        }
    }
    return(result);
}

#include <servers/bootstrap.h>    // bootstrap mach ports
/*******************************************************************************
 * isKextdRunning() - we use this to tell if we're running before kextd is up.
 *******************************************************************************/
Boolean isKextdRunning(void)
{
    mach_port_t     kextd_port = MACH_PORT_NULL;
    kern_return_t   kern_result = 0;

    kern_result = bootstrap_look_up(bootstrap_port,
                                    (char *)KEXTD_SERVER_NAME,
                                    &kextd_port);
    if (kern_result == kOSReturnSuccess && kextd_port != MACH_PORT_NULL) {
        return( TRUE );
    }

    return( FALSE );
}

Boolean isNetBooted(void)
{
    Boolean isNetBooted = FALSE;
    uint32_t isNetBootInt = 0;
    size_t size = sizeof(isNetBootInt);
    int error = 0;

    error = sysctlbyname("kern.netboot", (void*)&isNetBootInt, &size, NULL, 0);
    if (error != 0)
    {
        OSKextLog(/* kext */ NULL,
                  kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                  "Unable to read netboot sysctl: %d", error);
        isNetBooted = FALSE;
        goto __out;
    }

    isNetBooted = isNetBootInt != 0;

__out:
    return isNetBooted;
}

#if HAVE_DANGERZONE

/*******************************************************************************
 * copyKextPathToBuffer - Helper function to copy a kext path into a provided buffer.
 *******************************************************************************/
void copyKextPathToBuffer(OSKextRef kext, char *buffer, size_t buffer_size) {
    CFStringRef kextPath = copyKextPath(kext);
    if (!kextPath) {
        goto __out;
    }
    if (!CFStringGetCString(kextPath, buffer, buffer_size, kCFStringEncodingUTF8)) {
        OSKextLog(/* kext */ NULL,
                  kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                  "failed to copy kext path");
        goto __out;
    }
__out:
    SAFE_RELEASE(kextPath);
}

/*******************************************************************************
 * dzRecordKextLoad* - Helper functions to notify the DangerZone subsystem about
 * various types of kext load scenarios.
 *******************************************************************************/
void dzRecordKextLoadUser(OSKextRef kext, bool allowed) {
    if (dz_notify_kext_load_v2 == NULL) {
        // libdz is weak linked for use in old OS environments.
        return;
    }
    char localKextPath[PATH_MAX] = {0};
    copyKextPathToBuffer(kext, localKextPath, sizeof(localKextPath));
    dz_notify_kext_load_v2(DZ_KEXT_LOAD_KEXTD_USER, localKextPath, DZ_KEXT_LOAD_FLAG_NONE, allowed);
}

void dzRecordKextLoadKernel(OSKextRef kext, bool allowed) {
    if (dz_notify_kext_load_v2 == NULL) {
        // libdz is weak linked for use in old OS environments.
        return;
    }
    char localKextPath[PATH_MAX] = {0};
    copyKextPathToBuffer(kext, localKextPath, sizeof(localKextPath));

    dz_notify_kext_load_v2(DZ_KEXT_LOAD_KEXTD_KERNEL, localKextPath, DZ_KEXT_LOAD_FLAG_NONE, allowed);
}

void dzRecordKextLoadBypass(OSKextRef kext, bool allowed) {
    // Note: dzRecordKextLoadBypass will only be called for kexts that are allowed to load. The current code structure
    // is difficult to instrument for denials, and in the future shouldn't be a code path that exists for non-early
    // boot scenarios.
    if (dz_notify_kext_load_v2 == NULL) {
        // libdz is weak linked for use in old OS environments.
        return;
    }
    char localKextPath[PATH_MAX] = {0};
    copyKextPathToBuffer(kext, localKextPath, sizeof(localKextPath));
    dz_notify_kext_load_v2(DZ_KEXT_LOAD_KEXTD_BYPASS, localKextPath, DZ_KEXT_LOAD_FLAG_NONE, allowed);
}

/*******************************************************************************
 * dzRecordKextCacheAdd - Helper function to notify the Danger Zone subsystem
 * of the inclusion of a kext in a kext cache.
 *******************************************************************************/
void dzRecordKextCacheAdd(OSKextRef kext, bool allowed) {
    if (dz_notify_kextcache_update_v2 == NULL) {
        // libdz is weak linked for use in old OS environments.
        return;
    }
    char localKextPath[PATH_MAX] = {0};
    copyKextPathToBuffer(kext, localKextPath, sizeof(localKextPath));
    dz_notify_kextcache_update_v2(localKextPath, allowed);
}

#endif // HAVE_DANGERZONE
