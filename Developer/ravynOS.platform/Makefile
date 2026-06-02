# ------------------------------------------------------------------------
#  RAVYNOS BUILD SYSTEM - Developer Platform
# ------------------------------------------------------------------------

.include <bsd.own.mk>

.if "${.MAKE.OS}" == "Linux"
BINGRP = root
DIRGRP = root
.endif

PLATFORM_DIR = ${ROOT_BINARY_DIR}/Developer/Platforms/ravynOS.platform

all: ${PLATFORM_DIR}

${PLATFORM_DIR}:
	mkdir -p ${PLATFORM_DIR}
	cp -f ${.CURDIR}/Info.plist ${.TARGET}/
.if !NO_ROOT
	chown ${DIROWN}:${DIRGRP} ${.TARGET}
	chmod ${DIRMODE} ${.TARGET}
	chown ${BINOWN}:${BINGRP} ${.TARGET}/Info.plist
	chmod 0644 ${.TARGET}/Info.plist
.endif
