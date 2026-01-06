add_library(libsystem_darwin STATIC)
set_property(TARGET libsystem_c PROPERTY OUTPUT_NAME "system_darwin")
set_property(TARGET libsystem_c PROPERTY SUFFIX ".dylib")
set_property(TARGET libsystem_c PROPERTY PREFIX "lib")

add_library(libsystem_darwin
    bsd.c
    ctl.c
    dirstat.c
    dirstat_collection.c
    err.c
    exception.c
    init.c
    mach.c
    stdio.c
    stdlib.c
    string.c
    variant.c
)

target_include_directories(libsystem_darwin PRIVATE
    # ${ROOT_SOURCE_DIR}/Developer/Platforms/ravynOS.platform/Developer/SDKs/ravynOS.sdk
    # ${RAVYN_SDKROOT}/System/Library/Frameworks/Kernel.framework/Versions/A/Headers
    # ${RAVYN_SDKROOT}/System/Library/Frameworks/Kernel.framework/Versions/A/PrivateHeaders
    h ../private ${SDK_SOURCE_DIR}/usr/include
)

target_compile_definitions(libsystem_darwin PRIVATE
    TARGET_OS_BRIDGE=0 __DARWIN_UNIX03 
)

target_compile_options(libsystem_darwin PRIVATE
    --sysroot=${RAVYN_SDKROOT} -isysroot=${RAVYN_SDKROOT} -w
    --target=${TARGET_TRIPLE}
    -Wno-implicit-function-declaration
    -Wno-int-conversion -Wno-implicit-int
)
