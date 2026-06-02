MK_WERROR = no
KEXT = hfs
KERNEL_PRIVATE = yes
INFO_PLIST = core/macosx-Info.plist
BUNDLE_IDENTIFIER = com.apple.filesystems.hfs.kext
BUNDLE_VERSION = 1.7
#hfs_key_roll.c
SRCS = core/hfs_resize.c \
	core/hfs_vnops.c \
	core/hfs_btreeio.c \
	core/BTreeNodeOps.c \
	core/BTree.c \
	core/rangelist.c \
	core/hfs_journal.c \
	core/hfs_fsinfo.c \
	core/hfs_chash.c \
	core/hfs_xattr.c \
	core/FileIDsServices.c \
	core/hfs_extents.c \
	core/hfs_endian.c \
	core/hfs_cprotect.c \
	core/hfs_readwrite.c \
	core/CatalogUtilities.c \
	core/BTreeWrapper.c \
	core/hfs_catalog.c \
	core/hfs_hotfiles.c \
	core/hfs_quota.c \
	core/hfs_lookup.c \
	core/hfs_search.c \
	core/hfs_vfsops.c \
	core/BTreeAllocate.c \
	core/VolumeAllocation.c \
	core/hfs_cnode.c \
	core/BTreeTreeOps.c \
	core/BTreeMiscOps.c \
	core/FileExtentMapping.c \
	core/BTreeNodeReserve.c \
	core/hfs_vfsutils.c \
	core/hfs_attrlist.c \
	core/MacOSStubs.c \
	core/BTreeScanner.c \
	core/UnicodeWrappers.c \
	core/hfs_notification.c \
	core/hfs_link.c \
	core/hfs_iokit.cpp

SLF = /System/Library/Frameworks
CFLAGS = -DPRIVATE=1 -DKERNEL=1 -D__APPLE_API_PRIVATE=1 \
	-Wno-implicit-int-conversion -Wno-sign-conversion \
	-I${.CURDIR}/core \
	-I${ROOT_SOURCE_DIR}/Kernel \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/iokit \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/osfmk \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/libkern \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/bsd \
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
CXXFLAGS = ${CFLAGS}

SUBDIR = hfs_encodings

.include <rvn.kext.mk>
.include <bsd.subdir.mk>
