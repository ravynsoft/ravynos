/*
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

#include <IOKit/IOLib.h>
#include <IOKit/IOService.h>
#include <IOKit/smc/AppleSMCFamily.h>

#include "KextAudit.h"

#define super IOService
OSDefineMetaClassAndStructors(KextAudit, IOService);

void
KextAudit::free(void)
{
	super::free();
}

bool
KextAudit::init(OSDictionary *dict)
{
	return super::init(dict);
}

IOService *
KextAudit::probe(IOService *provider, SInt32 *score)
{
	return super::probe(provider, score);
}

/*
 * NOTE: Although these SMC operations could be done from user space,
 *       there is no good way to serialize all SMC/bridge communication
 *       from the three different user space utilities that can load kexts:
 *       kextd, kextutil, kextload. The kext load audit operation *must* be
 *       atomic and synchronous with the kext load. The best way to force
 *       atomicity is to use this kext as the serialization point for the
 *       bridgeOS kext audit communication protocol.
 */
bool KextAudit::notifyBridgeWithReplySync(struct KextAuditLoadNotificationKext *kaln,
					  struct KextAuditBridgeResponse *kabr)
{
	bool      result = false;
	uint8_t   status;
	SMCResult sr;

	// serialize all SMC key writing to the bridge
	IOLockLock(_kalnLock);

	/*
	 * The protocol is as follows:
	 * 1. Write 'BootPolicyNotReady' to SMCKEY_EFI_MULTIBOOT_STATUS
	 * 2. Write (kaln data) to SMCKEY_KEXT_AUDIT_IDENTITY
	 * 3. Write BOOT_STATE_KERNEL_AUDIT_LOAD to SMCKEY_EFI_BOOT_STATUS
	 * 4. Poll on SMCKEY_EFI_MULTIBOOT_STATUS waiting for a value of BootPolicyOk
	 */

	// 1. Write 'BootPolicyNotReady' to SMCKEY_EFI_MULTIBOOT_STATUS
	status = (uint8_t)BootPolicyNotReady;
	sr = fSMCDriver->smcWriteKey(SMCKEY_EFI_MULTIBOOT_STATUS, sizeof(status), &status);
	if (sr != kSMCSuccess) {
		DEBUG_LOG("FAILED to write EFI_MULTIBOOT_STATUS SMC key!");
		goto error;
	}

	// 2. Write (kaln data) to SMCKEY_KEXT_AUDIT_IDENTITY
	sr = fSMCDriver->smcWriteKey(SMCKEY_KEXT_AUDIT_IDENTITY, sizeof(*kaln), kaln);
	if (sr != kSMCSuccess) {
		DEBUG_LOG("FAILED to write KEXT_AUDIT_IDENTITY SMC key!");
		goto error;
	}

	// 3. Write BOOT_STATE_KERNEL_AUDIT_LOAD to SMCKEY_EFI_BOOT_STATUS
	status = BOOT_STATE_KERNEL_AUDIT_LOAD;
	sr = fSMCDriver->smcWriteKey(SMCKEY_EFI_BOOT_STATUS, sizeof(status), &status);
	if (sr != kSMCSuccess) {
		DEBUG_LOG("FAILED to write EFI_BOOT_STATUS SMC key!");
		goto error;
	}

	// 4. Poll on SMCKEY_EFI_MULTIBOOT_STATUS waiting for a value of BootPolicyOk
	do {
		sr = fSMCDriver->smcReadKey(SMCKEY_EFI_MULTIBOOT_STATUS, sizeof(status), &status);
		if (sr != kSMCSuccess) {
			DEBUG_LOG("FAILED to poll EFI_MULTIBOOT_STATUS SMC key for load status!!");
			goto error;
		}
		IOSleep(kKextAuditPollIntervalMs);
	} while (status != BootPolicyOk);

	/*
	 * This is a placeholder for the bridge's response. Add more fields to
	 * `struct KextAuditBridgeResponse' in KextAudit.h to make the protocol
	 * more interesting. Be sure to bump the protocol version.
	 *
	 * In the future, the bridge might want to write to the KEXT_AUDIT_IDENTITY
	 * key with some additional data when it writes BootPolicyOk to
	 * EFI_MUILTIBOOT_STATUS. We could read that in here, and then write
	 * another status value to acknowledge receipt.
	 */
	kabr->status = kKALNStatusBridgeAck;
	result = true;

error:
	IOLockUnlock(_kalnLock);
	return result;
}

bool
KextAudit::testBridgeConnection(struct KextAuditBridgeResponse *kabr)
{
	bool      result = false;
	uint8_t   status;
	SMCResult sr;

	// serialize all SMC key writing to the bridge
	IOLockLock(_kalnLock);

	// 1. Write 'BootPolicyNotReady' to SMCKEY_EFI_MULTIBOOT_STATUS
	status = (uint8_t)BootPolicyNotReady;
	sr = fSMCDriver->smcWriteKey(SMCKEY_EFI_MULTIBOOT_STATUS, sizeof(status), &status);
	if (sr != kSMCSuccess) {
		DEBUG_LOG("TEST: FAILED to write EFI_MULTIBOOT_STATUS SMC key!");
		goto error;
	}

	// 3. Write BOOT_STATE_KEXT_AUDIT_TEST to SMCKEY_EFI_BOOT_STATUS
	status = BOOT_STATE_KEXT_AUDIT_TEST;
	sr = fSMCDriver->smcWriteKey(SMCKEY_EFI_BOOT_STATUS, sizeof(status), &status);
	if (sr != kSMCSuccess) {
		DEBUG_LOG("TEST: FAILED to write EFI_BOOT_STATUS SMC key!");
		goto error;
	}

	// 4. Poll on SMCKEY_EFI_MULTIBOOT_STATUS waiting for a value of BootPolicyOk
	do {
		sr = fSMCDriver->smcReadKey(SMCKEY_EFI_MULTIBOOT_STATUS, sizeof(status), &status);
		if (sr != kSMCSuccess) {
			DEBUG_LOG("TEST: FAILED to poll EFI_MULTIBOOT_STATUS SMC key for load status!!");
			goto error;
		}
		IOSleep(kKextAuditPollIntervalMs);
	} while (status != BootPolicyOk);

	kabr->status = kKALNStatusBridgeAck;
	result = true;

error:
	IOLockUnlock(_kalnLock);
	return result;
}


bool
KextAudit::start(IOService *provider)
{
	DEBUG_LOG("start");
	const OSSymbol *ucClassName;
	bool result = true;

	result = super::start(provider);
	if (!result) {
		DEBUG_LOG("Could not start provider");
		goto error;
	}

	fSMCDriver = OSDynamicCast(AppleSMCFamily, provider);
	if (!fSMCDriver) {
		DEBUG_LOG("Provider is not AppleSMCFamily");
		result = false;
		goto error;
	}

	ucClassName = OSSymbol::withCStringNoCopy("KextAuditUserClient");
	if (ucClassName) {
		setProperty(gIOUserClientClassKey, (OSObject *)ucClassName);
		ucClassName->release();
	}

	_kalnLock = IOLockAlloc();
	if (!_kalnLock) {
		DEBUG_LOG("Can't initialize _kalnLock!");
		result = false;
		goto error;
	}

	registerService();
error:
	return result;
}

void
KextAudit::stop(IOService *provider)
{
	IOLog("%s called!?", __FUNCTION__);
}

bool
KextAudit::terminate(IOOptionBits options)
{
	IOLog("%s called!?", __FUNCTION__);
	return false;
}
