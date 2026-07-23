MK_WERROR = no
KEXT = IOUSBFamily
KERNEL_PRIVATE = yes
INFO_PLIST = Info.plist
BUNDLE_IDENTIFIER = com.ravynos.iokit.IOUSBFamily
BUNDLE_VERSION = 1.0.0

.PATH: ${.CURDIR}/src

SRCS = IOUSBBus.cpp IOUSBNub.cpp IOUSBController.cpp IOUSBControllerV2.cpp \
	IOUSBDevice.cpp IOUSBInterface.cpp IOUSBPipe.cpp

SLF = /System/Library/Frameworks
CFLAGS = -Wno-implicit-int-conversion -Wno-inconsistent-missing-override \
	-Wno-sign-conversion -Wno-shadow -Wno-unused-parameter \
	-I${.CURDIR}/include \
	-I${.CURDIR}/include/IOKit/usb \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/iokit \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/osfmk \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/bsd \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/libkern \
	-I${ROOT_BINARY_DIR}${ROOT_SOURCE_DIR}/Kernel/xnu/EXPORT_HDRS/osfmk
CXXFLAGS = ${CFLAGS} -std=c++11

.include <rvn.kext.mk>
