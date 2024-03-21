PACKAGE=lib${LIB}
.PATH: ${.CURDIR}/pixman

.include <src.opts.mk>

LIB=		pixman-1
MAJOR=          0
MINOR=          43
MICRO=          2
SHLIB_MAJOR=    ${MAJOR}

SRCS=	\
    pixman-linear-gradient.c \
    pixman-matrix.c \
    pixman-edge-accessors.c \
    pixman-combine-float.c \
    pixman-gradient-walker.c \
    pixman-noop.c \
    pixman-filter.c \
    pixman-bits-image.c \
    pixman-radial-gradient.c \
    pixman-fast-path.c \
    pixman-trap.c \
    pixman-access.c \
    pixman-combine32.c \
    pixman-region16.c \
    pixman-region32.c \
    dither/make-blue-noise.c \
    pixman-glyph.c \
    pixman-solid-fill.c \
    pixman-conical-gradient.c \
    pixman-timer.c \
    pixman-utils.c \
    pixman-access-accessors.c \
    pixman-implementation.c \
    pixman-image.c \
    pixman.c \
    pixman-general.c \
    pixman-edge.c \
    pixman-arm.c \
    pixman-mips.c \
    pixman-ppc.c \
    pixman-x86.c \
    pixman-version.h


INCS=	pixman.h pixman-version.h

CFLAGS+= -I${.CURDIR} -I${MAKEOBJDIR}

.if ${MACHINE} == "amd64"
CFLAGS+= \
        -DUSE_X86_MMX \
	-DUSE_SSE2 \
        -DUSE_SSSE3 \
        -msse2 -mssse3 -Winline

SRCS+= \
    pixman-sse2.c \
    pixman-ssse3.c \
    pixman-mmx.c
.endif

.if ${MACHINE} == "arm64"
CFLAGS+= \
	-DUSE_ARM_SIMD \
	-DUSE_ARM_A64_NEON
SRCS+= \
    pixman-arm-neon.c \
    pixman-arm-simd.c
.endif

CFLAGS+= \
	-DHAVE_ALARM \
        -DHAVE_BUILTIN_CLZ \
        -DHAVE_FEDIVBYZERO \
        -DHAVE_FEENABLEEXCEPT \
        -DHAVE_FENV_H \
        -DHAVE_GCC_VECTOR_EXTENSIONS \
        -DHAVE_GETPAGESIZE \
        -DHAVE_GETTIMEOFDAY \
        -DHAVE_MMAP \
        -DHAVE_MPROTECT \
        -DHAVE_POSIX_MEMALIGN \
        -DHAVE_PTHREADS \
        -DHAVE_SIGACTION \
        -DHAVE_SYS_MMAN_H \
        -DHAVE_UNISTD_H \
        -DUSE_GCC_INLINE_ASM \
        -DTOOLCHAIN_SUPPORTS_ATTRIBUTE_CONSTRUCTOR \
        -DTLS=__thread \
        -DPACKAGE=pixman-1 \
	-DASM_LEADING_UNDERSCORE \
        -DUSE_OPENMP \
	-DHAVE_LIBPNG \
	-DHAVE_PTHREADS \
	-DHAVE_FLOAT128 \
	-DPIXMAN_TIMERS \
	-DSIZEOF_LONG=8

WARNS?=	2
CFLAGS+=        -Wno-error=format -Wno-error=expansion-to-defined

pixman-version.h: pixman/pixman-version.h.in
	sed -e "s/@PIXMAN_VERSION_MAJOR@/${MAJOR}/" \
            -e "s/@PIXMAN_VERSION_MINOR@/${MINOR}/" \
            -e "s/@PIXMAN_VERSION_MICRO@/${MICRO}/" \
            ${.CURDIR}/pixman/pixman-version.h.in > pixman-version.h

.include <bsd.lib.mk>
