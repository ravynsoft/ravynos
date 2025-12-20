set(TOOLS ${TOOLCHAIN}/usr/bin)
set(PATH "${TOOLS}:$ENV{PATH}")

set(CC ${TOOLS}/cc)
set(CXX ${TOOLS}/c++)
set(LD ${TOOLS}/ld)
set(AR ${TOOLS}/ar)
set(RANLIB ${TOOLS}/ranlib)
set(NM ${TOOLS}/nm)
set(CODESIGN_ALLOCATE ${TOOLS}/codesign_allocate)
set(CTFCONVERT ${TOOLS}/ctfconvert)
set(CTFMERGE ${TOOLS}/ctfmerge)
set(CTFINSERT ${TOOLS}/ctfinsert)
set(IIG /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/iig)
set(MIG ${TOOLS}/mig)
set(MIGCOM ${TOOLS}/../libexec/migcom)
set(STRIP ${TOOLS}/strip)
set(LIPO ${TOOLS}/lipo)
set(NM ${TOOLS}/nm)
set(NMEDIT ${TOOLS}/nmedit)
set(LIBTOOL ${TOOLS}/libtool)
set(UNIFDEF ${TOOLS}/unifdef)
set(DSYMUTIL ${TOOLS}/dsymutil)
set(XCRUN ${TOOLS}/xcrun)
set(XCBUILD ${TOOLS}/xcbuild)
set(XCODEBUILD ${TOOLS}/xcodebuild)

string(TOUPPER "${CMAKE_SYSTEM_PROCESSOR}" MACHINE)
# Do not set these CONFIGS to multiple values! Only RELEASE|DEBUG and one arch
set(KERNEL_CONFIGS RELEASE)
set(ARCH_CONFIGS X86_64)
set(AVAILABILITY_PL ${CMAKE_SOURCE_DIR}/Kernel/availability.pl)
set(AVAILABILITY_PL_PATH ${CMAKE_SOURCE_DIR}/Kernel/availability.pl)
set(SEED_DEFINES 
    -DRC_ENABLE_XNU_PRODUCT_INFO_FILTER
)

if("${KERNEL_CONFIGS}" STREQUAL "DEBUG")
    set(DEBUG ".debug")
else()
    set(DEBUG "")
endif()

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/sysroot/usr/include
    ${CMAKE_BINARY_DIR}/sysroot/usr/local/include/kernel)
file(COPY ${CMAKE_SOURCE_DIR}/Libraries/Libc/include/limits.h
    DESTINATION ${CMAKE_BINARY_DIR}/sysroot/usr/include/)
file(COPY ${CMAKE_SOURCE_DIR}/Libraries/libdispatch/os
    DESTINATION ${CMAKE_BINARY_DIR}/sysroot/usr/local/include/kernel/)

ExternalProject_Add(xnu_headers
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/Kernel/xnu
    BINARY_DIR ${CMAKE_BINARY_DIR}/Kernel/xnu
    CONFIGURE_COMMAND ""
    BUILD_COMMAND
    	${GMAKE} -C <SOURCE_DIR>
    	    PLATFORM=MacOSX BUILD_LTO=0 BUILD_WERROR=0 DO_CTFMERGE=0
    	    SDKVERSION=${CMAKE_MACOSX_MIN_VERSION} SRCROOT=<SOURCE_DIR>
    	    OBJROOT=<BINARY_DIR> DSTROOT=${CMAKE_BINARY_DIR}/sysroot
            MIGCOM=${MIGCOM} MIG=${MIG} IIG=${IIG} CC=${CC} LD=${CC}
            UNIFDEF=${UNIFDEF} CTFCONVERT=${CTFCONVERT} CTFMERGE=${CTFMERGE}
            CTFINSERT=${CTFINSERT} AVAILABILITY_PL=${AVAILABILITY_PL}
            AVAILABILITY_PL_PATH=${AVAILABILITY_PL_PATH}
            KERNEL_CONFIGS=${KERNEL_CONFIGS} ARCH_CONFIGS=${ARCH_CONFIGS}
    	    installhdrs
    INSTALL_COMMAND
        ${GMAKE} -C <SOURCE_DIR>
    	    PLATFORM=MacOSX BUILD_LTO=0 BUILD_WERROR=0 DO_CTFMERGE=0
    	    SDKVERSION=${CMAKE_MACOSX_MIN_VERSION} SRCROOT=<SOURCE_DIR>
    	    OBJROOT=<BINARY_DIR>
            DSTROOT=${CMAKE_BINARY_DIR}/Developer/Platforms/ravynOS.platform/Developer/SDKs/ravynOS.sdk
            MIGCOM=${MIGCOM} MIG=${MIG} IIG=${IIG} CC=${CC} LD=${CC}
            UNIFDEF=${UNIFDEF} CTFCONVERT=${CTFCONVERT} CTFMERGE=${CTFMERGE}
            CTFINSERT=${CTFINSERT} AVAILABILITY_PL=${AVAILABILITY_PL}
            AVAILABILITY_PL_PATH=${AVAILABILITY_PL_PATH}
            KERNEL_CONFIGS=${KERNEL_CONFIGS} ARCH_CONFIGS=${ARCH_CONFIGS}
    	    installhdrs
	DEPENDS BootstrapTools host_migcom
)

