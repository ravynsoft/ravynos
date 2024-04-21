FRAMEWORK=CoreText
.include "../framework.common.mk"

SRCS= CTFont.m KTFont.m
INCS= CoreText.h CoreTextExport.h CTFont.h KTFont.h
RESOURCES=Info.plist 
CFLAGS+= ${FMWK_CFLAGS} -I.. -I../libobjc2 -I../Foundation/Headers -I${.CURDIR}/..

.include <rvn.framework.mk>

installresources: std_install_hook
