/*
 * Copyright (C) 2026 Zoe Knox. All rights reserved.
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
 * project, and extensively modified by Zoe Knox in 2026 to support ravynOS.
 * This notice is included in support of clause 2.2(b) of the License.
 */

#ifndef _IOKIT_APPLEI386PLATFORM_H
#define _IOKIT_APPLEI386PLATFORM_H

#include <IOKit/IOPlatformExpert.h>
#include <IOKit/acpi/IOACPIPlatformDevice.h>
#include <IOKit/acpi/IOACPITypes.h>

#include "ACPICPU.h"
#include "ACPIPCIBridge.h"

#define PE_VERBOSE 1

struct RSDP
{
    uint8_t signature[8];
    uint8_t checksum;
    char OEMID[6];
    uint8_t revision;
    uint32_t rsdtAddress;

    // v.2
    uint32_t length;
    uint64_t xsdtAddress;
    uint8_t extChecksum;
    uint8_t reserved[3];
} __attribute__((packed));

struct XSDT
{
    uint8_t signature[4];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    char OEMID[6];
    char OEMTableID[8];
    uint32_t OEMRevision;
    uint32_t creatorID;
    uint32_t creatorRevision;
    uint64_t tables[0];
} __attribute__((packed));

typedef struct {
    uint8_t type;
    uint8_t length;
} __attribute__((packed)) MADT_Record;

struct MADT
{
    uint8_t signature[4];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    char OEMID[6];
    char OEMTableID[8];
    uint32_t OEMRevision;
    uint32_t creatorID;
    uint32_t creatorRevision;
    uint32_t lapicAddress;
    uint32_t flags;
    MADT_Record records[1];
} __attribute__((packed));


enum {
	kIRQAvailable   = 0,
	kIRQExclusive   = 1,
	kIRQSharable    = 2,
	kSystemIRQCount = 16
};

class IOACPIPlatformExpertGlobals {
public:
    bool isInitialized;
    IOACPIPlatformExpertGlobals();
    ~IOACPIPlatformExpertGlobals();
    inline bool isValid() const;
};

class ACPIPlatformExpert : public IODTPlatformExpert {
    OSDeclareDefaultStructors(ACPIPlatformExpert)
    friend class IOACPIPlatformDevice;

 private:
    const char *RSDP_SIGNATURE = "RSD PTR";
    
    const OSSymbol *_interruptControllerName;
    OSSet * topLevel;

    void PE_Log(const char *fmt, ...);
    bool parseACPI(IOService *provider);
    void parseAPIC(void * table, IOService * nub);
    void parseFADT(void * table, IOService * nub);
    void parseMCFG(void * table, IOService * nub);
    static int handlePEHaltRestart(unsigned int type);
    IOService * createNub(OSDictionary *dict, IORegistryEntry *from = NULL);


protected:
    virtual SInt32 installDeviceInterruptForFixedEvent(IOService *device,
                                                       UInt32 fixedEvent);
    virtual SInt32 installDeviceInterruptForGPE(IOService *device,
                                                UInt32 gpeNumber,
                                                void *gpeBlockDevice,
                                                IOOptionBits options);

    virtual IOReturn acquireGlobalLock(IOService *client,
                                       UInt32 *lockToken,
                                       const mach_timespec_t *timeout);
    virtual void releaseGlobalLock(IOService *client,
                                   UInt32 lockToken);

    virtual IOReturn validateObject(IOACPIPlatformDevice *device,
                                    const OSSymbol *objectName);
    virtual IOReturn validateObject(IOACPIPlatformDevice *device,
                                    const char *objectName);

    virtual IOReturn evaluateObject(IOACPIPlatformDevice *device,
                                    const OSSymbol *objectName,
                                    OSObject **result,
                                    OSObject *params[],
                                    IOItemCount paramCount,
                                    IOOptionBits options);
    virtual IOReturn evaluateObject(IOACPIPlatformDevice *device,
                                    const char *objectName,
                                    OSObject **result,
                                    OSObject *params[],
                                    IOItemCount paramCount,
                                    IOOptionBits options);

    virtual const OSData *getACPITableData(const char *tableName,
                                           UInt32 tableInstance);

    virtual IOReturn registerAddressSpaceHandler(IOACPIPlatformDevice *device,
                                                 IOACPIAddressSpaceID spaceID,
                                                 IOACPIAddressSpaceHandler handler,
                                                 void *context,
                                                 IOOptionBits options);

    virtual void unregisterAddressSpaceHandler(IOACPIPlatformDevice *device,
                                               IOACPIAddressSpaceID spaceID,
                                               IOACPIAddressSpaceHandler handler,
                                               IOOptionBits options);

    virtual IOReturn readAddressSpace(UInt64 *value,
                                      IOACPIAddressSpaceID spaceID,
                                      IOACPIAddress address,
                                      UInt32 bitWidth,
                                      UInt32 bitOffset,
                                      IOOptionBits options);

    virtual IOReturn writeAddressSpace(UInt64 value,
                                       IOACPIAddressSpaceID spaceID,
                                       IOACPIAddress address,
                                       UInt32 bitWidth,
                                       UInt32 bitOffset,
                                       IOOptionBits options);

    virtual IOReturn setDevicePowerState(IOACPIPlatformDevice *device,
                                         UInt32 powerState);
    virtual IOReturn getDevicePowerState(IOACPIPlatformDevice *device,
                                         UInt32 *powerState);
    virtual IOReturn setDeviceWakeEnable(IOACPIPlatformDevice *device,
                                         bool enable);

    virtual const char * deleteList( void );
    virtual const char * excludeList( void );

public:
    virtual bool init(OSDictionary *properties) APPLE_KEXT_OVERRIDE;
    virtual IOService *probe(IOService *provider, SInt32 *score) APPLE_KEXT_OVERRIDE;
    virtual bool start(IOService *provider) APPLE_KEXT_OVERRIDE;
    virtual bool configure(IOService *provider) APPLE_KEXT_OVERRIDE;
    virtual bool matchNubWithPropertyTable(IOService *nub, OSDictionary *table);
    virtual bool reserveSystemInterrupt(IOService *client, UInt32 vectorNumber, bool exclusive);
    virtual void releaseSystemInterrupt(IOService *client, UInt32 vectorNumber, bool exclusive);
    virtual bool setNubInterruptVectors(IOService *nub, const UInt32 vectors[], UInt32 vectorCount);
    virtual bool setNubInterruptVector(IOService *nub, UInt32 vector);
    virtual IOReturn callPlatformFunction(const OSSymbol *functionName, bool waitForFunction, void *param1, void *param2, void *param3, void *param4) APPLE_KEXT_OVERRIDE;
    virtual bool getModelName(char *name, int maxLengh) APPLE_KEXT_OVERRIDE;
    virtual bool getMachineName(char *name, int maxLength) APPLE_KEXT_OVERRIDE;
};

#endif /* ! _IOKIT_APPLEI386PLATFORM_H */
