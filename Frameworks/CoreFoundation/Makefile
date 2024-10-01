FRAMEWORK=CoreFoundation
.include "../framework.common.mk"

SRCS= \
	CFArray.m \
	CFAttributedString.m \
	CFBag.m \
	CFBase.m \
	CFBinaryHeap.m \
	CFBitVector.m \
	CFBundle.m \
	CFByteOrder.m \
	CFCalendar.m \
	CFCharacterSet.m \
	CFData.m \
	CFDate.m \
	CFDateFormatter.m \
	CFDictionary.m \
	CFError.m \
	CFFileDescriptor.m \
	CFLocale.m \
	CFMachPort.m \
	CFMessagePort.m \
	CFNotificationCenter.m \
	CFNumber.m \
	CFNumberFormatter.m \
	CFPlugIn.m \
	CFPreferences.m \
	CFPropertyList.m \
	CFRunLoop.m \
	CFSet.m \
	CFSocket.m \
	CFStream.m \
	CFString.m \
	CFStringTokenizer.m \
	CFTimeZone.m \
	CFTree.m \
	CFURL.m \
	CFUserNotification.m \
	CFUUID.m \
	CFXMLNode.m \
	CFXMLParser.m

INCS= \
	CFArray.h \
	CFAttributedString.h \
	CFBag.h \
	CFBase.h \
	CFBinaryHeap.h \
	CFBitVector.h \
	CFBundle.h \
	CFByteOrder.h \
	CFCalendar.h \
	CFCharacterSet.h \
	CFData.h \
	CFDate.h \
	CFDateFormatter.h \
	CFDictionary.h \
	CFError.h \
	CFFileDescriptor.h \
	CFLocale.h \
	CFMachPort.h \
	CFMessagePort.h \
	CFNotificationCenter.h \
	CFNumber.h \
	CFNumberFormatter.h \
	CFPlugIn.h \
	CFPreferences.h \
	CFPropertyList.h \
	CFRunLoop.h \
	CFSet.h \
	CFSocket.h \
	CFStream.h \
	CFString.h \
	CFStringEncodingExt.h \
	CFStringTokenizer.h \
	CFTimeZone.h \
	CFTree.h \
	CFURL.h \
	CFUserNotification.h \
	CFUUID.h \
	CFXMLNode.h \
	CFXMLParser.h \
	CoreFoundation.h

RESOURCES=Info.plist English.lproj
CFLAGS+= ${FMWK_CFLAGS} -I${.CURDIR}/.. -I../libobjc2 -I../Foundation/Headers \
	 -F${BUILDROOT}/System/Library/Frameworks -framework Foundation
LDFLAGS+= ${FMWK_LDFLAGS} -lmach

.include <rvn.framework.mk>

installresources: std_install_hook
