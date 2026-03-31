add_kext_bundle(IOStorageFamily INFO_PLIST Info.plist KERNEL_PRIVATE
    BUNDLE_IDENTIFIER com.apple.iokit.IOStorageFamily BUNDLE_VERSION 2.1)

target_sources(IOStorageFamily PRIVATE
    IOAppleLabelScheme.cpp
    IOApplePartitionScheme.cpp
    IOBlockStorageDevice.cpp
    IOBlockStorageDriver.cpp
    IOFDiskPartitionScheme.cpp
    IOFilterScheme.cpp
    IOGUIDPartitionScheme.cpp
    IOMedia.cpp
    IOMediaBSDClient.cpp
    IOPartitionScheme.cpp
    IOStorage.cpp
)
target_include_directories(IOStorageFamily PRIVATE include)
target_compile_options(IOStorageFamily PRIVATE
    -Wno-implicit-int-conversion -Wno-sign-conversion
    -Wno-deprecated-declarations
    -Wno-incompatible-library-redeclaration -Wno-undef-prefix
    -I${CMAKE_CURRENT_SOURCE_DIR}
    -I${CMAKE_BINARY_DIR}/Kernel/libkmod/include -I${CMAKE_BINARY_DIR}/Kernel/Extensions/include
    -I${CMAKE_BINARY_DIR}/release/System/Library/Frameworks/Kernel.framework/Versions/A/PrivateHeaders
    -I${CMAKE_BINARY_DIR}/release/System/Library/Frameworks/Kernel.framework/Versions/B/Headers
    -I${CMAKE_SOURCE_DIR}/Kernel/xnu/bsd
    -I${CMAKE_BINARY_DIR}/sysroot/usr/include
)
set_property(TARGET IOStorageFamily PROPERTY CXX_STANDARD 11)
install(TARGETS IOStorageFamily DESTINATION System/Library/Extensions COMPONENT BaseSystem)

# FIXME: Re-enable these when we have our own tools for handling image resource files.
# add_bundle_iconset(IOStorageFamily IconResources/External.iconset)
# add_bundle_iconset(IOStorageFamily IconResources/Internal.iconset)
# add_bundle_iconset(IOStorageFamily IconResources/Removable.iconset)
# add_bundle_resources(IOStorageFamily en.lproj/InfoPlist.strings)

add_dependencies(IOStorageFamily kextBuild)

add_library(IOStorageFamilyHeaders INTERFACE)
target_include_directories(IOStorageFamilyHeaders INTERFACE .)
target_sources(IOStorageFamilyHeaders INTERFACE
    IOAppleLabelScheme.h
    IOApplePartitionScheme.h
    IOBlockStorageDevice.h
    IOBlockStorageDriver.h
    IOFDiskPartitionScheme.h
    IOFilterScheme.h
    IOGUIDPartitionScheme.h
    IOMedia.h
    IOMediaBSDClient.h
    IOPartitionScheme.h
    IOStorage.h
    IOStorageCardCharacteristics.h
    IOStorageControllerCharacteristics.h
    IOStorageDeviceCharacteristics.h
    IOStorageProtocolCharacteristics.h
)
add_dependencies(IOStorageFamily IOStorageFamilyUserHeaders)

add_darwin_object_library(IOStorageFamilyUserHeaders)
target_sources(IOStorageFamilyUserHeaders PRIVATE user_stub.c)
target_include_directories(IOStorageFamilyUserHeaders INTERFACE ${CMAKE_CURRENT_BINARY_DIR}/user_include)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/user_include/IOKit/storage)

foreach(file IN LISTS HEADERS)
    add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/user_include/IOKit/storage/${file}
        COMMAND ${CMAKE_BINARY_DIR}/tmp/obj-tools/usr/bin/unifdef -UKERNEL -o ${CMAKE_CURRENT_BINARY_DIR}/user_include/IOKit/storage/${file} ${CMAKE_CURRENT_SOURCE_DIR}/${file}
        DEPENDS ${CMAKE_BINARY_DIR}/tmp/obj-tools/usr/bin/unifdef ${CMAKE_CURRENT_SOURCE_DIR}/${file}
        COMMENT "unifdef ${file}" VERBATIM
    )
    target_sources(IOStorageFamilyUserHeaders PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/user_include/IOKit/storage/${file})
endforeach()

install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/user_include/IOKit/storage DESTINATION System/Library/Frameworks/IOKit.framework/Versions/A/Headers/IOKit COMPONENT DeveloperTools)
install(TARGETS IOStorageFamilyHeaders DESTINATION System/Library/Frameworks/Kernel.framework/Versions/A/Headers/IOKit/storage/ COMPONENT DeveloperTools)
