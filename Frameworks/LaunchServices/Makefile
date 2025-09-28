FRAMEWORK=LaunchServices
.include "../framework.common.mk"

WARNS= 0
SRCS= LaunchServices.mm LSAppRecord.mm UTTypes.mm LSIPC.mm
INCS= LaunchServices.h UTCoreTypes.h UTTypes.h
RESOURCES= Info.plist InitDB.sql DBSchema_3_4.sql
CFLAGS+= ${FMWK_CXXFLAGS} -fobjc-arc \
	 -I${OBJTOP}/Frameworks/Foundation/Headers
LDFLAGS+= -L${OBJTOP}/tmp/usr/lib \
	  -lsqlite3 -llaunch -lmach

.include <rvn.framework.mk>

installresources: std_install_hook
