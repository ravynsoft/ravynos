MK_WERROR = no
KEXT = IOHIDSystem
INFO_PLIST = Info-IOHIDSystem.plist
BUNDLE_IDENTIFIER = com.apple.iokit.IOHIDSystem
BUNDLE_VERSION = 1.5.2
KERNEL_PRIVATE = yes

.PATH: ${.CURDIR:H}
IIGS= \
	HIDDriverKit/IOHIDDevice.iig \
	HIDDriverKit/IOHIDEventService.iig \
	HIDDriverKit/IOHIDInterface.iig

SRCS = \
	IOHIDSystem/IOHIDevice.cpp \
	IOHIDSystem/IOHIKeyboard.cpp \
	IOHIDSystem/IOHIKeyboardMapper.cpp \
	IOHIDSystem/IOBSDConsole.cpp

REAL_SRCS = \
	IOHIDFamily/IOHIDUserDevice.cpp \
	IOHIDFamily/AppleUserHIDDevice.cpp \
	IOHIDFamily/IOHIDEventServiceFastPathUserClient.cpp \
	IOHIDSystem/IOHIDKeyboardEventDevice.cpp \
	IOHIDFamily/IOHIDEventServiceUserClient.cpp \
	IOHIDFamily/IOHIDLibUserClient.cpp \
	HIDDriverKit/IOHIDDevice.cpp \
	IOHIDFamily/IOHIDEventQueue.cpp \
	IOHIDSystem/IOHIDDescriptorParser/HIDGetButtonCaps.c \
	IOHIDSystem/IOHIDDescriptorParser/HIDGetCaps.c \
	IOHIDSystem/IOHIDDescriptorParser/HIDGetCollectionNodes.c \
	IOHIDSystem/IOHIDDescriptorParser/HIDGetValueCaps.c \
	IOHIDSystem/IOHIDDescriptorParser/HIDHasUsage.c \
	IOHIDSystem/IOHIDDescriptorParser/HIDMaxUsageListLength.c \
	IOHIDSystem/IOHIDDescriptorParser/HIDNextItem.c \
	IOHIDSystem/IOHIDDescriptorParser/HIDOpenCloseDescriptor.c \
	IOHIDSystem/IOHIDDescriptorParser/HIDParseDescriptor.c \
	HIDDriverKit/IOHIDElementContainer.cpp \
	IOHIDSystem/IOHIDDescriptorParser/HIDProcessCollection.c \
	IOHIDSystem/IOHIDDescriptorParser/HIDProcessGlobalItem.c \
	IOHIDSystem/IOHIDDescriptorParser/HIDProcessLocalItem.c \
	IOHIDSystem/IOHIDDescriptorParser/HIDProcessMainItem.c \
	IOHIDSystem/IOHIDDescriptorParser/HIDProcessReportItem.c \
	IOHIDFamily/AppleUserHIDEventService.cpp \
	IOHIDSystem/IOHIDDescriptorParser/HIDUsageInRange.c \
	IOHIDFamily/IOHIDDeviceElementContainer.cpp \
	IOHIDSystem/IOHIDDescriptorParser/PoolAlloc.c \
	IOHIDSystem/IOHIDevice.cpp \
	IOHIDSystem/IOHIDIO.cpp \
	HIDDriverKit/IOHIDElementPrivate.cpp \
	HIDDriverKit/IOHIDInterface.cpp \
	HIDDriverKit/IOHIDEventService.cpp \
	IOHIDFamily/IOHIDActionQueue.cpp \
	IOHIDFamily/IOHIDProviderPropertyMerger.cpp \
	IOHIDFamily/IOHIDEventDriver.cpp \
	IOHIDFamily/AppleEmbeddedKeyboard.cpp \
	IOHIDFamily/IOHIDEventRepairDriver.cpp \
	IOHIDFamily/IOHIDFamilyPrivate.cpp \
	HIDDriverKit/IOHIDEvent.cpp \
	IOHIDSystem/IOHIDDescriptorParser/HIDCountDescriptorItems.c \
	IOHIDSystem/IOHIDDescriptorParser/HIDGetReportLength.c \
	IOHIDSystem/IOHIDDescriptorParser/HIDIsButtonOrValue.c \
	IOHIDSystem/IOHIDDescriptorParser/HIDCheckReport.c \
	IOHIDFamily/IOHIDEventSource.cpp \
	IOHIDSystem/IOHIDDescriptorParser/HIDGetButtonsOnPage.c \
	IOHIDSystem/IOHIDDescriptorParser/HIDGetButtons.c \
	IOHIDSystem/IOHIDDescriptorParser/HIDGetUsageValue.c \
	IOHIDSystem/IOHIDDescriptorParser/HIDGetUsageValueArray.c \
	IOHIDSystem/IOHIDDescriptorParser/HIDGetNextUsageValueInfo.c \
	IOHIDSystem/IOHIDDescriptorParser/HIDGetNextButtonInfo.c \
	IOHIDSystem/IOHIDDescriptorParser/HIDGetData.c \
	IOHIDSystem/IOHIDDescriptorParser/HIDInitReport.c \
	IOHIDSystem/IOHIDDescriptorParser/HIDPostProcessRIValue.c \
	IOHIDSystem/IOHIDDescriptorParser/HIDPutData.c \
	IOHIDSystem/IOHIDDescriptorParser/HIDScaleUsageValue.c \
	IOHIDSystem/IOHIDDescriptorParser/HIDUsageListDifference.c \
	IOHIDSystem/IOHIDPointingEventDevice.cpp \
	IOHIDSystem/IOHIDDescriptorParser/HIDUsageAndPageFromIndex.c \
	IOHIDSystem/IOHIDDescriptorParser/HIDSetUsageValueArray.c \
	IOHIDSystem/IOHIDDescriptorParser/HIDSetUsageValue.c \
	IOHIDSystem/IOHIDDescriptorParser/HIDSetScaledUsageValue.c \
	IOHIDSystem/IOHIDDescriptorParser/HIDSetButtons.c \
	IOHIDFamily/Cosmo_USB2ADB.c \
	IOHIDFamily/IOHIDConsumer.cpp \
	IOHIDFamily/IOHIDKeyboard.cpp \
	IOHIDFamily/IOHIDPointing.cpp \
	IOHIDSystem/IOHIDPrivate.cpp \
	IOHIDSystem/IOHIDPointingDevice.cpp \
	IOHIDSystem/IOHIDKeyboardDevice.cpp \
	IOHIDSystem/IOHIDSystem.cpp \
	IOHIDSystem/IOHIKeyboard.cpp \
	IOHIDSystem/IOHIKeyboardMapper.cpp \
	IOHIDSystem/IOHIPointing.cpp \
	IOHIDSystem/IOHITablet.cpp \
	IOHIDSystem/IOHITabletPointer.cpp \
	IOHIDSystem/IOHIDDeviceShim.cpp \
	IOHIDFamily/IOHIDEventServiceQueue.cpp \
	IOHIDSystem/IOHIDUserClient.cpp \
	IOHIDSystem/IOFixed64.cpp \
	IOHIDSystem/IOHIDWorkLoop.cpp \
	IOHIDSystem/IOFixedPoint64.cpp \
	IOHIDSystem/IOHIDSystemCursorHelper.cpp \
	IOHIDFamily/IOHIDResourceUserClient.cpp \
	IOHIDFamily/IOHIDResource.cpp

