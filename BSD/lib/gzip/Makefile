MK_WERROR = no
SUBDIR =

LIB = gzip
MAJOR = 1
MINOR = 3
PATCH = 12
SRCS = src/lib/creat-safer.c \
    src/lib/dup-safer.c \
    src/lib/error.c \
    src/lib/exitfail.c \
    src/lib/fd-safer.c \
    src/lib/getopt.c \
    src/lib/getopt1.c \
    src/lib/open-safer.c \
    src/lib/pipe-safer.c \
    src/lib/xalloc-die.c \
    src/lib/xmalloc.c \
    src/lib/yesno.c

CFLAGS += --sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN} \
        -DGNU_STANDARD=0 -DNO_UTIME -mdynamic-no-pic -I${.CURDIR}/src/lib \
        -Wno-nullability-completeness -D__GETOPT_PREFIX="" \
        -D__getopt_argv_const="const"
LDFLAGS += --sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN} \
        -fuse-ld=${LD}

PROG_CFLAGS = --sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN} \
        -DS_IRWXUGO=ACCESSPERMS -DNO_UTIME -DO_BINARY=0 -I${.CURDIR}/src/lib
PROG_LDFLAGS = ${LDFLAGS} -L${SYSROOT_DIR}/usr/lib -lgzip
PROG_SRCS = src/bits.c \
        src/crypt.c \
        src/deflate.c \
        src/gzip.c \
        src/inflate.c \
        src/lzw.c \
        src/trees.c \
        src/unlzh.c \
        src/unlzw.c \
        src/unpack.c \
        src/unzip.c \
        src/util.c \
        src/zip.c
PROG_MANS = src/gunzip.1 \
        src/gzexe.1 \
        src/gzip.1 \
        src/zcat.1 \
        src/zcmp.1 \
        src/zdiff.1 \
        src/zforce.1 \
        src/zgrep.1 \
        src/zless.1 \
        src/zmore.1 \
        src/znew.1 \

VERSION = ${MAJOR}.${MINOR}.${PATCH}
MAJMIN = ${MAJOR}.${MINOR}

all: lib${LIB}.dylib zcat

zcat: gzip
	mkdir -p ${SYSROOT_DIR}/usr/share/man/man1
	sed -e 's/BINDIR/\/usr\/bin/' <${.CURDIR}/src/zcat.in >${.TARGET}
	cp -f ${.CURDIR}/src/zcat.1 ${SYSROOT_DIR}/usr/share/man/man1/
	rm -f ${SYSROOT_DIR}/usr/bin/uncompress
	ln -f ${SYSROOT_DIR}/bin/gzip ${SYSROOT_DIR}/bin/zcat
	ln -f ${SYSROOT_DIR}/bin/gzip ${SYSROOT_DIR}/bin/gzcat
	ln -f ${SYSROOT_DIR}/usr/share/man/man1/zcat.1 \
          ${SYSROOT_DIR}/usr/share/man/man1/gzcat.1

gzip: ${PROG_SRCS}
.for src in ${PROG_SRCS}
	${CC} -o ${src:%.c=%.o} -c ${.CURDIR}/${src} ${PROG_CFLAGS}
.endfor
	${CC} -o ${.TARGET} ${PROG_SRCS:%.c=%.o} ${PROG_LDFLAGS}
	mkdir -p ${SYSROOT_DIR}/bin
	cp -f ${.TARGET} ${SYSROOT_DIR}/bin/${.TARGET}

lib${LIB}.dylib: lib${LIB}.a
	${CC} ${LDFLAGS} -shared -o ${.TARGET} -Wl,-force_load,lib${LIB}.a \
	  -Wl,-undefined,suppress -Wl,-flat_namespace \
	  -Wl,-install_name,/usr/lib/${.TARGET} -Wl,-current_version,${VERSION}
	cp -f ${.TARGET} ${SYSROOT_DIR}/usr/lib/

.include <bsd.lib.mk>
