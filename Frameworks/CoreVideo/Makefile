FRAMEWORK=CoreVideo
.include "../framework.common.mk"

SRCS= CVDisplayLink.m
INCS= CVBase.h CVDisplayLink.h CVReturn.h CoreVideo.h
RESOURCES=Info.plist 
CFLAGS+= ${FMWK_CFLAGS} -F../OpenGL -framework OpenGL -I${.CURDIR}/.. \
	-I../libobjc2 -I../Foundation/Headers

.include <rvn.framework.mk>

installresources: std_install_hook
