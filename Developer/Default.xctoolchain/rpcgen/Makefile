CC := ${HOST_CC}
SRCS =  rpc_clntout.c rpc_cout.c rpc_hout.c rpc_main.c \
	rpc_parse.c rpc_sample.c rpc_scan.c rpc_svcout.c rpc_tblout.c \
	rpc_util.c
OBJS = ${SRCS:%.c=%.o}
CFLAGS += -D__dead2="" -D__unused="" -I${.CURDIR}/../include \
	  -D__APPLE_CPP__ -D__LITTLE_ENDIAN__

.if "${.MAKE.OS}" == "Linux"
## Need libbsd for strlcat, strlcpy, etc.
LDFLAGS += -lbsd
.endif

all: ${TOOLS}/rpcgen

${TOOLS}/rpcgen: ${OBJS}
	${CC} -o ${.TARGET} ${OBJS} ${LDFLAGS}
