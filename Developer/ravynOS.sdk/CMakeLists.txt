file(COPY ../ravynOS.platform/Info.plist
    DESTINATION ${ROOT_BINARY_DIR}/Developer/Platforms/ravynOS.platform/)
file(CREATE_LINK ravynOS.platform
    ${ROOT_BINARY_DIR}/Developer/Platforms/MacOSX.platform SYMBOLIC)

set(FPATH /System/Library/Frameworks/System.framework)

add_custom_target(ravynOS_sdkPrep ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory ${RAVYN_SDKROOT}${FPATH}/Versions/B
    COMMAND ${CMAKE_COMMAND} -E create_symlink Versions/Current/PrivateHeaders ${RAVYN_SDKROOT}${FPATH}/PrivateHeaders
    COMMAND ${CMAKE_COMMAND} -E create_symlink Versions/Current/Resources ${RAVYN_SDKROOT}${FPATH}/Resources
    COMMAND ${CMAKE_COMMAND} -E create_symlink B ${RAVYN_SDKROOT}${FPATH}/Versions/Current
    COMMAND ${CMAKE_COMMAND} -E create_symlink Versions/Current/System ${RAVYN_SDKROOT}${FPATH}/System
    COMMAND ${CMAKE_COMMAND} -E create_symlink ../../../../../../usr/lib/libSystem.B.dylib ${RAVYN_SDKROOT}${FPATH}/Versions/B/System
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
file(COPY usr USE_SOURCE_PERMISSIONS DESTINATION ${RAVYN_SDKROOT})
execute_process(COMMAND ${ROOT_SOURCE_DIR}/Kernel/xnu/bsd/sys/make_symbol_aliasing.sh
    ${RAVYN_SDKROOT} ${RAVYN_SDKROOT}/usr/include/sys/_symbol_aliasing.h)

file(CREATE_LINK Versions/B/Headers ${RAVYN_SDKROOT}/System/Library/Frameworks/System.framework/Headers SYMBOLIC)
file(CREATE_LINK Versions/B/PrivateHeaders ${RAVYN_SDKROOT}/System/Library/Frameworks/System.framework/PrivateHeaders SYMBOLIC)
file(CREATE_LINK Versions/A/Headers ${RAVYN_SDKROOT}/System/Library/Frameworks/Kernel.framework/Headers SYMBOLIC)
file(CREATE_LINK Versions/A/PrivateHeaders ${RAVYN_SDKROOT}/System/Library/Frameworks/Kernel.framework/PrivateHeaders SYMBOLIC)
