THISDIR:= ${.CURDIR}

all: clean build
clean:
	rm -rf ${BUILDROOT}/System/Library/Colors

build:
	mkdir -p ${BUILDROOT}/System/Library/Colors
	cp -vf *.clr ${BUILDROOT}/System/Library/Colors/
