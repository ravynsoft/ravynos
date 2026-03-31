add_kext_bundle(IOPCIFamily KERNEL_PRIVATE INFO_PLIST Info.plist BUNDLE_IDENTIFIER com.apple.iokit.IOPCIFamily BUNDLE_VERSION 2.4.1)
target_sources(IOPCIFamily PRIVATE
    IOPCIBridge.cpp
    IOPCIConfigurator.cpp
    IOPCIDevice.cpp
    IOPCIDeviceI386.cpp
    IOPCIDeviceMappedIO.cpp
    IOPCIMessagedInterruptController.cpp
    IOPCIRange.cpp
)

set_property(TARGET IOPCIFamily PROPERTY CXX_STANDARD 11)
target_compile_options(IOPCIFamily PRIVATE -Wno-implicit-int-conversion -Wno-inconsistent-missing-override -Wno-sign-conversion -Wno-shadow)
target_link_libraries(IOPCIFamily PRIVATE IOPCIFamilyHeaders)
install(TARGETS IOPCIFamily DESTINATION System/Library/Extensions COMPONENT BaseSystem)

add_library(IOPCIFamilyHeaders INTERFACE)
target_include_directories(IOPCIFamilyHeaders INTERFACE include)

add_subdirectory(ApplePIODMA)
add_dependencies(ApplePIODMA kextBuild)

