#
# FIXME: zoe 2/11/26 This should live in SkyLight now. See
# https://eclecticlight.co/2020/06/08/windowserver-display-compositor-and-input-event-router/
# "In Catalina, it's to be found in /System/Library/PrivateFrameworks/SkyLight.framework/Resources,
# alongside pointer/cursor support."
# Plist goes in /Library/Preferences/com.apple.windowserver.plist
#
APP=	        WindowServer
.PATH:          ${.CURDIR}
SRCS=           WindowServer.m \
		IOFramebuffer.mm \
		WSInput.m \
		WSAppRecord.m \
		WSWindowRecord.m \
		WSDisplay.m \
		main.m
RESOURCES=	${.CURDIR}/Icon.png \
		${.CURDIR}/Cursors/arrowCursor.png \
		${.CURDIR}/Cursors/resizeCursor.png \
		${.CURDIR}/Decorations/close.png \
		${.CURDIR}/Decorations/closeHover.png \
		${.CURDIR}/Decorations/mini.png \
		${.CURDIR}/Decorations/miniHover.png \
		${.CURDIR}/Decorations/zoom.png \
		${.CURDIR}/Decorations/zoomUpHover.png \
		${.CURDIR}/Decorations/zoomDownHover.png

MK_WERROR=	no
MK_PIE=		no
WARNS=	        1
SYSROOT=        --sysroot=${RAVYN_SDKROOT} -B${TOOLS}
CFLAGS+=	-fobjc-arc -g -D__WSRPC -pthread \
		-I${.CURDIR} -I${.OBJDIR} -I${.CURDIR}/libinput/src \
		-I${.CURDIR}/libinput/include -I${ROOT_SOURCE_DIR}/Frameworks \
		-I${.CURDIR}/libxkbcommon/include -I${ROOT_SOURCE_DIR}/CoreServices \
		-I${ROOT_SOURCE_DIR}/Frameworks/Foundation/Headers \
		-I${ROOT_SOURCE_DIR}/Frameworks/AppKit/Headers

LDFLAGS+=	-L${.OBJDIR} \
		-lAppKit -lFoundation -lCoreGraphics -lcrypt -llaunch \
		-lCoreFoundation -lOnyx2D -lxml2 -pthread -lobjc -lSystem -lkvm -lm \
		${OBJTOP}/tmp/lib/libc.so.7
#.export SYSROOT

SUBDIR+=        libevdev .WAIT
SUBDIR+=	mtdev .WAIT
SUBDIR+=        libinput .WAIT
SUBDIR+=	libxkbcommon

${MAKEOBJDIR}/Resources:
	cp -fv ${.CURDIR}/libinput/quirks/* ${APP_DIR}/Contents/Resources/
	${MAKE} -C ${.CURDIR}/libevdev tools
	${MAKE} -C ${.CURDIR}/libevdev tools-install APP_DIR=${APP_DIR}
	${MAKE} -C ${.CURDIR}/libinput tools
	${MAKE} -C ${.CURDIR}/libinput tools-install APP_DIR=${APP_DIR}

make-obj-dirs: .EXEC
	mkdir -p ${MAKEOBJDIR}
.for subdir in ${SUBDIR}
	mkdir -p ${MAKEOBJDIR}/${subdir}
.endfor
	mkdir -p ${MAKEOBJDIR}/LoadingWindow
	mkdir -p ${MAKEOBJDIR}/LoginWindow
	mkdir -p ${MAKEOBJDIR}/SystemUIServer
	mkdir -p ${MAKEOBJDIR}/libxkbcommon/src/xkbcomp
	mkdir -p ${MAKEOBJDIR}/libxkbcommon/src/compose

clean: .PHONY
	rm -rf ${MAKEOBJDIR}

.include <sys.mk>
.include <bsd.subdir.mk>
.include <rvn.app.mk>

${OBJS}: ${MAKEOBJDIR}/Resources

client: client.m message.h
	mkdir -p ${.CURDIR}/client.app/Contents/ravynOS
	cd ${.CURDIR}; ${CC} -g -fobjc-arc -fobjc-runtime=gnustep-2.0 -fobjc-nonfragile-abi \
		-fPIC -o ${.CURDIR}/client.app/Contents/ravynOS/client client.m \
		-framework Foundation -framework CoreFoundation \
		-framework CoreGraphics -framework AppKit -lSystem

