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

#ifndef _KEXT_AUDIT_USER_CLIENT_H_
#define _KEXT_AUDIT_USER_CLIENT_H_

#include <IOKit/IOLib.h>
#include <IOKit/IOUserClient.h>

#include "KextAudit.h"

#define VALID_KEXT_LOADTYPE(loadType)		\
	((loadType) == kKALTKextCDHashSha1 ||	\
	 (loadType) == kKALTKextCDHashSha256)

// <rdar://problem/30172421> Expose t208/t290 defines into macOS SDK
enum KextAuditBridgeDeviceType {
	kKextAuditBridgeDeviceTypeNoCoprocessor = 0x00000000,
	kKextAuditBridgeDeviceTypeT208 = 0x00010000,
	kKextAuditBridgeDeviceTypeT290 = 0x00020000,
};

class KextAuditUserClient : public IOUserClient
{
	OSDeclareDefaultStructors(KextAuditUserClient);

protected:
	static const IOExternalMethodDispatch sMethods[kKextAuditMethodCount];
	static IOReturn notifyLoad(KextAuditUserClient *target, void *, IOExternalMethodArguments *args);
	static IOReturn test(KextAuditUserClient *target, void *, IOExternalMethodArguments *args);

private:
	task_t fTask;
	KextAudit *fProvider;
	bool fUserClientHasEntitlement;
	KextAuditBridgeDeviceType fDeviceType;

public:
	virtual IOReturn clientClose(void) override;
	virtual bool initWithTask(task_t owningTask, void *security_id, UInt32 type,
	                          OSDictionary *properties) override;
	virtual bool start(IOService *provider) override;
	virtual void stop(IOService *provider) override;
	virtual void free(void) override;
	IOReturn externalMethod(uint32_t selector, IOExternalMethodArguments *arguments,
	                        IOExternalMethodDispatch *dispatch, OSObject *target,
	                        void *reference) override;
	KextAuditBridgeDeviceType getBridgeDeviceType(void);
};

#endif /* !_KEXT_AUDIT_USER_CLIENT_H_ */
