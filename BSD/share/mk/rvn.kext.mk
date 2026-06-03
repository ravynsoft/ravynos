.include <bsd.suffixes.mk>

_KEXT_FOLDER = ${.OBJDIR}/${KEXT}.kext
_KEXT_LIB = ${_KEXT_FOLDER}/Contents/MacOS/${KEXT}

.if defined(RAVYN_SDKROOT)
SDKROOT = ${RAVYN_SDKROOT}
.endif

.PATH: ${.OBJDIR}
SRCS += kmod_info.c
OBJS = ${SRCS:C/\..*$/.o/}

CFLAGS += --sysroot=${SDKROOT} -DKERNEL -I${SDKROOT}/usr/include \
	-I${SDKROOT}/usr/local/include -I${SDKROOT}/usr/local/include/kernel
CXXFLAGS += -fapple-kext ${CFLAGS}
LDFLAGS += -nostdlib -Wl,-bundle -Wl,-undefined,dynamic_lookup \
	-Wl,-kext -Wl,-segalign,0x1000

.if defined(RPATHS)
.for rpath in ${RPATHS}
LDFLAGS += -Wl,-rpath,${rpath}
.endfor
.endif

.if "${INSTALL_NAME_DIR}" != ""
LDFLAGS += -Wl,-install_name,${INSTALL_NAME_DIR}/${KEXT}
.elsif "${INSTALL_NAME}" != ""
LDFLAGS += -Wl,-install_name,${INSTALL_NAME}
.endif

.if defined(KERNEL_PRIVATE)
CFLAGS += -DKERNEL_PRIVATE
LDFLAGS += -L${SDKROOT}/usr/local/lib/kernel -lkmod
.endif

.if defined(MACOS_VERSION_MIN)
CFLAGS += -mmacos-version-min=${MACOS_VERSION_MIN}
LDFLAGS += -mmacos-version-min=${MACOS_VERSION_MIN}
.endif

.if defined(MAIN_FUNCTION)
MAIN_FUNCTION_DECL = extern kern_return_t ${MAIN_FUNCTION}(kmod_info_t *ki, void *data);
.else
MAIN_FUNCTION = 0
.endif

.if defined(ANTIMAIN_FUNCTION)
ANTIMAIN_FUNCTION_DECL = extern kern_return_t ${ANTIMAIN_FUNCTION}(kmod_info_t *ki, void *data);
.else
ANTIMAIN_FUNCTION = 0
.endif

all: ${_KEXT_LIB}

${_KEXT_FOLDER}:
	mkdir -p ${.TARGET}/Contents/MacOS

${_KEXT_FOLDER}/Contents/Info.plist: ${.CURDIR}/${INFO_PLIST}
	sed -e 's/@BUNDLE_IDENTIFIER@/${BUNDLE_IDENTIFIER}/' \
	    -e 's/@BUNDLE_VERSION@/${BUNDLE_VERSION}/' \
	    ${.CURDIR}/${INFO_PLIST} >${.TARGET}

${_KEXT_LIB}: ${_KEXT_FOLDER} ${_KEXT_FOLDER}/Contents/Info.plist ${OBJS} \
		${.OBJDIR}/kmod_info.c 
	${CXX} -o ${.TARGET} ${OBJS} ${LDFLAGS}

.if defined(KERNEL_PRIVATE)
${_KEXT_LIB}: ${SDKROOT}/usr/local/lib/kernel/libkmod.a
.endif

kmod_info.c: ${.OBJDIR}/kmod_info.c
${.OBJDIR}/kmod_info.c:
	(echo '#include <mach/kmod.h>'; \
	 echo 'extern kern_return_t _start(kmod_info_t *ki, void *data);'; \
	 echo 'extern kern_return_t _stop(kmod_info_t *ki, void *data);'; \
	 echo '${MAIN_FUNCTION_DECL}'; \
	 echo '${ANTIMAIN_FUNCTION_DECL}'; \
	 echo '__attribute__((visibility("default"))) KMOD_EXPLICIT_DECL(${BUNDLE_IDENTIFIER}, "${BUNDLE_VERSION}", _start, _stop)'; \
	 echo '__private_extern__ kmod_start_func_t *_realmain = ${MAIN_FUNCTION};'; \
	 echo '__private_extern__ kmod_stop_func_t *_antimain = ${ANTIMAIN_FUNCTION};'; \
	 echo '__private_extern__ int _kext_apple_cc = __APPLE_CC__;') >${.TARGET}

.include <bsd.lib.mk>
