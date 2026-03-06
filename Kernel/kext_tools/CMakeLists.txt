add_executable(host_plktool)
target_sources(host_plktool PRIVATE plktool.c ../IOKitUser/IOCFSerialize.c)
target_include_directories(host_plktool PRIVATE
    ./FILES/3rd . ..
    ${ROOT_SOURCE_DIR}/Developer/Default.xctoolchain/include
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libkern
    ../IOKitUser/kext.subproj
    ../IOKitUser
)
target_compile_options(host_plktool PRIVATE -w)

# This uses Open CF-Lite, not our CoreFoundation. Make sure you have
# it installed! (opencflite in AUR)
target_link_options(host_plktool PRIVATE
    -v -L${ROOT_BINARY_DIR}/Developer/Default.xctoolchain/cctools/libmacho
)
target_link_libraries(host_plktool PRIVATE
    host_libmacho -lCoreFoundation -luuid -lssl -lcrypto -lz
)

