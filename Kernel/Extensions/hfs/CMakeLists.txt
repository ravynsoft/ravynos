add_kext_bundle(hfs
    KERNEL_PRIVATE
    INFO_PLIST core/macosx-Info.plist
    BUNDLE_IDENTIFIER com.apple.filesystems.hfs.kext
    BUNDLE_VERSION 1.7)

target_sources(hfs PRIVATE
    ./core/hfs_resize.c
    ./core/hfs_vnops.c
    ./core/hfs_btreeio.c
    ./core/BTreeNodeOps.c
    ./core/BTree.c
    ./core/rangelist.c
    ./core/hfs_journal.c
    ./core/hfs_fsinfo.c
    ./core/hfs_chash.c
    ./core/hfs_xattr.c
     #hfs_key_roll.c
    ./core/FileIDsServices.c
    ./core/hfs_extents.c
    ./core/hfs_endian.c
    ./core/hfs_cprotect.c
    ./core/hfs_readwrite.c
    ./core/CatalogUtilities.c
    ./core/BTreeWrapper.c
    ./core/hfs_catalog.c
    ./core/hfs_hotfiles.c
    ./core/hfs_quota.c
    ./core/hfs_lookup.c
    ./core/hfs_search.c
    ./core/hfs_vfsops.c
    ./core/BTreeAllocate.c
    ./core/VolumeAllocation.c
    ./core/hfs_cnode.c
    ./core/BTreeTreeOps.c
    ./core/hfs_iokit.cpp
    ./core/BTreeMiscOps.c
    ./core/FileExtentMapping.c
    ./core/BTreeNodeReserve.c
    ./core/hfs_vfsutils.c
    ./core/hfs_attrlist.c
    ./core/MacOSStubs.c
    ./core/BTreeScanner.c
    ./core/UnicodeWrappers.c
    ./core/hfs_notification.c
    ./core/hfs_link.c)

target_include_directories(hfs PRIVATE
    core
    ../../xnu/iokit
    ../../xnu/osfmk
    ../../xnu/libkern
    ../../xnu/bsd
    ${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/osfmk
    ${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/san
    ${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/
    ../../xnu/EXTERNAL_HEADERS
    ../IOStorageFamily
    ${RAVYN_SDKROOT}/System/Library/Frameworks/System.framework/PrivateHeaders)

target_compile_options(hfs PRIVATE
    -mmacos-version-min=${MACOS_VERSION_MIN}
    -include${ROOT_SOURCE_DIR}/Kernel/xnu/osfmk/libsa/i386/types.h
    -include${ROOT_SOURCE_DIR}/Kernel/xnu/bsd/sys/disk.h
    -include${ROOT_SOURCE_DIR}/Kernel/xnu/bsd/sys/fcntl.h
    -include${ROOT_SOURCE_DIR}/Kernel/xnu/bsd/sys/attr.h
    -DPRIVATE=1 -DKERNEL=1 -D__APPLE_API_PRIVATE=1
    -Wno-implicit-int-conversion -Wno-sign-conversion
    $<$<COMPILE_LANGUAGE:ASM>:-x assembler-with-cpp>)

target_link_options(hfs PRIVATE
    -mmacos-version-min=${MACOS_VERSION_MIN})

add_kext_bundle(hfs_encodings
    KERNEL_PRIVATE
    INFO_PLIST hfs_encodings/HFSEncodings-Info.plist
    BUNDLE_IDENTIFIER com.apple.filesystems.hfs.encodings.kext
    BUNDLE_VERSION 1.0.0)

target_sources(hfs_encodings PRIVATE
	./hfs_encodings/hfs_encodinghint.c
	./hfs_encodings/hfs_encodings_kext.cpp
	./hfs_encodings/hfs_encodings.c)

target_include_directories(hfs_encodings PRIVATE
    core
    ../../xnu/bsd
    ../../xnu/iokit
    ../../xnu/osfmk
    ../../xnu/libkern
    ${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/osfmk
    ${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/san
    ${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/
    ../../xnu/EXTERNAL_HEADERS
    ../IOStorageFamily
    ${RAVYN_SDKROOT}/System/Library/Frameworks/System.framework/PrivateHeaders)

target_compile_options(hfs_encodings PRIVATE
    -mmacos-version-min=${MACOS_VERSION_MIN}
    -include${ROOT_SOURCE_DIR}/Kernel/xnu/osfmk/libsa/i386/types.h
    -include${ROOT_SOURCE_DIR}/Kernel/xnu/bsd/sys/disk.h
    -include${ROOT_SOURCE_DIR}/Kernel/xnu/bsd/sys/fcntl.h
    -include${ROOT_SOURCE_DIR}/Kernel/xnu/bsd/sys/attr.h
    -DPRIVATE=1 -DKERNEL=1
    -Wno-implicit-int-conversion -Wno-sign-conversion
    $<$<COMPILE_LANGUAGE:ASM>:-x assembler-with-cpp>)

target_link_options(hfs_encodings PRIVATE
    -mmacos-version-min=${MACOS_VERSION_MIN})

add_dependencies(hfs kextBuild)
add_dependencies(hfs_encodings kextBuild)