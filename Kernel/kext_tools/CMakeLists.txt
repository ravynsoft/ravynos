add_executable(host_kextcache)
target_sources(host_kextcache PRIVATE
    compression.c
    kextcache_main.c
    kext_tools_util.c
    kernelcache.c
    security.c
    linuxshims.c
    ../IOKitUser/kext.subproj/macho_util.c
    FILES/OSKext.c
)
target_include_directories(host_kextcache PRIVATE
    ./FILES/3rd . ..
    ${ROOT_SOURCE_DIR}/Developer/Default.xctoolchain/include
    ${ROOT_SOURCE_DIR}/Frameworks/Foundation/Headers
    ${ROOT_SOURCE_DIR}/Frameworks
    ${ROOT_SOURCE_DIR}/Libraries/Libsystem/libxpc
    ${ROOT_SOURCE_DIR}/Libraries/Libsystem/libsystem_notify
    ${ROOT_SOURCE_DIR}/Libraries/Libsystem/libsystem_asl
    ${ROOT_SOURCE_DIR}/Libraries/openbsm/sys
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libkern
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libkern/libkern
    ../IOKitUser/kext.subproj
)
target_compile_options(host_kextcache PRIVATE -w
    -include${CMAKE_CURRENT_SOURCE_DIR}/FILES/linuxshims.h
)

# This uses Open CF-Lite, not our CoreFoundation. Make sure you have
# it installed! (opencflite in AUR)
target_link_options(host_kextcache PRIVATE
    -v -L${ROOT_BINARY_DIR}/Developer/Default.xctoolchain/cctools/libmacho
)
target_link_libraries(host_kextcache PRIVATE
    host_libmacho -lCoreFoundation -luuid -lz
)