add_subdirectory(libfirehose_kernel)
add_dependencies(libfirehose_kernel CarbonHeaders)

ExternalProject_Add(xnu
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/Kernel/xnu
    BINARY_DIR ${CMAKE_BINARY_DIR}/Kernel/xnu
    CONFIGURE_COMMAND ""
    BUILD_COMMAND
    	${GMAKE} -C <SOURCE_DIR>
    	    PLATFORM=MacOSX BUILD_LTO=0 BUILD_WERROR=0 DO_CTFMERGE=0
    	    SDKVERSION=${CMAKE_MACOSX_MIN_VERSION} SRCROOT=<SOURCE_DIR>
    	    OBJROOT=<BINARY_DIR> DSTROOT=${CMAKE_BINARY_DIR}/sysroot
    	    SYMROOT=${CMAKE_BINARY_DIR} BUILD_STATIC_LINK=1
            MIGCOM=${MIGCOM} MIG=${MIG} IIG=${IIG} CC=${CC} LD=${CC}
            UNIFDEF=${UNIFDEF} CTFCONVERT=${CTFCONVERT} CTFMERGE=${CTFMERGE}
            CTFINSERT=${CTFINSERT} AR=${AR} NM=${NM} NMEDIT=${NMEDIT}
            DSYMUTIL=${DSYMUTIL} LIPO=${LIPO} STRIP=${STRIP} AS=${AS}
            AVAILABILITY_PL=${AVAILABILITY_PL} MACHINE=${MACHINE}
            AVAILABILITY_PL_PATH=${AVAILABILITY_PL_PATH} SDKROOT=macosx
            KERNEL_CONFIGS=${KERNEL_CONFIGS} ARCH_CONFIGS=${ARCH_CONFIGS}
    	    all
    INSTALL_COMMAND
        ${GMAKE} -C <SOURCE_DIR>
            PLATFORM=MacOSX BUILD_LTO=0 BUILD_WERROR=0 DO_CTFMERGE=0
            SDKVERSION=${CMAKE_MACOSX_MIN_VERSION} SRCROOT=<SOURCE_DIR>
            OBJROOT=<BINARY_DIR> DSTROOT=${CMAKE_BINARY_DIR}/sysroot
            MIGCOM=${MIGCOM} MIG=${MIG} IIG=${IIG} CC=${CC} LD=${CC}
            UNIFDEF=${UNIFDEF} CTFCONVERT=${CTFCONVERT} CTFMERGE=${CTFMERGE}
            CTFINSERT=${CTFINSERT} AR=${AR} NM=${NM} NMEDIT=${NMEDIT}
            DSYMUTIL=${DSYMUTIL} LIPO=${LIPO} STRIP=${STRIP} AS=${AS}
            AVAILABILITY_PL=${AVAILABILITY_PL} MACHINE=${MACHINE}
            AVAILABILITY_PL_PATH=${AVAILABILITY_PL_PATH}
            KERNEL_CONFIGS=${KERNEL_CONFIGS} ARCH_CONFIGS=${ARCH_CONFIGS}
            install && ${CMAKE_COMMAND} -E make_directory 
            ${RAVYN_SDKROOT}/System/Library/Kernels && ${CMAKE_COMMAND} -E 
            copy <BINARY_DIR>/${KERNEL_CONFIGS}_${ARCH_CONFIGS}/kernel${DEBUG}.dSYM
            ${RAVYN_SDKROOT}/System/Library/Kernels/
	DEPENDS xnu_headers CarbonHeaders libfirehose_kernel BootstrapTools
)

# Build kernel extensions
add_subdirectory(libkmod)
add_subdirectory(Extensions)

# kext: libkmod
# 	mkdir -pv ${OBJTOP}/Kernel/Extensions/include/pthread
# 	mkdir -pv ${OBJTOP}/Kernel/Extensions/include/IOKit/{storage,ata}
# 	cp -fv ${SRCTOP}/Libraries/libpthread/private/qos_private.h \
# 		${OBJTOP}/Kernel/Extensions/include/pthread/
# 	cp -fv ${SRCTOP}/Kernel/Extensions/IOATAFamily/IOATARegI386.h \
# 		${OBJTOP}/Kernel/Extensions/include/IOKit/ata/
# 	cp -fv ${SRCTOP}/Kernel/Extensions/IOStorageFamily/*.h \
# 		${OBJTOP}/Kernel/Extensions/include/IOKit/storage/
# 	cd ${OBJTOP}/Kernel/Extensions;	cmake -Wno-dev \
# 		-DCMAKE_INSTALL_PREFIX=/System/Library/Extensions \
# 		-DCMAKE_BUILD_TYPE=Release -DSRCTOP=${SRCTOP} -DOBJTOP=${OBJTOP} \
# 		-G"Unix Makefiles" ${SRCTOP}/Kernel/Extensions
# 	export PATH PLATFORM=MacOSX MAKEFLAGS="" MAKE=${GMAKE}; \
# 		${GMAKE} DESTDIR=${BUILDROOT} -C ${OBJTOP}/Kernel/Extensions \
# 		install
