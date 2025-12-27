file(COPY ${CMAKE_SOURCE_DIR}/Developer/Platforms/ravynOS.platform/Info.plist
    DESTINATION ${CMAKE_BINARY_DIR}/Developer/Platforms/ravynOS.platform/)
file(CREATE_LINK ravynOS.platform
    ${CMAKE_BINARY_DIR}/Developer/Platforms/MacOSX.platform SYMBOLIC)

add_library(ravynOS_sdk OBJECT stub.c)
foreach(file SDKSettings.plist SDKSettings.json Entitlements.plist)
    file(COPY_FILE ${CMAKE_CURRENT_SOURCE_DIR}/${file}
        ${CMAKE_CURRENT_BINARY_DIR}/${file} ONLY_IF_DIFFERENT)
endforeach()
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/System/Library/Frameworks)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/usr/local/include)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/usr/local/lib)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/usr/local/bin)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/usr/include)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/usr/lib)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/usr/bin)

file(GLOB_RECURSE HEADER_SET LIST_DIRECTORIES false
    ${CMAKE_SOURCE_DIR}/TargetConditionals.h
    ${CMAKE_SOURCE_DIR}/ConditionalMacros.h
    ${CMAKE_SOURCE_DIR}/MacTypes.h
    ${CMAKE_SOURCE_DIR}/Kernel/xnu/EXTERNAL_HEADERS/Availability*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/os
    ${CMAKE_CURRENT_SOURCE_DIR}/architecture/byte_order.h
    ${CMAKE_CURRENT_SOURCE_DIR}/architecture/alignment.h
    ${CMAKE_CURRENT_SOURCE_DIR}/architecture/i386
    ${CMAKE_CURRENT_SOURCE_DIR}/architecture/arm
)
file(COPY ${HEADER_SET} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/usr/include/)
file(COPY ${HEADER_SET} DESTINATION
    ${CMAKE_CURRENT_BINARY_DIR}/System/Library/Frameworks/System.framework/Versions/B/PrivateHeaders/)
add_dependencies(ravynOS_sdk CarbonHeaders)

add_custom_target(sdk_Libc_manpages DEPENDS ravynOS_sdk
    COMMAND
        SRCROOT=${CMAKE_SOURCE_DIR}/Libraries/Libc 
        DSTROOT=${RAVYN_SDKROOT}
        PLATFORM_NAME=macosx
        ${CMAKE_SOURCE_DIR}/Libraries/Libc/xcodescripts/manpages.sh
)

add_custom_target(sdk_Libc_headers DEPENDS ravynOS_sdk
    COMMAND
        SDK_INSTALL_ROOT=/
        SRCROOT=${CMAKE_SOURCE_DIR}/Libraries/Libc 
        DSTROOT=${RAVYN_SDKROOT}
        PLATFORM_NAME=macosx
        ARCHS=X86_64 VARIANT_PLATFORM_NAME=macosx 
        DERIVED_FILES_DIR=${DSTROOT}/usr/include
        ${CMAKE_SOURCE_DIR}/Libraries/Libc/xcodescripts/headers.sh
)
