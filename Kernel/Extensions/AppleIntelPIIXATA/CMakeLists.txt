add_kext_bundle(AppleIntelPIIXATA KERNEL_PRIVATE INFO_PLIST Info.plist BUNDLE_IDENTIFIER com.apple.driver.AppleIntelPIIXATA BUNDLE_VERSION 2.5.1)
target_sources(AppleIntelPIIXATA PRIVATE
    AppleIntelICHxSATA.cpp
    AppleIntelPIIXATAChannel.cpp
    AppleIntelPIIXATARoot.cpp
    AppleIntelPIIXPATA.cpp
)

target_compile_options(AppleIntelPIIXATA PRIVATE -Wno-implicit-int-conversion -Wno-shorten-64-to-32 -Wno-sign-conversion)
target_link_libraries(AppleIntelPIIXATA PRIVATE IOACPIFamilyHeaders IOATAFamilyHeaders IOATAFamilyPrivateHeaders IOPCIFamilyHeaders IOStorageFamilyHeaders)
install(TARGETS AppleIntelPIIXATA DESTINATION System/Library/Extensions/IOATAFamily.kext/Contents/PlugIns COMPONENT BaseSystem)
add_dependencies(AppleIntelPIIXATA kextBuild IOATAFamily)
