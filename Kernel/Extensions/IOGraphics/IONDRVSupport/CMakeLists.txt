add_kext_bundle(IONDRVSupport
    KERNEL_PRIVATE
    INFO_PLIST Info.plist
    BUNDLE_IDENTIFIER com.apple.iokit.IONDRVSupport
    BUNDLE_VERSION 576.1.0)

target_sources(IONDRVSupport PRIVATE
	IONDRV.cpp
	IONDRVFramebuffer.cpp
	IONDRVLibraries.cpp
)

target_include_directories(IONDRVSupport PRIVATE
    .
    ../../../xnu/iokit
    ../../../xnu/osfmk
    ../../../xnu/bsd
    ../../../xnu/libkern
    ${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/osfmk
    ../IOGraphicsFamily
    ../../IOPCIFamily)

target_compile_options(IOGraphicsFamily PRIVATE
    -Wno-implicit-int-conversion -Wno-sign-conversion
    $<$<COMPILE_LANGUAGE:ASM>:-x assembler-with-cpp>)

target_link_options(IOGraphicsFamily PRIVATE
    -mmacos-version-min=${MACOS_VERSION_MIN})

add_dependencies(IOGraphicsFamily kextBuild)
