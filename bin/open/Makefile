all: open

open: open.m
	clang ${CFLAGS} -o ${.TARGET} ${.ALLSRC} \
		-I${TOPDIR}/Frameworks/Foundation/Headers \
		-I${TOPDIR}/Frameworks/LaunchServices \
		-I${TOPDIR}/Frameworks \
		-framework LaunchServices -framework Foundation
	mkdir -p ${BUILDROOT}/usr/bin
	${SUDO} install -m755 -o root -g wheel open ${BUILDROOT}/usr/bin

clean:
	rm -f open

