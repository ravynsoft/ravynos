THISDIR:= ${.CURDIR}

all: clean build
clean:
	rm -rf ${BUILDROOT}/System/Library/CoreServices
	rm -rf /usr/obj/${TOPDIR}/${MACHINE}.${MACHINE_CPUARCH}/CoreServices/Dock
	rm -rf /usr/obj/${TOPDIR}/${MACHINE}.${MACHINE_CPUARCH}/CoreServices/WindowServer

DMIHelper: ${.CURDIR}/DMIHelper.c
	mkdir -p ${BUILDROOT}/System/Library/CoreServices
	$(CC) -o ${BUILDROOT}/System/Library/CoreServices/DMIHelper ${.CURDIR}/DMIHelper.c
	${SUDO} chown 0:0 ${BUILDROOT}/System/Library/CoreServices/DMIHelper
	${SUDO} chmod 4755 ${BUILDROOT}/System/Library/CoreServices/DMIHelper

build: buildWindowServer DMIHelper buildDock buildFiler
	mkdir -p ${BUILDROOT}/System/Library/CoreServices/
	mkdir -p ${BUILDROOT}/Applications/Utilities/
	cp -av ${OBJTOP}/CoreServices/WindowServer/WindowServer.app ${BUILDROOT}/System/Library/CoreServices
	cp -av ${OBJTOP}/CoreServices/Dock/Dock.app ${BUILDROOT}/System/Library/CoreServices
	cp -av ${OBJTOP}/CoreServices/Filer/Filer.app ${BUILDROOT}/System/Library/CoreServices
.for f in ${.CURDIR}/askpass 
	cp -fv ${f} ${BUILDROOT}/System/Library/CoreServices/
	chmod 755 ${BUILDROOT}/System/Library/CoreServices/$$(basename ${f})
.endfor

buildFiler:
	${MAKE} -C ${.CURDIR}/Filer obj all

buildDock:
	mkdir -p ${MAKEOBJDIR}/Dock/Trash
	mkdir -p ${MAKEOBJDIR}/Dock/Downloads
	${MAKE} -C ${.CURDIR}/Dock obj all

buildWindowServer:
	${MAKE} -C ${.CURDIR}/WindowServer make-obj-dirs obj all
	${MAKE} -C ${.CURDIR}/WindowServer/SystemUIServer
	${MAKE} -C ${.CURDIR}/WindowServer/LoadingWindow
	${MAKE} -C ${.CURDIR}/WindowServer/LoginWindow
	cp -afv ${OBJTOP}/CoreServices/WindowServer/SystemUIServer/SystemUIServer.app \
	        ${OBJTOP}/CoreServices/WindowServer/LoadingWindow/LoadingWindow.app \
	        ${OBJTOP}/CoreServices/WindowServer/LoginWindow/LoginWindow.app \
                ${OBJTOP}/CoreServices/WindowServer/WindowServer.app/Contents/Resources/

