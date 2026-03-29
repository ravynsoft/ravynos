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

#include <IOKit/IOLib.h>
#include <IOKit/assert.h>
#include <IOKit/system.h>
#include <IOKit/IORegistryEntry.h>
#include <IOKit/platform/ApplePlatformExpert.h>
#include <libkern/c++/OSContainers.h>
#include <libkern/c++/OSUnserialize.h>
#include <pexpert/i386/boot.h>

extern "C" {
#include <i386/cpuid.h>
#include <pexpert/i386/protos.h>
}

#include "AppleI386PlatformExpert.h"

enum {
	kIRQAvailable   = 0,
	kIRQExclusive   = 1,
	kIRQSharable    = 2,
	kSystemIRQCount = 16
};

static struct {
	UInt16 consumers;
	UInt16 status;
} IRQ[kSystemIRQCount];

static IOLock *ResourceLock;

class AppleI386PlatformExpertGlobals {
public:
	bool isValid;
	AppleI386PlatformExpertGlobals();
	~AppleI386PlatformExpertGlobals();
};

static AppleI386PlatformExpertGlobals AppleI386PlatformExpertGlobals;
AppleI386PlatformExpertGlobals::AppleI386PlatformExpertGlobals() {
	ResourceLock = IOLockAlloc();
	bzero(IRQ, sizeof(IRQ));
}

AppleI386PlatformExpertGlobals::~AppleI386PlatformExpertGlobals() {
	if (ResourceLock) IOLockFree(ResourceLock);
}

#pragma mark -

#define super IOPlatformExpert

OSDefineMetaClassAndStructors(AppleI386PlatformExpert, IOPlatformExpert);

IOService *AppleI386PlatformExpert::probe(IOService *provider, SInt32 *score) {
    kprintf("PE: probe %p\n", provider);
	if (score != 0) *score = 10000;
	return this;
}

bool AppleI386PlatformExpert::init(OSDictionary *properties) {
    kprintf("PE: init %p\n", properties);
	if (!super::init()) return false;

	OSString *name = (OSString *)getProperty("InterruptControllerName");
	if (name == 0) name = OSString::withCStringNoCopy("AppleI386CPUInterruptController");
	_interruptControllerName = OSSymbol::withString(name);

	return true;
}

bool AppleI386PlatformExpert::start(IOService *provider) {
    kprintf("PE: start %p\n", provider);
	setBootROMType(kBootROMTypeNewWorld);

	if (!super::start(provider)) return false;
	PE_halt_restart = handlePEHaltRestart;
	registerService();

	// Hack: Initialize AppleI386CPU ourself because no one else will.
	bootCPU = new AppleI386CPU;
	if (bootCPU == 0) return false;

	bootCPU->init();
	bootCPU->attach(0);
	if (!bootCPU->startCommon()) return false;

	return true;
}

bool AppleI386PlatformExpert::configure(IOService *provider) {
	OSArray *topLevel;
	OSDictionary *dict;
	IOService *nub;

    kprintf("PE: configure %p\n", provider);
	topLevel = OSDynamicCast(OSArray, getProperty("top-level"));

	if (topLevel) {
		while ((dict = OSDynamicCast(OSDictionary, topLevel->getObject(0)))) {
			dict->retain();
			topLevel->removeObject(0);
			nub = createNub(dict);
			if (nub == 0) continue;

			dict->release();
			nub->attach(this);
			nub->registerService();
		}
	}

	return true;
}

bool AppleI386PlatformExpert::matchNubWithPropertyTable(IOService *nub, OSDictionary *table) {
	OSString *nameProp;
	OSString *match;

        kprintf("PE: matchNub %p\n", nub);

	if ((nameProp = (OSString *)nub->getProperty(gIONameKey)) == 0) return false;
	if ((match = (OSString *)table->getObject(gIONameMatchKey)) == 0) return false;

	return match->isEqualTo(nameProp);
}

