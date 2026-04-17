/*
 * Copyright (c) 2019-2019 Apple Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 *
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */

#ifndef _IOKIT_UIOPCIDEVICE_H
#define _IOKIT_UIOPCIDEVICE_H

#if KERNEL

/*
 * Kernel-side IOPCIDevice is declared in <IOKit/pci/IOPCIDevice.h>.
 * That header expects IOPCIDevice_Methods via OSMetaClass dispatch macros.
 * Provide empty stubs here so kernel declarations compile cleanly.
 */
#ifndef IOPCIDevice_Methods
#define IOPCIDevice_Methods
#endif

#ifndef IOPCIDevice_KernelMethods
#define IOPCIDevice_KernelMethods
#endif

#ifndef IOPCIDevice_VirtualMethods
#define IOPCIDevice_VirtualMethods
#endif

#else /* !KERNEL */

#include <DriverKit/IOTypes.h>
#include <DriverKit/IOService.h>           /* .iig include */
#include <DriverKit/IOMemoryDescriptor.h>  /* .iig include */

/* source class IOPCIDevice IOPCIDevice.iig */

#if __DOCUMENTATION__

class IOPCIDevice : public IOService
{
public:
#pragma mark IOService Overrides
    virtual bool
    init() override;

    virtual void
    free() override;

#pragma mark Session Management
    kern_return_t
    Open(IOService * forClient,
         IOOptionBits options);

    void
    Close(IOService * forClient,
          IOOptionBits options);

#pragma mark Memory Accessors
    void
    ConfigurationRead32(uint64_t offset, uint32_t * readData);

    void
    ConfigurationRead16(uint64_t offset, uint16_t * readData);

    void
    ConfigurationRead8(uint64_t offset, uint8_t * readData);

    void
    ConfigurationWrite32(uint64_t offset, uint32_t data);

    void
    ConfigurationWrite16(uint64_t offset, uint16_t data);

    void
    ConfigurationWrite8(uint64_t offset, uint8_t data);

    void
    MemoryRead64(uint8_t memoryIndex, uint64_t offset, uint64_t * readData);

    void
    MemoryRead32(uint8_t memoryIndex, uint64_t offset, uint32_t * readData);

    void
    MemoryRead16(uint8_t memoryIndex, uint64_t offset, uint16_t * readData);

    void
    MemoryRead8(uint8_t memoryIndex, uint64_t offset, uint8_t * readData);

    void
    MemoryWrite64(uint8_t memoryIndex, uint64_t offset, uint64_t data);

    void
    MemoryWrite32(uint8_t memoryIndex, uint64_t offset, uint32_t data);

    void
    MemoryWrite16(uint8_t memoryIndex, uint64_t offset, uint16_t data);

    void
    MemoryWrite8(uint8_t memoryIndex, uint64_t offset, uint8_t data);

#pragma mark Configuration Space helpers
    virtual kern_return_t
    FindPCICapability(uint32_t capabilityID,
                      uint64_t searchOffset,
                      uint64_t * foundCapabilityOffset);

    virtual kern_return_t
    GetBusDeviceFunction(uint8_t * returnBusNumber,
                         uint8_t * returnDeviceNumber,
                         uint8_t * returnFunctionNumber);

#pragma mark Power Management
    virtual kern_return_t
    HasPCIPowerManagement(uint64_t state);

    virtual kern_return_t
    EnablePCIPowerManagement(uint64_t state);
};

#else /* __DOCUMENTATION__ */

/*
 * Placeholder IDs (no Apple IIG hash tool available in this environment).
 */
#define IOPCIDevice_FindPCICapability_ID            0x1000000000000001ULL
#define IOPCIDevice_GetBusDeviceFunction_ID         0x1000000000000002ULL
#define IOPCIDevice_HasPCIPowerManagement_ID        0x1000000000000003ULL
#define IOPCIDevice_EnablePCIPowerManagement_ID     0x1000000000000004ULL
#define IOPCIDevice__MemoryAccess_ID                0x1000000000000005ULL
#define IOPCIDevice__CopyDeviceMemoryWithIndex_ID   0x1000000000000006ULL
#define IOPCIDevice__ManageSession_ID               0x1000000000000007ULL

#define IOPCIDevice_FindPCICapability_Args \
        uint32_t capabilityID, \
        uint64_t searchOffset, \
        uint64_t * foundCapabilityOffset

#define IOPCIDevice_GetBusDeviceFunction_Args \
        uint8_t * returnBusNumber, \
        uint8_t * returnDeviceNumber, \
        uint8_t * returnFunctionNumber

#define IOPCIDevice_HasPCIPowerManagement_Args \
        uint64_t state

#define IOPCIDevice_EnablePCIPowerManagement_Args \
        uint64_t state

#define IOPCIDevice__MemoryAccess_Args \
        uint64_t operation, \
        uint64_t offset, \
        uint64_t data, \
        uint64_t * readData, \
        IOService * forClient, \
        IOOptionBits options

#define IOPCIDevice__CopyDeviceMemoryWithIndex_Args \
        uint64_t memoryIndex, \
        IOMemoryDescriptor ** returnMemory, \
        IOService * forClient

