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
CFLAGS+= -fobjc-runtime=gcc \
	 -I.. -I../libobjc2 \
	 -O0 -g \
	 -F${BUILDROOT}/System/Library/Frameworks -framework Foundation

.include <he.framework.mk>
