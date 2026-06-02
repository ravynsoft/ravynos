MK_WERROR = no

LIB = dyld
SRCS = ./src/start_glue.s \
	./src/glue.c \
	./dyld3/Tracing.cpp \
	./dyld3/Closure.cpp \
	./src/dyldLock.cpp \
	./src/dyld_stub_binder.s \
	./dyld3/ClosureFileSystemPhysical.cpp \
	./src/dyldLibSystemGlue.c \
	./src/dyldAPIsInLibSystem.cpp \
	./src/threadLocalVariables.c \
	./src/threadLocalHelpers.s \
	./src/dyld_process_info.cpp \
	./src/dyld_process_info_notify.cpp \
	./dyld3/Diagnostics.cpp \
	./dyld3/shared-cache/DyldSharedCache.cpp \
	./dyld3/AllImages.cpp \
	./dyld3/APIs.cpp \
	./dyld3/APIs_macOS.cpp \
	./dyld3/Logging.cpp \
	./dyld3/Loading.cpp \
	./dyld3/MachOFile.cpp \
	./dyld3/MachOLoaded.cpp \
	./dyld3/MachOAnalyzer.cpp \
	./dyld3/PathOverrides.cpp \
	./dyld3/ClosureBuilder.cpp \
	./dyld3/ClosureWriter.cpp \
	./dyld3/libdyldEntryVector.cpp
OBJS = ${SRCS:S/.c$/.o/:S/.cpp$/.o/:S/.s$/.o/}

COMMONFLAGS = -w --target=${CpuArch}-apple-darwin -dylib \
	-mmacos-version-min=${MACOS_VERSION_MIN} \
	--sysroot=${RAVYN_SDKROOT} -DPRIVATE  \
	-DBUILDING_LIBDYLD -DWITHOUT_SANDBOX -DBROKEN_VECTOR_BASE \
	-D__USING_SJLJ_EXCEPTIONS__=0 -D_USE_EXTENDED_LOCALES_ \
	-D__DARWIN_C_LEVEL=__DARWIN_C_FULL -DTARGET_OS_BRIDGE=0 \
	-Wno-nullability-completeness -Wno-incompatible-sysroot \
	-Wno-availability -I${SDK_SOURCE_DIR}/usr/include/c++/v1 \
	-I${.CURDIR}/src -I${.CURDIR}/dyld3/shared-cache \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/bsd -I${.CURDIR}/dyld3 \
	-I${.CURDIR}/../Libsystem/libsystem_c/include \
	-I${.CURDIR}/../Libsystem/libsystem_c/locale \
	-I${.CURDIR}/../Libsystem/libsystem_c/locale/FreeBSD \
	-I${.CURDIR}/../Libsystem/libsystem_c/stdtime \
	-I${.CURDIR}/../Libsystem/libsystem_c/stdtime/FreeBSD \
	-I${.CURDIR}/../Libsystem/private \
	-I${.CURDIR}/../Libsystem/liblaunch -I${.CURDIR}/../openbsm \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/libkern \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/libproc \
	-I${RAVYN_SDKROOT}/System/Library/Frameworks/System.framework/Versions/B/PrivateHeaders \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/EXTERNAL_HEADERS

C_INCS= \
	-include${SDK_SOURCE_DIR}/usr/include/AvailabilityInternal.h \
	-include${SDK_SOURCE_DIR}/usr/include/AvailabilityVersions.h \
	-include${SDK_SOURCE_DIR}/usr/include/AvailabilityMacros.h \
	-include${SDK_SOURCE_DIR}/usr/include/Availability.h \
	-include${SDK_SOURCE_DIR}/usr/include/mach.h \
	-include${ROOT_SOURCE_DIR}/Libraries/Libsystem/libsystem_c/include/xlocale/_stdlib.h

CFLAGS.Loading.o = -fvisibility=hidden -fno-exceptions
CFLAGS.MachOFile.o = -fno-exceptions
CFLAGS.MachOLoaded.o = -fno-exceptions
CFLAGS.MachOAnalyzer.o = -fno-exceptions
CFLAGS.Diagnostics.o = -fno-exceptions
CFLAGS.PathOverrides.o = -fno-exceptions
CFLAGS.Closure.o = -fno-exceptions
CFLAGS.ClosureWriter.o = -fno-exceptions
CFLAGS.ClosureBuilder.o = -fno-exceptions
CFLAGS.ClosureFileSystemPhysical.o = -fno-exceptions
CFLAGS.Tracing.o = -fno-exceptions
CFLAGS.glue.o = -fno-builtin ${C_INCS}
CFLAGS.ImageLoaderMachOCompressed.o = -O3

ACFLAGS = ${COMMONFLAGS}
CFLAGS = ${COMMONFLAGS} ${CFLAGS.${.TARGET:T}}
CXXFLAGS = ${COMMONFLAGS} -std=c++20 ${C_INCS} ${CFLAGS.${.TARGET:T}}
LDFLAGS = --target=${CpuArch}-apple-darwin --sysroot=${RAVYN_SDKROOT} \
	-Wl,-no_inits -Wl,-no_warn_unused_dylibs -fuse-ld=${LD} -shared \
	-umbrella System "-Wl,-unexported_symbol,__ZNSt3*" \
	-mmacos-version-min=${MACOS_VERSION_MIN} \
	-Wl,-install_name,/usr/lib/system/libdyld.dylib \
	-Wl,-flat_namespace -Wl,-undefined,suppress \
	-Wl,-current_version,732.2 \
	-L${RAVYN_SDKROOT}/usr/lib/system -L${RAVYN_SDKROOT}/usr/lib \
	-lsystem_malloc -lsystem_pthread -lsystem_platform -lsystem_kernel \
	-Wl,-upward-lsystem_platform -Wl,-upward-lsystem_malloc \
	-Wl,-upward-lsystem_c -Wl,-upward-lsystem_pthread \
	-Wl,-upward-lxpc \
	-Wl,-upward-lsystem_blocks -Wl,-upward-lsystem_kernel \
	-Wl,-upward-ldispatch \
	-Wl,-upward-lcompiler_rt -lunwind -lc++

all: ${.OBJDIR}/lib${LIB}.dylib

mkdirs: .PHONY
	mkdir -p ${.OBJDIR}/src ${.OBJDIR}/dyld3/shared-cache

${.OBJDIR}/lib${LIB}.dylib: mkdirs ${OBJS}
	${CC} -o ${.TARGET} ${OBJS} ${LDFLAGS}
	mkdir -p ${RAVYN_SDKROOT}/usr/lib/system
	cp -f ${.TARGET} ${RAVYN_SDKROOT}/usr/lib/system/

libdyld: ${.OBJDIR}/lib${LIB}.dylib

${.OBJDIR}/lib${LIB}.a: mkdirs ${OBJS}
	${AR} rcs ${.TARGET} ${OBJS}
	mkdir -p ${RAVYN_SDKROOT}/usr/lib/system
	cp -f ${.TARGET} ${RAVYN_SDKROOT}/usr/lib/system/

SUBDIR = dyld

.include <bsd.subdir.mk>

