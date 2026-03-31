add_kext_bundle(AppleAPIC INFO_PLIST Info.plist BUNDLE_IDENTIFIER com.apple.driver.AppleAPIC BUNDLE_VERSION 1.4)
target_sources(AppleAPIC PRIVATE
    Apple8259PIC.cpp
    AppleAPIC.cpp
)
target_compile_options(AppleAPIC PRIVATE -Wno-implicit-int-conversion -Wno-sign-conversion)
target_include_directories(AppleAPIC PRIVATE
    ${RAVYN_SDKROOT}/System/Library/Frameworks/Kernel.framework/Versions/A/PrivateHeaders)
install(TARGETS AppleAPIC DESTINATION ${SYSROOT_DIR}/System/Library/Extensions COMPONENT BaseSystem)

add_dependencies(AppleAPIC kextBuild)
