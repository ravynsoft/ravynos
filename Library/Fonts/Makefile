SYSLIB= /System/Library/Fonts
FILES!= ls ${.CURDIR}/TTF/*.ttf

all: clean build
build:
	mkdir -pv ${BUILDROOT}${SYSLIB}/TTF
.for f in ${FILES}
	${INSTALL} -m 0644 ${f} ${BUILDROOT}${SYSLIB}/TTF
.endfor

clean:
	rm -rf ${BUILDROOT}${SYSLIB}

