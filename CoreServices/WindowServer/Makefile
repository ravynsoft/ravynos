MK_AUTO_OBJ=    yes
APP=	        WindowServer
.PATH:          ${.CURDIR}
.PATH:          ${.CURDIR}/gplv2/src
.PATH:          ${.CURDIR}/gplv2/src/menu
.PATH:          ${.CURDIR}/gplv2/src/config
.PATH:          ${.CURDIR}/gplv2/src/xbm
.PATH:          ${.CURDIR}/gplv2/src/common
SRCS=           WindowServer.m \
                cursor.c \
                key-state.c \
                menu.c \
                server.c \
                resistance.c \
                osd.c \
                view.c \
                xdg-popup.c \
                ssd.c \
                keyboard.c \
                xdg.c \
                seat.c \
                theme.c \
                subsurface.c \
                view-impl.c \
                layers.c \
                view-child.c \
                interactive.c \
                action.c \
                xdg-deco.c \
                foreign.c \
                dir.c \
                nodename.c \
                font.c \
                spawn.c \
                grab-file.c \
                buf.c \
                zfree.c \
                string-helpers.c \
                parse.c \
                tokenize.c \
                xbm.c \
                damage.c \
                output.c \
                rcxml.c \
                libinput.c \
                keybind.c \
                mousebind.c \
                session.c \
                desktop.c \
                wlr-output-management-unstable-v1-protocol.c \
                wlr-layer-shell-unstable-v1-protocol.c \
                wlr-input-inhibitor-unstable-v1-protocol.c \
                xdg-shell-protocol.c
RESOURCES=	${.CURDIR}/Icon.png ${.CURDIR}/ws.conf \
                ${MAKEOBJDIR}/libcairo.so.2

MK_WERROR=	no
WARNS=	        1
SYSROOT=        --sysroot=${OBJTOP}/tmp -B${OBJTOP}/tmp/usr/bin
CFLAGS+=	${SYSROOT} -g -fobjc-arc -I${.CURDIR} \
                -I${.CURDIR}/gplv2/include -I${MAKEOBJDIR} \
		-I${.CURDIR}/wlroots/include -I${.CURDIR}/libdrm/include/drm \
                -I${SRCTOP}/Frameworks/AppKit/libxkbcommon/include \
                -I${MAKEOBJDIR}/wlroots/include -DWLR_USE_UNSTABLE \
                -I${MAKEOBJDIR}/wlroots -I${MAKEOBJDIR}/wlroots/protocol \
                -I${SRCTOP}/Frameworks -I${.CURDIR}/libinput/include \
                -I${.CURDIR}/libinput/src -I${OBJTOP}/tmp/usr/include/libxml2 \
                -I${SRCTOP}/Frameworks/OpenGL/wayland/src \
                -I${OBJTOP}/Frameworks/OpenGL/wayland/src \
                -I${OBJTOP}/Frameworks/Foundation/Headers \
                -I${OBJTOP}/Frameworks/Foundation -lFoundation \
                -Wl,-R/System/Library/Frameworks/Foundation.framework/Versions/A
LDFLAGS+=	-Wl,-R'$$ORIGIN/../Resources' -L${MAKEOBJDIR}/seatd/libseat \
		-L${MAKEOBJDIR}/libevdev -L${MAKEOBJDIR}/libinput \
		-L${OBJTOP}/Frameworks/AppKit/libxkbcommon \
		-L${MAKEOBJDIR}/wlroots -lwlroots -lm -linput -lpixman-1 \
                -lobjc -lwayland -lSystem -lxkbcommon -lseat -lutil
WLPROTOS=       ${.CURDIR}/wayland-protocols
WLR_VER!=       grep version: ${.CURDIR}/wlroots/meson.build|sed -e '1s/[^0-9.]*//g;q'
.export SYSROOT

SUBDIR+=        libevdev .WAIT
SUBDIR+=        libinput .WAIT
SUBDIR+=        wayland-protocols
SUBDIR+=        seatd
SUBDIR+=        wlroots

make-obj-dirs:
	mkdir -p ${MAKEOBJDIR}
.for subdir in ${SUBDIR}
	mkdir -p ${MAKEOBJDIR}/${subdir}
.endfor

${MAKEOBJDIR}/libcairo.so.2:
	${MAKE} -C ${.CURDIR}/gplv2/cairo build
	cp -fv gplv2/cairo/libcairo.so.2 ${MAKEOBJDIR}

.include <sys.mk>
.include <bsd.subdir.mk>
.include <rvn.app.mk>

beforebuild: make-obj-dirs obj
${OBJS}: ${MAKEOBJDIR}/libcairo.so.2
