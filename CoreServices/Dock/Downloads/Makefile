APP=	Downloads
SRCS=	main.m Downloads.m
RESOURCES=	${.CURDIR}/Downloads.png

Downloads.o: Downloads.m Downloads.h

MK_WERROR=	no
CFLAGS+=	-g -fPIC -fobjc-arc --sysroot=${OBJTOP}/tmp \
		-B${OBJTOP}/tmp/usr/bin -I${SRCTOP}/Frameworks \
		-I${OBJTOP}/Frameworks/Foundation/Headers

LDFLAGS+=	-L${OBJTOP}/Frameworks/Foundation/Foundation.framework \
		-L${OBJTOP}/Frameworks/LaunchServices/LaunchServices.framework \
		-Wl,-R/System/Library/Frameworks/Foundation.framework/Versions/A \
		-Wl,-R/System/Library/Frameworks/LaunchServices.framework/Versions/A \
		-lFoundation -lLaunchServices -lobjc


.include <rvn.app.mk>

all:
	cp -fv ${.CURDIR}/Info.plist ${APP_DIR}/Contents/
