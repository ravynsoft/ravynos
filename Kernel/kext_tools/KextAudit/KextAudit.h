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

#ifndef _KEXT_AUDIT_H_
#define _KEXT_AUDIT_H_

#include <stdint.h>
#include <IOKit/IOLib.h>
#include <IOKit/IOLocks.h>
#include <IOKit/IOService.h>
#include <IOKit/smc/AppleSMCFamily.h>

#include <MultiverseSupport/kext_audit_plugin_common.h>
#include "efi_smc.h"

#ifdef DEBUG
#define DEBUG_LOG(fmt, ...) IOLog("%s, in %s, line %d: " fmt "\n", "KextAudit",\
				__func__, __LINE__, ##__VA_ARGS__)
#else
#define DEBUG_LOG(fmt, ...)
#endif /* DEBUG */

#define kKextAuditPollIntervalMs  2

#define kKextAuditUserAccessEntitlement "com.apple.private.KextAudit.user-access"

class KextAudit : public IOService
{
	OSDeclareDefaultStructors(KextAudit)

public:
	virtual bool init(OSDictionary *dictionary) override;
	virtual void free(void) override;
	virtual IOService *probe(IOService *provider, SInt32 *score) override;
	virtual bool start(IOService *provider) override;
	virtual void stop(IOService *provider) override;
	virtual bool terminate(IOOptionBits options) override;

	bool notifyBridgeWithReplySync(struct KextAuditLoadNotificationKext *kaln,
	                               struct KextAuditBridgeResponse *kabr);

	bool testBridgeConnection(struct KextAuditBridgeResponse *kabr);

private:
	AppleSMCFamily *fSMCDriver;
	IOLock *_kalnLock;
};

#endif /* _KEXT_AUDIT_H_ */
