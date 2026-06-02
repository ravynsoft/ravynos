MK_WERROR = no
KEXT = AppleFileSystemDriver
KERNEL_PRIVATE = yes
INFO_PLIST = Info.plist
BUNDLE_IDENTIFIER = com.apple.driver.AppleFileSystemDriver
BUNDLE_VERSION = 1.0.0

SRCS = AppleFileSystemDriver.cpp

SLF = /System/Library/Frameworks
CFLAGS = -Wno-implicit-int-conversion -Wno-sign-conversion \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/iokit \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/osfmk \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/bsd \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/libkern \
	-I${ROOT_BINARY_DIR}${ROOT_SOURCE_DIR}/Kernel/xnu/EXPORT_HDRS/osfmk \
	-I${ROOT_BINARY_DIR}${ROOT_SOURCE_DIR}/Kernel/xnu/EXPORT_HDRS \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/EXTERNAL_HEADERS \
	-I${.CURDIR:H}/IOStorageFamily \
	-I${.OBJDIR} \
	-I${RAVYN_SDKROOT}${SLF}/System.framework/PrivateHeaders

all: copyheaders

copyheaders:
	mkdir -p ${.OBJDIR}/hfs
	cp -f ${.CURDIR:H}/hfs/core/hfs_format.h \
	  ${.CURDIR:H}/hfs/core/hfs_unistr.h ${.OBJDIR}/hfs/

.include <rvn.kext.mk>
