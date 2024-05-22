FRAMEWORK=CoreText
.include "../framework.common.mk"

SRCS= CTFont.m KTFont.m
INCS= CoreText.h CoreTextExport.h CTFont.h KTFont.h
RESOURCES=Info.plist 
LDFLAGS+= -Wl,--no-undefined -L${OBJTOP}/Frameworks/Onyx2D/Onyx2D.framework \
	-lOnyx2D -Wl,-R/System/Library/Frameworks/Onyx2D.framework/Versions/A \
	-L${OBJTOP}/Frameworks/CoreGraphics/CoreGraphics.framework -lCoreGraphics \
	-Wl,-R/System/Library/Frameworks/CoreGraphics.framework/Versions/A \
	-L${OBJTOP}/Frameworks/Foundation/Foundation.framework -lFoundation \
	-Wl,-R/System/Library/Frameworks/Foundation.framework/Versions/A \
	-L${OBJTOP}/tmp/usr/lib -lxml2 -lobjc
WARNS= 1
MK_WERROR= no

SUBDIR+=	gperf .WAIT
SUBDIR+=	fontconfig
LINK_SUBDIR=	fontconfig/src
LINK_EXCLUDE.fontconfig/src= fcarch.o

clean:
	rm -f ${MAKEOBJDIR}/*.o ${MAKEOBJDIR}/*.pico

.include <rvn.framework.mk>

CFLAGS+= ${FMWK_CFLAGS} -I.. -I../libobjc2 -I../Foundation/Headers \
	 -I${.CURDIR}/.. -I${.CURDIR}/fontconfig/fontconfig

installresources: std_install_hook
