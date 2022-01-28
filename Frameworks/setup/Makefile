.include <bsd.lib.mk>

all:
	mkdir -p ${.CURDIR}/../AppKit/Headers/AppKit
	find ${.CURDIR}/../AppKit -name \*.h -exec cp -f {} \
		${.CURDIR}/../AppKit/Headers/AppKit \;
	ln -f ${.CURDIR}/../Foundation/NSException/NSRaise.h \
		${.CURDIR}/../AppKit/Headers/AppKit
	ln -f ${.CURDIR}/../CoreGraphics/CGEvent.h ${.CURDIR}/../AppKit/
	ln -f ${.CURDIR}/../CoreText/KTFont.h ${.CURDIR}/../AppKit/
