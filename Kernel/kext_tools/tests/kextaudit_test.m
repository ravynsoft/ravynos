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
#include "../kext_tools_util.h"
#include "../kextaudit.h"
#include "../security.h"

#include "unit_test.h"

#define kKextAuditTestRemoteServiceName "com.apple.internal.xpc.remote.kext_audit"

xpc_remote_connection_t kextaudit_conn;

static bool test_kextaudit_find_remote_service(void)
{
	bool result = true;
	remote_device_t device = NULL;
	remote_service_t service = NULL;
	dispatch_queue_t reply_queue = NULL;

	device = remote_device_copy_unique_of_type(REMOTE_DEVICE_TYPE_BRIDGE_COPROC);
	if (device == NULL) {
		TEST_LOG("No COPROC found, trying external.");
		device = remote_device_copy_unique_of_type(REMOTE_DEVICE_TYPE_BRIDGE_COPROC_EXTERNAL);
	}
	TEST_REQUIRE("Find bridge", device, result, false, error);

	service = remote_device_copy_service(device, kKextAuditTestRemoteServiceName);
	TEST_REQUIRE("Get kextaudit service", service, result, false, error);

	reply_queue = dispatch_queue_create("kextaudit_test", NULL);
	TEST_REQUIRE("Allocate dispatch queue", reply_queue, result, false, error);

	kextaudit_conn = xpc_remote_connection_create_with_remote_service(service, reply_queue, 0);
	TEST_REQUIRE("Create remote connection", kextaudit_conn, result, false, error);

	/* Empty block, since we call send_message_with_reply */
	xpc_remote_connection_set_event_handler(kextaudit_conn, ^(xpc_object_t event) {});
	xpc_remote_connection_activate(kextaudit_conn);
error:
	return result;
}

static bool test_kextaudit_multiversed_roundtrip(void)
{
	Boolean       result;
	__block bool  gotIt = false;

	NSURL         *kextURL;
	OSKextRef     theKext;
	CFStringRef   cdHashCFString;
	const char    *cdHash = NULL;
	AuthOptions_t testOptions = {
		.performFilesystemValidation = true,
		.performSignatureValidation  = true,
		.requireSecureLocation       = true,
		.respectSystemPolicy         = false,
		.checkDextApproval           = false,
		.allowNetwork                = false,
		.isCacheLoad                 = false,
	};

	const char    *cmdString[] = { "cmd" };
	xpc_object_t  cmd   = xpc_string_create("sendloadinfo");
	xpc_object_t  msg   = xpc_dictionary_create(cmdString, &cmd, 1);
	xpc_object_t  reply, kexts;
	char rawCDHash[kKALNKCDHashSize] = { };
	char* ptrrawCDHash = rawCDHash;
	size_t cdHashSize;
	CFIndex cdHashBufLen;
	char *cdHashBuffer;

	kextURL = [NSURL fileURLWithPath:@"/System/Library/Extensions/Dont Steal Mac OS X.kext"];
	theKext = OSKextCreate(NULL, (__bridge CFURLRef)kextURL);
	TEST_REQUIRE("Able to create reference to DSMOS kext", theKext != NULL, result, false, error);

	result  = authenticateKext(theKext, &testOptions);
	TEST_REQUIRE("Kext authenticates without network as runtime load", result, result, false, error);

	result  = KextAuditLoadCallback(theKext);
	TEST_REQUIRE("Success sending load notification", result, result, false, error);

	copySigningInfo((__bridge CFURLRef)kextURL, &cdHashCFString, NULL, NULL, NULL);
	TEST_REQUIRE("Get cdHash from security subsystem", cdHashCFString, result, false, error);

	cdHashBufLen = CFStringGetMaximumSizeForEncoding(CFStringGetLength(cdHashCFString) + 1, kCFStringEncodingUTF8);
	cdHashBuffer = malloc(cdHashBufLen);
	TEST_REQUIRE("Able to malloc cdHashBuffer", cdHashBuffer != NULL, result, false, error);
	GET_CSTRING_PTR(cdHashCFString, cdHash, cdHashBuffer, cdHashBufLen);
	TEST_REQUIRE("Get cdHash string pointer", cdHash, result, false, error);
	cdHashSize = strlen(cdHash);

	result = createRawBytesFromHexString(&rawCDHash[0], kKALNKCDHashSize, cdHash, cdHashSize);
	TEST_REQUIRE("Success creading raw bytes", result, result, false, error);

	reply = xpc_remote_connection_send_message_with_reply_sync(kextaudit_conn, msg);
	TEST_REQUIRE("Successful reply from multiversed", xpc_get_type(reply) != XPC_TYPE_ERROR, result, false, error);

	kexts = xpc_dictionary_get_value(reply, "kexts");
	TEST_REQUIRE("Retreive kext array from reply", kexts, result, false, error);

	xpc_array_apply(kexts, (xpc_array_applier_t)^(size_t index, xpc_object_t kext) {
		xpc_object_t thisHashObj = xpc_dictionary_get_value(kext, "cdhash");

        char remote_cdHash[kKALNKCDHashSize];
        size_t copied = xpc_data_get_bytes(thisHashObj, remote_cdHash, 0, kKALNKCDHashSize);

        if (copied != cdHashSize/2) {
            return true;
        }

        int i;
        for (i = 0; i < copied; i++) {
            if(ptrrawCDHash[i] != remote_cdHash[i]) {
                return TRUE;
            }
        }
        gotIt = true;
        return false;

	});
	TEST_REQUIRE("Found cdHash on the other end", gotIt, result, false, error);

error:
	return result;
}

