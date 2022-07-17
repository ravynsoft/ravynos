APP=Install_ravynOS
SRCS=Source/main.m Source/AppDelegate.m Source/GSGeomDisk.m
MK_DEBUG_FILES=no
RESOURCES=${.CURDIR}/MarmosetLogo.tiff ${.CURDIR}/header.rtf ${.CURDIR}/terms.rtf \
        ${.CURDIR}/English.lproj ${.CURDIR}/Icon.png
SLF=/System/Library/Frameworks
FRAMEWORKS=${SLF}/AppKit ${SLF}/CoreGraphics ${SLF}/Onyx2D ${SLF}/OpenGL \
	${SLF}/Foundation ${SLF}/CoreFoundation ${SLF}/DBusKit
#LDFLAGS+=-L/usr/lib -lGL --sysroot=${BUILDROOT}/../../../tmp/
LDFLAGS+=-L/usr/lib -lGL
RAVYNOS_VERSION!= head -1 ${.CURDIR}/../../version.txt

do:
	echo ${CFLAGS}

clean:
	rm -rf ${APP_DIR} "${APP_DIR:S/_/ /}"
	rm -f Source/*.o

build: clean all
	rm -f ${APP_DIR}/${APP}
	ln -sf "Contents/ravynOS/${APP:S/_/ /}" "${APP_DIR}/${APP:S/_/ /}"
	mv -f ${APP_DIR}/Contents/ravynOS/${APP} "${APP_DIR}/Contents/ravynOS/${APP:S/_/ /}"
	mv -f ${APP_DIR} "${APP_DIR:S/_/ /}"

.include <rvn.app.mk>
CFLAGS:= ${CFLAGS:S/-Werror//g} -g -DRAVYNOS_VERSION=\"${RAVYNOS_VERSION}\"
#CFLAGS:= ${CFLAGS:S/-Werror//g} -g --sysroot=${BUILDROOT} \
	 -I${BUILDROOT}/../../../tmp/usr/include \
	 -DRAVYNOS_VERSION=\"${RAVYNOS_VERSION}\"

