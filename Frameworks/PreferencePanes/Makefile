FRAMEWORK=PreferencePanes
.include "../framework.common.mk"

WARNS= 0
SRCS= NSPreferencePane.m
INCS= NSPreferencePane.h
RESOURCES= Info.plist 
CFLAGS+= ${FMWK_CFLAGS} -fobjc-arc -I${OBJTOP}/Frameworks/Foundation/Headers \
	 -I${SRCTOP}/Frameworks/AppKit/Headers/
LDFLAGS+= -L${OBJTOP}/tmp/usr/lib 

.include <rvn.framework.mk>

installresources: std_install_hook
