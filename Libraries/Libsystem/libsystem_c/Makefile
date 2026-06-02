##----------------------------------------------------------------------------
#                         RAVYNOS LIBSYSTEM_C BUILD
#
# The C library (libsystem_c or libc - part of libSystem) has a very
# complex build process. This make script has been puzzled out from the Xcode
# project files and link errors as the OS was built up.
#
# How this should work:
#  - Base, Platform, FreeBSD, NetBSD, TRE, FortifySource are built as
#    `ar` archives for a standard config. These are the big archives with
#    most of the libc code.
#  - vCancelable, vDarwinExtsn, vDarwinExtsnCancelable, vPre1050, vLegacy,
#    vInode32 are built as `ar` archives with differing compiler flags
#    (most notably the -DVARIANT_xxx flag). These are small archives with
#    only a few .c files each. They provide variants of certain functions,
#    identified by a suffix like `$NOCANCEL` or `$DARWIN_EXTSN`.
#  - libc_static.a is created from the static variant archives and a few
#    additional source files. This goes in /usr/local/lib/system/libc.a
#    and a symlink is created from /usr/local/lib/loaderd/libc.a.
# - libc_dyld.a is created and installed to /usr/local/lib/dyld/libc.a
# - Finally, certain static and variant archives are combined into
#   libsystem_c.dylib. This ends up in /usr/lib/libsystem and is part of
#   libSystem.dylib.
#
# !! Note that the two libc.a's above are completely different archives !!
#-----------------------------------------------------------------------------

# FIXME: This badly needs to be refactored now that we understand the layers

#-----------------------------------------------------------------------------
#                             LIBSYSTEM_C
#-----------------------------------------------------------------------------

MK_WERROR = no

LIB = system_c

SLF = /System/Library/Frameworks

COMMON_DEFS = -D__LIBC__ -D_FORTIFY_SOURCE=0 -DPR_13085474_CHECK \
	-D__DARWIN_UNIX03 -D__DARWIN_64_BIT_INO_T -D__DARWIN_NON_CANCELABLE \
	-D__DARWIN_VERS_1050

CFLAGS = --target=${CpuArch}-apple-darwin --sysroot=${RAVYN_SDKROOT} \
	 ${COMMON_DEFS} \
	-mmacos-version-min=${MACOS_VERSION_MIN} \
	-std=gnu11 -fdollars-in-identifiers -fno-common -fverbose-asm -w \
	-Wno-error=shorten-64-to-32 -Wno-int-conversion -Wno-implicit-int \
	-Wno-error=incompatible-pointer-types-discards-qualifiers \
	-Wno-nullability-completeness -Wno-implicit-function-declaration \
	-isystem${RAVYN_SDKROOT}/usr/include/c++/v1 \
	-isystem${ROOT_BINARY_DIR}/${CpuArch} \
	-isystem${RAVYN_SDKROOT}/usr/local/include \
	-isystem${RAVYN_SDKROOT}/usr/include \
	-I${.CURDIR}/fbsdcompat -I${.CURDIR}/include -I${.CURDIR}/gen \
	-I${.CURDIR}/locale -I${.CURDIR}/locale/FreeBSD \
	-I${.CURDIR}/stdtime/FreeBSD -I${.CURDIR}/darwin \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/spawn
.export CFLAGS

SRCS = darwin/forceLibcToBuild.c darwin/compatibility_hacks.c

lib${LIB}.a: ${ROOT_BINARY_DIR}/${CpuArch}/libc-features.h

# Generate features.h and patch headers for variants
${ROOT_BINARY_DIR}/${CpuArch}/libc-features.h:
	ARCHS="${CpuArch} " VARIANT_PLATFORM_NAME=macosx \
	DERIVED_FILES_DIR=${ROOT_BINARY_DIR} SRCROOT=${.CURDIR} \
		perl ${.CURDIR}/xcodescripts/generate_features.pl
	perl ${.CURDIR}/xcodescripts/patch_headers_variants.pl \
		${RAVYN_SDKROOT}${SLF}/System.framework/Versions/B/PrivateHeaders \
		${ROOT_BINARY_DIR}/${CpuArch}/System.framework/Versions/B

#-----------------------------------------------------------------------------
#                           ARCHIVE TARGETS
#-----------------------------------------------------------------------------

SUBDIR = libBase libPlatform libFreeBSD libNetBSD libTRE libFortifySource

