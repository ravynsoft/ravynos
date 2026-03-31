/*
 * kextaudit.c - code for forwarding kext load information to bridgeOS over
 *               SMC to allow for certain system policy decisions.
 *
 * TODO: Move DangerZone code into this file since it does very similar things.
 *
 * Copyright (c) 2018 Apple Computer, Inc. All rights reserved.
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

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include <CommonCrypto/CommonDigest.h>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/kext/OSKext.h>
#include <IOKit/kext/OSKextPrivate.h>
#include <MultiverseSupport/kext_audit_plugin_common.h>
#include <sys/csr.h>

#include "security.h"
#include "kext_tools_util.h"
#include "signposts.h"

#define kSha1HexDigestSize (CC_SHA1_DIGEST_LENGTH * 2 + 1)
#define kSha1ZeroHash      "0000000000000000000000000000000000000000"

static bool         gKextAuditIsInitialized = false;
static bool         gShouldAuditKexts = true;
static io_connect_t gKextAuditIOConn;

/* KextAuditMakeKALNFromInfo
 *
 * Summary:
 *
 * Given information about a kext, construct a kext load notification containing
 * the cdhash, truncated SHA-1 hash of the bundleID, teamID, and version string.
 *
 * Arguments: The pointer to the buffer, and the signing information for the kext.
 *
 * Returns: Success condition.
 */
