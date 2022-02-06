APP=Install_airyxOS
SRCS=Source/main.m Source/AppDelegate.m Source/GSGeomDisk.m
MK_DEBUG_FILES=no
RESOURCES=MarmosetLogo.tiff header.rtf terms.rtf English.lproj Icon.png
SLF=/System/Library/Frameworks
FRAMEWORKS=${SLF}/AppKit ${SLF}/CoreGraphics ${SLF}/Onyx2D ${SLF}/OpenGL \
	${SLF}/Foundation ${SLF}/CoreFoundation ${SLF}/DBusKit
LDFLAGS+=-L/usr/lib -lGL --sysroot=${BUILDROOT}/../../../tmp/
AIRYX_VERSION!= head -1 ${.CURDIR}/../../version.txt

do:
	echo ${CFLAGS}

clean:
	rm -rf ${APP_DIR} "${APP_DIR:S/_/ /}"
	rm -f Source/*.o

build: clean all
	rm -f ${APP_DIR}/${APP}
	ln -sf "Contents/Airyx/${APP:S/_/ /}" "${APP_DIR}/${APP:S/_/ /}"
	mv -f ${APP_DIR}/Contents/Airyx/${APP} "${APP_DIR}/Contents/Airyx/${APP:S/_/ /}"
	mv -f ${APP_DIR} "${APP_DIR:S/_/ /}"

.include <airyx.app.mk>
CFLAGS:= ${CFLAGS:S/-Werror//g} -g --sysroot=${BUILDROOT} \
	 -I${BUILDROOT}/../../../tmp/usr/include \
	 -DAIRYX_VERSION=\"${AIRYX_VERSION}\"

