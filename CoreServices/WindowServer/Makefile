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
                server.c \
                resistance.c \
                view.c \
                xdg-popup.c \
                keyboard.c \
                xdg.c \
                seat.c \
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
                desktop.c 
RESOURCES=	${.CURDIR}/Icon.png ${.CURDIR}/ws.conf \
		${MAKEOBJDIR}/seatd/seatd/seatd \
		${MAKEOBJDIR}/seatd/libseat/libseat.so.0 \
		${MAKEOBJDIR}/seatd/libseat/libseat.so

MK_WERROR=	no
WARNS=	        1
SYSROOT=        --sysroot=${OBJTOP}/tmp -B${OBJTOP}/tmp/usr/bin
CFLAGS+=	${SYSROOT} -g -fobjc-arc -D__RAVYNOS__ -D__MACH__ \
		-I${.CURDIR} -I${MAKEOBJDIR} -I${.CURDIR}/gplv2/include \
		-I${.CURDIR}/gplv2/cairo/src -I${MAKEOBJDIR}/gplv2/cairo \
		-I${.CURDIR}/wlroots/include -I${MAKEOBJDIR}/wlroots/include \
                -I${MAKEOBJDIR}/wlroots -I${MAKEOBJDIR}/wlroots/protocol \
		-DWLR_USE_UNSTABLE \
                -I${SRCTOP}/Frameworks/AppKit/libxkbcommon/include \
                -I${SRCTOP}/Frameworks -I${.CURDIR}/libinput/include \
                -I${.CURDIR}/libinput/src -I${OBJTOP}/tmp/usr/include/libxml2 \
                -I${SRCTOP}/Frameworks/OpenGL/wayland/src \
                -I${OBJTOP}/Frameworks/OpenGL/wayland/src \
		-I${SRCTOP}/Frameworks/OpenGL/libdrm/include/drm \
                -I${OBJTOP}/Frameworks/Foundation/Headers 

LDFLAGS+=	-Wl,-R'$$ORIGIN/../Resources' -L${MAKEOBJDIR} \
		-L${OBJTOP}/Frameworks/AppKit/AppKit.framework -lAppKit \
		-Wl,-R/System/Library/Frameworks/AppKit.framework/Versions/A \
                -L${OBJTOP}/Frameworks/Foundation/Foundation.framework \
                -Wl,-R/System/Library/Frameworks/Foundation.framework/Versions/A \
		-lWindowServer -lFoundation -lm -L${MAKEOBJDIR}/wlroots \
		-lwlroots -lwayland -lobjc -lSystem -lpixman-1 -lutil -lepoll-shim \
		-L${OBJTOP}/Frameworks/OpenGL/OpenGL.framework/Versions/A -lOpenGL \
		-Wl,-R/System/Library/Frameworks/OpenGL.framework/Versions/A \
		-ldrm -ludev-devd -ldevinfo -lxml2
WLPROTOS=       ${.CURDIR}/wayland-protocols
WLR_VER!=       grep version: ${.CURDIR}/wlroots/meson.build|sed -e '1s/[^0-9.]*//g;q'
.export SYSROOT

SUBDIR+=        libevdev .WAIT
SUBDIR+=        libinput .WAIT
SUBDIR+=        wayland-protocols
SUBDIR+=        seatd
SUBDIR+=        wlroots
SUBDIR+=	gplv2/cairo
SUBDIR+=	mtdev

${MAKEOBJDIR}/libWindowServer.so:
	${LD} -o ${.TARGET} -shared -soname ${.TARGET:T} --no-as-needed \
		$$(find ${MAKEOBJDIR} -name \*.pico | grep -v '.depend.') \
		-L${OBJTOP}/Frameworks/Onyx2D/Onyx2D.framework -lOnyx2D \
		-R/System/Library/Frameworks/Onyx2D.framework/Versions/A \
		-L${OBJTOP}/Frameworks/CoreText/CoreText.framework -lCoreText \
		-R/System/Library/Frameworks/CoreText.framework/Versions/A \
		-L${OBJTOP}/tmp/usr/lib \
		-L${OBJTOP}/CoreServices/WindowServer/gplv2/cairo -lcairo \
		-lpng -lz
	cp -f ${.TARGET} ${APP_DIR}/Contents/Resources/
	cp -fv ${.CURDIR}/libinput/quirks/* ${APP_DIR}/Contents/Resources/

make-obj-dirs: .EXEC
	mkdir -p ${MAKEOBJDIR}
.for subdir in ${SUBDIR}
	mkdir -p ${MAKEOBJDIR}/${subdir}
.endfor
	mkdir -p ${MAKEOBJDIR}/LoginWindow
	mkdir -p ${MAKEOBJDIR}/SystemUIServer

clean: .PHONY
	rm -rf ${MAKEOBJDIR}

.include <sys.mk>
.include <bsd.subdir.mk>
.include <rvn.app.mk>

${OBJS}: ${MAKEOBJDIR}/libWindowServer.so
