APP=	        WindowServer
SRCS=           WindowServer.m
RESOURCES=	${.CURDIR}/Icon.png ${.CURDIR}/ws.conf

MK_WERROR=	no
WARNS=	        1
CFLAGS+=	-g -fobjc-arc -I${.CURDIR} -I${MAKEOBJDIR} -framework Foundation
LDFLAGS+=	-framework Foundation -Wl,-R'$$ORIGIN/../Resources' -lobjc -lmach
WLPROTOS=       ${.CURDIR}/wayland-protocols

build-wayland-protos:
	cd ${WLPROTOS} && meson ${MAKEOBJDIR}/wayland-protocols
	ninja -C ${MAKEOBJDIR}/wayland-protocols

build-wlroots: build-wayland-protos
	cd ${.CURDIR}/wlroots && PKG_CONFIG_PATH=${MAKEOBJDIR}/wayland-protocols meson -Dexamples=false \
                ${MAKEOBJDIR}/wlroots/
	ninja -C ${MAKEOBJDIR}/wlroots
	sed -e 's,@CURDIR@,${.CURDIR},g' -e 's,@MAKEOBJDIR@,${MAKEOBJDIR},g' < ${.CURDIR}/wlroots.pc.in \
                > ${MAKEOBJDIR}/wlroots/wlroots.pc
	cp -afv ${MAKEOBJDIR}/wlroots/libwlroots.so* ${MAKEOBJDIR}/${APP_DIR}/Contents/Resources/

build-waybox:
	cd ${.CURDIR}/waybox && PKG_CONFIG_PATH=${MAKEOBJDIR}/wayland-protocols:${MAKEOBJDIR}/wlroots \
                meson ${MAKEOBJDIR}/waybox
	ninja -C ${MAKEOBJDIR}/waybox
	cp -fv ${MAKEOBJDIR}/waybox/waybox/waybox ${MAKEOBJDIR}/${APP_DIR}/Contents/Resources/

clean-wlroots:
	rm -rf ${MAKEOBJDIR}/wlroots

clean-waybox:
	rm -rf ${MAKEOBJDIR}/waybox

clean-deps:
	rm -f .depend.*

.include <airyx.app.mk>

${OBJS}: build-wlroots build-waybox
clean: clean-wlroots clean-waybox clean-deps

