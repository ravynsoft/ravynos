#add_subdirectory(IOGraphicsFamily)
#add_subdirectory(IONDRVSupport)

add_kext_bundle(IOGOPFramebuffer
    KERNEL_PRIVATE
    INFO_PLIST Info-IOGOPFramebuffer.plist
    BUNDLE_IDENTIFIER com.ravynos.iokit.IOGOPFramebuffer
    BUNDLE_VERSION 1.0.0)

target_sources(IOGOPFramebuffer PRIVATE
    IOGOPFramebuffer.cpp
)

target_include_directories(IOGOPFramebuffer PRIVATE
    ../../xnu/iokit
    ../../xnu/osfmk
    ../../xnu/bsd
    ../../xnu/libkern
    ../IOPCIFamily
    ${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/osfmk
    ${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/libkern
    ${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/san
    ../../xnu/EXTERNAL_HEADERS)

target_compile_options(IOGOPFramebuffer PRIVATE
    -w -Wno-error -Wno-implicit-int-conversion -Wno-sign-conversion
    -Wno-c++11-narrowing -DKERNEL=1
    $<$<COMPILE_LANGUAGE:ASM>:-x assembler-with-cpp>)

target_link_libraries(IOGOPFramebuffer PRIVATE)

target_link_options(IOGOPFramebuffer PRIVATE
    -mmacos-version-min=${MACOS_VERSION_MIN})

set_property(TARGET IOGOPFramebuffer PROPERTY CXX_STANDARD 11)

add_dependencies(IOGOPFramebuffer kextBuild)
