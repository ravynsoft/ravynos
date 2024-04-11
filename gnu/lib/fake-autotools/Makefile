INSTDIR=	${OBJTOP}/tmp/legacy/bin
VERSION=	1.16
TOOLS=		automake autoconf autoheader

all: build-tools
build-tools:
	cp -fv ${.CURDIR}/aclocal-${VERSION} ${INSTDIR}
	chmod +x ${INSTDIR}/aclocal-${VERSION}
.for tool in ${TOOLS}
	ln -sf aclocal-${VERSION} ${INSTDIR}/${tool}-${VERSION}
.endfor

build: .PHONY
clean: .PHONY
install: .PHONY
