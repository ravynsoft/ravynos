add_kext_bundle(IOGraphicsFamily
    KERNEL_PRIVATE
    INFO_PLIST Info.plist
    BUNDLE_IDENTIFIER com.apple.iokit.IOGraphicsFamily
    BUNDLE_VERSION 576.1.0)

target_sources(IOGraphicsFamily PRIVATE
	IOBacklightDisplay.cpp
	IOBootFramebuffer.cpp
	IODisplay.cpp
	IODisplayWrangler.cpp
	IOFramebuffer.cpp
	IOFramebufferUserClient.cpp
	IOGraphicsDevice.cpp
	IOKit/graphics/tl/osmemory.cpp
	IODisplayWranglerUserClients.cpp
	IOI2CInterface.cpp
	../GTrace/Kernel/GTrace.cpp
	../GMetric/GMetric.cpp)

target_include_directories(IOGraphicsFamily PRIVATE
    .
    ../../../xnu/iokit
    ../../../xnu/osfmk
    ../../../xnu/bsd
    ../../../xnu/libkern
    ${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/osfmk
    ../../IOPCIFamily
    ../../../xnu/EXTERNAL_HEADERS)

target_compile_options(IOGraphicsFamily PRIVATE
    -Wno-implicit-int-conversion -Wno-sign-conversion
    $<$<COMPILE_LANGUAGE:ASM>:-x assembler-with-cpp>)

target_link_options(IOGraphicsFamily PRIVATE
    -mmacos-version-min=${MACOS_VERSION_MIN})

add_dependencies(IOGraphicsFamily kextBuild)
