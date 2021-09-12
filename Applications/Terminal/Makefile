APP=Terminal
SRCS=Terminal.mm
MK_DEBUG_FILES=no
RESOURCES=qterminal.ini
SLF=/System/Library/Frameworks
FRAMEWORKS=${SLF}/Foundation

# Source: https://github.com/lxqt/qtermwidget
QTERMWIDGET_DIR= qtermwidget-0.17.0

# Source: https://github.com/lxqt/qterminal
QTERMINAL_DIR= qterminal-0.17.0

RESOURCES_DIR= ${.CURDIR}/${APP_DIR}/Contents/Resources

build: clean all widget terminal install
widget:
	mkdir -p ${QTERMWIDGET_DIR}/_build
	cmake -B ${QTERMWIDGET_DIR}/_build -S ${QTERMWIDGET_DIR} \
		-DQTERMWIDGET_USE_UTEMPTER=ON \
		-DCMAKE_INSTALL_PREFIX=/Resources \
		-DCMAKE_INSTALL_BINDIR=. -DCMAKE_INSTALL_SBINDIR=. \
		-DCMAKE_INSTALL_LIBEXECDIR=. -DCMAKE_INSTALL_LIBDIR=. \
		-DCMAKE_INSTALL_DATAROOTDIR=.
	make -C ${QTERMWIDGET_DIR}/_build

terminal:
	mkdir -p ${QTERMINAL_DIR}/_build
	QTermWidget5_DIR=${QTERMWIDGET_DIR}/_build \
		cmake -B ${QTERMINAL_DIR}/_build -S ${QTERMINAL_DIR} \
		-DCMAKE_INSTALL_PREFIX=/Resources \
		-DCMAKE_INSTALL_BINDIR=/Airyx -DCMAKE_INSTALL_SBINDIR=. \
		-DCMAKE_INSTALL_LIBEXECDIR=. -DCMAKE_INSTALL_LIBDIR=. \
		-DCMAKE_INSTALL_DATAROOTDIR=. -DCMAKE_CXX_FLAGS="-DAPPLE_BUNDLE" \
		-DCMAKE_INSTALL_RPATH="\$$ORIGIN/../Resources"
	make -C ${QTERMINAL_DIR}/_build

install:
	make -C ${QTERMWIDGET_DIR}/_build DESTDIR=${.CURDIR}/${APP_DIR}/Contents install
	make -C ${QTERMINAL_DIR}/_build DESTDIR=${.CURDIR}/${APP_DIR}/Contents install
	rm -rf ${RESOURCES_DIR}/app* ${RESOURCES_DIR}/libdata
	rm -rf ${RESOURCES_DIR}/cmake ${RESOURCES_DIR}/include
	mv -f ${RESOURCES_DIR}/qtermwidget5/* ${RESOURCES_DIR}/
	mv -f ${RESOURCES_DIR}/qterminal/translations/* ${RESOURCES_DIR}/translations
	rm -rf ${RESOURCES_DIR}/qtermwidget5
	rm -rf ${RESOURCES_DIR}/qterminal
	mv -f ${RESOURCES_DIR}/icons/hicolor/64x64/apps/qterminal.png \
		${RESOURCES_DIR}/Terminal.png
	rm -rf ${RESOURCES_DIR}/icons
	mkdir -p ${BUILDROOT}/Applications/Utilities/
	rm -rf ${BUILDROOT}/Applications/Utilities/${APP_DIR}
	cp -a ${.CURDIR}/${APP_DIR} ${BUILDROOT}/Applications/Utilities/

clean:
	rm -rf ${QTERMINAL_DIR}/_build ${QTERMWIDGET_DIR}/_build
	rm -rf ${.CURDIR}/${APP_DIR}
	rm -f ${APP}.o

.include <airyx.app.mk>
