GNU_CONFIGURE_ARGS = --prefix=/usr --sysconfdir=/etc --bindir=/bin \
		--sbindir=/sbin --localstatedir=/var
BUILDDIR = ${ROOT_BINARY_DIR}${.CURDIR}

.if "${.MAKE.OS}" == "Linux"
BINGRP = 0
.endif

all: ${TOOLS}/gmake

${BUILDDIR}:
	mkdir -p ${BUILDDIR}

${TOOLS}/gmake: ${BUILDDIR}
	cd ${BUILDDIR} && \
	CC=${HOST_CC} CFLAGS="" OUTDIR=${BUILDDIR} \
	       ${.CURDIR}/configure ${GNU_CONFIGURE_ARGS} \
	       && ./build.sh
	cp -f ${BUILDDIR}/make ${.TARGET}
.if !NO_ROOT
	chown ${BINOWN}:${BINGRP} ${.TARGET}
	chmod ${BINMODE} ${.TARGET}
.endif

.include <bsd.own.mk>

