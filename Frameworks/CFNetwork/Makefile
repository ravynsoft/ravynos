.include "../../mk/internal/framework.common.mk"

FRAMEWORK=CFNetwork
SRCS= \
	CFHost.m \
	CFNetServices.m \
	CFSocketStream.m

INCS= \
	CFHost.h \
	CFNetServices.h \
	CFNetwork.h \
	CFNetworkExport.h \
	CFSocketStream.h

RESOURCES=Info.plist English.lproj
CFLAGS+= ${FMWK_CFLAGS} -I.. -I../libobjc2 -I../Foundation/Headers \
	-F${BUILDROOT}/System/Library/Frameworks -framework CoreFoundation
LDFLAGS+=-L${BUILDROOT}/System/Library/Frameworks/CoreFoundation.framework/Versions/Current \
	-lCoreFoundation \
	-Wl,-R/System/Library/Frameworks/CoreFoundation.framework/Versions/A

install:
	rm -rf ${BUILDROOT}/System/Library/Frameworks/${FRAMEWORK}.framework
	cp -av ${.CURDIR}/${FRAMEWORK}.framework ${BUILDROOT}/System/Library/Frameworks/
_libinstall: install

.include <airyx.framework.mk>
