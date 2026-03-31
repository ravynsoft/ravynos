/*
 *  kextaudit_test.m
 *  kext_tools
 *
 *  Copyright 2018 Apple Inc. All rights reserved.
 *
 */
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include <Foundation/Foundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/kext/OSKext.h>
#include <IOKit/kext/OSKextPrivate.h>
#include <RemoteXPC/RemoteXPC.h>
#include <RemoteServiceDiscovery/RemoteServiceDiscovery.h>
#include <MultiverseSupport/kext_audit_plugin_common.h>
#include <darwintest.h>

T_GLOBAL_META(T_META_NAMESPACE("kext_tools.kextaudit"));

#include "../../security.h"
#include "../../kextaudit.h"
#include "../../kext_tools_util.h"

#define kKextAuditTestRemoteServiceName "com.apple.internal.xpc.remote.kext_audit"
#define kKextAuditTestKext @"/System/Library/Extensions/Dont Steal Mac OS X.kext"

/* Adding some LASIO definitions directly here */
#define LASIOBridgeStateIsSecure			(1 << 0)
#define LASIOBridgeStateNoKernelLoaded			(1 << 1)
#define LASIOBridgeStateOffendingKernelLoaded		(1 << 2)
#define LASIOBridgeStateOffendingKextLoaded		(1 << 3)
#define LASIOBridgeLoadErrorRecorded			(1 << 4)
#define LASIOBridgeStateBlacklistMissing		(1 << 5)

xpc_remote_connection_t kextaudit_conn;

static void kextaudit_connect_remote_service(void)
{
	remote_device_t device = NULL;
	remote_service_t service = NULL;
	dispatch_queue_t reply_queue = NULL;

	device = remote_device_copy_unique_of_type(REMOTE_DEVICE_TYPE_BRIDGE_COPROC);
	if (device == NULL) {
		T_LOG("No COPROC found, trying external.");
		device = remote_device_copy_unique_of_type(REMOTE_DEVICE_TYPE_BRIDGE_COPROC_EXTERNAL);
	}
	if (!device) {
		T_SKIP("Did not find coproc device -- might not be a Gibraltar device");
	}

	service = remote_device_copy_service(device, kKextAuditTestRemoteServiceName);
	T_QUIET; T_ASSERT_NOTNULL(service, "Get kextaudit service on the bridge");

	reply_queue = dispatch_queue_create("kextaudit_test", NULL);
	T_QUIET; T_ASSERT_NOTNULL(reply_queue, "Create a kextaudit_test dispatch queue");

	kextaudit_conn = xpc_remote_connection_create_with_remote_service(service, reply_queue, 0);
	T_QUIET; T_ASSERT_NOTNULL(kextaudit_conn, "Create a remote connection for kextaudit service");

	/* Empty block, since we call send_message_with_reply */
	xpc_remote_connection_set_event_handler(kextaudit_conn, ^(xpc_object_t event) {});
	xpc_remote_connection_activate(kextaudit_conn);

	T_PASS("Found and connected to the kextaudit multiversed plugin remote service");
}

static void kextaudit_audit_new_kext_load(NSString *kextPath) {
	NSURL         *kextURL = nil;
	OSKextRef     theKext = nil;
	bool result = false;

	AuthOptions_t testOptions = {
		.performFilesystemValidation = true,
		.performSignatureValidation  = true,
		.requireSecureLocation       = true,
		.respectSystemPolicy         = false,
		.allowNetwork                = false,
		.isCacheLoad                 = false,
	};

	kextURL = [NSURL fileURLWithPath:kextPath];
	theKext = OSKextCreate(NULL, (__bridge CFURLRef)kextURL);
	T_QUIET; T_ASSERT_NOTNULL(theKext, "Create reference to %s kext", [kextPath UTF8String]);

	result  = authenticateKext(theKext, &testOptions);
	T_QUIET; T_ASSERT_TRUE(result, "Authenticate kext without network as runtime load");

	result  = KextAuditLoadCallback(theKext);
	T_ASSERT_TRUE(result, "Sending load audit notification. If this returns successfully,"
		"the kext at - %s has loaded", [kextPath UTF8String]);
}

