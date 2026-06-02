MK_WERROR = no
KEXT = msdosfs
KERNEL_PRIVATE = yes
INFO_PLIST = Info-msdosfs_kext.plist
BUNDLE_IDENTIFIER = com.apple.filesystems.msdosfs
BUNDLE_VERSION = 1.10.0

.PATH: ${.CURDIR}/msdosfs.kextproj/msdosfs.kmodproj
SRCS = 	msdosfs_conv.c \
	msdosfs_denode.c \
	msdosfs_fat.c \
	msdosfs_lookup.c \
	msdosfs_vfsops.c \
	msdosfs_vnops.c

CFLAGS = -DPRIVATE -DDEBUG=0 -DKERNEL=1 \
	-Wno-implicit-int-conversion -Wno-sign-conversion \
	-I${.CURDIR}/include \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/iokit \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/osfmk \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/bsd \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/libkern \
	-I${ROOT_BINARY_DIR}${ROOT_SOURCE_DIR}/Kernel/xnu/EXPORT_HDRS/osfmk \
	-I${ROOT_BINARY_DIR}${ROOT_SOURCE_DIR}/Kernel/xnu/EXPORT_HDRS/san \
	-I${ROOT_BINARY_DIR}${ROOT_SOURCE_DIR}/Kernel/xnu/EXPORT_HDRS \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/EXTERNAL_HEADERS \
	-I${.CURDIR:H}/IOStorageFamily \
	-include${ROOT_SOURCE_DIR}/Kernel/xnu/bsd/i386/endian.h

.include <rvn.kext.mk>
