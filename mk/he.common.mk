.SUFFIXES: .out .o .bc .ll .po .pico .nossppico .pieo .S .asm .s .c .cc .cpp .cxx .C .f .y .l .ln .m
OBJC_FLAG+= -fobjc-nonfragile-abi -fobjc-runtime=gnustep-2.0

.m.o:
	${CC} ${OBJC_FLAG} ${FMWK_FLAG} ${PO_FLAG} ${STATIC_CFLAGS} ${PO_CFLAGS} -c ${.IMPSRC} -o ${.TARGET}
	${CTFCONVERT_CMD}

.m.po:
	${CC} ${OBJC_FLAG} ${FMWK_FLAG} ${PO_FLAG} ${STATIC_CFLAGS} ${PO_CFLAGS} -c ${.IMPSRC} -o ${.TARGET}
	${CTFCONVERT_CMD}

.m.pico:
	${CC} ${OBJC_FLAG} ${FMWK_FLAG} ${PICFLAG} -DPIC ${SHARED_CFLAGS} ${CFLAGS} -c ${.IMPSRC} -o ${.TARGET}
	${CTFCONVERT_CMD}

