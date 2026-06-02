MK_WERROR = no
KEXT = hfs_encodings
KERNEL_PRIVATE = yes
INFO_PLIST = HFSEncodings-Info.plist
BUNDLE_IDENTIFIER = com.apple.filesystems.hfs.encodings.kext
BUNDLE_VERSION = 1.0.0

SRCS = hfs_encodinghint.c \
	hfs_encodings_kext.cpp\
	hfs_encodings.c

SLF = /System/Library/Frameworks
CFLAGS = -DPRIVATE=1 -DKERNEL=1 \
	-Wno-implicit-int-conversion \
	-Wno-sign-conversion \
	-I${.CURDIR}/core \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/bsd \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/iokit \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/osfmk \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/libkern \
	-I${ROOT_BINARY_DIR}${ROOT_SOURCE_DIR}/Kernel/xnu/EXPORT_HDRS/osfmk \
	-I${ROOT_BINARY_DIR}${ROOT_SOURCE_DIR}/Kernel/xnu/EXPORT_HDRS/san \
	-I${ROOT_BINARY_DIR}${ROOT_SOURCE_DIR}/Kernel/xnu/EXPORT_HDRS \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/EXTERNAL_HEADERS \
	-I${.CURDIR:H}/IOStorageFamily \
	-I${RAVYN_SDKROOT}${SLF}/System.framework/PrivateHeaders \
	-include${ROOT_SOURCE_DIR}/Kernel/xnu/osfmk/libsa/i386/types.h \
	-include${ROOT_SOURCE_DIR}/Kernel/xnu/bsd/sys/disk.h \
	-include${ROOT_SOURCE_DIR}/Kernel/xnu/bsd/sys/fcntl.h \
	-include${ROOT_SOURCE_DIR}/Kernel/xnu/bsd/sys/attr.h

.include <rvn.kext.mk>
