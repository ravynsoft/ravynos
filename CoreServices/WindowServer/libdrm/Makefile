.include <src.opts.mk>
.include "Makefile.sources"

LIB=		drm
PACKAGE=        lib${LIB}
SHLIB_MAJOR=    2

.PATH:          ${.CURDIR}
SRCS=           ${LIBDRM_FILES}
INCSDIR=        ${INCLUDEDIR}/${PACKAGE}
INCS=	        
CFLAGS+=        -I${.CURDIR}/include/drm \
                -I${MAKEOBJDIR} \
                -DUDEV \
                -DHAVE_LIBDRM_ATOMIC_PRIMITIVES \
                -DHAVE_LIB_ATOMIC_OPS \
                -DHAVE_SYS_SYSCTL_H \
                -DHAVE_OPEN_MEMSTREAM \
                -DHAVE_VISIBILITY \
                -DHAVE_INTEL \
                -DHAVE_RADEON \
                -DHAVE_VMWGFX \
                -DHAVE_VC4 \
                -D_GNU_SOURCE \
                -Wno-error=macro-redefined \
                -Wno-error=unused-but-set-variable \
                -Wno-error=unused-variable \
                -Wno-error=\#warnings
WARNS?=	2

config.h: .PHONY
	echo >${.TARGET}

generated_static_table_fourcc.h:: 
	python3 ${.CURDIR}/gen_table_fourcc.py \
        ${.CURDIR}/include/drm/drm_fourcc.h $@

.include <bsd.lib.mk>

${OBJS}: generated_static_table_fourcc.h