static void get_cdHash_for_kext(NSString *kextPath, char **cdHash) {
	NSURL         *kextURL;
	CFStringRef   cdHashCFString;
	char *cdHashBuffer =  NULL;

	kextURL = [NSURL fileURLWithPath:kextPath];

	copySigningInfo((__bridge CFURLRef)kextURL, &cdHashCFString, NULL, NULL, NULL);
	T_QUIET; T_ASSERT_NOTNULL(cdHashCFString, "Get cdHash of %s kext from Security subsystem", [kextPath UTF8String]);

	size_t cdHashBufLen = CFStringGetMaximumSizeForEncoding(CFStringGetLength(cdHashCFString) + 1, kCFStringEncodingUTF8);
	T_QUIET; T_ASSERT_NOTNULL(cdHashBuffer = malloc(cdHashBufLen), "mallocing for cdHash buffer");

	const char *cdHash_cpy = NULL;
	GET_CSTRING_PTR(cdHashCFString, cdHash_cpy, cdHashBuffer, cdHashBufLen);
	T_QUIET; T_ASSERT_NOTNULL(cdHash_cpy, "Get cdHash string pointer");
	T_QUIET; T_ASSERT_NOTNULL(strncpy(*cdHash, cdHash_cpy, kKALNCDHashSize), "Copy out cdHash string of kext");

	free(cdHashBuffer);
	CFRelease(cdHashCFString);
}

static void kextaudit_verify_kext_with_remote_service(NSString *kextPath) {
	__block bool gotIt = false;
	char *cdHash = malloc(kKALNCDHashSize);
	char    rawCDHash[kKALNCDHashSize] = {};
	char *ptrrawCDHash= rawCDHash;
	get_cdHash_for_kext(kextPath, (char **)&cdHash);

	size_t cdHashSize = strlen(cdHash);
	T_QUIET; T_ASSERT_TRUE(createRawBytesFromHexString(&rawCDHash[0], kKALNKCDHashSize, cdHash, cdHashSize), "create string of raw bytes");

	const char    *cmdString[] = { "cmd" };
	xpc_object_t  cmd   = xpc_string_create("sendloadinfo");
	xpc_object_t  msg   = xpc_dictionary_create(cmdString, &cmd, 1);
	xpc_object_t  reply, kexts;

	reply = xpc_remote_connection_send_message_with_reply_sync(kextaudit_conn, msg);
	T_QUIET; T_ASSERT_NE(xpc_get_type(reply), XPC_TYPE_ERROR, "Get a successful reply from multiversed");

	kexts = xpc_dictionary_get_value(reply, "kexts");
	T_QUIET; T_ASSERT_TRUE(kexts, "Retrieve kext array from reply");

	xpc_array_apply(kexts, (xpc_array_applier_t)^(size_t index, xpc_object_t kext) {
		xpc_object_t thisHashObj = xpc_dictionary_get_value(kext, "cdhash");
		char remote_cdHash[kKALNCDHashSize];
		size_t copied = xpc_data_get_bytes(thisHashObj, remote_cdHash, 0, kKALNCDHashSize);
		if (copied != cdHashSize / 2) {
		T_QUIET; T_EXPECT_EQ_ULONG(copied, cdHashSize / 2, "Expect cdHash of kexts to be half of kKALNCDHashSize");
			return true;
		}
		if (! memcmp((void *)ptrrawCDHash, (void*)remote_cdHash, copied)) {
			gotIt = true;
			return false;
		}
		return true;
	});

	T_ASSERT_TRUE(gotIt, "Verify that multiversed KextAudit plugin knows about the kext at %s",
		[kextPath UTF8String]);

	free((void *)cdHash);
}

