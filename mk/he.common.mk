.SUFFIXES: .out .o .bc .ll .po .pico .nossppico .pieo .S .asm .s .c .cc .cpp .cxx .C .f .y .l .ln

.m.po:
	${CC} ${PO_FLAG} ${STATIC_CFLAGS} ${PO_CFLAGS} -c ${.IMPSRC} -o ${.TARGET}
	${CTFCONVERT_CMD}

.m.pico:
	${CC} ${PICFLAG} -DPIC ${SHARED_CFLAGS} ${CFLAGS} -c ${.IMPSRC} -o ${.TARGET}
	${CTFCONVERT_CMD}