static bool KextAuditMakeKALNFromInfo(struct KextAuditLoadNotificationKext *kaln, OSKextVersion version,
                                      CFStringRef cdHashCFString, CFStringRef bundleIDCFString,
                                      CFStringRef teamIDCFString)
{
	bool result  = true;
	enum KextAuditLoadType loadType;

	const char *cdHash, *teamID, *bundleID;
	size_t     cdHashLen, bundleIDLen;

	char versionString[kKALNKKextVersionSize] = { };
	char rawBundleHash[CC_SHA256_DIGEST_LENGTH]   = { };
	char rawCDHash[kKALNKCDHashSize]             = { };
	CFIndex cdHashBufLen, bundleIDBufLen, teamIDBufLen;
	char *cdHashBuffer = NULL;
	char *bundleIDBuffer = NULL;
	char *teamIDBuffer = NULL;

	cdHashBufLen = CFStringGetMaximumSizeForEncoding(CFStringGetLength(cdHashCFString) + 1, kCFStringEncodingUTF8);
	cdHashBuffer = malloc(cdHashBufLen);
	if (!cdHashBuffer) {
		OSKextLog(/* kext */ NULL,
			  kOSKextLogErrorLevel | kOSKextLogLoadFlag,
			  "Impossible to malloc cdHashBuffer %zu bytes", cdHashBufLen);
		result = false;
		goto error;
	}
	GET_CSTRING_PTR(cdHashCFString, cdHash, cdHashBuffer, cdHashBufLen);
	if (!cdHash) {
		OSKextLog(/* kext */ NULL,
		          kOSKextLogErrorLevel | kOSKextLogLoadFlag,
		          "Could not get CDHash string");
		result = false;
		goto error;
	}
	cdHashLen = strlen(cdHash);

	/* Since the load notifications contain the raw cdHash instead of a
	 * string, we need a way to tell listeners how long the hash is. */
	if (cdHashLen == 40) {
		loadType = kKALTKextCDHashSha1;
	} else if (cdHashLen == 64) {
		loadType = kKALTKextCDHashSha256;
	} else {
		OSKextLog(/* kext */ NULL,
		          kOSKextLogErrorLevel | kOSKextLogLoadFlag,
		          "Kext had a strange hash: %s, length %zu", cdHash, cdHashLen);
		result = false;
		goto error;
	}

	result = createRawBytesFromHexString(&rawCDHash[0], kKALNKCDHashSize, cdHash, cdHashLen);
	if (!result) {
		OSKextLog(/* kext */ NULL,
		          kOSKextLogErrorLevel | kOSKextLogLoadFlag,
		          "Could not create raw hash from CDHash %s", cdHash);
		goto error;
	}

	bundleIDBufLen = CFStringGetMaximumSizeForEncoding(CFStringGetLength(bundleIDCFString) + 1, kCFStringEncodingUTF8);
	bundleIDBuffer = malloc(bundleIDBufLen);
	if (!bundleIDBuffer) {
		OSKextLog(/* kext */ NULL,
			kOSKextLogErrorLevel | kOSKextLogLoadFlag,
			"Impossible to malloc bundleIDBuffer %zu bytes", bundleIDBufLen);
		result = false;
		goto error;
	}
	GET_CSTRING_PTR(bundleIDCFString, bundleID, bundleIDBuffer, bundleIDBufLen);
	if (!bundleID) {
		OSKextLog(/* kext */ NULL,
		          kOSKextLogErrorLevel | kOSKextLogLoadFlag,
		          "Could not get bundleID string");
		result = false;
		goto error;
	}
	bundleIDLen = strlen(bundleID);

	result = CC_SHA256(bundleID, (unsigned int)bundleIDLen, (unsigned char*)&rawBundleHash[0]);
	if (!result) {
		OSKextLog(/* kext */ NULL,
				kOSKextLogErrorLevel | kOSKextLogLoadFlag,
				"Could not create hash of bundleID");
		goto error;
	}

	/* Note
	 * it is valid to have a void string for teamID,
	 * just zero the kaln field in that case.
	 */
	teamIDBufLen = CFStringGetMaximumSizeForEncoding(CFStringGetLength(teamIDCFString) + 1, kCFStringEncodingUTF8);
	teamIDBuffer = malloc(bundleIDBufLen);
	if (!teamIDBuffer) {
		OSKextLog(/* kext */ NULL,
			kOSKextLogErrorLevel | kOSKextLogLoadFlag,
			"Impossible to maloc teamIDBuffer %zu bytes", teamIDBufLen);
		result = false;
		goto error;
	}
	GET_CSTRING_PTR(teamIDCFString, teamID, teamIDBuffer, teamIDBufLen);

	result = OSKextVersionGetString(version, &versionString[0], kOSKextVersionMaxLength);
	if (!result) {
		OSKextLog(/* kext */ NULL,
		          kOSKextLogErrorLevel | kOSKextLogLoadFlag,
		          "Could not get version string");
		goto error;
	}

	kaln->loadType = loadType;

	memcpy(&kaln->cdHash[0], rawCDHash, kKALNKCDHashSize);

	if (teamID) {
		strncpy(&kaln->teamID[0], teamID, kKALNKTeamIDSize);
	} else {
		memset(&kaln->teamID[0], 0, kKALNKTeamIDSize);
	}
	memcpy(&kaln->bundleHash[0], rawBundleHash, kKALNKBundleHashSize);
	strncpy(&kaln->kextVersion[0], &versionString[0], kKALNKKextVersionSize);

	kaln->teamID[kKALNKTeamIDSize-1] = '\0';
	kaln->kextVersion[kKALNKKextVersionSize-1] = '\0';

error:

	if (cdHashBuffer) {
		free(cdHashBuffer);
	}
	if (bundleIDBuffer) {
		free(bundleIDBuffer);
	}
	if (teamIDBuffer) {
		free(teamIDBuffer);
	}

	return result;
}

/* KextAuditMakeKALNFromKext
 *
 * Summary:
 *
 * Retrieves the relevant signing/identity fields (hopefully) from the caches in
 * the security code, given a reference to a kext, and serializes this information
 * into a structure.
 *
 * Arguments: The structure to put the data, and the kext to look up.
 *
 * Returns: Success condition.
 */
