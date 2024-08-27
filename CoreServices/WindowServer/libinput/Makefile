LIB=	        input

.PATH:          ${.CURDIR}
.PATH:          ${.CURDIR}/src

MAJOR=          1
MINOR=          25
MICRO=          0
VERSION=        ${MAJOR}.${MINOR}.${MICRO}
SHLIB_MAJOR=	${MAJOR}

SRCS=   libinput-version.h \
	libinput-git-version.h .WAIT \
	libinput.c \
	libinput-private-config.c \
	evdev.c \
	evdev-debounce.c \
	evdev-mt-touchpad-edge-scroll.c \
	evdev-fallback.c \
	evdev-totem.c \
	evdev-middle-button.c \
	evdev-mt-touchpad.c \
	evdev-mt-touchpad-thumb.c \
	evdev-mt-touchpad-tap.c \
	evdev-mt-touchpad-buttons.c \
	evdev-mt-touchpad-gestures.c \
	evdev-tablet.c \
	evdev-tablet-pad-leds.c \
	evdev-tablet-pad.c \
	evdev-wheel.c \
	path-seat.c \
	udev-seat.c \
	timer.c 

INCS=   include/valgrind/valgrind.h \
	include/linux/input.h \
	include/linux/freebsd/input.h \
	include/linux/freebsd/input-event-codes.h \
	src/filter-private.h \
	src/evdev-tablet.h \
	src/evdev-fallback.h \
	src/evdev.h \
	src/libinput-private-config.h \
	src/libinput-private.h \
	src/util-matrix.h \
	src/evdev-tablet-pad.h \
	src/quirks.h \
	src/util-strings.h \
	src/udev-seat.h \
	src/timer.h \
	src/util-prop-parsers.h \
	src/util-time.h \
	src/util-input-event.h \
	src/filter.h \
	src/libinput.h \
	src/util-macros.h \
	src/util-list.h \
	src/util-bits.h \
	src/libinput-versionsort.h \
	src/builddir.h \
	src/util-ratelimit.h \
	src/evdev-mt-touchpad.h \
	src/libinput-util.h

MK_WERROR=	no
WARNS=	        1
CFLAGS+=	-I${.CURDIR}/src -I${.CURDIR}/include -I${MAKEOBJDIR}
CFLAGS+=        -I${.CURDIR}/../libevdev -I${.CURDIR}
CFLAGS+=        -I${OBJTOP}/tmp/usr/include/libepoll-shim
CFLAGS+=	-I${SRCTOP}/lib/libudev -I${.CURDIR}/../mtdev/include
CFLAGS+=        -DHTTP_DOC_LINK=\"https://wayland.freedesktop.org/libinput/doc/${MAJOR}.${MINOR}.${MICRO}\"
CFLAGS+=        -DLIBINPUT_QUIRKS_DIR=\"/System/Library/CoreServices/WindowServer.app/Contents/Resources\"
CFLAGS+=        -DLIBINPUT_QUIRKS_OVERRIDE_FILE=\"/System/Library/CoreServices/WindowServer.app/Contents/Resources/local-overrides.quirks\"
CFLAGS+=	-DLIBINPUT_TOOL_PATH=\"/System/Library/CoreServices/WindowServer.app/Contents/Resources\"
CFLAGS+=        -DLIBINPUT_QUIRKS_SRCDIR=\"${.CURDIR}/quirks\" -D_GNU_SOURCE=1 -DHAVE_LOCALE_H
CFLAGS+=        -DHAVE_VERSIONSORT -DMESON_BUILD_ROOT=\"\" -DPTRACE_ATTACH=PT_ATTACH -DHAVE_XLOCALE_H
CFLAGS+=	-DPTRACE_CONT=PT_CONTINUE -DPTRACE_DETACH=PT_DETACH -DHAVE_INSTALLED_TESTS=0
CFLAGS+=	-Dprogram_invocation_short_name=\"getprogname\(\)\" -fvisibility=hidden
CFLAGS+=	-DHAVE_LIBEVDEV_DISABLE_PROPERTY -DHAVE_LIBWACOM=0 -DHAVE_DEBUG_GUI=0 -fPIC
LDFLAGS+=	-L${OBJTOP}/CoreServices/WindowServer/mtdev -lmtdev
LDFLAGS+=	-L${OBJTOP}/CoreServices/WindowServer/libevdev -levdev
LDFLAGS+=	-L${OBJTOP}/tmp/usr/lib -lepoll-shim -ludev
LDFLAGS+=	-L${MAKEOBJDIR} -linput-util -lquirks -lfilter -lm -lrt 
LDFLAGS+=	-Wl,-R'$$ORIGIN/../Resources' -Wl,--version-script=${.CURDIR}/src/libinput.sym

GIT_SHA!=	cd ${.CURDIR}; git log -1 --pretty=format:%h

libinput-version.h:
	sed -e 's/@LIBINPUT_VERSION_MAJOR@/${MAJOR}/' \
            -e 's/@LIBINPUT_VERSION_MINOR@/${MINOR}/' \
            -e 's/@LIBINPUT_VERSION_MICRO@/${MICRO}/' \
            -e 's/@LIBINPUT_VERSION@/${VERSION}/' \
            ${.CURDIR}/src/libinput-version.h.in > libinput-version.h