#if DEBUG
extern bool KextAuditInitialize(void);
extern bool KextAuditTestBridge(uint8_t *kaln,
                                struct KextAuditBridgeResponse *kabr);

static bool send_kernel_load_notification(const char *hash_str, const char *csrActiveConfig_str, const char *type_tag_str)
{
	if (!KextAuditInitialize()) {
		fprintf(stderr, "Error initializing KextAudit connection\n");
		return false;
	}

	struct KextAuditLoadNotificationEFI kaln = {};
	struct KextAuditBridgeResponse   kabr = {};

	if (!createRawBytesFromHexString((char *)&kaln.cdHash[0], sizeof(kaln.cdHash),
	                                 hash_str, strlen(hash_str))) {
		fprintf(stderr, "Invalid kernel hash: '%s'\n", hash_str);
		return false;
	}

	kaln.loadType = kKALTImmutableKernel;
	kabr.status = kKALNStatusTest;

	if (strlen(type_tag_str) != 4) {
		fprintf(stderr, "Invalid type_tag. Must be 4 characters, e.g. mkrn");
		return false;
	}

	if (strlen(csrActiveConfig_str) != 4) {
		fprintf(stderr, "Invalid csrActiveConfig. Must be 4 characters, e.g. mkrn");
		return false;
	}

	uint32_t type_tag = 0;
	type_tag  = (uint32_t)(type_tag_str[0]) << 24;
	type_tag |= (uint32_t)(type_tag_str[1]) << 16;
	type_tag |= (uint32_t)(type_tag_str[2]) << 8;
	type_tag |= (uint32_t)(type_tag_str[3]);

	kaln.typeTag = type_tag;

	uint32_t csrActiveConfig = 0;
	csrActiveConfig  = (uint32_t)(csrActiveConfig_str[0]) << 24;
	csrActiveConfig |= (uint32_t)(csrActiveConfig_str[1]) << 16;
	csrActiveConfig |= (uint32_t)(csrActiveConfig_str[2]) << 8;
	csrActiveConfig |= (uint32_t)(csrActiveConfig_str[3]);

	kaln.csrActiveConfig = csrActiveConfig;
	kaln.csrActiveConfig_status = csrActiveConfigSet;

	fprintf(stdout, "Sending test load, {type:%d, tag:0x%x, csrActiveConfig:0x%x, hash:%s}\n", kaln.loadType, type_tag, csrActiveConfig, hash_str);
	if (!KextAuditTestBridge((uint8_t*)&kaln, &kabr)) {
		fprintf(stderr, "KextAuditTestBridge failed. Check logs...\n");
		return false;
	}

	fprintf(stdout, "Sent kernel load notifiction. response:%d\n", kabr.status);
	return true;
}

static bool send_bridgeos_ping(void)
{
	if (!KextAuditInitialize()) {
		fprintf(stderr, "Error initializing KextAudit connection\n");
		return false;
	}

	struct KextAuditLoadNotificationKext kaln = {};
	struct KextAuditBridgeResponse   kabr = {};

	kaln.loadType = kKALTMax;
	kabr.status = kKALNStatusTest;

	if (!KextAuditTestBridge((uint8_t )&kaln, &kabr)) {
		fprintf(stderr, "KextAuditTestBridge failed. Check logs...\n");
		return false;
	}

	fprintf(stdout, "Sent bridge ping. response:%d\n", kabr.status);
	return true;
}
#endif /* DEBUG */

int main(int argc, char **argv)
{
	bool result = true;

	/* check for root privileges */
	if (geteuid() != 0){
		fprintf(stderr, "%s requires root privileges to run\n", argv[0]);
		return 1;
	}

#if DEBUG
	if (argc > 3) {
                result = send_kernel_load_notification(argv[1], argv[2], argv[3]);
                return !result;
	} else if (argc == 2 && !strcmp(argv[1], "ping")) {
		result = send_bridgeos_ping();
		return !result;
	} else if (argc > 1) {
		fprintf(stderr, "invalid invocation of %s...\n", argv[0]);
		return -1;
	}
#endif /* DEBUG */

	result = test_kextaudit_find_remote_service();
	TEST_RESULT("Find the kextaudit multiversed plugin remote service", result);
	if (!result) {
		goto error;
	}

	result = test_kextaudit_multiversed_roundtrip();
	TEST_RESULT("Roundtrip message between KextAudit and multiversed", result);
	if (!result) {
		goto error;
	}
error:
	return !result;
}