#define IOPCIDevice__ManageSession_Args \
        IOService * forClient, \
        bool openClient, \
        IOOptionBits openOptions

#define IOPCIDevice_Methods \
\
public:\
\
    virtual kern_return_t\
    Dispatch(const IORPC rpc) APPLE_KEXT_OVERRIDE;\
\
    static kern_return_t\
    _Dispatch(IOPCIDevice * self, const IORPC rpc);\
\
    kern_return_t\
    Open(\
        IOService * forClient,\
        IOOptionBits options);\
\
    void\
    Close(\
        IOService * forClient,\
        IOOptionBits options);\
\
    void\
    ConfigurationRead32(\
        uint64_t offset,\
        uint32_t * readData);\
\
    void\
    ConfigurationRead16(\
        uint64_t offset,\
        uint16_t * readData);\
\
    void\
    ConfigurationRead8(\
        uint64_t offset,\
        uint8_t * readData);\
\
    void\
    ConfigurationWrite32(\
        uint64_t offset,\
        uint32_t data);\
\
    void\
    ConfigurationWrite16(\
        uint64_t offset,\
        uint16_t data);\
\
    void\
    ConfigurationWrite8(\
        uint64_t offset,\
        uint8_t data);\
\
    void\
    MemoryRead64(\
        uint8_t memoryIndex,\
        uint64_t offset,\
        uint64_t * readData);\
\
    void\
    MemoryRead32(\
        uint8_t memoryIndex,\
        uint64_t offset,\
        uint32_t * readData);\
\
    void\
    MemoryRead16(\
        uint8_t memoryIndex,\
        uint64_t offset,\
        uint16_t * readData);\
\
    void\
    MemoryRead8(\
        uint8_t memoryIndex,\
        uint64_t offset,\
        uint8_t * readData);\
\
    void\
    MemoryWrite64(\
        uint8_t memoryIndex,\
        uint64_t offset,\
        uint64_t data);\
\
    void\
    MemoryWrite32(\
        uint8_t memoryIndex,\
        uint64_t offset,\
        uint32_t data);\
\
    void\
    MemoryWrite16(\
        uint8_t memoryIndex,\
        uint64_t offset,\
        uint16_t data);\
\
    void\
    MemoryWrite8(\
        uint8_t memoryIndex,\
        uint64_t offset,\
        uint8_t data);\
\
    kern_return_t\
    FindPCICapability(\
        uint32_t capabilityID,\
        uint64_t searchOffset,\
        uint64_t * foundCapabilityOffset,\
        OSDispatchMethod supermethod = NULL);\
\
    kern_return_t\
    GetBusDeviceFunction(\
        uint8_t * returnBusNumber,\
        uint8_t * returnDeviceNumber,\
        uint8_t * returnFunctionNumber,\
        OSDispatchMethod supermethod = NULL);\
\
    kern_return_t\
    HasPCIPowerManagement(\
        uint64_t state,\
        OSDispatchMethod supermethod = NULL);\
\
    kern_return_t\
    EnablePCIPowerManagement(\
        uint64_t state,\
        OSDispatchMethod supermethod = NULL);\
\
    kern_return_t\
    _MemoryAccess(\
        uint64_t operation,\
        uint64_t offset,\
        uint64_t data,\
        uint64_t * readData,\
        IOService * forClient,\
        IOOptionBits options,\
        OSDispatchMethod supermethod = NULL);\
\
    kern_return_t\
    _CopyDeviceMemoryWithIndex(\
        uint64_t memoryIndex,\
        IOMemoryDescriptor ** returnMemory,\
        IOService * forClient,\
        OSDispatchMethod supermethod = NULL);\
\
    kern_return_t\
    _ManageSession(\
        IOService * forClient,\
        bool openClient,\
        IOOptionBits openOptions,\
        OSDispatchMethod supermethod = NULL);\

#define IOPCIDevice_KernelMethods
#define IOPCIDevice_VirtualMethods

extern OSMetaClass * gIOPCIDeviceMetaClass;
extern const OSClassLoadInformation IOPCIDevice_Class;

class IOPCIDeviceMetaClass : public OSMetaClass
{
public:
    virtual kern_return_t
    New(OSObject * instance) override;
    virtual kern_return_t
    Dispatch(const IORPC rpc) override;
};

class IOPCIDeviceInterface : public OSInterface
{
public:
};

struct IOPCIDevice_IVars;
struct IOPCIDevice_LocalIVars;

class IOPCIDevice : public IOService, public IOPCIDeviceInterface
{
    friend class IOPCIDeviceMetaClass;

public:
#ifdef IOPCIDevice_DECLARE_IVARS
IOPCIDevice_DECLARE_IVARS
#else
    union
    {
        IOPCIDevice_IVars * ivars;
        IOPCIDevice_LocalIVars * lvars;
    };
#endif

    static OSMetaClass *
    sGetMetaClass() { return gIOPCIDeviceMetaClass; };

    using super = IOService;

    IOPCIDevice_Methods
    IOPCIDevice_VirtualMethods
};

#endif /* !__DOCUMENTATION__ */
#endif /* !KERNEL */

#endif /* ! _IOKIT_UIOPCIDEVICE_H */