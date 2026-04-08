add_kext_bundle(AppleIntelPIIXATA
    KERNEL_PRIVATE
    INFO_PLIST Info.plist
    BUNDLE_IDENTIFIER com.apple.driver.AppleIntelPIIXATA
    BUNDLE_VERSION 2.5.1)

target_sources(AppleIntelPIIXATA PRIVATE
    AppleIntelICHxSATA.cpp
    AppleIntelPIIXATAChannel.cpp
    AppleIntelPIIXATARoot.cpp
    AppleIntelPIIXPATA.cpp
)

target_include_directories(AppleIntelPIIXATA PRIVATE
    .
    ../../xnu/bsd
    ../../xnu/iokit
    ../../xnu/osfmk
    ../../xnu/libkern
    ../IOATAFamily
    ../IOStorageFamily
    ../IOPCIFamily
    ../IOACPIFamily/include
    ../IOGraphics/IONDRVSupport
    ${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/osfmk
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_compile_options(AppleIntelPIIXATA PRIVATE
    -Wno-implicit-int-conversion
    -Wno-shorten-64-to-32
    -Wno-sign-conversion
)

add_dependencies(AppleIntelPIIXATA kextBuild IOATAFamily)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/../IOATAFamily/IOATATypes.h
	${CMAKE_CURRENT_SOURCE_DIR}/../IOATAFamily/IOATAController.h
	${CMAKE_CURRENT_SOURCE_DIR}/../IOATAFamily/ATADeviceNub.h
	${CMAKE_CURRENT_SOURCE_DIR}/../IOATAFamily/IOPCIATA.h
	DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/IOKit/ata/)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/../IOATAFamily/IOATAFamilyPriv.h
        DESTINATION IOKit/storage/ata/)
