OBJDIR= ${MAKEOBJDIR}${.CURDIR}

all: build

clean: .PHONY
	rm -rf ${OBJDIR}

build: .PHONY
	mkdir -p ${OBJDIR}
	cd ${OBJDIR}; cmake \
		-DCMAKE_C_FLAGS="-D__RAVYNOS__ -DNO_SELECTOR_MISMATCH_WARNINGS" \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_INSTALL_PREFIX=/usr \
		-DOLDABI_COMPAT=false -DLEGACY_COMPAT=false \
		-S ${.CURDIR}

install:
	${MAKE} -C ${OBJDIR} DESTDIR=${DESTDIR} install

.include <bsd.lib.mk>