static bool KextAuditMakeKALNFromKext(struct KextAuditLoadNotificationKext *kaln, OSKextRef theKext)
{
	CFURLRef	kextURL          = NULL; /* do not release */
	CFStringRef	bundleIDCFString = NULL; /* do not release */
	CFStringRef	cdHashCFString   = NULL; /* must release */
	CFStringRef	teamIDCFString   = NULL; /* must release */
	char		*adHocHash       = NULL; /* must free */
	OSKextVersion   version		 = 0;
	bool		haveAdHocHash    = false;
	bool		result           = false;

	/* If any of these calls fail, something is seriously wrong, since we should be
	 * past all validation in OSKextLoad() at this point.
	 */
	kextURL = OSKextGetURL(theKext);
	if (!kextURL) {
		OSKextLog(/* kext */ theKext,
		          kOSKextLogErrorLevel | kOSKextLogLoadFlag,
		          "Could not get URL from kext.");
		result = false;
		goto error;
	}
	bundleIDCFString = OSKextGetIdentifier(theKext);
	if (!bundleIDCFString) {
		OSKextLog(/* kext */ theKext,
		          kOSKextLogErrorLevel | kOSKextLogLoadFlag,
		          "Could not get bundleID from kext.");
		result = false;
		goto error;
	}
	version = OSKextGetVersion(theKext);
	if (!version) {
		OSKextLog(/* kext */ theKext,
		          kOSKextLogErrorLevel | kOSKextLogLoadFlag,
		          "Could not get version from kext.");
		result = false;
		goto error;
	}

	copySigningInfo(kextURL, &cdHashCFString, &teamIDCFString, NULL, NULL);

	/* No team ID -> likely a kext under development. Set it to an empty
	 * string, and let the bridge make its own policy decisions. */
	if (!teamIDCFString) {
		teamIDCFString = CFSTR("");
	}

	/* If we don't find a cdHash, the kext is unsigned. Try to generate an
	 * ad-hoc hash, and if _that_ fails, make the hash all zeroes so the
	 * bridge knows what happened. */
	if (!cdHashCFString) {
		getAdhocSignatureHash(kextURL, &adHocHash, NULL);
		haveAdHocHash = (adHocHash != NULL);
		if (!haveAdHocHash) {
			adHocHash = kSha1ZeroHash;
		}
		cdHashCFString = CFStringCreateWithCString(NULL, adHocHash, kCFStringEncodingUTF8);
	}

	result = KextAuditMakeKALNFromInfo(kaln, version, cdHashCFString,
	                                   bundleIDCFString, teamIDCFString);
	if (!result) {
		OSKextLog(/* kext */ theKext,
		          kOSKextLogErrorLevel | kOSKextLogLoadFlag,
		          "Could not create kext load notification from kext info.");
		goto error;
	}

error:
	SAFE_RELEASE(cdHashCFString);
	SAFE_RELEASE(teamIDCFString);
	if (haveAdHocHash) {
		free(adHocHash);
	}
	return result;
}

/* KextAuditNotifyBridgeWithReplySync
 *
 * Summary:
 *
 * This function writes a kext load's information to SMC using the KextAudit
 * kext, which waits for an `ack' from bridgeOS before returning.
 */
static bool KextAuditNotifyBridgeWithReplySync(struct KextAuditLoadNotificationKext *kaln,
                                               struct KextAuditBridgeResponse *kabr)
{
	if (!kaln || !kabr) {
		return false;
	}
	if (!gKextAuditIsInitialized) {
		OSKextLog(/* kext */ NULL,
		          kOSKextLogErrorLevel | kOSKextLogLoadFlag,
		          "KextAudit IOKit connection not initialized!");
		return false;
	}

	IOByteCount inStructSize  = sizeof(*kaln);
	size_t      outStructSize = sizeof(*kabr);
	IOReturn    ir;

	ir = IOConnectCallStructMethod(gKextAuditIOConn,
	                               kKextAuditMethodNotifyLoad,
	                               kaln,
	                               inStructSize,
	                               kabr,
	                               &outStructSize);
	if (ir != kIOReturnSuccess) {
		OSKextLog(/* kext */ NULL,
		          kOSKextLogErrorLevel | kOSKextLogLoadFlag,
		          "Error communicating with KextAudit kext.");
		return false;
	}

