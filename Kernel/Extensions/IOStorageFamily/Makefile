MK_WERROR = no
KEXT = IOStorageFamily
KERNEL_PRIVATE = yes
INFO_PLIST = Info.plist
BUNDLE_IDENTIFIER = com.apple.iokit.IOStorageFamily
BUNDLE_VERSION = 2.1

CXXSTD = gnu++11

SRCS = IOAppleLabelScheme.cpp \
	IOApplePartitionScheme.cpp \
	IOBlockStorageDevice.cpp \
	IOBlockStorageDriver.cpp \
	IOFDiskPartitionScheme.cpp \
	IOFilterScheme.cpp \
	IOGUIDPartitionScheme.cpp \
	IOMedia.cpp \
	IOMediaBSDClient.cpp \
	IOPartitionScheme.cpp \
	IOStorage.cpp

CFLAGS = -Wno-implicit-int-conversion \
	-Wno-sign-conversion \
	-Wno-deprecated-declarations \
	-Wno-incompatible-library-redeclaration \
	-Wno-undef-prefix \
	-I${.CURDIR} \
	-I${.CURDIR}/../../xnu/iokit \
	-I${.CURDIR}/../../xnu/osfmk \
	-I${.CURDIR}/../../xnu/bsd \
	-I${.CURDIR}/../../xnu/libkern \
	-I${ROOT_BINARY_DIR}${ROOT_SOURCE_DIR}/Kernel/xnu/EXPORT_HDRS/osfmk

# FIXME: Re-enable these when we have our own tools for handling image resource files.
# add_bundle_iconset(IOStorageFamily IconResources/External.iconset)
# add_bundle_iconset(IOStorageFamily IconResources/Internal.iconset)
# add_bundle_iconset(IOStorageFamily IconResources/Removable.iconset)
# add_bundle_resources(IOStorageFamily en.lproj/InfoPlist.strings)

IOSTORAGE_HEADERS = IOAppleLabelScheme.h \
	IOApplePartitionScheme.h \
	IOBlockStorageDevice.h \
	IOBlockStorageDriver.h \
	IOFDiskPartitionScheme.h \
	IOFilterScheme.h \
	IOGUIDPartitionScheme.h \
	IOMedia.h \
	IOMediaBSDClient.h \
	IOPartitionScheme.h \
	IOStorage.h \
	IOStorageCardCharacteristics.h \
	IOStorageControllerCharacteristics.h \
	IOStorageDeviceCharacteristics.h \
	IOStorageProtocolCharacteristics.h

#add_dependencies(IOStorageFamily IOStorageFamilyUserHeaders)

#add_library(IOStorageFamilyUserHeaders OBJECT)
#target_sources(IOStorageFamilyUserHeaders PRIVATE user_stub.c)
#target_include_directories(IOStorageFamilyUserHeaders INTERFACE ${CMAKE_CURRENT_BINARY_DIR}/user_include)
#file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/user_include/IOKit/storage)

#foreach(file IN LISTS IOSTORAGE_HEADERS)
#    add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/user_include/IOKit/storage/${file}
#        COMMAND ${TOOLS}/unifdef -UKERNEL -o ${CMAKE_CURRENT_BINARY_DIR}/user_include/IOKit/storage/${file} ${CMAKE_CURRENT_SOURCE_DIR}/${file}
#        DEPENDS ${TOOLS}/unifdef ${CMAKE_CURRENT_SOURCE_DIR}/${file}
#        COMMENT "unifdef ${file}" VERBATIM
#    )
#    target_sources(IOStorageFamilyUserHeaders PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/user_include/IOKit/storage/${file})
#endforeach()

.include <rvn.kext.mk>
