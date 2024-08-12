# Do not lint the GNU stuff. It is all externally maintained and
# lint output is wasteful noise here.

NO_LINT=

BINDIR?=	/bin
SBINDIR?=       /sbin
SYSCONFDIR?=    /etc
CONFDIR?=	/etc
LOCALSTATEDIR?= /var
DIST?= 		base

CONFIGURE_ARGS+=        --prefix=${PREFIX}
CONFIGURE_ARGS+=        --bindir=${BINDIR}
CONFIGURE_ARGS+=        --sbindir=${SBINDIR}
CONFIGURE_ARGS+=        --sysconfdir=${SYSCONFDIR}
CONFIGURE_ARGS+=        --localstatedir=${LOCALSTATEDIR}

.if exists(${DESTDIR}/legacy/bin/gmake)
GMAKE=          ${DESTDIR}/legacy/bin/gmake
.else
GMAKE=          gmake
.endif

gnu_configure:
	${CONFIGURE_ENV} ${.CURDIR}/configure ${CONFIGURE_ARGS}

gnu_build:
	${GMAKE} ${GMAKE_ARGS}

gnu_install:
	${GMAKE} ${GMAKE_ARGS} DESTDIR=${DESTDIR} install

gnu_clean:
	${GMAKE} clean

gnu_distribute:
	${OBJTOP}/tmp/legacy/bin/gmake install \
		DESTDIR=${OBJTOP}/release/dist/${DIST}
