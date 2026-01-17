add_library(libsystem_darwin STATIC)
set_property(TARGET libsystem_c PROPERTY OUTPUT_NAME "system_darwin")
#set_property(TARGET libsystem_c PROPERTY SUFFIX ".dylib")
set_property(TARGET libsystem_c PROPERTY PREFIX "lib")

target_sources(libsystem_darwin PRIVATE
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
    ../libsystem_c ../private ../libsystem_c/darwin ../libsystem_c/locale
    ../libsystem_c/locale/FreeBSD ../libsystem_c/stdtime/FreeBSD
    ../../../Kernel/xnu/libsyscall/mach ../../../Kernel/xnu/libkern
    ../../openbsm ../../../Kernel/xnu/libsyscall/wrappers/spawn
    ../../../Kernel/xnu/libsyscall/wrappers/libproc ../liblaunch
#    ${SDK_SOURCE_DIR}/usr/local/include ${SDK_SOURCE_DIR}/usr/include
#    ${RAVYN_SDKROOT}/System/Library/Frameworks/System.framework/Versions/B/PrivateHeaders
#    ${RAVYN_SDKROOT}/System/Library/Frameworks/Kernel.framework/Versions/A/PrivateHeaders
)

target_compile_definitions(libsystem_darwin PRIVATE
    TARGET_OS_BRIDGE=0 __DARWIN_UNIX03
    "au_evname_map_t=struct au_evname_map"
)

target_compile_options(libsystem_darwin PRIVATE
    --sysroot=${RAVYN_SDKROOT} -isysroot${RAVYN_SDKROOT} -w
    #--target=${TARGET_TRIPLE}
    -isystem ${CMAKE_CURRENT_SOURCE_DIR}/h
    -Wno-implicit-function-declaration
    -Wno-int-conversion -Wno-implicit-int
    $<$<COMPILE_LANGUAGE:ASM>:-x assembler-with-cpp>
)

