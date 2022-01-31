THISDIR:= ${.CURDIR}
EXTRA_CXXFLAGS= -I/usr/include/gio-unix-2.0

all: clean build
clean:
	rm -rf ${BUILDROOT}/System/Library/CoreServices
	rm -rf ${BUILDROOT}/Applications/Utilities/"Install airyxOS.app"
	rm -rf Filer/build Filer/Filer.app Installer/"Install airyxOS.app"
	rm -rf /usr/obj/${TOPDIR}/${MACHINE}.${MACHINE}/CoreServices/Filer

DMIHelper: DMIHelper.c
	mkdir -p ${BUILDROOT}/System/Library/CoreServices
	$(CC) -o ${BUILDROOT}/System/Library/CoreServices/DMIHelper DMIHelper.c
	${SUDO} chown 0:0 ${BUILDROOT}/System/Library/CoreServices/DMIHelper
	${SUDO} chmod 4755 ${BUILDROOT}/System/Library/CoreServices/DMIHelper

build: buildFiler DMIHelper buildInstaller
	mkdir -p ${BUILDROOT}/System/Library/CoreServices/
	mkdir -p ${BUILDROOT}/Applications/Utilities/
	cp -av Filer/Filer.app ${BUILDROOT}/System/Library/CoreServices
	cp -av Installer/"Install airyxOS.app" ${BUILDROOT}/Applications/Utilities/
.for f in askpass enable_tapping tpquirk startDesktop
		cp -fv ${f} ${BUILDROOT}/System/Library/CoreServices/
		chmod 755 ${BUILDROOT}/System/Library/CoreServices/${f}
.endfor

buildInstaller:
	${MAKE} -C Installer build

${.CURDIR}/Filer:
	git clone https://github.com/mszoek/Filer.git
	cd Filer && git checkout airyx

buildFiler: ${.CURDIR}/Filer
	mkdir -p Filer/build/src
	${MAKE} -C Filer build
	cmake -S ${.CURDIR}/Filer -B ${.CURDIR}/Filer/build \
		-DCMAKE_BUILD_TYPE="Debug" \
		-DAIRYX=1 -DCMAKE_CXX_FLAGS="${EXTRA_CXXFLAGS}" \
		-DSYSTEM_FRAMEWORKS="-framework LaunchServices -framework Foundation -framework CoreFoundation -framework CFNetwork -lthr"
	${MAKE} -C Filer/build -j${CORES}
	${MAKE} -C Filer/build DESTDIR=${.CURDIR}/Filer/Filer.app install
	cd ${.CURDIR}/Filer/Filer.app/Contents/Resources && mv -fv filer-qt.1 Filer.1
