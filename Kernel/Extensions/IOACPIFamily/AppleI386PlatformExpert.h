/*
 * Portions Copyright (c) 1999-2003 Apple Computer, Inc. All Rights
 * Reserved.
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
 * This file was modified by William Kent in 2017 to support the PureDarwin
 * project. This notice is included in support of clause 2.2(b) of the License.
 */

#ifndef _IOKIT_APPLEI386PLATFORM_H
#define _IOKIT_APPLEI386PLATFORM_H

#include <IOKit/IOPlatformExpert.h>
#include "AppleI386CPU.h"

class AppleI386PlatformExpert : public IOPlatformExpert {
	OSDeclareDefaultStructors(AppleI386PlatformExpert)

private:
	const OSSymbol *_interruptControllerName;
	AppleI386CPU *bootCPU;

	void setupPIC(IOService *nub);
	void setupBIOS(IOService *nub);

	static int handlePEHaltRestart(unsigned int type);

public:
	virtual bool init(OSDictionary *properties) APPLE_KEXT_OVERRIDE;
	virtual IOService *probe(IOService *provider, SInt32 *score) APPLE_KEXT_OVERRIDE;
	virtual bool start(IOService *provider) APPLE_KEXT_OVERRIDE;
	virtual bool configure(IOService *provider) APPLE_KEXT_OVERRIDE;
	virtual bool matchNubWithPropertyTable(IOService *nub, OSDictionary *table);
	virtual IOService *createNub(OSDictionary *from) APPLE_KEXT_OVERRIDE;
	virtual bool reserveSystemInterrupt(IOService *client, UInt32 vectorNumber, bool exclusive);
	virtual void releaseSystemInterrupt(IOService *client, UInt32 vectorNumber, bool exclusive);
	virtual bool setNubInterruptVectors(IOService *nub, const UInt32 vectors[], UInt32 vectorCount);
	virtual bool setNubInterruptVector(IOService *nub, UInt32 vector);
	virtual IOReturn callPlatformFunction(const OSSymbol *functionName, bool waitForFunction, void *param1, void *param2, void *param3, void *param4) APPLE_KEXT_OVERRIDE;
	virtual bool getModelName(char *name, int maxLengh) APPLE_KEXT_OVERRIDE;
	virtual bool getMachineName(char *name, int maxLength) APPLE_KEXT_OVERRIDE;
};

#endif /* ! _IOKIT_APPLEI386PLATFORM_H */
