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
CFLAGS+= -I.. -F${BUILDROOT}/System/Library/Frameworks -framework Foundation

.include <he.framework.mk>
