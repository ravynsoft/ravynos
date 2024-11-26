APP= System_Preferences
SRCS= main.m SysPrefs.m
MK_DEBUG_FILES= no
MK_WERROR= no
RESOURCES= ${.CURDIR}/Info.plist Icon.png

WARNS= 1
CFLAGS= -g -fobjc-arc --sysroot=${OBJTOP}/tmp -B${OBJTOP}/tmp/usr/bin \
	-Wno-missing-method-return-type \
	-I${SRCTOP}/Frameworks -I${OBJTOP}/Frameworks/Foundation/Headers \
	-I${SRCTOP}/Frameworks/AppKit/Headers -F${OBJTOP}/Frameworks/OpenGL
LDFLAGS= -L${OBJTOP}/Frameworks/AppKit/AppKit.framework \
	 -L${OBJTOP}/Frameworks/Foundation/Foundation.framework \
	 -L${OBJTOP}/Frameworks/PreferencePanes/PreferencePanes.framework \
	 -Wl,-R/System/Library/Frameworks/AppKit.framework/Versions/A \
	 -Wl,-R/System/Library/Frameworks/PreferencePanes.framework/Versions/A \
	 -Wl,-R/System/Library/Frameworks/Foundation.framework/Versions/A \
	 -lPreferencePanes -lAppKit -lFoundation -lobjc -lSystem

clean:
	rm -rf ${APP_DIR} "${APP_DIR:S/_/ /}"
	rm -f *.o

makedirs:
	mkdir -p ${MAKEOBJDIR}

build: clean makedirs obj all
	rm -f ${APP_DIR}/${APP}
	ln -sf "Contents/ravynOS/${APP:S/_/ /}" "${APP_DIR}/${APP:S/_/ /}"
	mv -f ${APP_DIR}/Contents/ravynOS/${APP} "${APP_DIR}/Contents/ravynOS/${APP:S/_/ /}"
	mv -f ${APP_DIR} "${APP_DIR:S/_/ /}"

.include <rvn.app.mk>
