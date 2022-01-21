# $FreeBSD$

# Build a tiny .so to provide missing symbols to MachO needed
# by the ELF linker

.include <src.opts.mk>

PROG=    	progname.so
MAN=
BINDIR?= 	/usr/lib
SRCS= 		progname.c
INSTALLFLAGS=	-C -b
CFLAGS+= 	-fpic -DPIC  -ffreestanding
LDFLAGS+=	-nostdlib -shared -Wl,-Bsymbolic -Wl,-z,defs

all:
	$(CC) $(CFLAGS) ${LDFLAGS} -o ${PROG} ${.CURDIR}/${SRCS}

install:
	${INSTALL} ${INSTALLFLAGS} -o 0 -g 0 -m 0555 ${PROG} ${DESTDIR}${BINDIR}

clean:
	rm -f ${PROG}

.include <bsd.prog.mk>
