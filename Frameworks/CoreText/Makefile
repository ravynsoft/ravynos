FRAMEWORK=CoreText
.include "../framework.common.mk"

SRCS= CTFont.m KTFont.m
INCS= CoreText.h CoreTextExport.h CTFont.h KTFont.h
RESOURCES=Info.plist 
LDFLAGS+= -Wl,--no-undefined -L${OBJTOP}/Frameworks/Onyx2D/Onyx2D.framework \
	-lOnyx2D -Wl,-R/System/Library/Frameworks/Onyx2D.framework/Versions/A \
	-L${OBJTOP}/Frameworks/CoreGraphics/CoreGraphics.framework -lCoreGraphics \
	-Wl,-R/System/Library/Frameworks/CoreGraphics.framework/Versions/A \
	-L${OBJTOP}/Frameworks/Foundation/Foundation.framework -lFoundation \
	-Wl,-R/System/Library/Frameworks/Foundation.framework/Versions/A \
	-L${OBJTOP}/tmp/usr/lib -lxml2 -lintl -lobjc
WARNS= 1
MK_WERROR= no

SUBDIR+=	gperf .WAIT
SUBDIR+=	fontconfig
LINK_SUBDIR=	fontconfig/src
LINK_EXCLUDE.fontconfig/src= fcarch.o

clean:
	rm -f ${MAKEOBJDIR}/*.o ${MAKEOBJDIR}/*.pico

.include <rvn.framework.mk>

CFLAGS+= ${FMWK_CFLAGS} -I.. -I../libobjc2 -I../Foundation/Headers \
	 -I${.CURDIR}/.. -I${.CURDIR}/fontconfig/fontconfig

fontresources:
	cp -fv ${.CURDIR}/fontconfig/conf.d/*.conf ${FRAMEWORK_DIR}/Versions/A/Resources/
.for f in  \
	10-hinting-medium.conf \
	10-scale-bitmap-fonts.conf \
	10-sub-pixel-bgr.conf \
	10-yes-antialias.conf \
	11-lcdfilter-default.conf \
	20-unhint-small-vera.conf \
	25-unhint-nonlatin.conf \
	30-metric-aliases.conf \
	35-lang-normalize.conf \
	40-nonlatin.conf \
	45-generic.conf \
	45-latin.conf \
	48-spacing.conf \
	49-sansserif.conf \
	50-user.conf \
	51-local.conf \
	60-generic.conf \
	60-latin.conf \
	65-fonts-persian.conf \
	65-khmer.conf \
	65-nonlatin.conf \
	69-unifont.conf \
	70-yes-bitmaps.conf \
	80-delicious.conf \
	90-synthetic.conf 
	ln -sf /System/Library/Frameworks/CoreText.framework/Resources/${f} ${BUILDROOT}/etc/fonts/conf.d/
.endfor

installresources: std_install_hook fontresources
