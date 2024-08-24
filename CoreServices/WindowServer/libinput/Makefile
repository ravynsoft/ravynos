LIB=	        input
SHLIB_NAME=	libinput

.PATH:          ${.CURDIR}
.PATH:          ${.CURDIR}/src

MAJOR=          1
MINOR=          25
MICRO=          0
VERSION=        ${MAJOR}.${MINOR}.${MICRO}

SRCS=   libinput-version.h .WAIT \
	evdev-mt-touchpad-edge-scroll.c \
	filter.c \
	libinput.c \
	timer.c \
	evdev-mt-touchpad-thumb.c \
	util-prop-parsers.c \
	filter-custom.c \
	filter-trackpoint.c \
	quirks.c \
	util-strings.c \
	udev-seat.c \
	util-ratelimit.c \
	evdev-mt-touchpad.c \
	util-list.c \
	filter-low-dpi.c \
	filter-flat.c \
	evdev-mt-touchpad-tap.c \
	filter-mouse.c \
	evdev.c \
	evdev-mt-touchpad-buttons.c \
	filter-trackpoint-flat.c \
	libinput-private-config.c \
	evdev-middle-button.c \
	evdev-tablet.c \
	evdev-fallback.c \
	path-seat.c \
	filter-tablet.c \
	filter-touchpad-x230.c \
	evdev-wheel.c \
	evdev-debounce.c \
	evdev-mt-touchpad-gestures.c \
	evdev-totem.c \
	filter-touchpad-flat.c \
	filter-touchpad.c \
	evdev-tablet-pad-leds.c \
	evdev-tablet-pad.c

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
CFLAGS+=	-I${SRCTOP}/lib/libudev-devd -I${.CURDIR}/../mtdev/include
CFLAGS+=        -DHTTP_DOC_LINK=\"https://wayland.freedesktop.org/libinput/doc/${MAJOR}.${MINOR}.${MICRO}\"
CFLAGS+=        -DLIBINPUT_QUIRKS_DIR=\"/System/Library/CoreServices/WindowServer.app/Contents/Resources\"
CFLAGS+=        -DLIBINPUT_QUIRKS_OVERRIDE_FILE=\"/System/Library/CoreServices/WindowServer.app/Contents/Resources/local-overrides.quirks\"
CFLAGS+=	-DLIBINPUT_TOOL_PATH=\"/System/Library/CoreServices/WindowServer.app/Contents/Resources\"
CFLAGS+=        -DLIBINPUT_QUIRKS_SRCDIR=\"${.CURDIR}/quirks\" -D_GNU_SOURCE=1 -DHAVE_LOCALE_H
CFLAGS+=        -DHAVE_VERSIONSORT -DMESON_BUILD_ROOT=\"\" -DPTRACE_ATTACH=PT_ATTACH -DHAVE_XLOCALE_H
CFLAGS+=	-DPTRACE_CONT=PT_CONTINUE -DPTRACE_DETACH=PT_DETACH -DHAVE_INSTALLED_TESTS=0
CFLAGS+=	-Dprogram_invocation_short_name=\"getprogname\(\)\"
CFLAGS+=	-DHAVE_LIBEVDEV_DISABLE_PROPERTY -DHAVE_LIBWACOM=0 -DHAVE_DEBUG_GUI=0 -fPIC
LDFLAGS+=	-Wl,-R'$$ORIGIN/../Resources'

libinput-version.h:
	sed -e 's/@LIBINPUT_VERSION_MAJOR@/${MAJOR}/' \
            -e 's/@LIBINPUT_VERSION_MINOR@/${MINOR}/' \
            -e 's/@LIBINPUT_VERSION_MICRO@/${MICRO}/' \
            -e 's/@LIBINPUT_VERSION@/${VERSION}/' \
            ${.CURDIR}/src/libinput-version.h.in > libinput-version.h

.include <bsd.lib.mk>

