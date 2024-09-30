.include <src.opts.mk>

LIB=		xkbcommon
PACKAGE=        lib${LIB}
SHLIB_MAJOR=    1

.PATH:          ${.CURDIR} ${MAKEOBJDIR}

SRCS=   src/keysym.c \
        src/context.c \
        src/utils.c \
        src/state.c \
        src/registry.c \
        src/keymap.c \
        src/compose/paths.c \
        src/compose/table.c \
        src/compose/parser.c \
        src/compose/state.c \
        src/utf8.c \
        src/context-priv.c \
        src/keysym-utf.c \
        src/atom.c \
        src/util-list.c \
        src/keymap-priv.c \
	src/xkbcomp/parser.c \
        src/xkbcomp/symbols.c \
        src/xkbcomp/ast-build.c \
        src/xkbcomp/action.c \
        src/xkbcomp/xkbcomp.c \
        src/xkbcomp/keymap.c \
        src/xkbcomp/keycodes.c \
        src/xkbcomp/types.c \
        src/xkbcomp/keywords.c \
        src/xkbcomp/rules.c \
        src/xkbcomp/scanner.c \
        src/xkbcomp/keymap-dump.c \
        src/xkbcomp/include.c \
        src/xkbcomp/expr.c \
        src/xkbcomp/compat.c \
        src/xkbcomp/vmod.c \
        src/text.c 
INCSDIR=        ${INCLUDEDIR}/${PACKAGE}
INCS=	        
CFLAGS+=        -I${.CURDIR}/include/drm \
		-I${.CURDIR}/include -I${.CURDIR}/src \
                -I${SRCTOP}/lib/libxml2/include \
		-I${OBJTOP}/tmp/usr/include/libxml2 \
		-I${OBJTOP}/lib/libxml2 -I${MAKEOBJDIR} \
                -DUDEV -DHAVE_STRNDUP -DHAVE_UNISTD_H \
                -DHAVE_LIBDRM_ATOMIC_PRIMITIVES \
                -DHAVE_LIB_ATOMIC_OPS -DHAVE_ASPRINTF \
                -DHAVE_SYS_SYSCTL_H \
                -DHAVE_OPEN_MEMSTREAM \
                -DHAVE_VISIBILITY \
                -DHAVE_INTEL -DHAVE_RADEON \
                -DHAVE_VMWGFX -DHAVE_VC4 \
                -D_GNU_SOURCE \
		-DDFLT_XKB_CONFIG_ROOT=\"/System/Library/KeyboardLayout\" \
		-DDFLT_XKB_CONFIG_EXTRA_PATH=\"/Library/KeyboardLayout\" \
		-DXLOCALEDIR=\"/System/Library/Locale\" \
		-DDEFAULT_XKB_RULES=\"evdev\" \
		-DDEFAULT_XKB_MODEL=\"pc105\" \
		-DDEFAULT_XKB_LAYOUT=\"us\" \
		-DDEFAULT_XKB_VARIANT=NULL \
		-DDEFAULT_XKB_OPTIONS=NULL \
                -Wno-error=macro-redefined \
                -Wno-error=unused-but-set-variable \
                -Wno-error=unused-variable \
                -Wno-error=\#warnings
CFLAGS.parser.c+= -I${MAKEOBJDIR}/src/xkbcomp
CFLAGS.keywords.c+= -I${MAKEOBJDIR}/src/xkbcomp
CFLAGS.scanner.c+= -I${MAKEOBJDIR}/src/xkbcomp
WARNS?=	2

config.h: .PHONY
	mkdir -p ${MAKEOBJDIR}/src/compose
	echo >${MAKEOBJDIR}/${.TARGET}

.SUFFIXES: .y
.y.c:
	mkdir -p ${MAKEOBJDIR}/src/xkbcomp
	bison -o${MAKEOBJDIR}/${.TARGET} -p_xkbcommon_ \
		-H${MAKEOBJDIR}/${.TARGET:R}.h ${.IMPSRC}

src/xkbcomp/parser.c: src/xkbcomp/parser.y

.include <bsd.lib.mk>

${OBJS}: obj config.h src/xkbcomp/parser.c
