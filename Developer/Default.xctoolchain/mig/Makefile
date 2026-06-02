CC := ${HOST_CC}
SRCS =	error.c \
	global.c \
	header.c \
	mig.c \
	routine.c \
	server.c \
	statement.c \
	string.c \
	type.c \
	user.c \
	utils.c \
	${.OBJDIR}/y.tab.c \
	${.OBJDIR}/lexxer.yy.c
OBJS = ${SRCS:%.c=%.o}
CFLAGS += -DMIG_VERSION=\"\" -D__APPLE_CPP__ -D__LITTLE_ENDIAN__ \
	  -I${.CURDIR} -I${.OBJDIR}
.if "${.MAKE.OS}" == "Linux"
CFLAGS += -I${.CURDIR}/../cctools/include/foreign \
	  -I${.CURDIR}/../cctools/include \
	  -I${DEVEL}/Default.xctoolchain/include \
	  -I${ROOT_SOURCE_DIR}/Kernel/xnu/EXTERNAL_HEADERS \
	  -w -std=c99 -Wno-implicit-function-declaration
.endif

all: ${TOOLS}/mig ${TOOLCHAIN}/usr/libexec/migcom

.if "${.MAKE.OS}" == "Linux"
BINGRP = 0
.endif

${TOOLS}/mig: ${.CURDIR}/mig.sh
	cp -f ${.ALLSRC} ${.TARGET}
.if !NO_ROOT
	chown ${BINOWN}:${BINGRP} ${.TARGET}
	chmod ${BINMODE} ${.TARGET}
.endif

${.OBJDIR}/y.tab.c ${.OBJDIR}/y.tab.h: ${.CURDIR}/parser.y
	${YACC} -d -b "${.OBJDIR}/y" ${.ALLSRC}

${.OBJDIR}/lexxer.yy.c: ${.CURDIR}/lexxer.l ${.OBJDIR}/y.tab.h
	${LEX} -o ${.TARGET} ${.CURDIR}/lexxer.l

${TOOLCHAIN}/usr/libexec/migcom: ${OBJS}
	${HOST_CC} -o ${.TARGET} ${OBJS} ${LDFLAGS}

.include <bsd.own.mk>

