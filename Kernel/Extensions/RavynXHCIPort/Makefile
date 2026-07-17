MK_WERROR = no
KEXT = RavynXHCIPort
KERNEL_PRIVATE = yes
INFO_PLIST = Info.plist
BUNDLE_IDENTIFIER = com.ravynos.driver.RavynXHCIPort
BUNDLE_VERSION = 1.0.0

SRCS = RavynXHCIPort.cpp RavynXHCIMassStorageDisk.cpp RavynXHCIUSBBus.cpp

SLF = /System/Library/Frameworks
CFLAGS = -Wno-implicit-int-conversion -Wno-sign-conversion \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/iokit \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/osfmk \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/bsd \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/libkern \
	-I${.CURDIR:H}/IOStorageFamily \
	-I${.CURDIR:H}/IOPCIFamily \
	-I${.CURDIR:H}/IOUSBFamily/include \
	-I${.CURDIR:H}/IOUSBFamily/include/IOKit/usb \
	-I${ROOT_BINARY_DIR}${ROOT_SOURCE_DIR}/Kernel/xnu/EXPORT_HDRS/osfmk

.include <rvn.kext.mk>
