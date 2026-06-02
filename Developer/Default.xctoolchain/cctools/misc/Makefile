CFLAGS += -std=gnu99 -D__DARWIN_UNIX03 -Wno-deprecated \
	  -Wno-deprecated-declarations -I${SRCDIR}/include \
	  -D__private_extern__=extern
CFLAGS_bitcode_strip = -DALLOW_ARCHIVES
LDFLAGS += -L${TOOLCHAIN}/usr/lib -lmacho -lstuff -lm
TARGETS = ${TOOLS}/checksyms ${TOOLS}/lipo ${TOOLS}/size ${TOOLS}/strings \
	  ${TOOLS}/nm ${TOOLS}/libtool ${TOOLS}/redo_prebinding ${TOOLS}/seg_addr_table \
	  ${TOOLS}/seg_hack ${TOOLS}/install_name_tool ${TOOLS}/indr ${TOOLS}/vtool \
	  ${TOOLS}/strip ${TOOLS}/segedit ${TOOLS}/pagestuff ${TOOLS}/codesign_allocate \
	  ${TOOLS}/bitcode_strip ${TOOLS}/ctf_insert ${TOOLS}/cmpdylib ${TOOLS}/inout \


all: ${TARGETS} ${TOOLS}/nmedit ${TOOLS}/ranlib

.for name in ${TARGETS}
${name}: ${.CURDIR}/${name:T}.c
	${HOST_CC} -o ${.TARGET} ${CFLAGS} ${CFLAGS_${name}} ${.ALLSRC} ${LDFLAGS}
.endfor

${TOOLS}/nmedit: strip.c
	${HOST_CC} ${CFLAGS} -DNMEDIT -I${.CURDIR}/../libstuff \
		-o ${.TARGET} ${.ALLSRC} ${LDFLAGS}

${TOOLS}/ranlib: libtool.c
	${HOST_CC} ${CFLAGS} -DRANLIB -I${.CURDIR}/../libstuff \
		-o ${.TARGET} ${.ALLSRC} ${LDFLAGS}

