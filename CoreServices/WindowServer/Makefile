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
RESOURCES=	${.CURDIR}/Icon.png ${.CURDIR}/ws.conf
RESOURCES+=     ${MAKEOBJDIR}/wayland/src/libwayland.so

MK_WERROR=	no
WARNS=	        1
CFLAGS+=	-g -fobjc-arc -I${.CURDIR} -I${.CURDIR}/gplv2/include -I${MAKEOBJDIR} \
		-I${.CURDIR}/wlroots/include -I${.CURDIR}/libdrm/include/drm \
                -I${MAKEOBJDIR}/wlroots/include -DWLR_USE_UNSTABLE \
                -I${MAKEOBJDIR}/wlroots/protocol/ \
                -framework Foundation 
LDFLAGS+=	-Wl,-R'$$ORIGIN/../Resources' \
		-L${MAKEOBJDIR}/libevdev -L${MAKEOBJDIR}/libinput \
		-L${MAKEOBJDIR}/wlroots -lwlroots -lm -linput -lpixman-1 \
                -lobjc -lwayland -lSystem -lxkbcommon -lseat -lutil
WLPROTOS=       ${.CURDIR}/wayland-protocols
WLR_VER!=       grep version: ${.CURDIR}/wlroots/meson.build|sed -e '1s/[^0-9.]*//g;q'
SYSROOT=        --sysroot=${OBJTOP}/tmp -B${OBJTOP}/tmp/usr/bin
.export SYSROOT

SUBDIR+=        libevdev .WAIT
SUBDIR+=        libinput .WAIT
SUBDIR+=        libdrm .WAIT
SUBDIR+=        wayland
SUBDIR+=        wayland-protocols
SUBDIR+=        wlroots

build-wayland:
#	${MAKE} -C ${.CURDIR}/wayland
#	cp -afv ${MAKEOBJDIR}/wayland/src/libwayland.so ${.CURDIR}/${APP_DIR}/Contents/Resources/
#	cp -afv ${MAKEOBJDIR}/wayland/scanner/wayland-scanner ${OBJTOP}/tmp/legacy/bin

#build-wayland-protos:
#	cd ${WLPROTOS} && CFLAGS="${SYSROOT}" \
        meson -Dtests=false ${MAKEOBJDIR}/wayland-protocols
#	ninja -C ${MAKEOBJDIR}/wayland-protocols

#build-wlroots: build-wayland-protos 
#	cd ${.CURDIR}/wlroots && PKG_CONFIG_PATH=${MAKEOBJDIR}/wayland-protocols \
	CFLAGS="${SYSROOT}" PATH="${OBJTOP}/tmp/usr/bin" meson -Dexamples=false \
        ${MAKEOBJDIR}/wlroots/
#	ninja -C ${MAKEOBJDIR}/wlroots
#	sed -e 's,@CURDIR@,${.CURDIR},g' -e 's,@MAKEOBJDIR@,${MAKEOBJDIR},g' -e 's,@VERSION@,${WLR_VER},g' \
                < ${.CURDIR}/wlroots.pc.in > ${MAKEOBJDIR}/wlroots/wlroots.pc
#	cp -afv ${MAKEOBJDIR}/wlroots/libwlroots.so ${.CURDIR}/${APP_DIR}/Contents/Resources/
#	cp -afv ${MAKEOBJDIR}/wlroots/libwlroots.so.10 ${.CURDIR}/${APP_DIR}/Contents/Resources/

clean-wlroots:
	rm -rf ${MAKEOBJDIR}/wlroots

clean-deps:
	rm -f .depend.*

.include <bsd.subdir.mk>
.include <rvn.app.mk>

${OBJS}: build-wlroots
clean: clean-wlroots clean-deps

