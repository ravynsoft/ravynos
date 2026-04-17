add_kext_bundle(AppleAPIC
    INFO_PLIST Info.plist
    BUNDLE_IDENTIFIER com.apple.driver.AppleAPIC
    BUNDLE_VERSION 1.4)

target_sources(AppleAPIC PRIVATE
    Apple8259PIC.cpp
    AppleAPIC.cpp)

target_include_directories(AppleAPIC PRIVATE
    ../../xnu/iokit
    ../../xnu/osfmk
    ../../xnu/bsd
    ../../xnu/libkern
    ${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/osfmk)

target_compile_options(AppleAPIC PRIVATE
    -mmacos-version-min=${MACOS_VERSION_MIN}
    -Wno-implicit-int-conversion -Wno-sign-conversion)

target_link_options(AppleAPIC PRIVATE
    -mmacos-version-min=${MACOS_VERSION_MIN})

target_link_libraries(AppleAPIC PRIVATE
    -L${ROOT_BINARY_DIR}/Kernel/libkmod -lkmod)

add_dependencies(AppleAPIC kextBuild)
