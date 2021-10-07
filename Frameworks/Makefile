THISDIR:= ${.CURDIR}
CIRRUS_BUILD_ID?= local

all: .PHONY clean frameworks package
package:
	rm -rf /tmp/meta
	mkdir -p /tmp/meta
	cd ${BUILDROOT}; find ./System/Library/Frameworks -not -type d | \
	  sed 's@^\.@@' > /tmp/meta/pkg-plist
	sed "s/%%VERSION%%/${AIRYX_VERSION}_${CIRRUS_BUILD_ID}/" < +MANIFEST \
	  > /tmp/meta/+MANIFEST
	mkdir -p ${TOPDIR}/dist
	pkg create -o ${TOPDIR}/dist/ -r ${BUILDROOT} -m /tmp/meta \
	  --plist /tmp/meta/pkg-plist
	ls -l ${TOPDIR}/dist

clean:
	rm -rf ${BUILDROOT}/System/Library/Frameworks/*.framework
	for fmwk in ${.ALLTARGETS:M*.framework:R}; do \
		${MAKE} ${MKINCDIR} -C ${THISDIR}/$$fmwk clean; \
		rm -rf ${THISDIR}/$$fmwk/$$fmwk.framework; \
	done
	rm -rf Foundation/Headers
	rm -rf ${OBJPREFIX}/libobjc2

_FRAMEWORK_TARGETS=
.if defined(FRAMEWORKS) && !empty(FRAMEWORKS)
.for fmwk in ${FRAMEWORKS}
_FRAMEWORK_TARGETS+=${fmwk}.framework
.endfor
.else
_FRAMEWORK_TARGETS=${.ALLTARGETS:M*.framework}
.endif
frameworks: 
	for fmwk in ${_FRAMEWORK_TARGETS}; do \
		${MAKE} ${MKINCDIR} -C ${THISDIR} $$fmwk; done

libobjc2: .PHONY
	mkdir -p ${OBJPREFIX}/libobjc2
	cd ${OBJPREFIX}/libobjc2; cmake \
		-DCMAKE_C_FLAGS=" -D__AIRYX__ -DNO_SELECTOR_MISMATCH_WARNINGS" \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_INSTALL_PREFIX=/usr \
		-DOLDABI_COMPAT=false -DLEGACY_COMPAT=false \
		${THISDIR}/libobjc2
	${MAKE} -C ${OBJPREFIX}/libobjc2 DESTDIR=${BUILDROOT} install

libunwind: .PHONY
	cd ${THISDIR}/libunwind-1.5.0 && ./configure --prefix=/usr --enable-coredump --enable-ptrace --enable-cxx-exceptions \
		--enable-block-signals --enable-debug-frame && ${MAKE} -j${CORES}
	${MAKE} -C ${THISDIR}/libunwind-1.5.0 install prefix=${BUILDROOT}/usr

marshallheaders:
	${MAKE} -C ${THISDIR}/Foundation marshallheaders

# DO NOT change the order of these 4 frameworks!
CoreFoundation.framework: marshallheaders
	rm -rf ${THISDIR}/CoreFoundation/${.TARGET}
	${MAKE} -C ${THISDIR}/CoreFoundation BUILDROOT=${BUILDROOT} clean
	${MAKE} -C ${THISDIR}/CoreFoundation BUILDROOT=${BUILDROOT}
	cp -Rvf ${THISDIR}/${.TARGET:R}/${.TARGET} ${BUILDROOT}/System/Library/Frameworks

CFNetwork.framework:
	rm -rf ${THISDIR}/CFNetwork/${.TARGET}
	${MAKE} -C ${THISDIR}/CFNetwork BUILDROOT=${BUILDROOT} clean
	${MAKE} -C ${THISDIR}/CFNetwork BUILDROOT=${BUILDROOT}
	cp -Rvf ${THISDIR}/${.TARGET:R}/${.TARGET} ${BUILDROOT}/System/Library/Frameworks

Foundation.framework: libobjc2 
	rm -rf ${THISDIR}/Foundation/${.TARGET}
	${MAKE} -C ${THISDIR}/Foundation BUILDROOT=${BUILDROOT} clean build
	cp -Rvf ${THISDIR}/${.TARGET:R}/${.TARGET} ${BUILDROOT}/System/Library/Frameworks
	cp -vf ${THISDIR}/${.TARGET:R}/NSException/NSRaise.h ${THISDIR}/AppKit

ApplicationServices.framework:
	rm -rf ${THISDIR}/ApplicationServices/${.TARGET}
	${MAKE} -C ${THISDIR}/ApplicationServices BUILDROOT=${BUILDROOT} clean
	${MAKE} -C ${THISDIR}/ApplicationServices BUILDROOT=${BUILDROOT}
	cp -Rvf ${THISDIR}/${.TARGET:R}/${.TARGET} ${BUILDROOT}/System/Library/Frameworks

CoreServices.framework:
	rm -rf ${THISDIR}/CoreServices/${.TARGET}
	${MAKE} -C ${THISDIR}/CoreServices BUILDROOT=${BUILDROOT} clean
	${MAKE} -C ${THISDIR}/CoreServices BUILDROOT=${BUILDROOT}
	cp -Rvf ${THISDIR}/${.TARGET:R}/${.TARGET} ${BUILDROOT}/System/Library/Frameworks

CoreData.framework:
	rm -rf ${THISDIR}/CoreData/${.TARGET}
	${MAKE} -C ${THISDIR}/CoreData BUILDROOT=${BUILDROOT} clean
	${MAKE} -C ${THISDIR}/CoreData BUILDROOT=${BUILDROOT}
	cp -Rvf ${THISDIR}/${.TARGET:R}/${.TARGET} ${BUILDROOT}/System/Library/Frameworks

Onyx2D.framework:
	rm -rf ${THISDIR}/Onyx2D/${.TARGET}
	${MAKE} -C ${THISDIR}/Onyx2D BUILDROOT=${BUILDROOT} clean
	${MAKE} -C ${THISDIR}/Onyx2D BUILDROOT=${BUILDROOT}
	cp -Rvf ${THISDIR}/${.TARGET:R}/${.TARGET} ${BUILDROOT}/System/Library/Frameworks

OpenGL.framework:
	rm -rf ${THISDIR}/OpenGL/${.TARGET}
	${MAKE} -C ${THISDIR}/OpenGL BUILDROOT=${BUILDROOT} clean
	${MAKE} -C ${THISDIR}/OpenGL BUILDROOT=${BUILDROOT}
	cp -Rvf ${THISDIR}/${.TARGET:R}/${.TARGET} ${BUILDROOT}/System/Library/Frameworks

CoreGraphics.framework:
	rm -rf ${THISDIR}/CoreGraphics/${.TARGET}
	${MAKE} -C ${THISDIR}/CoreGraphics BUILDROOT=${BUILDROOT} clean
	${MAKE} -C ${THISDIR}/CoreGraphics BUILDROOT=${BUILDROOT}
	cp -Rvf ${THISDIR}/${.TARGET:R}/${.TARGET} ${BUILDROOT}/System/Library/Frameworks
	cp -vf ${THISDIR}/${.TARGET:R}/CGEvent.h ${THISDIR}/AppKit

CoreText.framework:
	rm -rf ${THISDIR}/CoreText/${.TARGET}
	${MAKE} -C ${THISDIR}/CoreText BUILDROOT=${BUILDROOT} clean
	${MAKE} -C ${THISDIR}/CoreText BUILDROOT=${BUILDROOT}
	cp -Rvf ${THISDIR}/${.TARGET:R}/${.TARGET} ${BUILDROOT}/System/Library/Frameworks
	cp -vf ${THISDIR}/${.TARGET:R}/KTFont.h ${THISDIR}/AppKit

QuartzCore.framework:
	rm -rf ${THISDIR}/QuartzCore/${.TARGET}
	${MAKE} -C ${THISDIR}/QuartzCore BUILDROOT=${BUILDROOT} clean
	${MAKE} -C ${THISDIR}/QuartzCore BUILDROOT=${BUILDROOT}
	cp -Rvf ${THISDIR}/${.TARGET:R}/${.TARGET} ${BUILDROOT}/System/Library/Frameworks


AppKit.framework:
	rm -rf ${THISDIR}/AppKit/${.TARGET}
	${MAKE} -C ${THISDIR}/AppKit BUILDROOT=${BUILDROOT} clean build
	cp -Rvf ${THISDIR}/${.TARGET:R}/${.TARGET} ${BUILDROOT}/System/Library/Frameworks

DBusKit.framework:
	rm -rf ${THISDIR}/DBusKit/${.TARGET}
	${MAKE} -C ${THISDIR}/DBusKit BUILDROOT=${BUILDROOT} clean build
	cp -Rvf ${THISDIR}/${.TARGET:R}/${.TARGET} ${BUILDROOT}/System/Library/Frameworks


Cocoa.framework:
	rm -rf ${THISDIR}/Cocoa/${.TARGET}
	${MAKE} -C ${THISDIR}/Cocoa BUILDROOT=${BUILDROOT} clean
	${MAKE} -C ${THISDIR}/Cocoa BUILDROOT=${BUILDROOT}
	cp -Rvf ${THISDIR}/${.TARGET:R}/${.TARGET} ${BUILDROOT}/System/Library/Frameworks

LaunchServices.framework:
	rm -rf ${THISDIR}/LaunchServices/${.TARGET}
	${MAKE} -C ${THISDIR}/LaunchServices BUILDROOT=${BUILDROOT} clean build
	cp -Rvf ${THISDIR}/${.TARGET:R}/${.TARGET} ${BUILDROOT}/System/Library/Frameworks

