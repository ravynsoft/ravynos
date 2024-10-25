APP=	Dock
SRCS=	main.m Dock.m DockItem.m DesktopWindow.m DockTileData.m
RESOURCES=	${.CURDIR}/Dock.png ${.CURDIR}/running.png \
		${.CURDIR}/window.png Trash.app \
		Downloads.app

Dock.o: Dock.m Dock.h 
DockItem.o: DockItem.m DockItem.h

MK_WERROR=	no
WARNS=		1
CFLAGS+=	-g --sysroot=${OBJTOP}/tmp -D__MACH__ -fPIC \
		-B${OBJTOP}/tmp/usr/bin -D__RAVYNOS__ \
		-Wno-missing-method-return-type -I${SRCTOP}/CoreServices \
		-Wno-macro-redefined -I${SRCTOP}/Frameworks \
		-I${SRCTOP}/Frameworks/AppKit/Headers \
		-I${OBJTOP}/Frameworks/Foundation/Headers \
		-I${MAKEOBJDIR}/../WindowServer/SystemUIServer

LDFLAGS+=	-L${OBJTOP}/Frameworks/Foundation/Foundation.framework \
		-L${OBJTOP}/Frameworks/LaunchServices/LaunchServices.framework \
		-L${OBJTOP}/Frameworks/AppKit/AppKit.framework \
		-Wl,-R/System/Library/Frameworks/Foundation.framework/Versions/A \
		-Wl,-R/System/Library/Frameworks/AppKit.framework/Versions/A \
		-Wl,-R/System/Library/Frameworks/LaunchServices.framework/Versions/A \
                -lAppKit -lLaunchServices -lFoundation -lobjc -lSystem

Trash.app::
	rm -rf ${.TARGET}
	${MAKE} -C ${.CURDIR}/Trash
	mv Trash/Trash.app .

Downloads.app::
	rm -rf ${.TARGET}
	${MAKE} -C ${.CURDIR}/Downloads
	mv Downloads/Downloads.app .

.include <rvn.app.mk>

all: Trash.app Downloads.app
	cp -fv ${.CURDIR}/Info.plist ${APP_DIR}/Contents/
