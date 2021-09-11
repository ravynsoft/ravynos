all: open

open: open.m
	clang ${CFLAGS} -o ${.TARGET} ${.ALLSRC} \
		-I${TOPDIR}/Frameworks/LaunchServices \
		-framework LaunchServices -framework Foundation

install: open
	mkdir -p ${BUILDROOT}/usr/bin
	${SUDO} install -m755 -o root -g wheel open ${BUILDROOT}/usr/bin

clean:
	rm -f open
