.include "../framework.common.mk"

FRAMEWORK=CoreText
SRCS= CTFont.m KTFont.m
INCS= CoreText.h CoreTextExport.h CTFont.h KTFont.h
RESOURCES=Info.plist 
CFLAGS+= ${FMWK_CFLAGS} -I.. -I../libobjc2 -I../Foundation/Headers

_install-hook:
	rm -rf ${BUILDROOT}/System/Library/Frameworks/${FRAMEWORK}.framework
	cp -av ${.CURDIR}/${FRAMEWORK}.framework ${BUILDROOT}/System/Library/Frameworks/

.include <rvn.framework.mk>

installresources: _install-hook
