APP=	        WindowServer
SRCS=           WindowServer.m
RESOURCES=      ${MAKEOBJDIR}/wlroots/libwlroots.so.11
RESOURCES+=	${.CURDIR}/Icon.png

MK_WERROR=	no
CFLAGS+=	-g -fPIC -fobjc-arc -I${.CURDIR} -I${MAKEOBJDIR}
LDFLAGS+=	-framework Foundation -framework CoreFoundation \
                -Wl,-R'$$ORIGIN/../Resources' -lmach
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

.include <airyx.app.mk>

${OBJS}: build-wlroots build-waybox
clean: clean-wlroots clean-waybox clean-deps

