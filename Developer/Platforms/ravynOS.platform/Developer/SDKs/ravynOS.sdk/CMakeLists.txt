file(COPY ${CMAKE_SOURCE_DIR}/Developer/Platforms/ravynOS.platform/Info.plist
    DESTINATION ${CMAKE_BINARY_DIR}/Developer/Platforms/ravynOS.platform/)
file(CREATE_LINK ravynOS.platform
    ${CMAKE_BINARY_DIR}/Developer/Platforms/MacOSX.platform SYMBOLIC)

add_custom_target(ravynOS_sdkPrep ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/System/Library/Frameworks
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/usr/local/include
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/usr/local/lib
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/usr/local/bin
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/usr/include
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/usr/lib
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/usr/bin
    COMMAND ${CMAKE_COMMAND} -E copy 
        ${CMAKE_CURRENT_SOURCE_DIR}/SDKSettings.plist
        ${CMAKE_CURRENT_SOURCE_DIR}/SDKSettings.json
        ${CMAKE_CURRENT_SOURCE_DIR}/Entitlements.plist
        ${CMAKE_CURRENT_BINARY_DIR}/
)

add_library(ravynOS_sdk OBJECT stub.c)
add_dependencies(ravynOS_sdk ravynOS_sdkPrep)

file(GLOB_RECURSE architecture LIST_DIRECTORIES false architecture/*.h)
file(GLOB_RECURSE usr_include
    AssertMacros.h TargetConditionals.h Availability.h AvailabilityInternal.h
    AvailabilityInternalLegacy.h AvailabilityMacros.h AvailabilityVersions.h
    os ${ROOT_SOURCE_DIR}/Kernel/xnu/EXTERNAL_HEADERS/stdarg.h
)

file(GLOB_RECURSE kernelFramework
    MacTypes.h MacErrors.h ConditionalMacros.h Endian.h
)

file(COPY ${usr_include} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/usr/include/)
file(COPY ${architecture} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/usr/include/architecture/)

file(COPY ${usr_include} DESTINATION
    ${CMAKE_CURRENT_BINARY_DIR}/System/Library/Frameworks/System.framework/Versions/B/PrivateHeaders/)

file(COPY ${kernelFramework} DESTINATION
    ${CMAKE_CURRENT_BINARY_DIR}/System/Library/Frameworks/Kernel.framework/Versions/A/Headers)
file(COPY ${architecture} DESTINATION
    ${CMAKE_CURRENT_BINARY_DIR}/System/Library/Frameworks/Kernel.framework/Versions/A/Headers)
file(COPY ${architecture} DESTINATION
    ${CMAKE_CURRENT_BINARY_DIR}/System/Library/Frameworks/Kernel.framework/Versions/A/PrivateHeaders)

file(CREATE_LINK Versions/Current/Headers ${CMAKE_CURRENT_BINARY_DIR}/System/Library/Frameworks/System.framework/Headers SYMBOLIC)
file(CREATE_LINK Versions/Current/PrivateHeaders ${CMAKE_CURRENT_BINARY_DIR}/System/Library/Frameworks/System.framework/PrivateHeaders SYMBOLIC)
file(CREATE_LINK Versions/Current/Headers ${CMAKE_CURRENT_BINARY_DIR}/System/Library/Frameworks/Kernel.framework/Headers SYMBOLIC)
file(CREATE_LINK Versions/Current/PrivateHeaders ${CMAKE_CURRENT_BINARY_DIR}/System/Library/Frameworks/Kernel.framework/PrivateHeaders SYMBOLIC)

# add_custom_target(sdk_Libc_manpages DEPENDS ravynOS_sdkPrep
#     COMMAND
#         SDK_INSTALL_ROOT=/
#         SRCROOT=${CMAKE_SOURCE_DIR}/Libraries/Libc 
#         DSTROOT=${RAVYN_SDKROOT} PLATFORM_NAME=macosx
#         ARCHS=X86_64 VARIANT_PLATFORM_NAME=macosx
#         DERIVED_FILES_DIR=${RAVYN_SDKROOT}/usr/include
#         PATH=$ENV{PATH}:${CMAKE_BINARY_DIR}/Developer/unifdef
#         ${CMAKE_SOURCE_DIR}/Libraries/Libc/xcodescripts/manpages.sh
# )
# add_dependencies(ravynOS_sdk sdk_Libc_manpages)

add_custom_target(sdk_Libc_headers DEPENDS ravynOS_sdkPrep unifdef
    COMMAND
        SDK_INSTALL_ROOT=/
        SRCROOT=${CMAKE_SOURCE_DIR}/Libraries/Libc
        DSTROOT=${RAVYN_SDKROOT} PLATFORM_NAME=macosx
        ARCHS=X86_64 VARIANT_PLATFORM_NAME=macosx 
        DERIVED_FILES_DIR=${RAVYN_SDKROOT}/usr/include
        PATH=$ENV{PATH}:${CMAKE_BINARY_DIR}/Developer/unifdef
        ${CMAKE_SOURCE_DIR}/Libraries/Libc/xcodescripts/headers.sh
)
add_dependencies(ravynOS_sdk sdk_Libc_headers)
