MK_WERROR = no

.PATH: ${.CURDIR}/..

LIB = dyld
SRCS = src/dyldStartup.s \
	src/dyldInitialization.cpp \
	src/dyld2.cpp \
	dyld3/ClosureFileSystemPhysical.cpp \
	src/dyldAPIs.cpp \
	src/dyldExceptions.c \
	src/dyld_debugger.cpp \
	dyld3/Tracing.cpp \
	src/glue.c \
	src/ImageLoaderMegaDylib.cpp \
	src/ImageLoader.cpp \
	src/ImageLoaderMachO.cpp \
	src/stub_binding_helper.s \
	src/dyldNew.cpp \
	src/ImageLoaderMachOClassic.cpp \
	src/ImageLoaderMachOCompressed.cpp \
	dyld3/Loading.cpp \
	dyld3/Diagnostics.cpp \
	dyld3/SharedCacheRuntime.cpp \
	dyld3/shared-cache/DyldSharedCache.cpp \
	dyld3/shared-cache/FileUtils.cpp \
	dyld3/MachOLoaded.cpp \
	dyld3/MachOFile.cpp \
	dyld3/MachOAnalyzer.cpp \
	dyld3/Closure.cpp \
	dyld3/ClosureWriter.cpp \
	dyld3/BootArgs.cpp \
	dyld3/ClosureBuilder.cpp \
	dyld3/PathOverrides.cpp \
	src/mh_dylib_header_alias.s \
	src/missing_symbol_stubs.c \
	src/cc_stubs.c
OBJS = ${SRCS:S/.c$/.o/:S/.cpp$/.o/:S/.s$/.o/:S/.S$/.o/}
MAN = ../doc/man/man1/dyld.1 \
	../doc/man/man1/dyld_usage.1 \
	../doc/man/man1/update_dyld_shared_cache.1


COMMONFLAGS = --target=${CpuArch}-apple-darwin \
		--sysroot=${RAVYN_SDKROOT} \
		-mmacos-version-min=${MACOS_VERSION_MIN} \
		-DBUILDING_DYLD -DDYLD_VERSION=732.1 -DWITHOUT_SANDBOX \
		-D__USING_SJLJ_EXCEPTIONS__=0 -DPRIVATE \
		-D__DARWIN_C_LEVEL=__DARWIN_C_FULL \
		-DTARGET_OS_BRIDGE=0 -DBROKEN_VECTOR_BASE \
		-D'VIS_HIDDEN=__attribute__((visibility("hidden")))' \
		-D_LIBCPP_BEGIN_NAMESPACE_STD="" \
		-D_LIBCPP_END_NAMESPACE_STD="" \
		-Wno-nullability-completeness -Wno-incompatible-sysroot \
		-Wno-availability -fno-stack-check \
		-isystem${RAVYN_SDKROOT}/usr/include/c++/v1 \
		-isystem${TOOLS}/../lib/clang/17 \
		-I${.OBJDIR}/.. \
		-I${ROOT_BINARY_DIR}/${CpuArch} \
		-I${ROOT_SOURCE_DIR}/Libraries/Libsystem/private \
		-I${.CURDIR}/../include \
		-I${.CURDIR}/../dyld3 \
		-I${.CURDIR}/../dyld3/shared-cache \
		-I${.CURDIR}/../src \
		-I${.CURDIR}/.. \
		-I${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/libproc \
		-I${ROOT_SOURCE_DIR}/Kernel/Extensions/corecrypto/include \
		-I${RAVYN_SDKROOT}/usr/local/include/kernel \
		-isystem${RAVYN_SDKROOT}/usr/include \
		-iframework${RAVYN_SDKROOT}/System/Library/Frameworks

CXXFLAGS = ${COMMONFLAGS} -std=c++17 -fno-threadsafe-statics \
		${CFLAGS.${.TARGET:T}}
CFLAGS = ${COMMONFLAGS} ${CFLAGS.${.TARGET:T}}
ACFLAGS = ${COMMONFLAGS}

.for file in Diagnostics.pieo \
	    dyldInitialization.pieo \
	    ClosureFileSystemPhysical.pieo \
	    dyld_debugger.pieo \
	    Tracing.pieo \
	    dyldNew.pieo \
	    Loading.pieo \
	    SharedCacheRuntime.pieo \
	    shared-cache/DyldSharedCache.pieo \
	    shared-cache/FileUtils.pieo \
	    MachOLoaded.pieo \
	    MachOFile.pieo \
	    MachOAnalyzer.pieo \
	    Closure.pieo \
	    ClosureWriter.pieo \
	    BootArgs.pieo \
	    ClosureBuilder.pieo \
	    PathOverrides.pieo
CFLAGS.${file} = -fno-exceptions
.endfor

LDFLAGS = -Wl,-static -Wl,-dylinker -Wl,-fixup_chains -Wl,-data_const \
	-Wl,-pagezero_size,0 -Wl,-image_base,0 -nodefaultlibs \
	-Wl,-dylinker_install_name,/usr/lib/dyld -Wl,-dead_strip \
	-Wl,-exported_symbol,__dyld_start  -e __dyld_start \
	-Wl,-exported_symbols_list,${.CURDIR}/../src/dyld.exp \
	-Wl,-section_order,__DATA,__all_image_info:__nl_symbol_ptr:__got:__auth_ptr:__const:__crash_info:__data:__bss:__common \
	-mmacos-version-min=${MACOS_VERSION_MIN} \
	-L${RAVYN_SDKROOT}/usr/local/lib/dyld -lc -lpthread -lplatform \
	-L${RAVYN_SDKROOT}/usr/lib/system -lCrashReporterClient \
	-L${RAVYN_SDKROOT}/usr/lib -lunwind -lc++ -lc++abi \
	-L${RAVYN_SDKROOT}/usr/local/lib/kernel -lkernel

.for file in ${SRCS}
basefile = ${file:S/.cpp$/.o/:S/.c/.o/:S/.s/.o/}
${.OBJDIR}/${basefile}: ${.CURDIR}/${file}
	${CC} ${COMMONFLAGS} -o ${.TARGET} \
.if "${file:E}" == ".c"
		${CFLAGS} \
.else if "${file:E}" == ".cpp"
		${CXXFLAGS} \
.else
		-x assembler-with-cpp \
.endif
		-c ${.IMPSRC}
.endfor

all: ${LIB}

${LIB}: ${.OBJDIR}/${LIB}

${.OBJDIR}/${LIB}: ${OBJS}
	${CC} -fuse-ld=${LD} -o ${.TARGET} ${OBJS:O:u} ${LDFLAGS}
	mkdir -p ${RAVYN_SDKROOT}/usr/lib ${ROOT_BINARY_DIR}/sysroot/usr/lib
	cp -f ${.TARGET} ${RAVYN_SDKROOT}/usr/lib/
	cp -f ${.TARGET} ${SYSROOT_DIR}/usr/lib/
	mkdir -p ${RAVYN_SDKROOT}${MANDIR}/man1 ${SYSROOT_DIR}${MANDIR}/man1
.for man in ${MAN:T}
	cp -f ${.OBJDIR}/${man}.gz ${RAVYN_SDKROOT}${MANDIR}/man1/
	cp -f ${.OBJDIR}/${man}.gz ${SYSROOT_DIR}${MANDIR}/man1/
.endfor

.include <bsd.lib.mk>
