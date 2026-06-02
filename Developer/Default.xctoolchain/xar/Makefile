CC := ${HOST_CC}
LIBXAR_SRCS =	lib/archive.c \
		lib/arcmod.c \
		lib/b64.c \
		lib/bzxar.c \
		lib/darwinattr.c \
		lib/data.c \
		lib/ea.c \
		lib/err.c \
		lib/ext2.c \
		lib/fbsdattr.c \
		lib/filetree.c \
		lib/hash.c \
		lib/io.c \
		lib/linuxattr.c \
		lib/lzmaxar.c \
		lib/macho.c \
		lib/script.c \
		lib/signature.c \
		lib/stat.c \
		lib/subdoc.c \
		lib/util.c \
		lib/zxar.c
LIBXAR_OBJS = ${LIBXAR_SRCS:%.c=%.o}
CFLAGS += -I${.CURDIR}/include -I/usr/include/libxml2 -w -D__APPLE_CPP__ \
	  -D__LITTLE_ENDIAN__

XAR_SRCS = src/xar.c
XAR_OBJS = ${XAR_SRCS:%.c=%.o}

.if "${.MAKE.OS}" == "Linux"
LDFLAGS += -lacl -lbsd -L/usr/lib/x86_64-linux-gnu
.endif
LDFLAGS += -lssl -lcrypto -lxml2 -lz
LIBS = ${TOOLCHAIN}/usr/lib/libstuff.a ${TOOLCHAIN}/usr/lib/libxar.a

all: ${.OBJDIR}/lib ${.OBJDIR}/src ${TOOLS}/xar

${.OBJDIR}/lib ${.OBJDIR}/src:
	mkdir -p ${.TARGET}

${TOOLCHAIN}/usr/lib/libxar.a: ${LIBXAR_OBJS}
	${HOST_AR} rcs ${.TARGET} ${.ALLSRC}

${TOOLS}/xar: ${XAR_OBJS} ${LIBS}
	${HOST_CC} -o ${.TARGET} ${.ALLSRC} ${LDFLAGS}

