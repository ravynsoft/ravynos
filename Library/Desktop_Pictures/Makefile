PICSDIR= /System/Library/Desktop Pictures

all: clean build
build:
	mkdir -pv "${BUILDROOT}${PICSDIR}"
	for pic in *.jpg *.png Attribution.json; do \
	    ${INSTALL} -m 0644 "$$pic" '${BUILDROOT}${PICSDIR}/'; done

clean:
	rm -rf "${BUILDROOT}${PICSDIR}"