	return true;
}


#if DEBUG
/*
 * KextAuditTestBridge
 *
 * Summary:
 *
 * Send either a ping to the bridge (and wait for an ack), or send a
 * completely custom KextAuditLoadNotificationKext structure with limited validity
 * checking from the kext.
 *
 */
bool KextAuditTestBridge(uint8_t* kaln,
                         struct KextAuditBridgeResponse *kabr)
{
	if (!gKextAuditIsInitialized) {
		OSKextLog(/* kext */ NULL,
		          kOSKextLogErrorLevel | kOSKextLogLoadFlag,
		          "KextAudit IOKit connection not initialized!");
		return false;
	}

	if (!gShouldAuditKexts) {
		OSKextLog(/* kext */ NULL,
		          kOSKextLogErrorLevel | kOSKextLogLoadFlag,
		          "KextAudit disabled!");
		return false;
	}

	IOByteCount inStructSize  = kKALNStructSize;
	size_t      outStructSize = sizeof(*kabr);
	IOReturn    ir;

	ir = IOConnectCallStructMethod(gKextAuditIOConn,
	                               kKextAuditMethodTest,
	                               kaln,
	                               inStructSize,
	                               kabr,
	                               &outStructSize);
	if (ir != kIOReturnSuccess) {
		OSKextLog(/* kext */ NULL,
		          kOSKextLogErrorLevel | kOSKextLogLoadFlag,
		          "Error communicating with KextAudit kext.");
		return false;
	}

	return true;
}
#endif /* DEBUG */


/* KextAuditInitialize
 *
 * Summary:
 *
 * This function is called by KextAuditLoadCallback() to initialize the Kext
 * Audit subsystem when it has not yet been called.
 *
 * Returns: Success condition.
 *
 */
bool KextAuditInitialize(void)
{
	mach_port_t  port;
	io_service_t service;
	IOReturn     ir;
	bool         result = true;
	int          wait_time_us = 1000000; /* 1 second */

	if (gKextAuditIsInitialized) {
		return true;
	}

	gShouldAuditKexts = true;

	if (csr_check(CSR_ALLOW_UNTRUSTED_KEXTS) == 0) {
		/* if SIP is off: don't bother auditing kexts */
		OSKextLog(/* kext */ NULL,
		          kOSKextLogErrorLevel | kOSKextLogLoadFlag,
		          "Disabling KextAudit: SIP is off");
		gShouldAuditKexts = false;
		goto out;
	}

	ir = IOMasterPort(MACH_PORT_NULL, &port);
	if (ir != kIOReturnSuccess) {
		OSKextLog(/* kext */ NULL,
		          kOSKextLogErrorLevel | kOSKextLogLoadFlag,
		          "Could not get master port.");
		result = false;
		goto error;
	}

	do {
		service = IOServiceGetMatchingService(port, IOServiceMatching("KextAudit"));
		if (!service) {
			OSKextLog(/* kext */ NULL,
			          kOSKextLogBasicLevel | kOSKextLogLoadFlag,
			          "waiting for KextAudit to start");
			usleep(10000);
			wait_time_us -= 10000;
		}
	} while (!service && wait_time_us > 0);

	if (!service) {
		OSKextLog(/* kext */ NULL,
		          kOSKextLogErrorLevel | kOSKextLogLoadFlag,
		          "Disabling KextAudit: Could not find KextAudit kext");
		/*
		 * This cannot be fatal: we may be in the installer, or
		 * recovery mode, or on a system w/o the KextAudit kext
		 * in the prelinkedkernel image.
		 */
		gShouldAuditKexts = false;
		goto out;
	}

	ir = IOServiceOpen(service, mach_task_self(), 0, &gKextAuditIOConn);
	if (ir != kIOReturnSuccess) {
		OSKextLog(/* kext */ NULL,
		          kOSKextLogErrorLevel | kOSKextLogLoadFlag,
		          "Could not open connection with KextAudit kext.");
		result = false;
		goto error;
	}

out:
	gKextAuditIsInitialized = true;
	OSKextLog(/* kext */ NULL,
              kOSKextLogBasicLevel | kOSKextLogLoadFlag,
	      "KextAudit initialized: audit=%c", gShouldAuditKexts ? 'T' : 'F');

error:
	return result;
}