static void kextaudit_verify_kext_with_lasio(NSString *kextPath) {
	__block bool gotIt =  false;
	const char    *cdHash = malloc(kKALNCDHashSize);
	char    rawCDHash[kKALNCDHashSize] = {};
	char *ptrrawCDHash= rawCDHash;
	get_cdHash_for_kext(kextPath, (char **)&cdHash);

	size_t cdHashSize = strlen(cdHash);
	T_QUIET; T_ASSERT_TRUE(createRawBytesFromHexString(&rawCDHash[0], kKALNKCDHashSize, cdHash, cdHashSize), "create string of raw bytes");

	const char *cmdString[] = { "cmd" };
	xpc_object_t cmd = xpc_string_create("sendlasioinfo");
	xpc_object_t msg = xpc_dictionary_create(cmdString, &cmd, 1);
	xpc_object_t reply, loadedKexts;

	reply = xpc_remote_connection_send_message_with_reply_sync(kextaudit_conn, msg);
	T_QUIET; T_ASSERT_NE(xpc_get_type(reply), XPC_TYPE_ERROR, "Get multiversed reply");

	loadedKexts = xpc_dictionary_get_value(reply, "loadedKextHashes");
	T_QUIET; T_ASSERT_TRUE(loadedKexts, "Retrieve loaded kext hashes from reply");

	xpc_array_apply(loadedKexts, (xpc_array_applier_t)^(size_t index, xpc_object_t loadedKext) {
		uint8_t remote_cdHash[kKALNCDHashSize];
		size_t copied = xpc_data_get_bytes(loadedKext, remote_cdHash, 0, kKALNCDHashSize);

		if (copied != cdHashSize / 2) {
			T_QUIET; T_EXPECT_EQ_ULONG(copied, cdHashSize / 2, "Expect cdHash of kexts to be half of kKALNCDHashSize");
			return true;
		}
		if (! memcmp((void *)ptrrawCDHash, (void*)remote_cdHash, copied)) {
			gotIt = true;
			return false;
		}
		return true;
	});

	T_ASSERT_TRUE(gotIt, "Verify that lasio has kext at %s in its list of loaded kexts",
		[kextPath UTF8String]);

	free((void *)cdHash);
}

static uint64_t kextaudit_lasio_get_state() {
	const char *cmdString[] = { "cmd" };
	xpc_object_t cmd = xpc_string_create("sendlasioinfo");
	xpc_object_t msg = xpc_dictionary_create(cmdString, &cmd, 1);
	xpc_object_t reply;
	uint64_t currentState = 0;

	reply = xpc_remote_connection_send_message_with_reply_sync(kextaudit_conn, msg);
	T_QUIET; T_ASSERT_NE(xpc_get_type(reply), XPC_TYPE_ERROR, "Get multiversed reply");

	currentState = xpc_dictionary_get_uint64(reply, "currentState");

	return currentState;
}

T_DECL(lasio_verify_kext_registration, "testing if a loaded kext is recorded by the kext_audit plugin and lasio", T_META_ASROOT(true))
{
	bool lasio_initialState = false; // initializing to insecure

	kextaudit_connect_remote_service();

	lasio_initialState = kextaudit_lasio_get_state();
	T_LOG("Lasio is at a/an %s state before new kext load",
		  lasio_initialState & LASIOBridgeStateIsSecure ? "secure" : "not secure");

	kextaudit_audit_new_kext_load(kKextAuditTestKext);

	if (lasio_initialState & LASIOBridgeStateIsSecure) /* initially secure - should remain secure */ {
		kextaudit_verify_kext_with_remote_service(kKextAuditTestKext);

		kextaudit_verify_kext_with_lasio(kKextAuditTestKext);

		T_ASSERT_TRUE((kextaudit_lasio_get_state() & LASIOBridgeStateIsSecure), "Lasio must remain in a secure state");
	} else /* initially insecure - should still remain insecure */ {
		T_ASSERT_FALSE((kextaudit_lasio_get_state() & LASIOBridgeStateIsSecure), "Lasio must remain in an insecure state");
	}
}
