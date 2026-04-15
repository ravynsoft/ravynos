add_kext_bundle(IOPCIFamily KERNEL_PRIVATE
    INFO_PLIST Info.plist
    BUNDLE_IDENTIFIER com.apple.iokit.IOPCIFamily
    BUNDLE_VERSION 2.9)

target_sources(IOPCIFamily PRIVATE
    IOPCIBridge.cpp
    IOPCIDevice.cpp
    PCIDriverKit/IOPCIDevice.iig
    IOPCIDeviceI386.cpp
    IOPCIConfigurator.cpp
    IOPCIDeviceMappedIO.cpp
    IOPCIRange.cpp
    IOPCIMessagedInterruptController.cpp
)

add_dependencies(IOPCIFamily kextBuild)

target_include_directories(IOPCIFamily PRIVATE
    .
    ../../xnu/osfmk
    ../../xnu/pexpert
    ${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/osfmk
    ${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/pexpert
    ../../xnu/bsd
    ../../xnu/iokit
    ../../xnu/libkern
    ../../xnu
    ../IOACPIFamily/include
    ${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/libkern
    ${ROOT_SOURCE_DIR}/Kernel/xnu/EXTERNAL_HEADERS)
    
set_property(TARGET IOPCIFamily PROPERTY CXX_STANDARD 11)

target_compile_definitions(IOPCIFamily PRIVATE
    PRIVATE=1
    XNU_KERNEL_PRIVATE
    ACPI_SUPPORT=1
)

set(IOPCIFAMILY_IIG_SHIM "${CMAKE_CURRENT_BINARY_DIR}/iopcifamily_iig_shims.h")
file(WRITE "${IOPCIFAMILY_IIG_SHIM}" "
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
")

set_source_files_properties(IOPCIDevice.cpp PROPERTIES
    COMPILE_OPTIONS "-include;${IOPCIFAMILY_IIG_SHIM}")

target_compile_options(IOPCIFamily PRIVATE
    -Wno-implicit-int-conversion
    -Wno-inconsistent-missing-override
    -Wno-sign-conversion -Wno-shadow
)

target_link_options(IOPCIFamily PRIVATE
    -mmacos-version-min=${MACOS_VERSION_MIN}
)

install(TARGETS IOPCIFamily DESTINATION System/Library/Extensions COMPONENT BaseSystem)

#add_library(PCIDriverKitPEX8733 STATIC
#	./PEX8733/PCIDriverKitPEX8733/PCIDriverKitPEX8733.iig
#	./PEX8733/PCIDriverKitPEX8733/PCIDriverKitPEX8733.cpp
#)
#add_library(PCIDriverKit STATIC
#	./IOPCIDevice.cpp
#	./PCIDriverKit/IOPCIDevice.iig
#)