CSTD = none
CXXSTD = none

SLF = /System/Library/Frameworks
CFLAGS = -DOMITPENDINGKEYCAPS \
        -Wno-implicit-int-conversion -Wno-sign-conversion \
        -Wno-elaborated-enum-base -Wno-nullability-completeness \
	-DAVAILABLE_MAC_OS_X_VERSION_10_13_AND_LATER="" \
	-I${.CURDIR} -I${.CURDIR:H} -I${.CURDIR:H}/IOHIDFamily \
	-I${.CURDIR}/IOKit/hidsystem \
        -I${RAVYN_SDKROOT}/usr/local/include/kernel \
	-I${ROOT_BINARY_DIR}${ROOT_SOURCE_DIR}/Kernel/xnu/EXPORT_HDRS/iokit \
	-I${ROOT_BINARY_DIR}${ROOT_SOURCE_DIR}/Kernel/xnu/EXPORT_HDRS/bsd \
	-I${ROOT_BINARY_DIR}${ROOT_SOURCE_DIR}/Kernel/xnu/EXPORT_HDRS/libkern \
	-I${ROOT_BINARY_DIR}${ROOT_SOURCE_DIR}/Kernel/xnu/EXPORT_HDRS/osfmk \
	-I${ROOT_BINARY_DIR}${ROOT_SOURCE_DIR}/Kernel/xnu/EXPORT_HDRS/san \
	-include${RAVYN_SDKROOT}/usr/include/i386/endian.h \
	-include${RAVYN_SDKROOT}/usr/local/include/kernel/os/object.h \
	${CFLAGS.${.TARGET}}

LDFLAGS = -mmacos-version-min=${MACOS_VERSION_MIN} --sysroot=${RAVYN_SDKROOT} \
	-fuse-ld=${LD}


.include <rvn.kext.mk>
