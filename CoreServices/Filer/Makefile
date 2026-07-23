APP=	Filer
SRCS=	FilerMain.m
RESOURCES= ${.CURDIR}/filer.png

MK_WERROR=	no
MK_PIE=		no
WARNS=		1
CFLAGS+=	-g -O0 -fobjc-arc --sysroot=${OBJTOP}/tmp -D__MACH__ -fPIC \
		-B${OBJTOP}/tmp/usr/bin -D__RAVYNOS__ \
		-I${SRCTOP}/Frameworks -I${SRCTOP}/Frameworks/AppKit/Headers \
		-I${OBJTOP}/Frameworks/Foundation/Headers

LDFLAGS+=	-L${OBJTOP}/Frameworks/Foundation/Foundation.framework \
		-L${OBJTOP}/Frameworks/CoreServices/LaunchServices \
		-L${OBJTOP}/Frameworks/CoreGraphics/CoreGraphics.framework \
		-L${OBJTOP}/Frameworks/AppKit/AppKit.framework \
		-L${OBJTOP}/tmp/usr/lib \
		-Wl,-R/System/Library/Frameworks/Foundation.framework/Versions/A \
		-Wl,-R/System/Library/Frameworks/AppKit.framework/Versions/A \
		-Wl,-R/System/Library/Frameworks/CoreGraphics.framework/Versions/A \
		-lAppKit -lCoreGraphics -lLaunchServices -lFoundation -lobjc -lSystem

.include <rvn.app.mk>

all: obj
