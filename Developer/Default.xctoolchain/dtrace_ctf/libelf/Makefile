CC := ${HOST_CC}
AR := ${HOST_AR}
CFLAGS += -I${.CURDIR}/../include -I${.CURDIR}/../include/sys -D__APPLE_CPP__ \
	  -D__LITTLE_ENDIAN__ -Wno-error
SRCS =	begin.c \
	clscook.c \
	clscook_ELF64.c \
	cntl.c \
	cook.c \
	data.c \
	end.c \
	error.c \
	gelf.c \
	getdata.c \
	getehdr.c \
	getident.c \
	getscn.c \
	getshdr.c \
	getshstrndx.c \
	input.c \
	kind.c \
	ndxscn.c \
	nextscn.c \
	strptr.c \
	xlate.c \
	xlate64.c
OBJS = ${SRCS:%.c=%.o}

all: ${TOOLCHAIN}/usr/lib/libelf.a

${TOOLCHAIN}/usr/lib/libelf.a: ${OBJS}
	${HOST_AR} rcs ${.TARGET} ${.ALLSRC}