libinput-git-version.h:
	sed -e 's/@VCS_TAG@/${GIT_SHA}/' \
	${.CURDIR}/src/libinput-git-version.h.in > libinput-git-version.h

TOOLS=  tools/libinput-debug-events \
	tools/libinput-debug-tablet \
	tools/libinput-quirks \
	tools/libinput-list-devices \
	tools/libinput-measure \
	tools/libinput-analyze \
	tools/libinput-record \
	tools/ptraccel-debug \
	tools/libinput

TOOL_CFLAGS= ${CFLAGS:S/-fvisibility=hidden//}
TOOL_LDFLAGS= -Ltools -ltools -L. -linput \
		-L../libevdev -levdev -Wl,-R'$$ORIGIN' \
		-L${OBJTOP}/tmp/usr/lib -lepoll-shim -ludev \
		-L../mtdev -lmtdev -ldevinfo -lthr -lm

tools: tools/libtools.so ${TOOLS}
tools/libtools.so: tools/shared.c
	mkdir -p ${MAKEOBJDIR}/tools
	touch ${MAKEOBJDIR}/config.h
	${CC} ${TOOL_CFLAGS} -c ${.CURDIR}/tools/shared.c -o tools/shared.o
	${CC} -shared -Wl,-soname=libtools.so -o tools/libtools.so \
		tools/shared.o -L. -linput -Wl,-R'$$ORIGIN' \
		-L../libevdev -levdev \
		-L../mtdev -lmtdev -L${OBJTOP}/tmp/usr/lib -lepoll-shim -ludev \
		-ldevinfo -lthr -lm -lrt

tools/libinput-debug-events: tools/libinput-debug-events.c
	${CC} ${TOOL_CFLAGS} -o $@ ${.ALLSRC} ${TOOL_LDFLAGS}

tools/libinput-debug-tablet: tools/libinput-debug-tablet.c
	${CC} ${TOOL_CFLAGS} -o $@ ${.ALLSRC} ${TOOL_LDFLAGS}

tools/libinput-quirks: tools/libinput-quirks.c
	${CC} ${TOOL_CFLAGS} -o $@ ${.ALLSRC} ${TOOL_LDFLAGS} -lquirks -linput-util

tools/libinput-list-devices: tools/libinput-list-devices.c
	${CC} ${TOOL_CFLAGS} -o $@ ${.ALLSRC} ${TOOL_LDFLAGS}

tools/libinput-measure: tools/libinput-measure.c
	${CC} ${TOOL_CFLAGS} -o $@ ${.ALLSRC} ${TOOL_LDFLAGS}

tools/libinput-analyze: tools/libinput-analyze.c
	${CC} ${TOOL_CFLAGS} -o $@ ${.ALLSRC} ${TOOL_LDFLAGS}

tools/libinput-record: tools/libinput-record.c
	${CC} ${TOOL_CFLAGS} -o $@ ${.ALLSRC} ${TOOL_LDFLAGS} -lquirks -linput-util

tools/ptraccel-debug: tools/ptraccel-debug.c
	${CC} ${TOOL_CFLAGS} -o $@ ${.ALLSRC} ${TOOL_LDFLAGS} -linput-util -lfilter

tools/libinput: tools/libinput-tool.c
	${CC} ${TOOL_CFLAGS} -o $@ ${.ALLSRC} ${TOOL_LDFLAGS}


PYTHON_TOOLS= tools/libinput-analyze-per-slot-delta.py \
	      tools/libinput-analyze-recording.py \
	      tools/libinput-analyze-touch-down-state.py \
	      tools/libinput-list-kernel-devices.py \
	      tools/libinput-measure-fuzz.py \
	      tools/libinput-measure-touchpad-size.py \
	      tools/libinput-measure-touchpad-tap.py \
	      tools/libinput-measure-touchpad-pressure.py \
	      tools/libinput-measure-touch-size.py \
	      tools/libinput-replay.py

tools-install:
.for tool in ${TOOLS} tools/libtools.so
	cp -fv ${tool} ${APP_DIR}/Contents/Resources/
	chmod 0755 ${APP_DIR}/Contents/Resources/${tool:T}
.endfor
.for tool in ${PYTHON_TOOLS}
	cp -fv ${.CURDIR}/${tool} ${APP_DIR}/Contents/Resources/
	chmod 0755 ${APP_DIR}/Contents/Resources/${tool:T}
.endfor

SRC_UTIL= util-list.c \
	  util-ratelimit.c \
	  util-strings.c \
	  util-prop-parsers.c

libinput-util.a: ${SRC_UTIL:S/.c/.pico/g}
	ar rcs libinput-util.a ${SRC_UTIL:S/.c/.pico/g}

libquirks.a: quirks.c quirks.o
	ar rcs libquirks.a quirks.o

SRC_FILTER= filter.c \
	filter-custom.c \
	filter-trackpoint.c \
	filter-low-dpi.c \
	filter-flat.c \
	filter-mouse.c \
	filter-trackpoint-flat.c \
	filter-tablet.c \
	filter-touchpad-x230.c \
	filter-touchpad-flat.c \
	filter-touchpad.c

libfilter.a: ${SRC_FILTER:S/.c/.pico/g}
	ar rcs libfilter.a ${SRC_FILTER:S/.c/.pico/g}

.include <bsd.lib.mk>

${OBJS}: libinput-util.a libquirks.a libfilter.a
