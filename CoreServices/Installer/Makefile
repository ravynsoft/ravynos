APP=Install_airyxOS
SRCS=Source/main.m Source/AppDelegate.m
MK_DEBUG_FILES=no
RESOURCES=TanukiLogo.jpg terms.rtf English.lproj
SLF=/System/Library/Frameworks
FRAMEWORKS=${SLF}/AppKit ${SLF}/CoreGraphics ${SLF}/Onyx2D ${SLF}/OpenGL \
	${SLF}/Foundation ${SLF}/CoreFoundation ${SLF}/DBusKit
LDFLAGS+=-lGL
AIRYX_VERSION!= head -1 ../../version.txt
AIRYX_CODENAME!= tail -1 ../../version.txt

clean:
	rm -rf ${APP_DIR} "${APP_DIR:S/_/ /}"
	rm -f *.o

build: clean all
	sed -i_ -e 's@%C@${AIRYX_CODENAME}@' -e 's@%v@${AIRYX_VERSION}@' "${APP_DIR}/Contents/Resources/terms.rtf"
	rm -f "${APP_DIR}/Contents/Resources/terms.rtf_"
	rm -f ${APP_DIR}/${APP}
	ln -sf "Contents/Airyx/${APP:S/_/ /}" "${APP_DIR}/${APP:S/_/ /}"
	mv -f ${APP_DIR}/Contents/Airyx/${APP} "${APP_DIR}/Contents/Airyx/${APP:S/_/ /}"
	mv -f ${APP_DIR} "${APP_DIR:S/_/ /}"

.include <airyx.app.mk>
