.include <src.opts.mk>
.include "Makefile.sources"
.include "amdgpu/Makefile.sources"
.include "radeon/Makefile.sources"
.include "intel/Makefile.sources"
.include "nouveau/Makefile.sources"

.PATH:          ${.CURDIR}
.PATH:          ${.CURDIR}/amdgpu
.PATH:          ${.CURDIR}/radeon
.PATH:          ${.CURDIR}/nouveau
.PATH:          ${.CURDIR}/intel

LIB=		drm
PACKAGE=        lib${LIB}
SHLIB_MAJOR=    2

SRCS=           ${LIBDRM_FILES} ${LIBDRM_NOUVEAU_FILES} ${LIBDRM_AMDGPU_FILES}
SRCS+=          ${LIBDRM_RADEON_FILES}

.if ${MACHINE} == "amd64" || ${MACHINE} == "i386"
SRCS+=          ${LIBDRM_INTEL_FILES}
.endif

INCSDIR=        ${INCLUDEDIR}/${PACKAGE}
INCS=	        
CFLAGS+=        -I${.CURDIR}/include/drm -I${.CURDIR} \
                -I${MAKEOBJDIR} -I${.CURDIR}/radeon \
                -I${.CURDIR}/../libpciaccess/include -DUDEV \
                -DHAVE_LIBDRM_ATOMIC_PRIMITIVES \
                -DHAVE_SYS_SYSCTL_H \
                -DHAVE_OPEN_MEMSTREAM \
                -DHAVE_VISIBILITY \
                -DHAVE_RADEON \
                -DHAVE_VMWGFX \
                -DHAVE_VC4 \
                -D_GNU_SOURCE \
                -DAMDGPU_ASIC_ID_TABLE=\"${.CURDIR}/data/amdgpu.ids\" \
                -Wno-error=macro-redefined \
                -Wno-error=unused-but-set-variable \
                -Wno-error=unused-variable \
                -Wno-error=gnu-variable-sized-type-not-at-end \
                -Wno-error=\#warnings

.if ${MACHINE} == "amd64" || ${MACHINE} == "i386"
CFLAGS+=        -DHAVE_INTEL
.endif

LDFLAGS+=       -L${MAKEOBJDIR}/../libpciaccess -lpciaccess -Wl,-R\$$ORIGIN

# Newly-built ar might need libmd and liblzma from the updated base
# We also need the python libs we just built
LD_LIBRARY_PATH=	${OBJTOP}/tmp/lib:${OBJTOP}/tmp/usr/lib
.export LD_LIBRARY_PATH

WARNS?=	2

config.h: .PHONY
	echo >${.TARGET}

generated_static_table_fourcc.h:: 
	${OBJTOP}/tmp/usr/bin/python3 \
	${.CURDIR}/gen_table_fourcc.py \
        ${.CURDIR}/include/drm/drm_fourcc.h $@

beforebuild: obj

.include <bsd.lib.mk>

${OBJS}: generated_static_table_fourcc.h
