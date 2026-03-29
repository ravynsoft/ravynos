add_kext_bundle(IOACPIFamily
    KERNEL_PRIVATE
    INFO_PLIST Info.plist
    BUNDLE_IDENTIFIER com.apple.iokit.IOACPIFamily
    BUNDLE_VERSION 1.0.0d1)

target_sources(IOACPIFamily PRIVATE
    IOACPIPlatformDevice.cpp
    IOACPIPlatformExpert.cpp
    hack.s
    AppleI386CPU.cpp
    AppleI386PlatformExpert.cpp
)

target_include_directories(IOACPIFamily PRIVATE
    include
    ../../xnu/iokit
    ../../xnu/osfmk
    ../../xnu/bsd
    ../../xnu/libkern
    ${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/osfmk
    ${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/)    
    
target_compile_options(IOACPIFamily PRIVATE
    -Wno-implicit-int-conversion -Wno-sign-conversion
    $<$<COMPILE_LANGUAGE:ASM>:-x assembler-with-cpp>)

target_link_options(IOACPIFamily PRIVATE
    -mmacos-version-min=${MACOS_VERSION_MIN})

add_dependencies(IOACPIFamily kextBuild)
