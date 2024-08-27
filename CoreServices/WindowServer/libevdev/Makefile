.include <src.opts.mk>

LIB=		evdev
PACKAGE=        lib${LIB}
SHLIB_MAJOR=    2

.PATH:          ${.CURDIR}/libevdev
SRCS=           libevdev-names.c \
                libevdev-uinput.c \
                libevdev.c \

INCSDIR=        ${INCLUDEDIR}/${PACKAGE}
INCS=	        libevdev/libevdev-uinput.h \
                libevdev/libevdev.h

CFLAGS+=        -I${SRCTOP}/sys/compat/linuxkpi/common/include \
                -I${.CURDIR}/include \
	        -I${.CURDIR} \
                -I${MAKEOBJDIR} \
                -DHAVE_DLFCN_H \
                -DHAVE_INTTYPES_H \
                -DHAVE_LIBM \
                -DHAVE_MEMORY_H \
                -DHAVE_STDINT_H \
                -DHAVE_STDLIB_H \
                -DHAVE_STRINGS_H \
                -DHAVE_STRING_H \
                -DHAVE_SYS_STAT_H \
                -DHAVE_SYS_TYPES_H \
                -DHAVE_UNISTD_H \
                -DSTDC_HEADERS \
                -D_GNU_SOURCE \
                -Wno-error=macro-redefined
WARNS?=	2

config.h: .PHONY
	echo '#define _IOC_NONE 1U' >${.TARGET}
	echo '#define _IOC_READ 2U' >>${.TARGET}
	echo '#define _IOC_WRITE 4U' >>${.TARGET}
	echo '#define _STANDALONE' >>${.TARGET}
	echo '#include <linux/types.h>' >>${.TARGET}
	echo '#undef _STANDALONE' >>${.TARGET}


${INCSDIR}: .PHONY
	mkdir -vp ${OBJTOP}/tmp/${INCSDIR}

event-names.h: .PHONY
	LD_LIBRARY_PATH=${OBJTOP}/tmp/usr/lib ${OBJTOP}/tmp/usr/bin/python3 \
		${.CURDIR}/libevdev/make-event-names.py \
        	${.CURDIR}/include/linux/freebsd/input.h \
        	${.CURDIR}/include/linux/freebsd/input-event-codes.h >${.TARGET}

TOOL_CFLAGS= -D__float_t=float -D__double_t=double
TOOL_LDFLAGS= -L${MAKEOBJDIR} -levdev -Wl,-R'$$ORIGIN'
TOOLS=	libevdev-events touchpad-edge-detector \
	mouse-dpi-tool libevdev-tweak-device

tools: ${TOOLS}
libevdev-events: tools/libevdev-events.c
	${CC} ${CPPFLAGS} ${CFLAGS} ${TOOL_CFLAGS} -c $< -o $@.o
	${CC} -o $@ $@.o ${TOOL_LDFLAGS}
touchpad-edge-detector: tools/touchpad-edge-detector.c
	${CC} ${CPPFLAGS} ${CFLAGS} ${TOOL_CFLAGS} -c $< -o $@.o
	${CC} -o $@ $@.o ${TOOL_LDFLAGS} -lm
mouse-dpi-tool: tools/mouse-dpi-tool.c
	${CC} ${CPPFLAGS} ${CFLAGS} ${TOOL_CFLAGS} -c $< -o $@.o
	${CC} -o $@ $@.o ${TOOL_LDFLAGS}
libevdev-tweak-device: tools/libevdev-tweak-device.c
	${CC} ${CPPFLAGS} ${CFLAGS} ${TOOL_CFLAGS} -c $< -o $@.o
	${CC} -o $@ $@.o ${TOOL_LDFLAGS}

tools-install: .PHONY
	cp -fv ${TOOLS} ${APP_DIR}/Contents/Resources/

.include <bsd.lib.mk>

${OBJS}: ${INCSDIR} config.h event-names.h
