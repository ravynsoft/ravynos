add_executable(host_plktool)

target_sources(host_plktool PRIVATE
    plktool.c
    macho.c
    kext.c
    ${ROOT_SOURCE_DIR}/Kernel/IOKitUser/IOCFSerialize.c)

target_include_directories(host_plktool PRIVATE
    ${ROOT_SOURCE_DIR}/Kernel/kext_tools/FILES/3rd
    ${ROOT_SOURCE_DIR}/Developer/Default.xctoolchain/include
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libkern
    ${ROOT_SOURCE_DIR}/Kernel/IOKitUser/kext.subproj
    ${ROOT_SOURCE_DIR}/Kernel/IOKitUser
)

target_compile_options(host_plktool PRIVATE -w -g -O0)

target_link_options(host_plktool PRIVATE
    -L${ROOT_BINARY_DIR}/Developer/Default.xctoolchain/cctools/libmacho
    -L${ROOT_BINARY_DIR}/libkxld
)

# This uses Open CF-Lite, not our CoreFoundation. Make sure you have
# it installed! (opencflite in AUR)

target_link_libraries(host_plktool PRIVATE
    -lkxld host_libmacho host_libstuff
    -lCoreFoundation -luuid -lssl -lcrypto -lz -lstdc++
)

# you need to manually build libkxld first
#add_dependencies(host_plktool libkxld)
