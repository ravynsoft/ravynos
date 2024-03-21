.include <src.opts.mk>

LIB=		xkbcommon
PACKAGE=        lib${LIB}
SHLIB_MAJOR=    1

.PATH:          ${.CURDIR}
SRCS=   keysym.c \
        context.c \
        utils.c \
        state.c \
        registry.c \
        keymap.c \
        compose/paths.c \
        compose/table.c \
        compose/parser.c \
        compose/state.c \
        utf8.c \
        context-priv.c \
        keysym-utf.c \
        atom.c \
        util-list.c \
        keymap-priv.c \
        xkbcomp/symbols.c \
        xkbcomp/ast-build.c \
        xkbcomp/action.c \
        xkbcomp/xkbcomp.c \
        xkbcomp/keymap.c \
        xkbcomp/keycodes.c \
        xkbcomp/types.c \
        xkbcomp/keywords.c \
        xkbcomp/rules.c \
        xkbcomp/scanner.c \
        xkbcomp/keymap-dump.c \
        xkbcomp/include.c \
        xkbcomp/expr.c \
        xkbcomp/compat.c \
        xkbcomp/vmod.c \
        x11/state.c \
        x11/util.c \
        x11/keymap.c \
        text.c 
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
