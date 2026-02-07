file(COPY ../ravynOS.platform/Info.plist
    DESTINATION ${ROOT_BINARY_DIR}/Developer/Platforms/ravynOS.platform/)

set(FPATH /System/Library/Frameworks/System.framework)

file(MAKE_DIRECTORY ${RAVYN_SDKROOT}${FPATH}/Versions/B)
file(MAKE_DIRECTORY ${RAVYN_SDKROOT}/usr/local/include)
file(MAKE_DIRECTORY ${RAVYN_SDKROOT}/usr/local/lib)
file(MAKE_DIRECTORY ${RAVYN_SDKROOT}/usr/local/bin)
file(MAKE_DIRECTORY ${RAVYN_SDKROOT}/usr/include)
file(MAKE_DIRECTORY ${RAVYN_SDKROOT}/usr/lib)
file(MAKE_DIRECTORY ${RAVYN_SDKROOT}/usr/bin)
file(COPY
    ${CMAKE_CURRENT_SOURCE_DIR}/SDKSettings.plist
    ${CMAKE_CURRENT_SOURCE_DIR}/SDKSettings.json
    ${CMAKE_CURRENT_SOURCE_DIR}/Entitlements.plist
    DESTINATION ${RAVYN_SDKROOT}/
)
file(CREATE_LINK Versions/Current/PrivateHeaders ${RAVYN_SDKROOT}${FPATH}/PrivateHeaders SYMBOLIC)
file(CREATE_LINK Versions/Current/Resources ${RAVYN_SDKROOT}${FPATH}/Resources SYMBOLIC)
file(CREATE_LINK B ${RAVYN_SDKROOT}${FPATH}/Versions/Current SYMBOLIC)
file(CREATE_LINK Versions/Current/System.dylib ${RAVYN_SDKROOT}${FPATH}/System.dylib SYMBOLIC)
file(CREATE_LINK ../../../../../../usr/lib/libSystem.B.dylib ${RAVYN_SDKROOT}${FPATH}/Versions/B/System.dylib SYMBOLIC)
file(CREATE_LINK libSystem.B.dylib ${RAVYN_SDKROOT}/usr/lib/libSystem.dylib SYMBOLIC)

file(MAKE_DIRECTORY ${RAVYN_SDKROOT}/System/Library/Frameworks/Kernel.framework/Versions/A)
file(CREATE_LINK Versions/A/Headers ${RAVYN_SDKROOT}/System/Library/Frameworks/Kernel.framework/Headers SYMBOLIC)
file(CREATE_LINK Versions/A/PrivateHeaders ${RAVYN_SDKROOT}/System/Library/Frameworks/Kernel.framework/PrivateHeaders SYMBOLIC)

# Compatibility symlinks
file(CREATE_LINK ravynOS.platform ${ROOT_BINARY_DIR}/Developer/Platforms/MacOSX.platform SYMBOLIC)
file(CREATE_LINK ravynOS.sdk ${ROOT_BINARY_DIR}/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk SYMBOLIC)

file(COPY usr USE_SOURCE_PERMISSIONS DESTINATION ${RAVYN_SDKROOT})
file(COPY System USE_SOURCE_PERMISSIONS DESTINATION ${RAVYN_SDKROOT})
file(COPY ${ROOT_SOURCE_DIR}/Libraries/Libsystem/private/pthread/tsd_private.h
    DESTINATION ${RAVYN_SDKROOT}/usr/local/include/pthread/)

add_library(ravynOS_sdk OBJECT stub.c)
execute_process(COMMAND 
    ${ROOT_SOURCE_DIR}/Kernel/xnu/bsd/sys/make_symbol_aliasing.sh
    ${RAVYN_SDKROOT} ${RAVYN_SDKROOT}/usr/include/sys/_symbol_aliasing.h)

set(libc_path ${ROOT_SOURCE_DIR}/Libraries/Libsystem/libsystem_c)
execute_process(COMMAND env SRCROOT=${libc_path} DSTROOT=${RAVYN_SDKROOT}
    SDK_INSTALL_HEADERS_ROOT=. "PATH=${TOOLS}:$ENV{PATH}" ARCHS="${CpuArch} "
    VARIANT_PLATFORM_NAME=macosx bash ${libc_path}/xcodescripts/headers.sh
)
execute_process(
    COMMAND
        cd ${ROOT_SOURCE_DIR}/Libraries/Libsystem/libsystem_pthread &&
        env DSTROOT=${RAVYN_SDKROOT} SDK_INSTALL_HEADERS_ROOT=.
            sh xcodescripts/install-sys-headers.sh
)

execute_process(
    COMMAND
        cd ${ROOT_SOURCE_DIR}/Libraries/Libsystem/libdispatch &&
        env DSTROOT=${RAVYN_SDKROOT} INSTALL_OWNER=0 INSTALL_GROUP=0 INSTALL_MODE_FLAG=0644
        xcodescripts/install-manpages.sh
)

set(OBJC ${ROOT_SOURCE_DIR}/Libraries/objc4)
file(COPY ${OBJC}/runtime/runtime.h ${OBJC}/runtime/objc.h
    ${OBJC}/runtime/objc-sync.h ${OBJC}/runtime/objc-exception.h
    ${OBJC}/runtime/objc-auto.h ${OBJC}/runtime/objc-api.h
    ${OBJC}/runtime/message.h ${OBJC}/runtime/NSObject.h
    ${OBJC}/runtime/NSObjCRuntime.h ${OBJC}/runtime/Module
    DESTINATION ${RAVYN_SDKROOT}/usr/include/objc/)
file(COPY ${OBJC}/runtime/maptable.h ${OBJC}/runtime/NSObject-internal.h
    ${OBJC}/runtime/objc-abi.h ${OBJC}/runtime/objc-block-trampolines.h
    ${OBJC}/runtime/objc-gdb.h ${OBJC}/runtime/objc-internal.h
    ${OBJC}/runtime/objc-ptrauth.h ${OBJC}/runtime/hashtable.h
    ${OBJC}/runtime/hashtable2.h ${OBJC}/runtime/objc-class.h
    DESTINATION ${RAVYN_SDKROOT}/usr/local/include/objc/)
