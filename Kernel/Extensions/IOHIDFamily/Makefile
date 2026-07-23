MK_WERROR = no
KEXT = IOHIDFamily
BUNDLE_IDENTIFIER = com.apple.iokit.IOHIDFamily
BUNDLE_VERSION = 1.5.2
INFO_PLIST = IOHIDFamily/Info-IOHIDFamily.plist
KERNEL_PRIVATE = yes

SRCS = \
        IOHIDSystem/IOHIDevice.cpp \
        IOHIDSystem/IOHIKeyboard.cpp \
        IOHIDSystem/IOHIKeyboardMapper.cpp \
        IOHIDSystem/IOBSDConsole.cpp \
        IOHIDSystem/IOFixed64.cpp \
        IOHIDSystem/IOFixedPoint64.cpp \
        IOHIDSystem/IOHIDPrivate.cpp \
        IOHIDSystem/IOHIDSystem.cpp \
        IOHIDSystem/IOHIDSystemCursorHelper.cpp \
        IOHIDSystem/IOHIDUserClient.cpp \
        IOHIDSystem/IOHIDWorkLoop.cpp \
        \
        IOHIDFamily/IOHIDEvent.cpp \
        IOHIDFamily/IOHIDFamilyPrivate.cpp \
        IOHIDFamily/IOHIDResource.cpp

CSTD = none
CXXSTD = none

SLF = /System/Library/Frameworks
CFLAGS = -Wno-implicit-int-conversion -Wno-sign-conversion \
        -Wno-elaborated-enum-base -Wno-nullability-completeness \
	-DAVAILABLE_MAC_OS_X_VERSION_10_13_AND_LATER="" -DPRIVATE \
        -DOMITPENDINGKEYCAPS \
	-I${.CURDIR}/IOHIDSystem -I${.CURDIR} -I${.CURDIR}/IOHIDFamily \
	-I${.CURDIR}/IOHIDSystem/IOKit/hidsystem \
        -I${RAVYN_SDKROOT}/usr/local/include/kernel \
	-I${ROOT_BINARY_DIR}${ROOT_SOURCE_DIR}/Kernel/xnu/EXPORT_HDRS/iokit \
	-I${ROOT_BINARY_DIR}${ROOT_SOURCE_DIR}/Kernel/xnu/EXPORT_HDRS/bsd \
	-I${ROOT_BINARY_DIR}${ROOT_SOURCE_DIR}/Kernel/xnu/EXPORT_HDRS/libkern \
	-I${ROOT_BINARY_DIR}${ROOT_SOURCE_DIR}/Kernel/xnu/EXPORT_HDRS/osfmk \
	-I${ROOT_BINARY_DIR}${ROOT_SOURCE_DIR}/Kernel/xnu/EXPORT_HDRS/san \
	-I${ROOT_BINARY_DIR}${ROOT_SOURCE_DIR}/Kernel/xnu/EXPORT_HDRS/security \
	-include${RAVYN_SDKROOT}/usr/include/i386/endian.h \
	-include${RAVYN_SDKROOT}/usr/local/include/kernel/os/object.h \
	${CFLAGS.${.TARGET}}

LDFLAGS = -mmacos-version-min=${MACOS_VERSION_MIN} --sysroot=${RAVYN_SDKROOT} \
	-fuse-ld=${LD}

CFLAGS.IOHIDSystem.o = -include${.CURDIR}/IOHIDFamily/IOHIDElement.h

all: copyheaders

copyheaders:
	cp -fv ${.CURDIR}/IOHIDFamily/IOHIDInterface.h \
	    ${RAVYN_SDKROOT}${SLF}/IOKit.framework/Headers/hid/
	mkdir -p ${RAVYN_SDKROOT}${SLF}/IOKit.framework/Headers/usb
	touch ${RAVYN_SDKROOT}${SLF}/IOKit.framework/Headers/usb/IOUSBHostFamily.h

SUBDIR = IOHIDSystem
#SUBDIR += HID

.include <bsd.subdir.mk>
.include <rvn.kext.mk>


