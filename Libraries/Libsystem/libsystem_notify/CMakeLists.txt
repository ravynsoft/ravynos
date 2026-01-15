add_library(libsystem_notify OBJECT)
set_property(TARGET libsystem_notify PROPERTY OUTPUT_NAME "system_notify")
set_property(TARGET libsystem_notify PROPERTY SUFFIX ".dylib")
set_property(TARGET libsystem_notify PROPERTY PREFIX "lib")

target_sources(libsystem_notify PRIVATE
    libnotify.c notify_client.c table.c
)

target_include_directories(libsystem_notify PRIVATE
    ${SDK_SOURCE_DIR}/usr/include ../../../Kernel/xnu/EXTERNAL_HEADERS
    ${RAVYN_SDKROOT}/System/Library/Frameworks/System.framework/Versions/B/PrivateHeaders
    . ../libsystem_c/darwin ../libsystem_c ../../../Kernel/xnu/libsyscall/mach
    ../private ../../../Kernel/xnu/libkern
    ${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/osfmk
    ../liblaunch ${SDK_SOURCE_DIR}/usr/include/pthread
    ../libdispatch/private ../libxpc ${RAVYN_SDKROOT}/usr/include
)

target_compile_definitions(libsystem_notify PRIVATE
    DISPATCH_USE_DTRACE=0 "atomic_uint_fast32_t=_Atomic(uint_fast32_t)"
)

target_compile_options(libsystem_notify PRIVATE
    --sysroot=${RAVYN_SDKROOT} -isysroot=${RAVYN_SDKROOT} -w
    --target=${TARGET_TRIPLE}
    -iframework ${RAVYN_SDKROOT}/System/Library/Frameworks
    -isystem ${RAVYN_SDKROOT}/usr/include
    -fno-exceptions -Wno-implicit-function-declaration
    -Wno-int-conversion -Wno-implicit-int
    $<$<COMPILE_LANGUAGE:ASM>:-x assembler-with-cpp>
)

