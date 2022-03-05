APP=	WindowServer
SRCS=           main.c \
                xdg-shell.c \
                wlr-screencopy-unstable-v1.c \
                WindowServer.c

RESOURCES=      ${MAKEOBJDIR}/wlroots/libwlroots.so.11
RESOURCES+=	${.CURDIR}/Icon.png

MK_WERROR=	no
PKGCONFIGS=     freetype2 pixman-1 xkbcommon
PKGCONFIG_CFLAGS!=     pkg-config --cflags ${PKGCONFIGS}
PKGCONFIG_LIBS!=       pkg-config --libs ${PKGCONFIGS}
CFLAGS+=	-g -fPIC -fobjc-arc ${PKGCONFIG_CFLAGS} -I/usr/include/fontconfig \
                -I/usr/src/Frameworks/Onyx2D -I${.CURDIR}/wlroots/include \
                -I${.CURDIR} -I${MAKEOBJDIR} -DWLR_USE_UNSTABLE \
                -framework Onyx2D -framework OpenGL
LDFLAGS+=	-framework Foundation -framework OpenGL \
                -framework Onyx2D -framework CoreGraphics -framework CoreFoundation \
                -L${MAKEOBJDIR}/wlroots -lwlroots -Wl,-R'$$ORIGIN/../Resources' \
                -lEGL -lEGL_mesa -lGLESv2 ${PKGCONFIG_LIBS} -lfontconfig -lobjc -lmach -lwayland-server
WLPROTOS=       ${.CURDIR}/wlroots/wayland-protocols

xdg-shell-protocol.h:
	wayland-scanner server-header \
                ${WLPROTOS}/stable/xdg-shell/xdg-shell.xml ${.TARGET}

xdg-shell.c:
	wayland-scanner private-code \
                ${WLPROTOS}/stable/xdg-shell/xdg-shell.xml ${.TARGET}

wlr-screencopy-unstable-v1.h:
	wayland-scanner server-header \
                ${.CURDIR}/wlroots/protocol/wlr-screencopy-unstable-v1.xml ${.TARGET}

wlr-screencopy-unstable-v1.c:
	wayland-scanner private-code \
                ${.CURDIR}/wlroots/protocol/wlr-screencopy-unstable-v1.xml ${.TARGET}

build-wlroots:
	cd ${.CURDIR}/wlroots && PKG_CONFIG_PATH=. meson -Dexamples=false \
                -Dbackends=drm,libinput ${MAKEOBJDIR}/wlroots/
	ninja -C ${MAKEOBJDIR}/wlroots

clean-wlroots:
	rm -rf ${MAKEOBJDIR}/wlroots

clean-deps:
	rm -f .depend.*

clean-gen-files:
	rm -f xdg-shell-protocol.h xdg-shell.c

.include <airyx.app.mk>

${OBJS}: build-wlroots xdg-shell-protocol.h xdg-shell.c \
        wlr-screencopy-unstable-v1.c wlr-screencopy-unstable-v1.h
clean: clean-wlroots clean-deps clean-gen-files

