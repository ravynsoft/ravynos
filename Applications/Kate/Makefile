APP=Kate
SRCS=Kate.mm
MK_DEBUG_FILES=no
RESOURCES=
SLF=/System/Library/Frameworks
FRAMEWORKS=${SLF}/Foundation

KATE_DIR= kate-21.08.1

RESOURCES_DIR= ${.CURDIR}/${APP_DIR}/Contents/Resources

build: clean all kate install
kate:
	mkdir -p ${KATE_DIR}/_build
	cmake -B ${KATE_DIR}/_build -S ${KATE_DIR} \
		-DCMAKE_INSTALL_PREFIX=/Resources \
		-DCMAKE_INSTALL_BINDIR=/Airyx -DCMAKE_INSTALL_SBINDIR=. \
		-DCMAKE_INSTALL_LIBEXECDIR=. -DCMAKE_INSTALL_LIBDIR=. \
		-DCMAKE_INSTALL_DATAROOTDIR=. -DAPPLE=1 \
		-DMACOSX_BUNDLE_ICON_FILE="Kate.png" \
		-DCMAKE_INSTALL_RPATH="\$$ORIGIN/../Resources"
	make -C ${KATE_DIR}/_build

install:
	make -C ${KATE_DIR}/_build DESTDIR=${.CURDIR}/${APP_DIR}/Contents install
	mv ${APP_DIR}/Contents/Applications/KDE/kate.app/Contents/Info.plist \
		${APP_DIR}/Contents
	mv ${APP_DIR}/Contents/Applications/KDE/kate.app/Contents/MacOS/kate \
		${APP_DIR}/Contents/Airyx/
	mv ${APP_DIR}/Resources/icons/hicolor/512x512/apps/kate.png ${APP_DIR}/Resources/Kate.png
	rm -rf ${APP_DIR}/Contents/Applications ${APP_DIR}/Resources/applications \
		${APP_DIR}/Resources/icons
	mkdir -p ${BUILDROOT}/Applications
	rm -rf ${BUILDROOT}/Applications/${APP_DIR}
	cp -a ${.CURDIR}/${APP_DIR} ${BUILDROOT}/Applications/

clean:
	rm -rf ${KATE_DIR}/_build
	rm -rf ${.CURDIR}/${APP_DIR}
	rm -f ${APP}.o

.include <airyx.app.mk>
