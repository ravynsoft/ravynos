add_kext_bundle(IOATAFamily
    KERNEL_PRIVATE
    INFO_PLIST Info.plist
    BUNDLE_IDENTIFIER com.apple.iokit.IOATAFamily
    BUNDLE_VERSION 2.5.3)

target_sources(IOATAFamily PRIVATE
    ATADeviceNub.cpp
    ATATimerEventSource.cpp
    IOATABusCommand.cpp
    IOATABusInfo.cpp
    IOATACommand.cpp
    IOATAController.cpp
    IOATADevConfig.cpp
    IOATADevice.cpp
    IOATARegI386.cpp
    IOPCIATA.cpp
)

set_target_properties(IOATAFamily PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED YES
)

file(COPY
    ${CMAKE_CURRENT_SOURCE_DIR}/IOKit/ata/IOATARegI386.h
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

target_include_directories(IOATAFamily PRIVATE
    .
    ${CMAKE_CURRENT_BINARY_DIR}
    ../../xnu/bsd
    ../../xnu/osfmk
    ../../xnu/iokit
    ../../xnu/libkern
    include/IOKit/ata
    private_include/IOKit/ata
    ../IOStorageFamily/include
    ../IOStorageFamily
    ../IOATAFamily
    ../IOGraphics/IONDRVSupport
    ../../../Frameworks
    ${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/osfmk
    ${ROOT_BINARY_DIR}/Kernel/libkmod/include
)

target_compile_options(IOATAFamily PRIVATE
    -mmacos-version-min=${MACOS_VERSION_MIN}
    -Wno-sign-conversion
    -Wno-int-conversion
    -Wno-implicit-function-declaration
    -Wno-incompatible-library-redeclaration
    -Wno-undef-prefix
)

target_link_options(IOATAFamily PRIVATE
    -mmacos-version-min=${MACOS_VERSION_MIN}
)
target_link_libraries(IOATAFamily PRIVATE
    -L${ROOT_BINARY_DIR}/Kernel/libkmod -lkmod)

add_dependencies(IOATAFamily kextBuild)

