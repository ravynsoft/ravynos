/*
 *  security.h
 *  kext_tools
 *
 *  Copyright 2019 Apple Inc. All rights reserved.
 *
 */
#ifndef _SECURITY_H
#define _SECURITY_H

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/kext/OSKext.h>
#include <mach/mach_error.h>

//  <rdar://problem/12435992>
#include <asl.h>
#include <Security/SecCode.h>
#include <Security/SecCodeSigner.h>
#include <Security/SecStaticCode.h>
#include <Security/SecRequirement.h>
#include <Security/SecRequirementPriv.h>
#include <Security/SecCodePriv.h>

#define kMessageTracerDomainKey     "com.apple.message.domain"
#define kMessageTracerHashKey       "com.apple.message.hash"
#define kMessageTracerBundleIDKey   "com.apple.message.bundleID"
#define kMessageTracerVersionKey    "com.apple.message.version"
#define kMessageTracerKextNameKey   "com.apple.message.kextname"
#define kMessageTracerFatKey        "com.apple.message.fat"
#define kMessageTracerArchKey       "com.apple.message.architecture"

#define kMessageTracerTeamIdKey     "com.apple.message.teamid"
#define kMessageTracerSubjectCNKey  "com.apple.message.subjectcn"
#define kMessageTracerIssuerCNKey   "com.apple.message.issuercn"

#define kMessageTracerSignatureTypeKey "com.apple.message.signaturetype"
#define kMessageTracerPathKey       "com.apple.message.kextpath"
#define kMessageTracerExecPathKey   "com.apple.message.kextexecpath"
#define kMessageTracerCodelessKey   "com.apple.message.codeless"
#define kMessageTracerPersonalityNamesKey   "com.apple.message.personalitynames"
#define kMessageTracerSigningTimeKey   "com.apple.message.signingtime"
#define kMessageTracerUserLoadKey      "com.apple.message.userload"

#define kAppleKextWithAppleRoot \
"Apple kext with Apple root"
#define k3rdPartyKextWithAppleRoot \
"3rd-party kext with Apple root"
#define k3rdPartyKextWithoutAppleRoot \
"3rd-party kext without Apple root"
#define k3rdPartyKextWithDevIdPlus \
"3rd-party kext with devid+ certificate"
#define k3rdPartyKextWithRevokedDevIdPlus \
"3rd-party kext with revoked devid+ certificate"
#define kUnsignedKext \
"Unsigned kext"

/* "com.apple.libkext.kext.loading" was used in 10.8
 * "com.apple.libkext.kext.loading.v4"  is used in LoboFox+ */
#define kMTKextLoadingDomain        "com.apple.libkext.kext.loading.v4"
#define kMTKextBlockedDomain        "com.apple.libkext.kext.blocked"

#define GET_CSTRING_PTR(the_cfstring, the_ptr, the_buffer, the_size) \
do { \
	the_ptr = CFStringGetCStringPtr(the_cfstring, kCFStringEncodingUTF8); \
	if (the_ptr == NULL) { \
		the_buffer[0] = 0x00; \
		the_ptr = the_buffer;  \
		CFStringGetCString(the_cfstring, the_buffer, the_size, kCFStringEncodingUTF8); \
	} \
} while(0)

void    messageTraceExcludedKext(OSKextRef aKext);
void    recordKextLoadListForMT(CFArrayRef kextList, Boolean userLoad);
void    recordKextLoadForMT(OSKextRef aKext, Boolean userLoad);

OSStatus checkKextSignature(OSKextRef aKext,
                            Boolean checkExceptionList,
                            Boolean allowNetwork);
Boolean checkEntitlementAtURL(CFURLRef anURL, CFStringRef entitlementString, Boolean allowNetwork);
Boolean isAllowedToLoadThirdPartyKext(OSKextRef theKext);
Boolean isInExceptionList(OSKextRef theKext, CFURLRef theKextURL, Boolean useCache);
Boolean isInStrictExceptionList(OSKextRef theKext, CFURLRef theKextURL, Boolean useCache);
Boolean isInLibraryExtensionsFolder(OSKextRef theKext);
Boolean isInSystemLibraryExtensionsFolder(OSKextRef theKext);
Boolean isPrelinkedKernelAutoRebuildDisabled(void);
Boolean isInvalidSignatureAllowed(void);
Boolean isKextdRunning(void);
int callSecKeychainMDSInstall( void );

CFStringRef copyCDHashFromURL(CFURLRef anURL);
void copySigningInfo(CFURLRef kextURL, CFStringRef* cdhash, CFStringRef* teamId,
                        CFStringRef* subjectCN, CFStringRef* issuerCN);
void getAdhocSignatureHash(CFURLRef kextURL, char ** signatureBuffer, CFDictionaryRef codesignAttributes);

Boolean isNetBooted(void);

// A set of authentication options for use with the global authentication function.
typedef struct AuthOptions {
    Boolean allowNetwork;
    Boolean isCacheLoad;
    Boolean performFilesystemValidation;
    Boolean performSignatureValidation;
    Boolean requireSecureLocation;
    Boolean respectSystemPolicy;
    Boolean checkDextApproval;
    Boolean is_kextcache;
} AuthOptions_t;

// context is expected to be a pointer to an AuthOptions_t structure.
Boolean authenticateKext(OSKextRef theKext, void *context);

#if !TARGET_OS_EMBEDDED
#include <dz/dz.h>
#if DZ_API_VERSION >= 20170214
#define HAVE_DANGERZONE 1
#endif // DZ_API_VERSION
#endif // !TARGET_OS_EMBEDDED

#if HAVE_DANGERZONE
void dzRecordKextLoadUser(OSKextRef kext, bool allowed);
void dzRecordKextLoadKernel(OSKextRef kext, bool allowed);
void dzRecordKextLoadBypass(OSKextRef kext, bool allowed);
void dzRecordKextCacheAdd(OSKextRef kext, bool allowed);
#endif

#endif // _SECURITY_H
