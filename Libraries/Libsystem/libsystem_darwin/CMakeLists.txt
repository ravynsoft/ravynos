add_library(libsystem_darwin SHARED)
set_target_properties(libsystem_darwin PROPERTIES 
    OUTPUT_NAME "system_darwin" SUFFIX ".dylib" PREFIX "lib" NO_SONAME true)

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
    ${ROOT_SOURCE_DIR}/Kernel/xnu/bsd
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libkern
    ${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/osfmk
    ${CMAKE_CURRENT_BINARY_DIR}/../libsystem_kernel/mig_hdr/include
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/spawn
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/libproc
    ${ROOT_SOURCE_DIR}/Kernel/xnu/osfmk
    ${RAVYN_SDKROOT}/usr/include ${RAVYN_SDKROOT}/usr/local/include
    
    ../libsystem_c/include ../libsystem_c ../libsystem_c/locale
    ../libsystem_c/locale/FreeBSD ../libsystem_c/stdtime/FreeBSD
    ../liblaunch ../libdispatch/private ../private h ../../openbsm
)

target_compile_definitions(libsystem_darwin PRIVATE
    TARGET_OS_BRIDGE=0 __DARWIN_UNIX03 guardid_t=uint64_t
    "au_evname_map_t=struct au_evname_map"
    _COMM_PAGE_ATM_DIAGNOSTIC_CONFIG=0x00007fffffe00048ULL
    _LIBC_NO_FEATURE_VERIFICATION DARWIN_BUILDING_LIBSYSTEM_DARWIN
	OS_CRASH_ENABLE_EXPERIMENTAL_LIBTRACE
)

target_compile_options(libsystem_darwin PRIVATE
    --sysroot=${RAVYN_SDKROOT} -isysroot${RAVYN_SDKROOT} -w
    -include${RAVYN_SDKROOT}/usr/include/os/availability.h
    -Wno-implicit-function-declaration
    -Wno-int-conversion -Wno-implicit-int
    $<$<COMPILE_LANGUAGE:ASM>:-x assembler-with-cpp>
)

target_link_options(libsystem_darwin PRIVATE
    -mmacos-version-min=10.15
    -rdynamic -nodefaultlibs -Wl,-search_paths_first
    -Wl,-install_name,/usr/lib/system/libsystem_darwin.dylib
    -Wl,-all_load -nostdlib -L${RAVYN_SDKROOT}/usr/lib/system -umbrella System
    -lsystem_kernel
    #-lcompiler-rt -ldyld  -lm -lsystem_malloc -lsystem_platform
    #-lsystem_pthread -lsystem_c -ldispatch -lxpc
)

file(GLOB darwinpvthdrs h/*.h)
file(COPY ${darwinpvthdrs} DESTINATION ${RAVYN_SDKROOT}/usr/local/include/os/)
file(COPY AppleFactoryVariant.plist AppleInternalVariant.plist
    DESTINATION ${RAVYN_SDKROOT}/System/Library/CoreServices/)

install(FILES AppleFactoryVariant.plist AppleInternalVariant.plist
    DESTINATION ${SYSROOT_DIR}/System/Library/CoreServices/)
