#pragma once

/* Ensure kernel build does not compile macOS DriverKit IMPL block in IOPCIDevice.cpp */
#if defined(__has_include)
#if __has_include(<TargetConditionals.h>)
#include <TargetConditionals.h>
#endif
#endif
#ifdef TARGET_OS_OSX
#undef TARGET_OS_OSX
#endif
#define TARGET_OS_OSX 0

/* DriverKit-style IMPL arg macros needed by IOPCIDevice.cpp in kernel build */
#ifndef IOPCIDevice__ManageSession_Args
#define IOPCIDevice__ManageSession_Args \\
    IOService * forClient, \\
    bool openClient, \\
    IOOptionBits openOptions
#endif

#ifndef IOPCIDevice__MemoryAccess_Args
#define IOPCIDevice__MemoryAccess_Args \\
    uint64_t operation, \\
    uint64_t offset, \\
    uint64_t data, \\
    uint64_t * readData, \\
    IOService * forClient, \\
    IOOptionBits options
#endif

#ifndef IOPCIDevice__CopyDeviceMemoryWithIndex_Args
#define IOPCIDevice__CopyDeviceMemoryWithIndex_Args \\
    uint64_t memoryIndex, \\
    IOMemoryDescriptor ** returnMemory, \\
    IOService * forClient
#endif

#ifndef IOPCIDevice_FindPCICapability_Args
#define IOPCIDevice_FindPCICapability_Args \\
    uint32_t capabilityID, \\
    uint64_t searchOffset, \\
    uint64_t * foundCapabilityOffset
#endif

#ifndef IOPCIDevice_GetBusDeviceFunction_Args
#define IOPCIDevice_GetBusDeviceFunction_Args \\
    uint8_t * returnBusNumber, \\
    uint8_t * returnDeviceNumber, \\
    uint8_t * returnFunctionNumber
#endif

#ifndef IOPCIDevice_HasPCIPowerManagement_Args
#define IOPCIDevice_HasPCIPowerManagement_Args \\
    uint64_t state
#endif

#ifndef IOPCIDevice_EnablePCIPowerManagement_Args
#define IOPCIDevice_EnablePCIPowerManagement_Args \\
    uint64_t state
#endif
