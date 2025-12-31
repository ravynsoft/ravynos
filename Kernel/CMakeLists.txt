# ------------------------------------------------------------------------
#  RAVYNOS BUILD SYSTEM - Darwin Base System
# ------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.15.1)
project(ravynOS_Darwin)

include(${ROOT_SOURCE_DIR}/Developer/cmake/Version.cmake)
include(${ROOT_SOURCE_DIR}/Developer/cmake/Paths.cmake)
include(${ROOT_SOURCE_DIR}/Developer/cmake/Tools.cmake)

string(TOUPPER "${CMAKE_SYSTEM_PROCESSOR}" MACHINE)
set(AVAILABILITY_PL ${CMAKE_SOURCE_DIR}/availability.pl)
set(AVAILABILITY_PL_PATH ${CMAKE_SOURCE_DIR}/availability.pl)
set(SEED_DEFINES 
    -DRC_ENABLE_XNU_PRODUCT_INFO_FILTER
)
set(PATH $ENV{PATH})

if("${KERNEL_CONFIGS}" STREQUAL "DEBUG")
    set(DEBUG ".debug")
else()
    set(DEBUG "")
endif()

if(CMAKE_HOST_SYSTEM_NAME MATCHES "Linux")
    # set(HOST_CC /usr/bin/clang)
    # set(HOST_CXX /usr/bin/clang++)
    # set(HOST_LD /usr/bin/ld)
    set(HOST_CODESIGN=/bin/true)
    set(IIG /bin/true)
    file(GLOB IIG_HDRS xnu/iokit/DriverKit/*.h)
    file(COPY ${IIG_HDRS} DESTINATION 
        ${CMAKE_CURRENT_BINARY_DIR}/xnu/${KERNEL_CONFIGS}_${ARCH_CONFIGS}/iokit/DriverKit/)
endif()

file(MAKE_DIRECTORY ${ROOT_BINARY_DIR}/sysroot/usr/include
    ${ROOT_BINARY_DIR}/sysroot/usr/local/include/kernel)
file(COPY ${ROOT_SOURCE_DIR}/Libraries/Libc/include/limits.h
    DESTINATION ${ROOT_BINARY_DIR}/sysroot/usr/include/)
file(COPY ${ROOT_SOURCE_DIR}/Libraries/libdispatch/os
    DESTINATION ${ROOT_BINARY_DIR}/sysroot/usr/local/include/kernel/)

ExternalProject_Add(xnu_headers
    SOURCE_DIR ${ROOT_SOURCE_DIR}/Kernel/xnu
    BINARY_DIR ${ROOT_BINARY_DIR}/Kernel/xnu
    STEP_TARGETS build install
    CONFIGURE_COMMAND ""
    BUILD_COMMAND
    	PATH=${PATH} ${GMAKE} -C <SOURCE_DIR>
    	    PLATFORM=MacOSX BUILD_LTO=0 BUILD_WERROR=0 DO_CTFMERGE=0
    	    SDKVERSION=${CMAKE_MACOSX_MIN_VERSION} SRCROOT=<SOURCE_DIR>
    	    OBJROOT=<BINARY_DIR> DSTROOT=${RAVYN_SDKROOT}
            MIGCOM=${MIGCOM} MIG=${MIG} IIG=${IIG} CC=${CC} LD=${CC}
            UNIFDEF=${UNIFDEF} CTFCONVERT=${CTFCONVERT} CTFMERGE=${CTFMERGE}
            CTFINSERT=${CTFINSERT} AVAILABILITY_PL=${AVAILABILITY_PL}
            AVAILABILITY_PL_PATH=${AVAILABILITY_PL_PATH} XCRUN=${XCRUN}
            KERNEL_CONFIGS=${KERNEL_CONFIGS} ARCH_CONFIGS=${ARCH_CONFIGS}
            DEVELOPER_DIR=${ROOT_BINARY_DIR}/Developer
            SYMROOT=${ROOT_BINARY_DIR} HOST_CC=${HOST_CC}
            HOST_CXX=${HOST_CXX} HOST_LD=${HOST_LD}
    	    installhdrs
    INSTALL_COMMAND
        PATH=${PATH} ${GMAKE} -C <SOURCE_DIR>
    	    PLATFORM=MacOSX BUILD_LTO=0 BUILD_WERROR=0 DO_CTFMERGE=0
    	    SDKVERSION=${CMAKE_MACOSX_MIN_VERSION} SRCROOT=<SOURCE_DIR>
    	    OBJROOT=<BINARY_DIR> DSTROOT=${ROOT_BINARY_DIR}/sysroot
            MIGCOM=${MIGCOM} MIG=${MIG} IIG=${IIG} CC=${CC} LD=${CC}
            UNIFDEF=${UNIFDEF} CTFCONVERT=${CTFCONVERT} CTFMERGE=${CTFMERGE}
            CTFINSERT=${CTFINSERT} AVAILABILITY_PL=${AVAILABILITY_PL}
            AVAILABILITY_PL_PATH=${AVAILABILITY_PL_PATH} XCRUN=${XCRUN}
            KERNEL_CONFIGS=${KERNEL_CONFIGS} ARCH_CONFIGS=${ARCH_CONFIGS}
            DEVELOPER_DIR=${ROOT_BINARY_DIR}/Developer
            SYMROOT=${ROOT_BINARY_DIR} HOST_CC=${HOST_CC}
            HOST_CXX=${HOST_CXX} HOST_LD=${HOST_LD}
    	    installhdrs
)

add_subdirectory(libfirehose_kernel)

ExternalProject_Add(xnu
    EXCLUDE_FROM_ALL
    SOURCE_DIR ${ROOT_SOURCE_DIR}/Kernel/xnu
    BINARY_DIR ${ROOT_BINARY_DIR}/Kernel/xnu
    STEP_TARGETS build install
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    	# ${GMAKE} -C <SOURCE_DIR>
        #     SDKROOT=${RAVYN_SDKROOT}
    	#     PLATFORM=MacOSX BUILD_LTO=0 BUILD_WERROR=0 DO_CTFMERGE=0
    	#     SDKVERSION=${CMAKE_MACOSX_MIN_VERSION} SRCROOT=<SOURCE_DIR>
    	#     OBJROOT=<BINARY_DIR> DSTROOT=${ROOT_BINARY_DIR}/sysroot
    	#     SYMROOT=${ROOT_BINARY_DIR} BUILD_STATIC_LINK=1
        #     MIGCOM=${MIGCOM} MIG=${MIG} IIG=${IIG} CC=${CC} LD=${CC}
        #     UNIFDEF=${UNIFDEF} CTFCONVERT=${CTFCONVERT} CTFMERGE=${CTFMERGE}
        #     CTFINSERT=${CTFINSERT} AR=${AR} NM=${NM} NMEDIT=${NMEDIT}
        #     DSYMUTIL=${DSYMUTIL} LIPO=${LIPO} STRIP=${STRIP} AS=${AS}
        #     AVAILABILITY_PL=${AVAILABILITY_PL} MACHINE=${MACHINE}
        #     AVAILABILITY_PL_PATH=${AVAILABILITY_PL_PATH} SDKROOT=macosx
        #     DEVELOPER_DIR=${ROOT_BINARY_DIR}/Developer XCRUN=${XCRUN}
        #     KERNEL_CONFIGS=${KERNEL_CONFIGS} ARCH_CONFIGS=${ARCH_CONFIGS}
        #     HOST_CC=${HOST_CC} HOST_CXX=${HOST_CXX} HOST_LD=${HOST_LD}
    	#     all
    INSTALL_COMMAND ""
        # ${GMAKE} -C <SOURCE_DIR>
        #     SDKROOT=${RAVYN_SDKROOT}
        #     PLATFORM=MacOSX BUILD_LTO=0 BUILD_WERROR=0 DO_CTFMERGE=0
        #     SDKVERSION=${CMAKE_MACOSX_MIN_VERSION} SRCROOT=<SOURCE_DIR>
        #     OBJROOT=<BINARY_DIR> DSTROOT=${ROOT_BINARY_DIR}/sysroot
        #     MIGCOM=${MIGCOM} MIG=${MIG} IIG=${IIG} CC=${CC} LD=${CC}
        #     UNIFDEF=${UNIFDEF} CTFCONVERT=${CTFCONVERT} CTFMERGE=${CTFMERGE}
        #     CTFINSERT=${CTFINSERT} AR=${AR} NM=${NM} NMEDIT=${NMEDIT}
        #     DSYMUTIL=${DSYMUTIL} LIPO=${LIPO} STRIP=${STRIP} AS=${AS}
        #     DEVELOPER_DIR=${ROOT_BINARY_DIR}/Developer XCRUN=${XCRUN}
        #     AVAILABILITY_PL=${AVAILABILITY_PL} MACHINE=${MACHINE}
        #     AVAILABILITY_PL_PATH=${AVAILABILITY_PL_PATH}
        #     KERNEL_CONFIGS=${KERNEL_CONFIGS} ARCH_CONFIGS=${ARCH_CONFIGS}
        #     HOST_CC=${HOST_CC} HOST_CXX=${HOST_CXX} HOST_LD=${HOST_LD}
        #     install && ${CMAKE_COMMAND} -E make_directory 
        #     ${RAVYN_SDKROOT}/System/Library/Kernels && ${CMAKE_COMMAND} -E 
        #     copy <BINARY_DIR>/${KERNEL_CONFIGS}_${ARCH_CONFIGS}/kernel${DEBUG}.dSYM
        #     ${RAVYN_SDKROOT}/System/Library/Kernels/
	DEPENDS libfirehose_kernel
)

add_custom_target(Darwin)
add_dependencies(Darwin xnu_headers libfirehose_kernel xnu)

# Build kernel extensions
#add_subdirectory(libkmod)
#add_subdirectory(Extensions)

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

add_subdirectory(${ROOT_SOURCE_DIR}/Libraries ${ROOT_BINARY_DIR}/Libraries)