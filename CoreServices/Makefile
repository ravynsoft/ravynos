THISDIR:= ${.CURDIR}

all: clean build
clean:
	rm -rf ${BUILDROOT}/System/Library/CoreServices
	rm -rf Filer/build Filer/Filer.app

build: ${.CURDIR}/Menu buildFiler
	mkdir -p ${BUILDROOT}/System/Library/CoreServices
	cp -av Filer/Filer.app ${BUILDROOT}/System/Library/CoreServices

${.CURDIR}/Filer:
	git clone https://github.com/mszoek/Filer.git
	cd Filer && git checkout airyx

buildFiler: ${.CURDIR}/Filer
	mkdir -p Filer/build/src
	${MAKE} -C Filer build
	cmake -S ${.CURDIR}/Filer -B ${.CURDIR}/Filer/build
	${MAKE} -C Filer/build -j${CORES}
	${MAKE} -C Filer/build DESTDIR=${.CURDIR}/Filer/Filer.app install
	cd ${.CURDIR}/Filer/Filer.app/Contents/Resources && mv -fv filer-qt.1 Filer.1

${.CURDIR}/Menu:
	git clone https://github.com/mszoek/Menu.git