IOService *AppleI386PlatformExpert::createNub(OSDictionary *from) {
	IOService *nub;

    kprintf("PE: createNub %p\n", from);
	nub = super::createNub(from);
	if (nub) {
		const char *name = nub->getName();

		if (strcmp(name, "pci") == 0) {
			// TODO: Get the PCI info from the boot args
			// and set it as the `pci-bus-info` property in the `from` dict.
		} else if (strcmp(name, "bios") == 0) {
			setupBIOS(nub);
		} else if (strcmp(name, "8259-pic") == 0) {
			setupPIC(nub);
		}
	}

	return nub;
}

void AppleI386PlatformExpert::setupPIC(IOService *nub) {
	int i;
	OSDictionary *propTable;
	OSArray *controller;
	OSArray *specifier;
	OSData *tmpData;
	long tmpLong;

	propTable = nub->getPropertyTable();

	// For the moment... assume a classic 8259 interrupt controller
	// with 16 interrupts. Later, this will be changed to detect
	// an APIC and/or MP-Table and then will set the nubs appropriately.

	specifier = OSArray::withCapacity(kSystemIRQCount);
	assert(specifier);

	for (i = 0; i < kSystemIRQCount; i++) {
		tmpLong = i;
		tmpData = OSData::withBytes(&tmpLong, sizeof(tmpLong));
		specifier->setObject(tmpData);
	}

	controller = OSArray::withCapacity(kSystemIRQCount);
	assert(controller);

	for (i = 0; i < kSystemIRQCount; i++) controller->setObject(_interruptControllerName);

	propTable->setObject(gIOInterruptControllersKey, controller);
	propTable->setObject(gIOInterruptSpecifiersKey, specifier);

	specifier->release();
	controller->release();
}

void AppleI386PlatformExpert::setupBIOS(IOService *nub) {
	// TODO: Implement this function.
	// This function is dependent upon being able to retrieve the
	// PCI bus data. While the booter does collect some PCI data,
	// but it does not include the data needed here.
}

bool AppleI386PlatformExpert::getMachineName(char *name, int maxLength) {
	strncpy(name, "x86", maxLength);
	return true;
}

bool AppleI386PlatformExpert::getModelName(char *name, int maxLengh) {
	i386_cpu_info_t *cpuid_cpu_info = cpuid_info();

	if (cpuid_cpu_info->cpuid_brand_string[0] != '\0') {
		strncpy(name, cpuid_cpu_info->cpuid_brand_string, maxLengh);
	} else {
		strncpy(name, cpuid_cpu_info->cpuid_model_string, maxLengh);
	}

	return true;
}

int AppleI386PlatformExpert::handlePEHaltRestart(unsigned int type) {
	int ret = -1;
	int temporary_sum = 0;

	switch (type) {
		case kPERestartCPU:
			// Note: This code may or may not work reliably on all systems.
			// The original author of it indicated that it should work on any
			// system with a compliant PCI controller.

			// Obtained from: http://smackerelofopinion.blogspot.nl/2009/06/rebooting-pc.html
			outb(0xCF9, 0x02);

			// A delay of some sort is required here.
			temporary_sum = 2;
			temporary_sum += 2;

			outb(0xCF9, 0x04);

			// This should not be reached, but just in case...
			break;

		case kPEHaltCPU:
		default:
			ret = -1;
			break;
	}

	return ret;
}

