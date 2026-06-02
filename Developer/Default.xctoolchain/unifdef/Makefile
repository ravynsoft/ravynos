CC := ${HOST_CC}
CFLAGS += -std=c99 -Wno-implicit-function-declaration -Wno-int-conversion
SRCS = unifdef.c
OBJS = unifdef.o

.if "${.MAKE.OS}" == "Linux"
BINGRP = 0
.endif

all: ${TOOLS}/unifdef

${TOOLS}/unifdef: ${OBJS}
	${HOST_CC} -o ${.TARGET} ${OBJS}
.if !NO_ROOT
	chown ${BINOWN}:${BINGRP} ${.TARGET}
	chmod ${BINMODE} ${.TARGET}
.endif

.include <bsd.own.mk>
