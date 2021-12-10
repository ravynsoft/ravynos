THISDIR:= ${.CURDIR}

all: clean build
clean:
	rm -rf ${BUILDROOT}/System/Library/LaunchDaemons/*.json

build:
	cp -vf *.json ${BUILDROOT}/System/Library/LaunchDaemons/
