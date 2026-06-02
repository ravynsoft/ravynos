CC := ${HOST_CC}
CXX := ${HOST_CXX}

.if "${.MAKE.OS}" == "Linux"
IIG = /bin/true
.endif

SRCS =	kxld.c \
	kxld_array.c \
	kxld_copyright.c \
	kxld_demangle.c \
	kxld_dict.c \
	kxld_kext.c \
	kxld_object.c \
	kxld_reloc.c \
	kxld_sect.c \
	kxld_seg.c \
	kxld_srcversion.c \
	kxld_splitinfolc.c \
	kxld_sym.c \
	kxld_symtab.c \
	kxld_util.c \
	kxld_uuid.c \
	kxld_vtable.c \
	kxld_versionmin.c
OBJS = ${SRCS:%.c=%.o}

CFLAGS = -std=gnu99 -D__DARWIN_BYTE_ORDER=__DARWIN_LITTLE_ENDIAN \
    -D"__unused=__attribute__((unused))" -D__header_always_inline=inline \
    -I${.CURDIR}/../libkern -I${.CURDIR}/../../EXTERNAL_HEADERS \
    -I${.CURDIR}/../../../../Developer/Default.xctoolchain/include \
    -I${.CURDIR}/.. -DPRIVATE -g -include${.CURDIR}/../../bsd/sys/queue.h

all: ${TOOLCHAIN}/usr/lib/libkxld.a

${TOOLCHAIN}/usr/lib/libkxld.a: ${OBJS}
	mkdir -p ${.TARGET:H}
	ar rcs ${.TARGET} ${OBJS}