#-------------------------------------------------------------------------
#                             VARIANT TARGETS
#-------------------------------------------------------------------------

SUBDIR += vCancelable vDarwinExtsn vDarwinExtsnCancelable vPre1050 vLegacy
SUBDIR += vInode32

#-------------------------------------------------------------------------
#                          STATIC LIB TARGETS
#-------------------------------------------------------------------------

SUBDIR += libc_dyld libc_static


libBase libPlatform libFreeBSD libNetBSD libFortifySource libTRE \
libc_dyld libc_static vCancelable vDarwinExtsn vDarwinExtsnCancelable \
vPre1050 vLegacy vInode32: ${ROOT_BINARY_DIR}/${CpuArch}/libc-features.h

#-----------------------------------------------------------------------------
#                            LIBSYSTEM_C DYLIB
#-----------------------------------------------------------------------------

all: lib${LIB}.dylib

lib${LIB}.a: generateLinkLists libBase libPlatform libFreeBSD libNetBSD \
	libFortifySource libTRE vCancelable vDarwinExtsn vPre1050 vInode32 \
	libc_static libc_dyld

ARCHIVES = Base NetBSD FortifySource FreeBSD TRE vCancelable 
ARCHIVES += vDarwinExtsn vInode32 vPre1050
generateLinkLists:
	NM=${NM} BUILT_PRODUCTS_DIR=${.OBJDIR} \
	DERIVED_FILES_DIR=${.OBJDIR} \
	BUILD_ARCHIVES="${ARCHIVES}" \
	bash ${.CURDIR}/xcodescripts/build_linklists.sh

LDFLAGS = -mmacos-version-min=${MACOS_VERSION_MIN} \
	-Wl,-syslibroot,${RAVYN_SDKROOT}/usr/lib \
	-Wl,-current_version,1353.0.10 -Wl,-compatibility_version,1.0.0 \
	-Wl,-install_name,/usr/lib/system/libsystem_c.dylib -umbrella System \
	-L${RAVYN_SDKROOT}/usr/lib/system -L${RAVYN_SDKROOT}/usr/lib \
	-Wl,-search_paths_first -Wl,-all_load \
	${.OBJDIR}/libPlatform/libPlatform.a \
	${.OBJDIR}/libBase/libBase.a ${.OBJDIR}/libNetBSD/libNetBSD.a \
	${.OBJDIR}/libFortifySource/libFortifySource.a \
	${.OBJDIR}/libFreeBSD/libFreeBSD.a ${.OBJDIR}/libTRE/libTRE.a \
	${.OBJDIR}/vCancelable/libvCancelable.a \
	${.OBJDIR}/vDarwinExtsn/libvDarwinExtsn.a \
	${.OBJDIR}/vInode32/libvInode32.a ${.OBJDIR}/vPre1050/libvPre1050.a \
	-Wl,-upward-lsystem_blocks -Wl,-upward-lxpc -Wl,-upward-ldispatch \
	-Wl,-upward-lsystem_asl -Wl,-upward-lsystem_info \
	-Wl,-upward-lsystem_notify -Wl,-upward-lsystem_trace \
	-Wl,-interposable_list,${.OBJDIR}/interposable.list \
	-Wl,-unexported_symbols_list,${.OBJDIR}/unexport.list \
	-Wl,-alias_list,${.CURDIR}/xcodescripts/alias.list \
	-Wl,-order_file,${.CURDIR}/xcodescripts/Libc.order \
	-lsystem_malloc -lsystem_platform -lsystem_pthread -lsystem_kernel \
	-ldyld -llaunch -lsystem_m -lCrashReporterClient
	#-Wl,-upward-lcorecrypto -Wl,-upward-lmacho

# FIXME: should also build `debug` variant

lib${LIB}.dylib: lib${LIB}.a
	${CC} -shared -dylib -fuse-ld=${LD} -o ${.TARGET} ${LDFLAGS}
	mkdir -p ${SYSROOT_DIR}/usr/lib/system ${RAVYN_SDKROOT}/usr/lib/system
	cp -f ${.TARGET} ${SYSROOT_DIR}/usr/lib/system/
	cp -f ${.TARGET} ${RAVYN_SDKROOT}/usr/lib/system/

.include <bsd.lib.mk>
.include <bsd.subdir.mk>

