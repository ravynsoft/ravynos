APP=	        WindowServer
.if ${MAKEOBJDIR}
MODIR=          ${MAKEOBJDIR}
.else
MODIR=          .
.endif
.PATH:          ${MODIR}/wlroots/protocol
.PATH:          ${.CURDIR}
.PATH:          ${.CURDIR}/gplv2/src
SRCS=           WindowServer.m \
                cursor.c \
                key-state.c \
                menu/menu.c \
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
                common/dir.c \
                common/nodename.c \
                common/font.c \
                common/spawn.c \
                common/grab-file.c \
                common/buf.c \
                common/zfree.c \
                common/string-helpers.c \
                xbm/parse.c \
                xbm/tokenize.c \
                xbm/xbm.c \
                damage.c \
                output.c \
                config/rcxml.c \
                config/libinput.c \
                config/keybind.c \
                config/mousebind.c \
                config/session.c \
                desktop.c \
                Desktop/AppDelegate.m \
                Desktop/ClockView.m \
                Desktop/MenuView.m \
                Desktop/ExtrasView.m \
                Desktop/DesktopWindow.m \
                Desktop/MenuBarWindow.m
RESOURCES=	${.CURDIR}/Icon.png ${.CURDIR}/ws.conf ${.CURDIR}/ravynos-mark-64.png

MK_WERROR=	no
WARNS=	        1
CFLAGS_PKGS!=   pkg-config --cflags libxml-2.0 pangocairo
LDFLAGS_PKGS!=  pkg-config --libs libxml-2.0 pangocairo
CFLAGS+=	-g -fobjc-arc -I${.CURDIR} -I${.CURDIR}/gplv2/include -I${MAKEOBJDIR} \
		-I${.CURDIR}/wlroots/include ${CFLAGS_PKGS} -I/usr/include/libdrm \
                -I${MAKEOBJDIR}/wlroots/include -DWLR_USE_UNSTABLE \
                -I/usr/include/pixman-1 -I${MAKEOBJDIR}/wlroots/protocol/ \
                -framework Foundation -framework AppKit
LDFLAGS+=	-Wl,-R'$$ORIGIN/../Resources' ${LDFLAGS_PKGS} \
		-L${MAKEOBJDIR}/wlroots -lwlroots -lm -linput -lpixman-1 \
                -lobjc -lwayland-client -lwayland-server -lSystem -lxkbcommon -lseat
WLPROTOS=       ${.CURDIR}/wayland-protocols
WLR_VER!=       grep version: ${.CURDIR}/wlroots/meson.build|sed -e '1s/[^0-9.]*//g;q'

build-wayland-protos:
	cd ${WLPROTOS} && meson ${MAKEOBJDIR}/wayland-protocols
	ninja -C ${MAKEOBJDIR}/wayland-protocols

build-wlroots: build-wayland-protos
	cd ${.CURDIR}/wlroots && PKG_CONFIG_PATH=${MAKEOBJDIR}/wayland-protocols meson -Dexamples=false \
                ${MAKEOBJDIR}/wlroots/
	ninja -C ${MAKEOBJDIR}/wlroots
	sed -e 's,@CURDIR@,${.CURDIR},g' -e 's,@MAKEOBJDIR@,${MAKEOBJDIR},g' -e 's,@VERSION@,${WLR_VER},g' \
                < ${.CURDIR}/wlroots.pc.in > ${MAKEOBJDIR}/wlroots/wlroots.pc
	cp -afv ${MAKEOBJDIR}/wlroots/libwlroots.so* ${.CURDIR}/${APP_DIR}/Contents/Resources/

clean-wlroots:
	rm -rf ${MAKEOBJDIR}/wlroots

clean-deps:
	rm -f .depend.*

.include <rvn.app.mk>

${OBJS}: build-wlroots
clean: clean-wlroots clean-deps

