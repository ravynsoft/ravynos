APP=	        WindowServer
SRCS=           WindowServer.m \
                waybox/waybox/config.c \
                waybox/waybox/cursor.c \
                waybox/waybox/decoration.c \
                waybox/waybox/layer_shell.c \
                waybox/waybox/output.c \
                waybox/waybox/seat.c \
                waybox/waybox/server.c \
                waybox/waybox/xdg_shell.c \
                Desktop/AppDelegate.m \
                Desktop/ClockView.m \
                Desktop/MenuView.m \
                Desktop/ExtrasView.m \
                Desktop/DesktopWindow.m \
                Desktop/MenuBarWindow.m
RESOURCES=	${.CURDIR}/Icon.png ${.CURDIR}/ws.conf ${.CURDIR}/ravynos-mark-64.png

MK_WERROR=	no
WARNS=	        1
CFLAGS_XML2!=   pkg-config --cflags libxml-2.0
LDFLAGS_XML2!=  pkg-config --libs libxml-2.0
CFLAGS+=	-g -fobjc-arc -I${.CURDIR} -I${.CURDIR}/waybox/include -I${MAKEOBJDIR} \
		-I${.CURDIR}/wlroots/include -I${.CURDIR}/waybox/waybox \
                -I${MAKEOBJDIR}/wlroots/include -DWLR_USE_UNSTABLE \
                -I/usr/include/pixman-1 -I${MAKEOBJDIR}/wlroots/protocol/ \
                ${CFLAGS_XML2} \
                -framework Foundation -framework AppKit
LDFLAGS+=	-Wl,-R'$$ORIGIN/../Resources' ${LDFLAGS_XML2} \
		-L${MAKEOBJDIR}/wlroots -lwlroots \
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

build-waybox:
	cd ${.CURDIR}/waybox && PKG_CONFIG_PATH=${MAKEOBJDIR}/wayland-protocols:${MAKEOBJDIR}/wlroots \
                meson ${MAKEOBJDIR}/waybox
	ninja -C ${MAKEOBJDIR}/waybox

clean-wlroots:
	rm -rf ${MAKEOBJDIR}/wlroots

clean-waybox:
	rm -rf ${MAKEOBJDIR}/waybox

clean-deps:
	rm -f .depend.*

.include <rvn.app.mk>

${OBJS}: build-wlroots build-waybox
clean: clean-wlroots clean-deps clean-waybox

