INSTDIR=	${OBJTOP}/tmp/legacy/bin
VERSION=	1.16
TOOLS=		automake autoconf autoheader makeinfo

all: build-tools
build-tools:
	cp -fv ${.CURDIR}/aclocal-${VERSION} ${INSTDIR}
	chmod +x ${INSTDIR}/aclocal-${VERSION}
.for tool in ${TOOLS}
	ln -sf aclocal-${VERSION} ${INSTDIR}/${tool}-${VERSION}
	ln -sf aclocal-${VERSION} ${INSTDIR}/${tool}
.endfor

build: .PHONY
clean: .PHONY
install: .PHONY
includes: .PHONY

