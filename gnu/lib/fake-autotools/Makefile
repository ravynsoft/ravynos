INSTDIR=	${OBJTOP}/tmp/legacy/bin
VERSION=	1.16
TOOLS=		automake autoconf autoheader makeinfo
TOOLSV+=	aclocal-1.15 automake-1.15 autoconf-1.15
TOOLSV+=	autoheader-1.15

all: build-tools
build-tools:
	cp -fv ${.CURDIR}/aclocal-${VERSION} ${INSTDIR}
	chmod +x ${INSTDIR}/aclocal-${VERSION}
.for tool in ${TOOLS}
	ln -sf aclocal-${VERSION} ${INSTDIR}/${tool}-${VERSION}
.endfor
.for tool in ${TOOLS} ${TOOLSV}
	ln -sf aclocal-${VERSION} ${INSTDIR}/${tool}
.endfor

build: .PHONY
clean: .PHONY
install: .PHONY
installconfig: .PHONY
includes: .PHONY
distribute: .PHONY
