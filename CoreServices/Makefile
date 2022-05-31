THISDIR:= ${.CURDIR}
EXTRA_CXXFLAGS= -I/usr/include/gio-unix-2.0

all: clean build
clean:
	rm -rf ${BUILDROOT}/System/Library/CoreServices
	rm -rf ${BUILDROOT}/Applications/Utilities/"Install ravynOS.app"
	rm -rf Filer/build Filer/Filer.app Installer/"Install ravynOS.app"
	rm -rf /usr/obj/${TOPDIR}/${MACHINE}.${MACHINE}/CoreServices/Dock
	rm -rf /usr/obj/${TOPDIR}/${MACHINE}.${MACHINE}/CoreServices/Filer
	rm -rf /usr/obj/${TOPDIR}/${MACHINE}.${MACHINE}/CoreServices/WindowServer

DMIHelper: ${.CURDIR}/DMIHelper.c
	mkdir -p ${BUILDROOT}/System/Library/CoreServices
	$(CC) -o ${BUILDROOT}/System/Library/CoreServices/DMIHelper ${.CURDIR}/DMIHelper.c
	${SUDO} chown 0:0 ${BUILDROOT}/System/Library/CoreServices/DMIHelper
	${SUDO} chmod 4755 ${BUILDROOT}/System/Library/CoreServices/DMIHelper

build: buildWindowServer DMIHelper buildInstaller #buildDock buildFiler 
	mkdir -p ${BUILDROOT}/System/Library/CoreServices/
	mkdir -p ${BUILDROOT}/Applications/Utilities/
	cp -av ${.CURDIR}/WindowServer/WindowServer.app ${BUILDROOT}/System/Library/CoreServices
	#cp -av ${.CURDIR}/Dock/Dock.app ${BUILDROOT}/System/Library/CoreServices
	#cp -av ${.CURDIR}/Filer/Filer.app ${BUILDROOT}/System/Library/CoreServices
	cp -av ${.CURDIR}/Installer/"Install ravynOS.app" ${BUILDROOT}/Applications/Utilities/
.for f in ${.CURDIR}/askpass 
		cp -fv ${f} ${BUILDROOT}/System/Library/CoreServices/
		chmod 755 ${BUILDROOT}/System/Library/CoreServices/$$(basename ${f})
.endfor

buildDock:
	#${MAKE} -C ${.CURDIR}/Dock

buildInstaller:
	${MAKE} -C ${.CURDIR}/Installer build

buildWindowServer:
	${MAKE} -C ${.CURDIR}/WindowServer
	${MAKE} -C ${.CURDIR}/WindowServer/Desktop
	cp -afv ${.CURDIR}/WindowServer/Desktop/SystemUIServer.app \
                ${.CURDIR}/WindowServer/WindowServer.app/Contents/Resources/

${.CURDIR}/Filer:
	git clone https://github.com/mszoek/Filer.git ${.CURDIR}/Filer
	cd ${.CURDIR}/Filer && git checkout airyx

buildFiler: ${.CURDIR}/Filer
	mkdir -p ${.CURDIR}/Filer/build/src
	${MAKE} -C ${.CURDIR}/Filer build
	cmake -S ${.CURDIR}/Filer -B ${.CURDIR}/Filer/build \
		-DCMAKE_BUILD_TYPE="Debug" \
		-DRAVYNOS=1 -DCMAKE_CXX_FLAGS="${EXTRA_CXXFLAGS}" \
		-DSYSTEM_FRAMEWORKS="-framework LaunchServices -framework Foundation -framework CoreFoundation -framework CFNetwork -lthr -lobjc"
	${MAKE} -C ${.CURDIR}/Filer/build -j${CORES}
	${MAKE} -C ${.CURDIR}/Filer/build DESTDIR=${.CURDIR}/Filer/Filer.app install
	cd ${.CURDIR}/Filer/Filer.app/Contents/Resources && mv -fv filer-qt.1 Filer.1
