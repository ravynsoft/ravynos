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
#include <IOKit/IOUserClient.h>

#include "KextAuditUserClient.h"

#define super IOUserClient
OSDefineMetaClassAndStructors(KextAuditUserClient, IOUserClient);

const IOExternalMethodDispatch KextAuditUserClient::sMethods[kKextAuditMethodCount] =
{
	{
		reinterpret_cast<IOExternalMethodAction>(&KextAuditUserClient::notifyLoad),
		0, sizeof(struct KextAuditLoadNotificationKext),
		0, sizeof(struct KextAuditBridgeResponse)
	},
	{
		reinterpret_cast<IOExternalMethodAction>(&KextAuditUserClient::test),
		0, sizeof(struct KextAuditLoadNotificationKext),
		0, sizeof(struct KextAuditBridgeResponse)
	},
};

KextAuditBridgeDeviceType
KextAuditUserClient::getBridgeDeviceType(void)
{
	KextAuditBridgeDeviceType result = kKextAuditBridgeDeviceTypeNoCoprocessor;
	OSObject *value = NULL;
	OSData *num = NULL;

	IORegistryEntry *defaults = IORegistryEntry::fromPath("IODeviceTree:/efi/platform");
	if (!defaults) {
		goto error;
	}

	value = defaults->getProperty("apple-coprocessor-version");
	if (!value) {
		goto error;
	}

	num = OSDynamicCast(OSData, value);
	if (!num) {
		goto error;
	}

	result = *(KextAuditBridgeDeviceType *)num->getBytesNoCopy(0, 4);
error:
	OSSafeReleaseNULL(defaults);
	return result;
}

IOReturn
KextAuditUserClient::externalMethod(uint32_t selector, IOExternalMethodArguments *args,
                                    IOExternalMethodDispatch *dispatch, OSObject *target, void *reference)
{
	IOReturn err = kIOReturnSuccess;

	if (!fUserClientHasEntitlement) {
		DEBUG_LOG("User client does not have entitlement for KextAudit access");
		err = kIOReturnNotPrivileged;
		goto error;
	}

	if (selector >= kKextAuditMethodCount) {
		DEBUG_LOG("Invalid selector: %u", selector);
		err = kIOReturnBadArgument;
		goto error;
	}
	dispatch = (IOExternalMethodDispatch *)&sMethods[selector];
	target = this;
	err = IOUserClient::externalMethod(selector, args, dispatch, target, reference);
error:
	return err;
}

IOReturn
KextAuditUserClient::notifyLoad(KextAuditUserClient *target, void * /* reference */, IOExternalMethodArguments *args)
{
	struct KextAuditLoadNotificationKext *kaln = NULL;
	struct KextAuditBridgeResponse       *kabr = NULL;

	IOReturn err = kIOReturnSuccess;
	if (!(target && args)) {
		err = kIOReturnBadArgument;
		goto error;
	}

	kaln = (struct KextAuditLoadNotificationKext *)args->structureInput;
	kabr = (struct KextAuditBridgeResponse *)args->structureOutput;

	/* Fake notification success on non-Gibraltar machines */
	if (target->fDeviceType < kKextAuditBridgeDeviceTypeT290) {
		kabr->status = kKALNStatusNoBridge;
		return kIOReturnSuccess;
	}

	if (!VALID_KEXT_LOADTYPE(kaln->loadType)) {
		err = kIOReturnBadArgument;
		goto error;
	}
	if (!target->fProvider->notifyBridgeWithReplySync(kaln, kabr)) {
		err = kIOReturnError;
		goto error;
	}
error:
	return err;
}

IOReturn
KextAuditUserClient::test(KextAuditUserClient *target, void * /* reference */, IOExternalMethodArguments *args)
{
	IOReturn err = kIOReturnSuccess;
	struct KextAuditLoadNotificationKext *kaln = NULL;
	struct KextAuditBridgeResponse       *kabr = NULL;
#ifdef DEBUG
	if (!(target && args)) {
		err = kIOReturnBadArgument;
		goto error;
	}

	kaln = (struct KextAuditLoadNotificationKext *)args->structureInput;
	kabr = (struct KextAuditBridgeResponse *)args->structureOutput;

	/* Fake notification success on non-Gibraltar machines */
	if (target->fDeviceType < kKextAuditBridgeDeviceTypeT290) {
		kabr->status = kKALNStatusNoBridge;
		return kIOReturnSuccess;
	}

	if (kaln->loadType > kKALTMax) {
		err = kIOReturnBadArgument;
		goto error;
	} else if (kaln->loadType == kKALTMax) {
		/*
		 * use the highest unused loadType value as a test /
		 * ping with bridgeOS
		 */
		if (!target->fProvider->testBridgeConnection(kabr)) {
			err = kIOReturnError;
		}
		goto error;
	} else {
		/*
		 * provide a hook that can send other loadTypes to the bridge
		 * we still use KextAuditLoadNotificationKext, anyway it has
		 * the same size of the other loadTypes and it is treated as
		 * raw bytes.
		 */
		if (!target->fProvider->notifyBridgeWithReplySync(kaln, kabr)) {
			err = kIOReturnError;
			goto error;
		}
	}
error:
#else
	(void)target;
	(void)args;
	(void)kaln;
	(void)kabr;
#endif /* DEBUG */
	return err;
}

bool
KextAuditUserClient::initWithTask(task_t owningTask, void *securityID, UInt32 type, OSDictionary *properties)
{
	OSObject *entitlement = NULL;
	fUserClientHasEntitlement = false;
	bool result = true;
	if (!super::initWithTask(owningTask, securityID, type, properties)) {
		DEBUG_LOG("Could not initialize IOUserClient with task");
		result = false;
		goto error;
	}

	if (IOUserClient::clientHasPrivilege(securityID, kIOClientPrivilegeAdministrator) != kIOReturnSuccess) {
		DEBUG_LOG("User client does not have root privileges");
		result = false;
		goto error;
	}

	entitlement = IOUserClient::copyClientEntitlement(owningTask, kKextAuditUserAccessEntitlement);
	if (!entitlement || entitlement != kOSBooleanTrue) {
		DEBUG_LOG("User client does not have entitlement for access");
		result = false;
		goto error;
	}
	entitlement->release();
	fUserClientHasEntitlement = true;

	fTask = owningTask;
	fDeviceType = getBridgeDeviceType();
error:
	return result;
}

bool
KextAuditUserClient::start(IOService * provider)
{
	bool result = true;

	fProvider = OSDynamicCast(KextAudit, provider);
	if (!fProvider) {
		DEBUG_LOG("Could not find provider");
		result = false;
		goto error;
	}

	// only call super::start if everything else has been successful
	if (!super::start(provider)) {
		DEBUG_LOG("Could not start provider");
		fProvider = NULL;
		result = false;
		goto error;
	}

error:
	return result;
}

void
KextAuditUserClient::stop(IOService * provider)
{
	super::stop(provider);
	fProvider = NULL;
}

void
KextAuditUserClient::free(void)
{
	if (fTask) {
		fTask = NULL;
	}

	super::free();
}

IOReturn
KextAuditUserClient::clientClose(void)
{
	terminate();

	return kIOReturnSuccess;
}
