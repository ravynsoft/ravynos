MK_WERROR = no
KEXT = IOGOPFramebuffer
KERNEL_PRIVATE = yes
INFO_PLIST = Info-IOGOPFramebuffer.plist
BUNDLE_IDENTIFIER = com.ravynos.iokit.IOGOPFramebuffer
BUNDLE_VERSION = 1.0.0

CSTD = gnu11
CXXSTD = gnu++11

SRCS = IOGOPFramebuffer.cpp

SLF = /System/Library/Frameworks
CFLAGS = -DKERNEL -Wno-implicit-int-conversion -Wno-sign-conversion \
	-Wno-c++11-narrowing \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/iokit \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/osfmk \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/bsd \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/libkern \
	-I${.CURDIR:H}/IOPCIFamily \
	-I${ROOT_BINARY_DIR}${ROOT_SOURCE_DIR}/Kernel/xnu/EXPORT_HDRS/osfmk \
	-I${ROOT_BINARY_DIR}${ROOT_SOURCE_DIR}/Kernel/xnu/EXPORT_HDRS/libkern \
	-I${ROOT_BINARY_DIR}${ROOT_SOURCE_DIR}/Kernel/xnu/EXPORT_HDRS/san \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/EXTERNAL_HEADERS

#SUBDIR = IOGraphicsFamily IONDRVSupport

.include <rvn.kext.mk>
.include <bsd.subdir.mk>
