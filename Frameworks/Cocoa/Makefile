FRAMEWORK=Cocoa
.include "../framework.common.mk"

SRCS=Cocoa.m
INCS=Cocoa.h

RESOURCES=Info.plist
CFLAGS+= ${FMWK_CFLAGS} -I../Foundation/Headers \
	-I${.CURDIR}/../AppKit/Headers -F../OpenGL \
	-I${SRCTOP}/CoreServices -framework OpenGL
LDFLAGS+= -L${OBJTOP}/Frameworks/AppKit/AppKit.framework \
	-L${OBJTOP}/Frameworks/Foundation/Foundation.framework \
	-L${OBJTOP}/Frameworks/CoreFoundation/CoreFoundation.framework \
        -lAppKit -lFoundation -lCoreFoundation -lobjc -lmach \
        -Wl,-R/System/Library/Frameworks/AppKit.framework/Versions/A \
        -Wl,-R/System/Library/Frameworks/Foundation.framework/Versions/A \
        -Wl,-R/System/Library/Frameworks/CoreFoundation.framework/Versions/A 

.include <rvn.framework.mk>

installresources: std_install_hook
