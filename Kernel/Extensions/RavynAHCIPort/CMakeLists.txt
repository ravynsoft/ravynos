add_kext_bundle(RavynAHCIPort
    KERNEL_PRIVATE
    INFO_PLIST Info.plist
    BUNDLE_IDENTIFIER com.ravynos.driver.RavynAHCIPort
    BUNDLE_VERSION 1.0.0)

target_sources(RavynAHCIPort PRIVATE
    RavynAHCIPort.cpp
    RavynAHCIDisk.cpp)

target_include_directories(RavynAHCIPort PRIVATE
    .
    ../../xnu/bsd
    ../../xnu/iokit
    ../../xnu/osfmk
    ../../xnu/libkern
    ../IOPCIFamily
    ../IOStorageFamily
    ${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/osfmk)

target_compile_options(RavynAHCIPort PRIVATE
    -Wno-implicit-int-conversion
    -Wno-sign-conversion)

target_link_options(RavynAHCIPort PRIVATE
    -mmacos-version-min=${MACOS_VERSION_MIN})

target_link_libraries(RavynAHCIPort PRIVATE
    -L${ROOT_BINARY_DIR}/Kernel/libkmod -lkmod)

add_dependencies(RavynAHCIPort kextBuild IOPCIFamily)