bool AppleI386PlatformExpert::setNubInterruptVectors(IOService *nub, const UInt32 *vectors, UInt32 vectorCount) {
	OSArray *controller = 0;
	OSArray *specifier = 0;
	bool success = false;

	if (vectorCount == 0) {
		nub->removeProperty(gIOInterruptControllersKey);
		nub->removeProperty(gIOInterruptSpecifiersKey);
		return true;
	}

	specifier = OSArray::withCapacity(vectorCount);
	controller = OSArray::withCapacity(vectorCount);
	if (!specifier || !controller) goto done;

	for (UInt32 i = 0; i < vectorCount; i++) {
		OSData *data = OSData::withBytes(&vectors[i], sizeof(vectors[i]));
		specifier->setObject(data);
		controller->setObject(_interruptControllerName);
		if (data) data->release();
	}

	nub->setProperty(gIOInterruptControllersKey, controller);
	nub->setProperty(gIOInterruptSpecifiersKey, specifier);
	success = true;

done:
	if (specifier) specifier->release();
	if (controller) controller->release();
	return success;
}

bool AppleI386PlatformExpert::setNubInterruptVector(IOService *nub, UInt32 vector) {
	return setNubInterruptVectors(nub, &vector, 1);
}

IOReturn AppleI386PlatformExpert::callPlatformFunction(const OSSymbol *functionName, bool waitForFunction, void *param1, void *param2, void *param3, void *param4) {
	bool ok;

	if (functionName->isEqualTo("SetDeviceInterrupts")) {
		IOService *nub = (IOService *)param1;
		UInt32 *vectors = (UInt32 *)param2;
		UInt32 vectorCount = (UInt32)((UInt64)param3);
		bool exclusive = (bool)param4;

		if (vectorCount != 1) return kIOReturnBadArgument;

		ok = reserveSystemInterrupt(nub, vectors[0], exclusive);
		if (ok == false) return kIOReturnNoResources;

		ok = setNubInterruptVector(nub, vectors[0]);
		if (ok == false) releaseSystemInterrupt(nub, vectors[0], exclusive);

		return ok ? kIOReturnSuccess : kIOReturnNoMemory;
	} else if (functionName->isEqualTo("SetBusClockRateMHz")) {
		UInt32 rateMHz = (UInt32)((UInt64)param1);
		gPEClockFrequencyInfo.bus_clock_rate_hz = rateMHz * 1000000;
		return kIOReturnSuccess;
	} else if (functionName->isEqualTo("SetCPUClockRateMHz")) {
		UInt32 rateMHz = (UInt32)((UInt64)param1);
		gPEClockFrequencyInfo.cpu_clock_rate_hz = rateMHz * 1000000;
		return kIOReturnSuccess;
	}

	return super::callPlatformFunction(functionName, waitForFunction, param1, param2, param3, param4);
}

bool AppleI386PlatformExpert::reserveSystemInterrupt(IOService *client, UInt32 vectorNumber, bool exclusive) {
	bool ok = false;
	if (vectorNumber >= kSystemIRQCount) return ok;

	IOLockLock(ResourceLock);

	if (exclusive) {
		if (IRQ[vectorNumber].status == kIRQAvailable) {
			IRQ[vectorNumber].status = kIRQExclusive;
			IRQ[vectorNumber].consumers = 1;
			ok = true;
		}
	} else {
		if (IRQ[vectorNumber].status == kIRQAvailable || IRQ[vectorNumber].status == kIRQSharable) {
			IRQ[vectorNumber].status = kIRQSharable;
			IRQ[vectorNumber].consumers++;
			ok = true;
		}
	}

	IOLockUnlock(ResourceLock);
	return ok;
}

void AppleI386PlatformExpert::releaseSystemInterrupt(IOService *client, UInt32 vectorNumber, bool exclusive) {
	if (vectorNumber >= kSystemIRQCount) return;
	IOLockLock(ResourceLock);

	if (exclusive) {
		if (IRQ[vectorNumber].status == kIRQExclusive) {
			IRQ[vectorNumber].status = kIRQAvailable;
			IRQ[vectorNumber].consumers = 0;
		}
	} else {
		if (IRQ[vectorNumber].status == kIRQSharable && --IRQ[vectorNumber].consumers == 0) {
			IRQ[vectorNumber].status = kIRQAvailable;
		}
	}

	IOLockUnlock(ResourceLock);
}
