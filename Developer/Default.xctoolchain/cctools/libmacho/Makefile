CC := ${HOST_CC}
SRCS = arch.c getsecbyname.c getsegbyname.c
OBJS = ${SRCS:%.c=%.o}
CFLAGS += -I${DEVEL}/Default.xctoolchain/dtrace_ctf/include

all: ${TOOLCHAIN}/usr/lib/libmacho.a

${TOOLCHAIN}/usr/lib/libmacho.a: ${OBJS}
	${HOST_AR} rcs ${.TARGET} ${OBJS}
