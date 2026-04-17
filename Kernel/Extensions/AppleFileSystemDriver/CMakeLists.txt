add_kext_bundle(AppleFileSystemDriver
    KERNEL_PRIVATE
    INFO_PLIST Info.plist
    BUNDLE_IDENTIFIER com.apple.driver.AppleFileSystemDriver
    BUNDLE_VERSION 1.0.0)

target_sources(AppleFileSystemDriver PRIVATE
	./AppleFileSystemDriver.cpp)

target_include_directories(AppleFileSystemDriver PRIVATE
    ../../xnu/iokit
    ../../xnu/osfmk
    ../../xnu/bsd
    ../../xnu/libkern
    ${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/osfmk
    ${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/
    ../../xnu/EXTERNAL_HEADERS
    ../IOStorageFamily
    ${CMAKE_CURRENT_BINARY_DIR}
    ${RAVYN_SDKROOT}/System/Library/Frameworks/System.framework/PrivateHeaders)

target_compile_options(AppleFileSystemDriver PRIVATE
    -mmacos-version-min=${MACOS_VERSION_MIN}
    -Wno-implicit-int-conversion -Wno-sign-conversion
    $<$<COMPILE_LANGUAGE:ASM>:-x assembler-with-cpp>)

target_link_options(AppleFileSystemDriver PRIVATE
    -mmacos-version-min=${MACOS_VERSION_MIN})

add_dependencies(AppleFileSystemDriver kextBuild)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/../hfs/core/hfs_format.h
          ${CMAKE_CURRENT_SOURCE_DIR}/../hfs/core/hfs_unistr.h
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/hfs/)