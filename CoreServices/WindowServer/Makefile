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
RESOURCES=	${.CURDIR}/Icon.png ${.CURDIR}/ws.conf

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
		-L${OBJTOP}/Frameworks/AppKit/libxkbcommon \
		-L${OBJTOP}/Frameworks/OpenGL/OpenGL.framework/Resources \
		-L${OBJTOP}/Frameworks/OpenGL/OpenGL.framework \
		-Wl,-R/System/Library/Frameworks/OpenGL.framework/Versions/A \
		-Wl,-R/System/Library/Frameworks/OpenGL.framework/Resources \
                -L${OBJTOP}/Frameworks/Foundation/Foundation.framework \
                -Wl,-R/System/Library/Frameworks/Foundation.framework/Versions/A \
		-lFoundation -ldrm -lGLESv2 -lEGL -lgbm -lOpenGL \
		-lm -lpixman-1 -lxml2 -L${MAKEOBJDIR}/wlroots -lwlroots \
                -lobjc -lwayland -lSystem -lxkbcommon -lutil \
		-lepoll-shim -ldevinfo -lWindowServer
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
	${LD} -o ${.TARGET} -shared -soname ${.TARGET:T} \
		--no-as-needed --no-undefined \
		$$(find ${MAKEOBJDIR} -name \*.pico | grep -v '.depend.') \
		-L${OBJTOP}/tmp/usr/lib -ludev-devd -ldevinfo \
		-L${OBJTOP}/CoreServices/WindowServer/gplv2/cairo -lcairo \
		-lpixman-1 -lc

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
