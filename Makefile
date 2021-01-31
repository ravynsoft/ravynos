# Master makefile for the Helium OS

TOPDIR := ${.CURDIR}
DISTDIR := ${TOPDIR}/dist
MAKEOBJDIRPREFIX := ${HOME}/obj.${MACHINE}
BSDCONFIG := GENERIC

build: prep freebsd-noclean

world: prep freebsd freebsd-release

prep:
	mkdir -p ${MAKEOBJDIRPREFIX} ${DESTDIR} ${DISTDIR}

${TOPDIR}/freebsd-src/sys/${MACHINE}/compile/${BSDCONFIG}: ${TOPDIR}/freebsd-src/sys/${MACHINE}/conf/${BSDCONFIG}
	mkdir -p ${TOPDIR}/freebsd-src/sys/${MACHINE}/compile/${BSDCONFIG}
	(cd ${TOPDIR}/freebsd-src/sys/${MACHINE}/conf && config ${BSDCONFIG} 
		\ && cd ../compile/${BSDCONFIG} && make depend)

checkout:
	test -d ${TOPDIR}/freebsd-src || \
		(cd ${TOPDIR} && git clone https://github.com/freebsd/freebsd-src.git && \
		cd freebsd-src && git checkout stable/12)

freebsd: checkout ${TOPDIR}/freebsd-src/sys/${MACHINE}/compile/${BSDCONFIG}
	export MAKEOBJDIRPREFIX=${MAKEOBJDIRPREFIX}; make -C ${TOPDIR}/freebsd-src buildkernel buildworld

freebsd-noclean:
	export MAKEOBJDIRPREFIX=${MAKEOBJDIRPREFIX}; make -C ${TOPDIR}/freebsd-src -DNO_CLEAN buildkernel buildworld

freebsd-install:
	export MAKEOBJDIRPREFIX=${MAKEOBJDIRPREFIX}; make -C ${TOPDIR}/freebsd-src installkernel installworld

freebsd-release:
	export MAKEOBJDIRPREFIX=${MAKEOBJDIRPREFIX}; sudo -E make -C ${TOPDIR}/freebsd-src/release release
	cp -fv ${MAKEOBJDIRPREFIX}${TOPDIR}/freebsd-src/${MACHINE}.${MACHINE}/release/ftp/* ${DISTDIR}
	cp -fv ${MAKEOBJDIRPREFIX}${TOPDIR}/freebsd-src/${MACHINE}.${MACHINE}/release/*.iso ${DISTDIR}
	cp -fv ${MAKEOBJDIRPREFIX}${TOPDIR}/freebsd-src/${MACHINE}.${MACHINE}/release/*.img ${DISTDIR}