/* KextAuditLoadCallback
 *
 * Summary:
 *
 * This function should be called whenever a kext is about to be loaded by
 * IOKitUser, after every check has been made on the kext to make sure that
 * it's authentic, has its dependencies resolved, and is more or less ready
 * to go. We want a callback instead of calling this before OSKextLoad()
 * to make sure that we don't accidentally audit kexts that aren't even
 * getting loaded.
 *
 * Arguments: The kext that's about to be loaded.
 *
 * Returns: Success condition.
 *
 * NB: ***If this function returns false, the kext load will fail!***
 *
 */
Boolean KextAuditLoadCallback(OSKextRef theKext)
{
	struct KextAuditLoadNotificationKext kaln = { };
	struct KextAuditBridgeResponse   kabr = { };
	bool   result = true;
	os_signpost_id_t spid_callback, spid_kaln;

	/*
	 * Measure KextAuditLoadCallback time with a signpost.
	 */
	spid_callback = generate_signpost_id();
	os_signpost_interval_begin(get_signpost_log(), spid_callback, SIGNPOST_KEXTD_KEXTAUDITLOADCALLBACK);

	if (!theKext) {
		OSKextLog(/* kext */ theKext,
		          kOSKextLogErrorLevel | kOSKextLogLoadFlag,
		          "Can't audit kext load if kext is nonexistent");
		result = false;
		goto error;
	}

	if (!gKextAuditIsInitialized) {
		result = KextAuditInitialize();
		if (!result) {
			goto error;
		}
	}

	if (!gShouldAuditKexts) {
		result = true;
		goto error;
	}

	spid_kaln = os_signpost_id_make_with_pointer(get_signpost_log(), (void *)theKext);
	os_signpost_interval_begin(get_signpost_log(), spid_kaln, SIGNPOST_KEXTD_KEXTAUDITMAKEKALN);

	result = KextAuditMakeKALNFromKext(&kaln, theKext);
	if (!result) {
		OSKextLog(/* kext */ theKext,
		          kOSKextLogErrorLevel | kOSKextLogLoadFlag,
		          "Could not create kext load notification.");
		os_signpost_interval_end(get_signpost_log(), spid_kaln, SIGNPOST_KEXTD_KEXTAUDITMAKEKALN);
		goto error;
	}

	os_signpost_interval_end(get_signpost_log(), spid_kaln, SIGNPOST_KEXTD_KEXTAUDITMAKEKALN);

	kabr.status = kKALNStatusLoad;
	result = KextAuditNotifyBridgeWithReplySync(&kaln, &kabr);
	if (!result) {
		OSKextLog(/* kext */ theKext,
		          kOSKextLogErrorLevel | kOSKextLogLoadFlag,
		          "Could not notify bridge of kext load.");
		goto error;
	}

	if (kabr.status == kKALNStatusNoBridge) {
		gShouldAuditKexts = false;
		OSKextLog(/* kext */ NULL,
				kOSKextLogBasicLevel | kOSKextLogLoadFlag,
				"KextAudit didn't find a bridge: audit=%c", gShouldAuditKexts ? 'T' : 'F');
		goto error;
	}

	if (kabr.status != kKALNStatusBridgeAck) {
		OSKextLog(/* kext */ theKext,
			kOSKextLogErrorLevel | kOSKextLogLoadFlag,
			"KextAuditNotifyBridgeWithReplySync returned %d", kabr.status);
		goto error;
	}

error:
	os_signpost_interval_end(get_signpost_log(), spid_callback,
				 SIGNPOST_KEXTD_KEXTAUDITLOADCALLBACK, "%s", (gShouldAuditKexts ? (result ? "success" : "failure") : "no-audit"));
	return result;
}
