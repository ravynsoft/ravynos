add_kext_bundle(msdosfs
    KERNEL_PRIVATE
    INFO_PLIST Info-msdosfs_kext.plist
    BUNDLE_IDENTIFIER com.apple.filesystems.msdosfs
    BUNDLE_VERSION 1.10.0)

target_sources(msdosfs PRIVATE
	./msdosfs.kextproj/msdosfs.kmodproj/msdosfs_conv.c
	./msdosfs.kextproj/msdosfs.kmodproj/msdosfs_denode.c
	./msdosfs.kextproj/msdosfs.kmodproj/msdosfs_fat.c
	./msdosfs.kextproj/msdosfs.kmodproj/msdosfs_lookup.c
	./msdosfs.kextproj/msdosfs.kmodproj/msdosfs_vfsops.c
	./msdosfs.kextproj/msdosfs.kmodproj/msdosfs_vnops.c
)

target_include_directories(msdosfs PRIVATE
    include
    ../../xnu/iokit
    ../../xnu/osfmk
    ../../xnu/bsd
    ../../xnu/libkern
    ${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/osfmk
    ${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/
    ../../xnu/EXTERNAL_HEADERS
    ../IOStorageFamily)

target_compile_definitions(msdosfs PRIVATE DEBUG=0 KERNEL=1)

target_compile_options(msdosfs PRIVATE
    -include${CMAKE_CURRENT_SOURCE_DIR}/../../xnu/bsd/i386/endian.h
    -Wno-implicit-int-conversion -Wno-sign-conversion
    $<$<COMPILE_LANGUAGE:ASM>:-x assembler-with-cpp>)

target_link_options(msdosfs PRIVATE
    -mmacos-version-min=${MACOS_VERSION_MIN})

add_dependencies(msdosfs kextBuild)
