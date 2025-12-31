file(COPY ${CMAKE_SOURCE_DIR}/Platforms/ravynOS.platform/Info.plist
    DESTINATION ${ROOT_BINARY_DIR}/Developer/Platforms/ravynOS.platform/)
file(CREATE_LINK ravynOS.platform
    ${ROOT_BINARY_DIR}/Developer/Platforms/MacOSX.platform SYMBOLIC)

add_custom_target(ravynOS_sdkPrep ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory ${RAVYN_SDKROOT}/System/Library/Frameworks
    COMMAND ${CMAKE_COMMAND} -E make_directory ${RAVYN_SDKROOT}/usr/local/include
    COMMAND ${CMAKE_COMMAND} -E make_directory ${RAVYN_SDKROOT}/usr/local/lib
    COMMAND ${CMAKE_COMMAND} -E make_directory ${RAVYN_SDKROOT}/usr/local/bin
    COMMAND ${CMAKE_COMMAND} -E make_directory ${RAVYN_SDKROOT}/usr/include
    COMMAND ${CMAKE_COMMAND} -E make_directory ${RAVYN_SDKROOT}/usr/lib
    COMMAND ${CMAKE_COMMAND} -E make_directory ${RAVYN_SDKROOT}/usr/bin
    COMMAND ${CMAKE_COMMAND} -E copy 
        ${CMAKE_CURRENT_SOURCE_DIR}/SDKSettings.plist
        ${CMAKE_CURRENT_SOURCE_DIR}/SDKSettings.json
        ${CMAKE_CURRENT_SOURCE_DIR}/Entitlements.plist
        ${RAVYN_SDKROOT}/
)

add_library(ravynOS_sdk OBJECT stub.c)
add_dependencies(ravynOS_sdk ravynOS_sdkPrep)

set(architecture architecture/byte_order.h architecture/alignment.h)
set(arch_i386
    architecture/i386/alignment.h architecture/i386/asm_help.h
    architecture/i386/byte_order.h architecture/i386/cpu.h
    architecture/i386/desc.h architecture/i386/fpu.h
    architecture/i386/frame.h architecture/i386/io.h
    architecture/i386/pio.h architecture/i386/reg_help.h
    architecture/i386/sel.h architecture/i386/table.h
    architecture/i386/tss.h)
set(arch_arm architecture/arm/asm_help.h architecture/arm/byte_order.h
    architecture/arm/cframe.h architecture/arm/reg_help.h)
set(os os/availability.h)
set(usr_include
    AssertMacros.h TargetConditionals.h Availability.h AvailabilityInternal.h
    AvailabilityInternalLegacy.h AvailabilityMacros.h AvailabilityVersions.h
    ${ROOT_SOURCE_DIR}/Kernel/xnu/EXTERNAL_HEADERS/stdarg.h
)

file(GLOB_RECURSE kernelFramework
    MacTypes.h MacErrors.h ConditionalMacros.h Endian.h
)

file(COPY ${usr_include} DESTINATION ${RAVYN_SDKROOT}/usr/include/)

file(COPY ${architecture} DESTINATION ${RAVYN_SDKROOT}/usr/include/architecture/)
file(COPY ${arch_arm} DESTINATION ${RAVYN_SDKROOT}/usr/include/architecture/arm/)
file(COPY ${arch_i386} DESTINATION ${RAVYN_SDKROOT}/usr/include/architecture/i386/)

file(COPY ${os} DESTINATION ${RAVYN_SDKROOT}/usr/include/os/)

file(COPY ${usr_include} DESTINATION
    ${RAVYN_SDKROOT}/System/Library/Frameworks/System.framework/Versions/B/PrivateHeaders/)

file(COPY ${kernelFramework} DESTINATION
    ${RAVYN_SDKROOT}/System/Library/Frameworks/Kernel.framework/Versions/A/Headers)
file(COPY ${architecture} DESTINATION
    ${RAVYN_SDKROOT}/System/Library/Frameworks/Kernel.framework/Versions/A/PrivateHeaders/architecture)
file(COPY ${arch_i386} DESTINATION
    ${RAVYN_SDKROOT}/System/Library/Frameworks/Kernel.framework/Versions/A/PrivateHeaders/architecture/i386/)
file(COPY ${arch_arm} DESTINATION
    ${RAVYN_SDKROOT}/System/Library/Frameworks/Kernel.framework/Versions/A/PrivateHeaders/architecture/arm/)

file(COPY ${os} DESTINATION
    ${RAVYN_SDKROOT}/System/Library/Frameworks/Kernel.framework/Versions/A/Headers/os)

file(CREATE_LINK Versions/B/Headers ${RAVYN_SDKROOT}/System/Library/Frameworks/System.framework/Headers SYMBOLIC)
file(CREATE_LINK Versions/B/PrivateHeaders ${RAVYN_SDKROOT}/System/Library/Frameworks/System.framework/PrivateHeaders SYMBOLIC)
file(CREATE_LINK Versions/A/Headers ${RAVYN_SDKROOT}/System/Library/Frameworks/Kernel.framework/Headers SYMBOLIC)
file(CREATE_LINK Versions/A/PrivateHeaders ${RAVYN_SDKROOT}/System/Library/Frameworks/Kernel.framework/PrivateHeaders SYMBOLIC)

add_custom_target(sdk_Libc_headers DEPENDS ravynOS_sdkPrep unifdef
    COMMAND
        SDK_INSTALL_ROOT=/
        SRCROOT=${ROOT_SOURCE_DIR}/Libraries/Libc
        DSTROOT=${RAVYN_SDKROOT} PLATFORM_NAME=macosx
        ARCHS=X86_64 VARIANT_PLATFORM_NAME=macosx 
        DERIVED_FILES_DIR=${RAVYN_SDKROOT}/usr/include
        PATH=$ENV{PATH}:${CMAKE_BINARY_DIR}/unifdef
        ${ROOT_SOURCE_DIR}/Libraries/Libc/xcodescripts/headers.sh
)
add_dependencies(ravynOS_sdk sdk_Libc_headers)

add_custom_target(sdk_pthread_headers DEPENDS ravynOS_sdkPrep unifdef
    WORKING_DIRECTORY ${ROOT_SOURCE_DIR}/Libraries/libpthread
    COMMAND
        SDK_INSTALL_ROOT=/ DSTROOT=${RAVYN_SDKROOT} sh 
            ${ROOT_SOURCE_DIR}/Libraries/libpthread/xcodescripts/install-sys-headers.sh
    COMMAND
        SDK_INSTALL_ROOT=/ DSTROOT=${RAVYN_SDKROOT} sh 
            ${ROOT_SOURCE_DIR}/Libraries/libpthread/xcodescripts/install-symlinks.sh
)
add_dependencies(ravynOS_sdk sdk_pthread_headers)
