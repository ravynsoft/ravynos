OBJDIR= ${MAKEOBJDIR}${.CURDIR}

all: build

clean: .PHONY
	rm -rf ${OBJDIR}

build: .PHONY
	mkdir -p ${OBJDIR}
	cd ${OBJDIR}; CMAKE_ROOT=/usr cmake \
		-DCMAKE_C_FLAGS="-D__RAVYNOS__ -DNO_SELECTOR_MISMATCH_WARNINGS" \
		-DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_SYSCONFDIR=/etc \
		-DCMAKE_INSTALL_PREFIX=/usr \
		-DOLDABI_COMPAT=false -DLEGACY_COMPAT=false \
		-DCMAKE_SHARED_LINKER_FLAGS="-lstdc++" \
		-S ${.CURDIR}

install:
	${MAKE} -C ${OBJDIR} DESTDIR=${DESTDIR} install

.include <bsd.lib.mk>
