SRCS = decomp.c lzvn_decode.c
OBJS = ${SRCS:%.c=%.o}

all: ${TOOLS}/lzvndecode

${TOOLS}/lzvndecode: ${OBJS}
	${HOST_CC} -o ${.TARGET} ${OBJS}
