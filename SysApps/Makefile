THISDIR:= ${.CURDIR}
.if "${CIRRUS_CI}"
SHA!= echo ${CIRRUS_CHANGE_IN_REPO}|cut -c1-7|tr '[a-z]' '[A-Z]'
.else
SHA!= cd ${.CURDIR} && git log -1|head -1|cut -c8-14|tr '[a-z]' '[A-Z]'
.endif

all: clean build
clean:
	rm -rf ${BUILDROOT}/Applications/Utilities/"Install ravynOS.app"
	rm -rf Installer/"Install ravynOS.app"

build: buildInstaller
	mkdir -p ${BUILDROOT}/Applications/Utilities/
	cp -av ${.CURDIR}/Installer/"Install ravynOS.app" ${BUILDROOT}/Applications/Utilities/
	cp -av ${.CURDIR}/foot.app ${BUILDROOT}/Applications/Utilities/

buildInstaller:
	${MAKE} -C ${.CURDIR}/Installer build

